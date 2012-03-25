//#include <crtdbg.h>

#include "AMPDecoder.h"

IAMPDecoder *CreateAMPDecoder() {
	return new AMPDecoder();
}

AMPDecoder::AMPDecoder() {
}

AMPDecoder::~AMPDecoder() {
}

void AMPDecoder::Destroy() {
	delete this;
}

char *AMPDecoder::GetAmpVersionString() {
	return "NekoAmp 1.4, based on FreeAmp 1.1.0 source";
}

char *AMPDecoder::getErrorString(int err) {
	switch(err) {
	case ERR_NONE:		return "no error";
	case ERR_EOF:		return "unexpected end of stream";
	case ERR_READ:		return "read error";
	case ERR_MPEG25:	return "cannot process Fraunhofer-IIS MPEG 2.5 stream";
	case ERR_FREEFORM:	return "cannot process free-form streams";
	case ERR_SYNC:		return "sync error";
	case ERR_INTERNAL:	return "**Internal Error**";
	case ERR_INCOMPLETEFRAME:	return "not enough data to decode frame";
	}

	return "unknown error";
}

void AMPDecoder::Init() {
	int i;
	float *pp;

	winptr = 0;

	for(i=0; i<512; i++)
		window[0][i] = window[1][i] = 0.0f;

	pp = (float *)prevblck;

	for(i=0; i<SBLIMIT*SSLIMIT*2; i++) {
		*pp++ = 0.0f;
	}

	resetbits(0);

	Initialize();
}

void AMPDecoder::setSource(IAMPBitsource *pSource) {
	this->pSource = pSource;
}

void AMPDecoder::setDestination(short *psDest) {
	this->psDest = psDest;
}

long AMPDecoder::getSampleCount() {
	return lSampleCount;
}

void AMPDecoder::Reset() {
}

//////////////////////////////////////

static bool isValidMPEGHeader(long hdr) {
	// 0000F0FF 12 bits	sync mark
	//
	// 00000800  1 bit	version
	// 00000600  2 bits	layer (3 = layer I, 2 = layer II, 1 = layer III)
	// 00000100  1 bit	error protection (0 = enabled)
	//
	// 00F00000  4 bits	bitrate_index
	// 000C0000  2 bits	sampling_freq
	// 00020000  1 bit	padding
	// 00010000  1 bit	extension
	//
	// C0000000  2 bits	mode (0=stereo, 1=joint stereo, 2=dual channel, 3=mono)
	// 30000000  2 bits	mode_ext
	// 08000000  1 bit	copyright
	// 04000000  1 bit	original
	// 03000000  2 bits	emphasis

	// sync mark?

	if ((hdr & 0xf0ff) != 0xf0ff)
		return false;
	
	// 00 for layer ("layer 4") is not valid
	if (!(hdr & 0x00000600))
		return false;

	// 1111 for bitrate is not valid
	if ((hdr & 0x00F00000) == 0x00F00000)
		return false;

	// 11 for sampling frequency is not valid
	if ((hdr & 0x000C0000) == 0x000C0000)
		return false;

	// Looks okay to me...
	return true;
}


static const int bitrates2[3][15] = {
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, },
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, },
	{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, },
};

