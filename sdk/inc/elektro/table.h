/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	table.H					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_TABLE_H_
#define							_TABLE_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<mmsystem.h>
#include						"elektrodef.h"
#include						"thread.h"
#include						"effect.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidTABLE						(Atable::CI.guid)
#define							guidTABLEFRONT					(AtableFront::CI.guid)
#define							guidTABLEBACK					(AtableBack::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MAXEFFECTINFO					(1024)
#define							MAXEFFECT						(64)
#define							MAXCTRLBYEFFECT					(128)
#define							MAXCONTROL						(MAXEFFECT*MAXCTRLBYEFFECT)
#define							MAXPRESET						(128)
#define							MAXPRESETFILE					(2048)

#define							MAXMIDIIN						(64)

#define							DSOUNDSIZE						(44100/4)
#define							DSOUNDSIZEMIN					(256)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	char			*name;
	char			*help;
	int				width;
	int				height;
} AinfoReso;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	midiNoteOff					= 0x80,
	midiNoteOn					= 0x90,
	midiAftertouch				= 0xa0,
	midiControl					= 0xb0,
	midiProgram					= 0xc0,
	midiChannel					= 0xd0,
	midiPitch					= 0xe0,
	midiSystem					= 0xf0
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	clockINT					= 0,
	clockMIDI					= 1,
	clockAUDIOPIN				= 2
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							SEEKLAST						(-1.f)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Atable : public Aobject, public Athread, public Aidle
{
public:
	ELIBOBJ

	enum
	{
		reso_80_60,
		reso_88_72,
		reso_128_96,
		reso_160_120,
		reso_176_144,
		reso_240_176,
		reso_240_180,
		reso_320_240,
		reso_352_240,
		reso_352_288,
		reso_384_288,
		reso_400_300,
		reso_512_384,
		reso_640_240,
		reso_640_288,
		reso_640_480,
		reso_704_576,
		reso_720_240,
		reso_720_288,
		reso_720_480,
		reso_720_576,
		reso_768_576,
		reso_800_600,
		reso_1024_768,
		reso_1280_720,
		reso_1920_1080,
		reso_1200_400,
		reso_MAXIMUM
	};

	EDLL static AinfoReso		infoReso[reso_MAXIMUM];

	enum
	{
								panelFRONT=false,
								panelBACK=true
	};

	enum
	{
								START,
								STOP
	};

	EDLL						Atable							(char *name, Aobject *l, int x, int y, int w, int h);
	EDLL virtual				~Atable							();

	EDLL virtual void			idlePulse						();
	
	EDLL virtual void			paint							(Abitmap *);
	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual bool			size							(int w, int h);
	EDLL virtual void			pulse							();
	
	EDLL static Atable *		getTable						(Aobject *o);
	
	EDLL virtual void			tcpSysex						(char *messsage);

	EDLL virtual bool			keyMapping						(int event, int ascii, int scan, int state);
	EDLL virtual bool			midiMapping						(int channel, int status, int p0, int p1, dword time);
	EDLL virtual bool			midiSysex						(Abuffer *b, dword time);
	
	EDLL virtual bool			savePreset						(int n, bool all);
	EDLL virtual bool			loadPreset						(int n, bool all);
	EDLL virtual void			clearPreset						(int n);
	
	EDLL virtual int			addPresetFile					(char *file);			// file access for preset
	EDLL virtual bool			getPresetFile					(int n, char *file);	// file access for preset

	EDLL virtual bool			add								(class Aplugz *ei, bool view);	// view=false, old module, not displayed in the create effect menu
	EDLL virtual void			sort							();

	EDLL virtual bool			add								(class Apin *p);
	EDLL virtual bool			del								(class Apin *p);

	EDLL virtual bool			add								(class Aconnect *c);
	EDLL virtual bool			del								(class Aconnect *c);
	
	EDLL virtual void			select							(class Acontrol *c);
	EDLL virtual void			select							(class Aeffect *e);
	EDLL virtual void			randomize						(class Aeffect *e);

	EDLL virtual bool			del								(class Aeffect *e);
	
	EDLL bool					deplace							(int my, int ly); // (class Aeffect *e, int my);
	EDLL void					arrange							();
	EDLL void					order							();
	EDLL void					clear							();

	EDLL bool					seek							(double beat);
	EDLL bool					start							();
	EDLL bool					stop							();
	EDLL virtual void			run								();

	EDLL virtual double			getTime							();
	EDLL virtual double			getBeat							();

	EDLL virtual void			settings						(bool emergency);

	EDLL bool					save							(Afile *f);
	EDLL bool					load							(Afile *f);

	EDLL void					command							(int cmd);

	EDLL bool					add								(class Acontrol *c);
	EDLL void					del								(class Acontrol *c);
	EDLL bool					record							(class Acontrol *c, int ctrl, float value);

	bool						memShowMap;
	EDLL void					showMap							(bool bshow);

	EDLL void					memValues						();
	
	EDLL void					computeAudioBPM					();
	// video

	bool						threading;

	float						bpmPhase;
	float						bpmFreq;
	int							videoW;
	int							videoH;
	int							frameRate;
	bool						render;
	bool						highPriority;
	bool						recording;
	
	bool						compressRamVidz;
	float						compressJpegQuality;
	
	int							vlooptime;						// max of video loop (ex: rnbox or sampler) in seconds
	
	dword						clockType;
	float						clockMidiBpm;
	int							clockMidiTick;
	int							clockMidiCount;
	int							clockMidiCountBpm;
	
	float						audioBPM;

	double						instantFPS;
	double						instantFrameTime;
	double						instantFrameRenderTime;
	int							instantWait;
	double						instantWaitReal;


	EDLL void					setBPM							(float bpm);
	EDLL float					getBPM							();
	

	// midi

	qword						midiInDeviceF;
	//int						midiInDevice;
	int							midiOutDevice;
	byte						mohdata[256];
	MIDIHDR						moh;
	HMIDIOUT					midiOut;
	HMIDIIN						midiIn[MAXMIDIIN];

	bool						midiInit						();
	void						midiClose						();

	void						addMidiCtrl						(int nctrl, Acontrol *ctrl);
	void						delMidiCtrl						(int nctrl, Acontrol *ctrl);

	EDLL void					midiSend						(int channel, int status, int p0, int p1);

	void						addKeyCtrl						(int nctrl, Acontrol *ctrl);
	void						delKeyCtrl						(int nctrl, Acontrol *ctrl);

	void						addOscNode						(class AoscNode *n);

	EDLL void					oscMessage						(class AoscStream *osc, class AoscMessage *msg);
	EDLL bool					oscMasterMessage				(class AoscMessage *reply, class AoscMessage *msg, char *path);
	EDLL bool					oscMasterMessage				(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *node, char *path);


	// plugz list

	class Aitem					*plugz;

	// path

	char						rootdir[ALIB_PATH];

	// private

	int							testUp							(class Aeffect *e0, class Aeffect *e1, bool *beffect, int level);
	int							testDown						(class Aeffect *e0, class Aeffect *e1, bool *beffect, int level);

	void						genEffectName					(char *name, class AeffectInfo *ei);


	class AeffectInfo			*effectInfo[MAXEFFECTINFO];
	int							nbEffectInfo;
	class Aeffect				*effect[MAXEFFECT];
	class Acapsule				*capsule[MAXEFFECT];
	int							nbEffect;
	class Aeffect				*effectOrder[MAXEFFECT];
//	int							nbEffectOrder;
	int							nEffectOrder[MAXEFFECT];

	class Acontrol				*control[MAXCONTROL];
	int							nbControl;

	class AmidiCtrl				*midiCtrl[2][16][128];
	Asection					smidiCtrl;
	class AkeyCtrl				*keyCtrl[128];

	class Apin					*pins;
	class Aconnect				*connects;
	
	class Abuffer				*preset[MAXPRESET];
	int							nbPreset;

	bool						panelShowed;

	class Alayer				*layer;
	class AtableFront			*front;
	class AtableBack			*back;
	class AtableSelect			*tselect;
	class Asequencer			*sequencer;
	class Actbar				*ctbar;

	class Amapping				*mapping;
	Asection					mappingSection;
	int							midiFilterType;
	int							midiFilterChannel;
	int							midiProgChangeChannel;
	byte						midiMemoCtrl[17][127];	// 0=all channel 
#ifdef VSTUDIO
	Apipe						*midiPipe;
	bool						midiPipeConnected;
	class AtcpRemote			*remote;
#endif

	Asection					synchronize;
	bool						outrun;
	bool						running;
	double						timeStart;
	double						timeCurrent;
	double						beat;
	double						beatTime;
	
	char						presetFile[MAXPRESETFILE][ALIB_PATH];
	short						presetPos[MAXPRESETFILE][MAXPRESET];
	int							curPreset;
	bool						recalcPresetPos;	// used to align old saved file with presetPos bug
	
	int							lastMouseY;
	bool						tobeRepaint;

	float						dsoundsize;
	
	class AbpmTracker			*bpmTracker;
	int							nseqpass;

	class AoscNode				*oscNodes;

	class AoscNode				*oscBpm;
	class AoscNode				*oscPlay;
	class AoscNode				*oscPreset;
	class AoscNode				*oscSeqPattern;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AtableFront : public Aobject
{
public:
	ELIBOBJ

	EDLL						AtableFront						(char *name, Aobject *l, Atable *t, int x, int y, int w, int h);
	EDLL virtual				~AtableFront					();

	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);

	int							wy,ly;
	bool						bac;

	Atable						*table;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AtableBack : public Aobject
{
public:
	ELIBOBJ

	EDLL						AtableBack						(char *name, Aobject *l, Atable *t, int x, int y, int w, int h);
	EDLL virtual				~AtableBack						();

	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);
	EDLL virtual void			update							(Abitmap *b);

	int							wy,ly;
	bool						bac;

	Atable						*table;

// connect mode
	Apin						*fromPin;
	int							fromX,fromY;
	Apin						*toPin;
	int							toX,toY;

// hilight pin
	Apin						*hiPin;
	class ApinPreview			*hiPinPreview;

	void						setConnects						();
	void						drawConnects					(Abitmap *b);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Actbar : public Aobject
{
public:
	ELIBOBJ

	EDLL						Actbar							(char *name, Aobject *father, Atable *table, int x, int y, int w, int h);
	EDLL virtual				~Actbar							();

	EDLL virtual bool			mouse							(int x, int y, int state, int event);
	EDLL virtual void			pulse							();

	Atable						*table;
	class Aonoff				*onoff[MAXEFFECT];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aonoff : public Aobject
{
public:
	ELIBOBJ

	EDLL						Aonoff							(char *name, Actbar *ctbar, Aeffect *effect, int x, int y, int w, int h);
	EDLL virtual				~Aonoff							();

	EDLL virtual void			paint							(Abitmap *);
	EDLL virtual bool			notify							(Anode *o, int event, dword p=0);
	EDLL virtual bool			mouse							(int x, int y, int state, int event);
	EDLL virtual void			pulse							();

	Abutton						*bon;
	Actbar						*ctbar;
	Aeffect						*effect;
	float						power;
	bool						test;
	int							lastY;
	bool						efsel;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Alayer : public Aobject
{
public:
	ELIBOBJ

	EDLL						Alayer							(char *name, Aobject *father, int x, int y, int w, int h);
	EDLL virtual 				~Alayer							();

	class Aitem					*effectItems;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_TABLE_H
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
