// coded by Renan
// project : VIDEOASTESIS
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include					<windows.h>
#include					<math.h>
#include					"resource.h"

//////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

#include					"zygo.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


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
} TVar;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

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

#pragma optimize("" , off)
#pragma code_seg("automod")

static void addzygo(TVar *pVar, int x, int y, int n)
{
	dword	hauteur=(dword)&pVar->hauteur[0];
	dword	wave=(dword)&pVar->wave[0];
	dword	dist=(dword)&distance[n][x+y*644];
	{
		__asm
		{
			mov		esi,dist
			mov		ecx,wave
			sub		esi,322*2
			lea		edx,cg_wave
			mov		edi,hauteur
			mov		[edx-4],ecx
			push	ebp
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
}
#pragma code_seg()
#pragma optimize("",on)

static void cadrehauteur(TVar *pVar)
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

static void blurhauteur(TVar *pVar)
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
static void dec(TVar *pVar, TParam *pParam)
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

static void hauteur2image(TVar *pVar, TParam *pParam)
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

static void affiche(TVar *pVar, TParam *pParam)
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

	palette(pVar, pParam);
	wave(pVar, pParam);
	affiche(pVar, pParam);

	return AEE_NOERROR;
}
	
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

__inline float frand()
{
	return (float)rand()/(float)RAND_MAX;
}

static void *Init()
{
	TVar	*pVar=(TVar *)malloc(sizeof(TVar));
	if(pVar)
		memset(pVar, 0, sizeof(TVar));
	{ // init palette cycling value
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
	return (void *)pVar;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void Free(void *p)
{
	if(p)
		free(p);
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
	AET_ECRAN_OLDP,
	MAKEINTRESOURCE(IDB_ZYGO),
	"ZygO",
	"ZygO",
	"ZygO properties:"RET\
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

int InitializeZygO(TInfo **ppInfo, TTooltips **ppTooltips)
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
