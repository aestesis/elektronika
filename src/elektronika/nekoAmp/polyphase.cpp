#include <math.h>
#include <string.h>
#include <crtdbg.h>
#include <stdio.h>

#include "AMPDecoder.h"

#define USE_ASM


static float wincoef[16][32] =
{				/* window coefs */
	{
		0.000000000f, 0.000442505f, -0.003250122f, 0.007003784f,
		-0.031082151f, 0.078628540f, -0.100311279f, 0.572036743f,
		-1.144989014f, -0.572036743f, -0.100311279f, -0.078628540f,
		-0.031082151f, -0.007003784f, -0.003250122f, -0.000442505f,
	},{
		0.000015259f, 0.000473022f, -0.003326416f, 0.007919312f,
		-0.030517576f, 0.084182739f, -0.090927124f, 0.600219727f,
		-1.144287109f, -0.543823242f, -0.108856201f, -0.073059082f,
		-0.031478882f, -0.006118774f, -0.003173828f, -0.000396729f,
	},{
		0.000015259f, 0.000534058f, -0.003387451f, 0.008865356f,
		-0.029785154f, 0.089706421f, -0.080688477f, 0.628295898f,
		-1.142211914f, -0.515609741f, -0.116577141f, -0.067520142f,
		-0.031738281f, -0.005294800f, -0.003082275f, -0.000366211f,
	},{
		0.000015259f, 0.000579834f, -0.003433228f, 0.009841919f,
		-0.028884888f, 0.095169067f, -0.069595337f, 0.656219482f,
		-1.138763428f, -0.487472534f, -0.123474121f, -0.061996460f,
		-0.031845093f, -0.004486084f, -0.002990723f, -0.000320435f,
	},{
		0.000015259f, 0.000625610f, -0.003463745f, 0.010848999f,
		-0.027801514f, 0.100540161f, -0.057617184f, 0.683914185f,
		-1.133926392f, -0.459472656f, -0.129577637f, -0.056533810f,
		-0.031814575f, -0.003723145f, -0.002899170f, -0.000289917f,
	},{
		0.000015259f, 0.000686646f, -0.003479004f, 0.011886597f,
		-0.026535034f, 0.105819702f, -0.044784546f, 0.711318970f,
		-1.127746582f, -0.431655884f, -0.134887695f, -0.051132202f,
		-0.031661987f, -0.003005981f, -0.002792358f, -0.000259399f,
	},{
		0.000015259f, 0.000747681f, -0.003479004f, 0.012939452f,
		-0.025085449f, 0.110946655f, -0.031082151f, 0.738372803f,
		-1.120223999f, -0.404083252f, -0.139450073f, -0.045837402f,
		-0.031387329f, -0.002334595f, -0.002685547f, -0.000244141f,
	},{
		0.000030518f, 0.000808716f, -0.003463745f, 0.014022826f,
		-0.023422241f, 0.115921021f, -0.016510010f, 0.765029907f,
		-1.111373901f, -0.376800537f, -0.143264771f, -0.040634155f,
		-0.031005858f, -0.001693726f, -0.002578735f, -0.000213623f,
	},{
		0.000030518f, 0.000885010f, -0.003417969f, 0.015121460f,
		-0.021575928f, 0.120697014f, -0.001068115f, 0.791213989f,
		-1.101211548f, -0.349868774f, -0.146362305f, -0.035552979f,
		-0.030532837f, -0.001098633f, -0.002456665f, -0.000198364f,
	},{
		0.000030518f, 0.000961304f, -0.003372192f, 0.016235352f,
		-0.019531250f, 0.125259399f, 0.015228271f, 0.816864014f,
		-1.089782715f, -0.323318481f, -0.148773193f, -0.030609131f,
		-0.029937742f, -0.000549316f, -0.002349854f, -0.000167847f,
	},{
		0.000030518f, 0.001037598f, -0.003280640f, 0.017349243f,
		-0.017257690f, 0.129562378f, 0.032379150f, 0.841949463f,
		-1.077117920f, -0.297210693f, -0.150497437f, -0.025817871f,
		-0.029281614f, -0.000030518f, -0.002243042f, -0.000152588f,
	},{
		0.000045776f, 0.001113892f, -0.003173828f, 0.018463135f,
		-0.014801024f, 0.133590698f, 0.050354004f, 0.866363525f,
		-1.063217163f, -0.271591187f, -0.151596069f, -0.021179199f,
		-0.028533936f, 0.000442505f, -0.002120972f, -0.000137329f,
	},{
		0.000045776f, 0.001205444f, -0.003051758f, 0.019577026f,
		-0.012115479f, 0.137298584f, 0.069168091f, 0.890090942f,
		-1.048156738f, -0.246505737f, -0.152069092f, -0.016708374f,
		-0.027725220f, 0.000869751f, -0.002014160f, -0.000122070f,
	},{
		0.000061035f, 0.001296997f, -0.002883911f, 0.020690918f,
		-0.009231566f, 0.140670776f, 0.088775635f, 0.913055420f,
		-1.031936646f, -0.221984863f, -0.151962280f, -0.012420653f,
		-0.026840210f, 0.001266479f, -0.001907349f, -0.000106812f,
	},{
		0.000061035f, 0.001388550f, -0.002700806f, 0.021789551f,
		-0.006134033f, 0.143676758f, 0.109161377f, 0.935195923f,
		-1.014617920f, -0.198059082f, -0.151306152f, -0.008316040f,
		-0.025909424f, 0.001617432f, -0.001785278f, -0.000106812f,
	},{
		0.000076294f, 0.001480103f, -0.002487183f, 0.022857666f,
		-0.002822876f, 0.146255493f, 0.130310059f, 0.956481934f,
		-0.996246338f, -0.174789429f, -0.150115967f, -0.004394531f,
		-0.024932859f, 0.001937866f, -0.001693726f, -0.000091553f,
	}
};

static const float wincoef2[]={
	-0.001586914f, -0.023910521f, -0.148422241f, -0.976852417f,
	0.152206421f, 0.000686646f, -0.002227783f, 0.000076294f,
};

static const float coef32[31]={	/* 32 pt dct coefs */
	.5006029983604062f,
	.5054709610427671f,
	.5154473132093669f,
	.5310425978615345f,
	.553103908021273f,
	.5829349877738026f,
	.6225041535926976f,
	.674808388163145f,
	.7445363421017894f,
	.8393497548926808f,
	.9725684113484575f,
	1.16944022296938f,
	1.484165144274385f,
	2.057782139240278f,
	3.407611810249888f,
	10.19004086100494f,
	.5024192866920285f,
	.5224986197814337f,
	.5669440502448403f,
	.6468218211965951f,
	.7881547114622637f,
	1.060677908257709f,
	1.722447849872342f,
	5.101156529442028f,
	.5097955811692735f,
	.6013449114834756f,
	.8999763602860835f,
	2.562917284521227f,
	.5411961092767109f,
	1.306563350304716f,
	.7071068387877132f,
};

