// coded by Renan
// project : VIDEOASTESIS
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include			<windows.h>
#include			<math.h>
#include			"resource.h"

//////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

#include			"flammes.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static byte			sinus[256+64];
static byte			sinus_deph[256+64];
static byte			*cosin;
static byte			*cosin_deph;
static byte			table[65536*2];

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
	
typedef struct SVarFlammes
{
	byte			deform[1024];						// 	deforma    = deform+512;
	byte			image[262*256];
	DWORD			cyc,cyc1,cyc2,cyc3;
	DWORD			effet[256];
	DWORD			memosp[128];
	DWORD			decal1,decal2,decal3,decal4;
	DWORD			teta_flammes;
	float			dep;								//d‚calage couleur choix2=4 
} TVarFlammes;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void flam_up(TVarFlammes *pVarFlammes)
{
	DWORD	image=(DWORD)pVarFlammes->image;
	__asm
	{
        mov     esi,image
        add     esi,(262*256)-1

        mov     ecx,262-3
        xor     edx,edx
        xor     eax,eax

	myboucy:
        mov     al,[esi-(256*2-1)]
        mov     edx,eax

        mov     al,[esi-(256*3+1)]
        add     edx,eax

        mov     al,[esi-(256*2)]
        add     edx,eax

        mov     al,[esi-(256*2+1)]
        add     edx,eax

        shr     edx,2
        mov     [esi],dl
        dec     esi

		mov		ebx,256-2

	myboucx:
        mov     al,[esi-(256*2-1)]
        mov     edx,eax

        mov     al,[esi-(256*2+1)]
        add     edx,eax

        mov     al,[esi-(256*2)]
        add     edx,eax

        mov     al,[esi-(256*1+1)]
        add     edx,eax

        shr     edx,2
        mov     [esi],dl
        dec     esi

        dec     ebx
        jnz     myboucx

        mov     al,[esi-(256-1)]
        mov     edx,eax

        mov     al,[esi-(256*2+1)]
        add     edx,eax

        mov     al,[esi-(256*2)]
        add     edx,eax

        mov     al,[esi-(256*1+1)]
        add     edx,eax

        shr     edx,2
        mov     [esi],dl
        dec     esi

		dec		ecx
		jnz		myboucy
	}
}

#pragma optimize("",off)
#pragma code_seg("automod")
static void affiche(TVarFlammes *pVarFlammes, byte *dest)
{
	DWORD	image=(DWORD)pVarFlammes->image;
	DWORD	deform=(DWORD)pVarFlammes->deform;
	__asm
	{
        lea     ebx,table[0]
        lea     eax,table[0]
		inc		ebx
		lea		esi,cg_table0
		lea		edi,cg_table1
		mov		[esi-4],eax
		mov		[edi-4],ebx

		mov		ecx,deform
        mov     ebx,deform
		add		ecx,512
		lea		edi,cg_deform1
		lea		edx,cg_deform2
		mov		[edi-4],ebx
		mov		[edx-4],ecx

        mov     eax,image
		add		eax,(262-256)*256
		lea		esi,cg_image
		mov		[esi-4],eax

        mov     edi,dest
        mov     esi,64000*2
        xor     eax,eax

	mybouc4:
        sub     esi,2
        js		lafin

		movzx	eax,byte ptr[esi+012345678h]
	cg_table0:
		movzx	ebx,byte ptr[esi+012345678h]
	cg_table1:

		movzx	ecx,word ptr[eax*2+012345678h]
	cg_deform1:
		movzx	edx,word ptr[ebx*2+012345678h]
	cg_deform2:

		mov		ah,bl
		add		al,cl
		add		ah,ch
		add		al,dl
		add		ah,dh

		mov		bl,[eax+012345678h]
	cg_image:

		mov		[edi],bl
		inc		edi
		jmp		mybouc4
	lafin:
	}
}
#pragma code_seg()
#pragma optimize("",on)

