#include <crtdbg.h>
#include <math.h>
#include <string.h>

#define M_PI (3.141592653589793238462643)
#define PI (3.141592653589793238462643)

//#define MDCT_BLOCKTYPE_PROFILE

#ifdef MDCT_BLOCKTYPE_PROFILE
	#include <windows.h>
#endif

static double win[4][36];

static double w[18];
static double w2[9];
static double coef[9][4];

static double v[6];
static double v2[3];
static const double coef87 = 1.7320508075689;

void init_mdct()
{
	int k, p, n;
	double t, pi;
	
	/*--- 18 point --*/
	n = 18;
	pi = 4.0 * atan(1.0);
	t = pi / (4 * n);
	for (p = 0; p < n; p++)
		w[p] = (double) (2.0 * cos(t * (2 * p + 1)));
	for (p = 0; p < 9; p++)
		w2[p] = (double) 2.0 *cos(2 * t * (2 * p + 1));
	
	t = pi / (2 * n);
	for (k = 0; k < 9; k++)
	{
		for (p = 0; p < 4; p++)
			coef[k][p] = (double) cos(t * (2 * k) * (2 * p + 1));
	}
	
	/*--- 6 point */
	n = 6;
	pi = 4.0 * atan(1.0);
	t = pi / (4 * n);
	for (p = 0; p < n; p++)
		v[p] = (double) 2.0 *cos(t * (2 * p + 1));
	
	for (p = 0; p < 3; p++)
		v2[p] = (double) 2.0 *cos(2 * t * (2 * p + 1));
	
#if 0
	t = pi / (2 * n);
	k = 1;
	p = 0;
	coef87 = (double) cos(t * (2 * k) * (2 * p + 1));
#endif
	
	/* adjust scaling to save a few mults */
	for (p = 0; p < 6; p++)
		v[p] = v[p] / 2.0f;

	//coef87 = (double) 2.0 * coef87;
	
	int i;

    /* type 0  - invert for speed */
      for(i=0;i<36;i++)
         win[0][i] = -sin( PI/36 *(i+0.5) );

    /* type 1 - invert for speed*/
      for(i=0;i<18;i++)
         win[1][i] = -sin( PI/36 *(i+0.5) );
//      for(i=18;i<24;i++)
//         win[1][i] = 1.0;
      for(i=24;i<30;i++)
         win[1][i] = -sin( PI/12 *(i+0.5-18) );
//      for(i=30;i<36;i++)
//         win[1][i] = 0.0;

    /* type 3 - invert for speed*/
//      for(i=0;i<6;i++)
//         win[3][i] = 0.0;
      for(i=6;i<12;i++)
         win[3][i] = -sin( PI/12 *(i+0.5-6) );
//      for(i=12;i<18;i++)
//         win[3][i] =1.0;
      for(i=18;i<36;i++)
         win[3][i] = -sin( PI/36*(i+0.5) );

    /* type 2*/
      for(i=0;i<12;i++)
         win[2][i] = sin( PI/12*(i+0.5) ) ;
//      for(i=12;i<36;i++)
//         win[2][i] = 0.0 ;
	
	return;
}

