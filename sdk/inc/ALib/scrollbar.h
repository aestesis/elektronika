/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SCROLLBAR.H					(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_SCROLLBAR_H_
#define							_SCROLLBAR_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"types.h"
#include						"surface.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidSCROLLBAR					(AscrollBar::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! a gui scrollBar
class AscrollBar : public Aobject
{
public:
	ALIBOBJ
	
	enum
	{
								HORIZONTAL,
								VERTICAL
	};

	ADLL						AscrollBar						(char *name, Aobject *L, int x, int y, int w, int h, int type);
	ADLL virtual				~AscrollBar						();

	ADLL virtual void			paint							(Abitmap *b);
	ADLL virtual bool			mouse							(int x, int y, int state, int event);
	
	ADLL virtual void			setPosition						(float pos);
	ADLL virtual void			setView							(float view);
	
	virtual float				getPosition						()				{ return ipos; }
	virtual float				getView							()				{ return view; }

	dword						colorBACK;
	dword						colorFORE;
	dword						colorBORDER;

private:	

	ADLL virtual void			calcMouse						(int x, int y);
	
	int							type;
	float						ipos;
	float						view;
	
	bool						m_test;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_SCROLLBAR_H_
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
