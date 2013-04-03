// coded by Renan
// project : VIDEOASTESIS
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include					<windows.h>
#include					<math.h>
#include					<stdlib.h>
#include					"resource.h"

//////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

#include					"patchly.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define						NBVAL						30
#define						NBZYG						5
#define						PI							3.1415

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static word					distance[2][644*404];
static word					decrement[10][4096];
static byte					color[512];

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

typedef struct SPal
{
	float					a01;
	float					a02;
	float					a03;
	float					a04;
	float					a11;
	float					a12;
	float					a13;
	float					a14;
	float					a21;
	float					a22;
	float					a23;
	float					a24;
} TPal;

typedef struct SVar
{
	TPal					pal1;
	word					hauteur[322*202];
	byte					wave[512];
	float					a1;
	float					a2;
	float					a3;
	float					a4;
	int						affiche_bool;

	int						color[1024];
	float					xx[NBZYG];
	float					yy[NBZYG];
	float					zz[NBZYG];
	float					*ol;
	float					*nl;

	float					val[NBVAL];
	float					ang[NBVAL];
	float					acc[NBVAL];
} TVar;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline float crop(float x)
{
	x*=2.f;
	if(x<1024.f)
		return x;
	if(x<2048.f)
		return 2047.99999f-x;
	if(x<3072.f)
		return x-2047.99999f;
	return 4096.00001f-x;
}

__inline float crop2(float x)
{
	if(x<256.f)
		return x*4.f;
	if(x<512.f)
		return 2047.99999f-x*4.f;
	if(x<(512.f+256.f))
		return x*4.f-2047.99999f;
	return 4096.00001f-x*4.f;
}

__inline float frand()
{
	return (float)rand()/(float)RAND_MAX;
}

__inline float calc(TVar *pVar,int x, int y)
{
	float	z=0;
	int		i;
	for(i=0; i<NBZYG; i++)
	{
		float	dx=(float)x-pVar->xx[i];
		float	dy=(float)y-pVar->yy[i];
		z+=(float)sin(sqrt(dx*dx+dy*dy)*pVar->zz[i])*(511.99f/(float)NBZYG)+(512.f/(float)NBZYG);
	}
	return z;
}

__inline DWORD mcolor1(DWORD a, DWORD b)
{
	int	r1=(a>>16)&255;
	int	g1=(a>> 8)&255;
	int	b1=(a    )&255;
	return (((r1+b)&510)<<15)|(((g1+b)&510)<<7)|((b1+b)>>1);
}

