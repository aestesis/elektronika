/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#include "fixed.h"

#ifdef FIXEDUSEFLOAT
#include <math.h>
#endif

SFIXED FixedNearestMultiple(SFIXED x, SFIXED factor)
{	
	SFIXED n;
	n = (x < 0 ? x - factor/2 : x + factor/2) / factor;
	return n * factor;
}

SFIXED FixedTan(SFIXED d)
{
 	return FixedDiv(FixedSin(d), FixedCos(d));
}

#ifdef FIXEDUSEFLOAT
   
SFIXED FixedMul(SFIXED a, SFIXED b)
{
	return (S32)(((double)a * (double)b)*(1/65536.0));
}

SFIXED FixedDiv(SFIXED a, SFIXED b)
{
	if ( b == 0 ) return 0;
	return (S32)(((double)a / (double)b)*65536.0);
}
	
SFIXED FixedSin(SFIXED d)
{
	return (S32)(sin((double)d*(3.141592654/180/65536.0))*65536.0);
}

SFIXED FixedCos(SFIXED d)
{
	return (S32)(cos((double)d*(3.141592654/180/65536.0))*65536.0);
}

SFIXED FixedAtan2(SFIXED dy, SFIXED dx)
{
	if ( dy == 0 && dx == 0 ) return 0;
	return (S32)(atan2((double)dy, (double)dx)*(180/3.141592654)*65536.0);
}

S32 _FPMul( S32 a, S32 b, int rshift )
{
	return (S32)(((double)a*(double)b)/(1L<<rshift));
}

S32 _FPDiv( S32 a, S32 b, int rshift )
{
	if ( b == 0 ) return a < 0 ? minS32 : maxS32;
	return (S32)(((double)a)/((double)b)*(1L<<rshift));
}

#else   

//#define Test						// Test mode
//#define FileData					// Output tables to file

// Trigonometric constants based in degrees
#define kPiPrec		20				// Pi constants' max precision
#define kPiIntBits	7				// number of integer bits in pi

#define kFPTwoPi	0x016800000L	// 2 * pi     in 12.20 format
#define kFPThreePi2	0x010e00000L	// 3 * pi / 2 in 12.20 format
#define kFPPi		0x00b400000L	//     pi     in 12.20 format
#define kFPPi2		0x005a00000L	//     pi / 2 in 12.20 format

#define SinPrec		31				// Sine & Cosine's max precision
#define SinBits		9				//                 lookup table index size
#define SinMaxIdx	(1 << SinBits)	//                 lookup table max index

#define ATanPrec	25				// ATan's max precision
#define ATanN		11				//        approx. polynomial degree + 1

// data conversion
#define Rad2Deg( rad ) ((double) ((rad) * 180.0L / 3.14159265358979312L))
#define Deg2Rad( deg ) ((double) ((deg) * 3.14159265358979312L / 180.0L))

//char gTrigUseFPU = 0;				// Non-zero if we should use FPU

