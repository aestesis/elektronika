/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-pixel.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"ef3d-pixel.h"
#include						"resource.h"
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<d3dx9effect.h>
#include						<assert.h>
#include						<io.h>
#include						<direct.h>
#include						"ef3d-node.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dpixelInfo::CI	= ACI("Aef3dpixelInfo",	GUID(0x11111112,0xB0000090), &Aeffect3dInfo::CI, 0, NULL);
ACI								Aef3dpixel::CI		= ACI("Aef3dpixel",		GUID(0x11111112,0xB0000091), &Aeffect3d::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define	TEXSOUNDW				(128)
#define	TEXSOUNDH				(4)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CUSTOMVERTEX
{
    FLOAT						x, y, z,rhw;
    DWORD						color;
    float						u,v;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define D3DFVF_CUSTOMVERTEX		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define TEXSIZE					512
#define	VBUFSIZE				(sizeof(CUSTOMVERTEX)*6)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class Adatapixel
{
public:
	IDirect3DVertexBuffer9		*vb;
	IDirect3DTexture9			*texsound;
	IDirect3DTexture9			*texsoundcpy;

	Ad3dPixShader				*pix;



	Asection					section;
	bool						bchanged;
	bool						fchange;

	float						apad[PIX_MAXINPUT];


	Adatapixel()
	{
		vb=null;
		texsound=null;
		texsoundcpy=null;
		pix=null;
		bchanged=false;
		fchange=false;
		memset(apad, 0, sizeof(apad));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int count=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!count)
	{
	}
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

static QIID pad_qiid[PIX_MAXINPUT]={	0x18f27f10255f1328, 0xdb794fc3b98045e8, 0x8e11e14b36bfbf9c, 0x8559e08e6a762018,
										0xb87a6176cfc70154, 0xfc8655d5cd245287, 0x80675590a0518420, 0x2e8eb1f10171f400 };

Aef3dpixel::Aef3dpixel(QIID qiid, char *name, Aobject *f, int x, int y) : Aeffect3d(qiid, name, f, x, y)
{
	Afont	*font=alib.getFont(fontTERMINAL05);
	char	str[1024];
	init();
	dt=new Adatapixel();
	error=false;
	ok=true;
	
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_PIXSHAD), "PNG");
	back=new Abitmap(&o);

	setTooltips("pixel shader effect");


	int	i;
	for(i=0; i<PIX_MAXINPUT; i++)
	{
		int	x=i*26;
		int	y=(i&1)*10;
		int	y2=(i&1)*35+7;
		//int	y=0;
		sprintf(str, "%s/pad[%d]", name, i);
		pad[i]=new Apaddle(MKQIID(qiid,pad_qiid[i]), str, this, 18+x, 16+y, paddleYOLI16);
		pad[i]->setTooltips("");
		pad[i]->set(0.5f);
		pad[i]->show(TRUE);

		sprintf(str, "sta[%d]", i);
		sta[i]=new Astatic(str, this, x+8, y2, 38, 10);
		sta[i]->set("", 0xff404040, font, Astatic::CENTER);
		sta[i]->show(true);
	}
	
	items=new Aitem("shaders", "shaders list");
	
	dt->pix=new Ad3dPixShader();

	{ // add list of fx effects
		Aitem	*sel=null;
		int		i;
		for(i=0; i<dt->pix->nfile; i++)
		{
			char	str[ALIB_PATH];
			char	*s=strrchr(dt->pix->filename[i], '\\');
			strcpy(str, s+1);
			*strstr(str, ".fx")=0;
			sel=new Aitem(str, str, items, i);
		}
		items->sort();
		
		list=new Alist("shaders", this, 68, 60, 100, 16, items);
		list->setTooltips("shaders");
		list->setCurrent(sel);
		list->show(TRUE);
	}
	
	settings(false);
	timer(500);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dpixel::~Aef3dpixel()
{
	int	i;
	for(i=0; i<PIX_MAXINPUT; i++)
		delete(pad[i]);
	delete(items);
	delete(list);
	delete(dt->pix);
	delete(dt);
	delete(back);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==list)
		{
			Afont	*font=alib.getFont(fontTERMINAL05);
			int		i;
			dt->section.enter(__FILE__,__LINE__);
			for(i=0; i<PIX_MAXINPUT; i++)
			{
				pad[i]->setTooltips("");
				sta[i]->set("", 0xff404040, font, Astatic::CENTER);
				pad[i]->set(0.5f);
				dt->apad[i]=0.5f;
			}
			dt->fchange=true;
			dt->section.leave();
		}
		break;	
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dpixel::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dpixel::pulse()
{
	Afont	*font=alib.getFont(fontTERMINAL05);

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
			sta[i]->set(str, 0xff404040, font, Astatic::CENTER);
		}
	}
	dt->section.leave();
	if(error)
	{
		notify(this, nyERROR, (dword)"this shader can't be compiled on this 3D hardware");
		error=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::save(class Afile *f)
{
	savePreset(f);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::load(class Afile *f)
{
	loadPreset(f);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::savePreset(class Afile *f)
{
	dword	sel=list->getCurrentData();
	f->writeString(dt->pix->filename[sel]);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::loadPreset(class Afile *f)
{
	char	str[ALIB_PATH];
	int		i;
	f->readString(str);
	{
		for(i=0; i<dt->pix->nfile; i++)
			if(!strcmp(dt->pix->filename[i], str))
			{
				list->setCurrentByData(i);
				break;
			}
	}
	{
		Afont	*font=alib.getFont(fontTERMINAL05);
		int		i;
		for(i=0; i<PIX_MAXINPUT; i++)
		{
			pad[i]->setTooltips("");
			sta[i]->set("", 0xff404040, font, Astatic::CENTER);
			pad[i]->set(0.5f);
		}
		dt->fchange=true;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dpixel::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::init3D(Aef3dInfo *info)
{
	ok=false;
	if(FAILED(info->d3d->d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &dt->vb, NULL)))
		return false;

	if(D3DXCreateTexture(info->d3d->d3dev, TEXSOUNDW, TEXSOUNDH, 1, 0, D3DFMT_V16U16, D3DPOOL_DEFAULT, &dt->texsound)!=D3D_OK)
		return false;

	if(D3DXCreateTexture(info->d3d->d3dev, TEXSOUNDW, TEXSOUNDH, 1, 0, D3DFMT_V16U16, D3DPOOL_SYSTEMMEM, &dt->texsoundcpy)!=D3D_OK)
		return false;

	dt->section.enter(__FILE__,__LINE__);
	error=!(dt->bchanged=dt->pix->init(info->d3d->d3dev, list->getCurrentData()));
	dt->section.leave();

	if(!error)
		ok=true;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::release3D(Aef3dInfo *info)
{
	if(dt->vb)
		dt->vb->Release();
	dt->vb=NULL;

	if(dt->texsound)
		dt->texsound->Release();
	dt->texsound=NULL;

	if(dt->texsoundcpy)
		dt->texsoundcpy->Release();
	dt->texsoundcpy=NULL;

	dt->pix->release();
	// need pix class exist after
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void soundcpy(byte *dest, short *left, short *right)
{
	short	*d=(short *)dest;
	int		i;
	for(i=0; i<TEXSOUNDW; i++)
	{
		*(d++)=*(left++);
		*(d++)=*(right++);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::action(Aef3dInfo *info)
{
	if(dt->fchange)
	{
		dt->fchange=false;
		release3D(info);
		init3D(info);
	}
	if(ok)
	{
		{
			IDirect3DSurface9	*surf;
			if(dt->texsoundcpy->GetSurfaceLevel(0, &surf)==D3D_OK)
			{
				D3DLOCKED_RECT	lrect;
				if(surf->LockRect(&lrect, NULL, D3DLOCK_READONLY)==D3D_OK)
				{
					int		y=0;

					byte	*d=(byte *)lrect.pBits+lrect.Pitch*y;
					soundcpy(d, info->sound->smpL, info->sound->smpR);
					y++;

					d=(byte *)lrect.pBits+lrect.Pitch*y;
					soundcpy(d, info->sound->basL, info->sound->basR);
					y++;

					d=(byte *)lrect.pBits+lrect.Pitch*y;
					soundcpy(d, info->sound->medL, info->sound->medR);
					y++;

					d=(byte *)lrect.pBits+lrect.Pitch*y;
					soundcpy(d, info->sound->treL, info->sound->treR);

					surf->UnlockRect();
				}
				{
					IDirect3DSurface9	*sdest;
					if(dt->texsound->GetSurfaceLevel(0, &sdest)==D3D_OK)
						info->d3d->d3dev->UpdateSurface(surf, NULL, sdest, NULL);
				}
				surf->Release();
			}
		}

		CUSTOMVERTEX vertices[] =
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
		
		dword				c=D3DCOLOR_RGBA(rc, gc, bc, (int)(info->power*255.99f));
		
		vertices[0].x=0;
		vertices[0].y=0;
		vertices[1].x=w;
		vertices[1].y=0;
		vertices[2].x=w;
		vertices[2].y=h;
		vertices[3].x=0;
		vertices[3].y=0;
		vertices[4].x=w;
		vertices[4].y=h;
		vertices[5].x=0;
		vertices[5].y=h;
		
		vertices[0].color=c;
		vertices[1].color=c;
		vertices[2].color=c;
		vertices[3].color=c;
		vertices[4].color=c;
		vertices[5].color=c;
		
		assert(VBUFSIZE==sizeof(vertices));

		if(dt->vb->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
		{
			memcpy(pVertices, vertices, VBUFSIZE);
			dt->vb->Unlock();
		}
		
		
		d3d->d3dev->BeginScene();
		
		if(dt->pix->effect)
		{
			dt->pix->effect->SetTechnique("elektronika");
			dt->pix->effect->SetTexture("texture0", tex);
			tex->Release();
			dt->pix->effect->SetTexture("texsound", dt->texsound);
			
			if(dt->pix->hColor)
			{
				D3DXVECTOR4	v;
				float		bo=(info->colorBoost?2.f:1.f);
				v.x=bo*(float)rc/255.f;
				v.y=bo*(float)gc/255.f;
				v.z=bo*(float)bc/255.f;
				v.w=info->power;
				dt->pix->effect->SetVector(dt->pix->hColor, &v);
			}
			
			if(dt->pix->hBeat)
				dt->pix->effect->SetFloat(dt->pix->hBeat, (FLOAT)info->beat);

			if(dt->pix->hRbeat)
			{
				FLOAT	rbeat=(float)(info->beat-(float)(int)info->beat);
				rbeat=(rbeat<0.5f)?((0.5f-rbeat)*2.f):((rbeat-0.5f)*2.f);
				dt->pix->effect->SetFloat(dt->pix->hRbeat, rbeat);
			}

			if(dt->pix->hBass)
				dt->pix->effect->SetFloat(dt->pix->hBass, (FLOAT)info->sound->bass);
			if(dt->pix->hMedium)
				dt->pix->effect->SetFloat(dt->pix->hMedium, (FLOAT)info->sound->medium);
			if(dt->pix->hTreeble)
				dt->pix->effect->SetFloat(dt->pix->hTreeble, (FLOAT)info->sound->treble);
				
			dt->section.enter(__FILE__,__LINE__);
			{
				int	i;
				for(i=0; i<dt->pix->nperso; i++)
				{
					float	v=(dt->apad[i]=dt->apad[i]*0.5f+pad[i]->get()*0.5f);
					dt->pix->effect->SetFloat(dt->pix->hPerso[i], (FLOAT)v);
				}
			}
			dt->section.leave();

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

			{
				UINT uPasses;
				dt->pix->effect->Begin( &uPasses, 0 );
				for( UINT uPass = 0; uPass < uPasses; ++uPass )
				{
					dt->pix->effect->BeginPass( uPass );

					r=d3d->d3dev->SetStreamSource(0, dt->vb, 0, sizeof(CUSTOMVERTEX));
					r=d3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
					r=d3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

					dt->pix->effect->EndPass();
				}
				dt->pix->effect->End();
			}
		}
		else
		{
			r=d3d->d3dev->SetStreamSource(0, dt->vb, 0, sizeof(CUSTOMVERTEX));
			r=d3d->d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);

			r=d3d->d3dev->SetTexture(0, tex);
			tex->Release();

			r=d3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

			r=d3d->d3dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			r=d3d->d3dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	
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
			r=d3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
		}
		
		d3d->d3dev->EndScene();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dpixel::fillSurface()
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect3d * Aef3dpixelInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dpixel(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dpixelGetInfo()
{
	return new Aef3dpixelInfo("pixel shader", &Aef3dpixel::CI, "pixel shader", "pixel shader");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
