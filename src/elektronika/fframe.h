/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FFRAME.H					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_FFRAME_H_
#define							_FFRAME_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<interface.h>
#include						"../elektro/elektro.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidFFRAME						(Afframe::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Afframe : public Aeffect
{
	AOBJ

								Afframe							(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule);
	virtual						~Afframe						();

	virtual bool				save							(Afile *f);
	virtual bool				load							(Afile *f);

	virtual bool				savePreset						(Afile *f);
	virtual bool				loadPreset						(Afile *f);

	virtual int					priority						()	{ return priorityFILTER; }
	virtual void				settings						(bool emergency);
	
	//virtual void				actionStart						(double time);
	virtual void				action							(double time, double dtime, double beat, double dbeat);
	//virtual void				actionStop						();

	byte						*imagedata;
	void						*ffeffect;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AfframeFront : public AeffectFront
{
	AOBJ

								AfframeFront					(QIID qiid, char *name, Afframe *e, int h);
	virtual						~AfframeFront					();

	bool						save							(Afile *f);
	bool						load							(Afile *f);

	bool						notify							(Anode *o, int event, dword p);
	void						paint							(Abitmap *b);

	void						pulse							();
	
	void						refresh							();
	
	Asection					section;
	Abitmap						*back;
	Alist						*feffect;
	Aitem						*ifeffect;
	Apaddle						*pad[16];
	Astatic						*sta[16];
	ActrlButton					*bypass;
	ActrlButton					*rst;
	int							npad;
	int							neffect;
	bool						binit;
	float						def[16];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AfframeBack : public AeffectBack
{
	AOBJ

								AfframeBack						(QIID qiid, char *name, Afframe *e, int h);
	virtual						~AfframeBack					();

	void						paint							(Abitmap *b);

	Abitmap						*back;

	Avideo						*in;
	Avideo						*video;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AfframeInfo : public AeffectInfo
{
	AOBJ 

								AfframeInfo						(char *name, ACI *eci, char *ename, char *ehelp);
	virtual int					getEffectClass					() { return EFFECT2D; }
	virtual dword				version							() { return 0x00010500; }

	virtual Aeffect *			create							(QIID qiid, char *name, Acapsule *capsule);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline AfframeInfo::AfframeInfo(char *name, ACI *eci, char *ename, char *ehelp) : AeffectInfo(name, eci, ename, ehelp)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz *					fframeGetInfo					();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_FFRAME_H
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
