/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONTROL.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<assert.h>
#include						<math.h>

#include						"table.h"
#include						"acontrol.h"
#include						"effect.h"
#include						"elektro.h"
#include						"sequencer.h"
#include						"tcpRemote.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Acontrol::CI=ACI("Acontrol", GUID(0xE4EC7600,0x00010040), &Anode::CI, 0, NULL);
EDLL ACI						Amapping::CI=ACI("Amapping", GUID(0xE4EC7600,0x00010041), &Aobject::CI, 0, NULL);
EDLL ACI						AcontrolObj::CI=ACI("AcontrolObj", GUID(0xE4EC7600,0x00010042), &Aobject::CI, 0, NULL);
EDLL ACI						AcontrolObjMap::CI=ACI("AcontrolObjMap", GUID(0xE4EC7600,0x00010043), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct midiBits
{
	union
	{
		struct
		{
			unsigned short	ubits:14;
			unsigned short	upad:2;
		};
		struct
		{
			signed short	sbits:14;
			signed short	spad:2;
		};
	};
} midiBits;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char	*keyt[]=
{
	"NOP",
	"ESC",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"0",
	"-",
	"=",
	"BACKSPACE",
	"TAB",
	"Q",
	"W",
	"E",
	"R",
	"T",
	"Y",
	"U",
	"I",
	"O",
	"P",
	"[",
	"]",
	"RETURN",
	"CTRL",
	"A",
	"S",
	"D",
	"F",
	"G",
	"H",
	"J",
	"K",
	"L",
	";",
	"\'",
	"\'",
	"LSHIFT",
	"\\",
	"Z",
	"X",
	"C",
	"V",
	"B",
	"N",
	"M",
	",",
	".",
	"/",
	"RSHIFT",
	"*",
	"ALT",
	"SPACE",
	"CLOCK",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"NLOCK",
	"SLOCK",
	"HOME",
	"UP",
	"PGUP",
	"F52",
	"LEFT",
	"F54",
	"RIGHT",
	"F56",
	"F57",
	"DOWN",
	"PGDOWN",
	"F60",
	"DEL",
	"F60",
	"F58",
	"F53",
	"F11",
	"F12",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"NOP",
	"F53",
	"NOP",
	"NOP",
	"NOP",
	"F51",
	"NOP",
	"NOP",
	"RALT",
	"CTRL",
	"SLOCK",
	"/",
	"NOP",
	"SLOCK"
	"F50"
	"DEL",
	"END",
	"INS",
	"NOP",
	"F55",
	"F59",
	"F49"
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL char *notes[]=
{
								"C",
								"C#",
								"D",
								"D#",
								"E",
								"F",
								"F#",
								"G",
								"G#",
								"A",
								"A#",
								"B"
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							COLORINST				0xffffffff

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Acontrol::Acontrol(QIID qiid, char *name, class Atable *table, class AcontrolObj *o, int type) : Anode(name)
{
	this->qiid=qiid;
	this->type=type;
	oscType=oscNOTDEFINED;
	object=o;
	effect=null;
	this->table=table;
	table->add(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Acontrol::Acontrol(QIID qiid, char *name, class Aeffect *ef, class AcontrolObj *o, int type) : Anode(name)
{
	this->qiid=qiid;
	assert(type!=KEY_01);
	this->type=type;
	oscType=oscNOTDEFINED;
	object=o;
	effect=ef;
	table=ef->getTable();
	table->add(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Acontrol::~Acontrol()
{
	int	nb=1+(KEY_01?0:(type-CONTROLER_01));
	int	i;

	for(i=0; i<nb; i++)
	{
		table->delMidiCtrl(i, this);
		table->delKeyCtrl(i, this);
	}

	table->del(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::save(Afile *f)
{
	int	nb=(type>=KEY_01)?(1):(1+type-CONTROLER_01);
	int	i;
	
	f->writeString(object->name);
	if(effect)
		f->writeString(effect->name);
	else
		f->writeString("common");

	smidimap.enter(__FILE__,__LINE__);
	for(i=0; i<nb; i++)
	{
		f->write(&midimap[i].status, sizeof(midimap[i].status));
		if(midimap[i].status!=-1)
		{
			f->write(&midimap[i].channel, sizeof(midimap[i].channel));
			f->write(&midimap[i].data, sizeof(midimap[i].data));
			{
				dword magic=magicMODECTRL;
				f->write(&magic, sizeof(magic));
				f->write(&midimap[i].modeCtrl, sizeof(midimap[i].modeCtrl));
			}
		}
	}
	for(i=0; i<nb; i++)
	{
		f->write(&keymap[i].kmode, sizeof(keymap[i].kmode));
		if(keymap[i].kmode)
			f->write(&keymap[i].key, sizeof(keymap[i].key));
	}
	smidimap.leave();

	for(i=0; i<nb; i++)
		f->write(&memValue[i], sizeof(memValue[i]));

	{
		dword magic=magicMINMAXCTRL;
		f->write(&magic, sizeof(magic));
		for(i=0; i<nb; i++)
		{
			f->write(&midimap[i].minCtrl, sizeof(midimap[i].minCtrl));
			f->write(&midimap[i].maxCtrl, sizeof(midimap[i].maxCtrl));
		}
	}
		
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::load(Afile *f)
{
	char	oname[256];
	char	ename[256];
	int		nb=(type>=KEY_01)?(1):(1+type-CONTROLER_01);
	int		i;
	
	f->readString(oname);
	f->readString(ename);
	
	smidimap.enter(__FILE__,__LINE__);
	for(i=0; i<nb; i++)
	{
		f->read(&midimap[i].status, sizeof(midimap[i].status));
		if(midimap[i].status!=-1)
		{
			f->read(&midimap[i].channel, sizeof(midimap[i].channel));
			f->read(&midimap[i].data, sizeof(midimap[i].data));
			if(safemagictest(f, magicMODECTRL))
				f->read(&midimap[i].modeCtrl, sizeof(midimap[i].modeCtrl));
			table->addMidiCtrl(i, this);
		}
	}
	for(i=0; i<nb; i++)
	{
		f->read(&keymap[i].kmode, sizeof(keymap[i].kmode));
		if(keymap[i].kmode)
		{
			f->read(&keymap[i].key, sizeof(keymap[i].key));
			table->addKeyCtrl(i, this);
		}
	}
	smidimap.leave();

	for(i=0; i<nb; i++)
		f->read(&memValue[i], sizeof(memValue[i]));
		
	if(safemagictest(f, magicMINMAXCTRL))
	{
		for(i=0; i<nb; i++)
		{
			f->read(&midimap[i].minCtrl, sizeof(midimap[i].minCtrl));
			f->read(&midimap[i].maxCtrl, sizeof(midimap[i].maxCtrl));
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::saveValues(Afile *f)
{
	int		nb=(type>=KEY)?1:(1+type-CONTROLER_01);
	int		i;
	for(i=0; i<nb; i++)
	{
		if(f->write(&memValue[i], sizeof(memValue[i]))!=sizeof(memValue[i]))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::loadValues(Afile *f)
{
	int		nb=(type>=KEY)?1:(1+type-CONTROLER_01);
	int		i;
	for(i=0; i<nb; i++)
	{
		if(f->read(&memValue[i], sizeof(memValue[i]))!=sizeof(memValue[i]))
			return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Acontrol::setInfo(int control, char *name, dword color)
{
	smidimap.enter(__FILE__,__LINE__);
	if(control>=KEY_01)
	{
		strcpy(midimap[0].name, name);
		strupr(midimap[0].name);
		midimap[0].color=color;
	}
	else
	{
		strcpy(midimap[control-CONTROLER_01].name, name);
		strupr(midimap[control-CONTROLER_01].name);
		midimap[control-CONTROLER_01].color=color;
	}
	smidimap.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::isMapped()
{
	bool mapped=false;
	smidimap.enter(__FILE__,__LINE__);
	{
		if(type>=KEY)
		{
			Amidimap	*mm=&midimap[0];
			if(mm->status!=-1)
				mapped=true;
		}
		else
		{
			int	i;
			for(i=CONTROLER_01; i<type+1; i++)
			{
				Amidimap	*mm=&midimap[i-CONTROLER_01];
				if(mm->status!=-1)
					mapped=true;
			}
		}
	}
	if(type>=Acontrol::KEY_01)
	{
		if(keymap[0].kmode)
			mapped=true;
	}
	else
	{
		int	i;
		for(i=CONTROLER_01; i<type+1; i++)
		{
			if(keymap[i-CONTROLER_01].kmode)
				mapped=true;
		}
	}
	smidimap.leave();
	return mapped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Acontrol::selectMap(AcontrolObjMap *map)
{
	object->selectMap(map);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL float Acontrol::get(int nctrl)
{
	if(this->type>=KEY)
		return memValue[0];
	else
		return memValue[nctrl-CONTROLER_01];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::set(int nctrl, float value)
{
	assert(nctrl<=this->type);
	
	midiSend(nctrl, value);
	
	if(table->running&&table->recording)
		return table->record(this, nctrl, value);
	{
		if(this->type>=KEY)
			memValue[0]=value;
		else
			memValue[nctrl-CONTROLER_01]=value;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::keyseq(int nctrl, int kmode, int nkey, int event)
{
	bool	r=false;
	smidimap.enter(__FILE__,__LINE__);
	switch(kmode)
	{
		case keymodePRESS:
		assert(nkey==0);
		switch(event)
		{
			case keyboardDOWN:
			set(nctrl+CONTROLER_01, 127);
			r=object->sequence(nctrl+CONTROLER_01, 1.f);
			break;

			case keyboardUP:
			set(nctrl+CONTROLER_01, 0);
			r=object->sequence(nctrl+CONTROLER_01, 0.f);
			break;
		}
		break;

		case keymodeSWITCH:
		assert(nkey==0);
		{
			if(memValue[nctrl]>0.f)
			{
				set(nctrl+CONTROLER_01, 0.f);
				r=object->sequence(nctrl+CONTROLER_01, 0.f);
			}
			else
			{
				set(nctrl+CONTROLER_01, 1.f);
				r=object->sequence(nctrl+CONTROLER_01, 1.f);
			}
		}
		break;

		case keymodeSELECT:
		assert(nctrl==0);
		set(KEY, (float)nkey);
		r=object->sequence(KEY, (float)nkey);
		break;

		case keymodeCYCLE:
		assert(nctrl==0);
		assert(nkey==0);
		{
			int		max=1+this->type-KEY_01;
			float	v=(float)(((int)memValue[0]+1)%max);
			set(KEY, v);
			r=object->sequence(KEY, v);
		}
		break;
	}
	smidimap.leave();
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::sequence(int nctrl, float value)
{
	object->fromSequencer=true;
	
	if(this->type>=KEY)
		memValue[0]=value;
	else
		memValue[nctrl-Acontrol::CONTROLER_01]=value;
		
	midiSend(nctrl, value);
	
	return object->sequence(nctrl, value);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::midiseq(int nctrl, int status, int midi0, int midi1)
{
	bool r=false;
	smidimap.enter(__FILE__,__LINE__);
	if(this->type>=KEY)
	{
		Amidimap	*mm=&midimap[0];
		assert(nctrl==0);
		switch(status)
		{
			case midiNoteOn:
			{
				int	no=midi0-mm->data;
				if(midi1)
				{
					set(KEY, (float)no);
					r=object->sequence(KEY, (float)no);
				}
			}
			break;
			
			case midiControl:
			{
				int	nb=1+this->type-KEY_01;
				int	no=(midi1*nb)/128;
				set(KEY, (float)no);
				r=object->sequence(KEY, (float)no);
			}
			break;

			default:
			assert(false);
			break;
		}
	}
	else
	{
		switch(status)
		{
			case midiNoteOn:
			{
				float	v=(float)midi1/127.f;
				set(nctrl+CONTROLER_01, v);
				r=object->sequence(nctrl+CONTROLER_01, v);
			}
			break;
			
			case midiControl:
			{
				Amidimap	*mm=&midimap[nctrl];
				float		v=midiCtrl2Value(memValue[nctrl], mm->modeCtrl, table->midiMemoCtrl[mm->channel][mm->data], table->midiMemoCtrl[mm->channel][(mm->data+32)&127], mm->minCtrl, mm->maxCtrl);
				set(nctrl+CONTROLER_01, v);
				r=object->sequence(nctrl+CONTROLER_01, v);
			}
			break;
		}
	}
	smidimap.leave();
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Acontrol::oscMessage(AoscMessage *reply, AoscMessage *query, int action)	// return true if using reply
{
	if(object->implementOsc())
		return object->oscMessage(reply, query, action);

	switch(action)
	{
		case oscDOCUMENTATION:
		if(object->tooltips)
			reply->add(new AoscString(object->tooltips));
		else if(object->name)
			reply->add(new AoscString(object->name));
		else
			reply->add(new AoscString(""));
		return true;

		case oscTYPESIGNATURE:
		if(this->type>=KEY)
		{
			reply->add(new AoscString(",i"));
		}
		else
		{
			static char base[]=",ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
			int			nb=1+type-CONTROLER_01;
			char		real[128];
			strncpy(real, base, nb+1);
			real[nb+1]=0;
			reply->add(new AoscString(real));
		}
		return true;

		case oscUISIGNATURE:
		switch(oscType)
		{
			case oscPRESS:
			reply->add(new AoscString("press"));
			return true;

			case osc2STATES:
			reply->add(new AoscString("2states"));
			return true;

			default:
			reply->add(new AoscString(""));
			return true;
		}
		break;

		case oscSETVALUE:
		if(this->type>=KEY)
		{
			if(query->fchild&&query->fchild->isCI(&AoscInteger::CI))
			{
				int v=((AoscInteger *)query->fchild)->value;
				set(KEY, (float)v);
				object->sequence(KEY, (float)v);
			}
		}
		else
		{
			Anode	*n=query->fchild;
			int		nb=1+type-CONTROLER_01;
			int		i=0;
			while((i<nb)&&n)
			{
				if(n->isCI(&AoscFloat::CI))
				{
					set(CONTROLER_01+i, ((AoscFloat *)n)->value);
					object->sequence(CONTROLER_01+i, ((AoscFloat *)n)->value);
				}
				n=n->next;
				i++;
			}
		}
		return false;	// return false, cause not using reply

		case oscGETVALUE:
		if(this->type>=KEY)
			reply->add(new AoscInteger((int)get(KEY)));
		else
		{
			int		nb=1+type-CONTROLER_01;
			int		i;
			for(i=0; i<nb; i++)
				reply->add(new AoscFloat(get(CONTROLER_01+i)));
		}
		return true;

		case oscGETMINVALUE:
		if(this->type>=KEY)
			reply->add(new AoscInteger(0));
		else
		{
			int		nb=1+type-CONTROLER_01;
			int		i;
			for(i=0; i<nb; i++)
				reply->add(new AoscFloat(0.f));
		}
		return true;

		case oscGETMAXVALUE:
		if(this->type>=KEY)
			reply->add(new AoscInteger(type-KEY_01));
		else
		{
			int		nb=1+type-CONTROLER_01;
			int		i;
			for(i=0; i<nb; i++)
				reply->add(new AoscFloat(1.f));
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Acontrol::set2memValue()
{
	if(this->type>=KEY)
	{
		midiSend(KEY, memValue[0]);
		object->sequence(KEY, memValue[0]);
	}
	else
	{
		int	i;
		int	nb=1+this->type-CONTROLER_01;
		for(i=0; i<nb; i++)
		{
			midiSend(i+CONTROLER_01, memValue[i]);
			object->sequence(i+CONTROLER_01, memValue[i]);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Acontrol::midiSend(int nctrl, float value)
{
	smidimap.enter(__FILE__,__LINE__);
	{
		if(this->type>=KEY)
		{
			Amidimap	*mm=&midimap[0];
			assert(nctrl==KEY);
			switch(mm->status)
			{
				case midiNoteOn:
				table->midiSend(mm->channel, mm->status, mm->data+(int)value, 127);
				break;

				case midiControl:
				table->midiSend(mm->channel, mm->status, mm->data, (int)(value*127.f/(float)(type-KEY_01)));
				break;
			}
		}
		else
		{
			assert(nctrl<KEY);
			assert(nctrl>=Acontrol::CONTROLER_01);
			{
				Amidimap	*mm=&midimap[nctrl-CONTROLER_01];
				double		d=mm->maxCtrl-mm->minCtrl;
				switch(mm->modeCtrl)
				{
					case Amidimap::modeABS:
					case Amidimap::modeREL1:
					case Amidimap::modeREL2:
					case Amidimap::modeREL3:
					{
						double		v=((double)value-(double)mm->minCtrl)*127.0/d;
						int			vi=(int)maxi(mini(v, 127.0), 0.0);
						table->midiSend(mm->channel, mm->status, mm->data, vi);
					}
					break;
					
					case Amidimap::modeABS_14:
					case Amidimap::modeREL1_14:
					case Amidimap::modeREL2_14:
					case Amidimap::modeREL3_14:
					{
						double		v=((double)value-(double)mm->minCtrl)*16383.0/d;
						int			vi=(int)maxi(mini(v, 16383.0), 0.0);
						table->midiSend(mm->channel, mm->status, mm->data, (vi>>7)&127);
						table->midiSend(mm->channel, mm->status, (mm->data+32)&127, vi&127);
					}
					break;
				}
			}
		}
	}
	smidimap.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL float Acontrol::midiCtrl2Value(float value, int mode, int msb, int lsb, float minRange, float maxRange)
{
	double		d=(double)maxRange-(double)minRange;
	midiBits	mb;
	mb.ubits=(msb<<7)|lsb;
	
	switch(mode)
	{
		case Amidimap::modeABS:
		value=(float)((double)minRange+d*(double)msb/127.0);
		break;
		
		case Amidimap::modeREL1:
		if(msb<64)
			value+=(float)(d*(double)(msb)/127.0);
		else
			value+=(float)(d*(double)(msb-128)/127.0);
		break;
		
		case Amidimap::modeREL2:
		if(msb>64)
			value+=(float)(d*(double)(msb-64)/127.0);
		else
			value+=(float)(d*(double)(msb-65)/127.0);
		break;
		
		case Amidimap::modeREL3:
		if(msb<64)
			value+=(float)(d*(double)(msb)/127.0);
		else
			value+=(float)(d*(double)(64-msb)/127.0);
		break;
		
		case Amidimap::modeABS_14:
		value=(float)((double)minRange+d*((double)mb.ubits)/16383.0);
		break;
		
		case Amidimap::modeREL1_14:
		value+=(float)(d*((double)mb.sbits)/16383.0);
		break;
		
		case Amidimap::modeREL2_14:
		value+=(float)(d*((double)(mb.ubits-8192))/16383.0);
		break;
		
		case Amidimap::modeREL3_14:
		if(mb.ubits<8192)
			value+=(float)(d*((double)(mb.ubits))/16383.0);
		else
			value+=(float)(d*((double)(8192-mb.ubits))/16383.0);
		break;
	}
	{
		float	min=maxi(mini(minRange, maxRange), 0.f);
		float	max=mini(maxi(minRange, maxRange), 1.f);
		value=maxi(mini(value, max), min);
	}
	return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Acontrol::randomize()
{
	if(this->type>=KEY_01)
	{
		int	n=this->type-KEY_01;
		if(n>0)
			object->sequence(KEY, (float)(rand()%n));
	}
	else
	{
		int	i;
		int	nb=1+this->type-CONTROLER_01;
		for(i=0; i<nb; i++)
			object->sequence(i+CONTROLER_01, (float)(rand()&255)/255.f);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Acontrol::select()
{
	if(effect)
		effect->capsule->select(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aselkey : public Aobject
{
public:
	ELIBOBJ

							Aselkey								(char *name, Aobject *father, int x, int y, int w, int h);
	virtual					~Aselkey							();

	virtual void			paint								(Abitmap *b);
	virtual bool			notify								(Anode *o, int event, dword p=0);
	virtual bool			mouse								(int x, int y, int state, int event);
	virtual bool			keyboard							(int event, int ascii, int scan, int state);
	virtual void			pulse								();

	int						get									()		{ return key; }
	void					set									(int k) { key=k&127; repaint(); }

	dword					key;
	bool					disp;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI							Aselkey::CI=ACI("Aselkey", GUID(0xE4EC7600,0x00010047), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aselkey::Aselkey(char *name, Aobject *father, int x, int y, int w, int h) : Aobject(name, father, x, y, w, h)
{
	key=0;
	disp=true;
	timer(150);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aselkey::~Aselkey()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aselkey::pulse()
{
    if((state&stateFOCUS)!=0)
    {
        disp=!disp;
        repaint();
    }
}

bool Aselkey::keyboard(int event, int ascii, int scan, int state)
{
	switch(event)
	{
        case keyboardLOSTFOCUS:
		disp=true;
        repaint();
        break;

		case keyboardDOWN:
		if(scan==1)
		{
			key=0;
			father->notify(this, nyCHANGE, 0);
			repaint();
			return true;
		}
		else if((scan!=57)&&(scan!=28))
		{
			key=scan&127;
			father->notify(this, nyCHANGE, key);
			repaint();
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aselkey::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontTERMINAL06);
	b->boxf(0, 0, pos.w, pos.h, 0xffc0c0c0);
	if(disp)
	{
		if(key)
			f->set(b, 2, 2, keyt[key], 0xff000000);
		else
			f->set(b, 2, 2, "NONE", 0xff000000);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aselkey::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aselkey::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseNORMAL:
		cursor(cursorTEXT);
		break;

		case mouseLEAVE:
		cursor();
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::del(Acontrol *c)
{
	if(control&&control==c)
		setControl(null);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::del(Aeffect *e)
{
	if(control&&control->effect==e)
		setControl(null);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::setTable(Atable *table)
{
	this->table=table;
	setControl(null);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::setControl(Acontrol *c)
{
	control=c;

	fstatus=table->midiFilterType;
	fchannel=table->midiFilterChannel;
	fltChannel->setCurrentByData(table->midiFilterChannel);
	fltType->setCurrentByData(table->midiFilterType);

	if(c)
	{
		itemControls->clear();
		if(control->type>=Acontrol::KEY_01)
		{
			char	str[128];
			sprintf(str, "key %d states", 1+control->type-Acontrol::KEY_01);
			new Aitem(str, str, itemControls, 0, this);
		}
		else
		{
			int		i;
			int		nb=(control->type-Acontrol::CONTROLER_01)+1;
			char	str[128];
			for(i=0; i<nb; i++)
			{
				sprintf(str, "controller %d", i+1);
				new Aitem(str, str, itemControls, i, this);
			}
		}
		controls->setCurrentByData(0);

		keyGet();
		keySet();
		midiControlGet();
		midiSet();

		types->show(true);
	}
	else
	{
		int i;
		itemControls->clear();
		for(i=0; i<CTRLMAXKEY; i++)
			key[i]->show(false);
		channels->show(false);
		notes->show(false);
		numctrl->show(false);
		minRange->show(false);
		maxRange->show(false);
		modectrl->show(false);
		ctrlValue->show(false);
		types->show(false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Amapping::Amapping(char *name, Aobject *father, int x, int y, int w, int h) : Aobject(name, father, x, y, w, h)
{
	int xspy=pos.w-98;

	table=null;
	control=null;
	setTooltips("mapping dialog");

	fstatus=0;
	fchannel=0;

	active=true;
	
	{	
		int	i;
		for(i=0; i<NBINSTANT; i++)
			minstant[i].status=-1;
	}

	font=alib.getFont(fontTERMINAL06);

	bactive=new Abutton("bactive", this, xspy, 82, 86, 14, "MIDI SPY", Abutton::btCAPTION|Abutton::bt2STATES);
	bactive->setTooltips("midi spy (learn mmode)");
	bactive->setChecked(true);
	bactive->show(TRUE);

	itemChannel=new Aitem("channels", "channels", NULL, 0);
	new Aitem("all", "all midi channels", itemChannel, 0, this);
	new Aitem("1", "midi channel 1", itemChannel, 1, this);
	new Aitem("2", "midi channel 2", itemChannel, 2, this);
	new Aitem("3", "midi channel 3", itemChannel, 3, this);
	new Aitem("4", "midi channel 4", itemChannel, 4, this);
	new Aitem("5", "midi channel 5", itemChannel, 5, this);
	new Aitem("6", "midi channel 6", itemChannel, 6, this);
	new Aitem("7", "midi channel 7", itemChannel, 7, this);
	new Aitem("8", "midi channel 8", itemChannel, 8, this);
	new Aitem("9", "midi channel 9", itemChannel, 9, this);
	new Aitem("10", "midi channel 10", itemChannel, 10, this);
	new Aitem("11", "midi channel 11", itemChannel, 11, this);
	new Aitem("12", "midi channel 12", itemChannel, 12, this);
	new Aitem("13", "midi channel 13", itemChannel, 13, this);
	new Aitem("14", "midi channel 14", itemChannel, 14, this);
	new Aitem("15", "midi channel 15", itemChannel, 15, this);
	new Aitem("16", "midi channel 16", itemChannel, 16, this);
	
	fltChannel=new Alist("fltChannel", this, xspy+8, 110, 70, 14, itemChannel);
	fltChannel->setTooltips("channels filter");
	fltChannel->show(TRUE);

	itemType=new Aitem("types", "typess", NULL, 0);
	new Aitem("all", "all midi types", itemType, 0, this);
	new Aitem("note on", "note on", itemType, 1, this);
	new Aitem("control change", "control change", itemType, 2, this);

	fltType=new Alist("fltType", this, xspy+8, 130, 70, 14, itemType);
	fltType->setTooltips("events filter");
	fltType->show(TRUE);

	{
		int	i;
		for(i=0; i<NBINSTANT; i++)
		{
			binstant[i]=new Abutton("binstant[]", this, xspy, 160+i*14, 86, 12, "", Abutton::btCAPTION|Abutton::btALIGNLEFT);
			binstant[i]->colorBACKNORMAL=0xffc0c0c0;
			binstant[i]->colorFORENORMAL=0xff202020;
			binstant[i]->setTooltips("midi events");
			binstant[i]->show(TRUE);
		}
	}

	itemControls=new Aitem("controls", "controls", NULL, 0);

	controls=new Alist("controls", this, 10, 50, 100, 14, itemControls);
	controls->setCurrentByData(0);
	controls->setTooltips("controls list");
	controls->show(TRUE);

	ctrlName=new Astatic("control name", this, 130, 50, 100, 14);
	//ctrlName->set("control name", 0xffffffff, font, Astatic::LEFT);
	ctrlName->setTooltips("control name");
	ctrlName->show(true);

	itemTypes=new Aitem("types", "types", NULL, 0);
	new Aitem("none", "no midi mapping", itemTypes, 0, this);
	new Aitem("note on", "midi note/on", itemTypes, 1, this);
	new Aitem("controller", "midi controller", itemTypes, 2, this);

	types=new Alist("midi types", this, 10, 90, 100, 14, itemTypes);
	types->setCurrentByData(0);
	types->setTooltips("midi mapping type");
	types->show(TRUE);

	itemChannels=new Aitem("types", "types", NULL, 0);
	new Aitem("1", "midi channel #1", itemChannels, 1, this);
	new Aitem("2", "midi channel #2", itemChannels, 2, this);
	new Aitem("3", "midi channel #3", itemChannels, 3, this);
	new Aitem("4", "midi channel #4", itemChannels, 4, this);
	new Aitem("5", "midi channel #5", itemChannels, 5, this);
	new Aitem("6", "midi channel #6", itemChannels, 6, this);
	new Aitem("7", "midi channel #7", itemChannels, 7, this);
	new Aitem("8", "midi channel #8", itemChannels, 8, this);
	new Aitem("9", "midi channel #9", itemChannels, 9, this);
	new Aitem("10", "midi channel #10", itemChannels, 10, this);
	new Aitem("11", "midi channel #11", itemChannels, 11, this);
	new Aitem("12", "midi channel #12", itemChannels, 12, this);
	new Aitem("13", "midi channel #13", itemChannels, 13, this);
	new Aitem("14", "midi channel #14", itemChannels, 14, this);
	new Aitem("15", "midi channel #15", itemChannels, 15, this);
	new Aitem("16", "midi channel #16", itemChannels, 16, this);

	channels=new Alist("midi channel", this, 10, 110, 100, 14, itemChannels);
	channels->setCurrentByData(1);
	channels->setTooltips("midi channel");

	itemNotes=new Aitem("notes", "notes", NULL, 0);
	{
		int		i;
		char	str[128];
		char	str2[128];
		for(i=0; i<127; i++)
		{
			strcpy(str, ::notes[i%12]);
			strcat(str, " ");
			itoa((i/12)-1, str+strlen(str), 10);
			strcpy(str2, "midi note ");
			strcat(str2, str);
			new Aitem(str, str2, itemNotes, i, this);
		}
	}

	notes=new Alist("midi note", this, 10, 130, 100, 14, itemNotes);
	notes->setCurrentByData(0);
	notes->setTooltips("midi note");

	itemNumctrl=new Aitem("control number", "control number", NULL, 0);
	{
		int		i;
		char	str[128];
		char	str2[128];
		for(i=0; i<127; i++)
		{
			sprintf(str, "control #%d", i);
			sprintf(str2, "midi control #%d", i);
			new Aitem(str, str2, itemNumctrl, i, this);
		}
	}

	numctrl=new Alist("midi control", this, 10, 130, 100, 14, itemNumctrl);
	numctrl->setCurrentByData(0);
	numctrl->setTooltips("midi control number");

	{
		char	str[256];
		int		i;
		for(i=0; i<CTRLMAXKEY; i++)
		{
			int	y=i&3;
			int	x=i>>2;
			key[i]=new Aselkey("key", this, 10+x*59, 340+y*20, 49, 12);
			sprintf(str, "key #%d", i+1);
			key[i]->setTooltips(str);
			//key[i]->show(true);
		}
	}
	
	imodectrl=new Aitem("control mode", "control mode", NULL, 0);
	new Aitem("absolute", "absolute (7 bits)", imodectrl, Amidimap::modeABS, this);
	new Aitem("relative 1", "relative 1 (7 bits)", imodectrl, Amidimap::modeREL1, this);
	new Aitem("relative 2", "relative 2 (7 bits)", imodectrl, Amidimap::modeREL2, this);
	new Aitem("relative 3", "relative 3 (7 bits)", imodectrl, Amidimap::modeREL3, this);
	new Aitem("absolute (14)", "absolute (14 bits)", imodectrl, Amidimap::modeABS_14, this);
	new Aitem("relative 1 (14)", "relative 1 (14 bits)", imodectrl, Amidimap::modeREL1_14, this);
	new Aitem("relative 2 (14)", "relative 2 (14 bits)", imodectrl, Amidimap::modeREL2_14, this);
	new Aitem("relative 3 (14)", "relative 3 (14 bits)", imodectrl, Amidimap::modeREL3_14, this);

	modectrl=new Alist("modectrl", this, 10, 170, 100, 14, imodectrl);
	modectrl->setCurrentByData(Amidimap::modeABS);
	modectrl->setTooltips("control mode");

	iminRange=new Aitem("control range", "control range", NULL, 0);
	{
		int		i;
		char	str[128];
		for(i=0; i<21; i++)
		{
			float	v=(float)i/20.f;
			sprintf(str, "min %1.2f", v);
			new Aitem(str, str, iminRange, i, this);
		}
	}

	imaxRange=new Aitem("control range", "control range", NULL, 0);
	{
		int		i;
		char	str[128];
		for(i=0; i<21; i++)
		{
			float	v=(float)i/20.f;
			sprintf(str, "max %1.2f", v);
			new Aitem(str, str, imaxRange, i, this);
		}
	}


	minRange=new Alist("minRange", this, 10, 210, 100, 14, iminRange);
	minRange->setCurrentByData(0);
	minRange->setTooltips("min control range");

	maxRange=new Alist("minRange", this, 10, 230, 100, 14, imaxRange);
	maxRange->setCurrentByData(20);
	maxRange->setTooltips("max control range");

	ctrlValue=new Astatic("ctrlValue", this, 10, 270, 100, 14),
	ctrlValue->set("0.000000", 0xffffffff, font, Astatic::LEFT);
	ctrlValue->setTooltips("control value");

	ikeykey=new Aitem("ikeykey", "key mode", NULL, 0);
	new Aitem("select", "select key mode", ikeykey, 0, this);
	new Aitem("cycle", "cycle key mode", ikeykey, 1, this);

	keykey=new Alist("keykey", this, 10, 320, 100, 14, ikeykey);
	keykey->setCurrentByData(0);
	keykey->setTooltips("key mode");

	ikeyctrl=new Aitem("ikeyctrl", "controller mode", NULL, 0);
	new Aitem("press", "press key mode", ikeyctrl, 0, this);
	new Aitem("switch", "switch key mode", ikeyctrl, 1, this);

	keyctrl=new Alist("keyctrl", this, 10, 320, 100, 14, ikeyctrl);
	keyctrl->setCurrentByData(0);
	keyctrl->setTooltips("key mode");

	timer(50);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Amapping::~Amapping()
{
	int	i;
	for(i=0; i<CTRLMAXKEY; i++)
		delete(key[i]);
	delete(keykey);
	delete(ikeykey);
	delete(controls);
	delete(ctrlName);
	delete(itemControls);
	delete(types);
	delete(itemTypes);
	delete(channels);
	delete(itemChannels);
	delete(notes);
	delete(itemNotes);
	delete(numctrl);
	delete(itemNumctrl);
	delete(bactive);
	for(i=0; i<NBINSTANT; i++)
		delete(binstant[i]);
	delete(fltType);
	delete(itemType);
	delete(fltChannel);
	delete(itemChannel);
	delete(minRange);
	delete(maxRange);
	delete(iminRange);
	delete(imaxRange);
	delete(modectrl);
	delete(imodectrl);
	delete(ctrlValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amapping::mouse(int x, int y, int state, int event)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amapping::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		{
			int	i;
			for(i=0; i<NBINSTANT; i++)
			{
				if(binstant[i]==o)
				{
					sinstant.enter(__FILE__,__LINE__);
					switch(minstant[i].status)
					{
						case midiNoteOn:
						types->setCurrentByData(1);
						channels->setCurrentByData(minstant[i].channel);
						notes->setCurrentByData(minstant[i].data0);
						break;

						case midiControl:
						if((types->getCurrentData()==2)&&(modectrl->getCurrentData()>=Amidimap::modeABS_14))
							numctrl->setCurrentByData(minstant[i].data0);
						else
							numctrl->setCurrentByData(minstant[i].data0);
						types->setCurrentByData(2);
						channels->setCurrentByData(minstant[i].channel);
						break;

						default:
						types->setCurrentByData(0);
						break;
					}
					sinstant.leave();
					midiSet();
					midiControlSet();
					return true;
				}
			}
		}
		break;

		case nyCHANGE:
		if(o==fltChannel)
		{
			sinstant.enter(__FILE__,__LINE__);
			Aitem *i=fltChannel->getCurrent();
			fchannel=i->data;
			table->midiFilterChannel=i->data;
			sinstant.leave();
			return true;
		}
		else if(o==fltType)
		{
			sinstant.enter(__FILE__,__LINE__);
			Aitem *i=fltType->getCurrent();
			fstatus=i->data;
			table->midiFilterType=i->data;
			sinstant.leave();
			return true;
		}
		else if(o==bactive)
		{
			sinstant.enter(__FILE__,__LINE__);
			active=bactive->isChecked();
			sinstant.leave();
			return true;
		}
		else if(o==controls)
		{
			keyGet();
			keySet();
			midiControlGet();
			midiSet();
			return true;
		}
		else if(o==types)
		{
			midiSet();
			midiControlSet();
			return true;
		}
		else if(o==channels)
		{
			midiControlSet();
			return true;
		}
		else if(o==notes)
		{
			midiControlSet();
			return true;
		}
		else if(o==numctrl)
		{
			midiControlSet();
			return true;
		}
		else if((o==minRange)||(o==maxRange)||(o==modectrl))
		{
			midiControlSet();
			return true;
		}
		else if(o==keykey)
		{	
			keySet();
			keySetSet();
			return true;
		}
		else if(o==keyctrl)
		{
			keySet();
			keySetSet();
			return true;
		}
		else
		{
			int	i;
			for(i=0; i<CTRLMAXKEY; i++)
			{
				if(o==key[i])
				{
					keySetSet();
					return true;
				}
			}
		}
		break;
	}
	return Aobject::notify(o, event , p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amapping::paint(Abitmap *b)
{
	Afont	*f=alib.getFont(fontCONFIDENTIAL14);
	Afont	*f0=alib.getFont(fontTERMINAL06);
	b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.5f);
	/*
	b->box(0, 0, pos.w-1, pos.h-1, 0xff000000);
	b->box(1, 1, pos.w-2, pos.h-2, 0xff808080);
	*/
	b->boxfa(pos.w-101, 80, pos.w-10, 300, 0xff808080, 0.5f);
	b->boxfa(8, 80, pos.w-120, 300, 0xff808080, 0.5f);
	f->set(b, 8, 8, "midi/keyboard mapping", 0xffc0c0c0);
	if(control)
	{
		char	str[1024];
		if(control->effect)
			strcpy(str, control->effect->name);
		else
			strcpy(str, "common");
		strcat(str, " / ");
		strcat(str, control->name);
		strupr(str);
		f0->set(b, 8, 28, str, 0xffffffff);
	}
	b->boxfa(8, 310, pos.w-10, 420, 0xff808080, 0.5f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amapping::pulse()
{
	if(control)
	{
		int	st=types->getCurrentData();
		if(st==2)
		{
			int		nctrl=controls->getCurrentData();
			char	str[256];
			sprintf(str, "%7.6f", control->memValue[nctrl]);
			ctrlValue->set(str, 0xffffffff, font, Astatic::LEFT);
		}
	}
	if(refreshInstant)
	{
		sinstant.enter(__FILE__,__LINE__);
		refreshInstant=false;
		{
			int	i;
			for(i=0; i<NBINSTANT; i++)
			{
				char		str[256];
				AmidiInfo	*it=&minstant[i];
				str[0]=0;
				switch(it->status)
				{
					case midiNoteOff:
					sprintf(str, "%2d-NF-%s", it->channel, ::notes[it->data0%12]);
					break;
					case midiNoteOn:
					sprintf(str, "%2d-NO-%s/%3d", it->channel, ::notes[it->data0%12], it->data1);
					break;
					case midiAftertouch:
					sprintf(str, "%2d-AT-", it->channel);
					break;
					case midiControl:
					sprintf(str, "%2d-CC-%3d/%3d", it->channel, it->data0, it->data1);
					break;
					case midiProgram:
					sprintf(str, "%2d-PC-", it->channel);
					break;
					case midiChannel:
					sprintf(str, "%2d-CH-", it->channel);
					break;
					case midiPitch:
					sprintf(str, "%2d-PB-%3d/%3d", it->data0, it->data1);
					break;
					case midiSystem:
					assert(false);
					break;
				}
				binstant[i]->setCaption(str);
			}
		}
		sinstant.leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::keySet()
{
	if(control)
	{
		Aitem	*it=controls->getCurrent();
		int		c=it->data;
		int		max=1;

		if(control->type>=Acontrol::KEY_01)
		{
			switch(keykey->getCurrentData())
			{
				case 0:
				max=mini(CTRLMAXKEY, 1+control->type-Acontrol::KEY_01);
				break;
			}
		}

		control->smidimap.enter(__FILE__,__LINE__);
		{
			int	i;
			for(i=0; i<max; i++)
			{
				key[i]->show(true);
				key[i]->set(control->keymap[c].key[i]);
			}
			for(i=max; i<CTRLMAXKEY; i++)
			{
				key[i]->show(false);
				key[i]->set(0);
			}
		}
		control->smidimap.leave();

		repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::keyGet()
{
	if(control)
	{
		Aitem		*it=controls->getCurrent();
		int			c=it->data;
		keyctrl->show(false);
		keykey->show(false);
		if(control->type>=Acontrol::KEY_01)
		{
			control->smidimap.enter(__FILE__,__LINE__);
			switch(control->keymap[c].kmode)
			{
				case keymodeNOTHING:
				case keymodeSELECT:
				keykey->setCurrentByData(0);
				break;

				case keymodeCYCLE:
				keykey->setCurrentByData(1);
				break;

				default:
				assert(false);
				break;
			}
			control->smidimap.leave();
			keykey->show(true);
		}
		else
		{
			control->smidimap.enter(__FILE__,__LINE__);
			switch(control->keymap[c].kmode)
			{
				case keymodeNOTHING:
				case keymodePRESS:
				keyctrl->setCurrentByData(0);
				break;

				case keymodeSWITCH:
				keyctrl->setCurrentByData(1);
				break;

				default:
				assert(false);
				break;
			}
			control->smidimap.leave();
			keyctrl->show(true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Amapping::keySetSet()
{
	if(control)
	{
		Aitem		*it=controls->getCurrent();
		int			c=it->data;

		if(control)
		{
			table->delKeyCtrl(c, control);
			control->smidimap.enter(__FILE__,__LINE__);
			{
				int	kmode=0;
				int	max=1;
				if(control->type>=Acontrol::KEY_01)
				{
					switch(keykey->getCurrentData())
					{
						case 0:
						max=1+control->type-Acontrol::KEY_01;
						kmode=keymodeSELECT;
						break;

						case 1:
						kmode=keymodeCYCLE;
						break;
					}
				}
				else
				{
					switch(keyctrl->getCurrentData())
					{
						case 0:
						kmode=keymodePRESS;
						break;

						case 1:
						kmode=keymodeSWITCH;
						break;
					}
				}
				control->keymap[c].kmode=kmode;
				{
					int	i;
					for(i=0; i<max; i++)
					{
						int	scan=key[i]->get();
						if(scan)
							control->keymap[c].key[i]=scan;
					}
				}
			}
			control->smidimap.leave();
			control->table->addKeyCtrl(c, control);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amapping::midiSet()
{
	Aitem *i=types->getCurrent();
	switch(i->data)
	{
		case 0:	// none
		channels->show(false);
		notes->show(false);
		numctrl->show(false);
		minRange->show(false);
		maxRange->show(false);
		modectrl->show(false);
		ctrlValue->show(false);
		break;

		case 1:	// note on
		channels->show(true);
		notes->show(true);
		numctrl->show(false);
		minRange->show(false);
		maxRange->show(false);
		modectrl->show(false);
		ctrlValue->show(false);
		break;

		case 2:	// controller
		channels->show(true);
		notes->show(false);
		numctrl->show(true);
		minRange->show(true);
		maxRange->show(true);
		modectrl->show(true);
		ctrlValue->show(true);
		break;
	}
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amapping::midiControlGet()
{
	if(control)
	{
		control->smidimap.enter(__FILE__,__LINE__);
		{
			Aitem		*it=controls->getCurrent();
			int			c=it->data;
			control->smidimap.enter(__FILE__,__LINE__);
			{
				Amidimap	*mm=&control->midimap[c];
				ctrlName->set(mm->name, 0xffffffff, font, Astatic::LEFT);
				switch(mm->status)
				{
					case midiNoteOn:
					types->setCurrentByData(1);
					channels->setCurrentByData(mm->channel);
					notes->setCurrentByData(mm->data);
					break;

					case midiControl:
					types->setCurrentByData(2);
					channels->setCurrentByData(mm->channel);
					numctrl->setCurrentByData(mm->data);
					minRange->setCurrentByData((int)((float)mm->minCtrl*20.f));
					maxRange->setCurrentByData((int)((float)mm->maxCtrl*20.f));
					modectrl->setCurrentByData(mm->modeCtrl);
					ctrlValue->set("0.000000", 0xffffffff, font, Astatic::LEFT);
					break;

					default:
					types->setCurrentByData(0);
					//channels->setCurrentByData(1);
					//notes->setCurrentByData(0);
					//numctrl->setCurrentByData(1);
					break;
				}
			}
			control->smidimap.leave();
		}
		control->smidimap.leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amapping::midiControlSet()
{
	if(control)
	{
		control->smidimap.enter(__FILE__,__LINE__);
		{
			int			c=controls->getCurrentData();
			table->delMidiCtrl(c, control);
			{
				Amidimap	*mm=&control->midimap[c];
				int			st=types->getCurrentData();
				int			ch=channels->getCurrentData();
				int			no=notes->getCurrentData();
				int			nc=numctrl->getCurrentData();
				int			minc=minRange->getCurrentData();
				int			maxc=maxRange->getCurrentData();
				int			mode=modectrl->getCurrentData();
				switch(st)
				{
					case 1:
					mm->status=midiNoteOn;
					mm->channel=ch;
					mm->data=no;
					break;

					case 2:
					mm->status=midiControl;
					mm->channel=ch;
					mm->data=nc;
					mm->minCtrl=(float)minc/20.f;
					mm->maxCtrl=(float)maxc/20.f;
					mm->modeCtrl=mode;
					{
						int	nctrl=c+((control->type>=Acontrol::KEY)?Acontrol::KEY_01:Acontrol::CONTROLER_01);
						control->midiSend(nctrl, control->memValue[c]);
					}
					break;

					default:
					mm->status=-1;
					mm->channel=1;
					mm->data=0;
					break;
				}
			}
			table->addMidiCtrl(c, control);
		}
		control->smidimap.leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Amapping::midiMapping(int channel, int status, int p0, int p1, dword time)
{
	if(status!=midiSystem)
	{
		sinstant.enter(__FILE__,__LINE__);
		if(!active)
		{
			sinstant.leave();
			return true;
		}
		if((fchannel!=0)&&(fchannel!=channel))
		{
			sinstant.leave();
			return true;
		}
		switch(status)
		{
			case midiNoteOn:
			if(fstatus==2)
			{
				sinstant.leave();
				return true;
			}
			break;

			case midiControl:
			if(fstatus==1)
			{
				sinstant.leave();
				return true;
			}
			break;

			default:
			sinstant.leave();
			return true;
		}
		
		{
			int	i;
			for(i=NBINSTANT-1; i>=1; i--)
				minstant[i]=minstant[i-1];
		}
		
		minstant[0].channel=channel;
		minstant[0].status=status;
		minstant[0].data0=p0;
		minstant[0].data1=p1;
		
		refreshInstant=true;
		
		sinstant.leave();
	}
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

EDLL AcontrolObj::AcontrolObj(char *name, Aobject *father, int x, int y, int w, int h) : Aobject(name, father, x, y, w, h)
{
	fromSequencer=false;	
	control=null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AcontrolObj::update(Abitmap *b)
{
	Aobject::update(b);	
	if(fromSequencer)
	{
		b->box(0, 0, pos.w-1, pos.h-1, 0xff00ff00);
		fromSequencer=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AcontrolObj::size(int w, int h)
{
	bool			b=Aobject::size(w, h);
	AcontrolObjMap	*map=(AcontrolObjMap *)getChild(&AcontrolObjMap::CI);
	if(map)
			map->size(w, h);
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AcontrolObj::selectMap(AcontrolObjMap *map)
{
	AcontrolObjMap *cur=(AcontrolObjMap *)getChild(&AcontrolObjMap::CI);
	if(cur)
	{
		bool b=cur->selected;
		cur->selected=(cur==map);
		if(b!=cur->selected)
			repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AcontrolObj::showMap(bool show)
{
	if(show)
	{
		if(!getChild(&AcontrolObjMap::CI))
		{
			AcontrolObjMap *map=new AcontrolObjMap("map", this);
			map->show(true);
		}
	}
	else
	{
		AcontrolObjMap *map=(AcontrolObjMap *)getChild(&AcontrolObjMap::CI);
		if(map)
			map->destroy();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AcontrolObjMap::AcontrolObjMap(char *name, class AcontrolObj *ctrlobj) : Aobject(name, ctrlobj, 0, 0, ctrlobj->pos.w, ctrlobj->pos.h)
{
	this->zorder(zorderTOP);
	this->ctrlobj=ctrlobj;
	selected=false;
	npulse=0;
	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AcontrolObjMap::~AcontrolObjMap()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AcontrolObjMap::pulse()
{
	npulse=(npulse+1)&31;
	if(selected)
		repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AcontrolObjMap::paint(Abitmap *b)
{
	dword c0=0xff004000;
	dword c1=0xff00ff00;
	if(ctrlobj->control&&ctrlobj->control->isMapped())
	{
		c0=0xffffffff;
		c1=0xff00ff00;
	}
	if(selected)
	{
		int n0=(int)((abs(sin(2.0*PI*(double)npulse/32.0)))*255.99);
		int n=255-n0;
		c0=0xffff0000;
		c1=0xff000000|(n<<16)|(n0<<8);
	}
	b->linea(0, 0, pos.w, 0, c0, 0.8f);
	b->linea(0, pos.h-1, pos.w, pos.h-1, c0, 0.8f);
	b->linea(0, 1, 0, pos.h-2, c0, 0.8f);
	b->linea(pos.w-1, 1, pos.w-1, pos.h-2, c0, 0.8f);
	b->boxfa(1, 1, pos.w-2, pos.h-2, c1, 0.5f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool AcontrolObjMap::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		if(ctrlobj->control)
			ctrlobj->control->table->notify(this, nySELECT, 0);
		case mouseMOVE:
		case mouseLUP:
		return true;
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
