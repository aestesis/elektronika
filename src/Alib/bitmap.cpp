////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BITMAP.CPP					(c)	YoY'99						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h> 
#include						<stdio.h>
#include						<math.h>
#include						<mmsystem.h>
#include						<assert.h>

#include						<bitmap.h>
#include						<filehd.h>
#include						<filemem.h>
#include						"resources.h"

#include						"png.h"

#include						"flashSDK/Source/JPEG/jpeglib.h"
#include						"flashSDK/Source/jpeg.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
static Aproperties				properties[]=	
							{	
								{	guidDWORD,	(int)&(((Abitmap *)NULL)->state),		"state",		"bitmap states"					},
								{	guidDWORD,	(int)&(((Abitmap *)NULL)->flags),		"flags",		"bitmap flags"					},
								{	guidDWORD,	(int)&(((Abitmap *)NULL)->colorkey),	"colorkey",		"bitmap transparency colorkey"	},
								{	guidFLOAT,	(int)&(((Abitmap *)NULL)->alpha),		"alpha",		"bitmap global alpha [0..1]"	},
								{	guidSDWORD,	(int)&(((Abitmap *)NULL)->w),			"w",			"bitmap width"					},
								{	guidSDWORD,	(int)&(((Abitmap *)NULL)->h),			"h",			"bitmap height"					} 
							};
*/
ADLL ACI						Abitmap::CI=ACI("Abitmap", GUID(0xAE57E515,0x00000100), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	byte	a;
	byte	b;
	byte	c;
	byte	d;
} mctype;

static bool						binit=FALSE;
static mctype					mcalias[256];

#ifdef MMX ////////////////////////////////////////////

static qword					mulmmx[256];

static void calcmulmmx()
{
	int	i;
	for(i=0; i<256; i++)
	{
		qword	v=(qword)(i);
		mulmmx[i]=((qword)v)|((qword)v<<16)|((qword)v<<32)|((qword)v<<48);
	}
}

#endif ///////////////////////////////////////////////

static void calculmc()
{
	int	i;
	for(i=0; i<256; i++)
	{
		float	dy=(float)(i&15);
		float	dx=(float)(i>>4);
		float	idx=15.f-dx;
		float	idy=15.f-dy;
		float	d0=(float)sqrt(idx*idx+idy*idy);
		float	d1=(float)sqrt(dx*dx+idy*idy);
		float	d2=(float)sqrt(dx*dx+dy*dy);
		float	d3=(float)sqrt(idx*idx+dy*dy);
		float	s=1.f/(d0+d1+d2+d3);
		d0*=s;
		d1*=s;
		d2*=s;
		d3*=s;
		mcalias[i].a=(byte)(d0*255.f);
		mcalias[i].b=(byte)(d1*255.f);
		mcalias[i].c=(byte)(d2*255.f);
		mcalias[i].d=(byte)(d3*255.f);
	}
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!binit)
	{
		calculmc();
#ifdef MMX
		calcmulmmx();
#endif
		binit=true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap::Abitmap(Aresobj *o)
{
	png_structp		png_ptr;
	png_infop		info_ptr;
	
	init();

	adr=NULL;
	body=NULL;
	palette=NULL;
	
	if(!strcmp(o->type, "PNG"))
	{
		png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(png_ptr==NULL)
			return;
		info_ptr=png_create_info_struct(png_ptr);
		if(info_ptr==NULL)
		{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			return;
		}
		if(setjmp(png_ptr->jmpbuf))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return;
		}
		png_ptr->io_ptr.type=1;
		png_ptr->io_ptr.memory=o->lock();
		png_ptr->io_ptr.offset=0;
		png_ptr->io_ptr.size=o->getSize();
		NCreadPNG(png_ptr, info_ptr);
		o->unlock();
	}
	else
	{
		Afilemem	*fm=new Afilemem("readBMP", o->lock(), o->getSize());
		NCreadBMP(fm);
		o->unlock();
		delete(fm);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap::~Abitmap()
{
	NCfree();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap::Abitmap(int w, int h, int nbBits) : Anode("bitmap")
{
	assert((nbBits==8)||(nbBits==16)||(nbBits==32));
	assert((w>0)&&(h>0)&&(w<bitmapMAXSIZE)&&(h<bitmapMAXSIZE));
	init();
	this->w=w;
	this->h=h;
	this->nbBits=nbBits;
	NCcreate();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap::Abitmap(char *filename) : Anode(filename)
{
	init();
	w=4;
	h=4;
	nbBits=32;
	NCcreate();
	load(filename);
}	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::size(int w, int h)
{
	if((w!=this->w)||(h!=this->h))
	{
		int		nsize=w*h*(nbBits>>3);
		void	*b=malloc(nsize);
		int		ww=mini(w, this->w);
		int		hh=mini(h, this->h);
		int		i;
		memset(b, 0, nsize);
		switch(nbBits)
		{
			case 8:
			for(i=0; i<hh; i++)
				memcpy(&((byte *)b)[i*w], &body8[adr[i]], ww);
			break;

			case 16:
			for(i=0; i<hh; i++)
				memcpy(&((word *)b)[i*w], &body16[adr[i]], ww<<1);
			break;

			case 32:
			for(i=0; i<hh; i++)
				memcpy(&(((dword *)b)[i*w]), &body32[adr[i]], ww<<2);
			break;

			default:
			free(b);
			Alasterror="unknown depth resolution.";
			return FALSE;
		}
		free(body);
		body=b;
		bodysize=nsize;
		this->w=w;
		this->h=h;
		free(adr);
		adr=(dword *)malloc(sizeof(dword)*h);
		{
			int		i;
			dword	*a=adr;
			dword	v=0;
			for(i=0; i<h; i++)
			{
				*(a++)=v;
				v+=w;
			}
		}
	}
	view.x=0;
	view.y=0;
	view.w=w;
	view.h=h;
	clip.x=0;
	clip.y=0;
	clip.w=w;
	clip.h=h;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::set(HDC hdc, int x, int y, int mblt)
{
	switch(nbBits)
	{
		case 8:
		{	
			bool				r=false;
			BITMAPINFOHEADER	*bmi=(BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER)+256*4);
			if(!bmi)
			{
				Alasterror="memory error.";
				return FALSE;
			}
			memset(bmi, 0, sizeof(BITMAPINFOHEADER));
			bmi->biSize=sizeof(bmi);
			bmi->biWidth=w;
			bmi->biHeight=-h;
			bmi->biPlanes=1;
			bmi->biBitCount=nbBits;
			bmi->biCompression=BI_RGB;
			memcpy(bmi+1, palette, 256*4);
			r=SetDIBitsToDevice(hdc, x, y, w, h, 0, 0, 0, h, body, (BITMAPINFO *)bmi, DIB_RGB_COLORS)?TRUE:FALSE;
			free(bmi);
			return r;
		}
		break;

		case 16:
		case 32:
		{
			bool				r=false;
			BITMAPINFOHEADER	bmi;
			memset(&bmi, 0, sizeof(bmi));
			bmi.biSize=sizeof(bmi);
			bmi.biWidth=w;
			bmi.biHeight=-h;
			bmi.biPlanes=1;
			bmi.biBitCount=nbBits;
			bmi.biCompression=BI_RGB;
			r=SetDIBitsToDevice(hdc, x, y, w, h, 0, 0, 0, h, body, (BITMAPINFO *)&bmi, DIB_RGB_COLORS)?TRUE:FALSE;
			return r;
		}
		break;
	}
	Alasterror="unknown depth resolution.";
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::set(HDC hdc, int x, int y, int w, int h, int mblt)
{
	switch(nbBits)
	{
		case 8:
		{	
			bool				r=false;
			BITMAPINFOHEADER	*bmi=(BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER)+256*4);
			if(!bmi)
			{
				Alasterror="memory error.";
				return FALSE;
			}
			memset(bmi, 0, sizeof(BITMAPINFOHEADER));
			bmi->biSize=sizeof(bmi);
			bmi->biWidth=this->w;
			bmi->biHeight=-this->h;
			bmi->biPlanes=1;
			bmi->biBitCount=nbBits;
			bmi->biCompression=BI_RGB;
			memcpy(bmi+1, palette, 256*4);
			switch(mblt)
			{
				case bltmodeANTIALIAS:
				SetStretchBltMode(hdc, HALFTONE);
				break;
				
				default:
				SetStretchBltMode(hdc, COLORONCOLOR);
				break;
			}
			r=StretchDIBits(hdc, x, y, w, h, 0, 0, this->w, this->h, body, (BITMAPINFO *)bmi, DIB_RGB_COLORS, SRCCOPY)?TRUE:FALSE;
			free(bmi);
			return r;
		}
		break;

		case 16:
		case 32:
		{
			bool				r=false;
			BITMAPINFOHEADER	bmi;
			memset(&bmi, 0, sizeof(bmi));
			bmi.biSize=sizeof(bmi);
			bmi.biWidth=this->w;
			bmi.biHeight=-this->h;
			bmi.biPlanes=1;
			bmi.biBitCount=nbBits;
			bmi.biCompression=BI_RGB;
			switch(mblt)
			{
				case bltmodeANTIALIAS:
				SetStretchBltMode(hdc, HALFTONE);
				break;
				
				default:
				SetStretchBltMode(hdc, COLORONCOLOR);
				break;
			}
			r=StretchDIBits(hdc, x, y, w, h, 0, 0, this->w, this->h, body, (BITMAPINFO *)&bmi, DIB_RGB_COLORS, SRCCOPY)?TRUE:FALSE;
			return r;
		}
		break;
	}
	Alasterror="unknown depth resolution.";
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCsetMMX(int xd, int yd, int xs, int ys, int w, int h, class Abitmap *b, int action, int flags)
{
#ifdef MMX
	int	cstate=(action==bitmapDEFAULT)?b->state:action;
	int	cflags=(flags==bitmapDEFAULT)?b->flags:flags;

	xd+=view.x;
	yd+=view.y;

	if(xd<clip.x)
	{
		xs-=(xd-clip.x);
		w+=(xd-clip.x);
		xd=clip.x;
	}
	if(yd<clip.y)
	{
		ys-=(yd-clip.y);
		h+=(yd-clip.y);
		yd=clip.y;
	}
	if((w<=0)||(h<=0)||(xd>=(clip.x+clip.w))||(yd>=(clip.y+clip.h)))
	{
		Alasterror="outside region.";
		return FALSE;
	}

	int		ww=mini(mini(w, b->w), (clip.x+clip.w)-xd);
	int		hh=mini(mini(h, b->h), (clip.y+clip.h)-yd);

	switch(cstate)
	{
		case bitmapNORMAL:
		if(cflags&bitmapALPHA)								// bitmapNORMAL & bitmapALPHA ///////////////////////////////////////////////////////
		{
			int		j;
			{
				dword	nb=(ww>>1);
				__asm
				{
					pxor		mm1,mm1
				}
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[ys+j]+xs];
					dword	*d=&body32[adr[yd+j]+xd];
					if(nb)
					{
						dword	dest=(dword)d;
						dword	s0=(dword)s;
						__asm
						{
							mov			edi,dest
							mov			ecx,s0

							mov			eax,nb
						boucle:
							dec			eax
							js			theend

							movd		mm5,[edi]
							movd		mm7,[edi+4]
							movzx		ebx,byte ptr [ecx+3]
							movzx		edx,byte ptr [ecx+7]
							movq		mm2,mulmmx[ebx*8];
							movd		mm4,[ecx]
							neg			ebx
							movd		mm6,[ecx+4]
							add			ebx,255
							movq		mm3,mulmmx[ebx*8];
							add			ecx,8

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1

							pmullw		mm4,mm2
							punpcklbw	mm6,mm1

							pmullw		mm5,mm3
							punpcklbw	mm7,mm1

							movq		mm2,mulmmx[edx*8];
							paddw		mm4,mm5

							psrlw		mm4,8
							neg			edx

							packuswb	mm4,mm1
							add			edx,255

							movd		[edi],mm4
							movq		mm3,mulmmx[edx*8];

							pmullw		mm6,mm2
							pmullw		mm7,mm3

							paddw		mm6,mm7
							add			edi,4

							psrlw		mm6,8
							packuswb	mm6,mm1

							movd		[edi],mm6
							add			edi,4

							jmp			boucle

						theend:
							mov			eax,ww
							and			eax,1
							jz			theend2

							movd		mm5,[edi]
							movzx		ebx,byte ptr [ecx+3]
							movq		mm2,mulmmx[ebx*8];
							movd		mm4,[ecx]
							neg			ebx
							add			ebx,255
							movq		mm3,mulmmx[ebx*8];

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1

							pmullw		mm4,mm2
							pmullw		mm5,mm3

							paddw		mm4,mm5

							psrlw		mm4,8

							packuswb	mm4,mm1

							movd		[edi],mm4

						theend2:
						}
					}
				}
			}
			__asm
			{
				emms
			}
			return TRUE;
		}
		else if(cflags&bitmapCOLORKEY)						// bitmapNORMAL & bitmapCOLORKEY ////////////////////////////////////////////////////
		{
			int		i,j;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[ys+j]+xs];
				dword	*d=&body32[adr[yd+j]+xd];
				for(i=0; i<ww; i++)
				{
					if(*s!=b->colorkey)
						*d=*s;
					s++;
					d++;
				}
			}
			return TRUE;
		}
		else if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			int		a0=(int)((1.f-b->alpha)*255.99f);
			int		a1=(int)(b->alpha*255.99f);
			{
				int		j;
				{
					qword	m0=(int)(255.99f*(1.f-b->alpha));
					qword	m1=(int)(255.99f*b->alpha);
					dword	nb=(ww>>1);
					m0=(m0<<16)|m0;
					m1=(m1<<16)|m1;
					m0=QWORD(m0, m0);
					m1=QWORD(m1, m1);
					__asm
					{
						pxor		mm1,mm1
						movq		mm2,m1
						movq		mm3,m0
					}
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						if(nb)
						{
							dword	dest=(dword)d;
							dword	s0=(dword)d;
							dword	s1=(dword)s;
							__asm
							{
								mov			edi,dest
								mov			ecx,s1
								mov			edx,s0

								mov			eax,nb
							boucle00:
								dec			eax
								js			theend00

								movd		mm4,[ecx]
								add			ecx,4
								movd		mm5,[edx]
								add			edx,4

								movd		mm6,[ecx]
								add			ecx,4
								movd		mm7,[edx]
								add			edx,4

								punpcklbw	mm4,mm1
								punpcklbw	mm5,mm1
								pmullw		mm4,mm2
								pmullw		mm5,mm3

								punpcklbw	mm6,mm1
								punpcklbw	mm7,mm1
								pmullw		mm6,mm2
								pmullw		mm7,mm3

								paddusw		mm4,mm5
								paddusw		mm6,mm7

								psrlw		mm4,8
								psrlw		mm6,8

								packuswb	mm4,mm1
								packuswb	mm6,mm1

								movd		[edi],mm4
								movd		[edi+4],mm6
								add			edi,8

								jmp			boucle00

							theend00:
							}
							if(ww&1)
							{
								s+=ww-1;
								d+=ww-1;
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *s);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*d=color32(rd, gd, bd);
							}
						}
					}
					__asm
					{
						emms
					}
					return TRUE;
				}
			}
		}
		else													// bitmapNORMAL /////////////////////////////////////////////////////////////////////
		{
			int		i;
			for(i=0; i<hh; i++)
				memcpy(&body32[adr[yd+i]+xd], &b->body32[b->adr[ys+i]+xs], ww<<2);
			return TRUE;
		}
		break;

		case bitmapOR:											// bitmapOR /////////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[ys+j]+xs];
				dword	*d=&body32[adr[yd+j]+xd];
				for(i=0; i<ww; i++)
					*(d++)|=*(s++);
			}
		}
		return TRUE;

		case bitmapXOR:											// bitmapXOR ////////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[ys+j]+xs];
				dword	*d=&body32[adr[yd+j]+xd];
				for(i=0; i<ww; i++)
					*(d++)^=*(s++);
			}
		}
		return TRUE;


		case bitmapAND:											// bitmapAND ////////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[ys+j]+xs];
				dword	*d=&body32[adr[yd+j]+xd];
				for(i=0; i<ww; i++)
					*(d++)&=*(s++);
			}
		}
		return TRUE;

		case bitmapADD:											// bitmapADD ////////////////////////////////////////////////////////////////////////
		if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a0=(int)((1.f-b->alpha)*255.99f);
				int		a1=(int)(b->alpha*255.99f);
				int		j;
				{
					dword	m1=0xffffffff;
					dword	m2=(int)(255.99f*b->alpha)&255;
					dword	nb=(ww>>1);
					m2=(m2<<24)|(m2<<16)|(m2<<8)|m2;
					__asm
					{
						pxor		mm1,mm1
						movd		mm2,m1
						movd		mm3,m2
						punpcklbw	mm2,mm1
						punpcklbw	mm3,mm1		// mm3 = 1-mix  1-mix  1-mix  1-mix 
					}
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						if(cflags&bitmapSATURATION)
						{
							if(nb)
							{
								dword	dest=(dword)d;
								dword	s0=(dword)d;
								dword	s1=(dword)s;
								__asm
								{
									mov			edi,dest
									mov			ecx,s0
									mov			edx,s1

									mov			eax,nb
								boucle000:
									dec			eax
									js			theend000

									movd		mm4,[ecx]
									add			ecx,4
									movd		mm5,[edx]
									add			edx,4

									movd		mm6,[ecx]
									add			ecx,4
									movd		mm7,[edx]
									add			edx,4

									punpcklbw	mm4,mm1
									punpcklbw	mm5,mm1
									pmullw		mm4,mm2
									pmullw		mm5,mm3

									punpcklbw	mm6,mm1
									punpcklbw	mm7,mm1
									pmullw		mm6,mm2
									pmullw		mm7,mm3

									paddusw		mm4,mm5
									paddusw		mm6,mm7

									psrlw		mm4,8
									psrlw		mm6,8

									packuswb	mm4,mm1
									packuswb	mm6,mm1

									movd		[edi],mm4
									movd		[edi+4],mm6
									add			edi,8

									jmp			boucle000

								theend000:
								}
							}
							if(ww&1)
							{
								s+=ww-1;
								d+=ww-1;
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *s);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)mini(((int)rd+(((int)rs*a1)>>8)), 255);
								gd=(byte)mini(((int)gd+(((int)gs*a1)>>8)), 255);
								bd=(byte)mini(((int)bd+(((int)bs*a1)>>8)), 255);
								*d=color32(rd, gd, bd);
							}
						}
						else
						{
							if(nb)
							{
								dword	dest=(dword)d;
								dword	s0=(dword)d;
								dword	s1=(dword)s;
								__asm
								{
									mov			edi,dest
									mov			ecx,s0
									mov			edx,s1

									mov			eax,nb
								boucle001:
									dec			eax
									js			theend001

									movd		mm4,[ecx]
									add			ecx,4
									movd		mm5,[edx]
									add			edx,4

									movd		mm6,[ecx]
									add			ecx,4
									movd		mm7,[edx]
									add			edx,4

									punpcklbw	mm4,mm1
									punpcklbw	mm5,mm1
									pmullw		mm4,mm2
									pmullw		mm5,mm3

									punpcklbw	mm6,mm1
									punpcklbw	mm7,mm1
									pmullw		mm6,mm2
									pmullw		mm7,mm3

									paddw		mm4,mm5
									paddw		mm6,mm7

									psrlw		mm4,8
									psrlw		mm6,8

									packuswb	mm4,mm1
									packuswb	mm6,mm1

									movd		[edi],mm4
									movd		[edi+4],mm6
									add			edi,8

									jmp			boucle001

								theend001:
								}
							}
							if(ww&1)
							{
								s+=ww-1;
								d+=ww-1;
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *s);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)((int)rd+(((int)rs*a1)>>8));
								gd=(byte)((int)gd+(((int)gs*a1)>>8));
								bd=(byte)((int)bd+(((int)bs*a1)>>8));
								*d=color32(rd, gd, bd);
							}
						}
					}
					__asm
					{
						emms
					}
				}
				return true;
			}
		}
		else
		{
			int		i,j;
			if(cflags&bitmapSATURATION)
			{
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[ys+j]+xs];
					dword	*d=&body32[adr[yd+j]+xd];
					dword	n4=ww>>2;
					dword	n1=ww&3;
					__asm
					{
						mov			esi,s
						mov			edi,d
						mov			ecx,n4
					bcaddmmxsat4:
						movq		mm0,[esi]
						movq		mm1,[edi]
						movq		mm2,[esi+8]
						movq		mm3,[edi+8]
						paddusb		mm1,mm0
						paddusb		mm3,mm2
						movq		[edi],mm1
						add			esi,16
						movq		[edi+8],mm3
						add			edi,16
						dec			ecx
						jnz			bcaddmmxsat4
					}
					{
						s+=(n4<<2);
						d+=(n4<<2);
						for(i=0; i<(int)n1; i++)
						{
							byte	r0,g0,b0;
							byte	r1,g1,b1;
							colorRGB(&r0, &g0, &b0, *(s++));
							colorRGB(&r1, &g1, &b1, *d);
							*(d++)=color32(mini((int)r0+(int)r1, 255), mini((int)g0+(int)g1, 255), mini((int)b0+(int)b1, 255));
						}
					}
				}
			}
			else
			{
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[ys+j]+xs];
					dword	*d=&body32[adr[yd+j]+xd];
					dword	n4=ww>>2;
					dword	n1=ww&3;
					__asm
					{
						mov			esi,s
						mov			edi,d
						mov			ecx,n4
					bcaddmmx4:
						movq		mm0,[esi]
						movq		mm1,[edi]
						movq		mm2,[esi+8]
						movq		mm3,[edi+8]
						paddb		mm1,mm0
						paddb		mm3,mm2
						movq		[edi],mm1
						add			esi,16
						movq		[edi+8],mm3
						add			edi,16
						dec			ecx
						jnz			bcaddmmx4
					}
					if(n1)
					{
						s+=(n4<<2);
						d+=(n4<<2);
						for(i=0; i<(int)n1; i++)
						{
							*d=	(((*d&0x0000ff)+(*s&0x0000ff))&0x0000ff) |
								(((*d&0x00ff00)+(*s&0x00ff00))&0x00ff00) |
								(((*d&0xff0000)+(*s&0xff0000))&0xff0000);
							s++;
							d++;
						}
					}
				}
			}
		}
		__asm
		{
			emms
		}
		return TRUE;

		case bitmapSUB:											// bitmapSUB ////////////////////////////////////////////////////////////////////////
		if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a0=(int)((1.f-b->alpha)*255.99f);
				int		a1=(int)(b->alpha*255.99f);
				int		j;
				{
					dword	m1=0xffffffff;
					dword	m2=(int)(255.99f*b->alpha)&255;
					dword	nb=(ww>>1);
					m2=(m2<<24)|(m2<<16)|(m2<<8)|m2;
					__asm
					{
						pxor		mm1,mm1
						movd		mm2,m1
						movd		mm3,m2
						punpcklbw	mm2,mm1
						punpcklbw	mm3,mm1		// mm3 = 1-mix  1-mix  1-mix  1-mix 
					}
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						if(cflags&bitmapSATURATION)
						{
							if(nb)
							{
								dword	dest=(dword)d;
								dword	s0=(dword)d;
								dword	s1=(dword)s;
								__asm
								{
									mov			edi,dest
									mov			ecx,s0
									mov			edx,s1

									mov			eax,nb
								boucle000s:
									dec			eax
									js			theend000s

									movd		mm4,[ecx]
									add			ecx,4
									movd		mm5,[edx]
									add			edx,4

									movd		mm6,[ecx]
									add			ecx,4
									movd		mm7,[edx]
									add			edx,4

									punpcklbw	mm4,mm1
									punpcklbw	mm5,mm1
									pmullw		mm4,mm2
									pmullw		mm5,mm3

									punpcklbw	mm6,mm1
									punpcklbw	mm7,mm1
									pmullw		mm6,mm2
									pmullw		mm7,mm3

									psubusw		mm4,mm5
									psubusw		mm6,mm7

									psrlw		mm4,8
									psrlw		mm6,8

									packuswb	mm4,mm1
									packuswb	mm6,mm1

									movd		[edi],mm4
									movd		[edi+4],mm6
									add			edi,8

									jmp			boucle000s

								theend000s:
								}
							}
							if(ww&1)
							{
								s+=ww-1;
								d+=ww-1;
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *s);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)maxi(((int)rd-(((int)rs*a1)>>8)), 0);
								gd=(byte)maxi(((int)gd-(((int)gs*a1)>>8)), 0);
								bd=(byte)maxi(((int)bd-(((int)bs*a1)>>8)), 0);
								*d=color32(rd, gd, bd);
							}
						}
						else
						{
							if(nb)
							{
								dword	dest=(dword)d;
								dword	s0=(dword)d;
								dword	s1=(dword)s;
								__asm
								{
									mov			edi,dest
									mov			ecx,s0
									mov			edx,s1

									mov			eax,nb
								boucle001s:
									dec			eax
									js			theend001s

									movd		mm4,[ecx]
									add			ecx,4
									movd		mm5,[edx]
									add			edx,4

									movd		mm6,[ecx]
									add			ecx,4
									movd		mm7,[edx]
									add			edx,4

									punpcklbw	mm4,mm1
									punpcklbw	mm5,mm1
									pmullw		mm4,mm2
									pmullw		mm5,mm3

									punpcklbw	mm6,mm1
									punpcklbw	mm7,mm1
									pmullw		mm6,mm2
									pmullw		mm7,mm3

									psubw		mm4,mm5
									psubw		mm6,mm7

									psrlw		mm4,8
									psrlw		mm6,8

									packuswb	mm4,mm1
									packuswb	mm6,mm1

									movd		[edi],mm4
									movd		[edi+4],mm6
									add			edi,8

									jmp			boucle001s

								theend001s:
								}
							}
							if(ww&1)
							{
								s+=ww-1;
								d+=ww-1;
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *s);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)((int)rd-(((int)rs*a1)>>8));
								gd=(byte)((int)gd-(((int)gs*a1)>>8));
								bd=(byte)((int)bd-(((int)bs*a1)>>8));
								*d=color32(rd, gd, bd);
							}
						}
					}
				}
				__asm
				{
					emms
				}
			}
		}
		else
		{
			int		i,j;
			if(cflags&bitmapSATURATION)
			{
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[ys+j]+xs];
					dword	*d=&body32[adr[yd+j]+xd];
					dword	n4=ww>>2;
					dword	n1=ww&3;
					__asm
					{
						mov			esi,s
						mov			edi,d
						mov			ecx,n4
					bcsubmmxsat4:
						movq		mm0,[esi]
						movq		mm1,[edi]
						movq		mm2,[esi+8]
						movq		mm3,[edi+8]
						psubusb		mm1,mm0
						psubusb		mm3,mm2
						movq		[edi],mm1
						add			esi,16
						movq		[edi+8],mm3
						add			edi,16
						dec			ecx
						jnz			bcsubmmxsat4
					}
					{
						s+=(n4<<2);
						d+=(n4<<2);
						for(i=0; i<(int)n1; i++)
						{
							byte	r0,g0,b0;
							byte	r1,g1,b1;
							colorRGB(&r0, &g0, &b0, *(s++));
							colorRGB(&r1, &g1, &b1, *d);
							*(d++)=color32(maxi((int)r0-(int)r1, 0), maxi((int)g0-(int)g1, 0), maxi((int)b0-(int)b1, 0));
						}
					}
				}
				__asm
				{
					emms
				}
			}
			else
			{
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[ys+j]+xs];
					dword	*d=&body32[adr[yd+j]+xd];
					dword	n4=ww>>2;
					dword	n1=ww&3;
					__asm
					{
						mov			esi,s
						mov			edi,d
						mov			ecx,n4
					bcsubmmx4:
						movq		mm0,[esi]
						movq		mm1,[edi]
						movq		mm2,[esi+8]
						movq		mm3,[edi+8]
						psubb		mm1,mm0
						psubb		mm3,mm2
						movq		[edi],mm1
						add			esi,16
						movq		[edi+8],mm3
						add			edi,16
						dec			ecx
						jnz			bcsubmmx4
					}
					if(n1)
					{
						s+=(n4<<2);
						d+=(n4<<2);
						for(i=0; i<(int)n1; i++)
						{
							*d=	(((*d&0x0000ff)-(*s&0x0000ff))&0x0000ff) |
								(((*d&0x00ff00)-(*s&0x00ff00))&0x00ff00) |
								(((*d&0xff0000)-(*s&0xff0000))&0xff0000);
							s++;
							d++;
						}
					}
				}
				__asm
				{
					emms
				}
			}
		}
		__asm
		{
			emms
		}
		return TRUE;

		default:
		Alasterror="unknown bitmap state.";
		return FALSE;
	}
	Alasterror="incompatible depth resolution or bitmap state.";