static void __inline forward_bf(int m, int n, const float x[], float f[], const float coef[])
{
	int j, n2;
	int p, q, p0;
	
	n2 = n >> 1;
	for (p0 = 0; p0 < 32; p0 += n) {
		p = p0;
		q = p + n - 1;
		for (j = 0; j < n2; j++, p++, q--)
		{
			f[p] = x[p] + x[q];
			f[n2 + p] = coef[j] * (x[p] - x[q]);
		}
	}
}
/*------------------------------------------------------------*/
static void __inline back_bf(int m, int n, const float x[], float f[])
{
	int j, n2, n21;
	int p, q, p0;
	
	n2 = n >> 1;
	n21 = n2 - 1;
	for (p0 = 0; p0 < 32; p0 += n) {
		p = p0;
		q = p0;
		for (j = 0; j < n2; j++, p += 2, q++)
			f[p] = x[q];
		p = p0 + 1;
		for (j = 0; j < n21; j++, p += 2, q++)
			f[p] = x[q] + x[q + 1];
		f[p] = x[q];
	}
}
/*------------------------------------------------------------*/

void fdct32(float x[], float c[], bool odd, bool invert_odd)
{
	float a[32];			/* ping pong buffers */
	float b[32];
	int p, q;

	// special first stage
	// forward_bf(2, 16, a, b, coef32 + 16);

	if (invert_odd) {
		q = 15;
		for(p = 0; p < 8; p+=2, q-=2) {
			double a1, a2, a3, a4;

			// p: normal, q:inverted.

			a1 =  x[p] - x[16+q];
			a2 = -x[q] + x[16+p];
			a3 = (x[p] + x[16+q]) * coef32[p];
			a4 = (-x[q] - x[16+p]) * coef32[q];

			b[p   ] =  a1 + a2;
			b[p+ 8] = (a1 - a2) * coef32[16+p];
			b[p+16] =  a3 + a4;
			b[p+24] = (a3 - a4) * coef32[16+p];

			// p: inverted, q: normal.

			a1 = -x[p+1] + x[15+q];
			a2 =  x[q-1] - x[17+p];
			a3 = (-x[p+1] - x[15+q]) * coef32[p+1];
			a4 = (x[q-1] + x[17+p]) * coef32[q-1];

			b[p+ 1] =  a1 + a2;
			b[p+ 9] = (a1 - a2) * coef32[17+p];
			b[p+17] =  a3 + a4;
			b[p+25] = (a3 - a4) * coef32[17+p];
		}
	} else {
		q = 15;
		for(p = 0; p < 8; p++, q--) {
			double a1 =  x[p] + x[16+q];
			double a2 =  x[q] + x[16+p];
			double a3 = (x[p] - x[16+q]) * coef32[p];
			double a4 = (x[q] - x[16+p]) * coef32[q];

			b[p   ] =  a1 + a2;
			b[p+ 8] = (a1 - a2) * coef32[16+p];
			b[p+16] =  a3 + a4;
			b[p+24] = (a3 - a4) * coef32[16+p];
		}
	}

	forward_bf(4, 8, b, a, coef32 + 16 + 8);

//	forward_bf(8, 4, a, b, coef32 + 16 + 8 + 4);
//  forward_bf(16, 2, b, a, coef32 + 16 + 8 + 4 + 2);
//	back_bf(8, 4, a, b);
//	back_bf(4, 8, b, a);
//	back_bf(2, 16, a, b);
	
	for (p = 0; p < 32; p += 16) {
		double a0, a1, a2, a3;
		double b0, b1, b2, b3;
		double d0, d1, d2, d3, d4, d5, d6, d7;
		double d8, d9, dA, dB, dC, dD, dE, dF;

		b0 = a[p  ] + a[p+3];
		b1 = a[p+1] + a[p+2];
		b2 = coef32[28] * (a[p  ] - a[p+3]);
		b3 = coef32[29] * (a[p+1] - a[p+2]);

		a0 = b0 + b1;
		a1 = coef32[30] * (b0 - b1);
		a2 = b2 + b3;
		a3 = coef32[30] * (b2 - b3);

		d0 = a0;
		d1 = a2 + a3;
		d2 = a1;
		d3 = a3;

		b0 = a[p+4] + a[p+7];
		b1 = a[p+5] + a[p+6];
		b2 = coef32[28] * (a[p+4] - a[p+7]);
		b3 = coef32[29] * (a[p+5] - a[p+6]);

		a0 = b0 + b1;
		a1 = coef32[30] * (b0 - b1);
		a2 = b2 + b3;
		a3 = coef32[30] * (b2 - b3);

		d4 = a0;
		d5 = a2 + a3;
		d6 = a1;
		d7 = a3;

		b0 = a[p+8] + a[p+11];
		b1 = a[p+9] + a[p+10];
		b2 = coef32[28] * (a[p+8] - a[p+11]);
		b3 = coef32[29] * (a[p+9] - a[p+10]);

		a0 = b0 + b1;
		a1 = coef32[30] * (b0 - b1);
		a2 = b2 + b3;
		a3 = coef32[30] * (b2 - b3);

		d8 = a0;
		d9 = a2 + a3;
		dA = a1;
		dB = a3;

		b0 = a[p+12] + a[p+15];
		b1 = a[p+13] + a[p+14];
		b2 = coef32[28] * (a[p+12] - a[p+15]);
		b3 = coef32[29] * (a[p+13] - a[p+14]);

		a0 = b0 + b1;
		a1 = coef32[30] * (b0 - b1);
		a2 = b2 + b3;
		a3 = coef32[30] * (b2 - b3);

		dC = a0;
		dD = a2 + a3;
		dE = a1;
		dF = a3;

		//////////////////////////////////////////////////

		double s1 = dC + dD;
		double s2 = dD + dE;
		double s3 = dE + dF;

		a[p   ] = d0;
		a[p+ 2] = d4 + d5;
		a[p+ 4] = d1;
		a[p+ 6] = d5 + d6;
		a[p+ 8] = d2;
		a[p+10] = d6 + d7;
		a[p+12] = d3;
		a[p+14] = d7;

		a[p+ 1] = d8 + s1;
		a[p+ 3] = d9 + s1;
		a[p+ 5] = d9 + s2;
		a[p+ 7] = dA + s2;
		a[p+ 9] = dA + s3;
		a[p+11] = dB + s3;
		a[p+13] = dB + dF;
		a[p+15] = dF;
	}



//	back_bf(1, 32, b, c);
//
// reordering for windowing:
//		flip around 256 if odd

	if (odd) {
		c[0] = a[0];
		c[512 - 16] = a[16] + a[17];

		q = 1;
		for (p = 2; p < 30; p+=2, q++) {
			c[512 - p*16     ] = a[q];
			c[512 - p*16 - 16] = a[q+16] + a[q+17];
		}

		c[ 2*16] = a[15];
		c[ 1*16] = a[31];
	} else {
		q = 0;
		for (p = 0; p < 30; p+=2, q++) {
			c[p*16+0 ] = a[q];
			c[p*16+16] = a[q+16] + a[q+17];
		}

		c[30*16] = a[15];
		c[31*16] = a[31];
	}
}




#define float_to_long(x) ((converter = (double)(x)+6755399441055744.0),*(long *)&converter)