__inline DWORD mcolor2(DWORD a, DWORD b)
{
	int	r1=(a>>16)&255;
	int	g1=(a>> 8)&255;
	int	b1=(a    )&255;
	return (((r1*b)&(0xff<<8))<<8)|(((g1*b)&(0xff<<8)))|((b1*b)>>8);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void effect(TVar *pVar, TParam *pParam, DWORD *p, int w, int h)
{
	int		y,x,i;
	float	*val=pVar->val;
	for(i=0; i<NBZYG; i++)
	{
		pVar->xx[i]=(float)val[25]*w;
		pVar->yy[i]=(float)val[26]*h;
		pVar->zz[i]=50.f/(val[27]*w*10.f+(float)w);
	}
	switch(0)
	{
		case 0:
		{
			float	ar=val[0]*100.f;
			float	ag=val[1]*100.f;
			float	ab=val[2]*100.f;
			float	dr=(val[3]+0.1f)*0.1f;
			float	dg=(val[4]+0.1f)*0.1f;
			float	db=(val[5]+0.1f)*0.1f;
			float	ar0=val[6]*100.f;
			float	ag0=val[7]*100.f;
			float	ab0=val[8]*100.f;
			float	dr0=(val[9]+0.1f)*0.09f;
			float	dg0=(val[10]+0.1f)*0.09f;
			float	db0=(val[11]+0.1f)*0.09f;
			float	ar1=val[12]*100.f;
			float	ag1=val[13]*100.f;
			float	ab1=val[14]*100.f;
			float	dr1=(val[15]+0.1f)*0.03f;
			float	dg1=(val[16]+0.1f)*0.03f;
			float	db1=(val[17]+0.1f)*0.03f;
			float	ar10=val[18]*100.f;
			float	ag10=val[19]*100.f;
			float	ab10=val[20]*100.f;
			float	dr10=(val[21]+0.1f)*0.01f;
			float	dg10=(val[22]+0.1f)*0.01f;
			float	db10=(val[23]+0.1f)*0.01f;
			float	n=1.f/(val[24]*512.f+16.f);
			for(i=0; i<1024; i++)
			{
				float	m=(float)sin(PI*(float)i*n)*0.4999f+0.5f;
				int		r=(int)((1.f-m)*((float)sin(ar1)*sin(ar10)*127.9f+128.f)+m*((float)sin(ar)*sin(ar0)*127.9f+128.f))&255;
				int		g=(int)((1.f-m)*((float)sin(ag1)*sin(ag10)*127.9f+128.f)+m*((float)sin(ag)*sin(ag0)*127.9f+128.f))&255;
				int		b=(int)((1.f-m)*((float)sin(ab1)*sin(ab10)*127.9f+128.f)+m*((float)sin(ab)*sin(ab0)*127.9f+128.f))&255;
				pVar->color[i]=r<<16|g<<8|b;
				ar+=dr;
				ag+=dg;
				ab+=db;
				ar0+=dr0;
				ag0+=dg0;
				ab0+=db0;
				ar1+=dr1;
				ag1+=dg1;
				ab1+=db1;
				ar10+=dr10;
				ag10+=dg10;
				ab10+=db10;
			}
		}
		break;
	}
	switch(pParam->choix&7)
	{
		case 0:
		{
			int	*p=pVar->color;
			for(i=0; i<1024; i++)
			{
				*p=((*p>>1)&0x7f0000)|(*p&0x00ffff);
				p++;
			}
		}
		break;

		case 1:
		{
			int	*p=pVar->color;
			for(i=0; i<1024; i++)
			{
				*p=((*p>>1)&0x007f00)|(*p&0xff00ff);
				p++;
			}
		}
		break;

		case 2:
		{
			int	*p=pVar->color;
			for(i=0; i<1024; i++)
			{
				*p=((*p>>1)&0x00007f)|(*p&0xffff00);
				p++;
			}
		}
		break;

		case 3:
		{
			int	*p=pVar->color;
			for(i=0; i<1024; i++)
			{
				*p=((*p>>1)&0x7f7f00)|(*p&0x0000ff);
				p++;
			}
		}
		break;

		case 4:
		{
			int	*p=pVar->color;
			for(i=0; i<1024; i++)
			{
				*p=((*p>>1)&0x7f007f)|(*p&0x00ff00);
				p++;
			}
		}
		break;

		case 5:
		{
			int	*p=pVar->color;
			for(i=0; i<1024; i++)
			{
				*p=((*p>>1)&0x007f7f)|(*p&0xff0000);
				p++;
			}
		}
		break;
	}
	{
		switch(pParam->choix4&7)
		{
			case 0:
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=calc(pVar,x,-1);
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=calc(pVar,-1, y);
				for(x=0; x<w; x++)
				{
					float	z=calc(pVar,x, y);
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor1(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 1:	
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=calc(pVar,x,-1);
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=calc(pVar,-1, y);
				for(x=0; x<w; x++)
				{
					float	z=calc(pVar,x, y);
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor2(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 2:
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=calc(pVar,x,-1);
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=calc(pVar,-1, y);
				for(x=0; x<w; x++)
				{
					float	z=calc(pVar,x, y);
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor1(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 3:	
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=crop2(calc(pVar,x,-1));
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=crop2(calc(pVar,-1, y));
				for(x=0; x<w; x++)
				{
					float	z=crop2(calc(pVar,x, y));
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor2(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 4:
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=crop(calc(pVar,x,-1)+calc(pVar,w-x,h));
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=crop(calc(pVar,-1, y)+calc(pVar,w,h-y));
				for(x=0; x<w; x++)
				{
					float	z=crop(calc(pVar,x, y)+calc(pVar,w-x, h-y));
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor1(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 5:
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=crop(calc(pVar,x,-1)+calc(pVar,w-x,h));
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=crop(calc(pVar,-1, y)+calc(pVar,w,h-y));
				for(x=0; x<w; x++)
				{
					float	z=crop(calc(pVar,x, y)+calc(pVar,w-x, h-y));
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor2(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 6:
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=(calc(pVar,x,-1)+calc(pVar,w-x,h))*0.5f;
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=0.5f*(calc(pVar,-1, y)+calc(pVar,w,h-y));
				for(x=0; x<w; x++)
				{
					float	z=0.5f*(calc(pVar,x, y)+calc(pVar,w-x, h-y));
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor1(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;

			case 7:
			{
				float	*pol=pVar->ol;
				for(x=0; x<w; x++)
					*(pol++)=0.5f*(calc(pVar,x,-1)+calc(pVar,w-x,h));
			}
			for(y=0; y<h; y++)
			{
				float	*pnl=pVar->nl;
				float	*pol=pVar->ol;
				float	lp=0.5f*(calc(pVar,-1, y)+calc(pVar,w,h-y));
				for(x=0; x<w; x++)
				{
					float	z=0.5f*(calc(pVar,x, y)+calc(pVar,w-x, h-y));
					int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
					*(p++)=(int)mcolor2(pVar->color[(int)z], l);
					*(pnl++)=z;
					lp=z;
				}
				memcpy(pVar->ol, pVar->nl, w*sizeof(float));
			}
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void efree(TVar *pVar)
{
	if(pVar->ol)
		free(pVar->ol);
	if(pVar->nl)
		free(pVar->nl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool einit(TVar *pVar,int w, int h)
{
	pVar->ol=(float *)malloc(w*sizeof(float));
	pVar->nl=(float *)malloc(w*sizeof(float));
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static __inline void palette(TVar *pVar, TParam *pParam)
{
	switch(pParam->choix2)
	{
		case 1:
		{
			byte	*p=pParam->palette;
			int		i;
			for(i=0; i<256; i++)
			{
				*(p++)=i;
				*(p++)=i;
				*(p++)=i;
				p++;
			}
		}
		break;

		case 2:
		{
			byte	*p=pParam->palette;
			TPal	*pal=&pVar->pal1;
			int		i;
			float	a1=pal->a01;
			float	a2=pal->a02;
			float	a3=pal->a03;
			float	a4=pal->a04;
			float	d1=pal->a21;
			float	d2=pal->a22;
			float	d3=pal->a23;
			float	d4=pal->a24;
			for(i=0; i<256; i++)
			{
				*(p++)=((byte)(sin(a1)*127.f+128.f)*i)>>8;
				*(p++)=((byte)(sin(a2)*127.f+128.f)*i)>>8;
				*(p++)=((byte)(sin(a3)*127.f+128.f)*i)>>8;
				p++;
				a1+=d1*5.f;
				a2+=d2*5.f;
				a3+=d3*5.f;
			}
			pal->a01+=pal->a11;
			pal->a02+=pal->a12;
			pal->a03+=pal->a13;
			pal->a04+=pal->a14;
		}
		break;

		case 3:
		{
			byte	*p=pParam->palette;
			TPal	*pal=&pVar->pal1;
			int		i;
			float	a1=pal->a01;
			float	a2=pal->a02;
			float	a3=pal->a03;
			float	a4=pal->a04;
			float	d1=pal->a21;
			float	d2=pal->a22;
			float	d3=pal->a23;
			float	d4=pal->a24;
			for(i=0; i<256; i++)
			{
				float	v=(float)sin(a4)*0.5f+0.5f;
				*(p++)=((byte)((sin(a1+v)*127.f+128.f))*i)>>8;
				*(p++)=((byte)((sin(a2-v)*127.f+128.f))*i)>>8;
				*(p++)=((byte)((sin(a3)*127.f+128.f))*i)>>8;
				p++;
				a1+=d1*5.f;
				a2+=d2*5.f;
				a3+=d3*5.f;
				a4+=d4*10.f;
			}
			pal->a01+=pal->a11;
			pal->a02+=pal->a12;
			pal->a03+=pal->a13;
			pal->a04+=pal->a14;
		}
		break;

		case 4:
		{
			byte	*p=pParam->palette;
			TPal	*pal=&pVar->pal1;
			int		i;
			float	a1=pal->a01;
			float	a2=pal->a02;
			float	a3=pal->a03;
			float	a4=pal->a04;
			float	d1=pal->a21;
			float	d2=pal->a22;
			float	d3=pal->a23;
			float	d4=pal->a24;
			for(i=0; i<256; i++)
			{
				float	v=(float)sin(a4)*0.5f+0.5f;
				*(p++)=(byte)((sin(a1)*127.f+128.f)*v);
				*(p++)=(byte)((sin(a2)*127.f+128.f)*v);
				*(p++)=(byte)((sin(a3)*127.f+128.f)*v);
				p++;
				a1+=d1*5.f;
				a2+=d2*5.f;
				a3+=d3*5.f;
				a4+=d4*10.f;
			}
			pal->a01+=pal->a11*5.f;
			pal->a02+=pal->a12*5.f;
			pal->a03+=pal->a13*5.f;
			pal->a04+=pal->a14*5.f;
		}
		break;
	}
}

static __inline void wave(TVar *pVar, TParam *pParam)
{
/*
	switch(pParam->choix2)
	{
		case 1:
*/
		if(!pParam->touche[0])
		{
			byte	*p0=&pVar->wave[0];
			byte	*p1=&pVar->wave[511];
			byte	*s=&pParam->sample[0];
			byte	min=255;
			int		i;
			for(i=0; i<256; i++)
			{
				if(*s<min)
					min=*s;
				s++;
			}
			s=&pParam->sample[0];
			for(i=0; i<256; i++)
			{
				byte	v=*(s++)-min;
				*(p0++)=v;
				*(p1--)=v;
			}
		}
/*
		break;
	}
*/
}

#pragma optimize("",off)
#pragma code_seg("automod")
static __inline void addzygo(TVar *pVar, int x, int y, int n)
{
	dword	hauteur=(dword)&pVar->hauteur[0];
	dword	wave=(dword)&pVar->wave[0];
	dword	dist=(dword)&distance[n][x+y*644];
	__asm
	{
		mov		esi,dist
		mov		ecx,wave
		sub		esi,322*2
		lea		edx,cg_wave
		mov		edi,hauteur
		push	ebp
		mov		[edx-4],ecx
		mov		ebp,202
	boucy:
		add		esi,322*2
		dec		ebp
		js		theend
	
		mov		ecx,322
	boucx:
		dec		ecx
		js		boucy

		movzx	eax,word ptr[esi]
		add		esi,2
		movzx	eax,byte ptr[eax+012345678h]
	cg_wave:
		add		[edi],ax
		add		edi,2
		jmp		boucx

	theend:
		pop		ebp
	}
}
#pragma code_seg()
#pragma optimize("",on)

static __inline void cadrehauteur(TVar *pVar)
{
	dword	h1=(dword)&pVar->hauteur[0];
	dword	h2=(dword)&pVar->hauteur[201*322];
	dword	h3=(dword)&pVar->hauteur[1*322];
	dword	h4=(dword)&pVar->hauteur[1*322+321];
	__asm
	{
		mov		esi,h1
		mov		edi,h2
		mov		ecx,h3
		mov		edx,h4
		push	ebp
		mov		ebp,322
	boucx:
		movzx	eax,word ptr[esi]
		movzx	ebx,word ptr[edi]
		shr		eax,1
		shr		ebx,1
		mov		[esi],ax
		add		esi,2
		mov		[edi],bx
		add		edi,2
		dec		ebp
		jnz		boucx

		mov		ebp,200
	boucy:
		movzx	eax,word ptr[ecx]
		movzx	ebx,word ptr[edx]
		shr		eax,1
		shr		ebx,1
		mov		[ecx],ax
		add		ecx,2*322
		mov		[edx],bx
		add		edx,2*322
		dec		ebp
		jnz		boucy

		pop		ebp
	}
}

static __inline void blurhauteur(TVar *pVar)
{
	dword	hauteur=(dword)&pVar->hauteur[1*322+1];
	__asm
	{
		mov		esi,hauteur	
		push	ebp
		sub		esi,2*2
		mov		ebp,200
	boucy:
		add		esi,2*2
		dec		ebp
		js		theend
	
		mov		ecx,320
	boucx:
		dec		ecx
		js		boucy

		movzx	eax,word ptr[esi+(((-1*322)-1)*2)]
		movzx	ebx,word ptr[esi+(((-1*322)+0)*2)]
		mov		edx,eax
		movzx	eax,word ptr[esi+(((-1*322)+1)*2)]
		add		edx,ebx
		movzx	ebx,word ptr[esi+((( 0*322)-1)*2)]
		add		edx,eax
		movzx	eax,word ptr[esi+((( 0*322)+1)*2)]
		add		edx,ebx
		movzx	ebx,word ptr[esi+((( 1*322)-1)*2)]
		add		edx,eax
		movzx	eax,word ptr[esi+((( 1*322)+0)*2)]
		add		edx,ebx
		movzx	ebx,word ptr[esi+((( 1*322)+1)*2)]
		add		edx,eax
		add		edx,ebx
		shr		edx,3
		mov		[esi],dx
		add		esi,1*2
		jmp		boucx

	theend:
		pop		ebp
	}
}

#pragma optimize("",off)
#pragma code_seg("automod")
static __inline void dec(TVar *pVar, TParam *pParam)
{
	dword	hauteur=(dword)&pVar->hauteur[0];
	dword	decre=(dword)&decrement[pParam->choix4-1][0];
	__asm
	{
		mov		eax,decre
		lea		esi,cgdecrement1
		lea		edi,cgdecrement2
		mov		[esi-4],eax
		mov		[edi-4],eax

		mov		esi,hauteur	
		push	ebp
		mov		ebp,202
	boucy:
		dec		ebp
		js		theend
	
		mov		ecx,322/2
	boucx:
		dec		ecx
		js		boucy

		movzx	eax,word ptr[esi]
		movzx	ebx,word ptr[esi+2]
		and		eax,4095
		and		ebx,4095
		mov		ax,[eax*2+012345678h]
	cgdecrement1:
		mov		bx,[ebx*2+012345678h]
	cgdecrement2:
		mov		[esi],ax
		mov		[esi+2],bx
		add		esi,2*2
		jmp		boucx

	theend:
		pop		ebp
	}
}
#pragma code_seg()
#pragma optimize("",on)

static __inline void hauteur2image(TVar *pVar, TParam *pParam)
{
	dword	hauteur=(dword)&pVar->hauteur[1*322+1];
	dword	ecran=(dword)&pParam->ecran;
	__asm
	{
		mov		esi,hauteur	
		mov		edi,ecran
		push	ebp
		sub		esi,2*2
		mov		ebp,200
	boucy:
		add		esi,2*2
		dec		ebp
		js		theend
	
		mov		ecx,320
	boucx:
		dec		ecx
		js		boucy

		movzx	eax,word ptr[esi]
		add		esi,2
		and		eax,511
		mov		al,color[eax]
		mov		[edi],al
		inc		edi
		jmp		boucx

	theend:
		pop		ebp
	}
}

static __inline void affiche(TVar *pVar, TParam *pParam)
{
	{
		int		ndist=0;

		switch(((pParam->choix-1)&12)>>2)
		{
			case 0:
			ndist=0;
			break;

			case 1:
			ndist=1;
			break;

			case 2:
			pVar->affiche_bool=!pVar->affiche_bool;
			ndist=(pVar->affiche_bool)?1:0;
			break;

			case 3:
			pVar->affiche_bool=(pVar->affiche_bool+1)&3;
			ndist=(pVar->affiche_bool)?1:0;
			break;
		}

		switch((pParam->choix-1)&3)
		{
			case 0:
			addzygo(pVar, rand()%322, rand()%202, ndist);
			break;

			case 1:
			pVar->a1=(float)((sin(pVar->a2)+sin(pVar->a3))*PI/2.f);
			pVar->a2=(float)((cos(pVar->a1)+sin(pVar->a4))*PI/2.f);
			pVar->a3+=0.012445122f;
			pVar->a4+=0.01562125f;
			addzygo(pVar, (int)(sin(pVar->a1)*160.f+161.f), (int)(sin(pVar->a2)*100.f+101.f), ndist);
			break;

			case 2:
			pVar->a1=(float)((sin(pVar->a2)+sin(pVar->a3))*PI/2.f);
			pVar->a2=(float)((cos(pVar->a1)+sin(pVar->a4))*PI/2.f);
			pVar->a3+=0.052445122f;
			pVar->a4+=0.06562125f;
			addzygo(pVar, (int)(sin(pVar->a1)*160.f+161.f), (int)(sin(pVar->a2)*100.f+101.f), ndist);
			break;

			case 3:
			pVar->a1=(float)((sin(pVar->a2)+sin(pVar->a3))*PI/2.f);
			pVar->a2=(float)((cos(pVar->a1)+sin(pVar->a4))*PI/2.f);
			pVar->a3+=0.022551211f;
			pVar->a4+=0.0258899954f;
			{
				int		x=(int)(sin(pVar->a1)*160.f+161.f);
				int		y=(int)(sin(pVar->a2)*100.f+101.f);
				addzygo(pVar, x, y, ndist);
				addzygo(pVar, 321-x, 201-y, ndist);
			}
			break;
		}
	}
	/*
	switch(pParam->choix3)
	{
		case 1:
	*/
		cadrehauteur(pVar);
		blurhauteur(pVar);
		dec(pVar, pParam);
		hauteur2image(pVar, pParam);
	/*
		break;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static int Action(void *p, TParam *pParam)
{
	TVar	*pVar=(TVar *)p;

	{
		int	i;
		for(i=0; i<NBVAL; i++)
		{
			pVar->val[i]=(float)sin(pVar->ang[i])*0.5f+0.5f;
			pVar->ang[i]+=pVar->acc[i];
		}
	}

	//palette(pVar, pParam);
	//wave(pVar, pParam);
	//affiche(pVar, pParam);
	effect(pVar, pParam, (DWORD *)pParam->ecran, 320, 200);
	return AEE_NOERROR;
}
	
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void *Init()
{
	TVar	*pVar=(TVar *)malloc(sizeof(TVar));
	if(pVar)
	{
		memset(pVar, 0, sizeof(TVar));
		{ // init palette cycling value // for zygo
			TPal	*pal=&pVar->pal1;
			pal->a01=frand()*100.f;
			pal->a02=frand()*100.f;
			pal->a03=frand()*100.f;
			pal->a04=frand()*100.f;
			pal->a11=frand()*0.01f;
			pal->a12=frand()*0.01f;
			pal->a13=frand()*0.01f;
			pal->a14=frand()*0.01f;
			pal->a21=frand()*0.01f;
			pal->a22=frand()*0.01f;
			pal->a23=frand()*0.01f;
			pal->a24=frand()*0.01f;
		}
		{	// init cycling random (pathchly)
			int	i;
			for(i=0; i<25; i++)
			{
				pVar->ang[i]=frand()*1000;
				pVar->acc[i]=frand()*0.1f;
			}
			for(i=25; i<NBVAL; i++)
			{
				pVar->ang[i]=frand()*1000;
				pVar->acc[i]=frand()*0.1f;
			}
		}
		einit(pVar,320, 200);
	}
	return (void *)pVar;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void Free(void *p)
{
	if(p)
	{
		efree((TVar *)p);
		free(p);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static TTooltips	tooltips=
{
/* choix11		*/	"form: random circular",
/* choix12		*/	"form: slow circular",
/* choix13		*/	"form: speed circular",
/* choix14		*/	"form: symetric circular",
/* choix15		*/	"form: random angular",
/* choix16		*/	"form: slow angular",
/* choix17		*/	"form: speed angular",
/* choix18		*/	"form: symetric angular",
/* choix19		*/	"form: random mixed",
/* choix10		*/	"form: slow mixed",

/* choix21		*/	"color: black & white",
/* choix22		*/	"color: low palette",
/* choix23		*/	"color: middle palette",
/* choix24		*/	"color: high palette",
/* choix25		*/	NULL,
/* choix26		*/	NULL,
/* choix27		*/	NULL,
/* choix28		*/	NULL,
/* choix29		*/	NULL,
/* choix20		*/	NULL,

/* choix31		*/	NULL,
/* choix32		*/	NULL,
/* choix33		*/	NULL,
/* choix34		*/	NULL,
/* choix35		*/	NULL,
/* choix36		*/	NULL,
/* choix37		*/	NULL,
/* choix38		*/	NULL,
/* choix39		*/	NULL,
/* choix30		*/	NULL,

/* choix41		*/	"dec speed: level 1",
/* choix42		*/	"dec speed: level 2",
/* choix43		*/	"dec speed: level 3",
/* choix44		*/	"dec speed: level 4",
/* choix45		*/	"dec speed: level 5",
/* choix46		*/	"dec speed: level 6",
/* choix47		*/	"dec speed: level 7",
/* choix48		*/	"dec speed: level 8",
/* choix49		*/	"dec speed: level 9",
/* choix40		*/	"dec speed: level 10",

/* vb1			*/	NULL,
/* vb2			*/	NULL,
/* vb3			*/	NULL,
/* vb4			*/	NULL,

/* t0			*/	"freeze wave input",
/* t1			*/	NULL,
/* t2			*/	NULL,
/* t3			*/	NULL,
/* t4			*/	NULL,
/* t5			*/	NULL,
/* t6			*/	NULL,
/* t7			*/	NULL,
/* t8			*/	NULL,
/* t9			*/	NULL
};

static TInfo	info=
{
	AET_INFO,
	AET_ECRAN_NEW,
	NULL, /*MAKEINTRESOURCE(IDB_FLAMMES),*/
	"Patchly",
	"Patchly",
	"Patchly properties:"RET\
	""RET\
	"line 1\tselect form and zygo movement."RET\
	"line 3\timage movement"RET\
	"line 4\tdefragmentation speed"RET\
	"0\t\tkeep the wave",
	"code by renan",
    "an aestesis product",
    "aestesis'98",
    "renan@aestesis.org",
    "http://www.aestesis.org/",
	NULL,
	Action,
	Init,
	Free,
	NULL,
	10,
	4,
	0,
	10,
	{TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	{FALSE, FALSE, FALSE, FALSE},
};

int InitializePatchly(TInfo **ppInfo, TTooltips **ppTooltips)
{
	// set info
	*ppInfo=&info;
	*ppTooltips=&tooltips;
	
	// init global
	{// calcul table distance
		int		x;
		int		y;
		word	*p=&distance[0][0];
		for(y=0; y<404; y++)
			for(x=0; x<644; x++)
			{
				float	dx=(float)x-319.5f;
				float	dy=(float)y-199.5f;
				*(p++)=(word)sqrt(dx*dx+dy*dy);
			}				

	}
	{// calcul table distance
		int		x;
		int		y;
		word	*p=&distance[1][0];
		for(y=0; y<404; y++)
			for(x=0; x<644; x++)
			{
				float	dx=(float)x-319.5f;
				float	dy=(float)y-199.5f;
				*(p++)=((word)(atan2(dy, dx)*511.99f/(2.f*PI)))&511;
			}				

	}
	{// calcul color (ping pong)
		byte	*p0=&color[0];
		byte	*p1=&color[511];
		int		i;
		for(i=0; i<256; i++)
			*(p0++)=*(p1--)=i;
	}
	{// calcul decrement
		int	j;
		for(j=0; j<10; j++)
		{
			word	*p=&decrement[j][0];
			int		i;
			int		d=((9-j)*3+3);
			for(i=0; i<4096; i++)
				*(p++)=i-(i/d);
		}
	}
	return AEE_NOERROR;		
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
