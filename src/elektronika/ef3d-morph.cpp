/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-morph.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"ef3d-morph.h"
#include						"resource.h"
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<assert.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dmorphInfo::CI	= ACI("Aef3dmorphInfo",	GUID(0x11111112,0xB0000040), &Aeffect3dInfo::CI, 0, NULL);
ACI								Aef3dmorph::CI		= ACI("Aef3dmorph",		GUID(0x11111112,0xB0000041), &Aeffect3d::CI, 0, NULL);

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
#define	VBUFSIZE				(sizeof(CUSTOMVERTEX)*(24*32*2*3))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adatamorph
{
public:
	IDirect3DVertexBuffer9		*vb;
	float						azoom;
	float						azx;
	float						azy;
	float						aaspect;
	
	Adatamorph()
	{
		vb=null;
		azoom=0.f;
		azx=0.5f;
		azy=0.5f;
		aaspect=0.5f;
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
	"swirl"
};

static char *efan[]=
{
	"bass zoom",
	"bass XL",
	"medium vibes"
};

static QIID qiidefa[]={ 0x076fbb9346361b98, 0x49eb36606e48f116, 0x8561c3da74dec298};
static QIID qiidef[]={ 0x893a68d125a7a828, 0x89de3894828770c0, 0x08358b040633ec7a, 0x8779ad2515620000,
					0x3aca10469834864c, 0xd496d24858d43dd0, 0x76071c22d1d3ffb8, 0xe275b630ea6b3f90};

Aef3dmorph::Aef3dmorph(QIID qiid, char *name, Aobject *f, int x, int y) : Aeffect3d(qiid, name, f, x, y)
{
	char	str[1024];
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_MORPH), "PNG");
	back=new Abitmap(&o);
	
	setTooltips("morph effect");
	
	init();
	dt=new Adatamorph();
	
	sprintf(str, "%s/zoom", name);
	zoom=new Apaddle(MKQIID(qiid,0x240ac205d619ee10), str, this, 64, 16, paddleYOLI16);
	zoom->setTooltips("zoom");
	zoom->set(0.5f);
	zoom->show(TRUE);

	sprintf(str, "%s/amplitude", name);
	mastef=new Apaddle(MKQIID(qiid,0x04ac213374106d9e), str, this, 16+44, 41, paddleYOLI24);
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

	sprintf(str, "%s/position", name);
	zone=new Azone(MKQIID(qiid, 0xf34e845dce9a1f90), str, this, 146, 4, 30, 30, 0.5f, 0.5f);
	zone->setTooltips("position");
	zone->set(0.5f, 0.5f);
	zone->show(TRUE);

	sprintf(str, "%s/aspect", name);
	aspect=new Apaddle(MKQIID(qiid,0x2485d5ea51c9ee10), str, this, 182, 10, paddleYOLI16);
	aspect->setTooltips("aspect");
	aspect->set(0.5f);
	aspect->show(TRUE);


