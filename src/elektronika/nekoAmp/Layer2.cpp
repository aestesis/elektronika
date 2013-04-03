#include <crtdbg.h>
#include <string.h>

#include <stdio.h>

#include "AMPDecoder.h"

////////////////////////////////////////////////////////////////////////////

//#define RDTSC_PROFILE

#ifdef RDTSC_PROFILE

	#include <windows.h>

	static long p_lasttime;
	static long p_frames=0;
	static __int64 p_total=0;
	static __int64 p_read=0;
	static __int64 p_bitalloc=0;
	static __int64 p_scfsi=0;
	static __int64 p_scalefac=0;
	static __int64 p_decode=0;
	static __int64 p_polyphase=0;

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

// 'c' table

static const double c_f_tab[17] = {
#if 0
#define F(c,steps) ( 32768.0 * (c) * 2.0 / ((steps)+1) )

	F(1.33333333333,3),
	F(1.60000000000,7),
	F(1.14285714286,7),
	F(1.77777777777,15),
	F(1.06666666666,15),
	F(1.03225806452,31),
	F(1.01587301587,63),
	F(1.00787401575,127),
	F(1.00392156863,255),
	F(1.00195694716,511),
	F(1.00097751711,1023),
	F(1.00048851979,2047),
	F(1.00024420024,4095),
	F(1.00012208522,8191),
	F(1.00006103888,16383),
	F(1.00003051851,32767),
	F(1.00001525902,65535),
#else
#define F(steps1, steps0) (32767.0 * 2.0 / (double)steps0)

	F(    4, 3),
	F(    8, 5),
	F(    8, 7),
	F(   16, 9),
	F(   16, 15),
	F(   32, 31),
	F(   64, 63),
	F(  128, 127),
	F(  256, 255),
	F(  512, 511),
	F( 1024, 1023),
	F( 2048, 2047),
	F( 4096, 4095),
	F( 8192, 8191),
	F(16384, 16383),
	F(32768, 32767),
	F(65536, 65535),
#endif

#undef F

};

static const double scalefactors[64] = {
2.00000000000000, 1.58740105196820, 1.25992104989487,
1.00000000000000, 0.79370052598410, 0.62996052494744, 0.50000000000000,
0.39685026299205, 0.31498026247372, 0.25000000000000, 0.19842513149602,
0.15749013123686, 0.12500000000000, 0.09921256574801, 0.07874506561843,
0.06250000000000, 0.04960628287401, 0.03937253280921, 0.03125000000000,
0.02480314143700, 0.01968626640461, 0.01562500000000, 0.01240157071850,
0.00984313320230, 0.00781250000000, 0.00620078535925, 0.00492156660115,
0.00390625000000, 0.00310039267963, 0.00246078330058, 0.00195312500000,
0.00155019633981, 0.00123039165029, 0.00097656250000, 0.00077509816991,
0.00061519582514, 0.00048828125000, 0.00038754908495, 0.00030759791257,
0.00024414062500, 0.00019377454248, 0.00015379895629, 0.00012207031250,
0.00009688727124, 0.00007689947814, 0.00006103515625, 0.00004844363562,
0.00003844973907, 0.00003051757813, 0.00002422181781, 0.00001922486954,
0.00001525878906, 0.00001211090890, 0.00000961243477, 0.00000762939453,
0.00000605545445, 0.00000480621738, 0.00000381469727, 0.00000302772723,
0.00000240310869, 0.00000190734863, 0.00000151386361, 0.00000120155435,
1E-20
};


/* ABCD_INDEX = lookqt[mode][sr_index][br_index]  */
/* -1 = invalid  */
static signed char lookqt[4][3][16] =
{
   1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  44ks stereo */
   0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,		/*  48ks */
   1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  32ks */
   1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  44ks joint stereo */
   0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,		/*  48ks */
   1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  32ks */
   1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  44ks dual chan */
   0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,		/*  48ks */
   1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,		/*  32ks */
// mono extended beyond legal br index
//  1,2,2,0,0,0,1,1,1,1,1,1,1,1,1,-1,          /*  44ks single chan */
//  0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,-1,          /*  48ks */
//  1,3,3,0,0,0,1,1,1,1,1,1,1,1,1,-1,          /*  32ks */
// legal mono
   1, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1,		/*  44ks single chan */
   0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1,		/*  48ks */
   1, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1,		/*  32ks */
};

