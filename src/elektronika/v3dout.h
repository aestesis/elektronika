/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	V3DOUT.H					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_V3DOUT_H_
#define							_V3DOUT_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<interface.h>
#include						"../elektro/elektro.h"
#include						"ef3d-pixel.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guid3DVOUT						(Av3dout::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							NBEF3D							4

enum
{
								orderINPUT,
								orderEFFECT
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3dout : public Aeffect , public Athread
{
	AOBJ

								Av3dout							(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule);
	virtual						~Av3dout						();

	virtual bool				oscMessage						(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *onode, int action);

	bool						notify							(Anode *o, int event, dword p);

	virtual void				settings						(bool emergency);
	bool						change							(int c);
	
	virtual void				run								();
	
	virtual void				action							(double time, double dtime, double beat, double dbeat);

	virtual bool				save							(Afile *f);
	virtual bool				load							(Afile *f);

	virtual bool				savePreset						(Afile *f);
	virtual bool				loadPreset						(Afile *f);

	virtual int					priority						()	{ return priorityEXPORT; }
	
	int							initWin							();
	void						closeWin						();
	
	void						setEf3List						(int k, Aeffect3dInfo *ef3i);
	void						setEi3List						(int k, Ainput3dInfo *ei3i);
	
	void						init3D							(int k, bool input);
	void						release3D						(int k, bool input);

	void						initPix							();
	void						releasePix						();
	
	Aef3dSound					*sound;
	bool						ef3run;
	class Ad3d					*ad3d;
	Asection					section;
	Asection					winChange;
	HWND						hwnd;
	class Awindow				*win;
	bool						winrun;
	bool						runout;
	int							current;
	int							error;
	int							winX;
	int							winY;
	class AdataV3d				*dt;
	int							renderSTATUS;
	bool						escape;
	bool						OKI_registred;
	class Awindow				*preview;
	Abitmap						*imgMask;
	bool						newMask;
	Asection					sMask;
	Apoint						place[4];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3doutFront : public AeffectFront
{
	AOBJ

								Av3doutFront					(QIID qiid, char *name, Av3dout *e, int h);
	virtual						~Av3doutFront					();

	bool						notify							(Anode *o, int event, dword p);
	void						paint							(Abitmap *b);

	void						pulse							();
	
	void						moveEffect						(int n, int p);

	Abitmap						*back;

	Aselcolor					*shadecolor;
	ActrlButton					*shadeboost;

	Apaddle						*vol;
	Aselcolor					*color;
	Aitem						*items;
	Alist						*list;
	
	int							order[NBEF3D];

	Abutton						*up[NBEF3D];
	Abutton						*down[NBEF3D];
	
	Aitem						*eii;
	Alist						*eil[NBEF3D];
	Ainput3d					*ei3[NBEF3D];
	
	Aitem						*efi;
	Alist						*efl[NBEF3D];
	Apaddle						*efv[NBEF3D];
	Aeffect3d					*ef3[NBEF3D];
	Aselect						*efb[NBEF3D];
	Aselcolor					*efc[NBEF3D];
	ActrlButton					*efbo[NBEF3D];
	Abutton						*mask;
	class Av3dMask				*winMask;
	Abutton						*bPreview;
	ActrlButton					*feedback;
	Apaddle						*feedzoom;
	ActrlButton					*pixb;
	Aitem						*pixi;
	Alist						*pixl;
	Apaddle						*pad[PIX_MAXINPUT];
	Astatic						*sta[PIX_MAXINPUT];
	Apaddle						*master;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3doutBack : public AeffectBack
{
	AOBJ

								Av3doutBack						(QIID qiid, char *name, Av3dout *e, int h);
	virtual						~Av3doutBack					();

	void						paint							(Abitmap *b);

	Abitmap						*back;
	Avideo						*in[effect3dNBVIDEO];
	Asample						*ins;
	Avideo						*out;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3doutInfo : public AeffectInfo
{
	AOBJ 

								Av3doutInfo						(char *name, ACI *eci, char *ename, char *ehelp);
	virtual int					getEffectClass					() { return EFFECT3D; }
	virtual dword				version							() { return 0x00020072; }

	virtual Aeffect *			create							(QIID qiid, char *name, Acapsule *capsule);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline Av3doutInfo::Av3doutInfo(char *name, ACI *eci, char *ename, char *ehelp) : AeffectInfo(name, eci, ename, ehelp)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz *					v3doutGetInfo						();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_V3DOUT_H
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