void window_dual(const float vbuf[2][512], int vb_ptr, short *pcm) {
	double converter;
	int i, j;
	int si, bx;
	int coef;
	float *window, *window2;
	float sum1, sum2, sum3, sum4;
	long tmp;

	coef = 0;

/* first */

	i = 0;
	if (vb_ptr & 1) {
			sum1 = 0.0F;
			sum2 = 0.0F;

			window = &wincoef[i][(-vb_ptr) & 15];

			si = 256-i*16;

			sum1 -= window[ 0] * vbuf[0][si+ 0];
			sum2 -= window[ 0] * vbuf[1][si+ 0];
			sum1 += window[ 1] * vbuf[0][si+ 1];
			sum2 += window[ 1] * vbuf[1][si+ 1];
			sum1 -= window[ 2] * vbuf[0][si+ 2];
			sum2 -= window[ 2] * vbuf[1][si+ 2];
			sum1 += window[ 3] * vbuf[0][si+ 3];
			sum2 += window[ 3] * vbuf[1][si+ 3];
			sum1 -= window[ 4] * vbuf[0][si+ 4];
			sum2 -= window[ 4] * vbuf[1][si+ 4];
			sum1 += window[ 5] * vbuf[0][si+ 5];
			sum2 += window[ 5] * vbuf[1][si+ 5];
			sum1 -= window[ 6] * vbuf[0][si+ 6];
			sum2 -= window[ 6] * vbuf[1][si+ 6];
			sum1 += window[ 7] * vbuf[0][si+ 7];
			sum2 += window[ 7] * vbuf[1][si+ 7];
			sum1 -= window[ 8] * vbuf[0][si+ 8];
			sum2 -= window[ 8] * vbuf[1][si+ 8];
			sum1 += window[ 9] * vbuf[0][si+ 9];
			sum2 += window[ 9] * vbuf[1][si+ 9];
			sum1 -= window[10] * vbuf[0][si+10];
			sum2 -= window[10] * vbuf[1][si+10];
			sum1 += window[11] * vbuf[0][si+11];
			sum2 += window[11] * vbuf[1][si+11];
			sum1 -= window[12] * vbuf[0][si+12];
			sum2 -= window[12] * vbuf[1][si+12];
			sum1 += window[13] * vbuf[0][si+13];
			sum2 += window[13] * vbuf[1][si+13];
			sum1 -= window[14] * vbuf[0][si+14];
			sum2 -= window[14] * vbuf[1][si+14];
			sum1 += window[15] * vbuf[0][si+15];
			sum2 += window[15] * vbuf[1][si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2] = tmp;

			tmp = float_to_long(sum2);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2+1] = tmp;
	} else {
			sum1 = 0.0F;
			sum2 = 0.0F;

			window = &wincoef[i][(-vb_ptr) & 15];

			si = 256+i*16;

			sum1 += window[ 0] * vbuf[0][si+ 0];
			sum2 += window[ 0] * vbuf[1][si+ 0];
			sum1 -= window[ 1] * vbuf[0][si+ 1];
			sum2 -= window[ 1] * vbuf[1][si+ 1];
			sum1 += window[ 2] * vbuf[0][si+ 2];
			sum2 += window[ 2] * vbuf[1][si+ 2];
			sum1 -= window[ 3] * vbuf[0][si+ 3];
			sum2 -= window[ 3] * vbuf[1][si+ 3];
			sum1 += window[ 4] * vbuf[0][si+ 4];
			sum2 += window[ 4] * vbuf[1][si+ 4];
			sum1 -= window[ 5] * vbuf[0][si+ 5];
			sum2 -= window[ 5] * vbuf[1][si+ 5];
			sum1 += window[ 6] * vbuf[0][si+ 6];
			sum2 += window[ 6] * vbuf[1][si+ 6];
			sum1 -= window[ 7] * vbuf[0][si+ 7];
			sum2 -= window[ 7] * vbuf[1][si+ 7];
			sum1 += window[ 8] * vbuf[0][si+ 8];
			sum2 += window[ 8] * vbuf[1][si+ 8];
			sum1 -= window[ 9] * vbuf[0][si+ 9];
			sum2 -= window[ 9] * vbuf[1][si+ 9];
			sum1 += window[10] * vbuf[0][si+10];
			sum2 += window[10] * vbuf[1][si+10];
			sum1 -= window[11] * vbuf[0][si+11];
			sum2 -= window[11] * vbuf[1][si+11];
			sum1 += window[12] * vbuf[0][si+12];
			sum2 += window[12] * vbuf[1][si+12];
			sum1 -= window[13] * vbuf[0][si+13];
			sum2 -= window[13] * vbuf[1][si+13];
			sum1 += window[14] * vbuf[0][si+14];
			sum2 += window[14] * vbuf[1][si+14];
			sum1 -= window[15] * vbuf[0][si+15];
			sum2 -= window[15] * vbuf[1][si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2] = tmp;

			tmp = float_to_long(sum2);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2+1] = tmp;
	}

