/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TITLEBAR.CPP				(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"titleBar.h"
#include						"interface.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						AtitleBar::CI=ACI("AtitleBar", GUID(0xAE57E515,0x00001000), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AtitleBar::AtitleBar(char *name, Aobject *L, int x, int y, int w, int h, char *caption) : Aobject(name, L, x, y, w, h)
{
	icon=new Abitmap(&alibres.get(MAKEINTRESOURCE(PNG_AESTTV), "PNG"));
	bac=FALSE;
	zorder(zorderTOP);
	if(caption)
		this->caption=strdup(caption);
	else
		this->caption=strdup(name);
	buttonClose=new Abutton("close", this, w-h, 2, 16, 16, &alibres.get(MAKEINTRESOURCE(PNG_BUTTONCLOSE), "PNG"));
	buttonClose->setTooltips("close button");
	buttonClose->show(TRUE);
	buttonMin=new Abutton("minimize", this, w-h*2-2, 4, 16, 12, &alibres.get(MAKEINTRESOURCE(PNG_BUTTONMIN), "PNG"));
	buttonMin->setTooltips("minimize button");
	buttonMin->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AtitleBar::size(int w, int h)
{
	if(Aobject::size(w, h))
	{
		buttonClose->pos.x=w-h;
		buttonMin->pos.x=w-h*2-2;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AtitleBar::~AtitleBar()
{
	delete(icon);
	delete(buttonMin);
	delete(buttonClose);
	if(caption)
		free(caption);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtitleBar::paint(Abitmap *bitmap)
{
	//Afont	*font=alib.getFont(fontROUND10);
	Afont	*font=alib.getFont(fontCONFIDENTIAL14);
	bitmap->boxfa(0, 0, bitmap->w, bitmap->h, 0x80000000);
//	font->set(bitmap, 6, 3, name, 0xff000000);
//	font->set(bitmap, 4, 1, name, 0xff000000);
//	font->set(bitmap, 6, 1, name, 0xff000000);
//	font->set(bitmap, 4, 3, name, 0xff000000);
	font->set(bitmap, 24, 2, name, 0xffffffff);
	bitmap->set(2, 2, icon);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AtitleBar::mouse(int x, int y, int state, int event)
{
/*
	switch(event)
	{
		case mouseLDOWN:
		wx=((Aobject *)father)->pos.x;
		wy=((Aobject *)father)->pos.y;
		lx=((Aobject *)father)->pos.x+x;
		ly=((Aobject *)father)->pos.y+y;
		bac=TRUE;
		cursor(cursorSIZEALL);
		mouseCapture(TRUE);
		return TRUE;

		case mouseNORMAL:
		if((state&mouseL)&&bac)
		{
			((Aobject *)father)->move(wx+(x+((Aobject *)father)->pos.x)-lx, wy+(y+((Aobject *)father)->pos.y)-ly);
			return TRUE;
		}
		break;

		case mouseLUP:
		bac=FALSE;
		mouseCapture(FALSE);
		return TRUE;
	}
*/
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AtitleBar::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if(o==buttonClose)
			father->notify(this, nyCLOSE);
		else if(o==buttonMin)
			father->notify(this, nyMINIMIZE);
		return TRUE;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AtitleBar::setCaption(char *caption)
{
	if(this->caption)
		free(this->caption);
	this->caption=strdup(caption);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