void imdct18(double f[18], float src[18])	/* 18 point */
{
   int p;
   double a[9], b[9];
   double ap, bp, a8p, b8p;
   double g1, g2;


   for (p = 0; p < 4; p++)
   {
      g1 = w[p] * src[p];
      g2 = w[17 - p] * src[17 - p];
      ap = g1 + g2;		// a[p]

      bp = w2[p] * (g1 - g2);	// b[p]

      g1 = w[8 - p] * src[8 - p];
      g2 = w[9 + p] * src[9 + p];
      a8p = g1 + g2;		// a[8-p]

      b8p = w2[8 - p] * (g1 - g2);	// b[8-p]

      a[p] = ap + a8p;
      a[5 + p] = ap - a8p;
      b[p] = bp + b8p;
      b[5 + p] = bp - b8p;
   }
   g1 = w[p] * src[p];
   g2 = w[17 - p] * src[17 - p];
   a[p] = g1 + g2;
   b[p] = w2[p] * (g1 - g2);


   f[0] = 0.5f * (a[0] + a[1] + a[2] + a[3] + a[4]);
   f[1] = 0.5f * (b[0] + b[1] + b[2] + b[3] + b[4]);

   f[2] = coef[1][0] * a[5] + coef[1][1] * a[6] + coef[1][2] * a[7] + coef[1][3] * a[8];
   f[3] = coef[1][0] * b[5] + coef[1][1] * b[6] + coef[1][2] * b[7] + coef[1][3] * b[8] - f[1];
   f[1] = f[1] - f[0];
   f[2] = f[2] - f[1];

   f[4] = coef[2][0] * a[0] + coef[2][1] * a[1] + coef[2][2] * a[2] + coef[2][3] * a[3] - a[4];
   f[5] = coef[2][0] * b[0] + coef[2][1] * b[1] + coef[2][2] * b[2] + coef[2][3] * b[3] - b[4] - f[3];
   f[3] = f[3] - f[2];
   f[4] = f[4] - f[3];

   f[6] = coef[3][0] * (a[5] - a[7] - a[8]);
   f[7] = coef[3][0] * (b[5] - b[7] - b[8]) - f[5];
   f[5] = f[5] - f[4];
   f[6] = f[6] - f[5];

   f[8] = coef[4][0] * a[0] + coef[4][1] * a[1] + coef[4][2] * a[2] + coef[4][3] * a[3] + a[4];
   f[9] = coef[4][0] * b[0] + coef[4][1] * b[1] + coef[4][2] * b[2] + coef[4][3] * b[3] + b[4] - f[7];
   f[7] = f[7] - f[6];
   f[8] = f[8] - f[7];

   f[10] = coef[5][0] * a[5] + coef[5][1] * a[6] + coef[5][2] * a[7] + coef[5][3] * a[8];
   f[11] = coef[5][0] * b[5] + coef[5][1] * b[6] + coef[5][2] * b[7] + coef[5][3] * b[8] - f[9];
   f[9] = f[9] - f[8];
   f[10] = f[10] - f[9];

   f[12] = 0.5f * (a[0] + a[2] + a[3]) - a[1] - a[4];
   f[13] = 0.5f * (b[0] + b[2] + b[3]) - b[1] - b[4] - f[11];
   f[11] = f[11] - f[10];
   f[12] = f[12] - f[11];

   f[14] = coef[7][0] * a[5] + coef[7][1] * a[6] + coef[7][2] * a[7] + coef[7][3] * a[8];
   f[15] = coef[7][0] * b[5] + coef[7][1] * b[6] + coef[7][2] * b[7] + coef[7][3] * b[8] - f[13];
   f[13] = f[13] - f[12];
   f[14] = f[14] - f[13];

   f[16] = coef[8][0] * a[0] + coef[8][1] * a[1] + coef[8][2] * a[2] + coef[8][3] * a[3] + a[4];
   f[17] = coef[8][0] * b[0] + coef[8][1] * b[1] + coef[8][2] * b[2] + coef[8][3] * b[3] + b[4] - f[15];
   f[15] = f[15] - f[14];
   f[16] = f[16] - f[15];
   f[17] = f[17] - f[16];


   return;
}
/*--------------------------------------------------------------------*/
/* does 3, 6 pt dct.  changes order from f[i][window] c[window][i] */
void imdct6_3(double f[], float src[])	/* 6 point */
{
   int w;
   double buf[18];
   double *a, *c;		// b[i] = a[3+i]

   double g1, g2;
   double a02, b02;

   c = f;
   a = buf;
   for (w = 0; w < 3; w++)
   {
      g1 = v[0] * src[3 * 0];
      g2 = v[5] * src[3 * 5];
      a[0] = g1 + g2;
      a[3 + 0] = v2[0] * (g1 - g2);

      g1 = v[1] * src[3 * 1];
      g2 = v[4] * src[3 * 4];
      a[1] = g1 + g2;
      a[3 + 1] = v2[1] * (g1 - g2);

      g1 = v[2] * src[3 * 2];
      g2 = v[3] * src[3 * 3];
      a[2] = g1 + g2;
      a[3 + 2] = v2[2] * (g1 - g2);

      a += 6;
      src++;
   }

   a = buf;
   for (w = 0; w < 3; w++)
   {
      a02 = (a[0] + a[2]);
      b02 = (a[3 + 0] + a[3 + 2]);
      c[0] = a02 + a[1];
      c[1] = b02 + a[3 + 1];
      c[2] = coef87 * (a[0] - a[2]);
      c[3] = coef87 * (a[3 + 0] - a[3 + 2]) - c[1];
      c[1] = c[1] - c[0];
      c[2] = c[2] - c[1];
      c[4] = a02 - a[1] - a[1];
      c[5] = b02 - a[3 + 1] - a[3 + 1] - c[3];
      c[3] = c[3] - c[2];
      c[4] = c[4] - c[3];
      c[5] = c[5] - c[4];
      a += 6;
      c += 6;
   }

   return;
}

