#ifndef f_AMPLIB_AMPDECODER_H
#define f_AMPLIB_AMPDECODER_H

#include "IAMPDecoder.h"
#include "AMPBitstream.h"

#define SBLIMIT (32)
#define SSLIMIT (18)

struct gr_info_s {
	unsigned part2_3_length;
	unsigned big_values;
	unsigned global_gain;
	unsigned scalefac_compress;
	unsigned window_switching_flag;
	unsigned block_type;
	unsigned mixed_block_flag;
	unsigned table_select[3];
	unsigned subblock_gain[3];
	unsigned region0_count;
	unsigned region1_count;
	unsigned preflag;
	unsigned scalefac_scale;
	unsigned count1table_select;
};

typedef struct {
	unsigned main_data_begin;
	unsigned private_bits;
	struct {
	    unsigned scfsi[4];
		struct gr_info_s gr[2];
		} ch[2];
	} III_side_info_t;

typedef struct III_scalefac1_t {
	int l[23];            /* [cb] */
	int s[3][13];         /* [window][cb] */
	} III_scalefac_t[2];  /* [ch] */

class AMPDecoder : public IAMPDecoder, AMPBitstream {
public:
	AMPDecoder();
	~AMPDecoder();

	void Destroy();

	char *	GetAmpVersionString();
	void	Init();
	void	setSource(IAMPBitsource *pSource);
	void	setDestination(short *psDest);
	long	getSampleCount();
	void	getStreamInfo(AMPStreamInfo *pasi);
	char *	getErrorString(int err);
	void	Reset();
	void	ReadHeader();
	void	PrereadFrame();
	bool	DecodeFrame();

private:
	float	window[2][512];
	float	prevblck[2][SBLIMIT][SSLIMIT];
	int		winptr;
	short *	psDest;
	long	lSampleCount;

	unsigned char *l3bitptr;
	int		l3bitidx;

	long	frame_size;

	int		bitrate;
	int		frequency;
	int		br_index;
	int		sr_index;
	int		is_mpeg2;
	bool	is_errorprotected;
	bool	is_padded;
	bool	is_extended;
	bool	is_copyrighted;
	bool	is_original;
	int		layer;
	int		mode;
	int		mode_ext;
	int		channels;
	int		emphasis;

	III_side_info_t		sideinfo;
	int		sideinfo_size;

	int		slen[4];				// MPEG-2 layer 3
	const int *	scale_block_indexes;	// MPEG-2 layer 3

	static	int init_count;

	static	signed char group3[31][3];
	static	signed char group5[127][3];
	static	signed char group9[1023][3];
	static	signed char (*group_tbls[3])[3];

	static	void Initialize();
	static	void Deinitialize();

	void	L1_PrereadFrame();
	bool	L1_DecodeFrame();

	void	L2_PrereadFrame();
	bool	L2_DecodeFrame();

	int		L3_GetBits(int bits);
	void	L3_GetSideInfo();
	void	L3_PrereadFrame();
	bool	L3_DecodeFrame();

	void	L3_GetScaleFactors1(III_scalefac_t *scalefac, int gr, int ch);
	void	L3_GetScaleFactors2(III_scalefac_t *scalefac, int ch);

	int		L3_HuffmanDecode(long int is[SBLIMIT*SSLIMIT],
							int ch,
							int gr,
							int part2_start);
	int		L3_DequantizeSample(long int is[SBLIMIT*SSLIMIT],
								float xr[SBLIMIT*SSLIMIT],
								const III_scalefac1_t *scalefac,
								struct gr_info_s *gr_info,
								int ch, int limit);
	int		L3_Stereo(
					float lr[2][SBLIMIT*SSLIMIT],
					const III_scalefac_t *scalefac,
					struct gr_info_s *gr_info,
					int nz0, int nz1);

	void	L3_Reorder (float xr[SBLIMIT*SSLIMIT],
				float ro[SBLIMIT*SSLIMIT],
				struct gr_info_s *gr_info);

	void	L3_Antialias(float xr[SBLIMIT][SSLIMIT],
						struct gr_info_s *gr_info);

	void	L3_Hybrid(float fsIn[SBLIMIT][SSLIMIT],
				float fsOut[SSLIMIT][SBLIMIT],
				int ch,
				struct gr_info_s *gr_info,
				int nonzero_entries);

	void	L3_GetHuffmanBig(int table_id, long *is, int count) throw();
	int		L3_GetHuffmanCount1_32(long *is, int i, int bit_threshold) throw();
	int		L3_GetHuffmanCount1_33(long *is, int i, int bit_threshold) throw();

	void	polyphase(float *band_l, float *band_r, short *samples, bool invert_odd);
//	void	polyphase_single(float *band, int ch, short *samples);
	void	polyphase_retract(int amount);
};

#define MODE_STEREO			(0)
#define	MODE_JOINTSTEREO	(1)
#define	MODE_DUALCHANNEL	(2)
#define MODE_MONO			(3)

#endif
