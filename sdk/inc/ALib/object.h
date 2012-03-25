/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	OBJECT.H					(c)	YoY'99						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_OBJECT_H_
#define							_OBJECT_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include						<windows.h>
#include						"node.h"
#include						"rectangle.h"
#include						"bitmap.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// system message

enum
{
								systemDEVICECHANGE
};

// mouse events

enum
{
								mouseNORMAL=0,
								mouseMOVE=0,
								mouseLDOWN,
								mouseLUP,
								mouseLDBLCLK,
								mouseMDOWN,
								mouseMUP,
								mouseMDBLCLK,
								mouseRDOWN,
								mouseRUP,
								mouseRDBLCLK,
								mouseLOSTCAPTURE,
								mouseWHEEL,
								mouseSTAY,
								mouseENTER,
								mouseLEAVE
};

extern ADLL char				*mouseEVENTS[];

// mouse states

#define							mouseCTRL						(1<<31)
#define							mouseSHIFT						(1<<30)
#define							mouseALT						(1<<29)
#define							mouseL							(1<<28)
#define							mouseM							(1<<27)
#define							mouseR							(1<<26)
#define							mouseFLAG						(1<<10)
#define							mousePULSE						(1<<01)		// alib internal message
#define							mouseCAPTURE					(1<<00)		// alib internal message


enum
{
								cursorNONE=-1,
								cursorNORMAL=0,
								cursorCROSS,
								cursorHELP,
								cursorTEXT,
								cursorSIZEALL,
								cursorSIZENESW,
								cursorSIZENWSE,
								cursorSIZENS,
								cursorSIZEWE,
								cursorHAND,
								cursorHANDKEEP,
								cursorHANDSEL,
								cursorINK,
								cursorWAIT,
								cursorMAGNIFY,
								cursorMAGNIFYADD,
								cursorMAGNIFYSUB,
								cursorNO,
								cursorCUSTOM=100000
};

// dragdrop events

enum
{
								dragdropENTER,
								dragdropOVER,
								dragdropDROP,
								dragdropLEAVE
};

// dragdrop states

#define							dragdropCTRL					mouseCTRL
#define							dragdropSHIFT					mouseSHIFT
#define							dragdropALT						mouseALT
#define							dragdropL						mouseL
#define							dragdropM						mouseM
#define							dragdropR						mouseR
#define							dragdropTEXT					(1<<25)
#define							dragdropBITMAP					(1<<24)
#define							dragdropSOUND					(1<<23)
#define							dragdropFILE					(1<<22)


// keyboard events

enum
{
								keyboardNORMAL=0,
								keyboardDOWN,
								keyboardUP,
								keyboardCHAR,
								keyboardGETFOCUS,
								keyboardLOSTFOCUS
};

// keyboard states

#define							keyboardCTRL					(1<<31)
#define							keyboardSHIFT					(1<<30)
#define							keyboardALT						(1<<29)

#define							keyboardLCTRL					(1<<28)
#define							keyboardLSHIFT					(1<<27)
#define							keyboardLALT					(1<<26)
#define							keyboardRCTRL					(1<<25)
#define							keyboardRSHIFT					(1<<24)
#define							keyboardRALT					(1<<23)

// keyboard states (getkeyboardstate)

#define							keyboardCURSORLEFT				(1<<19)
#define							keyboardCURSORRIGHT				(1<<18)
#define							keyboardHOME					(1<<17)
#define							keyboardEND						(1<<16)
#define							keyboardINSERT					(1<<15)
#define							keyboardDELETE					(1<<14)
#define							keyboardCLEAR					(1<<13)
#define							keyboardBACKSPACE				(1<<12)
#define							keyboardENTER					(1<<11)
#define							keyboardCURSORUP				(1<<10)
#define							keyboardCURSORDOWN				(1<<9)
#define							keyboardPAGEUP					(1<<8)
#define							keyboardPAGEDOWN				(1<<7)
#define							keyboardTAB						(1<<6)
#define							keyboardESCAPE					(1<<5)

// zorder actions

enum
{
								zorderFRONT=0,
								zorderBACK,
								zorderTOP
};



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! base GUI object
class Aobject : public Anode
{
public:

	enum
	{
								stateVISIBLE					= Anode::stateLAST,
								stateFOCUS						= stateVISIBLE<<1,
								stateNOCONTEXT					= stateFOCUS<<1,
								stateSIZEABLE					= stateNOCONTEXT<<1,
								stateLAST						= stateSIZEABLE<<1
	};

	ADLL static ACI				CI;
	virtual ACI					*getCI							()						{ return &CI; }

	Arectangle					pos;

	ADLL						Aobject							(char *name, class Aobject *father, int x, int y, int w, int h);
	ADLL virtual				~Aobject						();

								// action

	ADLL void					destroy							();	// asynchronous delete

	ADLL virtual bool			show							(bool visible, bool repaint=false);
	ADLL virtual bool			zorder							(int action);

	ADLL virtual bool			move							(int x, int y);
	virtual bool				size							(int w, int h)								{ pos.w=maxi(w, 0); pos.h=maxi(h, 0); return TRUE; }
	ADLL virtual bool			mouseCapture					(bool b, Aobject *o=NULL);
	ADLL virtual void			mousePosition					(int *x, int *y);
	ADLL virtual bool			focus							(Aobject *o=NULL);
	ADLL virtual bool			timer							(int time=0);
	virtual bool				setTooltips						(char *s)									{ if(tooltips) free(tooltips); return (tooltips=strdup(s))?TRUE:FALSE; }

	ADLL class Awindow *		getWindow						();
	ADLL bool					getPosition						(Aobject *o, int *x, int *y);
	ADLL class Aobject *		getObject						(int x, int y);

	ADLL int					getKeyboardState				();

	ADLL static bool			cursor							(int cursor=cursorNORMAL);
	ADLL static int				makeCursor						(Aresobj *res, int hx, int hy);
	
	ADLL static void			dropText						(char *text);

								// notify
	
	ADLL virtual void			update							(Abitmap *b);
	virtual void				paint							(Abitmap *b)								{ }
	ADLL virtual void			repaint							(Arectangle r, bool now=FALSE);
	ADLL virtual void			repaint							(bool now=FALSE);

	virtual bool				mouse							(int x, int y, int state, int event)		{ return FALSE; }
	virtual bool				dragdrop						(int x, int y, int state, int event, void *data=NULL)		{ return FALSE; }
	virtual bool				keyboard						(int event, int ascii, int scan, int state)	{ return FALSE; }
	virtual bool				system							(int event, dword d0=0, dword d1=0)			{ return FALSE; }

	virtual void				pulse							()											{ }


	class Aitem					*context;

								// private

	virtual bool				NCkeyboard						(int event, int ascii, int scan, int state)	{ if(keyboard(event, ascii, scan, state)) return true; if(father) return ((Aobject *)father)->NCkeyboard(event, ascii, scan, state); return false; }
	ADLL virtual bool			NCmouse							(int x, int y, int state, int event);
	ADLL virtual bool			NCdragdrop						(int x, int y, int state, int event, void *data=NULL);
	bool						NCzorder						(Anode *o, int action);

	char						*tooltips;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_OBJECT_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
