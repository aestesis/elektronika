#ifndef f_AMPLIB_BITSTREAM_H
#define f_AMPLIB_BITSTREAM_H

#include "IAMPDecoder.h"

// max layer2 buffer -> 144000 * 384 / 32000 + 1 - 4 = 1725 bytes
// max layer3 buffer -> 144000 * 320 / 32000 + 512 - 4 = 1948 bytes

#define BUFFER_SIZE		(2048)
#define BUFFER_MASK		(BUFFER_SIZE - 1)

class AMPBitstream {
protected:
	IAMPBitsource *pSource;

private:
	unsigned char buf[BUFFER_SIZE];
	int bitcnt;
	int bufpoint;
	int bufindex, bufindexw;
	unsigned long bitheap;

	long _getbits(unsigned char bits);
	long _getbitsL2(unsigned char bits);
	long _peekbits(unsigned char bits);
	long _peekbits2(unsigned char bits);

public:

	void resetbits(int bytes);
	void resetbitsL2(int bytes);
	void fillbits(int bytes);
	void rewind(int bytes);
	void rewindbits(int bits);
	int tellbits();

	long peekbits(unsigned char bits) {
		if (bitcnt > 24-(int)bits)
			return _peekbits(bits);

		return bitheap >> (32-bits);
	}

	long peekbits2(unsigned char bits) {
		if (bitcnt > 24-(int)bits)
			return _peekbits2(bits);

		return bitheap >> (32-bits);
	}

	void skipbits(unsigned char bits) {
		bitcnt += bits;
		bitheap <<= bits;
	}

	void skipbit() {
		bitcnt++;
		bitheap += bitheap;
	}

	long getbits(unsigned char bits) {
		long rv;
		
		if (!bits)
			return 0;

		if (bitcnt > 24-(int)bits)
			return _getbits(bits);

		rv = bitheap >> (32-bits);

		bitcnt += bits;

		bitheap <<= bits;

		return rv;
	}

	long getbitsL2(unsigned char bits) {
		long rv;
		
		if (!bits)
			return 0;

		if (bitcnt < bits)
			return _getbitsL2(bits);

		rv = bitheap >> (32-bits);

		bitcnt -= bits;

		bitheap <<= bits;

		return rv;
	}

	unsigned long getflag() {
		long rv;
		
		if (bitcnt > 23)
			return _getbits(1);

		rv = bitheap;

		++bitcnt;

		bitheap <<= 1;

		return rv&0x80000000;
	}
};

#endif