static void blur(byte *dest, byte *src)
{
	__asm
	{
        mov     edi,dest
        mov     esi,src
        push    ebp
        add     edi,320+1-2-1
        add     esi,320+1-2
        xor     eax,eax
        xor     ebx,ebx

        mov     ebp,198
        push    ebp
    blur_by:
        pop     ebp

        add     esi,2
        add     edi,2

        dec     ebp
        js      blur_fy

        push    ebp
        mov     ebp,318
    blur_bx:
        dec     ebp
        js      blur_by

        mov     al,[esi-321]
        mov     bl,[esi-320]
        mov     ecx,eax
        mov     edx,ebx
        mov     al,[esi-319]
        mov     bl,[esi-1]
        add     ecx,eax
        add     edx,ebx
        mov     al,[esi+1]
        mov     bl,[esi+319]
        add     ecx,eax
        add     edx,ebx
        mov     al,[esi+320]
        mov     bl,[esi+321]
        add     ecx,eax
        add     edx,ebx
        add     edx,ecx
        inc     edi
        shr     edx,3
        inc     esi
        mov     [edi],dl
        jmp     blur_bx

    blur_fy:

        pop     ebp
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void calc_effet(TVarFlammes *pVarFlammes, TParam *pParam, int n, int n2)
{
	int		i;
	DWORD	amort;
	byte	*sample=(byte *)(void  *)pParam->sample;
	byte	*spectre=(byte *)(void *)pParam->spectre;
	DWORD	*effet=pVarFlammes->effet;
	DWORD	*memosp=pVarFlammes->memosp;
	amort=0;
	if (n2==1)
	{
		switch(n)
		{
			case 1:
			for (i=0;i<256;i++)
			{
				amort=(amort*5+sample[i]+spectre[i/2])/7;
				effet[i]=(amort+effet[i]*6)/7;
			}
			break;

			case 2:
			for (i=0;i<256;i++)
				effet[i]=(effet[i]*9+sample[i])/10;
			break;

			case 3:
			for (i=0;i<256;i++)
			{
				amort= (sample[i/8]+amort*5)/6;
				effet[i]=(amort+effet[i]*5)/6;
			}
			break;

			case 4:
			for (i=0;i<256;i++)
			{
				amort= (spectre[i/8]+amort*5)/6;
				effet[i]=(amort+effet[i]*5)/6;
			}
			break;
  
			case 5:
			for (i=0;i<256;i++)
			{
				amort= (sample[i/8]+amort*5)/6;
				effet[i]=(amort+effet[i])/2;
			}
			break;
  
			case 6:
			for (i=0;i<256;i++)
			{
				amort= (spectre[i/8]+amort*5)/6;
				effet[i]=(amort+effet[i])/2;
			}
			break;

			case 7:
			for(i=0;i<256;i++)
			{
				amort=(amort+spectre[i/2])/2;
				effet[i]=(effet[i]+amort)/2;
			}
			break;

			case 10:
			for(i=0;i<256;i++)
			{
				amort=(amort+spectre[i/2]-memosp[i/2])/2;
				effet[i]=(amort+effet[i])/2;
				if((i/2&1)==1) memosp[i/2]=spectre[i/2];
			}
			break;
		}
	}
	else
	{
		switch(n)
		{
			case 1:
			for (i=0;i<256;i++)
			{
				amort=(amort*5+sample[i]+spectre[i/2])/7;
				effet[i]=amort;
			}
			break;

			case 2:
			for (i=0;i<256;i++)
				effet[i]=sample[i];
			break;

			case 3:
			for (i=0;i<256;i++)
			{
				amort=(sample[i/8]+amort*5)/6;
				effet[i]=amort;
			}
			break;

			case 4:
			for (i=0;i<256;i++)
			{
				amort=(spectre[i/8]+amort*5)/6;
				effet[i]=amort;
			}
			break;
  
			case 5:
			for(i=0;i<256;i++)
			{
				amort=(sample[i/8]+amort*5)/6;
				effet[i]=amort;
			}
			break;
  
			case 6:
			for(i=0;i<256;i++)
			{
				amort= (spectre[i/8]+amort*5)/6;
				effet[i]=amort;
			}
			break;
  
			case 7:
			for(i=0;i<256;i++)
			{
				amort=(amort+spectre[i/2])/2;
				effet[i]=amort;
			}
			break;

			case 10:
			for(i=0;i<256;i++)
			{
				int	i2=i>>1;
				amort=(amort+spectre[i2]-memosp[i2])/2;
				effet[i]=amort;
				if(i2&1) memosp[i2]=spectre[i2];
			}
			break;
		}
	}
}
  
//////////////////////////////////////////////////////////////////////////////////////////

static void calcul_deform(TVarFlammes *pVarFlammes, TParam *pParam)
{
	byte	*touche=(byte *)(void *)pParam->touche;
	byte	*deform=pVarFlammes->deform;
	DWORD	*effet=pVarFlammes->effet;
	byte	*mybuf=(byte *)deform;
	byte	*mybufa=((byte *)deform)+512;
	int		i;

	switch(pParam->choix)
	{
		case 1:
		calc_effet(pVarFlammes, pParam, 2, 1);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = (byte)((*(mybufa)+effet[i])>>1); // angle/rayon
			*(mybufa++) = (byte)effet[i]; // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i]; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i+64]; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[127-i]; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[63-i]; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
        break;

		case 2:
        calc_effet(pVarFlammes, pParam, 3, 0);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = 0; // angle/rayon
			*(mybufa++) = (byte)effet[i]; // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}

		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
        break;

		case 3:
		calc_effet(pVarFlammes, pParam, 2, 0);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = (byte)(((256-i)*(effet[i]))>>8); // angle/rayon
			*(mybufa++) = (byte)(((256-i)*(effet[i]))>>8); // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i]; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i+64]; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[127-i]; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[63-i]; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
        break;

		case 4:
		{
			DWORD	t4=touche[4]?1:0;
			DWORD	t7=touche[7]?1:0;
			DWORD	t8=touche[8]?1:0;
			DWORD	t9=touche[9]?1:0;
			calc_effet(pVarFlammes, pParam, pParam->choix4, touche[5]);
			for (i=0 ; i<256 ; i++ )
			{
				*(mybufa++) = (byte)((*(mybufa)+effet[i]*t4)>>1); // angle/rayon
				*(mybufa++) = (byte)(effet[i]*t7); // rayon/rayon
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[i]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[i]*t9); // rayon/angle
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[i+64]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[63-i]*t9); // rayon/angle
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[127-i]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[i]*t9); // rayon/angle
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[63-i]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[63-i]*t9); // rayon/angle
			}
		}
		break;
		
		case 5:
		calc_effet(pVarFlammes, pParam, 1, 1);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = 0; // angle/rayon
			*(mybufa++) = (byte)(-i/2+(((256-i)*effet[i])>>8)); // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i+64]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[127-i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[63-i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		break;

		case 6:
		calc_effet(pVarFlammes, pParam, 1, 0);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = 0; // angle/rayon
			*(mybufa++) = (byte)(-i/2+(((256-i)*effet[i])>>8)); // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i+64]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[127-i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[63-i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		break;

		case 7:
		calc_effet(pVarFlammes, pParam, 1, 0);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = 0; // angle/rayon
			*(mybufa++) = (byte)(-i/2+(((256-i)*effet[i])>>8)); // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[i]; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (byte)effet[63-i]; // rayon/angle
		}
		break;

		case 8:
		{
			DWORD	t4=touche[4]?1:0;
			DWORD	t7=touche[7]?1:0;
			DWORD	t8=touche[8]?1:0;
			DWORD	t9=touche[9]?1:0;
			calc_effet(pVarFlammes, pParam, pParam->choix4, touche[5]);
			for (i=0 ; i<256 ; i++ )
			{
				*(mybufa++) = (byte) ((*(mybufa)+effet[i]*t4)>>1); // angle/rayon
				*(mybufa++) = (byte) ( (-i>>1) + (((256-i)*effet[i])>>8)*t7); // rayon/rayon
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[i]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[i]*t9); // rayon/angle
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[i+64]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[63-i]*t9); // rayon/angle
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[127-i]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[i]*t9); // rayon/angle
			}
			for (i=0; i<64 ; i++)
			{
				*(mybuf++)  = (byte)(effet[63-i]*t8); // angle/angle
				*(mybuf++)  = (byte)(effet[63-i]*t9); // rayon/angle
			}
		}
        break;
