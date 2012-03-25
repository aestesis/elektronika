/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-video.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"ef3d-video.h"
#include						"resource.h"
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<assert.h>
#include						"ef3d-node.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dvideoInfo::CI	= ACI("Aef3dvideoInfo",	GUID(0x11111112,0xB0000020), &Ainput3dInfo::CI, 0, NULL);
ACI								Aef3dvideo::CI		= ACI("Aef3dvideo",		GUID(0x11111112,0xB0000021), &Ainput3d::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TEXSIZE					512

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

class ApixShader
{
public:
	char						root[1024];
	char						filename[1024];
	IDirect3DPixelShader9		*ps;
    ID3DXEffect					*effect;

	D3DXHANDLE					hColor;
	D3DXHANDLE					hPower;

	IDirect3DVertexBuffer9		*vb;


	ApixShader(char *fname)
	{
		ps=null;
		effect=null;
		vb=null;
		{
			memset(filename, 0, sizeof(filename));
			GetModuleFileName(GetModuleHandle(null), filename, sizeof(filename));
			if(filename[0])
			{
				char	*s=strrchr(filename, '\\');
				if(s)
					*s=0;
			}
		}
		strcat(filename, "\\");
		strcpy(root, filename);
		strcat(filename, fname);
	}

	/////////////////////////////////////////////////////////////////

	bool Init(IDirect3DDevice9 *d3dev)
	{
		D3DXEFFECT_DESC		desc;
		
		{
			D3DCAPS9 hal_caps;
			ZeroMemory(&hal_caps, sizeof(D3DCAPS9));
			if(d3dev->GetDeviceCaps(&hal_caps)!=S_OK)
				return false;
			UINT ps_major = D3DSHADER_VERSION_MAJOR(hal_caps.PixelShaderVersion);
			UINT ps_minor = D3DSHADER_VERSION_MAJOR(hal_caps.PixelShaderVersion);
			if(ps_major<2)
				return false;
 		}

		{
			LPD3DXBUFFER		pErrorMsgs = NULL;
			if(FAILED(D3DXCreateEffectFromFile(d3dev, filename, NULL, NULL, 0, NULL, &effect, &pErrorMsgs)))
			{
				if(pErrorMsgs != NULL)
				{
					unsigned char* message = (unsigned char*)pErrorMsgs->GetBufferPointer();
					{ // save error log
						char					path[ALIB_PATH];
						FILE					*fic;
						strcpy(path, root);
						strcat(path, "\\shaders\\_errors.txt");
						fic=fopen(path, "w");
						if(fic)
						{
							fprintf(fic, (char *)message);
							fclose(fic);
						}
					}
				}
				return false;
			}
		}
		
		if(FAILED(effect->GetDesc(&desc)))
			return false;
		
		{
			int					i;
			for(i=0; i<(int)desc.Parameters; i++)
			{
				D3DXHANDLE			hp=effect->GetParameter(NULL, i);
				D3DXPARAMETER_DESC	descp;
				
				if(!hp)
					continue;
				
				if(FAILED(effect->GetParameterDesc(hp, &descp)))
					continue;
					
				if((descp.Class==D3DXPC_VECTOR)&&(descp.Type==D3DXPT_FLOAT))
				{
					if(!strcmp(descp.Name, "color"))
					{
						hColor=hp;
						hp=NULL;
					}
				}
				else if((descp.Class==D3DXPC_SCALAR)&&(descp.Type==D3DXPT_FLOAT))
				{	
					if(!strcmp(descp.Name, "power"))
						hPower=hp;
				}
			}
		}

		if(FAILED(d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vb, NULL)))
			return false;

		return true;
	}

	void Close()
	{
		if(effect)
			effect->Release();
		effect=null;
		if(ps)
			ps->Release();
		ps=null;
		if(vb)
			vb->Release();
		vb=null;
	}

	bool Action(IDirect3DDevice9 *d3dev, IDirect3DTexture9 *src, IDirect3DTexture9 *dst, float w, float h, float power, dword color)
	{
		CUSTOMVERTEX vertices[] =
		{
			{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f},
			{ 320.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 1.f, 0.f},
			{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f}, 
			{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f},
			{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f},
			{   0.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 0.f, 1.f}
		};
		HRESULT				r;
		VOID				*pVertices;
		int					texType;
		float				texCx,texCy;
		
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
/*		
		vertices[0].color=0xFFFFFFFF;
		vertices[1].color=0xFFFFFFFF;
		vertices[2].color=0xFFFFFFFF;
		vertices[3].color=0xFFFFFFFF;
		vertices[4].color=0xFFFFFFFF;
		vertices[5].color=0xFFFFFFFF;
*/		
		assert(VBUFSIZE==sizeof(vertices));

		if(vb->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
		{
			memcpy(pVertices, vertices, VBUFSIZE);
			vb->Unlock();
		}
		
		IDirect3DSurface9 *renderO=null;
		IDirect3DSurface9 *renderS=null;	
		d3dev->GetRenderTarget(0, &renderO);
		if(dst->GetSurfaceLevel(0, &renderS)==D3D_OK)
		{
			d3dev->BeginScene();
			d3dev->SetRenderTarget(0, renderS);
			if(effect)
			{
				effect->SetTechnique("elektronika");
				effect->SetTexture("texture0", src);
				
				if(hColor)
				{
					D3DXVECTOR4	v;
					byte	rc,gc,bc;
					colorRGB(&rc,&gc, &bc, (dword)color);
					v.x=(float)rc/255.f;
					v.y=(float)gc/255.f;
					v.z=(float)bc/255.f;
					v.w=1.f;
					effect->SetVector(hColor, &v);
				}
				
				if(hPower)
					effect->SetFloat(hPower, (FLOAT)power);

				r=d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				r=d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

				r=d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				r=d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

				r=d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				
				r=d3dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				r=d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
				r=d3dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

				{
					UINT uPasses;
					effect->Begin( &uPasses, 0 );
					for( UINT uPass = 0; uPass < uPasses; ++uPass )
					{
						effect->BeginPass( uPass );

						r=d3dev->SetStreamSource(0, vb, 0, sizeof(CUSTOMVERTEX));
						r=d3dev->SetFVF(D3DFVF_CUSTOMVERTEX);
						r=d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

						effect->EndPass();
					}
					effect->End();
				}
			}
			d3dev->EndScene();
			renderS->Release();
		}
		d3dev->SetRenderTarget(0, renderO);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adatavideo
{
public:
	//IDirect3DSurface9			*sftemp;
	IDirect3DTexture9			*texS;
	IDirect3DTexture9			*texD;
	IDirect3DTexture9			*texT;
	IDirect3DTexture9			*texOpti;
	int							texType;
	float						cx;
	float						cy;
	bool						filled;
	bool						bTexAnySize;
	double						lastBeat;
	RECT						rectS;
	RECT						rectD;

	// color key & tint
	ApixShader					*pixCkey;
	ApixShader					*pixTint;

	IDirect3DTexture9			*texC;

	bool						isOK;

	Adatavideo()
	{
		//sftemp=NULL;
		texS=null;
		texD=null;
		texT=null;
		texOpti=null;
		texType=ef3dTEXnone;
		cx=0.f;
		cy=0.f;
		filled=false;
		bTexAnySize=false;
		lastBeat=0.0;
		pixCkey=null;
		pixTint=null;
		texC=null;
		isOK=false;
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

Aef3dvideo::Aef3dvideo(QIID qiid, char *name, Aobject *f, int x, int y) : Ainput3d(qiid, name, f, x, y)
{
	char	str[1024];
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_EF3D_VIDEO), "PNG");
	back=new Abitmap(&o);

	init();

	setTooltips("video input");

	sprintf(str, "%s/video in", name);
	sel=new Aselect(MKQIID(qiid,0x6dccfeba5814cda8), str, this, 24, 20, effect3dNBVIDEO, 1, &resource.get(MAKEINTRESOURCE(PNG_EF3D_SOURCE), "PNG"), 16, 16);
	sel->setTooltips("video in selector");
	sel->set(0);
	sel->show(TRUE);

	///

	sprintf(str, "%s/color key", name);
	ckey=new Aselcolor(MKQIID(qiid, 0xb36f16656729fec4), str, this, 16, 56);
	ckey->setTooltips("color key");
	ckey->set(0xffffffff);
	ckey->show(true);

	sprintf(str, "%s/power key", name);
	pkey=new Apaddle(MKQIID(qiid, 0x9856980924565a00), str, this, 36, 52, paddleYOLI24);
	pkey->setTooltips("power key");
	pkey->set(1.0f);
	pkey->show(true);

	///

	sprintf(str, "%s/tint color", name);
	ctint=new Aselcolor(MKQIID(qiid, 0xb36f11d95429fec4), str, this, 90, 56);
	ctint->setTooltips("tint color");
	ctint->set(0xffffffff);
	ctint->show(true);

	sprintf(str, "%s/tint power", name);
	ptint=new Apaddle(MKQIID(qiid, 0x981bb4f47ca45a00), str, this, 70, 56, paddleYOLI16);
	ptint->setTooltips("tint power");
	ptint->set(0.0f);
	ptint->show(true);

	dt=new Adatavideo();
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dvideo::~Aef3dvideo()
{
	delete(dt);
	delete(sel);
	delete(back);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dvideo::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dvideo::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	//b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.1f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dvideo::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dvideo::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dvideo::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Aef3dvideo::init3D(class Aef3dInfo *info)
{
	dt->isOK=false;
	dt->texType=info->effect->getTEXtype();
	dt->bTexAnySize=false;

	_D3DFORMAT fsrc=D3DFMT_A8R8G8B8;
	_D3DFORMAT fdst=D3DFMT_A8R8G8B8;

	Aeffect	*e=Aeffect::getEffect(this);
	int		w=e->getVideoWidth();
	int		h=e->getVideoHeight();
	int		s=1;

	{
		int i;
		for(i=0; i<16; i++)
		{
			if((s>w)&&(s>h))
				break;
			s<<=1;
		}
	}

	dt->pixCkey=new ApixShader("shaders\\sources\\colorkey.fx");
	if(!dt->pixCkey->Init(info->d3d->d3dev))
		return false;
	dt->pixTint=new ApixShader("shaders\\sources\\tint.fx");
	if(!dt->pixTint->Init(info->d3d->d3dev))
		return false;

	//
	dt->texT=NULL;
	dt->texC=NULL;

	if(dt->texType==ef3dTEXpart)
	{
		bool	ok=false;

		dt->texS=NULL;
		dt->texD=NULL;

		if(D3DXCreateTexture(info->d3d->d3dev, w, h, 1, 0, fsrc, D3DPOOL_SYSTEMMEM, &dt->texS)==D3D_OK)
		{
			if(D3DXCreateTexture(info->d3d->d3dev, w, h, 1, D3DUSAGE_RENDERTARGET, fdst, D3DPOOL_DEFAULT, &dt->texD)==D3D_OK)
			{
				if(D3DXCreateTexture(info->d3d->d3dev, w, h, 1, D3DUSAGE_RENDERTARGET, fdst, D3DPOOL_DEFAULT, &dt->texC)==D3D_OK)
				{
					ok=true;
					dt->cx=1.f;
					dt->cy=1.f;
					dt->rectS.left=0;
					dt->rectS.top=0;
					dt->rectS.right=w;
					dt->rectS.bottom=h;
					dt->rectD.left=0;
					dt->rectD.top=0;
					dt->rectD.right=w;
					dt->rectD.bottom=h;
					dt->isOK=true;
					return true;
				}
			}
		}
		
		if(D3DXCreateTexture(info->d3d->d3dev, s, s, 1, 0, fsrc, D3DPOOL_SYSTEMMEM, &dt->texS)==D3D_OK)
		{
			if(D3DXCreateTexture(info->d3d->d3dev, s, s, 1, D3DUSAGE_RENDERTARGET, fdst, D3DPOOL_DEFAULT, &dt->texD)==D3D_OK)
			{
				if(D3DXCreateTexture(info->d3d->d3dev, s, s, 1, D3DUSAGE_RENDERTARGET, fdst, D3DPOOL_DEFAULT, &dt->texC)==D3D_OK)
				{
					dt->rectS.left=0;
					dt->rectS.top=0;
					dt->rectS.right=s;
					dt->rectS.bottom=s;
					dt->rectD.left=0;
					dt->rectD.top=0;
					dt->rectD.right=s;
					dt->rectD.bottom=s;
					ok=true;
				}
			}
		}
		
		if(ok)
		{
			dt->cx=(float)w/(float)s;
			dt->cy=(float)h/(float)s;
			dt->isOK=true;
			return true;
		}

		if(dt->texS)
			dt->texS->Release();
		if(dt->texD)
			dt->texD->Release();
		if(dt->texC)
			dt->texC->Release();
		dt->texS=NULL;
		dt->texD=NULL;
		dt->texC=NULL;
	}

	if(D3DXCreateTexture(info->d3d->d3dev, w, h, 1, 0, fsrc, D3DPOOL_SYSTEMMEM, &dt->texS)!=D3D_OK)
		return false;

	if(D3DXCreateTexture(info->d3d->d3dev, w, h, 1, 0, fsrc, D3DPOOL_DEFAULT, &dt->texT)!=D3D_OK)
		return false;

	if(D3DXCreateTexture(info->d3d->d3dev, s, s, 1, D3DUSAGE_RENDERTARGET, fdst, D3DPOOL_DEFAULT, &dt->texC)!=D3D_OK)	// 
		return false;

	if(D3DXCreateTexture(info->d3d->d3dev, s, s, 1, D3DUSAGE_RENDERTARGET, fdst, D3DPOOL_DEFAULT, &dt->texD)!=D3D_OK)	// 
		return false;

	dt->rectS.left=0;
	dt->rectS.top=0;
	dt->rectS.right=w;
	dt->rectS.bottom=h;
	dt->rectD.left=0;
	dt->rectD.top=0;
	dt->rectD.right=s;
	dt->rectD.bottom=s;

	dt->texType=ef3dTEXfull;
	dt->cx=1.f;
	dt->cy=1.f;
	dt->isOK=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dvideo::release3D(class Aef3dInfo *info)
{
	if(dt->pixTint)
	{
		dt->pixTint->Close();
		delete(dt->pixTint);
		dt->pixTint=null;
	}
	if(dt->pixCkey)
	{
		dt->pixCkey->Close();
		delete(dt->pixCkey);
		dt->pixCkey=null;
	}

	if(dt->texS)
		dt->texS->Release();
	dt->texS=NULL;

	if(dt->texC)
		dt->texC->Release();
	dt->texC=NULL;

	if(dt->texD)
		dt->texD->Release();
	dt->texD=NULL;

	if(dt->texT)
		dt->texT->Release();
	dt->texT=NULL;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dvideo::action(Aef3dInfo *info)
{
	if(dt->isOK)
	{
		Ad3d				*d3d=info->d3d;
		int					nin=sel->get();
		Abitmap				*b=info->in[nin]->getBitmap();
		
		dt->texOpti=null;
		/*
		{
			int	k;
			for(k=0; k<info->current; k++)
			{
				Ainput3d	*i3d=info->inputs[k];
				if(i3d->isCI(&Aef3dvideo::CI))
				{
					Aef3dvideo	*ev3=(Aef3dvideo *)i3d;
					if((ev3->dt->lastBeat==info->beat)&&(ev3->sel->get()==nin))
					{
						if((ev3->dt->cx==dt->cx)&&(ev3->dt->cy==dt->cy)&&(ev3->dt->texType==dt->texType))
						{
							dt->texOpti=ev3->dt->texD;
							dt->filled=ev3->dt->filled;
							return true;
						}
					}
				}
			}
		}
		*/

		dt->lastBeat=info->beat;
		
		{
			IDirect3DSurface9	*surf;
			if(dt->texS->GetSurfaceLevel(0, &surf)==D3D_OK)
			{
				if(b)
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
					dt->filled=false;
				}
				else	// no video in // fill black
				{
					if(!dt->filled)
					{
						D3DLOCKED_RECT lck;
						if(surf->LockRect(&lck, &dt->rectS, D3DLOCK_DISCARD)==D3D_OK)
						{
							byte *d=(byte *)lck.pBits;
							int y;
							for(y=0; y<dt->rectS.bottom; y++)
							{
								memset(d, 0, dt->rectS.right*4);
								d+=lck.Pitch;
							}
							surf->UnlockRect();
						}
						dt->filled=true;
					}
				}

				switch(dt->texType)
				{
					case ef3dTEXpart:
	//				dt->texD->AddDirtyRect(NULL);
					info->d3d->d3dev->UpdateTexture(dt->texS, dt->texD);
					break;
					
					case ef3dTEXfull:
					info->d3d->d3dev->UpdateTexture(dt->texS, dt->texT);
					{
						IDirect3DSurface9 *surfS;
						IDirect3DSurface9 *surfD;
						if(dt->texT->GetSurfaceLevel(0, &surfS)==D3D_OK)
						{
							if(dt->texD->GetSurfaceLevel(0, &surfD)==D3D_OK)
							{
								HRESULT r=info->d3d->d3dev->StretchRect(surfS, NULL, surfD, NULL, D3DTEXF_LINEAR);	//  D3DTEXF_NONE
								surfD->Release();
							}
							surfS->Release();
						}
					}
					break;
				}
				surf->Release();
			}
		}

		if(!dt->filled)
		{
			float cpow=1.f-this->pkey->get();
			dword ccol=this->ckey->get();
			float tpow=this->ptint->get();
			dword tcol=this->ctint->get();

			if((cpow>0.f)||(tpow>0.f))
			{
				dt->pixCkey->Action(info->d3d->d3dev, dt->texD, dt->texC, dt->rectD.right, dt->rectD.bottom, cpow, ccol); 
				dt->pixTint->Action(info->d3d->d3dev, dt->texC, dt->texD, dt->rectD.right, dt->rectD.bottom, tpow, tcol); 
			}
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IDirect3DTexture9 *	Aef3dvideo::getTexture(int *type, float *cx, float *cy)
{
	if(dt->texOpti)
	{
		*type=dt->texType;
		*cx=dt->cx;
		*cy=dt->cy;
		dt->texOpti->AddRef();
		return dt->texOpti;
	}
	if(dt->texD)
	{
		*type=dt->texType;
		*cx=dt->cx;
		*cy=dt->cy;
		dt->texD->AddRef();
		return dt->texD;
	}
	*type=ef3dTEXnone;
	*cx=0.f;
	*cy=0.f;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ainput3d * Aef3dvideoInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dvideo(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dvideoGetInfo()
{
	return new Aef3dvideoInfo("video", &Aef3dvideo::CI, "video", "video");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
