/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EXPORT.H					(c)	YoY'05						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_EXPORT_H_
#define							_EXPORT_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<interface.h>
#include						"../elektro/elektro.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidEXPORT						(Aexport::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aexport : public Aeffect
{
	AOBJ

								Aexport							(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule);
	virtual						~Aexport						();

	virtual bool				save							(Afile *f);
	virtual bool				load							(Afile *f);

	virtual bool				savePreset						(Afile *f);
	virtual bool				loadPreset						(Afile *f);

	virtual int					priority						()	{ return priorityEXPORT; }
	virtual void				settings						(bool emergency);
	
	//virtual bool				midiSysex						(Abuffer *b, dword time);

	virtual void				actionStart						(double time);
	virtual void				actionStop						();
	virtual void				action							(double time, double dtime, double beat, double dbeat);

	void						nextFile						();


	Asection					section;
	char						filename[ALIB_PATH];
	Abitmap						*image;
	AaviWrite					*avi;
	float						vleft;
	float						vright;
	double						m_time;
	int							maxsize;
	double						starttime;
	class Asharedmem			*sharedmem;
	Abitmap						*bitshare;
	Abitmap						*notregistred;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AexportFront : public AeffectFront
{
	AOBJ

								AexportFront					(QIID qiid, char *name, Aexport *e, int h);
	virtual						~AexportFront					();

	bool						save							(Afile *f);
	bool						load							(Afile *f);

	bool						notify							(Anode *o, int event, dword p);
	void						paint							(Abitmap *b);

	void						pulse							();

	void						updateFilename					();
	
	Abitmap						*back;

	Adisplay					*left;
	Adisplay					*right;
	float						m_vleft;
	float						m_vright;
	Abutton						*record;
	Abutton						*open;
	Abutton						*camera;
	Abutton						*compress;
	Abutton						*flip;
	Asegment					*fps;
	Astatic						*filename;
	char						ttime[128];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AexportBack : public AeffectBack
{
	AOBJ

								AexportBack						(QIID qiid, char *name, Aexport *e, int h);
	virtual						~AexportBack					();

	void						paint							(Abitmap *b);

	Abitmap						*back;

	Avideo						*video;
	Asample						*audio;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AexportInfo : public AeffectInfo
{
	AOBJ 

								AexportInfo						(char *name, ACI *eci, char *ename, char *ehelp);
	virtual int					getEffectClass					() { return INOUT; }
	virtual dword				version							() { return 0x00020002; }

	virtual Aeffect *			create							(QIID qiid, char *name, Acapsule *capsule);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline AexportInfo::AexportInfo(char *name, ACI *eci, char *ename, char *ehelp) : AeffectInfo(name, eci, ename, ehelp)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz *					exportGetInfo						();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_EXPORT_H
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