/*
		case 9:
		calc_effet(pVarFlammes, pParam, 1, 0);
		for (i=0 ; i<256 ; i++ )
		{
			*(mybufa++) = (byte)((*(mybufa)+effet[i])>>1); // angle/rayon
			*(mybufa++) = 0; // rayon/rayon
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[i+64]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[127-i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
		for (i=0; i<64 ; i++)
		{
			*(mybuf++)  = (byte)effet[63-i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle
		}
        break;
*/
	}
	pVarFlammes->decal1-=3;
	pVarFlammes->decal2+=5;
	pVarFlammes->decal3++;
	pVarFlammes->decal4-=2;
}

//////////////////////////////////////////////////////////////////////////////////////////

static void calc_table( void )
{
	int		x;
	int		y;
	byte	*p=(byte *)table;
	for(y=0; y<200; y++)
		for(x=0; x<320; x++)
		{
			float	dx=(float)x-159.5f;
			float	dy=(float)y-99.5f;
			*(p++)=(byte) (atan2(dy,dx)*128.f/PI);
			*(p++)=(byte) sqrt(dx*dx+dy*dy);
		}
}

//////////////////////////////////////////////////////////////////////////////////////////

static void calc_sinus( void )
{
	int i;
	for (i=0; i<(256+64); i++)
	{
		sinus[i]      = (byte) (sin((2*PI*(double)i)/256.f)*127.f+128.f);
		sinus_deph[i] = (byte) (sin((2*PI*(double)i)/256.f)*60.f+128.f);
	}
	cosin=sinus+64;
	cosin_deph=sinus_deph+64;
}

