/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FONT.CPP					(c)	YoY'99						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"font.h"
#include						"bitmap.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
static Aproperties				properties[]=	
							{	
								{	guidSDWORD,		(int)&(((Afont *)NULL)->w),				"w",			"characters with"		},
								{	guidSDWORD,		(int)&(((Afont *)NULL)->h),				"h",			"characters height"		},
								{	guidSTRINGP,	(int)&(((Afont *)NULL)->characters),	"characters",	"character list"		},
								{	guidBITMAP,		(int)&(((Afont *)NULL)->bitmap),		"bitmap",		"font bitmap"			}
							};
*/
ADLL ACI						Afont::CI=ACI("Afont", GUID(0xAE57E515,0x00000110), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Afont::NCcalcChar(int min)
{
	int	nbc=characters?strlen((char *)characters):256;
	charInfo=(ATcharInfo *)malloc(sizeof(ATcharInfo)*nbc);
	if(!characters)
	{
		w=bitmap->w>>4;
		h=bitmap->h>>4;
	}
	switch(bitmap->nbBits)
	{
		case 8:
		{
			int		n;
			for(n=0; n<nbc; n++)
			{
				int		x0=(n%(bitmap->w/w))*w;
				int		y0=(n/(bitmap->w/w))*h;
				int		x,y;
				for(x=0; x<w; x++)
				{
					for(y=0; y<h; y++)
						if(bitmap->body8[bitmap->adr[y+y0]+x+x0]>min)
							break;
					if(y!=h)
					{
						charInfo[n].b=x;
						break;
					}
				}
				if(x==w)
				{
					charInfo[n].b=0;
					charInfo[n].w=w;
				}
				else
				{
					for(x=w-1; x>charInfo[n].b; x--)
					{
						for(y=0; y<h; y++)
							if(bitmap->body8[bitmap->adr[y+y0]+x+x0]>min)
								break;
						if(y!=h)
						{
							charInfo[n].w=1+x-charInfo[n].b;
							break;
						}
					}
					if(x==charInfo[n].b)
						charInfo[n].w=1;
				}
			}
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Afont::Afont(char *name, char *filename, int w, int h, char *c, int e, int min) : Anode(name)
{
	bitmap=new Abitmap(filename);
	this->w=w;
	this->h=h;
	this->e=e;
	characters=(byte *)strdup(c);
	NCcalcChar(min);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Afont::Afont(char *name, Aresobj *o, int w, int h, char *c, int e, int min) : Anode(name)
{
	bitmap=new Abitmap(o);
	this->w=w;
	this->h=h;
	this->e=e;
	characters=(byte *)strdup(c);
	NCcalcChar(min);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Afont::Afont(char *name, Aresobj *o, int e, int min) : Anode(name)
{
	bitmap=new Abitmap(o);
	this->e=e;
	characters=NULL;
	NCcalcChar(min);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Afont::~Afont()
{
	delete(bitmap);
	if(characters)
		free(characters);
	free(charInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Afont::set(class Abitmap *b, int x, int y, char *s, dword colorText, dword colorBorder, int type)
{
	switch(type)
	{
		case fontBORDER:
		{
			bool	t=TRUE;
			t&=set(b, x-1, y-1, s, colorBorder);
			t&=set(b, x+1, y-1, s, colorBorder);
			t&=set(b, x-1, y+1, s, colorBorder);
			t&=set(b, x+1, y+1, s, colorBorder);
			t&=set(b, x, y, s, colorText);
			return t;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NCalign(class Afont *f, class Abitmap *b, int x, int y, int w, char *s, dword color, int align)
{
	switch(align)
	{
		case Afont::alignCENTER:
		{
			int	wf=f->getWidth(s);
			f->set(b, x+(w-wf)/2, y, s, color);
		}
		break;
		
		case Afont::alignLEFT:
		f->set(b, x, y, s, color);
		break;
		
		case Afont::alignRIGHT:
		{
			int	wf=f->getWidth(s);
			f->set(b, x+w-wf, y, s, color);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Afont::setAlign(class Abitmap *b, int x, int y, int w, char *s, dword color, int align)
{
	char	str[10000];
	char	*os=s;
	while(*s)
	{
		if(*s==13)
		{
			int	n=s-os;
			memcpy(str, os, n);
			os=s+1;
			str[n]=0;
			NCalign(this, b, x, y, w, str, color, align);
			y+=this->h;
		}
		s++;
	}
	NCalign(this, b, x, y, w, os, color, align);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Afont::set(class Abitmap *b, int x, int y, char *s0, dword color)	// todo: add the clip
{
	byte	*s=(byte *)s0;
	int		tab=x;

	x+=b->view.x;
	y+=b->view.y;

	if(y>=(b->clip.y+b->clip.h))
		return false;
	while(*s&&(y<b->clip.y-h))
	{
		while(*s&&(*s!=13))
			s++;
		if(*s)
		{
			s++;
			y+=h+e;
		}
	}
	while(*s)
	{
		switch(*s)
		{
			case 10:
			break;

			case 13:
			y+=h+e;
			x=tab+b->view.x;
			if(y>=(b->clip.y+b->clip.h))
				return true;
			break;

			case 32:
			x+=w>>1;
			break;

			default:
			{
				int		n=(int)(*s)&255;
				if(characters)
				{
					byte	*p=(byte *)strchr((char *)characters, *s);
					if(!p)
					{
						x+=w;
						s++;
						continue;
					}
					n=p-characters;
				}
				{
					int		x0=(n%(bitmap->w/w))*w+charInfo[n].b;
					int		y0=(n/(bitmap->w/w))*h;
					int		i,j;
					int		wc=charInfo[n].w;
					int		xx=x;
					int		yy=y;
					int		hc=h;
					x+=e+wc;
					if(xx>=(b->clip.x+b->clip.w))
						break;
					if(xx<b->clip.x)
					{
						int	d=b->clip.x-xx;
						wc-=d;
						x0+=d;
						xx=b->clip.x;
					}
					if(yy<b->clip.y)
					{
						int	d=b->clip.y-yy;
						hc-=d;
						y0+=d;
						yy=b->clip.y;
					}
					wc=mini(xx+wc, b->clip.x+b->clip.w)-xx;
					hc=mini(yy+hc, b->clip.y+b->clip.h)-yy;
					if((wc>0)&&(hc>0))
					{
						switch(b->nbBits)
						{
							case 16:
							break;

							case 32:
							switch(bitmap->nbBits)
							{
								case 8:
								{
									byte	rs,gs,bs;
									colorRGB(&rs, &gs, &bs, color);
									for(j=0; j<hc; j++)
									{
										byte	*si=bitmap->body8+bitmap->adr[y0+j]+x0;
										dword	*di=b->body32+b->adr[yy+j]+xx;
										byte	rd=0,gd=0,bd=0,alpha=0;
										for(i=0; i<wc; i++)
										{
											int		as=*(si++);
											int		ad=255-as;
											colorRGBA(&rd, &gd, &bd, &alpha, *di);
											rd=(byte)(((int)rd*ad+(int)rs*as)>>8);
											gd=(byte)(((int)gd*ad+(int)gs*as)>>8);
											bd=(byte)(((int)bd*ad+(int)bs*as)>>8);
											*(di++)=(dword)((color32((byte)rd, (byte)gd, (byte)bd)&0xffffff)|(maxi(as, alpha)<<24));
										}
									}
								}
								break;

								case 32:
								b->set(xx, yy, wc, hc, x0, y0, wc, hc, bitmap, bitmapDEFAULT, bitmapDEFAULT);
								break;
							}
							break;
						}
					}
				}
			}
			break;
		}
		s++;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Afont::setFixed(class Abitmap *b, int x, int y, char *s0, dword color, int ew)	// todo: add the clip
{
	byte	*s=(byte *)s0;
	int		tab=x;

	x+=b->view.x;
	y+=b->view.y;

	if(y>=(b->clip.y+b->clip.h))
		return false;
	while(*s&&(y<b->clip.y-h))
	{
		while(*s&&(*s!=13))
			s++;
		if(*s)
		{
			s++;
			y+=h+e;
		}
	}
	while(*s)
	{
		switch(*s)
		{
			case 10:
			break;

			case 13:
			y+=h+e;
			x=tab+b->view.x;
			if(y>=(b->clip.y+b->clip.h))
				return true;
			break;

			case 32:
			x+=w+ew;
			break;

			default:
			{
				int		n=(int)(*s)&255;
				if(characters)
				{
					byte	*p=(byte *)strchr((char *)characters, *s);
					if(!p)
					{
						x+=w+ew;
						s++;
						continue;
					}
					n=p-characters;
				}
				{
					int		x0=(n%(bitmap->w/w))*w;
					int		y0=(n/(bitmap->w/w))*h;
					int		i,j;
					int		wc=w;
					int		xx=x;
					int		yy=y;
					int		hc=h;
					x+=wc+ew;
					if(xx>=(b->clip.x+b->clip.w))
						break;
					if(xx<b->clip.x)
					{
						int	d=b->clip.x-xx;
						wc-=d;
						x0+=d;
						xx=b->clip.x;
					}
					if(yy<b->clip.y)
					{
						int	d=b->clip.y-yy;
						hc-=d;
						y0+=d;
						yy=b->clip.y;
					}
					wc=mini(xx+wc, b->clip.x+b->clip.w)-xx;
					hc=mini(yy+hc, b->clip.y+b->clip.h)-yy;
					if((wc>0)&&(hc>0))
					{
						switch(b->nbBits)
						{
							case 16:
							break;

							case 32:
							switch(bitmap->nbBits)
							{
								case 8:
								{
									byte	rs,gs,bs;
									colorRGB(&rs, &gs, &bs, color);
									for(j=0; j<hc; j++)
									{
										byte	*si=bitmap->body8+bitmap->adr[y0+j]+x0;
										dword	*di=b->body32+b->adr[yy+j]+xx;
										byte	rd=0,gd=0,bd=0,alpha=0;
										for(i=0; i<wc; i++)
										{
											int		as=*(si++);
											int		ad=255-as;
											colorRGBA(&rd, &gd, &bd, &alpha, *di);
											rd=(byte)(((int)rd*ad+(int)rs*as)>>8);
											gd=(byte)(((int)gd*ad+(int)gs*as)>>8);
											bd=(byte)(((int)bd*ad+(int)bs*as)>>8);
											*(di++)=(dword)((color32((byte)rd, (byte)gd, (byte)bd)&0xffffff)|(maxi(as, alpha)<<24));
										}
									}
								}
								break;

								case 32:
								b->set(xx, yy, wc, hc, x0, y0, wc, hc, bitmap, bitmapDEFAULT, bitmapDEFAULT);
								break;
							}
							break;
						}
					}
				}
			}
			break;
		}
		s++;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Afont::getWidth(char *s0)
{
	byte	*s=(byte *)s0;
	int		x=0;
	int		y=0;
	int		max=0;

	while(*s)
	{
		switch(*s)
		{
			case 13:
			y+=h+e;
			x=0;
			break;

			case 32:
			x+=w>>1;
			break;

			default:
			if(characters)
			{
				byte	*p=(byte *)strchr((char *)characters, *s);
				if(p)
				{
					int		n=p-characters;
					int		wc=charInfo[n].w;
					x+=e+wc;
				}
				else
					x+=w;
			}
			else
				x+=e+charInfo[((int)*s)&255].w;
			break;
		}
		max=maxi(x, max);
		s++;
	}
	return max;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int Afont::getHeight(char *s0)
{
	byte	*s=(byte *)s0;
	int		x=0;
	int		y=h;
	int		max=0;

	while(*s)
	{
		switch(*s)
		{
			case 13:
			y+=h+e;
			x=0;
			break;

			case 32:
			x+=w>>1;
			break;

			default:
			if(characters)
			{
				byte	*p=(byte *)strchr((char *)characters, *s);
				if(p)
				{
					int		n=p-characters;
					int		wc=charInfo[n].w;
					x+=e+wc;
				}
				else
					x+=w;
			}
			else
				x+=e+charInfo[((int)*s)&255].w;
			break;
		}
		max=maxi(max, y);
		s++;
	}
	return max;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Arectangle Afont::getRectangle(int x, int y, char *s0, int fc, int lc)
{
	byte	*s=(byte *)s0;
	int		xx=0;
	int		yy=h;
	int		maxx=0;
	int		maxy=0;

	if(!((fc==-1)&&(lc==-1)))
	{
		if(fc<0)
			return Arectangle(x, y, 0, h);

		while(*s&&fc)
		{
			fc--;
			lc--;
			s++;
		}

		if(fc||(lc<=0))
			return Arectangle(x, y, 0, h);
	}
		
	while(*s&&(lc--))
	{
		switch(*s)
		{
			case 13:
			yy+=h+e;
			xx=0;
			break;

			case 32:
			xx+=w>>1;
			break;

			default:
			if(characters)
			{
				byte	*p=(byte *)strchr((char *)characters, *s);
				if(p)
				{
					int		n=p-characters;
					int		wc=charInfo[n].w;
					xx+=e+wc;
				}
				else
					xx+=w;
			}
			else
				xx+=e+charInfo[(byte)*s].w;
			break;
		}
		maxx=maxi(xx, maxx);
		maxy=maxi(maxy, yy);
		s++;
	}
	return Arectangle(x, y, maxx, maxy);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Apoint Afont::getPosition(char *s0, int nc)
{
	byte		*s=(byte *)s0;
	int			i;
	int			m=strlen((char *)s);
	int			xx=0;
	int			yy=0;

	nc=maxi(mini(nc, m), 0);

	for(i=0; i<nc; i++)
	{
		int	c=(byte)*(s++);
		switch(c)
		{
			case 32:
			xx+=(w>>1);
			break;

			case 13:
   			xx=0;
   			yy+=h+e;
   			break;

			default:
			if(characters)
			{
				byte	*p=(byte *)strchr((char *)characters, c);
				if(p)
				{
					int		n=p-characters;
					int		wc=charInfo[n].w;
					xx+=e+wc;
				}
				else
					xx+=w;
			}
			else
				xx+=e+charInfo[c].w;
			break;
		}
	}
	return Apoint(xx, yy);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