/*-- 1-16, last 15 --*/

	if (vb_ptr & 1) {
#ifdef USE_ASM
		window = &wincoef[1][16-vb_ptr];
		window2 = &wincoef[1][vb_ptr];

		const void *vbuf2 = (void *)&vbuf[0][240];

		__asm {
			push	esi
			push	vbuf2
			mov		ecx,window
			mov		edx,window2
			mov		ebx,pcm
			pop		esi
			push	edi
			mov		edi,ebx
			add		ebx,2*2
			add		edi,62*2
			push	ebp
			push	0
			push	0
			push	0
			push	0
			mov		ebp,15

			;ESI = FP input
			;EBX = PCM output (low)
			;ECX = window
			;EDX = window2
			;EDI = PCM output (high)
			;EBP = counter
			;
			;A sum = window[0...15] * vbuf[0]
			;B sum = window2[15...0] * vbuf[0]
			;C sum = window[0...15] * vbuf[1]
			;D sum = window2[15...0] * vbuf[1]

div0_loop:
			fld		dword ptr [esi+15*4+0*2048]	;Lx
			fmul	dword ptr [ecx+15*4]		;Ax
			fld		dword ptr [esi+15*4+0*2048]	;Lx Ax
			fmul	dword ptr [edx+ 0*4]		;Bx Ax
			fld		dword ptr [esi+15*4+1*2048]	;Rx Bx Ax
			fmul	dword ptr [ecx+15*4]		;Cx Bx Ax
			fld		dword ptr [esi+15*4+1*2048]	;Rx Cx Bx Ax
			fmul	dword ptr [edx+ 0*4]		;Dx Cx Bx Ax

			fld		dword ptr [esi+14*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+14*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+14*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 1*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+14*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+14*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+14*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 1*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+13*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+13*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+13*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 2*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+13*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+13*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+13*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 2*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+12*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+12*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+12*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 3*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+12*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+12*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+12*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 3*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+11*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+11*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+11*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 4*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+11*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+11*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+11*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 4*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+10*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+10*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+10*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 5*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+10*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+10*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+10*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 5*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 9*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 9*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 9*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 6*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 9*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 9*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 9*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 6*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 8*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 8*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 8*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 7*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 8*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 8*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 8*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 7*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 7*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 7*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 7*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 8*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 7*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 7*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 7*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 8*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 6*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 6*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 6*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 9*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 6*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 6*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 6*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 9*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 5*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 5*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 5*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+10*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 5*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 5*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 5*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+10*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 4*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 4*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 4*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+11*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 4*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 4*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 4*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+11*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 3*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 3*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 3*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+12*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 3*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 3*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 3*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+12*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 2*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 2*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 2*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+13*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 2*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 2*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 2*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+13*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 1*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 1*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 1*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+14*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 1*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 1*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 1*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+14*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 0*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 0*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 0*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+15*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 0*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 0*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 0*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+15*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			add		ecx,32*4
			add		edx,32*4
			sub		esi,16*4

			fistp	dword ptr [esp+12]			;[esp+8] = Dt
			fistp	dword ptr [esp+8]			;[esp+12] = Ct
			fistp	dword ptr [esp+4]			;[esp+0] = Bt
			fistp	dword ptr [esp+0]			;[esp+4] = At

			mov		eax,[esp+4]
			cmp		eax,-32768
			jge		div0_noclip1low
			mov		eax,-32768
div0_noclip1low:
			cmp		eax,32767
			jle		div0_noclip1high
			mov		eax,32767
div0_noclip1high:
			mov		[edi],ax

			mov		eax,[esp+12]
			cmp		eax,-32768
			jge		div0_noclip3low
			mov		eax,-32768
div0_noclip3low:
			cmp		eax,32767
			jle		div0_noclip3high
			mov		eax,32767
div0_noclip3high:
			mov		[edi+2],ax


			mov		eax,[esp+0]
			cmp		eax,-32768
			jge		div0_noclip2low
			mov		eax,-32768
div0_noclip2low:
			cmp		eax,32767
			jle		div0_noclip2high
			mov		eax,32767
div0_noclip2high:
			mov		[ebx],ax
			nop

			mov		eax,[esp+8]
			cmp		eax,-32768
			jge		div0_noclip4low
			mov		eax,-32768
div0_noclip4low:
			cmp		eax,32767
			jle		div0_noclip4high
			mov		eax,32767
div0_noclip4high:
			mov		[ebx+2],ax
			nop


			add		ebx,4
			sub		edi,4

			dec		ebp
			jne		div0_loop
			pop		eax
			pop		eax
			pop		eax
			pop		eax
			pop		ebp

			pop		edi
			mov		ebx,esi
			pop		esi
;			mov		window,ecx
;			mov		window2,edx
;			mov		u_ptr,ebx
		};
#else
		for (i = 1; i < 16; i++) {
			sum1 = 0.0F;
			sum2 = 0.0F;
			sum3 = 0.0F;
			sum4 = 0.0F;

			window = &wincoef[i][16-vb_ptr];
			window2 = &wincoef[i][vb_ptr];

			si = 256-i*16;

			sum1 -= window [ 0] * vbuf[0][si+ 0];
			sum2 += window2[15] * vbuf[0][si+ 0];
			sum3 -= window [ 0] * vbuf[1][si+ 0];
			sum4 += window2[15] * vbuf[1][si+ 0];
			sum1 += window [ 1] * vbuf[0][si+ 1];
			sum2 += window2[14] * vbuf[0][si+ 1];
			sum3 += window [ 1] * vbuf[1][si+ 1];
			sum4 += window2[14] * vbuf[1][si+ 1];
			sum1 -= window [ 2] * vbuf[0][si+ 2];
			sum2 += window2[13] * vbuf[0][si+ 2];
			sum3 -= window [ 2] * vbuf[1][si+ 2];
			sum4 += window2[13] * vbuf[1][si+ 2];
			sum1 += window [ 3] * vbuf[0][si+ 3];
			sum2 += window2[12] * vbuf[0][si+ 3];
			sum3 += window [ 3] * vbuf[1][si+ 3];
			sum4 += window2[12] * vbuf[1][si+ 3];
			sum1 -= window [ 4] * vbuf[0][si+ 4];
			sum2 += window2[11] * vbuf[0][si+ 4];
			sum3 -= window [ 4] * vbuf[1][si+ 4];
			sum4 += window2[11] * vbuf[1][si+ 4];
			sum1 += window [ 5] * vbuf[0][si+ 5];
			sum2 += window2[10] * vbuf[0][si+ 5];
			sum3 += window [ 5] * vbuf[1][si+ 5];
			sum4 += window2[10] * vbuf[1][si+ 5];
			sum1 -= window [ 6] * vbuf[0][si+ 6];
			sum2 += window2[ 9] * vbuf[0][si+ 6];
			sum3 -= window [ 6] * vbuf[1][si+ 6];
			sum4 += window2[ 9] * vbuf[1][si+ 6];
			sum1 += window [ 7] * vbuf[0][si+ 7];
			sum2 += window2[ 8] * vbuf[0][si+ 7];
			sum3 += window [ 7] * vbuf[1][si+ 7];
			sum4 += window2[ 8] * vbuf[1][si+ 7];
			sum1 -= window [ 8] * vbuf[0][si+ 8];
			sum2 += window2[ 7] * vbuf[0][si+ 8];
			sum3 -= window [ 8] * vbuf[1][si+ 8];
			sum4 += window2[ 7] * vbuf[1][si+ 8];
			sum1 += window [ 9] * vbuf[0][si+ 9];
			sum2 += window2[ 6] * vbuf[0][si+ 9];
			sum3 += window [ 9] * vbuf[1][si+ 9];
			sum4 += window2[ 6] * vbuf[1][si+ 9];
			sum1 -= window [10] * vbuf[0][si+10];
			sum2 += window2[ 5] * vbuf[0][si+10];
			sum3 -= window [10] * vbuf[1][si+10];
			sum4 += window2[ 5] * vbuf[1][si+10];
			sum1 += window [11] * vbuf[0][si+11];
			sum2 += window2[ 4] * vbuf[0][si+11];
			sum3 += window [11] * vbuf[1][si+11];
			sum4 += window2[ 4] * vbuf[1][si+11];
			sum1 -= window [12] * vbuf[0][si+12];
			sum2 += window2[ 3] * vbuf[0][si+12];
			sum3 -= window [12] * vbuf[1][si+12];
			sum4 += window2[ 3] * vbuf[1][si+12];
			sum1 += window [13] * vbuf[0][si+13];
			sum2 += window2[ 2] * vbuf[0][si+13];
			sum3 += window [13] * vbuf[1][si+13];
			sum4 += window2[ 2] * vbuf[1][si+13];
			sum1 -= window [14] * vbuf[0][si+14];
			sum2 += window2[ 1] * vbuf[0][si+14];
			sum3 -= window [14] * vbuf[1][si+14];
			sum4 += window2[ 1] * vbuf[1][si+14];
			sum1 += window [15] * vbuf[0][si+15];
			sum2 += window2[ 0] * vbuf[0][si+15];
			sum3 += window [15] * vbuf[1][si+15];
			sum4 += window2[ 0] * vbuf[1][si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2] = tmp;

			tmp = float_to_long(sum3);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2+1] = tmp;


			tmp = float_to_long(sum2);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[64-i*2] = tmp;

			tmp = float_to_long(sum4);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[64-i*2+1] = tmp;
		}
#endif
	} else {
#ifdef USE_ASM
		window = &wincoef[1][16-vb_ptr];
		window2 = &wincoef[1][vb_ptr];

		const void *vbuf2 = (void *)&vbuf[0][272];

		__asm {
			push	esi
			push	vbuf2
			mov		ecx,window
			mov		edx,window2
			mov		ebx,pcm
			pop		esi
			push	edi
			mov		edi,ebx
			add		ebx,2*2
			add		edi,62*2
			push	ebp
			push	0
			push	0
			push	0
			push	0
			mov		ebp,15

			;ESI = FP input
			;EBX = PCM output (low)
			;ECX = window
			;EDX = window2
			;EDI = PCM output (high)
			;EBP = counter
			;
			;A sum = window[0...15] * vbuf[0] (alternating)
			;B sum = window2[15...0] * vbuf[0]
			;C sum = window[0...15] * vbuf[1] (alternating)
			;D sum = window2[15...0] * vbuf[1]

div1_loop:
			fld		dword ptr [esi+ 0*4+0*2048]	;Lx
			fmul	dword ptr [ecx+ 0*4]		;Ax
			fld		dword ptr [esi+ 0*4+0*2048]	;Lx Ax
			fmul	dword ptr [edx+15*4]		;Bx Ax
			fld		dword ptr [esi+ 0*4+1*2048]	;Rx Bx Ax
			fmul	dword ptr [ecx+ 0*4]		;Cx Bx Ax
			fld		dword ptr [esi+ 0*4+1*2048]	;Rx Cx Bx Ax
			fmul	dword ptr [edx+15*4]		;Dx Cx Bx Ax

			fld		dword ptr [esi+ 1*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 1*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 1*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+14*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 1*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 1*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 1*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+14*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 2*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 2*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 2*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+13*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 2*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 2*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 2*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+13*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 3*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 3*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 3*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+12*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 3*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 3*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 3*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+12*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 4*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 4*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 4*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+11*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 4*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 4*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 4*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+11*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 5*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 5*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 5*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+10*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 5*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 5*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 5*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+10*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 6*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 6*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 6*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 9*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 6*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 6*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 6*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 9*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 7*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 7*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 7*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 8*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 7*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 7*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 7*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 8*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 8*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 8*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 8*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 7*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 8*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 8*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 8*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 7*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+ 9*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+ 9*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+ 9*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 6*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 9*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+ 9*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+ 9*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 6*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+10*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+10*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+10*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 5*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+10*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+10*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+10*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 5*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+11*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+11*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+11*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 4*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+11*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+11*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+11*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 4*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+12*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+12*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+12*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 3*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+12*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+12*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+12*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 3*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+13*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+13*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+13*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 2*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+13*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+13*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+13*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 2*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+14*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+14*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+14*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 1*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+14*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+14*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+14*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 1*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			faddp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			faddp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			fld		dword ptr [esi+15*4+0*2048]	;Lx Dt Ct Bt At
			fmul	dword ptr [ecx+15*4]		;Ax Dt Ct Bt At
			fld		dword ptr [esi+15*4+0*2048]	;Lx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 0*4]		;Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+15*4+1*2048]	;Rx Bx Ax Dt Ct Bt At
			fmul	dword ptr [ecx+15*4]		;Cx Bx Ax Dt Ct Bt At
			fld		dword ptr [esi+15*4+1*2048]	;Rx Cx Bx Ax Dt Ct Bt At
			fmul	dword ptr [edx+ 0*4]		;Dx Cx Bx Ax Dt Ct Bt At
			fxch	st(3)						;Ax Cx Bx Dx Dt Ct Bt At
			fsubp	st(7),st					;Cx Bx Dx Dt Ct Bt At
			fxch	st(1)						;Bx Cx Dx Dt Ct Bt At
			faddp	st(5),st					;Cx Dx Dt Ct Bt At
			fsubp	st(3),st					;Dx Dt Ct Bt At
			faddp	st(1),st					;Dt Ct Bt At

			add		ecx,32*4
			add		edx,32*4
			add		esi,16*4

			fistp	dword ptr [esp+12]			;[esp+8] = Dt
			fistp	dword ptr [esp+8]			;[esp+12] = Ct
			fistp	dword ptr [esp+4]			;[esp+0] = Bt
			fistp	dword ptr [esp+0]			;[esp+4] = At

			mov		eax,[esp+4]
			cmp		eax,-32768
			jge		div1_noclip1low
			mov		eax,-32768
div1_noclip1low:
			cmp		eax,32767
			jle		div1_noclip1high
			mov		eax,32767
div1_noclip1high:
			mov		[edi],ax

			mov		eax,[esp+12]
			cmp		eax,-32768
			jge		div1_noclip3low
			mov		eax,-32768
div1_noclip3low:
			cmp		eax,32767
			jle		div1_noclip3high
			mov		eax,32767
div1_noclip3high:
			mov		[edi+2],ax


			mov		eax,[esp+0]
			cmp		eax,-32768
			jge		div1_noclip2low
			mov		eax,-32768
div1_noclip2low:
			cmp		eax,32767
			jle		div1_noclip2high
			mov		eax,32767
div1_noclip2high:
			mov		[ebx],ax
			nop

			mov		eax,[esp+8]
			cmp		eax,-32768
			jge		div1_noclip4low
			mov		eax,-32768
div1_noclip4low:
			cmp		eax,32767
			jle		div1_noclip4high
			mov		eax,32767
div1_noclip4high:
			mov		[ebx+2],ax
			nop


			add		ebx,4
			sub		edi,4

			dec		ebp
			jne		div1_loop
			pop		eax
			pop		eax
			pop		eax
			pop		eax
			pop		ebp

			pop		edi
			mov		ebx,esi
			pop		esi
;			mov		window,ecx
;			mov		window2,edx
;			mov		u_ptr,ebx
		};