//////////////////////////////////////////////////////////////////////////////////////////

static void calc_pal(TVarFlammes *pVarFlammes, TParam *pParam)
{
	int		i;
	int		cyc=pVarFlammes->cyc;
	byte	*palette=(byte *)(void *)pParam->palette;
	byte	*spectre=pParam->spectre;
	word	*spectre16=pParam->spectre16;
	switch(pParam->choix2)
	{
		case 1:                     // normal
		{
			int		c1,c2,c3;
			byte	*p=palette;
			c1=((int)(pVarFlammes->dep*5.f)&255);
			c2=((int)(pVarFlammes->dep*7.f)&255);
			c3=((int)(pVarFlammes->dep*9.f)&255);
			for (i=0 ; i<84; i++)
			{
				*(p++)= (char) ((sinus[ (c1+cyc+ 90+i*9)  & 255 ])/4);
				*(p++)= (char) ((sinus[ (c2+cyc+180+i*8)  & 255 ])/4);
				*(p++)= (char) ((sinus[ (c3+cyc+851+i*7)  & 255 ])/4);
				p++;
			}
			memset(&palette[85*4], 0, sizeof(DWORD)*256-86);

			pVarFlammes->dep+=0.000003f;
		}
		break;

		case 2:             // noir & blanc fixe
		{
			byte	*p=palette;
			int		i;
			for(i=0;i<48;i++)
			{
				*(p++)= 0;
				*(p++)= 0;
				*(p++)= 0;
				p++;

			}
			for(i=48;i<90;i++)
			{
				byte	v=((i-48)*63)/42;
				*(p++)= v;
				*(p++)= v;
				*(p++)= v;
				p++;
			}
			for(i=90;i<128;i++)
			{
				*(p++)= 63;
				*(p++)= 63;
				*(p++)= 63;
				p++;
			}
			for(i=128;i<256;i++)
			{
				*(p++)= (255-i)/2;
				*(p++)= (255-i)/2;
				*(p++)= (255-i)/2;
				p++;
			}
		}
		break;

		case 3:                     // par rappport aux sons
		for (i=0;i<85;i++)
		{
			palette[i*4]=palette[(i+1)*4];
			palette[i*4+1]=palette[(i+1)*4+1];
			palette[i*4+2]=palette[(i+1)*4+2];
		}
		palette[85*4]  =(palette[85*4  ]+spectre[10]*4)/2;
		palette[85*4+1]=(palette[85*4+1]+spectre[20]*6)/2;
		palette[85*4+2]=(palette[85*4+2]+spectre[30]*8)/2;
		break;

		case 4:             // normal2 mieux plus plus (  enfin... normalement)
		{
			int c1,c2,c3;
			c1=((int)(pVarFlammes->dep*5)&255);
			c2=((int)(pVarFlammes->dep*7)&255);
			c3=((int)(pVarFlammes->dep*9)&255);
	        for (i=1 ; i<84; i++)
			{
				palette[i*4]   = (char) ((sinus[ (c1+cyc+ 90+i*9)  & 255 ])/4);
				palette[i*4+1] = (char) ((sinus[ (c2+cyc+180+i*8)  & 255 ])/4);
				palette[i*4+2] = (char) ((sinus[ (c3+cyc+851+i*7)  & 255 ])/4);
			}
			memset(&palette[85*4], 0, sizeof(DWORD)*256-86);
			pVarFlammes->dep+=1.93f;
		}
		break;

		case 5:                     // lent en couleur
		for (i=0;i<85;i++)
		{
			palette[i*4]=palette[(i+1)*4];
			palette[i*4+1]=palette[(i+1)*4+1];
			palette[i*4+2]=palette[(i+1)*4+2];
		}
		palette[85*4]  =(char)(sinus[pVarFlammes->cyc1&255]/4);
		palette[85*4+1]=(char)(sinus[pVarFlammes->cyc2&255]/4);
		palette[85*4+2]=(char)(sinus[pVarFlammes->cyc3&255]/4);
		pVarFlammes->cyc1+=sinus[cyc&255]/100;
		pVarFlammes->cyc2+=sinus[(cyc*3)&255]/80;
		pVarFlammes->cyc3+=sinus[(cyc/2)&255]/120;
		break;

		case 6:                     // rapide couleur
	    memcpy(palette,palette+4,86*4);
		palette[85*4]  =(char)(sinus[pVarFlammes->cyc1&255]/4);
		palette[85*4+1]=(char)(sinus[pVarFlammes->cyc2&255]/4);
		palette[85*4+2]=(char)(sinus[pVarFlammes->cyc3&255]/4);
		pVarFlammes->cyc1+=sinus[cyc&255]/30;
		pVarFlammes->cyc2+=sinus[(cyc*3)&255]/24;
		pVarFlammes->cyc3+=sinus[(cyc/2)&255]/47;
		break;

		case 7:                     // rapide couleur
	    memcpy(palette,palette+4,86*4);
		palette[85*4]  =(char)(sinus[pVarFlammes->cyc1&255]/4);
		palette[85*4+1]=(char)(sinus[pVarFlammes->cyc2&255]/4);
		palette[85*4+2]=(char)(sinus[pVarFlammes->cyc3&255]/4);
		pVarFlammes->cyc1+=sinus[(cyc/10)&255]/30;
		pVarFlammes->cyc2+=sinus[(cyc/3)&255]/24;
		pVarFlammes->cyc3+=sinus[(cyc/19)&255]/47;
		break;

		case 8:                     // noir & blanc scroll
	    memcpy(palette,palette+4,86*4);
		palette[85*4]  =(char)(sinus[pVarFlammes->cyc1&255]/4);
		palette[85*4+1]=(char)(sinus[pVarFlammes->cyc1&255]/4);
		palette[85*4+2]=(char)(sinus[pVarFlammes->cyc1&255]/4);
		pVarFlammes->cyc1+=sinus[cyc&255]/30;
		pVarFlammes->cyc2+=sinus[(cyc*3)&255]/24;
		pVarFlammes->cyc3+=sinus[(cyc/2)&255]/47;
		break;

	}
	pVarFlammes->cyc++;
}

