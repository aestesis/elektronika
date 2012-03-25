#include <crtdbg.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "AMPDecoder.h"

////////////////////////////////////////////////////////////////////////////

//#define RDTSC_PROFILE

#ifdef RDTSC_PROFILE

	#include <windows.h>

	static long p_lasttime;
	static long p_frames=0;
	static __int64 p_total=0;
	static __int64 p_header=0;
	static __int64 p_scalefac=0;
	static __int64 p_huffdec=0;
	static __int64 p_dequan=0;
	static __int64 p_stereo=0;
	static __int64 p_antialias=0;
	static __int64 p_hybrid=0;
	static __int64 p_polyphase1=0;
	static __int64 p_polyphase2=0;

	static void __inline profile_set(int) {
		__asm {
			rdtsc
			mov		p_lasttime,eax
		};
	}

	static void __inline profile_add(__int64& counter) {
		long diff;

		__asm {
			rdtsc
			sub		eax,p_lasttime
			mov		diff,eax
		}

		counter += diff;
		p_total += diff;

		__asm {
			rdtsc
			mov		p_lasttime,eax

		}
	}
#else

	#define profile_set(x)
	#define profile_add(x)

#endif

////////////////////////////////////////////////////////////////////////////

int AMPDecoder::L3_GetBits(int bits) {
	unsigned char *ptr = l3bitptr + (l3bitidx>>3);
	unsigned long bitheap;

	bitheap = ((unsigned long)ptr[0] << 24)
			+ ((unsigned long)ptr[1] << 16)
			+ ((unsigned long)ptr[2] << 8)
			+ (unsigned long)ptr[3];

	bitheap <<= l3bitidx & 7;
	bitheap >>= 32-bits;

	l3bitidx += bits;

	return (int)bitheap;
}

void AMPDecoder::L3_GetSideInfo() {
	unsigned char heap[32];
	int gr, ch, i;
	
	l3bitptr = heap;
	l3bitidx = 0;

	if (sideinfo_size != pSource->read(heap, sideinfo_size))
		throw (int)ERR_EOF;

	// process side info

	if (is_mpeg2) {
		sideinfo.main_data_begin		= L3_GetBits(8);
		if (mode == MODE_MONO)
			sideinfo.private_bits		= L3_GetBits(1);
		else
			sideinfo.private_bits		= L3_GetBits(2);
	} else {
		sideinfo.main_data_begin		= L3_GetBits(9);
		if (mode == MODE_MONO)
			sideinfo.private_bits		= L3_GetBits(5);
		else
			sideinfo.private_bits		= L3_GetBits(3);

		// read in scale factor selectors

		for(ch=0; ch<channels; ch++)
			for(i=0; i<4; i++)
				sideinfo.ch[ch].scfsi[i] = L3_GetBits(1);

	}

	// read in data per region/channel

	for(gr=0; gr<(is_mpeg2?1:2); gr++) {
		for(ch=0; ch<channels; ch++) {
			sideinfo.ch[ch].gr[gr].part2_3_length = L3_GetBits(12);
			sideinfo.ch[ch].gr[gr].big_values = L3_GetBits(9);
			sideinfo.ch[ch].gr[gr].global_gain = L3_GetBits(8);

			if (is_mpeg2)
				sideinfo.ch[ch].gr[gr].scalefac_compress = L3_GetBits(9);
			else
				sideinfo.ch[ch].gr[gr].scalefac_compress = L3_GetBits(4);

			sideinfo.ch[ch].gr[gr].window_switching_flag = L3_GetBits(1);
			if (sideinfo.ch[ch].gr[gr].window_switching_flag) {
				sideinfo.ch[ch].gr[gr].block_type = L3_GetBits(2);
				sideinfo.ch[ch].gr[gr].mixed_block_flag = L3_GetBits(1);
				for (i=0; i<2; i++)
					sideinfo.ch[ch].gr[gr].table_select[i] = L3_GetBits(5);
				for (i=0; i<3; i++)
					sideinfo.ch[ch].gr[gr].subblock_gain[i] = L3_GetBits(3);
               
				/* Set region_count parameters since they are implicit in this case. */
            
				if (sideinfo.ch[ch].gr[gr].block_type == 0)
					throw (int)ERR_INTERNAL;	// bad?
				else if (sideinfo.ch[ch].gr[gr].block_type == 2
                     && sideinfo.ch[ch].gr[gr].mixed_block_flag == 0)
					sideinfo.ch[ch].gr[gr].region0_count = 8; /* MI 9; */
				else
					sideinfo.ch[ch].gr[gr].region0_count = 7; /* MI 8; */

				sideinfo.ch[ch].gr[gr].region1_count = 20 -
						sideinfo.ch[ch].gr[gr].region0_count;
			} else {
				for (i=0; i<3; i++)
					sideinfo.ch[ch].gr[gr].table_select[i] = L3_GetBits(5);
				sideinfo.ch[ch].gr[gr].region0_count = L3_GetBits(4);
				sideinfo.ch[ch].gr[gr].region1_count = L3_GetBits(3);
				sideinfo.ch[ch].gr[gr].block_type = 0;
			}

			if (!is_mpeg2)
				sideinfo.ch[ch].gr[gr].preflag = L3_GetBits(1);

			sideinfo.ch[ch].gr[gr].scalefac_scale = L3_GetBits(1);
			sideinfo.ch[ch].gr[gr].count1table_select = L3_GetBits(1);
		}
	}
}

void AMPDecoder::L3_PrereadFrame() {
	fillbits(frame_size);
}