/* bit allocation table look up */
/* table per mpeg spec tables 3b2a/b/c/d  /e is mpeg2 */
/* look_bat[abcd_index][4][16]  */
static const unsigned char look_bat[5][4][16] =
{
/* LOOK_BATA */
   0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17,
   0, 1, 2, 3, 4, 5, 6, 17, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATB */
   0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17,
   0, 1, 2, 3, 4, 5, 6, 17, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATC */
   0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATD */
   0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* LOOK_BATE */
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 5, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const unsigned char look_nbat[5][4] =
{
   3, 8, 12, 4,
   3, 8, 12, 7,
   2, 0, 6, 0,
   2, 0, 10, 0,
   4, 0, 7, 19,
};

static const unsigned char nbit[4]={4,4,3,2};

///////////////////////////////////////////////////////////////////////////

void AMPDecoder::L1_PrereadFrame() {
	resetbitsL2(frame_size);
}

bool AMPDecoder::L1_DecodeFrame() {
	int i, j;
	int sb, ch;
	int subbands, sidebands;
	int bitalloc[32][2];
	float scalefac[32][2];
	float sample[2][32];

	// header's already been read... so fill the bit reservoir.

	profile_set(0);

	resetbitsL2(frame_size);

	profile_add(p_read);

	// decode bit allocations for each subband.

//_RPT0(0,"Decoding bit allocations\n");

	if (mode == MODE_JOINTSTEREO)
		sidebands = 4*(mode_ext+1);
	else
		sidebands = 32;

	for(sb=0; sb<32; sb++) {
		if (sb < sidebands)
			for(ch=0; ch<channels; ch++)
				bitalloc[sb][ch] = getbitsL2(4);
		else
			bitalloc[sb][0] = bitalloc[sb][1] = getbitsL2(4);
	}

	subbands = sb;
	if (sidebands > subbands)
		sidebands = subbands;

	profile_add(p_bitalloc);

	// decode scale factors
	//
	// 6 bits per subband per channel, left-then-right if stereo

	static const unsigned char bitlookup[]={
		0, 64, 65, 3, 66, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
	};

	for(sb=0; sb<32; sb++) {
		for(ch=0; ch<channels; ch++) {
			if (bitalloc[sb][ch]) {
				double c = c_f_tab[bitalloc[sb][ch]-1];

				++bitalloc[sb][ch];

				scalefac[sb][ch] = c*scalefactors[getbitsL2(6)];
			}
		}
	}

	// decode samples (12 sets)

	for(sb=0; sb<32; sb++)
		for(ch=0; ch<channels; ch++)
			sample[ch][sb] = 0;

	profile_add(p_scalefac);

	for(i=0; i<12; i++) {
		for(sb=0; sb<sidebands; sb++) {
			for(ch=0; ch<channels; ch++) {
				if (bitalloc[sb][ch]) {
					const unsigned char bits = bitalloc[sb][ch];
					const float sf = scalefac[sb][ch];

					int bias = (1<<(bits-1))-1;
					sample[ch][sb] = scalefac[sb][ch]*(getbitsL2(bits) - bias);
				}
			}
		}

		for(; sb<subbands; sb++) {
			if (bitalloc[sb][0]) {
				const unsigned char bits = bitalloc[sb][0];
				const double sf = scalefac[sb][0];

				int bias = (1<<(bits-1))-1;
				sample[0][sb] = sf*(getbitsL2(bits) - bias);
			}

			if (channels>1)
				sample[1][sb] = sample[0][sb];
		}

		profile_add(p_decode);

		// subband synthesis

		if (mode == MODE_MONO)
			polyphase(&sample[0][0], NULL, psDest + 32*i, false);
		else
			polyphase(&sample[0][0], &sample[1][0], psDest + 64*i, false);

		profile_add(p_polyphase);
	}

#ifdef RDTSC_PROFILE

	if (!(++p_frames & 127)) {
		static char buf[256];

		sprintf(buf, "%d frames: total %I64d, read %d%%, bitalloc %d%%, scalefac %d%%, decode %d%%, poly %d%%\n"
				,p_frames
				,p_total
				,(long)((p_read*100)/p_total)
				,(long)((p_bitalloc*100)/p_total)
				,(long)((p_scalefac*100)/p_total)
				,(long)((p_decode*100)/p_total)
				,(long)((p_polyphase*100)/p_total)
				);
		OutputDebugString(buf);
	}
#endif

	// report proper sample count

	if (mode == MODE_MONO)
		lSampleCount = 384;
	else
		lSampleCount = 384*2;

	return true;
}

void AMPDecoder::L2_PrereadFrame() {
	resetbitsL2(frame_size);
}

bool AMPDecoder::L2_DecodeFrame() {
	int i, j;
	int sb, ch;
	int subbands, sidebands;
	int alloctbl_idx;
	int bitalloc[32][2];
	int scfsi[32][2];
	float scalefac[3][32][2];
	float sample[3][2][32];

	// header's already been read... so fill the bit reservoir.

	profile_set(0);

	resetbitsL2(frame_size);

	profile_add(p_read);

	// decode bit allocations for each subband.
	//
	// 1) Figure out which A-D table we want, based on mode, bitrate and sampling rate.

	if (is_mpeg2)
		alloctbl_idx = 4;
	else
		alloctbl_idx = lookqt[mode][sr_index][br_index];

	// 2) Decode actual bit allocations.

//_RPT0(0,"Decoding bit allocations\n");

	if (mode == MODE_JOINTSTEREO)
		sidebands = 4*(mode_ext+1);
	else
		sidebands = 32;

	sb=0;
	for(i=0; i<4; i++)
		for(j=0; j<look_nbat[alloctbl_idx][i]; j++) {

			if (sb < sidebands)
				for(ch=0; ch<channels; ch++)
					bitalloc[sb][ch] = look_bat[alloctbl_idx][i][getbitsL2(nbit[i])];
			else
				bitalloc[sb][0] = bitalloc[sb][1] = look_bat[alloctbl_idx][i][getbitsL2(nbit[i])];

			++sb;
		}

	subbands = sb;
	if (sidebands > subbands)
		sidebands = subbands;

	profile_add(p_bitalloc);

	// decode scale factor selector indices (scfsi)
	//
	// 2 bits per subband per channel, left-then-right if stereo

	for(sb=0; sb<subbands; sb++)
		for(ch=0; ch<channels; ch++)
			if (bitalloc[sb][ch])
				scfsi[sb][ch] = getbitsL2(2);

	profile_add(p_scfsi);

	// decode scale factors
	//
	// 6, 12, or 18 bits per subband per channel, left-then-right if stereo

	static const unsigned char bitlookup[]={
		0, 64, 65, 3, 66, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
	};

	for(sb=0; sb<subbands; sb++) {
		for(ch=0; ch<channels; ch++) {
			if (bitalloc[sb][ch]) {
				double c = c_f_tab[bitalloc[sb][ch]-1];

				bitalloc[sb][ch] = bitlookup[bitalloc[sb][ch]];

				scalefac[0][sb][ch] = c*scalefactors[getbitsL2(6)];

				switch(scfsi[sb][ch]) {

					case 0:		// three distinct scale factors
						scalefac[1][sb][ch] = c*scalefactors[getbitsL2(6)];
						scalefac[2][sb][ch] = c*scalefactors[getbitsL2(6)];
						break;

					case 1:		// first two scale factors identical
						scalefac[1][sb][ch] = scalefac[0][sb][ch];
						scalefac[2][sb][ch] = c*scalefactors[getbitsL2(6)];
						break;

					case 3:		// last two scale factors identical
						scalefac[1][sb][ch] =
						scalefac[2][sb][ch] = c*scalefactors[getbitsL2(6)];
						break;

					case 2:		// all three scale factors identical
						scalefac[1][sb][ch] =
						scalefac[2][sb][ch] = scalefac[0][sb][ch];
						break;

					default:
						__assume(false);

				}
			}
		}
	}

	// decode samples (12 sets)

	static const unsigned char group_bits[]={
		5, 7, 10,
	};

/*	for(sb=subbands; sb<32; sb++)
		for(ch=0; ch<channels; ch++)
			sample[0][ch][sb] = sample[1][ch][sb] = sample[2][ch][sb] = 0;*/

	for(sb=0; sb<32; sb++)
		for(ch=0; ch<channels; ch++)
			sample[0][ch][sb] = sample[1][ch][sb] = sample[2][ch][sb] = 0;

	profile_add(p_scalefac);

	for(i=0; i<12; i++) {
		const float (*const sf_ptr)[2] = scalefac[i>>2];


		for(sb=0; sb<sidebands; sb++) {
			for(ch=0; ch<channels; ch++) {
				if (bitalloc[sb][ch]) {
					const unsigned char bits = bitalloc[sb][ch];
					const double sf = sf_ptr[sb][ch];

					if (bits<64) {
						int bias = (1<<(bits-1))-1;
						sample[0][ch][sb] = sf*(getbitsL2(bits) - bias);
						sample[1][ch][sb] = sf*(getbitsL2(bits) - bias);
						sample[2][ch][sb] = sf*(getbitsL2(bits) - bias);
					} else {

						// select between 3 grouping of 3, 5, or 9 levels each

						const int v = getbitsL2(group_bits[bits-64]);
						const signed char (*const grp_tbl)[3] = group_tbls[bits-64];

						sample[0][ch][sb] = sf*grp_tbl[v][0];
						sample[1][ch][sb] = sf*grp_tbl[v][1];
						sample[2][ch][sb] = sf*grp_tbl[v][2];
					}
				}
			}
		}

		for(; sb<subbands; sb++) {
			if (bitalloc[sb][0]) {
				const unsigned char bits = bitalloc[sb][0];
				const double sf = sf_ptr[sb][0];

				if (bits<64) {
					int bias = (1<<(bits-1))-1;
					sample[0][0][sb] = sf*(getbitsL2(bits) - bias);
					sample[1][0][sb] = sf*(getbitsL2(bits) - bias);
					sample[2][0][sb] = sf*(getbitsL2(bits) - bias);
				} else {

					// select between 3 grouping of 3, 5, or 9 levels each

					const int v  = getbitsL2(group_bits[bits-64]);
					const signed char (*grp_tbl)[3] = group_tbls[bits-64];

					sample[0][0][sb] = sf*grp_tbl[v][0];
					sample[1][0][sb] = sf*grp_tbl[v][1];
					sample[2][0][sb] = sf*grp_tbl[v][2];
				}
			}

			if (channels>1) {
				sample[0][1][sb] = sample[0][0][sb];
				sample[1][1][sb] = sample[1][0][sb];
				sample[2][1][sb] = sample[2][0][sb];
			}
		}

		profile_add(p_decode);

		// subband synthesis

		if (mode == MODE_MONO)
			for(j=0; j<3; j++) {
				polyphase(&sample[j][0][0], NULL, psDest + 32*(i*3+j), false);
			}
		else
			for(j=0; j<3; j++) {
				polyphase(&sample[j][0][0], &sample[j][1][0], psDest + 64*(i*3+j), false);
			}

		profile_add(p_polyphase);
	}

#ifdef RDTSC_PROFILE

	if (!(++p_frames & 127)) {
		static char buf[256];

		sprintf(buf, "%d frames: total %I64d, read %d%%, bitalloc %d%%, scfsi %d%%, scalefac %d%%, decode %d%%, poly %d%%\n"
				,p_frames
				,p_total
				,(long)((p_read*100)/p_total)
				,(long)((p_bitalloc*100)/p_total)
				,(long)((p_scfsi*100)/p_total)
				,(long)((p_scalefac*100)/p_total)
				,(long)((p_decode*100)/p_total)
				,(long)((p_polyphase*100)/p_total)
				);
		OutputDebugString(buf);
	}
#endif


	// report proper sample count

	if (mode == MODE_MONO)
		lSampleCount = 1152;
	else
		lSampleCount = 1152*2;

	return true;
}