//
// Sine & Cosine's lookup table
//
static const U32 SinTable[ SinMaxIdx ] =
{
	0x00000000L, 0x008efa17L, 0x011df37cL, 0x01aceb7cL, 
	0x023be165L, 0x02cad484L, 0x0359c428L, 0x03e8af9dL, 
	0x04779632L, 0x05067734L, 0x059551f1L, 0x062425b6L, 
	0x06b2f1d2L, 0x0741b592L, 0x07d07044L, 0x085f2136L, 
	0x08edc7b6L, 0x097c6312L, 0x0a0af299L, 0x0a997597L, 
	0x0b27eb5cL, 0x0bb65335L, 0x0c44ac72L, 0x0cd2f660L, 
	0x0d61304dL, 0x0def5989L, 0x0e7d7162L, 0x0f0b7727L, 
	0x0f996a26L, 0x102749aeL, 0x10b5150fL, 0x1142cb97L, 
	0x11d06c96L, 0x125df75bL, 0x12eb6b35L, 0x1378c774L, 
	0x14060b67L, 0x1493365fL, 0x152047aaL, 0x15ad3e9aL, 
	0x163a1a7eL, 0x16c6daa6L, 0x17537e63L, 0x17e00505L, 
	0x186c6dddL, 0x18f8b83cL, 0x1984e373L, 0x1a10eed2L, 
	0x1a9cd9acL, 0x1b28a351L, 0x1bb44b13L, 0x1c3fd044L, 
	0x1ccb3236L, 0x1d56703bL, 0x1de189a5L, 0x1e6c7dc7L, 
	0x1ef74bf2L, 0x1f81f37bL, 0x200c73b4L, 0x2096cbf0L, 
	0x2120fb83L, 0x21ab01bfL, 0x2234ddfaL, 0x22be8f87L, 
	0x234815baL, 0x23d16fe7L, 0x245a9d64L, 0x24e39d85L, 
	0x256c6f9fL, 0x25f51307L, 0x267d8713L, 0x2705cb19L, 
	0x278dde6eL, 0x2815c069L, 0x289d7061L, 0x2924edabL, 
	0x29ac37a0L, 0x2a334d95L, 0x2aba2ee3L, 0x2b40dae2L, 
	0x2bc750e9L, 0x2c4d9050L, 0x2cd39870L, 0x2d5968a2L, 
	0x2ddf003fL, 0x2e645ea0L, 0x2ee9831fL, 0x2f6e6d15L, 
	0x2ff31bddL, 0x30778ed2L, 0x30fbc54dL, 0x317fbeaaL, 
	0x32037a45L, 0x3286f778L, 0x330a35a1L, 0x338d341aL, 
	0x340ff242L, 0x34926f73L, 0x3514ab0dL, 0x3596a46cL, 
	0x36185aeeL, 0x3699cdf1L, 0x371afcd4L, 0x379be6f6L, 
	0x381c8bb5L, 0x389cea71L, 0x391d028bL, 0x399cd362L, 
	0x3a1c5c56L, 0x3a9b9cc9L, 0x3b1a941cL, 0x3b9941b0L, 
	0x3c17a4e8L, 0x3c95bd25L, 0x3d1389cbL, 0x3d910a3cL, 
	0x3e0e3ddbL, 0x3e8b240eL, 0x3f07bc37L, 0x3f8405bbL, 
	0x3fffffffL, 0x407baa69L, 0x40f7045fL, 0x41720d45L, 
	0x41ecc483L, 0x42672980L, 0x42e13ba3L, 0x435afa54L, 
	0x43d464faL, 0x444d7affL, 0x44c63bcbL, 0x453ea6c7L, 
	0x45b6bb5dL, 0x462e78f8L, 0x46a5df02L, 0x471cece6L, 
	0x4793a210L, 0x4809fdebL, 0x487fffe3L, 0x48f5a767L, 
	0x496af3e1L, 0x49dfe4c2L, 0x4a547975L, 0x4ac8b16bL, 
	0x4b3c8c11L, 0x4bb008d8L, 0x4c232730L, 0x4c95e687L, 
	0x4d084651L, 0x4d7a45fdL, 0x4debe4feL, 0x4e5d22c5L, 
	0x4ecdfec6L, 0x4f3e7874L, 0x4fae8f42L, 0x501e42a5L, 
	0x508d9211L, 0x50fc7cfbL, 0x516b02d8L, 0x51d92320L, 
	0x5246dd48L, 0x52b430c8L, 0x53211d18L, 0x538da1aeL, 
	0x53f9be04L, 0x54657194L, 0x54d0bbd6L, 0x553b9c45L, 
	0x55a6125bL, 0x56101d94L, 0x5679bd6bL, 0x56e2f15dL, 
	0x574bb8e6L, 0x57b41383L, 0x581c00b3L, 0x58837ff3L, 
	0x58ea90c3L, 0x595132a2L, 0x59b7650fL, 0x5a1d278cL, 
	0x5a827999L, 0x5ae75ab9L, 0x5b4bca6cL, 0x5bafc836L, 
	0x5c13539aL, 0x5c766c1cL, 0x5cd91140L, 0x5d3b428bL, 
	0x5d9cff83L, 0x5dfe47adL, 0x5e5f1a90L, 0x5ebf77b4L, 
	0x5f1f5ea0L, 0x5f7eceddL, 0x5fddc7f3L, 0x603c496cL, 
	0x609a52d2L, 0x60f7e3b0L, 0x6154fb90L, 0x61b19a00L, 
	0x620dbe8bL, 0x626968beL, 0x62c49827L, 0x631f4c54L, 
	0x637984d4L, 0x63d34136L, 0x642c810bL, 0x648543e3L, 
	0x64dd894fL, 0x653550e2L, 0x658c9a2dL, 0x65e364c4L, 
	0x6639b03aL, 0x668f7c24L, 0x66e4c817L, 0x673993a8L, 
	0x678dde6eL, 0x67e1a7ffL, 0x6834eff3L, 0x6887b5e1L, 
	0x68d9f964L, 0x692bba13L, 0x697cf789L, 0x69cdb161L, 
	0x6a1de736L, 0x6a6d98a4L, 0x6abcc547L, 0x6b0b6cbcL, 
	0x6b598ea2L, 0x6ba72a97L, 0x6bf4403aL, 0x6c40cf2bL, 
	0x6c8cd70bL, 0x6cd8577aL, 0x6d23501aL, 0x6d6dc08eL, 
	0x6db7a879L, 0x6e01077fL, 0x6e49dd44L, 0x6e92296dL, 
	0x6ed9eba1L, 0x6f212385L, 0x6f67d0c0L, 0x6fadf2fcL, 
	0x6ff389dfL, 0x70389513L, 0x707d1443L, 0x70c10717L, 
	0x71046d3dL, 0x71474660L, 0x7189922bL, 0x71cb504eL, 
	0x720c8074L, 0x724d224eL, 0x728d358bL, 0x72ccb9daL, 
	0x730baeedL, 0x734a1474L, 0x7387ea23L, 0x73c52fabL, 
	0x7401e4c0L, 0x743e0917L, 0x74799c65L, 0x74b49e5fL, 
	0x74ef0ebbL, 0x7528ed32L, 0x75623979L, 0x759af34bL, 
	0x75d31a60L, 0x760aae72L, 0x7641af3cL, 0x76781c79L, 
	0x76adf5e6L, 0x76e33b3eL, 0x7717ec41L, 0x774c08abL, 
	0x777f903bL, 0x77b282b3L, 0x77e4dfd1L, 0x7816a758L, 
	0x7847d909L, 0x787874a6L, 0x78a879f4L, 0x78d7e8b5L, 
	0x7906c0afL, 0x793501a8L, 0x7962ab66L, 0x798fbdb0L, 
	0x79bc384dL, 0x79e81b06L, 0x7a1365a4L, 0x7a3e17f2L, 
	0x7a6831b9L, 0x7a91b2c7L, 0x7aba9ae6L, 0x7ae2e9e3L, 
	0x7b0a9f8dL, 0x7b31bbb2L, 0x7b583e20L, 0x7b7e26a9L, 
	0x7ba3751dL, 0x7bc8294cL, 0x7bec430aL, 0x7c0fc229L, 
	0x7c32a67dL, 0x7c54efdbL, 0x7c769e17L, 0x7c97b108L, 
	0x7cb82885L, 0x7cd80464L, 0x7cf7447eL, 0x7d15e8adL, 
	0x7d33f0c9L, 0x7d515caeL, 0x7d6e2c37L, 0x7d8a5f3fL, 
	0x7da5f5a4L, 0x7dc0ef44L, 0x7ddb4bfbL, 0x7df50babL, 
	0x7e0e2e32L, 0x7e26b371L, 0x7e3e9b49L, 0x7e55e59dL, 
	0x7e6c9250L, 0x7e82a146L, 0x7e981262L, 0x7eace58aL, 
	0x7ec11aa4L, 0x7ed4b197L, 0x7ee7aa4bL, 0x7efa04a8L, 
	0x7f0bc096L, 0x7f1cde00L, 0x7f2d5cd1L, 0x7f3d3cf3L, 
	0x7f4c7e53L, 0x7f5b20dfL, 0x7f692483L, 0x7f76892eL, 
	0x7f834ed0L, 0x7f8f7558L, 0x7f9afcb8L, 0x7fa5e4e1L, 
	0x7fb02dc5L, 0x7fb9d758L, 0x7fc2e18eL, 0x7fcb4c5aL, 
	0x7fd317b4L, 0x7fda4390L, 0x7fe0cfe7L, 0x7fe6bcafL, 
	0x7fec09e2L, 0x7ff0b779L, 0x7ff4c56eL, 0x7ff833bdL, 
	0x7ffb025fL, 0x7ffd3153L, 0x7ffec096L, 0x7fffb025L, 
	0x80000000L, 0x7fffb025L, 0x7ffec096L, 0x7ffd3153L, 
	0x7ffb025fL, 0x7ff833bdL, 0x7ff4c56eL, 0x7ff0b779L, 
	0x7fec09e2L, 0x7fe6bcafL, 0x7fe0cfe7L, 0x7fda4390L, 
	0x7fd317b4L, 0x7fcb4c5aL, 0x7fc2e18eL, 0x7fb9d758L, 
	0x7fb02dc5L, 0x7fa5e4e1L, 0x7f9afcb8L, 0x7f8f7558L, 
	0x7f834ed0L, 0x7f76892eL, 0x7f692483L, 0x7f5b20dfL, 
	0x7f4c7e53L, 0x7f3d3cf3L, 0x7f2d5cd1L, 0x7f1cde00L, 
	0x7f0bc096L, 0x7efa04a8L, 0x7ee7aa4bL, 0x7ed4b197L, 
	0x7ec11aa4L, 0x7eace58aL, 0x7e981262L, 0x7e82a146L, 
	0x7e6c9250L, 0x7e55e59dL, 0x7e3e9b49L, 0x7e26b371L, 
	0x7e0e2e32L, 0x7df50babL, 0x7ddb4bfbL, 0x7dc0ef44L, 
	0x7da5f5a4L, 0x7d8a5f3fL, 0x7d6e2c37L, 0x7d515caeL, 
	0x7d33f0c9L, 0x7d15e8adL, 0x7cf7447eL, 0x7cd80464L, 
	0x7cb82885L, 0x7c97b108L, 0x7c769e17L, 0x7c54efdbL, 
	0x7c32a67dL, 0x7c0fc229L, 0x7bec430aL, 0x7bc8294cL, 
	0x7ba3751dL, 0x7b7e26a9L, 0x7b583e20L, 0x7b31bbb2L, 
	0x7b0a9f8dL, 0x7ae2e9e3L, 0x7aba9ae6L, 0x7a91b2c7L, 
	0x7a6831b9L, 0x7a3e17f2L, 0x7a1365a4L, 0x79e81b06L, 
	0x79bc384dL, 0x798fbdb0L, 0x7962ab66L, 0x793501a8L, 
	0x7906c0afL, 0x78d7e8b5L, 0x78a879f4L, 0x787874a6L, 
	0x7847d909L, 0x7816a758L, 0x77e4dfd1L, 0x77b282b3L, 
	0x777f903bL, 0x774c08abL, 0x7717ec41L, 0x76e33b3eL, 
	0x76adf5e6L, 0x76781c79L, 0x7641af3cL, 0x760aae72L, 
	0x75d31a60L, 0x759af34bL, 0x75623979L, 0x7528ed32L, 
	0x74ef0ebbL, 0x74b49e5fL, 0x74799c65L, 0x743e0917L, 
	0x7401e4c0L, 0x73c52fabL, 0x7387ea23L, 0x734a1474L, 
	0x730baeedL, 0x72ccb9daL, 0x728d358bL, 0x724d224eL, 
	0x720c8074L, 0x71cb504eL, 0x7189922bL, 0x71474660L, 
	0x71046d3dL, 0x70c10717L, 0x707d1443L, 0x70389513L, 
	0x6ff389dfL, 0x6fadf2fcL, 0x6f67d0c0L, 0x6f212385L, 
	0x6ed9eba1L, 0x6e92296dL, 0x6e49dd44L, 0x6e01077fL, 
	0x6db7a879L, 0x6d6dc08eL, 0x6d23501aL, 0x6cd8577aL, 
	0x6c8cd70bL, 0x6c40cf2bL, 0x6bf4403aL, 0x6ba72a97L, 
	0x6b598ea2L, 0x6b0b6cbcL, 0x6abcc547L, 0x6a6d98a4L, 
	0x6a1de736L, 0x69cdb161L, 0x697cf789L, 0x692bba13L, 
	0x68d9f964L, 0x6887b5e1L, 0x6834eff3L, 0x67e1a7ffL, 
	0x678dde6eL, 0x673993a8L, 0x66e4c817L, 0x668f7c24L, 
	0x6639b03aL, 0x65e364c4L, 0x658c9a2dL, 0x653550e2L, 
} ;

