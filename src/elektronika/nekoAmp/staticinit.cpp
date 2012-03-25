#include <math.h>
#include <float.h>

#include "AMPDecoder.h"

int AMPDecoder::init_count = 0;

signed char AMPDecoder::group3[31][3];
signed char AMPDecoder::group5[127][3];
signed char AMPDecoder::group9[1023][3];

signed char (*AMPDecoder::group_tbls[3])[3]={
	group3,
	group5,
	group9,
};

extern void init_mdct();

void AMPDecoder::Initialize()
{
	int i,j;
	int v;

	if (init_count)
		return;

	++init_count;

	// initialize IMDCT for layer 3 decoding

	init_mdct();

	// initialize 3/5/9 group decoding tables for layer 2

	for(i=0; i<31; i++) {
		v = i;

		for(j=0; j<3; j++) {
			group3[i][j] = v % 3 - 1;
			v /= 3;
		}
	}

	for(i=0; i<127; i++) {
		v = i;

		for(j=0; j<3; j++) {
			group5[i][j] = v % 5 - 2;
			v /= 5;
		}
	}

	for(i=0; i<1023; i++) {
		v = i;

		for(j=0; j<3; j++) {
			group9[i][j] = v % 9 - 4;
			v /= 9;
		}
	}

}
