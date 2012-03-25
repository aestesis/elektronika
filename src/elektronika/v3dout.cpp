/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	v3dout.CPP					(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<assert.h>
#include						<stdio.h>
#include						<math.h>

#include						"main.h"
#include						"v3dout.h"
#include						"resource.h"
#include						"ef3d-node.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Av3doutInfo::CI		= ACI("Av3doutInfo",	GUID(0x11111112,0x00000320), &AeffectInfo::CI, 0, NULL);
ACI								Av3dout::CI			= ACI("Av3dout",		GUID(0x11111112,0x00000321), &Aeffect::CI, 0, NULL);
ACI								Av3doutFront::CI	= ACI("Av3doutFront",	GUID(0x11111112,0x00000322), &AeffectFront::CI, 0, NULL);
ACI								Av3doutBack::CI		= ACI("Av3doutBack",	GUID(0x11111112,0x00000323), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define	POSEF(k)				(92*k+60)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	renderOK,
	renderDEVICELOST,
	renderINTERNALERROR
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CUSTOMVERTEX
{
    FLOAT						x, y, z, rhw;
    DWORD						color;
	float						u,v;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define D3DFVF_CUSTOMVERTEX		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define	VBUFSIZE				(sizeof(CUSTOMVERTEX)*2400)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define	TEXSOUNDW				(128)
#define	TEXSOUNDH				(4)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFORMSTEPS				(20)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID pad_qiid[PIX_MAXINPUT]={	0x18f8455d54ce8328, 0xdb7a84ad48ec4fe8, 0x8a848d8e984fbf9c, 0x855e84d5e8ac2018,
										0xb87a7845e6d70154, 0xfc84d5c8ea245287, 0x804d84f8c8a08420, 0x2e884c8de8f4a811 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AdataV3d
{
public:
	IDirect3DVertexBuffer9		*vb;
	IDirect3DVertexBuffer9		*vbreg;
	IDirect3DTexture9			*texreg;
	IDirect3DTexture9			*texBG[2];
	IDirect3DSurface9			*zbufBG;
	IDirect3DSwapChain9			*chain[2];

	IDirect3DTexture9			*texsound;
	IDirect3DTexture9			*texsoundcpy;

	IDirect3DTexture9			*texT;
	IDirect3DTexture9			*texD;

	IDirect3DTexture9			*texMaskS;
	IDirect3DTexture9			*texMaskD;

	Ad3dPixShader				*pix;
	Asection					section;
	bool						bchanged;
	bool						fchange;
	bool						pixerror;
	bool						pixok;

	int							nlost;

	float						apad[PIX_MAXINPUT];
	
	AdataV3d()
	{
		vb=null;
		vbreg=NULL;
		texreg=NULL;
		texBG[0]=NULL;
		texBG[1]=NULL;
		texsound=null;
		texsoundcpy=null;
		zbufBG=NULL;
		nlost=0;
		chain[0]=null;
		chain[1]=null;
		pix=null;
		bchanged=false;
		fchange=false;
		pixerror=false;
		pixok=false;
		texT=null;
		texD=null;
		texMaskS=null;
		texMaskD=null;
		memset(apad, 0, sizeof(apad));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawTexture(Ad3d *ad3d, AdataV3d *dt, float x0, float y0, float x1, float y1, IDirect3DTexture9 *tex, dword color, bool boost)
{
	CUSTOMVERTEX vertices[] =
	{
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f },
		{ 320.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 1.f, 0.f },
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f },
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f },
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f },
		{   0.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 0.f, 1.f }
	};
	VOID		*pVertices;
	HRESULT		res;

	vertices[0].x=x0;
	vertices[1].x=x1;
	vertices[2].x=x1;
	vertices[3].x=x0;
	vertices[4].x=x1;
	vertices[5].x=x0;

	vertices[0].y=y0;
	vertices[1].y=y0;
	vertices[2].y=y1;
	vertices[3].y=y0;
	vertices[4].y=y1;
	vertices[5].y=y1;

	vertices[0].color=color;
	vertices[1].color=color;
	vertices[2].color=color;
	vertices[3].color=color;
	vertices[4].color=color;
	vertices[5].color=color;

	if(dt->vbreg->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
	{
		memcpy(pVertices, vertices, sizeof(vertices));
		dt->vbreg->Unlock();
	}

	ad3d->d3dev->BeginScene();
	res=ad3d->d3dev->SetTexture(0, tex);
	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
	res=ad3d->d3dev->SetStreamSource(0, dt->vbreg, 0, sizeof(CUSTOMVERTEX));
	res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	if(boost)
		res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	else
		res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	res=ad3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	res=ad3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	res=ad3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	res=ad3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

	res=ad3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

	ad3d->d3dev->EndScene();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void drawTexture(Ad3d *ad3d, AdataV3d *dt, Apoint p[4], IDirect3DTexture9 *tex, dword color, bool boost)
{
	CUSTOMVERTEX ov[DEFORMSTEPS+1][DEFORMSTEPS+1];

	{
		int x,y;
		for(y=0; y<=DEFORMSTEPS; y++)
		{
			Apoint	left=Apoint((p[0].x*y+p[3].x*(DEFORMSTEPS-y))/DEFORMSTEPS, (p[0].y*y+p[3].y*(DEFORMSTEPS-y))/DEFORMSTEPS);
			Apoint	right=Apoint((p[1].x*y+p[2].x*(DEFORMSTEPS-y))/DEFORMSTEPS, (p[1].y*y+p[2].y*(DEFORMSTEPS-y))/DEFORMSTEPS);
			float v=(float)(DEFORMSTEPS-y)/(float)DEFORMSTEPS;
			for(x=0; x<=DEFORMSTEPS; x++)
			{
				Apoint p=Apoint((left.x*x+right.x*(DEFORMSTEPS-x))/DEFORMSTEPS, (left.y*x+right.y*(DEFORMSTEPS-x))/DEFORMSTEPS);
				float u=(float)(DEFORMSTEPS-x)/(float)DEFORMSTEPS;
				CUSTOMVERTEX cv={ p.x, p.y, 0.5f, 1.f, 0xffffffff, u, v };
				ov[y][x]=cv;
			}
		}
	}

	CUSTOMVERTEX		*pVertices;
	HRESULT				res;

	if(dt->vbreg->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
	{
		int x,y;
		for(y=0; y<DEFORMSTEPS; y++)
		{
			for(x=0; x<DEFORMSTEPS; x++)
			{
				CUSTOMVERTEX vertices[6]=
				{
					ov[y][x], ov[y][x+1], ov[y+1][x+1],
					ov[y][x], ov[y+1][x+1], ov[y+1][x],
				};
				memcpy(pVertices, vertices, sizeof(vertices));
				pVertices+=countof(vertices);
			}
		}
		dt->vbreg->Unlock();
	}

	ad3d->d3dev->BeginScene();
	res=ad3d->d3dev->SetTexture(0, tex);
	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
	res=ad3d->d3dev->SetStreamSource(0, dt->vbreg, 0, sizeof(CUSTOMVERTEX));
	res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	if(boost)
		res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	else
		res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	res=ad3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	res=ad3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	res=ad3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	res=ad3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

	res=ad3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, (DEFORMSTEPS*DEFORMSTEPS*6)-1);

	ad3d->d3dev->EndScene();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3dMaskEdit : public Aobject
{
	AOBJ

	enum Mode
	{
		mask,
		placement
	};

	Apoint						point[256];
	int							nbPoints;
	int							selPoint;
	Apoint						place[4];
	int							mode;


								Av3dMaskEdit				(char *name, Aobject *L, int x, int y);
								~Av3dMaskEdit				();

	virtual void				paint						(Abitmap *b);
	virtual bool				mouse						(int x, int y, int state, int event);

	void						setMode						(Mode mode);

	void						getMask						(Abitmap *b);
	void						getPlace					(Apoint *dest);	// size 4
	void						fill						(Abitmap *b, int x, int y);

	void						load						(Afile *f);
	void						save						(Afile *f);

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Av3dMaskEdit::CI	= ACI("Av3dMaskEdit",	GUID(0x11111112,0x00000327), &Aobject::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3dMaskEdit::Av3dMaskEdit(char *name, Aobject *L, int x, int y) : Aobject(name, L, x, y, 512, 512)
{
	nbPoints=4;
	point[0].x=0;
	point[0].y=0;
	point[1].x=511;
	point[1].y=0;
	point[2].x=511;
	point[2].y=511;
	point[3].x=0;
	point[3].y=511;
	selPoint=-1;
	mode=Mode::placement;
	place[0]=point[0];
	place[1]=point[1];
	place[2]=point[2];
	place[3]=point[3];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3dMaskEdit::~Av3dMaskEdit()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dMaskEdit::load(Afile *f)
{
	f->read(&nbPoints, sizeof(nbPoints));
	f->read(&point[0], sizeof(point));
	f->read(&place[0], sizeof(place));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dMaskEdit::save(Afile *f)
{
	f->write(&nbPoints, sizeof(nbPoints));
	f->write(&point[0], sizeof(point));
	f->write(&place[0], sizeof(place));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dMaskEdit::setMode(Mode mode)
{
	this->mode=mode;
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dMaskEdit::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseRDOWN:
		if((mode==Mode::mask)&&(nbPoints>3))
		{
			Apoint *p=&point[0];
			int i;
			for(i=0; i<nbPoints; i++)
			{
				int dx=abs(p->x-x);
				int dy=abs(p->y-y);
				if((dx<6)&&(dy<6))
				{
					int j;
					for(j=i; j<nbPoints-1; j++)
						point[j]=point[j+1];
					nbPoints--;
					repaint();
					notify(this, nyCHANGE);
					return true;
				}
				p++;
			}
		}
		break;

		case mouseMDOWN:
			if((mode==Mode::mask)&&(nbPoints<countof(point)))
			{
				float dist=512;
				int iPoint=-1;
				int i;
				for(i=1; i<nbPoints; i++)
				{
					int j;
					for(j=1; j<10; j++)
					{
						float a=(float)j*0.1f;
						float ia=1.f-a;
						float xi=point[i-1].x*a+point[i].x*ia;
						float yi=point[i-1].y*a+point[i].y*ia;
						float dx=x-xi;
						float dy=y-yi;
						float d=sqrt(dx*dx+dy*dy);
						if(d<dist)
						{
							dist=d;
							iPoint=i;
						}
					}
				}
				{	// ligne: premier <--> dernier
					int j;
					for(j=1; j<10; j++)
					{
						float a=(float)j*0.1f;
						float ia=1.f-a;
						float xi=point[0].x*a+point[nbPoints-1].x*ia;
						float yi=point[0].y*a+point[nbPoints-1].y*ia;
						float dx=x-xi;
						float dy=y-yi;
						float d=sqrt(dx*dx+dy*dy);
						if(d<dist)
						{
							point[nbPoints].x=x;
							point[nbPoints].y=y;
							nbPoints++;
							repaint();
							notify(this, nyCHANGE);
							return true;
						}
					}					
				}
				if(iPoint!=-1)
				{
					for(i=nbPoints; i>iPoint; i--)
						point[i]=point[i-1];
					point[iPoint].x=x;
					point[iPoint].y=y;
					nbPoints++;
					repaint();
					notify(this, nyCHANGE);
					return true;
				}
			}
		break;
		
		case mouseLDOWN:
		if(mode==Mode::mask)
		{
			Apoint *p=&point[0];
			int i;
			for(i=0; i<nbPoints; i++)
			{
				int dx=abs(p->x-x);
				int dy=abs(p->y-y);
				if((dx<6)&&(dy<6))
				{
					selPoint=i;
					point[i].x=x;
					point[i].y=y;
					mouseCapture(true);
					repaint();
					return true;
				}
				p++;
			}
		}
		else	// placement
		{
			Apoint *p=&place[0];
			int i;
			for(i=0; i<countof(place); i++)
			{
				int dx=abs(p->x-x);
				int dy=abs(p->y-y);
				if((dx<6)&&(dy<6))
				{
					selPoint=i;
					place[i].x=x;
					place[i].y=y;
					mouseCapture(true);
					repaint();
					return true;
				}
				p++;
			}
		}
		break;
	case mouseLUP:
		if(selPoint!=-1)
		{
			selPoint=-1;
			mouseCapture(false);
			repaint();
			notify(this, nyCHANGE);
			return true;
		}
		break;
	case mouseMOVE:
		if(mode==Mode::mask)
		{
			if(selPoint!=-1)
			{
				cursor(cursorCROSS);
				point[selPoint].x=mini(maxi(x, 0),511);
				point[selPoint].y=mini(maxi(y, 0),511);
				repaint();
				return true;
			}
			else
			{
				Apoint *p=&point[0];
				int i;
				for(i=0; i<nbPoints; i++)
				{
					int dx=abs(p->x-x);
					int dy=abs(p->y-y);
					if((dx<6)&&(dy<6))
					{
						cursor(cursorCROSS);
						return true;
					}
					p++;
				}		
			}
		}
		else
		{
			if(selPoint!=-1)
			{
				cursor(cursorCROSS);
				place[selPoint].x=mini(maxi(x, 0),511);
				place[selPoint].y=mini(maxi(y, 0),511);
				repaint();
				return true;
			}
			else
			{
				Apoint *p=&place[0];
				int i;
				for(i=0; i<4; i++)
				{
					int dx=abs(p->x-x);
					int dy=abs(p->y-y);
					if((dx<6)&&(dy<6))
					{
						cursor(cursorCROSS);
						return true;
					}
					p++;
				}		
			}
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dMaskEdit::paint(Abitmap *b)
{
	int i;
	b->boxf(0, 0, pos.w, pos.h, 0xff000000);

	switch(mode)
	{
		case Mode::mask:
			{
				int x,y;
				for(y=1; y<DEFORMSTEPS; y++)
				{
					Apoint	left=Apoint((place[0].x*y+place[3].x*(DEFORMSTEPS-y))/DEFORMSTEPS, (place[0].y*y+place[3].y*(DEFORMSTEPS-y))/DEFORMSTEPS);
					Apoint	right=Apoint((place[1].x*y+place[2].x*(DEFORMSTEPS-y))/DEFORMSTEPS, (place[1].y*y+place[2].y*(DEFORMSTEPS-y))/DEFORMSTEPS);
					for(x=1; x<DEFORMSTEPS; x++)
					{
						Apoint p=Apoint((left.x*x+right.x*(DEFORMSTEPS-x))/DEFORMSTEPS, (left.y*x+right.y*(DEFORMSTEPS-x))/DEFORMSTEPS);
						b->pixel(p.x, p.y, 0xff404000);
					}
				}
			}
			for(i=1; i<4; i++)
				b->line(place[i-1].x, place[i-1].y, place[i].x, place[i].y, 0xff404000);
			b->line(place[0].x, place[0].y, place[3].x, place[3].y, 0xff404000);
			for(i=1; i<nbPoints; i++)
				b->line(point[i-1].x, point[i-1].y, point[i].x, point[i].y, 0xff404040);
			if(nbPoints>0)
				b->line(point[0].x, point[0].y, point[nbPoints-1].x, point[nbPoints-1].y, 0xff404040);
			for(i=0; i<nbPoints; i++)
			{
				Apoint *p=&point[i];
				if(selPoint==i)
					b->boxf(p->x-2, p->y-2, p->x+2, p->y+2, 0xffffffff);
				else
					b->boxf(p->x-1, p->y-1, p->x+1, p->y+1, 0xffa0a0a0);
			}
			break;
	
		case Mode::placement:
			for(i=1; i<nbPoints; i++)
				b->line(point[i-1].x, point[i-1].y, point[i].x, point[i].y, 0xff404040);
			if(nbPoints>0)
				b->line(point[0].x, point[0].y, point[nbPoints-1].x, point[nbPoints-1].y, 0xff404040);
			{
				int x,y;
				for(y=1; y<DEFORMSTEPS; y++)
				{
					Apoint	left=Apoint((place[0].x*y+place[3].x*(DEFORMSTEPS-y))/DEFORMSTEPS, (place[0].y*y+place[3].y*(DEFORMSTEPS-y))/DEFORMSTEPS);
					Apoint	right=Apoint((place[1].x*y+place[2].x*(DEFORMSTEPS-y))/DEFORMSTEPS, (place[1].y*y+place[2].y*(DEFORMSTEPS-y))/DEFORMSTEPS);
					for(x=1; x<DEFORMSTEPS; x++)
					{
						Apoint p=Apoint((left.x*x+right.x*(DEFORMSTEPS-x))/DEFORMSTEPS, (left.y*x+right.y*(DEFORMSTEPS-x))/DEFORMSTEPS);
						b->pixel(p.x, p.y, 0xff404000);
					}
				}
			}
			for(i=1; i<4; i++)
				b->line(place[i-1].x, place[i-1].y, place[i].x, place[i].y, 0xff404000);
			b->line(place[0].x, place[0].y, place[3].x, place[3].y, 0xff404000);
			for(i=0; i<4; i++)
			{
				Apoint *p=&place[i];
				if(selPoint==i)
					b->boxf(p->x-2, p->y-2, p->x+2, p->y+2, 0xffffffff);
				else
					b->boxf(p->x-1, p->y-1, p->x+1, p->y+1, 0xffa0a0a0);
			}
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dMaskEdit::getPlace(Apoint *p)
{
	p[0]=place[0];
	p[1]=place[1];
	p[2]=place[2];
	p[3]=place[3];
}

void Av3dMaskEdit::getMask(Abitmap *b)
{
	assert(b->w==512&&b->h==512&&b->nbBits==32);
	int i;
	b->boxf(0, 0, pos.w, pos.h, 0x00000000);
	for(i=1; i<nbPoints; i++)
		b->line(point[i-1].x, point[i-1].y, point[i].x, point[i].y, 0xff000000);
	if(nbPoints>0)
		b->line(point[0].x, point[0].y, point[nbPoints-1].x, point[nbPoints-1].y, 0xff000000);
	fill(b, 0, 0);
	fill(b, 256, 0);
	fill(b, 511, 0);
	fill(b, 0, 511);
	fill(b, 256, 511);
	fill(b, 511, 511);
	fill(b, 0, 256);
	fill(b, 511, 256);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dMaskEdit::fill(Abitmap *b, int x, int y)
{
	dword *p=b->body32+(y<<9)+x;
	if(*p)
		return;
	{
		Apoint	q0[1024];
		Apoint	q1[1024];
		int		n0=0;
		int		n1=0;
		Apoint	*q=&q0[0];
		int		*n=&n0;
		Apoint	*nq=&q1[0];
		int		*nn=&n1;

		q[0].x=x;
		q[0].y=y;
		(*n)=1;

		while(*n>0)
		{
			*nn=0;
			int i;
			for(i=0; i<*n; i++)
			{
				int x=q[i].x;
				int y=q[i].y;

				if(*(b->body32+(y<<9)+x)==0x00000000)
				{
					int l=x;
					int r=x;

					while(l>0)
					{
						l--;
						if(*(b->body32+(y<<9)+l)!=0x00000000)
						{
							l++;
							break;
						}
					}
					while(r<511)
					{
						r++;
						if(*(b->body32+(y<<9)+r)!=0x00000000)
						{
							r--;
							break;
						}
					}
					b->line(l, y, r, y, 0xFF000000);
					if(y>0)
					{
						int xp;
						int yp=y-1;
						for(xp=l; xp<=r; xp++)
						{
							if(*(b->body32+(yp<<9)+xp)==0x00000000)
							{
								nq[*nn].x=xp;
								nq[*nn].y=yp;
								(*nn)++;
							}
						}
					}
					if(y<(b->h-1))
					{
						int xp;
						int yp=y+1;
						for(xp=l; xp<=r; xp++)
						{
							if(*(b->body32+(yp<<9)+xp)==0x00000000)
							{
								nq[*nn].x=xp;
								nq[*nn].y=yp;
								(*nn)++;
							}
						}
					}
				}
			}
			{
				Apoint *t=q;
				q=nq;
				nq=t;
			}
			{
				int *t=n;
				n=nn;
				nn=t;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3dMask : public Awindow
{
	AOBJ

	Av3dMaskEdit				*edit;
	bool						bac;
	int							lx,ly,wx,wy;
	Av3dout						*v3dout;
	Abutton						*butMask;
	Abutton						*butDeform;
	Abutton						*close;

								Av3dMask					(char *name, int x, int y, Av3dout *v3dout);
	virtual						~Av3dMask					();

	virtual bool				keyboard					(int event, int ascii, int scan, int state);
	virtual bool				mouse						(int x, int y, int state, int event);
	bool						notify						(Anode *o, int event, dword p);

	void						getMask						(Abitmap *b) { edit->getMask(b); }
	void						getPlace					(Apoint *p) { edit->getPlace(p); }

	void						load						(Afile *f) { edit->load(f); }
	void						save						(Afile *f) { edit->save(f); }

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Av3dMask::CI	= ACI("Av3dMask",	GUID(0x11111112,0x00000326), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3dMask::Av3dMask(char *name, int x, int y, Av3dout *v3dout) : Awindow(name, x, y, 548, 520)
{
	bac=false;
	zorder(zorderTOP);
	this->v3dout=v3dout;
	bitmap->boxf(0, 0, bitmap->w, bitmap->h, 0xff202020);
	edit=new Av3dMaskEdit("maskedit", this, 4, 4);
	edit->show(true);
	butMask=new Abutton("butMask", this, 520, 4, 24, 24, &resource.get(MAKEINTRESOURCE(PNG_V3DOUT_MASK), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	butMask->setTooltips("edit mask");
	butMask->show(true);
	butDeform=new Abutton("butMask", this, 520, 32, 24, 24, &resource.get(MAKEINTRESOURCE(PNG_V3DOUT_DEFORM), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	butDeform->setTooltips("edit deformation");
	butDeform->show(true);
	butDeform->setChecked(true);
	close=new Abutton("close", this, 520, 492, 24, 24, "OK");
	close->setTooltips("Close mask editing window");
	close->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3dMask::~Av3dMask()
{
	delete(edit);
	delete(butMask);
	delete(butDeform);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dMask::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			wx=pos.x;
			wy=pos.y;
			lx=pos.x+x;
			ly=pos.y+y;
			bac=true;
			cursor(cursorSIZEALL);
			mouseCapture(TRUE);
		}
		return TRUE;

		case mouseNORMAL:
		if(state&mouseL)
		{
			if(bac)
			{
				move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
			}
		}
		return TRUE;

		case mouseLUP:
		if(bac)
		{
			bac=false;
			mouseCapture(FALSE);
		}
		return TRUE;
		
		case mouseRDOWN:
			v3dout->front->notify(this, nyCLOSE, 0);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dMask::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLICK:
			if(o==close)
			{
				v3dout->front->notify(this, nyCLOSE, 0);
				return true;
			}
		break;

		case nyCHANGE:
			if(o==butMask)
			{
				if(p)
				{
					edit->setMode(Av3dMaskEdit::Mode::mask);
					butDeform->setChecked(false);
				}
				else
				{
					edit->setMode(Av3dMaskEdit::Mode::placement);
					butDeform->setChecked(true);
				}
			}
			else if(o==butDeform)
			{
				if(p)
				{
					edit->setMode(Av3dMaskEdit::Mode::placement);
					butMask->setChecked(false);
				}
				else
				{
					edit->setMode(Av3dMaskEdit::Mode::mask);
					butMask->setChecked(true);
				}
			}
			else
				v3dout->front->notify(this, nyCHANGE, 0);
		return true;
	}
	return Awindow::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dMask::keyboard(int event, int ascii, int scan, int state)
{
//	safeGUI.enter(__FILE__,__LINE__);
	if(ascii==27)
	{
		v3dout->front->notify(this, nyCLOSE, 0);
		return true;
	}
//	safeGUI.leave();
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Av3doutDisp : public Awindow
{
	AOBJ

								Av3doutDisp						(char *name, int x, int y, int w, int h, Av3dout *v3dout);
	virtual						~Av3doutDisp					();

	virtual bool				mouse							(int x, int y, int state, int event);
	virtual bool				keyboard						(int event, int ascii, int scan, int state);

	virtual void				repaint							(bool now=FALSE)					{ }
	virtual void				repaint							(Arectangle r, bool now=FALSE)		{ }

	int							bac,lx,ly,wx,wy;
	Av3dout						*v3dout;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Av3doutDisp::CI	= ACI("Av3doutDisp",	GUID(0x11111112,0x00000324), &Awindow::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3doutDisp::Av3doutDisp(char *name, int x, int y, int w, int h, Av3dout *v3dout) : Awindow(name, x, y, w, h)
{
	zorder(zorderTOP);
	state|=stateNOSURFACESIZEUPDATE;
	this->v3dout=v3dout;
	keyboardFocusOBJ=v3dout->front->getWindow();
	bitmap->boxf(0, 0, bitmap->w, bitmap->h, 0xff00ff00);
	bac=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3doutDisp::~Av3doutDisp()
{
	v3dout->winX=pos.x;
	v3dout->winY=pos.y;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3doutDisp::keyboard(int event, int ascii, int scan, int state)
{
	bool	b=true;
	safeGUI.enter(__FILE__,__LINE__);
	if(ascii==27)
		v3dout->escape=true;
	else
		b=v3dout->front->getWindow()->keyboard(event, ascii, scan, state);
	safeGUI.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3doutDisp::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		{
			int max=((pos.w*pos.w+pos.h*pos.h)*7)>>3;
			int	n=x*x+y*y;
			if(n>max)
			{
				wx=pos.x;
				wy=pos.y;
				lx=pos.x+x;
				ly=pos.y+y;
				bac=2;
			}
			else
			{
				wx=pos.x;
				wy=pos.y;
				lx=pos.x+x;
				ly=pos.y+y;
				bac=1;
				cursor(cursorSIZEALL);
			}
			mouseCapture(TRUE);
		}
		return TRUE;

		case mouseNORMAL:
		if(state&mouseL)
		{
			switch(bac)
			{
				case 1:
				move(wx+(x+pos.x)-lx, wy+(y+pos.y)-ly);
				break;

				case 2:
				v3dout->section.enter(__FILE__,__LINE__);
				size(maxi(x+1, 32), maxi(y+1, 24));
				v3dout->section.leave();
				break;
			}
		}
		else
		{
			int max=((pos.w*pos.w+pos.h*pos.h)*7)>>3;
			int	n=x*x+y*y;
			if(n>max)
				cursor(cursorSIZENWSE);
			else
				cursor(cursorCROSS);
		}
		return TRUE;

		case mouseLUP:
		if(bac)
		{
			bac=0;
			mouseCapture(FALSE);
		}
		return TRUE;

		case mouseRDOWN:
		v3dout->section.enter(__FILE__,__LINE__);
		size(bitmap->w, bitmap->h);
		v3dout->section.leave();
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static long FAR PASCAL WindowProc(HWND hw, UINT m, WPARAM wp, LPARAM lp)
{
   return DefWindowProc(hw, m, wp, lp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Alaunch : public Athread
{
	public:
										Alaunch(Av3dout *v3do, int x, int y, int w, int h, Ad3d *d3d, int device);
	void								run();
	
	Av3dout								*v3do;
	Ad3d								*ad3d;
	int									device;
	int									x,y,w,h;
	bool								ok;
	//Awindow								*win;
	HWND								hwnd;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Alaunch::Alaunch(Av3dout *v3do, int x, int y, int w, int h, Ad3d *ad3d, int device)
{
	this->v3do=v3do;
	this->device=device;
	this->x=x;
	this->y=y;
	this->w=w;
	this->h=h;
	//win=NULL;
	hwnd=NULL;
	this->ad3d=ad3d;
	ok=false;
}

void Alaunch::run()
{
	{	
		WNDCLASS		wc;
		wc.style=CS_NOCLOSE|CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
		wc.lpfnWndProc=WindowProc;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=GetModuleHandle(NULL);
		wc.hIcon=NULL;
		wc.hCursor=NULL;
		wc.hbrBackground=NULL;
		wc.lpszMenuName=NULL;
		wc.lpszClassName="v3doutYoYclassDD";
		RegisterClass(&wc);
	}
	
	hwnd=CreateWindowEx(WS_EX_TOPMOST, "v3doutYoYclassDD", "v3doutYoYclassDD", WS_POPUPWINDOW, x, y, w, h, NULL, NULL, GetModuleHandle(NULL), NULL);
	if(!hwnd)
		return;

//	win=new Av3doutDisp("display", x, y, w, h, v3do);
//	win->show(TRUE);
	ad3d->createDevice(hwnd, device, true);
	ok=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static qword	qidef[NBEF3D]={ 0xee3819954c644d80, 0x23f89ee234c59d00, 0x4739f3b7caecaae0, 0x4739f678987caab0 };
static qword	qidin[NBEF3D]={ 0x370e5b689ea9df50, 0x7ecc41ae10fc3f60, 0x8ae2da30fb454690, 0x473acd9098098900 };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3dout::Av3dout(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	renderSTATUS=renderOK;
	escape=false;
	dt=new AdataV3d();
	ad3d=new Ad3d("d3d");
	dt->pix=new Ad3dPixShader();
	preview=null;
	imgMask=new Abitmap(512,512);
	imgMask->boxf(0,0,512,512,0x00000000);
	place[0]=Apoint(0,0);
	place[1]=Apoint(511,0);
	place[2]=Apoint(511,511);
	place[3]=Apoint(0,511);
	front=new Av3doutFront(qiid, "dimension front", this, 466);
	back=new Av3doutBack(qiid, "dimension back", this, 466);
	back->setTooltips("DIMENSION module");
	win=NULL;
	hwnd=NULL;
	runout=true;
	winrun=false;
	ef3run=false;
	winX=20;
	winY=20;
	
	sound=new Aef3dSound();
	
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3dout::~Av3dout()
{
	change(0);
	delete(dt->pix);
	delete(dt);
	delete(sound);
	stop();
	if(ad3d)
		delete(ad3d);
	delete(imgMask);
	ad3d=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::oscMessage(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *onode, int action)
{
	if(Aeffect::oscMessage(reply, msg, onode, action))
		return true;
	{
		char *slash=strchr(onode->name, '/');
		if(slash)
			slash=strchr(slash+1, '/');
		if(slash)
		{
			Av3doutFront	*front=(Av3doutFront *)this->front;
			int				i;
			char			efname[1024];
			memset(efname, 0, sizeof(efname));
			strncpy(efname, onode->name, slash-onode->name);
			for(i=0; i<NBEF3D; i++)
			{
				if(!strcmp(efname, front->ef3[i]->name))
					return front->ef3[i]->oscMessage(reply, msg, onode, action);
			}
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::load(class Afile *f)
{
	Av3doutFront	*front=((Av3doutFront *)this->front);
	char			drv[1024];
	
	if(!f->readString(drv))
		return false;
	{
		Aitem	*i=(Aitem *)(front->items->fchild);
		while(i)
		{
			if(!strcmp(drv, i->name))
				break;
			i=(Aitem *)i->next;
		}
		if(i)
		{
			if(change(i->data))
				front->list->setCurrentByData(i->data);
			else
				front->list->setCurrentByData(0);
		}
		else
		{
			change(0);
			front->list->setCurrentByData(0);
		}
	}
	if(safemagictest(f, magicMASK3D))
	{
		front->winMask->load(f);
		front->notify(front->winMask, nyCHANGE, 0);
	}
	return loadPreset(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::save(class Afile *f)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	{
		Aitem		*i=front->list->getCurrent();
		if(i)
		{
			if(!f->writeString(i->name))
				return false;
		}
		else
		{
			if(!f->writeString("[none]"))
				return false;
		}
	}
	{
		dword	magic=magicMASK3D;
		f->write(&magic, sizeof(magic));
		front->winMask->save(f);
	}
	return savePreset(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::loadPreset(class Afile *f)
{
	Av3doutFront	*front=((Av3doutFront *)this->front);
	int				i;
	bool			itest[NBEF3D];
	bool			etest[NBEF3D];
	
	memset(itest, 0, sizeof(itest));
	memset(etest, 0, sizeof(etest));
	
	section.enter(__FILE__,__LINE__);
	
	for(i=0; i<NBEF3D; i++)
	{
		if(ef3run)
			release3D(i, true);
	}

	f->read(front->order, sizeof(front->order));
	
	for(i=0; i<NBEF3D; i++)
	{
		qword	guid;

		if(!f->read(&guid, sizeof(guid)))
		{
			section.leave();
			return false;
		}

		if(front->ei3[i]->getCI()->guid!=guid)
		{
			delete(front->ei3[i]);
			front->ei3[i]=NULL;

			{
				Aitem	*it=(Aitem *)front->eii->fchild;
				while(it)
				{
					Ainput3dInfo	*i3di=(Ainput3dInfo *)it->link;
					if(i3di->guid==guid)
					{
						itest[i]=true;
						front->eil[i]->setCurrent(it);
						break;
					}
					it=(Aitem *)it->next;
				}
			}

			{
				Ainput3dInfo 	*i3di=(Ainput3dInfo *)front->eil[i]->getCurrentLink();
				int				y=POSEF(front->order[i]);
				char			str[1024];
				sprintf(str, "%d/%s", i, i3di->name);
				front->ei3[i]=i3di->create(MKQIID(getQIID(), qidin[i]), str, front, 15, y-7);
				front->ei3[i]->show(true);
			}
		}
		
		if(!f->read(&guid, sizeof(guid)))
		{
			section.leave();
			return false;
		}

		if(front->ef3[i]->getCI()->guid!=guid)
		{
			delete(front->ef3[i]);
			front->ef3[i]=NULL;

			{
				Aitem	*it=(Aitem *)front->efi->fchild;
				while(it)
				{
					Aeffect3dInfo	*e3di=(Aeffect3dInfo *)it->link;
					if(e3di->guid==guid)
					{
						etest[i]=true;
						front->efl[i]->setCurrent(it);
						break;
					}
					it=(Aitem *)it->next;
				}
			}

			{
				Aeffect3dInfo 	*e3di=(Aeffect3dInfo *)front->efl[i]->getCurrentLink();
				int				y=POSEF(front->order[i]);
				char			str[1024];
				sprintf(str, "%d/%s", i, e3di->name);
				front->ef3[i]=e3di->create(MKQIID(getQIID(), qidef[i]), str, front, 154, y-7);
				front->ef3[i]->show(true);
			}
		}
	}
	
	for(i=0; i<NBEF3D; i++)
	{
		Abuffer	*buf=new Abuffer(f->name, 1024*1024);
		
		if(magictest(f, magicIN3D))
		{
			int		size;
			f->read(&size, sizeof(size));
			if(itest[i])
			{	
				if(!front->ei3[i]->load(f))
				{
					section.leave();
					return false;
				}
			}
			else
				f->seek(f->offset+size);
		}
		else
		{
			section.leave();
			return false;
		}

		if(magictest(f, magicEF3D))
		{
			int	size;
			f->read(&size, sizeof(size));
			if(etest[i])
			{	
				if(!front->ef3[i]->load(f))
				{
					section.leave();
					return false;
				}
			}
			else
				f->seek(f->offset+size);
		}
		else
		{
			section.leave();
			return false;
		}
	}

	if(safemagictest(f, magicEF3DPIX))
	{
		char	str[1024];
		f->readString(str);
		dt->section.enter(__FILE__,__LINE__);
		{
			for(i=0; i<dt->pix->nfile; i++)
				if(!strcmp(dt->pix->filename[i], str))
				{
					front->pixl->setCurrentByData(i);
					break;
				}
		}
		{
			Afont	*font=alib.getFont(fontTERMINAL05);
			int		i;
			for(i=0; i<PIX_MAXINPUT; i++)
			{
				front->pad[i]->setTooltips("");
				front->sta[i]->set("", 0xff404040, font, Astatic::CENTER);
				front->pad[i]->set((!i)?0.f:0.5f);
			}
			dt->fchange=true;
		}
		dt->section.leave();
	}

	if(ef3run)
	{
		for(i=0; i<NBEF3D; i++)
			init3D(i, true);
	}
	
	section.leave();
	front->moveEffect(0, 0);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::savePreset(class Afile *f)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	section.enter(__FILE__,__LINE__);
	
	f->write(front->order, sizeof(front->order));
	
	{
		int			i;
		for(i=0; i<NBEF3D; i++)
		{
			{
				ACI		*ci=front->ei3[i]->getCI();
				f->write(&ci->guid, sizeof(ci->guid));
			}
			{
				ACI		*ci=front->ef3[i]->getCI();
				f->write(&ci->guid, sizeof(ci->guid));
			}
		}
		{
			Abuffer	*buf=new Abuffer(f->name, 1024*1024);
			dword	magic;
			int		size;
			for(i=0; i<NBEF3D; i++)
			{
				buf->clear();
				if(!front->ei3[i]->save(buf))
				{
					delete(buf);
					section.leave();
					return false;
				}
				magic=magicIN3D;
				size=(int)buf->size;
				f->write(&magic, sizeof(magic));
				f->write(&size, sizeof(size));
				f->write(buf->buffer, (int)buf->size);
				
				buf->clear();
				if(!front->ef3[i]->save(buf))
				{
					delete(buf);
					section.leave();
					return false;
				}
				magic=magicEF3D;
				size=(int)buf->size;
				f->write(&magic, sizeof(magic));
				f->write(&size, sizeof(size));
				f->write(buf->buffer, (int)buf->size);
			}
		}
		{
			dword	magic=magicEF3DPIX;
			dword	sel=front->pixl->getCurrentData();
			f->write(&magic, sizeof(magic));
			f->writeString(dt->pix->filename[sel]);
		}
	}

	section.leave();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::settings(bool emergency)
{
	if(emergency)
	{
		change(0);
		((Av3doutFront *)front)->list->setCurrentByData(0);
	}
	if(win||hwnd)
	{
		if(!change(current))
			((Av3doutFront *)front)->list->setCurrentByData(0);
	}
	{
		int	i;
		for(i=0; i<NBEF3D; i++)
		{
			((Av3doutFront *)front)->ei3[i]->settings();
			((Av3doutFront *)front)->ef3[i]->settings();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLOSE:
		change(0);
		((Av3doutFront *)front)->list->setCurrentByData(0);
		return true;
	}
	return this->Aeffect::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3dout::change(int c)
{
	int	err;
	winChange.enter(__FILE__,__LINE__);
	current=c;
	winrun=false;
	closeWin();
	{
		int i;
		Athread::sleep(100);	// on load multiple dxoutput can't open if too speed
		for(i=0;i<5; i++)		// try open dx 5 times
		{
			err=initWin();
			if(!err)
				break;
			Athread::sleep(100);
		}
	}
	winChange.leave();
	switch(err)
	{
		case 0:
		dt->fchange=true;
		break;

		default:
		closeWin();		
		getTable()->notify(getTable(), nyERROR, (dword)"can't open directX device.");
		((Av3doutFront *)front)->list->setCurrentByData(0);
		break;
	}
	return (error==1);

/*
	winChange.enter(__FILE__,__LINE__);
	current=c;
	winrun=false;
	while(!runout)
		sleep(10);
	stop();
	error=0;
	start();
	while(error==0)
		sleep(10);
	winChange.leave();
	switch(error)
	{
		case 1:
		break;

		default:		
		table->notify(table, nyERROR, (dword)"can't open device (change the depth screen windows to 32 bits could be better)");
		break;
	}
	return (error==1);
*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Av3dout::action(double time, double dtime, double beat, double dbeat)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	Av3doutBack		*back=(Av3doutBack *)this->back;
	float			w=(float)ad3d->w;
	float			h=(float)ad3d->h;
	Asample			*ins=back->ins;
	HRESULT			res;
	bool			aNewMask=newMask;
	if(aNewMask)
		newMask=false;

	if(ins->isConnected())
	{
		sword	smp[1024*2];
		sword	bas[1024*2];
		sword	med[1024*2];
		sword	tre[1024*2];
		int		mbas=0;
		int		mmed=0;
		int		mtre=0;
		int		avaible;
		int		sget;
		int		i;

		ins->enter(__FILE__,__LINE__);
		avaible=ins->avaible();
		avaible-=ins->skip(maxi(avaible-1024, 0));
		sget=ins->getBuffer(smp, bas, med, tre, 1024);
		ins->leave();

		for(i=0; i<(sget<<1); i++)
		{
			mbas=max(mbas, abs((int)bas[i]));
			mmed=max(mmed, abs((int)med[i]));
			mtre=max(mtre, abs((int)tre[i]));
		}
		
		sound->bass=(float)mbas/32768.f;
		sound->medium=(float)mmed/32768.f;
		sound->treble=(float)mtre/32768.f;

		winChange.enter(__FILE__,__LINE__);
		section.enter(__FILE__,__LINE__);
		{
			IDirect3DSurface9	*surf;
			if(dt->texsoundcpy&&dt->texsoundcpy->GetSurfaceLevel(0, &surf)==D3D_OK)
			{
				D3DLOCKED_RECT	lrect;
				if(surf->LockRect(&lrect, NULL, D3DLOCK_READONLY)==D3D_OK)
				{
					int		y=0;

					byte	*d=(byte *)lrect.pBits+lrect.Pitch*y;
					memcpy(d, smp, TEXSOUNDW*4);
					y++;

					d=(byte *)lrect.pBits+lrect.Pitch*y;
					memcpy(d, bas, TEXSOUNDW*4);
					y++;

					d=(byte *)lrect.pBits+lrect.Pitch*y;
					memcpy(d, med, TEXSOUNDW*4);
					y++;

					d=(byte *)lrect.pBits+lrect.Pitch*y;
					memcpy(d, tre, TEXSOUNDW*4);

					surf->UnlockRect();
				}
				{
					IDirect3DSurface9	*sdest;
					if(dt->texsound->GetSurfaceLevel(0, &sdest)==D3D_OK)
						res=ad3d->d3dev->UpdateSurface(surf, NULL, sdest, NULL);
				}
				surf->Release();
			}
		}
		section.leave();
		winChange.leave();
		
		if(sget==1024)
		{
			sword	*s=smp;
			sword	*b=bas;
			sword	*m=med;
			sword	*t=tre;
			sword	*sl=sound->smpL;
			sword	*sr=sound->smpR;
			sword	*bl=sound->basL;
			sword	*br=sound->basR;
			sword	*ml=sound->medL;
			sword	*mr=sound->medR;
			sword	*tl=sound->treL;
			sword	*tr=sound->treR;
			int		i;
			for(i=0; i<1024; i++)
			{
				*(sl++)=*(s++);
				*(sr++)=*(s++);
				*(bl++)=*(b++);
				*(br++)=*(b++);
				*(ml++)=*(m++);
				*(mr++)=*(m++);
				*(tl++)=*(t++);
				*(tr++)=*(t++);
			}
		}
		else
		{
			sword	*sl=sound->smpL;
			sword	*sr=sound->smpR;
			sword	*bl=sound->basL;
			sword	*br=sound->basR;
			sword	*ml=sound->medL;
			sword	*mr=sound->medR;
			sword	*tl=sound->treL;
			sword	*tr=sound->treR;
			int		i;
			for(i=0; i<1024; i++)
			{
				*(sl++)=0;
				*(sr++)=0;
				*(bl++)=0;
				*(br++)=0;
				*(ml++)=0;
				*(mr++)=0;
				*(tl++)=0;
				*(tr++)=0;
			}
		}
	}

	winChange.enter(__FILE__,__LINE__);
	section.enter(__FILE__,__LINE__);
	if(win||hwnd)
	{
		Aeffect3d	*e3d[NBEF3D];
		Ainput3d	*i3d[NBEF3D];
		int			iorder[NBEF3D];
		{
			int	i;
			for(i=0; i<NBEF3D; i++)
			{
				iorder[front->order[i]]=i;
				e3d[front->order[i]]=front->ef3[i];
				i3d[front->order[i]]=front->ei3[i];
			}
		}
		
		if(ad3d&&ad3d->isOK)
		{
			CUSTOMVERTEX vertices[] =
			{
				{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f },
				{ 320.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 1.f, 0.f },
				{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f },
				{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f },
				{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f },
				{   0.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 0.f, 1.f }
			};
			VOID		*pVertices;
			HRESULT		res;
			int			i;
			dword		c=front->color->get();
			float		v=front->vol->get();
			float		vmaster=front->master->get();
			bool		shadeboost=front->shadeboost->get();
			bool		feedback=front->feedback->get();
			byte		a,r,g,b;

			vertices[0].x=0;
			vertices[1].x=w;
			vertices[2].x=w;
			vertices[3].x=0;
			vertices[4].x=w;
			vertices[5].x=0;
			
			vertices[0].y=0;
			vertices[1].y=0;
			vertices[2].y=h;
			vertices[3].y=0;
			vertices[4].y=h;
			vertices[5].y=h;

			{
				float x0=0.f;
				float y0=0.f;
				float x1=1.f;
				float y1=1.f;

				vertices[0].u=x0;
				vertices[1].u=x1;
				vertices[2].u=x1;
				vertices[3].u=x0;
				vertices[4].u=x1;
				vertices[5].u=x0;

				vertices[0].v=y0;
				vertices[1].v=y0;
				vertices[2].v=y1;
				vertices[3].v=y0;
				vertices[4].v=y1;
				vertices[5].v=y1;
			}
				
			{
				IDirect3DSurface9	*surf;
				dt->texBG[0]->GetSurfaceLevel(0, &surf);
				ad3d->d3dev->SetRenderTarget(0, surf);
				surf->Release();
			}

			ad3d->d3dev->SetDepthStencilSurface(dt->zbufBG);

			colorRGB(&r, &g, &b, c);
			a=(byte)(v*255.99f);
			c=D3DCOLOR_ARGB(a, r, g, b);
			ad3d->d3dev->Clear(0, NULL, D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, c, 1.f, 0L);
			ad3d->d3dev->SetRenderState(D3DRS_ZENABLE, FALSE);
			ad3d->d3dev->SetRenderState(D3DRS_STENCILENABLE, FALSE);

			if(a)
			{
				vertices[0].color=c;
				vertices[1].color=c;
				vertices[2].color=c;
				vertices[3].color=c;
				vertices[4].color=c;
				vertices[5].color=c;

				if(dt->vb->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
				{
					memcpy(pVertices, vertices, sizeof(vertices));
					dt->vb->Unlock();
				}
				
				ad3d->d3dev->BeginScene();
				res=ad3d->d3dev->SetTexture(0, NULL);
				res=ad3d->d3dev->SetStreamSource(0, dt->vb, 0, sizeof(CUSTOMVERTEX));
				res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
				res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
				res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
				res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);	// test
				res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);	// test
				res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
				res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				res=ad3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				res=ad3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				res=ad3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				res=ad3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				res=ad3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
				ad3d->d3dev->EndScene();
			}
			else
			{
				if(dt->vb->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
				{
					memcpy(pVertices, vertices, sizeof(vertices));
					dt->vb->Unlock();
				}
			}

			res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);	// test
			res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);	// test

			for(i=0; i<NBEF3D; i++)
			{
				int	k=iorder[i];
				Aef3dInfo	info;
				info.d3d=ad3d;
				info.time=time;
				info.dtime=dtime;
				info.beat=beat;
				info.dbeat=dbeat;
				info.power=front->efv[k]->get()*vmaster;
				info.bm=(Aef3dInfo::blendmode)front->efb[k]->get();
				info.in[0]=back->in[0];
				info.in[1]=back->in[1];
				info.in[2]=back->in[2];
				info.in[3]=back->in[3];
				info.sound=sound;
				info.inputs=i3d;
				info.effects=e3d;
				info.current=i;
				info.color=front->efc[k]->get();
				info.colorBoost=front->efbo[k]->get();
				info.input=front->ei3[k];
				info.effect=front->ef3[k];
				if(info.power>0.f)
				{
					info.input->action(&info);
					info.effect->action(&info);
				}
			}

			res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);	// test
			res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);	// test

			dt->section.enter(__FILE__,__LINE__);
			if(front->pixb->get())
			{
				if(dt->fchange)
				{
					dt->fchange=false;
					releasePix();
					initPix();
				}
			}
			else if(dt->fchange)
			{
				dt->fchange=false;
				releasePix();
			}
			dt->section.leave();

			{
				IDirect3DSurface9	*sfback;
				VOID				*pVertices;
				dword				sc=0xffffffff;

				vertices[0].color=sc;
				vertices[1].color=sc;
				vertices[2].color=sc;
				vertices[3].color=sc;
				vertices[4].color=sc;
				vertices[5].color=sc;

				if(dt->vbreg->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
				{
					memcpy(pVertices, vertices, sizeof(vertices));
					dt->vbreg->Unlock();
				}

				if(dt->chain[0])
				{
					if(dt->texBG[1]->GetSurfaceLevel(0, &sfback)==D3D_OK)	
					{
						ad3d->d3dev->SetRenderTarget(0, sfback);

						ad3d->d3dev->BeginScene();

						res=ad3d->d3dev->SetTexture(0, dt->texBG[0]);
						res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
						res=ad3d->d3dev->SetStreamSource(0, dt->vbreg, 0, sizeof(CUSTOMVERTEX));
						res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
						res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
						res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

						res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
						res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

						res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
						res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		
						res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

						dt->section.enter(__FILE__,__LINE__);
						if(dt->pix->effect)
						{
							dt->pix->effect->SetTechnique("elektronika");
							dt->pix->effect->SetTexture("texture0", dt->texBG[0]);
							res=dt->pix->effect->SetTexture("texsound", dt->texsound);
							
							if(dt->pix->hColor)
							{
								D3DXVECTOR4	v;
								v.w=1.f;
								v.x=v.y=v.z=1.f;
								dt->pix->effect->SetVector(dt->pix->hColor, &v);
							}
							
							if(dt->pix->hBeat)
								dt->pix->effect->SetFloat(dt->pix->hBeat, (FLOAT)beat);

							if(dt->pix->hRbeat)
							{
								FLOAT	rbeat=(float)(beat-(float)(int)beat);
								rbeat=(rbeat<0.5f)?((0.5f-rbeat)*2.f):((rbeat-0.5f)*2.f);
								dt->pix->effect->SetFloat(dt->pix->hRbeat, rbeat);
							}

							if(dt->pix->hBass)
								dt->pix->effect->SetFloat(dt->pix->hBass, (FLOAT)sound->bass);
							if(dt->pix->hMedium)
								dt->pix->effect->SetFloat(dt->pix->hMedium, (FLOAT)sound->medium);
							if(dt->pix->hTreeble)
								dt->pix->effect->SetFloat(dt->pix->hTreeble, (FLOAT)sound->treble);
								
							{
								int	i;
								for(i=0; i<dt->pix->nperso; i++)
								{
									float	v=(dt->apad[i]=dt->apad[i]*0.5f+front->pad[i]->get()*0.5f);
									dt->pix->effect->SetFloat(dt->pix->hPerso[i], (FLOAT)v);
								}
							}

							dt->pix->effect->SetTechnique("elektronika");

						}
						dt->section.leave();

						res=ad3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
						res=ad3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
						res=ad3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
						res=ad3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
						res=ad3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

						if(dt->pix->effect)
						{
							UINT uPasses;
							dt->pix->effect->Begin( &uPasses, 0 );
							for( UINT uPass = 0; uPass < uPasses; ++uPass )
							{
								dt->pix->effect->BeginPass( uPass );

								res=ad3d->d3dev->SetStreamSource(0, dt->vb, 0, sizeof(CUSTOMVERTEX));
								res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
								res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

								dt->pix->effect->EndPass();
							}
							dt->pix->effect->End();
						}
						else
							res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

						ad3d->d3dev->EndScene();
						sfback->Release();
					}
				}
			}

			{	// send video to dimension output pin
				Abitmap				*b=back->out->getBitmap();
				if(b)
				{
					{
						IDirect3DSurface9	*sfback;
						if(dt->texT->GetSurfaceLevel(0, &sfback)==D3D_OK)
						{
							dword		sc=front->shadecolor->get()|0xff000000;
							ad3d->d3dev->BeginScene();
							ad3d->d3dev->SetRenderTarget(0, sfback);
							drawTexture(ad3d, dt, 0, 0, this->getVideoWidth(), this->getVideoHeight(), dt->texBG[1], sc, shadeboost);
							sfback->Release();
							ad3d->d3dev->EndScene();
						}
					}

					{
						IDirect3DSurface9 *sfT;
						IDirect3DSurface9 *sfD;
						if(dt->texT->GetSurfaceLevel(0, &sfT)==D3D_OK)
						{
							if(dt->texD->GetSurfaceLevel(0, &sfD)==D3D_OK)
							{
								HRESULT r=ad3d->d3dev->GetRenderTargetData(sfT, sfD);
								sfD->Release();
							}
							sfT->Release();
						}
					}

					{
						IDirect3DSurface9 *sfD;
						if(dt->texD->GetSurfaceLevel(0, &sfD)==D3D_OK)
						{
							D3DLOCKED_RECT lck;
							RECT rectT;
							rectT.left=0;
							rectT.top=0;
							rectT.right=this->getVideoWidth();
							rectT.bottom=this->getVideoHeight();
							if(sfD->LockRect(&lck, &rectT, D3DLOCK_DISCARD)==D3D_OK)
							{
								dword	*d=b->body32;
								byte	*s=(byte *)lck.pBits;
								int y;
								for(y=0; y<b->h; y++)
								{
									memcpy(d, s, b->w*4);
									d+=b->w;
									s+=lck.Pitch;
								}
								sfD->UnlockRect();
							}
							sfD->Release();
						}
					}
				}
			}

			{
				{	// send to videout (real screen)
					IDirect3DSurface9	*sfback;
					if(dt->chain[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &sfback)==D3D_OK)
					{
						Apoint p[4];
						p[0]=Apoint(w*place[0].x/511, h*place[0].y/511);
						p[1]=Apoint(w*place[1].x/511, h*place[1].y/511);
						p[2]=Apoint(w*place[2].x/511, h*place[2].y/511);
						p[3]=Apoint(w*place[3].x/511, h*place[3].y/511);
						dword		sc=front->shadecolor->get()|0xff000000;
						ad3d->d3dev->BeginScene();
						ad3d->d3dev->SetRenderTarget(0, sfback);
						if(aNewMask)
							ad3d->d3dev->Clear(0, NULL, D3DCLEAR_TARGET, c, 1.f, 0L);
						drawTexture(ad3d, dt, p, dt->texBG[1], sc, shadeboost);
						sfback->Release();
						ad3d->d3dev->EndScene();
					}
				}

				if(feedback)
				{
					double	feedzoom=pow((front->feedzoom->get()-0.5f)*2.f, 3.f);
					if(feedzoom<0.0)
						feedzoom*=0.25f;
					IDirect3DSurface9	*sfback;
					if(dt->texBG[0]->GetSurfaceLevel(0, &sfback)==D3D_OK)
					{
						ad3d->d3dev->BeginScene();
						{
							double	zx=feedzoom*w;
							double	zy=feedzoom*h;
							ad3d->d3dev->SetRenderTarget(0, sfback);
							drawTexture(ad3d, dt, -(float)zx, -(float)zy, (float)((w)+zx), (float)((h)+zy), dt->texBG[1], 0xffffffff, false);
						}
						ad3d->d3dev->EndScene();
						sfback->Release();
					}
				}
				else
				{
					IDirect3DTexture9 *t=dt->texBG[0];
					dt->texBG[0]=dt->texBG[1];
					dt->texBG[1]=t;
				}
			}



			{	// apply mask	// update textures with new mask
				sMask.enter(__FILE__,__LINE__);
				if(aNewMask)
				{
					IDirect3DSurface9	*surf;
					if(dt->texMaskS->GetSurfaceLevel(0, &surf)==D3D_OK)
					{
						Abitmap *b=imgMask;
						{
							RECT srcR={0, 0, b->w, b->h};
							D3DLOCKED_RECT lck;
							srcR.left=0;
							srcR.top=0;
							srcR.right=b->w;
							srcR.bottom=b->h;
							if(surf->LockRect(&lck, &srcR, D3DLOCK_DISCARD)==D3D_OK)
							{
								dword *s=b->body32;
								byte *d=(byte *)lck.pBits;
								int y;
								for(y=0; y<b->h; y++)
								{
									memcpy(d, s, b->w*4);
									s+=b->w;
									d+=lck.Pitch;
								}
								surf->UnlockRect();
							}
						}
						surf->Release();
						ad3d->d3dev->UpdateTexture(dt->texMaskS, dt->texMaskD);
					}
				}
				sMask.leave();
			}

			{	// apply mask // set texture in alpha blending
				IDirect3DSurface9	*sfback;
				VOID				*pVertices;

				{
					float	x0=0;
					float	y0=0;
					float	x1=w;
					float	y1=h;
					
					vertices[0].x=x0;
					vertices[1].x=x1;
					vertices[2].x=x1;
					vertices[3].x=x0;
					vertices[4].x=x1;
					vertices[5].x=x0;
				
					vertices[0].y=y0;
					vertices[1].y=y0;
					vertices[2].y=y1;
					vertices[3].y=y0;
					vertices[4].y=y1;
					vertices[5].y=y1;
				}

				{
					float x0=0.01f;
					float y0=0.01f;
					float x1=0.99f;
					float y1=0.99f;

					vertices[0].u=x0;
					vertices[1].u=x1;
					vertices[2].u=x1;
					vertices[3].u=x0;
					vertices[4].u=x1;
					vertices[5].u=x0;

					vertices[0].v=y0;
					vertices[1].v=y0;
					vertices[2].v=y1;
					vertices[3].v=y0;
					vertices[4].v=y1;
					vertices[5].v=y1;
				}
				
				vertices[0].color=0xffffffff;
				vertices[1].color=0xffffffff;
				vertices[2].color=0xffffffff;
				vertices[3].color=0xffffffff;
				vertices[4].color=0xffffffff;
				vertices[5].color=0xffffffff;

				if(dt->vbreg->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
				{
					memcpy(pVertices, vertices, sizeof(vertices));
					dt->vbreg->Unlock();
				}
				
				if(dt->chain[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &sfback)==D3D_OK)
				{
					ad3d->d3dev->SetRenderTarget(0, sfback);

					ad3d->d3dev->BeginScene();
					res=ad3d->d3dev->SetTexture(0, dt->texMaskD);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
					res=ad3d->d3dev->SetStreamSource(0, dt->vbreg, 0, sizeof(CUSTOMVERTEX));
					res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
					res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
					res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
					res=ad3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					res=ad3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					res=ad3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					res=ad3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					res=ad3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
					res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
					ad3d->d3dev->EndScene();

					sfback->Release();
				}
			}

			/*
			if(!OKI_registred)
			{
				IDirect3DSurface9	*sfback;
				VOID				*pVertices;

				{
					float	x0=0.f;
					float	y0=0.f;
					float	x1=w;
					float	y1=h;
					
					vertices[0].x=x0;
					vertices[1].x=x1;
					vertices[2].x=x1;
					vertices[3].x=x0;
					vertices[4].x=x1;
					vertices[5].x=x0;
				
					vertices[0].y=y0;
					vertices[1].y=y0;
					vertices[2].y=y1;
					vertices[3].y=y0;
					vertices[4].y=y1;
					vertices[5].y=y1;
				}
				
				vertices[0].color=0xffffffff;
				vertices[1].color=0xffffffff;
				vertices[2].color=0xffffffff;
				vertices[3].color=0xffffffff;
				vertices[4].color=0xffffffff;
				vertices[5].color=0xffffffff;

				if(dt->vbreg->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
				{
					memcpy(pVertices, vertices, sizeof(vertices));
					dt->vbreg->Unlock();
				}
				
				if(dt->chain[0]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &sfback)==D3D_OK)
				{
					ad3d->d3dev->SetRenderTarget(0, sfback);

					ad3d->d3dev->BeginScene();
					res=ad3d->d3dev->SetTexture(0, dt->texreg);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
					res=ad3d->d3dev->SetStreamSource(0, dt->vbreg, 0, sizeof(CUSTOMVERTEX));
					res=ad3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
					res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
					res=ad3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
					res=ad3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					res=ad3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
					res=ad3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					res=ad3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					res=ad3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
					res=ad3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
					res=ad3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
					ad3d->d3dev->EndScene();

					sfback->Release();
				}
			}
			*/

			switch(ad3d->d3dev->Present(NULL, NULL, NULL, NULL))
			{
				case D3D_OK:
				renderSTATUS=renderOK;
				dt->nlost=0;
				break;
				
				case D3DERR_DRIVERINTERNALERROR:
				renderSTATUS=renderINTERNALERROR;
				break;
				
				case D3DERR_INVALIDCALL:
				case D3DERR_DEVICELOST:
				dt->fchange=true;	// re_init dimension pixshader 
				dt->nlost++;
				if(dt->nlost>5)
					renderSTATUS=renderDEVICELOST;
				break;
			}

		}
	}
	section.leave();
	winChange.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::initPix()
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	dt->section.enter(__FILE__,__LINE__);
	dt->pixok=dt->pix->init(ad3d->d3dev, front->pixl->getCurrentData());
	dt->pixerror=!dt->pixok;
	dt->bchanged=true;
	dt->section.leave();
}

void Av3dout::releasePix()
{
	dt->section.enter(__FILE__,__LINE__);
	dt->pixok=false;
	dt->section.leave();
	dt->pix->release();
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Av3dout::initWin()
{
	int	k;
	
	win=NULL;
	renderSTATUS=renderOK;
	dt->nlost=0;

	if(current==0)
	{
		return 0;
	}
	if(current==1)
	{
		win=new Av3doutDisp("display", winX, winY, getVideoWidth(), getVideoHeight(), this);
		win->show(TRUE);
		win->repaint();
		ad3d->createDevice(win);
	}
	else if((current>=10)&&(current<42))
	{
		int			dev=current-10;
		HMONITOR	hm=ad3d->getAdapterMonitor(dev);
		MONITORINFO	mi;
		mi.cbSize=sizeof(mi);
		if(!GetMonitorInfo(hm, &mi))
		{
			return -1;
		}

		int	x=mi.rcMonitor.left;
		int	y=mi.rcMonitor.top;
		int	w=mi.rcMonitor.right-x;
		int	h=mi.rcMonitor.bottom-y;

		{
			Alaunch	*l=new Alaunch(this, x, y, w, h, ad3d, current-10);
			l->start();
			while(!l->ok)
				Athread::sleep(1);
			hwnd=l->hwnd;
			delete(l);
		}
	}
	else
		return 0;

	if(((Av3doutFront *)front)->bPreview->isChecked())
	{
		preview=new Av3doutDisp("display", winX, winY, getVideoWidth(), getVideoHeight(), this);
		preview->show(TRUE);
		preview->repaint();
	}
	
	if(!ad3d->isOK)
		return -1;

	ad3d->d3dev->GetSwapChain( 0, &dt->chain[0] );
	if(preview)
		dt->chain[1]=(IDirect3DSwapChain9 *)ad3d->addSwapChain(preview);

	//setPriority(priorityHIGH);
	
	if(FAILED(ad3d->d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &dt->vb, NULL)))
		return -2;

	if(FAILED(ad3d->d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &dt->vbreg, NULL)))
		return -2;
	
	{
		Aresobj				o=resource.get(MAKEINTRESOURCE(PNG_ELEKREG), "PNG");
		Abitmap				*b=new Abitmap(&o);
		IDirect3DTexture9	*tex;
		
		if(D3DXCreateTexture(ad3d->d3dev, 256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &tex)!=D3D_OK)
			return -2;
		if(D3DXCreateTexture(ad3d->d3dev, 256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dt->texreg)!=D3D_OK)
			return -2;

		if(b)
		{
			IDirect3DSurface9	*surf;
			if(tex->GetSurfaceLevel(0, &surf)==D3D_OK)
			{
				D3DLOCKED_RECT	lrect;
				if(surf->LockRect(&lrect, NULL, D3DLOCK_READONLY)==D3D_OK)
				{
					int		y;
					for(y=0; y<256; y++)
					{
						byte	*d=(byte *)lrect.pBits+lrect.Pitch*y;
						byte	*s=(byte *)&b->body32[b->adr[y]];
						memcpy(d, s, 256*4);
					}
					surf->UnlockRect();
				}
				surf->Release();
			}
			dt->texreg->AddDirtyRect(NULL);
			ad3d->d3dev->UpdateTexture(tex, dt->texreg);
			delete(b);
		}
		tex->Release();
	}

	if(D3DXCreateTexture(ad3d->d3dev, ad3d->w, ad3d->h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dt->texBG[0])!=D3D_OK)
		return -2;

	if(D3DXCreateTexture(ad3d->d3dev, ad3d->w, ad3d->h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dt->texBG[1])!=D3D_OK)
		return -2;

	if(D3DXCreateTexture(ad3d->d3dev, TEXSOUNDW, TEXSOUNDH, 1, 0, D3DFMT_V16U16, D3DPOOL_DEFAULT, &dt->texsound)!=D3D_OK)
		return -2;

	if(D3DXCreateTexture(ad3d->d3dev, TEXSOUNDW, TEXSOUNDH, 1, 0, D3DFMT_V16U16, D3DPOOL_SYSTEMMEM, &dt->texsoundcpy)!=D3D_OK)
		return -2;

	// out textures
	int vw=this->getVideoWidth();
	int vh=this->getVideoHeight();

	if(D3DXCreateTexture(ad3d->d3dev, vw, vh, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dt->texT)!=D3D_OK)
		return -2;

	if(D3DXCreateTexture(ad3d->d3dev, vw, vh, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &dt->texD)!=D3D_OK)
		return -2;

	if(D3DXCreateTexture(ad3d->d3dev, 512, 512, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &dt->texMaskS)!=D3D_OK)
		return false;

	if(D3DXCreateTexture(ad3d->d3dev, 512, 512, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &dt->texMaskD)!=D3D_OK)
		return false;

	{
		D3DSURFACE_DESC desc;
		IDirect3DSurface9	*surf;
		dt->texBG[0]->GetSurfaceLevel(0, &surf);
		surf->GetDesc(&desc);
		surf->Release();
		if(ad3d->d3dev->CreateDepthStencilSurface(ad3d->w, ad3d->h, D3DFMT_D24S8, desc.MultiSampleType, 0, FALSE, &dt->zbufBG, NULL)!=D3D_OK)
			return -2;
	}

	winrun=TRUE;
	runout=false;
	error=1;

	section.enter(__FILE__,__LINE__);
	for(k=0; k<NBEF3D; k++)
		init3D(k, true);
	ef3run=true;
	section.leave();
	
	newMask=true;

	return 0;
}

void Av3dout::closeWin()
{
	int	k;
	
	section.enter(__FILE__,__LINE__);
	for(k=0; k<NBEF3D; k++)
		release3D(k, true);
	ef3run=false;
	section.leave();

	if(dt->vb)
		dt->vb->Release();
	dt->vb=NULL;
	
	if(dt->vbreg)
		dt->vbreg->Release();
	dt->vbreg=NULL;
	
	if(dt->texreg)
		dt->texreg->Release();
	dt->texreg=NULL;

	if(dt->texBG[0])
		dt->texBG[0]->Release();
	dt->texBG[0]=NULL;

	if(dt->texBG[1])
		dt->texBG[1]->Release();
	dt->texBG[1]=NULL;

	if(dt->texsound)
		dt->texsound->Release();
	dt->texsound=NULL;

	if(dt->texsoundcpy)
		dt->texsoundcpy->Release();
	dt->texsoundcpy=NULL;

	if(dt->texT)
		dt->texT->Release();
	dt->texT=NULL;

	if(dt->texD)
		dt->texD->Release();
	dt->texD=NULL;

	if(dt->texMaskS)
		dt->texMaskS->Release();
	dt->texMaskS=NULL;

	if(dt->texMaskD)
		dt->texMaskD->Release();
	dt->texMaskD=NULL;

	if(dt->zbufBG)
		dt->zbufBG->Release();
	dt->zbufBG=NULL;

	if(dt->chain[0])
		dt->chain[0]->Release();
	dt->chain[0]=null;

	if(dt->chain[1])
		dt->chain[1]->Release();
	dt->chain[1]=null;

	ad3d->releaseDevice();
	
	if(win)
		delete(win);
	win=NULL;

	if(preview)
		delete(preview);
	preview=null;

	if(hwnd)
		DestroyWindow(hwnd);
	hwnd=NULL;

	runout=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::run()
{
	MSG				msg;
	
	CoInitialize(NULL);

	memset(&msg, 0, sizeof(msg));
	
	if(!initWin())
	{
		error=-1;
		goto lblOut;
	}

	while(winrun)
	{
		while(PeekMessage(&msg, NULL, 0, 0, TRUE)&&(msg.message!=WM_QUIT)&&(winrun))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		sleep(1);
	}
	
lblOut:
	closeWin();
	CoUninitialize();

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::init3D(int k, bool effect)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	Aef3dInfo		info;
	
	info.d3d=ad3d;
	info.time=0;
	info.dtime=0;
	info.beat=0;
	info.dbeat=0;
	info.power=0.f;
	info.bm=Aef3dInfo::blendFADE;
	info.in[0]=NULL;
	info.in[1]=NULL;
	info.in[2]=NULL;
	info.in[3]=NULL;
	info.sound=NULL;
	info.inputs=null;
	info.effects=null;
	info.current=-1;
	info.color=0xffffffff;
	info.colorBoost=false;
	info.input=front->ei3[k];
	info.effect=front->ef3[k];

	info.input->init3D(&info);
	if(effect)
		info.effect->init3D(&info);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::release3D(int k, bool effect)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	Aef3dInfo		info;
	
	info.d3d=ad3d;
	info.time=0;
	info.dtime=0;
	info.beat=0;
	info.dbeat=0;
	info.power=0.f;
	info.bm=Aef3dInfo::blendFADE;
	info.in[0]=NULL;
	info.in[1]=NULL;
	info.in[2]=NULL;
	info.in[3]=NULL;
	info.sound=NULL;
	info.inputs=null;
	info.effects=null;
	info.current=-1;
	info.color=0xffffffff;
	info.colorBoost=false;
	info.input=front->ei3[k];
	info.effect=front->ef3[k];
	
	info.input->release3D(&info);
	if(effect)
		info.effect->release3D(&info);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::setEf3List(int k, Aeffect3dInfo *e3di)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	Aeffect3d		*e3d;
	int				y=POSEF(front->order[k]);
	section.enter(__FILE__,__LINE__);
	if(ef3run)
		release3D(k, true);
	delete(front->ef3[k]);
	char	str[1024];
	sprintf(str, "%d/%s", k, e3di->name);
	e3d=front->ef3[k]=e3di->create(MKQIID(getQIID(), qidef[k]), str, front, 154, y-7);
	if(ef3run)
		init3D(k, true);
	section.leave();
	e3d->show(true);
	front->repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3dout::setEi3List(int k, Ainput3dInfo *i3di)
{
	Av3doutFront	*front=(Av3doutFront *)this->front;
	Ainput3d		*i3d;
	int				y=POSEF(front->order[k]);
	section.enter(__FILE__,__LINE__);
	if(ef3run)
		release3D(k, false);
	delete(front->ei3[k]);
	char	str[1024];
	sprintf(str, "%d/%s", k, i3di->name);
	i3d=front->ei3[k]=i3di->create(MKQIID(getQIID(), qidin[k]), str, front, 15, y-7);
	if(ef3run)
		init3D(k, false);
	section.leave();
	i3d->show(true);
	front->repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3doutFront::pulse()
{
	Av3dout	*v3dout=(Av3dout *)effect;
	if(v3dout->winrun)
	{
		if(v3dout->escape)
		{
			v3dout->change(0);
			list->setCurrentByData(0);
			v3dout->escape=false;
			return;
		}
		if(v3dout->renderSTATUS!=renderOK)
		{
			switch(v3dout->renderSTATUS)
			{
				case renderINTERNALERROR:
				notify(this, nyALERT, (dword)"[directX] video driver internal error");
				break;
				
				case renderDEVICELOST:
				notify(this, nyALERT, (dword)"[directX] device lost");
				break;
			}
			v3dout->renderSTATUS=renderOK;
			if(!v3dout->change(list->getCurrentData()))
				list->setCurrentByData(0);
			return;
		}
	}
	{
		MYwindow	*w=(MYwindow *)getWindow();
		Aregistry	*reg=w->regpwd;
		char		sname[256]="";
		char		spwd[256]="";

		reg->read("name", sname, sizeof(sname));
		reg->read("pwd", spwd, sizeof(spwd));
		
		v3dout->OKI_registred=true; //VerifyPassword(sname, spwd);
	}

	{
		Afont		*font=alib.getFont(fontTERMINAL05);
		AdataV3d	*dt=((Av3dout *)effect)->dt;

		dt->section.enter(__FILE__,__LINE__);
		if(dt->bchanged)
		{
			int	i;
			dt->bchanged=false;
			for(i=0; i<dt->pix->nperso; i++)
			{
				char	str[1024];
				strcpy(str, dt->pix->namePerso[i]);
				strupr(str);
				pad[i]->setTooltips(dt->pix->namePerso[i]);
				sta[i]->set(str, 0xffBBB29F, font, Astatic::CENTER);
			}
		}
		dt->section.leave();

		if(dt->pixerror)
		{
			notify(this, nyERROR, (dword)"this shader can't be compiled on this 3D hardware");
			dt->pixerror=false;
			pixb->set(false);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3doutFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3doutFront::Av3doutFront(QIID qiid, char *name, Av3dout *e, int h) : AeffectFront(qiid, name, e, h)
{
	setTooltips("DIMENSION module");
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VIDEO3DOUT), "PNG");
	back=new Abitmap(&o);

	{
		int	i;
		for(i=0; i<NBEF3D; i++)
			order[i]=i;
	}
	
	items=new Aitem("video out", "video out");

	winMask=new Av3dMask("mask", 10, 10, (Av3dout*)effect);

/*
	if(!dxInit)
	{
		DDenum();
		dxInit=TRUE;
	}
*/

	new Aitem("[none]", "no video out", items, 0);
	
	if(e->ad3d->isOK)
	{
		new Aitem("[window] a window", "a simple window", items, (dword)1);
		{
			char	nm[1024],name[1024];
			int		nb=e->ad3d->getAdapterCount();
			int		i;
			for(i=0; i<nb; i++)
			{
				e->ad3d->getAdapterName(name, i);
				sprintf(nm, "[DX] %s", name);
				new Aitem(nm, name, items, (dword)10+i);
			}
		}
	}

/*
	new Aitem("[window] overlay window", "overlay window - activate dual head DVDmax on matrox board", items, (dword)2);

	{  
		int	i;
		for(i=0; i<nbDDinfo; i++)
		{
			char	str[256];
			sprintf(str, "[direct-x]   %s (%d)", DDinfo[i].desc, i+1);
			new Aitem(str, DDinfo[i].desc, items, (dword)(i+10));
		}
	}
*/
	mask=new Abutton("mask", this, 128, 24, 16, 16, &resource.get(MAKEINTRESOURCE(PNG_V3DOUT_MASKICON), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	mask->setTooltips("edit maskink area");	
	mask->show(true);

	bPreview=new Abutton("preview", this, 440, 22, 40, 20, "PREVIEW", Abutton::btCAPTION|Abutton::bt2STATES);
	bPreview->setTooltips("preview window");
	//bPreview->show(true);

	vol=new Apaddle(MKQIID(qiid, 0x04cdaef5acbec440), "master/clear power", this, 51, 20, paddleYOLI24);
	vol->setTooltips("clear power");
	vol->set(1.f);
	vol->show(TRUE);
			
	color=new Aselcolor(MKQIID(qiid, 0x723a2105c9d0e084), "master/clear color", this, 32, 24);
	color->setTooltips("clear color");
	color->set(0xff000000);
	color->show(true);

	master=new Apaddle(MKQIID(qiid, 0x04cd75578cbec440), "master/master power", this, 404, 20, paddleYOLI24);
	master->setTooltips("master power");
	master->set(1.f);
	master->show(TRUE);

	shadeboost=new ActrlButton(MKQIID(qiid, 0xb5845e8d4c5ae7a0), "master/color boost", this, 461, 28, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	shadeboost->setTooltips("master color boost");
	shadeboost->set(false);
	shadeboost->show(true);

	shadecolor=new Aselcolor(MKQIID(qiid, 0x723a2445484a4cd4), "master/out color", this, 474, 24);
	shadecolor->setTooltips("master color");
	shadecolor->set(0xffffffff);
	shadecolor->show(true);

	list=new Alist("video out", this, 164, 22, 217, 20, items);
	list->setTooltips("video out selector");
	list->show(TRUE);

	feedback=new ActrlButton(MKQIID(qiid, 0xb5ade548c5e5e7a0), "feedback/activate", this, 39, h-32, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	feedback->setTooltips("feedback on:off");
	feedback->set(false);
	feedback->show(true);
	
	feedzoom=new Apaddle(MKQIID(qiid, 0x04c85d0e566845ad), "feedback/zoom", this, 51, h-40, paddleYOLI24);
	feedzoom->setTooltips("feedback zoom");
	feedzoom->set(0.5f);
	feedzoom->show(TRUE);
			
	pixb=new ActrlButton(MKQIID(qiid, 0xb5a581218ed48fc8), "shader/activate", this, 160, h-40, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	pixb->setTooltips("shader on:off");
	pixb->set(false);
	pixb->show(true);
	
	{
		Afont	*font=alib.getFont(fontTERMINAL05);
		char	str[256];
		int		i;
		for(i=0; i<PIX_MAXINPUT; i++)
		{
			int	x=i*26+158;
			int	y=(i&1)*11+h-57;
			int	y2=(~(i&1))*16+h-8;
			//int	y=0;
			sprintf(str, "shader/sta[%d]", i);
			sta[i]=new Astatic(str, this, x+8, y2, 38, 10);
			sta[i]->set("", 0xffBBB29F, font, Astatic::CENTER);
			sta[i]->show(true);

			sprintf(str, "shader/pad[%d]", i);
			pad[i]=new Apaddle(MKQIID(qiid,pad_qiid[i]), str, this, 18+x, 16+y, paddleYOLI16);
			pad[i]->setTooltips("");
			pad[i]->set((i>0)?0.5f:0.f);
			
			pad[i]->show(TRUE);
		}
	}

	pixi=new Aitem("shaders", "shaders list");
	
	{ // add list of fx effects
		Aitem	*sel=null;
		int		i;
		for(i=0; i<e->dt->pix->nfile; i++)
		{
			char	str[ALIB_PATH];
			char	*s=strrchr(e->dt->pix->filename[i], '\\');
			strcpy(str, s+1);
			*strstr(str, ".fx")=0;
			sel=new Aitem(str, str, pixi, i);
		}
		pixi->sort();
		
		pixl=new Alist("shaders", this, 402, h-36, 88, 16, pixi);
		pixl->setTooltips("shaders");
		pixl->setCurrent(sel);
		pixl->show(TRUE);
	}

	
	efi=new Aitem("effect", "effect 3D list");
	eii=new Aitem("input", "input 3D list");
	
	{
		static QIID qiidefv[NBEF3D]={ 0xb5af70cf9ce5e7a0, 0x554f366251f0aa98, 0xb325b12858f97d64, 0xb325462858f97d64 };
		static QIID qiidefb[NBEF3D]={ 0x18d7f9e18a8e1010, 0x58538902b8959200, 0x42de7732384ad168, 0xb325b12887897d64 };
		static QIID qiidefbo[NBEF3D]={ 0xc6d0eec4d4beb230, 0x1a74d251db682dd4, 0x4c937ccca2435ba0, 0xb325435458f97d64 };
		static QIID qiidefc[NBEF3D]={ 0x9d01fa6bedab99e0, 0x77ccec6221cf63ae, 0x452a30a418a4a000, 0xb390972858f97d64 };
		int	k;
		for(k=0; k<NBEF3D; k++)
		{
			char	str[1024];
			int	y=POSEF(k);
			
			sprintf(str, "%d/blend/power", k);
			efv[k]=new Apaddle(MKQIID(qiid, qiidefv[k]), str, this, 404, y-6, paddleYOLI24);
			efv[k]->setTooltips("effect power");
			efv[k]->set((k==0)?1.f:0.f);
			efv[k]->show(TRUE);
			
			up[k]=new Abutton("up[]", this, 440, y+2, 16, 9, &resource.get(MAKEINTRESOURCE(PNG_V3DOUT_UP), "PNG"), Abutton::btBITMAP);
			up[k]->setTooltips("deplace layer");
			up[k]->show(true);

			down[k]=new Abutton("down[]", this, 460, y+2, 16, 9, &resource.get(MAKEINTRESOURCE(PNG_V3DOUT_DOWN), "PNG"), Abutton::btBITMAP);
			down[k]->setTooltips("deplace layer");
			down[k]->show(true);

			eil[k]=new Alist("input list", this, 402, y+18, 88, 16, eii);
			eil[k]->setTooltips("input list");
			eil[k]->show(TRUE);

			efl[k]=new Alist("effect list", this, 402, y+38, 88, 16, efi);
			efl[k]->setTooltips("effect list");
			efl[k]->show(TRUE);

			sprintf(str, "%d/blend/blend op", k);
			efb[k]=new Aselect(MKQIID(qiid, qiidefb[k]), str, this, 404, y+58, 3, 1, &resource.get(MAKEINTRESOURCE(PNG_EF3D_BLEND), "PNG"), 16, 16);
			efb[k]->setTooltips("blend mode [fade/add/sub]");
			efb[k]->set(0);
			efb[k]->show(TRUE);

			sprintf(str, "%d/blend/luma boost", k);
			efbo[k]=new ActrlButton(MKQIID(qiid, qiidefbo[k]), str, this, 461, y+62, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
			efbo[k]->setTooltips("luminosity boost");
			efbo[k]->set(false);
			efbo[k]->show(TRUE);

			sprintf(str, "%d/blend/color", k);
			efc[k]=new Aselcolor(MKQIID(qiid, qiidefc[k]), str, this, 474, y+58);
			efc[k]->setTooltips("color");
			efc[k]->set(0xffffffff);
			efc[k]->show(true);
		}
	}

	{
		Aitem	*pi=(Aitem *)e->getTable()->plugz->fchild;
		Aitem	*sele=NULL;
		Aitem	*seli=NULL;
		while(pi)
		{
			Aplugz	*p=(Aplugz *)pi->link;
			if(p->isCI(&Aeffect3dInfo::CI))
			{
				Aitem	*i=new Aitem(p->ename, p->ehelp, efi, p);
				sele=i;
			}
			if(p->isCI(&Ainput3dInfo::CI))
			{
				Aitem	*i=new Aitem(p->ename, p->ehelp, eii, p);
				seli=i;
			}
			pi=(Aitem *)pi->next;
		}
		{
			int	k;
			for(k=0; k<NBEF3D; k++)
			{
				int	y=POSEF(k);
				char	str[1024];

				eil[k]->setCurrent(seli);
				e->section.enter(__FILE__,__LINE__);
				sprintf(str, "%d/%s", k, ((Ainput3dInfo *)(seli->link))->name);
				ei3[k]=((Ainput3dInfo *)(seli->link))->create(MKQIID(qiid, qidin[k]), str, this, 15, y-7);
				ei3[k]->show(true);
				e->section.leave();
				
				efl[k]->setCurrent(sele);
				e->section.enter(__FILE__,__LINE__);
				sprintf(str, "%d/%s", k, ((Aeffect3dInfo *)(sele->link))->name);
				ef3[k]=((Aeffect3dInfo *)(sele->link))->create(MKQIID(qiid, qidef[k]), str, this, 154, y-7);
				ef3[k]->show(true);
				e->section.leave();
			}
		}
	}

	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3doutFront::~Av3doutFront()
{
	int	k;
	delete(back);
	delete(list);
	delete(items);

	delete(winMask);

	for(k=0; k<NBEF3D; k++)
	{
		delete(down[k]);
		delete(up[k]);
		delete(eil[k]);
		delete(ei3[k]);
		delete(efb[k]);
		delete(efl[k]);
		delete(efv[k]);
		delete(efbo[k]);
		delete(efc[k]);
		delete(ef3[k]);
	}
	
	delete(efi);
	delete(eii);
	delete(color);
	delete(vol);
	delete(shadecolor);
	delete(shadeboost);
	delete(feedback);
	delete(feedzoom);
	delete(master);
	delete(mask);
	delete(bPreview);

	{
		int	i;
		for(i=0; i<PIX_MAXINPUT; i++)
		{
			delete(pad[i]);
			delete(sta[i]);
		}
	}

	delete(pixb);
	delete(pixi);
	delete(pixl);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Av3doutFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCLOSE:
		if(o==winMask)
		{
			winMask->show(false);
			mask->setChecked(false);
		}
		break;

		case nyCHANGE:
		if(o==winMask)
		{
			Av3dout *v3d=(Av3dout*)effect;
			v3d->sMask.enter(__FILE__,__LINE__);
			winMask->getMask(v3d->imgMask);
			winMask->getPlace(v3d->place);
			v3d->newMask=true;
			v3d->sMask.leave();
		}
		else if(o==mask)
		{
			if(!p)
			{
				winMask->show(false);
			}
			else
			{
				winMask->show(true);
				winMask->repaint();
			}
		}
		else if((o==pixl)||(o==pixb))
		{
			Afont		*font=alib.getFont(fontTERMINAL05);
			AdataV3d	*dt=((Av3dout *)effect)->dt;
			int		i;
			dt->section.enter(__FILE__,__LINE__);
			for(i=0; i<PIX_MAXINPUT; i++)
			{
				pad[i]->setTooltips("");
				sta[i]->set("", 0xff404040, font, Astatic::CENTER);
				float	v=(i>0)?0.5f:0.f;
				pad[i]->set(v);
				dt->apad[i]=v;
			}
			dt->fchange=true;
			dt->section.leave();
		}
		else if((o==list)||(o==bPreview))
		{
			if(!((Av3dout *)effect)->change(list->getCurrentData()))
			{
				list->setCurrentByData(0);
				bPreview->setChecked(false);
			}
			return true;
		}
		else
		{
			int	k;
			for(k=0; k<NBEF3D; k++)
			{
				if(o==efl[k])
				{
					Av3dout	*v3d=(Av3dout *)effect;
					Aitem	*i=efl[k]->getCurrent();
					v3d->setEf3List(k, (Aeffect3dInfo *)i->link);
				}
				else if(o==eil[k])
				{
					Av3dout	*v3d=(Av3dout *)effect;
					Aitem	*i=eil[k]->getCurrent();
					v3d->setEi3List(k, (Ainput3dInfo *)i->link);
				}
			}
		}
		break;
		
		case nyCLICK:
		{
			int	k;
			for(k=0; k<NBEF3D; k++)
			{
				if(o==up[k])
					moveEffect(k, -1);
				else if(o==down[k])
					moveEffect(k, 1);
			}
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3doutFront::moveEffect(int n, int p)
{
	Av3dout *v3dout=(Av3dout *)effect;	
	v3dout->section.enter(__FILE__,__LINE__);
	if(p)
	{
		int		i;
		bool	b=false;
		int		oo=order[n];
		order[n]+=p;
		if(order[n]<0)
		{
			order[n]=NBEF3D-1;
			b=true;
		}
		else if(order[n]>=NBEF3D)
		{
			order[n]=0;
			b=true;
		}
		if(b)
		{
			for(i=0; i<NBEF3D; i++)
			{
				if(i!=n)
					order[i]+=p;
			}
		}
		else
		{
			for(i=0; i<NBEF3D; i++)
			{
				if((i!=n)&&(order[i]==order[n]))
				{
					order[i]=oo;
					break;
				}
			}
		}
	}
	v3dout->section.leave();
	{
		int	i;
		for(i=0; i<NBEF3D; i++)
		{
			int	y=POSEF(order[i]);
			efv[i]->pos.y=y-6;
			up[i]->pos.y=y+2;
			down[i]->pos.y=y+2;
			eil[i]->pos.y=y+18;
			efl[i]->pos.y=y+38;
			efb[i]->pos.y=y+58;
			efbo[i]->pos.y=y+62;
			efc[i]->pos.y=y+58;
			ei3[i]->pos.y=y-7;
			ef3[i]->pos.y=y-7;
		}
	}
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID	qiidin[]={	0x7eff57c0ad279040, 0xbd7ef9f4c33443a6, 0xd6dfb039d8dd3e44, 0xeaebaa3ca477a559,
						0x02a3813fd62e0aa8, 0x0448d5e554fa4ec0, 0xc74811adff626eb0, 0x87a352833b5700e8};
						
Av3doutBack::Av3doutBack(QIID qiid, char *name, Av3dout *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_VIDEO3DOUT2), "PNG");
	back=new Abitmap(&o);

	{
		int	i;
		for(i=0; i<effect3dNBVIDEO; i++)
		{
			char	str[128];
			sprintf(str, "%d/input", i+1);
			in[i]=new Avideo(MKQIID(qiid, qiidin[i]), str, this, pinIN, 16+19*i, 12);
			in[i]->setTooltips(str);
			in[i]->show(TRUE);
		}
	}
	
	ins=new Asample(MKQIID(qiid, 0x1072d058e6a14660), "audio input", this, pinIN, 120, 12);
	ins->setTooltips("audio input");
	ins->show(TRUE);
	
	out=new Avideo(MKQIID(qiid, 0x09a053945d867f30), "video output", this, pinOUT, pos.w-26, 12);
	out->setTooltips("video output");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Av3doutBack::~Av3doutBack()
{
	delete(back);
	delete(in[0]);
	delete(in[1]);
	delete(in[2]);
	delete(in[3]);
	delete(ins);
	delete(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Av3doutBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * Av3doutInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Av3dout(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * v3doutGetInfo()
{
	return new Av3doutInfo("v3doutInfo", &Av3dout::CI, "dimension", "dimension - 3D module");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
