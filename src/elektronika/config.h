#ifndef _CONFIG_H_
#define _CONFIG_H_

#include						"global.h"

enum
{
	rate8,
	rate12,
	rate20,
	rate24,
	rate25,
	rate30,
	rate50,
	rate60
};

enum
{
	compressNONE,
	compressJPEG100,
	compressJPEG80,
	compressJPEG50
};

class Aconfig : public Aobject
{
public:
	AOBJ

								Aconfig							(char *name, Aobject *father);
	virtual						~Aconfig						();

	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual void				pulse							();

	class Abutton				*close;
	class Aitem					*resItems;
	class Alist					*resList;
	class Aitem					*rateItems;
	class Alist					*rateList;

	class Aitem					*timeItems;
	class Alist					*timeList;
	class Aitem					*cmpItems;
	class Alist					*cmpList;
	
	//class Abutton				*render;
	//class Abutton				*priority;
	class Aitem					*midiInItems;
	class Amlist				*midiIn;
	class Aitem					*midiOutItems;
	class Alist					*midiOut;
	class Aitem					*psetItems;
	class Alist					*pset;
	class Abutton				*splash;
	class Abutton				*tips;
	class Abutton				*regist;

	class Aselsize				*sndsize;
};

class Apass : public Aobject
{
public:
	AOBJ

								Apass							(char *name, Aobject *father);
	virtual						~Apass							();

	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual void				pulse							();

	class Abutton				*close;
	class Aedit					*ename;
	class Aedit					*epwd;
	class Abutton				*web;
};

#endif