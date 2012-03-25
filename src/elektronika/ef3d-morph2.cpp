/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-morph2.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"ef3d-morph2.h"
#include						"resource.h"
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<assert.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dmorph2Info::CI	= ACI("Aef3dmorph2Info",	GUID(0x11111112,0xB0000050), &Aeffect3dInfo::CI, 0, NULL);
ACI								Aef3dmorph2::CI		= ACI("Aef3dmorph2",		GUID(0x11111112,0xB0000051), &Aeffect3d::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
#define TEXSIZE					512
#define	VBUFSIZE				(sizeof(CUSTOMVERTEX)*(24*32*2*3))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adatamorph2
{
public:
	IDirect3DVertexBuffer9		*vb;
	float						azoom;
	float						avef[8];
	float						asef[3];
	
	Adatamorph2()
	{
		vb=null;
		azoom=0.f;
		memset(avef, 0, sizeof(avef));
		memset(asef, 0, sizeof(asef));
	}
};

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

static char *efn[]=
{
	"vertical",
	"horizontal",
	"radial zoom",
	"radial wave",
	"vertical wave",
	"horizontal wave",
	"chaos swirl",
	"twirl"
};

static char *efan[]=
{
	"bass vibes",
	"medium vibes",
	"treble vibes"
};

static QIID qiidef[]={ 0x0ef3f45a755e7100, 0xc4517fd3b8e96d3a, 0x41706ae3b744deb8, 0x9ada4581880b4980,
						0xd685c61379626344, 0xc281c2ac5df66190, 0x86626f21fc27d0f7, 0x842b358aca309b90};

static QIID qiidefa[]={ 0x7673928763131ad0, 0xeb747b4bc631d000, 0x69fcb8838a6478c0};

Aef3dmorph2::Aef3dmorph2(QIID qiid, char *name, Aobject *f, int x, int y) : Aeffect3d(qiid, name, f, x, y)
{
	char	str[1024];
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_MORPH2), "PNG");
	back=new Abitmap(&o);

	init();
	dt=new Adatamorph2();

	setTooltips("morph 2 effect");
	
	sprintf(str, "%s/zoom", name);
	zoom=new Apaddle(MKQIID(qiid,0xccf561cc97cc9000), str, this, 64, 16, paddleYOLI16);
	zoom->setTooltips("zoom");
	zoom->set(0.5f);
	zoom->show(TRUE);

	sprintf(str, "%s/amplitude", name);
	mastef=new Apaddle(MKQIID(qiid,0x1dbe606b4fa7aa00), str, this, 16+44, 41, paddleYOLI24);
	mastef->setTooltips("amplitude");
	mastef->set(0.5f);
	mastef->show(TRUE);

	{
		int	i;
		for(i=0; i<8; i++)
		{
			int	y=(i<4)?34:58;
			int	n=i&3;
			sprintf(str, "%s/%s", name, efn[i]);
			ef[i]=new Apaddle(MKQIID(qiid,qiidef[i]), str, this, 16+n*24+((n>>1)*24), y, paddleYOLI16);
			ef[i]->setTooltips(efn[i]);
			ef[i]->set(0.f);
			ef[i]->show(TRUE);
		}
	}
	
	{
		int	i;
		for(i=0; i<3; i++)
		{
			int	n=i&3;
			sprintf(str, "%s/%s", name, efan[i]);
			efa[i]=new Apaddle(MKQIID(qiid,qiidefa[i]), str, this, 146+n*24, 46, paddleYOLI16);
			efa[i]->setTooltips(efan[i]);
			efa[i]->set(0.f);
			efa[i]->show(TRUE);
		}
	}
	
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dmorph2::~Aef3dmorph2()
{
	int	i;
	for(i=0; i<8; i++)
		delete(ef[i]);
	for(i=0; i<3; i++)
		delete(efa[i]);
	delete(mastef);
	delete(zoom);
	delete(dt);
	delete(back);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph2::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dmorph2::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	//b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.1f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph2::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph2::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dmorph2::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Aef3dmorph2::init3D(Aef3dInfo *info)
{
	if(FAILED(info->d3d->d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &dt->vb, NULL)))
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph2::release3D(Aef3dInfo *info)
{
	if(dt->vb)
		dt->vb->Release();
	dt->vb=NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph2::action(Aef3dInfo *info)
{
	CUSTOMVERTEX		point[25][33];
	CUSTOMVERTEX		vertices[] =
	{
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f},
		{ 320.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 1.f, 0.f},
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f}, 
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f},
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f},
		{   0.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 0.f, 1.f}
	};
	Aeffect				*e=Aeffect::getEffect(this);
	Ad3d				*d3d=info->d3d;
	HRESULT				r;
	VOID				*pVertices;
	float				w=(float)d3d->w;
	float				h=(float)d3d->h;
	byte				rc,gc,bc;
	int					texType;
	float				texCx,texCy;
	IDirect3DTexture9	*tex=info->input->getTexture(&texType, &texCx, &texCy);

	if(!tex)
		return false;
	
	colorRGB(&rc,&gc, &bc, info->color);
	
	{
		float			zo=dt->azoom=dt->azoom*0.8f+(zoom->get()*0.9f)*0.2f;
		dword			c=D3DCOLOR_RGBA(rc, gc, bc, (int)(info->power*255.99f));
		double			s=((zo<0.5f)?(4.f-zo*6.f):((1.1f-zo)*(1.f/0.6f)))/32.0;
		double			mx=0.5;
		double			my=0.5;
		int				x,y;
		float			vef[8];
		float			sef[3];
		float			aef=mastef->get()*2.f;
		float			yy=0.f;
		double			rx=24.0/32.0;
		//float			smpx[33];
		//float			smpy[25];
		float			bas[33];
		float			med[33];
		float			tre[33];

		for(x=0; x<17; x++)
		{
			//smpx[x]=smpx[32-x]=(float)sound->smpR[x]/8192.f;
			bas[x]=bas[32-x]=(float)info->sound->basL[x]/8192.f;
			med[x]=med[32-x]=(float)info->sound->medL[x]/8192.f;
			tre[x]=tre[32-x]=(float)info->sound->treL[x]/8192.f;
		}
		/*
		for(y=0; y<13; y++)
			smpy[y]=smpy[24-y]=(float)sound->smpL[y]/8192.f;
		*/
		for(x=0; x<8; x++)
			vef[x]=(dt->avef[x]=dt->avef[x]*0.5f+ef[x]->get()*0.5f)*aef;
		for(x=0; x<3; x++)
			sef[x]=dt->asef[x]=dt->asef[x]*0.5f+efa[x]->get()*aef*0.5f;
		
		for(y=0; y<25; y++)
		{
			CUSTOMVERTEX	*v=&point[y][0];
			float			xx=0.f;
			for(x=0; x<33; x++)
			{
				//float	ibeat=(float)(info->beat-( ((int)info->beat)&0xfffffffe));
				double	dy=(double)y-12.0;
				double	dx=(double)x-16.0;
				/*
				dx=dx+dx*smpx[x]*sef[1];
				dy=dy+dy*smpy[y]*sef[1];
				*/
				dx=dx+sin(info->beat+dx*sin(info->beat*0.51202f))*vef[0];
				dy=dy+sin(info->beat+dy*sin(info->beat*0.44213f))*vef[1];

				dx=dx+sin(info->beat*1.121851+dy*sin(info->beat*0.051202f))*vef[4];
				dy=dy+sin(info->beat*1.912202+dx*sin(info->beat*0.044213f))*vef[5];

				double	r=sqrt(dx*dx+dy*dy);
				double	a=atan2(dy, dx);
				
				r=r+sin(info->beat+r*0.1)*r*vef[2]*0.5;
				r=r+sin(info->beat*1.1521321+r*(sin(info->beat*0.333333)+sin(info->beat*0.25)))*vef[3];
				
				a=a+(sin(info->beat+a))*vef[6];

				//a+=dx*sin(beat)*vef[7]/16.f;
				a+=r*sin(info->beat*0.25f)*vef[7]*0.5f;
				
				r+=r*(bas[((int)r)&31]*sef[0]+med[((int)r)&31]*sef[1]+tre[((int)r)&31]*sef[2]);
				
				//r=r-r*sound->bass*sef[0];
				
				r*=s;
				
				v->x=xx;
				v->y=yy;
				v->z=0.5f;
				v->rhw=1.f;
				v->color=c;
				v->u=(float)((cos(a)*r*rx)+mx);
				v->v=(float)((sin(a)*r)+my);
				v++;
				xx+=w/32.f;
			}
			yy+=h/24.f;
		}
	}
	
	if(dt->vb->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
	{
		CUSTOMVERTEX	*v=(CUSTOMVERTEX *)pVertices;
		int				x,y;
		
		for(y=0; y<24; y++)
		{
			for(x=0; x<32; x++)
			{
				*(v++)=point[y][x];
				*(v++)=point[y+1][x+1];
				*(v++)=point[y+1][x];
				
				*(v++)=point[y][x];
				*(v++)=point[y][x+1];
				*(v++)=point[y+1][x+1];
			}
		}
		dt->vb->Unlock();
	}
	
	d3d->d3dev->BeginScene();

	r=d3d->d3dev->SetStreamSource(0, dt->vb, 0, sizeof(CUSTOMVERTEX));
	r=d3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);

	r=d3d->d3dev->SetTexture(0, tex);
	tex->Release();

	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);	// test
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);	// test

	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	
	if(info->colorBoost)
		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	else
		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	r=d3d->d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	
	switch(info->bm)
	{
		case Aef3dInfo::blendFADE:	// normal
		r=d3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		r=d3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		r=d3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		break;
		
		case Aef3dInfo::blendADD:	// add
		r=d3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		r=d3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		r=d3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		break;

		case Aef3dInfo::blendSUB:	// sub
		r=d3d->d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		r=d3d->d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		r=d3d->d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
		break;
	}
	
	d3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        
	r=d3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 24*32*2);

	d3d->d3dev->EndScene();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph2::fillSurface()
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect3d * Aef3dmorph2Info::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dmorph2(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dmorph2GetInfo()
{
	return new Aef3dmorph2Info("morph2", &Aef3dmorph2::CI, "morph II", "morph II");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
