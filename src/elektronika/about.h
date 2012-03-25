#ifndef	_ABOUT_H
#define	_ABOUT_H

#include						"global.h"



class Aabout : public Awindow
{
	AOBJ

								Aabout							(char *name, int w, int h, class MYwindow *myw);
	virtual						~Aabout							();

	virtual void				paint							(Abitmap *b);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual void				pulse							();

	int							bac,lx,ly,wx,wy;
	Abitmap						*back;
	class Astatic				*title;
	class Abutton				*buttonClose;
	class Abutton				*hlp;
	class Abutton				*web;
	class Abutton				*reg;
	class AaboutMenu			*menu;
	class MYwindow				*myw;

	struct 
	{
		float					ar;
		float					ag;
		float					ab;
		float					dr;
		float					dg;
		float					db;
		float					ar0;
		float					ag0;
		float					ab0;
		float					dr0;
		float					dg0;
		float					db0;
		float					ar1;
		float					ag1;
		float					ab1;
		float					dr1;
		float					dg1;
		float					db1;
		float					ar10;
		float					ag10;
		float					ab10;
		float					dr10;
		float					dg10;
		float					db10;
		float					n;
	}							
								pal;

	struct
	{
		float					zz1;
		float					zz2;
		float					zz3;
		float					ax1;
		float					ay1;
		float					ax2;
		float					ay2;
		float					ax3;
		float					ay3;
		float					dx1;
		float					dy1;
		float					dx2;
		float					dy2;
		float					dx3;
		float					dy3;
	}
								zyg;
};

#endif