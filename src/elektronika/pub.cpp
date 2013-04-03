/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PUB.CPP						(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>
#include						"tcpRemote.h"
#include						"resource.h"
#include						"pub.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								ApubInfo::CI	= ACI("ApubInfo",		GUID(0x11111112,0x00000420), &AeffectInfo::CI, 0, NULL);
ACI								Apub::CI		= ACI("Apub",			GUID(0x11111112,0x00000421), &Aeffect::CI, 0, NULL);
ACI								ApubFront::CI	= ACI("ApubFront",		GUID(0x11111112,0x00000422), &AeffectFront::CI, 0, NULL);
ACI								ApubBack::CI	= ACI("ApubBack",		GUID(0x11111112,0x00000423), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void rotate(Abitmap *d, Abitmap *s, int sens)
{
	dword	*sp=&s->body32[0];
	int		x,y;
	int		ww;
	switch(sens)
	{
		case 1:
		ww=d->h-1;
		for(y=0; y<s->h; y++)
		{
			for(x=0; x<s->w; x++)
				d->body32[d->adr[ww-x]+y]=*(sp++);
		}
		break;
		
		case -1:
		ww=d->w-1;
		for(y=0; y<s->h; y++)
		{
			for(x=0; x<s->w; x++)
				d->body32[d->adr[x]+ww-y]=*(sp++);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apub::Apub(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	image=NULL;
	rot=NULL;
	ovr=NULL;
	ovrb=NULL;
	overlay=NULL;
	overlayb=NULL;
	text=NULL;
	textwidth=0;
	memset(filename, 0, sizeof(filename));
	font=alib.getFont(fontROUND20);
	front=new ApubFront(qiid, "pub front", this, 48);
	front->setTooltips("pub module");
	back=new ApubBack(qiid, "pub back", this, 48);
	back->setTooltips("pub module");
	oscText=new AoscNode("text body", this, ",s", "text banner");
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Apub::~Apub()
{
	if(image)
		delete(image);
	if(text)
		delete(text);
	if(rot)
		delete(rot);
	if(ovr)
		delete(ovr);
	if(ovrb)
		delete(ovrb);
	if(overlay)
		delete(overlay);
	if(overlayb)
		delete(overlayb);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apub::load(class Afile *f)
{
	{
		bool	b;
		f->read(&b, sizeof(b));
		if(b)
		{
			char	path[ALIB_PATH];
			char	abspath[ALIB_PATH];
			char	fn[ALIB_PATH];
			f->readString(path);
			if(f->absoluPath(abspath, path))
				strcpy(fn, abspath);
			else
				strcpy(fn, path);
			drop(fn);
		}
	}
	{
		char	*s;
		f->readStringAlloc(&s);
		if(s)
		{
			((ApubFront *)front)->dtext->set(s);
			setText(s);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apub::save(class Afile *f)
{
	if(filename[0])
	{
		char	relpath[ALIB_PATH];
		bool	b=true;
		f->write(&b, sizeof(b));
		if(f->relativePath(relpath, filename))
			f->writeString(relpath);
		else
			f->writeString(filename);
	}
	else
	{
		bool	b=false;
		f->write(&b, sizeof(b));
	}
	f->writeString(((ApubFront *)front)->dtext->caption);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apub::loadPreset(class Afile *f)
{
	{
		int	n=-1;
		f->read(&n, sizeof(n));
		if(n!=-1)
		{
			char	fn[ALIB_PATH];
			if(getPresetFile(n, fn))
				drop(fn);
		}
		else
			filename[0]=0;
	}
	{
		char	*s;
		f->readStringAlloc(&s);
		if(s)
		{
			((ApubFront *)front)->dtext->set(s);
			setText(s);
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apub::savePreset(class Afile *f)
{
	{
		int	n=-1;
		if(filename[0])
			n=addPresetFile(filename);
		f->write(&n, sizeof(n));
	}
	f->writeString(((ApubFront *)front)->dtext->caption);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apub::settings(bool emergency)
{
	if(overlay)
		overlay->size(getVideoWidth(), getVideoHeight());
	else
		overlay=new Abitmap(getVideoWidth(), getVideoHeight());
	if(overlayb)
		overlayb->size(getVideoWidth(), getVideoHeight());
	else
		overlayb=new Abitmap(getVideoWidth(), getVideoHeight());
	if(ovr)
		ovr->size(getVideoWidth(), font->h+8);
	else
		ovr=new Abitmap(getVideoWidth(), font->h+8);
	if(ovrb)
		ovrb->size(getVideoWidth(), font->h+8);
	else
		ovrb=new Abitmap(getVideoWidth(), font->h+8);
	if(rot)
		rot->size(font->h+8, getVideoWidth());
	else
		rot=new Abitmap(font->h+8, getVideoWidth());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apub::midiSysex(Abuffer *b, dword time)
{
	dword magic;
	b->read(&magic, sizeof(magic));
	switch(magic)
	{
		case 0x02030405:
		{
			char	file[ALIB_PATH];
			b->readString(file);
			drop(file);
		}
		return true;
		
		case 0x01020304:
		{
			char	*tt;
			b->readStringAlloc(&tt);
			setText(tt);
			((ApubFront *)front)->dtext->set(tt);
			free(tt);
		}
		return true;
		
		case 0x03040506:
		{
			ApubFront	*front=(ApubFront *)this->front;
			int			mode;
			b->read(&mode, sizeof(mode));
			front->image->set((mode&1)!=0);
			front->text->set((mode&2)!=0);
		}
		break;
		
		case 0x05040302:
		{
			ApubFront	*front=(ApubFront *)this->front;
			int			pos;
			b->read(&pos, sizeof(pos));
			front->position->set(pos);
		}
		break;
		
		case 0x05010302:
		{
			ApubFront	*front=(ApubFront *)this->front;
			int			pos;
			int			mode;

			b->read(&pos, sizeof(pos));
			b->read(&mode, sizeof(mode));

			front->position->set(pos);
			front->image->set((mode&1)!=0);
			front->text->set((mode&2)!=0);
		}
		break;
		
		default:
		//assert(false);
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Apub::oscMessage(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *onode, int action)
{
	switch(action)
	{
		case oscGETVALUE:
		if(onode==oscText)
		{
			if(text)
				reply->add(new AoscString(text));
			else
				reply->add(new AoscString(""));
			return true;	// reply used
		}
		break;

		case oscSETVALUE:
		if(onode==oscText)
		{
			if(msg->fchild&&msg->fchild->isCI(&AoscString::CI))
			{
				char *s=((AoscString *)msg->fchild)->value;
				if(s)
				{
					((ApubFront *)front)->dtext->set(s);
					setText(s);
				}
			}
			return false;	// reply not used
		}
		break;
	}
	return Aeffect::oscMessage(reply, msg, onode, action);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apub::drop(char *s)
{
	ApubFront	*front=(ApubFront *)this->front;
	int			l=strlen(s);
	char		*st=s+l-4;
	strlwr(s);
	if(l>5)
	{
		if(!strcmp(st, ".txt"))
		{
			FILE	*fic=fopen(s, "r");
			if(fic)
			{
				int		lf=0;
				char	c;
				while(fread(&c, 1, 1, fic))
					lf++;
				fseek(fic, 0, SEEK_SET);
				char *tt=(char *)malloc(lf+10);
				fread(tt, lf, 1, fic);
				front->dtext->set(text);
				setText(tt);
				free(tt);
				fclose(fic);
				front->dtext->repaint();
			}
			else
				notify(this, nyERROR, (dword)"can't open file");
		}
		else if((!strcmp(st, ".png"))||(!strcmp(st, ".jpg"))||(!strcmp(st-1, ".jpeg")))
		{
			section.enter(__FILE__,__LINE__);
			if(image)
				delete(image);
			image=new Abitmap(s);
			front->snap->set(0, 0, front->snap->w, front->snap->h, 0, 0, image->w, image->h, image, bitmapNORMAL, bitmapNORMAL);
			section.leave();
			strcpy(filename, s);
			front->repaint();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apub::setText(char *s)
{
	section.enter(__FILE__,__LINE__);
	if(text)
		free(text);
	text=(char *)malloc(strlen(s)+1);
	strcpy(text, s);
	ptime=getTable()->getTime();
	textwidth=font->getWidth(text)+getVideoWidth();
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Apub::action(double time, double dtime, double beat, double dbeat)
{
	ApubFront		*front=(ApubFront *)this->front;
	ApubBack		*back=(ApubBack *)this->back;
	Abitmap			*bb=back->out->getBitmap();
	Abitmap			*bin=back->in->getBitmap();
	bool			bimage=front->image->get();
	bool			btext=front->text->get();
	int				mode=front->position->get();

	if(bb)
	{
		if(bin)
			bb->set(0, 0, bin, bitmapNORMAL, bitmapNORMAL);
		else
			bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
		section.enter(__FILE__,__LINE__);
		if(bimage&&image)
			bb->set(0, 0, bb->w, bb->h, 0, 0, image->w, image->h, image, bitmapDEFAULT, bitmapDEFAULT);
		if(btext&&text)
		{
			double	t=time-ptime;
			int		p=(int)(t*bb->w/3000);
			if(p>(textwidth+bb->w))
				ptime=time;

			if(mode!=5)
			{
				char	str[32768];
				char	*s=str;
				strncpy(str, text, 32768);
				str[32767]=0;
				while(*s)
				{
					switch(*s)
					{
						case 13:
						case 10:
						*s=32;
						break;
					}
					s++;
				}
				ovr->boxf(0, 0, ovr->w, ovr->h, 0x00000000);
				font->set(ovr, ovr->w-p, 4, str, 0xffffffff);
				ovrb->ablur(ovr, 3.f, 3.f);
				ovr->flags=bitmapALPHA;
				ovrb->set(0, 0, ovr, bitmapDEFAULT, bitmapDEFAULT);
				ovrb->flags=bitmapALPHA;
			}
			
			switch(mode)
			{
				case 0:
				bb->set(0, (bb->h>>4), ovrb, bitmapDEFAULT, bitmapDEFAULT);
				break;
				
				case 1:
				bb->set(0, (bb->h-ovrb->h)/2, ovrb, bitmapDEFAULT, bitmapDEFAULT);
				break;
				
				case 2:
				bb->set(0, bb->h-((bb->h>>4)+font->h), ovrb, bitmapDEFAULT, bitmapDEFAULT);
				break;
				
				case 3:
				rotate(rot, ovrb, 1);
				rot->flags=bitmapALPHA;
				bb->set(0, 0, rot, bitmapDEFAULT, bitmapDEFAULT);
				break;
				
				case 4:
				rotate(rot, ovrb, -1);
				rot->flags=bitmapALPHA;
				bb->set(bb->w-rot->w, 0, rot, bitmapDEFAULT, bitmapDEFAULT);
				break;
				
				case 5:
				{
					Arectangle	r=font->getRectangle(0, 0, text);
					int			x=(bb->w-r.w)>>1;
					int			y=(bb->h-r.h)>>1;
					
					overlay->boxf(0, 0, overlay->w, overlay->h, 0x00000000);
					font->setAlign(overlay, x, y, r.w, text, 0xffffffff);
					overlayb->ablur(overlay, 3.f, 3.f);
					overlay->flags=bitmapALPHA;
					overlayb->set(0, 0, overlay, bitmapDEFAULT, bitmapDEFAULT);
					overlayb->flags=bitmapALPHA;
					bb->set(0, 0, overlayb, bitmapDEFAULT, bitmapDEFAULT);
				}
				break;
			}
			//font->set(ovr, ovr->w-p, ovr->h-((bb->h>>4)+font->h), text, 0xffffffff, 0x20000000, fontBORDER);
		}
		section.leave();
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

ApubFront::ApubFront(QIID qiid, char *name, Apub *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_PUB_FRONT), "PNG");
	back=new Abitmap(&o);
	
	snap=new Abitmap(56, 40);
	
	image=new ActrlButton(MKQIID(qiid, 0xf58d202613412888), "image", this, 96, 16, 40, 16, &resource.get(MAKEINTRESOURCE(PNG_PUB_IMAGE), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	image->setTooltips("display image");
	image->show(true);

	text=new ActrlButton(MKQIID(qiid, 0xfeec3850be801209), "text", this, 368, 16, 40, 16, &resource.get(MAKEINTRESOURCE(PNG_PUB_TEXT), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	text->setTooltips("display text");
	text->show(true);
	
	dtext=new Aedit("text", this, 164, 4, 184, 40);
	dtext->colorBackground=0x80ffffff;
	dtext->setTooltips("text");
	dtext->state|=Aedit::stateMULTILINE;
	dtext->show(true);
	
	position=new AselButton("position", this, 432, 8, 3, 2, &resource.get(MAKEINTRESOURCE(PNG_PUB_TEXT_POS), "PNG"), 16, 16);
	position->setTooltips("text position");
	position->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ApubFront::~ApubFront()
{
	delete(back);
	delete(image);
	delete(text);
	delete(snap);
	delete(dtext);
	delete(position);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ApubFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	b->set(20,4, snap, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ApubFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==text)
		{
			if(text->get())
				((Apub *)effect)->setText(dtext->caption);
		}
		else if(o==position)
		{
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ApubFront::dragdrop(int x, int y, int state, int event, void *data)
{
	switch(event)
	{
		case dragdropENTER:
		case dragdropOVER:
		if(state&dragdropFILE)	
			return true;
		break;
		
		case dragdropDROP:
		if(state&dragdropFILE)
		{
			Apub	*pub=(Apub *)effect;
			char	*s=(char *)data;
			while(*s)
			{
				pub->drop(s);
				s+=strlen(s)+1;
			}
			return true;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ApubBack::ApubBack(QIID qiid, char *name, Apub *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_PUB_BACK), "PNG");
	back=new Abitmap(&o);

	in=new Avideo(MKQIID(qiid,0x184af6d1cede8d94), "video in", this, pinIN, 10, 10);
	in->setTooltips("video in");
	in->show(TRUE);
	
	out=new Avideo(MKQIID(qiid,0xd59faf06a376b03a), "video out", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video out");
	out->show(TRUE);
}

///////////////////////////////////////////////////////////////lo//////////////////////////////////////////////////////////

ApubBack::~ApubBack()
{
	delete(back);
	delete(in);
	delete(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ApubBack::paint(Abitmap *b)
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

Aeffect * ApubInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Apub(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * pubGetInfo()
{
	return new ApubInfo("pubInfo", &Apub::CI, "pub", "pub module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
