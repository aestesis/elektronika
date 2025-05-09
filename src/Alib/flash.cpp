/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FLASH.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"flash.h"
#include						"flashSDK/Source/win32/playerwnd.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Aflash::CI=ACI("Aflash", GUID(0xAE57E515,0x00000120), &Asurface::CI, 0, NULL); // countof(properties), properties);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aflash::Aflash(char *name, Aobject *l, int x, int y, int w, int h) : Asurface(name, l, x, y, w, h)
{
	pw=new NativePlayerWnd(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Aflash::~Aflash()
{
	delete(pw);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aflash::play(char *filename)
{
	pw->ControlOpen(filename);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aflash::play(class Aresobj *o)
{
	pw->ControlOpen(o);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Aflash::pulse()
{
	int	t=GetTickCount();
	if(t>cursorTime)
	{
		pw->CursorTimerFire();
		cursorTime=t+cursorTimeInterval;
	}
	pw->PlayTimerFire();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aflash::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		mouseCapture(TRUE);
		pw->MouseDown(x, y);
		return TRUE;

		case mouseLUP:
		mouseCapture(FALSE);
		pw->MouseUp(x, y);
		return TRUE;

		case mouseNORMAL:
		pw->MouseMove(x, y, state&mouseL);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aflash::keyboard(int event, int ascii, int scan)
{
	switch(event)
	{
		case keyboardDOWN:
		{
			static byte vkCodes[] = 
			{
				VK_LEFT,	//ID_KEY_LEFT
				VK_RIGHT,	//ID_KEY_RIGHT
				VK_HOME,	//ID_KEY_HOME
				VK_END,		//ID_KEY_END
				VK_INSERT,	//ID_KEY_INSERT
				VK_DELETE,	//ID_KEY_DELETE
				0,			//ID_KEY_CLEAR
				VK_BACK,	//ID_KEY_BACKSPACE
				0,			//ID_KEY_CUT		
				0,			//ID_KEY_COPY		
				0,			//ID_KEY_PASTE	
				0,			//ID_KEY_SELECT_ALL
				VK_RETURN,	//ID_KEY_ENTER	
				VK_UP,		//ID_KEY_UP		
				VK_DOWN,	//ID_KEY_DOWN		
				VK_PRIOR,	//ID_KEY_PAGE_UP	
				VK_NEXT,	//ID_KEY_PAGE_DOWN
				VK_TAB,		//ID_KEY_TAB		
				VK_ESCAPE	//ID_KEY_ESCAPE
			};
			int modifiers = 0;
			int	ks=getKeyboardState();
			if (ks&keyboardSHIFT)
				modifiers|=Keyboard::ID_KEY_SHIFT;
			if (ks&keyboardCTRL)
				modifiers|=Keyboard::ID_KEY_CTRL;
			for (int i=0; i<sizeof(vkCodes); i++) 
			{
				if(ascii==vkCodes[i]) 
				{
					pw->HandleKeyDown(i+1, modifiers);
					return TRUE;
				}
			}
			return TRUE;
		}
		break;

		case keyboardCHAR:
		if(ascii!=VK_TAB&&ascii!=VK_RETURN&&ascii!=VK_BACK)
		{
			pw->HandleKeyDown(ascii, 0);
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AflashVars * Aflash::getVars()	// don't forget to free the AflashVars return buffer
{
	AflashVars		vars[1024];
	ScriptThread	*p=&pw->player;
	int				n=0;
	while(p&&(n<1024))
	{
		ScriptVariable	*v=p->firstVariable;
		while(v&&(n<1024))
		{
			strcpy(vars[n].name, v->name);
			strcpy(vars[n].value, v->value);
			v=v->next;
			n++;
		}
		p=p->next;
	}
	{
		AflashVars	*v=new AflashVars [n+1];
		if(n)
			memcpy(v, vars, sizeof(AflashVars)*n);
		v[n].name[0]=0;
		v[n].value[0]=0;
		return v;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