// ATan's table of Xis
static const S32 ATanXi[ ATanN ] =
{
	0x00000000L,
	0x00000d1bL,
	0x00004189L,
	0x00008312L,
	0x00051eb8L,
	0x00333333L,
	0x00999999L,
	0x01000000L,
	0x01666666L,
	0x01ccccccL,
	0x02000000L,
} ;

// ATan's table of polynomial coefficients
static const S32 ATanCoef[ ATanN ] =
{
	0x00000000L,
	0x72977063L,
	0xfffa2214L,
	0xd9cd8811L,
	0x00440d74L,
	0x16bc31bbL,
	0xf9c12219L,
	0xf59cab63L,
	0x0a44f19dL,
	0xfceb901aL,
	0xfe34e3a5L,
} ;


#ifdef Test

#include <stdlib.h>
#include <stdio.h>

//
// Build precomputed sine table
//

void BuildSinTable( void )
{
	register int i, j;
	double theta, sintheta;

	double libPi2	= acos( 0 );
	double fpPi2	= (double) kFPPi2 / (double) (1L << kPiPrec);
	double scale	= libPi2 / fpPi2;
	
	FILE *fp = 0;

	// build sine table
	for ( i = 0; i < SinMaxIdx; i++ )
	{
		// theta is in [0..128] which is a superset of [0..pi/2]
		theta			 = (double) i / (SinMaxIdx >> kPiIntBits);
		sintheta		 = sin( theta * scale );
		sintheta		*= (double) (1L << (SinPrec >> 1));
		sintheta		*= (double) (1L << (SinPrec - (SinPrec >> 1)));
		SinTable[ i ]	 = (U32) sintheta;
	}

	#ifdef FileData     
		fp = fopen( "sintbl.dat", "w"  );
	#else
		fp = stdout;
	#endif
	
	if ( fp )
	{
		// print sine table
		for ( i = 0; i < SinMaxIdx; i += 4 )
		{
			fprintf( fp, "\t" );

			for ( j = 0; j < 4; j++ )
				fprintf( fp, "0x%08lxL, ", SinTable[ i + j ] );

			fprintf( fp, "\n" );
		}

		fprintf( fp, "\n" );
		if ( fp != stdout ) fclose( fp );
	}
}

