/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FFRAME.CPP					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<io.h>
#include						<assert.h>
#include						"resource.h"
#include						"fframe.h"
#include						"FreeFrame.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AfframeInfo::CI		= ACI("AfframeInfo",		GUID(0x11111112,0x00000390), &AeffectInfo::CI, 0, NULL);
ACI								Afframe::CI			= ACI("Afframe",			GUID(0x11111112,0x00000391), &Aeffect::CI, 0, NULL);
ACI								AfframeFront::CI	= ACI("AfframeFront",		GUID(0x11111112,0x00000392), &AeffectFront::CI, 0, NULL);
ACI								AfframeBack::CI		= ACI("AfframeBack",		GUID(0x11111112,0x00000393), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef __declspec(dllimport) plugMainUnion (__stdcall *FFA_Main_FuncPtr)(DWORD,LPVOID,void *);

typedef struct
{
	HINSTANCE			hDLL;
	FFA_Main_FuncPtr	main;
	PlugInfoStructTag	*info;
	char				name[17];
	bool				initialized;
	int					nbinstance;		// 4 debug
} Tplugz;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int				count=0;
static int				nplugz=0;
static Tplugz			plugz[512];
static Asection			sstatic;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!(count++))
	{ // add list of externals effects
		struct _finddata_t		fd;
		char					path[ALIB_PATH];
		char					error[ALIB_PATH];
		FILE					*ferror;
		GetModuleFileName(GetModuleHandle(null), path, sizeof(path));
		if(path[0])
		{
			char	*s=strrchr(path, '\\');
			if(s)
				*s=0;
		}
		strcpy(error, path);
		strcat(path, "\\freeframe\\*.dll");
		strcat(error, "\\freeframe\\_errors.txt");
		ferror=fopen(error, "w");
		long					h=_findfirst(path, &fd);
		if(h!=-1)
		{
			int		r=0;
			while(r!=-1)
			{
				char		str[256];
				HINSTANCE	hDLL=NULL;
				sprintf(str, "freeframe\\%s", fd.name);
				hDLL=LoadLibrary(str);
				if(hDLL)
				{
					FFA_Main_FuncPtr	main=(FFA_Main_FuncPtr)GetProcAddress(hDLL, "plugMain");
					try
					{
						if(main)
						{
							plugMainUnion	ret=main(FF_GETINFO, 0, 0);
							if(ret.PISvalue)
							{
								plugz[nplugz].info=ret.PISvalue;
								memcpy(plugz[nplugz].name, plugz[nplugz].info->pluginName, 16);
								plugz[nplugz].name[16]=0;
								ret=main(FF_GETPLUGINCAPS,(void*)1,0);
								if(ret.ivalue==FF_TRUE)
								{
									plugz[nplugz].main=main;
									plugz[nplugz].hDLL=hDLL;
									plugz[nplugz].nbinstance=0;
									{
										plugMainUnion	ret=main(FF_INITIALISE, NULL, 0);
										plugz[nplugz].initialized=(ret.ivalue==FF_SUCCESS);
									}
									nplugz++;
									hDLL=NULL;
								}
							}
						}
					}
					catch(...)
					{
						if(ferror)
							fprintf(ferror, "ERROR, loading FreeFrame plugins %s\r\n", fd.name); 
					}
					if(hDLL)
						FreeLibrary(hDLL);
				}
				r=_findnext(h, &fd);
			}
			_findclose(h);
		}
		if(ferror)
			fclose(ferror);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void release()
{
	if(!(--count))
	{
		int	i;
		for(i=0; i<nplugz; i++)
		{
			if(plugz[i].initialized)
			{
				plugz[i].main(FF_DEINITIALISE, 0, 0);
				plugz[i].initialized=false;
			}
			assert(plugz[i].nbinstance==0);
			FreeLibrary(plugz[i].hDLL);
		}
		nplugz=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Afframe::Afframe(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	ffeffect=null;
	init();
	imagedata=NULL;
	front=new AfframeFront(qiid, "freeframe front", this, 48);
	front->setTooltips("freeframe module");
	back=new AfframeBack(qiid, "freeframe back", this, 48);
	back->setTooltips("freeframe module");
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Afframe::~Afframe()
{
	AfframeFront	*front=(AfframeFront *)this->front;
	front->section.enter(__FILE__,__LINE__);
	if(ffeffect)
	{
		FFA_Main_FuncPtr	main=plugz[front->neffect].main;
		plugMainUnion		ret=main(FF_DEINSTANTIATE, NULL, ffeffect);
		plugz[front->neffect].nbinstance--;
		ffeffect=NULL;
		front->neffect=-1;
	}
	front->section.leave();
	free(imagedata);
	release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Afframe::load(class Afile *f)
{
	AfframeFront	*front=(AfframeFront *)this->front;
	bool			b;
	f->read(&b, sizeof(b));
	if(b)
	{
		char	id[4];
		int		i;
		f->read(&id, sizeof(id));
		for(i=0; i<nplugz; i++)
		{
			int	j;
			for(j=0; j<4; j++)
				if(plugz[i].info->uniqueID[j]!=id[j])
					break;
			if(j==4)
			{
				front->feffect->setCurrentByData(i);
				front->notify(front->feffect, nyCHANGE, 0);
				break;
			}
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Afframe::save(class Afile *f)
{
	AfframeFront	*front=(AfframeFront *)this->front;
	bool			b=(front->neffect>=0);
	f->write(&b, sizeof(b));
	if(b)
		f->write(plugz[front->neffect].info->uniqueID, sizeof(plugz[front->neffect].info->uniqueID));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Afframe::loadPreset(class Afile *f)
{
	AfframeFront	*front=(AfframeFront *)this->front;
	bool			b;
	f->read(&b, sizeof(b));
	if(b)
	{
		char	id[4];
		int		i;
		f->read(&id, sizeof(id));
		for(i=0; i<nplugz; i++)
		{
			int	j;
			for(j=0; j<4; j++)
				if(plugz[i].info->uniqueID[j]!=id[j])
					break;
			if(j==4)
			{
				front->feffect->setCurrentByData(i);
				front->notify(front->feffect, nyCHANGE, 0);
				break;
			}
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Afframe::savePreset(class Afile *f)
{
	AfframeFront	*front=(AfframeFront *)this->front;
	bool			b=(front->neffect>=0);
	f->write(&b, sizeof(b));
	if(b)
		f->write(plugz[front->neffect].info->uniqueID, sizeof(plugz[front->neffect].info->uniqueID));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Afframe::settings(bool emergency)
{
	if(imagedata)
		free(imagedata);
	imagedata=(byte*)malloc(getVideoWidth()*getVideoHeight()*3+32000);
	{
		AfframeFront	*front=(AfframeFront *)this->front;
		front->notify(front->feffect, nyCHANGE, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Afframe::action(double time, double dtime, double beat, double dbeat)
{
	AfframeFront	*front=(AfframeFront *)this->front;
	AfframeBack		*back=(AfframeBack *)this->back;
	Abitmap			*bb=back->video->getBitmap();
	Abitmap			*bin=back->in->getBitmap();
	
	if(bb)
	{
		if(front->bypass->get())
		{
			bb->set(0, 0, bin, bitmapNORMAL, bitmapNORMAL);
		}
		else
		{
			if(bin)
			{
				dword	*d=bin->body32;
				byte	*s=(byte *)imagedata+16000;
				int		n=bin->w*bin->h;
				int		i;
				for(i=0; i<n; i++)
				{
					byte	r,g,b;
					colorRGB(&r, &g, &b, *(d++));
					*(s++)=b;
					*(s++)=g;
					*(s++)=r;
				}
			}
			front->section.enter(__FILE__,__LINE__);
			if(front->neffect>=0)
			{
				FFA_Main_FuncPtr	main=plugz[front->neffect].main;
				main(FF_PROCESSFRAME, (LPVOID)(imagedata+16000), ffeffect);
			}
			front->section.leave();
			{
				dword	*d=bb->body32;
				byte	*s=(byte *)(imagedata+16000);
				int		n=bb->w*bb->h;
				int		i;
				for(i=0; i<n; i++)
				{
					byte	b=*(s++);
					byte	g=*(s++);
					byte	r=*(s++);
					*(d++)=color32(r, g, b);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID qiidpad[]={	0x582f8b0775fed4b0, 0x663419cdc477f0b8, 0xe18566ca2bb1efc6, 0x4f71d9f8848a7400,
						0xc52a4864d8a69084, 0x9b3e4ecbd5688060, 0xaf3a7cab349bed0c, 0x8459cc165318dc54,
						0x2d2ed80c8ec8ea74, 0x6042d78ea1a8efc0, 0x790634ab2985de00, 0x275d7f0768a9925c,
						0x8e4d7c163c749500, 0x19313a0e1ed02b88, 0x01cbe332f8096010, 0x2e6f39b8389bdb00};

AfframeFront::AfframeFront(QIID qiid, char *name, Afframe *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_FFRAME_FRONT), "PNG");
	Afont	*font=alib.getFont(fontTERMINAL05);
	int		i;
	back=new Abitmap(&o);
	
	neffect=-1;
	
	ifeffect=new Aitem("effect", "effect");
	new Aitem("[none]", "no effect", ifeffect, (dword)-1);
	for(i=0; i<nplugz; i++)
	{
		char	str[1024];
		strcpy(str, plugz[i].name);
		strupr(str);
		new Aitem((char *)str, (char *)plugz[i].name, ifeffect, (dword)i);
	}
/*
	{
		Aitem	*it=new Aitem("dir", "dir", ifeffect, 0);
		new Aitem("test 1", "test 1", it, 0);
		new Aitem("test 2", "test 2", it, 0);
	}
*/
	ifeffect->sort();

	feffect=new Alist("freeframe effect", this, 12, 24, 100, 16, ifeffect);
	feffect->setTooltips("freeframe effect");
	feffect->show(TRUE);
	feffect->setCurrentByData(-1);

	bypass=new ActrlButton(MKQIID(qiid, 0x86268cc1613a89f0), "bypass", this, 494, 14, 7, 7, &resource.get(MAKEINTRESOURCE(PNG_FFRAME_BUTTON), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	bypass->setTooltips("bypass");
	bypass->show(TRUE);

	rst=new ActrlButton(MKQIID(qiid, 0xa849d5251ee2d800), "reset", this, 114, 25, 21, 17, &resource.get(MAKEINTRESOURCE(PNG_FFRAME_RST), "PNG"), Abutton::btBITMAP);
	rst->setTooltips("default value");
	rst->show(TRUE);

	for(i=0; i<16; i++)
	{
		char	nmpad[256];
		int	x=i*22+140;
		int	y=(i&1)*8+12;
		int	y2=(i&1)*33+3;
		
		sprintf(nmpad, "pad[%2d]", i+1);
		
		pad[i]=new Apaddle(MKQIID(qiid, qiidpad[i]), nmpad, this, x, y, paddleYOLI16);
		pad[i]->setTooltips("");
		pad[i]->set(0.5f);
		pad[i]->show(TRUE);
		
		sta[i]=new Astatic("sta[]", this, x-10, y2, 38, 10);
		sta[i]->setTooltips("");
		sta[i]->show(true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AfframeFront::~AfframeFront()
{
	int	i;
	for(i=0; i<16; i++)
	{
		delete(pad[i]);
		delete(sta[i]);
	}
	delete(feffect);
	delete(ifeffect);
	delete(bypass);
	delete(rst);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AfframeFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AfframeFront::pulse()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AfframeFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
		if((o==rst)&&(neffect>=0))
		{
			section.enter(__FILE__,__LINE__);
			{
				FFA_Main_FuncPtr	main=plugz[neffect].main;
				int					i;
				for(i=0; i<npad; i++)
				{
					SetParameterStruct	param;
					pad[i]->set(def[i]);
					param.index=i;
					param.value=def[i];
					main(FF_SETPARAMETER, (LPVOID)&param, ((Afframe *)effect)->ffeffect);
				}
				for(i=npad; i<16; i++)
					pad[i]->set(0.5f);
			}
			section.leave();			
		}
		break;
		
		case nyCHANGE:
		if(o==feffect)
		{
			int	n=feffect->getCurrentData();
			section.enter(__FILE__,__LINE__);
			if(((Afframe *)effect)->ffeffect)
			{
				Afont				*font=alib.getFont(fontTERMINAL05);
				FFA_Main_FuncPtr	main=plugz[neffect].main;
				plugMainUnion		ret=main(FF_DEINSTANTIATE, NULL, ((Afframe *)effect)->ffeffect);
				int					i;
				plugz[neffect].nbinstance--;
				((Afframe *)effect)->ffeffect=NULL;
				neffect=-1;
				npad=0;
				for(i=0; i<16; i++)
				{
					sta[i]->set("", 0xffCC0030, font);
					pad[i]->setTooltips("");
					sta[i]->setTooltips("");
					def[i]=maxi(mini(0.5f, 1.f), 0.f);
					pad[i]->set(def[i]);
				}
			}
			{
				if(n>=0)
				{
					Afont				*font=alib.getFont(fontTERMINAL05);
					FFA_Main_FuncPtr	main=plugz[neffect=n].main;
					plugMainUnion		ret=main(FF_GETNUMPARAMETERS, 0, 0);
					int					i;
					npad=mini(ret.ivalue, 16);
					sstatic.enter(__FILE__,__LINE__);
					for(i=0; i<npad; i++)
					{
						static void		*pCastCount;
						pCastCount=reinterpret_cast<void*>(i);
						plugMainUnion	ret=main(FF_GETPARAMETERNAME, pCastCount, ((Afframe *)effect)->ffeffect);
						char			name[32];
						if(ret.svalue&&ret.svalue[0])
						{
							memcpy(name, ret.svalue, 16);
							name[15]=0;
							strupr(name);
						}
						else
							strcpy(name, "NONAME");
						ret=main(FF_GETPARAMETERDEFAULT, pCastCount, 0);
						if(strlen(name)>8)
							sta[i]->set(name, 0xffCC0030, font, Astatic::LEFT);
						else
							sta[i]->set(name, 0xffCC0030, font);
						pad[i]->setTooltips(name);
						sta[i]->setTooltips(name);
						def[i]=maxi(mini(ret.fvalue, 1.f), 0.f);
						pad[i]->set(def[i]);
					}
					sstatic.leave();
					for(i=npad; i<16; i++)
					{
						sta[i]->set("", 0xff000000, font);
						pad[i]->set(0.5f);
						pad[i]->setTooltips("");
						sta[i]->setTooltips("");
					}
					{
						Afframe	*ff=(Afframe *)effect;
						VideoInfoStruct	info;
						info.bitDepth=1;
						info.frameWidth=ff->getVideoWidth();
						info.frameHeight=ff->getVideoHeight();
						plugMainUnion	ret=main(FF_INSTANTIATE, (LPVOID)&info, 0);
						if(ret.ivalue)
						{
							ff->ffeffect=(void *)ret.ivalue;
							plugz[neffect].nbinstance++;
						}
						else
						{
							feffect->setCurrentByData(-1);
							neffect=-1;
						}
					}
				}
			}
			section.leave();
		}
		else 
		{
			section.enter(__FILE__,__LINE__);
			if(neffect>=0)
			{
				int	i;
				for(i=0; i<npad; i++)
				{
					if(pad[i]==o)
					{
						FFA_Main_FuncPtr	main=plugz[neffect].main;
						SetParameterStruct	param;
						param.index=i;
						param.value=pad[i]->get();
						main(FF_SETPARAMETER, (LPVOID)&param, ((Afframe *)effect)->ffeffect);
					}
				}
			}
			section.leave();
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AfframeBack::AfframeBack(QIID qiid, char *name, Afframe *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_FFRAME_BACK), "PNG");
	back=new Abitmap(&o);

	in=new Avideo(MKQIID(qiid, 0x467f104db0f3e988), "video in", this, pinIN, 10, 10);
	in->setTooltips("video in");
	in->show(TRUE);

	video=new Avideo(MKQIID(qiid, 0x2b4d5a1c0b0422d8), "video out", this, pinOUT, pos.w-18, 10);
	video->setTooltips("video out");
	video->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AfframeBack::~AfframeBack()
{
	delete(back);
	delete(video);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AfframeBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AfframeInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Afframe(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * fframeGetInfo()
{
	return new AfframeInfo("fframeInfo", &Afframe::CI, "freeframe", "freeframe module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