//////////////////////////////////////////////////////////////////////////////////////////

static void flammes(TVarFlammes *pVarFlammes, TParam *pParam)
{
	unsigned int		i;
	int		eff[256];
	float	amort;
	byte	*image=pVarFlammes->image;
	byte	*spectre=(byte *)(void *)pParam->spectre;
	byte	*sample=(byte *)(void *)pParam->sample;
	int		teta_flammes=pVarFlammes->teta_flammes;
	flam_up(pVarFlammes);
	switch(pParam->choix3)
	{
		case 1:
		for(i=0; i<256 ; i++)
			*(image+i+256)=*(image+i)=spectre[(teta_flammes+i)&127]*10+150;
		break;

		case 2:
		amort=0;
		for(i=0;i<256;i++)
		{
			amort=(amort*4.f+(float)spectre[i/5])/5.f;
			eff[i]=(int)amort;
		}
		for(i=0; i<128 ; i++)
			*(image+i+256+128)=*(image+i+128)=*(image+i+256)=*(image+i)=eff[(teta_flammes+i)&255]*10+150;
		break;

		case 3:
		for(i=0; i<128 ; i++)
			*(image+i+256+128)=*(image+i+128)=*(image+i+256)=*(image+i)=sample[(teta_flammes+i)&255]*10+150;
		break;
	}
	pVarFlammes->teta_flammes++;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

int FlammesAction(void *p, TParam *pParam)
{
	TVarFlammes	*pVarFlammes=(TVarFlammes *)p;

	flammes(pVarFlammes, pParam);
	calcul_deform(pVarFlammes, pParam);
	calc_pal(pVarFlammes, pParam);

	if(pParam->touche[0])
	{
		byte	vscreen[64000];
		affiche(pVarFlammes, vscreen);
		blur((byte *)(void *)pParam->ecran, vscreen);
	}
	else
	{
		affiche(pVarFlammes, (byte *)(void *)pParam->ecran);
	}

	return AEE_NOERROR;
}
	
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void *FlammesInit()
{
	TVarFlammes	*pVarFlammes=(TVarFlammes *)malloc(sizeof(TVarFlammes));
	if(pVarFlammes)
	{
		memset(pVarFlammes, 0, sizeof(TVarFlammes));
		pVarFlammes->dep=(float)rand();
		pVarFlammes->cyc=rand();
		pVarFlammes->cyc1=rand();
		pVarFlammes->cyc2=rand();
		pVarFlammes->cyc3=rand();
		{
			DWORD	*p=pVarFlammes->effet;
			int		i;
			for(i=0; i<256; i++)
				*(p++)=127;
		}
	}
	return (void *)pVarFlammes;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void FlammesFree(void *p)
{
	if(p)
		free(p);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static TInfo	info=
{
	AET_INFO,
	AET_ECRAN_OLD,
	MAKEINTRESOURCE(IDB_FLAMME),
	"GoaVibes",
	"GoaVibes (flammes)",
	"fire on the sound",
	"code by renan",
    "an aestesis product",
    "aestesis'98",
    "renan@aestesis.org",
    "http://www.aestesis.org/",
	NULL,
	FlammesAction,
	FlammesInit,
	FlammesFree,
	NULL,
	8,
	8,
	3,
	0,
	{TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE},
	{FALSE, FALSE, FALSE, FALSE},
};

static TTooltips	tooltips=
{
/* choix11		*/	"form: normal",
/* choix12		*/	"form: mooth",
/* choix13		*/	"form: trash",
/* choix14		*/	"form: active toggle",
/* choix15		*/	"form: fly 1",
/* choix16		*/	"form: fly 2",
/* choix17		*/	"form: fly 3",
/* choix18		*/	"form: zoomed active toggle",
/* choix19		*/	NULL,
/* choix10		*/	NULL,

/* choix21		*/	"color: normal",
/* choix22		*/	"color: black & white",
/* choix23		*/	"color: on sound",
/* choix24		*/	"color: speed",
/* choix25		*/	"color: cycle low",
/* choix26		*/	"color: cycle medium",
/* choix27		*/	"color: cycle high",
/* choix28		*/	"color: cycle black & white",
/* choix29		*/	NULL,
/* choix20		*/	NULL,

/* choix31		*/	"fire: spectre 1",
/* choix32		*/	"fire: spectre 2",
/* choix33		*/	"fire: sample",
/* choix34		*/	NULL,
/* choix35		*/	NULL,
/* choix36		*/	NULL,
/* choix37		*/	NULL,
/* choix38		*/	NULL,
/* choix39		*/	NULL,
/* choix30		*/	NULL,

/* choix41		*/	NULL,
/* choix42		*/	NULL,
/* choix43		*/	NULL,
/* choix44		*/	NULL,
/* choix45		*/	NULL,
/* choix46		*/	NULL,
/* choix47		*/	NULL,
/* choix48		*/	NULL,
/* choix49		*/	NULL,
/* choix40		*/	NULL,

/* vb1			*/	NULL,
/* vb2			*/	NULL,
/* vb3			*/	NULL,
/* vb4			*/	NULL,

/* t0			*/	"blur",
/* t1			*/	NULL,
/* t2			*/	NULL,
/* t3			*/	NULL,
/* t4			*/	"toggle: deform 1",
/* t5			*/	"toggle: smooth",
/* t6			*/	NULL,
/* t7			*/	"toggle: deform 2",
/* t8			*/	"toggle: deform 3",
/* t9			*/	"toggle: deform 4"
};

int InitializeFlammes(TInfo **ppInfo, TTooltips **ppTooltips)
{
	// set info
	*ppInfo=&info;
	*ppTooltips=&tooltips;
	
	// init global
	calc_sinus();
	calc_table();
	return AEE_NOERROR;		
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