#endif
	__asm
	{
		emms
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::set(int xd, int yd, int xs, int ys, int w, int h, class Abitmap *b, int action, int flags)
{
	int	cstate=(action==bitmapDEFAULT)?b->state:action;
	int	cflags=(flags==bitmapDEFAULT)?b->flags:flags;

	if((ys>b->h)||(xs>b->w)||(xs+b->w<0)||(ys+b->h<0))
		return false;

#ifdef MMX
	if((nbBits==32)&&(b->nbBits==32)&&(cstate!=bitmapMUL)&&(!(((cstate==bitmapADD)||(cstate==bitmapSUB))&&(!(cflags&bitmapSATURATION)))))
	{
		if(!((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA)))
			return NCsetMMX(xd, yd, xs, ys, w, h, b, cstate, cflags);
	}
#endif

	xd+=view.x;
	yd+=view.y;

	if(xd<clip.x)
	{
		xs-=(xd-clip.x);
		w+=(xd-clip.x);
		xd=clip.x;
	}
	if(yd<clip.y)
	{
		ys-=(yd-clip.y);
		h+=(yd-clip.y);
		yd=clip.y;
	}
	if((w<=0)||(h<=0)||(xd>=(clip.x+clip.w))||(yd>=(clip.y+clip.h)))
	{
		Alasterror="outside region.";
		return FALSE;
	}

	int		ww=mini(mini(w, b->w), (clip.x+clip.w)-xd);
	int		hh=mini(mini(h, b->h), (clip.y+clip.h)-yd);

	switch(cstate)
	{
		case bitmapNORMAL:
		if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
		{
			int		i,j;
			int		alphai=(int)(b->alpha*256.f);
			switch(nbBits)
			{
				case 16:
				switch(b->nbBits)
				{
					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs,as;
							byte	rd,gd,bd,ad;
							colorRGBA(&rs, &gs, &bs, &as, *(s++));
							as=(byte)(((int)as*(int)alphai)>>8);
							colorRGB(&rd, &gd, &bd, *d);
							ad=255-as;
							rd=(byte)(((int)rd*ad+(int)rs*as)>>8);
							gd=(byte)(((int)gd*ad+(int)gs*as)>>8);
							bd=(byte)(((int)bd*ad+(int)bs*as)>>8);
							*(d++)=color16(rd, gd, bd);
						}
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs,as,asi;
							byte	rd,gd,bd,ad;
							colorRGBA(&rs, &gs, &bs, &as, *(s++));
							as=(byte)(((int)as*(int)alphai)>>8);
							colorRGBA(&rd, &gd, &bd, &ad, *d);
							asi=255-as;
							rd=(byte)(((int)rd*asi+(int)rs*as)>>8);
							gd=(byte)(((int)gd*asi+(int)gs*as)>>8);
							bd=(byte)(((int)bd*asi+(int)bs*as)>>8);
							ad=(byte)(((int)ad*asi+(int)as*as)>>8);
							*(d++)=color32(rd, gd, bd, ad);
						}
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		else if(cflags&bitmapALPHA)								// bitmapNORMAL & bitmapALPHA ///////////////////////////////////////////////////////
		{
			int		i,j;
			switch(nbBits)
			{
				case 16:
				switch(b->nbBits)
				{
					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs,as;
							byte	rd,gd,bd,ad;
							colorRGBA(&rs, &gs, &bs, &as, *(s++));
							colorRGB(&rd, &gd, &bd, *d);
							ad=255-as;
							rd=(byte)(((int)rd*ad+(int)rs*as)>>8);
							gd=(byte)(((int)gd*ad+(int)gs*as)>>8);
							bd=(byte)(((int)bd*ad+(int)bs*as)>>8);
							*(d++)=color16(rd, gd, bd);
						}
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs,as,asi;
							byte	rd,gd,bd,ad;
							colorRGBA(&rs, &gs, &bs, &as, *(s++));
							colorRGBA(&rd, &gd, &bd, &ad, *d);
							asi=255-as;
							rd=(byte)(((int)rd*asi+(int)rs*as)>>8);
							gd=(byte)(((int)gd*asi+(int)gs*as)>>8);
							bd=(byte)(((int)bd*asi+(int)bs*as)>>8);
							ad=(byte)(((int)ad*asi+(int)as*as)>>8);
							*(d++)=color32(rd, gd, bd, ad);
						}
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		else if(cflags&bitmapCOLORKEY)						// bitmapNORMAL & bitmapCOLORKEY ////////////////////////////////////////////////////
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						byte	*d=&body8[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							if(*s!=b->colorkey)
								*d=*s;
							s++;
							d++;
						}
					}
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							if(*s!=b->colorkey)
								*d=*s;
							s++;
							d++;
						}
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							if(*s!=b->colorkey)
								*d=color32to16(*s);
							s++;
							d++;
						}
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							if(*s!=b->colorkey)
								*d=b->palette[*s];
							s++;
							d++;
						}
					}
					return TRUE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							if(*s!=b->colorkey)
								*d=color16to32(*s);
							s++;
							d++;
						}
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							if(*s!=b->colorkey)
								*d=*s;
							s++;
							d++;
						}
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		else if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a0=(int)((1.f-b->alpha)*255.99f);
				int		a1=(int)(b->alpha*255.99f);
				int		i,j;
				switch(nbBits)
				{
					case 8:
					switch(b->nbBits)
					{
						case 8:
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[ys+j]+xs];
							byte	*d=&body8[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
								*d=(*s*a1+*d*a0)>>8;
						}
						return TRUE;

						case 16:
						Alasterror="incompatible depth resolution between the two pictures.";
						return FALSE;

						case 32:
						Alasterror="incompatible depth resolution between the two pictures.";
						return FALSE;
					}
					break;

					case 16:
					switch(b->nbBits)
					{
						case 8:
						Alasterror="incompatible depth resolution between the two pictures.";
						return FALSE;

						case 16:
						for(j=0; j<hh; j++)
						{
							word	*s=&b->body16[b->adr[ys+j]+xs];
							word	*d=&body16[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *(s++));
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*(d++)=color16(rd, gd, bd);
							}
						}
						return TRUE;

						case 32:
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							word	*d=&body16[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *(s++));
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*(d++)=color16(rd, gd, bd);
							}
						}
						return TRUE;
					}
					break;

					case 32:
					switch(b->nbBits)
					{
						case 8:
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, b->palette[*(s++)]);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*(d++)=color32(rd, gd, bd);
							}
						}
						return TRUE;

						case 16:
						for(j=0; j<hh; j++)
						{
							word	*s=&b->body16[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *(s++));
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*(d++)=color32(rd, gd, bd);
							}
						}
						return TRUE;

						case 32:
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	rs,gs,bs;
									byte	rd,gd,bd;
									colorRGB(&rs, &gs, &bs, *(s++));
									colorRGB(&rd, &gd, &bd, *d);
									rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
									gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
									bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
									*(d++)=color32(rd, gd, bd);
								}
							}
						}
						return TRUE;
					}
					break;
				}
				break;
			}
		}
		else													// bitmapNORMAL /////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					for(i=0; i<hh; i++)
						memcpy(&body8[adr[yd+i]+xd], &b->body8[adr[ys+i]+xs], ww);
					memcpy(palette, b->palette, sizeof(dword)*256);
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					for(i=0; i<hh; i++)
						memcpy(&body16[adr[yd+i]+xd], &b->body16[b->adr[ys+i]+xs], ww<<1);
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)=color32to16(*(s++));
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)=b->palette[*(s++)];
					}
					return TRUE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)=color16to32(*(s++));
					}
					return TRUE;

					case 32:
					for(i=0; i<hh; i++)
						memcpy(&body32[adr[yd+i]+xd], &b->body32[b->adr[ys+i]+xs], ww<<2);
					return TRUE;
				}
				break;
			}
			break;
		}
		break;

		case bitmapOR:											// bitmapOR /////////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						byte	*d=&body8[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)|=*(s++);
					}
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)|=*(s++);
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)|=color32to16(*(s++));
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)|=b->palette[*(s++)];
					}
					return TRUE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)|=color16to32(*(s++));
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)|=*(s++);
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		break;

		case bitmapXOR:											// bitmapXOR ////////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						byte	*d=&body8[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)^=*(s++);
					}
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)^=*(s++);
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)^=color32to16(*(s++));
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)^=b->palette[*(s++)];
					}
					return TRUE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)^=color16to32(*(s++));
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)^=*(s++);
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		break;

		case bitmapAND:											// bitmapAND ////////////////////////////////////////////////////////////////////////
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						byte	*d=&body8[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)&=*(s++);
					}
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)&=*(s++);
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)&=color32to16(*(s++));
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)&=b->palette[*(s++)];
					}
					return TRUE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)&=color16to32(*(s++));
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)&=*(s++);
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		break;

		case bitmapADD:											// bitmapADD ////////////////////////////////////////////////////////////////////////
		if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				//int		a0=(int)((1.f-b->alpha)*256.f);
				int		a1=(int)(b->alpha*256.f);
				int		i,j;

				switch(nbBits)
				{
					case 32:
					switch(b->nbBits)
					{
						case 32:
						if(cflags&bitmapSATURATION)
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0,aa;
									byte	r1,g1,b1;
									colorRGBA(&r0, &g0, &b0, &aa, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									aa=(byte)(((int)aa*(int)a1)>>8);
									*(d++)=color32(mini((int)r1+(((int)r0*aa)>>8), 255), mini((int)g1+(((int)g0*aa)>>8), 255), mini((int)b1+(((int)b0*aa)>>8), 255));
								}
							}
						}
						else
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0,aa;
									byte	r1,g1,b1;
									colorRGBA(&r0, &g0, &b0, &aa, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									aa=(byte)(((int)aa*(int)a1)>>8);
									*(d++)=revers32((int)r1+(((int)r0*aa)>>8), (int)g1+(((int)g0*aa)>>8), (int)b1+(((int)b0*aa)>>8));
								}
							}
						}
						return TRUE;
					}
				}
				break;
			}
		}
		else if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a0=(int)((1.f-b->alpha)*255.99f);
				int		a1=(int)(b->alpha*255.99f);
				int		i,j;

				switch(nbBits)
				{
					case 32:
					switch(b->nbBits)
					{
						case 32:
						if(cflags&bitmapSATURATION)
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0;
									byte	r1,g1,b1;
									colorRGB(&r0, &g0, &b0, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									*(d++)=color32(mini((int)r1+(((int)r0*a1)>>8), 255), mini((int)g1+(((int)g0*a1)>>8), 255), mini((int)b1+(((int)b0*a1)>>8), 255));
								}
							}
						}
						else
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0;
									byte	r1,g1,b1;
									colorRGB(&r0, &g0, &b0, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									*(d++)=revers32((int)r1+(((int)r0*a1)>>8), (int)g1+(((int)g0*a1)>>8), (int)b1+(((int)b0*a1)>>8));
								}
							}
						}
						return TRUE;
					}
				}
				break;
			}
		}
		else
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[ys+j]+xs];
							byte	*d=&body8[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								*(d)=(byte)mini(*d+*s, 255);
								d++;
								s++;
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[ys+j]+xs];
							byte	*d=&body8[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
								*(d++)+=*(s++);
						}
					}
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							word	*s=&b->body16[b->adr[ys+j]+xs];
							word	*d=&body16[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *s);
								colorRGB(&r1, &g1, &b1, *d);
								*d=color16(mini((int)r0+(int)r1, 255), mini((int)g0+(int)g1, 255), mini((int)b0+(int)b1, 255));
								s++;
								d++;
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							word	*s=&b->body16[b->adr[ys+j]+xs];
							word	*d=&body16[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								*d=(((*d&0x001f)+(*s&0x001f))&0x001f)|(((*d&0x03e0)+(*s&0x03e0))&0x03e0)|(((*d&0x7c00)+(*s&0x7c00))&0x7c00);
								s++;
								d++;
							}
						}
					}
					return TRUE;

					case 32:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							word	*d=&body16[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *s);
								colorRGB(&r1, &g1, &b1, *d);
								*d=color16(mini((int)r0+(int)r1, 255), mini((int)g0+(int)g1, 255), mini((int)b0+(int)b1, 255));
								s++;
								d++;
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							word	*d=&body16[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								*d=	(word) (
									((((*d&0x0000ff)>>3)+(*s&0x001f))&0x001f) |
									((((*d&0x00ff00)>>6)+(*s&0x03e0))&0x03e0) |
									((((*d&0xff0000)>>9)+(*s&0x7c00))&0x7c00));
								s++;
								d++;
							}
						}
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								dword	v=b->palette[*(s++)];
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, v);
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=color32(mini((int)r0+(int)r1, 255), mini((int)g0+(int)g1, 255), mini((int)b0+(int)b1, 255));
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								dword	v=b->palette[*(s++)];
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, v);
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=revers32((int)r0+(int)r1, (int)g0+(int)g1, (int)b0+(int)b1);
							}
						}
					}
					return TRUE;

					case 16:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							word	*s=&b->body16[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *(s++));
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=color32(mini((int)r0+(int)r1, 255), mini((int)g0+(int)g1, 255), mini((int)b0+(int)b1, 255));
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							word	*s=&b->body16[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *(s++));
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=revers32((int)r0+(int)r1, (int)g0+(int)g1, (int)b0+(int)b1);
							}
						}
					}
					return TRUE;

					case 32:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *(s++));
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=color32(mini((int)r0+(int)r1, 255), mini((int)g0+(int)g1, 255), mini((int)b0+(int)b1, 255));
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *(s++));
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=revers32((int)r0+(int)r1, (int)g0+(int)g1, (int)b0+(int)b1);
							}
						}
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		break;

		case bitmapSUB:											// bitmapSUB ////////////////////////////////////////////////////////////////////////
		if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				//int		a0=(int)((1.f-b->alpha)*256.f);
				int		a1=(int)(b->alpha*256.f);
				int		i,j;

				switch(nbBits)
				{
					case 32:
					switch(b->nbBits)
					{
						case 32:
						if(cflags&bitmapSATURATION)
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0,aa;
									byte	r1,g1,b1;
									colorRGBA(&r0, &g0, &b0, &aa, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									aa=(byte)(((int)aa*(int)a1)>>8);
									*(d++)=color32(maxi((int)r1-(((int)r0*aa)>>8), 0), maxi((int)g1-(((int)g0*aa)>>8), 0), maxi((int)b1-(((int)b0*aa)>>8), 0));
								}
							}
						}
						else
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0,aa;
									byte	r1,g1,b1;
									colorRGBA(&r0, &g0, &b0, &aa, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									aa=(byte)(((int)aa*(int)a1)>>8);
									*(d++)=revers32((int)r1-(((int)r0*aa)>>8), (int)g1-(((int)g0*aa)>>8), (int)b1-(((int)b0*aa)>>8));
								}
							}
						}
						return TRUE;
					}
				}
				break;
			}
		}
		else if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a0=(int)((1.f-b->alpha)*256.f);
				int		a1=(int)(b->alpha*256.f);
				int		i,j;

				switch(nbBits)
				{
					case 32:
					switch(b->nbBits)
					{
						case 32:
						if(cflags&bitmapSATURATION)
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0;
									byte	r1,g1,b1;
									colorRGB(&r0, &g0, &b0, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									*(d++)=color32(maxi((int)r1-(((int)r0*a1)>>8), 0), maxi((int)g1-(((int)g0*a1)>>8), 0), maxi((int)b1-(((int)b0*a1)>>8), 0));
								}
							}
						}
						else
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	r0,g0,b0;
									byte	r1,g1,b1;
									colorRGB(&r0, &g0, &b0, *(s++));
									colorRGB(&r1, &g1, &b1, *d);
									*(d++)=revers32((int)r1-(((int)r0*a1)>>8), (int)g1-(((int)g0*a1)>>8), (int)b1-(((int)b0*a1)>>8));
								}
							}
						}
						return TRUE;
					}
				}
				break;
			}
		}
		else
		{
			int		i,j;
			switch(nbBits)
			{
				case 8:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						byte	*d=&body8[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
							*(d++)-=*(s++);
					}
					return TRUE;

					case 16:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 32:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;
				}
				break;

				case 16:
				switch(b->nbBits)
				{
					case 8:
					Alasterror="incompatible depth resolution between the two pictures.";
					return FALSE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							*d=(((*d&0x001f)-(*s&0x001f))&0x001f)|(((*d&0x03e0)-(*s&0x03e0))&0x03e0)|(((*d&0x7c00)-(*s&0x7c00))&0x7c00);
							s++;
							d++;
						}
					}
					return TRUE;

					case 32:
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						word	*d=&body16[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							*d=	(word) (
								((((*d&0x0000ff)>>3)-(*s&0x001f))&0x001f) |
								((((*d&0x00ff00)>>6)-(*s&0x03e0))&0x03e0) |
								((((*d&0xff0000)>>9)-(*s&0x7c00))&0x7c00));
							s++;
							d++;
						}
					}
					return TRUE;
				}
				break;

				case 32:
				switch(b->nbBits)
				{
					case 8:
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							dword	v=b->palette[*(s++)];
							*d=	(((*d&0x0000ff)-(v&0x0000ff))&0x0000ff) |
								(((*d&0x00ff00)-(v&0x00ff00))&0x00ff00) |
								(((*d&0xff0000)-(v&0xff0000))&0xff0000);
							d++;
						}
					}
					return TRUE;

					case 16:
					for(j=0; j<hh; j++)
					{
						word	*s=&b->body16[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						for(i=0; i<ww; i++)
						{
							*d=	((*d&0x0000ff)-((*s&0x001f)<<3)&0x0000ff) |
								((*d&0x00ff00)-((*s&0x03e0)<<6)&0x00ff00) |
								((*d&0xff0000)-((*s&0x7c00)<<9)&0xff0000);
							s++;
							d++;
						}
					}
					return TRUE;

					case 32:
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *(s++));
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=color32(maxi((int)r1-(int)r0, 0), maxi((int)g1-(int)g0, 0), maxi((int)b0-(int)b0, 0));
							}
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							dword	*s=&b->body32[b->adr[ys+j]+xs];
							dword	*d=&body32[adr[yd+j]+xd];
							for(i=0; i<ww; i++)
							{
								byte	r0,g0,b0;
								byte	r1,g1,b1;
								colorRGB(&r0, &g0, &b0, *(s++));
								colorRGB(&r1, &g1, &b1, *d);
								*(d++)=revers32((int)r1-(int)r0, (int)g1-(int)g0, (int)b1-(int)b0);
							}
						}
					}
					return TRUE;
				}
				break;
			}
			break;
		}
		break;

		case bitmapMUL:
		if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a10=(int)(b->alpha*255.99f);
				//int		a00=(int)((255.99f-a1)*255.99f);
				int		i,j;

				switch(nbBits)
				{
					case 32:
					switch(b->nbBits)
					{
						case 32:
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	rs,gs,bs,aa;
									byte	rd,gd,bd;
									colorRGBA(&rs, &gs, &bs, &aa, *(s++));
									colorRGB(&rd, &gd, &bd, *d);
									int		a1=((int)aa*a10)>>8;
									int		a0=65536-(a1<<8);  
									*(d++)=color32( (((int)rs*a1+a0)*(int)rd)>>16, (((int)gs*a1+a0)*(int)gd)>>16, (((int)bs*a1+a0)*(int)bd)>>16 );
								}
							}
						}
						return TRUE;
					}
				}
				break;
			}
		}
		else if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a1=(int)(b->alpha*255.99f);
				int		a0=(int)((255.99f-a1)*255.99f);
				int		i,j;

				switch(nbBits)
				{
					case 32:
					switch(b->nbBits)
					{
						case 8:
						{
							for(j=0; j<hh; j++)
							{
								byte	*s=&b->body8[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	rs,gs,bs;
									byte	rd,gd,bd;
									colorRGB(&rs, &gs, &bs, b->palette[*(s++)]);
									colorRGB(&rd, &gd, &bd, *d);
									*(d++)=color32( (((int)rs*a1+a0)*(int)rd)>>16, (((int)gs*a1+a0)*(int)gd)>>16, (((int)bs*a1+a0)*(int)bd)>>16 );
								}
							}
						}
						return TRUE;

						case 32:
						{
							for(j=0; j<hh; j++)
							{
								dword	*s=&b->body32[b->adr[ys+j]+xs];
								dword	*d=&body32[adr[yd+j]+xd];
								for(i=0; i<ww; i++)
								{
									byte	rs,gs,bs;
									byte	rd,gd,bd;
									colorRGB(&rs, &gs, &bs, *(s++));
									colorRGB(&rd, &gd, &bd, *d);
									*(d++)=color32( (((int)rs*a1+a0)*(int)rd)>>16, (((int)gs*a1+a0)*(int)gd)>>16, (((int)bs*a1+a0)*(int)bd)>>16 );
								}
							}
						}
						return TRUE;
					}
				}
				break;
			}
		}
		else
		{
			Alasterror="incompatible depth resolution between the two pictures.";
			return FALSE;
		}
		break;

		default:
		Alasterror="unknown bitmap state.";
		return FALSE;
	}
	Alasterror="incompatible depth resolution or bitmap state.";
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCsetMMX(int xd, int yd, int wd, int hd, int xs, int ys, int ws, int hs, class Abitmap *b, int action, int flags)
{
#ifdef MMX
	int	cstate=(action==bitmapDEFAULT)?b->state:action;
	int	cflags=(flags==bitmapDEFAULT)?b->flags:flags;

	if((wd<=0)||(hd<=0))
	{
		Alasterror="invalid width or height source.";
		return FALSE;
	}

	if((xs<0)||(ys<0))
	{
		Alasterror="invalid source region.";
		return FALSE;
	}

	xd+=view.x;
	yd+=view.y;

	ws=maxi(0, mini(ws, b->w-xs));
	hs=maxi(0, mini(hs, b->h-ys));

	xs<<=16;
	ys<<=16;
	ws<<=16;
	hs<<=16;

	int	dw=ws/wd;
	int	dh=hs/hd;

	if(xd<clip.x)
	{
		int	d=(xd-clip.x)*dw;
		xs-=d;
		ws+=d;
		xd=clip.x;
	}
	if(yd<clip.y)
	{
		int	d=(yd-clip.y)*dh;
		ys-=d;
		hs+=d;
		yd=clip.y;
	}
	if((ws<=0)||(hs<=0)||(xd>=(clip.x+clip.w))||(yd>=(clip.y+clip.h)))
	{
		Alasterror="outside region.";
		return FALSE;
	}

	int		ww=mini(mini(ws/dw, w), (clip.x+clip.w)-xd);
	int		hh=mini(mini(hs/dh, h), (clip.y+clip.h)-yd);

	if((ww<=0)||(hh<0))
	{
		Alasterror="outside region.";
		return FALSE;
	}

	switch(cstate)
	{
		case bitmapNORMAL:
		if(cflags&bitmapALPHA)
		{
			dword	y=ys;
			int		j;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];

				{
					qword	v255=QWORD(0x00ff00ff, 0x00ff00ff);
					dword	dest=(dword)d;
					dword	s0=(dword)s;
					dword	nb=ww;
					__asm
					{
						pxor		mm1,mm1
						mov			edi,dest
						mov			esi,s0
						mov			edx,xs

						mov			eax,nb
					boucleBNA:
						dec			eax
						js			theendBNA

						mov			ecx,edx
						movd		mm5,[edi]
						add			edx,dw
						shr			ecx,16
						movzx		ebx,byte ptr [esi+ecx*4+3]
						movq		mm3,v255
						movq		mm2,mulmmx[ebx*8];
						movd		mm4,[esi+ecx*4]
						psubusb		mm3,mm2

						punpcklbw	mm4,mm1
						punpcklbw	mm5,mm1
						pmullw		mm4,mm2
						pmullw		mm5,mm3

						paddw		mm4,mm5
						psrlw		mm4,8
						packuswb	mm4,mm1

						movd		[edi],mm4
						add			edi,4

						jmp			boucleBNA

					theendBNA:
					}
				}
				y+=dh;
			}
			__asm
			{
				emms
			}
		}
		else if(cflags&bitmapCOLORKEY)
		{
			dword	y=ys;
			int		j;
			for(j=0; j<hh; j++)
			{
				dword	x=xs;
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];
				int		i;
				for(i=0; i<ww; i++)
				{
					dword	v=s[x>>16];
					if(v!=b->colorkey)
						*d=v;
					d++;
					x+=dw;
				}
				y+=dh;
			}
		}
		else if(cflags&bitmapGLOBALALPHA)
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			{
				int		a0=(int)((1.f-b->alpha)*255.99f);
				int		a1=(int)(b->alpha*255.99f);
				qword	mul0=(a0<<16)|a0;
				qword	mul1=(a1<<16)|a1;
				dword	y=ys;
				int		j;

				mul0=QWORD(mul0, mul0);
				mul1=QWORD(mul1, mul1);

				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];

					{
						dword	dest=(dword)d;
						dword	s0=(dword)s;
						dword	nb=ww;
						__asm
						{
							pxor		mm1,mm1
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs
							movq		mm2,mul1
							movq		mm3,mul0

							mov			eax,nb
						boucleBNGA:
							dec			eax
							js			theendBNGA

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1
							pmullw		mm4,mm2
							pmullw		mm5,mm3

							paddusw		mm4,mm5
							psrlw		mm4,8
							packuswb	mm4,mm1

							movd		[edi],mm4
							add			edi,4

							jmp			boucleBNGA

						theendBNGA:
						}
					}
					y+=dh;
				}
				__asm
				{
					emms
				}
			}
		}
		else
		{
			int		j;
			dword	y=ys;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];
				int		i;
				dword	x=xs;
				for(i=0; i<ww; i++)
				{
					*(d++)=s[x>>16];
					x+=dw;
				}
				y+=dh;
			}
		}
		return TRUE;

		case bitmapOR:
		{
			int		j;
			dword	y=ys;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];
				int		i;
				dword	x=xs;
				for(i=0; i<ww; i++)
				{
					*(d++)|=s[x>>16];
					x+=dw;
				}
				y+=dh;
			}
		}
		return TRUE;

		case bitmapXOR:
		{
			int		j;
			dword	y=ys;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];
				int		i;
				dword	x=xs;
				for(i=0; i<ww; i++)
				{
					*(d++)^=s[x>>16];
					x+=dw;
				}
				y+=dh;
			}
		}
		return TRUE;

		case bitmapAND:
		{
			int		j;
			dword	y=ys;
			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];
				int		i;
				dword	x=xs;
				for(i=0; i<ww; i++)
				{
					*(d++)&=s[x>>16];
					x+=dw;
				}
				y+=dh;
			}
		}
		return TRUE;

		case bitmapADD:
		if(cflags&bitmapGLOBALALPHA)
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			int		a0=(int)(255.99f);
			int		a1=(int)(b->alpha*255.99f);
			qword	mul0=(a0<<16)|a0;
			qword	mul1=(a1<<16)|a1;
			dword	y=ys;
			int		j;

			mul0=QWORD(mul0, mul0);
			mul1=QWORD(mul1, mul1);

			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];

				{
					dword	dest=(dword)d;
					dword	s0=(dword)s;
					dword	nb=ww;
					if(cflags&bitmapSATURATION)
					{
						__asm
						{
							pxor		mm1,mm1
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs
							movq		mm2,mul1
							movq		mm3,mul0

							mov			eax,nb
						boucleBNAGAS:
							dec			eax
							js			theendBNAGAS

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1
							pmullw		mm4,mm2
							pmullw		mm5,mm3

							paddusw		mm4,mm5
							psrlw		mm4,8
							packuswb	mm4,mm1

							movd		[edi],mm4
							add			edi,4

							jmp			boucleBNAGAS

						theendBNAGAS:
						}
					}
					else	// no saturation // bitmap ADD
					{
						__asm
						{
							pxor		mm1,mm1
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs
							movq		mm2,mul1
							movq		mm3,mul0

							mov			eax,nb
						boucleBNAGA:
							dec			eax
							js			theendBNAGA

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1
							pmullw		mm4,mm2
							pmullw		mm5,mm3

							paddw		mm4,mm5
							psrlw		mm4,8
							packuswb	mm4,mm1

							movd		[edi],mm4
							add			edi,4

							jmp			boucleBNAGA

						theendBNAGA:
						}
					}
				}
				y+=dh;
			}
			__asm
			{
				emms
			}
		}
		else	// no global alpha
		{
			dword	y=ys;
			int		j;

			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];

				{
					dword	dest=(dword)d;
					dword	s0=(dword)s;
					dword	nb=ww;
					if(cflags&bitmapSATURATION)
					{
						__asm
						{
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs

							mov			eax,nb
						boucleBNADS:
							dec			eax
							js			theendBNADS

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]
							paddusb		mm4,mm5

							movd		[edi],mm4
							add			edi,4

							jmp			boucleBNADS

						theendBNADS:
						}
					}
					else	// no saturation
					{
						__asm
						{
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs

							mov			eax,nb
						boucleBNAD:
							dec			eax
							js			theendBNAD

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]
							paddb		mm4,mm5

							movd		[edi],mm4
							add			edi,4

							jmp			boucleBNAD

						theendBNAD:
						}
					}
				}
				y+=dh;
			}
			__asm
			{
				emms
			}
		}
		return TRUE;

		case bitmapSUB:
		if(cflags&bitmapGLOBALALPHA)
		{
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			int		a0=(int)(255.99f);
			int		a1=(int)(b->alpha*255.99f);
			qword	mul0=(a0<<16)|a0;
			qword	mul1=(a1<<16)|a1;
			dword	y=ys;
			int		j;

			mul0=QWORD(mul0, mul0);
			mul1=QWORD(mul1, mul1);

			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];

				{
					dword	dest=(dword)d;
					dword	s0=(dword)s;
					dword	nb=ww;
					if(cflags&bitmapSATURATION)
					{
						__asm
						{
							pxor		mm1,mm1
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs
							movq		mm2,mul1
							movq		mm3,mul0

							mov			eax,nb
						boucleBNSGAS:
							dec			eax
							js			theendBNSGAS

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1
							pmullw		mm4,mm2
							pmullw		mm5,mm3

							psubusw		mm5,mm4
							psrlw		mm5,8
							packuswb	mm5,mm1

							movd		[edi],mm5
							add			edi,4

							jmp			boucleBNSGAS

						theendBNSGAS:
						}
					}
					else	// no saturation
					{
						__asm
						{
							pxor		mm1,mm1
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs
							movq		mm2,mul1
							movq		mm3,mul0

							mov			eax,nb
						boucleBNSGA:
							dec			eax
							js			theendBNSGA

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]

							punpcklbw	mm4,mm1
							punpcklbw	mm5,mm1
							pmullw		mm4,mm2
							pmullw		mm5,mm3

							psubw		mm5,mm4
							psrlw		mm5,8
							packuswb	mm5,mm1

							movd		[edi],mm5
							add			edi,4

							jmp			boucleBNSGA

						theendBNSGA:
						}
					}
				}
				y+=dh;
			}
			__asm
			{
				emms
			}
		}
		else	// no global alpha
		{
			dword	y=ys;
			int		j;

			for(j=0; j<hh; j++)
			{
				dword	*s=&b->body32[b->adr[y>>16]];
				dword	*d=&body32[adr[yd+j]+xd];

				{
					dword	dest=(dword)d;
					dword	s0=(dword)s;
					dword	nb=ww;
					if(cflags&bitmapSATURATION)
					{
						__asm
						{
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs

							mov			eax,nb
						boucleBNSDS:
							dec			eax
							js			theendBNSDS

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]
							psubusb		mm5,mm4

							movd		[edi],mm5
							add			edi,4

							jmp			boucleBNSDS

						theendBNSDS:
						}
					}
					else	// no saturation
					{
						__asm
						{
							mov			edi,dest
							mov			esi,s0
							mov			edx,xs

							mov			eax,nb
						boucleBNSD:
							dec			eax
							js			theendBNSD

							mov			ecx,edx
							movd		mm5,[edi]
							add			edx,dw
							shr			ecx,16
							movd		mm4,[esi+ecx*4]
							psubb		mm5,mm4

							movd		[edi],mm5
							add			edi,4

							jmp			boucleBNSD

						theendBNSD:
						}
					}
				}
				y+=dh;
			}
			__asm
			{
				emms
			}
		}
		return TRUE;
	}