/*
	zoom=new Apaddle("zoom", this, 160, 16, paddleYOLI16);
	zoom->setTooltips("zoom");
	zoom->set(0.5f);
	zoom->show(TRUE);

	mastef=new Apaddle("amplitude", this, 264, 14, paddleYOLI24);
	mastef->setTooltips("amplitude");
	mastef->set(0.5f);
	mastef->show(TRUE);

	{
		int	i;
		for(i=0; i<8; i++)
		{
			int	y=(i<4)?6:30;
			int	n=i&3;
			ef[i]=new Apaddle(efn[i], this, 220+n*24+((n>>1)*24), y, paddleYOLI16);
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
			efa[i]=new Apaddle(efan[i], this, 350+n*24, 16, paddleYOLI16);
			efa[i]->setTooltips(efan[i]);
			efa[i]->set(0.f);
			efa[i]->show(TRUE);
		}
	}
*/	
	settings();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dmorph::~Aef3dmorph()
{
	int	i;
	for(i=0; i<8; i++)
		delete(ef[i]);
	for(i=0; i<3; i++)
		delete(efa[i]);
	delete(mastef);
	delete(zoom);
	delete(zone);
	delete(aspect);
	delete(dt);
	delete(back);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dmorph::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	//b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.1f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dmorph::settings()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Aef3dmorph::init3D(class Aef3dInfo *info)
{
	if(FAILED(info->d3d->d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &dt->vb, NULL)))
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph::release3D(class Aef3dInfo *info)
{
	if(dt->vb)
		dt->vb->Release();
	dt->vb=NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dmorph::action(Aef3dInfo *info)
{
	static int			tmin=0.001;
	static int			tmax=0.999;
	CUSTOMVERTEX		point[25][33];
	CUSTOMVERTEX		vertices[] =
	{
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, tmin, tmin},
		{ 320.0f,   0.0f, 0.5f, 1.f, 0xffffffff, tmax, tmin},
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, tmax, tmax}, 
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, tmin, tmin},
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, tmax, tmax},
		{   0.0f, 240.0f, 0.5f, 1.f, 0xffffffff, tmin, tmax}
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
	float				zx,zy;
	float				axy=dt->aaspect=aspect->get()*0.5f+dt->aaspect*0.5f;
	zone->get(&zx, &zy);

	zx=dt->azx=dt->azx*0.5f+zx*0.5f;
	zy=dt->azy=dt->azy*0.5f+zy*0.5f;

	if(!tex)
		return false;

	colorRGB(&rc,&gc, &bc, info->color);
	
	{
		dword			c=D3DCOLOR_RGBA(rc, gc, bc, (int)(info->power*255.99f));
		double			s=((dt->azoom=dt->azoom*0.5f+(zoom->get()+0.01f)*0.5f)*2.0*w)/32.0;
		double			mx=w*zx;
		double			my=h*zy;
		int				x,y;
		float			vef[8];
		float			sef[3];
		float			aef=mastef->get()*2.f;
		float			tx=texCx/32.f;
		float			ty=texCy/24.f;
		float			ratioy=((float)h/(float)w)/(24.f/32.f);
		float			aax,aay;

		if(axy>0.5f)
		{
			aax=1.f;
			aay=1.f+(axy-0.5f)*4.f;
		}
		else
		{
			aay=1.f;
			aax=1.f+(0.5f-axy)*4.f;
		}
		
		for(x=0; x<8; x++)
			vef[x]=ef[x]->get()*aef;
		for(x=0; x<3; x++)
			sef[x]=efa[x]->get()*aef;
		
		for(y=0; y<25; y++)
		{
			CUSTOMVERTEX	*v=&point[y][0];
			for(x=0; x<33; x++)
			{
				//float	ibeat=(float)(info->beat-( ((int)info->beat)&0xfffffffe));
				double	dy=(double)y-12.0;
				double	dx=(double)x-16.0;

				dx=dx+dx*info->sound->bass*sef[1]*4.f;
				dy=dy+dy*info->sound->medium*sef[2]*4.f;

				dx=dx+sin(info->beat+dx*sin(info->beat*0.51202f))*vef[0];
				dy=dy+sin(info->beat+dy*sin(info->beat*0.44213f))*vef[1];

				dx=dx+sin(info->beat*1.121851+dy*sin(info->beat*0.051202f))*vef[4];
				dy=dy+sin(info->beat*1.912202+dx*sin(info->beat*0.044213f))*vef[5];

				double	r=sqrt(dx*dx+dy*dy);
				double	a=atan2(dy, dx);
				
				r=r+sin(info->beat+r*0.1)*r*vef[2]*0.5f;
				r=r+sin(info->beat*1.1521321+r*sin(info->beat*0.333333f))*vef[3];
				
				a=a+(sin(info->beat+a))*vef[6];

				a+=dx*sin(info->beat)*vef[7]/16.f;
				
				r=r+r*info->sound->bass*sef[0]*4.f;
				
				r*=s;
				
				v->x=(float)((cos(a)*r*aax)+mx);
				v->y=(float)((sin(a)*r*ratioy*aay)+my);
				v->z=0.5f;
				v->rhw=1.f;
				v->color=c;
				v->u=(float)x*tx;
				v->v=(float)y*ty;
				v++;
			}
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
	
	
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);	// test
	r=d3d->d3dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);	// test

	r=d3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);

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

bool Aef3dmorph::fillSurface()
{
	int	x;
	if((mastef->get()==0.f)&&(zoom->get()>=0.5f))
		return true;
	for(x=0; x<8; x++)
	{
		if(ef[x]->get()!=0.f)
			return false;
	}
	for(x=0; x<3; x++)
	{
		if(efa[x]->get()!=0.f)
			return false;
	}
	if(zoom->get()<0.5f)
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect3d * Aef3dmorphInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dmorph(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dmorphGetInfo()
{
	return new Aef3dmorphInfo("morph", &Aef3dmorph::CI, "morph", "morph");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
