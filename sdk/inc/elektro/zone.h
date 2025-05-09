/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ZONE.H						(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektrodef.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_ZONE_H_
#define							_ZONE_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"acontrol.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidZONE						(Azone::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Azone : public AcontrolObj
{
public:
	ELIBOBJ
	
	EDLL 						Azone							(QIID qiid, char *name, Aobject *father, int x, int y, int w, int h, float wsel, float hsel);
	EDLL virtual				~Azone							();

	EDLL virtual bool			mouse							(int x, int y, int state, int event);
	EDLL virtual void			paint							(Abitmap *b);
	EDLL virtual void			pulse							();
	EDLL virtual bool			keyboard						(int event, int ascii, int scan, int state);

	EDLL virtual bool			sequence						(int nctrl, float value);

	EDLL void					get								(float *x, float *y) { *x=xx; *y=yy; };
	EDLL void					set								(float x, float y);

private:
	float						xx,yy;
	float						wsel,hsel;

	bool						ctrlPaint;

	int							ow,oh;
	int							x0,y0;
	float						xx0,yy0;
	bool						test;
	bool						testSHIFT;
	
	float						defxx,defyy;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_ZONE_H_
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