#endif
	Alasterror="incompatible depth resolution or bitmap state. (not implemented)";
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::set(int xd, int yd, int wd, int hd, int xs, int ys, int ws, int hs, class Abitmap *b, int action, int flags)
{
	int	cstate=(action==bitmapDEFAULT)?b->state:action;
	int	cflags=(flags==bitmapDEFAULT)?b->flags:flags;

	if(!body||!b->body)
		return false;

	if((wd==ws)&&(hs==hd))
		return set(xd, yd, xs, ys, ws, hs, b, cstate, cflags);

#ifdef MMX
	if((nbBits==32)&&(b->nbBits==32)&&(cstate!=bitmapMUL)&&(!(((cstate==bitmapADD)||(cstate==bitmapSUB))&&(!(cflags&bitmapSATURATION)))))
		if(!((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA)))
			return NCsetMMX(xd, yd, wd, hd, xs, ys, ws, hs, b, cstate, cflags);
#endif

	xd+=view.x;
	yd+=view.y;

	if((wd<=0)||(hd<=0))
	{
		Alasterror="invalid width or height source.";
		return FALSE;
	}

	if((xs<0)||(ys<0))
	{
		Alasterror="invalid source region.";
		return FALSE;
	}

	ws=maxi(0, mini(ws, b->w-xs));
	hs=maxi(0, mini(hs, b->h-ys));

	xs<<=16;
	ys<<=16;
	ws<<=16;
	hs<<=16;

	int	dw=ws/wd;
	int	dh=hs/hd;

	if(xd<clip.x)
	{
		int	d=(xd-clip.x)*dw;
		xs-=d;
		ws+=d;
		xd=clip.x;
	}
	if(yd<clip.y)
	{
		int	d=(yd-clip.y)*dh;
		ys-=d;
		hs+=d;
		yd=clip.y;
	}
	if((ws<=0)||(hs<=0)||(xd>=(clip.x+clip.w))||(yd>=(clip.y+clip.h)))
	{
		Alasterror="outside region.";
		return FALSE;
	}

	int		ww=mini(mini(ws/dw, w), (clip.x+clip.w)-xd);
	int		hh=mini(mini(hs/dh, h), (clip.y+clip.h)-yd);

	if((ww<=0)||(hh<0))
	{
		Alasterror="outside region.";
		return FALSE;
	}

	if((b->nbBits==32)&&(nbBits==32))
	{
		switch(cstate)
		{
			case bitmapNORMAL:
			if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				int		a0=(int)(b->alpha*256.f);
				dword	y=ys;
				int		j;
				for(j=0; j<hh; j++)
				{
					dword	x=xs;
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					for(i=0; i<ww; i++)
					{
						byte	rs,gs,bs,as,asi;
						byte	rd,gd,bd,ad;
						colorRGBA(&rs, &gs, &bs, &as, s[x>>16]);
						colorRGBA(&rd, &gd, &bd, &ad, *d);
						as=(byte)(((int)as*a0)>>8);
						asi=255-as;

						//rd=(byte)(((int)rd*asi+(int)rs*as)>>8);
						//gd=(byte)(((int)gd*asi+(int)gs*as)>>8);
						//bd=(byte)(((int)bd*asi+(int)bs*as)>>8);
						//ad=(byte)(((int)ad*asi+(int)as*as)>>8);

						rd=(byte)((int)rd+((((int)rs-(int)rd)*as)>>8));
						gd=(byte)((int)gd+((((int)gs-(int)gd)*as)>>8));
						bd=(byte)((int)bd+((((int)bs-(int)bd)*as)>>8));
						//ad=(byte)((int)ad+((((int)as-(int)ad)*as)>>8));

						*(d++)=color32(rd, gd, bd);
						x+=dw;
					}
					y+=dh;
				}
			}
			else if(cflags&bitmapALPHA)
			{
				dword	y=ys;
				int		j;
				for(j=0; j<hh; j++)
				{
					dword	x=xs;
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					for(i=0; i<ww; i++)
					{
						byte	rs,gs,bs,as,asi;
						byte	rd,gd,bd,ad;
						colorRGBA(&rs, &gs, &bs, &as, s[x>>16]);
						colorRGBA(&rd, &gd, &bd, &ad, *d);
						asi=255-as;

						//rd=(byte)(((int)rd*asi+(int)rs*as)>>8);
						//gd=(byte)(((int)gd*asi+(int)gs*as)>>8);
						//bd=(byte)(((int)bd*asi+(int)bs*as)>>8);
						//ad=(byte)(((int)ad*asi+(int)as*as)>>8);

						rd=(byte)((int)rd+((((int)rs-(int)rd)*as)>>8));
						gd=(byte)((int)gd+((((int)rs-(int)gd)*as)>>8));
						bd=(byte)((int)bd+((((int)bs-(int)bd)*as)>>8));
						//ad=(byte)((int)ad+((((int)as-(int)ad)*as)>>8));

						*(d++)=color32(rd, gd, bd);
						x+=dw;
					}
					y+=dh;
				}
			}
			else if(cflags&bitmapCOLORKEY)
			{
				dword	y=ys;
				int		j;
				for(j=0; j<hh; j++)
				{
					dword	x=xs;
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					for(i=0; i<ww; i++)
					{
						dword	v=s[x>>16];
						if(v!=b->colorkey)
							*d=v;
						d++;
						x+=dw;
					}
					y+=dh;
				}
			}
			else if(cflags&bitmapGLOBALALPHA)
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				{
					int		a0=(int)((1.f-b->alpha)*255.99f);
					int		a1=(int)(b->alpha*255.99f);
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs;
							byte	rd,gd,bd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
							gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
							bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
							*(d++)=color32(rd, gd, bd);
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			else
			{
				int		j;
				dword	y=ys;
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						*(d++)=s[x>>16];
						x+=dw;
					}
					y+=dh;
				}
			}
			return TRUE;

			case bitmapOR:
			{
				int		j;
				dword	y=ys;
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						*(d++)|=s[x>>16];
						x+=dw;
					}
					y+=dh;
				}
			}
			return TRUE;

			case bitmapXOR:
			{
				int		j;
				dword	y=ys;
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						*(d++)^=s[x>>16];
						x+=dw;
					}
					y+=dh;
				}
			}
			return TRUE;

			case bitmapAND:
			{
				int		j;
				dword	y=ys;
				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						*(d++)&=s[x>>16];
						x+=dw;
					}
					y+=dh;
				}
			}
			return TRUE;

			case bitmapADD:
			if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				//int		a0=(int)(255.99f);
				int		a1=(int)(b->alpha*256.f);
				if(cflags&bitmapSATURATION)
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs,aa;
							byte	rd,bd,gd;
							colorRGBA(&rs, &gs, &bs, &aa, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							aa=(byte)(((int)aa*a1)>>8);
							*(d++)=color32(mini(255, (int)rd+(((int)rs*aa)>>8)), mini(255, (int)gd+(((int)gs*aa)>>8)), mini(255, (int)bd+(((int)bs*aa)>>8)));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs,aa;
							byte	rd,bd,gd;
							colorRGBA(&rs, &gs, &bs, &aa, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							aa=(byte)(((int)aa*a1)>>8);
							*(d++)=revers32((int)rd+(((int)rs*aa)>>8), (int)gd+(((int)gs*aa)>>8), (int)bd+(((int)bs*aa)>>8));
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			else if(cflags&bitmapGLOBALALPHA)
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				int		a0=(int)(255.99f);
				int		a1=(int)(b->alpha*255.99f);
				if(cflags&bitmapSATURATION)
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32(mini(255, (int)rd+(((int)rs*a1)>>8)), mini(255, (int)gd+(((int)gs*a1)>>8)), mini(255, (int)bd+(((int)bs*a1)>>8)));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=revers32((int)rd+(((int)rs*a1)>>8), (int)gd+(((int)gs*a1)>>8), (int)bd+(((int)bs*a1)>>8));
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			else
			{
				if(cflags&bitmapSATURATION)
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32(mini(255, (int)rs+(int)rd), mini(255, (int)gs+(int)gd), mini(255, (int)bs+(int)bd));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=revers32((int)rs+(int)rd, (int)gs+(int)gd, (int)bs+(int)bd);
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			return TRUE;

			case bitmapSUB:
			if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				int		a1=(int)(b->alpha*256.f);
				//int		a0=(int)(255.99f-a1);
				if(cflags&bitmapSATURATION)
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs,aa;
							byte	rd,bd,gd;
							colorRGBA(&rs, &gs, &bs, &aa, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							aa=(byte)(((int)aa*a1)>>8);
							*(d++)=color32(maxi(0, (int)rd-(((int)rs*aa)>>8)), maxi(0, (int)gd-(((int)gs*aa)>>8)), maxi(0, (int)bd-(((int)bs*aa)>>8)));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	r0,g0,b0,aa;
							byte	r1,g1,b1;
							colorRGBA(&r0, &g0, &b0, &aa, s[x>>16]);
							colorRGB(&r1, &g1, &b1, *d);
							aa=(byte)(((int)aa*a1)>>8);
							*(d++)=revers32((int)r1-(((int)r0*aa)>>8), (int)g1-(((int)g0*aa)>>8), (int)b1-(((int)b0*aa)>>8));
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			else if(cflags&bitmapGLOBALALPHA)
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				int		a1=(int)(b->alpha*255.99f);
				int		a0=(int)(255.99f-a1);
				if(cflags&bitmapSATURATION)
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32(maxi(0, (int)rd-(((int)rs*a1)>>8)), maxi(0, (int)gd-(((int)gs*a1)>>8)), maxi(0, (int)bd-(((int)bs*a1)>>8)));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	r0,g0,b0;
							byte	r1,g1,b1;
							colorRGB(&r0, &g0, &b0, s[x>>16]);
							colorRGB(&r1, &g1, &b1, *d);
							*(d++)=revers32((int)r1-(((int)r0*a1)>>8), (int)g1-(((int)g0*a1)>>8), (int)b1-(((int)b0*a1)>>8));
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			else
			{
				if(cflags&bitmapSATURATION)
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32(maxi(0, (int)rd-(int)rs), maxi(0, (int)gd-(int)gs), maxi(0, (int)bd-(int)bs));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					int		j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,bs,gs;
							byte	rd,bd,gd;
							colorRGB(&rs, &gs, &bs, s[x>>16]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=revers32((int)rd-(int)rs, (int)gd-(int)gs, (int)bd-(int)bs);
							x+=dw;
						}
						y+=dh;
					}
				}
			}
			return TRUE;

			case bitmapMUL:	
			if((cflags&(bitmapALPHA|bitmapGLOBALALPHA))==(bitmapALPHA|bitmapGLOBALALPHA))
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				int		a10=(int)(b->alpha*255.99f);
				int		j;
				dword	y=ys;

				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						byte	rs,bs,gs,aa;
						byte	rd,bd,gd;
						colorRGBA(&rs, &gs, &bs, &aa, s[x>>16]);
						colorRGB(&rd, &gd, &bd, *d);
						int		a1=((int)aa*a10)>>8;
						int		a0=65536-(a1<<8);  
						*(d++)=color32( (((int)rs*a1+a0)*(int)rd)>>16, (((int)gs*a1+a0)*(int)gd)>>16, (((int)bs*a1+a0)*(int)bd)>>16 );
						x+=dw;
					}
					y+=dh;
				}
			}
			else if(cflags&bitmapGLOBALALPHA)
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				int		a1=(int)(b->alpha*255.99f);
				int		a0=(int)((255.99f-a1)*255.99f);
				int		j;
				dword	y=ys;

				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						byte	rs,bs,gs;
						byte	rd,bd,gd;
						colorRGB(&rs, &gs, &bs, s[x>>16]);
						colorRGB(&rd, &gd, &bd, *d);
						*(d++)=color32( (((int)rs*a1+a0)*(int)rd)>>16, (((int)gs*a1+a0)*(int)gd)>>16, (((int)bs*a1+a0)*(int)bd)>>16 );
						x+=dw;
					}
					y+=dh;
				}
			}
			else
			{
				int		j;
				dword	y=ys;

				for(j=0; j<hh; j++)
				{
					dword	*s=&b->body32[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						byte	rs,bs,gs;
						byte	rd,bd,gd;
						colorRGB(&rs, &gs, &bs, s[x>>16]);
						colorRGB(&rd, &gd, &bd, *d);
						*(d++)=color32( ((int)rs*(int)rd)>>8, ((int)gs*(int)gd)>>8, ((int)bs*(int)bd)>>8 );
						x+=dw;
					}
					y+=dh;
				}
			}
			break;
		}
	}
	else if((b->nbBits==8)&&(nbBits==32))
	{
		switch(cstate)
		{
			case bitmapNORMAL:
			if(cflags)
			{
				if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
				{
					b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
					{
						int		a0=(int)((1.f-b->alpha)*255.99f);
						int		a1=(int)(b->alpha*255.99f);
						int		i,j;
						dword	y=ys;
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[y>>16]];
							dword	*d=&body32[adr[yd+j]+xd];
							dword	x=xs;
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
								//colorRGB(&rs, &gs, &bs, (dword)s[x>>16]);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*(d++)=color32(rd, gd, bd);
								x+=dw;
							}
							y+=dh;
						}
						return TRUE;
					}
				}
			}
			else
			{
				int		j;
				dword	y=ys;
				for(j=0; j<hh; j++)
				{
					byte	*s=&b->body8[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						*(d++)=b->palette[s[x>>16]];
						x+=dw;
					}
					y+=dh;
				}
				return TRUE;
			}
			break;

			case bitmapADD:
			if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				{
					int		a0=(int)((1.f-b->alpha)*255.99f);
					int		a1=(int)(b->alpha*255.99f);
					int		i,j;
					dword	y=ys;
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[y>>16]];
							dword	*d=&body32[adr[yd+j]+xd];
							dword	x=xs;
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)mini(255, ((int)rd+((int)(rs*a1)>>8)));
								gd=(byte)mini(255, ((int)gd+((int)(gs*a1)>>8)));
								bd=(byte)mini(255, ((int)bd+((int)(bs*a1)>>8)));
								*(d++)=color32(rd, gd, bd);
								x+=dw;
							}
							y+=dh;
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[y>>16]];
							dword	*d=&body32[adr[yd+j]+xd];
							dword	x=xs;
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)revers8(((int)rd+((int)(rs*a1)>>8)));
								gd=(byte)revers8(((int)gd+((int)(gs*a1)>>8)));
								bd=(byte)revers8(((int)bd+((int)(bs*a1)>>8)));
								*(d++)=color32(rd, gd, bd);
								x+=dw;
							}
							y+=dh;
						}
					}
					return TRUE;
				}
			}
			else
			{
				int		j;
				dword	y=ys;
				if(cflags&bitmapSATURATION)
				{
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs;
							byte	rd,gd,bd;
							colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32(mini(255, (int)rd+(int)rs), mini(255, (int)gd+(int)gs), mini(255, (int)bd+(int)bs));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs;
							byte	rd,gd,bd;
							colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=revers32((int)rd+(int)rs, (int)gd+(int)gs, (int)bd+(int)bs);
							x+=dw;
						}
						y+=dh;
					}
				}
				return TRUE;
			}
			break;

			case bitmapSUB:
			if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				{
					int		a0=(int)((1.f-b->alpha)*255.99f);
					int		a1=(int)(b->alpha*255.99f);
					int		i,j;
					dword	y=ys;
					if(cflags&bitmapSATURATION)
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[y>>16]];
							dword	*d=&body32[adr[yd+j]+xd];
							dword	x=xs;
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)maxi(0, (int)rd-((int)(rs*a1)>>8));
								gd=(byte)maxi(0, (int)gd-((int)(gs*a1)>>8));
								bd=(byte)maxi(0, (int)bd-((int)(bs*a1)>>8));
								*(d++)=color32(rd, gd, bd);
								x+=dw;
							}
							y+=dh;
						}
					}
					else
					{
						for(j=0; j<hh; j++)
						{
							byte	*s=&b->body8[b->adr[y>>16]];
							dword	*d=&body32[adr[yd+j]+xd];
							dword	x=xs;
							for(i=0; i<ww; i++)
							{
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)revers8((int)rd-((int)(rs*a1)>>8));
								gd=(byte)revers8((int)gd-((int)(gs*a1)>>8));
								bd=(byte)revers8((int)bd-((int)(bs*a1)>>8));
								*(d++)=color32(rd, gd, bd);
								x+=dw;
							}
							y+=dh;
						}
					}
					return TRUE;
				}
			}
			else
			{
				int		j;
				dword	y=ys;
				if(cflags&bitmapSATURATION)
				{
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs;
							byte	rd,gd,bd;
							colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32(maxi(0, (int)rd-(int)rs), maxi(0, (int)gd-(int)gs), maxi(0, (int)bd-(int)bs));
							x+=dw;
						}
						y+=dh;
					}
				}
				else
				{
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						int		i;
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs;
							byte	rd,gd,bd;
							colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=revers32((int)rd-(int)rs, (int)gd-(int)gs, (int)bd-(int)bs);
							x+=dw;
						}
						y+=dh;
					}
				}
				return TRUE;
			}
			break;

			case bitmapMUL:
			if(cflags&bitmapGLOBALALPHA)						// bitmapNORMAL & bitmapGLOBALALPHA /////////////////////////////////////////////////
			{
				b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
				{
					int		a1=(int)(b->alpha*255.99f);
					int		a0=(int)((255.99f-a1)*255.99f);
					int		i,j;
					dword	y=ys;
					for(j=0; j<hh; j++)
					{
						byte	*s=&b->body8[b->adr[y>>16]];
						dword	*d=&body32[adr[yd+j]+xd];
						dword	x=xs;
						for(i=0; i<ww; i++)
						{
							byte	rs,gs,bs;
							byte	rd,gd,bd;
							colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
							colorRGB(&rd, &gd, &bd, *d);
							*(d++)=color32( (((int)rs*a1+a0)*(int)rd)>>16, (((int)gs*a1+a0)*(int)gd)>>16, (((int)bs*a1+a0)*(int)bd)>>16 );
							x+=dw;
						}
						y+=dh;
					}
					return TRUE;
				}
			}
			else
			{
				int		j;
				dword	y=ys;
				for(j=0; j<hh; j++)
				{
					byte	*s=&b->body8[b->adr[y>>16]];
					dword	*d=&body32[adr[yd+j]+xd];
					int		i;
					dword	x=xs;
					for(i=0; i<ww; i++)
					{
						byte	rs,gs,bs;
						byte	rd,gd,bd;
						colorRGB(&rs, &gs, &bs, b->palette[s[x>>16]]);
						colorRGB(&rd, &gd, &bd, *d);
						*(d++)=color32( ((int)rs*(int)rd)>>8, ((int)gs*(int)gd)>>8, ((int)bs*(int)bd)>>8 );
						x+=dw;
					}
					y+=dh;
				}
				return TRUE;
			}
			break;
		}		
	}
	Alasterror="incompatible depth resolution or bitmap state. (not implemented)";
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline void line8(int adr, int ldx, int ldy, int x1, int y1, int n, int couleur, int body)
{
	__asm
	{
		pushad
		mov		esi,x1
		mov		edi,y1
		mov		ecx,n
		mov		eax,couleur
	line8_bouc:
		mov		edx,edi
		mov		ebx,esi
		shr		edx,16
		shr		ebx,16
		add		edx,adr
		mov		edx,[edx]
		add		edx,ebx
		add		edx,body
		mov		[edx],al
		add		esi,ldx
		add		edi,ldy
		dec		ecx
		jnz		line8_bouc
		popad
	}
}