#else
		for (i = 1; i < 16; i++) {
			sum1 = 0.0F;
			sum2 = 0.0F;
			sum3 = 0.0F;
			sum4 = 0.0F;

			window = &wincoef[i][16-vb_ptr];
			window2 = &wincoef[i][vb_ptr & 15];

			si = 256+i*16;

			sum1 += window [ 0] * vbuf[0][si+ 0];
			sum2 += window2[15] * vbuf[0][si+ 0];
			sum3 += window [ 0] * vbuf[1][si+ 0];
			sum4 += window2[15] * vbuf[1][si+ 0];
			sum1 -= window [ 1] * vbuf[0][si+ 1];
			sum2 += window2[14] * vbuf[0][si+ 1];
			sum3 -= window [ 1] * vbuf[1][si+ 1];
			sum4 += window2[14] * vbuf[1][si+ 1];
			sum1 += window [ 2] * vbuf[0][si+ 2];
			sum2 += window2[13] * vbuf[0][si+ 2];
			sum3 += window [ 2] * vbuf[1][si+ 2];
			sum4 += window2[13] * vbuf[1][si+ 2];
			sum1 -= window [ 3] * vbuf[0][si+ 3];
			sum2 += window2[12] * vbuf[0][si+ 3];
			sum3 -= window [ 3] * vbuf[1][si+ 3];
			sum4 += window2[12] * vbuf[1][si+ 3];
			sum1 += window [ 4] * vbuf[0][si+ 4];
			sum2 += window2[11] * vbuf[0][si+ 4];
			sum3 += window [ 4] * vbuf[1][si+ 4];
			sum4 += window2[11] * vbuf[1][si+ 4];
			sum1 -= window [ 5] * vbuf[0][si+ 5];
			sum2 += window2[10] * vbuf[0][si+ 5];
			sum3 -= window [ 5] * vbuf[1][si+ 5];
			sum4 += window2[10] * vbuf[1][si+ 5];
			sum1 += window [ 6] * vbuf[0][si+ 6];
			sum2 += window2[ 9] * vbuf[0][si+ 6];
			sum3 += window [ 6] * vbuf[1][si+ 6];
			sum4 += window2[ 9] * vbuf[1][si+ 6];
			sum1 -= window [ 7] * vbuf[0][si+ 7];
			sum2 += window2[ 8] * vbuf[0][si+ 7];
			sum3 -= window [ 7] * vbuf[1][si+ 7];
			sum4 += window2[ 8] * vbuf[1][si+ 7];
			sum1 += window [ 8] * vbuf[0][si+ 8];
			sum2 += window2[ 7] * vbuf[0][si+ 8];
			sum3 += window [ 8] * vbuf[1][si+ 8];
			sum4 += window2[ 7] * vbuf[1][si+ 8];
			sum1 -= window [ 9] * vbuf[0][si+ 9];
			sum2 += window2[ 6] * vbuf[0][si+ 9];
			sum3 -= window [ 9] * vbuf[1][si+ 9];
			sum4 += window2[ 6] * vbuf[1][si+ 9];
			sum1 += window [10] * vbuf[0][si+10];
			sum2 += window2[ 5] * vbuf[0][si+10];
			sum3 += window [10] * vbuf[1][si+10];
			sum4 += window2[ 5] * vbuf[1][si+10];
			sum1 -= window [11] * vbuf[0][si+11];
			sum2 += window2[ 4] * vbuf[0][si+11];
			sum3 -= window [11] * vbuf[1][si+11];
			sum4 += window2[ 4] * vbuf[1][si+11];
			sum1 += window [12] * vbuf[0][si+12];
			sum2 += window2[ 3] * vbuf[0][si+12];
			sum3 += window [12] * vbuf[1][si+12];
			sum4 += window2[ 3] * vbuf[1][si+12];
			sum1 -= window [13] * vbuf[0][si+13];
			sum2 += window2[ 2] * vbuf[0][si+13];
			sum3 -= window [13] * vbuf[1][si+13];
			sum4 += window2[ 2] * vbuf[1][si+13];
			sum1 += window [14] * vbuf[0][si+14];
			sum2 += window2[ 1] * vbuf[0][si+14];
			sum3 += window [14] * vbuf[1][si+14];
			sum4 += window2[ 1] * vbuf[1][si+14];
			sum1 -= window [15] * vbuf[0][si+15];
			sum2 += window2[ 0] * vbuf[0][si+15];
			sum3 -= window [15] * vbuf[1][si+15];
			sum4 += window2[ 0] * vbuf[1][si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2] = tmp;

			tmp = float_to_long(sum3);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i*2+1] = tmp;
		

			tmp = float_to_long(sum2);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[64-i*2] = tmp;
		
			tmp = float_to_long(sum4);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[64-i*2+1] = tmp;
		
		}
