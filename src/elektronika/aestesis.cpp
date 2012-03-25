/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AESTESIS.CPP				(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<dsound.h>
#include						<math.h>
#include						"aestesis.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AaestesisInfo::CI	= ACI("AaestesisInfo",	GUID(0x11111112,0x00000010), &AeffectInfo::CI, 0, NULL);
ACI								Aaestesis::CI		= ACI("Aaestesis",		GUID(0x11111112,0x00000011), &Aeffect::CI, 0, NULL);
ACI								AaestesisFront::CI	= ACI("AaestesisFront",	GUID(0x11111112,0x00000012), &AeffectFront::CI, 0, NULL);
ACI								AaestesisBack::CI	= ACI("AaestesisBack",	GUID(0x11111112,0x00000013), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
enum
{
	sourceTRASH=0,
	sourceZYGO,
	sourceIMAGE
};

enum
{
	sourceTRASH0=0,
	sourceTRASH1,
	sourceTRASHPOL,
	sourceTRASHR0
};

enum
{
	colorNB=0,
	colorNBSOUND,
	colorTRASH,
	colorHOUSE
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static byte				*tatan=NULL;
static int				count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!tatan)
	{
		tatan=(byte *)malloc(512*512);
		{
			byte	*d=tatan;
			int		i,j;
			for(i=0; i<512; i++)
			{
				int	dy=i-256;
				for(j=0; j<512; j++)
				{
					int		dx=j-256;
					//float	v=atan2(dy, dx);
					tatan[(dx&511)|((dy&511)<<9)]=(byte)((atan2((float)dy, (float)dx)*127.999/PI)+128);
				}
			}
		}
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		free(tatan);
		tatan=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aaestesis::Aaestesis(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	front=new AaestesisFront(qiid, "aestesis front", this, 160);
	front->setTooltips("aestesis front");
	back=new AaestesisBack(qiid, "aestesis back", this, 160);
	back->setTooltips("aestesis back");
	sourceType=sourceTRASH;
	oldSourceType=-1;
	colorType=colorNB;
	oldColorType=-1;
	trashType=sourceTRASH0;
	oldTrashType=-1;
	cycle=0.5f;
	cycleval=0;
	hauteurs=NULL;
	hauteurs2=NULL;
	init();
	settings(false);

	nbZygo=1;
	vZygo=0.0001f;
	zZygo=0.1f;

	memset(msmp, 0, sizeof(msmp));
	memset(mbas, 0, sizeof(mbas));
	memset(mmed, 0, sizeof(mmed));
	memset(mtre, 0, sizeof(mtre));
	memset(ssmp, 0, sizeof(ssmp));
	memset(sbas, 0, sizeof(sbas));
	memset(smed, 0, sizeof(smed));
	memset(stre, 0, sizeof(stre));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aaestesis::~Aaestesis()
{
	if(hauteurs)
		free(hauteurs);
	if(hauteurs2)
		free(hauteurs2);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aaestesis::save(class Afile *f)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aaestesis::settings(bool emergency)
{
	if(hauteurs)
		free(hauteurs);
	if(hauteurs2)
		free(hauteurs2);
	hauteurs=(byte *)malloc(getVideoWidth()*getVideoHeight());
	hauteurs2=(byte *)malloc(getVideoWidth()*getVideoHeight());
	oldColorType=-1;
	oldSourceType=-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aaestesis::action(double time, double dtime, double beat, double dbeat)
{
	AaestesisFront	*front=(AaestesisFront *)this->front;

	this->sourceType=front->source->get();
	this->trashType=front->trash->get();
	this->nbZygo=(int)((front->nbZygo)->get()*10.9f)+1;
	this->vZygo=front->vZygo->get()+0.0001f;
	this->zZygo=front->zZygo->get()+0.1f;
	this->colorType=front->color->get();
	this->cycle=front->cycle->get();

	int				w=getVideoWidth();
	int				h=getVideoHeight();
	int				mw=w>>1;
	int				mh=h>>1;
	Avideo			*vin=((AaestesisBack *)back)->vin;
	Asample			*snd=((AaestesisBack *)back)->in;
	Avideo			*out=((AaestesisBack *)back)->out;
	int				sourceType=this->sourceType;
	int				colorType=this->colorType;
	int				trashType=this->trashType;
	sword			smp[256*2];
	sword			bas[256*2];
	sword			med[256*2];
	sword			tre[256*2];
	dword			pal[256];
	int				avaible=0,sget=0;
	float			smooth=((AaestesisFront *)front)->smooth->get();
	float			delay=((AaestesisFront *)front)->delay->get();
	float			level=((AaestesisFront *)front)->level->get();
	float			speed=((AaestesisFront *)front)->speed->get();

	if(snd->isConnected())
	{
		snd->enter(__FILE__,__LINE__);
		avaible=snd->avaible();
		avaible-=snd->skip(maxi(avaible-128, 0));
		sget=snd->getBuffer(smp, bas, med, tre, 128);
		snd->leave();
		if((sget<128)&&(sget>0))
		{
			int	dx=(sget<<16)>>7;
			int	x=(dx<<7)-1;
			int	i;
			for(i=127; i>=0; i--)
			{
				int	s=i<<1;
				int	d=(x>>15)&(~1);
				smp[s]=smp[d];
				smp[s+1]=smp[d+1];
				bas[s]=bas[d];
				bas[s+1]=bas[d+1];
				med[s]=med[d];
				med[s+1]=med[d+1];
				tre[s]=tre[d];
				tre[s+1]=tre[d+1];
				x-=dx;
			}
		}
		if(speed!=0.f)
		{
			int	n=(int)((pow(1.f-speed, 2))*127.f+1.f);
			int	n0=128-n;
			int	i;
			for(i=127; i>=n; i--)
			{
				int	d=i<<1;
				int	s=(i-n)<<1;
				ssmp[d]=ssmp[s];
				ssmp[d+1]=ssmp[s+1];
				sbas[d]=sbas[s];
				sbas[d+1]=sbas[s+1];
				smed[d]=smed[s];
				smed[d+1]=smed[s+1];
				stre[d]=stre[s];
				stre[d+1]=stre[s+1];
			}
			memcpy(ssmp, smp, sizeof(sword)*2*n);
			memcpy(sbas, bas, sizeof(sword)*2*n);
			memcpy(smed, med, sizeof(sword)*2*n);
			memcpy(stre, tre, sizeof(sword)*2*n);
			memcpy(smp, ssmp, sizeof(ssmp));
			memcpy(bas, sbas, sizeof(sbas));
			memcpy(med, smed, sizeof(smed));
			memcpy(tre, stre, sizeof(stre));
		}
		else
		{
			memcpy(ssmp, smp, sizeof(ssmp));
			memcpy(sbas, bas, sizeof(sbas));
			memcpy(smed, med, sizeof(smed));
			memcpy(stre, tre, sizeof(stre));
		}
		{
			int	i;
			for(i=0; i<128; i++)
			{
				int	n=255-i;
				smp[n<<1]=smp[i<<1];
				smp[(n<<1)+1]=smp[(i<<1)+1];
			}
		}
		if(smooth!=0.f)
		{
			int		m=(int)(255.f*smooth);
			int		im=256-m;
			int		i;
			int		vs0=smp[0];
			int		vb0=bas[0];
			int		vm0=med[0];
			int		vt0=tre[0];
			int		vs1=smp[1];
			int		vb1=bas[1];
			int		vm1=med[1];
			int		vt1=tre[1];
			for(i=0; i<256; i++)
			{
				int	n=i<<1;
				int	n1=n+1;
				vs0=smp[n]=(sword)(((int)smp[n]*im+vs0*m)>>8);
				vb0=bas[n]=(sword)(((int)bas[n]*im+vb0*m)>>8);
				vm0=med[n]=(sword)(((int)med[n]*im+vm0*m)>>8);
				vt0=tre[n]=(sword)(((int)tre[n]*im+vt0*m)>>8);
				vs1=smp[n1]=(sword)(((int)smp[n1]*im+vs1*m)>>8);
				vb1=bas[n1]=(sword)(((int)bas[n1]*im+vb1*m)>>8);
				vm1=med[n1]=(sword)(((int)med[n1]*im+vm1*m)>>8);
				vt1=tre[n1]=(sword)(((int)tre[n1]*im+vt1*m)>>8);
			}
			for(i=255; i>=0; i--)
			{
				int	n=i<<1;
				int	n1=n+1;
				vs0=smp[n]=(sword)(((int)smp[n]*im+vs0*m)>>8);
				vb0=bas[n]=(sword)(((int)bas[n]*im+vb0*m)>>8);
				vm0=med[n]=(sword)(((int)med[n]*im+vm0*m)>>8);
				vt0=tre[n]=(sword)(((int)tre[n]*im+vt0*m)>>8);
				vs1=smp[n1]=(sword)(((int)smp[n1]*im+vs1*m)>>8);
				vb1=bas[n1]=(sword)(((int)bas[n1]*im+vb1*m)>>8);
				vm1=med[n1]=(sword)(((int)med[n1]*im+vm1*m)>>8);
				vt1=tre[n1]=(sword)(((int)tre[n1]*im+vt1*m)>>8);
			}
		}
		if(delay==0.f)
		{
			memcpy(msmp, smp, sizeof(msmp));
			memcpy(mbas, bas, sizeof(mbas));
			memcpy(mmed, med, sizeof(mmed));
			memcpy(mtre, tre, sizeof(mtre));
		}
		else
		{
			int	m=(int)(16384.f*delay);
			int	im=16384-m;
			int	i;
			for(i=0; i<512; i++)
			{
				msmp[i]=(sword)(((int)smp[i]*im+(int)msmp[i]*m)>>14);
				mbas[i]=(sword)(((int)bas[i]*im+(int)mbas[i]*m)>>14);
				mmed[i]=(sword)(((int)med[i]*im+(int)mmed[i]*m)>>14);
				mtre[i]=(sword)(((int)tre[i]*im+(int)mtre[i]*m)>>14);
			}
		}
		/*
		{
			int	i;
			int	m=level<0.5f(int)(256.f*level);
			for(i=0; i<512; i++)
			{
				smp[i]=(sword)maxi(mini(((int)msmp[i]*m)>>8, 32767), -32767);
				bas[i]=(sword)maxi(mini(((int)mbas[i]*m)>>8, 32767), -32767);
				med[i]=(sword)maxi(mini(((int)mmed[i]*m)>>8, 32767), -32767);
				tre[i]=(sword)maxi(mini(((int)mtre[i]*m)>>8, 32767), -32767);
			}
		}*/
		{
			int		i;
			double	v=(level<0.5f)?(level*2.f+0.001f):((level-0.5f)*10.f+1.001f);
			for(i=0; i<512; i++)
			{
				if(smp[i]<0)
					smp[i]=-(int)(pow((double)-msmp[i]/32768, v)*32767.f);
				else
					smp[i]=(int)(pow((double)msmp[i]/32768, v)*32767.f);
				if(bas[i]<0)
					bas[i]=-(int)(pow((double)-mbas[i]/32768, v)*32767.f);
				else
					bas[i]=(int)(pow((double)mbas[i]/32768, v)*32767.f);
				if(med[i]<0)
					med[i]=-(int)(pow((double)-mmed[i]/32768, v)*32767.f);
				else
					med[i]=(int)(pow((double)mmed[i]/32768, v)*32767.f);
				if(tre[i]<0)
					tre[i]=-(int)(pow((double)-mtre[i]/32768, v)*32767.f);
				else
					tre[i]=(int)(pow((double)mtre[i]/32768, v)*32767.f);
			}
		}
	}

	switch(sourceType)
	{
		case sourceTRASH:
		if(sourceType!=oldSourceType)
			oldTrashType=-1;
		switch(trashType)
		{
			case sourceTRASH0:
			if(oldTrashType!=trashType)
			{
				int		x,y;
				byte	*d=hauteurs;
				int		max=65536/maxi(mw, mh);
				for(y=0; y<h; y++)
					for(x=0; x<w; x++)
						*(d++)=255-((mini(abs(x-mw), abs(y-mh))*max)>>8);
			}
			break;

			case sourceTRASH1:
			if(oldTrashType!=trashType)
			{
				int		x,y;
				byte	*d=hauteurs;
				int		max=65536/maxi(mw, mh);
				for(y=0; y<h; y++)
					for(x=0; x<w; x++)
						*(d++)=255-((maxi(abs(x-mw), abs(y-mh))*max)>>8);
			}
			break;

			case sourceTRASHPOL:
			if(oldTrashType!=trashType)
			{
				int		x,y;
				byte	*d=hauteurs;
				float	max1=255.99f/(float)sqrt((float)(mw*mw+mh*mh));
				for(y=0; y<h; y++)
					for(x=0; x<w; x++)
					{
						int	dx=x-mw;
						int	dy=y-mh;
						*(d++)=255-(byte)(sqrt((float)(dx*dx+dy*dy))*max1);
					}
			}
			break;

			case sourceTRASHR0:
			if(oldTrashType!=trashType)
			{
				if((w<=512)&&(h<=512))
				{
					int		x,y;
					byte	*d=hauteurs;
					for(y=0; y<h; y++)
					{
						int	dy=y-mh;
						for(x=0; x<w; x++)
						{
							int	dx=x-mw;
							*(d++)=tatan[(dx&511)|((dy&511)<<9)]&255;
						}
					}
				}
				else
				{
					int		x,y;
					byte	*d=hauteurs;
					for(y=0; y<h; y++)
					{
						int	dy=y-mh;
						for(x=0; x<w; x++)
						{
							int	dx=x-mw;
							*(d++)=(byte)((atan2((float)dy, (float)dx)*127.999/PI)+128);
						}
					}
					
				}
			}
			break;
		}
		oldTrashType=trashType;
		break;

		case sourceZYGO:
		switch(colorType)
		{
			default:
			{
				int		x,y,i;
				byte	*dd=hauteurs;

				double	zygoX[12];	//	11
				double	zygoY[12];

				double	mulz=127.9/nbZygo;
				double	addz=128/nbZygo;

				double	ax=((double)(w)*0.5);
				double	dx=((double)(w));
				double	ay=((double)(h)*0.5);
				double	dy=((double)(h));

				double	cycle=(double)time*0.00055528556;

				for(i=0; i<nbZygo; i++)
				{
					double	n=cycle*vZygo+sin(i+cycle*0.001455885);
					zygoX[i]=sin(n)*sin(n*0.75841205105+1552.62662251)*dx+ax;
					zygoY[i]=sin(n*0.85841205105+152.62662251)*sin(n*0.7125105+15.6262)*dy+ay;
				}

				double	zoom=zZygo*10*6.28/(double)w;

				for(y=0; y<h; y++)
				{
					for(x=0; x<w; x++)
					{
						double	v=0;
						for(i=0; i<nbZygo; i++)
						{
							double	dx=x-zygoX[i];
							double	dy=y-zygoY[i];
							double	d=sqrt(dx*dx+dy*dy)*zoom;
							//int		a=(int)((atan2(dy, dx)*127.999/PI)+128)&255;
							v+=sin(d)*mulz+addz;
						}
						*(dd++)=(byte)v;
					}
				}
			}
			break;

			case colorHOUSE:
			break;
		}
		break;

		case sourceIMAGE:
		if((colorType!=colorHOUSE)&&vin->isConnected())
		{
			vin->enter(__FILE__,__LINE__);
			Abitmap	*b=vin->getBitmap();
			if(b)
			{
				bool	blur=((AaestesisFront *)front)->blur->get();
				byte	*d=blur?hauteurs2:hauteurs;
				dword	*s=b->body32;
				int		x,y;
				float	contrast=((AaestesisFront *)front)->contrast->get();
				double	vc=(contrast<0.5f)?(contrast*2.f+0.001f):((contrast-0.5f)*10.f+1.001f);
				byte	t[256];
				{
					int	i;
					for(i=0; i<256; i++)
						t[i]=(int)(pow((double)i/255, vc)*255.f);
				}
				{	
					for(y=0; y<h; y++)
					{
						for(x=0; x<w; x++)
						{
							byte	r,g,b;
							colorRGB(&r, &g, &b, *(s++));
							*(d++)=t[(((int)r+(int)g+(int)b)/3)];
						}
					}
				}
				if(blur)
				{
					int		nw=w-1;
					int		nh=h-1;
					byte	*s=hauteurs2;
					{
						int		n=w+1;
						for(y=1; y<nh; y++)
						{
							for(x=1; x<nw; x++)
							{
								int	nm=n-w;
								int	np=n+w;
								hauteurs[n]=(byte)(((int)s[nm-1]+(int)s[nm]+(int)s[nm+1]+(int)s[n-1]+(int)s[n+1]+(int)s[np-1]+(int)s[np]+(int)s[np+1])>>3);
								n++;
							}
							n+=2;
						}
					}
					{
						int		nx=(h-1)*w+1;
						for(x=1; x<nw; x++)
						{
							int	np=x+w;
							int	nm=nx-w;
							hauteurs[x]=(byte)(((int)s[x-1]+(int)s[x+1]+(int)s[np-1]+(int)s[np]+(int)s[np+1])/5);
							hauteurs[nx]=(byte)(((int)s[nx-1]+(int)s[nx+1]+(int)s[nm-1]+(int)s[nm]+(int)s[nm+1])/5);
						}
					}
					{
						int	ny1=(2*w-1);
						int	ny0=w;
						for(y=1; y<nh; y++)
						{
							int	nm0=ny0-w;
							int	np0=ny0+w;
							int	nm1=ny1-w;
							int	np1=ny1+w;
							hauteurs[ny0]=(byte)(((int)s[nm0]+(int)s[nm0+1]+(int)s[ny0+1]+(int)s[np0]+(int)s[np0+1])/5);
							hauteurs[ny1]=(byte)(((int)s[nm1-1]+(int)s[nm1]+(int)s[ny1-1]+(int)s[np1-1]+(int)s[np1])/5);
						}
					}
				}
			}
			vin->leave();
		}
		else
			memset(hauteurs, 0, w*h);
		break;

		default:
		memset(hauteurs, 0, w*h);
		break;
	}
	out->enter(__FILE__,__LINE__);
	switch(colorType)
	{
		case colorNB:
		case colorNBSOUND:
		case colorTRASH:
		{
			dword	c0=((AaestesisFront *)front)->pal0->get();
			dword	c1=((AaestesisFront *)front)->pal1->get();
			dword	c2=((AaestesisFront *)front)->pal2->get();
			dword	c3=((AaestesisFront *)front)->pal3->get();
			byte	r,g,b;
			int		r0,b0,g0;
			int		r1,b1,g1;
			int		r2,b2,g2;
			int		r3,b3,g3;
			int		dr0,dg0,db0;
			int		dr1,dg1,db1;
			int		dr2,dg2,db2;
			int		dr3,dg3,db3;
			int		i;
			colorRGB(&r, &g, &b, c0);
			r0=(int)r<<16;
			g0=(int)g<<16;
			b0=(int)b<<16;
			colorRGB(&r, &g, &b, c1);
			r1=(int)r<<16;
			g1=(int)g<<16;
			b1=(int)b<<16;
			colorRGB(&r, &g, &b, c2);
			r2=(int)r<<16;
			g2=(int)g<<16;
			b2=(int)b<<16;
			colorRGB(&r, &g, &b, c3);
			r3=(int)r<<16;
			g3=(int)g<<16;
			b3=(int)b<<16;
			dr0=(r1-r0)>>6;
			dg0=(g1-g0)>>6;
			db0=(b1-b0)>>6;
			dr1=(r2-r1)>>6;
			dg1=(g2-g1)>>6;
			db1=(b2-b1)>>6;
			dr2=(r3-r2)>>6;
			dg2=(g3-g2)>>6;
			db2=(b3-b2)>>6;
			dr3=(r0-r3)>>6;
			dg3=(g0-g3)>>6;
			db3=(b0-b3)>>6;
			for(i=0; i<64; i++)
			{
				pal[i]=color32((byte)(r0>>16), (byte)(g0>>16), (byte)(b0>>16));
				r0+=dr0;
				g0+=dg0;
				b0+=db0;
			}
			for(i=64; i<128; i++)
			{
				pal[i]=color32((byte)(r0>>16), (byte)(g0>>16), (byte)(b0>>16));
				r0+=dr1;
				g0+=dg1;
				b0+=db1;
			}
			for(i=128; i<192; i++)
			{
				pal[i]=color32((byte)(r0>>16), (byte)(g0>>16), (byte)(b0>>16));
				r0+=dr2;
				g0+=dg2;
				b0+=db2;
			}
			for(i=192; i<256; i++)
			{
				pal[i]=color32((byte)(r0>>16), (byte)(g0>>16), (byte)(b0>>16));
				r0+=dr3;
				g0+=dg3;
				b0+=db3;
			}
		}
		break;
	}
	switch(colorType)
	{
		case colorNB:
		{
			int		size=w*h;
			int		i;
			dword	*d=&out->image->body32[0];
			byte	*s=hauteurs;
			cycleval+=(int)((pow((cycle-0.5)*4, 3)/8)*dbeat*800.f);
			for(i=0; i<size; i++)
			{
				int	v=(*s+cycleval)&255;
				*(d++)=pal[v];
				s++;
			}
		}
		break;

		case colorNBSOUND:
		if(snd->isConnected())
		{
			int		size=w*h;
			int		i;
			dword	*d=&out->image->body32[0];
			byte	*s=hauteurs;
			dword	t[256];
			for(i=0; i<256; i++)
			{
				int	v=((int)smp[i<<1]+32768)>>8;
				t[i]=pal[v&255];
			}
			for(i=0; i<size; i++)
			{
				*(d++)=t[*s];
				s++;
			}
		}
		else
			out->image->boxf(0, 0, out->image->w, out->image->h, 0xff000000);
		break;

		case colorTRASH:
		switch(sourceType)
		{
			case sourceTRASH:
			switch(trashType)
			{
				case sourceTRASH1:
				if(snd->isConnected())
				{
					int		x,y;
					int		mw=w>>1;
					int		mh=h>>1;
					for(y=0; y<h; y++)
					{
						int		dy=abs(y-mh);
						int		v1=dy&255;
						dword	*d=&out->image->body32[out->image->adr[y]];
						for(x=0; x<w; x++)
						{
							int	dx=abs(x-mw);
							int	v0=dx&255;
							int	v=((int)((smp[v0<<1]+32768)>>9)+(int)((smp[(v1<<1)+1]+32768)>>9))&255;
							*(d++)=pal[v];
						}
					}
				}
				break;

				case sourceTRASHPOL:
				if(snd->isConnected())
				{
					int		x,y;
					int		mw=w>>1;
					int		mh=h>>1;
					for(y=0; y<h; y++)
					{
						int		dy=abs(y-mh);
						dword	*d=&out->image->body32[out->image->adr[y]];
						byte	*s=hauteurs+out->image->adr[y];
						for(x=0; x<w; x++)
						{
							int	dx=abs(x-mw);
							int	v0=tatan[(dx&511)|((dy&511)<<9)]&255;
							int	v1=*s;
							int	v=((int)((smp[v0<<1]+32768)>>9)+(int)((smp[(v1<<1)+1]+32768)>>9))&255;
							*(d++)=pal[v];
							s++;
						}
					}
				}
				break;

				default:
				goto lbl000;
			}
			break;

			case sourceZYGO:
			if(snd->isConnected())
			{
				int		x,y,i;
				dword	*dd=&out->image->body32[0];

				double	zygoX[12];	//	11
				double	zygoY[12];

				double	mulz=127.9/nbZygo;
				double	addz=128/nbZygo;

				double	ax=((double)(w)*0.5);
				double	dx=((double)(w));
				double	ay=((double)(h)*0.5);
				double	dy=((double)(h));

				double	cycle=(double)time*0.00055528556;

				double	*memo=(double *)malloc(sizeof(double)*w);
				memset(memo, 0, sizeof(double)*w);

				for(i=0; i<nbZygo; i++)
				{
					double	n=cycle*vZygo+sin(i+cycle*0.001455885);
					zygoX[i]=sin(n)*sin(n*0.75841205105+1552.62662251)*dx+ax;
					zygoY[i]=sin(n*0.85841205105+152.62662251)*sin(n*0.7125105+15.6262)*dy+ay;
				}

				double	zoom=zZygo*10*6.28/(double)w;

				for(y=0; y<h; y++)
				{
					double	om=0.f;
					for(x=0; x<w; x++)
					{
						double	dv0=0;
						for(i=0; i<nbZygo; i++)
						{
							double	dx=x-zygoX[i];
							double	dy=y-zygoY[i];
							double	d=sqrt(dx*dx+dy*dy)*zoom;
							dv0+=sin(d)*mulz+addz;
						}
						double	dv1=atan2(dv0-memo[x], dv0-om);
						int		v0=(((int)dv0)&255)<<1;
						int		v1=(((int)((dv1*127.999/PI)+128)&255)<<1)+1;

						int	v=((int)((smp[v0]+32768)>>9)+(int)((smp[v1]+32768)>>9))&255;

						*(dd++)=pal[v];

						memo[x]=dv0;
						om=dv0;
					}
				}
				free(memo);
			}
			break;

			default:
			lbl000:
			if(snd->isConnected())
			{
				int		x,y;
				for(y=1; y<h; y++)
				{
					dword	*d=&out->image->body32[out->image->adr[y]+1];
					byte	*s=hauteurs+out->image->adr[y]+1;
					for(x=1; x<w; x++)
					{
						int	dx=*(s-1)-*s;
						int	dy=*(s-w)-*s;
						int	v0=tatan[(dx&511)|((dy&511)<<9)]&255;
						int	v1=*s;
						int	v=((int)((smp[v0<<1]+32768)>>9)+(int)((smp[(v1<<1)+1]+32768)>>9))&255;
						*(d++)=pal[v];
						s++;
					}
				}
				out->image->line(0, 0, w, 0, 0xff808080);
				out->image->line(0, 0, 0, h, 0xff808080);
			}
			break;
		}
		break;

		case colorHOUSE:
		{
			byte	rr[256];
			byte	rg[256];
			byte	rb[256];
			byte	gr[256];
			byte	gg[256];
			byte	gb[256];
			byte	br[256];
			byte	bg[256];
			byte	bb[256];
			dword	colorR=((AaestesisFront *)front)->colorR->get();
			dword	colorG=((AaestesisFront *)front)->colorG->get();
			dword	colorB=((AaestesisFront *)front)->colorB->get();
			{
				int	i;
				for(i=0; i<256; i++)
				{
					bas[i]=(int)sqrt((float)(abs(bas[i])<<1))>>1;
					med[i]=(int)sqrt((float)(abs(med[i])<<1))>>1;
					tre[i]=(int)sqrt((float)(abs(tre[i])<<1))>>1;
				}
			}
			{
				int	i;
				for(i=0; i<128; i++)
				{
					int	s=i<<1;
					int	d=(255-i)<<1;
					bas[d]=bas[s];
					med[d]=med[s];
					tre[d]=tre[s];
					s++;
					d++;
					bas[d]=bas[s];
					med[d]=med[s];
					tre[d]=tre[s];
				}
			}
			{
				float	vr=((AaestesisFront *)front)->imaR->get();
				float	vg=((AaestesisFront *)front)->imaG->get();
				float	vb=((AaestesisFront *)front)->imaB->get();
				int		rm=(int)(256.f*((vr<0.5f)?(vr*2.f):(1.f+(vr-0.5f)*10.f)));
				int		gm=(int)(256.f*((vg<0.5f)?(vg*2.f):(1.f+(vg-0.5f)*10.f)));
				int		bm=(int)(256.f*((vb<0.5f)?(vb*2.f):(1.f+(vb-0.5f)*10.f)));
				int		i;

				for(i=0; i<512; i++)
				{
					bas[i]=mini(((int)bas[i]*rm)>>8, 127);
					med[i]=mini(((int)med[i]*gm)>>8, 127);
					tre[i]=mini(((int)tre[i]*bm)>>8, 127);
				}
			}
			if(sourceType!=sourceIMAGE)
			{
				{
					byte	r,g,b;
					int		dr,dg,db;
					int		i;
					colorRGB(&r, &g, &b, colorR);
					dr=((int)r<<8);
					dg=((int)g<<8);
					db=((int)b<<8);
					for(i=0; i<256; i++)
					{
						rr[i]=(i*dr)>>16;
						rg[i]=(i*dg)>>16;
						rb[i]=(i*db)>>16;
					}
				}
				{
					byte	r,g,b;
					int		dr,dg,db;
					int		i;
					colorRGB(&r, &g, &b, colorG);
					dr=((int)r<<8);
					dg=((int)g<<8);
					db=((int)b<<8);
					for(i=0; i<256; i++)
					{
						gr[i]=(i*dr)>>16;
						gg[i]=(i*dg)>>16;
						gb[i]=(i*db)>>16;
					}
				}
				{
					byte	r,g,b;
					int		dr,dg,db;
					int		i;
					colorRGB(&r, &g, &b, colorB);
					dr=((int)r<<8);
					dg=((int)g<<8);
					db=((int)b<<8);
					for(i=0; i<256; i++)
					{
						br[i]=(i*dr)>>16;
						bg[i]=(i*dg)>>16;
						bb[i]=(i*db)>>16;
					}
				}
			}
			switch(sourceType)
			{
				case sourceTRASH:
				switch(trashType)
				{
					case sourceTRASH1:
					if(snd->isConnected())
					{
						int		x,y;
						int		mw=w>>1;
						int		mh=h>>1;
						for(y=0; y<h; y++)
						{
							int		dy=abs(y-mh);
							int		v1=dy&255;
							dword	*d=&out->image->body32[out->image->adr[y]];
							for(x=0; x<w; x++)
							{
								int	dx=abs(x-mw);
								int	v0=dx&255;
								//int	v=((int)((smp[v0<<1]+32768)>>9)+(int)((smp[(v1<<1)+1]+32768)>>9))&255;
								int	r=((int)((bas[v0]))+(int)((bas[v1+1])))&255;
								int	g=((int)((med[v0]))+(int)((med[v1+1])))&255;
								int	b=((int)((tre[v0]))+(int)((tre[v1+1])))&255;
								*(d++)=color32(	(byte)mini((int)rr[r]+(int)gr[g]+(int)br[b], 255),
												(byte)mini((int)rg[r]+(int)gg[g]+(int)bg[b], 255),
												(byte)mini((int)rb[r]+(int)gb[g]+(int)bb[b], 255) );
							}
						}
					}
					break;

					case sourceTRASHPOL:
					if(snd->isConnected())
					{
						int		x,y;
						int		mw=w>>1;
						int		mh=h>>1;
						for(y=0; y<h; y++)
						{
							int		dy=abs(y-mh);
							dword	*d=&out->image->body32[out->image->adr[y]];
							byte	*s=hauteurs+out->image->adr[y];
							for(x=0; x<w; x++)
							{
								int	dx=abs(x-mw);
								int	v0=(tatan[(dx&511)|((dy&511)<<9)]&255)<<1;
								int	v1=(*s)<<1;
								int	r=((int)((bas[v0]))+(int)((bas[v1+1])))&255;
								int	g=((int)((med[v0]))+(int)((med[v1+1])))&255;
								int	b=((int)((tre[v0]))+(int)((tre[v1+1])))&255;
								*(d++)=color32(	(byte)mini((int)rr[r]+(int)gr[g]+(int)br[b], 255),
												(byte)mini((int)rg[r]+(int)gg[g]+(int)bg[b], 255),
												(byte)mini((int)rb[r]+(int)gb[g]+(int)bb[b], 255) );
								s++;
							}
						}
					}
					break;

					default:
					goto lbl001;
				}
				break;

				case sourceZYGO:
				if(snd->isConnected())
				{
					int		x,y,i;
					dword	*dd=&out->image->body32[0];

					double	zygoX[12];	//	11
					double	zygoY[12];

					double	mulz=127.9/nbZygo;
					double	addz=128/nbZygo;

					double	ax=((double)(w)*0.5);
					double	dx=((double)(w));
					double	ay=((double)(h)*0.5);
					double	dy=((double)(h));

					double	cycle=(double)time*0.00055528556;

					double	*memo=(double *)malloc(sizeof(double)*w);
					memset(memo, 0, sizeof(double)*w);

					for(i=0; i<nbZygo; i++)
					{
						double	n=cycle*vZygo+sin(i+cycle*0.001455885);
						zygoX[i]=sin(n)*sin(n*0.75841205105+1552.62662251)*dx+ax;
						zygoY[i]=sin(n*0.85841205105+152.62662251)*sin(n*0.7125105+15.6262)*dy+ay;
					}

					double	zoom=zZygo*10*6.28/(double)w;

					for(y=0; y<h; y++)
					{
						double	om=0.f;
						for(x=0; x<w; x++)
						{
							double	dv0=0;
							for(i=0; i<nbZygo; i++)
							{
								double	dx=x-zygoX[i];
								double	dy=y-zygoY[i];
								double	d=sqrt(dx*dx+dy*dy)*zoom;
								dv0+=sin(d)*mulz+addz;
							}
							double	dv1=atan2(dv0-memo[x], dv0-om);
							int		v0=(((int)dv0)&255)<<1;
							int		v1=(((int)((dv1*127.999/PI)+128)&255)<<1)+1;
							int		r=((int)((bas[v0]))+(int)((bas[v1])))&255;
							int		g=((int)((med[v0]))+(int)((med[v1])))&255;
							int		b=((int)((tre[v0]))+(int)((tre[v1])))&255;

							*(dd++)=color32(	(byte)mini((int)rr[r]+(int)gr[g]+(int)br[b], 255),
											(byte)mini((int)rg[r]+(int)gg[g]+(int)bg[b], 255),
											(byte)mini((int)rb[r]+(int)gb[g]+(int)bb[b], 255) );
							memo[x]=dv0;
							om=dv0;
						}
					}
					free(memo);
				}
				break;

				case sourceIMAGE:
				{
					Abitmap *bo=out->image;
					if(vin->isConnected())
					{
						vin->enter(__FILE__,__LINE__);
						Abitmap	*b=vin->getBitmap();
						if(b)
						{
							dword	*d=bo->body32;
							dword	*s=b->body32;
							int		x,y;
							float	contrast=((AaestesisFront *)front)->contrast->get();
							double	vc=(contrast<0.5f)?(contrast*2.f+0.001f):((contrast-0.5f)*10.f+1.001f);
							byte	t[256];
							int		i;
							for(i=0; i<256; i++)
								t[i]=(int)(pow((double)i/255, vc)*255.f);
							for(i=0; i<256; i++)
							{
								int n=i<<1;
								bas[i]=bas[n]+bas[n+1];
								med[i]=med[n]+med[n+1];
								tre[i]=tre[n]+tre[n+1];
							}
							{
								byte	r,g,b;
								int		dr,dg,db;
								int		i;
								colorRGB(&r, &g, &b, colorR);
								dr=((int)r<<8);
								dg=((int)g<<8);
								db=((int)b<<8);
								for(i=0; i<256; i++)
								{
									int	v=bas[t[i]];
									rr[i]=(v*dr)>>16;
									rg[i]=(v*dg)>>16;
									rb[i]=(v*db)>>16;
								}
							}
							{
								byte	r,g,b;
								int		dr,dg,db;
								int		i;
								colorRGB(&r, &g, &b, colorG);
								dr=((int)r<<8);
								dg=((int)g<<8);
								db=((int)b<<8);
								for(i=0; i<256; i++)
								{
									int	v=med[t[i]];
									gr[i]=(v*dr)>>16;
									gg[i]=(v*dg)>>16;
									gb[i]=(v*db)>>16;
								}
							}
							{
								byte	r,g,b;
								int		dr,dg,db;
								int		i;
								colorRGB(&r, &g, &b, colorB);
								dr=((int)r<<8);
								dg=((int)g<<8);
								db=((int)b<<8);
								for(i=0; i<256; i++)
								{
									int	v=tre[t[i]];
									br[i]=(v*dr)>>16;
									bg[i]=(v*dg)>>16;
									bb[i]=(v*db)>>16;
								}
							}
							for(y=0; y<h; y++)
							{
								for(x=0; x<w; x++)
								{
									byte	r,g,b;
									colorRGB(&r, &g, &b, *(s++));
									*(d++)=color32(	(byte)mini((int)rr[r]+(int)gr[g]+(int)br[b], 255),
													(byte)mini((int)rg[r]+(int)gg[g]+(int)bg[b], 255),
													(byte)mini((int)rb[r]+(int)gb[g]+(int)bb[b], 255) );
								}
							}
						}
						vin->leave();
					}
					else
						bo->boxf(0, 0, bo->w, bo->h, 0xff000000);
				}
				break;

				default:
				lbl001:
				if(snd->isConnected())
				{
					int		x,y;
					for(y=1; y<h; y++)
					{
						dword	*d=&out->image->body32[out->image->adr[y]+1];
						byte	*s=hauteurs+out->image->adr[y]+1;
						for(x=1; x<w; x++)
						{
							int	dx=(int)*(s-1)-(int)*s;
							int	dy=(int)*(s-w)-(int)*s;
							int	v0=(tatan[(dx&511)|((dy&511)<<9)]&255)<<1;
							int	v1=(*s)<<1;
							int	r=((int)((bas[v0]))+(int)((bas[v1+1])))&255;
							int	g=((int)((med[v0]))+(int)((med[v1+1])))&255;
							int	b=((int)((tre[v0]))+(int)((tre[v1+1])))&255;
							*(d++)=color32(	(byte)mini((int)rr[r]+(int)gr[g]+(int)br[b], 255),
											(byte)mini((int)rg[r]+(int)gg[g]+(int)bg[b], 255),
											(byte)mini((int)rb[r]+(int)gb[g]+(int)bb[b], 255) );
							s++;
						}
					}
					out->image->line(0, 0, w, 0, 0xff808080);
					out->image->line(0, 0, 0, h, 0xff808080);
				}
				break;
			}
		}
		break;
	}
	out->leave();
	oldSourceType=sourceType;
	oldColorType=oldColorType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AaestesisFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

#define hdx	(-6)
#define hdy	(-6)

AaestesisFront::AaestesisFront(QIID qiid, char *name, Aaestesis *e, int h) : AeffectFront(qiid, name, e, h)
{
	setTooltips("aestesis module");
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_AESTESIS), "PNG");
	back=new Abitmap(&o);

	source=new Aselect(MKQIID(qiid, 0x1aedf9fe79205860), "source", this, 160, 50, 1, 3, &resource.get(MAKEINTRESOURCE(PNG_AESTESIS_SOURCES), "PNG"), 16, 16);
	source->setTooltips("source selector");
	source->set(0);
	source->show(TRUE);

	trash=new Aselect(MKQIID(qiid, 0x65f0110bc9c5b100), "form", this, 48, 22, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_AESTESIS_FORMS), "PNG"), 16, 16);
	trash->setTooltips("form type");
	trash->set(0);
	trash->show(TRUE);
	
	nbZygo=new Apaddle(MKQIID(qiid, 0x6b87cca300c795c4), "zygo/level", this, 40, 64, paddleBUTTON07);
	nbZygo->setTooltips("zygo level selector");
	nbZygo->set(0.0f);
	nbZygo->show(TRUE);

	vZygo=new Apaddle(MKQIID(qiid, 0x2cb5003c3d63c480), "zygo/speed", this, 108, 64, paddleBUTTON07);
	vZygo->setTooltips("zygo speed");
	vZygo->set(0.0f);
	vZygo->show(TRUE);

	zZygo=new Apaddle(MKQIID(qiid, 0x675edef558217ba0), "zygo/depth", this, 74, 64, paddleBUTTON07);
	zZygo->setTooltips("zygo depth");
	zZygo->set(0.0f);
	zZygo->show(TRUE);

	color=new Aselect(MKQIID(qiid, 0x4b03d64d35403600), "color", this, 256+76, 40, 1, 4, &resource.get(MAKEINTRESOURCE(PNG_AESTESIS_COLORS), "PNG"), 16, 16);
	color->setTooltips("color type selector");
	color->set(0);
	color->show(TRUE);

	cycle=new Apaddle(MKQIID(qiid, 0x94a43e9bf6bb0b30), "palette/cycle", this, 258+150, 22, paddleBUTTON07);
	cycle->setTooltips("cycle variator");
	cycle->set(0.5f);
	cycle->show(TRUE);

	pal0=new Aselcolor(MKQIID(qiid, 0x2a5f26d755b8afd8), "palette/color 0", this, 260+150+25+10, 18);
	pal0->setTooltips("palette color 0");
	pal0->set(0xffffffff);
	pal0->show(true);

	pal1=new Aselcolor(MKQIID(qiid, 0x9c008e3943f38400), "palette/color 1", this, 260+150+45+10, 18);
	pal1->setTooltips("palette color 1");
	pal1->set(0xffffaa00);
	pal1->show(true);

	pal2=new Aselcolor(MKQIID(qiid, 0x962f43fc1ed75a68), "palette/color 2", this, 260+150+25+10, 38);
	pal2->setTooltips("palette color 2");
	pal2->set(0xff000000);
	pal2->show(true);

	pal3=new Aselcolor(MKQIID(qiid, 0xbcbe2c565c3a3140), "palette/color 3", this, 260+150+45+10, 38);
	pal3->setTooltips("palette color 3");
	pal3->set(0xffffaa00);
	pal3->show(true);

	level=new Apaddle(MKQIID(qiid, 0xe58e0c3d5a8c85c0), "audio/expand", this, 264+150+hdx, 80+hdy, paddleYOLI16);
	level->setTooltips("audio expand");
	level->set(0.5f);
	level->show(TRUE);

	smooth=new Apaddle(MKQIID(qiid, 0x18d695d3f7d30b90), "audio/smooth", this, 264+150+20+hdx, 80+hdy, paddleYOLI16);
	smooth->setTooltips("audio smooth");
	smooth->set(0.f);
	smooth->show(TRUE);

	delay=new Apaddle(MKQIID(qiid, 0x4cd40adbf80ac3b0), "audio/delay", this, 264+150+40+hdx, 80+hdy, paddleYOLI16);
	delay->setTooltips("audio delay");
	delay->set(0.f);
	delay->show(TRUE);

	speed=new Apaddle(MKQIID(qiid, 0x00e5a3b9b5cc3155), "audio/speed", this, 264+150+60+hdx, 80+hdy, paddleYOLI16);
	speed->setTooltips("audio speed");
	speed->set(0.f);
	speed->show(TRUE);

	imaR=new Apaddle(MKQIID(qiid, 0x47bed0971a6a8c1a), "equalizer/bass level", this, 264+150+hdx, 136+hdy, paddleYOLI16);
	imaR->setTooltips("bass level");
	imaR->set(.5f);
	imaR->show(TRUE);

	imaG=new Apaddle(MKQIID(qiid, 0xeec515819416afa8), "equalizer/medium level", this, 264+150+30+hdx, 136+hdy, paddleYOLI16);
	imaG->setTooltips("medium level");
	imaG->set(.5f);
	imaG->show(TRUE);

	imaB=new Apaddle(MKQIID(qiid, 0xdb2d6fe625ded77c), "equalizer/treble level", this, 264+150+60+hdx, 136+hdy, paddleYOLI16);
	imaB->setTooltips("treble level");
	imaB->set(.5f);
	imaB->show(TRUE);

	colorR=new Aselcolor(MKQIID(qiid, 0x95b4f2586d487200), "equalizer/bass color", this, 258+150, 108);
	colorR->setTooltips("bass color");
	colorR->set(0xffff0000);
	colorR->show(true);

	colorG=new Aselcolor(MKQIID(qiid, 0x81e9a92ee0946c06), "equalizer/medium color", this, 257+150+30, 108);
	colorG->setTooltips("medium color");
	colorG->set(0xff00ff00);
	colorG->show(true);

	colorB=new Aselcolor(MKQIID(qiid, 0x71d57ff9e80b0a14), "equalizer/treble color", this, 256+150+60, 108);
	colorB->setTooltips("treble color");
	colorB->set(0xff0000ff);
	colorB->show(true);

	contrast=new Apaddle(MKQIID(qiid, 0xa5b6f72556f46f80), "image/contrast", this, 49+hdx, 136+hdy, paddleYOLI16);
	contrast->setTooltips("contrast");
	contrast->set(.5f);
	contrast->show(TRUE);

	blur=new ActrlButton(MKQIID(qiid, 0x75d835ba7776ed08), "image/blur", this, 70, 129, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_SPHERE_CROSS), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	blur->setTooltips("blur");
	blur->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaestesisFront::~AaestesisFront()
{
	delete(back);
	delete(source);
	delete(trash);
	delete(color);
	delete(nbZygo);
	delete(vZygo);
	delete(zZygo);
	delete(imaR);
	delete(imaG);
	delete(imaB);
	delete(pal0);
	delete(pal1);
	delete(pal2);
	delete(pal3);
	delete(level);
	delete(smooth);
	delete(delay);
	delete(speed);
	delete(colorR);
	delete(colorG);
	delete(colorB);
	delete(contrast);
	delete(blur);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AaestesisFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AaestesisFront::pulse()
{
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaestesisBack::AaestesisBack(QIID qiid, char *name, Aaestesis *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_AESTESIS2), "PNG");
	back=new Abitmap(&o);

	in=new Asample(MKQIID(qiid, 0x870638b4484b1540), "audio input", this, pinIN, 10, 10);
	in->setTooltips("audio input");
	in->show(TRUE);

	vin=new Avideo(MKQIID(qiid, 0x2cc085e6de02cc88), "video input", this, pinIN, 30, 10);
	vin->setTooltips("video input");
	vin->show(TRUE);

	out=new Avideo(MKQIID(qiid, 0x3c8937523e4d1200), "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaestesisBack::~AaestesisBack()
{
	delete(in);
	delete(vin);
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AaestesisBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AaestesisInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Aaestesis(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * aestesisGetInfo()
{
	return new AaestesisInfo("aestesisInfo", &Aaestesis::CI, "aestesis", "aestesis module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