//
// Build table of coefficients for ATan approximating polynomial
//

void BuildATanTable( void )
{
	double ATanXif[ ATanN ] =			// ATan table of node points used
	{									// to approximate atan
		0.0000L,
		0.0001L,
		0.0005L,
		0.0010L,
		0.0100L,
		0.1000L,
		0.3000L,
		0.5000L,
		0.7000L,
		0.9000L,
		1.0000L
	} ;

	register int i, j;
	double (*ATanCoefF)[ ATanN ] = (double (*)[ATanN]) malloc( sizeof( double ) * ATanN * ATanN );

	double libPi2	= acos( 0 );
	double fpPi2	= (double) kFPPi2 / (double) (1L << kPiPrec);
	double scale	= fpPi2 / libPi2;
	
	FILE *fp = 0;

	//
	// Use Newton Divided Differences
	//

	// Compute x, and ATan( x )
	for ( i = 0; i < ATanN; i++ )
	{
		ATanXi[ i ] = (S32) (ATanXif[ i ] * (1L << ATanPrec));
		ATanCoefF[ 0 ][ i ] = scale * atan( (double) ATanXi[ i ] / (1L << ATanPrec) );
	}         

	// Compute divided differences
	for ( i = 1; i < ATanN; i++ )
		for ( j = 0; j < (ATanN - i); j++ )
			ATanCoefF[ i ][ j ] = (ATanCoefF[ i - 1 ][ j + 1 ] - ATanCoefF[ i - 1 ][ j ]) / ((double) (ATanXi[ i + j ] - ATanXi[ j ]) / (1L << ATanPrec));

	// Save the coefficient divided differenes as fixed point numbers
	for ( i = 0; i < ATanN; i++ )
		ATanCoef[ i ] = (S32) (ATanCoefF[ i ][ 0 ] * (1L << ATanPrec));

	free( ATanCoefF );

	#ifdef FileData
		fp = fopen( "atantbl.dat", "w" );
	#else
		fp = stdout;
	#endif
	
	if ( fp )
	{
		// print xis
		for ( i = 0; i < ATanN; i++ )
			fprintf( fp, "\t0x%08lxL,\n", ATanXi[ i ] );

		fprintf( fp, "\n" );

		// print coefficients
		for ( i = 0; i < ATanN; i++ )
			fprintf( fp, "\t0x%08lxL,\n", ATanCoef[ i ] );

		fprintf( fp, "\n" );
		if ( fp != stdout ) fclose( fp );
	}
}