void inv_mdct(float *in0,
			  float out2[18][32],
				float *prevblock,
				int block_type) {
	int i,m,p;

#ifdef MDCT_BLOCKTYPE_PROFILE
	static int prof_bt[4]={0,0,0,0};
	static int prof_bt_total=0;

	prof_bt[block_type]++;
	prof_bt_total++;

	if (!(prof_bt_total&262143)) {
		static char buf[256];

		wsprintf(buf, "%d MDCTs, %d%% type 0, %d%% type 1, %d%% type 2, %d%% type 3\n"
			,prof_bt_total
			,(prof_bt[0]*100)/prof_bt_total
			,(prof_bt[1]*100)/prof_bt_total
			,(prof_bt[2]*100)/prof_bt_total
			,(prof_bt[3]*100)/prof_bt_total);
		OutputDebugString(buf);
	}
#endif

	double  sum;
	double in[18], out[36];

	/*
	for(p=0;p<36;p++) {
		sum = 0.0;
		for(m=0;m<18;m++)
			sum += in[m] * cos( (PI/72) * (2*p+1+18)*(2*m+1));

		out[p] = sum;
	}
	*/

	if (block_type == 2) {
		int j;

		imdct6_3(in, in0);

		for(i=0;i<36;i++)
			out[i]=0.0;

		for(i=0;i<3;i++){
			for(p=0;p<3;p++) {
				out[6*i+6+p] += in[i*6+p+3] * win[2][p];
				out[6*i+11-p] += -in[i*6+p+3] * win[2][5-p];
				out[6*i+14-p] += -in[i*6+p] * win[2][3+p];
				out[6*i+15+p] += -in[i*6+p] * win[2][2-p];
			}
		}

		for(i=0; i<18; i++) {
			out2[i][0] = out[i] + prevblock[i];
			prevblock[i] = out[i+18];
		}

	} else {

		imdct18(in, in0);

		switch(block_type) {
		case 0:
//			for(i=0; i<18; i++) {
//				out[i   ] *= win[0][i];
//				out[35-i] *= win[0][i];
//			}

//			for(i=0; i<9; i++) {
//				out[i] = in[i+9];
//				out[17-i] = -in[i+9];
//				out[26-i] = -in[i];
//				out[27+i] = -in[i];
//			}

			for(i=0; i<9; i++) {
				out2[i  ][0] = prevblock[i  ] - in[ 9+i]*win[0][i  ];
				out2[i+9][0] = prevblock[i+9] + in[17-i]*win[0][i+9];
			}

			for(i=0; i<4; i++) {
				double in1 = in[8-i];
				double in2 = in[i];

				prevblock[   i] = in1 * win[0][17-i];
				prevblock[ 9+i] = in2 * win[0][ 8-i];
				prevblock[ 8-i] = in2 * win[0][ 9+i];
				prevblock[17-i] = in1 * win[0][   i];
			}

			prevblock[4  ] = in[4] * win[0][13];
			prevblock[4+9] = in[4] * win[0][ 4];

			break;
		case 1:
			for(i=0; i<9; i++) {
				out[i] = -in[i+9];
				out[17-i] = in[i+9];
				out[26-i] = in[i];
				out[27+i] = in[i];
			}

			for(i=0; i<18; i++)
				out[i] *= win[1][i];

			for(i=18; i<24; i++)
				out[i] = -out[i];

			for(i=24; i<30; i++)
				out[i] *= win[1][i];

			for(i=30; i<36; i++)
				out[i] = 0.0;

			for(i=0; i<18; i++) {
				out2[i][0] = out[i] + prevblock[i];
				prevblock[i] = out[i+18];
			}
			break;
		case 3:
			for(i=0; i<9; i++) {
				out[i] = -in[i+9];
				out[17-i] = in[i+9];
				out[26-i] = in[i];
				out[27+i] = in[i];
			}

			for(i=0; i<6; i++)
				out[i] = 0.0;

			for(i=6; i<12; i++)
				out[i] *= win[3][i];

			for(i=12; i<18; i++)
				out[i] = -out[i];

			for(i=18; i<36; i++)
				out[i] *= win[3][i];

			for(i=0; i<18; i++) {
				out2[i][0] = out[i] + prevblock[i];
				prevblock[i] = out[i+18];
			}
			break;
		}
	}
}