bool AMPDecoder::L3_DecodeFrame() {
	int gr, ch, sb, ss;

	profile_set(0);

	// bring in new bits

	fillbits(frame_size);

	// rewind read point back to beginning of data for this frame

//_RPT2(0,"Layer III: added %ld bytes, rewind to %ld\n", frame_size, sideinfo.main_data_begin + frame_size);

	rewind(sideinfo.main_data_begin + frame_size);

	profile_add(p_header);

	// process granules.
	//
	// 2 for MPEG-1, 1 for MPEG-2

	for (gr=0;gr<(is_mpeg2 ? 1 : 2);gr++) {
		float lr[2][SBLIMIT][SSLIMIT],ro[SBLIMIT][SSLIMIT];
		III_scalefac_t scalefac;
		int sample_end[2];
		int new_end;

		// process each channel in the granule.
		
		for (ch=0; ch<channels; ch++) {
			long int is[SBLIMIT*SSLIMIT];   /* Quantized samples. */
			int part2_start;
			part2_start = tellbits();
			int limit;

			// decode scale factors

			if (is_mpeg2)
				L3_GetScaleFactors2(&scalefac, ch);
			else
				L3_GetScaleFactors1(&scalefac, gr, ch);

			profile_add(p_scalefac);

			// decode samples

			limit = L3_HuffmanDecode(is, ch, gr, part2_start);

			profile_add(p_huffdec);

			// dequantize samples

			sample_end[ch] = L3_DequantizeSample(is, (float *)lr[ch], &scalefac[ch],
				&sideinfo.ch[ch].gr[gr], ch, limit);

			profile_add(p_dequan);
		}

		// process joint stereo in lr

		new_end = L3_Stereo((float (*)[576])lr, &scalefac, &sideinfo.ch[0].gr[gr], sample_end[0], sample_end[1]);

		if (new_end > 0)
			sample_end[0] = sample_end[1] = new_end;

		profile_add(p_stereo);

		float polyPhaseIn[2][SSLIMIT][SBLIMIT];
			
		for (ch=0; ch<channels; ch++) {
			
			// reorder samples; use 'hybridOut' as our out here, to reuse memory

			L3_Reorder((float *)lr[ch],(float *)ro,&sideinfo.ch[ch].gr[gr]);

			// antialiasing

			L3_Antialias(lr[ch], /* Antialias butterflies. */
				&sideinfo.ch[ch].gr[gr]);

			profile_add(p_antialias);

			// Hybrid synthesis.

			L3_Hybrid(lr[ch], polyPhaseIn[ch], ch, &sideinfo.ch[ch].gr[gr], sample_end[ch]);

			profile_add(p_hybrid);
		}

		if (mode == MODE_MONO)
			for(ss=0; ss<18; ss++)
				polyphase(polyPhaseIn[0][ss], NULL, psDest + 32*ss + 576*gr, !!(ss&1));
		else
			for(ss=0; ss<18; ss++)
				polyphase(polyPhaseIn[0][ss], polyPhaseIn[1][ss], psDest + 64*ss + 1152*gr, !!(ss&1));

		profile_add(p_polyphase2);
	}

#ifdef RDTSC_PROFILE

	if (!(++p_frames & 127)) {
		static char buf[256];

		sprintf(buf, "%d frames: total %I64d, header %d%%, scalefac %d%%, huffdec %d%%, dequan %d%%, stereo %d%%, antialias %d%%, hybrid %d%%, poly %d%%/%d%%\n"
				,p_frames
				,p_total
				,(long)((p_header*100)/p_total)
				,(long)((p_scalefac*100)/p_total)
				,(long)((p_huffdec*100)/p_total)
				,(long)((p_dequan*100)/p_total)
				,(long)((p_stereo*100)/p_total)
				,(long)((p_antialias*100)/p_total)
				,(long)((p_hybrid*100)/p_total)
				,(long)((p_polyphase1*100)/p_total)
				,(long)((p_polyphase2*100)/p_total)
				);
		OutputDebugString(buf);
	}
#endif

	// set sample count

	lSampleCount = 1152;
	if (is_mpeg2)
		lSampleCount = 576;

	if (channels>1)
		lSampleCount <<=1;

//	_RPT2(0,"Layer III: terminated with %ld bits left (%ld bytes)\n", tellbits(), tellbits()/8);
			 
	return true;
}

struct {
   int l[5];
   int s[3];} sfbtable = {{0, 6, 11, 16, 21},
                          {0, 6, 12}};
                         