//
// Initialize the fixed point trig routines
//

void _FPInit( int fpuFlag )
{
	gTrigUseFPU = fpuFlag;
	
	#ifdef Test
		BuildSinTable();
		BuildATanTable();
	#endif
}

#endif


//
// Calculate the sine of a fixed point number in [0..PI/2)
//
// Note:  outBPoint < SinPrec
//

SFIXED __FPSin( SFIXED x, int inBPoint, int outBPoint )
{
	register int i, j;
	U32 dx, v1, v2;

//	// force sine of Pi/2 to be exact
//	if ( (S32) x == ((S32) kFPPi2 >> (kPiPrec - inBPoint)) ) return 1L << outBPoint;

	// compute index to sine table
	j = inBPoint + kPiIntBits - SinBits;
	i = (j >= 0) ?	(int)((U32) x >>  j) :
					(int)((U32) x << -j);

	i &= SinMaxIdx - 1;

	// compute stored point's difference from x
	dx = (U32) x - ((j >= 0) ? ((U32) i << j) : ((U32) i >> -j));

	// extract two points from the table
	v1  = SinTable[ i ];
	v2  = SinTable[ i + 1 ];

	// compute Legendre 1st order approximation
	if ( v2 >= v1 )
		v1 += _FPMul( v2 - v1, dx, SinPrec - inBPoint + 1 );
	else
		v1 -= _FPMul( v1 - v2, dx, SinPrec - inBPoint + 1 );

	// return sine in appropriate precision
	FLASHASSERT(SinPrec>outBPoint);
	int shift = SinPrec - outBPoint;
	return (v1+(1<<(shift-1))) >> shift;
// This has a problem since v1 needs to be treated as unsigned...
//	return( _FPRound( v1, SinPrec - outBPoint ) );
}