__inline void line16(int adr, int ldx, int ldy, int x1, int y1, int n, int couleur, int body)
{
	__asm
	{
		pushad
		mov		esi,x1
		mov		edi,y1
		mov		ecx,n
		mov		eax,couleur
	bline16_bouc:
		mov		edx,edi
		mov		ebx,esi
		shr		edx,16
		shr		ebx,16
		add		edx,adr
		mov		edx,[edx]
		add		edx,ebx
		shl		edx,1
		add		edx,body
		mov		[edx],ax
		add		esi,ldx
		add		edi,ldy
		dec		ecx
		jnz		bline16_bouc
		popad
	}
}

__inline void line32(int adr, int ldx, int ldy, int x1, int y1, int n, int couleur, int body)
{
	__asm
	{
		pushad
		mov		esi,x1
		mov		edi,y1
		mov		ecx,n
		mov		eax,couleur
	cline32_bouc:
		mov		edx,edi
		mov		ebx,esi
		shr		edx,16
		shr		ebx,16
		shl		edx,2
		add		edx,adr
		mov		edx,[edx]
		add		edx,ebx
		shl		edx,2
		add		edx,body
		mov		[edx],eax
		add		esi,ldx
		add		edi,ldy
		dec		ecx
		jnz		cline32_bouc
		popad
	}
}


