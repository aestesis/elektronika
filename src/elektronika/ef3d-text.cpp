/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ef3f-text.CPP				(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"ef3d-text.h"
#include						"resource.h"
#include						<math.h>
#include						<d3d9.h>
#include						<D3dx9mesh.h>
#include						<d3dx9shape.h>
#include						<assert.h>
#include						"tcpRemote.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Aef3dtextInfo::CI	= ACI("Aef3dtextInfo",	GUID(0x11111112,0xB0000100), &Aeffect3dInfo::CI, 0, NULL);
ACI								Aef3dtext::CI		= ACI("Aef3dtext",		GUID(0x11111112,0xB0000101), &Aeffect3d::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Vertex
{
    float x, y, z;    // Position of vertex in 3D space
    float nx, ny, nz; // Normal for lighting calculations
    DWORD diffuse;    // Diffuse color of vertex

	enum FVF
	{
		FVF_Flags = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE
	};
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AdataChar
{
public:
								AdataChar						(Ad3d *d3d, HDC hdc, char c);
								~AdataChar						();

	void						draw							(dword color);

	//Vertex						*vertex;
	bool						ok;
	ID3DXMesh					*mesh;
	D3DXVECTOR4					vmin;
	D3DXVECTOR4					vmax;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdataChar::AdataChar(Ad3d *d3d, HDC hdc, char c)
{
	LPD3DXMESH	tMesh = NULL;
	char		str[2];

	ok=false;
	mesh=null;
	str[0]=c;
	str[1]=0;

	vmin.x=vmin.y=vmin.z=vmin.w=0.f;
	vmax.x=vmax.y=vmax.z=vmax.w=0.f;

	if(!FAILED(D3DXCreateText(d3d->d3dev, hdc, str, 0.001f, 0.4f, &tMesh, NULL, NULL)))
	{
		tMesh->CloneMeshFVF( 0, Vertex::FVF_Flags, d3d->d3dev, &mesh);
		tMesh->Release();
		D3DXComputeNormals(mesh, NULL);
		ok=true;
		//mesh->Optimize(...);
		{
			LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;
			if( SUCCEEDED( mesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
			{
				int		nNumVerts=mesh->GetNumVertices();
				Vertex	*pV=NULL;

				pTempVertexBuffer->Lock( 0, 0, (void**)&pV, 0 );
				{
					for( int i = 0; i < nNumVerts; ++i )
					{
						pV->diffuse=0xffffffff;
						vmin.x=mini(pV->x, vmin.x);
						vmax.x=maxi(pV->x, vmax.x);
						vmin.y=mini(pV->y, vmin.y);
						vmax.y=maxi(pV->y, vmax.y);
						vmin.z=mini(pV->z, vmin.z);
						vmax.z=maxi(pV->z, vmax.z);
						pV++;
					}
				}
				pTempVertexBuffer->Unlock();
				pTempVertexBuffer->Release();

			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdataChar::~AdataChar()
{
	if(mesh)
		mesh->Release();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdataChar::draw(dword color)
{
	LPDIRECT3DVERTEXBUFFER9 pTempVertexBuffer;
	if( SUCCEEDED( mesh->GetVertexBuffer( &pTempVertexBuffer ) ) )
	{
		int		nNumVerts=mesh->GetNumVertices();
		Vertex	*pV=NULL;
		pTempVertexBuffer->Lock( 0, 0, (void**)&pV, 0 );
		if(pV->diffuse!=color)
		{
			for( int i = 0; i < nNumVerts; ++i )
			{
				pV->diffuse=color;
				pV++;
			}
		}
		pTempVertexBuffer->Unlock();
		pTempVertexBuffer->Release();
		mesh->DrawSubset(0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TEXSIZE					512

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Adatatext
{
public:
	HDC							dc;
	class AdataChar				*fchar[256];
	
	float						azoom;
	float						arot;

	D3DXVECTOR4					vcmin;
	D3DXVECTOR4					vcmax;
	D3DXVECTOR4					vcmoy;

	Adatatext()
	{
		azoom=0.f;
		arot=0.f;
		vcmin.x=vcmin.y=vcmin.z=vcmin.w=0.f;
		vcmax.x=vcmax.y=vcmax.z=vcmax.w=0.f;
		vcmoy.x=vcmoy.y=vcmoy.z=vcmoy.w=0.f;
		dc=CreateCompatibleDC(NULL);
		memset(fchar, 0, sizeof(fchar));
	}
	
	~Adatatext()
	{
		int	i;
		DeleteDC(dc);
		for(i=0; i<256; i++)
			if(fchar[i])
				delete(fchar[i]);
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

#define							MAXFONT						4096

static int						nbfont=0;
static char						fontname[MAXFONT][LF_FULLFACESIZE];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int CALLBACK MyEnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,  DWORD FontType, LPARAM lParam)
{
	if(nbfont<MAXFONT)
	{
		if(FontType==TRUETYPE_FONTTYPE)
		{
			int	bir=lpntme->ntmTm.ntmFlags&(NTM_ITALIC|NTM_BOLD|NTM_REGULAR);
			memcpy(fontname[nbfont++], lpelfe->elfFullName, LF_FULLFACESIZE);
		}
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!count)
	{
		LOGFONT	lf;
		memset(&lf, 0, sizeof(lf));
		EnumFontFamiliesEx(GetDC(null), &lf, (FONTENUMPROC)MyEnumFontFamExProc, null, 0);
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		nbfont=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dtext::Aef3dtext(QIID qiid, char *name, Aobject *f, int x, int y) : Aeffect3d(qiid, name, f, x, y)
{
	char	str[1024];
	init();
	dt=new Adatatext();

	memset(text, 0, sizeof(text));

	ipos=0.f;

	cfont=0;
	strcpy(text, "Elektronika Live");
	bText=true;
	biFont=0;

	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_EF3D_ZOOM), "PNG");
	back=new Abitmap(&o);

	sprintf(str, "%s/text", name);
	oscText=new AoscNode(str, Aeffect::getEffect(this), ",s", "text body");
	
	setTooltips("text effect");
	
	sprintf(str, "%s/speed", name);
	speed=new Apaddle(qiid*0x4e899215a725e478, str, this, 8, 4, paddleYOLI16);
	speed->setTooltips("speed");
	speed->set(0.4f);
	speed->show(TRUE);
	
	sprintf(str, "%s/zoom", name);
	zoom=new Apaddle(qiid*0x4e89b26c4725e478, str, this, 32, 4, paddleYOLI16);
	zoom->setTooltips("zoom");
	zoom->set(0.25f);
	zoom->show(TRUE);
	iZoom=1.f+pow(zoom->get(), 3.f)*20.f;
	
	sprintf(str, "%s/rotation", name);
	rot=new Apaddle(qiid*0x1ef8a7c5b1241314, str, this, 56, 4, paddleYOLI16);
	rot->setTooltips("rotation");
	rot->set(0.5f);
	rot->show(TRUE);
	
	edit=new Aedit("edit", this, 4, 24, pos.w-52, pos.h-48);
	edit->state|=Aedit::stateMULTILINE;
	edit->setTooltips("text");
	edit->set(text);
	edit->show(true);

	sprintf(str, "%s/position", name);
	zone=new Azone(MKQIID(qiid, 0x4eab585de665c478), str, this, pos.w-44, 24, 40, 40, 0.5f, 0.5f);
	zone->setTooltips("position");
	zone->set(0.5f, 0.5f);
	zone->show(TRUE);

	ifont=new Aitem("font", "font");
	{
		int	i;
		for(i=0; i<nbfont; i++)
			new Aitem(fontname[i], fontname[i], ifont, i);
	}
	
	ifont->sort();
	
	lfont=new Alist("lfont", this, 4, pos.h-20, pos.w-80, 14, ifont);
	lfont->setCurrentByData(0);
	lfont->setTooltips("fonts");
	lfont->show(TRUE);
	
	bBold=false;
	bold=new Abutton("bold", this, pos.w-72, pos.h-20, 14, 14, &resource.get(MAKEINTRESOURCE(PNG_BOLD), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	bold->setTooltips("bold");
	bold->show(true);
	
	bItalic=false;
	italic=new Abutton("italic", this, pos.w-54, pos.h-20, 14, 14, &resource.get(MAKEINTRESOURCE(PNG_ITALIC), "PNG"), Abutton::btBITMAP|Abutton::bt2STATES);
	italic->setTooltips("italic");
	italic->show(true);

	//
	iType=0;
	sprintf(str, "%s/type", name);
	type=new Aselect(MKQIID(qiid, 0x5698541256985475), str, this, pos.w-73, 6, 4, 1, &resource.get(MAKEINTRESOURCE(PNG_EF3D_TEXTTYPE), "PNG"), 14, 14);
	type->setTooltips("display type");
	type->show(true);
	
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aef3dtext::~Aef3dtext()
{
//	delete(oscText); deleted by the Anode delete
	delete(speed);
	delete(zoom);
	delete(rot);
	delete(edit);
	delete(lfont);
	delete(ifont);
	delete(bold);
	delete(italic);
	delete(type);
	delete(zone);
	delete(dt);
	delete(back);

	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::oscMessage(class AoscMessage *reply, class AoscMessage *msg, class AoscNode *onode, int action)
{
	if(onode==oscText)
	{
		switch(action)
		{
			case oscGETVALUE:
			reply->add(new AoscString(text));
			return true;	// reply used

			case oscSETVALUE:
			if(msg->fchild&&msg->fchild->isCI(&AoscString::CI))
			{
				char *s=((AoscString *)msg->fchild)->value;
				if(s)
				{
					strcpy(text, s);
					edit->set(text);
				}
			}
			return false;	// reply not used
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==edit)
		{
			section.enter(__FILE__,__LINE__);
			strncpy(text, edit->caption, sizeof(text));
			text[sizeof(text)-1]=0;
			bText=true;
			section.leave();
		}
		else if(o==lfont)
		{
			section.enter(__FILE__,__LINE__);
			cfont=lfont->getCurrentData();
			biFont=0;
			bText=true;
			section.leave();
		}
		else if(o==bold)
		{
			section.enter(__FILE__,__LINE__);
			bBold=bold->isChecked();
			biFont=0;
			bText=true;
			section.leave();
		}
		else if(o==italic)
		{
			section.enter(__FILE__,__LINE__);
			bItalic=italic->isChecked();
			biFont=0;
			bText=true;
			section.leave();
		}
		else if(o==type)
		{
			section.enter(__FILE__,__LINE__);
			this->iType=type->get();
			bText=true;
			section.leave();
		}
		else if(o==zoom)
		{
			section.enter(__FILE__,__LINE__);
			float v=1.f+pow(zoom->get(), 3.f)*20.f;
			switch(iType)
			{
				case ttHORIZONTAL:
				case ttMOVIE:
				ipos*=v/iZoom;
				break;
			}
			iZoom=v;
			section.leave();
		}

		return true;
	}
	return Aobject::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dtext::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
	//b->boxfa(0, 0, pos.w, pos.h, 0xff000000, 0.1f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::save(class Afile *f)
{
	Aitem *i=lfont->getCurrent();
	f->write(&bBold, sizeof(bBold));
	f->write(&bItalic, sizeof(bItalic));
	f->writeString(i->name);
	f->writeString(text);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::load(class Afile *f)
{
	char	name[1024];
	f->read(&bBold, sizeof(bBold));
	f->read(&bItalic, sizeof(bItalic));
	f->readString(name);
	f->readString(text);
	bold->setChecked(bBold);
	italic->setChecked(bItalic);
	if(!lfont->setCurrentByName(name))
		lfont->setCurrentByData(0);
	cfont=lfont->getCurrentData();
	edit->set(text);
	biFont=0;
	bText=true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aef3dtext::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::init3D(Aef3dInfo *info)
{
	setFont(info);
	setText(info);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::setText(Aef3dInfo *info)
{
	switch(iType)
	{
		case ttHORIZONTAL:
		{
			float	sp=(0.5f-speed->get())*30.f;
			if(sp>0.f)
			{
				ipos=-5.f;
			}
			else 
			{
				int		i;
				int		len=strlen(text);
				float	x=0;

				for(i=0; i<len; i++)
				{
					float	z=iZoom;
					int		n=text[i];
					if((n>32)&&(n<256))
					{
						if(dt->fchar[n]&&dt->fchar[n]->ok)
							x+=(dt->fchar[n]->vmax.x+dt->vcmoy.x*0.1f)*z;
						else
							x+=(dt->vcmoy.x)*z;
					}
					else if(n==13)
						x+=(dt->vcmoy.x*5.f)*z;
					else
						x+=(dt->vcmoy.x)*z;
				}
				ipos=x+5.f;
			}
		}
		break;

		case ttZOOM:
		{
			float	sp=(0.5f-speed->get())*30.f;
			int		i;
			int		len=strlen(text);
			iLine=1;
			for(i=0; i<len; i++)
			{
				int		n=text[i];
				if(n==13)
					iLine++;
			}
			if(sp>0.f)
				ipos=0;
			else
				ipos=iLine*10.f-0.0001f;
		}
		break;

		case ttMOVIE:
		{
			float	sp=(0.5f-speed->get())*30.f;
			int		i;
			int		len=strlen(text);
			iLine=1;
			for(i=0; i<len; i++)
			{
				int		n=text[i];
				if(n==13)
					iLine++;
			}
			if(sp>0.f)
				ipos=-5.f-(dt->vcmoy.y*1.5f)*iZoom;
			else
				ipos=5.f+iLine*(dt->vcmoy.y*1.5f)*iZoom;
		}
		break;

		case ttCHAR:
		{
			ipos=0;
		}
		break;
	}

	bText=false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::setFont(Aef3dInfo *info)
{
	bool b=false;

	section.enter(__FILE__,__LINE__);

	if(biFont==0)
	{
		hFont=CreateFont(	10,							//Height
								0,							//Width
								0,							//Escapement
								0,							//Orientation
								bBold?FW_BOLD:FW_NORMAL,	//Weight
								bItalic,					//Italic
								false,						//Underline
								false,						//Strikeout
								DEFAULT_CHARSET,			//Charset 
								OUT_DEFAULT_PRECIS,			//Output Precision
								CLIP_DEFAULT_PRECIS,		//Clipping Precision
								DEFAULT_QUALITY,			//Quality
								DEFAULT_PITCH|FF_DONTCARE,	//Pitch and Family
								fontname[cfont]);
								
		if(!hFont)
		{
			biFont=256;
			section.leave();
			return true;
		}

		hFont=(HFONT)SelectObject(dt->dc, hFont);

		dt->vcmin.x=dt->vcmin.y=dt->vcmin.z=dt->vcmin.w=0.f;
		dt->vcmax.x=dt->vcmax.y=dt->vcmax.z=dt->vcmax.w=0.f;
		dt->vcmoy.x=dt->vcmoy.y=dt->vcmoy.z=dt->vcmoy.w=0.f;

		tmpFontCount=0;
	}

	if(biFont<256)
	{
		int	i=biFont;
		if(dt->fchar[i])
			delete(dt->fchar[i]);
		dt->fchar[i]=new AdataChar(info->d3d, dt->dc, (char)i);
		if(dt->fchar[i]->ok)
		{
			AdataChar	*fc=dt->fchar[i];
			dt->vcmin.x=mini(fc->vmin.x, dt->vcmin.x);
			dt->vcmax.x=maxi(fc->vmax.x, dt->vcmax.x);
			dt->vcmin.y=mini(fc->vmin.y, dt->vcmin.y);
			dt->vcmax.y=maxi(fc->vmax.y, dt->vcmax.y);
			dt->vcmin.z=mini(fc->vmin.z, dt->vcmin.z);
			dt->vcmax.z=maxi(fc->vmax.z, dt->vcmax.z);
			dt->vcmoy.x+=fc->vmax.x-fc->vmin.x;
			dt->vcmoy.y+=fc->vmax.y-fc->vmin.y;
			dt->vcmoy.z+=fc->vmax.z-fc->vmin.z;
			tmpFontCount++;
		}
	}

	if(biFont==255)
	{
		if(tmpFontCount)
		{
			dt->vcmoy.x/=(float)tmpFontCount;
			dt->vcmoy.y/=(float)tmpFontCount;
			dt->vcmoy.z/=(float)tmpFontCount;
		}
		SelectObject(dt->dc, hoFont);
		DeleteObject(hFont);
	}

	biFont++;

	section.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::release3D(Aef3dInfo *info)
{
	int	i;
	for(i=0; i<96; i++)
		if(dt->fchar[i])
		{
			delete(dt->fchar[i]);
			dt->fchar[i]=null;
		}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::action(Aef3dInfo *info)
{
	Aeffect				*e=Aeffect::getEffect(this);
	Ad3d				*d3d=info->d3d;
	HRESULT				r;
	float				w=(float)d3d->w;
	float				h=(float)d3d->h;
	byte				rc,gc,bc;
	int					texType;
	float				texCx,texCy;
	float				minx=0.f;
	float				maxx=0.f;
	float				miny=0.f;
	float				maxy=0.f;
	float				minz=0.f;
	float				maxz=0.f;

	colorRGB(&rc,&gc, &bc, info->color);
	dword				color=D3DCOLOR_RGBA(rc, gc, bc, (int)(info->power*255.99f));
	
	section.enter(__FILE__,__LINE__);
	if(biFont<256)
	{
		setFont(info);
		section.leave();
		return true;
	}
	if(bText)
		setText(info);
	section.leave();

	IDirect3DTexture9	*tex=info->input->getTexture(&texType, &texCx, &texCy);

	d3d->d3dev->BeginScene();

	{
		D3DXMATRIX view_matrix;
		D3DXMATRIX projection_matrix;
		D3DXVECTOR3 eye_vector;
		D3DXVECTOR3 lookat_vector;
		D3DXVECTOR3 up_vector;
		D3DXMATRIX world_matrix;
		float aspect;

		eye_vector=D3DXVECTOR3( 0.0f, 0.0f, -8.0f );
		lookat_vector=D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		up_vector=D3DXVECTOR3(0.0f,1.0f,0.0f);

		D3DXMatrixLookAtLH(&view_matrix, &eye_vector, &lookat_vector, &up_vector);

		d3d->d3dev->SetTransform(D3DTS_VIEW, &view_matrix);

		aspect=((float)e->getVideoWidth() / (float)e->getVideoHeight());

		D3DXMatrixPerspectiveFovLH(&projection_matrix, //Result Matrix
                              D3DX_PI/4,          //Field of View, in radians.
                              aspect,             //Aspect ratio
                              0.0f,               //Near view plane
                              100.0f );           //Far view plane

		d3d->d3dev->SetTransform(D3DTS_PROJECTION, &projection_matrix);
	}

	// light
	{
		D3DXVECTOR3 vecDir;
		D3DLIGHT9	light;
		ZeroMemory(&light, sizeof(light));

		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Direction = D3DXVECTOR3( -1.0f, 1.0f, 1.f );

		light.Diffuse.r = 1.0f;
		light.Diffuse.g = 1.0f;
		light.Diffuse.b = 1.0f;
		light.Diffuse.a = 1.0f;

		light.Specular.r = 1.0f;
		light.Specular.g = 1.0f;
		light.Specular.b = 1.0f;
		light.Specular.a = 1.0f;

		d3d->d3dev->SetLight( 0, &light );
		d3d->d3dev->LightEnable( 0, TRUE );
	}

	if(0)//tex)
	{
		d3d->d3dev->SetRenderState(D3DRS_LIGHTING, TRUE);
		d3d->d3dev->SetRenderState(D3DRS_AMBIENT, 0xff202020);	// debug

		r=d3d->d3dev->SetTexture(0, tex);

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
	}
	else
	{
		d3d->d3dev->SetRenderState(D3DRS_LIGHTING, TRUE);
		d3d->d3dev->SetRenderState(D3DRS_SPECULARENABLE, FALSE); 
		d3d->d3dev->SetRenderState(D3DRS_AMBIENT, 0xff000000);

		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		
		if(info->colorBoost)
			r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
		else
			r=d3d->d3dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}

	if(tex)
		tex->Release();

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

	{
		D3DMATERIAL9	mat;
		memset(&mat, 0, sizeof(mat));
		
		// Set the material's main color to a silver-like gray color.
		mat.Diffuse.r = (float)rc/256.f;
		mat.Diffuse.g = (float)gc/256.f;
		mat.Diffuse.b = (float)bc/256.f;
		mat.Diffuse.a = 1.f;

		// A silver metal would be very shiny, so we'll reflect back all ambient.
		mat.Ambient.r = 0.f;
		mat.Ambient.g = 0.f;
		mat.Ambient.b = 0.f;
		mat.Ambient.a = 1.f;

		// We can make it seem extra shiny by having it actually emit some light 
		// of its own... but not too much are we'll wash the color out.
		mat.Emissive.r = 0.0f;
		mat.Emissive.g = 0.0f;
		mat.Emissive.b = 0.0f;
		mat.Emissive.a = 1.f;

		// Polished silver can reflect back pure white highlights, so set the 
		// specular to pure white.

		mat.Specular.r = (float)rc/256.f;
		mat.Specular.g = (float)gc/256.f;
		mat.Specular.b = (float)bc/256.f;
		mat.Specular.a = 0.f;

		// Set the Power value to a small number to make the highlight's size bigger.
		mat.Power = 1000.0f;
		d3d->d3dev->SetMaterial(&mat);
	}

	float	posx,posy;
	zone->get(&posx, &posy);

	float	ro=(float)(rot->get()*2*PI+PI);
	float	dx=(float)((posx-0.5f)*10.f);
	float	dy=(float)((0.5f-posy)*10.f);
	float	sp=(float)((0.5f-speed->get())*30.f);

	d3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);


	section.enter(__FILE__,__LINE__);
	switch(iType)
	{
		case ttHORIZONTAL:
		{
			int		i;
			int		len=strlen(text);
			float	x=(float)-ipos;

			for(i=0; i<len; i++)
			{
				float	z=iZoom;
				int		n=(byte)text[i];
				if((n>32)&&(n<256))
				{
					if(dt->fchar[n]&&dt->fchar[n]->ok)
					{
						D3DXMATRIX	world;
						D3DXMATRIX	scale;
						D3DXMATRIX	trans;
						D3DXMATRIX	rotat;

						float		moy=dt->vcmoy.y*z*0.5f;
						float		dys=0;//sin(info->beat+x-ipos);

						D3DXMatrixScaling(&scale, z, z, 0.1f);
						D3DXMatrixTranslation(&trans, x, dy+dys-moy, 1.f);
						trans._11=scale._11;
						trans._22=scale._22;
						trans._33=scale._33;
						D3DXMatrixRotationZ(&rotat, ro);
						D3DXMatrixMultiply(&world, &trans, &rotat);

						d3d->d3dev->SetTransform(D3DTS_WORLD, &world);

						x+=(dt->fchar[n]->vmax.x+dt->vcmoy.x*0.1f)*z;
						if(x>-5.f)
							dt->fchar[n]->draw(color);
					}
					else
						x+=(dt->vcmoy.x)*z;
				}
				else if(n==13)
					x+=(dt->vcmoy.x*5.f)*z;
				else
					x+=(dt->vcmoy.x)*z;
				if(x>5.f)
					break;
			}

			if(sp>0.f)
			{
				if(x<-5.f)
					setText(info);
			}
			else 
			{
				if(ipos<-5.f)
					setText(info);
			}

			ipos+=info->dbeat*sp;
		}
		break;

		case ttZOOM:
		{
			int		n=(int)(ipos*0.1f);
			float	v=(float)(ipos-(float)(n*10));
			char	tt[8192];
			float	zz=1.f;

			tt[0]=0;
			if(v<3.f)
			{
				v=3.f-v;
				zz=1.f/((1.f+v)*(1.f+v));
			}
			else if(v>7.f)
			{
				v=v-7.f;
				zz=(1.f+v)*(1.f+v);
			}
			else 
				v=0.f;

			float	p=info->power*(3.f-v)*0.3333f;
			color=D3DCOLOR_RGBA(rc, gc, bc, (int)(p*255.99f));

			{
				char	*s=text;
				int		ns=0;
				while(*s)
				{
					if(ns==n)
						break;
					if(*s==13)
						ns++;
					s++;
				}

				while(*s&&(*s<32)&&(*s!=13))
					s++;
				
				strcpy(tt, s);

				{
					char	*st=tt;
					while(*st&&(*st!=13))
						st++;
					*st=0;
				}
			}
			
			float	xs=0.f;

			{
				char	*s=tt;
				while(*s)
				{
					float	z=iZoom*zz;
					int		n=*(s++);
					if((n>32)&&(n<256))
					{
						if(dt->fchar[n]&&dt->fchar[n]->ok)
							xs+=(dt->fchar[n]->vmax.x+dt->vcmoy.x*0.1f)*z;
						else
							xs+=(dt->vcmoy.x)*z;
					}
					else if(n==13)
						xs+=(dt->vcmoy.x*5.f)*z;
					else
						xs+=(dt->vcmoy.x)*z;
				}
			}
			
			{
				int		i;
				int		len=strlen(tt);
				float	x=-xs*0.5f;

				for(i=0; i<len; i++)
				{
					float	z=iZoom*zz;
					int		n=(byte)tt[i];
					if((n>32)&&(n<256))
					{
						if(dt->fchar[n]&&dt->fchar[n]->ok)
						{
							D3DXMATRIX	world;
							D3DXMATRIX	scale;
							D3DXMATRIX	trans;
							D3DXMATRIX	rotat;

							float		moy=dt->vcmoy.y*z*0.5f;
							float		dys=0;//sin(info->beat+x-ipos);

							D3DXMatrixScaling(&scale, z, z, 0.1f);
							D3DXMatrixTranslation(&trans, x+dx*zz, dy*zz+dys-moy, 1.f);
							trans._11=scale._11;
							trans._22=scale._22;
							trans._33=scale._33;
							D3DXMatrixRotationZ(&rotat, ro);
							D3DXMatrixMultiply(&world, &trans, &rotat);

							d3d->d3dev->SetTransform(D3DTS_WORLD, &world);

							x+=(dt->fchar[n]->vmax.x+dt->vcmoy.x*0.1f)*z;
							if(x>-5.f)
								dt->fchar[n]->draw(color);
						}
						else
							x+=(dt->vcmoy.x)*z;
					}
					else if(n==13)
						x+=(dt->vcmoy.x*5.f)*z;
					else
						x+=(dt->vcmoy.x)*z;
					if(x>6.f)
						break;
				}
			}

			ipos+=info->dbeat*sp;
			if(sp>0.f)
			{
				if(ipos>(float)(10*iLine))
					setText(info);
			}
			else 
			{
				if(ipos<0.f)
					setText(info);
			}

		}
		break;

		case ttMOVIE:
		{
			int		i;
			int		len=strlen(text);
			float	y=(float)(-ipos);
			float	z=iZoom;

			for(i=0; i<len; i++)
			{
				int		n=(byte)text[i];
				float	oy=y;
				if((n>32)&&(n<256))
				{
					float	xs=0.f;

					char	*tt=&text[i];
					{
						char	*s=tt;
						while(*s)
						{
							int		n=(byte)*(s++);
							if((n>32)&&(n<256))
							{
								if(dt->fchar[n]&&dt->fchar[n]->ok)
									xs+=(dt->fchar[n]->vmax.x+dt->vcmoy.x*0.1f)*z;
								else
									xs+=(dt->vcmoy.x)*z;
							}
							else if(n==13)
								break;
							else
								xs+=(dt->vcmoy.x)*z;
							i++;
						}
					}
					
					{
						int		len=0;
						float	x=-xs*0.5f;
						char	*s=tt;


						while(*s&&(*s!=13))
						{
							int		n=(byte)*(s++);
							if((n>32)&&(n<256))
							{
								if(dt->fchar[n]&&dt->fchar[n]->ok)
								{
									D3DXMATRIX	world;
									D3DXMATRIX	scale;
									D3DXMATRIX	trans;
									D3DXMATRIX	rotat;

									float		dys=0;//sin(info->beat+x-ipos);

									D3DXMatrixScaling(&scale, z, z, 0.1f);
									D3DXMatrixTranslation(&trans, x+dx, -y, 1.f);
									trans._11=scale._11;
									trans._22=scale._22;
									trans._33=scale._33;
									D3DXMatrixRotationZ(&rotat, ro);
									D3DXMatrixMultiply(&world, &trans, &rotat);

									d3d->d3dev->SetTransform(D3DTS_WORLD, &world);

									x+=(dt->fchar[n]->vmax.x+dt->vcmoy.x*0.1f)*z;
									if(x>-5.f)
										dt->fchar[n]->draw(color);
								}
								else
									x+=(dt->vcmoy.x)*z;
							}
							else
								x+=(dt->vcmoy.x)*z;
							if(x>6.f)
								break;
						}
					}
					y+=(dt->vcmoy.y*1.5f)*z;
				}
				else if(n==13)
					y+=(dt->vcmoy.y*1.5f)*z;
				if(oy>5.f)
					break;
			}

			if(sp>0.f)
			{
				if(y<-5.f)
					setText(info);
			}
			else 
			{
				if(ipos<-5.f)
					setText(info);
			}

			ipos+=info->dbeat*sp*0.5f;
		}
		break;

		case ttCHAR:
		{
			int		len=strlen(text);
			int		nc=(int)ipos%len;

			if(nc<0)
				nc+=len;

			//if((ipos-(double)(int)ipos)<0.8f)
			{
				float	z=iZoom;
				int		n=(byte)text[nc];
				if((n>32)&&(n<256))
				{
					if(dt->fchar[n]&&dt->fchar[n]->ok)
					{
						D3DXMATRIX	world;
						D3DXMATRIX	scale;
						D3DXMATRIX	trans;
						D3DXMATRIX	rotat;

						float		moy=dt->vcmoy.y*z*0.5f;
						float		mox=dt->fchar[n]->vmax.x*z*0.5f;

						D3DXMatrixScaling(&scale, z, z, 0.1f);
						D3DXMatrixTranslation(&trans, dx-mox, dy-moy, 1.f);
						trans._11=scale._11;
						trans._22=scale._22;
						trans._33=scale._33;
						D3DXMatrixRotationZ(&rotat, ro);
						D3DXMatrixMultiply(&world, &trans, &rotat);

						d3d->d3dev->SetTransform(D3DTS_WORLD, &world);

						dt->fchar[n]->draw(color);
					}
				}
			}

			ipos+=info->dbeat*sp*0.5;
		}
		break;
	}
	section.leave();

	d3d->d3dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	d3d->d3dev->LightEnable(0, FALSE);
	d3d->d3dev->SetRenderState(D3DRS_SPECULARENABLE, FALSE); 
	d3d->d3dev->EndScene();
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aef3dtext::fillSurface()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect3d * Aef3dtextInfo::create(QIID qiid, char *name, Aobject *father, int x, int y)
{
	return new Aef3dtext(qiid, name, father, x, y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * ef3dtextGetInfo()
{
	return new Aef3dtextInfo("text", &Aef3dtext::CI, "text", "text");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
