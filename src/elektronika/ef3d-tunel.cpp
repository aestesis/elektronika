/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-tunel.CPP				(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"ef3d-tunel.h"
#include						"resource.h"
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<assert.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dtunelInfo::CI	= ACI("Aef3dtunelInfo",	GUID(0x11111112,0xB0000070), &Aeffect3dInfo::CI, 0, NULL);
ACI								Aef3dtunel::CI		= ACI("Aef3dtunel",		GUID(0x11111112,0xB0000071), &Aeffect3d::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CUSTOMVERTEX
{
    D3DXVECTOR3 position; // The 3-D position for the vertex.
    D3DXVECTOR3 normal;   // The surface normal for the vertex.
    dword		color;
    float		u,v;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define D3DFVF_CUSTOMVERTEX		(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define TEXSIZE					512
#define NBAXE					40
#define NBPOINT					32
#define	TOTALPOINT				(NBPOINT*NBAXE)
#define TOTALVERTICE			((NBAXE-1)*NBPOINT*2*3)		// TOTAL VERTICES IN TRIANGLE LIST
#define	VBUFSIZE				(sizeof(CUSTOMVERTEX)*TOTALVERTICE)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adatatunel
{
public:
	IDirect3DVertexBuffer9		*vb;
	float						azoom;
	float						arot;
	
	Adatatunel()
	{
		vb=null;
		azoom=0.f;
		arot=0.f;
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

static char *efan[]=
{
	"1",
	"2",
	"3"
};

static QIID qiidefa[]={ 0x0e7fb2ec791f9f40, 0xedc9246c1f26beb0, 0x1a22cf9d1afd7e68};

Aef3dtunel::Aef3dtunel(QIID qiid, char *name, Aobject *f, int x, int y) : Aeffect3d(qiid, name, f, x, y)
{
	char	str[1024];
	init();
	dt=new Adatatunel();
	
	beat=0.f;
	
	setTooltips("tunnel effect");

	sprintf(str, "%s/speed", name);
	zoom=new Apaddle(MKQIID(qiid,0xd4283bb4d5a973d8), str, this, 40, 16, paddleYOLI24);
	zoom->setTooltips("speed");
	zoom->set(0.5f);
	zoom->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dtunel::~Aef3dtunel()
{
	delete(zoom);
	delete(dt);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtunel::notify(Anode *o, int event, dword p)
{
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dtunel::paint(Abitmap *b)
{
	//b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.1f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtunel::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtunel::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dtunel::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool Aef3dtunel::init3D(Aef3dInfo *info)
{
	if(FAILED(info->d3d->d3dev->CreateVertexBuffer(VBUFSIZE, 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &dt->vb, NULL)))
		return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtunel::release3D(Aef3dInfo *info)
{
	if(dt->vb)
		dt->vb->Release();
	dt->vb=NULL;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* vertices
	{
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f},
		{ 320.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 1.f, 0.f},
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f}, 
		{   0.0f,   0.0f, 0.5f, 1.f, 0xffffffff, 0.f, 0.f},
		{ 320.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 1.f, 1.f},
		{   0.0f, 240.0f, 0.5f, 1.f, 0xffffffff, 0.f, 1.f}
	};
	*/
	
__inline void addvertice(CUSTOMVERTEX *pv, D3DXVECTOR3 *p, dword c, float u, float v, D3DXVECTOR3 *normal)
{
	pv->position=*p;
	//pv->position.x=p->x;
	//pv->position.y=p->y;
	//pv->position.z=p->z;
	pv->color=c;
	pv->u=u;
	pv->v=v;
	D3DXVec3Normalize((D3DXVECTOR3*)&pv->normal, normal);
}

bool Aef3dtunel::action(Aef3dInfo *info)
{
	D3DXVECTOR3			points[NBAXE][NBPOINT];
	D3DXVECTOR3			normals[NBAXE][NBPOINT];
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
	int					ntri=0;
	
	beat+=(float)(info->dbeat*(zoom->get()*2.f-0.9f));

	if(!tex)
		return false;
		
	colorRGB(&rc,&gc, &bc, info->color);

	{	// calc points ///////////////////
		D3DXVECTOR3		*p=&points[0][0];
		int				n;
		
		for(n=0; n<NBAXE; n++)
		{
			float	da=(float)((float)n*PI/(float)NBPOINT);
			float	cx=cos(beat+n*0.1f)*0.1f;
			float	cy=sin(beat+n*0.1f)*0.1f;
			int		i;
			float	db=(float)pow((double)(NBAXE-1-n)/(double)NBAXE, 1.5)*0.025f;
			
			D3DXMATRIX	roty;
			D3DXMATRIX	rotz;
			D3DXMATRIX	rot;
			
			D3DXMatrixRotationY(&roty, (float)(NBAXE-n)*db*sin(beat+(float)n*0.1f));
			D3DXMatrixRotationZ(&rotz, beat*0.15f);
			D3DXMatrixMultiply(&rot, &roty, &rotz);
			
			for(i=0; i<NBPOINT; i++)
			{
				float		a=(float)((float)i*PI*2.f/NBPOINT);
				D3DXVECTOR3 vin;
				vin.x=cx+cos(a);
				vin.y=cy+sin(a);
				vin.z=-10.f+(float)n*0.25f;
				D3DXVec3TransformCoord(p++, &vin, &rot);
			}
		}
	}	

	memset(normals, 0, sizeof(normals));
	{	// calc normals ///////////////////
		int	n;
		for(n=0; n<NBAXE-1; n++)
		{
			int	 i;
			for(i=0; i<NBPOINT; i++)
			{
				int			i1=(i+1)&(NBPOINT-1);
				D3DXVECTOR3	v1,v2,vout;
				D3DXVec3Subtract(&v1, &points[n+1][i], &points[n][i]);
				D3DXVec3Subtract(&v2, &points[n+1][i1], &points[n][i]);
				D3DXVec3Cross(&vout, &v1, &v2);
				D3DXVec3Normalize(&normals[n][i], &vout);
			}
		}
	}
	
	if(dt->vb->Lock(0, VBUFSIZE, (void**)&pVertices, 0)==D3D_OK)
	{
		CUSTOMVERTEX	*v=(CUSTOMVERTEX *)pVertices;
		int				n;
		float			db=beat-(int)beat;

		float			tdu=1.f/8.f;
		float			tdv=1.f/8.f;

		for(n=0; n<(NBAXE-1); n++)
		{
			int	n1=n+1;
			int	i;
			
			float	tv=(float)(n&7)*tdv-db;
			float	tv1=tv+tdv;

			dword	c=D3DCOLOR_RGBA(rc, gc, bc, (int)(maxi(0.f, mini(1.f, (((float)n/(float)NBAXE)+(info->power*2.f-1.f))))*255.99f));
			
			for(i=0; i<NBPOINT; i++)
			{
				int			i1=(i+1)&(NBPOINT-1);
				int			di=i&15;
				float		tu,tu1;
				if(di<8)
				{
					tu=(float)di*tdu;
					tu1=tu+tdu;
				}
				else
				{
					tu=(float)(16-di)*tdu;
					tu1=tu-tdu;
				}
				
				addvertice(v++, &points[n][i], c, tu, tv, &normals[n][i]);
				addvertice(v++, &points[n1][i], c, tu, tv1, &normals[n1][i]);
				addvertice(v++, &points[n][i1], c, tu1, tv, &normals[n][i1]);
				ntri++;
				
				addvertice(v++, &points[n][i1], c, tu1, tv, &normals[n][i1]);
				addvertice(v++, &points[n1][i], c, tu, tv1, &normals[n1][i]);
				addvertice(v++, &points[n1][i1], c, tu1, tv1, &normals[n1][i1]);
				ntri++;
			}
		}
		dt->vb->Unlock();
	}
	
//	d3d->d3dev->BeginScene();

	{	// matrix
		D3DXMATRIX	matWorld;
		D3DXMATRIX	matView;
		D3DXMATRIX	matProj;
		D3DXMATRIX	matTemp1;
		D3DXMATRIX	matTemp2;
	
		d3d->d3dev->BeginScene();

		D3DXMatrixRotationY(&matTemp1, 0.f);
		D3DXMatrixRotationZ(&matTemp2, 0.f);
		
		D3DXMatrixMultiply(&matWorld, &matTemp1, &matTemp2);
	
		d3d->d3dev->SetTransform(D3DTS_WORLD, &matWorld);
	
		// view matrix
		D3DXMatrixLookAtLH( &matView,	&D3DXVECTOR3( 0.0f, 0.0f, 0.01f),
										&D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
										&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) ); 
									
		d3d->d3dev->SetTransform(D3DTS_VIEW, &matView);
	
		// projection matrix
		D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4, 1.0f, 1.0f, 1000.0f );
		d3d->d3dev->SetTransform(D3DTS_PROJECTION, &matProj);
	}	

	// light
	{
		D3DXVECTOR3 vecDir;
		D3DLIGHT9	light;
		ZeroMemory(&light, sizeof(light));

		light.Type = D3DLIGHT_POINT;

		light.Diffuse.r = 3.0f;
		light.Diffuse.g = 3.0f;
		light.Diffuse.b = 3.0f;
		
		light.Specular.r= 0.5f;
		light.Specular.g= 0.5f;
		light.Specular.b= 0.5f;
		
		light.Phi=(float)PI;
		light.Theta=light.Phi*0.5f;
		
		light.Attenuation0=0.0f;
		light.Attenuation1=0.0f;
		light.Attenuation2=0.2f;
		
		light.Position=D3DXVECTOR3(-0.5f, -0.5f, 0.f);
		
		vecDir=D3DXVECTOR3(0.f, 0.f, 1.f);
		D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);

		light.Range = 30.0f;

		d3d->d3dev->SetLight(0, &light);
		d3d->d3dev->LightEnable(0, TRUE);
	}

	d3d->d3dev->SetRenderState(D3DRS_LIGHTING, TRUE);
	d3d->d3dev->SetRenderState(D3DRS_AMBIENT, 0xff202020);	// debug

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

/*
	if(nin==5)
		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
*/

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

	//d3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	d3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        
	r=d3d->d3dev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, ntri);

	d3d->d3dev->LightEnable(0, FALSE);

	d3d->d3dev->EndScene();


	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtunel::fillSurface()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect3d * Aef3dtunelInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dtunel(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dtunelGetInfo()
{
	return new Aef3dtunelInfo("tunel", &Aef3dtunel::CI, "tunel", "tunel");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
