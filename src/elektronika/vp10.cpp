/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	VP10.CPP					(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>
#include						"resource.h"
#include						"vp10.h"
#include						"global.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Avp10Info::CI	= ACI("Avp10Info",		GUID(0x11111112,0x00000870), &AeffectInfo::CI, 0, NULL);
ACI								Avp10::CI		= ACI("Avp10",			GUID(0x11111112,0x00000871), &Aeffect::CI, 0, NULL);
ACI								Avp10Front::CI	= ACI("Avp10Front",		GUID(0x11111112,0x00000872), &AeffectFront::CI, 0, NULL);
ACI								Avp10Back::CI	= ACI("Avp10Back",		GUID(0x11111112,0x00000873), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static dword					colors[]={ 0xffff0000, 0xffff8000, 0xffffff00, 0xff00ff00, 0xff00ffff, 0xff0000ff, 0xff8000ff, 0xffff0080};
static Arectangle				disp=Arectangle(9, 9, 436-7, 109-9);
static int						resbut[]={ PNG_BUTMETAL_0, PNG_BUTMETAL_1, PNG_BUTMETAL_2, PNG_BUTMETAL_3, PNG_BUTMETAL_4, PNG_BUTMETAL_5, PNG_BUTMETAL_6, PNG_BUTMETAL_7};

static byte						m_seq[64];
static bool						m_usedseq=false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MS_MAXFRAMES					(400)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static double					resofreq[9]	= { 0.125, 0.25, 0.5, 0.75, 1, 1.5, 2, 4, 8 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static __inline void dbgprint(char *s)
{
	/*
	FILE	*f=fopen("d:\\elek-vp10.txt", "a");
	if(f)
	{
		fprintf(f, s);
		fclose(f);
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	void						*data;
	int							size;
} AudioData;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmovieStream : public Anode, public Athread
{
public:
	AOBJ

	enum
	{
								fadeNONE=0,
								fadeIN=1,
								fadeOUT=2
	};

								AmovieStream					(char *file);
								~AmovieStream					();
				
	bool						isOK							();	
	
	void						run								();
	virtual void				stop							();
	
	void						setBegin						(int pos);
	void						setLenght						(int len);
	
	bool						setBitmap						(Abitmap *b, int pos, float level, int blendop);
	bool						setSample						(Asample *smp, double frame, double dframe, double dtime, float level, int fade);
	float						getMaxSample					(float p, float w);

	void						setVideoSize					(int w, int h);

private:
	int							getSample						(int pos, sword **buffer);
	void						writeSample						(Afilemem *fm, int pos, double s, double d);

public:
	char						filename[ALIB_PATH];

	Asection					section;					
	AaviRead					*avi;
	Abitmap						*bitmap[MS_MAXFRAMES];
	AudioData					audio[MS_MAXFRAMES];
	bool						bloaded[MS_MAXFRAMES];
	
	bool						running;
	bool						outrun;

	float						fps;
		
	int							begin;							// all value are exprimed in frame position
	int							lenght;
	int							moviesize;

	short						membuf[2];

	int							videoWidth;
	int							videoHeight;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AmovieStream::CI	= ACI("AmovieStream",		GUID(0x11111112,0x00000875), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmovieStream::AmovieStream(char *file) : Anode(file)
{
	videoWidth=320;
	videoHeight=240;
	moviesize=0;
	outrun=true;
	avi=null;
	fps=0.f;
	strcpy(filename, file);
	memset(bitmap, 0, sizeof(bitmap));
	memset(audio, 0, sizeof(audio));
	memset(bloaded, 0, sizeof(bloaded));
	memset(membuf, 0, sizeof(membuf));
	begin=0;
	lenght=MS_MAXFRAMES;
	start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AmovieStream::~AmovieStream()
{
	stop();
	{
		int	i;
		for(i=0; i<MS_MAXFRAMES; i++)
		{
			if(bitmap[i])
			{
				delete(bitmap[i]);
				bitmap[i]=null;
			}
			if(audio[i].data)
			{
				free(audio[i].data);
				audio[i].data=null;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmovieStream::stop()
{
	running=false;
	while(!outrun)
		sleep(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AmovieStream::isOK()
{
	return (avi&&avi->isOK);	// todo add a unbreak thread block function around the test
}

void AmovieStream::setVideoSize(int w, int h)
{
	section.enter(__FILE__,__LINE__);
	videoWidth=w;
	videoHeight=h;
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmovieStream::run()
{
	bool	ht=(getCPUcount()>1);
	outrun=false;
	running=true;
	
	CoInitialize(null);

	section.enter(__FILE__,__LINE__);
		avi=new AaviRead(filename, AaviRead::VIDEO|AaviRead::AUDIO);
	section.leave();

	if(avi&&avi->isOK)
	{
		moviesize=avi->nbframes;
		fps=avi->getFPS();
		
		if(moviesize<lenght)
			lenght=moviesize;
			
		while(running)
		{	
			double time=Athread::getTime();
			section.enter(__FILE__,__LINE__);
			try
			{
				if(avi->isOK)
				{
					int	i;
					for(i=0; i<lenght; i++)
					{
						int	c=i+begin;
						if(!bloaded[i])
						{
							{	// video
								Abitmap *b=avi->getFrame(c);
								if(b)
								{	
									if(!bitmap[i])
									{
										int	w=mini(videoWidth, b->w);
										int	h=mini(videoHeight, b->h);
										bitmap[i]=new Abitmap(w, h);
									}
									{
										if((bitmap[i]->w==b->w)&&(bitmap[i]->h==b->h))
											bitmap[i]->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
										else
											bitmap[i]->set(0, 0, bitmap[i]->w, bitmap[i]->h, 0, 0, b->w, b->h, b, bitmapNORMAL, bitmapNORMAL);
									}
									bloaded[i]=true;
								}
							}
							{	// audio
								short	buffer[44100][2];
								int		size=avi->setSample44ks(c, buffer, sizeof(buffer));
								if(size)
								{
									if(audio[i].data)
									{
										if(audio[i].size!=size)
										{
											free(audio[i].data);
											audio[i].size=0;
											audio[i].data=NULL;
										}
									}
									if(!audio[i].data)
									{
										audio[i].data=malloc(size);
										audio[i].size=size;
									}
									memcpy(audio[i].data, buffer, size);
								}
							}
							break;
						}
					}
				}
			}
			catch(...)
			{
				dbgprint("catch");
			}
			section.leave();
			{
				if(ht)
				{
					double	dtime=Athread::getTime()-time;
					int		t=maxi(1, 10-(int)dtime);
					sleep(t);
				}
				else
					sleep(30);
			}
		}
	}

	if(avi)
	{	
		section.enter(__FILE__,__LINE__);
		AaviRead	*mavi=avi;
		avi=null;
		section.leave();
		delete(mavi);
	}
	
	CoUninitialize();

	outrun=true;
	running=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmovieStream::setBegin(int pos)
{
	int	i;
	section.enter(__FILE__,__LINE__);
	try
	{
		if(pos!=begin)
		if(avi&&avi->isOK)
		{
			pos=maxi(0, mini(pos, avi->nbframes-1));
			if(pos>begin)
			{
				if(pos-begin<MS_MAXFRAMES)
				{
					int	d=pos-begin;
					int	nb=MS_MAXFRAMES-d;
					
					for(i=0; i<nb; i++)
					{
						Abitmap *b=bitmap[i];
						bitmap[i]=bitmap[i+d];
						bitmap[i+d]=b;
						AudioData	ad=audio[i];
						audio[i]=audio[i+d];
						audio[i+d]=ad;
						bloaded[i]=bloaded[i+d];
					}
					
					for(i=nb; i<MS_MAXFRAMES; i++)
						bloaded[i]=false;
				}
				else
				{
					memset(bloaded, 0, sizeof(bloaded));
				}
			}
			else if(pos<begin)
			{
				if(begin-pos<MS_MAXFRAMES)
				{
					int	d=begin-pos;
					for(i=MS_MAXFRAMES-1; i>=d; i--)
					{
						Abitmap *b=bitmap[i];
						bitmap[i]=bitmap[i-d];
						bitmap[i-d]=b;
						AudioData	ad=audio[i];
						audio[i]=audio[i-d];
						audio[i-d]=ad;
						bloaded[i]=bloaded[i-d];
					}
					
					for(i=0; i<d; i++)
						bloaded[i]=false;
				}
				else
				{
					memset(bloaded, 0, sizeof(bloaded));
				}
			}
			begin=pos;
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AmovieStream::setLenght(int len)
{
	section.enter(__FILE__,__LINE__);
	lenght=maxi(1, mini(len, MS_MAXFRAMES));
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AmovieStream::setBitmap(Abitmap *b, int pos, float level, int blendop)
{
	bool	ok=false;
	section.enter(__FILE__,__LINE__);
	try
	{
		if(avi&&avi->isOK)
		{
			int		ip=pos-begin;
			if((ip>=0)&&(ip<lenght))
			{
				if(bloaded[ip])
				{
					Abitmap	*bs=bitmap[ip];
					if(level==1.f)
						b->set(0, 0, b->w, b->h, 0, 0, bs->w, bs->h, bs, blendop, bitmapNORMAL|bitmapSATURATION);
					else if(level>0.f)
					{
						bs->alpha=level;
						b->set(0, 0, b->w, b->h, 0, 0, bs->w, bs->h, bs, blendop, bitmapGLOBALALPHA|bitmapSATURATION);
					}
					ok=true;
				}
			}
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
	section.leave();
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float AmovieStream::getMaxSample(float p, float w)
{
	int		m=0;
	section.enter(__FILE__,__LINE__);
	try
	{
		if(avi&&avi->isOK)
		{
			p=(p*(float)lenght)+(float)begin;
			w*=(float)lenght;
			int		n0=(int)p;
			int		n1=(int)(p+w);
			sword	*data=null;
			int		i;
			if(n0==n1)
			{
				int	nsmp=getSample(n0, &data)/(sizeof(sword)*2);
				if(nsmp)
				{
					int	s0=(int)((p-(float)n0)*(float)nsmp);
					int	s1=(int)(((p+w)-(float)n1)*(float)nsmp);
					data=&data[s0<<1];
					if(s0==s1)
						m=maxi(data[0], data[1]);
					else
					{
						for(i=s0; i<s1; i++)
						{
							if(data[0]>m)
								m=data[0];
							if(data[1]>m)
								m=data[1];
							data+=2;
						}
					}
				}
			}
			else
			{
				{
					int	nsmp=getSample(n0, &data)/(sizeof(sword)*2);
					int	j;
					int	s0=(int)((p-(float)n0)*(float)nsmp);
					data=&data[s0<<1];
					for(j=s0; j<nsmp; j++)
					{
						if(*data>m)
							m=*data;
						data++;
						if(*data>m)
							m=*data;
						data++;
					}
				}
				for(i=n0+1; i<n1; i++)
				{
					int	nsmp=getSample(i, &data)/(sizeof(sword)*2);
					int	j;
					for(j=0; j<nsmp; j++)
					{
						if(*data>m)
							m=*data;
						data++;
						if(*data>m)
							m=*data;
						data++;
					}
				}
				{
					int	nsmp=getSample(n1, &data)/(sizeof(sword)*2);
					int	j;
					int	s1=(int)((p+w-(float)n1)*(float)nsmp);
					for(j=0; j<s1; j++)
					{
						if(*data>m)
							m=*data;
						data++;
						if(*data>m)
							m=*data;
						data++;
					}
				}
			}
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
	section.leave();
	return (float)abs(m)/32768.f;
}

int AmovieStream::getSample(int pos, sword **buffer)
{
	int		s=0;
	try
	{
		int		ip=pos-begin;
		*buffer=NULL;
		if((ip>=0)&&(ip<lenght))
		{
			if(bloaded[ip]&&audio[ip].data)
			{
				*buffer=(sword *)audio[ip].data;
				s=audio[ip].size; 
			}
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
	return s;
}

void AmovieStream::writeSample(Afilemem *fm, int pos, double s, double d)
{
	try
	{
		int		smpsize=sizeof(sword)*2;
		sword	*buffer=NULL;
		int		size=getSample(pos, &buffer);

		if(size)
		{
			int	nsmp=size/smpsize;
			int	ns=(int)((double)nsmp*s);
			int	nd=(int)((double)nsmp*d);
			if(s<d)			// normal play
				fm->write(&buffer[ns<<1], smpsize*(nd-ns));
			else if(d<s)	// revers play
			{
				int	i;
				for(i=ns-1; i>nd; i--)
				{
					int	id=i<<1;
					fm->write(&buffer[id], smpsize);
				}
			}
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
}

static void calcFade(sword *buf, int nsmp, int fade)
{
	try
	{
		int	max=mini(256, nsmp/2);
		int	i;
		if(fade&AmovieStream::fadeIN)
		{
			for(i=0; i<max; i++)
			{
				int	i0=i<<1;
				float	n=(float)i/(float)max;
				buf[i0+0]=(short)((float)buf[i0+0]*n);
				buf[i0+1]=(short)((float)buf[i0+1]*n);
			}
		}
		if(fade&AmovieStream::fadeOUT)
		{
			for(i=0; i<max; i++)
			{
				int	i0=(nsmp-1-i)<<1;;
				float	n=(float)i/(float)max;
				buf[i0+0]=(short)((float)buf[i0+0]*n);
				buf[i0+1]=(short)((float)buf[i0+1]*n);
			}
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
}

bool AmovieStream::setSample(Asample *smp, double frame, double dframe, double dtime, float level, int fade)
{
	bool	ok=false;
	section.enter(__FILE__,__LINE__);
	try
	{
		if(avi&&avi->isOK)
		{
			sword		buf[44100*8][2];
			Afilemem	*fm=new Afilemem("fm", buf, sizeof(buf));
			double		oframe=frame-dframe;
			int			of=(int)oframe;
			int			f=(int)frame;
			double		st0=(double)(begin);
			double		st1=(double)(begin+lenght);

			if(dframe>0.f)
			{
				if((frame>st1)||(oframe<st0))
				{
					int		n=-1;
					double	len=(double)lenght;
					int		i,j;
					while(oframe<st0)
					{
						oframe+=len;
						n++;
					}
					while(frame>st1)
					{
						frame-=len;
						n++;
					}
					of=(int)oframe;
					f=(int)frame;
					{
						int	sf=begin;
						int	lf=begin+lenght;
						{
							writeSample(fm, of, oframe-of, 1.f);
							for(j=of+1; j<lf; j++)
								writeSample(fm, j, 0.f, 1.f);
						}
						{
							for(i=0; i<n; i++)
								for(j=sf; j<lf; j++)
									writeSample(fm, j, 0.f, 1.f);
						}
						{
							for(j=sf; j<f; j++)
								writeSample(fm, j, 0.f, 1.f);
							if(f<lf)
								writeSample(fm, f, 0.f, frame-f);
						}
					}
				}
				else if(f==of)
				{
					writeSample(fm, f, oframe-of, frame-f);
				}
				else 
				{
					int	i;
					writeSample(fm, of, oframe-of, 1.f);
					for(i=of+1; i<f; i++)
						writeSample(fm, i, 0.f, 1.f);
					if(f<begin+lenght)
						writeSample(fm, f, 0.f, frame-f);
				}
			}
			else if(dframe<0.f)
			{
				if((oframe>st1)||(frame<st0))
				{
					int		n=-1;
					double	len=(double)lenght;
					int		i,j;
					while(oframe>st1)
					{
						oframe-=len;
						n++;
					}
					while(frame<st0)
					{
						frame+=len;
						n++;
					}
					of=(int)oframe;
					f=(int)frame;
					{
						int	sf=begin;
						int	lf=begin+lenght;
						{
							if(of<lf)
								writeSample(fm, of, oframe-of, 0.f);
							for(j=of-1; j>=sf; j--)
								writeSample(fm, j, 1.f, 0.f);
						}
						{
							for(i=0; i<n; i++)
								for(j=lf-1; j>=sf; j--)
									writeSample(fm, j, 1.f, 0.f);
						}
						{
							for(j=lf-1; j>f; j--)
								writeSample(fm, j, 1.f, 0.f);
							writeSample(fm, f, 1.f, frame-f);
						}
					}
				}
				else if(f==of)
				{
					writeSample(fm, f, oframe-of, frame-f);
				}
				else 
				{
					int	i;
					writeSample(fm, of, oframe-of, 0.f);
					for(i=of-1; i>f; i--)
						writeSample(fm, i, 1.f, 0.f);
					writeSample(fm, f, 1.f, frame-f);
				}
			}

			if(fm->getSize())
			{
				sword	outbuf[44100][2];
				int		nout=mini((int)(dtime*44100/1000), sizeof(outbuf)/sizeof(outbuf[0]));	// no buffer overflow
				int		nsmp=(int)(fm->getSize()/sizeof(buf[0]));
				smp->enter(__FILE__,__LINE__);
				if(nout==nsmp)
				{
					calcFade((sword *)buf, nsmp, fade);
					smp->addBuffer((sword *)buf, nsmp);
				}
				else if(nout>nsmp)	// slow play
				{
					double	s=0.f;
					double	ds=(float)(nsmp)/(float)nout;
					int		i;
					int		di=0;
					for(i=0; i<nout; i++)
					{
						int		si=(int)s;
						float	a1=(float)(s-(float)si);
						float	a0=1.f-a1;
						float	v0,v1;
						int		di=si;
						si--;
						if(si<0)
						{
							v0=(float)membuf[0]*a0;
							v1=(float)membuf[1]*a0;
						}
						else
						{
							v0=(float)buf[si][0]*a0;
							v1=(float)buf[si][1]*a0;
						}
						v0+=(float)buf[di][0]*a1;
						v1+=(float)buf[di][1]*a1;
						outbuf[i][0]=(sword)(v0*level);
						outbuf[i][1]=(sword)(v1*level);
						s+=ds;
					}
					calcFade((sword *)outbuf, nout, fade);
					smp->addBuffer((sword *)outbuf, nout);
				}
				else	// nout<nsmp // speed play
				{
					double	s=0.f;
					double	ds=(float)nsmp/(float)nout;
					int		i;
					for(i=0; i<nout; i++)
					{
						double	sn=s+ds;
						int		n0=(int)s;
						int		n1=(int)sn;
						float	a0=(float)(1.0-(s-n0));
						float	v0=buf[n0][0]*a0;
						float	v1=buf[n0][1]*a0;
						int		k;
						for(k=n0+1; k<n1; k++)
						{
							v0+=buf[k][0];
							v1+=buf[k][1];
						}
						if(n1<nsmp)
						{
							float	a1=(float)(sn-n1);
							v0+=buf[n1][0]*a1;
							v1+=buf[n1][1]*a1;
						}
						outbuf[i][0]=(sword)((v0/ds)*level);
						outbuf[i][1]=(sword)((v1/ds)*level);
						s=sn;
					}
					calcFade((sword *)outbuf, nout, fade);
					smp->addBuffer((sword *)outbuf, nout);
				}

				membuf[0]=((sword *)buf)[(nsmp-1)<<1];
				membuf[1]=((sword *)buf)[((nsmp-1)<<1)+1];
				
				smp->leave();
			}
		}
	}
	catch(...)
	{
		dbgprint("catch");
	}
	section.leave();
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avp10Sel : public AcontrolObj
{
public:
	AOBJ
	
	 							Avp10Sel						(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h);
	virtual						~Avp10Sel						();

	virtual void				pulse							();
	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p);

	virtual bool				sequence						(int nctrl, float value);

	int							get								();
	void						set								(int n);

	Abutton						*press[8];
	bool						ctrlPaint;
	int							cur;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Avp10Sel::CI	= ACI("Avp10Sel",		GUID(0x11111112,0x00000874), &AcontrolObj::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Sel::Avp10Sel(QIID qiid, char *name, Aobject *L, int x, int y, int w, int h) : AcontrolObj(name, L, x, y, w, h)
{
	cur=0;
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::KEY_08);
	{
		int	i;
		for(i=0; i<8; i++)
		{
			char	str[256];
			int		x=i&1;
			int		y=i>>1;
			sprintf(str, "%d/rythm pad ", i+1);
			press[i]=new Abutton(str, this, 2+x*26, 2+y*18, 24, 16, &resource.get(MAKEINTRESOURCE(resbut[i]), "PNG"), Abutton::btBITMAP);
			sprintf(str, "rythm pad %d", i+1);
			press[i]->setTooltips(str);
			press[i]->show(true);
		}
	}
	ctrlPaint=false;
	timer(CTRLTIMEREPAINT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Sel::~Avp10Sel()
{
	delete(control);
	{
		int	i;
		for(i=0; i<8; i++)
			delete(press[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Sel::pulse()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Sel::paint(Abitmap *b)
{
	//b->boxf(0, 0, pos.w, pos.h, 0xff404040);
	controlPaint(b, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10Sel::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		control->select();
		{
			int	i;
			for(i=0; i<8; i++)
			{
				if(press[i]==o)
					break;
			}
			if(i<8)
			{
				cur=i;				
				control->set(Acontrol::KEY, (float)cur);
				father->notify(this, nyCHANGE, cur);
			}
		}
		return true;
	}
	return AcontrolObj::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10Sel::sequence(int nctrl, float value)
{
	cur=(int)value;
	father->notify(this, nyCHANGE, cur);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avp10Sel::get()
{
	return cur;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Sel::set(int n)
{
	cur=n;
	control->set(Acontrol::KEY, (float)cur);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avp10SeqStep : public Aobject
{
public:
	AOBJ

								Avp10SeqStep					(char *name, Aobject *L, int x, int y, int w, int h, int ipos);
	virtual						~Avp10SeqStep					();

	void						paint							(Abitmap *b);

	int							color;
	int							ipos;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avp10SeqPat : public Aobject
{
public:
	AOBJ

								Avp10SeqPat						(char *name, Aobject *L, int x, int y, int w, int h);
	virtual						~Avp10SeqPat					();

	int							getStep							(int x, int y);
	bool						mouse							(int x, int y, int state, int event);

	Avp10SeqStep				*step[16];
	int							editcolor;
	bool						editdel;
	bool						editpress;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avp10SeqPos : public Aobject
{
public:
	AOBJ

								Avp10SeqPos						(char *name, Aobject *L, int x, int y, int w, int h);
	virtual						~Avp10SeqPos					();

	virtual void				paint							(Abitmap *b);

	void						setCurrent						(int n);

	int							current;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avp10Seq : public Aobject, public Aidle
{
public:
	AOBJ

								Avp10Seq						(char *name, Aobject *L, int x, int y, int w, int h, Atable *table);
	virtual						~Avp10Seq						();

	virtual void				idlePulse						();
	virtual bool				notify							(Anode *n, int event, dword p);

	void						save							(Afile *f);
	void						load							(Afile *f);

	void						setON							(bool on);
	void						setCurColor						(int color);
	void						setCurPattern					(int pat);
	void						setNbStep						(int nstep);
	void						setStartStep					(int nstart);
	void						setResolution					(double reso);
	void						setEditStep						(int estep);					

	Atable						*table;

	Avp10SeqPat					*pattern;
	Avp10SeqPos					*position;

	byte						seq[32][64];

	double						beat;
	double						resolution;
	int							nstep;
	int							nstart;
	int							estep;
	int							cpat;
	bool						on;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Avp10SeqStep::CI	= ACI("Avp10SeqStep",	GUID(0x11111112,0x00000876), &Aobject::CI, 0, NULL);
ACI								Avp10SeqPat::CI		= ACI("Avp10SeqPat",	GUID(0x11111112,0x00000877), &Aobject::CI, 0, NULL);
ACI								Avp10SeqPos::CI		= ACI("Avp10SeqPos",	GUID(0x11111112,0x00000878), &Aobject::CI, 0, NULL);
ACI								Avp10Seq::CI		= ACI("Avp10Seq",		GUID(0x11111112,0x00000879), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10SeqStep::Avp10SeqStep(char *name, Aobject *L, int x, int y, int w, int h, int ipos) : Aobject(name, L, x, y, w, h)
{
	this->ipos=ipos;
	color=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10SeqStep::~Avp10SeqStep()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void  Avp10SeqStep::paint(Abitmap *b)
{
	if(color)
		b->boxfa(0, 0, pos.w, pos.h, colors[color-1], 0.7f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10SeqPat::Avp10SeqPat(char *name, Aobject *L, int x, int y, int w, int h) : Aobject(name, L, x, y, w, h)
{
	int	i;
	int	mw=pos.w>>4;
	for(i=0; i<16; i++)
	{
		int x=i*26;
		step[i]=new class Avp10SeqStep("step", this, x, 0, 22, 22, i);
		step[i]->show(true);
	}
	editcolor=1;
	editdel=false;
	editpress=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10SeqPat::~Avp10SeqPat()
{
	int	i;
	for(i=0; i<16; i++)
		delete(step[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Avp10SeqPat::getStep(int x, int y)
{
	Aobject	*o=(Aobject *)fchild;
	while(o&&((!(o->state&stateVISIBLE))||(x>=(o->pos.x+o->pos.w))||(x<o->pos.x)||(y>=(o->pos.y+o->pos.h))||(y<o->pos.y)))
		o=(Aobject *)o->next;
	if(o)
		return ((Avp10SeqStep *)o)->ipos+1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10SeqPat::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		mouseCapture(true);
		case mouseMOVE:
		if(state&mouseL)
		{
			int n=getStep(x, y);
			if(n--)
			{
				if(!editpress)
				{
					editpress=true;
					if(step[n]->color)
						editdel=true;
				}
				if(editdel)
				{
					if(step[n]->color)
					{
						step[n]->color=0;
						father->notify(this, nyCHANGE, n);
						step[n]->repaint();
					}
				}
				else if(step[n]->color!=editcolor)
				{
					step[n]->color=editcolor;
					father->notify(this, nyCHANGE, n);
					step[n]->repaint();
				}
			}
		}
		return true;

		case mouseLUP:
		mouseCapture(false);
		editpress=false;
		editdel=false;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10SeqPos::Avp10SeqPos(char *name, Aobject *L, int x, int y, int w, int h) : Aobject(name, L, x, y, w, h)
{
	current=-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10SeqPos::~Avp10SeqPos()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10SeqPos::setCurrent(int n)
{
	if(current!=n)
	{
		current=n;
		repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10SeqPos::paint(Abitmap *b)
{
	if((current>=0)&&(current<16))
	{
		int	x=current*26+2;
		b->boxf(x, 0, x+17, 3, 0xffffff00);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Seq::Avp10Seq(char *name, Aobject *L, int x, int y, int w, int h, Atable *table) : Aobject(name, L, x, y, w, h)
{
	this->table=table;

	memset(seq, 0, sizeof(seq));
	resolution=1.f;
	nstart=1;
	nstep=16;
	estep=0;
	beat=0;
	cpat=0;
	on=0;

	pattern=new class Avp10SeqPat("pattern", this, 0, 0, pos.w, 22);
	pattern->setTooltips("pattern edit");
	pattern->show(true);

	position=new class Avp10SeqPos("position", this, 0, 26, pos.w, 4);
	position->setTooltips("current position");
	position->show(true);

	new Aitem("copy pattern", "copy pattern", context, contextCOPYPATTERN);
	new Aitem("paste pattern", "paste pattern", context, contextPASTEPATTERN);
	new Aitem("randomize pattern", "randomize pattern", context, contextRANDPATTERN);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Seq::~Avp10Seq()
{
	delete(pattern);
	delete(position);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10Seq::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==pattern)
			seq[cpat][estep+p]=pattern->step[p]->color;
		return true;

		case nyCONTEXT:
		{
			Aitem *i=(Aitem *)p;
			switch(i->data)
			{
				case contextCOPYPATTERN:
				memcpy(m_seq, seq[cpat], sizeof(m_seq));
				m_usedseq=true;
				return true;

				case contextPASTEPATTERN:
				if(m_usedseq)
				{
					memcpy(seq[cpat], m_seq, sizeof(m_seq));
					setCurPattern(cpat);
				}
				else
					notify(this, nyALERT, (dword)"no pattern in the copy buffer");
				return true;

				case contextRANDPATTERN:
				{
					int	i;
					for(i=0; i<64; i++)
					{
						if((rand()&3)==0)
							seq[cpat][i]=(rand()&7)+1;
						else
							seq[cpat][i]=0;
					}
					setCurPattern(cpat);
				}
				return true;
			}
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::idlePulse()
{
	if(on&&table->running)
	{
		double b=table->getBeat();
		int	no=(int)(resolution*4.0*beat);
		int	n=(int)(resolution*4.0*b);
		if(no!=n)
		{
			int	i=n;
			while(i!=no)
			{
				int	i0=(i%nstep);
				while(i0<0)	// ???
					i0+=nstep;
				i0=(i0+nstart-1)%64;
				if(seq[cpat][i0])
				{
					father->notify(this, nyPRESS, seq[cpat][i0]-1);
					break;
				}
				i--;
			}
			this->position->setCurrent((((n%nstep)+(nstart-1))%64)-estep);
		}
		beat=b;
	}
	else
		this->position->setCurrent(-1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::save(Afile *f)
{
	f->write(seq, sizeof(seq));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::load(Afile *f)
{
	f->read(seq, sizeof(seq));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setCurColor(int color)
{
	pattern->editcolor=color;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setCurPattern(int pat)
{
	int	i;
	cpat=pat;
	for(i=0; i<16; i++)
		pattern->step[i]->color=seq[cpat][i+estep];
	pattern->repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setNbStep(int nstep)
{
	this->nstep=nstep;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setStartStep(int nstart)
{
	this->nstart=nstart;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setResolution(double reso)
{
	resolution=reso;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setEditStep(int estep)
{
	this->estep=estep;
	setCurPattern(cpat);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Seq::setON(bool on)
{
	beat=table->getBeat();
	this->on=on;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void initStates(Tvp10State *st)
{
	int	i;
	memset(st, 0, sizeof(Tvp10State));
	st->gpitch=0.5f;
	st->begin=0;
	st->gwin=1.f;
	for(i=0; i<8; i++)
	{
		st->position[i]=(float)i/8.f;
		st->pitch[i]=0.75f;
		//st->level[i]=1.f;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10::Avp10(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	m_dframe=0;
	scratch_cue=0.5;
	scratch_delta=0.0;
	movie=null;
	frame=0.f;
	pressed=0;	
	curstate=0;
	memset(events, 0, sizeof(events));
	memset(states, 0, sizeof(states));
	nbevents=0;
	m_pitch=0.75;
	m_level=1.0;
	{
		int	i;
		for(i=0; i<4; i++)
			initStates(&states[i]);
	}
	front=new Avp10Front(qiid, "VP10 front", this, 240);
	front->setTooltips("VP10 player");
	back=new Avp10Back(qiid, "VP10 back", this, 240);
	back->setTooltips("VP10 player");
	tmp=new Abitmap(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10::~Avp10()
{
	section.enter(__FILE__,__LINE__);
	if(movie)
		delete(movie);
	section.leave();
	delete(tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10::load(class Afile *f)
{
	return loadPreset(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10::save(class Afile *f)
{
	return savePreset(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10::savePreset(Afile *f)
{
	Avp10Front	*front=(Avp10Front *)this->front;
	int			nstep=front->nsteps->value;
	int			nstart=front->nstart->value;

	if(!front->play->get())
		saveState();

	if(movie)
		f->writeString(movie->filename);
	else
		f->writeString("");
	{
		int b=0;
		if(movie)
			b=movie->begin;
		f->write(&b, sizeof(b));
	}
	f->write(states, sizeof(states));
	front->sequence->save(f);
	f->write(&nstep, sizeof(nstep));
	f->write(&nstart, sizeof(nstart));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10::loadPreset(Afile *f)
{
	Avp10Front	*front=(Avp10Front *)this->front;
	char		filename[ALIB_PATH];
	int			nstep,nstart;

	f->readString(filename);
	if(!(movie&&!strcmp(movie->filename, filename)))
		newMovie(filename);

	if(filename[0])
	{
		int	n=20;
		while(n--)
		{
			if(movie->isOK())
				break;
			Athread::sleep(10);
		}
	}
	{
		int b=0;
		f->read(&b, sizeof(b));
		if(movie)
			movie->setBegin(b);
	}

	f->read(states, sizeof(states));
	loadState();
	front->sequence->load(f);
	f->read(&nstep, sizeof(nstep));
	front->nsteps->set(nstep);
	front->sequence->setNbStep(nstep);
	f->read(&nstart, sizeof(nstart));
	front->nstart->set(nstart);
	front->sequence->setStartStep(nstart);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::saveState()
{
	Avp10Front	*front=(Avp10Front *)this->front;
	Tvp10State	*st=&states[curstate];
	int			i;

	st->begin=movie?movie->begin:0;
	st->gpitch=front->gpitch->get();
	st->gwin=front->gwin->get();

	for(i=0; i<8; i++)
	{
		st->position[i]=front->position[i]->get();
		st->pitch[i]=front->pitch[i]->get();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::loadState()
{
	Avp10Front	*front=(Avp10Front *)this->front;
	Tvp10State	*st=&states[curstate];
	int			i;

	if(movie)
		movie->setBegin(st->begin);
	front->gpitch->set(st->gpitch);
	front->gwin->set(st->gwin);

	for(i=0; i<8; i++)
	{
		front->position[i]->set(st->position[i]);
		front->pitch[i]->set(st->pitch[i]);
	}

	setViewSize(pow(st->gwin, 3));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::settings(bool emergency)
{
	if(movie)
		movie->setVideoSize(getVideoWidth(), getVideoHeight());
	if(tmp)
		tmp->size(getVideoWidth(), getVideoHeight());
	else
		tmp=new Abitmap(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::action(double time, double dtime, double beat, double dbeat)
{
	Avp10Front	*front=(Avp10Front *)this->front;
	Avp10Back	*back=(Avp10Back *)this->back;
	Abitmap		*bb=back->video->getBitmap();
	Asample		*smp=back->audio;
	bool		ok=false;
	Tvp10Event	ev[VP10MAXEVENT];
	int			nev=0;
	int			m_nbevents=nbevents;

	section.enter(__FILE__,__LINE__);
	{	// filter and sort events
		double	stime=time-dtime;
		int		i,j;
		for(i=0; i<nbevents; i++)
		{
			Tvp10Event	*e=&events[i];
			if(e->time>=stime)
			{
				if(e->time<=time)
				{
					ev[nev].frame=e->frame;
					ev[nev].time=(e->time-stime)/dtime;
					ev[nev].pitch=e->pitch;
					ev[nev].level=e->level;
					nev++;
					e->time=-1.0;
				}
				else if(e->time>=time+dtime*2)
					e->time=-1.0;
			}
			else
				e->time=-1.0;
		}
		for(i=0; i<nbevents; i++)
		{
			if(events[i].time==-1.0)
			{
				bool	b=false;
				for(j=i+1; j<nbevents; j++)
				{
					if(events[j].time!=-1.0)
					{
						events[i]=events[j];
						b=true;
						break;
					}
				}
				if(!b)
					break;
			}
		}
		nbevents=i;
		for(i=0; i<nev-1; i++)
		{
			for(j=i; j>=0; j--)
			if(ev[j].time>ev[j+1].time)
			{
				Tvp10Event	e=ev[j];
				ev[j]=ev[j+1];
				ev[j+1]=e;
			}
		}
	}
	{
		float		gpitch=front->gpitch->get();
		float		shutter=front->shutter->get();
		
		if(bb&&movie&&movie->isOK())
		{
			bool	test=true;
			double	deltacue=getDeltaCue(dtime);


			//if(nev)
			{
				double	sev=0.0;
				int		i;

				for(i=0; i<nev; i++)
				{
					double	d=ev[i].time-sev;
					double	db=dbeat*d;
					double	dframe=((m_pitch-0.5)+(gpitch-0.5))*4.0*(db*60.0*movie->fps/BPMREF)+deltacue;
					frame+=dframe;
					m_dframe=dframe;
					while(test)
					{
						test=false;
						if((int)frame>=movie->begin+movie->lenght)
						{
							frame-=(float)movie->lenght;
							test=true;
						}
						else if((int)frame<movie->begin)
						{
							frame+=(float)movie->lenght;
							test=true;
						}
					}

					movie->setSample(smp, frame, dframe, d*dtime, (float)m_level, AmovieStream::fadeOUT|((i>0)?AmovieStream::fadeIN:0));

					m_pitch=ev[i].pitch,
					m_level=ev[i].level;
					frame=ev[i].frame;
					sev=ev[i].time;
				}

				{
					double	d=1.0-sev;
					double	db=dbeat*d;
					double	dframe=((m_pitch-0.5)+(gpitch-0.5))*4.0*(db*60.0*movie->fps/BPMREF)+deltacue;
					frame+=dframe;
					m_dframe=dframe;
					while(test)
					{
						test=false;
						if((int)frame>=movie->begin+movie->lenght)
						{
							frame-=(float)movie->lenght;
							test=true;
						}
						else if((int)frame<movie->begin)
						{
							frame+=(float)movie->lenght;
							test=true;
						}
					}
					movie->setSample(smp, frame, dframe, d*dtime, (float)m_level, (nev>0)?AmovieStream::fadeIN:0);

					{
						int	f=(int)frame;
						int	nframe=(m_dframe>0.f)?f+1:f-1;

						if(oframe!=f)
						{
							voframe=oframe;
							oframe=f;
						}

						if(shutter==0.f)
						{
							ok=movie->setBitmap(bb, f, (float)m_level, bitmapNORMAL);
						}
						else
						{
							float	d=(float)(frame-(double)f);
							float	ds0=0;
							float	ds1=0;
							if(shutter<0.5f)
							{
								ds0=1.f-shutter;
								ds1=shutter;
							}
							else
							{
								ds0=ds1=0.5f+(shutter-0.5f)*0.5f;
							}
							tmp->boxf(0, 0, tmp->w, tmp->h, 0xff000000);
							ok=movie->setBitmap(tmp, f, ds0, bitmapADD);
							movie->setBitmap(tmp, f+1, (d*ds1), bitmapADD);
							movie->setBitmap(tmp, f-1, ((1.f-d)*ds1), bitmapADD);
							tmp->alpha=(float)m_level;
							bb->set(0, 0, tmp, bitmapNORMAL, bitmapGLOBALALPHA);
						}
					}
				}
			}
			/*
			{	// debug
				Afont	*fo=alib.getFont(fontTERMINAL12);
				char str[256];
				sprintf(str, "%f", (float)frame);
				fo->set(bb, 10, 10, str, 0xff00ff00);
				sprintf(str, "fps %f", movie->fps);
				fo->set(bb, 10, 30, str, 0xff00ff00);
				sprintf(str, "nbevents %d  nev %d", m_nbevents, nev);
				fo->set(bb, 10, 50, str, 0xff00ff00);
				if(nev)
				{
					sprintf(str, "ev %f", ev[0].time);
					fo->set(bb, 10, 70, str, 0xff00ff00);
					sprintf(str, "ev %f", ev[nev-1].time);
					fo->set(bb, 10, 90, str, 0xff00ff00);
				}
			}
			*/
		}
	}
	section.leave();

	//if(!ok)
	//	bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::newMovie(char *file)
{
	section.enter(__FILE__,__LINE__);
	if(movie)
		delete(movie);
	movie=new AmovieStream(file);
	movie->setVideoSize(getVideoWidth(), getVideoHeight());
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Avp10::press(int npress)
{
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
	{
		if(nbevents<VP10MAXEVENT)
		{
			Avp10Front	*front=(Avp10Front *)this->front;
			pressed=npress;
			float	pos=front->position[pressed]->get();
			events[nbevents].time=getTable()->getTime();
			events[nbevents].frame=(double)movie->begin+pos*(double)movie->lenght;
			events[nbevents].pitch=front->pitch[pressed]->get();
			events[nbevents].level=front->level[pressed]->get();
			nbevents++;
		}
	}
	else
		frame=0.f;
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double Avp10::getDeltaCue(double dtime)
{
	double rf=0.0;
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
	{
		Avp10Front	*front=(Avp10Front *)this->front;
		double	pos=front->scratch->get();
		scratch_delta+=pos-scratch_cue;
		scratch_cue=pos;
		double d=scratch_delta*0.008*dtime;
		scratch_delta-=d;
		rf=(double)d*(double)movie->lenght;
	}
	section.leave();
	return rf;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10::setBitmap(Abitmap *b, float pos)
{
	bool	ok=false;
	pos=maxi(0.f, mini(pos, 1.f));
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK()&&movie->setBitmap(b, movie->begin+(int)(pos*(float)movie->lenght), 1.f, bitmapNORMAL))
		ok=true;
	section.leave();
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Avp10::getPos()
{
	float	pos=0.f;
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
		pos=(float)((frame-(double)movie->begin)/(double)movie->lenght);
	section.leave();
	return pos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Avp10::getViewPos()
{
	float	pos=0.f;
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
		pos=(float)movie->begin/(float)movie->moviesize;
	section.leave();
	return pos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Avp10::getViewSize()
{
	float	size=0.f;
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
		size=(float)movie->lenght/(float)movie->moviesize;
	section.leave();
	return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::setViewPos(float pos)
{
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
		movie->setBegin((int)(pos*(float)movie->moviesize));
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::setViewSize(float size)
{
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
		movie->setLenght((int)(size*(float)movie->moviesize));
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10::performPlay()
{
	section.enter(__FILE__,__LINE__);
	if(movie&&movie->isOK())
	{
		Avp10Front	*front=(Avp10Front *)this->front;
		float		gpitch=front->gpitch->get();
		float		pitch=front->pitch[pressed]->get();
		if(m_dframe>0)
		{
			float	pos=getPos();
			if(pos>0.7)
			{
				float	nframe=0.4f*(float)movie->lenght;
				float	vp=getViewPos()+nframe/movie->moviesize;
				float	vs=getViewSize();
				vp=maxi(mini(vp, 1.f-vs), 0.f);
				setViewPos(vp);
			}
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Front::Avp10Front(QIID qiid, char *name, Avp10 *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VP10_FRONT), "PNG");
	back=new Abitmap(&o);
	int		i;
	
	npulse=0;
	
	sample=new Abitmap(disp.w, 40);
	sample->flags=bitmapALPHA;
	
	for(i=0; i<6; i++)
		preview[i]=new Abitmap(64,48);

	sel=new Avp10Sel(MKQIID(qiid,0x4851ab151de86e00), "play", this, 444, 160, 54, 74);
	sel->setTooltips("play cue point");
	sel->set(0);
	sel->show(true);
	
	for(i=0; i<8; i++)
	{
		char	str[256];

		sprintf(str, "%d/position", i+1);
		position[i]=new Apaddle(MKQIID(qiid,0x48d5565615f86e00+i*17), str, this, 100, 122, paddleYOLI24);
		sprintf(str, "position %d", i+1);
		position[i]->set((float)i/8);
		position[i]->setTooltips(str);

		sprintf(str, "%d/pitch", i+1);
		pitch[i]=new Apaddle(MKQIID(qiid,0x48d558156121abc0+i*17), str, this, 140, 122, paddleYOLI24);
		sprintf(str, "pitch %d", i+1);
		pitch[i]->set(0.75f);
		pitch[i]->setTooltips(str);

		sprintf(str, "%d/level", i+1);
		level[i]=new Apaddle(MKQIID(qiid,0x488652ac54ed2cc0+i*17), str, this, 180, 122, paddleYOLI24);
		sprintf(str, "level %d", i+1);
		level[i]->set(1.f);
		level[i]->setTooltips(str);
	}

	position[0]->show(true);
	pitch[0]->show(true);
	level[0]->show(true);

	states=new Aselect(MKQIID(qiid,0x48685a45e6d510c0), "states", this, 401, 121, 2, 2, &resource.get(MAKEINTRESOURCE(PNG_VP10PATTERN), "PNG"), 14, 15, Aselect::dragdropNONE, "A,B,C,D");
	states->set(0);
	states->setTooltips("states [bank of cue points position and pitch values]");
	states->show(true);
	
	gwin=new Apaddle(MKQIID(qiid,0x48d556ab58abcdc9), "view size", this, 474, 122, paddleYOLI24);
	gwin->set(0.5f);
	gwin->setTooltips("view size");
	gwin->show(true);

	gpitch=new Apaddle(MKQIID(qiid,0x48d556ab58626e00), "global pitch", this, 446, 122, paddleYOLI24);
	gpitch->set(0.5f);
	gpitch->setTooltips("global pitch");
	gpitch->show(true);
	
	scratch=new Apaddle(MKQIID(qiid,0x4545de84c8626e00), "scratch", this, 222, 122, paddleYOLI24);
	scratch->set((float)e->scratch_cue);
	scratch->setTooltips("scratch");
	scratch->show(true);
	
	scbar=new AscrollBar("view position", this, disp.x, disp.y+disp.h, disp.w, 6, AscrollBar::HORIZONTAL);
	scbar->setTooltips("view position");	
	scbar->setView(1.f);
	scbar->show(true);

	curcolor=new AselButton("curcolor", this, 14, 124, 4, 2, &resource.get(MAKEINTRESOURCE(PNG_BUTMETAL_ALL), "PNG"), 16, 12);
	curcolor->set(0);
	curcolor->setTooltips("edit cue point");
	curcolor->show(true);
	
	bank=new Aselect(MKQIID(qiid,0x4862ac58d4a569a0), "bank", this, 16, 162, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_VP10PATTERN), "PNG"), 14, 15, Aselect::dragdropNONE, "A,B,C,D");
	bank->set(0);
	bank->setTooltips("bank");
	bank->show(true);
	
	pattern=new Aselect(MKQIID(qiid,0x4862585a85c05620), "pattern", this, 16, 180, 8, 1, &resource.get(MAKEINTRESOURCE(PNG_VP10PATTERN), "PNG"), 14, 15, Aselect::dragdropNONE, "1,2,3,4,5,6,7,8");
	pattern->set(0);
	pattern->setTooltips("pattern");
	pattern->show(true);

	patton=new ActrlButton(MKQIID(qiid, 0x5ad8454add847300), "pattern mode", this, 410, 162, 14, 27, &resource.get(MAKEINTRESOURCE(PNG_VP10_BUTTON), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	patton->setTooltips("pattern mode");
	patton->show(TRUE);

	shutter=new Apaddle(MKQIID(qiid,0x4584d8e56ca26e00), "shutter", this, 260, 122, paddleYOLI24);
	shutter->set(0.f);
	shutter->setTooltips("shutter");
	shutter->show(TRUE);

	sequence=new Avp10Seq("sequence", this, 17, 201, 412, 29, e->getTable());
	sequence->show(true);

	play=new ActrlButton(MKQIID(qiid, 0x5ad05654845edde0), "play", this, 357, 121, 30, 32, &resource.get(MAKEINTRESOURCE(PNG_VP10_PLAY), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	play->setTooltips("play all movie (auto load)");
	play->show(TRUE);

	nstart=new Aseg7("nstart", this, 160, 170, 33, 17, true);
	nstart->setTooltips("loop start");
	nstart->set(1);
	nstart->show(true);

	nsteps=new Aseg7("nsteps", this, 200, 170, 33, 17);
	nsteps->setTooltips("loop lenght");
	nsteps->show(true);

	selstep=new AselButton("selstep", this, 246, 171, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_VP10PATTERN), "PNG"), 14, 15, "1,2,3,4");
	selstep->setTooltips("select edit steps");
	selstep->show(true);

	reso=new Aselpad(MKQIID(qiid, 0x8f5648a4d8c39bcc), "resolution", this, 318, 166, 9, paddleYOLI24);
	reso->setTooltips("pattern step resolution [0.125, 0.25, 0.5, 0.75, 1, 1.5, 2, 4, 8]");
	reso->set(4);
	reso->show(true);

	new Aitem("reset cue points", "reset cue points", context, this, nyRESET);

	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Front::~Avp10Front()
{
	int	i;
	delete(back);
	delete(sample);
	delete(sel);
	delete(states);
	delete(gpitch);
	delete(gwin);
	delete(scbar);
	for(i=0; i<8; i++)
	{
		delete(position[i]);
		delete(pitch[i]);
		delete(level[i]);
	}
	for(i=0; i<6; i++)
		delete(preview[i]);
	delete(curcolor);
	delete(bank);
	delete(pattern);
	delete(patton);
	delete(shutter);
	delete(scratch);
	delete(sequence);
	delete(nsteps);
	delete(nstart);
	delete(play);
	delete(selstep);
	delete(reso);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void convTimeFormat(char *str, float n)
{
	int	h=(int)(n/3600.f);
	n-=(float)(h*3600);
	int	m=(int)(n/60.f);
	n-=(float)(m*60);
	sprintf(str, "%2d:%2d:%4.1f", h, m, n);
	{
		char	*s=str;
		while(*s)
		{
			if(*s==32)
				*s='0';
			s++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Front::paint(Abitmap *b)
{
	Avp10	*vp10=(Avp10 *)effect;
	int		i;

	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	
	for(i=0; i<6; i++)
		b->set(7+disp.x+i*70, disp.y+4, preview[i]);
		
	b->set(disp.x, disp.y+56, sample);
	
	for(i=0; i<8; i++)
	{
		int		x=(int)((float)(disp.w-1)*position[i]->get());
		b->line(disp.x+x, disp.y, disp.x+x, disp.y+disp.h-1, colors[i]);
	}
	
	{
		int		x=(int)((float)(disp.w-1)*((Avp10 *)effect)->getPos());
		b->line(disp.x+x, disp.y, disp.x+x, disp.y+disp.h-1, 0xffffffff);
	}

	{
		Afont	*fn=alib.getFont(fontTERMINAL06);
		Afont	*fg=alib.getFont(fontARMY10);
		Afont	*fs=alib.getFont(fontSEGMENT06);

		//fg->set(b, 440, 20, "TEST", 0xffffffff);

		{
			char	str[256];
			float	frame=0;
			float	begin=0;
			float	lenght=0;
			float	total=0;

			vp10->section.enter(__FILE__,__LINE__);
			if(vp10->movie&&vp10->movie->isOK())
			{
				float	fps=vp10->movie->fps;
				frame=(float)vp10->frame/fps;
				begin=(float)vp10->movie->begin/fps;
				lenght=(float)vp10->movie->lenght/fps;
				total=(float)vp10->movie->moviesize/fps;
			}
			vp10->section.leave();

			convTimeFormat(str, frame);
			fs->setFixed(b, 440,10, str, 0xff00e000);
			convTimeFormat(str, begin);
			fs->setFixed(b, 440,19, str, 0xffe0e000);
			convTimeFormat(str, begin+lenght);
			fs->setFixed(b, 440,28, str, 0xffe0e000);
			convTimeFormat(str, total);
			fs->setFixed(b, 440,37, str, 0xffe0e0e0);
		}

		//fn->set(b, 442, 88, "EDIT", 0xff808080);
		{	// edit color/track
			char	str[128];
			int	n=curcolor->get();
			sprintf(str, "%d", n+1);
			fg->setAlign(b, 444, 98, 20, str, colors[n], Afont::alignCENTER);
		}
		//fn->set(b, 472, 88, "PLAY", 0xff808080);
		{	// play color/track
			char	str[128];
			int	n=vp10->pressed;
			sprintf(str, "%d", n+1);
			fg->setAlign(b, 482, 98, 20, str, colors[n], Afont::alignCENTER);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Avp10Front::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCONTEXT:
		{
			Aitem *i=(Aitem *)p;
			switch(i->data)
			{
				case nyRESET:
				{
					int a;
					for(a=0; a<8; a++)
						position[a]->set((float)a/8);
				}
				return true;
			}
		}
		break;

		case nyPRESS:
		if(o==sequence)
		{
			((Avp10 *)effect)->press(p);
		}
		return true;

		case nyCHANGE:
		if(o==patton)
		{
			sequence->setON(patton->get());
		}
		else if(o==nsteps)
		{
			sequence->setNbStep(p);
		}
		else if(o==nstart)
		{
			sequence->setStartStep(p);
		}
		else if(o==states)
		{
			Avp10	*vp10=(Avp10 *)effect;
			int		n=states->get();
			vp10->section.enter(__FILE__,__LINE__);
			if(n!=vp10->curstate)
			{
				if(!play->get())
					vp10->saveState();
				vp10->curstate=n;
				vp10->loadState();
			}
			vp10->section.leave();
		}
		else if(o==selstep)
		{
			sequence->setEditStep(selstep->get()*16);
		}
		else if(o==curcolor)
		{
			int	n=curcolor->get();
			int	i;
			for(i=0; i<8; i++)
			{
				bool		b=(i==n);
				position[i]->show(b);
				pitch[i]->show(b);
				level[i]->show(b);
			}
			sequence->setCurColor(n+1);
			repaint();
		}
		else if(o==gpitch)
		{
		}
		else if(o==gwin)
		{
			((Avp10 *)effect)->setViewSize(pow(gwin->get(), 3));
		}
		else if(o==scbar)
		{
			((Avp10 *)effect)->setViewPos(scbar->getPosition());
		}
		else if(o==sel)
		{
			((Avp10 *)effect)->press(sel->get());
		}
		else if((o==bank)||(o==pattern))
		{
			sequence->setCurPattern(bank->get()*8+pattern->get());
		}
		else if(o==reso)
		{
			sequence->setResolution(resofreq[reso->get()]);
		}
		else
		{
			int	i;			for(i=0; i<8; i++)
				if(o==position[i])
				{
					repaint(disp);
					break;
				}
		}
		return TRUE;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Avp10Front::dragdrop(int x, int y, int state, int event, void *data)
{
	switch(event)
	{
		case dragdropENTER:
		case dragdropOVER:
		if(state&dragdropFILE)	
			return true;
		break;

		case dragdropDROP:
		if(state&dragdropFILE)
		{
			((Avp10 *)effect)->newMovie((char *)data);
			gwin->set(1.f);
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Front::pulse()
{
	Avp10	*vp=(Avp10 *)effect;
	if(npulse++>10)
	{
		float	vpos=0.f;
		float	vsize=1.f;
		int		i;
		vp->section.enter(__FILE__,__LINE__);
		if(vp->movie&&vp->movie->avi&&vp->movie->avi->error)
		{
			notify(this, nyWARNING, (dword)vp->movie->avi->error);
			vp->movie->avi->error=null;
		}
		if(vp->movie&&vp->movie->isOK())
		{
			for(i=0; i<6; i++)
			{
				Abitmap	*b=preview[i];
				if(!vp->setBitmap(b, (float)i/(float)6))
					b->boxf(0, 0, b->w, b->h, 0xff000000);
			}
			{
				float	p=0.f;
				float	w=1.f/(float)sample->w;
				int		dh=sample->h>>1;
				sample->boxf(0, 0, sample->w, sample->h, 0x00000000);
				for(i=0; i<sample->w; i++)
				{
					int	h=(int)(vp->movie->getMaxSample(p, w)*(float)dh);
					sample->line(i, dh-h, i, dh+h-1, 0xffc0c0c0);
					p+=w;
				}
			}
			vpos=vp->getViewPos();
			vsize=vp->getViewSize();

			vp->saveState();
		}
		else
		{
			for(i=0; i<6; i++)
			{
				Abitmap	*b=preview[i];
				b->boxf(0, 0, b->w, b->h, 0xff000000);
			}
			sample->boxf(0, 0, sample->w, sample->h, 0x00000000);
		}
		vp->section.leave();
		scbar->setView(vsize);
		scbar->setPosition(vpos);
		//gwin->set(vsize);
		scbar->repaint();
		npulse=0;
	}
	if(play->get())
		vp->performPlay();
	{
		Arectangle	r=disp;
		r.w+=80;
		repaint(r);
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

Avp10Back::Avp10Back(QIID qiid, char *name, Avp10 *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VP10_BACK), "PNG");
	back=new Abitmap(&o);

	video=new Avideo(MKQIID(qiid, 0x2bcddbc5189c68a0), "video out", this, pinOUT, pos.w-18, 10);
	video->setTooltips("video out");
	video->show(TRUE);

	audio=new Asample(MKQIID(qiid, 0x2bcd5153189c68a0), "audio out", this, pinOUT, pos.w-38, 10);
	audio->setTooltips("audio out");
	audio->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Avp10Back::~Avp10Back()
{
	delete(back);
	delete(video);
	delete(audio);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Avp10Back::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * Avp10Info::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Avp10(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * vp10GetInfo()
{
	return new Avp10Info("vp10Info", &Avp10::CI, "VP10 player", "VP10 player");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