//
// Calculate the sine of a fixed point number
//

S32 _FPSin( S32 x, int inBPoint, int outBPoint )
{
	int i, sign;
	U32 twoPi, Pi, Pi2;
	S32 sinx;

	i		= kPiPrec - inBPoint;

	// compute trig constants in appropriate accuracy
	twoPi	= (S32) kFPTwoPi	>> i;
	Pi		= (S32) kFPPi		>> i;
	Pi2		= (S32) kFPPi2		>> i;
	
	// assume positive result
	sign	= 0;

	// map x to [0..2*PI)
	if ( x >= (S32) twoPi )	x -= ( x / twoPi)     * twoPi;
	if ( x < 0 )			x += (-x / twoPi + 1) * twoPi;

	// map x to [0..PI)
	if ( (U32) x >= Pi )
	{
		sign = 1;
		x -= Pi;
	}

	// map x to [0..PI/2)
	if ( (U32) x > Pi2 ) x = Pi - x;

	// compute sine of x
	sinx = __FPSin( x, inBPoint, outBPoint );

	// return sine of x with appropriate sign
	return( sign ? -sinx : sinx );
}

//
// Calculate the cosine of a fixed point number
//

S32 _FPCos( S32 x, int inBPoint, int outBPoint )
{
	int i, sign;
	U32 twoPi, threePi2, Pi, Pi2;
	S32 cosx;

	i			= kPiPrec - inBPoint;

	// compute trig constants in appropriate accuracy
	twoPi		= (S32) kFPTwoPi	>> i;
	threePi2	= (S32) kFPThreePi2	>> i;
	Pi			= (S32) kFPPi		>> i;
	Pi2			= (S32) kFPPi2		>> i;

	// assume positive result
	sign		= 0;

	// map x to [0..2*PI)
	if ( x >= (S32) twoPi )	x -= ( x / twoPi)     * twoPi;
	if ( x < 0 )			x += (-x / twoPi + 1) * twoPi;

	// map x to [0..PI/2)
	if ( (U32) x >= threePi2 )
		x -= threePi2;
	else
		if ( (U32) x < Pi2 )
			x = Pi2 - x;
		else
		{
			sign = 1;
			x -= Pi2;

			if ( (U32) x > Pi2 )
				x = Pi - x;
		}

	// compute cosine of x
	cosx = __FPSin( x, inBPoint, outBPoint );

	// return cosine of x with appropriate sign
	return( sign ? -cosx : cosx );
}

