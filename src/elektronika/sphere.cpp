/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SPHERE.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>

#include						"sphere.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AsphereInfo::CI		= ACI("AsphereInfo",	GUID(0x11111112,0x00000120), &AeffectInfo::CI, 0, NULL);
ACI								Asphere::CI			= ACI("Asphere",		GUID(0x11111112,0x00000121), &Aeffect::CI, 0, NULL);
ACI								AsphereFront::CI	= ACI("AsphereFront",	GUID(0x11111112,0x00000122), &AeffectFront::CI, 0, NULL);
ACI								AsphereBack::CI		= ACI("AsphereBack",	GUID(0x11111112,0x00000123), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							ANGLE							1024

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _polar
{
	double						r;
	double						a;
} Tpolar;

class ApolarTab
{
public:
								ApolarTab						(int w, int h);
	virtual						~ApolarTab						();
	
	void						calcTab							();

	int							w,h;
	Tpolar						*tab;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ApolarTab::ApolarTab(int w, int h)
{
	int size=sizeof(Tpolar)*w*h;
	this->w=w;
	this->h=h;
	tab=(Tpolar *)malloc(size);
	calcTab();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ApolarTab::~ApolarTab()
{
	free(tab);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ApolarTab::calcTab()
{
	Tpolar	*t=tab;
	int		x,y;
	int		mw=w>>1;
	int		mh=h>>1;
	
	for(y=0; y<h; y++)
	{
		int	dy=y-mh;
		for(x=0; x<w; x++)
		{
			int		dx=x-mw;
			t->r=sqrt((double)(dx*dx+dy*dy));
			t->a=atan2((double)dy, (double)dx);
			t++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!count)
	{
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asphere::Asphere(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init();

	afade0=0.f;
	amplanar=0.f;
	polar=null;
	trr=null;
	taa=null;
	tra=null;
	tar=null;
	txx=null;
	tyy=null;
	txy=null;
	tyx=null;
	quart=NULL;
	mirror=NULL;
	front=new AsphereFront(qiid, "sphere front", this, 120);
	front->setTooltips("sphere module");
	back=new AsphereBack(qiid, "sphere module", this, 120);
	back->setTooltips("sphere module");
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asphere::~Asphere()
{
	if(quart)
		delete(quart);
	if(mirror)
		delete(mirror);
	if(trr)
		free(trr);
	if(taa)
		free(taa);
	if(tra)
		free(tra);
	if(tar)
		free(tar);
	if(txx)
		free(txx);
	if(tyy)
		free(tyy);
	if(txy)
		free(txy);
	if(tyx)
		free(tyx);
	if(polar)
		delete(polar);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Asphere::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Asphere::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asphere::settings(bool emergency)
{
	int		w=getVideoWidth();
	int		h=getVideoHeight();
	float	dw=(float)(w>>1);
	float	dh=(float)(h>>1);
	bool	changed=polar?((polar->w!=w)||(polar->h!=h)):true;
	
	if(quart)
		quart->size(w>>1, h>>1);
	else
		quart=new Abitmap(w>>1, h>>1);
		
	if(mirror)
		mirror->size(w, h);
	else
		mirror=new Abitmap(w, h);
		
	if(polar)
	{
		if((polar->w!=w)||(polar->h!=h))
		{
			delete(polar);
			polar=new ApolarTab(w, h);
		}
	}
	else
		polar=new ApolarTab(w, h);
		
	if(changed)
	{
		int raymax=(int)sqrt(dw*dw+dh*dh)+1;

		if(trr)
			free(trr);
		if(taa)
			free(taa);
		if(tra)
			free(tra);
		if(tar)
			free(tar);
		if(txx)
			free(txx);
		if(tyy)
			free(tyy);
		if(txy)
			free(txy);
		if(tyx)
			free(tyx);
			
		taa=(double *)malloc(sizeof(double)*ANGLE);
		memset(taa, 0, sizeof(double)*ANGLE);
		trr=(double *)malloc(sizeof(double)*raymax);
		memset(trr, 0, sizeof(double)*raymax);
		tra=(double *)malloc(sizeof(double)*ANGLE);
		memset(tra, 0, sizeof(double)*ANGLE);
		tar=(double *)malloc(sizeof(double)*raymax);
		memset(tar, 0, sizeof(double)*raymax);
		txx=(double *)malloc(sizeof(double)*w);
		memset(txx, 0, sizeof(double)*w);
		tyy=(double *)malloc(sizeof(double)*h);
		memset(tyy, 0, sizeof(double)*h);
		txy=(double *)malloc(sizeof(double)*h);
		memset(txy, 0, sizeof(double)*h);
		tyx=(double *)malloc(sizeof(double)*w);
		memset(tyx, 0, sizeof(double)*w);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asphere::action(double time, double dtime, double beat, double dbeat)
{
	int				w=getVideoWidth();
	int				h=getVideoHeight();
	int				mw=w>>1;
	int				mh=h>>1;
	AsphereFront	*front=(AsphereFront *)this->front;
	Avideo			*out=((AsphereBack *)back)->out;
	Avideo			*in=((AsphereBack *)back)->in[front->source->get()];
	out->enter(__FILE__,__LINE__);
	in->enter(__FILE__,__LINE__);
	{
		Abitmap	*bo=out->getBitmap();
		Abitmap	*bi=in->getBitmap();
		if(bo)
		{
			Asample	*snd=((AsphereBack *)back)->ins;
			float	fade0=(afade0=afade0*0.2f+(1.f-front->fade0->get())*0.8f);
			float	level=(front->level->get()-0.5f)*2.f;
			float	sndlvl=(front->sndlvl->get()-0.5f)*2.f;
			float	mplanar=(amplanar=amplanar*0.2f+(1.f-front->mplanar->get())*0.8f);
			float	eplanar=(front->eplanar->get()-0.5f)*2.f;
			float	splanar=(front->splanar->get()-0.5f)*2.f;
			float	master=front->master->get();
			float	sample[1024][2];
			float	smpmax=0.f;
			
			memset(sample, 0, sizeof(sample));

			if(snd->isConnected())
			{
				sword	smp[513*2];
				int		avaible,sget;
				snd->enter(__FILE__,__LINE__);
				avaible=snd->avaible();
				avaible-=snd->skip(maxi((avaible-1024)>>1, 0));
				sget=snd->getBuffer(smp, NULL, NULL, NULL, 513);
				snd->leave();
				if(sget==513)
				{
					int		i;
					sword	*s=smp;
					for(i=0; i<512; i++)
					{
						int		ii=i<<1;
						int		ii2=ii+1;
						float	v0=(float)s[ii]/32768.f;
						float	v1=(float)s[ii2]/32768.f;
						smpmax=maxi(smpmax, sample[ii][0]=v0);
						smpmax=maxi(smpmax, sample[ii][1]=v1);
						v0=(v0*0.5f)+((float)s[ii+2]/32768.f)*0.5f;
						v1=(v0*0.5f)+((float)s[ii2+2]/32768.f)*0.5f;
						smpmax=maxi(smpmax, sample[ii2][0]=v0);
						smpmax=maxi(smpmax, sample[ii2][1]=v1);
					}
				}
			}
			
			{	// mirrors
				bool	horz=front->horz->get();
				bool	vert=front->vert->get();
				bool	diag=front->diag->get();
				bool	cross=front->cross->get();
				float	xx,yy;

				front->zone->get(&xx, &yy);

				if(bi)
				{
					if(diag)
					{
						int	x=(int)(xx*mw);
						int	y=(int)(yy*mh);
						quart->set(0, 0, x, y, mw, mh, bi, bitmapNORMAL, bitmapNORMAL);
						fdiag(quart, cross);
						fset(mirror,  0,  0, mw, mh, quart, 0, 0, vert, horz);
						fset(mirror, mw,  0, mw, mh, quart, 0, 0, !vert, horz);
						fset(mirror,  0, mh, mw, mh, quart, 0, 0, vert, !horz);
						fset(mirror, mw, mh, mw, mh, quart, 0, 0, !vert, !horz);
					}
					else if(cross)
					{
						int	x=(int)(xx*mw);
						int	y=(int)(yy*mh);
						fset(mirror,  0,  0, mw, mh, bi, x, y, vert, horz);
						fset(mirror, mw,  0, mw, mh, bi, x, y, !vert, horz);
						fset(mirror,  0, mh, mw, mh, bi, x, y, vert, !horz);
						fset(mirror, mw, mh, mw, mh, bi, x, y, !vert, !horz);
					}
					else if(vert)
					{
						int	x=(int)(xx*mw);
						if(horz)
						{
							fset(mirror,  0,  0, mw, h, bi, x, 0, true, false);
							fset(mirror, mw,  0, mw, h, bi, x, 0, false, false);
						}
						else
						{
							fset(mirror,  0,  0, mw, h, bi, x, 0, false, false);
							fset(mirror, mw,  0, mw, h, bi, x, 0, true, false);
						}
					}
					else if(horz)
					{
						int	y=(int)(yy*mh);
						fset(mirror,  0,  0, w, mh, bi, 0, y, false, false);
						fset(mirror,  0, mh, w, mh, bi, 0, y, false, true);
					}
					else
					{
						mirror->set(0, 0, bi, bitmapNORMAL, bitmapNORMAL);
					}
				}
			}
			if(master)
			{
				bo->alpha=master;
				mirror->set(0, 0, bo, bitmapNORMAL, bitmapGLOBALALPHA);
			}
			
			//bo->set(0, 0, mirror, bitmapNORMAL, bitmapNORMAL);	// 4 test
			
			
			{
				dword	*d=&bo->body32[0];
				Tpolar	*t=polar->tab;
				int		x,y;
				float	mw=(float)(w>>1);
				float	mh=(float)(h>>1);
				int		raymax=(int)sqrt(mw*mw+mh*mh)+1;
				
				{
					double	rotat=(((pow(level*(0.5-front->rotat->get())*3.0, 3.0)*PI*1.037)));
					double	twirl=pow(level*(0.5-front->twirl->get())*4.0, 3.0)/raymax;
					double	*d=tar;
					int		i;
					for(i=0; i<raymax; i++)
						*(d++)+=(double)i*twirl+rotat;
				}
				
				
				{
					double	pinch=pow(level*(0.5-front->pinch->get())*8.0, 3.0)*0.03;
					double	pinchfreq=PI/(raymax*1.5);
					double	zoom=pow(level*(0.5-front->zoom->get())*8.0, 3.0)*0.1;
					double	szoom=pow(sndlvl*(0.5-front->sndflat->get())*8.0, 3.0)*0.1*smpmax;
					double	sblur=pow(sndlvl*(front->sndblur->get()-0.5)*3.0, 3.0)*10.0;
					double	*d=trr;
					int		i;
					for(i=0; i<raymax; i++)
						*(d++)+=(double)i*zoom+(double)i*cos(pinchfreq*(double)i)*pinch+(double)i*szoom+(double)i*sample[i&1023][1]*sblur;
				}
				
				{
					double	sinamp=pow(level*(front->sinamp->get()-0.5)*8.0, 3.0)*0.0001*ANGLE;
					double	samp=pow(sndlvl*(front->sndsize->get()-0.5)*8.0, 3.0)*0.0004*ANGLE;
					double	smp[1024];
					double	*d=taa;
					int		i;
					for(i=0; i<256; i++)
					{
						double	ii=(double)(256-i)/256.0;
						smp[1023-i]=smp[i]=sample[i][0]*ii;
						smp[512+i]=smp[511-i]=-sample[i][0]*ii;
					}
					for(i=0; i<ANGLE; i++)
						*(d++)+=sin((4*PI*(double)i)/(double)ANGLE)*sinamp+samp*smp[i&1023];
				}
				
				{
					double	rosace=pow(level*(front->rosace->get()-0.5)*8.0, 3.0)*0.02*raymax;
					double	sros=pow(sndlvl*(front->sndpow->get()-0.5)*8.0, 3.0)*0.04*raymax;
					double	*d=tra;
					int		i;
					for(i=0; i<ANGLE; i++)
					{
						int	n=((i<512)?i:(1023-i))&511;
						n=(n<256)?n:(511-n);
						*(d++)+=cos((8.0*PI*(double)i)/(double)ANGLE)*rosace+sample[n][0]*sros;
					}
				}
				
				if(fade0>0.f)
				{
					double	fi=1.0-pow((double)fade0, 4.0);
					double	*dar=tar;
					double	*drr=trr;
					double	*dra=tra;
					double	*daa=taa;
					int		i;
					for(i=0; i<raymax; i++)
					{
						*dar=(*dar)*fi;
						*drr=(*drr)*fi;
						dar++;
						drr++;
					}
					for(i=0; i<ANGLE; i++)
					{
						*dra=(*dra)*fi;
						*daa=(*daa)*fi;
						dra++;
						daa++;
					}
				}
				
				{				
					double	se1=pow(eplanar*(front->e1->get()-0.5)*8.0, 3.0)*0.01*w;
					double	se2=pow(eplanar*(front->e2->get()-0.5)*8.0, 3.0)*0.04*w;
					double	se3=pow(eplanar*(front->e3->get()-0.5)*8.0, 3.0)*0.04*w;
					double	sa1=pow(splanar*(front->s1->get()-0.5)*8.0, 3.0)*0.01*w;
					double	sa2=pow(splanar*(front->s2->get()-0.5)*8.0, 3.0)*0.04*w;
					double	sa3=pow(splanar*(front->s3->get()-0.5)*8.0, 3.0)*0.04*w;
					int		mw=w>>1;
					int		mh=h>>1;

					{
						double	*d=tyx;
						double	fe1=4*PI/w;
						int		n=512-mw;
						int		i;
						for(i=0; i<w; i++)
						{
							*(d++)+=cos(fe1*i)*se1+sample[n++][0]*sa1;
						}
					}
					{
						double	*d=txy;
						double	fe1=4*PI/h;
						int		n=512-mh;
						int		i;
						for(i=0; i<h; i++)
						{
							*(d++)+=cos(fe1*i)*se1+sample[n++][1]*sa1;
						}
					}
					{
						double	*d=txx;
						double	fe1=2*PI;
						int		n=512-mw;
						int		i;
						for(i=0; i<w; i++)
						{
							int		i0;
							double	dir;
							if(i<mw)
							{
								i0=mw-i;
								dir=-1.0;
							}
							else
							{
								i0=i-mw;
								dir=1.0;
							}
							double	ii=(double)(i0)/(double)mw;
							*(d++)+=ii*dir*(sin(fe1*ii)*se2+sample[i0][0]*sa2+ii*se3+ii*sa3*smpmax);
						}
					}
					{
						double	*d=tyy;
						double	fe1=2*PI;
						int		n=512-mh;
						int		i;
						for(i=0; i<h; i++)
						{
							int		i0;
							double	dir;
							if(i<mh)
							{
								i0=mh-i;
								dir=-1.0;
							}
							else
							{
								i0=i-mh;
								dir=1.0;
							}
							double	ii=(double)(i0)/(double)mh;
							*(d++)+=ii*dir*(sin(fe1*ii)*se2+sample[i0][1]*sa2+ii*se3+ii*sa3*smpmax);
						}
					}
				}
				
				if(mplanar>0.f)
				{
					double	fi=1.0-pow((double)mplanar, 4.0);
					double	*dxx=txx;
					double	*dyy=tyy;
					double	*dxy=txy;
					double	*dyx=tyx;
					int		i;
					for(i=0; i<w; i++)
					{
						*dxx=(*dxx)*fi;
						*dyx=(*dyx)*fi;
						dxx++;
						dyx++;
					}
					for(i=0; i<h; i++)
					{
						*dyy=(*dyy)*fi;
						*dxy=(*dxy)*fi;
						dyy++;
						dxy++;
					}
				}
				
				if((fade0!=1.f)&&(mplanar!=1.f))
				{
					double	*dxy=txy;
					double	*dyy=tyy;
					
					for(y=0; y<h; y++)
					{
						double	*dxx=txx;
						double	*dyx=tyx;
						
						double	dx=*(dxy++);
						double	dy=*(dyy++);
						
						for(x=0; x<w; x++)
						{
							double	a=t->a;
							double	r=t->r;
							int		ai=(int)ceil(a*1024.f/(2.f*PI))&1023;
							int		ri=(int)ceil(r);
							double	ra=r+tra[ai]*r/raymax;
							double	ar=a+tar[ri];
							double	rr=ra+trr[ri];
							double	aa=ar+taa[ai];
							
							double	fx=(double)(mw+cos(aa)*rr);
							double	fy=(double)(mh+sin(aa)*rr);
							
							int		xx=((int)(ceil(fx)+dx+*(dxx++)))%w;
							int		yy=((int)(ceil(fy)+dy+*(dyx++)))%h;
							
							if(xx<0)
								xx+=w;
							if(yy<0)
								yy+=h;

							*(d++)=mirror->body32[mirror->adr[yy]+xx];
								
							t++;
						}
					}
				}
				else if(fade0!=1.f)
				{
					for(y=0; y<h; y++)
					{
						for(x=0; x<w; x++)
						{
							double	a=t->a;
							double	r=t->r;
							int		ai=(int)ceil(a*1024.f/(2.f*PI))&1023;
							int		ri=(int)ceil(r);
							double	ra=r+tra[ai]*r/raymax;
							double	ar=a+tar[ri];
							double	rr=ra+trr[ri];
							double	aa=ar+taa[ai];
							
							double	fx=(double)(mw+cos(aa)*rr);
							double	fy=(double)(mh+sin(aa)*rr);
							
							int		xx=(int)(ceil(fx))%w;
							int		yy=(int)(ceil(fy))%h;
							
							if(xx<0)
								xx+=w;
							if(yy<0)
								yy+=h;

							*(d++)=mirror->body32[mirror->adr[yy]+xx];
								
							t++;
						}
					}
				}
				else if(mplanar!=1.f)
				{
					double	*dxy=txy;
					double	*dyy=tyy;
					
					for(y=0; y<h; y++)
					{
						double	*dxx=txx;
						double	*dyx=tyx;
						
						double	dx=*(dxy++);
						double	dy=*(dyy++);
						
						for(x=0; x<w; x++)
						{
							int		xx=((int)((double)x+dx+*(dxx++)))%w;
							int		yy=((int)((double)y+dy+*(dyx++)))%h;
							
							if(xx<0)
								xx+=w;
							if(yy<0)
								yy+=h;

							*(d++)=mirror->body32[mirror->adr[yy]+xx];
						}
					}
				}
				else
				{
					bo->set(0, 0, mirror, bitmapNORMAL, bitmapNORMAL);
				}
			}
		}
	}
	in->leave();
	out->leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asphere::fset(Abitmap *bd, int xd, int yd, int w, int h, Abitmap *bs, int xs, int ys, bool flipX, bool flipY)
{
	int	dx=flipX?-1:1;
	int	dy=flipY?-1:1;
	int	i,j;
	xs+=flipX?(w-1):0;
	ys+=flipY?(h-1):0;
	for(j=0; j<h; j++)
	{
		dword	*s=&bs->body32[bs->adr[ys]+xs];
		dword	*d=&bd->body32[bd->adr[yd]+xd];
		for(i=0; i<w; i++)
		{
			*d=*s;
			s+=dx;
			d++;
		}
		ys+=dy;
		yd++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asphere::fdiag(Abitmap *bd, bool inv)
{
	int	w=bd->w;
	int	h=bd->h;
	int	yd=(h<<16)/w;
	int	xd=(w<<16)/h;
	int	x=0;
	int y=0;
	if(inv)
	{
		for(x=0; x<w; x++)
		{
			int		y0;
			int		x0=x<<16;
			dword	*s=&bd->body32[bd->adr[y>>16]];
			dword	*d=&bd->body32[bd->adr[y>>16]+x];

			for(y0=(y>>16); y0<h; y0++)
			{
				*d=s[x0>>16];
				d+=w;
				x0+=xd;
			}

			y+=yd;
		}
	}
	else
	{
		for(x=0; x<w; x++)
		{
			int		y0=y;
			int		x0;

			dword	*s=&bd->body32[x];
			dword	*d=&bd->body32[bd->adr[y0>>16]+x];

			for(x0=x; x0<w; x0++)
			{
				*(d++)=s[(y0>>16)*w];
				y0+=yd;
			}

			y+=yd;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsphereFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsphereFront::AsphereFront(QIID qiid, char *name, Asphere *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_SPHERE), "PNG");
	back=new Abitmap(&o);

	source=new Aselect(MKQIID(qiid, 0x234ffa157ce3372c), "source", this, 15, 14, 2, 2, &resource.get(MAKEINTRESOURCE(PNG_SPHERE_SOURCES), "PNG"), 16, 16);
	source->setTooltips("source selector");
	source->set(0);
	source->show(TRUE);

	master=new Apaddle(MKQIID(qiid, 0x0bb9c310010dc1d0), "master feedback", this, 16, 60, paddleYOLI32);
	master->setTooltips("master feedback");
	master->set(0.f);
	master->show(TRUE);
	
	///////////
	///////////	polar
	///////////

	cross=new ActrlButton(MKQIID(qiid, 0xac25e736b3138000), "mirror/cross", this, 71, 14, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_SPHERE_CROSS), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	cross->setTooltips("cross mirror");
	cross->show(TRUE);

	diag=new ActrlButton(MKQIID(qiid, 0x2f5d052b82b13c7c), "mirror/diag", this, 89, 14, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_SPHERE_DIAG), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	diag->setTooltips("diagonal mirror");
	diag->show(TRUE);

	vert=new ActrlButton(MKQIID(qiid, 0xa9b490555c57fa74), "mirror/vertical", this, 71, 32, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_SPHERE_VERT), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	vert->setTooltips("vertical mirror");
	vert->show(TRUE);

	horz=new ActrlButton(MKQIID(qiid, 0x0f63977345322e46), "mirror/horizontal", this, 89, 32, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_SPHERE_HORZ), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	horz->setTooltips("horizontal mirror");
	horz->show(TRUE);

	zone=new Azone(MKQIID(qiid, 0xf34542ea5e9a1f90), "mirror/position", this, 71, 60, 34, 34, 0.5f, 0.5f);
	zone->setTooltips("position - used only for diagonal and cross mirror");
	zone->show(TRUE);
	
	///////////

	rosace=new Apaddle(MKQIID(qiid, 0xa7bd0ec11e736180), "polar/rosace", this, 184, 52, paddleYOLI16);
	rosace->setTooltips("rosace");
	rosace->set(.5f);
	rosace->show(TRUE);

	sinamp=new Apaddle(MKQIID(qiid, 0x713042a6cbd7e5e8), "polar/circle waves", this, 128, 52, paddleYOLI16);
	sinamp->setTooltips("circle waves");
	sinamp->set(0.5f);
	sinamp->show(TRUE);

	pinch=new Apaddle(MKQIID(qiid, 0xa7b8cd5e1e736180), "polar/punch-pinch", this, 156, 12, paddleYOLI16);
	pinch->setTooltips("punch-pinch");
	pinch->set(.5f);
	pinch->show(TRUE);

	zoom=new Apaddle(MKQIID(qiid, 0xb1fe4f2f25daaa88), "polar/zoom", this, 156, 28, paddleYOLI16);
	zoom->setTooltips("zoom");
	zoom->set(.5f);
	zoom->show(TRUE);

	rotat=new Apaddle(MKQIID(qiid, 0x0392c3ee8f0576b8), "polar/rotation", this, 156, 76, paddleYOLI16);
	rotat->setTooltips("rotation");
	rotat->set(.5f);
	rotat->show(TRUE);

	twirl=new Apaddle(MKQIID(qiid, 0xa7381264b65ef140), "polar/twirl", this, 156, 92, paddleYOLI16);
	twirl->setTooltips("twirl");
	twirl->set(.5f);
	twirl->show(TRUE);

	level=new Apaddle(MKQIID(qiid, 0xc83cd4df37108db0), "polar/effect direction", this, 152, 48, paddleYOLI24);
	level->setTooltips("effect direction");
	level->set(0.5f);
	level->show(TRUE);

	///////////

	fade0=new Apaddle(MKQIID(qiid, 0xc83c0a50d5e88db0), "polar/master", this, 192, 76, paddleYOLI32);
	fade0->setTooltips("polar master");
	fade0->set(0.5f);
	fade0->show(TRUE);

	///////////

	sndblur=new Apaddle(MKQIID(qiid, 0x7f75dfdb1c206082), "polar/audio wave", this, 244, 76, paddleYOLI16);
	sndblur->setTooltips("polar/audio wave");
	sndblur->set(0.5f);
	sndblur->show(TRUE);
	
	sndflat=new Apaddle(MKQIID(qiid, 0x85b478c639cf26a8), "polar/audio zoom", this, 244, 28, paddleYOLI16);
	sndflat->setTooltips("audio zoom");
	sndflat->set(0.5f);
	sndflat->show(TRUE);

	sndpow=new Apaddle(MKQIID(qiid, 0x48584e896493b200), "polar/audio pulse", this, 216, 52, paddleYOLI16);
	sndpow->setTooltips("audio pulse");
	sndpow->set(0.5f);
	sndpow->show(TRUE);
	
	sndsize=new Apaddle(MKQIID(qiid, 0xa03c88dbbdcf5a20), "audio fly", this, 272, 52, paddleYOLI16);
	sndsize->setTooltips("audio fly");
	sndsize->set(0.5f);
	sndsize->show(TRUE);

	sndlvl=new Apaddle(MKQIID(qiid, 0xd74c4732551eb9c8), "polar/audio direction", this, 240, 48, paddleYOLI24);
	sndlvl->setTooltips("audio direction");
	sndlvl->set(0.5f);
	sndlvl->show(TRUE);

	///////////
	///////////	planar
	///////////

	e1=new Apaddle(MKQIID(qiid, 0xa7395a2ab65ef140), "planar/ripple", this, 312, 52, paddleYOLI16);
	e1->setTooltips("e1");
	e1->set(.5f);
	e1->show(TRUE);

	e2=new Apaddle(MKQIID(qiid, 0xb18855e2d5eaaa88), "planar/deform", this, 340, 28, paddleYOLI16);
	e2->setTooltips("deform");
	e2->set(.5f);
	e2->show(TRUE);

	e3=new Apaddle(MKQIID(qiid, 0xa7bd095005ed6180), "planar/zoom", this, 368, 52, paddleYOLI16);
	e3->setTooltips("zoom (special)");
	e3->set(.5f);
	e3->show(TRUE);

	eplanar=new Apaddle(MKQIID(qiid, 0xc8963a2f37108db0), "planar/effect direction", this, 336, 48, paddleYOLI24);
	eplanar->setTooltips("effect direction");
	eplanar->set(0.5f);
	eplanar->show(TRUE);

	///////////

	mplanar=new Apaddle(MKQIID(qiid, 0xc83850d95aa1adb0), "planar/master", this, 376, 76, paddleYOLI32);
	mplanar->setTooltips("planar master");
	mplanar->set(0.5f);
	mplanar->show(TRUE);

	///////////

	s1=new Apaddle(MKQIID(qiid, 0xa038c65ebdcf5a20), "planar/audio wave", this, 456, 52, paddleYOLI16);
	s1->setTooltips("audio wave");
	s1->set(0.5f);
	s1->show(TRUE);

	s2=new Apaddle(MKQIID(qiid, 0x85562ed05ed5e6a8), "planar/audio stretch", this, 428, 28, paddleYOLI16);
	s2->setTooltips("audio stretch");
	s2->set(0.5f);
	s2->show(TRUE);

	s3=new Apaddle(MKQIID(qiid, 0x4850525e55d0e200), "planar/audio pulse", this, 400, 52, paddleYOLI16);
	s3->setTooltips("audio pulse");
	s3->set(0.5f);
	s3->show(TRUE);
	
	splanar=new Apaddle(MKQIID(qiid, 0xd74c96e5d2e0b9c8), "planar/audio direction", this, 424, 48, paddleYOLI24);
	splanar->setTooltips("audio direction");
	splanar->set(0.5f);
	splanar->show(TRUE);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsphereFront::~AsphereFront()
{
	delete(back);
	delete(master);
	delete(horz);
	delete(vert);
	delete(diag);
	delete(cross);
	delete(zone);
	delete(pinch);
	delete(rosace);
	delete(twirl);
	delete(rotat);
	delete(zoom);
	delete(level);
	delete(fade0);
	delete(sinamp);
	delete(sndlvl);
	delete(sndpow);
	delete(sndblur);
	delete(sndsize);
	delete(sndflat);
	
	delete(mplanar);
	delete(eplanar);
	delete(e1);
	delete(e2);
	delete(e3);
	delete(splanar);
	delete(s1);
	delete(s2);
	delete(s3);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsphereFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		break;

		case nyCLICK:
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsphereFront::pulse()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID qiidsph[MAXSPHEREINPUT]=
{ 0xa1662dfce4e8c8a0, 0xadcce325ca8847f0, 0x323d922b0f5a2dc0, 0x3235484a84cdedc0};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsphereBack::AsphereBack(QIID qiid, char *name, Asphere *e, int h) : AeffectBack(qiid, name, e, h)
{
	int		i;
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_SPHERE2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0xf326c7753728b340),  "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);

	for(i=0; i<MAXSPHEREINPUT; i++)
	{
		char	name[128];
		sprintf(name, "video input #%d", i);
		in[i]=new Avideo(MKQIID(qiid, qiidsph[i]), name, this, pinIN, 10+i*20, 10);
		in[i]->setTooltips(name);
		in[i]->show(TRUE);
	}

	ins=new Asample(MKQIID(qiid, 0xab6f52d1db078300), "audio input", this, pinIN, 10, 30);
	ins->setTooltips("audio input");
	ins->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AsphereBack::~AsphereBack()
{
	int	i;
	delete(out);
	delete(ins);
	delete(back);
	for(i=0; i<MAXSPHEREINPUT; i++)
		delete(in[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AsphereBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AsphereInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Asphere(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * sphereGetInfo()
{
	return new AsphereInfo("sphereInfo", &Asphere::CI, "sphere", "sphere module - coordinate translation (morph)");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