#endif
	}



	/*--  special case --*/
	coef = 0;
	bx = (vb_ptr+1)&15;

	sum1 = sum2 = 0.0F;

	for (j = 0; j < 8; j++) {
		sum1 += wincoef2[coef] * vbuf[0][bx];
		sum2 += wincoef2[coef] * vbuf[1][bx];	bx = (bx+2)&15;
		++coef;
	}

	tmp = (long)sum1;

	if (tmp > 32767)
		tmp = 32767;
	else if (tmp < -32768)
		tmp = -32768;
	pcm[32] = tmp;

	tmp = (long)sum2;

	if (tmp > 32767)
		tmp = 32767;
	else if (tmp < -32768)
		tmp = -32768;
	pcm[33] = tmp;
}

void window_mono(const float *vbuf, int vb_ptr, short *pcm) {
	double converter;
	int i, j;
	int si, bx;
	int coef;
	float *window, *window2;
	float sum1, sum2;
	long tmp;

	coef = 0;

/* first */

	i = 0;
	if (vb_ptr & 1) {
			sum1 = 0.0F;

			window = &wincoef[i][(-vb_ptr) & 15];

			si = 256-i*16;

			sum1 -= window[ 0] * vbuf[si+ 0];
			sum1 += window[ 1] * vbuf[si+ 1];
			sum1 -= window[ 2] * vbuf[si+ 2];
			sum1 += window[ 3] * vbuf[si+ 3];
			sum1 -= window[ 4] * vbuf[si+ 4];
			sum1 += window[ 5] * vbuf[si+ 5];
			sum1 -= window[ 6] * vbuf[si+ 6];
			sum1 += window[ 7] * vbuf[si+ 7];
			sum1 -= window[ 8] * vbuf[si+ 8];
			sum1 += window[ 9] * vbuf[si+ 9];
			sum1 -= window[10] * vbuf[si+10];
			sum1 += window[11] * vbuf[si+11];
			sum1 -= window[12] * vbuf[si+12];
			sum1 += window[13] * vbuf[si+13];
			sum1 -= window[14] * vbuf[si+14];
			sum1 += window[15] * vbuf[si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i] = tmp;
	} else {
			sum1 = 0.0F;

			window = &wincoef[i][(-vb_ptr) & 15];

			si = 256+i*16;

			sum1 += window[ 0] * vbuf[si+ 0];
			sum1 -= window[ 1] * vbuf[si+ 1];
			sum1 += window[ 2] * vbuf[si+ 2];
			sum1 -= window[ 3] * vbuf[si+ 3];
			sum1 += window[ 4] * vbuf[si+ 4];
			sum1 -= window[ 5] * vbuf[si+ 5];
			sum1 += window[ 6] * vbuf[si+ 6];
			sum1 -= window[ 7] * vbuf[si+ 7];
			sum1 += window[ 8] * vbuf[si+ 8];
			sum1 -= window[ 9] * vbuf[si+ 9];
			sum1 += window[10] * vbuf[si+10];
			sum1 -= window[11] * vbuf[si+11];
			sum1 += window[12] * vbuf[si+12];
			sum1 -= window[13] * vbuf[si+13];
			sum1 += window[14] * vbuf[si+14];
			sum1 -= window[15] * vbuf[si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i] = tmp;
	}

/*-- 1-16, last 15 --*/

	if (vb_ptr & 1) {
#ifdef USE_ASM
		window = &wincoef[1][16-vb_ptr];
		window2 = &wincoef[1][vb_ptr];

		const float *vbuf2 = vbuf+240;

		__asm {
			push	esi
			push	vbuf2
			mov		edx,window
			mov		ecx,window2
			mov		ebx,pcm
			pop		esi
			push	edi
			mov		edi,ebx
			add		ebx,2
			add		edi,62
			cmp		eax,2
			push	ebp
			push	0
			push	0
			mov		ebp,15
div0_loop:
			fld		dword ptr [esi+ 0*4]	;x0
			fmul	dword ptr [ecx+15*4]	;A0
			fld		dword ptr [esi+ 0*4]	;x0 A0
			fmul	dword ptr [edx+ 0*4]	;B0 A0
			fld		dword ptr [esi+ 1*4]	;x1 B0 A0
			fmul	dword ptr [ecx+14*4]	;A1 B0 A0
			fld		dword ptr [esi+ 1*4]	;x1 A1 B0 A0
			fmul	dword ptr [edx+ 1*4]	;B1 A1 B0 A0
			fld		dword ptr [esi+ 2*4]	;x2 B1 A1 B0 A0
			fxch	st(2)					;A1 B1 x2 B0 A0
			faddp	st(4),st				;B1 x2 B0 At
			fsubp	st(2),st				;x2 Bt At
			fmul	dword ptr [ecx+13*4]	;A2 Bt At
			fld		dword ptr [esi+ 2*4]	;x2 A2 Bt At
			fmul	dword ptr [edx+ 2*4]	;B2 A2 Bt At

			fld		dword ptr [esi+ 3*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+12*4]	;A3 Bt At
			fld		dword ptr [esi+ 3*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 3*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 4*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+11*4]	;A3 Bt At
			fld		dword ptr [esi+ 4*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 4*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 5*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+10*4]	;A3 Bt At
			fld		dword ptr [esi+ 5*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 5*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 6*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 9*4]	;A3 Bt At
			fld		dword ptr [esi+ 6*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 6*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 7*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 8*4]	;A3 Bt At
			fld		dword ptr [esi+ 7*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 7*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 8*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 7*4]	;A3 Bt At
			fld		dword ptr [esi+ 8*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 8*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 9*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 6*4]	;A3 Bt At
			fld		dword ptr [esi+ 9*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 9*4]	;B3 A3 Bt At

			fld		dword ptr [esi+10*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 5*4]	;A3 Bt At
			fld		dword ptr [esi+10*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+10*4]	;B3 A3 Bt At

			fld		dword ptr [esi+11*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 4*4]	;A3 Bt At
			fld		dword ptr [esi+11*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+11*4]	;B3 A3 Bt At

			fld		dword ptr [esi+12*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 3*4]	;A3 Bt At
			fld		dword ptr [esi+12*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+12*4]	;B3 A3 Bt At

			fld		dword ptr [esi+13*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 2*4]	;A3 Bt At
			fld		dword ptr [esi+13*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+13*4]	;B3 A3 Bt At

			fld		dword ptr [esi+14*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 1*4]	;A3 Bt At
			fld		dword ptr [esi+14*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+14*4]	;B3 A3 Bt At

			fld		dword ptr [esi+15*4]	;xF BE AE Bt At
			fxch	st(2)					;AE BE xF Bt At
			faddp	st(4),st				;BE xF Bt At
			faddp	st(2),st				;xF Bt At
			fmul	dword ptr [ecx+ 0*4]	;AF Bt At
			fld		dword ptr [esi+15*4]	;xF AF Bt At
			fmul	dword ptr [edx+15*4]	;BF AF Bt At

			add		ecx,32*4
			add		edx,32*4
			sub		esi,16*4

			fsubp	st(2),st				;AF Bt At
			faddp	st(2),st				;Bt At
			fchs
			fistp	dword ptr [esp+0]		;[esp+0] = outf3
			fistp	dword ptr [esp+4]		;[esp+4] = outf1

			mov		eax,[esp+4]
			cmp		eax,-32768
			jge		div0_noclip1low
			mov		eax,-32768
div0_noclip1low:
			cmp		eax,32767
			jle		div0_noclip1high
			mov		eax,32767
div0_noclip1high:
			mov		[edi],ax
			mov		eax,[esp+0]
			cmp		eax,-32768
			jge		div0_noclip2low
			mov		eax,-32768
div0_noclip2low:
			cmp		eax,32767
			jle		div0_noclip2high
			mov		eax,32767
div0_noclip2high:
			mov		[ebx],ax
			nop

			add		ebx,2
			sub		edi,2

			dec		ebp
			jne		div0_loop
			pop		eax
			pop		eax
			pop		ebp

			pop		edi
			mov		ebx,esi
			pop		esi
;			mov		window,ecx
;			mov		window2,edx
;			mov		u_ptr,ebx
		};
#else
		for (i = 1; i < 16; i++) {
			sum1 = 0.0F;
			sum2 = 0.0F;

			window = &wincoef[i][16-vb_ptr];
			window2 = &wincoef[i][vb_ptr];

			si = 256-i*16;

			sum1 -= window [ 0] * vbuf[si+ 0];
			sum2 += window2[15] * vbuf[si+ 0];
			sum1 += window [ 1] * vbuf[si+ 1];
			sum2 += window2[14] * vbuf[si+ 1];
			sum1 -= window [ 2] * vbuf[si+ 2];
			sum2 += window2[13] * vbuf[si+ 2];
			sum1 += window [ 3] * vbuf[si+ 3];
			sum2 += window2[12] * vbuf[si+ 3];
			sum1 -= window [ 4] * vbuf[si+ 4];
			sum2 += window2[11] * vbuf[si+ 4];
			sum1 += window [ 5] * vbuf[si+ 5];
			sum2 += window2[10] * vbuf[si+ 5];
			sum1 -= window [ 6] * vbuf[si+ 6];
			sum2 += window2[ 9] * vbuf[si+ 6];
			sum1 += window [ 7] * vbuf[si+ 7];
			sum2 += window2[ 8] * vbuf[si+ 7];
			sum1 -= window [ 8] * vbuf[si+ 8];
			sum2 += window2[ 7] * vbuf[si+ 8];
			sum1 += window [ 9] * vbuf[si+ 9];
			sum2 += window2[ 6] * vbuf[si+ 9];
			sum1 -= window [10] * vbuf[si+10];
			sum2 += window2[ 5] * vbuf[si+10];
			sum1 += window [11] * vbuf[si+11];
			sum2 += window2[ 4] * vbuf[si+11];
			sum1 -= window [12] * vbuf[si+12];
			sum2 += window2[ 3] * vbuf[si+12];
			sum1 += window [13] * vbuf[si+13];
			sum2 += window2[ 2] * vbuf[si+13];
			sum1 -= window [14] * vbuf[si+14];
			sum2 += window2[ 1] * vbuf[si+14];
			sum1 += window [15] * vbuf[si+15];
			sum2 += window2[ 0] * vbuf[si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i] = tmp;



			tmp = float_to_long(sum2);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[32-i] = tmp;

		}
#endif
	} else {
#ifdef USE_ASM
		window = &wincoef[1][16-vb_ptr];
		window2 = &wincoef[1][vb_ptr];

		const float *vbuf2 = vbuf+272;

		__asm {
			push	esi
			push	vbuf2
			mov		edx,window
			mov		ecx,window2
			mov		ebx,pcm
			pop		esi
			push	edi
			mov		edi,ebx
			add		ebx,2
			add		edi,62
			cmp		eax,2
			push	ebp
			push	0
			push	0
			mov		ebp,15
div1_loop:
			fld		dword ptr [esi+ 0*4]	;x0
			fmul	dword ptr [ecx+15*4]	;A0
			fld		dword ptr [esi+ 0*4]	;x0 A0
			fmul	dword ptr [edx+ 0*4]	;B0 A0
			fld		dword ptr [esi+ 1*4]	;x1 B0 A0
			fmul	dword ptr [ecx+14*4]	;A1 B0 A0
			fld		dword ptr [esi+ 1*4]	;x1 A1 B0 A0
			fmul	dword ptr [edx+ 1*4]	;B1 A1 B0 A0
			fld		dword ptr [esi+ 2*4]	;x2 B1 A1 B0 A0
			fxch	st(2)					;A1 B1 x2 B0 A0
			faddp	st(4),st				;B1 x2 B0 At
			fsubp	st(2),st				;x2 Bt At
			fmul	dword ptr [ecx+13*4]	;A2 Bt At
			fld		dword ptr [esi+ 2*4]	;x2 A2 Bt At
			fmul	dword ptr [edx+ 2*4]	;B2 A2 Bt At

			fld		dword ptr [esi+ 3*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+12*4]	;A3 Bt At
			fld		dword ptr [esi+ 3*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 3*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 4*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+11*4]	;A3 Bt At
			fld		dword ptr [esi+ 4*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 4*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 5*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+10*4]	;A3 Bt At
			fld		dword ptr [esi+ 5*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 5*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 6*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 9*4]	;A3 Bt At
			fld		dword ptr [esi+ 6*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 6*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 7*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 8*4]	;A3 Bt At
			fld		dword ptr [esi+ 7*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 7*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 8*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 7*4]	;A3 Bt At
			fld		dword ptr [esi+ 8*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 8*4]	;B3 A3 Bt At

			fld		dword ptr [esi+ 9*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 6*4]	;A3 Bt At
			fld		dword ptr [esi+ 9*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+ 9*4]	;B3 A3 Bt At

			fld		dword ptr [esi+10*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 5*4]	;A3 Bt At
			fld		dword ptr [esi+10*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+10*4]	;B3 A3 Bt At

			fld		dword ptr [esi+11*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 4*4]	;A3 Bt At
			fld		dword ptr [esi+11*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+11*4]	;B3 A3 Bt At

			fld		dword ptr [esi+12*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 3*4]	;A3 Bt At
			fld		dword ptr [esi+12*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+12*4]	;B3 A3 Bt At

			fld		dword ptr [esi+13*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			faddp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 2*4]	;A3 Bt At
			fld		dword ptr [esi+13*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+13*4]	;B3 A3 Bt At

			fld		dword ptr [esi+14*4]	;x3 B2 A2 Bt At
			fxch	st(2)					;A2 B2 x3 Bt At
			faddp	st(4),st				;B2 x3 Bt At
			fsubp	st(2),st				;x3 Bt At
			fmul	dword ptr [ecx+ 1*4]	;A3 Bt At
			fld		dword ptr [esi+14*4]	;x3 A3 Bt At
			fmul	dword ptr [edx+14*4]	;B3 A3 Bt At

			fld		dword ptr [esi+15*4]	;xF BE AE Bt At
			fxch	st(2)					;AE BE xF Bt At
			faddp	st(4),st				;BE xF Bt At
			faddp	st(2),st				;xF Bt At
			fmul	dword ptr [ecx+ 0*4]	;AF Bt At
			fld		dword ptr [esi+15*4]	;xF AF Bt At
			fmul	dword ptr [edx+15*4]	;BF AF Bt At

			add		ecx,32*4
			add		edx,32*4
			add		esi,16*4

			fsubp	st(2),st				;AF Bt At
			faddp	st(2),st				;Bt At
			fistp	dword ptr [esp+0]		;[esp+0] = outf3
			fistp	dword ptr [esp+4]		;[esp+4] = outf1

			mov		eax,[esp+0]
			cmp		eax,-32768
			jge		div1_noclip1low
			mov		eax,-32768
div1_noclip1low:
			cmp		eax,32767
			jle		div1_noclip1high
			mov		eax,32767
div1_noclip1high:
			mov		[ebx],ax
			mov		eax,[esp+4]
			cmp		eax,-32768
			jge		div1_noclip2low
			mov		eax,-32768
div1_noclip2low:
			cmp		eax,32767
			jle		div1_noclip2high
			mov		eax,32767
div1_noclip2high:
			mov		[edi],ax
			nop

			add		ebx,2
			sub		edi,2

			dec		ebp
			jne		div1_loop
			pop		eax
			pop		eax
			pop		ebp

			pop		edi
			mov		ebx,esi
			pop		esi
;			mov		window,ecx
;			mov		window2,edx
;			mov		u_ptr,ebx
		};
#else
		for (i = 1; i < 16; i++) {
			sum1 = 0.0F;
			sum2 = 0.0F;

			window = &wincoef[i][16-vb_ptr];
			window2 = &wincoef[i][vb_ptr & 15];

			si = 256+i*16;

			sum1 += window [ 0] * vbuf[si+ 0];
			sum2 += window2[15] * vbuf[si+ 0];
			sum1 -= window [ 1] * vbuf[si+ 1];
			sum2 += window2[14] * vbuf[si+ 1];
			sum1 += window [ 2] * vbuf[si+ 2];
			sum2 += window2[13] * vbuf[si+ 2];
			sum1 -= window [ 3] * vbuf[si+ 3];
			sum2 += window2[12] * vbuf[si+ 3];
			sum1 += window [ 4] * vbuf[si+ 4];
			sum2 += window2[11] * vbuf[si+ 4];
			sum1 -= window [ 5] * vbuf[si+ 5];
			sum2 += window2[10] * vbuf[si+ 5];
			sum1 += window [ 6] * vbuf[si+ 6];
			sum2 += window2[ 9] * vbuf[si+ 6];
			sum1 -= window [ 7] * vbuf[si+ 7];
			sum2 += window2[ 8] * vbuf[si+ 7];
			sum1 += window [ 8] * vbuf[si+ 8];
			sum2 += window2[ 7] * vbuf[si+ 8];
			sum1 -= window [ 9] * vbuf[si+ 9];
			sum2 += window2[ 6] * vbuf[si+ 9];
			sum1 += window [10] * vbuf[si+10];
			sum2 += window2[ 5] * vbuf[si+10];
			sum1 -= window [11] * vbuf[si+11];
			sum2 += window2[ 4] * vbuf[si+11];
			sum1 += window [12] * vbuf[si+12];
			sum2 += window2[ 3] * vbuf[si+12];
			sum1 -= window [13] * vbuf[si+13];
			sum2 += window2[ 2] * vbuf[si+13];
			sum1 += window [14] * vbuf[si+14];
			sum2 += window2[ 1] * vbuf[si+14];
			sum1 -= window [15] * vbuf[si+15];
			sum2 += window2[ 0] * vbuf[si+15];

			tmp = float_to_long(sum1);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[i] = tmp;

		

			tmp = float_to_long(sum2);
			if (tmp > 32767)
				tmp = 32767;
			else if (tmp < -32768)
				tmp = -32768;

			pcm[32-i] = tmp;
		
		}
#endif
	}



	/*--  special case --*/
	coef = 0;
	bx = (vb_ptr+1)&15;

	sum1 = 0.0F;

	for (j = 0; j < 8; j++) {
		sum1 += wincoef2[coef++] * vbuf[bx];	bx = (bx+2)&15;
	}

	tmp = (long)sum1;

	if (tmp > 32767)
		tmp = 32767;
	else if (tmp < -32768)
		tmp = -32768;
	pcm[16] = tmp;

}