static const int bitrates[3][15] = {
          {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
          {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
          {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
        };

static const long samp_freq[4] = {44100, 48000, 32000, 0};

void AMPDecoder::ReadHeader() {
	union {
		char buf[4];
		long hdr;
	};

	// MPEG headers are 4 bytes.

	if (4 != pSource->read(buf, 4))
		throw (int)ERR_EOF;

	// Is it a valid header?  If not, figure out how many
	// bytes obviously aren't sync and advance by that much.

	while(!isValidMPEGHeader(hdr)) {
		int advance = 1;

		if (buf[1] != 0xFF || (buf[2]&0xF0) != 0xF0) {
			if (buf[2] != 0xFF || (buf[3]&0xF0) != 0xF0) {
				if (buf[3] != 0xFF)
					advance = 4;
				else
					advance = 3;
			} else
				advance = 2;
		}

		*(long *)buf >>= advance<<3;

		if (advance != pSource->read(buf+4-advance, advance))
			throw (int)ERR_EOF;
	}

	// Yay!  A valid MPEG header!  Parse it!

	// 0000F0FF 12 bits	sync mark
	//
	// 00000800  1 bit	version
	// 00000600  2 bits	layer (3 = layer I, 2 = layer II, 1 = layer III)
	// 00000100  1 bit	error protection (0 = enabled)
	//
	// 00F00000  4 bits	bitrate_index
	// 000C0000  2 bits	sampling_freq
	// 00020000  1 bit	padding
	// 00010000  1 bit	extension
	//
	// C0000000  2 bits	mode (0=stereo, 1=joint stereo, 2=dual channel, 3=mono)
	// 30000000  2 bits	mode_ext
	// 08000000  1 bit	copyright
	// 04000000  1 bit	original
	// 03000000  2 bits	emphasis

//	_RPT1(0,"Header: %08lx\n", hdr);

	is_mpeg2			= !(hdr & 0x00000800);
	layer				= 4 - (hdr>>9)&3;
	is_errorprotected	= !(hdr & 0x00000100);
	br_index			= (hdr>>20)&15;
	bitrate				= (is_mpeg2 ? bitrates2 : bitrates)[layer-1][br_index];
	sr_index			= (hdr>>18)&3;
	frequency			= samp_freq[sr_index];

	if (is_mpeg2)
		frequency>>=1;

	is_padded			= !!(hdr & 0x00020000);
	is_extended			= !!(hdr & 0x00010000);
	mode				= (hdr>>30)&3;
	mode_ext			= (hdr>>28)&3;
	is_copyrighted		= !!(hdr & 0x08000000);
	is_original			= !!(hdr & 0x04000000);
	emphasis			= (hdr>>24)&3;

	if (mode == MODE_MONO)
		channels = 1;
	else
		channels = 2;

	// Compute the frame size, not including header and layer III side info

	if (layer == 1) {	// slots in layer I are 4 bytes
		frame_size = 4 * (12000 * bitrate / frequency);
		if (is_padded)
			frame_size+=4;
	} else {
		if (is_mpeg2)
			frame_size = 72000 * bitrate / frequency;
		else
			frame_size = 144000 * bitrate / frequency;

		if (is_padded)
			++frame_size;
	}

	frame_size -= 4;

	// if we're processing Layer III, subtract size of side info as well

	if (layer == 3) {
		if (is_mpeg2)
			if (mode == MODE_MONO)
				sideinfo_size = 9;
			else
				sideinfo_size = 17;
		else
			if (mode == MODE_MONO)
				sideinfo_size = 17;
			else
				sideinfo_size = 32;

		frame_size -= sideinfo_size;
	}

	// Read in CRC if we have error protection

	if (is_errorprotected) {
		unsigned short CRC;

		if (2 != pSource->read(&CRC, 2))
			throw (int)ERR_READ;

		frame_size -= 2;
	}

	// Read in Layer3 side info

	if (layer == 3)
		L3_GetSideInfo();

}

void AMPDecoder::getStreamInfo(AMPStreamInfo *pasi) {
	pasi->lBitrate		= bitrate;
	pasi->lSamplingFreq	= frequency;
	pasi->nLayer		= layer;
	pasi->nMPEGVer		= is_mpeg2 ? 2 : 1;
	pasi->fStereo		= mode != MODE_MONO;
}

void AMPDecoder::PrereadFrame() {
	switch(layer) {
	case 1:	L1_PrereadFrame(); break;
	case 2:	L2_PrereadFrame(); break;
	case 3:	L3_PrereadFrame(); break;
	default: __assume(false);
	}
}

bool AMPDecoder::DecodeFrame() {
	lSampleCount = 0;

	switch(layer) {
	case 1:	return L1_DecodeFrame();
	case 2: return L2_DecodeFrame();
	case 3: return L3_DecodeFrame();
	default:	__assume(false);
	}
}
