/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	omppwrap.CPP				(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>
#include						"resource.h"
#include						"omppwrap.h"
#include						"table.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AomppwrapInfo::CI		= ACI("AomppwrapInfo",		GUID(0x11111112,0x00000430), &AeffectInfo::CI, 0, NULL);
ACI								Aomppwrap::CI			= ACI("Aomppwrap",			GUID(0x11111112,0x00000431), &Aeffect::CI, 0, NULL);
ACI								AomppwrapFront::CI		= ACI("AomppwrapFront",		GUID(0x11111112,0x00000432), &AeffectFront::CI, 0, NULL);
ACI								AomppwrapBack::CI		= ACI("AomppwrapBack",		GUID(0x11111112,0x00000433), &AeffectBack::CI, 0, NULL);
ACI								AomppwrapFrontXml::CI	= ACI("AomppwrapFrontXml",	GUID(0x11111112,0x00000434), &AeffectFront::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class OmppMediaHost : public MediaHost
{
public:
	Aomppwrap					*ow;

	OmppMediaHost(Aomppwrap *ow)
	{
		this->ow=ow;
	}

	virtual double getBPM()
	{
		return ow->getTable()->bpmFreq;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static xmlNode * xmlParse(xmlNode *node, char *item)
{
	if(node)
	{
		xmlNode *n=node->xmlChildrenNode;
		while (n != NULL) 
		{
			if(!strcmp((char *)n->name, item))
				return n;
			n=n->next;
		}
	}
	return null;
}

static char * xmlAttrib(xmlNode *node, char *item)
{
	if(node&&item)
	{
		xmlAttr *a=node->properties;
		while(a)
		{
			if(!strcmp((char *)a->name, item))
			{
				xmlNode *n=a->children;
				if(n)
					return (char *)n->content;
				break;
			}
			a=a->next;
		}
	}
	return null;
}

static bool xmlAttribTest(xmlNode *node, char *item, char *value)
{
	char *xvalue=xmlAttrib(node, item);
	return (xvalue&&value&&!strcmp(xvalue, value));
}

static Abitmap * getBitmap(char *dll, char *fnbmp)
{
	char	path[ALIB_PATH];
	strcpy(path, dll);
	strrchr(path, '\\')[1]=0;
	strcat(path, fnbmp);
	{
		Abitmap *b=new Abitmap(4, 4);
		if(b->load(path))
			return b;
		delete(b);
	}
	return null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aomppwrap::Aomppwrap(QIID qiid, char *name, AomppwrapInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	xmlOK=false;
	char	nm[1024];
	this->mpi=info->mpi;
	sprintf(nm, "%s module", name);
	host=new OmppMediaHost(this);
	ompp=mpi->getNewInstance(host);
	getMediaInfo();
	if(info->xml)
	{
		xmlNode *root=xmlDocGetRootElement(info->xml);
		xmlNode	*node=root->xmlChildrenNode;
		while(node)
		{
			if(!strcmp((char *)node->name, "interface"))
			{
				if(xmlAttribTest(node, "host", "elektronika"))
				{
					char	*size=xmlAttrib(node, "size");
					xmlNode	*skin=xmlParse(node, "skin");
					if(skin&&size)
					{
						char	*virgule=strchr(size, ',');
						char	*fnfront=xmlAttrib(skin, "front");
						char	*fnback=xmlAttrib(skin, "back");
						if(fnfront&&fnback)
						{
							Abitmap	*bmpfront=getBitmap(info->dll, fnfront);
							Abitmap *bmpback=getBitmap(info->dll, fnback);
							if(virgule&&bmpfront&&bmpback)
							{
								int		heigth=atoi(virgule+1);
								front=new AomppwrapFrontXml(qiid, name, this, heigth, bmpfront, node);
								front->setTooltips(nm);
								back=new AomppwrapBack(qiid, name, this, heigth, bmpback);
								back->setTooltips(nm);
								xmlOK=true;
								break;
							}
							if(bmpfront)
								delete(bmpfront);
							if(bmpback)
								delete(bmpback);
						}
					}
				}
			}
			node=node->next;
		}
	}
	if(!xmlOK)
	{
		front=new AomppwrapFront(qiid, name, this, 48);
		front->setTooltips(nm);
		back=new AomppwrapBack(qiid, name, this, 48);
		back->setTooltips(nm);
	}
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aomppwrap::~Aomppwrap()
{
	if(ompp)
		delete(ompp);
	delete(host);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aomppwrap::load(class Afile *f)
{
	while(safemagictest(f, magicMPMEDIA))
	{
		int		imedia=-1;
		dword	type;
		qword	guid;

		f->read(&type, sizeof(type));
		f->read(&guid, sizeof(guid));

		{
			int i;
			for(i=0; i<nmedia; i++)
			{
				if(((media[i].media.type&MPmaskTYPE)==type)&&(media[i].media.guid==guid))
				{
					imedia=i;
					break;
				}
			}
		}

		switch(type)
		{
			case MPmediaSELECT:
			case MPmediaSELECTVIEW:
			{
				int		nb;
				char	*str;
				int		i;

				f->read(&nb, sizeof(nb));
				for(i=0; i<nb; i++)
				{
					char	abspath[ALIB_PATH];
					f->readStringAlloc(&str);
					if(f->absoluPath(abspath, str))
					{
						free(str);
						str=strdup(abspath);
					}

					if(imedia>=0)
						((MediaSelect *)media[imedia].buffer)->setDesc(i, str);
					free(str);
				}
			}
			break;

			default:
			assert(false);
			break;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aomppwrap::save(class Afile *f)
{
	int i;
	for(i=0; i<nmedia; i++)
	{
		switch(media[i].media.type&MPmaskTYPE)
		{
			case MPmediaSELECT:
			case MPmediaSELECTVIEW:
			{
				MediaSelect	*msv=(MediaSelect *)media[i].buffer;
				dword	magic=magicMPMEDIA;
				dword	type=media[i].media.type&MPmaskTYPE;
				qword	guid=media[i].media.guid;
				int		nb=msv->getMaxValue()+1;
				
				int		i;
				f->write(&magic, sizeof(magic));
				f->write(&type, sizeof(type));
				f->write(&guid, sizeof(guid));

				f->write(&nb, sizeof(nb));
				for(i=0; i<nb; i++)
				{
					char *str=msv->getDesc(i);
					if(str)
					{
						char	relpath[ALIB_PATH];
						if(f->relativePath(relpath, str))
							f->writeString(relpath);
						else
							f->writeString(str);
					}
					else
						f->writeString("");
				}
			}
			break;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aomppwrap::loadPreset(class Afile *f)
{
	while(safemagictest(f, magicMPMEDIA))
	{
		int		imedia=-1;
		dword	type;
		qword	guid;

		f->read(&type, sizeof(type));
		f->read(&guid, sizeof(guid));

		{
			int i;
			for(i=0; i<nmedia; i++)
			{
				if(((media[i].media.type&MPmaskTYPE)==type)&&(media[i].media.guid==guid))
				{
					imedia=i;
					break;
				}
			}
		}

		switch(type)
		{
			case MPmediaSELECT:
			case MPmediaSELECTVIEW:
			{
				int		nb;
				int		i;
				f->read(&nb, sizeof(nb));
				for(i=0; i<nb; i++)
				{
					char	fn[ALIB_PATH];
					int		n;
					f->read(&n, sizeof(n));
					if(getPresetFile(n, fn))
					{
						if(imedia>=0)
							((MediaSelect *)media[imedia].buffer)->setDesc(i, fn);
					}
					else
						if(imedia>=0)
							((MediaSelect *)media[imedia].buffer)->setDesc(i, "");
				}
			}
			break;

			default:
			assert(false);
			break;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aomppwrap::savePreset(class Afile *f)
{
	int i;
	for(i=0; i<nmedia; i++)
	{
		switch(media[i].media.type&MPmaskTYPE)
		{
			case MPmediaSELECT:
			case MPmediaSELECTVIEW:
			{
				MediaSelect	*msv=(MediaSelect *)media[i].buffer;
				dword	magic=magicMPMEDIA;
				dword	type=media[i].media.type&MPmaskTYPE;
				qword	guid=media[i].media.guid;
				int		nb=msv->getMaxValue()+1;
				
				int		i;
				f->write(&magic, sizeof(magic));
				f->write(&type, sizeof(type));
				f->write(&guid, sizeof(guid));

				f->write(&nb, sizeof(nb));
				for(i=0; i<nb; i++)
				{
					char	*str=msv->getDesc(i);
					int		n=-1;
					if(str)
						n=addPresetFile(str);
					f->write(&n, sizeof(n));
				}
			}
			break;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aomppwrap::settings(bool emergency)
{
	int				i;
	for(i=0; i<nmedia; i++)
	{
		switch(media[i].media.type&MPmaskTYPE)
		{
			case MPmediaBITMAP_XRGB:
			{
				MediaBitmap	*mb=(MediaBitmap *)media[i].buffer;
				mb->lock(__FILE__,__LINE__);
				mb->size(getVideoWidth(), getVideoHeight());
				mb->unlock();
			}
			break;

			case MPmediaSELECTVIEW:
			{
				MediaSelectView	*msv=(MediaSelectView *)media[i].buffer;
				msv->lock(__FILE__,__LINE__);
				msv->size(getVideoWidth(), getVideoHeight());
				msv->unlock();
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Aomppwrap::priority()
{
	switch(mpi->getPriority())
	{
		case MPpriorityIMPORT:
		return priorityIMPORT;
		case MPpriorityEXPORT:
		return priorityEXPORT;
	}
	return priorityFILTER;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aomppwrap::actionStart(double time)
{
	ompp->start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aomppwrap::actionStop()
{
	ompp->stop();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aomppwrap::action(double time, double dtime, double beat, double dbeat)	
{
	AomppwrapBack	*back=(AomppwrapBack *)this->back;
	int				i;
	
	for(i=0; i<nmedia; i++)
	{
		if((media[i].media.type&MPmaskDIR)&MPmediaIN)
		{
			switch(media[i].wrap)
			{
				case wrapPAD:
				if(!xmlOK)
				{
					media[i].buffer->lock(__FILE__,__LINE__);
					{
						AomppwrapFront	*front=(AomppwrapFront *)this->front;
						((MediaFloat *)media[i].buffer)->setValue(front->pad[media[i].id]->get());
					}
					media[i].buffer->unlock();
				}
				break;
				
				case wrapPIN:
				switch(media[i].media.type&MPmaskTYPE)
				{
					case MPmediaBITMAP_XRGB:
					media[i].buffer->lock(__FILE__,__LINE__);
					{
						Avideo		*vid=(Avideo *)back->pin[media[i].id];
						Abitmap		*bmp=(Abitmap *)vid->getBitmap();
						if(bmp)
						{
							MediaBitmap	*mb=(MediaBitmap *)media[i].buffer;
							int			w=mb->getWidth();
							int			h=mb->getHeight();
							int			sz=w*h*sizeof(dword);
							assert((w==bmp->w)&&(h==bmp->h));
							memcpy(mb->getPixels(), bmp->body, sz);
						}
						
					}
					media[i].buffer->unlock();
					break;
					
					case MPmediaSAMPLE_PCM_STEREO:
					break;
				}
				break;
			}
		}
	}
	
	ompp->process((double)time/1000.0, beat);

	for(i=0; i<nmedia; i++)
	{
		if((media[i].media.type&MPmaskDIR)&MPmediaOUT)
		{
			switch(media[i].wrap)
			{
				case wrapPIN:
				switch(media[i].media.type&MPmaskTYPE)
				{
					case MPmediaBITMAP_XRGB:
					{
						Avideo		*vid=(Avideo *)back->pin[media[i].id];
						Abitmap		*bmp=(Abitmap *)vid->getBitmap();
						if(bmp)
						{
							MediaBitmap	*mb=(MediaBitmap *)media[i].buffer;
							int			w=mb->getWidth();
							int			h=mb->getHeight();
							int			sz=w*h*sizeof(dword);
							assert((w==bmp->w)&&(h==bmp->h));
							memcpy(bmp->body, mb->getPixels(), sz);
						}
					}
					break;
					
					case MPmediaSAMPLE_PCM_STEREO:
					break;
				}
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AinfoOMPP * Aomppwrap::getInfo(int type, char *name)
{
	int	i;
	for(i=0; i<nmedia; i++)
	{
		if(((media[i].media.type&MPmaskTYPE)==type)&&(!strcmp(media[i].media.name, name)))
			return &media[i];
	}
	return null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aomppwrap::getMediaInfo()
{
	int		npad=0;
	int		npin=0;
	int		i;
	
	nmedia=mini(mpi->getMediaCount(), MAXOMPPMEDIA);
	
	for(i=0; i<nmedia; i++)
	{
		MediaInfo	mi=mpi->getMediaInfo(i);
		media[i].wrap=wrapUNDEFINED;
		switch(mi.type&MPmaskTYPE)
		{
			case MPmediaFLOAT:
			{
				media[i].wrap=wrapPAD;
				media[i].media=mi;
				media[i].id=npad++;
				media[i].buffer=new MediaFloat(mi);
				ompp->link(i, media[i].buffer);
			}
			break;

			case MPmediaINTEGER:
			if(mi.type&MPmediaIN)
			{
				media[i].wrap=wrapINTEGER;
				media[i].media=mi;
				media[i].id=0;
				media[i].buffer=new MediaInteger(mi);
				ompp->link(i, media[i].buffer);
			}
			break;

			case MPmediaSAMPLE_PCM_STEREO:
			break;
			
			case MPmediaBITMAP_XRGB:
			if(npin<MAXOMPPPIN)
			{
				media[i].wrap=wrapPIN;
				media[i].media=mi;
				media[i].id=0;
				media[i].buffer=new MediaBitmap(getVideoWidth(), getVideoHeight());
				ompp->link(i, media[i].buffer);
			}
			break;

			case MPmediaSELECTVIEW:
			{
				media[i].wrap=wrapSELECT;
				media[i].media=mi;
				media[i].id=0;
				media[i].buffer=new MediaSelectView(((MediaInteger *)mi.defvalue)->getValue(), ((MediaInteger *)mi.maxvalue)->getValue(), getVideoWidth(), getVideoHeight());
				ompp->link(i, media[i].buffer);
			}
			break;
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

AomppwrapFrontXml::AomppwrapFrontXml(QIID qiid, char *name, Aomppwrap *e, int h, Abitmap *b, xmlNode *xml) : AeffectFront(qiid, name, e, h)
{
	MediaPluginInfo	*mpi=e->mpi;
	back=b;
	memset(trans, 0, sizeof(trans));
	ntrans=0;
	{
		xmlNode *n=xml->xmlChildrenNode;
		while (n != NULL) 
		{
			if(!strcmp((char *)n->name, "button"))
			{
				char	*name=xmlAttrib(n, "name");
				char	*image=xmlAttrib(n, "image");
				char	*loc=xmlAttrib(n, "location");
				char	*link=xmlAttrib(n, "link");
				char	*help=xmlAttrib(n, "help");
				if(name&&image&&loc&&link)
				{
					char		*locy=strchr(loc, ',');
					AinfoOMPP	*info=e->getInfo(MPmediaINTEGER, link);
					Abitmap		*bmp=getBitmap(((AomppwrapInfo *)e->info)->dll, image);
					if(locy&&bmp&&info&&(ntrans<MAXTRANSOMPP))
					{
						int		x=atoi(loc);
						int		y=atoi(locy+1);
						ActrlButton *bt=new ActrlButton(MKQIID(qiid, info->media.guid), name, this, x, y, bmp->w/3, bmp->h/2, bmp, Abutton::btBITMAP|Abutton::bt2STATES); 
						if(help)
							bt->setTooltips(help);
						if(info->media.defvalue)
							bt->set(((MediaInteger *)info->media.defvalue)->getValue()?true:false);
						bt->show(true);
						trans[ntrans].object=bt;
						trans[ntrans].info=info;
						ntrans++;
					}
				}
			}
			else if(!strcmp((char *)n->name, "choice"))
			{
				char	*name=xmlAttrib(n, "name");
				char	*image=xmlAttrib(n, "image");
				char	*loc=xmlAttrib(n, "location");
				char	*size=xmlAttrib(n, "size");
				char	*disp=xmlAttrib(n, "disposition");
				char	*link=xmlAttrib(n, "link");
				char	*help=xmlAttrib(n, "help");
				if(name&&image&&loc&&link&&size)
				{
					int			nx=0;
					int			ny=0;
					int			ow=0;
					int			oh=0;
					char		*locy=strchr(loc, ',');
					char		*sizeh=strchr(size, ',');
					char		*dispy=strchr(disp, ',');
					AinfoOMPP	*info=e->getInfo(MPmediaINTEGER, link);
					Abitmap		*bmp=getBitmap(((AomppwrapInfo *)e->info)->dll, image);
					if(locy&&sizeh&&dispy&&bmp&&info&&(ntrans<MAXTRANSOMPP))
					{
						int		x=atoi(loc);
						int		y=atoi(locy+1);
						int		ow=atoi(size);
						int		oh=atoi(sizeh+1);
						int		nx=atoi(disp);
						int		ny=atoi(dispy+1);
						Aselect *bt=new Aselect(MKQIID(qiid, info->media.guid), name, this, x, y, nx, ny, bmp, ow, oh); 
						if(help)
							bt->setTooltips(help);
						if(info->media.defvalue)
							bt->set(((MediaInteger *)info->media.defvalue)->getValue()?true:false);
						bt->show(true);
						trans[ntrans].object=bt;
						trans[ntrans].info=info;
						ntrans++;
					}
				}
			}
			else if(!strcmp((char *)n->name, "knob"))
			{
				char	*name=xmlAttrib(n, "name");
				char	*loc=xmlAttrib(n, "location");
				char	*size=xmlAttrib(n, "size");
				char	*image=xmlAttrib(n, "image");
				char	*link=xmlAttrib(n, "link");
				char	*help=xmlAttrib(n, "help");
				if(name&&image&&loc&&size&&link)
				{
					char		*sizh=strchr(size, ',');
					char		*locy=strchr(loc, ',');
					AinfoOMPP	*info=e->getInfo(MPmediaFLOAT, link);
					Abitmap		*bmp=getBitmap(((AomppwrapInfo *)e->info)->dll, image);
					if(locy&&sizh&&bmp&&info&&(ntrans<MAXTRANSOMPP))
					{
						int		w=atoi(size);
						int		h=atoi(sizh+1);
						int		x=atoi(loc);
						int		y=atoi(locy+1);
						Apaddle *pad=new Apaddle(MKQIID(qiid, info->media.guid), name, this, x, y, w, h, bmp, w, h, paddleY); 
						if(help)
							pad->setTooltips(help);
						if(info->media.defvalue)
							pad->set((float)((MediaFloat *)info->media.defvalue)->getValue());
						pad->show(true);
						trans[ntrans].object=pad;
						trans[ntrans].info=info;
						ntrans++;
					}
				}			
			}
			else if(!strcmp((char *)n->name, "timeline"))
			{
				char	*name=xmlAttrib(n, "name");
				char	*loc=xmlAttrib(n, "location");
				char	*size=xmlAttrib(n, "size");
				//char	*image=xmlAttrib(n, "image");
				char	*lcur=xmlAttrib(n, "current");
				char	*lsta=xmlAttrib(n, "sel-start");
				char	*llen=xmlAttrib(n, "sel-len");
				char	*help=xmlAttrib(n, "help");
				if(name&&loc&&size&&lcur&&lsta&&llen)
				{
					char		*sizh=strchr(size, ',');
					char		*locy=strchr(loc, ',');
					AinfoOMPP	*icur=e->getInfo(MPmediaFLOAT, lcur);
					AinfoOMPP	*ista=e->getInfo(MPmediaFLOAT, lsta);
					AinfoOMPP	*ilen=e->getInfo(MPmediaFLOAT, llen);
					//Abitmap		*bmp=getBitmap(((AomppwrapInfo *)e->info)->dll, image);
					if(locy&&sizh&&icur&&ista&&ilen&&(ntrans+3<=MAXTRANSOMPP))
					{
						int		w=atoi(size);
						int		h=atoi(sizh+1);
						int		x=atoi(loc);
						int		y=atoi(locy+1);
						Atimeline *tl=new Atimeline(MKQIID(qiid, icur->media.guid), name, this, x, y, w, h); 
						if(help)
							tl->setTooltips(help);
						if(icur->media.defvalue)
							tl->setCurrent((float)((MediaFloat *)icur->media.defvalue)->getValue());
						if(ista->media.defvalue)
							tl->setSelBegin((float)((MediaFloat *)ista->media.defvalue)->getValue());
						if(ilen->media.defvalue)
							tl->setSelLength((float)((MediaFloat *)ilen->media.defvalue)->getValue());
						tl->show(true);
						trans[ntrans].object=tl;
						trans[ntrans].info=icur;
						trans[ntrans].data=Atimeline::changeCUR;
						ntrans++;
						trans[ntrans].object=tl;
						trans[ntrans].info=ista;
						trans[ntrans].data=Atimeline::changeSTA;
						ntrans++;
						trans[ntrans].object=tl;
						trans[ntrans].info=ilen;
						trans[ntrans].data=Atimeline::changeLEN;
						ntrans++;
					}
				}				
			}
			else if(!strcmp((char *)n->name, "selectmovie"))
			{
				char	*name=xmlAttrib(n, "name");
				char	*link=xmlAttrib(n, "link");
				char	*loc=xmlAttrib(n, "location");
				char	*size=xmlAttrib(n, "size");
				char	*view=xmlAttrib(n, "view");
				char	*disp=xmlAttrib(n, "disposition");
				char	*help=xmlAttrib(n, "help");
				if(name&&link&&loc&&size&&view&&disp)
				{
					char		*locy=strchr(loc, ',');
					char		*sizeh=strchr(size, ',');
					char		*viewh=strchr(view, ',');
					char		*dispy=strchr(disp, ',');
					AinfoOMPP	*info=e->getInfo(MPmediaSELECTVIEW, link);
					if(locy&&sizeh&&viewh&&dispy&&info)
					{
						int	x=atoi(loc);
						int y=atoi(locy+1);
						int	w=atoi(size);
						int h=atoi(sizeh+1);
						int vw=atoi(view);
						int vh=atoi(viewh+1);
						int dx=atoi(disp);
						int dy=atoi(dispy+1);
						AselectView *sview=new AselectView(MKQIID(qiid, info->media.guid), name, this, x, y, w, h, dx, dy, vw, vh);
						if(help)
							sview->setTooltips(help);
						sview->show(true);
						trans[ntrans].object=sview;
						trans[ntrans].info=info;
						ntrans++;
					}
				}
			}
			n=n->next;
		}
	}
	timer(40);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AomppwrapFrontXml::~AomppwrapFrontXml()
{
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void AomppwrapFrontXml::pulse()
{
	int	i;
	for(i=0; i<ntrans; i++)
	{
		AtransOMPP	*io=&trans[i];
		MediaInfo	*mi=&trans[i].info->media;
		if(mi->type&&MPmediaIN)
		{
			switch(mi->type&MPmaskTYPE)
			{
				case MPmediaFLOAT:
				{
					MediaFloat	*mf=(MediaFloat *)io->info->buffer;
					if(io->object->isCI(&Atimeline::CI))
					{
						Atimeline *tl=(Atimeline *)io->object;
						switch(io->data)
						{
							case Atimeline::changeCUR:
							if(mf->focus()!=MediaBuffer::focusHOST)
							{
								if(!mf->host_updated)
									tl->setCurrent((float)mf->getValue());
							}
							break;

							case Atimeline::changeSTA:
							if(mf->plug_updated)
							{
								tl->setSelBegin((float)mf->getValue());
								mf->plug_updated=false;
							}
							break;

							case Atimeline::changeLEN:
							if(mf->plug_updated)
							{
								tl->setSelLength((float)mf->getValue());
								mf->plug_updated=false;
							}
							break;
						}
					}
					else if(io->object->isCI(&Apaddle::CI))
					{
						Apaddle *pad=(Apaddle *)io->object;
						if(mf->plug_updated)
						{
							pad->set((float)mf->getValue());
							mf->plug_updated=false;
						}
					}
				}
				break;

				case MPmediaINTEGER:
				{
					MediaFloat	*mf=(MediaFloat *)io->info->buffer;
					if(io->object->isCI(&ActrlButton::CI))
					{
						ActrlButton *but=(ActrlButton *)io->object;
						if(mf->plug_updated)
						{
							but->set(mf->getValue()!=0);
							mf->plug_updated=false;
						}
					}
					else if(io->object->isCI(&Aselect::CI))
					{
						Aselect *sel=(Aselect *)io->object;
						if(mf->plug_updated)
						{
							sel->set((int)mf->getValue());
							mf->plug_updated=false;
						}
					}
				}
				break;

				case MPmediaSELECTVIEW:
				{
					MediaSelectView	*msv=(MediaSelectView *)io->info->buffer;
					msv->lock(__FILE__,__LINE__);
					if(io->object->isCI(&AselectView::CI))
					{
						AselectView *sv=(AselectView *)io->object;
						int			max=msv->getMaxValue()+1;
						int			i;
						for(i=0; i<max; i++)
						{
							MediaBitmap *mb=msv->getView(i);
							mb->lock(__FILE__,__LINE__);
							if(mb->plug_updated)
							{
								sv->change(i, mb->getPixels(), mb->getWidth(), mb->getHeight());
								mb->plug_updated=false;
							}
							mb->unlock();
						}
					}
					msv->unlock();
				}
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AomppwrapFrontXml::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AomppwrapFrontXml::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		{
			int i;
			for(i=0; i<ntrans; i++)
			{
				if(trans[i].object==o)
				{
					if(!trans[i].info->buffer->plug_updated)
					{
						if(o->isCI(&ActrlButton::CI))
						{
							MediaInteger	*buffer=(MediaInteger *)trans[i].info->buffer;
							int v=((ActrlButton *)o)->get()?1:0;
							buffer->lock(__FILE__,__LINE__);
							buffer->setValue(v);
							buffer->host_updated=true;
							buffer->unlock();
							return true;
						}
						else if(o->isCI(&Aselect::CI))
						{
							MediaInteger	*buffer=(MediaInteger *)trans[i].info->buffer;
							int v=((Aselect *)o)->get();
							buffer->lock(__FILE__,__LINE__);
							buffer->setValue(v);
							buffer->host_updated=true;
							buffer->unlock();
							return true;
						}
						else if(o->isCI(&Apaddle::CI))
						{
							MediaFloat	*buffer=(MediaFloat *)trans[i].info->buffer;
							float		v=((Apaddle *)o)->get();
							buffer->lock(__FILE__,__LINE__);
							buffer->setValue(v);
							buffer->host_updated=true;
							buffer->unlock();
							return true;
						}
						else if(o->isCI(&Atimeline::CI))
						{
							if(trans[i].data==p)
							{
								MediaFloat	*buffer=(MediaFloat *)trans[i].info->buffer;
								float		v=0.f;
								switch(p)
								{
									case Atimeline::changeCUR:
									v=((Atimeline *)o)->getCurrent();
									break;
									case Atimeline::changeSTA:
									v=((Atimeline *)o)->getSelBegin();
									break;
									case Atimeline::changeLEN:
									v=((Atimeline *)o)->getSelLength();
									break;
								}						
								buffer->lock(__FILE__,__LINE__);
								buffer->setValue(v);
								buffer->host_updated=true;
								buffer->unlock();
								return true;
							}
						}
						else if(o->isCI(&AselectView::CI))
						{
							AselViewChange	*svc=(AselViewChange *)p;
							MediaSelectView	*buffer=(MediaSelectView *)trans[i].info->buffer;
							switch(svc->cmd)
							{
								case AselViewChange::cmdSELECT:
								buffer->lock(__FILE__,__LINE__);
								buffer->setValue(maxi(mini(svc->n, buffer->getMaxValue()), 0));
								buffer->host_updated=true;
								buffer->unlock();
								break;
								case AselViewChange::cmdFILE:
								buffer->lock(__FILE__,__LINE__);
								buffer->setDesc(svc->n, svc->file);
								buffer->unlock();
								break;
							}
							return true;
						}
					}
				}
			}
		}
		break;

		case nyPRESS:
		case nyRELEASE:
		if(o->isCI(&Atimeline::CI))
		{
			if(p==Atimeline::changeCUR)
			{
				int i;
				for(i=0; i<ntrans; i++)
				{
					if(trans[i].object==o)
					{
						MediaFloat	*buffer=(MediaFloat *)trans[i].info->buffer;
						buffer->focus((event==nyPRESS)?MediaBuffer::focusHOST:MediaBuffer::focusNONE);
						return true;
					}
				}
			}
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

AomppwrapFront::AomppwrapFront(QIID qiid, char *name, Aomppwrap *e, int h) : AeffectFront(qiid, name, e, h)
{
	MediaPluginInfo	*mpi=e->mpi;
	Aresobj			o=resource.get(MAKEINTRESOURCE(PNG_OMPPWRAP_FRONT), "PNG");
	back=new Abitmap(&o);
	
	{
		Afont	*tf=new Afont("ompp font", &resource.get(MAKEINTRESOURCE(PNG_OMPPWRAP_FONT), "PNG"));
		tf->set(back, 4, 4, name, 0xffd0d0d0);
		delete(tf);
	}
	
	memset(pad, 0, sizeof(pad));
	memset(sta, 0, sizeof(sta));
	
	{
		Afont	*font=alib.getFont(fontTERMINAL05);
		char	namep[MPSTRINGSIZE];
		int		i;
		int		x=10;
		int		y=10;
		int		npad=0;
		for(i=0; i<e->nmedia; i++)
		{
			if(e->media[i].wrap==wrapPAD)
			{
				int		x=npad*22+140;
				int		y=(npad&1)*8+12;
				int		y2=(npad&1)*33+3;
				float	dv=0.5f;
				
				if(e->media[i].media.defvalue)
						dv=(float)((MediaFloat *)(e->media[i].media.defvalue))->getValue();

				strcpy(namep, e->media[i].media.name);
				strupr(namep);	
					
				pad[npad]=new Apaddle(MKQIID(qiid, e->media[i].media.guid), namep, this, x, y, paddleYOLI16);
				pad[npad]->set(dv);
				sta[npad]=new Astatic("sta[]", this, x-10, y2, 38, 10);
				sta[npad]->setName(namep);
				sta[npad]->set(namep, 0xffc0c0c0, font, Astatic::LEFT);

				if(e->media[i].media.help[0])
				{
					pad[npad]->setTooltips(e->media[i].media.help);
					sta[npad]->setTooltips(e->media[i].media.help);
				}
				else
				{
					pad[npad]->setTooltips(namep);
					sta[npad]->setTooltips(namep);
				}
				pad[npad]->show(TRUE);
				sta[npad]->show(true);
				
				assert(npad==e->media[i].id);
				
				npad++;
				if(npad>=MAXOMPPPAD)
					break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AomppwrapFront::~AomppwrapFront()
{
	int	i;
	delete(back);
	for(i=0; i<MAXOMPPPAD; i++)
	{
		if(pad[i])
			delete(pad[i]);
		if(sta[i])
			delete(sta[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AomppwrapFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AomppwrapFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
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

AomppwrapBack::AomppwrapBack(QIID qiid, char *name, Aomppwrap *e, int h, Abitmap *b) : AeffectBack(qiid, name, e, h)
{
	MediaPluginInfo	*mpi=e->mpi;
	if(b)
		back=b;
	else
	{
		Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_OMPPWRAP_BACK), "PNG");
		back=new Abitmap(&o);
		{
			Afont	*tf=new Afont("ompp font", &resource.get(MAKEINTRESOURCE(PNG_OMPPWRAP_FONT), "PNG"));
			int		wf=tf->getWidth(name);
			tf->set(back, (512-wf)>>1, h-tf->h, name, 0xffd0d0d0);
			delete(tf);
		}
	}

	memset(pin, 0, sizeof(pin));
	
	{
		int		i;
		int		xi=10;
		int		yi=10;
		int		xo=512-18;
		int		yo=10;
		int		nin=0;
		int		nout=0;
		int		npin=0;
		
		for(i=0; i<e->nmedia; i++)
		{
			switch(e->media[i].wrap)
			{
				case wrapPIN:
				{
					char	name[MPSTRINGSIZE];
					char	help[MPSTRINGSIZE];
					if(e->media[i].media.name[0])
						strcpy(name, e->media[i].media.name);
					else
						strcpy(name, "NONAME");
					if(e->media[i].media.help[0])
						strcpy(help, e->media[i].media.help);
					else
						strcpy(help, name);
					switch(e->media[i].media.type&MPmaskDIR)
					{
						case MPmediaIN:
						switch(e->media[i].media.type&MPmaskTYPE)
						{
							case MPmediaBITMAP_XRGB:
							pin[npin]=new Avideo(MKQIID(qiid, e->media[i].media.guid), name, this, pinIN, xi, yi);
							break;
							
							case MPmediaSAMPLE_PCM_STEREO:
							pin[npin]=new Asample(MKQIID(qiid, e->media[i].media.guid), name, this, pinIN, xi, yi);
							break;
						}
						pin[npin]->setTooltips(help);
						pin[npin]->show(TRUE);
						npin++;
						nin++;
						if((nin&8)==0)
						{
							xi=10;
							yi+=20;
						}
						else
							xi+=20;
						break;
						
						case MPmediaOUT:
						switch(e->media[i].media.type&MPmaskTYPE)
						{
							case MPmediaBITMAP_XRGB:
							pin[npin]=new Avideo(MKQIID(qiid, e->media[i].media.guid), name, this, pinOUT, xo, yo);
							break;
							
							case MPmediaSAMPLE_PCM_STEREO:
							pin[npin]=new Asample(MKQIID(qiid, e->media[i].media.guid), name, this, pinOUT, xo, yo);
							break;
						}
						pin[npin]->setTooltips(help);
						pin[npin]->show(TRUE);
						npin++;
						nout++;
						if((nout&8)==0)
						{
							xo=512-18;
							yo+=20;
						}
						else
							xo-=20;
						break;
					}
				}
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AomppwrapBack::~AomppwrapBack()
{
	int	i;
	delete(back);
	for(i=0; i<MAXOMPPPIN; i++)
		if(pin[i])
			delete(pin[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AomppwrapBack::paint(Abitmap *b)
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

AomppwrapInfo::AomppwrapInfo(char *dll, MediaPluginInfo *mpi, char *name, char *ename, char *ehelp) : AeffectInfo(name, mpi->getGUID(), ename, ehelp)
{
	xml=null;
	strcpy(this->dll, dll);
	windll=LoadLibrary(dll);
	this->mpi=mpi;
	checkXml();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AomppwrapInfo::~AomppwrapInfo()
{
	delete(mpi);
	if(xml)
	{
		xmlFreeDoc(xml);
		xml=null;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AomppwrapInfo::checkXml()
{
	char	xmlfile[ALIB_PATH];
	strcpy(xmlfile, dll);
	char *s=strstr(xmlfile, ".dll");
	if(s)
	{
		*s=0;
		strcat(xmlfile, ".xml");
		{
			xmlDocPtr doc=xmlParseFile(xmlfile);
			if(doc!=null)
			{
				xmlNodePtr root=xmlDocGetRootElement(doc);
				if(root!=NULL)
				{
					if(!strcmp((char *)root->name, "openMPP"))
					{
						xml=doc;
						return;
					}
				}
				xmlFreeDoc(doc);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AomppwrapInfo::getEffectClass()
{
	switch(mpi->getGroup())
	{
		case MPgroupINOUT:
		return INOUT;
		case MPgroupMIXER:
		return MIXER;
		case MPgroupPLAYER:
		return PLAYER;
		case MPgroupEFFECT2D:
		return EFFECT2D;
		case MPgroupEFFECT3D:
		return EFFECT3D;
		case MPgroupEFFECTAUDIO:
		return EFFECTAUDIO;
		case MPgroupRENDER:
		return RENDER;
	}
	return GENERIC;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AomppwrapInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Aomppwrap(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void omppwrapInfos(Atable *table, char *dll)
{
	HINSTANCE	hDLL=LoadLibrary(dll);
	if(hDLL)
	{
		MPgetMediaPluginVersion	*getVersion=(MPgetMediaPluginVersion *)GetProcAddress(hDLL, "getMediaPluginVersion");
		if(getVersion)
		{
			if(getVersion()==MPVERSION)
			{
				MPgetMediaPluginInfo	*getPlugz=(MPgetMediaPluginInfo *)GetProcAddress(hDLL, "getMediaPluginInfo");
				if(getPlugz)
				{
					MediaPluginInfo	*mpi=getPlugz();
					if(mpi)
					{
						char	name[MPSTRINGSIZE];
						strcpy(name, "OMPP");
						if(mpi->getName(name))
						{
							char	info[MPSTRINGSIZE+128];
							sprintf(info, "%s info", name);
							if(mpi->getMediaCount()<=MAXOMPPPIN)
								table->add(new AomppwrapInfo(dll, mpi, info, name, name), true);
							else
								delete(mpi);
						}
					}
				}
				else
				{
					// todo: getMediaPluginInfos  (plusieurs modules dans la DLL)
				}
			}
		}
		FreeLibrary(hDLL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
