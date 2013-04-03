/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PREVIEW.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<stdlib.h>
#include						<math.h>
#include						"background.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Abackground::CI=ACI("Abackground", GUID(0x11111111,0x00000052), &Abitmap::CI, 0, NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline float frand()
{
	return (float)rand()/(float)RAND_MAX;
}

__inline float calc(float *xx, float *yy, float *zz, int x, int y)
{
	float	z=0;
	int		i;
	for(i=0; i<NBZYG; i++)
	{
		float	dx=(float)x-xx[i];
		float	dy=(float)y-yy[i];
		z+=(float)sin(sqrt(dx*dx+dy*dy)*zz[i])*(511.99f/(float)NBZYG)+(512.f/(float)NBZYG);
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

void Abackground::calculate(int type)
{
	switch(type)
	{
		case 0:
		{
			int	i;
			for(i=0; i<h; i++)
				line(0, i, w-1, i, (i&4)?0xff208000:0xff206000);
		}
		break;

		case 1:
		{
			int	i;
			float	sub=(float)sqrt(10.f);
			float	mul=255.99f/((float)sqrt((float)h+10.f)-sub);
			for(i=0; i<h; i++)
			{
				int	n=(int)((sqrt((float)i+10.f)-sub)*mul);
				int	v=255-(n>>1);
				line(0, i, w-1, i, 0xffc00000|(v<<8));
			}
		}
		//boxfa(0, 0, w, h, 0xff000000, 0.5f);
		break;

		case 2:
		{
			int								color[1024];
			float							xx[NBZYG];
			float							yy[NBZYG];
			float							zz[NBZYG];
			float							*ol;
			float							*nl;
			int								y,x,i;
			ol=(float *)malloc(w*sizeof(float));
			nl=(float *)malloc(w*sizeof(float));
			for(i=0; i<NBZYG; i++)
			{
				xx[i]=(float)frand()*w;
				yy[i]=(float)frand()*h;
				zz[i]=50.f/(frand()*w*10.f+(float)w);
			}
			switch(0)
			{
				case 0:
				{
					float	ar=frand()*100.f;
					float	ag=frand()*100.f;
					float	ab=frand()*100.f;
					float	dr=(frand()+0.1f)*0.1f;
					float	dg=(frand()+0.1f)*0.1f;
					float	db=(frand()+0.1f)*0.1f;
					float	ar0=frand()*100.f;
					float	ag0=frand()*100.f;
					float	ab0=frand()*100.f;
					float	dr0=(frand()+0.1f)*0.09f;
					float	dg0=(frand()+0.1f)*0.09f;
					float	db0=(frand()+0.1f)*0.09f;
					float	ar1=frand()*100.f;
					float	ag1=frand()*100.f;
					float	ab1=frand()*100.f;
					float	dr1=(frand()+0.1f)*0.03f;
					float	dg1=(frand()+0.1f)*0.03f;
					float	db1=(frand()+0.1f)*0.03f;
					float	ar10=frand()*100.f;
					float	ag10=frand()*100.f;
					float	ab10=frand()*100.f;
					float	dr10=(frand()+0.1f)*0.01f;
					float	dg10=(frand()+0.1f)*0.01f;
					float	db10=(frand()+0.1f)*0.01f;
					float	n=1.f/(frand()*512.f+16.f);
					for(i=0; i<1024; i++)
					{
						float	m=(float)sin(PI*(float)i*n)*0.4999f+0.5f;
						int		r=(int)((1.f-m)*((float)sin(ar1)*sin(ar10)*127.9f+128.f)+m*((float)sin(ar)*sin(ar0)*127.9f+128.f))&255;
						int		g=(int)((1.f-m)*((float)sin(ag1)*sin(ag10)*127.9f+128.f)+m*((float)sin(ag)*sin(ag0)*127.9f+128.f))&255;
						int		b=0;//(int)((1.f-m)*((float)sin(ab1)*sin(ab10)*127.9f+128.f)+m*((float)sin(ab)*sin(ab0)*127.9f+128.f))&255;
						color[i]=r<<16|g<<8|b;
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
			{
				float	*pol=ol;
				for(x=0; x<w; x++)
					*(pol++)=calc(xx, yy, zz, x,-1);
			}
			switch(1)
			{
				case 0:
				for(y=0; y<h; y++)
				{
					float	*pnl=nl;
					float	*pol=ol;
					float	lp=calc(xx, yy, zz, -1, y);
					for(x=0; x<w; x++)
					{
						float	z=calc(xx, yy, zz, x, y);
						int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
						pixel(x, y, mcolor1(color[(int)z], l));
						*(pnl++)=z;
						lp=z;
					}
					memcpy(ol, nl, w*sizeof(float));
				}
				break;

				case 1:
				for(y=0; y<h; y++)
				{
					float	*pnl=nl;
					float	*pol=ol;
					float	lp=calc(xx, yy, zz, -1, y);
					for(x=0; x<w; x++)
					{
						float	z=calc(xx, yy, zz, x, y);
						int		l=(int)(sin(atan2(z-*(pol++),z-lp))*127.99f+128.f);
						pixel(x, y, mcolor2(color[(int)z], l));
						*(pnl++)=z;
						lp=z;
					}
					memcpy(ol, nl, w*sizeof(float));
				}
				break;
			}
			if(ol)
				free(ol);
			if(nl)
				free(nl);
		}
		boxfa(0, 0, w, h, 0xff404000, 0.3f);
		break;

		case 3:
		{
			int	i,j;
			for(i=0; i<h; i++)
			{
				for(j=0; j<w; j++)
				{
					int n=((i+(w-j))>>4)+80;
					pixel(j, i, ((n-30)<<16)|((n)<<8)|(n-20));
				}
			}
		}
		break;

		case 4:
		{
			int	i,j;
			for(i=0; i<h; i++)
			{
				for(j=0; j<w; j++)
				{
					int n=((i+(w-j))>>4)+80;
					pixel(j, i, (n<<16)|((n)<<8)|(n-20));
				}
			}
		}
		break;

		case 5:
		boxf(0, 0, w, h, 0xffFFAA00);
		break;

		case 6:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE001), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;

		case 7:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE002), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;

		case 8:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE003), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;

		case 9:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE004), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;

		case 10:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE005), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;

		case 11:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE006), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;

		case 12:
		{
			Abitmap	*b=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_TEXTURE007), "PNG"));
			if(b)
			{
				int	x,y;
				for(y=0; y<h; y+=b->h)
					for(x=0; x<w; x+=b->w)
						set(x, y, b, bitmapDEFAULT, bitmapDEFAULT);
				delete(b);
			}
		}
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
