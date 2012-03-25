/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TRANS-VIDEO.CPP				(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"trans-video.h"
#include						"resource.h"
#include						<math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AtransvideoInfo::CI	= ACI("AtransvideoInfo",	GUID(0x11111112,0xA0000050), &AtransitionInfo::CI, 0, NULL);
ACI								Atransvideo::CI		= ACI("Atransvideo",		GUID(0x11111112,0xA0000051), &Atransition::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Atransvideo::Atransvideo(QIID qiid, char *name, Aobject *f, int x, int y) : Atransition(qiid, name, f, x, y)
{
	char	str[1024];
	init();
	
	{
		Aeffect *e=Aeffect::getEffect(this);
		int		w=e->getVideoWidth();
		int		h=e->getVideoHeight();
		sources=new Abitmap(w*2, h);
		//blur[0]=new Abitmap(w, h);
		//blur[1]=new Abitmap(w, h);
		//norm=new Abitmap(w, h);
	}
	//cblur=0;

	sprintf(str, "%s/mask", name);
	mask=new Aselect(MKQIID(qiid, 0xdb184a84d85e21f0), str, this, 14, 12, 4, 2, &resource.get(MAKEINTRESOURCE(PNG_VMIX_SOURCES), "PNG"), 16, 16);
	mask->setTooltips("mask");
	mask->set(0);
	mask->show(TRUE);
	
	sprintf(str, "%s/channel", name);
	colormix=new Aselect(MKQIID(qiid, 0xa0f67698be07801a), str, this, 97, 15, 1, 3, &resource.get(MAKEINTRESOURCE(PNG_AUTOMIX_COLOR), "PNG"), 8, 8);
	colormix->setTooltips("channel");
	colormix->set(1);
	colormix->show(TRUE);
	/*
	powersrc=new Apaddle(MKQIID(qiid, 0x6581405aed690822), "source level", this, 20, 60, paddleYOLI24);
	powersrc->setTooltips("source level");
	powersrc->set(1.0f);
	powersrc->show(TRUE);
	
	powerblur=new Apaddle(MKQIID(qiid, 0x658a05ac5d6e90e2), "blur level", this, 50, 60, paddleYOLI24);
	powerblur->setTooltips("blur level");
	powerblur->set(0.f);
	powerblur->show(TRUE);
	
	powernorm=new Apaddle(MKQIID(qiid, 0x658149055a05de22), "normalize level", this, 80, 60, paddleYOLI24);
	powernorm->setTooltips("normalize level");
	powernorm->set(0.f);
	powernorm->show(TRUE);
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Atransvideo::~Atransvideo()
{
	delete(mask);
	delete(colormix);
	//delete(powersrc);
	//delete(powerblur);
	//delete(powernorm);
	//delete(blur[0]);
	//delete(blur[1]);
	//delete(norm);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransvideo::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atransvideo::paint(Abitmap *b)
{
	b->boxfa(8, 8, 112, 50, 0xff000040, 0.4f);
	b->boxf(92, 12, 108, 46, 0xff202020);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransvideo::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransvideo::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Atransvideo::settings(bool emergency)
{
	Aeffect *e=Aeffect::getEffect(this);
	int		w=e->getVideoWidth();
	int		h=e->getVideoHeight();
	sources->size(w*2, h);
	//blur[0]->size(w, h);
	//blur[1]->size(w, h);
	//norm->size(w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Atransvideo::action(Abitmap *bb, Abitmap *b0, Abitmap *b1, float cross, int src, float power, Abitmap *in[8])
{
	Abitmap	*bmix=in[mask->get()];
	int		w=bb->w;
	int		h=bb->h;
	int		colormix=this->colormix->get();
	
/**	
	float	powersrc=this->powersrc->get();
	float	powerblur=this->powerblur->get();
	float	powernorm=this->powernorm->get();
	
	
	int		min=0;
	int		max=255;
	
	if((bmix)&&(powersrc<1.f))
	{
		float	a=bmix->alpha;
		bmix->alpha=powersrc;
		blur[cblur]->set(0, 0, bmix, bitmapNORMAL, bitmapGLOBALALPHA);
		bmix->alpha=a;
		bmix=blur[cblur];
	}
	if(powerblur>0.f)
	{
		if(!bmix)
			bmix=blur[cblur];
		cblur=(cblur==0)?1:0;
		blur[cblur]->blur(bmix, powerblur*5.f, powerblur*5.f);
		bmix=blur[cblur];
	}
	if((bmix)&&(powernorm>0.f))
	{
		int	x,y;
		min=255;
		max=0;
		switch(colormix)
		{
			case 0:		// red
			for(y=0; y<bmix->h; y++)
			{
				dword	*ss=&bmix->body32[bmix->adr[y]];
				for(x=0; x<bmix->w; x++)
				{
					int	v=(*(ss++)>>16)&255;
					min=mini(v, min);
					max=maxi(v, max);
				}
			}
			break;
			
			case 1:		// green
			for(y=0; y<bmix->h; y++)
			{
				dword	*ss=&bmix->body32[bmix->adr[y]];
				for(x=0; x<bmix->w; x++)
				{
					int	v=(*(ss++)>>8)&255;
					min=mini(v, min);
					max=maxi(v, max);
				}
			}
			break;
			
			case 2:		// blue
			for(y=0; y<bmix->h; y++)
			{
				dword	*ss=&bmix->body32[bmix->adr[y]];
				for(x=0; x<bmix->w; x++)
				{
					int	v=*(ss++)&255;
					min=mini(v, min);
					max=maxi(v, max);
				}
			}
			break;
		}
		min=(int)(powernorm*(float)min);
		max=(int)(powernorm*(float)max+(1.f-powernorm)*255.f);
	}
	*/
	if(bmix)
	{
		int		tab[256];

		if(power<0.5f)
		{
			{
				float	fade=power*2.f;
				float	seuil=cross;
				
				if(b1&&(!b0))	// que la source droite -> inverser
					seuil=1.f-seuil;

				float	dseuil=(float)fabs(seuil-0.5f)*2.f;
				float	idseuil=1.f-dseuil;
				int		i;
				
				//float	dnorm=(max-min)/255.f;
				
				for(i=0; i<256; i++)
				{
					float	fi=(float)i/255.f;
					float	ff=(seuil*(1.f+fade))-(fade*0.5f);
					float	a=((fi-seuil)*(fade*fade*100.f+1.f)+0.5f)*idseuil+((1.f-seuil)*dseuil);
					tab[i]=maxi(mini((int)(a*256.f), 256), 0);
				}

				if(src==1)
				{
					int	i;
					//int	dm=(max+min)>>1;
					for(i=0; i<128; i++)
					{
						int	n=tab[i];
						tab[i]=tab[255-i];
						tab[255-i]=n;
					}
				}
			}
			
			if(b0&&b1)
			{
				int	x,y;
				for(y=0; y<h; y++)
				{
					int		xx=0;
					int		adr=bb->adr[y];
					dword	*ss=&bmix->body32[adr];
					dword	*s0=&b0->body32[adr];
					dword	*s1=&b1->body32[adr];
					dword	*dd=&bb->body32[adr];

					switch(colormix)
					{
						case 0:	// red
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>16)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							dword	c1=*(s1++);
							byte	rs0,gs0,bs0;
							byte	rs1,gs1,bs1;
							colorRGB(&rs0, &gs0, &bs0, c0);
							colorRGB(&rs1, &gs1, &bs1, c1);
							*(dd++)=color32(((int)rs0*v+(int)rs1*v0)>>8, ((int)gs0*v+(int)gs1*v0)>>8, ((int)bs0*v+(int)bs1*v0)>>8);
						}
						break;

						case 1:	// green
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>8)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							dword	c1=*(s1++);
							byte	rs0,gs0,bs0;
							byte	rs1,gs1,bs1;
							colorRGB(&rs0, &gs0, &bs0, c0);
							colorRGB(&rs1, &gs1, &bs1, c1);
							*(dd++)=color32(((int)rs0*v+(int)rs1*v0)>>8, ((int)gs0*v+(int)gs1*v0)>>8, ((int)bs0*v+(int)bs1*v0)>>8);
						}
						break;

						case 2:	// blue
						for(x=0; x<w; x++)
						{
							int		v=tab[*(ss++)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							dword	c1=*(s1++);
							byte	rs0,gs0,bs0;
							byte	rs1,gs1,bs1;
							colorRGB(&rs0, &gs0, &bs0, c0);
							colorRGB(&rs1, &gs1, &bs1, c1);
							*(dd++)=color32(((int)rs0*v+(int)rs1*v0)>>8, ((int)gs0*v+(int)gs1*v0)>>8, ((int)bs0*v+(int)bs1*v0)>>8);
						}
						break;
					}
				}
			}
			else if(b0||b1)
			{
				//int		n=b0?0:1;
				Abitmap	*b=b0?b0:b1;
				int		x,y;
				
				for(y=0; y<h; y++)
				{
					int		xx=0;
					int		adr=bb->adr[y];
					dword	*ss=&bmix->body32[adr];
					dword	*s0=&b->body32[adr];
					dword	*dd=&bb->body32[adr];

					switch(colormix)
					{
						case 0:	// red
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>16)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							byte	rs0,gs0,bs0;
							colorRGB(&rs0, &gs0, &bs0, c0);
							*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
						}
						break;

						case 1:	// green
						for(x=0; x<w; x++)
						{
							int		v=tab[(*(ss++)>>8)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							byte	rs0,gs0,bs0;
							colorRGB(&rs0, &gs0, &bs0, c0);
							*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
						}
						break;

						case 2:	// blue
						for(x=0; x<w; x++)
						{
							int		v=tab[*(ss++)&255];
							int		v0=256-v;
							dword	c0=*(s0++);
							byte	rs0,gs0,bs0;
							colorRGB(&rs0, &gs0, &bs0, c0);
							*(dd++)=color32(((int)rs0*v)>>8, ((int)gs0*v)>>8, ((int)bs0*v)>>8);
						}
						break;
					}
				}
			}
			else
				bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
		}
		else
		{
			Abitmap	*out=bb;
			Abitmap	*wipe=bmix;
			bool	bInv=(src!=0);
			float	fade=1.f-((power-0.5f)*2.f);
			float	seuil=bInv?(1.f-cross):cross;
			float	dseuil=(float)fabs(seuil-0.5f)*2.f;
			float	idseuil=1.f-dseuil;
			float	corec=(float)256-(float)fade;
			Abitmap	*b=sources;
			int		x,y;
			int		x0,x1;
			int		i;
			
			if(bInv)
			{
				x0=0;
				x1=out->w;
			}
			else
			{
				x0=out->w;
				x1=0;
			}
			
			//float	dnorm=(max-min)/255.f;

			for(i=0; i<256; i++)
			{
				float	fi=(float)i/255.f;
				float	ff=(seuil*(1.f+fade))-(fade*0.5f);
				float	a=((fi-seuil)*(fade*fade*100.f+1.f)+0.5f)*idseuil+((1.f-seuil)*dseuil);
				tab[i]=maxi(mini((int)(a*(float)out->w), out->w), 0);
			}
			
			if(b0)
				b->set(x0, 0, b0, bitmapNORMAL, bitmapNORMAL);
			else
				b->boxf(x0, 0, out->w, out->h, 0xff000000);
			if(b1)
				b->set(x1, 0, b1, bitmapNORMAL, bitmapNORMAL);
			else
				b->boxf(x1, 0, out->w*2, out->h, 0xff000000);
			
			switch(colormix)
			{
				case 0:	// red
				for(y=0; y<h; y++)
				{
					dword	*ss=&wipe->body32[wipe->adr[y]];
					dword	*s0=&b->body32[b->adr[y]];
					dword	*dd=&out->body32[out->adr[y]];

					for(x=0; x<w; x++)
					{
						int		v=tab[(*(ss++)>>16)&255];
						*(dd++)=s0[x+v];
					}
				}
				break;

				case 1:	// green
				for(y=0; y<h; y++)
				{
					dword	*ss=&wipe->body32[wipe->adr[y]];
					dword	*s0=&b->body32[b->adr[y]];
					dword	*dd=&out->body32[out->adr[y]];

					for(x=0; x<w; x++)
					{
						int		v=tab[(*(ss++)>>8)&255];
						*(dd++)=s0[x+v];
					}
				}
				break;

				case 2:	// blue
				for(y=0; y<h; y++)
				{
					dword	*ss=&wipe->body32[wipe->adr[y]];
					dword	*s0=&b->body32[b->adr[y]];
					dword	*dd=&out->body32[out->adr[y]];

					for(x=0; x<w; x++)
					{
						int		v=tab[*(ss++)&255];
						*(dd++)=s0[x+v];
					}
				}
				break;
			}	
		}
	}
	else
	{
		Abitmap	*bin[2];
		float	v[2];
		int		i;
		v[1]=cross;
		v[0]=1.f-v[1];
		bb->boxf(0, 0, bb->w, bb->h, 0xff000000);
		bin[0]=b0;
		bin[1]=b1;
		for(i=0; i<2; i++)
		{
			if(bin[i])
			{
				dword	state=bin[i]->state;
				dword	flags=bin[i]->flags;
				float	alpha=bin[i]->alpha;
				bin[i]->state=bitmapNORMAL;
				bin[i]->flags=bitmapGLOBALALPHA;
				bin[i]->alpha=v[i];
				bb->set(0, 0, bin[i], bitmapDEFAULT, bitmapDEFAULT);
				bin[i]->state=state;
				bin[i]->flags=flags;
				bin[i]->alpha=alpha;
			}
		}
	}	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Atransition * AtransvideoInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Atransvideo(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * transvideoGetInfo()
{
	return new AtransvideoInfo("video", &Atransvideo::CI, "video", "video");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
