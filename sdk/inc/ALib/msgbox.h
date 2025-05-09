/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MSGBOX.H					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_MSGBOX_H_
#define							_MSGBOX_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"types.h"
#include						"node.h"
#include						"object.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidMSGBOX						(Amsgbox::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! display a message box
class Amsgbox : public Anode
{
public:

	enum	// type
	{
		MBABORTRETRYIGNORE,
		MBCANCELTRYCONTINUE,
		MBOK,
		MBOKCANCEL,
		MBRETRYCANCEL,
		MBYESNO,
		MBYESNOCANCEL
	};

	enum	// icons
	{
		MBICONEXCLAMATION,
		MBICONWARNING,
		MBICONINFORMATION,
		MBICONASTERISK,
		MBICONQUESTION,
		MBICONSTOP,
		MBICONERROR,
		MBICONHAND
	};

	enum	// return value
	{
		MBRERROR,
		MBRABORT,
		MBRCANCEL,
		MBRCONTINUE,
		MBRIGNORE,
		MBRNO,
		MBROK,
		MBRRETRY,
		MBRTRYAGAIN,
		MBRYES
	};

	ALIBOBJ

	ADLL						Amsgbox							(char *name, Aobject *onotify, char *title, char *text, int type, int icon=MBICONINFORMATION);
	ADLL virtual				~Amsgbox						();

	ADLL int					getValue						();

	static ADLL int				msgbox							(Aobject *parent, char *title, char *text, int type=MBOK, int icon=MBICONINFORMATION);

	Aobject						*toNotify;
	int							rvalue;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_MSGBOX_H_
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
