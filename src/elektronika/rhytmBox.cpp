/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	rhytmbox.CPP				(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<ddraw.h>
#include						"rhytmbox.h"
#include						"resource.h"
#include						"global.h"
#include						<math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								ArhytmboxInfo::CI	= ACI("ArhytmboxInfo",	GUID(0x11111112,0x00000030), &AeffectInfo::CI, 0, NULL);
ACI								Arhytmbox::CI		= ACI("Arhytmbox",		GUID(0x11111112,0x00000031), &Aeffect::CI, 0, NULL);
ACI								ArhytmboxFront::CI	= ACI("ArhytmboxFront",	GUID(0x11111112,0x00000032), &AeffectFront::CI, 0, NULL);
ACI								ArhytmboxBack::CI	= ACI("ArhytmboxBack",	GUID(0x11111112,0x00000033), &AeffectBack::CI, 0, NULL);

ACI								Aseg7::CI		= ACI("Aseg7",	GUID(0x11111112,0x00000038), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Arnbreso : public AcontrolObj
{
public:
	AOBJ

								Arnbreso						(QIID qiid, char *name, Aobject *L, int x, int y, int size, dword color, int nbpos);
	virtual						~Arnbreso						();

	virtual void				pulse							();
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual bool				keyboard						(int event, int ascii, int scan, int state);

	virtual bool				sequence						(int nctrl, float value);

	virtual void				set								(int p);
	virtual int					get								();

	virtual void				paint							(Abitmap *b);

private:

	dword						lineColor;
	int							nbpos;
	int							ipos;
	int							defpos;

	float						yy;
	int							y0;
	float						yy0;

	bool						test;
	
	bool						ctrlPaint;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Arnbreso::CI		= ACI("Arnbreso",	GUID(0x11111112,0x00000039), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Arnbreso::Arnbreso(QIID qiid, char *name, Aobject *L, int x, int y, int size, dword color, int nbpos) : AcontrolObj(name, L, x-size, y-size, size<<1, size<<1)
{
	control=new Acontrol(qiid, name, Aeffect::getEffect(L), this, Acontrol::KEY_01+nbpos-1);
	lineColor=color;
	this->nbpos=nbpos;
	ctrlPaint=false;
	set(0);
	defpos=-1;
	test=false;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Arnbreso::~Arnbreso()
{
	delete(control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arnbreso::keyboard(int event, int ascii, int scan, int state)
{
	switch(event)
	{
		case keyboardDOWN:
		if(state&keyboardALT)
		{
			if(defpos!=-1)
				set(defpos);
			else
				set(0);
			father->notify(this, nyCHANGE);
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arnbreso::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		control->select();
		case mouseNORMAL:
		case mouseLUP:
		cursor(cursorHANDSEL);
		focus(this);
		{
			if(getKeyboardState()&keyboardLALT)
			{
				if(defpos!=-1)
					set(defpos);
				else
					set(0);
				father->notify(this, nyCHANGE);
				return true;
			}
		}
		switch(event)
		{
			case mouseLDOWN:
			mouseCapture(true);
			test=TRUE;
			y0=y;
			yy0=(float)ipos/(float)nbpos;
			break;
		}
		if(test&&(state&mouseL))
		{
			yy=mini(maxi(yy0-alib.mouseSensibility*(float)(y-y0), 0.f), 0.99f);
			set((int)(yy*nbpos));
			father->notify(this, nyCHANGE);
		}
		switch(event)
		{
			case mouseLUP:
			mouseCapture(FALSE);
			test=FALSE;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arnbreso::set(int v)
{
	ipos=maxi(mini(v, nbpos-1), 0);
	if(defpos==-1)
		defpos=ipos;
	control->set(Acontrol::KEY, (float)ipos);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Arnbreso::get()
{
	return ipos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arnbreso::sequence(int nctrl, float value)
{
	ipos=(int)value;
	ipos=maxi(mini(ipos, nbpos-1), 0);
	ctrlPaint=true;
	father->notify(this, nyCHANGE);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arnbreso::pulse()
{
	if(ctrlPaint)
	{
		repaint();
		ctrlPaint=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arnbreso::paint(Abitmap *bitmap)
{
	dword	cf=((lineColor>>1)&0x007f7f7f)|0xff000000;
	int		s=pos.w>>1;
	double	a=PI*1.37-(double)ipos*PI*1.9/(double)nbpos;
	int		px=(int)(cos(a)*s+s);
	int		py=(int)(-sin(a)*s+s);
	bitmap->linea((int)(cos(a+PI*0.5)+s), (int)(-sin(a+PI*0.5)+s), px, py, cf, 0.8f);
	bitmap->linea((int)(cos(a-PI*0.5)+s), (int)(-sin(a-PI*0.5)+s), px, py, cf, 0.8f);
	bitmap->line(s, s, px, py, lineColor);
	controlPaint(bitmap, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aseg7::Aseg7(char *name, Aobject *f, int x, int y, int w, int h, bool bstart) : Aobject(name, f, x, y, w, h)
{
	this->bstart=bstart;
	up=new Abutton("up", this, w-8, 0, 8, 8, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN_SWITCH), "PNG"), Abutton::btBITMAP);
	up->setTooltips("increment value");
	up->show(TRUE);
	down=new Abutton("down", this, w-8, h-8, 8, 8, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN_SWITCH), "PNG"), Abutton::btBITMAP);
	down->setTooltips("decrement value");
	down->show(TRUE);
	seg=new Asegment("segment", this, 1, 1, 2, 1, 64, alib.getFont(fontSEGMENT10), 0xffFFFC00, 1.f);
	seg->set(16);
	seg->show(true);
	value=16;
	wait=0;
	sens=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aseg7::~Aseg7()
{
	delete(up);
	delete(down);
	delete(seg);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aseg7::pulse()
{
	if((wait++)>8)
	{
		int	v=value;
		if(bstart)
			value=maxi(mini((((value-1)/4)+sens)*4+1, 64), 1);
		else
			value=maxi(mini(((value/4)+sens)*4, 64), 1);
		if(v!=value)
		{
			father->notify(this, nyCHANGE, value);
			seg->set(value);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aseg7::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==up)
		{
			value=seg->get();
			if(value<64)
			{
				if(bstart)
					value=maxi(mini((((value-1)/4)+1)*4+1, 64), 1);
				else
					value=maxi(mini(((value/4)+1)*4, 64), 1);
				father->notify(this, nyCHANGE, value);
				seg->set(value);
			}
		}
		else if(o==down)
		{
			value=seg->get();
			if(value>1)
			{
				if(bstart)
					value=maxi(mini((((value-1)/4)-1)*4+1, 64), 1);
				else
					value=maxi(mini(((value/4)-1)*4, 64), 1);
				father->notify(this, nyCHANGE, value);
				seg->set(value);
			}
		}
		return true;

		case nyPRESS:
		sens=(o==up)?1:-1;
		wait=0;
		timer(10);
		return true;

		case nyRELEASE:
		timer(0);
		return true;

		case nyCHANGE:
		if(o==seg)
		{
			value=p;
			father->notify(this, nyCHANGE, value);
		}
		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aseg7::paint(Abitmap *b)
{
	b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.7f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aseg7::set(int v)
{
	value=maxi(mini(v, 64), 1);
	seg->set(value);
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

class AseqPulse : public Aobject
{
	AOBJ

								AseqPulse						(char *name, ArhytmboxFront	*front, int x, int y, int w, int h);
	virtual						~AseqPulse						();

	void						paint							(Abitmap *b);
	void						pulse							();
	
	
	Arhytmbox					*rnbox;
	ArhytmboxFront				*front;
	int							current;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AseqPulse::CI		= ACI("AseqPulse",	GUID(0x11111112,0x00000037), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AseqPulse::AseqPulse(char *name, ArhytmboxFront *f, int x, int y, int w, int h) : Aobject(name, f, x, y, w, h)
{
	current=-1;
	front=f;
	rnbox=(Arhytmbox *)f->effect;
	timer(20);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AseqPulse::~AseqPulse()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AseqPulse::pulse()
{
	int	m=current;
	if(front->buttonPattern->get())
	{
		int	steps=3-front->steps->ipos;
		current=((int)rnbox->currentStep)-steps*16;
	}
	else
		current=-1;
	if(m!=current)
		repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AseqPulse::paint(Abitmap *b)
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

class RBvideo
{
public:

	Amovie						*movie;

	sbyte						seq[64];

	float						level;
	float						levelvel;
	float						lenght;
	float						start;
	float						startvel;
	float						pitch;
	float						s1;
	float						s2;
	float						pan;
	int							filter;
	bool						pingpong;
	bool						mute;
	bool						solo;

	double						cframe;

								RBvideo()
								{
									movie=NULL;
									memset(seq, 0, sizeof(seq));
									level=0.f;
									levelvel=0.5f;
									lenght=1.f;
									start=0.f;
									startvel=0.0f;
									pitch=0.55f;
									s1=0.5f;
									s2=0.5f;
									pan=0.5f;
									cframe=0.f;
									filter=0;
									pingpong=false;
									mute=false;
									solo=false;
								}

								~RBvideo()
								{
									if(movie)
										delete(movie);
								}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Asequence : public Aobject
{
	AOBJ

								Asequence						(char *name, ArhytmboxFront *e, int x, int y);
	virtual						~Asequence						();

	bool						mouse							(int x, int y, int state, int event);
	void						paint							(Abitmap *b);

	sbyte						seq[64];
	int							style;
	int							etat;
	int							step;
	Abitmap						*bmp[4];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Asequence::CI		= ACI("Asequence",	GUID(0x11111112,0x00000036), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asequence::Asequence(char *name, ArhytmboxFront *e, int x, int y) : Aobject(name, e, x, y, 16*26-2, 24)
{
	memset(seq, 0, sizeof(seq));
	etat=-1;
	style=1;
	step=0;
	bmp[0]=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_RNBOX_SEQ1), "PNG"));
	bmp[1]=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_RNBOX_SEQ2), "PNG"));
	bmp[2]=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_RNBOX_SEQ3), "PNG"));
	bmp[3]=new Abitmap(&resource.get(MAKEINTRESOURCE(PNG_RNBOX_SEQ4), "PNG"));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Asequence::~Asequence()
{
	delete(bmp[0]);
	delete(bmp[1]);
	delete(bmp[2]);
	delete(bmp[3]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asequence::paint(Abitmap *b)
{
	int	i;
	int	n=step*16;
	for(i=0; i<16; i++)
	{
		if(seq[n+i])
		{
			int	x=i*26;
			b->set(x+1, 1, bmp[seq[n+i]-1], bitmapDEFAULT, bitmapDEFAULT);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Asequence::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			int	n=maxi(mini(x/26, 15), 0)+step*16;
			etat=seq[n]?0:style;
			mouseCapture(true);
				father->notify(this, nyCHANGE, n);
			seq[n]=etat;
			repaint();
		}
		return true;

		case mouseNORMAL:
		if(etat!=-1)
		{
			if((y>=0)&&(y<pos.h))
			{
				int	n=x/26;
				int	nn=n+step*16;
				if((n>=0)&&(n<=15)&&(seq[nn]!=etat))
				{
					seq[nn]=etat;
					father->notify(this, nyCHANGE, nn);
					repaint();
				}
			}
		}
		return true;

		case mouseLOSTCAPTURE:
		case mouseLUP:
		if(etat!=-1)
		{
			if((y>=0)&&(y<pos.h))
			{
				int	n=x/26;
				int	nn=n+step*16;
				if((n>=0)&&(n<=15)&&(seq[nn]!=etat))
				{
					seq[nn]=etat;
					father->notify(this, nyCHANGE, nn);
					repaint();
				}
			}
			mouseCapture(false);
			etat=-1;
		}
		return true;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ArhytmB : public Aobject
{
	AOBJ

								ArhytmB							(QIID qiid, char *name, ArhytmboxFront *e, int x, int y, int nbox);
	virtual						~ArhytmB						();

	bool						notify							(Anode *o, int event, dword p);
	void						paint							(Abitmap *b);
	virtual bool				dragdrop						(int x, int y, int state, int event, void *data=NULL);
	virtual bool				mouse							(int x, int y, int state, int event);

	void						pulse							();
	int							nbox;

	Astatic						*affiche;

	Apaddle						*level;
	Apaddle						*s1;
	Apaddle						*s2;
	Apaddle						*pan;
	Apaddle						*levelv;
	Apaddle						*levelvel;
	Apaddle						*lenght;
	Apaddle						*pitch;
	Apaddle						*start;
	Apaddle						*startvel;
	Abutton						*buttonOpen;
	Abutton						*buttonSelect;
	ActrlButton					*buttonLoop;
	ActrlButton					*buttonMute;
	ActrlButton					*buttonSolo;
	Aselect						*filter;
	Apaddle						*senda;
	Apaddle						*sendb;

	Abitmap						*snap;
	bool						bSnapChange;
	
	int							played;
	int							mplayed;

	Asection					section;

	char						filename[ALIB_PATH];
	sbyte						seq[4][8][64];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								ArhytmB::CI			= ACI("ArhytmB",	GUID(0x11111112,0x00000035), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static dword					seqColors[5]={ 0xff808080, 0xff38CF45, 0xffB8CF45, 0xffC45145, 0xffC443AD };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArhytmB::ArhytmB(QIID qiid, char *name, ArhytmboxFront *rbf, int x, int y, int nbox) : Aobject(name, rbf, x, y, 65, 308)
{
	char	str[128];
	this->nbox=nbox;
	mplayed=played=0;
	sprintf(str, "%s/affiche", name);
	affiche=new Astatic(str, this, 4, 6, 56, 12);
	affiche->show(true);
	affiche->set("nothing", 0xffff0000, alib.getFont(fontTERMINAL06));
	affiche->setTooltips("movie title");
	setTooltips("movie");
	sprintf(str, "%s/luminosity", name);
	s1=new Apaddle(MKQIID(qiid,0x8a0ae90c4ccc6990), str, this, 7, 114, paddleYOLI16);
	s1->set(0.5f);
	s1->setTooltips("luminosity");
	s1->show(true);
	sprintf(str, "%s/hue", name);
	s2=new Apaddle(MKQIID(qiid,0xea788824da20a1c0), str, this, 42, 114, paddleYOLI16);
	s2->set(0.5f);
	s2->setTooltips("hue");
	s2->show(true);
	sprintf(str, "%s/saturation", name);
	pan=new Apaddle(MKQIID(qiid,0x304dcaeade24754f), str, this, 25, 124, paddleYOLI16);
	pan->set(0.5f);
	pan->setTooltips("saturation");
	pan->show(true);
	sprintf(str, "%s/level", name);
	levelv=new Apaddle(MKQIID(qiid,0x48d88cfe08f86e00), str, this, 16, 178, paddleYOLI32);
	levelv->setTooltips("level");
	levelv->show(true);
	sprintf(str, "%s/zoom", name);
	levelvel=new Apaddle(MKQIID(qiid,0xac8c1fe1c10f52e0), str, this, 40, 207, paddleYOLI16);
	levelvel->set(0.5f);
	levelvel->setTooltips("zoom");
	levelvel->show(true);
	sprintf(str, "%s/length", name);
	lenght=new Apaddle(MKQIID(qiid,0x68000a655de95000), str, this, 40, 267, paddleYOLI16);
	lenght->set(1.f);
	lenght->setTooltips("lenght");	
	lenght->show(true);
	sprintf(str, "%s/pitch", name);
	pitch=new Apaddle(MKQIID(qiid,0xed04d61b5e810100), str, this, 16, 220, paddleYOLI24);
	pitch->set(0.55f);
	pitch->setTooltips("pitch");
	pitch->show(true);
	sprintf(str, "%s/start", name);
	start=new Apaddle(MKQIID(qiid,0xa88a928ac00271d8), str, this, 16, 250, paddleYOLI24);
	start->set(0.f);
	start->setTooltips("start point");
	start->show(true);
	sprintf(str, "%s/lame", name);
	startvel=new Apaddle(MKQIID(qiid,0xbebacacc45c5770d), str, this, 40, 237, paddleYOLI16);
	startvel->set(0.0f);
	startvel->setTooltips("lame");
	startvel->show(true);
	buttonOpen=new Abutton("open", this, 44, 20, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_OPEN), "PNG"));
	buttonOpen->setTooltips("open file button");
	buttonOpen->show(TRUE);
	buttonSelect=new Abutton("select", this, 15, 292, 36, 9, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_SELECT), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonSelect->setTooltips("select button");
	buttonSelect->show(TRUE);
	sprintf(str, "%s/ping-pong", name);
	buttonLoop=new ActrlButton(MKQIID(qiid,0xb041fff414a18c00), str, this, 31, 116, 4, 4, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_LOOP), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonLoop->setTooltips("ping pong button");
	buttonLoop->show(TRUE);
	sprintf(str, "%s/mute", name);
	buttonMute=new ActrlButton(MKQIID(qiid,0xcdad82e325bae7f0), str, this, 9, 98, 18, 9, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_MUTE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonMute->setTooltips("mute channel");
	buttonMute->show(TRUE);
	sprintf(str, "%s/solo", name);
	buttonSolo=new ActrlButton(MKQIID(qiid,0xcba3b470a8f175d0), str, this, 39, 98, 18, 9, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_SOLO), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonSolo->setTooltips("solo channel");
	buttonSolo->show(TRUE);
	sprintf(str, "%s/blend filter", name);
	filter=new Aselect(MKQIID(qiid, 0xf88f8f5765749180), str, this, 12, 145, 3, 2, &resource.get(MAKEINTRESOURCE(PNG_RNBOX_BLEND), "PNG"), 12, 12);
	filter->setTooltips("blend filter");
	filter->set(0);
	filter->show(TRUE);
	sprintf(str, "%s/send A", name);
	senda=new Apaddle(MKQIID(qiid,0xf718f360dd97561a), str, this, 10, 20, paddleYOLI16);
	senda->set(0.f);
	senda->setTooltips("send A");
	senda->show(true);
	sprintf(str, "%s/send B", name);
	sendb=new Apaddle(MKQIID(qiid,0x48923ff80ff3c380), str, this, 26, 34, paddleYOLI16);
	sendb->set(0.f);
	sendb->setTooltips("send B");
	sendb->show(true);
	snap=new Abitmap(57, 42, 32);
	memset(filename, 0, sizeof(filename));
	memset(seq, 0, sizeof(seq));
	bSnapChange=false;
	
	new Aitem("clear movie", "clear movie track", context, contextCLEAR, this);
	
	timer(20);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArhytmB::~ArhytmB()
{
	timer();
	delete(s1);
	delete(s2);
	delete(levelv);
	delete(levelvel);
	delete(lenght);
	delete(pitch);
	delete(start);
	delete(startvel);
	delete(buttonOpen);
	delete(buttonMute);
	delete(buttonSolo);
	delete(buttonSelect);
	delete(buttonLoop);
	delete(filter);
	delete(senda);
	delete(sendb);
	delete(snap);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArhytmB::paint(Abitmap *b)
{
	section.enter(__FILE__,__LINE__);
	b->set(4, 51, snap, bitmapNORMAL, bitmapNORMAL);
	section.leave();
	{
		Afont	*f=alib.getFont(fontARMY10);
		char	str[128];
		sprintf(str, "%d", nbox+1);
		f->set(b, 48, 38, str, seqColors[played]);	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArhytmB::pulse()
{
	if(played!=mplayed)
	{
		Arectangle r;
		r.x=48;
		r.y=38;
		r.w=10;
		r.h=10;
		mplayed=played;
		repaint(r);
	}
	if(bSnapChange)
	{
		Arectangle r;
		r.x=4;
		r.y=51;
		r.w=snap->w;
		r.h=snap->h;
		bSnapChange=false;
		repaint(r);
	}
}

//////////////////////////q///////////////////////////////////////////////////////////////////////////////////////////////

bool ArhytmB::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			Arectangle r;
			r.x=4;
			r.y=51;
			r.w=snap->w;
			r.h=snap->h;
			if(r.contains(x, y))
			{
				char	str[1024];
				sprintf(str, "%s %I64x", name, ((Arhytmbox *)((ArhytmboxFront *)father)->effect)->getQIID());
				dropText(str);
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ArhytmB::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCONTEXT:
		{
			Aitem	*i=(Aitem *)p;
			switch(i->data)
			{
				case contextCLEAR:
				((Arhytmbox *)((ArhytmboxFront *)father)->effect)->loadFile(nbox, "-");
				break;
			}
		}
		break;
		
		case nyCHANGE:
		if(o==buttonSelect)
		{
			if(p)
				father->notify(this, nySELECT, nbox);
			else
				buttonSelect->setChecked(true);
		}
		else if(o==levelv)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->level=levelv->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==levelvel)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->levelvel=levelvel->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==lenght)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->lenght=lenght->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==pitch)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->pitch=pitch->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==start)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->start=start->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==startvel)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->startvel=startvel->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==s1)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->s1=s1->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==s2)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->s2=s2->get();;
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==pan)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->pan=pan->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==filter)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->filter=filter->get();
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==buttonLoop)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->pingpong=p?true:false;
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==buttonMute)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->mute=p?true:false;
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		else if(o==buttonSolo)
		{
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->video[nbox]->solo=p?true:false;
			((Arhytmbox *)((ArhytmboxFront *)father)->effect)->sectionVideo.leave();
		}
		break;

		case nyCLICK:
		if(o==buttonOpen)
		{
			AfileDlg	fdlg=AfileDlg("loadDlg", this, "all movies/pictures files\0*.png;*.jpg;*.jpeg;*.avi;*.mpg;*.mpeg;*.fli;*.flc\0picture file PNG\0*.png\0picture file JPEG\0*.jpg;*.jpeg\0movie file AVI\0*.avi\0movie file MPG\0*.mpg;*.mpeg\0movie file FLI/FLC\0*.fli;*.flc\0", filename, AfileDlg::LOAD);
			if(fdlg.getFile(filename))
				((Arhytmbox *)((ArhytmboxFront *)father)->effect)->loadFile(nbox, filename);
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ArhytmB::dragdrop(int x, int y, int state, int event, void *data)
{
	switch(event)
	{
		case dragdropENTER:
		case dragdropOVER:
		if(state&(dragdropFILE|dragdropTEXT))	
			return true;
		break;

		case dragdropDROP:
		if(state&dragdropFILE)
		{
			int		nb=7;
			int		n=nbox;
			char	*s=(char *)data;
			while(*s&&(nb--))
			{
				((Arhytmbox *)((ArhytmboxFront *)father)->effect)->loadFile(n, s);
				s+=strlen(s)+1;
				n=(n+1)%7;
			}
			return true;
		}
		else if(state&dragdropTEXT)
		{
			father->notify(this, nyCOMMAND, (dword)data);
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Arhytmbox::Arhytmbox(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	{
		int	i;
		for(i=0; i<7; i++)
			order[i]=i;
	}
	current=-1;
	percent=0;
	nbank=0;
	memset(memoname, 0, sizeof(memoname));
	memset(filename, 0, sizeof(filename));
	memset(loadedname, 0, sizeof(loadedname));
	front=new ArhytmboxFront(qiid, "rythmbox front", this, 400);
	front->setTooltips("rythmbox module");
	back=new ArhytmboxBack(qiid, "rythmbox back", this, 400);
	back->setTooltips("rythmbox module");
	image=new Abitmap(getVideoWidth(), getVideoHeight());
	{
		int	i;
		for(i=0; i<7; i++)
			video[i]=new RBvideo();
	}
	patternActive=false;
	mPatternActive=false;
	bCycle=false;
	bCut=false;
	patternStep=16;
	currentStep=-1;
	bRun=true;
	bStop=false;
	start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Arhytmbox::~Arhytmbox()
{
	int	i;
	bRun=false;
	while(!bStop)
		sleep(10);
	sectionVideo.enter(__FILE__,__LINE__);
	for(i=0; i<7; i++)
	{
		delete(video[i]);
		video[i]=NULL;
	}
	sectionVideo.leave();
	delete(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arhytmbox::load(class Afile *f)
{
	int	i,j;
	sectionVideo.enter(__FILE__,__LINE__);
	f->read(&order, sizeof(order));
	for(j=0; j<NBBANKS; j++)
	{
		for(i=0; i<7; i++)
		{
			bool	b;
			f->read(&b, sizeof(b));
			if(b)
			{
				char	path[ALIB_PATH];
				char	abspath[ALIB_PATH];
				f->readString(path);
				if(f->absoluPath(abspath, path))
					strcpy(memoname[j][i], abspath);
				else
					strcpy(memoname[j][i], path);
			}
		}
	}
	memset(filename, 0, sizeof(filename));
	sectionVideo.leave();
	{
		ArhytmboxFront	*front=(ArhytmboxFront *)this->front;
		for(i=0; i<7; i++)
			f->read(front->box[i]->seq, sizeof(front->box[i]->seq));
	}
	((ArhytmboxFront *)front)->moveBox(0, 0);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arhytmbox::save(class Afile *f)
{
	int	i,j;
	sectionVideo.enter(__FILE__,__LINE__);
	f->write(&order, sizeof(order));
	for(j=0; j<NBBANKS; j++)
	{
		for(i=0; i<7; i++)
		{
			if(memoname[j][i][0])
			{
				char	relpath[ALIB_PATH];
				bool	b=true;
				f->write(&b, sizeof(b));
				if(f->relativePath(relpath, memoname[j][i]))
					f->writeString(relpath);
				else
					f->writeString(memoname[j][i]);
			}
			else
			{
				bool	b=false;
				f->write(&b, sizeof(b));
			}
		}
	}
	sectionVideo.leave();
	{
		ArhytmboxFront	*front=(ArhytmboxFront *)this->front;
		for(i=0; i<7; i++)
			f->write(front->box[i]->seq, sizeof(front->box[i]->seq));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arhytmbox::loadPreset(class Afile *f)
{
	int	i,j;
	sectionVideo.enter(__FILE__,__LINE__);
	f->read(&order, sizeof(order));
	for(j=0; j<NBBANKS; j++)
	{
		for(i=0; i<7; i++)
		{
			int	n=-1;
			f->read(&n, sizeof(n));
			if(n!=-1)
			{
				char	fn[ALIB_PATH];
				if(getPresetFile(n, fn))
					strcpy(memoname[j][i], fn);
			}
			else
				memoname[j][i][0]=0;
		}
	}
	memset(filename, 0, sizeof(filename));
	sectionVideo.leave();
	{
		ArhytmboxFront	*front=(ArhytmboxFront *)this->front;
		for(i=0; i<7; i++)
			f->read(front->box[i]->seq, sizeof(front->box[i]->seq));
	}
	((ArhytmboxFront *)front)->moveBox(0, 0);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arhytmbox::savePreset(class Afile *f)
{
	int	i,j;
	sectionVideo.enter(__FILE__,__LINE__);
	f->write(&order, sizeof(order));
	for(j=0; j<NBBANKS; j++)
	{
		for(i=0; i<7; i++)
		{
			int	n=-1;
			if(memoname[j][i][0])
				n=addPresetFile(memoname[j][i]);
			f->write(&n, sizeof(n));
		}
	}
	sectionVideo.leave();
	{
		ArhytmboxFront	*front=(ArhytmboxFront *)this->front;
		for(i=0; i<7; i++)
			f->write(front->box[i]->seq, sizeof(front->box[i]->seq));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arhytmbox::settings(bool emergency)
{
	image->size(getVideoWidth(), getVideoHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arhytmbox::actionStop()
{
	ArhytmboxFront	*front=(ArhytmboxFront *)this->front;
	int				i;
	for(i=0; i<7; i++)
		front->box[i]->played=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arhytmbox::action(double time, double dtime, double beat, double dbeat)
{
	static const float	breso[]={1.f/8.f, 1.f/4.f, 1.f/2.f, 1.f-1.f/3.f, 1.f, 1.f+1.f/3.f, 2.f, 4.f, 8.f};
	ArhytmboxFront		*front=(ArhytmboxFront *)this->front;
	ArhytmboxBack		*back=(ArhytmboxBack *)this->back;
	int					w=getVideoWidth();
	int					h=getVideoHeight();
	Avideo				*out=back->out;
	float				master=front->master->get();
	float				reso=breso[front->reso->get()];
	float				gpitch=(float)(front->gpitch->get()-0.5f)*10.f;
	bool				sature;
	Abitmap				*bouta=back->outa->getBitmap();
	Abitmap				*boutb=back->outb->getBitmap();
	float				senda[7];
	float				sendb[7];
	int					played[7];
	float				tsenda=0.f;
	float				tsendb=0.f;
	bool				bsolo=false;
	float				fadetime=front->fadetime->get();
	

	sectionVideo.enter(__FILE__,__LINE__);
	memset(played, 0, sizeof(played));

	if(front->buttonSend->get())
	{	
		bouta->boxf(0, 0, bouta->w, bouta->h, 0xff000000);
		boutb->boxf(0, 0, boutb->w, boutb->h, 0xff000000);
	}
	
	{
		int	i;
		for(i=0; i<7; i++)
		{
			RBvideo	*v=video[i];
			if(v)
				bsolo|=v->solo;
		}
	}

	beat*=reso;
	dbeat*=reso;

	out->enter(__FILE__,__LINE__);
	{
		int		iorder[7];
		int		i,jj;
		for(i=0; i<7; i++)
			iorder[order[i]]=i;
		Abitmap	*bout=out->getBitmap();
		{	
			float	level=front->zoomLevel->get();
			float	size=front->zoomSize->get();
			float	lvlblur=front->blurLevel->get();
			float	szblur=front->blurSize->get();
			
			if(level>0.f)	// zoom
			{
				int		zx=(size>=0.5)?((int)(w*(size-0.5))):((int)((size-0.5)*w));
				int		zy=(size>=0.5)?((int)(h*(size-0.5))):((int)((size-0.5)*h));
				image->set(0, 0, bout, bitmapNORMAL, bitmapNORMAL);
				image->flags=bitmapGLOBALALPHA|bitmapSATURATION;
				image->alpha=level;
				image->state=bitmapNORMAL;
				bout->set(-zx, -zy, w+zx*2, h+zy*2, 0, 0, w, h, image, bitmapDEFAULT, bitmapDEFAULT);
			}
			if(lvlblur>0.f)	// blur
			{
				image->blur(bout, w*szblur*0.2f, h*szblur*0.2f);
				image->state=bitmapNORMAL;
				image->flags=bitmapGLOBALALPHA;
				image->alpha=lvlblur;
				bout->set(0, 0, image, bitmapDEFAULT, bitmapDEFAULT);
			}
		}
		{	// fade
			float	level=front->fadeLevel->get();
			if(level>0.f)
			{
				dword	color=front->fadeColor->get();
				bout->boxfa(0, 0, bout->w-1, bout->h-1, color, level);
			}
		}
		((ArhytmboxBack *)back)->in->enter(__FILE__,__LINE__);
		{	// fade video in
			float	level=front->inLevel->get();
			Abitmap	*bin=back->in->getBitmap();
			if(bin&&(level>0.f))
			{
				bin->state=bitmapNORMAL;
				bin->flags=bitmapGLOBALALPHA;
				bin->alpha=level;
				bout->set(0, 0, bin, bitmapDEFAULT, bitmapDEFAULT);
				bin->flags=0;
			}
		}
		((ArhytmboxBack *)back)->in->leave();

		int prem=0;
		if(patternActive)
		{
			double	cs=beat*4.f;
			currentStep=(double)(cs-(int)(cs/patternStep)*patternStep);
			{
				int	c=(int)currentStep;
				for(jj=0; jj<7; jj++)
				{
					int		i=iorder[jj];
					RBvideo	*v=video[i];
					if(v)
					{
						played[i]=v->seq[c];
						if(bCut&&v->movie&&v->seq[c]&&(v->level>0.f))
							if((bsolo&&v->solo)||((!v->mute)&&(!bsolo)))
								prem=jj;
					}
				}
			}
		}
		
		{
			int	i;
			for(i=0; i<7; i++)
			{
				RBvideo	*v=video[i];
				senda[i]=sendb[i]=0.f;
				if(v)
				{
					if((bsolo&&v->solo)||((!v->mute)&&(!bsolo)))
					{
						if((!patternActive)||played[i])
						{
							tsenda+=(senda[i]=front->box[i]->senda->get());
							tsendb+=(sendb[i]=front->box[i]->sendb->get());
							if(bCut&&patternActive)
							{
								if(senda[i]>0.f)
								{
									int	k;
									tsenda=senda[i];
									for(k=0; k<i; k++)
										senda[k]=0.f;
								}
								if(sendb[i]>0.f)
								{
									int	k;
									tsendb=sendb[i];
									for(k=0; k<i; k++)
										sendb[k]=0.f;
								}
							}
						}
					}
				}
			}
			tsenda=maxi(tsenda, 1.f);
			tsendb=maxi(tsendb, 1.f);
		}

		mPatternActive=patternActive;

		//bout->boxf(0, 0, bout->w-1, bout->h-1, 0xff000000);	// 4debug
		
		for(jj=0; jj<7; jj++)
		{
			int		i=iorder[jj];
			RBvideo	*v=video[i];
			ArhytmB	*box=front->box[i];

			if(v&&v->movie)
			{
				Amovie	*m=v->movie;
				double	zz=pow((v->levelvel-0.5)*3, 3);
				int		zx=(int)((zz<0)?(zz*w*0.2963):(zz*w));
				int		zy=(int)((zz<0)?(zz*h*0.2963):(zz*h));
				int		sf=(int)(v->start*0.9999*m->nbFrames);
				int		lf=(int)(v->lenght*0.9999*m->nbFrames);
				int		f;
				float	cpitch=v->pitch;
				float	level=v->level;
				
				cpitch=(float)((2.0*cpitch-1.0)*5.0)+gpitch;

				if((sf+lf)>=m->nbFrames)
					lf=(m->nbFrames-1-sf);
					
				{	// pitch depend on the movie size
					//cpitch*=0.5f;	
					if(m->nbFrames>25)
						cpitch*=(float)(25.f/(float)m->nbFrames);	
					/*
					else
					{
						int	nf=25;
						while(nf>m->nbFrames)
						{
							cpitch*=2;
							nf/=2;
						}
					}
					*/
				}

				cpitch+=v->startvel*10.f*((float)sin((float)beat+(float)i)*0.5f)*(float)sin(beat*((float)i+100.f)*0.002f);

				if(patternActive)
				{
					int	c=(int)currentStep;
					{
						int	k;
						int	lo=c;
						int	sq=0;
						
						for(k=0; k<patternStep; k++)
						{
							if(v->seq[lo])
							{
								sq=v->seq[lo];
								break;
							}
							lo--;
							if(lo<0)
								lo=patternStep-1;
						}
							
						if(k!=patternStep)
						{
							double	stepCount=(currentStep<lo)?(currentStep+(patternStep-lo)):(currentStep-lo);
							double	level_stepCount=stepCount;

							{
								int	i;
								for(i=c-1; i>=0; i--)
								{
									if(v->seq[i]!=sq)
										break;
									stepCount+=1.0;
								}
							}
				
							if(bCycle)
								v->cframe+=dbeat*cpitch;
							else
								v->cframe=(stepCount/4.f)*cpitch;

							v->cframe=v->cframe-(int)v->cframe;
							if(v->cframe<0.f)
							{
								v->cframe+=1.f;
								if(v->cframe>=1.f)
									v->cframe+=-1.f;
							}

							double	mod_cframe=0.f;
							switch(v->seq[lo])
							{
								case 1:
								mod_cframe=v->cframe;
								break;

								case 2:
								mod_cframe=v->cframe+0.25;
								break;

								case 3:
								mod_cframe=v->cframe+0.50;
								break;
								
								case 4:
								mod_cframe=v->cframe+0.75;
								break;
							}
							
							while(mod_cframe<0.0)
								mod_cframe+=1.0;
							while(mod_cframe>=1.0)
								mod_cframe+=-1.0;

							if(!v->pingpong)
							{
								f=(int)(mod_cframe*lf)+sf;
							}
							else
							{
								if(mod_cframe<0.5f)
									f=(int)(mod_cframe*2*lf)+sf;
								else
									f=sf+lf-(int)((mod_cframe-0.5)*2*lf);
							}
							
							if(fadetime<=0.5f)
								level=(float)(v->level-(level_stepCount-1.f)*((fadetime+0.1f)*1.6667f));
							else
							{
								float	lvl=(level_stepCount<1.f)?v->level:0.f;
								float	a=(fadetime-0.5f)*2.f;
								float	ia=1.f-a;
								level=(float)(((v->level-(level_stepCount-1.f))*ia)+(lvl*a));
							}
								
							level=maxi(mini(level, v->level), 0.f);
						}
						else
							continue;
					}
				}
				else
				{
					v->cframe+=dbeat*cpitch;
					v->cframe=v->cframe-(int)v->cframe;
					if(v->cframe<0.f)
					{
						v->cframe+=1.f;
						if(v->cframe>=1.f)
							v->cframe=-1.f;
					}
					if(!v->pingpong)
					{
						f=(int)(v->cframe*lf)+sf;
					}
					else
					{
						if(v->cframe<0.5f)
							f=(int)(v->cframe*2*lf)+sf;
						else
							f=sf+lf-(int)((v->cframe-0.5)*2*lf);
					}

					level=v->level;
				}

				level*=master;
				
				bool	sendHLS=false;
				bool	forceHLS=false;
				
				if((!patternActive)||played[i])
				{
					if(senda[i]>0.f)
					{
						float	p=senda[i]/tsenda;
						if((v->s1<0.48f)||(v->s1>0.52f)||(v->s2<0.48f)||(v->s2>0.52f)||(v->pan<0.48f)||(v->pan>0.52f))
							sendHLS=true;
						else
							m->get(bouta, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapNORMAL, bitmapGLOBALALPHA, p, 0, f);
					}

					if(sendb[i]>0.f)
					{
						float	p=sendb[i]/tsendb;
						if((v->s1<0.48f)||(v->s1>0.52f)||(v->s2<0.48f)||(v->s2>0.52f)||(v->pan<0.48f)||(v->pan>0.52f))
							sendHLS=true;
						else
							m->get(boutb, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapNORMAL, bitmapGLOBALALPHA, p, 0, f);
					}
				}

				if(sendHLS&&(!((jj>=prem)&&(level>0.f)&&((bsolo&&v->solo)||(!bsolo&&!v->mute)))))
					forceHLS=true;

				if(forceHLS||((jj>=prem)&&(level>0.f)&&((bsolo&&v->solo)||(!bsolo&&!v->mute))))
				{
					if((v->s1<0.48f)||(v->s1>0.52f)||(v->s2<0.48f)||(v->s2>0.52f)||(v->pan<0.48f)||(v->pan>0.52f))
					{
						int		tt[3][3][256];
						double	cc[256];
						Abitmap	*bs=m->get(f);
						image->set(-zx, -zy, w+zx*2, h+zy*2, 0, 0, bs->w, bs->h, bs, bitmapNORMAL, bitmapNORMAL);
						image->setState(bs);
						{
							int	i;
							for(i=128; i<256; i++)
								cc[i]=0;
							for(i=0; i<170; i++)
								cc[(i-21)&255]=sin(i*PI/170);
							//for(i=0; i<128; i++)
							//	cc[i]=sin(i*PI/128);
						}
						{
							int		i,j,n;
							for(i=0; i<3; i++)
							{
								for(j=0; j<3; j++)
								{
									int		d=(i-j);
									if(d==2)
										d=-1;
									else if(d==-2)
										d=1;
									double	s=cc[(int)(64+d*85+(v->s2-0.5)*256)&255];
									if(v->pan>=0.5f)
									{
										float	a0=(v->pan-0.5f)*2.f;
										float	a1=1.f-a0;
										s=s*a1+sqrt(s)*a0*2.f;
									}
									else
									{
										float	a0=(0.5f-v->pan)*2.f;
										float	a1=1.f-a0;
										s=0.33333f*a0+s*a1;
									}
									int	lum=(int)(v->s1*511-256);
									for(n=0; n<256; n++)
									{
										tt[i][j][n]=(int)(n*s)+lum;
									}
								}
							}
						}
						{
							int		i;
							int		m=w*h;
							dword	*p=image->body32;
							for(i=0; i<m; i++)
							{
								byte	r,g,b,a;
								byte	r2,g2,b2;
								colorRGBA(&r, &g, &b, &a, *p);
								r2=mini(maxi(tt[0][0][r]+tt[0][1][g]+tt[0][2][b], 0), 255);
								g2=mini(maxi(tt[1][0][r]+tt[1][1][g]+tt[1][2][b], 0), 255);
								b2=mini(maxi(tt[2][0][r]+tt[2][1][g]+tt[2][2][b], 0), 255);
								*(p++)=color32(r2, g2, b2, a);
							}
						}
						if(!forceHLS)
						{
							switch(v->filter)
							{
								case 0:
								image->state=bitmapNORMAL;
								sature=false;
								break;

								case 1:
								image->state=bitmapADD;
								sature=true;
								break;

								case 2:
								image->state=bitmapSUB;
								sature=true;
								break;

								case 3:
								image->state=bitmapMUL;
								sature=false;
								break;
								
								case 4:
								image->state=bitmapADD;
								sature=false;
								break;

								case 5:
								image->state=bitmapSUB;
								sature=false;
								break;
							}
							{
								int	flags=bitmapGLOBALALPHA|(sature?bitmapSATURATION:0);
								image->flags=(image->flags&bitmapALPHA)?(flags|bitmapALPHA):flags;
								image->alpha=level;
								if(zx<0)
									bout->set(-zx, -zy, w+zx*2, h+zy*2, -zx, -zy, w+zx*2, h+zy*2, image, bitmapDEFAULT, bitmapDEFAULT);
								else
									bout->set(0, 0, w, h, 0, 0, w, h, image, bitmapDEFAULT, bitmapDEFAULT);
								image->state=bitmapNORMAL;
								image->flags=0;
								image->alpha=0.f;
							}
						}
						if(sendHLS)
						{
							if(senda[i]>0.f)
							{
								image->alpha=senda[i]/tsenda;
								if(zx<0)
									bouta->set(-zx, -zy, w+zx*2, h+zy*2, -zx, -zy, w+zx*2, h+zy*2, image, bitmapNORMAL, bitmapGLOBALALPHA);
								else
									bouta->set(0, 0, w, h, 0, 0, w, h, image, bitmapNORMAL, bitmapGLOBALALPHA);
							}
							if(sendb[i]>0.f)
							{
								image->alpha=sendb[i]/tsendb;
								if(zx<0)
									boutb->set(-zx, -zy, w+zx*2, h+zy*2, -zx, -zy, w+zx*2, h+zy*2, image, bitmapNORMAL, bitmapGLOBALALPHA);
								else
									boutb->set(0, 0, w, h, 0, 0, w, h, image, bitmapNORMAL, bitmapGLOBALALPHA);
							}
						}
					}
					else
					{
						switch(v->filter)
						{
							case 0:
							m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapNORMAL, bitmapGLOBALALPHA, level, 0, f);
							break;

							case 1:
							m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapADD, bitmapGLOBALALPHA|bitmapSATURATION, level, 0, f);
							break;

							case 2:
							m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapSUB, bitmapGLOBALALPHA|bitmapSATURATION, level, 0, f);
							break;

							case 3:
							m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapMUL, bitmapGLOBALALPHA, level, 0, f);
							break;

							case 4:
							m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapADD, bitmapGLOBALALPHA, level, 0, f);
							break;

							case 5:
							m->get(bout, -zx, -zy, w+zx*2, h+zy*2, 0, 0, m->w, m->h, bitmapSUB, bitmapGLOBALALPHA, level, 0, f);
							break;

						}
					}
				}
			}
		}
	}
	out->leave();
	{
		int	i;
		for(i=0; i<7; i++)
			front->box[i]->played=played[i];
	}
	sectionVideo.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Arhytmbox::run()
{
	Atable		*table=getTable();
	Amovie		*movieStream=NULL;
	Amovie		*movie=NULL;
	Abitmap		*trans=NULL;
	int			nnbox=6;
	int			nbox=6;
	char		fname[ALIB_PATH];

	memset(fname, 0, sizeof(fname));

	this->setPriority(priorityNORMAL);
	
	while(bRun)
	{
		sleep(150);

		fname[0]=0;

		sectionVideo.enter(__FILE__,__LINE__);
		if(nbox==nnbox)
			nnbox=(nnbox+1)%7;
		nbox=nnbox;
		if(filename[nbox][0])
		{
			strcpy(fname, filename[nbox]);
			if(!strcmp(loadedname[nbox], fname))
				fname[0]=0;
			filename[nbox][0]=0;
		}
		sectionVideo.leave();

		if(fname[0]&&bRun)
		{
			current=nbox;
			percent=0;
			if(strstr(fname, ".png")||strstr(fname, ".jpg")||strstr(fname, ".jpeg"))
			{
				Abitmap	*bmp=new Abitmap(fname);
				if(bmp)
				{
					ArhytmboxFront	*fr=(ArhytmboxFront *)front;
					strcpy(loadedname[nbox], fname);
					movie=new Amovie("movie", bmp->w, bmp->h, bmp->nbBits);
					movie->append(bmp);
					sectionVideo.enter(__FILE__,__LINE__);
					if(video[nbox]->movie)
						delete(video[nbox]->movie);
					video[nbox]->movie=movie;
					fr->box[nbox]->section.enter(__FILE__,__LINE__);
					{
						Abitmap	*b=fr->box[nbox]->snap;
						b->boxf(0, 0, b->w, b->h, 0xff808080);
						b->set(0, 0, b->w, b->h, 0, 0, bmp->w, bmp->h, bmp, bitmapNORMAL, bitmapNORMAL);
						fr->box[nbox]->bSnapChange=true;
					}
					fr->box[nbox]->section.leave();
					sectionVideo.leave();
					movie=NULL;
					delete(bmp);
				}
			}
			else if(fname[0]=='-')	// clear movie track
			{
				ArhytmboxFront	*fr=(ArhytmboxFront *)front;
				sectionVideo.enter(__FILE__,__LINE__);
				if(video[nbox]->movie)
					delete(video[nbox]->movie);
				video[nbox]->movie=null;
				filename[nbox][0]=0;
				loadedname[nbox][0]=0;
				sectionVideo.leave();
				
				fr->box[nbox]->section.enter(__FILE__,__LINE__);
				{
					Abitmap	*b=fr->box[nbox]->snap;
					b->boxf(0, 0, b->w, b->h, 0xff000000);
					fr->box[nbox]->bSnapChange=true;
				}
				fr->box[nbox]->section.leave();
			}
			else	// load movie
			{
				bool	bsect=false;
				char	oldname[1024];
				bool	bok=false;
				strcpy(oldname, loadedname[nbox]);
				try
				{
					movieStream=new Amovie("stream", fname);
					if(movieStream->isOK)
					{
						if((movieStream->w<=MAXMOVIEWIDTH)&&(movieStream->h<=MAXMOVIEHEIGHT))
						{
							int	maxframes=(int)(getLoopTime()*movieStream->getFPS());
							trans=new Abitmap(movieStream->w, movieStream->h, movieStream->nbBits); 
							movie=null;
							strcpy(loadedname[nbox], fname);
							while(bRun)
							{
								if(!movieStream->get(trans))
									break;
								if(!movie)
								{
									if(table->compressRamVidz&&(trans->flags==bitmapNORMAL))
										movie=new Amovie("movie", movieStream->w, movieStream->h, table->compressJpegQuality);		// jpeg lossles
									else	
										movie=new Amovie("movie", movieStream->w, movieStream->h, movieStream->nbBits, true);				// normal, can use alpha
								}								
								if(!movie->append(trans))
									break;
								if(!movieStream->isLast)
									if(!movieStream->select(Amovie::NEXT))
										break;
								if((movieStream->isLast)||(movieStream->currentI>maxframes))
								{
									sectionVideo.enter(__FILE__,__LINE__);
									bsect=true;
									if(video[nbox]->movie)
										delete(video[nbox]->movie);
									video[nbox]->movie=movie;
									bok=true;
									if(movie->nbFrames>0)
									{
										ArhytmboxFront	*fr=(ArhytmboxFront *)front;
										movie->select(Amovie::FIRST);
										fr->box[nbox]->section.enter(__FILE__,__LINE__);
										{
											Abitmap	*b=fr->box[nbox]->snap;
											b->boxf(0, 0, b->w, b->h, 0xff000000);
											movie->get(b, 0, 0, b->w, b->h, 0, 0, movie->w, movie->h, 0, 0, 1.f, 0, Amovie::FIRST);
											fr->box[nbox]->bSnapChange=true;
										}
										fr->box[nbox]->section.leave();
									}
									sectionVideo.leave();
									bsect=false;
									movie=NULL;
									break;
								}
								percent=100*movieStream->currentI/((movieStream->nbFrames>maxframes)?maxframes:movieStream->nbFrames);
								if(!getTable()->render)
									sleep(1);
							}
							if(movie)
								delete(movie);
							movie=NULL;
							delete(trans);
							trans=NULL;
						}
					}
				}
				catch(...)
				{
					if(bsect)
					{
						sectionVideo.leave();
						bsect=false;
					}
					if(movie)
						delete(movie);
					movie=NULL;
					if(trans)
						delete(trans);
					trans=NULL;
				}
				if(!bok)
					strcpy(loadedname[nbox], oldname);
				if(movieStream)
					delete(movieStream);
				movieStream=NULL;
			}
			current=-1;
		}
	}
	bStop=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Arhytmbox::loadFile(int nbox, char *filename)
{
	sectionVideo.enter(__FILE__,__LINE__);
	strlwr(filename);
	strcpy(this->filename[nbox], filename);
	strcpy(this->memoname[nbank][nbox], filename);
	sectionVideo.leave();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArhytmboxFront::ArhytmboxFront(QIID qiid, char *name, Arhytmbox *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX), "PNG");
	back=new Abitmap(&o);
	
	memset(seqcopy, 0, sizeof(seqcopy));
	
	{
		static QIID qiidbx[]={	0xd3643972ba1abc82, 0x8239e995dde013e0, 0xf0b4306e8a062180, 0xc74e58eb293ec120,
								0x717decc87d8e0f88, 0x8c43407d0772aa90, 0xb141f08ad2758b84, 0x028ebaa9c6c85370};
		int	i;
		for(i=0; i<7; i++)
		{
			char	str[128];
			sprintf(str, "track %d", i+1);
			box[i]=new ArhytmB(MKQIID(qiid, qiidbx[i]), str, this, 56+i*65, 0, i);
			box[i]->show(true);
		}
		selected=0;
		box[selected]->buttonSelect->setChecked(true);
	}
	sequence=new Asequence("sequence", this, 87, 362);
	sequence->setTooltips("sequence");
	sequence->show(true);
	eventType=new AselButton("eventType", this, 370, 320, 2, 2, &resource.get(MAKEINTRESOURCE(PNG_RNBOX_SEQ_MINI), "PNG"), 14, 15);
	eventType->setTooltips("video phase");
	eventType->show(true);
	
	steps=new Atrigger("trigger", this, 303, 320, 9, 34, 4);
	steps->setTooltips("edit steps");
	steps->ipos=3;
	steps->show(true);
	buttonPattern=new ActrlButton(MKQIID(qiid, 0x017b088c21899ad0), "pattern", this, 8, 300, 8, 8, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN_SWITCH), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonPattern->setTooltips("pattern on/off switch");
	buttonPattern->show(TRUE);
	nsteps=new Aseg7("nsteps", this, 113, 330, 33, 17);
	nsteps->setTooltips("loop lenght");
	nsteps->show(true);
	pattern=new Aselect(MKQIID(qiid, 0xc52d7561bbc369e0), "pattern", this, 7, 326, 4, 2, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN), "PNG"), 14, 15);
	pattern->setTooltips("pattern selector");
	pattern->set(0);
	pattern->show(TRUE);
	patternABCD=new Aselect(MKQIID(qiid, 0xcbb1bb7ed92df440), "patternABCD", this, 7, 371, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN), "PNG"), 14, 15);
	patternABCD->setTooltips("pattern bank selector");
	patternABCD->set(0);
	patternABCD->show(TRUE);
	
	buttonCycle=new ActrlButton(MKQIID(qiid, 0x5537e6d2d2b16dfc), "cycle", this, 419, 332, 8, 8, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN_SWITCH), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonCycle->setTooltips("cycle switch");
	buttonCycle->show(TRUE);

	buttonCut=new ActrlButton(MKQIID(qiid, 0x747699050cfa1300), "cut", this, 450, 332, 8, 8, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN_SWITCH), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonCut->setTooltips("break/cut mode switch");
	buttonCut->show(TRUE);

	fadetime=new Apaddle(MKQIID(qiid, 0x235515ab48660b40), "fadetime", this, 470, 326, paddleYOLI24);
	fadetime->setTooltips("fade time");
	fadetime->set(0.5f);
	fadetime->show(TRUE);

	inLevel=new Apaddle(MKQIID(qiid, 0xefbebf8df709ff90), "video input level", this, 16, 90, paddleYOLI24);
	inLevel->setTooltips("video input level");
	inLevel->set(0.0f);
	inLevel->show(TRUE);

	fadeLevel=new Apaddle(MKQIID(qiid, 0x0f49d894d59c6b20), "fade level", this, 8, 130, paddleYOLI24);
	fadeLevel->setTooltips("fade level");
	fadeLevel->set(0.0f);
	fadeLevel->show(TRUE);

	fadeColor=new Aselcolor(MKQIID(qiid, 0x2529c9a5efef4940), "fade color", this, 34, 144, 12, 12);
	fadeColor->setTooltips("fade color");
	fadeColor->set(0xff000000);
	fadeColor->show(true);

	zoomLevel=new Apaddle(MKQIID(qiid, 0xaced42ea9633e000), "zoom level", this, 8, 170, paddleYOLI24);
	zoomLevel->setTooltips("zoom level");
	zoomLevel->set(0.0f);
	zoomLevel->show(TRUE);

	zoomSize=new Apaddle(MKQIID(qiid, 0x1b18b6187c245ed8), "zoom size", this, 32, 184, paddleYOLI16);
	zoomSize->setTooltips("zoom size");
	zoomSize->set(0.5f);
	zoomSize->show(TRUE);

	blurLevel=new Apaddle(MKQIID(qiid, 0x44871d73995bbbf0), "blur level", this, 8, 210, paddleYOLI24);
	blurLevel->setTooltips("blur level");
	blurLevel->set(0.0f);
	blurLevel->show(TRUE);

	blurSize=new Apaddle(MKQIID(qiid, 0xa15c140b1e5e83d8), "blur size", this, 32, 224, paddleYOLI16);
	blurSize->setTooltips("blur size");
	blurSize->set(0.0f);
	blurSize->show(TRUE);

	master=new Apaddle(MKQIID(qiid, 0x23cfb8e766a60b40), "master level", this, 11, 260, paddleYOLI32);
	master->setTooltips("master level");
	master->set(1.0f);
	master->show(TRUE);

	reso=new Arnbreso(MKQIID(qiid, 0x093e01ad84542704), "resolution", this, 200, 338, 7, 0xffff4000, 9);
	reso->setTooltips("resolution");
	reso->set(4);
	reso->show(TRUE);

	gpitch=new Apaddle(MKQIID(qiid, 0x0f8465de5c9c6b20), "global pitch", this, 242, 326, paddleYOLI24);
	gpitch->setTooltips("global pitch");
	gpitch->set(0.5f);
	gpitch->show(TRUE);

	bank=new Aselect(MKQIID(qiid, 0x18653eec1ef66d40), "bank", this, 8, 50, 3, 2, &resource.get(MAKEINTRESOURCE(PNG_MIMIX_SELECT), "PNG"), 12, 12);
	bank->setTooltips("bank selector");
	bank->set(0);
	bank->show(true);
	
	seqpulse=new AseqPulse("seqpulse", this, 88, 388, 412, 5);
	seqpulse->show(true);

	new Aitem("copy pattern", "copy pattern", context, contextCOPYPATTERN, this);
	new Aitem("paste pattern", "paste pattern", context, contextPASTEPATTERN, this);

	buttonSend=new ActrlButton(MKQIID(qiid, 0xac5e8d5c21899ad0), "send black", this, 42, 30, 8, 8, &resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX_PATTERN_SWITCH), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	buttonSend->setTooltips("send black");
	buttonSend->set(false);
	buttonSend->show(TRUE);

	timer(1000);
}

// 88, 388, 412, 5

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArhytmboxFront::~ArhytmboxFront()
{
	int	i;
	delete(bank);
	delete(back);
	delete(reso);
	delete(gpitch);
	delete(inLevel);
	for(i=0; i<7; i++)
		delete(box[i]);
	delete(sequence);
	delete(eventType);
	delete(steps);
	delete(buttonPattern);
	delete(nsteps);
	delete(pattern);
	delete(patternABCD);
	delete(buttonCycle);
	delete(buttonCut);
	delete(fadetime);
	delete(fadeLevel);
	delete(fadeColor);
	delete(zoomLevel);
	delete(zoomSize);
	delete(blurLevel);
	delete(blurSize);
	delete(master);
	delete(seqpulse);
	delete(buttonSend);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArhytmboxFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ArhytmboxFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCONTEXT:
		{
			Aitem	*i=(Aitem *)p;
			switch(i->data)
			{
				case contextCOPYPATTERN:
				{
					Arhytmbox	*rb=((Arhytmbox *)effect);
					int			np=pattern->get();
					int			npa=patternABCD->get();
					int			i;

					rb->sectionVideo.enter(__FILE__,__LINE__);
					for(i=0; i<7; i++)
						memcpy(&seqcopy[i][0], &box[i]->seq[npa][np][0], sizeof(sbyte)*64);
					rb->sectionVideo.leave();
				}
				break;
				
				case contextPASTEPATTERN:
				{
					Arhytmbox	*rb=((Arhytmbox *)effect);
					int			np=pattern->get();
					int			npa=patternABCD->get();
					int			i;

					rb->sectionVideo.enter(__FILE__,__LINE__);
					for(i=0; i<7; i++)
					{
						memcpy(&box[i]->seq[npa][np][0], &seqcopy[i][0], sizeof(rb->video[i]->seq));
						memcpy(rb->video[i]->seq, &seqcopy[i][0], sizeof(rb->video[i]->seq));
					}
					rb->sectionVideo.leave();
					memcpy(sequence->seq, &box[selected]->seq[npa][np][0], sizeof(sequence->seq));
					sequence->repaint();
				}
				break;
			}
		}
		break;
		
		case nyCHANGE:
		if(o==bank)
		{
			Arhytmbox	*rb=((Arhytmbox *)effect);
			rb->sectionVideo.enter(__FILE__,__LINE__);
			rb->nbank=bank->get();
			memcpy(rb->filename, rb->memoname[rb->nbank], sizeof(rb->filename));
			{
				int	i;
				for(i=0; i<7; i++)
					rb->filename[i][strlen(rb->filename[i])+1]=rb->nbank;
			}
			rb->sectionVideo.leave();
		}
		else if(o==eventType)
		{
			sequence->style=p+1;
		}
		else if(o==steps)
		{
			sequence->step=3-p;
			sequence->repaint();
		}
		else if(o==sequence)
		{
			Arhytmbox	*rb=((Arhytmbox *)effect);
			int			np=pattern->get();
			int			npa=patternABCD->get();
			rb->sectionVideo.enter(__FILE__,__LINE__);
			rb->video[selected]->seq[p]=sequence->etat;
			rb->sectionVideo.leave();
			box[selected]->seq[npa][np][p]=sequence->etat;
		}
		else if(o==buttonPattern)
		{
			((Arhytmbox *)effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)effect)->patternActive=p?true:false;
			((Arhytmbox *)effect)->sectionVideo.leave();
		}
		else if(o==nsteps)
		{
			((Arhytmbox *)effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)effect)->patternStep=p;
			((Arhytmbox *)effect)->sectionVideo.leave();
		}
		else if((o==patternABCD)||(o==pattern))
		{
			Arhytmbox *rb=((Arhytmbox *)effect);
			int			np=pattern->get();
			int			npa=patternABCD->get();
			int			i;

			rb->sectionVideo.enter(__FILE__,__LINE__);
			for(i=0; i<7; i++)
				memcpy(rb->video[i]->seq, &box[i]->seq[npa][np][0], sizeof(rb->video[i]->seq));
			rb->sectionVideo.leave();

			memcpy(sequence->seq, &box[selected]->seq[npa][np][0], sizeof(sequence->seq));
			sequence->repaint();
		}
		else if(o==buttonCycle)
		{
			((Arhytmbox *)effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)effect)->bCycle=p?true:false;
			((Arhytmbox *)effect)->sectionVideo.leave();
		}
		else if(o==buttonCut)
		{
			((Arhytmbox *)effect)->sectionVideo.enter(__FILE__,__LINE__);
			((Arhytmbox *)effect)->bCut=p?true:false;
			((Arhytmbox *)effect)->sectionVideo.leave();
		}
		break;

		case nySELECT:
		{
			int			np=pattern->get();
			int			npa=patternABCD->get();
			box[selected]->buttonSelect->setChecked(false);
			selected=p;
			memcpy(sequence->seq, box[selected]->seq[npa][np], sizeof(sequence->seq));
			sequence->repaint();
		}
		break;
		
		case nyCOMMAND:
		((Arhytmbox *)effect)->sectionVideo.enter(__FILE__,__LINE__);
		{
			int		i;
			char	*str=(char *)p;
			if(str)
			{
				for(i=0; i<7; i++)
				{
					if(box[i]==o)
					{
						if(!strncmp(str, "track", 5))
						{
							int	n=str[6]-'0'-1;
							if((n>=0)&&(n<7))
							{
								if(str[7]==' ')
								{
									qword q;
									sscanf(str+8, "%I64x", &q);
									if(q==((Arhytmbox *)effect)->getQIID())
										moveBox(n, i);
								}
							}
						}
						break;
					}
				}
			}
		}
		((Arhytmbox *)effect)->sectionVideo.leave();
		return true;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArhytmboxFront::pulse()
{
	Arhytmbox *e=(Arhytmbox *)effect;
	e->sectionVideo.enter(__FILE__,__LINE__);
	{
		int	i;
		for(i=0; i<7; i++)
		{
			if(i==e->current)
			{
				char	str[128];
				itoa(e->percent, str, 10);
				strcat(str, "%");
				box[i]->affiche->set(str, 0xff000000, alib.getFont(fontTERMINAL06), Astatic::CENTER);
				box[i]->affiche->setTooltips("loading");
				box[i]->setTooltips("loading");
			}
			else
			{
				if(e->filename[i][0])
				{
					box[i]->affiche->set("QUEUED", 0xffff0000, alib.getFont(fontTERMINAL06), Astatic::CENTER);
					box[i]->affiche->setTooltips(e->filename[i]);
					box[i]->setTooltips(e->filename[i]);
				}
				else if(e->loadedname[i][0])
				{
					box[i]->affiche->set(e->loadedname[i], 0xff000000, alib.getFont(fontTERMINAL06), Astatic::RIGHT);
					box[i]->affiche->setTooltips(e->loadedname[i]);
					box[i]->setTooltips(e->loadedname[i]);
				}
				else
				{
					box[i]->affiche->set("NOTHING", 0xffff0000, alib.getFont(fontTERMINAL06), Astatic::CENTER);
					box[i]->affiche->setTooltips("movie title");
					box[i]->setTooltips("movie");
				}
			}
		}
	}
	e->sectionVideo.leave();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArhytmboxFront::moveBox(int from, int to)
{
	Arhytmbox	*e=(Arhytmbox *)effect;
	e->sectionVideo.enter(__FILE__,__LINE__);
	if(from!=to)
	{
		int			n0=e->order[from];
		int			n1=e->order[to];
		if(abs(n1-n0)==1)
		{
			e->order[from]=n1;
			e->order[to]=n0;
		}
		else if(n1<n0)
		{
			int	i;
			for(i=0; i<7; i++)
			{
				if((e->order[i]>=n1)&&(e->order[i]<n0))
					e->order[i]++;
			}
			e->order[from]=n1;
		}
		else	// n1 > n0
		{
			int	i;
			for(i=0; i<7; i++)
			{
				if((e->order[i]>n0)&&(e->order[i]<=n1))
					e->order[i]--;
			}
			e->order[from]=n1;
		}
	}
	e->sectionVideo.leave();
	{
		int	i;
		for(i=0; i<7; i++)
			box[i]->pos.x=56+e->order[i]*65;
		repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArhytmboxBack::ArhytmboxBack(QIID qiid, char *name, Arhytmbox *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_RHYTMBOX2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0x55e4c338f93ea378), "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);

	outa=new Avideo(MKQIID(qiid, 0x55daa6247dd78240), "video output - send aux A", this, pinOUT, pos.w-38, 30);
	outa->setTooltips("video output");
	outa->show(TRUE);

	outb=new Avideo(MKQIID(qiid, 0x9b09c13d75880384), "video output - send aux B", this, pinOUT, pos.w-18, 30);
	outb->setTooltips("video output");
	outb->show(TRUE);

	in=new Avideo(MKQIID(qiid, 0xf4138cb06ee67000), "video input", this, pinIN, 10, 10);
	in->setTooltips("video intput");
	in->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArhytmboxBack::~ArhytmboxBack()
{
	delete(back);
	delete(out);
	delete(outa);
	delete(outb);
	delete(in);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ArhytmboxBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * ArhytmboxInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Arhytmbox(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * rhytmboxGetInfo()
{
	return new ArhytmboxInfo("rythmboxInfo", &Arhytmbox::CI, "rythmBox", "rythmBox module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