ADLL bool Abitmap::line(int x1, int y1, int x2, int y2, dword couleur)
{
	int		ldx,ldy;
	int		adx,ady;
	int		n;
	int		lineclipx1=clip.x;
	int		lineclipy1=clip.y;
	int		lineclipx2=clip.x+clip.w-1;
	int		lineclipy2=clip.y+clip.h-1;
	int		body=(int)this->body;
	int		adr=(int)this->adr;

	x1+=view.x;
	y1+=view.y;
	x2+=view.x;
	y2+=view.y;

	// clipping
	if((x1==x2)&&(y1==y2))
		return pixel(x1, y1, couleur);

	if(y1<lineclipy1)
	{
		if(y2<lineclipy1)
			return FALSE;
		x1=((x2-x1)*(lineclipy1-y1))/(y2-y1)+x1;
		y1=lineclipy1;
		if(y2>lineclipy2)
		{
			x2=((x2-x1)*(lineclipy2-y1))/(y2-y1)+x1;
			y2=lineclipy2;
		}
	}
	else if(y1>lineclipy2)
	{
		if(y2>lineclipy2)
			return FALSE;
		x1=((x2-x1)*(lineclipy2-y1))/(y2-y1)+x1;
		y1=lineclipy2;
		if(y2<lineclipy1)
		{
			x2=((x2-x1)*(lineclipy1-y1))/(y2-y1)+x1;
			y2=lineclipy1;
		}
	}
	else if(y2<lineclipy1)
	{
		x2=((x2-x1)*(lineclipy1-y1))/(y2-y1)+x1;
		y2=lineclipy1;
	}
	else if(y2>lineclipy2)
	{
		x2=((x2-x1)*(lineclipy2-y1))/(y2-y1)+x1;
		y2=lineclipy2;
	}

	if(x1<lineclipx1)
	{
		if(x2<lineclipx1)
			return FALSE;
		y1=((y2-y1)*(lineclipx1-x1))/(x2-x1)+y1;
		x1=lineclipx1;
		if(x2>lineclipx2)
		{
			y2=((y2-y1)*(lineclipx2-x1))/(x2-x1)+y1;
			x2=lineclipx2;
		}
	}
	else if(x1>lineclipx2)
	{
		if(x2>lineclipx2)
			return FALSE;
		y1=((y2-y1)*(lineclipx2-x1))/(x2-x1)+y1;
		x1=lineclipx2;
		if(x2<lineclipx1)
		{
			y2=((y2-y1)*(lineclipx1-x1))/(x2-x1)+y1;
			x2=lineclipx1;
		}
	}
	else if(x2<lineclipx1)
	{
		y2=((y2-y1)*(lineclipx1-x1))/(x2-x1)+y1;
		x2=lineclipx1;
	}
	else if(x2>lineclipx2)
	{
		y2=((y2-y1)*(lineclipx2-x1))/(x2-x1)+y1;
		x2=lineclipx2;
	}

	x1<<=16;
	x2<<=16;
	y1<<=16;
	y2<<=16;
	
	ldx=(x2-x1);
	ldy=(y2-y1);

	adx=abs(ldx)>>16;
	ady=abs(ldy)>>16;

	if(adx>ady)
		n=adx;
	else
		n=ady;

	if(n!=0)
	{
		ldx/=n;
		ldy/=n;
	}

	n++;

	switch(nbBits)
	{
		case 8:
		line8(adr, ldx, ldy, x1, y1, n, couleur, body);
		break;

		case 16:
		line16(adr, ldx, ldy, x1, y1, n, couleur, body);
		break;

		case 32:
		line32(adr, ldx, ldy, x1, y1, n, couleur, body);
		break;

		default:
		Alasterror="incompatible depth resolution.";
		return FALSE;
	}
	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::pixela(int x, int y, dword c, float alpha, dword mode)
{
	x+=view.x;
	y+=view.y;
	if(clip.contains(x, y))
	{
		switch(nbBits)
		{
			case 32:
			{
				dword	*d=&body32[adr[y]+x];
				int		as=(int)(alpha*256.f);
				int		ad=256-as;
				byte	rs,gs,bs,aas;
				byte	rd,gd,bd,aad;
				colorRGBA(&rs, &gs, &bs, &aas, c);
				colorRGBA(&rd, &gd, &bd, &aad, *d);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8; 
				bs=(bs*as)>>8;
				switch(mode&0xff)
				{
					case lineaNORMAL:
					*d=color32(((rd*ad)>>8)+rs, ((gd*ad)>>8)+gs, ((bd*ad)>>8)+bs);
					break;
					
					case lineaADD:
					if(mode&lineaSATURATION)
						*d=color32(mini((int)rd+(int)rs, 255), mini((int)gd+(int)gs, 255), mini((int)bd+(int)bs, 255));
					else
						*d=color32(rd+rs, gd+gs, bd+bs);
					break;
					
					case lineaSUB:
					if(mode&lineaSATURATION)
						*d=color32(maxi((int)rd-(int)rs, 0), maxi((int)gd-(int)gs, 0), maxi((int)bd-(int)bs, 0));
					else
						*d=color32(rd-rs, gd-gs, bd-bs);
					break;

					case lineaXOR:
					*d=color32(rd^rs, gd^gs, bd^bs);
					break;
					
					case lineaOLDALPHA:
					*d=color32(((rd*ad)>>8)+rs, ((gd*ad)>>8)+gs, ((bd*ad)>>8)+bs, aad);
					break;
				}
				break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::linea(int x1, int y1, int x2, int y2, dword color, float alpha, dword mode)
{
	if(alpha==-1.f)
		alpha=(float)(color>>24)/255.f;

	int		ldx,ldy;
	int		adx,ady;
	int		n;
	int		lineclipx1=clip.x;
	int		lineclipy1=clip.y;
	int		lineclipx2=clip.x+clip.w-1;
	int		lineclipy2=clip.y+clip.h-1;
	int		as=(int)(alpha*255.99f);
	int		ad=255-as;

	x1+=view.x;
	y1+=view.y;
	x2+=view.x;
	y2+=view.y;
	
	if((x2==x1)&&(y1==y2))
		return pixela(x1, y1, color, alpha, mode);

	// clipping

	if(y1<lineclipy1)
	{
		if(y2<lineclipy1)
			return FALSE;
		x1=((x2-x1)*(lineclipy1-y1))/(y2-y1)+x1;
		y1=lineclipy1;
		if(y2>lineclipy2)
		{
			x2=((x2-x1)*(lineclipy2-y1))/(y2-y1)+x1;
			y2=lineclipy2;
		}
	}
	else if(y1>lineclipy2)
	{
		if(y2>lineclipy2)
			return FALSE;
		x1=((x2-x1)*(lineclipy2-y1))/(y2-y1)+x1;
		y1=lineclipy2;
		if(y2<lineclipy1)
		{
			x2=((x2-x1)*(lineclipy1-y1))/(y2-y1)+x1;
			y2=lineclipy1;
		}
	}
	else if(y2<lineclipy1)
	{
		x2=((x2-x1)*(lineclipy1-y1))/(y2-y1)+x1;
		y2=lineclipy1;
	}
	else if(y2>lineclipy2)
	{
		x2=((x2-x1)*(lineclipy2-y1))/(y2-y1)+x1;
		y2=lineclipy2;
	}

	if(x1<lineclipx1)
	{
		if(x2<lineclipx1)
			return FALSE;
		y1=((y2-y1)*(lineclipx1-x1))/(x2-x1)+y1;
		x1=lineclipx1;
		if(x2>lineclipx2)
		{
			y2=((y2-y1)*(lineclipx2-x1))/(x2-x1)+y1;
			x2=lineclipx2;
		}
	}
	else if(x1>lineclipx2)
	{
		if(x2>lineclipx2)
			return FALSE;
		y1=((y2-y1)*(lineclipx2-x1))/(x2-x1)+y1;
		x1=lineclipx2;
		if(x2<lineclipx1)
		{
			y2=((y2-y1)*(lineclipx1-x1))/(x2-x1)+y1;
			x2=lineclipx1;
		}
	}
	else if(x2<lineclipx1)
	{
		y2=((y2-y1)*(lineclipx1-x1))/(x2-x1)+y1;
		x2=lineclipx1;
	}
	else if(x2>lineclipx2)
	{
		y2=((y2-y1)*(lineclipx2-x1))/(x2-x1)+y1;
		x2=lineclipx2;
	}

	x1<<=16;
	x2<<=16;
	y1<<=16;
	y2<<=16;
	
	ldx=(x2-x1);
	ldy=(y2-y1);

	adx=abs(ldx)>>16;
	ady=abs(ldy)>>16;

	if(adx>ady)
		n=adx;
	else
		n=ady;

	if(n!=0)
	{
		ldx/=n;
		ldy/=n;
	}

	n++;

	switch(nbBits)
	{
		case 8:
		switch(mode)
		{
			case lineaNORMAL:
			color=color*as;
			while(n--)
			{
				byte	*p=&body8[adr[(y1>>16)]+(x1>>16)];
				*p=(byte)(((dword)*p*ad)+color);
				x1+=ldx;
				y1+=ldy;
			}
			break;

			case lineaADD:
			color=color*as;
			while(n--)
			{
				byte	*p=&body8[adr[y1>>16]+(x1>>16)];
				*p+=(byte)color;
				x1+=ldx;
				y1+=ldy;
			}
			break;

			case lineaSUB:
			color=color*as;
			while(n--)
			{
				byte	*p=&body8[adr[y1>>16]+(x1>>16)];
				*p-=(byte)color;
				x1+=ldx;
				y1+=ldy;
			}
			break;

			case lineaXOR:
			color=color*as;
			while(n--)
			{
				byte	*p=&body8[adr[y1>>16]+(x1>>16)];
				*p=*p^(byte)color;
				x1+=ldx;
				y1+=ldy;
			}
			break;
		}
		break;

		case 16:
		switch(mode)
		{
			case lineaNORMAL:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				colorRGB(&rs, &gs, &bs, (word)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					word	*p=&body16[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color16(((rd*ad)>>8)+rs, ((gd*ad)>>8)+gs, ((bd*ad)>>8)+bs);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaADD:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				colorRGB(&rs, &gs, &bs, (word)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					word	*p=&body16[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color16(rd+rs, gd+gs, bd+bs);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaSUB:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				colorRGB(&rs, &gs, &bs, (word)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					word	*p=&body16[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color16(rd-rs, gd-gs, bd-bs);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaXOR:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				colorRGB(&rs, &gs, &bs, (word)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					word	*p=&body16[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color16(rd^rs, gd^gs, bd^bs);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;
		}
		break;

		case 32:
		switch(mode&0xff)
		{
			case lineaNORMAL:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd,adl;
				byte	al;
				colorRGBA(&rs, &gs, &bs, &al, (dword)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8; 
				bs=(bs*as)>>8;
				al=(al*as)>>8;
				while(n--)
				{
					dword	*p=&body32[adr[y1>>16]+(x1>>16)];
					colorRGBA(&rd, &gd, &bd, &adl, *p);
					*p=color32(((rd*ad)>>8)+rs, ((gd*ad)>>8)+gs, ((bd*ad)>>8)+bs, ((adl*ad)>>8)+al);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaADD:
			if(mode&lineaSATURATION)
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				byte	al;
				colorRGBA(&rs, &gs, &bs, &al, (dword)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					dword	*p=&body32[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color32(mini((int)rd+(int)rs, 255), mini((int)gd+(int)gs, 255), mini((int)bd+(int)bs, 255), al);
					x1+=ldx;
					y1+=ldy;
				}
			}
			else
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				byte	al;
				colorRGBA(&rs, &gs, &bs, &al, (dword)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					dword	*p=&body32[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color32(rd+rs, gd+gs, bd+bs, al);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaSUB:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				byte	al;
				colorRGBA(&rs, &gs, &bs, &al, (dword)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					dword	*p=&body32[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color32(rd-rs, gd-gs, bd-bs, al);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaXOR:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				byte	al;
				colorRGBA(&rs, &gs, &bs, &al, (dword)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					dword	*p=&body32[adr[y1>>16]+(x1>>16)];
					colorRGB(&rd, &gd, &bd, *p);
					*p=color32(rd^rs, gd^gs, bd^bs, al);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;

			case lineaOLDALPHA:
			{
				byte	rs,gs,bs;
				byte	rd,gd,bd;
				byte	al;
				colorRGB(&rs, &gs, &bs, (dword)color);
				rs=(rs*as)>>8;
				gs=(gs*as)>>8;
				bs=(bs*as)>>8;
				while(n--)
				{
					dword	*p=&body32[adr[y1>>16]+(x1>>16)];
					colorRGBA(&rd, &gd, &bd, &al, *p);
					*p=color32(((rd*ad)>>8)+rs, ((gd*ad)>>8)+gs, ((bd*ad)>>8)+bs, al);
					x1+=ldx;
					y1+=ldy;
				}
			}
			break;
		}
		break;

		default:
		Alasterror="incompatible depth resolution.";
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::box(int x0, int y0, int x1, int y1, dword c)
{
	line(x0, y0, x1, y0, c);
	line(x0, y1, x1, y1, c);
	line(x0, y0, x0, y1, c);
	line(x1, y0, x1, y1, c);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::boxf(int x0, int y0, int x1, int y1, dword c)
{
	int			xx0,yy0,xx1,yy1;
	int			x,y;
	Arectangle	tr;

	tr.x=mini(x0, x1);
	tr.y=mini(y0, y1);
	tr.w=(maxi(x0, x1)-tr.x)+1;
	tr.h=(maxi(y0, y1)-tr.y)+1;
	tr.translat(view.x, view.y);
	
	tr.intersect(clip);
	
	if(!(tr.w&&tr.h))
		return true;
		
	xx0=tr.x;
	yy0=tr.y;
	xx1=tr.x+tr.w;
	yy1=tr.y+tr.h;

	switch(nbBits)
	{
		case 8:
		for(y=yy0; y<yy1; y++)
		{
			byte	*p=&body8[adr[y]+xx0];
			for(x=xx0; x<xx1; x++)
				*(p++)=(byte)c;
		}
		break;

		case 16:
		for(y=yy0; y<yy1; y++)
		{
			word	*p=&body16[adr[y]+xx0];
			for(x=xx0; x<xx1; x++)
				*(p++)=(word)c;
		}
		break;

		case 32:
		for(y=yy0; y<yy1; y++)
		{
			dword	*p=&body32[adr[y]+xx0];
			for(x=xx0; x<xx1; x++)
				*(p++)=c;
		}
		break;

		default:
		Alasterror="incompatible depth resolution.";
		break;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::boxa(int x0, int y0, int x1, int y1, dword c, float alpha, dword mode)
{
	if(alpha==-1.f)
		alpha=(float)(c>>24)/255.f;
	linea(x0, y0, x1, y0, c, alpha, mode);
	linea(x0, y1, x1, y1, c, alpha, mode);
	linea(x0, y0+1, x0, y1-1, c, alpha, mode);
	linea(x1, y0+1, x1, y1-1, c, alpha, mode);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::boxfa(int x0, int y0, int x1, int y1, dword c, float alpha, dword mode)
{
	if(alpha==-1.f)
		alpha=(float)(c>>24)/255.f;
/*
#ifdef MMX
	if(nbBits==32)
	{
		int		xx0,yy0,xx1,yy1;
		int		x,y;

		x0+=view.x;
		x1+=view.x;
		y0+=view.y;
		y1+=view.y;

		xx0=maxi(mini(mini(x0, x1), clip.x+clip.w-1), clip.x);
		yy0=maxi(mini(mini(y0, y1), clip.y+clip.h-1), clip.y);
		xx1=maxi(mini(maxi(x0, x1), clip.x+clip.w-1), clip.x)+1;
		yy1=maxi(mini(maxi(y0, y1), clip.y+clip.h-1), clip.y)+1;
		{
			int		j;
			int		ww=1+xx1-xx0;
			int		hh=1+yy1-yy0;
			int		yd=yy0;
			int		xd=xx0;
			b->alpha=maxi(mini(b->alpha, 1.f), 0.f);
			int		a0=(int)((1.f-b->alpha)*255.99f);
			int		a1=(int)(b->alpha*255.99f);
			{
				int		j;
				{
					qword	m0=(int)(255.99f*(1.f-b->alpha));
					qword	m1=(int)(255.99f*b->alpha);
					dword	nb=(ww>>1);
					m0=(m0<<16)|m0;
					m1=(m1<<16)|m1;
					m0=QWORD(m0, m0);
					m1=QWORD(m1, m1);
					__asm
					{
						pxor		mm1,mm1
						movq		mm2,m1
						movq		mm3,m0
					}
					for(j=0; j<hh; j++)
					{
						dword	*s=&b->body32[b->adr[ys+j]+xs];
						dword	*d=&body32[adr[yd+j]+xd];
						if(nb)
						{
							dword	dest=(dword)d;
							dword	s0=(dword)d;
							dword	s1=(dword)s;
							__asm
							{
								mov			edi,dest
								mov			ecx,s1
								mov			edx,s0

								mov			eax,nb
							boucle00:
								dec			eax
								js			theend00

								movd		mm4,[ecx]
								add			ecx,4
								movd		mm5,[edx]
								add			edx,4

								movd		mm6,[ecx]
								add			ecx,4
								movd		mm7,[edx]
								add			edx,4

								punpcklbw	mm4,mm1
								punpcklbw	mm5,mm1
								pmullw		mm4,mm2
								pmullw		mm5,mm3

								punpcklbw	mm6,mm1
								punpcklbw	mm7,mm1
								pmullw		mm6,mm2
								pmullw		mm7,mm3

								paddusw		mm4,mm5
								paddusw		mm6,mm7

								psrlw		mm4,8
								psrlw		mm6,8

								packuswb	mm4,mm1
								packuswb	mm6,mm1

								movd		[edi],mm4
								movd		[edi+4],mm6
								add			edi,8

								jmp			boucle00

							theend00:
							}
							if(ww&1)
							{
								s+=ww-1;
								d+=ww-1;
								byte	rs,gs,bs;
								byte	rd,gd,bd;
								colorRGB(&rs, &gs, &bs, *s);
								colorRGB(&rd, &gd, &bd, *d);
								rd=(byte)(((int)rd*a0+(int)rs*a1)>>8);
								gd=(byte)(((int)gd*a0+(int)gs*a1)>>8);
								bd=(byte)(((int)bd*a0+(int)bs*a1)>>8);
								*d=color32(rd, gd, bd);
							}
						}
					}
					__asm
					{
						emms
					}
					return TRUE;
				}
			}
			__asm
			{
				emms
			}
			return TRUE;
		}		
	}
#endif
*/

/*

	int			y;
	Arectangle	r;
	r.x=x0;
	r.y=y0;
	r.add(x1, y1);
	r.translat(view.x, view.y);
	r.intersect(clip);
	r.translat(-view.x, -view.y);
	x0=r.x;
	x1=r.x+r.w-1;
	for(y=0; y<=r.h; y++)
	{
		int	yy=y+r.y;
		linea(x0, yy, x1, yy, c, alpha, mode);
	}
	
*/

	int	y;
	int	yy0=mini(y0, y1);
	int	yy1=maxi(y0, y1);
	for(y=yy0; y<=yy1; y++)
		linea(x0, y, x1, y, c, alpha, mode);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::fillAlpha(float alpha)
{
	byte a=(byte)(alpha*255.f);
	if(nbBits==32)
	{
		byte *d=(byte *)body32+3;
		int s=w*h;
		int i;
		for(i=0; i<s; i++)
		{
			*d=a;
			d+=4;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::flipX()
{
	int	nx=w>>1;
	int	x,y;
	switch(nbBits)
	{
		case 8:
		for(y=0; y<h; y++)
		{
			byte	*b0=&body8[adr[y]];
			byte	*b1=&body8[adr[y]-w-1];
			for(x=0; x<nx; x++)
			{
				byte	b=*b0;
				*b0=*b1;
				*b1=b;
				b0++;
				b1--;
			}
		}
		return true;
	
		case 32:
		for(y=0; y<h; y++)
		{
			dword	*b0=&body32[adr[y]];
			dword	*b1=&body32[adr[y]+w-1];
			for(x=0; x<nx; x++)
			{
				dword	b=*b0;
				*b0=*b1;
				*b1=b;
				b0++;
				b1--;
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::flipY()
{
	int	ny=h>>1;
	int	l=(adr[1]*nbBits)>>3;
	int	x,y;
	for(y=0; y<ny; y++)
	{
		byte	*b0=&body8[l*y];
		byte	*b1=&body8[l*(h-y-1)];
		for(x=0; x<l; x++)
		{
			byte	b=*b0;
			*b0=*b1;
			*b1=b;
			b0++;
			b1++;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::blur(Abitmap *bin, float coefx, float coefy)
{
	if((bin->w==w)&&(bin->h==h)&&(nbBits==32)&&(bin->nbBits==32))
	{
		int		x,y;
		int		dx[8];
		int		dy[8];
		int		i;
		dword	*dd=body32;

		for(i=0; i<8; i++)
		{
			float	v=(float)(i*PI/4);
			dx[i]=(int)(cos(v)*coefx);
			dy[i]=(int)(sin(v)*coefy);
		}

		for(y=0; y<h; y++)
		{
			for(x=0; x<w; x++)
			{
				int	rr=0;
				int	gg=0;
				int	bb=0;
				int	aa=0;
				int	n=0;
				for(i=0; i<8; i++)
				{
					int	xs=(int)(x+dx[i]);
					int	ys=(int)(y+dy[i]);
					if(((dword)ys<(dword)h)&&((dword)xs<(dword)w))
					{
						byte	r,g,b,a;
						colorRGBA(&r, &g, &b, &a, bin->body32[ys*w+xs]);
						rr+=(int)r;
						gg+=(int)g;
						bb+=(int)b;
						aa+=(int)a;
						n++;
					}
				}
				switch(n)
				{
					case 8:
					*(dd++)=color32(rr>>3, gg>>3, bb>>3, aa>>3);
					break;
					
					case 4:
					*(dd++)=color32(rr>>2, gg>>2, bb>>2, aa>>2);
					break;
					
					case 2:
					*(dd++)=color32(rr>>1, gg>>1, bb>>1, aa>>1);
					break;
					
					case 0:
					*(dd++)=0;
					break;
					
					default:
					*(dd++)=color32(rr/n, gg/n, bb/n, aa/n);
					break;
				}
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int px[4]={ 0, 1, 0, 1 };
static int py[4]={ 0, 0, 1, 1 };

ADLL bool Abitmap::stretch(Abitmap *bin)
{
	if((nbBits==32)&&(bin->nbBits==32))
	{
		if((bin->w<w)&&(bin->h<h))
		{
			float	dx=(float)bin->w/(float)w;
			float	dy=(float)bin->h/(float)h;
			float	ys=0.f;
			int		x,y;
			dword	*d=&body32[0];
			for(y=0; y<h; y++)
			{
				float	xs=0.f;
				int		ay[2];
				int		iy[2];

				iy[0]=(int)ys;
				iy[1]=mini(iy[0]+1, bin->h-1);
				ay[1]=(int)((ys-(float)(int)ys)*512.f);
				ay[0]=512-ay[1];

				for(x=0; x<w; x++)
				{
					int		ax[2];
					int		ix[2];
					int		i;
					int		r=0;
					int		g=0;
					int		b=0;
					int		a=0;
					int		max=0;
					
					ix[0]=(int)xs;
					ix[1]=mini(ix[0]+1, bin->w-1);
					ax[1]=(int)((xs-(float)(int)xs)*512.f);
					ax[0]=512-ax[1];
					
					for(i=0; i<4; i++)
					{
						byte	br,bg,bb,ba;
						int		nx=px[i];
						int		ny=py[i];
						int		m=ax[nx]+ay[ny];
						colorRGBA(&br, &bg, &bb, &ba, bin->body32[bin->adr[iy[ny]]+ix[nx]]);
						r+=(int)br*m;
						g+=(int)bg*m;
						b+=(int)bb*m;
						a+=(int)ba*m;
						max+=m;
					}
					*(d++)=color32(r/max, g/max, b/max, a/max);
					xs+=dx;
				}
				ys+=dy;
			}
			return true;
		}
		else if((bin->w>w)&&(bin->h>h))
		{
			return true;
		}
	}
	return set(0, 0, w, h, 0, 0, bin->w, bin->h, bin, bitmapNORMAL, bitmapNORMAL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::ablur(Abitmap *bin, float coefx, float coefy)
{
	if((bin->w==w)&&(bin->h==h)&&(nbBits==32)&&(bin->nbBits==32))
	{
		int		x,y;
		int		dx[8];
		int		dy[8];
		int		i;
		dword	*dd=body32;

		for(i=0; i<8; i++)
		{
			float	v=(float)(i*PI/4);
			dx[i]=(int)(cos(v)*coefx);
			dy[i]=(int)(sin(v)*coefy);
		}

		for(y=0; y<h; y++)
		{
			for(x=0; x<w; x++)
			{
				byte	rdd,gdd,bdd;
				int		rr=0;
				int		gg=0;
				int		bb=0;
				int		aa=0;
				int		n=0;
				
				colorRGB(&rdd, &gdd, &bdd, bin->body32[y*w+x]);

				for(i=0; i<8; i++)
				{
					int	xs=(int)(x+dx[i]);
					int	ys=(int)(y+dy[i]);
					if(((dword)ys<(dword)h)&&((dword)xs<(dword)w))
					{
						byte	r,g,b,a;
						colorRGBA(&r, &g, &b, &a, bin->body32[ys*w+xs]);
						rr+=(int)r;
						gg+=(int)g;
						bb+=(int)b;
						aa+=(int)a;
						n++;
					}
				}
				switch(n)
				{
					case 8:
					*(dd++)=color32(rdd, gdd, bdd, aa>>3);
					break;
					
					case 4:
					*(dd++)=color32(rdd, gdd, bdd, aa>>2);
					break;
					
					case 2:
					*(dd++)=color32(rdd, gdd, bdd, aa>>1);
					break;
					
					case 0:
					*(dd++)=0;
					break;
					
					default:
					*(dd++)=color32(rdd, gdd, bdd, aa/n);
					break;
				}
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Abitmap::operator=(const Abitmap& b)
{
	if(this!=&b)
		NCcopy(&b);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCcreate()
{
	state=bitmapNORMAL;
	flags=0;
	alpha=1.f;
	colorkey=0;
	bodysize=(w*h*nbBits)>>3;
	body=malloc(bodysize);
	if(body)
	{
		memset(body, 0, bodysize);
		if(nbBits==8)
		{
			palette=(dword *)malloc(256*4);
			assert(palette);
		}
		else 
			palette=NULL;
		adr=(dword *)malloc(sizeof(dword)*h);
		{
			int		i;
			dword	*a=adr;
			dword	v=0;
			for(i=0; i<h; i++)
			{
				*(a++)=v;
				v+=w;
			}
		}
		view.x=0;
		view.y=0;
		view.w=w;
		view.h=h;
		clip.x=0;
		clip.y=0;
		clip.w=w;
		clip.h=h;
		return TRUE;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCfree()
{
	if(body)
	{
		free(body);
		body=NULL;
	}
	if(palette)
	{
		free(palette);
		palette=NULL;
	}
	if(adr)
	{
		free(adr);
		adr=NULL;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCcopy(const Abitmap *b)
{
	NCfree();
	w=b->w;
	h=b->h;
	nbBits=b->nbBits;
	if(NCcreate())
	{
		memcpy(body, b->body, bodysize);
		if(palette&&b->palette)
			memcpy(palette, b->palette, 256*4);
		state=b->state;
		flags=b->flags;
		view=b->view;
		clip=b->clip;
		return TRUE;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCreadBMP(Afile *f)
{
	void				*rbody=NULL;
	RGBQUAD				*rpal=NULL;
	int					szSrc;
	BITMAPFILEHEADER	bf;
	BITMAPINFOHEADER	bi;
	
	if(f->read(&bf, sizeof(bf))!=sizeof(bf))
		goto lblError;
	if(f->read(&bi, sizeof(bi))!=sizeof(bi))
		goto lblError;
	if(bi.biBitCount<=8)
	{
		int	nc=bi.biClrUsed?bi.biClrUsed:256;
		int	sz=sizeof(RGBQUAD)*nc;
		if(f->read(rpal, sz)!=sz)
			goto lblError;
	}
	szSrc=bi.biWidth*bi.biHeight*(bi.biBitCount>>3);
	rbody=malloc(szSrc);
	if(!rbody)
		goto lblError;
	if(f->read(rbody, szSrc)!=szSrc)
		goto lblError;
		
	switch(bi.biBitCount)
	{
		case 8:
		NCfree();
		w=bi.biWidth;
		h=bi.biHeight;
		nbBits=8;
		NCcreate();
		// todo:
		break;

		case 24:
		NCfree();
		w=bi.biWidth;
		h=bi.biHeight;
		nbBits=32;
		NCcreate();
		{
			int		y;
			int		x;
			for(y=0; y<h; y++)
			{
				byte	*src=((UCHAR *)rbody)+bi.biWidth*(bi.biHeight-1-y)*3;
				dword	*dest=&body32[adr[y]];
				for(x=0; x<w;  x++)
				{
					*(dest++)=*(src++);
					*(dest++)=*(src++);
					*(dest++)=*(src++);
					*(dest++)=255;
				}
			}
		}
		break;

		case 32:
		NCfree();
		w=bi.biWidth;
		h=bi.biHeight;
		nbBits=32;
		NCcreate();
		flags=bitmapALPHA;
		{
			int		y=h-1;
			int		sw=w*4;
			UINT	*d=&body32[adr[y]];
			UINT	*s=(UINT *)rbody;
			for(;y>=0; y--)
			{
				memcpy(d, s, sw);
				d-=sw;
				s+=sw;
			}
		}
		break;

		default:
		goto lblError;
		break;
	}
	if(rpal)
		free(rpal);
	free(rbody);
	return true;
lblError:
	if(rbody)
		free(rbody);
	if(rpal)
		free(rpal);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCreadPNG(png_structp png_ptr, png_infop info_ptr)
{
	unsigned int	sig_read = 0;
	png_uint_32		width, height;
	int				bit_depth, color_type, interlace_type;

	png_set_sig_bytes(png_ptr, sig_read);

   /* The call to png_read_info() gives us all of the information from the
	* PNG file before the first IDAT (image data chunk).  REQUIRED
	*/
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

/*
	png_set_strip_16(png_ptr);
	png_set_strip_alpha(png_ptr);
	png_set_packing(png_ptr);
	png_set_packswap(png_ptr);
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
*/
	if(color_type==PNG_COLOR_TYPE_GRAY&&bit_depth<8)
		png_set_expand(png_ptr);

	{
		bool		ok=FALSE;
		png_bytep	row_pointers[bitmapMAXSIZE];
		png_uint_32	row;
		for(row=0; row<height; row++)
			row_pointers[row]=(png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
		png_read_image(png_ptr, row_pointers);
		png_read_end(png_ptr, info_ptr);

		NCfree();
		w=width;
		h=height;
		switch(color_type)
		{
			case PNG_COLOR_TYPE_GRAY:
			nbBits=8;
			NCcreate();
			{
				dword	y;
				byte	*d=body8;
				for(y=0; y<height; y++)
				{
					memcpy(d, row_pointers[y], width);
					d+=width;
				}
			}
			{
				int	i=0;
				dword	*d=palette;
				for(i=0; i<256; i++)
					*(d++)=color32((byte)i, (byte)i, (byte)i);
			}
			ok=TRUE;
			break;

			case PNG_COLOR_TYPE_PALETTE:
			nbBits=8;
			NCcreate();
			{
				dword	y;
				byte	*d=body8;
				for(y=0; y<height; y++)
				{
					memcpy(d, row_pointers[y], width);
					d+=width;
				}
			}
			{
				int	i;
				dword	*d=palette;
				for(i=0; i<256; i++)
					*(d++)=color32(info_ptr->palette[i].red, info_ptr->palette[i].green, info_ptr->palette[i].blue);
			}
			ok=TRUE;
			break;

			case PNG_COLOR_TYPE_RGB:
			nbBits=32;
			NCcreate();
			{
				dword	x,y;
				dword	*d=body32;
				for(y=0; y<height; y++)
				{
					byte	*s=(byte *)row_pointers[y];
					for(x=0; x<width; x++)
					{
						*(d++)=color32(s[0], s[1], s[2]);
						s+=3;
					}
				}
			}
			ok=TRUE;
			break;

			case PNG_COLOR_TYPE_RGB_ALPHA:
			nbBits=32;
			NCcreate();
			{
				dword	x,y;
				dword	*d=body32;
				for(y=0; y<height; y++)
				{
					byte	*s=(byte *)row_pointers[y];
					for(x=0; x<width; x++)
					{
						*(d++)=color32(s[0], s[1], s[2], s[3]);
						s+=4;
					}
				}
			}
			flags|=bitmapALPHA;
			ok=TRUE;
			break;

			case PNG_COLOR_TYPE_GRAY_ALPHA:
			nbBits=32;
			NCcreate();
			{
				dword	x,y;
				dword	*d=body32;
				for(y=0; y<height; y++)
				{
					byte	*s=(byte *)row_pointers[y];
					for(x=0; x<width; x++)
					{
						*(d++)=color32(s[0], s[0], s[0], s[1]);
						s+=2;
					}
				}
			}
			flags|=bitmapALPHA;
			ok=TRUE;
			break;
		}
		for(row=0; row<height; row++)
			free(row_pointers[row]);
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return ok;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define			JPEGBufferSize				512

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct jpeg_play_source_mgr 
{
	struct jpeg_source_mgr	pub;			/* public fields */
	Afile					*file;
	JOCTET					buffer[JPEGBufferSize];
	boolean					start_of_file;	/* have we gotten any data yet? */
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF void jpeg_play_init_source(j_decompress_ptr cinfo)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF boolean jpeg_play_fill_input_buffer(j_decompress_ptr cinfo)
{
	jpeg_play_source_mgr *src=(jpeg_play_source_mgr *)cinfo->src;
	
	assert(JPEGBufferSize<16000);

	int nbytes=(int)src->file->read(src->buffer, JPEGBufferSize);
	if( nbytes<=0 )
	{
		//assert(false);	// the jpeg code wanted more data but we did not have any...
		// Insert a fake EOI marker
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}
	src->pub.next_input_byte=src->buffer;
	src->pub.bytes_in_buffer=nbytes;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF void jpeg_play_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	jpeg_play_source_mgr *src=(jpeg_play_source_mgr*)cinfo->src;

	/* Just a dumb implementation for now.  Could use fseek() except
	* it doesn't work on pipes.  Not clear that being smart is worth
	* any trouble anyway --- large skips are infrequent.
	*/
	while(num_bytes>(long)src->pub.bytes_in_buffer)
	{
		num_bytes-=(long)src->pub.bytes_in_buffer;
		jpeg_play_fill_input_buffer(cinfo);
	}
	src->pub.next_input_byte+=(size_t)num_bytes;
	src->pub.bytes_in_buffer-=(size_t)num_bytes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF void jpeg_play_term_source(j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCreadJPG(Afile *file)
{
	if(file->isOK())
	{
		struct jpeg_play_source_mgr		src;
		struct jpeg_error_mgr			jerr;
		struct jpeg_decompress_struct	cinfo;

		memset(&src, 0, sizeof(src));
		memset(&jerr, 0, sizeof(jerr));
		memset(&cinfo, 0, sizeof(cinfo));

		cinfo.err = jpeg_afx_error(&jerr);
		jpeg_create_decompress(&cinfo);

		// Specify data source (eg, a file)
		cinfo.src = &src.pub;
		src.pub.init_source			= jpeg_play_init_source;
		src.pub.fill_input_buffer	= jpeg_play_fill_input_buffer;
		src.pub.skip_input_data		= jpeg_play_skip_input_data;
		src.pub.resync_to_restart	= jpeg_resync_to_restart; /* use default method */
		src.pub.term_source			= jpeg_play_term_source;
		src.file=file;
		src.pub.bytes_in_buffer=0;		/* forces fill_input_buffer on first read */
		src.pub.next_input_byte=NULL;	/* until buffer loaded */

		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);

		if((cinfo.output_width!=w)||(cinfo.output_height!=h)||(nbBits!=32))
		{
			NCfree();
			w=cinfo.output_width;
			h=cinfo.output_height;
			nbBits=32;
			NCcreate();
		}

		{
			JSAMPLE	*buffer=new JSAMPLE[w*cinfo.output_components];

			while(cinfo.output_scanline<cinfo.output_height)
			{
				int y=cinfo.output_scanline;
				jpeg_read_scanlines(&cinfo, &buffer, 1);

				switch(cinfo.output_components)
				{
					case 1:
					{
						int		x;
						dword	*d=&body32[adr[y]];
						JSAMPLE	*s=buffer;
						for(x=0; x<w; x++)
						{
							*(d++)=color32(*s, *s, *s);
							s++;
						}
					}
					break;

					case 3:
					{
						int		x;
						dword	*d=&body32[adr[y]];
						JSAMPLE	*s=buffer;
						for(x=0; x<w; x++)
						{
							*(d++)=color32(s[0], s[1], s[2]);
							s+=3;
						}
					}
					break;

					default:
					assert(NULL);
					break;
				}
			}

			delete(buffer);
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct jpeg_write_dest_mgr 
{
	struct jpeg_destination_mgr		pub;
	Afile							*file;
	JOCTET							buffer[JPEGBufferSize];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF void jpeg_write_init_destination(j_compress_ptr cinfo)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF boolean jpeg_write_empty_outputbuffer(j_compress_ptr cinfo)
{
	jpeg_write_dest_mgr *dest=(jpeg_write_dest_mgr *)cinfo->dest;
	dest->file->write(dest->buffer, JPEGBufferSize);
	dest->pub.free_in_buffer=JPEGBufferSize;
	dest->pub.next_output_byte=dest->buffer;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

METHODDEF void jpeg_write_term_destination(j_compress_ptr cinfo)
{
	jpeg_write_dest_mgr *dest=(jpeg_write_dest_mgr *)cinfo->dest;
	dest->file->write(dest->buffer, JPEGBufferSize-dest->pub.free_in_buffer);
	dest->pub.free_in_buffer=JPEGBufferSize;
	dest->pub.next_output_byte=dest->buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::NCwriteJPG(Afile *file, int quality)
{
	if(nbBits!=32)
		return false;
	if(file->isOK())
	{
		struct jpeg_write_dest_mgr		dest;
		struct jpeg_error_mgr			jerr;
		struct jpeg_compress_struct		cinfo;

		memset(&dest, 0, sizeof(dest));
		memset(&jerr, 0, sizeof(jerr));
		memset(&cinfo, 0, sizeof(cinfo));

		cinfo.err = jpeg_afx_error(&jerr);
		jpeg_create_compress(&cinfo);

		// Specify data dest (eg, a file)
		cinfo.dest = &dest.pub;

		dest.pub.init_destination	= jpeg_write_init_destination;
		dest.pub.empty_output_buffer = jpeg_write_empty_outputbuffer;
		dest.pub.term_destination 	= jpeg_write_term_destination;
		dest.file=file;
		dest.pub.free_in_buffer=JPEGBufferSize;
		dest.pub.next_output_byte=dest.buffer;

		cinfo.image_width=w;
		cinfo.image_height=h;
		cinfo.input_components=3;
		cinfo.in_color_space=JCS_RGB;

		//jpeg_write_header(&cinfo, TRUE);
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, TRUE);
		jpeg_start_compress(&cinfo, TRUE);

		{
			JSAMPLE	*buffer=new JSAMPLE[w*cinfo.input_components];
			int		y;

			for(y=0; y<h; y++)
			{
				int		x;
				dword	*s=&body32[adr[y]];
				JSAMPLE	*d=buffer;
				for(x=0; x<w; x++)
				{
					byte	r,g,b;
					colorRGB(&r, &g, &b, *(s++));
					*(d++)=r;
					*(d++)=g;
					*(d++)=b;
				}
				jpeg_write_scanlines(&cinfo, &buffer, 1);
			}
			delete(buffer);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::load(char *filename)
{
	char	lfilename[ALIB_PATH];
	strcpy(lfilename, filename);
	strlwr(filename);
	if(strstr(lfilename, ".bmp"))
	{
		Afilehd	*f=new Afilehd(filename, Afilehd::READ);
		bool	b=NCreadBMP(f);
		delete(f);
		return b;
	}
	else if(strstr(lfilename, ".png"))
	{
		png_structp		png_ptr;
		png_infop		info_ptr;
		FILE			*fp;
		if((fp=fopen(filename, "rb"))==NULL)
			return FALSE;
	   /* Create and initialize the png_struct with the desired error handler
		* functions.  If you want to use the default stderr and longjump method,
		* you can supply NULL for the last three parameters.  We also supply the
		* the compiler header file version, so that we know if the application
		* was compiled with a compatible version of the library.  REQUIRED
		*/
		png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(png_ptr==NULL)
		{
			fclose(fp);
			return FALSE;
		}

		/* Allocate/initialize the memory for image information.  REQUIRED. */
		info_ptr=png_create_info_struct(png_ptr);
		if(info_ptr==NULL)
		{
			fclose(fp);
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			return FALSE;
		}

		/* Set error handling if you are using the setjmp/longjmp method (this is
		* the normal method of doing things with libpng).  REQUIRED unless you
		* set up your own error handlers in the png_create_read_struct() earlier.
		*/
		if(setjmp(png_ptr->jmpbuf))
		{
			/* Free all of the memory associated with the png_ptr and info_ptr */
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			fclose(fp);
			/* If we get here, we had a problem reading the file */
			return FALSE;
		}

		//png_init_io(png_ptr, fp);
		png_ptr->io_ptr.type=0;
		png_ptr->io_ptr.file=fp;
		png_ptr->io_ptr.offset=0;
		
		bool bok=NCreadPNG(png_ptr, info_ptr);
		fclose(fp);
		return bok;
	}
	else if(strstr(lfilename, ".jpg")||strstr(lfilename, ".jpeg"))
	{
		Afilehd	*f=new Afilehd(filename, Afilehd::READ);
		bool	b=NCreadJPG(f);
		delete(f);
		return b;
	}
	return FALSE;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::load(Afile *file, int type)
{
	switch(type)
	{
		case bitmapBMP:
		break;

		case bitmapPNG:
		break;

		case bitmapJPG:
		return NCreadJPG(file);
	}
	Alasterror="not implemented";
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::save(char *filename, int quality)
{
	char	lfilename[ALIB_PATH];
	strcpy(lfilename, filename);
	strlwr(filename);
	if(strstr(lfilename, ".jpg")||strstr(lfilename, ".jpeg"))
	{
		Afilehd	*f=new Afilehd(filename, Afilehd::WRITE);
		bool	b=NCwriteJPG(f, maxi(mini(quality, 100), 0));
		delete(f);
		return b;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::save(Afile *f, int type, int quality)
{
	switch(type)
	{
		case bitmapBMP:
		break;

		case bitmapPNG:
		break;

		case bitmapJPG:
		return NCwriteJPG(f, maxi(mini(quality, 100), 0));
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abitmap::getDesktop(int x, int y)
{
	HDC					hdc=GetDC(NULL);
	HDC					dc=CreateCompatibleDC(hdc);
	HBITMAP				bmp=CreateCompatibleBitmap(hdc, w, h);
	HBITMAP				obmp=(HBITMAP)SelectObject(dc, bmp);
	BitBlt(dc, 0, 0, w, h, hdc, x, y, SRCCOPY);
	{
		BITMAPINFOHEADER	bmi;
		memset(&bmi, 0, sizeof(bmi));
		bmi.biSize=sizeof(bmi);
		bmi.biWidth=w;
		bmi.biHeight=h;
		bmi.biPlanes=1;
		bmi.biBitCount=nbBits;
		bmi.biCompression=BI_RGB;
		GetDIBits(hdc, bmp, 0, h, body, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);
		flipY();
	}
	SelectObject(dc, obmp);
	DeleteObject(bmp);
	DeleteDC(dc);
	ReleaseDC(NULL, hdc);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap * Abitmap::getDesktop()
{
	int					ww=GetSystemMetrics(SM_CXSCREEN);
	int					hh=GetSystemMetrics(SM_CYSCREEN);
	HDC					hdc=GetDC(NULL);
	HBITMAP				bmp=(HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
	Abitmap				*b=new Abitmap(ww, hh);
	BITMAPINFOHEADER	bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.biSize=sizeof(bmi);
	bmi.biWidth=b->w;
	bmi.biHeight=b->h;
	bmi.biPlanes=1;
	bmi.biBitCount=32;
	bmi.biCompression=BI_RGB;
	GetDIBits(hdc, bmp, 0, b->h, b->body, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);
	ReleaseDC(NULL, hdc);
	b->flipY();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