static bool pp_init_flag = true;

static void pp_init() {
	int i,j;

	pp_init_flag = false;

	for(i=0; i<16; i++)
		for(j=0; j<16; j++)
			wincoef[i][j+16] = wincoef[i][j];
}

void AMPDecoder::polyphase(float *band_l, float *band_r, short *samples, bool invert_odd) {
	if (pp_init_flag)
		pp_init();
	
	fdct32(band_l, window[0] + winptr, !!(winptr & 1), invert_odd);
	if (band_r) {
		fdct32(band_r, window[1] + winptr, !!(winptr & 1), invert_odd);
//		window_dual(window[0], winptr, samples);
//		window_dual(window[1], winptr, samples + 1);
		window_dual(window, winptr, samples);
	} else
		window_mono(window[0], winptr, samples);

	winptr = (winptr - 1) & 15;
}

#if 0
void AMPDecoder::polyphase_single(float *band, int ch, short *samples) {
	if (pp_init_flag)
		pp_init();

	fdct32(band, window[ch] + winptr, !!(winptr&1), false);
	window_dual(window[ch], winptr, samples);

	winptr = (winptr - 1) & 15;
}
#endif

void AMPDecoder::polyphase_retract(int amount) {
	winptr = (winptr +
		amount) & 15;
}
