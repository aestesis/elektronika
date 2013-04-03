/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SEQUENCER.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<assert.h>
#include						<math.h>
#include						"resource.h"
#include						"sequencer.h"
#include						"table.h"
#include						"effect.h"
#include						"pin.h"
#include						"elektro.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Asequencer::CI		= ACI("Asequencer",		GUID(0xE4EC7600,0x00000100), &Aobject::CI, 0, NULL);
EDLL ACI						AseqPattern::CI		= ACI("AseqPattern",	GUID(0xE4EC7600,0x00000101), &Aobject::CI, 0, NULL);
EDLL ACI						AseqTrackProp::CI	= ACI("AseqTrackProp",	GUID(0xE4EC7600,0x00000102), &Aobject::CI, 0, NULL);
EDLL ACI						AseqTrackEdit::CI	= ACI("AseqTrackEdit",	GUID(0xE4EC7600,0x00000103), &Aobject::CI, 0, NULL);
EDLL ACI						AseqEdit::CI		= ACI("AseqEdit",		GUID(0xE4EC7600,0x00000104), &Aobject::CI, 0, NULL);
EDLL ACI						AseqNote::CI		= ACI("AseqNote",		GUID(0xE4EC7600,0x00000105), &AseqEdit::CI, 0, NULL);
EDLL ACI						AseqControl::CI		= ACI("AseqControl",	GUID(0xE4EC7600,0x00000106), &AseqEdit::CI, 0, NULL);
EDLL ACI						Atrack::CI			= ACI("Atrack",			GUID(0xE4EC7600,0x00000110), &Aitem::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							COLORBACK						0xff606060


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						cursorPEN			= Aobject::makeCursor(&resdll.get(MAKEINTRESOURCE(PNG_SEQPENCURSOR), "PNG"), 1, 14);
static int						cursorERASE			= Aobject::makeCursor(&resdll.get(MAKEINTRESOURCE(PNG_SEQERASECURSOR), "PNG"), 0, 13);
static int						cursorBLUR			= Aobject::makeCursor(&resdll.get(MAKEINTRESOURCE(PNG_SEQBLURCURSOR), "PNG"), 7, 17);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static float					fres[]				= { 1.f/64.f, 1.f/32.f, 1.f/16.f, 1.f/8.f, 1.f/4.f, 1.f/2.f, 1.f, 2.f, 4.f };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqRef
{
public:
	class Acontrol				*control;
	class Atrack				*track;
	class AseqRef				*next;
	
								AseqRef							(Acontrol *c, Atrack *t);
	virtual						~AseqRef						();
								
#ifdef _DEBUG
	class AseqRef				*dbgNext;
#endif
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
static int						dbgNbSeqRef=0;
static AseqRef					*dbgSeqRef=NULL;
static int						dbgNbControlDeleted=0;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AseqRef::AseqRef(Acontrol *c, Atrack *t)
{
	control=c;
	track=t;
#ifdef _DEBUG	
	dbgNbSeqRef++;
	dbgNext=dbgSeqRef;
	dbgSeqRef=this;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AseqRef::~AseqRef()
{
#ifdef _DEBUG	
	dbgNbSeqRef--;
	{
		AseqRef	*s=dbgSeqRef;
		AseqRef	*o=NULL;
		while(s)
		{
			if(s==this)
			{
				if(o)
					o->dbgNext=dbgNext;
				else
					dbgSeqRef=dbgNext;
			}
			o=s;
			s=s->dbgNext;
		}
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
static int nbevents=0;
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aevent::Aevent(Atrack *t, int pattern, double beat, int nctrl, float value)
{
	this->track=t;
	this->pattern=pattern;
	this->beat=beat;
	this->nctrl=nctrl;
	this->value=value;
	this->flags=0;
	nextP=NULL;
	prevP=NULL;
	nextT=NULL;
	prevT=NULL;
#ifdef _DEBUG
	nbevents++;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aevent::~Aevent()
{
#ifdef _DEBUG
	nbevents--;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Atrack::Atrack(char *name, AseqPattern *spat, char *help, Anode *link) : Aitem(name, help)
{
	this->spat=spat;
	this->link=link;
	init();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Atrack::Atrack(char *name, AseqPattern *spat, Anode *link) : Aitem(name)
{
	this->spat=spat;
	this->link=link;
	init();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atrack::init()
{
	nSoloChild=0;
	controlType=0;
	if(link)
	{
		if(link->isCI(&Acontrol::CI))
		{
			int	i;
			type=CONTROL;
			cqiid=control->qiid^control->effect->getQIID();
			controlType=((Acontrol *)link)->type;
			for(i=0; i<CTRLMAXCONTROLER; i++)
			{
				colors[i]=control->midimap[i].color;
				strncpy(cname[i], control->midimap[i].name, sizeof(cname[i]));
				cname[i][sizeof(cname[i])-1]=0;
			}
		}
		else if(link->isCI(&Aeffect::CI))
		{
			type=EFFECT;
			eguid=effect->getCI()->guid;
		}
		else
			assert(false);
	}
	else
		type=GROUP;
	curE=NULL;
	curpat=0;
	memset(firstE, 0, sizeof(firstE));
	memset(lastE, 0, sizeof(lastE));
	setMode(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Atrack::~Atrack()
{
	spat->section.enter(__FILE__,__LINE__);
	{
		int	i;
		for(i=0; i<MAXPATTERN; i++)
			while(firstE[i])
				subEvent(firstE[i]);
	}
	switch(type&maskTYPE)
	{
		case CONTROL:
		spat->delref(this);
		break;
	}
	if(spat->treeview->current==this)
	{
		spat->treeview->current=NULL;
		spat->edit->setTrack(NULL);
		spat->prop->setTrack(NULL);
	}
	spat->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atrack::save(Afile *f)
{
	qword	q0=0;
	dword	magic=magicTRACK;
	f->write(&magic, sizeof(magic));
	f->write(&type, sizeof(type));
	f->write(&state, sizeof(state));
	switch(type&maskTYPE)
	{
		case 0:
		case PTRACK:
		break;
		
		case EFFECT:
		f->writeString(name);
		if(help)
			f->writeString(help);
		else
			f->writeString("");
		f->write(&eguid, sizeof(eguid));
		if(effect)
		{
			qword	q=effect->getQIID();
			f->write(&q, sizeof(q));
		}
		else
			f->write(&q0, sizeof(q0));
		break;
		
		case CONTROL:
		f->writeString(name);
		if(help)
			f->writeString(help);
		else
			f->writeString("");
		f->write(&cqiid, sizeof(cqiid));
		f->write(&controlType, sizeof(controlType));
		{
			int	nb=(controlType>=Acontrol::KEY)?0:(controlType-Acontrol::CONTROLER_01+1);
			int	i;
			f->write(&nb, sizeof(nb));
			for(i=0; i<nb; i++)
			{
				f->write(&colors[i], sizeof(colors[i]));
				f->writeString(cname[i]);
			}
		}
		{
			int	i;
			for(i=0; i<MAXPATTERN; i++)
			{
				int		nb=0;
				Aevent	*e=firstE[i];
				while(e)
				{
					nb++;
					e=e->nextT;
				}
				f->write(&nb, sizeof(nb));
				e=firstE[i];
				while(e)
				{
					f->write(&e->beat, sizeof(e->beat));
					f->write(&e->nctrl, sizeof(e->nctrl));
					f->write(&e->value, sizeof(e->value));
					e=e->nextT;
				}
			}
		}
		break;
		
		case GROUP:
		f->writeString(name);
		if(help)
			f->writeString(help);
		else
			f->writeString("");
		break;
	}
	{	// childrens
		Atrack	*t=(Atrack *)fchild;
		int		nb=0;
		while(t)
		{
			nb++;
			t=(Atrack *)t->next;
		}
		f->write(&nb, sizeof(nb));
		t=(Atrack *)fchild;
		while(t)
		{
			t->save(f);
			t=(Atrack *)t->next;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Atrack::load(Afile *f)
{
	nSoloChild=0;
	if(magictest(f, magicTRACK))
	{
		f->read(&type, sizeof(type));
		f->read(&state, sizeof(state));
		if(father&&isSolo())
			((Atrack *)father)->nSoloChild++;
		switch(type&maskTYPE)
		{
			case 0:
			case PTRACK:
			break;
			
			case EFFECT:
			if(name)
				free(name);
			f->readStringAlloc(&name);
			if(!name)
				name=strdup("no name");
			if(help)
				free(help);
			f->readStringAlloc(&help);
			if(!help)
				help=strdup("");
			f->read(&eguid, sizeof(eguid));
			{
				qword	q;
				f->read(&q, sizeof(q));
				if(q)
				{
					Atable	*table=spat->sequencer->table;
					int		i;
					for(i=0; i<table->nbEffect; i++)
					{
						Aeffect	*e=table->effect[i];
						if(e->getQIID()==q)
						{
							effect=e;
							break;
						}
					}
				}
			}
			break;
			
			case CONTROL:
			if(name)
				free(name);
			f->readStringAlloc(&name);
			if(!name)
				name=strdup("no name");
			if(help)
				free(help);
			f->readStringAlloc(&help);
			if(!help)
				help=strdup("");
			f->read(&cqiid, sizeof(cqiid));
			f->read(&controlType, sizeof(controlType));
			{
				int	nb;
				int	i;
				f->read(&nb, sizeof(nb));
				for(i=0; i<nb; i++)
				{
					f->read(&colors[i], sizeof(colors[i]));
					f->readString(cname[i]);
				}
			}
			{
				int	i;
				for(i=0; i<MAXPATTERN; i++)
				{
					int		nb;
					int		j;
					f->read(&nb, sizeof(nb));
					for(j=0; j<nb; j++)
					{
						double	beat;
						byte	nctrl;
						float	value;
						f->read(&beat, sizeof(beat));
						f->read(&nctrl, sizeof(nctrl));
						f->read(&value, sizeof(value));
						addEvent(i, nctrl, value, beat, beat, false);
					}
				}
			}
			break;
			
			case GROUP:
			if(name)
				free(name);
			f->readStringAlloc(&name);
			if(!name)
				name=strdup("no name");
			if(help)
				free(help);
			f->readStringAlloc(&help);
			if(!help)
				help=strdup("");
			break;

			default:
			assert(false);
			break;
		}
		{	// children
			int		nb;
			int		i;
			f->read(&nb, sizeof(nb));
			for(i=0; i<nb; i++)
			{
				Atrack	*t=new Atrack("", spat, null);
				add(t);
				if(!t->load(f))
					return false;
			}
		}
		switch(type&maskTYPE)
		{
			case EFFECT:
			spat->setTrackLink(this, effect);
			break;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atrack::subEvent(Aevent *e)
{
	spat->section.enter(__FILE__,__LINE__);
	{	// detach from track
		if(curE==e)
			curE=e->nextT;
		if(e->prevT)
			e->prevT->nextT=e->nextT;
		else
			firstE[e->pattern]=e->nextT;
		if(e->nextT)
			e->nextT->prevT=e->prevT;
		else
			lastE[e->pattern]=e->prevT;
		e->nextT=NULL;
		e->prevT=NULL;
	}
	{	// detach from seqPattern
		if(spat->curPlay==e)
			spat->curPlay=e->nextP;
		if(spat->curE==e)
			spat->curE=e->nextP;
		if(e->prevP)
			e->prevP->nextP=e->nextP;
		else
			spat->firstE[e->pattern]=e->nextP;
		if(e->nextP)
			e->nextP->prevP=e->prevP;
		else
			spat->lastE[e->pattern]=e->prevP;
		e->nextP=NULL;
		e->prevP=NULL;
	}
	spat->section.leave();
	delete(e);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atrack::delEvent(int curpat, int nctrl, double frombeat, double tobeat)
{
	spat->section.enter(__FILE__,__LINE__);
	{
		Aevent	*e=firstE[curpat];
		double	beat0=mini(frombeat, tobeat);
		double	beat1=maxi(frombeat, tobeat);
		while(e)
		{
			if(e->beat>=beat0)
				break;
			e=e->nextT;
		}
		while(e)
		{
			Aevent	*n=e->nextT;
			if(e->beat>beat1)
				break;
			if(e->nctrl==nctrl)
				subEvent(e);
			e=n;
		}
	}
	spat->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atrack::delEvent(int curpat, double frombeat, double tobeat)
{
	spat->section.enter(__FILE__,__LINE__);
	{
		Aevent	*e=firstE[curpat];
		double	beat0=mini(frombeat, tobeat);
		double	beat1=maxi(frombeat, tobeat);
		while(e)
		{
			if(e->beat>=beat0)
				break;
			e=e->nextT;
		}
		while(e)
		{
			Aevent	*n=e->nextT;
			if(e->beat>beat1)
				break;
			subEvent(e);
			e=n;
		}
	}
	spat->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Aevent * Atrack::addEvent(int curpat, int nctrl, float value, double beat, double gblbeat, bool skipsame)
{
	Aevent	*e=NULL;
	spat->section.enter(__FILE__,__LINE__);
	{	// create event and attach it to the track
		Aevent	*l;
		lastRecordEventBeat=gblbeat;
		
		if(curpat!=this->curpat)
		{
			curE=null;
			this->curpat=curpat;
		}

		if(!curE)
			curE=firstE[curpat];
		else if(curE&&curE->prevT&&(curE->prevT->beat>beat))
			curE=firstE[curpat];
			
		while(curE&&curE->prevT)
		{
			if(curE->prevT->beat<beat)
				break;
			curE=curE->prevT;
		}

		while(curE)
		{
			if(curE->beat>beat)
				break;
			curE=curE->nextT;
		}
		
		l=curE?curE:lastE[curpat];
		
		while(l)	
		{
			if(l->nctrl==nctrl)
			{
				if(l->beat<=beat)
					break;
			}
			l=l->prevT;
		}
		
		if(skipsame&&l)	// dont't create if previous event same value
		{
			if(l->value==value)
			{
				if(l->beat>beat)
					l->beat=beat;
				goto spat_attach;
			}
			else if(l->beat==beat)
			{
				l->value=value;
				goto spat_attach;
			}
		}
		/*
		if(l)	// remove next event if same value
		{	
			Aevent	*n=l->nextT;
			while(n)
			{
				if(n->beat-4.0>beat)
					break;
				if(n->nctrl==nctrl)
				{
					if(n->value==value)
						subEvent(n);
					break;
				}
				n=n->nextT;
			}
		}
		*/
		e=new Aevent(this, curpat, beat, nctrl, value);

		if(curE)
		{
			e->nextT=curE;
			e->prevT=curE->prevT;
			curE->prevT=e;
			if(e->prevT)
				e->prevT->nextT=e;
			else
				firstE[curpat]=e;
		}
		else
		{
			if(lastE[curpat])
			{
				e->prevT=lastE[curpat];
				e->nextT=NULL;
				lastE[curpat]->nextT=e;
				lastE[curpat]=e;
			}
			else
			{
				assert((e->nextT==NULL)&&(e->prevT==NULL));
				e->prevT=NULL;
				e->nextT=NULL;
				firstE[curpat]=e;
				lastE[curpat]=e;
			}
		}
	}
spat_attach:
	{
		if(e)
		{
			if(spat->curPlay&&(beat>=spat->curPlay->beat))
				e->flags|=Aevent::CREATE;
			{	// attach event to seqPattern
				if(!spat->curE)
					spat->curE=spat->firstE[curpat];
				else if(spat->curE&&spat->curE->prevP&&(spat->curE->prevP->beat>e->beat))
					spat->curE=spat->firstE[curpat];

				while(spat->curE)
				{
					if(spat->curE->beat>e->beat)
						break;
					spat->curE=spat->curE->nextP;
				}

				if(spat->curE)
				{
					e->nextP=spat->curE;
					e->prevP=spat->curE->prevP;
					spat->curE->prevP=e;
					if(e->prevP)
						e->prevP->nextP=e;
					else
						spat->firstE[curpat]=e;
				}
				else
				{
					if(spat->lastE[curpat])
					{
						e->prevP=spat->lastE[curpat];
						e->nextP=NULL;
						spat->lastE[curpat]->nextP=e;
						spat->lastE[curpat]=e;
					}
					else
					{
						assert((e->nextT==NULL)&&(e->prevT==NULL));
						e->prevP=NULL;
						e->nextP=NULL;
						spat->firstE[curpat]=e;
						spat->lastE[curpat]=e;
					}
				}
			}
			if(spat->recmode==AseqPattern::recmodeREPLACE)
			{
				Aevent	*ef=e->nextT;
				while(ef)
				{
					Aevent	*n=ef->nextT;
					assert(ef->beat>beat);
					subEvent(ef);
					ef=n;
				}
			}
		}
		else if(spat->recmode==AseqPattern::recmodeREPLACE)
		{
			Aevent	*e=firstE[curpat];
			while(e&&(e->beat<=beat))
				e=e->nextT;
			while(e)
			{
				Aevent	*n=e->nextT;
				assert(e->beat>beat);
				subEvent(e);
				e=n;
			}
		}
	}
	spat->section.leave();
	return e;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Atrack::setMode(int m)
{
	if(father)
	{
		int		o=type&SOLO;
		int		n=m&SOLO;
		if(o!=n)
			((Atrack *)father)->nSoloChild+=n?1:-1;
	}
	m&=maskMODE;
	type=(type&~maskMODE)|m;
	switch(m)
	{
		case NORMAL:
		data=0xff202020;
		break;
		
		case MUTE:
		data=0xff202080;
		break;
		
		case SOLO:
		data=0xff208020;
		break;
		
		case RECORD:
		data=0xff802020;
		break;

		case MUTE|SOLO:
		data=0xff208080;
		break;
		
		case RECORD|MUTE:
		data=0xff800080;
		break;
		
		case RECORD|SOLO:
		data=0xffffffff;
		break;
		
		case RECORD|SOLO|MUTE:
		data=0xffc0c0c0;
		break;
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

EDLL AseqTrackProp::AseqTrackProp(char *name, AseqPattern *spat, int x, int y, int w, int h) : Aobject(name, spat, x, y, w, h)
{
	this->spat=spat;	// 36
	
	mute=new Abutton("mute", this, 6, 4, 15, 16, &resdll.get(MAKEINTRESOURCE(PNG_MUTE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP|Abutton::btENABLEMODE);
	mute->setTooltips("mute track");
	mute->state&=~stateENABLE;
	mute->show(true);

	solo=new Abutton("solo", this, 26, 4, 15, 16, &resdll.get(MAKEINTRESOURCE(PNG_SOLO), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP|Abutton::btENABLEMODE);
	solo->setTooltips("solo track");
	solo->state&=~stateENABLE;
	solo->show(true);

	record=new Abutton("record", this, 46, 4, 15, 16, &resdll.get(MAKEINTRESOURCE(PNG_RECORD), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP|Abutton::btENABLEMODE);
	record->setTooltips("record track");
	record->state&=~stateENABLE;
	record->show(true);

	edit=new Aedit("name", this, 98, 4, 100, 15);
	edit->setTooltips("track name");
	edit->state&=~stateENABLE;
	edit->colorBorder=0xffc0c0c0;
	edit->yy=4;
	edit->show(true);

	refresh=new Abutton("refresh", this, 206, 4, 15, 16, &resdll.get(MAKEINTRESOURCE(PNG_REFRESH), "PNG"), Abutton::btBITMAP|Abutton::btENABLEMODE);
	refresh->setTooltips("refresh module's tracks");
	refresh->state&=~stateENABLE;
	refresh->show(true);

	del=new Abutton("deltrack", this, 226, 4, 15, 16, &resdll.get(MAKEINTRESOURCE(PNG_DELTRACK), "PNG"), Abutton::btBITMAP|Abutton::btENABLEMODE);
	del->setTooltips("delete track(s)");
	del->state&=~stateENABLE;
	del->show(true);

	ieffect=new Aitem("effect", "effect");
	
	leffect=new Alist("leffect", this, 270, 4, 104, 15, ieffect);
	leffect->setTooltips("effects list");
	leffect->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqTrackProp::~AseqTrackProp()
{
	delete(solo);
	delete(mute);
	delete(record);
	delete(edit);
	delete(refresh);
	delete(del);
	delete(leffect);
	delete(ieffect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqTrackProp::paint(Abitmap *b)
{
	Afont	*font=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xffffffff, 0.5f);
	font->set(b, 68, 8, "NAME", 0xff202020);
	font->set(b, 248, 8, "MOD", 0xff202020);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqTrackProp::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==record)
		{
			Atrack *t=(Atrack *)spat->treeview->getCurrent();
			while(t)
			{
				if((t->type&Atrack::maskTYPE)==Atrack::EFFECT)
					break;
				t=(Atrack *)t->father;
			}
			if(t)
			{
				if(t->effect)
				{
					spat->section.enter(__FILE__,__LINE__);
					if(record->isChecked())
					{
						Atrack	*tt=(Atrack *)spat->tracks->fchild;
						while(tt)
						{
							if((tt!=t)&&(tt->effect==t->effect))
								spat->setTrackRecordState(tt, false);
							tt=(Atrack *)tt->next;
						}						
					}
					spat->setTrackRecordState(t, record->isChecked());
					spat->section.leave();
					spat->treeview->repaint();
				}
			}
		}
		else if((o==mute)||(o==solo))
		{
			Atrack *t=(Atrack *)spat->treeview->getCurrent();
			if(t)
			{
				spat->section.enter(__FILE__,__LINE__);
				t->setMode((mute->isChecked()?Atrack::MUTE:0)|(solo->isChecked()?Atrack::SOLO:0)|(record->isChecked()?Atrack::RECORD:0));
				spat->section.leave();
				spat->treeview->repaint();
			}
		}
		else if(o==leffect)
		{
			spat->section.enter(__FILE__,__LINE__);
			Aitem *i=leffect->getCurrent();
			Atrack *t=(Atrack *)spat->treeview->getCurrent();
			while(t)
			{
				if((t->type&Atrack::maskTYPE)==Atrack::EFFECT)
				{
#ifdef _DEBUG			
					dbgNbControlDeleted=0;
#endif
					spat->setTrackLink(t, (Aeffect *)i->link);
					break;
				}
				t=(Atrack *)t->father;
			}
			spat->section.leave();
		}
		return true;

		case nyOK:
		if(o==edit)
		{
			Atrack *t=(Atrack *)spat->treeview->getCurrent();
			if(t)
			{
				if(edit->caption[0])
				{
					char	name[128];
					if(strlen(edit->caption)>128)
					{
						memcpy(name, edit->caption, 128);
						name[127]=0;
					}
					else
						strcpy(name, edit->caption);
					strupr(name);
					edit->set(name);
					t->setName(name);
					spat->treeview->refresh();
				}
				else
					edit->set(t->name);
			}
		}
		return true;
		
		case nyPRESS:
		if(o==del)
		{
			Atrack *t=(Atrack *)spat->treeview->getCurrent();
			if(t)
			{
				spat->section.enter(__FILE__,__LINE__);
				delete(t);
				spat->section.leave();
				spat->treeview->refresh();
			}
		}
		else if(o==refresh)
		{
			spat->section.enter(__FILE__,__LINE__);
			{
				Atable	*table=spat->sequencer->table;
				Atrack	*te=(Atrack *)spat->treeview->getCurrent();
				assert(te&&((te->type&Atrack::maskTYPE)==Atrack::EFFECT));
				Aeffect	*e=te->effect;
				if(e)
				{
					int		i;
					for(i=0; i<table->nbControl; i++)
					{
						Acontrol	*c=table->control[i];
						if(c->effect==e)
						{
							char	name[128];
							Atrack	*tc=te;
							strcpy(name, c->name);
							strupr(name);
							{
								char	*sf=name;
								char	*sl=strstr(sf, "/");

								if(sl)
									*sl=0;

								while(sl)
								{
									Atrack	*t=(Atrack *)tc->fchild;
									while(t)
									{
										if(!strcmp(t->name, sf))
											break;
										t=(Atrack *)t->next;
									}
									if(!t)
									{
										t=new Atrack(sf, spat, NULL);
										t->state|=Atrack::stateFOLDER|Atrack::stateCOLLAPSED;
										tc->add(t);
									}
									tc=t;

									sf=sl+1;
									sl=strstr(sf, "/");
									if(sl)
										*sl=0;
								}
								{
									Atrack	*t=(Atrack *)tc->fchild;
									while(t)
									{
										if(t->cqiid==(c->qiid^c->effect->getQIID()))
											break;
										t=(Atrack *)t->next;
									}
									if(!t)
									{
										Atrack	*t=new Atrack(sf, spat, ((Aobject *)c->object)->tooltips, c);
										tc->add(t);
										spat->addref(c, t);
									}
								}
							}
						}
					}
				}
			}
			spat->tracks->sort(Aitem::sortRECURSIVE);
			spat->section.leave();
			spat->treeview->refresh();
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqTrackProp::setTrack(Atrack *track)
{
	if(track)
	{
		Atrack	*te=NULL;
		
		mute->state|=stateENABLE;
		solo->state|=stateENABLE;
		refresh->state&=~stateENABLE;
		
		switch(track->type&Atrack::maskTYPE)
		{
			case Atrack::EFFECT:
			edit->state|=stateENABLE;
			edit->colorBorder=0xffffffff;
			record->state|=stateENABLE;
			del->state|=stateENABLE;
			refresh->state|=stateENABLE;
			del->state|=stateENABLE;
			break;
			
			case Atrack::GROUP:
			case Atrack::CONTROL:
			edit->state&=~stateENABLE;
			edit->colorBorder=0xffc0c0c0;
			record->state|=stateENABLE;
			del->state|=stateENABLE;
			break;

			default:
			edit->state&=~stateENABLE;
			edit->colorBorder=0xffc0c0c0;
			record->state&=~stateENABLE;
			del->state&=~stateENABLE;
			break;
		}
		{
			Atrack	*t=track;
			while(t)
			{
				if((t->type&Atrack::maskTYPE)==Atrack::EFFECT)
				{
					Atable	*table=spat->sequencer->table;
					Aitem	*si=NULL;
					int		i;
					ieffect->clear();
					si=new Aitem("[none]", "[none]", ieffect, NULL);
					for(i=0; i<table->nbEffect; i++)
					{
						Aeffect	*e=table->effect[i];
						if(e->getCI()->guid==t->eguid)
						{
							Aitem *it=new Aitem(e->name, e->name, ieffect, e);
							if(e==t->effect)
								si=it;
						}
					}
					leffect->setCurrent(si);
				}
				t=(Atrack *)t->father;
			}
		}
		
		edit->set(track->name);
		mute->setChecked((track->type&Atrack::MUTE)?true:false);
		solo->setChecked((track->type&Atrack::SOLO)?true:false);
		record->setChecked((track->type&Atrack::RECORD)?true:false);
		
		repaint();
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

EDLL AseqEdit::AseqEdit(char *name, class AseqTrackEdit *edit, int x, int y, int w, int h) : Aobject(name, edit, x, y, w, h)
{
	lastp=0.f;
	m_test=0;
	itool=toolNONE;
	this->edit=edit;
	track=NULL;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqEdit::~AseqEdit()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqEdit::tools(int itool)
{
	m_test=0;
	this->itool=itool;
	if(itool==toolNONE)
	{
		edit->toolbar->enableAll(false);
		edit->toolbar->enable(toolMAGNIFY);
		edit->toolbar->enable(toolHAND);
	}
	if(itool!=edit->toolbar->get())
		edit->toolbar->set(itool);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqEdit::pulse()
{
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqEdit::mouse(int x, int y, int state, int event)
{
	switch(itool)
	{
		case toolHAND:
		switch(event)
		{
			case mouseLDOWN:
			mouseCapture(true);
			cursor(cursorHANDKEEP);
			m_test=true;
			m_ivar=x;
			return true;

			case mouseLUP:
			case mouseMOVE:
			if(m_test)
			{
				edit->spat->section.enter(__FILE__,__LINE__);
				float	patlen=(float)edit->spat->patlen[edit->spat->curpat];
				edit->spat->section.leave();

				edit->xview+=edit->wview*(float)-(x-m_ivar)/(float)pos.w;
				m_ivar=x;
				edit->xview=maxi(mini(edit->xview, patlen-edit->wview), 0.f);
				edit->sbar->setPosition(edit->xview/patlen);
				
				if(event==mouseMOVE)
					cursor(cursorHANDKEEP);
				else
				{
					cursor(cursorHAND);
					m_test=false;
					mouseCapture(false);
				}
				
				repaint();
				edit->sbar->repaint();
			}
			else
				cursor(cursorHAND);
			return true;
		}
		break;
		
		case toolMAGNIFY:
		switch(event)
		{
			case mouseLDOWN:
			case mouseRDOWN:
			{
				float	w=edit->wview;
				if(event==mouseLDOWN)
				{
					w*=0.5f;
					cursor(cursorMAGNIFYADD);
					m_test=1;
				}
				else
				{
					w*=2.f;
					cursor(cursorMAGNIFYSUB);
					m_test=-1;
				}
				if(w<1.f)
					w=1.f;
				else if(w>128.f)
					w=128.f;
				if(w!=edit->wview)
				{
					edit->wview=w;
					edit->spat->section.enter(__FILE__,__LINE__);
					float	v=w/edit->spat->patlen[edit->spat->curpat];
					edit->spat->section.leave();
					if(v>1.f)
					{
						edit->xview=0.f;
						edit->sbar->setPosition(0.f);
					}
					edit->sbar->setView(v);
					edit->sbar->repaint();
					repaint();
				}
			}
			return true;

			case mouseRUP:
			case mouseLUP:
			m_test=0;
			case mouseMOVE:
			switch(m_test)
			{
				case 0:
				cursor(cursorMAGNIFY);
				break;
				
				case 1:
				cursor(cursorMAGNIFYADD);
				break;
				
				case -1:
				cursor(cursorMAGNIFYSUB);
				break;
			}
			return true;
			
//			case mouseRDBLCLK:
//			cursor(cursorMAGNIFYSUB);
//			return true;
		}
		break;

		case toolERASE:
		switch(event)
		{
			case mouseLDOWN:
			m_test=true;
			mouseCapture(true);
			cursor(cursorERASE);
			edit->spat->section.enter(__FILE__,__LINE__);
			lastp=edit->xview+edit->wview*(float)x/(float)pos.w;
			edit->spat->section.leave();
			return true;

			case mouseLUP:
			cursor(cursorERASE);
			case mouseLEAVE:
			if(m_test)
			{
				mouseCapture(false);
				m_test=false;
			}
			return true;

			case mouseMOVE:
			cursor(cursorERASE);
			if(m_test)
			{
				edit->spat->section.enter(__FILE__,__LINE__);
				{
					float	p=edit->xview+edit->wview*(float)x/(float)pos.w;
					track->delEvent(edit->spat->curpat, lastp, p);
					lastp=p;
				}
				edit->spat->section.leave();
				repaint();
			}
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqEdit::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xffffffff, 0.3f);
	edit->spat->section.enter(__FILE__,__LINE__);
	paintBack(b);
	{
		int	w=f->getWidth("NO TRACK");
		f->set(b, (pos.w-w)>>1, (pos.h-f->h)>>1, "NO TRACK", 0xff000000);
	}
	paintFront(b);
	edit->spat->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqEdit::setTrack(Atrack *t)
{
	track=t;
	tools(toolNONE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqEdit::paintBack(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	float	dx=(float)pos.w/edit->wview;
	float	x=-(edit->xview-(float)(int)edit->xview)*dx;
	int		n=(int)edit->xview;
	int		p=((int)edit->wview+7)/8;

	if(p<1)
		p=1;

	while(x<(float)pos.w)
	{
		char	str[128];
		int		xi=(int)x;
		itoa(n+1, str, 10);
		b->line(xi, 0, xi, pos.h, COLORBACK);
		f->set(b, xi+2, pos.h-f->h, str, 0xff00ff00);
		n+=p;
		x+=dx*(float)p;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqEdit::paintFront(Abitmap *b)
{
	float	dx=(float)pos.w/edit->wview;
	int	x=(int)((edit->spat->beat-edit->xview)*dx);
	b->line(x, 0, x, pos.h, 0xffffffff);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqControl::AseqControl(char *name, class AseqTrackEdit *edit, int x, int y, int w, int h) : AseqEdit(name, edit, x, y, w, h)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqControl::~AseqControl()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqControl::tools(int itool)
{
	AseqEdit::tools(itool);
	if(itool==toolNONE)
	{
		edit->toolbar->enable(toolPEN);
		edit->toolbar->enable(toolERASE);
		edit->toolbar->enable(toolSMOOTH);
		edit->toolbar->set(toolPEN);
		this->itool=toolPEN;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqControl::mouse(int x, int y, int state, int event)
{
	switch(itool)
	{
		case toolPEN:
		switch(event)
		{
			case mouseLDOWN:
			m_test=true;
			mouseCapture(true);
			cursor(cursorPEN);
			edit->spat->section.enter(__FILE__,__LINE__);
			lastp=edit->xview+edit->wview*(float)x/(float)pos.w;
			edit->spat->section.leave();
			// no break, need follow on mouseMOVE

			case mouseMOVE:
			cursor(cursorPEN);
			if(m_test)
			{
				edit->spat->section.enter(__FILE__,__LINE__);
				{
					float	v=maxi(mini((float)(pos.h-y)/(float)pos.h, 1.f), 0.f);
					float	p=maxi(edit->xview+edit->wview*(float)x/(float)pos.w, 0.f);
					float	pr=(float)((int)(p/edit->res))*edit->res;
					float	lastpr=(float)((int)(lastp/edit->res))*edit->res;
					if(p>lastpr)
						track->delEvent(edit->spat->curpat, nctrl, lastpr+0.0001, pr+edit->res-0.0001);
					else
						track->delEvent(edit->spat->curpat, nctrl, pr, lastpr-0.0001);
					track->addEvent(edit->spat->curpat, nctrl, v, pr, pr, true);
					lastp=p;
				}
				edit->spat->section.leave();
				repaint();
			}
			return true;

			case mouseLUP:
			cursor(cursorPEN);
			case mouseLEAVE:
			if(m_test)
			{
				mouseCapture(false);
				m_test=false;
			}
			return true;
		}
		break;

		case toolSMOOTH:
		switch(event)
		{
			case mouseLDOWN:
			cursor(cursorBLUR);
			return true;

			case mouseLUP:
			cursor(cursorBLUR);
			return true;

			case mouseMOVE:
			cursor(cursorBLUR);
			return true;
		}
		break;
	}
	return AseqEdit::mouse(x, y, state, event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqControl::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xffffffff, 0.3f);

	edit->spat->section.enter(__FILE__,__LINE__);

	paintBack(b);
	
	b->line(0, pos.h>>2, pos.w, pos.h>>2, COLORBACK);
	b->line(0, pos.h>>1, pos.w, pos.h>>1, COLORBACK);
	b->line(0, (pos.h>>1)+(pos.h>>2), pos.w, (pos.h>>1)+(pos.h>>2), COLORBACK);
	
	if(track)
	{
		float	dx=(float)pos.w/edit->wview;
		int		np=track->controlType+1;

		{
			//Aevent	*l=track->lastE[edit->spat->curpat];

			if(track->firstE[edit->spat->curpat])
			{
				int	i;
				for(i=1; i<np; i++)
				{
					Aevent	*e=track->firstE[edit->spat->curpat];
					int		x,y;
					int		xf=-1; 
					int		yf=-1;
					
					while(e)
					{
						int		nc=e->nctrl;
						if(nc==i)
						{
							x=(int)((e->beat-edit->xview)*dx);
							y=(pos.h-1)-(int)((float)(pos.h-1)*e->value);
							if((x>0)&&(xf!=-1))
							{
								dword	color=track->colors[nc-1];
								b->boxfa(xf, yf, x, pos.h-1, color, 0.4f);
								b->line(xf, yf, x, yf, color);
								b->line(x, yf, x, y, color);
							}
							else if(x>pos.w)
								break;
							xf=x;
							yf=y;
						}
						e=e->nextT;
					}
					{
						dword	color=track->colors[i-1];
						if((xf!=-1)&&(xf<pos.w))
						{
							b->boxfa(xf, yf, pos.w, pos.h-1, color, 0.4f);
							b->line(xf, yf, pos.w, yf, color);
						}
					}
				}
			}
			else
			{
				int		w=f->getWidth("NO EVENTS");
				f->set(b, (pos.w-w)>>1, (pos.h-f->h)>>1, "NO EVENTS", 0xff000000);
			}
		}
	}
	
	paintFront(b);
	edit->spat->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqNote::AseqNote(char *name, class AseqTrackEdit *edit, int x, int y, int w, int h) : AseqEdit(name, edit, x, y, w, h)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqNote::~AseqNote()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqNote::tools(int itool)
{
	AseqEdit::tools(itool);
	if(itool==toolNONE)
	{
		edit->toolbar->enable(toolPEN);
		edit->toolbar->enable(toolERASE);
		edit->toolbar->set(toolPEN);
		this->itool=toolPEN;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqNote::mouse(int x, int y, int state, int event)
{
	switch(itool)
	{
		case toolPEN:
		switch(event)
		{
			case mouseLDOWN:
			m_test=true;
			mouseCapture(true);
			cursor(cursorPEN);
			edit->spat->section.enter(__FILE__,__LINE__);
			lastp=edit->xview+edit->wview*(float)x/(float)pos.w;
			edit->spat->section.leave();
			// no break, need follow on mouseMOVE

			case mouseMOVE:
			cursor(cursorPEN);
			if(m_test)
			{
				edit->spat->section.enter(__FILE__,__LINE__);
				{
					int		d=track->controlType-Acontrol::KEY;
					int		v=maxi(mini((int)((float)(d+1)*(float)(pos.h-y)/(float)pos.h), d), 0);
					float	p=maxi(edit->xview+edit->wview*(float)x/(float)pos.w, 0.f);
					float	pr=(float)((int)(p/edit->res))*edit->res;
					float	lastpr=(float)((int)(lastp/edit->res))*edit->res;
					if(p>lastpr)
						track->delEvent(edit->spat->curpat, Acontrol::KEY, lastpr+0.0001, pr+edit->res-0.0001);
					else
						track->delEvent(edit->spat->curpat, Acontrol::KEY, pr, lastpr-0.0001);
					track->addEvent(edit->spat->curpat, Acontrol::KEY, (float)v, pr, pr, true);
					lastp=p;
				}
				edit->spat->section.leave();
				repaint();
			}
			return true;

			case mouseLUP:
			cursor(cursorPEN);
			case mouseLEAVE:
			if(m_test)
			{
				mouseCapture(false);
				m_test=false;
			}
			return true;
		}
		break;

	}
	return AseqEdit::mouse(x, y, state, event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqNote::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xffffffff, 0.3f);
	edit->spat->section.enter(__FILE__,__LINE__);

	if(track)
	{
		float	dx=(float)pos.w/edit->wview;
		int		n=(track->controlType-Acontrol::KEY_01)+1;
		double	l=((double)pos.h-1.0)/(double)n;
		Aevent	*e=track->firstE[edit->spat->curpat];
		int		i;
		int		x,y,y2;
		int		xf=-1;
		double	vf=0;
		bool	bne=(e==null);

		if(e)
		{
			while(e)
			{
				x=(int)((e->beat-edit->xview)*dx);
				y=(int)((double)(pos.h-1)-l*vf-1.0);
				y2=(int)((double)(pos.h-1)-l*(vf+1.0)+1.0);
				if((x>0)&&(xf!=-1))
				{
					b->boxfa(xf, y, x-1, y2, 0xff000000, 0.4f);
					b->box(xf, y, x-1, y2, 0xff000000);
				}
				xf=x;
				vf=e->value;
				e=e->nextT;
			}

			if(xf<pos.w)
			{
				y=(int)((double)(pos.h-1)-l*vf-1.0);
				y2=(int)((double)(pos.h-1)-l*(vf+1.f)+1.0);
				b->boxfa(xf, y, pos.w, y2, 0xff000000, 0.4f);
				b->box(xf, y, pos.w, y2, 0xff000000);
			}
		}

		for(i=0; i<(n+1); i++)
		{
			int	y=(int)((double)(pos.h-1)-((double)i*l));
			b->linea(0, y, pos.w, y, COLORBACK, 0.5f);
		}

		if(bne)
		{
			int		w=f->getWidth("NO EVENTS");
			f->set(b, (pos.w-w)>>1, (pos.h-f->h)>>1, "NO EVENTS", 0xff000000);
		}
	}
	paintBack(b);
	paintFront(b);
	edit->spat->section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqTrackEdit::AseqTrackEdit(char *name, class AseqPattern *spat, int x, int y, int w, int h) : Aobject(name, spat, x, y, w, h)
{
	this->spat=spat;
	
	econtrol=new AseqControl("edit control", this, 4, 24, pos.w-8, pos.h-38);
	enote=new AseqNote("edit note", this, 4, 24, pos.w-8, pos.h-38);
	enothing=new AseqEdit("edit nothing", this, 4, 24, pos.w-8, pos.h-38);
	ecurrent=enothing;
	ecurrent->show(true);

	wview=4.f;
	xview=0.f;
	
	sbar=new AscrollBar("sbar", this, 4, h-14, w-8, 10, AscrollBar::HORIZONTAL);
	sbar->setView(wview/spat->patlen[spat->curpat]);
	sbar->show(true);
	
	ictrl=new Aitem("controls", "controls");
	
	lctrl=new Alist("lctrl", this, 270, 4, 104, 15, ictrl);
	lctrl->setTooltips("controls list");
	lctrl->show(TRUE);

	ires=new Aitem("resolution", "resolution");

	new Aitem("1/64", "1/64", ires, 0);
	new Aitem("1/32", "1/32", ires, 1);
	new Aitem("1/16", "1/16", ires, 2);
	new Aitem("1/08", "1/8", ires, 3);
	new Aitem("1/04", "1/4", ires, 4);
	new Aitem("1/02", "1/2", ires, 5);
	new Aitem("1/01", "1/1", ires, 6);
	new Aitem("2/01", "2/1", ires, 7);
	new Aitem("4/01", "4/1", ires, 8);
	
	lres=new Alist("lres", this, 220, 4, 40, 15, ires);
	lres->setTooltips("resolution");
	lres->show(TRUE);
	lres->setCurrentByData(3);
	res=fres[3];
	
	toolbar=new AtoolBar("design tools", this, 12, 2, 150, 20);
	toolbar->colorSEPARATOR=0xff606060;
	toolbar->add("pen", "pen tool (draw events)", &resdll.get(MAKEINTRESOURCE(PNG_SEQPEN), "PNG"), AseqEdit::toolPEN);
	toolbar->add("eraser", "eraser tool (erase events)", &resdll.get(MAKEINTRESOURCE(PNG_SEQGOMME), "PNG"), AseqEdit::toolERASE);
	toolbar->addSeparator();
	//toolbar->add("smooth", "smooth tool (smooth events)", &resdll.get(MAKEINTRESOURCE(PNG_SEQBLUR), "PNG"), AseqEdit::toolSMOOTH);
	//toolbar->addSeparator();
	//toolbar->add("copy", "copy tool", &resdll.get(MAKEINTRESOURCE(PNG_SEQPIPETTE), "PNG"), AseqEdit::toolCOPY);
	//toolbar->add("paste", "paste tool", &resdll.get(MAKEINTRESOURCE(PNG_SEQTAMPON), "PNG"), AseqEdit::toolPASTE);
	//toolbar->addSeparator();
	toolbar->add("magnify", "magnify tool (stretch time view)", &resdll.get(MAKEINTRESOURCE(PNG_SEQMAGNIFY), "PNG"), AseqEdit::toolMAGNIFY);
	toolbar->add("hand", "hand tool (scroll time view)", &resdll.get(MAKEINTRESOURCE(PNG_SEQHAND), "PNG"), AseqEdit::toolHAND);
	toolbar->show(true); 

	ecurrent->tools(AseqEdit::toolNONE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqTrackEdit::~AseqTrackEdit()
{
	delete(econtrol);
	delete(enote);
	delete(enothing);
	delete(sbar);
	delete(toolbar);
	delete(ictrl);
	delete(lctrl);
	delete(ires);
	delete(lres);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqTrackEdit::paint(Abitmap *b)
{
	Afont	*font=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xffffffff, 0.5f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqTrackEdit::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==sbar)
		{
			float p=sbar->getPosition();
			spat->section.enter(__FILE__,__LINE__);
			xview=p*spat->patlen[spat->curpat];
			spat->section.leave();
			ecurrent->repaint();
		}
		else if(o==spat)
		{
			spat->section.enter(__FILE__,__LINE__);
			sbar->setView(wview/spat->patlen[spat->curpat]);
			spat->section.leave();
		}
		else if(o==toolbar)
		{
			ecurrent->tools(p);
			repaint();
		}
		else if(o==lctrl)
		{
			Aitem	*i=(Aitem *)p;
			econtrol->nctrl=i->data;
		}
		else if(o==lres)
		{
			res=fres[lres->getCurrentData()];
		}
		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqTrackEdit::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseWHEEL:
		{
			float	dw=(float)(-getWindow()->mouseW);
			float	ox=xview;
			float	dp=(float)pow(2.0, (double)dw/120.f);
			wview*=dp;
			if(wview<1.f)
				wview=1.f;
			else if(wview>128.f)
				wview=128.f;
			if(ox!=wview)
			{
				spat->section.enter(__FILE__,__LINE__);
				float	v=wview/spat->patlen[spat->curpat];
				spat->section.leave();
				if(v>1.f)
				{
					xview=0.f;
					sbar->setPosition(0.f);
				}
				sbar->setView(v);
				repaint();
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqTrackEdit::size(int w, int h)
{
	sbar->pos.y=h-14;
	sbar->size(sbar->pos.w, mini(10, h-24));
	econtrol->size(econtrol->pos.w, h-econtrol->pos.y-14);
	enote->size(enote->pos.w, h-enote->pos.y-14);
	enothing->size(enothing->pos.w, h-enothing->pos.y-14);
	return Aobject::size(w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqTrackEdit::setTrack(Atrack *track)
{
	econtrol->show(false);
	enote->show(false);
	enothing->show(false);
	ecurrent=enothing;
	ictrl->clear();
	
	if(track)
	{
		switch(track->type&Atrack::maskTYPE)
		{
			case Atrack::CONTROL:
			{
				if(track->controlType>=Acontrol::KEY_01)
				{
					new Aitem("key", "key", ictrl, Acontrol::KEY_01, this);	// not used, just to see something in the listbox
					lctrl->setCurrentByData(Acontrol::KEY_01);
					ecurrent=enote;
				}
				else
				{
					int	nb=1+track->controlType-Acontrol::CONTROLER_01;
					int	i;
					for(i=0; i<nb; i++)
					{
						char	str[1024];
						if(track->cname[i][0])
							strcpy(str, track->cname[i]);
						else
							sprintf(str, "ctrl #%d", i+1);
						new Aitem(str, str, ictrl, Acontrol::CONTROLER_01+i, this);
					}
					lctrl->setCurrentByData(Acontrol::CONTROLER_01);
					econtrol->nctrl=Acontrol::CONTROLER_01;
					ecurrent=econtrol;
				}
			}
			break;
			
			default:
			break;
		}
		ecurrent->setTrack(track);
	}
	else
	{
		enote->setTrack(NULL);
		econtrol->setTrack(NULL);
		enothing->setTrack(NULL);
	}
	ecurrent->show(true);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqPattern::AseqPattern(char *name, Asequencer *seq, int x, int y, int w, int h) : Aobject(name, seq, x, y, w, h)
{
	int	i;
	sequencer=seq;
	curpat=0;
	recmode=recmodeOVERDUB;
	
	memset(npreset, 0, sizeof(npreset));
	memset(usepreset, 0, sizeof(usepreset));
	
	memset(ref, 0, sizeof(ref));
	
	tracks=new Atrack("TRACKS", this, NULL);
	tracks->state|=Atrack::stateFOLDER;
	tracks->type=Atrack::NONE;
	
	for(i=0; i<MAXPATTERN; i++)
		patlen[i]=DEFPATTERNLEN;
	
	memset(firstE, 0, sizeof(firstE));
	memset(lastE, 0, sizeof(lastE));
	curE=NULL;
	
	curPlay=NULL;
	gblbeat=0.f;
	dlenght=0;
	ctrlgblbeat=-1.f;
	
	new Aitem("mute tracks", "mute all tracks", context, contextSEQMUTEALL, this);
	new Aitem("unmute tracks", "unmute all tracks", context, contextSEQUNMUTEALL, this);
	new Aitem("unsolo tracks", "remove the solo flag on all tracks", context, contextSEQUNSOLOALL, this);

	treeview=new AtreeView("tracks", this, 4, 4, 160, h-8, tracks);
	treeview->setTooltips("tracks tree view");
	treeview->state|=AtreeView::stateUSECOLORDATA;
	treeview->show(true);

	position=new Asegment("position", this, 320, 18, 4, 1, 9999, alib.getFont(fontSEGMENT10), 0xff00c000, 0.005f);
	position->set(0);
	position->setTooltips("current position");
	position->state&=~stateENABLE;
	position->show(true);

	position2=new Asegment("position", this, 368, 18, 3, 1, 999, alib.getFont(fontSEGMENT10), 0xff00c000, 0.005f);
	position2->set(0);
	position2->setTooltips("current position");
	position2->state&=~stateENABLE;
	position2->show(true);

	lenght=new Asegment("lenght", this, 320, 34, 4, 1, 9999, alib.getFont(fontSEGMENT10), 0xff00c000, 0.005f);
	lenght->set(DEFPATTERNLEN);
	lenght->setTooltips("pattern lenght");
	lenght->show(true);
	
	up1=new Abutton("up", this, 368, 34, 9, 5, &resdll.get(MAKEINTRESOURCE(PNG_LCDUP), "PNG"), Abutton::btBITMAP);
	up1->setTooltips("lenght increase");
	up1->show(true);

	down1=new Abutton("up", this, 368, 42, 9, 5, &resdll.get(MAKEINTRESOURCE(PNG_LCDDOWN), "PNG"), Abutton::btBITMAP);
	down1->setTooltips("lenght decrease");
	down1->show(true);

	up10=new Abutton("up", this, 382, 34, 9, 5, &resdll.get(MAKEINTRESOURCE(PNG_LCDUP), "PNG"), Abutton::btBITMAP);
	up10->setTooltips("lenght increase");
	up10->show(true);

	down10=new Abutton("up", this, 382, 42, 9, 5, &resdll.get(MAKEINTRESOURCE(PNG_LCDDOWN), "PNG"), Abutton::btBITMAP);
	down10->setTooltips("lenght decrease");
	down10->show(true);

	editmode=new Abutton("editmode", this, 420, 20, 14, 26, &resdll.get(MAKEINTRESOURCE(PNG_BIGSWITCH), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	editmode->setTooltips("recording mode [overdub/replace]");
	editmode->setChecked(recmode==recmodeREPLACE);
	editmode->show(true);

	preset=new Asegment("preset", this, 488, 18, 3, 0, 127, alib.getFont(fontSEGMENT10), 0xff00c000, 0.5f);
	preset->setTooltips("preset");
	preset->show(true);

	ponoff=new Abutton("ponoff", this, 498, 36, 9, 9, &resdll.get(MAKEINTRESOURCE(PNG_LCDONOFF), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	ponoff->setTooltips("use preset");
	ponoff->show(true);

	prop=new AseqTrackProp("prop", this, 168, 54, pos.w-4-168, 23);
	prop->show(true);
	
	edit=new AseqTrackEdit("edit", this, 168, 80, pos.w-4-168, pos.h-80-4);
	edit->show(true);

	// must be the last created cause of the midi control mapping of this objects

	bank=new Aselect(0x84d54e854f5e4854, "bank", this, 174, 16, 2, 2, &resdll.get(MAKEINTRESOURCE(PNG_BANK), "PNG"), 14, 15);
	bank->setTooltips("banks");
	bank->set(0);
	bank->show(TRUE);

	pat=new Aselect(0x84d48e45d4ce4d5a, "pat", this, 214, 16, 4, 2, &resdll.get(MAKEINTRESOURCE(PNG_PATTERN), "PNG"), 14, 15);
	pat->setTooltips("patterns");
	pat->set(0);
	pat->show(TRUE);
	
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqPattern::~AseqPattern()
{
	delete(bank);
	delete(pat);
	delete(position);
	delete(position2);
	delete(lenght);
	delete(up1);
	delete(down1);
	delete(up10);
	delete(down10);
	delete(editmode);
	delete(preset);
	delete(ponoff);
	delete(tracks);
	delete(treeview);
	delete(prop);
	delete(edit);
	assert(nbevents==0);
	assert(dbgNbSeqRef==0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::clear()
{
	section.enter(__FILE__,__LINE__);
	{
		Atrack	*t=(Atrack *)tracks->fchild;
		while(t)
		{
			Atrack	*n=(Atrack *)t->next;
			delete(t);
			t=n;
		}
	}
	section.leave();
	treeview->refresh();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::clearInit()
{
	Atable *table=sequencer->table;
	table->add(bank->control);
	table->add(pat->control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::load(Afile *f)
{
	if(magictest(f, magicPATTERN))
	{
		if(safemagictest(f, magicPATTERNLENGHT))
		{
			f->read(&curpat, sizeof(curpat));
			f->read(&patlen, sizeof(patlen));
		}
			
		
		if(safemagictest(f, magicPATTERNPRESET))
		{
			f->read(&npreset, sizeof(npreset));
			f->read(&usepreset, sizeof(usepreset));
		}
		
		section.enter(__FILE__,__LINE__);
		tracks->clear();
		if(!tracks->load(f))
			return false;
		tracks->sort(Aitem::sortRECURSIVE);
		section.leave();
		treeview->refresh();
	}
	else
		return false;

	bank->set(curpat>>3);
	pat->set(curpat&7);

	preset->set(npreset[curpat]);
	ponoff->setChecked(usepreset[curpat]);
	lenght->set(patlen[curpat]);

	if(usepreset[curpat])
		sequencer->table->loadPreset(npreset[curpat], true);
		
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::save(Afile *f)
{
	dword	magic=magicPATTERN;
	f->write(&magic, sizeof(magic));

	{
		dword	magic=magicPATTERNLENGHT;
		f->write(&magic, sizeof(magic));
		f->write(&curpat, sizeof(curpat));
		f->write(&patlen, sizeof(patlen));
	}
	
	{
		dword	magic=magicPATTERNPRESET;
		f->write(&magic, sizeof(magic));
		f->write(&npreset, sizeof(npreset));
		f->write(&usepreset, sizeof(usepreset));
	}
	
	tracks->save(f);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::pulse()
{
	position->set((int)beat+1);
	position2->set((int)((beat-(float)(int)beat)*1000.f));
	if(dlenght)
	{
		int	l=lenght->get()+dlenght;
		l=l&~(abs(dlenght)-1);
		if(l<=0)
			l=1;
		else if(l>9999)
			l=9999;
		section.enter(__FILE__,__LINE__);
		patlen[curpat]=l;
		section.leave();
		lenght->set(l);
		edit->notify(this, nyCHANGE);				
	}
	
#ifdef _DEBUG
	repaint();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyPRESS:
		if(o==up1)
			dlenght=4;
		else if(o==up10)
			dlenght=16;
		else if(o==down1)
			dlenght=-4;
		else if(o==down10)
			dlenght=-16;
		break;
		
		case nyRELEASE:
			if((o==up1)||(o==up10)||(o==down1)||(o==down10))
				dlenght=0;
		break;
		
		case nyCONTEXT:
		{
			Aitem	*i=(Aitem *)p;
			switch(i->data)
			{
				case contextSEQMUTEALL:
				section.enter(__FILE__,__LINE__);
				{
					Atrack	*t=(Atrack *)tracks->fchild;
					while(t)
					{
						t->setMode(t->type|Atrack::MUTE);
						t=(Atrack *)t->next;
					}					
				}
				section.leave();
				treeview->refresh();
				break;
				
				case contextSEQUNMUTEALL:
				section.enter(__FILE__,__LINE__);
				{
					Atrack	*t=(Atrack *)tracks;
					while(t)
					{
						t->setMode(t->type&~Atrack::MUTE);
						if(t->fchild)
							t=(Atrack *)t->fchild;
						else if(t->next)
							t=(Atrack *)t->next;
						else if(t->father)
						{
							while(t->father)
							{
								t=(Atrack *)t->father;
								if(t->next)
									break;
							}
							t=(Atrack *)t->next;
						}
						else
							break;
					}					
				}
				section.leave();
				treeview->refresh();
				break;
				
				case contextSEQUNSOLOALL:
				section.enter(__FILE__,__LINE__);
				{
					Atrack	*t=(Atrack *)tracks;
					while(t)
					{
						t->setMode(t->type&~Atrack::SOLO);
						if(t->fchild)
							t=(Atrack *)t->fchild;
						else if(t->next)
							t=(Atrack *)t->next;
						else if(t->father)
						{
							while(t->father)
							{
								t=(Atrack *)t->father;
								if(t->next)
									break;
							}
							t=(Atrack *)t->next;
						}
						else
							break;
					}					
				}
				section.leave();
				treeview->refresh();
				break;
			}
		}
		break;

		case nyCHANGE:
		if(o==preset)
		{
			int	p=preset->get();
			section.enter(__FILE__,__LINE__);
			int	cp=curpat;
			npreset[curpat]=p;
			section.leave();
			if(usepreset[cp])
				sequencer->table->loadPreset(npreset[cp], true);
		}
		else if(o==ponoff)
		{
			bool	use=ponoff->isChecked();
			section.enter(__FILE__,__LINE__);
			int	cp=curpat;
			usepreset[curpat]=use;
			section.leave();
			if(usepreset[cp])
				sequencer->table->loadPreset(npreset[cp], true);
		}
		else if(o==lenght)
		{
			int	l=lenght->get();
			section.enter(__FILE__,__LINE__);
			patlen[curpat]=l;
			section.leave();
			position->setMax(l);
			edit->notify(this, nyCHANGE);				
			//repaint();
		}
		else if(o==position)
		{
			// todo:
		}
		else if(o==bank)
		{
			pat->set(-1);
		}
		else if(o==pat)
		{
			if(pat->get()==-1)
				pat->set(0);
			set(bank->get()*8+pat->get());
		}
		else if(o==editmode)
		{
			recmode=editmode->isChecked()?AseqPattern::recmodeREPLACE:AseqPattern::recmodeOVERDUB;
		}
		return true;
		
		case nySELECT:
		if(o==treeview)
		{
			Aitem *i=treeview->getCurrent();
			prop->setTrack((Atrack *)i);
			edit->setTrack((Atrack *)i);
		}
		return TRUE;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::paint(Abitmap *b)
{
	Afont	*font=alib.getFont(fontTERMINAL06);
	b->boxfa(168, 4, pos.w-5, 50, 0xffffffff, 0.5f);
	b->boxfa(290, 16, 400, 48, 0xff000000, 0.7f);
	b->boxfa(452, 16, 520, 48, 0xff000000, 0.7f);
	font->set(b, 174, 8, "BANK", 0xff202020);
	font->set(b, 214, 8, "PATTERN", 0xff202020);
	font->set(b, 290, 8, "TIME", 0xff202020);
	font->set(b, 294, 22, "POS", 0xff00c000);
	font->set(b, 294, 38, "LEN", 0xff00c000);
	font->set(b, 418, 8, "REC", 0xff202020);
	font->set(b, 413, 22, "O", 0xff202020);
	font->set(b, 413, 38, "R", 0xff202020);
	font->set(b, 436, 22, "O", 0xff202020);
	font->set(b, 436, 38, "R", 0xff202020);
	font->set(b, 452, 8, "PRESET", 0xff202020);
	font->set(b, 456, 22, "NUM", 0xff00c000);
	font->set(b, 456, 38, "USE", 0xff00c000);
#ifdef _DEBUG
/*
	{
		Afont	*f=alib.getFont(fontTERMINAL06);
		char	str[1024];
		sprintf(str, "NBSEQREF %d", dbgNbSeqRef);
		f->set(b, 200, 86, str, 0xff000000);
		sprintf(str, "NBEVENTS %d", nbevents);
		f->set(b, 320, 86, str, 0xff000000);
	}
*/
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::size(int w, int h)
{
	treeview->size(treeview->pos.w, h-8);
	edit->size(edit->pos.w, h-edit->pos.y-4);
	return Aobject::size(w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::record(Acontrol *c, int nctrl, float value, double gblbeat)
{
	section.enter(__FILE__,__LINE__);
	{
		AseqRef	*r=getref(c);
		calcBeat(gblbeat);
		if((gblbeat>=ctrlgblbeat)&&(gblbeat<ctrlgblbeat+0.5f))	// ignore new control first set event
		{
			section.leave();
			return;
		}
		else
			ctrlgblbeat=-1.f;
		while(r)
		{
			if(r->control==c)
			{
				if(r->track&&r->track->isRecording())
				{
					r->track->addEvent(curpat, nctrl, value, beat, gblbeat, true);
					break;	// because, there can be only one effect track recording a module
				}
			}
			r=r->next;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::play(double gblbeat)
{
	section.enter(__FILE__,__LINE__);
	{
		double	mbeat=(float)patlen[curpat];
		int		loop=((int)gblbeat/patlen[curpat])-((int)this->gblbeat/patlen[curpat]);

		calcBeat(gblbeat);
		
		if(loop)
		{
			while(curPlay&&curPlay->beat<mbeat)
			{
				Aevent	*n=curPlay->nextP;
				play(curPlay, gblbeat);
				curPlay=n;
			}
			curPlay=firstE[curpat];
		}
		
		while(curPlay&&curPlay->beat<=beat)
		{
			Aevent	*n=curPlay->nextP;
			play(curPlay, gblbeat);
			curPlay=n;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::play(Aevent *e, double gblbeat)	// todo: add a clear mode comme dans reason !!!
{
	Atrack		*t=e->track;
	
	if(sequencer->table->recording&&(gblbeat-t->lastRecordEventBeat<0.5f))
	{
		if(!(e->flags&Aevent::CREATE))
		{
			if(recmode==recmodeOVERDUB)
			{
				if(e->beat+0.5f>beat)
				{
					t->subEvent(e);
					return;
				}
			}
		}
		else
		{
			e->flags&=~Aevent::CREATE;
			return;
		}
	}

	if(t->control)
	{
		bool	mute=false;
		{
			Atrack	*tt=t;
			while(tt)
			{
				if(tt->isMuted())
					mute=true;
				if(tt->father&&((Atrack *)tt->father)->nSoloChild&&(!tt->isSolo()))
					mute=true;
				tt=(Atrack *)tt->father;
			}
		}
		if(!mute)
			t->control->sequence(e->nctrl, e->value);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::start()
{
	ctrlgblbeat=-1.f;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::stop()
{
	ctrlgblbeat=-1.f;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::set(int npat)
{
	npat=maxi(mini(npat, 31), 0);
	if((npat/8)!=bank->get())
		bank->set(npat/8);
	if((npat&7)!=pat->get())
		pat->set(npat&7);
	int		l;
	section.enter(__FILE__,__LINE__);
	curpat=npat;
	l=patlen[curpat];
	curE=NULL;
	curPlay=firstE[curpat];
	while(curPlay&&(curPlay->beat<beat))
		curPlay=curPlay->nextP;
	section.leave();
	position->setMax(l);
	lenght->set(l);
	preset->set(npreset[curpat]);
	ponoff->setChecked(usepreset[curpat]);
	if(usepreset[curpat])
		sequencer->table->loadPreset(npreset[curpat], true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::seek(double gbeat)
{
	section.enter(__FILE__,__LINE__);
	ctrlgblbeat=-1.f;
	calcBeat(gbeat);
	curPlay=firstE[curpat];
	while(curPlay&&(curPlay->beat<beat))
		curPlay=curPlay->nextP;
	clearTrackRecordBeat(tracks);
	section.leave();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::calcBeat(double gbeat)
{
	this->gblbeat=gbeat;
	beat=gblbeat-(float)(((int)gblbeat/patlen[curpat])*patlen[curpat]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::add(Acontrol *c)	// used only when a module with plugins load a new plugin
{
	ctrlgblbeat=gblbeat;
	section.enter(__FILE__,__LINE__);
	{
		Atrack	*te=(Atrack *)tracks->fchild;
		while(te)
		{
			if(te->effect==c->effect)
			{
				char	name[128];
				Atrack	*tc=te;
				strcpy(name, c->name);
				strupr(name);
				{
					char	*sf=name;
					char	*sl=strstr(sf, "/");
						
					if(sl)
						*sl=0;
						
					while(sl)
					{
						Atrack	*t=(Atrack *)tc->fchild;
						while(t)
						{
							if(!strcmp(t->name, sf))
								break;
							t=(Atrack *)t->next;
						}
						if(!t)
						{
							section.leave();
							return true;
						}
						tc=t;
							
						sf=sl+1;
						sl=strstr(sf, "/");
						if(sl)
							*sl=0;
					}
					
					{
						Atrack	*t=(Atrack *)tc->fchild;
						while(t)
						{
							if(t->cqiid==(c->qiid^c->effect->getQIID()))
								break;
							t=(Atrack *)t->next;
						}
						if(t)
						{
							delref(t);
							t->control=c;
							addref(c, t);
						}
					}
				}
			}
			te=(Atrack *)next;
		}
	}
	section.leave();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::select(Acontrol *c)
{
	section.enter(__FILE__,__LINE__);
	{
		Atrack	*t=(Atrack *)tracks->fchild;
		while(t)
		{
			assert(t->type&Atrack::EFFECT);
			if(t->effect==c->effect)
			{
				Atrack	*tc=(Atrack *)t->fchild;
				while(tc)
				{
					if((tc->type&Atrack::CONTROL)&&(tc->control==c))
					{
						Aitem	*i=treeview->getCurrent();
						treeview->setCurrent(tc);
						prop->setTrack(tc);
						edit->setTrack(tc);
						break;
					}
					if(tc->fchild)
						tc=(Atrack *)tc->fchild;
					else if(tc->next)
						tc=(Atrack *)tc->next;
					else 
					{
						while(tc)
						{
							if(tc->father==t)
							{
								tc=null;
								break;
							}
							if(tc->father->next)
							{
								tc=(Atrack *)tc->father->next;
								break;
							}
							tc=(Atrack *)tc->father;
						}
					}
				}
			}
			t=(Atrack *)t->next;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::select(Aeffect *e)
{
	section.enter(__FILE__,__LINE__);
	{
		Atrack	*t=(Atrack *)tracks->fchild;
		while(t)
		{
			assert(t->type&Atrack::EFFECT);
			if(t->effect==e)
			{
				treeview->setCurrent(t);
				prop->setTrack(t);
				edit->setTrack(t);
				break;
			}
			t=(Atrack *)t->next;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::del(Acontrol *c)
{
	section.enter(__FILE__,__LINE__);
	{
		AseqRef	*r=getref(c);
		while(r)
		{
			AseqRef	*n=r->next;
			if(r->control==c)
			{
				Atrack	*t=r->track;
				delref(t);
				t->control=NULL;
			}
			r=n;
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AseqPattern::add(Aeffect *e)
{
	Atrack	*te;
	char	ename[128];
	section.enter(__FILE__,__LINE__);
	strcpy(ename, e->name);
	strupr(ename);
	{
		Atable	*table=e->capsule->table;
		int		i;
		te=new Atrack(ename, this, e);
		tracks->add(te);
		te->state|=Atrack::stateFOLDER|Atrack::stateCOLLAPSED;
		for(i=0; i<table->nbControl; i++)
		{
			Acontrol	*c=table->control[i];
			if(c->effect==e)
			{
				char	name[128];
				Atrack	*tc=te;
				strcpy(name, c->name);
				strupr(name);
				{
					char	*sf=name;
					char	*sl=strstr(sf, "/");
					
					if(sl)
						*sl=0;
					
					while(sl)
					{
						Atrack	*t=(Atrack *)tc->fchild;
						while(t)
						{
							if(!strcmp(t->name, sf))
								break;
							t=(Atrack *)t->next;
						}
						if(!t)
						{
							t=new Atrack(sf, this, NULL);
							t->state|=Atrack::stateFOLDER|Atrack::stateCOLLAPSED;
							tc->add(t);
						}
						tc=t;
						
						sf=sl+1;
						sl=strstr(sf, "/");
						if(sl)
							*sl=0;
					}
					{
						Atrack	*t=new Atrack(sf, this, ((Aobject *)c->object)->tooltips, c);
						tc->add(t);
						addref(c, t);
					}
				}
			}
		}
	}
	tracks->sort(Aitem::sortRECURSIVE);
	section.leave();
	treeview->refresh();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::del(Aeffect *e)
{
	section.enter(__FILE__,__LINE__);
	{
		Atable	*table=e->capsule->table;
		Atrack	*t=(Atrack *)tracks->fchild;

#ifdef _DEBUG			
		dbgNbControlDeleted=0;
#endif
		
		while(t)
		{
			if(t->effect==e)
			{
				setTrackLink(t, NULL);
				// todo: repaint ??
			}
			t=(Atrack *)t->next;
		}
	}
	section.leave();
	{
		Aitem *i=treeview->getCurrent();
		prop->setTrack((Atrack *)i);
		edit->setTrack((Atrack *)i);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::setTrackRecordState(Atrack *t, bool rec)
{
	section.enter(__FILE__,__LINE__);
	Atrack *tt=(Atrack *)t->fchild;
	
	if(rec)
		t->setMode(t->type|Atrack::RECORD);
	else
		t->setMode(t->type&~Atrack::RECORD);
		
	while(tt)
	{
		setTrackRecordState(tt, rec);
		tt=(Atrack *)tt->next;
	}
	
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::setTrackLink(Atrack *t, Aeffect *e)
{
	section.enter(__FILE__,__LINE__);
	
	switch(t->type&Atrack::maskTYPE)
	{
		case Atrack::EFFECT:
		t->effect=e;
		break;
		
		case Atrack::CONTROL:
		if(e)
		{
			Atable	*table=e->capsule->table;
			int		i;
			for(i=0; i<table->nbControl; i++)
			{
				Acontrol	*c=table->control[i];
				if((c->effect==e)&&((c->qiid^e->getQIID())==t->cqiid))
				{
					delref(t);
					t->control=c;
					addref(c, t);
					break;
				}
			}
		}
		else
		{
			if(t->control)
			{
				delref(t);
				t->control=NULL;
#ifdef _DEBUG
				dbgNbControlDeleted++;
#endif
			}
		}		
		break;

		case Atrack::GROUP:
		break;
	}
		
	Atrack *tt=(Atrack *)t->fchild;
	while(tt)
	{
		setTrackLink(tt, e);
		tt=(Atrack *)tt->next;
	}
	
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::clearTrackRecordBeat(Atrack *t)
{
	Atrack *tt=(Atrack *)t->fchild;
	t->lastRecordEventBeat=-20.f;
	while(tt)
	{
		clearTrackRecordBeat(tt);
		tt=(Atrack *)tt->next;
	}
}	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::addref(Acontrol *c, Atrack *t)
{
	dword	a=(dword)(void *)c;
	dword	v=((a>>24)&0xff)^((a>>16)&0xff)^((a>>8)&0xff)^(a&0xff);
	AseqRef	*r=new AseqRef(c, t);
	r->next=ref[v];
	ref[v]=r;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
EDLL void AseqPattern::delref(Acontrol *c)
{
	dword	a=(dword)(void *)c;
	dword	v=((a>>24)&0xff)^((a>>16)&0xff)^((a>>8)&0xff)^(a&0xff);
	AseqRef	*r=ref[v];
	AseqRef	*o=NULL;
	while(r)
	{
		if(r->control==c)
		{
			if(o)
				o->next=r->next;
			else
				ref[v]=r->next;
			delete(r);
			return;
		}
		o=r;
		r=r->next;
	}
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AseqPattern::delref(Atrack *t)
{
	Acontrol	*c=t->control;
	dword		a=(dword)(void *)c;
	dword		v=((a>>24)&0xff)^((a>>16)&0xff)^((a>>8)&0xff)^(a&0xff);
	AseqRef		*r=ref[v];
	AseqRef		*o=NULL;
	while(r)
	{
		if(r->track==t)
		{
			if(o)
				o->next=r->next;
			else
				ref[v]=r->next;
			delete(r);
			return;
		}
		o=r;
		r=r->next;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AseqRef * AseqPattern::getref(Acontrol *c)
{
	dword	a=(dword)(void *)c;
	dword	v=((a>>24)&0xff)^((a>>16)&0xff)^((a>>8)&0xff)^(a&0xff);
	return ref[v];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Asequencer::Asequencer(char *name, Aobject *l, Atable *t, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	bac=false;
	table=t;
	
	hlast=92;

	pattern=new AseqPattern("pattern", this, 0, 20, pos.w, 72);
	pattern->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Asequencer::~Asequencer()
{
	delete(pattern);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::clear()
{
	pattern->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::clearInit()
{
	pattern->clearInit();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::load(Afile *f)
{
	if(safemagictest(f, magicSEQUENCER))
		return pattern->load(f);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::save(Afile *f)
{
	dword	magic=magicSEQUENCER;
	f->write(&magic, sizeof(magic));
	return pattern->save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCONTEXT:
		break;

		case nyCHANGE:
		return TRUE;
		
		case nySWITCH:
		if(o==table)
		{
			int	ht=table->pos.h-20;
			int	lmy=pos.y;
			int	ny;
			if(pos.y==ht)
				ny=table->pos.h-hlast;
			else
			{
				hlast=pos.h;
				ny=ht;
			}
			if(ny>ht)
				ny=ht;
			else if(ny<20)
				ny=20;
			if(lmy!=ny)
			{
				table->layer->size(pos.w, ny-table->layer->pos.y);
				size(pos.w, ht+20-ny);
				move(0, ny);
				if(ny>lmy)
					table->layer->repaint();
			}
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	dword	lc=0xffe0e0e0;
	b->boxfa(0, 0, pos.w-1,pos.h-1, 0xff605550, 0.5f);
	b->boxf(0, 2, pos.w-1, 18, 0xffb0b0a0);
	b->line(0, 0, pos.w-1, 0, lc);
	b->line(0, 1, pos.w-1, 1, lc);
	b->line(0, 18, pos.w-1, 18, lc);
	b->line(0, 19, pos.w-1, 19, lc);
	f->set(b, 10, 6, "SEQUENCER", 0xff202020);
	b->line(0, 2, 0, pos.h, lc);
	b->line(1, 2, 1, pos.h, lc);
	b->line(pos.w-2, 2, pos.w-2, pos.h, lc);
	b->line(pos.w-1, 2, pos.w-1, pos.h, lc);
	b->line(0, pos.h-1, pos.w-1, pos.h-1, lc);
	b->line(0, pos.h-1, pos.w-1, pos.h-1, lc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::size(int w, int h)
{
	pattern->size(pattern->pos.w, maxi(72, h-(pattern->pos.y)));
	return Aobject::size(w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		wy=pos.y;
		ly=pos.y+y;
		bac=TRUE;
		mouseCapture(TRUE);
		return TRUE;

		case mouseNORMAL:
		if((state&mouseL)&&bac)
		{
			int	lmy=pos.y;
			int	ny=wy+(y+pos.y)-ly;
			int	ht=table->pos.h-20;
			if(ny>ht)
				ny=ht;
			else if(ny<20)
				ny=20;
			if(lmy!=ny)
			{
				table->layer->size(pos.w, ny-table->layer->pos.y);
				size(pos.w, ht+20-ny);
				move(0, ny);
				if(ny>lmy)
					table->layer->repaint();
//				else
//					repaint();
			}
		}
		return TRUE;

		case mouseLUP:
		bac=FALSE;
		mouseCapture(FALSE);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::add(Acontrol *c)
{
	return pattern->add(c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::del(Acontrol *c)
{
	pattern->del(c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::add(Aeffect *e)
{
	return pattern->add(e);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::del(Aeffect *e)
{
	pattern->del(e);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::select(Acontrol *control)
{
	pattern->select(control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::select(Aeffect *effect)
{
	pattern->select(effect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::record(Acontrol *c, int ctrl, float value, double beat)
{
	pattern->record(c, ctrl, value, beat);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::action(double beat)
{
	pattern->play(beat);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::start()
{
	return pattern->start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::stop()
{
	return pattern->stop();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asequencer::seek(double beat)
{
	return pattern->seek(beat);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asequencer::setPattern(int n)
{
	pattern->set(n);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

