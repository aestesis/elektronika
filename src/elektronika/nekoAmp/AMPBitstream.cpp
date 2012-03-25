#include <crtdbg.h>

#include "AMPBitstream.h"

void AMPBitstream::resetbits(int bytes) {
	bufindex = 0;
	bitcnt = 24;
	bitheap = 0;

	if (bytes) {
		bufindexw = bufpoint = pSource->read(buf, bytes);

		if (bufpoint != bytes)
			throw (int)IAMPDecoder::ERR_EOF;
	} else
		bufindexw = bufpoint = 0;
}

void AMPBitstream::fillbits(int bytes) {
	int actual;

	if (bufindexw + bytes > BUFFER_SIZE) {
		actual = pSource->read(buf + bufindexw, BUFFER_SIZE - bufindexw);
		if (actual != BUFFER_SIZE-bufindexw)
			throw (int)IAMPDecoder::ERR_EOF;

		bufindexw = bufindexw+bytes-BUFFER_SIZE;

		actual = pSource->read(buf, bufindexw);

		if (actual != bufindexw)
			throw (int)IAMPDecoder::ERR_EOF;

	} else {
		actual = pSource->read(buf + bufindexw, bytes);

		if (actual != bytes)
			throw (int)IAMPDecoder::ERR_EOF;

		bufindexw += actual;
		if (bufindexw >= BUFFER_SIZE)
			bufindexw -= BUFFER_SIZE;
	}

	bufpoint += bytes;
	if (bufpoint > BUFFER_SIZE)
		bufpoint = BUFFER_SIZE;
}

void AMPBitstream::rewind(int bytes) {
	// restore bytes in buffer

	if (bitcnt <= 16)
		bufpoint += (24-bitcnt)>>3;
//		bufpoint += (24+7-bitcnt)>>3;

//	if (bufpoint < bytes)
//		_RPT2(0,"\t\t\t------Not enough bytes! needed=%d, actual=%d\n", bytes, bufpoint);

	if (bufpoint < bytes)
		throw (int)IAMPDecoder::ERR_INCOMPLETEFRAME;

	bitcnt = 24;
	bitheap = 0;
	bufindex = (bufindexw + BUFFER_SIZE - bytes) & BUFFER_MASK;
	bufpoint = bytes;
}

void AMPBitstream::rewindbits(int bits) {
	long actualbits = tellbits();

	// allow up to 24 bits of oopsie

	if (bits > actualbits+24)
		throw (int)IAMPDecoder::ERR_INCOMPLETEFRAME;

	if (bits == actualbits)
		return;

	if (actualbits > bits && actualbits - bits <= 16)
		getbits(actualbits - bits);
	else {

		bitcnt = 24;
		bitheap = 0;
		bufpoint = ((bits+7)>>3);
		bufindex = (bufindexw + BUFFER_SIZE - bufpoint) & BUFFER_MASK;

		if (bits & 7)
			getbits(8-(bits & 7));
	}
}

int AMPBitstream::tellbits() {
	return bufpoint*8 + (24-bitcnt);
}

long AMPBitstream::_peekbits(unsigned char bits) {
	long rv;

	if (!bits)
		return 0;

	while(bitcnt >= 0 && bufpoint>0) {
		--bufpoint;
		bitheap += ((unsigned long)buf[bufindex++]) << bitcnt;
		bufindex &= BUFFER_MASK;
		bitcnt -= 8;
	}

	if (bitcnt > 24-bits)
		throw (int)IAMPDecoder::ERR_INCOMPLETEFRAME;

	return bitheap >> (32-bits);
}

long AMPBitstream::_peekbits2(unsigned char bits) {
	long rv;

	if (!bits)
		return 0;

	while(bitcnt >= 0 && bufpoint>0) {
		--bufpoint;
		bitheap += ((unsigned long)buf[bufindex++]) << bitcnt;
		bufindex &= BUFFER_MASK;
		bitcnt -= 8;
	}

	return bitheap >> (32-bits);
}

long AMPBitstream::_getbits(unsigned char bits) {
	long rv;

	if (!bits)
		return 0;

	while(bitcnt >= 0 && bufpoint>0) {
		--bufpoint;
		bitheap += ((unsigned long)buf[bufindex++]) << bitcnt;
		bufindex &= BUFFER_MASK;
		bitcnt -= 8;
	}

	if (bitcnt > 24-bits)
		throw (int)IAMPDecoder::ERR_INCOMPLETEFRAME;

	rv = bitheap >> (32-bits);

	bitcnt += bits;

	bitheap <<= bits;

	return rv;
}

void AMPBitstream::resetbitsL2(int bytes) {
	bufindexw = bufpoint = pSource->read(buf, bytes);

	if (bufpoint != bytes)
		throw (int)IAMPDecoder::ERR_EOF;

	bufindex = 4;
	bitcnt = 32;
	bitheap = *(long *)buf;
		__asm mov ecx,this
		__asm mov eax,[ecx]AMPBitstream.bitheap
		__asm bswap eax
		__asm mov [ecx]AMPBitstream.bitheap,eax
}

long AMPBitstream::_getbitsL2(unsigned char bits) {
	long rv;
	int t = bitcnt;

	if (!bits)
		return 0;

	rv = bitheap >> (32-bits);

	if (bufpoint < 4) {
		bitcnt = 0;
		bitheap = 0;
		while(bitcnt < 32 && bufpoint>0) {
			--bufpoint;
			bitheap += ((unsigned long)buf[bufindex++]) << (24-bitcnt);
			bitcnt += 8;
		}

		if (t+bitcnt < bits)
			throw (int)IAMPDecoder::ERR_INCOMPLETEFRAME;
	} else {
		bitheap = *(long *)(buf + bufindex);
		bufindex += 4;
		__asm mov ecx,this
		__asm mov eax,[ecx]AMPBitstream.bitheap
		__asm bswap eax
		__asm mov [ecx]AMPBitstream.bitheap,eax
		bitcnt = 32;
		bufpoint -= 4;
	}

	rv += bitheap >> (32+t - bits);
	bitheap <<= bits-t;
	bitcnt -= bits-t;

	return rv;
}
