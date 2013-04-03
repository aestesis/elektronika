/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<io.h>
#include						<assert.h>
#include						"main.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"dsglurf.h"
#include						"house.h"
#include						"xtime.h"
#include						"zygo.h"
#include						"player.h"
#include						"flammes.h"
#include						"patchly.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// elektronika SDK effect sample
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// if you want to use MFC/AFX
// add the MFC/AFX includes
// select "MFC  shared dll" in general tab of the project settings
// and remove the _USRDLL define in the project settings
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								A98Info::CI		= ACI("A98Info",	GUID(0x50000000,0x00000950), &AeffectInfo::CI, 0, NULL);
ACI								A98::CI			= ACI("A98",		GUID(0x50000000,0x00000951), &Aeffect::CI, 0, NULL);
ACI								A98Front::CI	= ACI("A98Front",	GUID(0x50000000,0x00000952), &AeffectFront::CI, 0, NULL);
ACI								A98Back::CI		= ACI("A98Back",	GUID(0x50000000,0x00000953), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Azone98 : public AcontrolObj
{
public:
	AOBJ
	
								Azone98							(QIID qiid, char *name, Aobject *father, int x, int y, int w, int h, float wsel, float hsel);
	virtual						~Azone98						();

	virtual bool				mouse							(int x, int y, int state, int event);
	virtual void				paint							(Abitmap *b);
	virtual void				pulse							();

	virtual bool				sequence						(int nctrl, float value);

	void						get								(float *x, float *y) { *x=xx; *y=yy; };
	void						set								(float x, float y);

private:
	float						xx,yy;
	float						wsel,hsel;


	int							ow,oh;
	int							x0,y0;
	float						xx0,yy0;
	bool						test;
	bool						ctrlPaint;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								Azone98::CI	= ACI("Azone98",	GUID(0x50000000,0x00000956), &AcontrolObj::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Azone98::Azone98(QIID qiid, char *name, Aobject *father, int x, int y, int w, int h, float wsel, float hsel) : AcontrolObj(name, father, x, y, w, h)
{
	control=new Acontrol(qiid, "control", Aeffect::getEffect(father), this, Acontrol::CONTROLER_02);
	this->wsel=wsel;
	this->hsel=hsel;
	set(0.f, 0.f);
	test=false;
	ctrlPaint=false;
	timer(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Azone98::~Azone98()
{
	delete(control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Azone98::set(float x, float y)
{
	xx=maxi(mini(x, 1.f), 0.f);
	yy=maxi(mini(y, 1.f), 0.f);
	control->set(Acontrol::CONTROLER_01, xx);
	control->set(Acontrol::CONTROLER_02, yy);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Azone98::sequence(int nctrl, float value)
{
	switch(nctrl)
	{
		case Acontrol::CONTROLER_01:
		xx=value;
		break;

		case Acontrol::CONTROLER_02:
		yy=value;
		break;

		default:
		assert(FALSE);
		break;
	}
	ctrlPaint=true;
	father->notify(this, nyCHANGE, 0);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Azone98::mouse(int x, int y, int state, int event)
{
	switch(event)
	{
		case mouseLDOWN:
		control->select();
		case mouseNORMAL:
		case mouseLUP:
		cursor(cursorHANDSEL);
		switch(event)
		{
			case mouseLDOWN:
			mouseCapture(true);
			test=TRUE;
			x0=x;
			y0=y;
			xx0=xx;
			yy0=1.f-yy;
			break;
		}
		if(test&&(state&mouseL))
		{
			xx=mini(maxi(xx0+alib.mouseSensibility*(float)(x-x0), 0.f), 1.f);
			yy=1.f-mini(maxi(yy0-alib.mouseSensibility*(float)(y-y0), 0.f), 1.f);
			control->set(Acontrol::CONTROLER_01, xx);
			control->set(Acontrol::CONTROLER_02, yy);
			repaint();
			father->notify(this, nyCHANGE);
		}
		switch(event)
		{
			case mouseLUP:
			mouseCapture(FALSE);
			test=FALSE;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Azone98::pulse()
{
	if(ctrlPaint)
	{
		repaint();
		ctrlPaint=false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Azone98::paint(Abitmap *b)
{
	int		wc=pos.w-4;
	int		hc=pos.h-4;
	int		ws=(int)(wc*wsel);
	int		hs=(int)(hc*hsel);
	int		wv=wc-ws;
	int		hv=hc-hs;
	int		x=(int)(wv*xx)+2;
	int		y=(int)(hv*yy)+2;

	b->boxf(0, 0, pos.w, pos.h, 0xff202020);
	b->boxf(x, y, x+ws-1, y+hs-1, 0xffffaa00);
	
	controlPaint(b, pos.w, pos.h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MAX_SOUNDBUFFER					512

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int						FInitialize						(TInfo **, TTooltips **);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct SRGBA
{
	byte						r;
	byte						g;
	byte						b;
	byte						a;
} TRGBA;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct SBitmap
{
	BITMAPINFOHEADER			*pInfo;
	void						*pBody;
	TRGBA						*palette;
} TBitmap;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct SPriv
{
	TBitmap						*pBmp;
	HINSTANCE					hDLL;
	dword						idcrc;
} TPriv;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct SList
{
	TInfo						*pInfo;
	TPriv						*pPriv;
	TTooltips					*pTooltips;
	struct SList				*next;
	struct SList				*previous;
} TList;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static TList					*pFirst				=NULL;
static TList					*pLast				=NULL;

static FInitialize				*Initialize[]=
{
								InitializeSglurf,
								InitializeHouse,
								InitializeZygO,
								InitializePlayer,
								InitializeXtime,
								InitializeFlammes,
								InitializePatchly
								/*
								InitializeGarage,
								*/
};

static TBitmap					*tbdef=NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeBitmap(TBitmap *pBitmap)
{
	if(pBitmap)
	{
		if(pBitmap->pInfo)
			free(pBitmap->pInfo);
		if(pBitmap->palette)
			free(pBitmap->palette);
		if(pBitmap->pBody)
			free(pBitmap->pBody);
		free(pBitmap);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TBitmap *NewBitmap(int width, int height, int nbbits)
{
	if((nbbits==16)||(nbbits==24)||(nbbits==32))
	{
		int					size=width*height*(nbbits>>3);
		TBitmap				*pBitmap=(TBitmap *)malloc(sizeof(TBitmap));
		BITMAPINFOHEADER	*pInfo=(BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
		void				*pBody=(void *)malloc(size);
		if(!(pBitmap&&pInfo&&pBody))
		{
			if(pBitmap)
				free(pBitmap);
			if(pBody)
				free(pBody);
			if(pInfo)
				free(pInfo);
			return NULL;
		}
		memset(pBody, 0, size);
		memset(pInfo, 0, sizeof(BITMAPINFOHEADER));
		pInfo->biSize=sizeof(BITMAPINFOHEADER);
		pInfo->biWidth=width;
		pInfo->biHeight=height;
		pInfo->biPlanes=1;
		pInfo->biBitCount=nbbits;
		pInfo->biCompression=BI_RGB;
		pInfo->biSizeImage=size;
		pBitmap->pInfo=pInfo;
		pBitmap->pBody=pBody;
		pBitmap->palette=NULL;
		return pBitmap;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TBitmap *LoadBitmap(HINSTANCE hinst,char *filename)
{
	if(filename>(char *)0x100000)
	{
		FILE				*f=fopen(filename, "rb");
		TBitmap				*image;
		BITMAPFILEHEADER	bf;
		BITMAPINFOHEADER	bi;
		void				*body;
		int					size;
		if(!f)
			return NULL;
		if(fread(&bf, sizeof(bf), 1, f)!=1)
			goto lblError;
		if(fread(&bi, sizeof(bi), 1, f)!=1)
			goto lblError;
		size=bi.biWidth*bi.biHeight*4;
		if(bi.biBitCount!=24)
			goto lblError;
		image=NewBitmap(bi.biWidth, bi.biHeight, 32);
		if(!image)
			goto lblError;
		body=malloc(bi.biWidth*bi.biHeight*(bi.biBitCount>>3));
		if(!body)
		{
			FreeBitmap(image);
			goto lblError;
		}
		if(fread(body, (size*3/4), 1, f)!=1)
		{
			FreeBitmap(image);
			free(body);
			goto lblError;
		}
		switch(bi.biBitCount)
		{
			case 8:
			break;

			case 16:
			break;

			case 24:
			{
				int		y;
				int		x;
				UCHAR	*src=((UCHAR *)body)+bi.biWidth*(bi.biHeight-1)*3;
				UCHAR	*dest=((UCHAR *)(image->pBody));
				int		w=bi.biWidth*3*2;
				for(y=0; y<bi.biHeight; y++)
				{
					for(x=0; x<bi.biWidth;  x++)
					{
						*(dest++)=*(src++);
						*(dest++)=*(src++);
						*(dest++)=*(src++);
						*(dest++)=0;
					}
					src-=w;
				}
			}
			break;

			case 32:
			{
				int		y=bi.biHeight-1;
				int		w=bi.biWidth*4;
				UINT	*d=((UINT *)(image->pBody))+bi.biWidth*y;
				UINT	*s=(UINT *)body;
				for(;y>=0; y--)
				{
					memcpy(d, s, size);
					d-=bi.biWidth;
					s+=bi.biWidth;
				}
			}
			break;
		}
		free(body);
		fclose(f);
		return image;
	lblError:
		fclose(f);
		return NULL;
	}
	else
	{
        HRSRC				h=FindResource(hinst, filename, RT_BITMAP);
        BITMAPINFOHEADER	*pbi=(BITMAPINFOHEADER *)LockResource(LoadResource(hinst, h));
		if(pbi)
		{
			int					size=pbi->biWidth*pbi->biHeight*4;
			void				*body=(void *)(pbi+1);
			TBitmap				*image;
			if(pbi->biBitCount!=24)
				return NULL;
			image=NewBitmap(pbi->biWidth, pbi->biHeight, 32);
			if(!image)
				return NULL;
			switch(pbi->biBitCount)
			{
				case 8:
				break;

				case 16:
				break;

				case 24:
				{
					int		y;
					int		x;
					UCHAR	*src=((UCHAR *)body)+pbi->biWidth*(pbi->biHeight-1)*3;
					UCHAR	*dest=((UCHAR *)(image->pBody));
					int		w=pbi->biWidth*3*2;
					for(y=0; y<pbi->biHeight; y++)
					{
						for(x=0; x<pbi->biWidth;  x++)
						{
							*(dest++)=*(src++);
							*(dest++)=*(src++);
							*(dest++)=*(src++);
							*(dest++)=0;
						}
						src-=w;
					}
				}
				break;

				case 32:
				{
					int		y=pbi->biHeight-1;
					int		w=pbi->biWidth*4;
					UINT	*d=((UINT *)(image->pBody))+pbi->biWidth*y;
					UINT	*s=(UINT *)body;
					for(;y>=0; y--)
					{
						memcpy(d, s, size);
						d-=pbi->biWidth;
						s+=pbi->biWidth;
					}
				}
				break;
			}
			return image;
		}
    }
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static dword calcCRC(char *name)
{
	dword	crc=0;
	dword	v;
	FILE	*fic=fopen(name, "rb");
	if(fic)
	{
		while(fread(&v, sizeof(v), 1, fic))
			crc^=v;
		fclose(fic);
	}
	return crc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void load98plugz(char *path)
{
	{ // make list of internals effects
		int	i;
		for(i=0; i<countof(Initialize); i++)
		{
			TInfo		*pInfo=NULL;
			TTooltips	*pTooltips=NULL;
			switch(Initialize[i](&pInfo, &pTooltips))
			{
				case AEE_NOERROR:
				{
					TList	*pList=(TList *)malloc(sizeof(TList));
					TPriv	*pPriv=(TPriv *)malloc(sizeof(TPriv));
					TBitmap	*pBmp=NULL;
					if(pList&&pInfo&&pPriv)
					{
						memset(pList, 0, sizeof(TList));
						memset(pPriv, 0, sizeof(TPriv));
						if(pInfo->rBitmap)
							pBmp=LoadBitmap(GetModuleHandle("98.dll"), pInfo->rBitmap);
						pPriv->pBmp=pBmp;
						pPriv->hDLL=NULL;
						pPriv->idcrc=*(dword *)pInfo->Name;
						pList->pInfo=pInfo;
						pList->pTooltips=pTooltips;
						pList->pPriv=pPriv;
						pList->next=pFirst;
						pList->previous=NULL;
						if(pFirst)
							pFirst->previous=pList;
						pFirst=pList;
						if(!pLast)
							pLast=pList;
					}
					else
					{
						if(pBmp)
							free(pBmp);
						if(pList)
							free(pList);
						if(pPriv)
							free(pPriv);
						return;
					}
				}
				break;
			}
		}
	}
	{	// external effects
		struct _finddata_t		fd;
		char					rpath[ALIB_PATH];
		strcpy(rpath, path);
		strcat(rpath, "\\plugins\\98\\*.dll");
		long					h=_findfirst(rpath, &fd);
		if(h!=-1)
		{
			int		r=0;
			while(r!=-1)
			{
				TInfo		*pInfo=NULL;
				TTooltips	*pTooltips=NULL;
				char		str[ALIB_PATH];
				HINSTANCE	hDLL=NULL;
				dword		crc;
				sprintf(str, "%s\\plugins\\98\\%s", path, fd.name);
				crc=calcCRC(str);
				crc^=*(dword *)fd.name;
				hDLL=LoadLibrary(str);
				if(hDLL)
				{
					FInitialize	*pfnInitialize=(FInitialize *)GetProcAddress(hDLL, "Initialize");
					if(pfnInitialize)
					{
						switch(pfnInitialize(&pInfo, &pTooltips))
						{
							case AEE_NOERROR:
							{
								TList	*pList=(TList *)malloc(sizeof(TList));
								TPriv	*pPriv=(TPriv *)malloc(sizeof(TPriv));
								TBitmap	*pBmp=NULL;
								if(pList&&pInfo&&pPriv)
								{
									memset(pList, 0, sizeof(TList));
									memset(pPriv, 0, sizeof(TPriv));
									if(pInfo->rBitmap)
										pBmp=LoadBitmap(hDLL, pInfo->rBitmap);
									pPriv->pBmp=pBmp;
									pPriv->hDLL=hDLL;
									pPriv->idcrc=crc;
									pList->pInfo=pInfo;
									pList->pTooltips=pTooltips;
									pList->pPriv=pPriv;
									pList->next=pFirst;
									pList->previous=NULL;
									if(pFirst)
										pFirst->previous=pList;
									pFirst=pList;
									if(!pLast)
										pLast=pList;
								}
								else
								{
									if(pBmp)
										free(pBmp);
									if(pList)
										free(pList);
									if(pPriv)
										free(pPriv);
									return;
								}
							}
							break;
						}
					}
				}
				r=_findnext(h, &fd);
			}
			_findclose(h);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void free98plugz()
{
	{
		TList	*pList=pFirst;
		TList	*pOld=NULL;
		while(pList)
		{
			pOld=pList;
			pList=pList->next;
			switch(pOld->pInfo->nType)
			{
				case AET_INFO:
				if(pOld->pInfo->Quit)
					pOld->pInfo->Quit();
				if(pOld->pPriv)
				{
					if(pOld->pPriv->pBmp)
						FreeBitmap(pOld->pPriv->pBmp);
					if(pOld->pPriv->hDLL)
						FreeLibrary(pOld->pPriv->hDLL);
					free(pOld->pPriv);
				}
				break;
			}
			free(pOld);
		}
		pFirst=NULL;
		pLast=NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
Aresource						resdll=Aresource("98", GetModuleHandle("98.dll"));

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init(char *path)
{
	if(count==0)
	{
		load98plugz(path);
		tbdef=LoadBitmap(GetModuleHandle("98.dll"), MAKEINTRESOURCE(IDB_DEFAULT));
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		FreeBitmap(tbdef);
		tbdef=NULL;
		free98plugz();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

A98::A98(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init(getRootPath());	// global init

	ef98=NULL;
	vars=NULL;
	memset(&param, 0, sizeof(param));
	param.choix=1;
	param.choix2=1;
	param.choix3=1;
	param.choix4=1;

	image=new Abitmap(320, 200, 32);

	front=new A98Front(qiid, "98 front", this, 104);
	front->setTooltips("98 module");
	back=new A98Back(qiid, "98 back", this, 104);
	back->setTooltips("98 module");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

A98::~A98()
{
	del98();
	delete(image);
	end();	// global free
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void A98::create98(TList *pl)
{
	del98();
	section98.enter(__FILE__,__LINE__);
	ef98=pl;
	vars=ef98->pInfo->Constructor();
	section98.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void A98::del98()
{
	section98.enter(__FILE__,__LINE__);
	if(ef98)
		ef98->pInfo->Destructor(vars);
	ef98=NULL;
	vars=NULL;
	memset(&param, 0, sizeof(param));
	section98.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool A98::save(class Afile *f)
{
	A98Front	*front=(A98Front *)this->front;
	Aitem		*i=front->plugz->getCurrent();
	dword		idcrc=0;
	if(i)
		idcrc=((TList *)i->data)->pPriv->idcrc;
	f->write(&idcrc, sizeof(idcrc));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool A98::load(class Afile *f)
{
	A98Front	*front=(A98Front *)this->front;
	Aitem		*i=(Aitem *)front->plugzItems->fchild;
	dword		idcrc;
	f->read(&idcrc, sizeof(idcrc));
	while(i)
	{
		if(((TList *)i->data)->pPriv->idcrc==idcrc)
		{
			front->plugz->setCurrent(i);
			create98((TList *)i->data);
			front->createInterface();
			return true;
		}
		i=(Aitem *)i->next;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool A98::savePreset(class Afile *f)
{
	return save(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool A98::loadPreset(class Afile *f)
{
	return load(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void A98::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void A98::fft(byte *sample)
{
	static unsigned char	desemb[256]	={
			 0 ,  128 ,  64 , 192 ,  32 , 160 ,  96 , 224 ,
             16 , 144 ,  80 , 208 ,  48 , 176 , 112 , 240 ,
             16 , 136 ,  72 , 200 ,  40 , 168 , 104 , 232 ,
             24 , 152 ,  88 , 216 ,  56 , 184 , 120 , 248 ,
             32 , 132 ,  68 , 196 ,  36 , 164 , 100 , 228 ,
             40 , 148 ,  84 , 212 ,  52 , 180 , 116 , 244 ,
             48 , 140 ,  76 , 204 ,  52 , 172 , 108 , 236 ,
             56 , 156 ,  92 , 220 ,  60 , 188 , 124 , 252 ,
             64 , 130 ,  66 , 194 ,  68 , 162 ,  98 , 226 ,
             72 , 146 ,  82 , 210 ,  76 , 178 , 114 , 242 ,
             80 , 138 ,  82 , 202 ,  84 , 170 , 106 , 234 ,
             88 , 154 ,  90 , 218 ,  92 , 186 , 122 , 250 ,
             96 , 134 ,  98 , 198 , 100 , 166 , 102 , 230 ,
            104 , 150 , 106 , 214 , 108 , 182 , 118 , 246 ,
            112 , 142 , 114 , 206 , 116 , 174 , 118 , 238 ,
            120 , 158 , 122 , 222 , 124 , 190 , 126 , 254 ,

            128 , 129 , 130 , 193 , 132 , 161 , 134 , 225 ,
            136 , 145 , 138 , 209 , 140 , 177 , 142 , 241 ,
            144 , 145 , 146 , 201 , 148 , 169 , 150 , 233 ,
            152 , 153 , 154 , 217 , 156 , 185 , 158 , 249 ,
            160 , 161 , 162 , 197 , 164 , 165 , 166 , 229 ,
            168 , 169 , 170 , 213 , 172 , 181 , 174 , 245 ,
            176 , 177 , 178 , 205 , 180 , 181 , 182 , 237 ,
            184 , 185 , 186 , 221 , 188 , 189 , 190 , 253 ,
            192 , 193 , 194 , 195 , 196 , 197 , 198 , 227 ,
            200 , 201 , 202 , 211 , 204 , 205 , 206 , 243 ,

            208 , 209 , 210 , 211 , 212 , 213 , 214 , 235 ,
            216 , 217 , 218 , 219 , 220 , 221 , 222 , 251 ,
            224 , 225 , 226 , 227 , 228 , 229 , 230 , 231 ,
            232 , 233 , 234 , 235 , 236 , 237 , 238 , 247 ,
            240 , 241 , 242 , 243 , 244 , 245 , 246 , 247 ,
            248 , 249 , 250 , 251 , 252 , 253 , 254 , 255 };

	static unsigned short	cos256[256]={
			0x00200,0x001FF,0x001FF,0x001FE,0x001FD,0x001FC,0x001FA,0x001F8,
            0x001F6,0x001F3,0x001F0,0x001ED,0x001E9,0x001E6,0x001E2,0x001DD,
            0x001D9,0x001D4,0x001CE,0x001C9,0x001C3,0x001BD,0x001B7,0x001B0,
            0x001A9,0x001A2,0x0019B,0x00193,0x0018B,0x00183,0x0017B,0x00172,
            0x0016A,0x00161,0x00157,0x0014E,0x00144,0x0013A,0x00130,0x00126,
            0x0011C,0x00111,0x00107,0x000FC,0x000F1,0x000E6,0x000DA,0x000CF,
            0x000C3,0x000B8,0x000AC,0x000A0,0x00094,0x00088,0x0007C,0x00070,
            0x00063,0x00057,0x0004B,0x0003E,0x00032,0x00025,0x00019,0x0000C,
            0x00000,0x0FFF4,0x0FFE7,0x0FFDB,0x0FFCE,0x0FFC2,0x0FFB5,0x0FFA9,
            0x0FF9D,0x0FF90,0x0FF84,0x0FF78,0x0FF6C,0x0FF60,0x0FF54,0x0FF48,
            0x0FF3D,0x0FF31,0x0FF26,0x0FF1A,0x0FF0F,0x0FF04,0x0FEF9,0x0FEEF,
            0x0FEE4,0x0FEDA,0x0FED0,0x0FEC6,0x0FEBC,0x0FEB2,0x0FEA9,0x0FE9F,
            0x0FE96,0x0FE8E,0x0FE85,0x0FE7D,0x0FE75,0x0FE6D,0x0FE65,0x0FE5E,
            0x0FE57,0x0FE50,0x0FE49,0x0FE43,0x0FE3D,0x0FE37,0x0FE32,0x0FE2C,
            0x0FE27,0x0FE23,0x0FE1E,0x0FE1A,0x0FE17,0x0FE13,0x0FE10,0x0FE0D,
            0x0FE0A,0x0FE08,0x0FE06,0x0FE04,0x0FE03,0x0FE02,0x0FE01,0x0FE01,
            0x0FE00,0x0FE01,0x0FE01,0x0FE02,0x0FE03,0x0FE04,0x0FE06,0x0FE08,
            0x0FE0A,0x0FE0D,0x0FE10,0x0FE13,0x0FE17,0x0FE1A,0x0FE1E,0x0FE23,
            0x0FE27,0x0FE2C,0x0FE32,0x0FE37,0x0FE3D,0x0FE43,0x0FE49,0x0FE50,
            0x0FE57,0x0FE5E,0x0FE65,0x0FE6D,0x0FE75,0x0FE7D,0x0FE85,0x0FE8E,
            0x0FE96,0x0FE9F,0x0FEA9,0x0FEB2,0x0FEBC,0x0FEC6,0x0FED0,0x0FEDA,
            0x0FEE4,0x0FEEF,0x0FEF9,0x0FF04,0x0FF0F,0x0FF1A,0x0FF26,0x0FF31,
            0x0FF3D,0x0FF48,0x0FF54,0x0FF60,0x0FF6C,0x0FF78,0x0FF84,0x0FF90,
            0x0FF9D,0x0FFA9,0x0FFB5,0x0FFC2,0x0FFCE,0x0FFDB,0x0FFE7,0x0FFF4,
            0x00000,0x0000C,0x00019,0x00025,0x00032,0x0003E,0x0004B,0x00057,
            0x00063,0x00070,0x0007C,0x00088,0x00094,0x000A0,0x000AC,0x000B8,
            0x000C3,0x000CF,0x000DA,0x000E6,0x000F1,0x000FC,0x00107,0x00111,
            0x0011C,0x00126,0x00130,0x0013A,0x00144,0x0014E,0x00157,0x00161,
            0x0016A,0x00172,0x0017B,0x00183,0x0018B,0x00193,0x0019B,0x001A2,
            0x001A9,0x001B0,0x001B7,0x001BD,0x001C3,0x001C9,0x001CE,0x001D4,
            0x001D9,0x001DD,0x001E2,0x001E6,0x001E9,0x001ED,0x001F0,0x001F3,
            0x001F6,0x001F8,0x001FA,0x001FC,0x001FD,0x001FE,0x001FF,0x001FF };
	
	static unsigned short	sin256[256]={
			0x00000,0x0000C,0x00019,0x00025,0x00032,0x0003E,0x0004B,0x00057,
            0x00063,0x00070,0x0007C,0x00088,0x00094,0x000A0,0x000AC,0x000B8,
            0x000C3,0x000CF,0x000DA,0x000E6,0x000F1,0x000FC,0x00107,0x00111,
            0x0011C,0x00126,0x00130,0x0013A,0x00144,0x0014E,0x00157,0x00161,
            0x0016A,0x00172,0x0017B,0x00183,0x0018B,0x00193,0x0019B,0x001A2,
            0x001A9,0x001B0,0x001B7,0x001BD,0x001C3,0x001C9,0x001CE,0x001D4,
            0x001D9,0x001DD,0x001E2,0x001E6,0x001E9,0x001ED,0x001F0,0x001F3,
            0x001F6,0x001F8,0x001FA,0x001FC,0x001FD,0x001FE,0x001FF,0x001FF,
            0x00200,0x001FF,0x001FF,0x001FE,0x001FD,0x001FC,0x001FA,0x001F8,
            0x001F6,0x001F3,0x001F0,0x001ED,0x001E9,0x001E6,0x001E2,0x001DD,
            0x001D9,0x001D4,0x001CE,0x001C9,0x001C3,0x001BD,0x001B7,0x001B0,
            0x001A9,0x001A2,0x0019B,0x00193,0x0018B,0x00183,0x0017B,0x00172,
            0x0016A,0x00161,0x00157,0x0014E,0x00144,0x0013A,0x00130,0x00126,
            0x0011C,0x00111,0x00107,0x000FC,0x000F1,0x000E6,0x000DA,0x000CF,
            0x000C3,0x000B8,0x000AC,0x000A0,0x00094,0x00088,0x0007C,0x00070,
            0x00063,0x00057,0x0004B,0x0003E,0x00032,0x00025,0x00019,0x0000C,
            0x00000,0x0FFF4,0x0FFE7,0x0FFDB,0x0FFCE,0x0FFC2,0x0FFB5,0x0FFA9,
            0x0FF9D,0x0FF90,0x0FF84,0x0FF78,0x0FF6C,0x0FF60,0x0FF54,0x0FF48,
            0x0FF3D,0x0FF31,0x0FF26,0x0FF1A,0x0FF0F,0x0FF04,0x0FEF9,0x0FEEF,
            0x0FEE4,0x0FEDA,0x0FED0,0x0FEC6,0x0FEBC,0x0FEB2,0x0FEA9,0x0FE9F,
            0x0FE96,0x0FE8E,0x0FE85,0x0FE7D,0x0FE75,0x0FE6D,0x0FE65,0x0FE5E,
            0x0FE57,0x0FE50,0x0FE49,0x0FE43,0x0FE3D,0x0FE37,0x0FE32,0x0FE2C,
            0x0FE27,0x0FE23,0x0FE1E,0x0FE1A,0x0FE17,0x0FE13,0x0FE10,0x0FE0D,
            0x0FE0A,0x0FE08,0x0FE06,0x0FE04,0x0FE03,0x0FE02,0x0FE01,0x0FE01,
            0x0FE00,0x0FE01,0x0FE01,0x0FE02,0x0FE03,0x0FE04,0x0FE06,0x0FE08,
            0x0FE0A,0x0FE0D,0x0FE10,0x0FE13,0x0FE17,0x0FE1A,0x0FE1E,0x0FE23,
            0x0FE27,0x0FE2C,0x0FE32,0x0FE37,0x0FE3D,0x0FE43,0x0FE49,0x0FE50,
            0x0FE57,0x0FE5E,0x0FE65,0x0FE6D,0x0FE75,0x0FE7D,0x0FE85,0x0FE8E,
            0x0FE96,0x0FE9F,0x0FEA9,0x0FEB2,0x0FEBC,0x0FEC6,0x0FED0,0x0FEDA,
            0x0FEE4,0x0FEEF,0x0FEF9,0x0FF04,0x0FF0F,0x0FF1A,0x0FF26,0x0FF31,
            0x0FF3D,0x0FF48,0x0FF54,0x0FF60,0x0FF6C,0x0FF78,0x0FF84,0x0FF90,
            0x0FF9D,0x0FFA9,0x0FFB5,0x0FFC2,0x0FFCE,0x0FFDB,0x0FFE7,0x0FFF4 };
	

	static short	samplear[256];			
	static short	sampleai[256];			
	static int		spectretemp[128];
	static short	n,n1,n2,n3,theta,nL3,n13,vr,vi;
	static int		ispectre[128];
	static float	fspectre[128];

	__asm
	{
        pushad
        
        // == recopie le sample en 16 bits dans samplear ==
		lea		edi,samplear[0]
        mov     esi,sample
		xor		eax,eax
		mov		ecx,256
recop_sample:
        movzx   eax,byte ptr[esi]
        inc		esi
		//sub		eax,127				// depend du signe du sample [-127..128] ou [0..255]
		mov     [edi],ax
		add		edi,2
		dec		ecx
		jnz		recop_sample

		push	ebp


		// == met à zero le tableau sampleai		==
		mov		ecx,256/2
		lea		edi,sampleai[0]
		xor		eax,eax
eff_i:
		mov		[edi],eax
		add		edi,4
		dec		ecx
		jnz		eff_i



		// ==  DESEMBROUILLAGE						==
		// == inverse les donnees dans le sample	==
		// == suivant la table de desembrouillage	==

        mov     ebx,offset desemb
		mov		esi,offset samplear
		mov		ecx,256
	remp_samplar:
        movzx   edx,byte ptr [ebx]
		inc		ebx

		add		edx,edx
		add     edx,offset samplear

		mov		ax,[esi]
		mov		di,[edx]
		mov		[esi],di
		mov		[edx],ax
		add		esi,2
		dec		ecx
		jnz		remp_samplar


		// FFT

		mov		ax,256
		mov		n,ax
		mov		n2,ax

		mov		ax,1
		mov		n1,ax

		mov		ecx,8

bcl_00:	
		push	ecx
		mov		ax,n2
		sar		ax,1
		mov		n2,ax

		mov		ax,-1
		mov		nL3,ax


		movzx	ecx,word ptr n2

bcl_01: 
		push	ecx

		xor		ax,ax
		mov		theta,ax

		mov		ax,nL3
		mov		n3,ax

		add		ax,n1
		mov		n13,ax

		movzx	ecx,n1

bcl_02:	
		push	ecx

		add		n3,1
		add		n13,1
		mov		bx,n2
		add		theta,bx

		movzx	ebx,theta
        movsx   eax,word ptr cos256[ebx*2]      ; ax = cos
        movsx   ebx,word ptr sin256[ebx*2]      ; bx = sin

		//movsx	eax,ax
		//movsx	ebx,bx

		push	eax
		push	ebx

		movsx	ecx,word ptr n13
		add		ecx,ecx
		add		ecx,offset samplear

		movsx	edi,word ptr n13
		add		edi,edi
		add		edi,offset sampleai

		movsx	esi,word ptr [ecx]
		imul	esi
		xchg	eax,ebx
		movsx	esi,word ptr [edi]
		imul	esi
		add		eax,ebx
		sar		eax,9
		mov		vr,ax

		pop		ebx
		pop		eax

		movsx	esi,word ptr [edi]
		imul	esi
		xchg	eax,ebx
		movsx	esi,word ptr [ecx]
		imul	esi
		sub		ebx,eax
		sar		ebx,9
		mov		vi,bx

		mov		ax,vr
		mov		dx,bx


		movzx	ecx,word ptr n3
		add		ecx,ecx

		lea		edi,samplear[0]
		mov		bx,[edi+ecx]
		add		[edi+ecx],ax
		lea		edi,sampleai[0]
		mov		si,[edi+ecx]
		add		[edi+ecx],dx

		sub		bx,ax
		sub		si,dx

		movzx	ecx,word ptr n13
		add		ecx,ecx

		mov		samplear[ecx],bx
		mov		sampleai[ecx],si

		pop		ecx

		dec		ecx
	jnz	bcl_02	//-----------------------


		mov		ax,[n1]
		add		ax,ax
		add		[nL3],ax

		pop		ecx
		dec		ecx
	jnz	bcl_01	//-----------------------


		mov		ax,[n1]
		add		ax,ax
		mov		[n1],ax

		pop		ecx
		dec		ecx
	jnz	bcl_00	//-----------------------


		// CalCul du module

		lea		edi,samplear[0]
		lea		esi,sampleai[0]
        lea     ebp,ispectre[0]

        mov     ecx,128
	rempl_spectre:
		xor		ebx,ebx
		movsx	eax,word ptr [edi]
		add		edi,2
		imul	eax
		add		ebx,eax
		movsx	eax,word ptr [esi]
		add		esi,2
		imul	eax
		add		ebx,eax

        mov     [ebp],ebx
        add     ebp,4

		dec		ecx
		jnz		rempl_spectre



		pop		ebp	// rapport au push ebp du debut
        popad
	}
	{
		int		i;
		float	*fs=&fspectre[0];
		int		*is=&ispectre[0];
		for(i=0; i<128; i++)
			*(fs++)=(float)(sqrt((float)*(is++))+128.f+(float)i)/128.f;
	}
	{
		int	i;
		int		*pis=ispectre;
		float	*pfs=fspectre;
		word	*sp16=&param.spectre16[0];
		byte	*sp=&param.spectre[0];
		for(i=0; i<128; i++)
		{
			*(sp++)=(int)*(pfs++);
			*(sp16++)=*(pis++);
		}
	}
}

void A98::action(double time, double dtime, double beat, double dbeat)
{
	A98Front	*front=(A98Front *)this->front;
	A98Back		*back=(A98Back *)this->back;
	Avideo		*out=back->out;
	Asample		*in=back->in;
	int			i;

	if(space)
	{
		param.spacebar=true;
		space=false;
	}

	for(i=0; i<4; i++)
	{
		float	x,y,m=1.f+front->mulzone[i]->get()*1000.f;
		front->zone[i]->get(&x, &y);
		int		rx=(int)((x-0.5f)*m);
		int		ry=(int)((y-0.5f)*m);
		param.moux[i]=rx*10;
		param.mouy[i]=ry*10;
		accx[i]=(accx[i]*9+relx[i])/10;
		accy[i]=(accy[i]*9+rely[i])/10;
		param.amoux[i]+=accx[i];
		param.amouy[i]+=accy[i];
		relx[i]=(255*relx[i])>>8;
		rely[i]=(255*rely[i])>>8;
		relx[i]=(relx[i]*15+rx)/16;
		rely[i]=(rely[i]*15+ry)/16;
	}
	
	section98.enter(__FILE__,__LINE__);
	if(ef98)
	{
		in->enter(__FILE__,__LINE__);
		{
			sword	wave[256*2];
			int		a=in->avaible();
			int		i,s;
			in->skip(maxi(a-256, 0));
			s=in->getBuffer(wave, NULL, NULL, NULL, 256);
			for(i=0; i<s; i++)
			{
				int	n=i<<1;
				int	v=((int)wave[n]+(int)wave[n+1])>>1;
				param.sample16[i]=(word)(sword)v;
				param.sample[i]=(byte)(sbyte)((v>>8)-128);
			}
			fft(param.sample);
		}
		in->leave();

		ef98->pInfo->Action(vars, &param);

		switch(ef98->pInfo->nTypeEcran)
		{
			case AET_ECRAN_OLD:
			{
				int		nb=320*200;
				int		i;
				dword	pal[256];
				dword	*d=image->body32;
				byte	*s=param.ecran;
				TRGBA	*pp=(TRGBA *)(void *)param.palette;
				for(i=0; i<256; i++)
				{
					pal[i]=color32(pp->r<<2, pp->g<<2, pp->b<<2);
					pp++;
				}

				for(i=0; i<nb; i++)
					*(d++)=pal[*(s++)];
			}
			break;

			case AET_ECRAN_OLDP:
			{
				int		nb=320*200;
				int		i;
				dword	*d=image->body32;
				byte	*s=param.ecran;
				dword	*pal=(dword *)(void *)param.palette;
				for(i=0; i<nb; i++)
					*(d++)=pal[*(s++)];
			}
			break;

			break;

			case AET_ECRAN_NEW:
			memcpy(image->body32, param.ecran, 320*200*4);
			break;
		}
	}
	section98.leave();

	out->enter(__FILE__,__LINE__);
	{
		Abitmap *b=out->getBitmap();
		if(b)
		{
			b->set(-1, -1, b->w+2, b->h+2, 0, 0, image->w, image->h, image, bitmapNORMAL, bitmapNORMAL);
			b->fillAlpha(1.f);
		}
	}
	out->leave();

	param.spacebar=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void A98Front::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

A98Front::A98Front(QIID qiid, char *name, A98 *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_FRONT), "PNG");
	back=new Abitmap(&o);

	plugzItems=new Aitem("plugz98", "aestesis'98 plugz");
	{
		TList	*pl=pFirst;
		while(pl)
		{
			new Aitem(pl->pInfo->Name, pl->pInfo->Description, plugzItems, (dword)pl);
			pl=pl->next;
		}
	}
	plugz=new Alist("plugz98", this, pos.w-150, pos.h-40, 140, 20, plugzItems);
	plugz->setTooltips("astesis'98 plugz selector");
	plugz->show(TRUE);

	space=new ActrlButton(MKQIID(qiid, 0x515984f6521695b0), "space", this, 336, 16, 12, 24, &resdll.get(MAKEINTRESOURCE(PNG_SPACE), "PNG"));
	space->setTooltips("send space key press event to effect");
	space->show(TRUE);

	choix[0]=choix[1]=choix[2]=choix[3]=NULL;
	touche[0]=touche[1]=touche[2]=touche[3]=touche[4]=NULL;
	touche[5]=touche[6]=touche[7]=touche[8]=touche[9]=NULL;

	{
		static QIID qiidzone[]={ 0x92dd2b7c241bf45c, 0x41a0c24a10ee5330, 0x69aef6e62cd0a530, 0xb5e655f0723efc60};
		static QIID qiidmzone[]={ 0x1dec79eeedf35608, 0x8032f865a9708340, 0x0cf0044eb5058680, 0x01e024bae34f0c00};
		int	i;
		for(i=0; i<4; i++)
		{
			zone[i]=new Azone98(MKQIID(qiid, qiidzone[i]), "pad pos", this, 268, 16+18*i, 16, 16, 0.3f, 0.3f);
			zone[i]->set(0.5f, 0.5f);
			zone[i]->setTooltips("pad pos");
			//zone[i]->show(TRUE);

			mulzone[i]=new Apaddle(MKQIID(qiid, qiidmzone[i]), "mul pad", this, 292, 18+18*i, 12, 12, &resdll.get(MAKEINTRESOURCE(PNG_PADDLE), "PNG"), 12, 12, paddleY);
			mulzone[i]->setTooltips("mul pad pos");
			mulzone[i]->set(0.f);
			//mulzone[i]->show(TRUE);
			clear[i]=new Abutton("clear[]", this, 312, 18+18*i, 8, 12, &resdll.get(MAKEINTRESOURCE(PNG_CLEAR), "PNG"), Abutton::btBITMAP);
			clear[i]->setTooltips("reset value");
			//clear[i]->show(TRUE);
		}
	}

	createInterface();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

A98Front::~A98Front()
{
	int	i;
	for(i=0; i<4; i++)
	{
		delete(zone[i]);
		delete(mulzone[i]);
		delete(clear[i]);
	}
	for(i=0; i<10; i++)
		delete(touche[i]);
	delete(choix[0]);
	delete(choix[1]);
	delete(choix[2]);
	delete(choix[3]);
	delete(space);
	delete(plugz);
	delete(plugzItems);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void A98Front::createInterface()
{
	A98		*a98=(A98 *)effect;
	Abitmap	*b=NULL;

	a98->section98.enter(__FILE__,__LINE__);
	if(a98->ef98)
	{
		TBitmap *tb=a98->ef98->pPriv->pBmp?a98->ef98->pPriv->pBmp:tbdef;
		b=new Abitmap(tb->pInfo->biWidth, tb->pInfo->biHeight, tb->pInfo->biBitCount);
		memcpy(b->body, tb->pBody, (b->nbBits>>3)*b->w*b->h);
	}
	else
	{
		TBitmap *tb=tbdef;
		b=new Abitmap(tb->pInfo->biWidth, tb->pInfo->biHeight, tb->pInfo->biBitCount);
		memcpy(b->body, tb->pBody, (b->nbBits>>3)*b->w*b->h);
	}
	a98->section98.leave();

	a98->param.choix=1;
	a98->param.choix2=1;
	a98->param.choix3=1;
	a98->param.choix4=1;
	a98->param.touche[0]=false;
	a98->param.touche[1]=false;
	a98->param.touche[2]=false;
	a98->param.touche[3]=false;
	a98->param.touche[4]=false;
	a98->param.touche[5]=false;
	a98->param.touche[6]=false;
	a98->param.touche[7]=false;
	a98->param.touche[8]=false;
	a98->param.touche[9]=false;
	a98->param.moux[0]=0;
	a98->param.mouy[0]=0;
	a98->param.moux[1]=0;
	a98->param.mouy[1]=0;
	a98->param.moux[2]=0;
	a98->param.mouy[2]=0;
	a98->param.moux[3]=0;
	a98->param.mouy[3]=0;
	a98->param.amoux[0]=0;
	a98->param.amouy[0]=0;
	a98->param.amoux[1]=0;
	a98->param.amouy[1]=0;
	a98->param.amoux[2]=0;
	a98->param.amouy[2]=0;
	a98->param.amoux[3]=0;
	a98->param.amouy[3]=0;
	a98->relx[0]=0;
	a98->rely[0]=0;
	a98->relx[1]=0;
	a98->rely[1]=0;
	a98->relx[2]=0;
	a98->rely[2]=0;
	a98->relx[3]=0;
	a98->rely[3]=0;
	a98->accx[0]=0;
	a98->accy[0]=0;
	a98->accx[1]=0;
	a98->accy[1]=0;
	a98->accx[2]=0;
	a98->accy[2]=0;
	a98->accx[3]=0;
	a98->accy[3]=0;

	memset(a98->relx, 0, sizeof(a98->relx));
	memset(a98->rely, 0, sizeof(a98->rely));
	memset(a98->accx, 0, sizeof(a98->accx));
	memset(a98->accy, 0, sizeof(a98->accy));

	if(b)
	{
		{	// choix
			Abitmap	*ch=new Abitmap(48, 160);
			int		i;
			int		nc[4];

			if(a98->ef98)
			{
				nc[0]=a98->ef98->pInfo->nbchoix;
				nc[1]=a98->ef98->pInfo->nbchoix2;
				nc[2]=a98->ef98->pInfo->nbchoix3;
				nc[3]=a98->ef98->pInfo->nbchoix4;
			}
			else
				nc[0]=nc[1]=nc[2]=nc[3]=0;

			for(i=0; i<4; i++)
			{
				int	x=i<<4;
				int	n;

				for(n=0; n<10; n++)
				{
					int	p=n<<4;
					ch->set(0, p, p, x, 16, 16, b, bitmapNORMAL, bitmapNORMAL);
				}

				{
					int		x,y;
					dword	c=ch->body32[0];
					for(y=0; y<160; y++)
					{
						dword	*s=&ch->body32[ch->adr[y]];
						for(x=0; x<16; x++)
						{
							if(*s==c)
								(*s)=0xffffffff;
							s++;
						}
					}
				}

				ch->set(16, 0, 0, 0, 16, 160, ch, bitmapNORMAL, bitmapNORMAL);
				ch->set(32, 0, 0, 0, 16, 160, ch, bitmapNORMAL, bitmapNORMAL);
				ch->boxfa(0, 0, 15, 159, 0xff414141, 0.7f);
				ch->boxfa(16, 0, 31, 159, 0xffffff00, 0.7f);

				if(choix[i]==NULL)
				{
					static QIID qiidch[]={ 0xbe942900e7c95180, 0x77e2e8e18190c650, 0x07e86ac555720a56, 0xf14bd4f33725b740};
					choix[i]=new Aselect(MKQIID(effect->getQIID(), qiidch[i]), "choix", this, 16, 16+i*18, 10, 1, ch, 16, 16);
					choix[i]->setTooltips(name);
					choix[i]->set(0);
				}
				if(nc[i]>1)
				{
					choix[i]->show(true);
					choix[i]->change(ch);
					choix[i]->change(nc[i], 1);
					choix[i]->set(0);
				}
				else
					choix[i]->show(false);
			}
			delete(ch);
		}
		{	// touche
			Abitmap	*tt=new Abitmap(48, 32, 32);
			int		i;
			for(i=1; i<10; i++)
			{
				int px=((i-1)%3);
				int py=(i-1)/3;
				tt->set(0, 0, 416+px*16, 32-py*16, 16, 16, b, bitmapNORMAL, bitmapNORMAL);
				{
					int		x,y;
					dword	c=tt->body32[0];
					for(y=0; y<16; y++)
					{
						dword	*s=&tt->body32[tt->adr[y]];
						for(x=0; x<16; x++)
						{
							if(*s==c)
								(*s)=0xffffffff;
							s++;
						}
					}
				}
				tt->set(0, 16, 0, 0, 16, 16, tt, bitmapNORMAL, bitmapNORMAL);
				tt->set(16, 0, 0, 0, 16, 16, tt, bitmapNORMAL, bitmapNORMAL);
				tt->set(16, 16, 0, 0, 16, 16, tt, bitmapNORMAL, bitmapNORMAL);
				tt->set(32, 0, 0, 0, 16, 16, tt, bitmapNORMAL, bitmapNORMAL);
				tt->set(32, 16, 0, 0, 16, 16, tt, bitmapNORMAL, bitmapNORMAL);

				tt->boxfa(0, 0, 15, 15, 0xff414141, 0.7f);
				tt->boxfa(32, 16, 47, 31, 0xff414141, 0.7f);
				tt->boxfa(16, 0, 31, 15, 0xffffff00, 0.7f);
				tt->boxfa(0, 16, 31, 31, 0xffffffff, 0.5f);
				//tt->boxfa(32, 0, 47, 15, 0xffffffff, 0.5f);

				static QIID qiidtc[]={	0xee444f193d9bf206, 0x68cfdfc1f068f088, 0xd80450dca3b3caf0,
										0x08a566cf2e5cae00, 0xe3c7ea2206d95c00, 0x3ef722f6b45d8000,
										0x7621817b90322770, 0x3d5171897c17abb0, 0x5e9a462080278b40,
										0x2cd7837ff7d0dbe4, 0x5bb95bb619c8b644, 0xcf9f5006a600d480};
				if(!touche[i])
					touche[i]=new ActrlButton(MKQIID(effect->getQIID(), qiidtc[i]), "touche", this, 202+px*18, (16+36)-py*18, 16, 16, tt, Abutton::btBITMAP|Abutton::bt2STATES);

				if(a98->ef98)
				{
					a98->param.touche[i]=false;
					touche[i]->set(false);
					touche[i]->change(tt);
					touche[i]->show(a98->ef98->pInfo->masktouche[i]);
					switch(i)
					{
						case 1:
						touche[i]->setTooltips(a98->ef98->pTooltips->t1);
						break;
						case 2:
						touche[i]->setTooltips(a98->ef98->pTooltips->t2);
						break;
						case 3:
						touche[i]->setTooltips(a98->ef98->pTooltips->t3);
						break;
						case 4:
						touche[i]->setTooltips(a98->ef98->pTooltips->t4);
						break;
						case 5:
						touche[i]->setTooltips(a98->ef98->pTooltips->t5);
						break;
						case 6:
						touche[i]->setTooltips(a98->ef98->pTooltips->t6);
						break;
						case 7:
						touche[i]->setTooltips(a98->ef98->pTooltips->t7);
						break;
						case 8:
						touche[i]->setTooltips(a98->ef98->pTooltips->t8);
						break;
						case 9:
						touche[i]->setTooltips(a98->ef98->pTooltips->t9);
						break;
					}
				}
				else
					touche[i]->show(false);
			}
		}
		{	// touche[0]
			Abitmap	*tt=new Abitmap(48*3, 32, 32);
			tt->set(0, 0, 416, 48, 48, 16, b, bitmapNORMAL, bitmapNORMAL);
			{
				int		x,y;
				dword	c=tt->body32[0];
				for(y=0; y<16; y++)
				{
					dword	*s=&tt->body32[tt->adr[y]];
					for(x=0; x<48; x++)
					{
						if(*s==c)
							(*s)=0xffffffff;
						s++;
					}
				}
			}
			tt->set(0, 16, 0, 0, 48, 16, tt, bitmapNORMAL, bitmapNORMAL);
			tt->set(48, 0, 0, 0, 48, 16, tt, bitmapNORMAL, bitmapNORMAL);
			tt->set(48, 16, 0, 0, 48, 16, tt, bitmapNORMAL, bitmapNORMAL);
			tt->set(96, 0, 0, 0, 48, 16, tt, bitmapNORMAL, bitmapNORMAL);
			tt->set(96, 16, 0, 0, 48, 16, tt, bitmapNORMAL, bitmapNORMAL);

			tt->boxfa(0, 0, 47, 15, 0xff414141, 0.7f);
			tt->boxfa(96, 16, 143, 31, 0xff414141, 0.7f);
			tt->boxfa(48, 0, 95, 15, 0xffffff00, 0.7f);
			tt->boxfa(0, 16, 95, 31, 0xffffffff, 0.5f);
			//tt->boxfa(32, 0, 47, 15, 0xffffffff, 0.5f);

			if(!touche[0])
				touche[0]=new ActrlButton(MKQIID(effect->getQIID(), 0xfd987cb1f84b5268), "touche", this, 204, (16+36+18), 48, 16, tt, Abutton::btBITMAP|Abutton::bt2STATES);

			if(a98->ef98)
			{
				a98->param.touche[0]=false;
				touche[0]->set(false);
				touche[0]->change(tt);
				touche[0]->show(a98->ef98->pInfo->masktouche[0]);
				touche[0]->setTooltips(a98->ef98->pTooltips->t0);
			}
			else
				touche[0]->show(false);
		}
		delete(b);
	}
	if(a98->ef98)
	{
		int	i;
		for(i=0; i<4; i++)
		{
			bool	b=a98->ef98->pInfo->maskvball[i];
			zone[i]->show(b);
			zone[i]->set(0.5f, 0.5f);
			mulzone[i]->show(b);
			mulzone[i]->set(0.f);
			clear[i]->show(b);
		}
		zone[0]->setTooltips(a98->ef98->pTooltips->vb1);
		mulzone[0]->setTooltips(a98->ef98->pTooltips->vb1);
		zone[1]->setTooltips(a98->ef98->pTooltips->vb2);
		mulzone[1]->setTooltips(a98->ef98->pTooltips->vb2);
		zone[2]->setTooltips(a98->ef98->pTooltips->vb3);
		mulzone[2]->setTooltips(a98->ef98->pTooltips->vb3);
		zone[3]->setTooltips(a98->ef98->pTooltips->vb4);
		mulzone[3]->setTooltips(a98->ef98->pTooltips->vb4);
	}
	else
	{
		int	i;
		for(i=0; i<4; i++)
		{
			zone[i]->show(false);
			mulzone[i]->show(false);
		}
	}
	repaint();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool A98Front::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyPRESS:
		if(o==space)
		{
			A98	*a98=(A98 *)effect;
			a98->space=true;
		}
		//else
		break;

		case nyCLICK:
		{
			A98	*a98=(A98 *)effect;
			int	i;
			for(i=0; i<4; i++)
			{
				if(o==clear[i])
				{
					zone[i]->set(0.5f, 0.5f);
					mulzone[i]->set(0.f);
					a98->param.moux[i]=0;
					a98->param.mouy[i]=0;
					a98->param.amoux[i]=0;
					a98->param.amouy[i]=0;
					a98->accx[i]=0;
					a98->accy[i]=0;
					a98->relx[i]=0;
					a98->rely[i]=0;
					repaint();
					return true;
				}
			}
		}
		break;

		case nyCHANGE:
		{
			A98	*a98=(A98 *)effect;
			if(o==plugz)
			{
				((A98 *)effect)->create98((TList *)plugz->getCurrent()->data);
				createInterface();
				return true;
			}
			else if(o==choix[0])
			{
				a98->param.choix=choix[0]->get()+1;
				return true;
			}
			else if(o==choix[1])
			{
				a98->param.choix2=choix[1]->get()+1;
				return true;
			}
			else if(o==choix[2])
			{
				a98->param.choix3=choix[2]->get()+1;
				return true;
			}
			else if(o==choix[3])
			{
				a98->param.choix4=choix[3]->get()+1;
				return true;
			}
			else
			{
				int	i;
				for(i=0; i<10; i++)
				{
					if(o==touche[i])
					{
						a98->param.touche[i]=touche[i]->get();
						return true;
					}
				}
			}
		}
		return true;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool A98Front::dragdrop(int x, int y, int state, int event, void *data)
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
			A98		*a98=(A98 *)effect;
			char	*s=(char *)data;
			while(*s)
			{
				a98->section98.enter(__FILE__,__LINE__);
				if(a98->ef98&&a98->ef98->pInfo->Load)
					a98->ef98->pInfo->Load(s, a98->vars, &a98->param);
				a98->section98.leave();
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

A98Back::A98Back(QIID qiid, char *name, A98 *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_BACK), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0x51efc2973a6260d0), "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);

	in=new Asample(MKQIID(qiid, 0xcdfb1bb8e31c1d96), "audio input", this, pinIN, 10, 10);
	in->setTooltips("audio input");
	in->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

A98Back::~A98Back()
{
	delete(in);
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void A98Back::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * A98Info::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new A98(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	DLLEXPORT class Aplugz * getPlugz()
	{
		return new A98Info("98", &A98::CI, "98 wrapper", "aestesis'98 wrapper module");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
