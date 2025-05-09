/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EXPLORER.CPP				(c)	YoY'05						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<io.h>
#include						<direct.h>
#include						<assert.h>
#include						<math.h>
#include						"explorer.h"
#include						"global.h"
#include						"main.h"
#include						"resource.h"
#include						"vpone.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aexplorer::CI=ACI("Aexplorer", GUID(0x11111111,0x00110000), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AexplorerView : public Aobject, public Athread
{
public:
	AOBJ

	enum
	{
		cmdPLAY=0,
		cmdPAUSE=1
	};

								AexplorerView					(char *name, Aobject *l, int x, int y, int w, int h);
	virtual						~AexplorerView					();

	virtual void				paint							(Abitmap *b);
	virtual void				pulse							();
	virtual void				run								();
	virtual bool				notify							(Anode *o, int event, dword p);
	virtual bool				mouse							(int x, int y, int state, int event);

	void						setPos							(float p);
	float						getPos							();

	Asection					section;
	int							command;
	Abitmap						*image;
	char						filename[ALIB_PATH];
	bool						brun;
	bool						bstop;
	bool						bnewframe;
	int							nbFrame;
	int							seekTo;
	int							currentPos;
};				

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AexplorerView::CI=ACI("AexplorerView", GUID(0x11111111,0x00110200), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AexplorerView::AexplorerView(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	memset(filename, 0, sizeof(filename));
	image=new Abitmap(w, h);
	bnewframe=false;
	brun=false;
	bstop=true;
	currentPos=0;
	nbFrame=0;
	seekTo=-1;
	command=cmdPAUSE;
	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AexplorerView::~AexplorerView()
{
	brun=false;
	while(!bstop)
		sleep(1);
	delete(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AexplorerView::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		command=(command==cmdPLAY)?cmdPAUSE:cmdPLAY;
		return TRUE;
		case mouseLUP:
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexplorerView::pulse()
{
	if(bnewframe)
	{
		repaint();
		if(command==cmdPAUSE)
			bnewframe=false;
		father->notify(this, nyCHANGE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexplorerView::paint(Abitmap *b)
{
	section.enter(__FILE__,__LINE__);
	b->set(0, 0, image, bitmapNORMAL, bitmapNORMAL);
	switch(command)
	{
		case cmdPAUSE:
		b->boxfa(134, 10, 140, 30, 0xE000FF00);
		b->boxfa(144, 10, 150, 30, 0xE000FF00);
		b->boxa(134, 10, 140, 30, 0xE0000000);
		b->boxa(144, 10, 150, 30, 0xE0000000);
		break;
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexplorerView::run()
{
	brun=true;
	bstop=false;
	Amovie *avi=new Amovie(filename, filename);
	seekTo=-1;
	int tw=((MYwindow *)getWindow())->table->videoW;
	int th=((MYwindow *)getWindow())->table->videoH;
	if(avi->isOK)
	{
		Afont	*f=alib.getFont(fontTERMINAL06);
		Abitmap	*img=new Abitmap(avi->w, avi->h);
		float	fps=avi->getFPS();
		int		wtime=(int)(1000.f/fps);
		seekTo=0;
		nbFrame=avi->nbFrames;
		while(brun)
		{
			bool	bseek=false;
			if(seekTo!=-1)
			{
				avi->select(seekTo);
				currentPos=seekTo;
				seekTo=-1;
				bseek=true;
			}
			if(bseek||(command==cmdPLAY))
			{
				avi->get(img);
				section.enter(__FILE__,__LINE__);
				image->set(0, 0, image->w, image->h, 0, 0, img->w, img->h, img, bitmapNORMAL, bitmapNORMAL);
				{
					dword	color=((avi->w==tw)&&(avi->h==th))?0xFF00FF00:0xFFFF0000;
					char	str[256];
					sprintf(str, "%d x %d", avi->w, avi->h);
					int w=f->getWidth(str);
					image->boxfa(6, image->h-18, 8+w, image->h-8, 0xb0000000);
					f->set(image, 8, image->h-16, str, color);
				}
				section.leave();
				bnewframe=true;
				currentPos=avi->currentI;
			}
			if(avi->isLast)
				avi->select(Amovie::FIRST);
			else if(command==cmdPLAY)
				avi->select(Amovie::NEXT);
			sleep(wtime);
		}
		delete(img);
	}
	delete(avi);
	nbFrame=0;
	seekTo=-1;
	bstop=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AexplorerView::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		seekTo=p;
		return true;

		case nySELECT:
		{
			brun=false;
			while(!bstop)
				sleep(1);
			stop();
			strcpy(filename, (char *)p);
			start();
		}
		return true;

		case nyCOMMAND:
		command=p;
		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AexplorerView::setPos(float p)
{
	seekTo=(int)(p*(float)nbFrame);
	currentPos=seekTo;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float AexplorerView::getPos()
{
	return (float)currentPos/(float)nbFrame;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AbrowserItem : public Aobject
{
public:
	AOBJ

								AbrowserItem					(char *name, Aobject *l, int x, int y, int w, int h, char *filename);
	virtual						~AbrowserItem					();

	virtual void				paint							(Abitmap *b);
	virtual bool				mouse							(int x, int y, int state, int event);

	bool						setSel							(int state);

	Abitmap						*image;
	Asection					section;
	char						filename[ALIB_PATH];
	bool						selected;
	bool						test;
	bool						donedown;
	bool						indrag;
	char						*dragFiles;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AbrowserItem::CI=ACI("AbrowserItem", GUID(0x11111111,0x00110006), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AbrowserItem::AbrowserItem(char *name, Aobject *l, int x, int y, int w, int h, char *filename) : Aobject(name, l, x, y, w, h)
{
	strcpy(this->filename, filename);
	image=new Abitmap(w, h);
	selected=false;
	test=false;
	donedown=false;
	indrag=0;
	dragFiles=null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AbrowserItem::~AbrowserItem()
{
	delete(image);
	if(dragFiles)
		free(dragFiles);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AbrowserItem::setSel(int state)
{
	if((state&mouseCTRL)==mouseCTRL)
	{
		selected=!selected;
		repaint();
		father->father->notify(this, nySWITCH);
		return true;
	}
	else if((state&mouseSHIFT)==mouseSHIFT)
	{
		father->father->notify(this, nyEXTEND);
		return true;		
	}
	else 
	{
		father->father->notify(this, nySELECT);
		return true;
	}
	return false;
}

bool AbrowserItem::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		cursor(cursorHANDSEL);
		donedown=false;
		mouseCapture(true);
		test=true;
		if(!selected)
			donedown=setSel(state);
		return true;

		case mouseMOVE:
		if(indrag)
		{
			Awindow *w=getWindow();
			if(w->NCdragdrop(w->mouseX, w->mouseY, dragdropFILE, dragdropOVER, NULL))
				cursor(cursorCROSS);
			else
				cursor(cursorNO);
		}
		else if(test&&((x<0)||(y<0)||(x>pos.w)||(y>pos.h)))
		{
			if(selected)
			{
				test=false;
				donedown=false;
				father->father->notify(this, nyCOMMAND);
				indrag=true;
			}
			else
				cursor(cursorHANDSEL);
		}
		else
		{
			cursor(cursorHANDSEL);
		}
		return true;

		case mouseLUP:
		if(indrag)
		{
			Awindow *w=getWindow();
			mouseCapture(false);
			indrag=false;
			w->NCdragdrop(w->mouseX, w->mouseY, dragdropFILE, dragdropDROP, dragFiles);
			free(dragFiles);
			dragFiles=NULL;
		}
		else if(test)
		{
			mouseCapture(false);
			if(!donedown)
				setSel(state);
			test=false;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AbrowserItem::paint(Abitmap *b)
{
	section.enter(__FILE__,__LINE__);
	b->set(0, 0, image, bitmapNORMAL, bitmapNORMAL);
	section.leave();
	if(selected)
	{
		b->box(1, 1, pos.w-2, pos.h-2, 0xFFFFAA00);
		b->box(0, 0, pos.w-1, pos.h-1, 0xFFFFAA00);
		b->box(2, 2, pos.w-3, pos.h-3, 0xFF000000);
		b->box(3, 3, pos.w-4, pos.h-4, 0xFF000000);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AfileList
{
public:
	char						name[ALIB_PATH];
	char						filename[ALIB_PATH];
	class AfileList				*next;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Abrowser : public Aobject, Athread
{
public:
	AOBJ

								Abrowser						(char *name, Aobject *l, int x, int y, int w, int h);
	virtual						~Abrowser						();

	virtual void				run								();
	virtual void				pulse							();
	virtual bool				mouse							(int x, int y, int state, int event);
	virtual bool				notify							(Anode *o, int event, dword p=0);
	virtual bool				size							(int w, int h);

	void						set								(class AfileList *files);
	void						adjustScroll					();

	AscrollBar					*scroll;
	Aobject						*panel;
	bool						th_running;
	bool						th_stopped;
	bool						torepaint;
	AbrowserItem				*lastItem;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Abrowser::CI=ACI("Abrowser", GUID(0x11111111,0x00110005), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Abrowser::Abrowser(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	lastItem=null;
	panel=new Aobject("panel", this, 0, 0, w-10, h);
	scroll=new AscrollBar("scroll", this, w-10, 0, 10, h, AscrollBar::VERTICAL);
	scroll->colorBACK=0x80000000;
	scroll->colorBORDER=0xffffaa00;
	scroll->colorFORE=0xffffaa00;
	scroll->show(true);
	panel->show(true);
	th_running=false;
	th_stopped=true; 
	torepaint=false;
	timer(50);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Abrowser::~Abrowser()
{
	th_running=false;
	while(!th_stopped)
		sleep(1);
	delete(scroll);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Abrowser::adjustScroll()
{
	float	v=((float)pos.h-(float)panel->pos.y)/(float)panel->pos.h;
	scroll->setView((float)pos.h/(float)panel->pos.h);
	scroll->setPosition(-(float)panel->pos.y/(float)panel->pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Abrowser::size(int w, int h)
{
	if(Aobject::size(w, h))
	{
		panel->size(panel->pos.w, maxi(panel->pos.h, pos.h));
		{
			int		y=panel->pos.y;
			y=maxi(y, pos.h-panel->pos.h);
			y=mini(y, 0);
			panel->pos.y=y;
		}
		scroll->size(scroll->pos.w, pos.h);
		adjustScroll();
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Abrowser::pulse()
{
	if(torepaint)
	{
		torepaint=false;
		repaint();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Abrowser::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==scroll)
		{
			int		y=(int)(-scroll->getPosition()*(float)panel->pos.h);
			y=maxi(y, pos.h-panel->pos.h);
			y=mini(y, 0);
			panel->pos.y=y;
			repaint();

		}
		return true;

		case nyCOMMAND:
		{
			int len=1;
			AbrowserItem *bi=(AbrowserItem *)panel->fchild;
			while(bi)
			{
				if(bi->selected)
					len+=strlen(bi->filename)+1;
				bi=(AbrowserItem *)bi->next;
			}
			char	*str=(char *)malloc(len);
			char	*s=str;
			bi=(AbrowserItem *)panel->fchild;
			while(bi)
			{
				if(bi->selected)
				{
					strcpy(s, bi->filename);
					s+=strlen(s)+1;
				}
				bi=(AbrowserItem *)bi->next;
			}
			*s=0;
			((AbrowserItem *)o)->dragFiles=str;
		}
		return true;

		case nySELECT:
		{
			AbrowserItem *item=(AbrowserItem *)o;
			AbrowserItem *bi=(AbrowserItem *)panel->fchild;
			while(bi)
			{
				bi->selected=false;
				bi=(AbrowserItem *)bi->next;
			}
			item->selected=true;
			lastItem=item;
			repaint();
			father->notify(this, nySELECT, (dword)item->filename);
		}
		return true;

		case nySWITCH:
		lastItem=(AbrowserItem *)o;
		if(lastItem->selected)
			father->notify(this, nySELECT, (dword)lastItem->filename);
		break;

		case nyEXTEND:
		if((!lastItem)||(lastItem==o))
			return notify(o, nySELECT, 0);
		{
			AbrowserItem *item=(AbrowserItem *)o;
			AbrowserItem *bi=(AbrowserItem *)panel->fchild;
			bool		sel=false;
			while(bi)
			{
				if((bi==lastItem)||(bi==item))
				{
					sel=!sel;
					bi->selected=true;
				}
				else
					bi->selected=sel;
				bi=(AbrowserItem *)bi->next;
			}
			repaint();
		}
		break;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Abrowser::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseWHEEL:
		{
			float	dy=(float)getWindow()->mouseW/8.f;
			int		y=panel->pos.y+(int)dy;
			y=maxi(y, pos.h-panel->pos.h);
			y=mini(y, 0);
			panel->pos.y=y;
			adjustScroll();
			repaint();
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Abrowser::run()
{
	AbrowserItem *item=(AbrowserItem *)panel->fchild;
	th_running=true;
	th_stopped=false;

	CoInitialize(NULL);
	while(th_running&&item)
	{
		Abitmap	*b=getElekSnapshot(item->filename);
		if(b)
		{
			item->section.enter(__FILE__,__LINE__);
			item->image->set(0, 0, item->image->w, item->image->h, 0, 0, b->w, b->h, b, bitmapNORMAL, bitmapNORMAL);
			item->image->box(0, 0, item->image->w-1, item->image->h-1, 0xFF000000);
			item->section.leave();
			delete(b);
		}
		else
		{
			Afont *f=alib.getFont(fontTERMINAL06);
			item->section.enter(__FILE__,__LINE__);
			item->image->boxf(0, 0, item->image->w, item->image->h, 0xFFFFAA00);
			{
				char	text[256];
				char	*s=strrchr(item->name, '.');
				if(s)
					strcpy(text, s+1);
				else
					strcpy(text, "???");
				strupr(text);
				int wt=f->getWidth(text);
				int ht=f->getHeight(text);
				f->set(item->image, (item->image->w-wt)/2, (item->image->h-ht)/2, text, 0xFF000000);
			}
			item->image->box(0, 0, item->image->w-1, item->image->h-1, 0xFF000000);
			item->section.leave();
		}
		item=(AbrowserItem *)item->next;
		torepaint=true;
		sleep(50);
	}
	CoUninitialize();
	th_stopped=true;
	th_running=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Abrowser::set(AfileList *files)
{
	lastItem=null;
	th_running=false;
	while(!th_stopped)
		sleep(1);
	stop();
	while(panel->fchild)
		delete(panel->fchild);
	panel->pos.y=0;
	panel->size(panel->pos.w, pos.h);
	adjustScroll();
	repaint();
	{
		int n=0;

		while(files)
		{
			AbrowserItem *bi=new AbrowserItem(files->name, panel, 0, 0, 40, 30, files->filename);
			bi->setTooltips(files->name);
			files=files->next;
			n++;
		}

		panel->sort(sortINC);

		{
			AbrowserItem	*bi=(AbrowserItem *)panel->fchild;
			n=0;
			while(bi)
			{
				int x=(n%5)*44+4;
				int y=(n/5)*34+4;
				bi->pos.x=x;
				bi->pos.y=y;
				bi->show(true);
				bi=(AbrowserItem *)bi->next;
				n++;
			}
		}

		panel->size(panel->pos.w, maxi(pos.h, (((n-1)/5)+1)*34+4));
		adjustScroll();
	}
	repaint();
	start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AlinkNode : public Anode
{
public:
	AOBJ
									
								AlinkNode						(char *name);
	virtual						~AlinkNode						();

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AlinkNode::CI=ACI("AlinkDirectory", GUID(0x11111111,0x00110010), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AlinkNode::AlinkNode(char *name) : Anode(name)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AlinkNode::~AlinkNode()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AlinkDirectory : public AlinkNode
{
public:
	AOBJ
									
								AlinkDirectory					(char *name);
	virtual						~AlinkDirectory					();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AlinkDirectory::CI=ACI("AlinkDirectory", GUID(0x11111111,0x00110011), &AlinkNode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AlinkDirectory::AlinkDirectory(char *name) : AlinkNode(name)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AlinkDirectory::~AlinkDirectory()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AlinkMediatheque : public AlinkNode
{
public:
	AOBJ
									
								AlinkMediatheque				(char *name);
	virtual						~AlinkMediatheque				();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AlinkMediatheque::CI=ACI("AlinkMediatheque", GUID(0x11111111,0x00110012), &AlinkNode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AlinkMediatheque::AlinkMediatheque(char *name) : AlinkNode(name)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AlinkMediatheque::~AlinkMediatheque()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aexplorer::Aexplorer(char *name, Aobject *l, int x, int y, int w, int h) : Aobject(name, l, x, y, w, h)
{
	items=new Aitem("drives");
	items->state|=Aitem::stateFOLDER;
	scanDrive();
	tree=new AtreeView("tree", this, 0, 20, w, 240, items);
	tree->show(true);
	browser=new Abrowser("browser", this, 0, 260, w, h-260-120-37);
	browser->show(true);
	view=new AexplorerView("view", this, (w-160)/2, pos.h-160-33, 160, 120);
	view->show(true);
	slide=new AvponeSlide("slide", this, 0, h-18, w, 16);
	slide->colorBACK=0x70ffffff;
	slide->colorFORE=0x80000000;
	slide->setTooltips("preview timeline");
	slide->show(true);
	refresh=new Abutton("refresh", this, 4, 4, 12, 13, &resource.get(MAKEINTRESOURCE(PNG_REFRESH), "PNG")); 
	refresh->setTooltips("refresh folders");
	refresh->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aexplorer::~Aexplorer()
{
	delete(tree);
	delete(items);
	delete(browser);
	delete(view);
	delete(slide);
	delete(refresh);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexplorer::save(Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexplorer::load(Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexplorer::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==view)
		{
			slide->setValue(view->getPos());
		}
		else if(o==slide)
		{
			view->setPos(slide->getValue());
		}
		break;

		case nyCLICK:
		if(o==refresh)
		{
			char	cur[128][ALIB_PATH];
			int		n=0;
			{
				Aitem *i=tree->getCurrent();
				while(i)
				{
					strcpy(cur[n], i->name);
					i=(Aitem *)i->father;
					n++;
				}
			}
			items->clear();
			scanDrive();
			{
				Aitem *t=items;
				n--;
				while((--n)>=0)
				{
					Aitem *i=(Aitem *)t->fchild;
					if(!i)
					{
						t->setState(stateCOLLAPSED, false);
						scan(t);
						i=(Aitem *)t->fchild;
					}
					while(i)
					{
						if(!strcmp(cur[n], i->name))
						{
							t=i;
							break;
						}
						i=(Aitem *)i->next;
					}
					if(!i)
						break;
				}
				tree->setCurrent(t);
				notify(tree, nySELECT, (dword)t);
			}
			tree->refresh();
		}
		break;
		
		case nyDBLCKICK:
		return true;
		
		case nySELECT:
		if(o==browser)
		{
			view->notify(o, event, p);
		}
		else if(o==tree)
		{
			Aitem	*i=(Aitem *)p;
			if(i->link)
			{
				if(i->link->isCI(&AlinkDirectory::CI))
				{
					struct _finddata_t		fd;
					char					path[ALIB_PATH];
					sprintf(path, "%s\\*.*", i->link->name);
					long					h=_findfirst(path, &fd);
					AfileList				*list=NULL;
					if(h!=-1)
					{
						int r=0;
						while(r!=-1)
						{
							if(!(fd.attrib&_A_SUBDIR))
							{
								AfileList	*fl=new AfileList();
								strcpy(fl->name, fd.name);
								sprintf(fl->filename, "%s\\%s", i->link->name, fd.name);
								fl->next=list;
								list=fl;
							}
							r=_findnext(h, &fd);
						}
						_findclose(h);
					}
					browser->set(list);
				}
			}
		}
		return true;

		case nyEXTEND:
		if(o==tree)
		{
			Aitem	*i=(Aitem *)p;
			if(i->link)
			{
				if(i->link->isCI(&AlinkDirectory::CI))
					scan(i);
			}
		}
		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexplorer::paint(Abitmap *b)
{
	int py=view->pos.y;
	b->boxfa(0, 0, pos.w, py-5, 0xff000000, 0.5f);
	b->boxfa(0, py-4, pos.w, pos.h-30, 0xff000000, 0.7f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aexplorer::size(int w, int h)
{
	if(Aobject::size(w, h))
	{
		browser->size(pos.w, h-260-120-37); 
		view->pos.y=pos.h-view->pos.h-33;
		slide->pos.y=h-18;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexplorer::scanDrive()
{
	int	i;
	for(i=2; i<26; i++)	// c: to z:
	{
		struct _finddata_t		fd;
		char					path[ALIB_PATH];
		sprintf(path, "%c:\\*.*", 'A'+i);
		long					h=_findfirst(path, &fd);
		if(h!=-1)
		{
			_findclose(h);
			path[2]=0;
			AlinkDirectory	*n=new AlinkDirectory(path);
			Aitem	*i=new Aitem(path, items, n);
			i->state|=Aitem::stateDELLINK;
			i->state|=Aitem::stateFOLDER|Aitem::stateCOLLAPSED;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aexplorer::scan(Aitem *i)	// stateENABLE is used to find file deleted using the windows explorer 
{
	//assert(i->state&Aitem::stateFOLDER);
	if(!i->link)
	{
		Aitem *it=(Aitem *)i->fchild;
		while(it)
		{
			if(it->state&Aitem::stateFOLDER)
				scan(it);
			it=(Aitem *)it->next;
		}
	}
	else if(!(i->state&Aitem::stateCOLLAPSED))
	{
		struct _finddata_t		fd;
		char					path[ALIB_PATH];
		sprintf(path, "%s\\*.*", i->link->name);
		long					h=_findfirst(path, &fd);
		{
			Aitem *it=(Aitem *)i->fchild;
			while(it)
			{
				it->state&=~Aitem::stateENABLE;
				it=(Aitem *)it->next;
			}
		}
		if(h!=-1)
		{
			int		r=0;
			while(r!=-1)
			{
				bool	b=false;
				Aitem	*it=null;
				{
					it=(Aitem *)i->fchild;
					while(it)
					{
						if(!strcmp(it->name, fd.name))
						{
							b=true;
							break;
						}
						it=(Aitem *)it->next;
					}
				}
				if(!b)
				{
					char	fname[ALIB_PATH];
					if(strcmp(fd.name, ".")&&strcmp(fd.name, ".."))
					{
						if(fd.attrib&_A_SUBDIR)
						{
							sprintf(fname, "%s\\%s", i->link->name, fd.name);
							AlinkDirectory *n=new AlinkDirectory(fname);
							it=new Aitem(fd.name, i, n);
							i->state|=Aitem::stateDELLINK;
							it->state|=Aitem::stateFOLDER|Aitem::stateCOLLAPSED;
						}
					}
				}
				if(it)
				{
					it->state|=Aitem::stateENABLE;
					if(!(it->state&Aitem::stateCOLLAPSED))
						scan(it);
				}
				r=_findnext(h, &fd);
			}
			_findclose(h);
		}
		{
			Aitem *it=(Aitem *)i->fchild;
			while(it)
			{
				Aitem *in=(Aitem *)it->next;
				if(!(it->state&Aitem::stateENABLE))
					delete(it);
				it=in;
			}
		}
		i->sort();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