const int slen1[2][16] = {
	{0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
	{0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}
};
 
void AMPDecoder::L3_GetScaleFactors1(III_scalefac_t *scalefac,
							int gr,
							int ch) {

	int sfb, i, window;
	struct gr_info_s *gr_info = &sideinfo.ch[ch].gr[gr];

	if (gr_info->window_switching_flag && (gr_info->block_type == 2)) { 
		if (gr_info->mixed_block_flag) { /* MIXED */ /* NEW - ag 11/25 */

			for (sfb = 0; sfb < 8; sfb++) {
				(*scalefac)[ch].l[sfb] = getbits(slen1[0][gr_info->scalefac_compress]);
			}

			for (sfb = 3; sfb < 6; sfb++)
				for (window=0; window<3; window++) {
					(*scalefac)[ch].s[window][sfb] = getbits(slen1[0][gr_info->scalefac_compress]);
				}

			for (sfb = 6; sfb < 12; sfb++)
				for (window=0; window<3; window++) {
					(*scalefac)[ch].s[window][sfb] = getbits(slen1[1][gr_info->scalefac_compress]);
				}
					
			for (window=0; window<3; window++)
				(*scalefac)[ch].s[window][12] = 0;

		} else {  /* SHORT*/

			for (i=0; i<2; i++) 
				for (sfb = sfbtable.s[i]; sfb < sfbtable.s[i+1]; sfb++)
					for (window=0; window<3; window++) {
						(*scalefac)[ch].s[window][sfb] = getbits(slen1[i][gr_info->scalefac_compress]);
					}

			for (window=0; window<3; window++)
				(*scalefac)[ch].s[window][12] = 0;
		}
	} else {   /* LONG types 0,1,3 */

		for (i=0; i<2; i++) {
			if ((sideinfo.ch[ch].scfsi[i] == 0) || (gr == 0))
				for (sfb = sfbtable.l[i]; sfb < sfbtable.l[i+1]; sfb++) {
					(*scalefac)[ch].l[sfb] = getbits(slen1[0][gr_info->scalefac_compress]);
				}
		}
		for (i=2; i<4; i++) {
			if ((sideinfo.ch[ch].scfsi[i] == 0) || (gr == 0))
				for (sfb = sfbtable.l[i]; sfb < sfbtable.l[i+1]; sfb++) {
					(*scalefac)[ch].l[sfb] = getbits(slen1[1][gr_info->scalefac_compress]);
				}
		}
		(*scalefac)[ch].l[21] = 0; 
		(*scalefac)[ch].l[22] = 0; 
	}

}

void AMPDecoder::L3_GetScaleFactors2(III_scalefac_t *scalefac, int ch) {
	static const int sfbblockindex[6][3][4]={
		{{ 6, 5, 5, 5},{ 9, 9, 9, 9},{ 6, 9, 9, 9}},
		{{ 6, 5, 7, 3},{ 9, 9,12, 6},{ 6, 9,12, 6}},
		{{11,10, 0, 0},{18,18, 0, 0},{15,18, 0, 0}},
		{{ 7, 7, 7, 0},{12,12,12, 0},{ 6,15,12, 0}},
		{{ 6, 6, 6, 3},{12, 9, 9, 6},{ 6,12, 9, 6}},
		{{ 8, 8, 5, 0},{15,12, 9, 0},{ 6,18, 9, 0}}
	};

	int sb[54];
	struct gr_info_s *gi=&(sideinfo.ch[ch].gr[0]);
	struct III_scalefac1_t *sf=(&(*scalefac)[ch]);

	{
		int blocktypenumber,sc;
		int blocknumber;
		
		if(gi->block_type==2)blocktypenumber=1+gi->mixed_block_flag;
		else blocktypenumber=0;
		
		sc=gi->scalefac_compress;
		if(!((mode_ext==1 || mode_ext==3) && (ch==1)))
		{
			if(sc<400) {
				slen[0]=(sc>>4)/5;
				slen[1]=(sc>>4)%5;
				slen[2]=(sc%16)>>2;
				slen[3]=(sc%4);
				gi->preflag=0;
				blocknumber=0;
			} else if(sc<500) {
				sc-=400;
				slen[0]=(sc>>2)/5;
				slen[1]=(sc>>2)%5;
				slen[2]=sc%4;
				slen[3]=0;
				gi->preflag=0;
				blocknumber=1;
			} else {
				sc-=500;
				slen[0]=sc/3;
				slen[1]=sc%3;
				slen[2]=0;
				slen[3]=0;
				gi->preflag=1;
				blocknumber=2;
			}
		} else {
			sc>>=1;
			if(sc<180) {
				slen[0]=sc/36;
				slen[1]=(sc%36)/6;
				slen[2]=(sc%36)%6;
				slen[3]=0;
				gi->preflag=0;
				blocknumber=3;
			} else if(sc<244) {
				sc-=180;
				slen[0]=(sc%64)>>4;
				slen[1]=(sc%16)>>2;
				slen[2]=sc%4;
				slen[3]=0;
				gi->preflag=0;
				blocknumber=4;
			} else {
				sc-=244;
				slen[0]=sc/3;
				slen[1]=sc%3;
				slen[2]=
					slen[3]=0;
				gi->preflag=0;
				blocknumber=5;
			}
		}
		
		{
			int i,j,k;
			const int *si;
			
			scale_block_indexes=sfbblockindex[blocknumber][blocktypenumber];
			for(i=0;i<45;i++)sb[i]=0;
			
			for(k=i=0;i<4;i++)
				for(j=0;j<scale_block_indexes[i];j++,k++)
					if(slen[i]==0)sb[k]=0;
					else sb[k]=getbits(slen[i]);
		}
	}
	
	
	{
		int sfb,window;
		int k=0;
		
		if(gi->window_switching_flag && (gi->block_type==2))
		{
			if(gi->mixed_block_flag)
			{
				for(sfb=0;sfb<8;sfb++)sf->l[sfb]=sb[k++];
				sfb=3;
			}
			else sfb=0;
			
			for(;sfb<12;sfb++)
				for(window=0;window<3;window++)
					sf->s[window][sfb]=sb[k++];
				
				sf->s[0][12]=sf->s[1][12]=sf->s[2][12]=0;
		}
		else
		{
			for(sfb=0;sfb<21;sfb++)
				sf->l[sfb]=sb[k++];
			sf->l[21]=sf->l[22]=0;
		}
	}
}

static struct  {
	int l[23];
	int s[14];} sfBandIndex[2][3] =   
	{
		{
			{{0,4,8,12,16,20,24,30,36,44,52,62,74,90,110,134,162,196,238,288,342,418,576},
			 {0,4,8,12,16,22,30,40,52,66,84,106,136,192}},
			{{0,4,8,12,16,20,24,30,36,42,50,60,72,88,106,128,156,190,230,276,330,384,576},
			 {0,4,8,12,16,22,28,38,50,64,80,100,126,192}},
			{{0,4,8,12,16,20,24,30,36,44,54,66,82,102,126,156,194,240,296,364,448,550,576},
			 {0,4,8,12,16,22,30,42,58,78,104,138,180,192}},
		},
		{
			{{0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
			 {0,4,8,12,18,24,32,42,56,74,100,132,174,192}},
			{{0,6,12,18,24,30,36,44,54,66,80,96,114,136,162,194,232,278,330,394,464,540,576},
			 {0,4,8,12,18,26,36,48,62,80,104,136,180,192}},
			{{0,6,12,18,24,30,36,44,54,66,80,96,116,140,168,200,238,284,336,396,464,522,576},
			 {0,4,8,12,18,26,36,48,62,80,104,134,174,192}},
		}
   };

int AMPDecoder::L3_HuffmanDecode(long int is[SBLIMIT*SSLIMIT],
						int ch,
						int gr,
						int part2_start) {

	int i, x, y;
	int v, w;
	int h;
	int region1Start;
	int region2Start;
	int bt = sideinfo.ch[ch].gr[gr].window_switching_flag && (sideinfo.ch[ch].gr[gr].block_type == 2);
		
	/* Find region boundary for short block case. */
	
	if ( (sideinfo.ch[ch].gr[gr].window_switching_flag) && 
        (sideinfo.ch[ch].gr[gr].block_type == 2) ) { 
		
		/* Region2. */
		
		region1Start = 36;  /* sfb[9/3]*3=36 */
		region2Start = 576; /* No Region2 for short block case. */
	}
	
	
	else {          /* Find region boundary for long block case. */
		
		region1Start = sfBandIndex[is_mpeg2][sr_index].l[sideinfo.ch[ch].gr[gr].region0_count + 1]; /* MI */
		region2Start = sfBandIndex[is_mpeg2][sr_index].l[sideinfo.ch[ch].gr[gr].region0_count + sideinfo.ch[ch].gr[gr].region1_count + 2]; /* MI */
	}
	
//_RPT1(0,"%d bits left (bigvalues)\n", tellbits());
	
	/* Read bigvalues area. */

	if (region1Start > sideinfo.ch[ch].gr[gr].big_values*2)
		region1Start = sideinfo.ch[ch].gr[gr].big_values*2;

	if (region2Start > sideinfo.ch[ch].gr[gr].big_values*2)
		region2Start = sideinfo.ch[ch].gr[gr].big_values*2;

	h = sideinfo.ch[ch].gr[gr].table_select[0];

	if (h)
		L3_GetHuffmanBig(h, &is[0], region1Start);
	else
		memset(is, 0, sizeof(is[0])*region1Start);

	i = region1Start;

	if (i < region2Start) {
		h = sideinfo.ch[ch].gr[gr].table_select[1];

		if (h)
			L3_GetHuffmanBig(h, &is[i], region2Start - i);
		else
			memset(is+i, 0, sizeof(is[0])*(region2Start-i));
	}

	i = region2Start;

	if (i < sideinfo.ch[ch].gr[gr].big_values*2) {
		h = sideinfo.ch[ch].gr[gr].table_select[2];

		if (h)
			L3_GetHuffmanBig(h, &is[i], sideinfo.ch[ch].gr[gr].big_values*2 - i);
		else
			memset(is+i, 0, sizeof(is[0])*(sideinfo.ch[ch].gr[gr].big_values*2-i));
	}

	i = sideinfo.ch[ch].gr[gr].big_values*2;

	/* Read count1 area. */

	if (sideinfo.ch[ch].gr[gr].count1table_select) {
		int count;

		// fixed 4-bit table

		i = L3_GetHuffmanCount1_33(&is[i], i, part2_start - sideinfo.ch[ch].gr[gr].part2_3_length);

	} else {
		// vbr huffman table

		i = L3_GetHuffmanCount1_32(&is[i], i, part2_start - sideinfo.ch[ch].gr[gr].part2_3_length);
	}

//	_RPT2(0,"%d big values, %d small values\n", sideinfo.ch[ch].gr[gr].big_values*2, i - sideinfo.ch[ch].gr[gr].big_values*2);


	if (tellbits() < part2_start - sideinfo.ch[ch].gr[gr].part2_3_length)
		i -= 4;

//_RPT2(0,"%d bits left, attempting rewind to %d\n", tellbits(), part2_start - sideinfo.ch[ch].gr[gr].part2_3_length);

	rewindbits(part2_start - sideinfo.ch[ch].gr[gr].part2_3_length);
	
	/* Zero out rest. */

	return i;
}

static const int pretab[22] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0};

// pow43[15 + i] = abs(i) ^ (4/3) * sign(i)

static const double pow43[32]={
	-36.99318111495705,
	-33.74199169845322,
	-30.56735094036985,
	-27.47314182127996,
	-24.46378099626247,
	-21.54434690031884,
	-18.72075440746714,
	-16.00000000000000,
	-13.39051827940672,
	-10.90272355699284,
	-8.549879733383484,
	-6.349604207872798,
	-4.326748710922225,
	-2.519842099789746,
	-1.000000000000000,
	 0,
	 1.000000000000000,
	 2.519842099789746,
	 4.326748710922225,
	 6.349604207872798,
	 8.549879733383484,
	 10.90272355699284,
	 13.39051827940672,
	 16.00000000000000,
	 18.72075440746714,
	 21.54434690031884,
	 24.46378099626247,
	 27.47314182127996,
	 30.56735094036985,
	 33.74199169845322,
	 36.99318111495705,
	 40.31747359663594,
};

int AMPDecoder::L3_DequantizeSample(long int is[SBLIMIT*SSLIMIT],
							float xr[SBLIMIT*SSLIMIT],
							const III_scalefac1_t *scaleptr,
							struct gr_info_s *gr_info,
							int ch, int limit) {

	int ind,cb=0,sfreq=sr_index;
	int stereo = channels;
	int next_cb_boundary, next_cb2_boundary, cb_width, sign;
	int last_nonzero = -1;


	// We optimize all the pow() out of this routine.
	//
	//	gr_info->global_gain:		0-255 (8 bits)
	//	gr_info->scalefac_scale:	0 or 1
	//	gr_info->preflag:			0 or 1
	//	gr_info->subblock_gain:		0-7 (3 bits)
	//	scalefac[ch].s[]:			0-15 (4 bits max)
	//	scalefac[ch].l[]:			0-15 (4 bits max)
	//	pretab[]:					0-3
	//
	//	Let f(x) = 2^(0.25*x), where f(a)f(b) = f(a+b).
	//
	//	xr0 = f(gr_info->global_gain - 210);
	//		range: f(-210) to f(45).
	//
	// short blocks:
	//
	//		xr = xr0 * f(-8.0 * gr_info->subblock_gain[(ind - cb_begin)/cb_width]
	//				-2.0 * (1.0+gr_info->scalefac_scale)
	//			* (*scalefac)[ch].s[(ind - cb_begin)/cb_width][cb]);
	//
	//		range: f(-116) to f(0).
	//
	// LONG block types 0,1,3 & 1st 2 subbands of switched blocks:
	//
	//		xr = xr0 * f(-2.0 * (1.0+gr_info->scalefac_scale)
	//			* ((*scalefac)[ch].l[cb]
	//			+ gr_info->preflag * pretab[cb]));
	//
	//		range: f(-72) to f(0).
	//
	//	CONCLUSION:
	//
	//		Necessary range is f(-326) to f(45).
	//
	//	We negate the sign, so it's really f(-45) to f(326).  Scaling
	//	to 16-bit PCM is also done here.

	static float pow2tbl[326+45+1];
	static bool init_flag = true;
	double xr_val;
	int		scalefac_shift = 1+gr_info->scalefac_scale;
	int		preflag_mask = gr_info->preflag ? ~0 : 0;
	int		cb_low, cb_high;

	if (init_flag) {
		init_flag = false;

		for(int i=-45; i<=326; i++)
			pow2tbl[i+45] = pow(2.0, -0.25 * i) * 16384.0;//32768.0;
	}



	// global_gain ranges from 0-255 (8 bits)

	float *xr0_ptr = pow2tbl + 45 + 210 - (int)gr_info->global_gain;
	
	/* choose correct scalefactor band per block type, initalize boundary */
	
	bool	fShortBlock		= gr_info->window_switching_flag && (gr_info->block_type == 2);
	int		iScaleThreshold = gr_info->mixed_block_flag ? 2*SSLIMIT : 0;

	next_cb_boundary = 0;
	cb = -1;
		
	/* apply formula per block type */

	for (ind=0 ; ind < limit ; ind++) {

		if ( ind == next_cb_boundary)  { /* Adjust critical band boundary */

			if (fShortBlock) {

				if (gr_info->mixed_block_flag) {
					if (ind == sfBandIndex[is_mpeg2][sfreq].l[8])  {
						next_cb_boundary=sfBandIndex[is_mpeg2][sfreq].s[4]*3; 
						cb = 3;
						cb_width = sfBandIndex[is_mpeg2][sfreq].s[cb+1] - sfBandIndex[is_mpeg2][sfreq].s[cb];
						
					} else if (ind < sfBandIndex[is_mpeg2][sfreq].l[8]) {

						next_cb_boundary = sfBandIndex[is_mpeg2][sfreq].l[(++cb)+1];

					} else {
						next_cb_boundary = sfBandIndex[is_mpeg2][sfreq].s[(++cb)+1]*3;
						cb_width = sfBandIndex[is_mpeg2][sfreq].s[cb+1] - sfBandIndex[is_mpeg2][sfreq].s[cb];
					}   
				} else {
					next_cb_boundary = sfBandIndex[is_mpeg2][sfreq].s[(++cb)+1]*3;
					cb_width = sfBandIndex[is_mpeg2][sfreq].s[cb+1] - sfBandIndex[is_mpeg2][sfreq].s[cb];
				} 

			} else /* long blocks */
				next_cb_boundary = sfBandIndex[is_mpeg2][sfreq].l[(++cb)+1];

			// block type 2?

			if (fShortBlock && ind >= iScaleThreshold) {
				xr_val = xr0_ptr[8*gr_info->subblock_gain[0] + (scaleptr->s[0][cb] << scalefac_shift)];
				cb_high = 0;
				next_cb2_boundary = ind + cb_width;
			} else {

				// long block, or first two subbands of switched blocks

				xr_val = xr0_ptr[(scaleptr->l[cb] + (pretab[cb] & preflag_mask)) << scalefac_shift];
				next_cb2_boundary = -1;
			}
		}

		// Do we need to switch the scaling factor for short blocks?

		if (ind == next_cb2_boundary) {
			++cb_high;
			xr_val = xr0_ptr[8*gr_info->subblock_gain[cb_high] + (scaleptr->s[cb_high][cb] << scalefac_shift)];
			next_cb2_boundary += cb_width;
		}
		
		/* Scale quantized value. */

		if (is[ind]) {
			int ind2 = 15 + is[ind];
			double y;

			if (ind2 & 0xffffffe0) {

				if (ind2<0)
					y = -pow((double)-is[ind], (double)(4.0/3.0));
				else
					y = pow((double)is[ind], (double)(4.0/3.0));

			} else {

				y = pow43[ind2];

			}

			xr[ind] = xr_val * y;

			last_nonzero = ind;
		} else {
			xr[ind] = 0.0;
		}
	}

	while(ind<SBLIMIT*SSLIMIT)
		xr[ind++] = 0.0;

	return last_nonzero+1;
}

// qbasic is the ultimate table generator
//
//	Intensity stereo for MPEG-1:
//
//	is_ratio[i][0] = 1/(1+y)
//	is_ratio[i][1] = y/(1+y)
//
//	where y=tan(i * (3.1415926535897932 / 12));
//
//	Alternatively (thank you FreeAmp, blah Fraunhofer):
//
//	is_ratio[i][0] = s/(s+c)
//	is_ratio[i][1] = c/(s+c)
//
//	where	s = sin(i * pi/12)
//			c = cos(i * pi/12)

static const double is_ratio1[16][2]={

#define T(y) { (y)/(1.0+(y)), 1.0/(1.0+(y)) }

	T(0),
	T( .2679491924311227),
	T( .5773502691896257),
	T( .9999999999999999),
	T( 1.732050807568877),
	T( 3.732050807568877),
	T( 1.632455227761907e+16),
	T(-3.732050807568879),
	T(-1.732050807568878),

//	T(-1),	stupid asymptotes
	{ -1000000, 1000000 },

	T(-.5773502691896260),
	T(-.2679491924311228),
	T(-1.22514845490862e-16),
	T( .2679491924311226),
	T( .5773502691896256),
	T( .9999999999999997),

#undef T
};

// Intensity stereo for MPEG-2:
//
//

int AMPDecoder::L3_Stereo(
				float lr[2][SBLIMIT*SSLIMIT],
				const III_scalefac_t *scalefac,
				struct gr_info_s *gr_info,
				int nz0, int nz1) {

	int sfreq = sr_index;
	int ms_stereo = (mode == MODE_JOINTSTEREO) && (mode_ext & 0x2); 
	int i_stereo = (mode == MODE_JOINTSTEREO) && (mode_ext & 0x1);
	int js_bound;  /* frequency line that marks the beggining of the zero part */  
	int sfb,next_sfb_boundary;
	int i,j,k,sb,ss,ch,is_pos[576]; 

	static float is_ratio2[2][2][64][2];
	static bool is_ratio2_inited = false;

	if (!is_ratio2_inited) {
		int k, n;
		double t;
		int intensity_scale, ms_mode, sf, sflen;
		float ms_factor[2];
		
		
		ms_factor[0] = 1.0;
		ms_factor[1] = (float) sqrt(2.0);
		
		/* intensity stereo MPEG2 */
		/* lr2[intensity_scale][ms_mode][sflen_offset+sf][left/right] */
		
		for (intensity_scale = 0; intensity_scale < 2; intensity_scale++)
		{
			t = pow(2.0, -0.25 * (1 + intensity_scale));
			for (ms_mode = 0; ms_mode < 2; ms_mode++)
			{
				
				n = 1;
				k = 0;
				for (sflen = 0; sflen < 6; sflen++)
				{
					for (sf = 0; sf < (n - 1); sf++, k++)
					{
						if (sf == 0)
						{
							is_ratio2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * 1.0f;
							is_ratio2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * 1.0f;
						}
						else if ((sf & 1))
						{
							is_ratio2[intensity_scale][ms_mode][k][0] =
								(float) (ms_factor[ms_mode] * pow(t, (sf + 1) / 2));
							is_ratio2[intensity_scale][ms_mode][k][1] = ms_factor[ms_mode] * 1.0f;
						}
						else
						{
							is_ratio2[intensity_scale][ms_mode][k][0] = ms_factor[ms_mode] * 1.0f;
							is_ratio2[intensity_scale][ms_mode][k][1] =
								(float) (ms_factor[ms_mode] * pow(t, sf / 2));
						}
					}
					
					/* illegal is_pos used to do ms processing */
					if (ms_mode == 0)
					{			/* ms_mode = 0 */
						is_ratio2[intensity_scale][ms_mode][k][0] = 1.0f;
						is_ratio2[intensity_scale][ms_mode][k][1] = 0.0f;
					}
					else
					{
						/* ms_mode = 1, in is bands is routine does ms processing */
						is_ratio2[intensity_scale][ms_mode][k][0] = 1.0f;
						is_ratio2[intensity_scale][ms_mode][k][1] = 1.0f;
					}
					k++;
					n = n + n;
				}
			}
		}

		is_ratio2_inited = true;
	}

	if (channels<2) {
		return -1;
	}
	
	if (i_stereo) {

		/* intialization */
		for ( i=0; i<576; i++ )
			is_pos[i] = 7;

		if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
			if( gr_info->mixed_block_flag ) {
				int max_sfb = 0;
				
				for ( j=0; j<3; j++ ) {
					int sfbcnt;
					
					sfbcnt = 2;
					
					for( sfb=12; sfb >=3; sfb-- ) {
						int lines;
						
						lines = sfBandIndex[is_mpeg2][sfreq].s[sfb+1]-sfBandIndex[is_mpeg2][sfreq].s[sfb];
						i = 3*sfBandIndex[is_mpeg2][sfreq].s[sfb] + (j+1) * lines - 1;
						
						while ( lines > 0 ) {
							if ( lr[1][i] != 0.0 ) {
								sfbcnt = sfb;
								goto found_nonzero;
							}
							lines--;
							i--;
						}
					}
found_nonzero:
					sfb = sfbcnt + 1;
					
					if ( sfb > max_sfb )
						max_sfb = sfb;
					
					while( sfb<12 )	{
						sb = sfBandIndex[is_mpeg2][sfreq].s[sfb+1]-sfBandIndex[is_mpeg2][sfreq].s[sfb];
						i = 3*sfBandIndex[is_mpeg2][sfreq].s[sfb] + j * sb;
						for ( ; sb > 0; sb--) {
							is_pos[i] = (*scalefac)[1].s[j][sfb];
							i++;
						}
						sfb++;
					}
					sb = sfBandIndex[is_mpeg2][sfreq].s[11]-sfBandIndex[is_mpeg2][sfreq].s[10];
					sfb = 3*sfBandIndex[is_mpeg2][sfreq].s[10] + j * sb;
					sb = sfBandIndex[is_mpeg2][sfreq].s[12]-sfBandIndex[is_mpeg2][sfreq].s[11];
					i = 3*sfBandIndex[is_mpeg2][sfreq].s[11] + j * sb;
					
					for ( ; sb > 0; sb-- ) {
						is_pos[i] = is_pos[sfb];
						i++;
					}
				}
				if ( max_sfb <= 3 ) {
					i = 2;
					ss = 17;
					sb = -1;
					while ( i >= 0 ) {
						if ( lr[1][i*SSLIMIT+ss] != 0.0 ) {
							sb = i*18+ss;
							i = -1;
						} else {
							ss--;
							if ( ss < 0 ) {
								i--;
								ss = 17;
							}
						}
					}
					i = 0;

					while ( sfBandIndex[is_mpeg2][sfreq].l[i] <= sb )
						i++;

					sfb = i;
					i = sfBandIndex[is_mpeg2][sfreq].l[i];

					for ( ; sfb<8; sfb++ ) {
						sb = sfBandIndex[is_mpeg2][sfreq].l[sfb+1]-sfBandIndex[is_mpeg2][sfreq].l[sfb];
						for ( ; sb > 0; sb--) {
							is_pos[i] = (*scalefac)[1].l[sfb];
							i++;
						}
					}
				}
			} else {
				for ( j=0; j<3; j++ ) {
					int sfbcnt;
					sfbcnt = -1;
					
					for( sfb=12; sfb >=0; sfb-- ) {
						int lines;
						lines = sfBandIndex[is_mpeg2][sfreq].s[sfb+1]-sfBandIndex[is_mpeg2][sfreq].s[sfb];
						i = 3*sfBandIndex[is_mpeg2][sfreq].s[sfb] + (j+1) * lines - 1;
						
						while ( lines > 0 ) {
							if ( lr[1][i] != 0.0 ) {
								sfbcnt = sfb;
								sfb = -10;
								lines = -10;
							}
							lines--;
							i--;
						}
					}
					sfb = sfbcnt + 1;
					
					while( sfb<12 ) {
						sb = sfBandIndex[is_mpeg2][sfreq].s[sfb+1]-sfBandIndex[is_mpeg2][sfreq].s[sfb];
						i = 3*sfBandIndex[is_mpeg2][sfreq].s[sfb] + j * sb;
						for ( ; sb > 0; sb--) {
							is_pos[i] = (*scalefac)[1].s[j][sfb];
														
							i++;
						}
						sfb++;
					}
	
					sb = sfBandIndex[is_mpeg2][sfreq].s[11]-sfBandIndex[is_mpeg2][sfreq].s[10];
					sfb = 3*sfBandIndex[is_mpeg2][sfreq].s[10] + j * sb;
					sb = sfBandIndex[is_mpeg2][sfreq].s[12]-sfBandIndex[is_mpeg2][sfreq].s[11];
					i = 3*sfBandIndex[is_mpeg2][sfreq].s[11] + j * sb;

					for ( ; sb > 0; sb-- ) {
						is_pos[i] = is_pos[sfb];
						i++;
					}
				}
			}
		} else {
			i = 31;
			ss = 17;
			sb = 0;

			while ( i >= 0 ) {
				if ( lr[1][i*SSLIMIT+ss] != 0.0 ) {
					sb = i*18+ss;
					i = -1;
				} else {
					ss--;
					if ( ss < 0 ) {
						i--;
						ss = 17;
					}
				}
			}
			i = 0;

			while ( sfBandIndex[is_mpeg2][sfreq].l[i] <= sb )
				i++;
			sfb = i;
			i = sfBandIndex[is_mpeg2][sfreq].l[i];

			for ( ; sfb<21; sfb++ )	{
				sb = sfBandIndex[is_mpeg2][sfreq].l[sfb+1] - sfBandIndex[is_mpeg2][sfreq].l[sfb];
				for ( ; sb > 0; sb--) {
					is_pos[i] = (*scalefac)[1].l[sfb];
					i++;
				}
			}
			sfb = sfBandIndex[is_mpeg2][sfreq].l[20];
			for ( sb = 576 - sfBandIndex[is_mpeg2][sfreq].l[21]; sb > 0; sb-- )
			{
				is_pos[i] = is_pos[sfb];
				i++;
			}
		}

		if (is_mpeg2) {	// BROKEN, esp with short blocks!!
			int ms_mode = !!(mode_ext & 2);
			int intensity_scale = gr_info->scalefac_compress&1;
			int is_max[54];

			// figure out illegal values for each scale factor band

			k=0;
			for(i=0; i<4; i++) {
				for(j=0; j<scale_block_indexes[i]; j++)
					is_max[k++] = (1<<slen[i])-1;
			}
			while(k<54) is_max[k++]=0;

			sfb=0;
			next_sfb_boundary = sfBandIndex[1][sfreq].l[1];

			for(i=0; i<SBLIMIT*SSLIMIT; i++) {
				double x = lr[0][i];

				lr[0][i] = x*is_ratio2[intensity_scale][ms_mode][is_max[sfb]+is_pos[i]][0];
				lr[1][i] = x*is_ratio2[intensity_scale][ms_mode][is_max[sfb]+is_pos[i]][1];

				if (i == next_sfb_boundary)
					next_sfb_boundary = sfBandIndex[1][sfreq].l[++sfb + 1];
			}
		} else {
			for(i=0; i<SBLIMIT*SSLIMIT; i++) {
				if ( is_pos[i] == 7 ) {
					if ( ms_stereo ) {
						double a = lr[0][i];
						double b = lr[1][i];

						lr[0][i] = (a+b)*0.707106781186547524400844362104849; ///1.41421356;
						lr[1][i] = (a-b)*0.707106781186547524400844362104849; ///1.41421356;
					}
				} else {
					double x = lr[0][i];

					lr[0][i] = x*is_ratio1[is_pos[i]][0];
					lr[1][i] = x*is_ratio1[is_pos[i]][1];
				}
			}
		}

		return nz0>nz1 ? nz0 : nz1;
	} else if (ms_stereo) {
   
		if (nz1 > nz0)
			nz0 = nz1;

		for(i=0; i<nz0; i++) {
			double a = lr[0][i];
			double b = lr[1][i];

			lr[0][i] = (a+b)*0.707106781186547524400844362104849; ///1.41421356;
			lr[1][i] = (a-b)*0.707106781186547524400844362104849; ///1.41421356;
		}

		return nz0;
	}

	return -1;
}

