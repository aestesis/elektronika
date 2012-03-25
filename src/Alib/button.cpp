/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BUTTON.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"button.h"
#include						"interface.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Abutton::CI=ACI("Abutton", GUID(0xAE57E515,0x00001010), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abutton::Abutton(char *name, Aobject *L, int x, int y, int w, int h, char *caption, int type) : Aobject(name, L, x, y, w, h)
{
	mode=modeLEAVE;
	if(!(type&btALIGNCENTER))
		type|=btALIGNCENTER;
	this->type=type;
	bmp1=NULL;
	checked=false;
	press=false;
	if(caption)
		this->caption=strdup(caption);
	else
		this->caption=strdup(name);
	colorBACKNORMAL=0x80404000;
	colorBACKOVER=0xc0404000;
	colorBACKDOWN=0xffffff00;
	colorBACKCHECK=0xffc0c000;
	colorFORENORMAL=0x80ffffff;
	colorFOREOVER=0xffffff00;
	colorFOREDOWN=0xff000000;
	colorFORECHECK=0xff404040;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abutton::Abutton(char *name, Aobject *L, int x, int y, int w, int h, Aresobj *o, int type) : Aobject(name, L, x, y, w, h)
{
	mode=modeLEAVE;
	checked=false;
	press=false;
	this->type=type;
	caption=NULL;
	bmp1=new Abitmap(o);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abutton::Abutton(char *name, Aobject *L, int x, int y, int w, int h, Abitmap *b, int type) : Aobject(name, L, x, y, w, h)
{
	mode=modeLEAVE;
	checked=false;
	press=false;
	this->type=type;
	caption=NULL;
	bmp1=new Abitmap(b->w, b->h);
	bmp1->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
	bmp1->state=b->state;
	bmp1->flags=b->flags;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Abutton::change(Abitmap *b)
{
	bmp1->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abutton::Abutton(char *name, Aobject *L, int x, int y, int def) : Aobject(name, L, x, y, 8, 8)
{
	mode=modeLEAVE;
	checked=false;
	press=false;
	caption=NULL;
	switch(def)
	{
		default:
		case btCHECK:
		type=btBITMAP|bt2STATES;
		bmp1=new Abitmap(&alibres.get(MAKEINTRESOURCE(PNG_BUTTONCHECK), "PNG"));
		size(16, 16);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abutton::~Abutton()
{
	if(caption)
		free(caption);
	if(bmp1)
		delete(bmp1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Abutton::paint(Abitmap *bitmap)
{
	if(type&btENABLEMODE)
	{
		switch(type&3)
		{
			case btBITMAP:
			if(state&stateENABLE)
			{
				if(type&bt2STATES)
				{
					if(checked)
						bitmap->set(0, 0, 0, bmp1->h>>1, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, 0, 0, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				else
				{
					if(checked)
						bitmap->set(0, 0, 2*bmp1->w/3, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, 0, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
			}
			else
			{
				if(type&bt2STATES)
				{
					if(checked)
						bitmap->set(0, 0, bmp1->w/3, bmp1->h>>1, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, bmp1->w/3, 0, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				else
				{
					if(checked)
						bitmap->set(0, 0, 2*bmp1->w/3, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, 0, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
			}
			break;
		};
	}
	else
	{
		switch(mode)
		{
			case modeLEAVE:
			switch(type&3)
			{
				case btCAPTION:
				{
					Afont	*font=alib.getFont(fontTERMINAL06);
					int		w=font->getWidth(caption);
					int		h=font->getHeight(caption);
					dword	cb=checked?colorBACKCHECK:colorBACKNORMAL;
					dword	cf=checked?colorFORECHECK:colorFORENORMAL;
					bitmap->boxfa(0, 0, pos.w-1, pos.h-1, cb, (float)(cb>>24)/255.f);
					switch(type&btALIGNCENTER)
					{
						case btALIGNLEFT:
						font->set(bitmap, 2, (pos.h-h)>>1, caption, cf);
						break;

						case btALIGNRIGHT:
						{
							Arectangle	r=font->getRectangle(0, 0, caption);
							font->set(bitmap, pos.w-r.w, (pos.h-h)>>1, caption, cf);
						}
						break;

						default:
						font->set(bitmap, (pos.w-w)>>1, (pos.h-h)>>1, caption, cf);
						break;
					}
				}
				break;

				case btBITMAP:
				if(type&bt2STATES)
				{
					if(checked)
						bitmap->set(0, 0, 0, bmp1->h>>1, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, 0, 0, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				else
				{
					if(checked)
						bitmap->set(0, 0, 2*bmp1->w/3, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, 0, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				break;
			}
			break;

			case modeOVER:
			switch(type&3)
			{
				case btCAPTION:
				{
					Afont	*font=alib.getFont(fontTERMINAL06);
					int		w=font->getWidth(caption);
					int		h=font->getHeight(caption);
					dword	cb=checked?colorBACKCHECK:colorBACKOVER;
					dword	cf=checked?colorFORECHECK:colorFOREOVER;
					bitmap->boxfa(0, 0, pos.w-1, pos.h-1, cb, (float)(cb>>24)/255.f);
					switch(type&btALIGNCENTER)
					{
						case btALIGNLEFT:
						font->set(bitmap, 2, (pos.h-h)>>1, caption, cf);
						break;

						case btALIGNRIGHT:
						{
							Arectangle	r=font->getRectangle(0, 0, caption);
							font->set(bitmap, pos.w-r.w, (pos.h-h)>>1, caption, cf);
						}
						break;

						default:
						font->set(bitmap, (pos.w-w)>>1, (pos.h-h)>>1, caption, cf);
						break;
					}
				}
				break;

				case btBITMAP:
				if(type&bt2STATES)
				{
					if(checked)
						bitmap->set(0, 0, bmp1->w/3, bmp1->h>>1, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, bmp1->w/3, 0, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				else
				{
					if(checked)
						bitmap->set(0, 0, 2*bmp1->w/3, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, bmp1->w/3, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				break;
			}
			break;
			
			case modePRESS:
			switch(type&3)
			{
				case btCAPTION:
				{
					Afont	*font=alib.getFont(fontTERMINAL06);
					int		w=font->getWidth(caption);
					int		h=font->getHeight(caption);
					bitmap->boxfa(0, 0, pos.w-1, pos.h-1, colorBACKDOWN, (float)(colorBACKDOWN>>24)/255.f);
					switch(type&btALIGNCENTER)
					{
						case btALIGNLEFT:
						font->set(bitmap, 2, (pos.h-h)>>1, caption, colorFOREDOWN);
						break;

						case btALIGNRIGHT:
						{
							Arectangle	r=font->getRectangle(0, 0, caption);
							font->set(bitmap, pos.w-r.w, (pos.h-h)>>1, caption, colorFOREDOWN);
						}
						break;

						default:
						font->set(bitmap, (pos.w-w)>>1, (pos.h-h)>>1, caption, colorFOREDOWN);
						break;
					}
				}
				break;


				case btBITMAP:
				if(type&bt2STATES)
				{
					if(checked)
						bitmap->set(0, 0, 2*bmp1->w/3, bmp1->h>>1, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
					else
						bitmap->set(0, 0, 2*bmp1->w/3, 0, bmp1->w/3, bmp1->h>>1, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				}
				else
					bitmap->set(0, 0, 2*bmp1->w/3, 0, bmp1->w/3, bmp1->h, bmp1, bitmapDEFAULT, bitmapDEFAULT);
				break;
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Abutton::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseNORMAL:
		cursor(cursorHANDSEL);
		return true;

		case mouseLDOWN:
		press=true;
		mode=modePRESS;
		cursor(cursorHANDSEL);
		mouseCapture(TRUE);
		if(!(type&bt2STATES))
			checked=true;
		notify(this, nyPRESS);
		repaint();
		return TRUE;

		case mouseENTER:
		mode=modeOVER;
		cursor(cursorHANDSEL);
		repaint();
		return TRUE;

		case mouseLEAVE:
		mode=modeLEAVE;
		repaint();
		return TRUE;

		case mouseLUP:
		cursor(cursorHANDSEL);
		if(press)
		{
			press=false;
			mode=modeOVER;
			mouseCapture(FALSE);
			if(type&bt2STATES)
				checked=!checked;
			else
				checked=false;
			notify(this, nyRELEASE);
			if(type&bt2STATES)
				notify(this, nyCHANGE, checked?1:0);
			else
				notify(this, nyCLICK);
			repaint();
		}
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Abutton::setCaption(char *caption)
{
	if(this->caption)
		free(this->caption);
	this->caption=strdup(caption);
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Abutton::setChecked(bool c)
{
	if(checked!=c)
	{
		checked=c;
		repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
