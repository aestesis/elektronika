/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SEQUENCER.H					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_SEQUENCER_H_
#define							_SEQUENCER_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektrodef.h"
#include						"table.h"
#include						"select.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidSEQUENCER					(Asequencer::CI.guid)
#define							MAXMODULE						MAXEFFECT
#define							MAXTRACK						MAXCTRLBYEFFECT
#define							MAXPATTERN						64
#define							DEFPATTERNLEN					4

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aevent
{
public:
	enum
	{
								CREATE=1
	};

	Aevent						*nextP;	// pattern chain list
	Aevent						*prevP;
	Aevent						*nextT;	// track chain list
	Aevent						*prevT;
	
	class Atrack				*track;
	
	double						beat;
	byte						flags;
	byte						nctrl;
	float						value;
	int							pattern;

								Aevent(class Atrack *t, int pattern, double beat, int nctrl, float value);
	virtual						~Aevent();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Atrack : public Aitem
{
public:
	ELIBOBJ
	
	enum
	{
								NONE=0,
								PTRACK=1,
								EFFECT=2,
								GROUP=3,
								CONTROL=4,
								maskTYPE=PTRACK|EFFECT|GROUP|CONTROL,
								NORMAL=0,
								MUTE=16,
								SOLO=32,
								RECORD=64,
								maskMODE=MUTE|SOLO|RECORD
	};
	
	int							type;
	
	union
	{
		Anode					*link;
		Acontrol				*control;
		Aeffect					*effect;
	};
	
	union
	{
		qword					cqiid;		// absolute qiid of the control
		qword					eguid;		// guid of the effect
	};
	
	int							controlType;
	dword						colors[10];	// colors by subtrack
	char						cname[10][256];
	
	class Aevent				*firstE[MAXPATTERN];
	class Aevent				*lastE[MAXPATTERN];
	class Aevent				*curE;
	int							curpat;
	class AseqPattern			*spat;
	
	double						lastRecordEventBeat;
	int							nSoloChild;
	
	EDLL						Atrack							(char *name, class AseqPattern *spat, Anode *link);
	EDLL						Atrack							(char *name, class AseqPattern *spat, char *help, Anode *link);
	EDLL virtual				~Atrack							();

	EDLL bool					save							(Afile *f);
	EDLL bool					load							(Afile *f);
	static Atrack *				sload							(Afile *f);
	
	EDLL Aevent *				addEvent						(int curpat, int ctrl, float value, double beat, double gblbeat, bool skipsame);
	EDLL void					delEvent						(int curpat, int nctrl, double frombeat, double tobeat);	// only this nctrl
	EDLL void					delEvent						(int curpat, double frombeat, double tobeat);				// all nctrl
	EDLL void					subEvent						(Aevent *e);
	EDLL void					setMode							(int m);

	bool						isRecording						();
	bool						isMuted							();
	bool						isSolo							();

private:
	EDLL void					init							();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool Atrack::isRecording()
{
	return (type&RECORD)?true:false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool Atrack::isMuted()
{
	return (type&MUTE)?true:false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool Atrack::isSolo()
{
	return (type&SOLO)?true:false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqEdit : public Aobject
{
public:
	ELIBOBJ
	
	enum
	{
		toolNONE=AtoolBar::NONE,
		toolMAGNIFY,
		toolHAND,
		toolPEN,
		toolERASE,
		toolSMOOTH,
		toolCOPY,
		toolPASTE
	};

	EDLL						AseqEdit						(char *name, class AseqTrackEdit *edit, int x, int y, int w, int h);
	EDLL virtual				~AseqEdit						();

	EDLL void					setTrack						(class Atrack *track);
	
	EDLL virtual void			pulse							();
	EDLL virtual void			paint							(Abitmap *b);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);

	EDLL virtual void			tools							(int itool);
								
	EDLL void					paintBack						(Abitmap *bitmap);
	EDLL void					paintFront						(Abitmap *bitmap);

	class AseqTrackEdit			*edit;
	class Atrack				*track;
	int							itool;
	int							m_test;
	int							m_ivar;
	float						lastp;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqControl : public AseqEdit
{
public:
	ELIBOBJ
	
	EDLL						AseqControl						(char *name, class AseqTrackEdit *edit, int x, int y, int w, int h);
	EDLL virtual				~AseqControl					();
								
	EDLL virtual void			paint							(Abitmap *b);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);

	EDLL virtual void			tools							(int itool);
	
	int							nctrl;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqNote : public AseqEdit
{
public:
	ELIBOBJ
	
	EDLL						AseqNote						(char *name, class AseqTrackEdit *edit, int x, int y, int w, int h);
	EDLL virtual				~AseqNote						();
								
	EDLL virtual void			paint							(Abitmap *b);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);

	EDLL virtual void			tools							(int itool);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqTrackProp : public Aobject
{
public:
	ELIBOBJ
	
	EDLL						AseqTrackProp					(char *name, class AseqPattern *spat, int x, int y, int w, int h);
	EDLL virtual				~AseqTrackProp					();

	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual void			paint							(Abitmap *b);
	
	EDLL virtual void			setTrack						(Atrack *track);
	
	class Abutton				*mute;
	class Abutton				*solo;
	class Abutton				*record;
	class Aedit					*edit;
	class Abutton				*refresh;
	class Abutton				*del;
	class AseqPattern			*spat;
	class Alist					*leffect;
	class Aitem					*ieffect;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqTrackEdit : public Aobject
{
public:
	ELIBOBJ
	
	EDLL						AseqTrackEdit					(char *name, class AseqPattern *spat, int x, int y, int w, int h);
	EDLL virtual				~AseqTrackEdit					();

	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual void			paint							(Abitmap *b);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);
	
	EDLL virtual bool			size							(int w, int h);

	EDLL virtual void			setTrack						(Atrack *track);

	class AseqPattern			*spat;
	class AseqControl			*econtrol;
	class AseqNote				*enote;
	class AseqEdit				*enothing;
	class AseqEdit				*ecurrent;
	class AtoolBar				*toolbar;
	Alist						*lctrl;
	Aitem						*ictrl;
	Alist						*lres;
	Aitem						*ires;
	float						res;

	class AscrollBar			*sbar;
	float						wview;
	float						xview;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AseqPattern : public Aobject
{
public:
	ELIBOBJ
	
	enum
	{
								recmodeOVERDUB,
								recmodeREPLACE
	};
	
	EDLL						AseqPattern						(char *name, class Asequencer *seq, int x, int y, int w, int h);
	EDLL virtual				~AseqPattern					();

	EDLL virtual void			paint							(Abitmap *);
	EDLL virtual bool			size							(int w, int h);
	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual void			pulse							();
	
	EDLL bool					save							(Afile *f);
	EDLL bool					load							(Afile *f);

	EDLL void					clear							();
	EDLL void					clearInit						();

	EDLL void					record							(class Acontrol *control, int ctrl, float value, double beat);
	
	EDLL void					select							(class Acontrol *control);
	EDLL void					select							(class Aeffect *effect);

	EDLL void					addref							(Acontrol *c, Atrack *t);
	EDLL void					delref							(Atrack *t);
	EDLL class AseqRef *		getref							(Acontrol *c);		

	EDLL bool					add								(Aeffect *e);
	EDLL void					del								(Aeffect *e);

	EDLL bool					add								(Acontrol *c);
	EDLL void					del								(Acontrol *c);

	EDLL void					setTrackRecordState				(Atrack *t, bool rec);
	EDLL void					setTrackLink					(Atrack *t, Aeffect *e);
	EDLL void					clearTrackRecordBeat			(Atrack *t);
	EDLL void					play							(double gblbeat);
	EDLL void					play							(Aevent *e, double gblbeat);

	EDLL bool					start							();
	EDLL bool					stop							();
	EDLL bool					seek							(double gblbeat);
	
	EDLL void					calcBeat						(double gblbeat);

	EDLL void					set								(int pattern);
	
	Asection					section;
								
	Asequencer					*sequencer;
	class AtreeView				*treeview;
	Aselect						*bank;
	Aselect						*pat;
	Asegment					*position;
	Asegment					*position2;
	Asegment					*lenght;
	Abutton						*up1;
	Abutton						*down1;
	Abutton						*up10;
	Abutton						*down10;
	int							dlenght;
	Abutton						*editmode;
	Asegment					*preset;
	Abutton						*ponoff;
	class AseqTrackProp			*prop;
	class AseqTrackEdit			*edit;
	
	int							npreset[MAXPATTERN];
	bool						usepreset[MAXPATTERN];

	int							patlen[MAXPATTERN];
	int							curpat;

	class Atrack				*tracks;
	class AseqRef				*ref[256];
	
	Aevent						*firstE[MAXPATTERN];
	Aevent						*lastE[MAXPATTERN];
	Aevent						*curE;
	Aevent						*curPlay;
	double						gblbeat;
	double						beat;
	int							recmode;
	double						ctrlgblbeat;			// used to ignore new control first set value
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Asequencer : public Aobject
{
public:
	ELIBOBJ

	EDLL						Asequencer						(char *name, Aobject *l, class Atable *t, int x, int y, int w, int h);
	EDLL virtual				~Asequencer						();

	EDLL virtual void			paint							(Abitmap *);
	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);
	EDLL virtual bool			size							(int w, int h);

	EDLL bool					save							(Afile *f);
	EDLL bool					load							(Afile *f);
	
	EDLL void					clear							();
	EDLL void					clearInit						();

	EDLL bool					action							(double beat);
	EDLL void					record							(class Acontrol *control, int ctrl, float value, double beat);
	
	EDLL void					select							(Acontrol *control);
	EDLL void					select							(Aeffect *effect);
	
	EDLL bool					start							();
	EDLL bool					stop							();
	EDLL bool					seek							(double beat);
	
	EDLL bool					add								(Aeffect *e);
	EDLL void					del								(Aeffect *e);

	EDLL bool					add								(Acontrol *c);
	EDLL void					del								(Acontrol *c);

	EDLL void					setPattern						(int n);

	int							wy,ly;
	bool						bac;
	int							hlast;

	Asection					section;
	class Atable				*table;

	AseqPattern					*pattern;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_SEQUENCER_H
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