void AMPDecoder::L3_Reorder (float xr[SBLIMIT*SSLIMIT],
			float ro[SBLIMIT*SSLIMIT],
			struct gr_info_s *gr_info) {

	int sfreq=sr_index;
	int sfb, sfb_start, sfb_lines;
	int sb, ss, window, freq, src_line, des_line;
	int i;
	
	if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
		if (gr_info->mixed_block_flag) {
			/* NO REORDER FOR LOW 2 SUBBANDS */
			for(i=0; i<2*SSLIMIT; i++)
				ro[i] = xr[i];

			for(i=2*SSLIMIT; i<SBLIMIT*SSLIMIT; i++)
				ro[i] = 0;
		
			/* REORDERING FOR REST SWITCHED SHORT */

			sfb_start=sfBandIndex[is_mpeg2][sfreq].s[3];
			sfb_lines=sfBandIndex[is_mpeg2][sfreq].s[4] - sfb_start;
			for(sfb=3; sfb < 13; sfb++) {
				for(window=0; window<3; window++)
					for(freq=0;freq<sfb_lines;freq++) {
						src_line = sfb_start*3 + window*sfb_lines + freq; 
						des_line = (sfb_start*3) + window + (freq*3);
						ro[des_line] = xr[src_line];
					}

				sfb_start=sfBandIndex[is_mpeg2][sfreq].s[sfb];
				sfb_lines=sfBandIndex[is_mpeg2][sfreq].s[sfb+1] - sfb_start;
			}
		} 
		else {  /* pure short */
			for(i=0; i<SBLIMIT*SSLIMIT; i++)
				ro[i] = 0;

			sfb_start = 0;
			sfb_lines = sfBandIndex[is_mpeg2][sfreq].s[1];

			for(sfb=0; sfb < 13; sfb++) {
				for(window=0; window<3; window++)
					for(freq=0;freq<sfb_lines;freq++) {
						src_line = sfb_start*3 + window*sfb_lines + freq; 
						des_line = (sfb_start*3) + window + (freq*3);
						ro[des_line] = xr[src_line];
					}

				sfb_start = sfBandIndex[is_mpeg2][sfreq].s[sfb];
				sfb_lines = sfBandIndex[is_mpeg2][sfreq].s[sfb+1] - sfb_start;
			}
		}

		memcpy(xr, ro, sizeof(xr[0])*SBLIMIT*SSLIMIT);
	}
