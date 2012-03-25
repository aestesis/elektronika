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

#include					"house.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static int					tabzoom[320*200];
static char					table[128000];

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

typedef struct SVar
{
	int						ndecaltab;
	char					tab[5][320*200*2];
	int						samps[256*3];
} TVar;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void negimage(dword *image)
{
	__asm
	{
        pushad
        mov     esi,image
        mov     ecx,64000/2
    addimage_bc:
        mov     eax,[esi]
        mov     ebx,[esi+4]
		neg		eax
		neg		ebx
        mov     [esi],eax
        mov     [esi+4],ebx
        add     esi,8
        dec     ecx
        jnz     addimage_bc
        popad
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

static void addimage(dword *dest, dword *src)
{
	__asm
	{
        pushad
        mov     esi,src
        mov     edi,dest
        mov     ecx,64000
    addimage_bc:
        mov     ebx,[edi]
        mov     eax,[esi]
        shr     ebx,1
        add     esi,4
        shr     eax,1
        and     ebx,01111111011111110111111101111111b
        and     eax,01111111011111110111111101111111b
        add     eax,ebx
        mov     [edi],eax
        add     edi,4
        dec     ecx
        jnz     addimage_bc
        popad
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("",off)
#pragma code_seg("automod")
static void myzoom(dword *dest, dword *src)
{
	__asm
	{
        pushad
		mov		eax,dest
		mov		edx,src
		lea		ebx,tabzoom
        lea     esi,myzoom_cgtabzoom
        mov     [esi-4],edx
        mov     edi,eax
        mov     esi,ebx
        mov     ecx,64000
        //dec     ecx				// peut etre a enlever ???
    myzoom_bc:
        mov     eax,[esi]
        add     esi,4
        mov     eax,[eax*4+012345678h]
    myzoom_cgtabzoom:
        mov     [edi],eax
        add     edi,4
        dec     ecx
        jnz     myzoom_bc
        popad
	}
}
#pragma code_seg()
#pragma optimize("",on)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("",off)
#pragma code_seg("automod")
static void sglurf32(dword *image, void *table, int ncolor, TVar *pVar)
{
	dword	samps=(dword)pVar->samps;
	__asm
	{
        pushad
		mov		eax,image
		mov		edx,table
		mov		ebx,samps
		mov		esi,ncolor
		cmp		esi,1
		jz		col1
		cmp		esi,2
		jz		col2

		//col3
        lea     esi,cg_samp3
        mov     [esi-4],ebx
        lea     esi,cg_samp5
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp1
        mov     [esi-4],ebx
        lea     esi,cg_samp4
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp6
        mov     [esi-4],ebx
        lea     esi,cg_samp2
        mov     [esi-4],ebx
		jmp		next

	col2:
        lea     esi,cg_samp3
        mov     [esi-4],ebx
        lea     esi,cg_samp4
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp1
        mov     [esi-4],ebx
        lea     esi,cg_samp2
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp5
        mov     [esi-4],ebx
        lea     esi,cg_samp6
        mov     [esi-4],ebx
		jmp		next

	col1:
        lea     esi,cg_samp1
        mov     [esi-4],ebx
        lea     esi,cg_samp2
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp3
        mov     [esi-4],ebx
        lea     esi,cg_samp4
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp5
        mov     [esi-4],ebx
        lea     esi,cg_samp6
        mov     [esi-4],ebx
		jmp		next

	next:
		push	ebp
        mov     edi,eax
        mov     esi,edx
        mov     ebp,64000
    sglurf32_bc:
        movzx   eax,word ptr[esi]
        add     esi,2
        mov     ebx,eax
        and     ebx,0ffh
        shr     eax,8
        xor     ecx,ecx
        mov     dl,[eax*4+012345678h]
    cg_samp1:
        mov     cl,[ebx*4+012345678h]
    cg_samp2:
        add     cl,dl
        shl     ecx,8
        mov     dl,[eax*4+012345678h]
    cg_samp3:
        mov     cl,[ebx*4+012345678h]
    cg_samp4:
        add     cl,dl
        shl     ecx,8
        mov     dl,[eax*4+012345678h]
    cg_samp5:
        mov     cl,[ebx*4+012345678h]
    cg_samp6:
        add     cl,dl
        mov     [edi],ecx
        add     edi,4
        dec     ebp
        jnz     sglurf32_bc
		pop		ebp
        popad
	}
}
#pragma code_seg()
#pragma optimize("",on)

//////////////////////////////////////////////////////////////////////////////////////////

#pragma optimize("",off)
#pragma code_seg("automod")
static void sglurf32b(dword *image, void *table, int ncolor, TVar *pVar)
{
	dword	samps=(dword)pVar->samps;
	__asm
	{
        pushad
		mov		eax,image
		mov		edx,table
		mov		ebx,samps
		mov		esi,ncolor
		cmp		esi,1
		jz		col1
		cmp		esi,2
		jz		col2

		//col3
        lea     esi,cg_samp3
        mov     [esi-4],ebx
        lea     esi,cg_samp5
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp1
        mov     [esi-4],ebx
        lea     esi,cg_samp4
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp6
        mov     [esi-4],ebx
        lea     esi,cg_samp2
        mov     [esi-4],ebx
		jmp		next

	col2:
        lea     esi,cg_samp3
        mov     [esi-4],ebx
        lea     esi,cg_samp4
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp1
        mov     [esi-4],ebx
        lea     esi,cg_samp2
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp5
        mov     [esi-4],ebx
        lea     esi,cg_samp6
        mov     [esi-4],ebx
		jmp		next

	col1:
        lea     esi,cg_samp1
        mov     [esi-4],ebx
        lea     esi,cg_samp2
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp3
        mov     [esi-4],ebx
        lea     esi,cg_samp4
        mov     [esi-4],ebx
        add     ebx,256*4
        lea     esi,cg_samp5
        mov     [esi-4],ebx
        lea     esi,cg_samp6
        mov     [esi-4],ebx
		jmp		next

	next:
		push	ebp
        mov     edi,eax
        mov     esi,edx
        mov     ebp,64000
    sglurf32_bc:
        movzx   eax,word ptr[esi]
        add     esi,2
        mov     ebx,eax
        and     ebx,0ffh
        shr     eax,8
        xor     ecx,ecx
        mov     dl,[eax*4+012345678h]
    cg_samp1:
        mov     cl,[ebx*4+012345678h]
    cg_samp2:
        add     cl,dl
        shl     ecx,8
        mov     dl,[eax*4+012345678h]
    cg_samp3:
        mov     cl,[ebx*4+012345678h]
    cg_samp4:
        add     cl,dl
        shl     ecx,8
        mov     dl,[eax*4+012345678h]
    cg_samp5:
        mov     cl,[ebx*4+012345678h]
    cg_samp6:
        add     cl,dl
		mov		edx,ecx
		cmp		ch,cl
		ja		vert

	// bleu
		mov		ch,cl
		shr		ecx,8
		cmp		cl,ch
		ja		bbleu

		// brouge
			mov		ecx,edx
			shr		ecx,1
			and		edx,0ff0000h
			and		ecx,0007f7fh
			or		edx,ecx
			jmp		itsok

		bbleu:
			mov		ecx,edx
			shr		ecx,1
			and		edx,00000ffh
			and		ecx,07f7f00h
			or		edx,ecx
			jmp		itsok
		
		
	vert:
		shr		ecx,8
		cmp		cl,ch
		ja		vvert
		
		// vrouge
			mov		ecx,edx
			shr		ecx,1
			and		edx,0ff0000h
			and		ecx,0007f7fh
			or		edx,ecx
			jmp		itsok
			
		vvert:
			mov		ecx,edx
			shr		ecx,1
			and		edx,000ff00h
			and		ecx,07f007fh
			or		edx,ecx

	itsok:
        mov     [edi],edx
        add     edi,4
        dec     ebp
        jnz     sglurf32_bc
		pop		ebp
        popad
	}
}
#pragma code_seg()
#pragma optimize("",on)

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void calc_tab(char *tabl)
{
	double	alp1=(double)rand();
	double	alp2=(double)rand();
	double	alp3=(double)rand();
	double	alp4=(double)rand();
	double	alp5=(double)(rand()%10000)/10000.0;
	double	alp6=(double)(rand()%10000)/10000.0;
	double	alp7=(double)(rand()%10000)/10000.0;
	double	alp8=(double)(rand()%10000)/10000.0;
	double	alp9=(double)rand();
	double	alp10=(double)(rand()%10000)/10000.0;
	double	alp11=(double)(rand()%10000)/10000.0;
	double	alp12=(double)(rand()%10000)/10000.0;
	double	alp13=(double)(rand()%10000)/10000.0;
	double	alp14=(double)(rand()%10000)/10000.0;
	double	xe1=(sin(alp1)*100)+80;
	double	ye1=(sin(alp2)*100)+100;
	double	xe2=(sin(alp3)*100)+80;
	double	ye2=(sin(alp4)*100)+100;
	double	xe3=(sin(alp11)*100)+80;
	double	ye3=(sin(alp12)*100)+100;
	double	zoom=(sin(alp9)*80)+120;
	int		n;
	for(n=0; n<32000; n++)
	{
        double  y=(double)(n/160);
        double  x=(double)(n%160);
        int     adr;
        double  v,vv1,vv2;
		{
			double  dx1=x-xe1;
			double  dx2=x-xe2;
			double  dx3=x-xe3;
			double  dy1=y-ye1;
			double  dy2=y-ye2;
			double  dy3=y-ye3;
			v=(sin(sqrt(dx1*dx1+dy1*dy1)/zoom)+sin(sqrt(dx2*dx2+dy2*dy2)/zoom)+sin(sqrt(dx3*dx3+dy3*dy3)/zoom))*0.666666666;
		}
		{
			double  dx1=x-xe1-1;
			double  dx2=x-xe2-1;
			double  dx3=x-xe3-1;
			double  dy1=y-ye1;
			double  dy2=y-ye2;
			double  dy3=y-ye3;
			vv1=(sin(sqrt(dx1*dx1+dy1*dy1)/zoom)+sin(sqrt(dx2*dx2+dy2*dy2)/zoom)+sin(sqrt(dx3*dx3+dy3*dy3)/zoom))*0.666666666;
		}
		{
			double  dx1=x-xe1;
			double  dx2=x-xe2;
			double  dx3=x-xe3;
			double  dy1=y-ye1-1;
			double  dy2=y-ye2-1;
			double  dy3=y-ye3-1;
			vv2=(sin(sqrt(dx1*dx1+dy1*dy1)/zoom)+sin(sqrt(dx2*dx2+dy2*dy2)/zoom)+sin(sqrt(dx3*dx3+dy3*dy3)/zoom))*0.666666666;
		}
		int		v2=(int)((v+2)*255.999/4.0);
        int     v1=(int)((atan2(v-vv1, v-vv2 )*127.999/PI)+128);
        adr=(((int)x)+((int)y)*320)*2;
        *(tabl+adr)=v1;
        *(tabl+adr+1)=v2;
        alp1+=alp5;
        alp2+=alp6;
        alp3+=alp7;
        alp4+=alp8;
        alp9+=alp10;
        alp11+=alp13;
        alp12+=alp14;
    }
	for(n=0; n<32000; n++)
    {
        int     y=n/160;
        int     x=n%160;
        int     adrd=(y*320+319-x)*2;
        int     adrs=(y*320+x)*2;
        *(tabl+adrd)=*(tabl+adrs);
        *(tabl+adrd+1)=*(tabl+adrs+1);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

static void calc_samps(unsigned char *sample, TVar *pVar)
{
    int     i;
    int     s1=0;
    int     s2=0;
    int     smp[256];
	int		*samps=pVar->samps;

	{
		int				*dest=smp;
		unsigned char	*src=sample;
		for(i=0; i<256; i++)
			*(dest++)=*(src++)-127;
	}

    for(i=-31; i<32; i++)
    {
        s1+=smp[i&255];
    }

	{
		int				*dest=samps;
		for(i=0; i<256; i++)
		{
			int     d=i-31;
			int     f=i+32;
			d=smp[d&255];
			f=smp[f&255];
			s1+=f-d;
			*(dest++)=s1>>5;
		}
	}

    for(i=0; i<256; i++)
        smp[i]-=samps[i];


    for(i=-4; i<5; i++)
    {
        s2+=smp[i&255];
    }

    for(i=0; i<256; i++)
    {
        int     d=i-4;
        int     f=i+5;
        d=smp[d&255];
        f=smp[f&255];
        s2+=f-d;
        samps[i+256]=s2>>3;
    }

    for(i=0; i<256; i++)
        samps[i+512]=smp[i]-samps[i+256];

    for(i=0; i<256; i++)
    {
         int     a;
    
         a=(samps[i]*samps[i])>>4;
         if(a>127) a=127;
         samps[i]=a;
     
         a=(samps[i+256]*samps[i+256])>>4;
         if(a>127) a=127;
         samps[i+256]=a;
     
         a=(samps[i+512]*samps[i+512])>>4;
         if(a>127) a=127;
         samps[i+512]=a;
    }
}
/*
void calc_samps(unsigned char *sample, TVar *pVar)
{
    int     i;
    int     s1=0;
    int     s2=0;
    int     smp[256];
	int		*samps=pVar->samps;

	{
		int				*dest=smp;
		unsigned char	*src=sample;
		for(i=0; i<256; i++)
			*(dest++)=*(src++)-127;
	}

    for(i=-31; i<32; i++)
    {
        s1+=smp[i&255];
    }

	{
		int				*dest=samps;
		for(i=0; i<256; i++)
		{
			int     d=i-31;
			int     f=i+32;
			d=smp[d&255];
			f=smp[f&255];
			s1+=f-d;
			*(dest++)=s1>>5;
		}
	}

    for(i=0; i<256; i++)
        smp[i]-=samps[i];


    for(i=-3; i<4; i++)
    {
        s2+=smp[i&255];
    }

    for(i=0; i<256; i++)
    {
        int     d=i-3;
        int     f=i+4;
        d=smp[d&255];
        f=smp[f&255];
        s2+=f-d;
        samps[i+256]=s2>>3;
    }

    for(i=0; i<256; i++)
        samps[i+512]=smp[i]-samps[i+256];

    for(i=0; i<256; i++)
    {
         int     a;
    
         a=(samps[i]*samps[i])>>4;
         if(a>127) a=127;
         samps[i]=a;
     
         a=(samps[i+256]*samps[i+256])>>4;
         if(a>127) a=127;
         samps[i+256]=a;
     
         a=(samps[i+512]*samps[i+512])>>4;
         if(a>127) a=127;
         samps[i+512]=a;
    }
}
*/
//////////////////////////////////////////////////////////////////////////////////////////

static void calc_tabzoom()
{
	int		*tabzoo=tabzoom;
    int     y,x;
    for(y=0; y<200; y++)
    for(x=0; x<320; x++)
    {
        int nx=x-(int)(((double)x-160.0)/10.0);
        int ny=y-(int)(((double)y-100.0)/10.0);
        *(tabzoo++)=ny*320+nx;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static int Action(void *p, TParam *pParam)
{
	TVar	*pVar=(TVar *)p;
	calc_samps(pParam->sample, pVar);

	if(pParam->spacebar||pParam->touche[2])
	{
		pVar->ndecaltab--;
		if(pVar->ndecaltab<0)
			pVar->ndecaltab=4;
		calc_tab(&pVar->tab[pVar->ndecaltab][0]);
	}

	switch(((pParam->choix-1)%3)+1)
	{
		case 1:
		if(!pParam->touche[1])
			sglurf32((dword *)pParam->ecran, &pVar->tab[(pVar->ndecaltab+pParam->choix2-1)%5][0], pParam->choix3, pVar);
		else
			sglurf32b((dword *)pParam->ecran, &pVar->tab[(pVar->ndecaltab+pParam->choix2-1)%5][0], pParam->choix3, pVar);
		break;

		case 2:
		{
			dword	image2[64000];
			if(!pParam->touche[1])
				sglurf32(image2, &pVar->tab[(pVar->ndecaltab+pParam->choix2-1)%5][0], pParam->choix3, pVar);
			else
				sglurf32b(image2, &pVar->tab[(pVar->ndecaltab+pParam->choix2-1)%5][0], pParam->choix3, pVar);
			negimage((dword *)pParam->ecran);
			addimage(image2, (dword *)pParam->ecran);
			myzoom((dword *)pParam->ecran, image2);
		}
		break;

		case 3:
		{
			dword	image2[64000];
			if(!pParam->touche[1])
				sglurf32(image2, &pVar->tab[(pVar->ndecaltab+pParam->choix2-1)%5][0], pParam->choix3, pVar);
			else
				sglurf32b(image2, &pVar->tab[(pVar->ndecaltab+pParam->choix2-1)%5][0], pParam->choix3, pVar);
			addimage(image2, (dword *)pParam->ecran);
			myzoom((dword *)pParam->ecran, image2);
		}
		break;
	}
	return AEE_NOERROR;
}
	
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void *Init()
{
	TVar	*pVar=(TVar *)malloc(sizeof(TVar));
	if(pVar)
		memset(pVar, 0, sizeof(TVar));
	pVar->ndecaltab=4;
	memcpy(&pVar->tab[4][0], table, 128000);
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
/* choix11		*/	"zoom: off",
/* choix12		*/	"zoom: gely",
/* choix13		*/	"zoom: on",
/* choix14		*/	NULL,
/* choix15		*/	NULL,
/* choix16		*/	NULL,
/* choix17		*/	NULL,
/* choix18		*/	NULL,
/* choix19		*/	NULL,
/* choix10		*/	NULL,

/* choix21		*/	"form: 1",
/* choix22		*/	"form: 2",
/* choix23		*/	"form: 3",
/* choix24		*/	"form: 4",
/* choix25		*/	"form: 5",
/* choix26		*/	NULL,
/* choix27		*/	NULL,
/* choix28		*/	NULL,
/* choix29		*/	NULL,
/* choix20		*/	NULL,

/* choix31		*/	"color: hot",
/* choix32		*/	"color: middle",
/* choix33		*/	"color: cold",
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

/* t0			*/	NULL,
/* t1			*/	"high contrast",
/* t2			*/	"generate forms",
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
	MAKEINTRESOURCE(IDB_HOUSE),
	"House",
	"House Machine",
	"You can use the space bar\r\n4 generate a new form",
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
	3,
	5,
	3,
	0,
	{FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	{FALSE, FALSE, FALSE, FALSE},
};

int InitializeHouse(TInfo **ppInfo, TTooltips **ppTooltips)
{
	// set info
	*ppInfo=&info;
	*ppTooltips=&tooltips;
	
	// init global
    calc_tabzoom();
	calc_tab(table);
	return AEE_NOERROR;		
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
