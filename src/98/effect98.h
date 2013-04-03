//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef		_EFFECT98_H_
#define		_EFFECT98_H_
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#include	<windows.h>
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4731)

#define						RET							"\r\n"

#define						AEC_MAXTEXT					128

#define						AEE_NOERROR					0
#define						AEE_MEMORYERROR				1

enum
{
							AET_INFO=1,
							AET_SONIQUE
};

#define						AET_PARAM					1

#define						AET_ECRAN_OLD				1	// 320x200 + palette[256rvba] composantes[0..63]
#define						AET_ECRAN_OLDP				2	// 320x200 + palette[256rvba] composantes[0..255]
#define						AET_ECRAN_NEW				3	// 320x200 16millions,RVBA

#ifndef PI
#define						PI							3.1415
#endif
#define						PIF							3.1415f

#define						countof(x)					(sizeof(x)/sizeof(x[0]))

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

/*
typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;
*/

typedef struct STootips											// helpers
{
	char					*choix11;							// choix
	char					*choix12;
	char					*choix13;
	char					*choix14;
	char					*choix15;
	char					*choix16;
	char					*choix17;
	char					*choix18;
	char					*choix19;
	char					*choix10;

	char					*choix21;							// choix2
	char					*choix22;
	char					*choix23;
	char					*choix24;
	char					*choix25;
	char					*choix26;
	char					*choix27;
	char					*choix28;
	char					*choix29;
	char					*choix20;

	char					*choix31;							// choix3
	char					*choix32;
	char					*choix33;
	char					*choix34;
	char					*choix35;
	char					*choix36;
	char					*choix37;
	char					*choix38;
	char					*choix39;
	char					*choix30;

	char					*choix41;							// choix4
	char					*choix42;
	char					*choix43;
	char					*choix44;
	char					*choix45;
	char					*choix46;
	char					*choix47;
	char					*choix48;
	char					*choix49;
	char					*choix40;

	char					*vb1;								// virtual balls
	char					*vb2;
	char					*vb3;
	char					*vb4;

	char					*t0;								// touches
	char					*t1;
	char					*t2;
	char					*t3;
	char					*t4;
	char					*t5;
	char					*t6;
	char					*t7;
	char					*t8;
	char					*t9;
} TTooltips;

typedef struct SParam
{
	int						type;								// always AET_PARAM (reserved for futur)
	byte					spacebar;
	byte					choix;								// 0..9
	byte					choix2;								// 0..9
	byte					choix3;								// 0..9
	byte					choix4;								// 0..9
	byte					touche[10];							// 0..9 TRUE/FALSE
	int						moux[4];
	int						mouy[4];
	int						amoux[4];
	int						amouy[4];
	byte					sample					[256];		// 256
	word					sample16				[256];		// 256
	byte					spectre					[128];		// 128
	word					spectre16				[128];		// 128
	byte					palette					[256*4];
	byte					ecran					[256000];
} TParam;

typedef char *				FLoad					(char *filename, void *p, TParam *);
typedef int					FAction					(void *p, TParam *);
typedef void *				FConstructor			();
typedef void				FDestructor				(void *p);

typedef void				FQuit					();

typedef struct SInfo
{
	int						nType;								// always AET_INFO (reserved for futur)

	int						nTypeEcran;							// AET_ECRAN_OLD, AET_ECRAN_OLDP or AET_ECRAN_NEW

	char					*rBitmap;							// resource bitmap name

    char					*Name;
    char					*Description;
    char					*Help;
    char					*Developer;
    char					*Enterprise;
    char					*Copyright;
    char					*Mail;
    char					*Web;

	FLoad					*Load;
	FAction					*Action;							// action			callback
	FConstructor			*Constructor;						// new  instance	callback
	FDestructor				*Destructor;						// free instance	callback

	FQuit					*Quit;

	int						nbchoix;
	int						nbchoix2;
	int						nbchoix3;
	int						nbchoix4;

	bool					masktouche[10];
	bool					maskvball[4];
} TInfo;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#endif
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