//
// Calculate the atan of a fixed point number in [0..1]
//
// Note:	 inBPoint < ATanPrec
// 			outBPoint < ATanPrec
//

S32 __FPATan( S32 x, int inBPoint, int outBPoint )
{
	int i;
	S32 p;

	// normalize x to internal precision
	x = x << (ATanPrec - inBPoint);

	// compute ATanN - 1 order polynomial
	p = ATanCoef[ ATanN - 1 ];
	for ( i = ATanN - 2; i >= 0; i-- )
		p = ATanCoef[ i ] + _FPMul( p, x - ATanXi[ i ], ATanPrec );

	// return atan of x in appropriate precision
	return( _FPRound( p, ATanPrec - outBPoint ) );
}

//
// Calculate the atan of a fixed point number
//

S32 _FPATan( S32 x, int inBPoint, int outBPoint )
{
	int sign = 0;
	S32 one = 1L << inBPoint;

	// if we can, use FPU
	//if ( gTrigUseFPU )
	//	return( (S32) (Rad2Deg( atan( (double) x / (1L << inBPoint) ) ) * (1L << outBPoint)) );

	// map x to [0..infinity)
	if ( x < 0 )
	{
		x = (x == 0x80000000L) ? 0x7fffffffL : -x;
		sign = 1;
	}

	// map x to [0..1]
	if ( x > one )
	{
		x = _FPRound( kFPPi2, kPiPrec - outBPoint ) -
			__FPATan( _FPDiv( one, x, inBPoint ), inBPoint, outBPoint );
	}
	else
		x = __FPATan( x, inBPoint, outBPoint );

	// return atan of x with appropriate sign
	return( sign ? -x : x );
}

//
// Calculate the atan of fixed point y / fixed point x
//

S32 _FPATan2( S32 y, S32 x, int inBPoint, int outBPoint )
{
	int i;
	U32 Pi;
	S32 atanx;

	// if we can, use FPU
	//if ( gTrigUseFPU )
	//	return( (S32) (Rad2Deg( atan2( (double) y / (1L << inBPoint), (double) x / (1L << inBPoint) ) ) * (1L << outBPoint)) );

	i = kPiPrec - outBPoint;

	// avoid division by zero
	if ( x == 0 )
	{
		atanx = _FPRound( kFPPi2, i );
		if ( y < 0 ) atanx = -atanx;
	}
	else
	{
		// compute atan y / x in appropriate accuracy
		atanx = _FPATan( _FPDiv( y, x, inBPoint ), inBPoint, outBPoint );

		// map atanx to appropriate quadrant
		if ( x < 0 )
		{
			// compute trig constants in appropriate accuracy
			Pi = _FPRound( kFPPi, i );

			if ( y < 0 )
				atanx -= Pi;
			else
				atanx += Pi;
		}
	}

	return( atanx );
}

#endif