//	else {   /*long blocks */
//		memcpy(ro, xr, sizeof(xr[0])*SBLIMIT*SSLIMIT);
////		for(i=0; i<SBLIMIT*SSLIMIT; i++)
////				ro[i] = xr[i];
//	}
}

void AMPDecoder::L3_Antialias(float xr[SBLIMIT][SSLIMIT],
					struct gr_info_s *gr_info) {

	static const double cs[8]={
		0.8574929166930334,
		0.8817419876991212,
		0.9496286453969656,
		0.9833145920724280,
		0.9955178161793186,
		0.9991605581318322,
		0.9998991952431355,
		0.9999931550702761,
	};

	static const double ca[8]={
		-0.5144957704600444,
		-0.4717319865436337,
		-0.3133774654334998,
		-0.1819132018778039,
		-0.09457419135028555,
		-0.04096558401494271,
		-0.01419856866483041,
		-0.003699974674877601,
	};

	double        bu,bd;  /* upper and lower butterfly inputs */
	int           ss,sb,sblim;
	
	/* clear all inputs */  
	
//    for(sb=0;sb<SBLIMIT;sb++)
//		for(ss=0;ss<SSLIMIT;ss++)
//			hybridIn[sb][ss] = xr[sb][ss];

/*	if  (gr_info->window_switching_flag && (gr_info->block_type == 2) &&
		!gr_info->mixed_block_flag ) {

		return;
	}*/
	
	if ( gr_info->window_switching_flag && gr_info->mixed_block_flag &&
		(gr_info->block_type == 2))
		sblim = 1;
	else
		sblim = SBLIMIT-1;

   /* 31 alias-reduction operations between each pair of sub-bands */
   /* with 8 butterflies between each pair                         */

   for(sb=0;sb<sblim;sb++)
	   for(ss=0;ss<8;ss++) {
		   bu = xr[sb][17-ss];
		   bd = xr[sb+1][ss];
		   xr[sb][17-ss] = (bu * cs[ss]) - (bd * ca[ss]);
		   xr[sb+1][ss] = (bd * cs[ss]) + (bu * ca[ss]);
	   }  
}

extern void inv_mdct(float *in,
				float out[SSLIMIT][SBLIMIT],
				float *prevblock,
				int block_type);

void AMPDecoder::L3_Hybrid(float fsIn[SBLIMIT][SSLIMIT],
				float fsOut[SSLIMIT][SBLIMIT],
				int ch,
				struct gr_info_s *gr_info,
				int nonzero_entries) {

	int sb;
	float rawout[36];
	int bt;
	int bands;

	bands = (nonzero_entries + SSLIMIT-1) / SSLIMIT;

	for(sb=0; sb<bands; sb++) {
		bt = (gr_info->window_switching_flag && gr_info->mixed_block_flag &&
			(sb < 2)) ? 0 : gr_info->block_type; 

		inv_mdct(fsIn[sb], (float(*)[SBLIMIT])&fsOut[0][sb], prevblck[ch][sb], bt);
	}

	for(; sb<SBLIMIT; sb++) {
		for(int i=0; i<18; i++) {
			fsOut[i][sb] = prevblck[ch][sb][i];
			prevblck[ch][sb][i] = 0.0;
		}
	}
}
