// coded by Renan
// project : VIDEOASTESIS
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include			<windows.h>
#include			<math.h>
#include			"resource.h"
#include			"effect.h"
#include			"dsglurf.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define				MAXSIN				4096
#define				HSIN				65535.9

static byte			table[65536*2];
static byte			sinus[256*256];
static dword		adrsinus=(dword)&sinus[0];

//////////////////////////////////////////////////////////////////////////////////////////

typedef struct SVarSglurf
{
	byte			image[64000];
	byte			tx[320];
	byte			ty[256];
	byte			sp_prim[128];
	byte			sp_amort[128];
	byte			sp_liss[128];
	byte			sp_liss2[128];
	byte			waitvbl;
	float			beta1;
	float			beta2;
	float			beta3;
	float			beta4;
	float			alpha;
} TVarSglurf;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void calc_sinus()
{
	int		i,j;
	byte	*p=sinus;
	for(j=0; j<256; j++)
	{
		float v=(float)j;
		for(i=0; i<256; i++)
			*(p++)=(byte)(v*(sin((float)i*2*PI/256.f)*0.5f+0.5f));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("",off)
#pragma code_seg("automod")
static void sglurf(TVarSglurf	*pVarSglurf, byte * dest)
{
	DWORD		tx=(DWORD)(&pVarSglurf->tx[0]);
	DWORD		ty=(DWORD)(&pVarSglurf->ty[0]);
	__asm
	{
		lea		esi,cg_tx
		lea		edi,cg_ty
		mov		eax,tx
		mov		ebx,ty
		mov		[esi-4],eax
		mov		[edi-4],ebx
        mov     edx,dest
        mov     esi,100

	boucy:
		dec		esi
		js		lafin

        mov     edi,160
	boucx:
		dec		edi
		js		boucy

        mov     al,[edi+012345678h]
	cg_tx:
        add     al,[esi+012345678h]
	cg_ty:
        mov     [edx],al
        inc     edx
        jmp     boucx
	lafin:
	}
}
#pragma code_seg()
#pragma optimize("",on)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("",off)
#pragma code_seg("automod")
static void mirror(byte * dest, byte * source)
{
	static DWORD	y;
	__asm
	{
		mov		eax,dest
		mov		ebx,source
		push	ebp

        mov     esi,eax
        mov     edi,eax
        mov     ecx,eax
        mov     edx,eax
        mov     ebp,ebx

        add     edi,320-4
        add     ecx,320*199
        add     edx,64000

        mov     y,100


boucy2:
        mov     ebx,160/4
boucx2:
        mov     eax,[ebp]
        sub     edx,4
        mov     [esi],eax
        add     ebp,4
        mov     [ecx],eax
        bswap   eax
        add     esi,4
        mov     [edi],eax
        add     ecx,4
        mov     [edx],eax
        sub     edi,4


        dec     ebx
        jnz     boucx2

        add     esi,160
        add     edi,160+320
        sub     ecx,160+320
        sub     edx,160
        
        dec     y
        jnz     boucy2
		pop		ebp
	}
}
#pragma code_seg()
#pragma optimize("",on)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void mirror2(byte * dest, byte * source)
{
	static DWORD	y;
	__asm
	{
		mov		eax,dest
		mov		ebx,source
		push	ebp
		
        mov     ebp,ebx
        mov     esi,eax
        mov     edi,eax
        mov     ecx,eax
        mov     edx,eax
        add     ebp,160*100-4

        add     edi,320-4
        add     ecx,320*199
        add     edx,64000-4

        mov     y,100


boucy3:
        mov     ebx,160/4
boucx3:
        mov     eax,[ebp]
        bswap   eax
        sub     edx,4
        mov     [esi],eax
        sub     ebp,4
        mov     [ecx],eax
        bswap   eax
        add     esi,4
        mov     [edi],eax
        add     ecx,4
        mov     [edx],eax
        sub     edi,4


        dec     ebx
        jnz     boucx3

        add     esi,160
        add     edi,160+320
        sub     ecx,160+320
        sub     edx,160
        
        dec     y
        jnz     boucy3
		pop		ebp
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("",off)
#pragma code_seg("automod")
static void polaire(TVarSglurf	*pVarSglurf, byte * dest)
{
	DWORD		tx=(DWORD)(&pVarSglurf->tx[0]);
	DWORD		ty=(DWORD)(&pVarSglurf->ty[0]);
	__asm
	{
		lea		esi,cg_table
		lea		eax,table[0]
		mov		[esi-4],eax
		lea		esi,cg_tx
		lea		edi,cg_ty
		mov		eax,tx
		mov		ebx,ty
		mov		[esi-4],eax
		mov		[edi-4],ebx
		mov		eax,dest
		push	ebp
        lea     esi,cg_dest
        mov     [esi-4],eax

        xor     edx,edx
        xor     eax,eax

        mov     ebp,100
        xor     ebx,ebx

boucy5:
        mov     ecx,160

boucx5:
        mov     ax,[edx*2+012345678h]
	cg_table:
        mov     edi,eax
        mov     esi,eax
        and     edi,0ffh
        shr     esi,8
        mov     al,[esi+012345678h]
	cg_tx:
        add     al,[edi+012345678h]
	cg_ty:
        mov     [012345678h+ebx],al
cg_dest:
        inc     edx
        inc     ebx

        dec     ecx
        jnz     boucx5

        add     edx,160

        dec     ebp
        jnz     boucy5
		pop		ebp
	}
}
#pragma code_seg()
#pragma optimize("",on)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

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
        shr     edx,4
        inc     esi
        mov     [edi],dl
        jmp     blur_bx

    blur_fy:

        pop     ebp
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void calc_table(void)
{
	int		x;
	int		y;
	byte	mteta;
	byte	*ptr=table;
	double	src_x;
	double	src_y;
	double	r;
	double	teta;
	for(y=0 ;y<200; y++)
		for(x=0; x<320; x++)
		{
			src_x=160-x;
			src_y=100-y;
			r=sqrt(src_x*src_x+src_y*src_y);
			if(r!=0)
			{
				if(src_y>0)
					teta=asin(src_y/r);
				else
					teta=asin(-src_y/r);
                              
				if(src_y>0)
				{
					if(src_x<0)
						teta=PI+teta;
					else
						teta=PI*2-teta;
				}      
				else if(src_x<0)
					teta=PI-teta;
			} 
			mteta=(byte)((teta*128)/PI);
			if(mteta<64)
				*(ptr++)=mteta;
			else if (mteta<128)
				*(ptr++)=127-mteta;
			else if (mteta<192)
				*(ptr++)=mteta-128;
			else
	            *(ptr++)=255-mteta;
			*(ptr++)=(char)r;
		}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void lissages(TVarSglurf *pVarSglurf, TParam *pParam)
{
	int		i;
	byte	accel,ancien;
	float	amorti,valeur;
	byte	*spectre=pParam->spectre;
	word	*spectre16=pParam->spectre16;
	byte	*touche=pParam->touche;
	byte	*sp_prim=pVarSglurf->sp_prim;
	byte	*sp_amort=pVarSglurf->sp_amort;
	byte	*sp_liss=pVarSglurf->sp_liss;
	byte	*sp_liss2=pVarSglurf->sp_liss2;

	if(pParam->touche[9]!=0)
	{
		byte	*sp=pParam->spectre;
		word	*sp16=pParam->spectre16;
		for(i=0;i<128;i++)
			*(sp++)=*(sp16++)&255;
	}

	ancien=0;
	valeur=0;
	amorti=0;
	for(i=0;i<128;i++)
	{
		accel=spectre[i]-ancien;
		ancien=spectre[i];
		amorti=(amorti*4+(float)accel)/5;
		valeur=valeur+amorti/50;
		sp_prim[i]=(byte) valeur;
	}
	for(i=0;i<128;i++)
	{
		sp_amort[i]=(sp_amort[i]*4+spectre[i])/5;
		sp_liss[i] =(sp_amort[i]+sp_amort[(i+1)&127]+sp_amort[(i+2)&127]+sp_amort[(i+3)&127]+sp_amort[(i-1)&127]+sp_amort[(i-2)&127]+sp_amort[(i-3)&127])*(5*touche[3]+1);
		sp_liss2[i]=(spectre[i]+spectre[(i+1)&127]+spectre[(i+2)&127]+spectre[(i+3)&127]+spectre[(i-1)&127]+spectre[(i-2)&127]+spectre[(i-3)&127])*(5*touche[3]+1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static __inline int sign(byte n)
{
	char val;
	val=n-126;
	if (val==0)
		return(0);
	else
		return (val/abs(val));
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void val_tx(TVarSglurf *pVarSglurf, TParam *pParam, int nb)
{
	int		i,j;
	byte	valeur;
	byte	*sample=pParam->sample;
	byte	*spectre=pParam->spectre;
	word	*spectre16=pParam->spectre16;
	byte	*touche=pParam->touche;
	byte	*sp_prim=pVarSglurf->sp_prim;
	byte	*sp_amort=pVarSglurf->sp_amort;
	byte	*sp_liss=pVarSglurf->sp_liss;
	byte	*sp_liss2=pVarSglurf->sp_liss2;
	byte	*tx=pVarSglurf->tx;
	float	beta1=pVarSglurf->beta1;
	float	beta2=pVarSglurf->beta1;
	float	beta3=pVarSglurf->beta1;
	float	beta4=pVarSglurf->beta1;
	switch(nb)
	{
		case 1:
		{
			byte	*s=sample+1;
			byte	*d=tx;
			byte	v0,v1=sample[0];
			for(i=0;i<40;i++)
			{
				v0=v1;
				v1=sample[i];
				*(d++)=(v0*3+v1)>>2;
				*(d++)=(v0+v1)>>1;
				*(d++)=(v0+v1*3)>>2;
				*(d++)=v1;
			}
		}
		break;

		case 2:
		i=0;
		while( ((valeur=sign(sample[i&255]))!=1) && (i<256) ) i++;
		while((sign(sample[i&255])==sign(valeur))&(i<500)) i++; 
		for(j=0;j<160;j++)
			tx[j]=sample[(((128*j)/160)+i)&255]-126;
		break;

		case 3:
		for(i=0;i<160;i++)
			tx[i]=sp_liss[(i*80)/160];
		break;

		case 4:
		for(i=0;i<160;i++)
			tx[i]=sp_liss2[(i*80)/160];
		break;

		case 5:
		for(i=0;i<160;i++)
			tx[i]=(byte) (	sin(beta1+(float)sp_liss[25]/1000+(float)((i*128)/160)*(float)(sp_liss[20]+1)/1000)*sin(beta2+(float)sp_liss[15]/1000+(float)((i*128)/160)*(float)(sp_liss[10]+1)/1000)  *
							sin(beta3+(float)sp_liss[ 5]/1000+(float)((i*128)/160)*(float)(sp_liss[ 0]+1)/1000)*sin(beta4+(float)sp_liss[35]/1000+(float)((i*128)/160)*(float)(sp_liss[30]+1)/1000)*63)  ;
        break;

		case 6:
		for(i=0;i<160;i++)
			tx[i]=(byte) (	sin(beta1+(float)((i*128)/160)*(sp_liss[20]+1)/100)*31+sin(beta2+(float)((i*128)/160)*(sp_liss[10]+1)/150)*31  +
							sin(beta3+(float)((i*128)/160)*(sp_liss[ 0]+1)/200)*31+sin(beta4+(float)((i*128)/160)*(sp_liss[30]+1)/50)*31)  ;
        break;

		case 7:
		{
			dword	t[160];
			dword	adrt=(DWORD)&t[0];
			dword	adrtx=(dword)&tx[0];
			dword	adrspectre=(dword)&spectre[0];
			memset(t, 0, 160*sizeof(dword));
			__asm
			{
				mov		esi,adrspectre
				add		esi,127
				mov		edx,128*4
				push	ebp
			boucspec:
				pop		ebp
				sub		edx,4
				js		theend

				movzx	ebx,byte ptr[esi]
				dec		esi

				shl		ebx,8
				xor		eax,eax
				add		ebx,adrsinus

				mov		ecx,160
				mov		edi,adrt

				push	ebp

			bouctx:
				mov		ebp,eax
				dec		ecx
				js		boucspec

				shr		ebp,2
				and		ebp,255
				movzx	ebp,byte ptr ds:[ebx+ebp]
				add		eax,edx
				add		[edi],ebp
				add		edi,4
				jmp		bouctx
				
			theend:
				
				mov		ecx,160
				mov		esi,adrt
				mov		edi,adrtx
			boucniv:
				mov		eax,[esi]
				add		esi,4
				mov		[edi],al
				inc		edi
				dec		ecx
				jnz		boucniv
			}
		}
		break;
	}
	for (i=0;i<96;i++)
		tx[i+160]=tx[159-(i*160/96)];
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void val_ty(TVarSglurf *pVarSglurf, TParam *pParam, int nb)
{
	int i,j;
	byte valeur;
	byte	*sample=pParam->sample;
	byte	*spectre=pParam->spectre;
	word	*spectre16=pParam->spectre16;
	byte	*touche=pParam->touche;
	byte	*sp_prim=pVarSglurf->sp_prim;
	byte	*sp_amort=pVarSglurf->sp_amort;
	byte	*sp_liss=pVarSglurf->sp_liss;
	byte	*sp_liss2=pVarSglurf->sp_liss2;
	byte	*ty=pVarSglurf->ty;
	float	beta1=pVarSglurf->beta1;
	float	beta2=pVarSglurf->beta1;
	float	beta3=pVarSglurf->beta1;
	float	beta4=pVarSglurf->beta1;
	switch(nb)
	{
		case 1:
		{
			byte	*s=sample+1;
			byte	*d=ty;
			byte	v0,v1=sample[0];
			for(i=0;i<25;i++)
			{
				v0=v1;
				v1=sample[i];
				*(d++)=(v0*3+v1)>>2;
				*(d++)=(v0+v1)>>1;
				*(d++)=(v0+v1*3)>>2;
				*(d++)=v1;
			}
		}
        break;

		case 2:
        i=0;
        while( ((valeur=sign(sample[i]))!=1) && (i<256) ) i++;
        while((sign(sample[i&255])==sign(valeur))&(i<500)) i++; 
        for(j=0;j<100;j++)
			ty[j]=sample[(((128*j)/100)+i)&255]-126;
        break;

		case 3:
        for(i=0;i<100;i++)
          ty[i]=sp_liss[(i*80)/100];
        break;

		case 4:
        for(i=0;i<100;i++)
          ty[i]=sp_liss2[(i*80)/100];
        break;


		case 5:
        for(i=0;i<100;i++)
          ty[i]=(byte) (sin(beta1+(float)sp_liss[25]/1000+(float)((i*128)/100)*(sp_liss[20]+1)/1000)*sin(beta2+(float)sp_liss[15]/1000+(float)((i*128)/100)*(sp_liss[10]+1)/1000) \
                       *sin(beta3+(float)sp_liss[ 5]/1000+(float)((i*128)/100)*(sp_liss[ 0]+1)/1000)*sin(beta4+(float)sp_liss[35]/1000+(float)((i*128)/100)*(sp_liss[30]+1)/1000)*63);

		case 6:
        for(i=0;i<100;i++)
			ty[i]=(byte) (	sin(beta1+(float)((i*128)/100)*(sp_liss[20]+1)/100)*31+sin(beta2+(float)((i*128)/100)*(sp_liss[10]+1)/150)*31+ \
							sin(beta3+(float)((i*128)/200)*(sp_liss[ 0]+1)/200)*31+sin(beta4+(float)((i*128)/100)*(sp_liss[30]+1)/50)*31);
        break;

		case 7:
		{
			dword	t[100];
			dword	adrt=(DWORD)&t[0];
			dword	adrty=(dword)&ty[0];
			dword	adrspectre=(dword)&spectre[0];
			memset(t, 0, 100*sizeof(dword));
			__asm
			{
				mov		esi,adrspectre
				add		esi,127
				mov		edx,128*4
				push	ebp
			boucspec:
				pop		ebp
				sub		edx,4
				js		theend

				movzx	ebx,byte ptr[esi]
				dec		esi

				shl		ebx,8
				xor		eax,eax
				add		ebx,adrsinus

				mov		ecx,100
				mov		edi,adrt

				push	ebp

			bouctx:
				mov		ebp,eax
				dec		ecx
				js		boucspec

				shr		ebp,2
				and		ebp,255
				movzx	ebp,byte ptr ds:[ebx+ebp]
				add		eax,edx
				add		[edi],ebp
				add		edi,4
				jmp		bouctx
				
			theend:
				
				mov		ecx,100
				mov		esi,adrt
				mov		edi,adrty
			boucniv:
				mov		eax,[esi]
				add		esi,4
				mov		[edi],al
				inc		edi
				dec		ecx
				jnz		boucniv
			}
		}
		break;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void the_sglurf(TVarSglurf *pVarSglurf, TParam *pParam)
{
    pVarSglurf->beta1+=0.00013f;
    pVarSglurf->beta2+=0.00254f;
    pVarSglurf->beta3+=0.00236f;
    pVarSglurf->beta4+=0.00125f;

	switch(pParam->choix)
	{
		case 1:
		val_tx(pVarSglurf, pParam, pParam->choix3);
		if(pParam->touche[2]==0)
			sglurf(pVarSglurf, pVarSglurf->image);
		else 
			polaire(pVarSglurf, pVarSglurf->image);
		val_ty(pVarSglurf, pParam, pParam->choix3);
		break;

		case 2:
		val_tx(pVarSglurf, pParam, pParam->choix3);
		val_ty(pVarSglurf, pParam, pParam->choix3);
		if(pParam->touche[2]==0)
			sglurf(pVarSglurf, pVarSglurf->image);
		else  
			polaire(pVarSglurf, pVarSglurf->image);
        break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void calc_pal(TVarSglurf *pVarSglurf, TParam *pParam)
{
	int		i;
	byte	*palette=pParam->palette;
	float	alpha=pVarSglurf->alpha;

	switch(pParam->choix2)
	{
		case 1:
		for(i=0;i<128;i++)
		{
			palette[i*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32)*i/128;
			palette[i*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32)*i/128;
			palette[i*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32)*i/128;
			palette[(255-i)*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32)*i/128;
			palette[(255-i)*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32)*i/128;
			palette[(255-i)*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32)*i/128;
		}
		break;

		case 2:
		for(i=0;i<128;i++)
        {
			palette[i*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32)*i/128;
			palette[i*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32)*i/128;
			palette[i*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32)*i/128;
			palette[(255-i)*4]=    (byte) (sin(alpha*2.36588+(float)i/7)*31+32)*i/128;
			palette[(255-i)*4+1]=  (byte) (sin(alpha*1.023658+(float)i/8)*31+32)*i/128;
			palette[(255-i)*4+2]=  (byte) (sin(alpha*0.895462+(float)i/11)*31+32)*i/128;
		}
		break;

		case 3:
		for(i=0;i<128;i++)
		{
			palette[i*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32);
			palette[i*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32);
			palette[i*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32);
			palette[(255-i)*4]=    (byte) (sin(alpha*2.36588+(float)i/7)*31+32);
			palette[(255-i)*4+1]=  (byte) (sin(alpha*1.023658+(float)i/8)*31+32);
			palette[(255-i)*4+2]=  (byte) (sin(alpha*0.895462+(float)i/11)*31+32);
		}
		break;

        case 4:
		for(i=0;i<64;i++)
		{
			palette[i*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32)*i/128;
			palette[i*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32)*i/128;
			palette[i*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32)*i/128;
			palette[(255-i)*4]=    (byte) (sin(alpha*2.36588+(float)i/7)*31+32)*i/128;
			palette[(255-i)*4+1]=  (byte) (sin(alpha*1.023658+(float)i/8)*31+32)*i/128;
			palette[(255-i)*4+2]=  (byte) (sin(alpha*0.895462+(float)i/11)*31+32)*i/128;
		}
		for(i=64;i<128;i++)
		{
			palette[i*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32);
			palette[i*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32);
			palette[i*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32);
			palette[(255-i)*4]=    (byte) (sin(alpha*2.36588+(float)i/7)*31+32);
			palette[(255-i)*4+1]=  (byte) (sin(alpha*1.023658+(float)i/8)*31+32);
			palette[(255-i)*4+2]=  (byte) (sin(alpha*0.895462+(float)i/11)*31+32);
		}
		break;

		case 5:
		for(i=0;i<100;i++)
		{
			palette[i*4]=    (byte) 0;
			palette[i*4+1]=  (byte) 0;
			palette[i*4+2]=  (byte) 0;
			palette[(255-i)*4]=    (byte) 0;
			palette[(255-i)*4+1]=  (byte) 0;
			palette[(255-i)*4+2]=  (byte) 0;
		}
		for(i=100;i<128;i++)
		{
			palette[i*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32)*(i-100)/28;
			palette[i*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32)*(i-100)/28;
			palette[i*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32)*(i-100)/28;
			palette[(255-i)*4]=    (byte) (sin(alpha+12+(float)i/10)*31+32)*(i-100)/28;
			palette[(255-i)*4+1]=  (byte) (sin(alpha*0.7566+5+(float)i/12)*31+32)*(i-100)/28;
			palette[(255-i)*4+2]=  (byte) (sin(alpha*1.53+(float)i/9)*31+32)*(i-100)/28;
		}
		break;

		case 6:
		for(i=0;i<100;i++)
		{
			palette[i*4]=    (byte) 0;
			palette[i*4+1]=  (byte) 0;
			palette[i*4+2]=  (byte) 0;
			palette[(255-i)*4]=    (byte) 0;
			palette[(255-i)*4+1]=  (byte) 0;
			palette[(255-i)*4+2]=  (byte) 0;
		}
		for(i=100;i<128;i++)
		{
			palette[i*4]=    (byte) (63*(i-100)/28);
			palette[i*4+1]=  (byte) (63*(i-100)/28);
			palette[i*4+2]=  (byte) (63*(i-100)/28);
			palette[(255-i)*4]=    (byte) (63*(i-100)/28);
			palette[(255-i)*4+1]=  (byte) (63*(i-100)/28);
			palette[(255-i)*4+2]=  (byte) (63*(i-100)/28);
		}
		break;

		case 7:
		for(i=0;i<64;i++)
		{
			palette[i*4]=    (byte) 0;
			palette[i*4+1]=  (byte) 0;
			palette[i*4+2]=  (byte) 0;
			palette[(255-i)*4]=    (byte) 0;
			palette[(255-i)*4+1]=  (byte) 0;
			palette[(255-i)*4+2]=  (byte) 0;
		}
		for(i=64;i<128;i++)
		{
			palette[i*4]=    (byte) (i-64);
			palette[i*4+1]=  (byte) (i-64);
			palette[i*4+2]=  (byte) (i-64);
			palette[(255-i)*4]=    (byte) (i-64);
			palette[(255-i)*4+1]=  (byte) (i-64);
			palette[(255-i)*4+2]=  (byte) (i-64);
		}
		break;
	}
	pVarSglurf->alpha+=0.0101253f;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void *SglurfInit()
{
	TVarSglurf	*pVarSglurf=(TVarSglurf *)malloc(sizeof(TVarSglurf));
	if(pVarSglurf)
	{
		memset(pVarSglurf, 0, sizeof(pVarSglurf));
		pVarSglurf->beta1=(float)rand();
		pVarSglurf->beta2=(float)rand();
		pVarSglurf->beta3=(float)rand();
		pVarSglurf->beta4=(float)rand();
	}
	return (void *)pVarSglurf;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void SglurfFree(void *p)
{
	if(p)
		free(p);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

int SglurfAction(void *p, TParam *pParam)
{
	TVarSglurf	*pVarSglurf=(TVarSglurf *)p;

	if(pParam->touche[0])
	{
		pVarSglurf->waitvbl--;
		if((pVarSglurf->waitvbl>100)||(pVarSglurf->waitvbl<0))
			pVarSglurf->waitvbl=10;
	}
	else
		pVarSglurf->waitvbl=0;

	if(pVarSglurf->waitvbl==0)
	{
		calc_pal(pVarSglurf, pParam);
		lissages(pVarSglurf, pParam);
		the_sglurf(pVarSglurf, pParam);
		if(pParam->touche[5])
		{
			byte toto[64000];
			if(pParam->touche[1]==0)
				mirror(toto, pVarSglurf->image);
			else
				mirror2(toto, pVarSglurf->image);
			blur(pParam->ecran, toto);
		}
		else
		{
			if(pParam->touche[1]==0)
				mirror (pParam->ecran , pVarSglurf->image);
			else
				mirror2(pParam->ecran , pVarSglurf->image);
		}
	}
	return AEE_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static TInfo	info=
{
	AET_INFO,
	AET_ECRAN_OLD,
	MAKEINTRESOURCE(IDB_SGLURF),
	"trash",
	"trash (sglurf)",
	"the technokrate effect."RET"(real underground)",
	"code by renan",
    "an aestesis product",
    "aestesis'98",
    "renan@aestesis.org",
    "http://www.aestesis.org/",
	NULL,
	SglurfAction,
	SglurfInit,
	SglurfFree,
	NULL,
	2,
	7,
	7,
	0,
	{TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE},
	{FALSE, FALSE, FALSE, FALSE},
};

static TTooltips	tooltips=
{
/* choix11		*/	"mode: instant response",
/* choix12		*/	"mode: two frame mixed response",
/* choix13		*/	NULL,
/* choix14		*/	NULL,
/* choix15		*/	NULL,
/* choix16		*/	NULL,
/* choix17		*/	NULL,
/* choix18		*/	NULL,
/* choix19		*/	NULL,
/* choix10		*/	NULL,

/* choix21		*/	"color: normal",
/* choix22		*/	"color: 2",
/* choix23		*/	"color: 3",
/* choix24		*/	"color: flashy",
/* choix25		*/	"color: pink",
/* choix26		*/	"color: high pass black & white",
/* choix27		*/	"color: black & white",
/* choix28		*/	NULL,
/* choix29		*/	NULL,
/* choix20		*/	NULL,

/* choix31		*/	"form: sample 1",
/* choix32		*/	"form: sample 2",
/* choix33		*/	"form: spectre 1",
/* choix34		*/	"form: spectre 2",
/* choix35		*/	"form: custom 1",
/* choix36		*/	"form: custom 2",
/* choix37		*/	"form: custom 3",
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

/* t0			*/	"frame rate",
/* t1			*/	"mirror",
/* t2			*/	"polar",
/* t3			*/	"boost spectre",
/* t4			*/	NULL,
/* t5			*/	"blur",
/* t6			*/	NULL,
/* t7			*/	NULL,
/* t8			*/	NULL,
/* t9			*/	"boost spectre"
};

int InitializeSglurf(TInfo **ppInfo, TTooltips **ppTooltips)
{
	// set info
	*ppInfo=&info;
	*ppTooltips=&tooltips;
		
	// init global
	calc_table();
	calc_sinus();
	return AEE_NOERROR;		
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////