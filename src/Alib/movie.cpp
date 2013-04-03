/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MOVIE.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<dshow.h>
#include						<uuids.h>		// ID Com Object
#include						<amstream.h>	// DirectShow multimedia stream interfaces
#include						<VFW.H>			// Video For Windows
#include						<ddraw.h>	    // DirectDrawEx interfaces
#include						<assert.h>
//#include						<avifmt.h>

#include						"defSampleGrabber.h"
#include						"filehd.h"
#include						"thread.h"
#include						"movie.h"
#include						"avi.h"
#include						"wflilib.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Amovie::CI=ACI("Amovie", GUID(0xAE57E515,0x00000130), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MILLISECONDS					((STREAM_TIME)10000)
#define							DEFAULTFTIME					((STREAM_TIME)MILLISECONDS*1000/12)

#define SAFE_AMSTREAM(X)		{if (pAMStream->X != S_OK) { pAMStream->Release(); return false; } }
#define RELEASE(X)				{if ((X) != NULL) (X)->Release(); (X) = NULL;}
#define RELEASEREF(X, Y)		{if ((X) != NULL) {HRESULT hr; do{ hr = (X)->Release(); }while(hr>(Y) /*&&hr<10*/); (X) = NULL; }}
#define RELEASEFALSE(X)			{if ((X) != NULL) (X)->Release(); (X) = NULL; return false; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int nTmpFile = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AinfoMovie
{
public:
    IAMMultiMediaStream			*m_pMMStream;
	IMediaStream				*m_pPrimaryVidStream;
	IDirectDrawMediaStream		*m_pDDStream;
	IDirectDrawSurface			*m_pDDSurfaceAnim;
	IDirectDrawStreamSample		*m_pSample;
	STREAM_TIME					fTime;
	STREAM_TIME					duration;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class bitmapL
{
public:

	class bitmapL				*previous; 
	class bitmapL				*next;
	
	union
	{
		Abitmap					*b;
		void					*data;
	};
	int							size;

								bitmapL(Abitmap *b, bitmapL *previous, bitmapL *next, float	quality)
								{
									this->previous=previous;
									this->next=next;
									data=null;
									size=0;
									if(quality==0.f)
										this->b=new Abitmap(*b);
									else
									{
										dword		buffer[MAXMOVIEWIDTH*MAXMOVIEHEIGHT];
										Afilemem	*fm=new Afilemem("jpg file", buffer, sizeof(buffer));
										b->save(fm, bitmapJPG, (int)(quality*100.f));
										int			size=(int)fm->getSize();
										if(size<sizeof(buffer))
										{
											data=malloc(size);
											this->size=size;
											memcpy(data, buffer, size);
										}
										delete(fm);
									}
								}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static IDirectDraw				*pDD=NULL;
static int						nMovies=0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void a2bstr(WCHAR *dest, char *src)
{
	while(*src)
		*(dest++)=*(src++);
	*dest=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void WINAPI FreeMediaType(AM_MEDIA_TYPE mt)
{
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);

        // Strictly unnecessary but tidier
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL) {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void WINAPI FreeMediaTypePtr(AM_MEDIA_TYPE *pmt)
{
    if (pmt->cbFormat != 0) {
        CoTaskMemFree((PVOID)pmt->pbFormat);

        // Strictly unnecessary but tidier
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL) {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }
    free(pmt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::getFileInfo(AmovieFileInfo *mfinfo, char * filename)
{
	bool	bok=false;
	char	fname[ALIB_PATH];
	
	memset(mfinfo, 0, sizeof(AmovieFileInfo));
	
	strcpy(fname, filename);
	strupr(fname);
	
	if(strstr(fname, ".MPG")||strstr(fname, ".MPEG"))
	{
		strcpy(mfinfo->codec, "MPEG");
		bok=true;
	}
	else if(strstr(fname, ".AVI"))
	{
		Afilehd	*f=new Afilehd(filename, Afilehd::READ);
		Tchunk	chunk[100];
		int		level=0;
		
		memset(chunk, 0, sizeof(chunk));
		
		if(f->isOK())
		{
			while(f->read(&chunk[level].magic, sizeof(dword)))
			{
				if(f->read(&chunk[level].size, sizeof(dword)))
				{
					chunk[level].start=f->offset;
					chunk[level].next=f->offset+chunk[level].size;
					chunk[level].type=0;
					switch(chunk[level].magic)
					{
						case MAKEFOURCC('R','I','F','F'):
						if(f->read(&chunk[level].type, sizeof(dword)))
						{
							switch(chunk[level].type)
							{
								case MAKEFOURCC('A','V','I',' '):
								level++;
								break;
								
								default:
								f->seek(chunk[level].next);
								break;
							}
						}
						else goto fileError;
						break;
						
						case MAKEFOURCC('L','I','S','T'):
						if(f->read(&chunk[level].type, sizeof(dword)))
						{
							switch(chunk[level].type)
							{
								case MAKEFOURCC('h','d','r','l'):
								level++;
								break;
								
								case MAKEFOURCC('s','t','r','l'):
								level++;
								break;
					
								default:
								f->seek(chunk[level].next);
								break;
							}
						}
						else goto fileError;
						break;
						
						case MAKEFOURCC('a','v','i','h'):
						if(chunk[level].size>=sizeof(MainAVIHeader))
						{
							MainAVIHeader	head;
							if(f->read(&head, sizeof(head)))
							{
								mfinfo->width=head.dwWidth;
								mfinfo->height=head.dwWidth;
								mfinfo->fps=1000000.f/(float)head.dwMicroSecPerFrame;
							}
							else goto fileError;
						}
						else if(!f->seek(chunk[level].next))
							goto fileError;
						break;
						
						case MAKEFOURCC('s','t','r','h'):
						if(chunk[level].size>=sizeof(myAVIStreamHeader))
						{
							myAVIStreamHeader	head;
							if(f->read(&head, sizeof(head)))
							{
								switch(head.fccType)
								{
									case MAKEFOURCC('v','i','d','s'):	// video
									mfinfo->codec4cc=head.fccHandler;
									Conv4cc(mfinfo->codec, head.fccHandler);
									bok=true;
									goto fileOK;
									break;
									
									case MAKEFOURCC('a','u','d','s'):	// audio
									break;
									
									case MAKEFOURCC('t','x','t','s'):	// text
									break;
									
									case MAKEFOURCC('m','i','d','s'):	// midi
									break;
								}
							}
							else goto fileError;
						}
						else if(!f->seek(chunk[level].next))
							goto fileError;
						break;
						
						default:
						if(!f->seek(chunk[level].next))
							goto fileError;
						break;
					}
				}
				else goto fileError;
				
				if(chunk[level].magic)	// not level++
				{
					if(f->offset!=chunk[level].next)	// skip rest chunk's data
					{
						//assert(false);
						if(!f->seek(chunk[level].next))
							goto fileError;
					}
						
					while((level>0)&&(chunk[level].next==chunk[level-1].next))
					{
						memset(&chunk[level], 0, sizeof(Tchunk));
						level--;
					}
				}
			}
		}
	fileError:
	fileOK:
		delete(f);
	}
	return bok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap * Amovie::getSnapshot(char *filename, float *fps)
{
	Abitmap		*bitmap=NULL;
	IMediaDet	*pMD=NULL;
	byte		*bits=NULL;
	WCHAR		wname[1024];

	*fps=0.f;

	if(CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, (void**)&pMD)!=S_OK)
		return NULL;

	if(pMD==NULL)
		return NULL;
	
	a2bstr(wname, filename);

	if(pMD->put_Filename((BSTR)wname)==S_OK)
	{
		long	nstreams=0;
		if(pMD->get_OutputStreams(&nstreams)==S_OK)
		{
			bool	bFound=false;
			long	i;
			
			for(i=0; i<nstreams; i++)
			{
				GUID major_type;
				pMD->put_CurrentStream(i);
				pMD->get_StreamType(&major_type);
				if(major_type==MEDIATYPE_Video)
				{
					bFound=true;
					break;
				}
			}
			
			if(bFound)
			{
				AM_MEDIA_TYPE	*pmt=(AM_MEDIA_TYPE *)malloc(sizeof(AM_MEDIA_TYPE));
				long			size;
				if(pmt)
				{
					memset(pmt, 0, sizeof(AM_MEDIA_TYPE));
					
					if(pMD->get_StreamMediaType(pmt)==S_OK)
					{
						if((pmt->formattype==FORMAT_VideoInfo)&&(pmt->pbFormat)&&(pmt->cbFormat>=sizeof(VIDEOINFOHEADER)))
						{
							VIDEOINFOHEADER *pVih=reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
							long			width=pVih->bmiHeader.biWidth;
							long			height=pVih->bmiHeader.biHeight;
							double			dfps;

							if(height<0)
								height*=-1;
								
							if(pMD->get_FrameRate(&dfps)==S_OK)
							{
								*fps=(float)dfps;
								bitmap=new Abitmap(width, height);
								
								if(pMD->GetBitmapBits(0, &size, NULL, width, height)==S_OK)
								{
									double	time=0.0;
									int		i;
									bool	bok=false;
									int		nok=0;
									int		ntry=5;
									
									bits=(byte *)malloc(size);		

									while((!bok)&&(ntry--))
									{
										byte	*s=bits;
										dword	*d=bitmap->body32;
										int		n=width*height;
										int		npix=0;
										HRESULT	ret=S_OK;
										
										if((ret=pMD->GetBitmapBits(time, &size, (char *)s, width, height))==S_OK)
										{
											s+=sizeof(BITMAPINFOHEADER);
											for(i=0; i<n; i++)
											{
												byte	b=*(s++);
												byte	g=*(s++);
												byte	r=*(s++);
												dword	v=r+g+b;
												*(d++)=color32(r, g, b);
												if(v>128)
													npix++;
											}
											bok=(npix>(width*height>>4));
											nok++;
										}
										else
										{
											char	*error=null;
											switch(ret)
											{
												case S_OK: error="Success"; break;
												case E_NOINTERFACE: error="Could not add the Sample Grabber filter to the graph"; break;
												case E_OUTOFMEMORY: error="Insufficient memory"; break;
												case E_POINTER: error="NULL pointer error"; break;
												case E_UNEXPECTED: error="Unexpected error"; break;
												case VFW_E_INVALIDMEDIATYPE: error="Invalid media type"; break;
											}
											if(nok)
												break;
											else
											{
												delete(bitmap);
												bitmap=NULL;
												break;
											}
										}
										time++;
									}
									if(bitmap)
										bitmap->flipY();
								}
							}
						}
					}
					FreeMediaTypePtr(pmt);
				}
			}
		}
	}
	RELEASE(pMD);
	if(bits)
		free(bits);
	return bitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amovie::Amovie(char *name, int w, int h, int nbBits) : Anode(name)
{
	jpegQuality=0.f;
	jpeg=null;
	fps=0.f;
	info=new AinfoMovie();
	avi=NULL;
	this->w=w;
	this->h=h;
	this->nbBits=nbBits;
	this->nbFrames=0;
	this->firstB=NULL;
	this->lastB=NULL;
	this->current=NULL;
	this->currentI=-1;
	this->fliflc=NULL;
	defaultFT=false;
	type=typeNORMAL;
	isOK=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amovie::Amovie(char *name, int w, int h, int nbBits, bool fileMemory) : Anode(name)
{
	hFile=NULL;
	bmpFile=NULL;
	jpegQuality=0.f;
	jpeg=null;
	fps=0.f;
	info=new AinfoMovie();
	avi=NULL;
	this->w=w;
	this->h=h;
	this->nbBits=nbBits;
	this->nbFrames=0;
	this->firstB=NULL;
	this->lastB=NULL;
	this->current=NULL;
	this->currentI=-1;
	this->fliflc=NULL;
	defaultFT=false;
	if(fileMemory)
	{
		char path[1024];
		char filename[1024];
		GetTempPath(sizeof(path), path);
		GetTempFileName(path, "rmb", nTmpFile++, filename);
		hFile=CreateFile(filename, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_RANDOM_ACCESS, NULL);
		bmpFile=new Abitmap(w, h, nbBits);
		type=typeFILEMEMORY;
	}
	else
		type=typeNORMAL;
	isOK=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amovie::Amovie(char *name, int w, int h, float quality) : Anode(name)
{
	hFile=NULL;
	bmpFile=NULL;
	jpegQuality=quality;
	jpeg=new Abitmap(w, h);
	fps=0.f;
	info=new AinfoMovie();
	avi=NULL;
	this->w=w;
	this->h=h;
	this->nbBits=32;
	this->nbFrames=0;
	this->firstB=NULL;
	this->lastB=NULL;
	this->current=NULL;
	this->currentI=-1;
	this->fliflc=NULL;
	defaultFT=false;
	type=typeJPEG;
	isOK=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amovie::Amovie(char *name, char *filename) : Anode(name)
{
	hFile=NULL;
	bmpFile=NULL;
	jpegQuality=0.f;
	fps=0.f;
	avi=NULL;

	info=new AinfoMovie();
    info->m_pMMStream=NULL;
	info->m_pPrimaryVidStream=NULL;
	info->m_pDDStream=NULL;
	info->m_pDDSurfaceAnim=NULL;
	info->m_pSample=NULL;

	this->w=0;
	this->h=0;
	this->nbBits=0;
	this->nbFrames=0;
	this->firstB=NULL;
	this->lastB=NULL;
	this->current=NULL;
	this->currentI=-1;
	this->fliflc=NULL;
	this->bfli=NULL;
	defaultFT=false;
	type=-1;
	isOK=true;

	strlwr(filename);
	if(strstr(filename, ".fli")||strstr(filename, ".flc"))
	{
		type=typeFLIFLCLOAD;
		isOK=openFLI(filename);
		if(isOK)
			return;
		else
			closeFLI();
	}
	if(strstr(filename, ".avi"))
	{
		type=typeOWNAVI;
		avi=new AaviRead(filename, AaviRead::VIDEO);
		isOK=avi->isOK;
		if(isOK)
		{
			type=typeOWNAVI;
			w=avi->getWidth();
			h=avi->getHeight();;
			nbBits=avi->getNbBits();
			fps=avi->getFPS();
			nbFrames=avi->nbframes;
			isLast=(avi->frame==(avi->nbframes-1));
			currentI=avi->frame;
			return;
		}
		else
			delete(avi);
		avi=NULL;
	}
	{
		type=typeSTREAMLOAD;
		isOK=open(filename);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Amovie::~Amovie()
{
	switch(type)
	{
		case typeFILEMEMORY:
		CloseHandle(hFile);
		if(bmpFile)
			delete(bmpFile);
		break;

		case typeNORMAL:
		case typeJPEG:
		clear();
		if(jpeg)
			delete(jpeg);
		break;

		case typeSTREAMLOAD:
		close();
		break;

		case typeFLIFLCLOAD:
		closeFLI();
		break;
		
		case typeOWNAVI:
		if(avi)
			delete(avi);
		break;
	}
	delete(info);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void Amovie::clear()
{
	switch(type)
	{
		case typeFILEMEMORY:
		nbFrames=0;
		currentI=-1;
		break;
		case typeNORMAL:
		case typeJPEG:
		while(nbFrames)
			remove();
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amovie::del(bitmapL *bl)
{
	if(bl)
	{
		switch(type)
		{
			case typeNORMAL:
			if(bl->b)
				delete(bl->b);
			break;
			case typeJPEG:
			if(bl->data)
				free(bl->data);
			break;
		}
		if(bl->previous&&bl->next)
		{
			bl->previous->next=bl->next;
			bl->next->previous=bl->previous;
		}
		else if(bl->previous)
		{
			bl->previous->next=NULL;
			lastB=bl->previous;
		}
		else if(bl->next)
		{
			bl->next->previous=NULL;
			firstB=bl->next;
		}
		else
		{
			firstB=lastB=NULL;
		}
		delete(bl);
		nbFrames--;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::remove(int pos)
{
	switch(type)
	{
		case typeNORMAL:
		case typeJPEG:
		switch(pos)
		{
			case FIRST:
			{
				bool b=del(firstB);
				current=firstB;
				currentI=current?0:-1;
				return b;
			}
			break;

			case LAST:
			{
				bool b=del(lastB);
				current=lastB;
				currentI=current?(nbFrames-1):-1;
				return b;
			}
			break;

			case PREVIOUS:
			if(current&&current->previous)
			{
				if(del(current->previous))
				{
					currentI--;
					return true;
				}
			}
			break;

			case NEXT:
			if(current&&current->next)
				return del(current->next);
			break;

			case CURRENT:
			if(current)
			{
				bitmapL	*bb=current->next;
				if(del(current))
				{
					if(bb)
						current=bb;
					else
					{
						current=lastB;
						currentI=current?(nbFrames-1):-1;
					}
					return true;
				}
			}
			break;

			default:
			if((pos>=0)&&(pos<nbFrames))
			{
				bitmapL	*bb=firstB;
				int		n=0;
				while(bb)
				{
					if(n==pos)
					{
						bitmapL	*bn=bb->next;
						if(del(bb))
						{
							if(bn)
								current=bn;
							else
							{
								current=lastB;
								currentI=current?(nbFrames-1):-1;
							}
							return true;
						}
					}
					bb=bb->next;
					n++;
				}
			}
			break;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::insert(Abitmap *b, int pos)
{
	switch(type)
	{
		case typeNORMAL:
		case typeJPEG:
		if(!firstB)
		{
			assert(!firstB&&!lastB&&!nbFrames);
			bitmapL	*bl= new bitmapL(b, NULL, NULL, jpegQuality);
			current=firstB=lastB=bl;
			currentI=0;
			nbFrames=1;
		}
		else
		{
			switch(pos)
			{
				case FIRST:
				{
					bitmapL	*bl= new bitmapL(b, NULL, firstB, jpegQuality);
					firstB->previous=bl;
					firstB=bl;
					current=bl;
					currentI=0;
					nbFrames++;
				}
				return true;

				case LAST:
				if(lastB->previous)
				{
					bitmapL	*bl= new bitmapL(b, lastB->previous, lastB, jpegQuality);
					lastB->previous->next=bl;
					lastB->previous=bl;
					current=bl;
					currentI=(nbFrames++)-1;
				}
				else
				{
					bitmapL	*bl= new bitmapL(b, NULL, lastB, jpegQuality);
					firstB=bl;
					lastB->previous=bl;
					current=bl;
					currentI=0;
					nbFrames++;
				}
				return true;

				case PREVIOUS:
				if(current->previous)
				{
					bitmapL	*bl= new bitmapL(b, current->previous->previous, current->previous, jpegQuality);
					if(current->previous->previous)
						current->previous->previous->next=bl;
					else
						firstB=bl;
					current->previous->previous=bl;
					current=bl;
					currentI--;
				}
				else
				{
					bitmapL	*bl= new bitmapL(b, NULL, current, jpegQuality);
					current->previous=bl;
					firstB=bl;
					current=bl;
					currentI=0;
				}
				nbFrames++;
				return true;

				case NEXT:
				if(current->next)
				{
					bitmapL	*bl= new bitmapL(b, current, current->next, jpegQuality);
					current->next->previous=bl;
					current->next=bl;
					current=bl;
					currentI++;
				}
				else
				{
					bitmapL	*bl= new bitmapL(b, current, NULL, jpegQuality);
					current->next=bl;
					lastB=bl;
					current=bl;
					currentI++;
				}
				nbFrames++;
				return true;

				case CURRENT:
				{
					bitmapL	*bl= new bitmapL(b, current->previous, current, jpegQuality);
					if(current->previous)
						current->previous->next=bl;
					else
						firstB=bl;
					current->previous=bl;
					current=bl;
					nbFrames++;
				}
				return true;

				default:
				if((pos>=0)&&(pos<nbFrames))
				{
					if(pos!=currentI)
						select(pos);
					return insert(b, CURRENT);
				}
				break;
			}
			break;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::append(Abitmap *b, int pos)
{
	switch(type)
	{
		case typeFILEMEMORY:
		{
			DWORD wb;
			WriteFile(hFile, b->body, (w*h*nbBits)>>3, &wb, NULL);
			bmpFile->state=b->state;
			bmpFile->flags=b->flags;
			bmpFile->alpha=b->alpha;
			bmpFile->colorkey=b->colorkey;
			currentI=nbFrames++;
			return true;
		}
		break;

		case typeNORMAL:
		case typeJPEG:
		if(!firstB)
		{
			assert(!firstB&&!lastB&&!nbFrames);
			bitmapL	*bl= new bitmapL(b, NULL, NULL, jpegQuality);
			current=firstB=lastB=bl;
			currentI=0;
			nbFrames=1;
			return true;
		}
		else
		{
			switch(pos)
			{
				case FIRST:
				{
					bitmapL	*bl= new bitmapL(b, firstB, firstB->next, jpegQuality);
					if(firstB->next)
						firstB->next->previous=bl;
					else
						lastB=bl;
					firstB->next=bl;
					current=bl;
					currentI=1;
					nbFrames++;
				}
				return true;

				case LAST:
				{
					bitmapL	*bl= new bitmapL(b, lastB, NULL, jpegQuality);
					lastB->next=bl;
					lastB=bl;
					current=bl;
					currentI=nbFrames++;
				}
				return true;

				case PREVIOUS:
				{
					bitmapL	*bl= new bitmapL(b, current->previous, current, jpegQuality);
					if(current->previous)
						current->previous->next=bl;
					else
						firstB=bl;
					current=bl;
					nbFrames++;
				}
				return true;

				case NEXT:
				if(current->next)
				{
					bitmapL	*bl= new bitmapL(b, current->next, current->next->next, jpegQuality);
					if(current->next->next)
						current->next->next->previous=bl;
					else
						lastB=bl;
					current->next->next=bl;
					current=bl;
					currentI+=2;
				}
				else
				{
					bitmapL	*bl= new bitmapL(b, current, NULL, jpegQuality);
					current->next=bl;
					lastB=bl;
					current=bl;
					currentI++;
				}
				nbFrames++;
				return true;

				case CURRENT:
				{
					bitmapL	*bl= new bitmapL(b, current, current->next, jpegQuality);
					if(current->next)
						current->next->previous=bl;
					else
						lastB=bl;
					current->next=bl;
					current=bl;
					currentI++;
					nbFrames++;
				}
				return true;

				default:
				if((pos>=0)&&(pos<nbFrames))
				{
					if(pos!=currentI)
						select(pos);
					return append(b, CURRENT);
				}
				break;
			}
			break;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::select(int pos)
{
	if(!isOK)
		return false;
	switch(type)
	{
		case typeFILEMEMORY:
		switch(pos)
		{
			case FIRST:
			currentI=0;
			return true;

			case LAST:
			currentI=nbFrames-1;
			return true;

			case NEXT:
			currentI++;
			if(currentI>=nbFrames)
				currentI=0;
			return true;

			case PREVIOUS:
			currentI--;
			if(currentI<0)
				currentI=nbFrames-1;
			if(currentI<0)
				currentI=0;
			return true;

			case CURRENT:
			return true;

			default:
			if((pos>=0)&&(pos<nbFrames))
			{
				currentI=pos;
				return true;
			}
			break;
		}
		break;

		case typeNORMAL:
		case typeJPEG:
		switch(pos)
		{
			case FIRST:
			currentI=0;
			current=firstB;
			return true;

			case LAST:
			current=lastB;
			currentI=nbFrames-1;
			break;

			case NEXT:
			if(current)
			{
				current=current->next;
				currentI++;
				if(!current)
				{
					current=firstB;
					currentI=0;
				}
				return true;
			}
			break;

			case PREVIOUS:
			if(current)
			{
				current=current->previous;
				currentI--;
				if(!current)
				{
					current=lastB;
					currentI=nbFrames-1;
				}
				return true;
			}
			break;

			case CURRENT:
			return true;

			default:
			if((pos>=0)&&(pos<nbFrames))
			{
				if(pos==currentI)
					return true;
				if(pos<=(currentI>>1))
				{
					current=firstB;
					currentI=0;
					while(pos!=currentI)
					{
						current=current->next;
						currentI++;
					}
				}
				else if(pos<currentI)
				{
					while(pos!=currentI)
					{
						current=current->previous;
						currentI--;
					}
				}
				else if(pos<=(currentI+((nbFrames-currentI)>>1)))
				{
					while(pos!=currentI)
					{
						current=current->next;
						currentI++;
					}
				}
				else
				{
					current=lastB;
					currentI=nbFrames-1;
					while(pos!=currentI)
					{
						current=current->previous;
						currentI--;
					}
				}
				return true;
			}
			break;
		}
		break;

		case typeSTREAMLOAD:
		switch(pos)
		{
			case FIRST:
			info->m_pMMStream->Seek(0);
			currentI=0;
			break;

			case LAST:
			currentI=nbFrames-1;
			info->m_pMMStream->Seek(currentI*info->fTime);
			break;

			case NEXT:
			if(isLast)
				currentI=0;
			else
				currentI++;
			info->m_pMMStream->Seek(currentI*info->fTime);
			break;

			case PREVIOUS:
			if(currentI>0)
				currentI--;
			else
				currentI=nbFrames-1;
			info->m_pMMStream->Seek(currentI*info->fTime);
			break;

			case CURRENT:
			return true;

			default:
			if((pos>=0)&&(pos<nbFrames))
			{
				if(pos==currentI)
					return true;
				currentI=pos;
				info->m_pMMStream->Seek(currentI*info->fTime);
			}
			else
				return false;
			break;
		}
		{
			HRESULT hr = info->m_pSample->Update(0, NULL, NULL, 0);
			isLast=false;
			switch(hr)
			{
				case MS_S_ENDOFSTREAM:
				isLast=true;
				return true;

				case S_OK:
				if(currentI>=(nbFrames-1))
					isLast=true;
				return true;

				default:
				return false;
			}
		}
		break;

		case typeFLIFLCLOAD:
		switch(pos)
		{
			case FIRST:
			break;

			case LAST:
			break;

			case NEXT:
			play_frame(fliflc, (char *)bfli->body8, w);
			memcpy(bfli->palette, fliflc->palette, 256*4);
			currentI++;
			if(currentI==(nbFrames-1))
				isLast=true;
			return true;

			case PREVIOUS:
			break;

			case CURRENT:
			return true;

			default:
			break;
		}
		return false;
		
		case typeOWNAVI:
		switch(pos)
		{
			case FIRST:
			currentI=0;
			break;

			case LAST:
			currentI=nbFrames-1;
			break;

			case NEXT:
			if(isLast)
				currentI=0;
			else
				currentI++;
			break;

			case PREVIOUS:
			if(currentI>0)
				currentI--;
			else
				currentI=nbFrames-1;
			break;

			case CURRENT:
			break;

			default:
			if((pos>=0)&&(pos<nbFrames))
			{
				if(pos==currentI)
					return true;
				currentI=pos;
			}
			else
				return false;
			break;
		}
		if(!avi->getFrame(currentI))
			return false;
		isLast=(currentI==(nbFrames-1));
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap * Amovie::get(int pos)
{
	if(!isOK)
		return false;

	switch(type)
	{
		case typeFILEMEMORY:
		{
			bool changed=false;

			if((pos!=currentI)&&(pos!=CURRENT))
				if(!select(pos))
					return false;
				else 
					changed=true;

			if(changed)
			{
				dword size=(w*h*nbBits)>>3;
				DWORD readed;
				SetFilePointer(hFile, size*currentI, 0, FILE_BEGIN);
				ReadFile(hFile, bmpFile->body, size, &readed, NULL);
			}
		}
		return bmpFile;

		case typeNORMAL:
		if((pos!=CURRENT)&&(pos!=currentI))
			if(!select(pos))
				return false;
		return current->b;
		
		case typeJPEG:
		{
			if((pos!=CURRENT)&&(pos!=currentI))
				if(!select(pos))
					return false;
			{
				Afilemem	*fm=new Afilemem("jpg file", current->data, current->size);
				jpeg->load(fm, bitmapJPG);
				delete(fm);
			}
			jpeg->state=bitmapNORMAL;
			jpeg->flags=bitmapNORMAL;
		}
		return jpeg;

		case typeSTREAMLOAD:
		return NULL;
		
		case typeOWNAVI:
		if((pos!=CURRENT)&&(pos!=currentI))
			if(!select(pos))
				return false;
		{
			Abitmap	*bavi=avi->getFrame(currentI);
			return bavi;
		}
		break;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::get(Abitmap *b, int pos)
{
	if((!isOK)||(b==NULL)||(b->w!=w)||(b->h!=h))
		return false;

	switch(type)
	{
		case typeFILEMEMORY:
		{
			if((pos!=currentI)&&(pos!=CURRENT))
				if(!select(pos))
					return false;
			{
				dword size=(w*h*nbBits)>>3;
				DWORD readed;
				SetFilePointer(hFile, size*currentI, 0, FILE_BEGIN);
				ReadFile(hFile, b->body, size, &readed, NULL);
			}

			b->state=bmpFile->state;
			b->flags=bmpFile->flags;
		}
		return true;

		case typeNORMAL:
		if((pos!=CURRENT)&&(pos!=currentI))
			if(!select(pos))
				return false;
		b->set(0, 0, current->b, bitmapNORMAL, 0);
		b->state=current->b->state;
		b->flags=current->b->flags;
		return true;
		
		case typeJPEG:
		{
			if((pos!=CURRENT)&&(pos!=currentI))
				if(!select(pos))
					return false;
			{
				Afilemem	*fm=new Afilemem("jpg file", current->data, current->size);
				b->load(fm, bitmapJPG);
				delete(fm);
			}
			b->state=bitmapNORMAL;
			b->flags=bitmapNORMAL;
		}
		return true;

		case typeSTREAMLOAD:
		if(b->nbBits!=nbBits)
			return false;
		switch(pos)
		{
			case CURRENT:
			break;

			default:
			if(!select(pos))
				return false;
			break;
		}
		{
			DDSURFACEDESC		ddsd;
			memset(&ddsd, 0, sizeof(DDSURFACEDESC));
			ddsd.dwSize=sizeof(DDSURFACEDESC);

			while(info->m_pDDSurfaceAnim->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL)!=DD_OK)
			{
			}

			{
				byte	*src=(byte *)ddsd.lpSurface;
				byte	*dst=(byte *)b->body32;
				int		y;
				for(y=0; y<h; y++)
				{
					memcpy(dst, src, 4*w);
					src+=ddsd.dwLinearSize;
					dst+=4*w;
				}
			}
			
			b->state=bitmapNORMAL;
			b->flags=bitmapNORMAL;

			info->m_pDDSurfaceAnim->Unlock(NULL);
		}
		return true;

		case typeFLIFLCLOAD:
		if((pos!=CURRENT)&&(pos!=currentI))
			if(!select(pos))
				return false;
		b->set(0, 0, bfli, bitmapNORMAL, bitmapNORMAL);
		b->state=bfli->state;
		b->flags=bfli->flags;
		return true;
		
		case typeOWNAVI:
		if((pos!=CURRENT)&&(pos!=currentI))
			if(!select(pos))
				return false;
		{
			Abitmap	*bavi=avi->getFrame(currentI);
			if(bavi)
			{
				b->set(0, 0, bavi, bitmapNORMAL, bitmapNORMAL);
				b->state=bavi->state;
				b->flags=bavi->flags;
			}
			else
				return false;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Amovie::get(class Abitmap *b, int xd, int yd, int wd, int hd, int xs, int ys, int ws, int hs, int state, int flags, float alpha, dword colorkey, int pos)
{
	switch(type)
	{
		case typeFILEMEMORY:
		{
			bool changed=false;

			if((pos!=currentI)&&(pos!=CURRENT))
				if(!select(pos))
					return false;
				else 
					changed=true;

			if(changed)
			{
				dword size=(w*h*nbBits)>>3;
				DWORD readed;
				SetFilePointer(hFile, size*currentI, 0, FILE_BEGIN);
				ReadFile(hFile, bmpFile->body, size, &readed, NULL);
			}

			bmpFile->flags=(bmpFile->flags&bitmapALPHA)?(flags|bitmapALPHA):flags;
			bmpFile->state=state;
			bmpFile->alpha=alpha;
			bmpFile->colorkey=colorkey;

			b->set(xd, yd, wd, hd, xs, ys, ws, hs, bmpFile, bitmapDEFAULT, bitmapDEFAULT);
		}
		return true;

		case typeNORMAL:
		if((pos!=currentI)&&(pos!=CURRENT))
			if(!select(pos))
				return false;
		{
			Abitmap	*bs=current->b;

			dword	mflags=bs->flags;
			dword	mstate=bs->state;
			float	malpha=bs->alpha;
			dword	mcolorkey=bs->colorkey;

			bs->flags=(mflags&bitmapALPHA)?(flags|bitmapALPHA):flags;
			bs->state=state;
			bs->alpha=alpha;
			bs->colorkey=colorkey;

			b->set(xd, yd, wd, hd, xs, ys, ws, hs, current->b, bitmapDEFAULT, bitmapDEFAULT);

			bs->state=mstate;
			bs->flags=mflags;
			bs->alpha=malpha;
			bs->colorkey=mcolorkey;
		}
		return true;
		
		case typeJPEG:
		{
			if((pos!=currentI)&&(pos!=CURRENT))
				if(!select(pos))
					return false;
			{
				{
					Afilemem	*fm=new Afilemem("jpg file", current->data, current->size);
					jpeg->load(fm, bitmapJPG);
					delete(fm);
				}
				jpeg->state=state;
				jpeg->flags=flags;
				jpeg->alpha=alpha;
				b->set(xd, yd, wd, hd, xs, ys, ws, hs, jpeg, bitmapDEFAULT, bitmapDEFAULT);
				
				jpeg->state=bitmapNORMAL;
				jpeg->flags=bitmapNORMAL;
			}
		}
		return true;

		case typeSTREAMLOAD:
		break;

		case typeFLIFLCLOAD:
		{
			Abitmap	*bs=bfli;

			dword	mflags=bs->flags;
			dword	mstate=bs->state;
			float	malpha=bs->alpha;
			dword	mcolorkey=bs->colorkey;

			bs->flags=flags;
			bs->state=state;
			bs->alpha=alpha;
			bs->colorkey=colorkey;

			b->set(xd, yd, wd, hd, xs, ys, ws, hs, current->b, bitmapDEFAULT, bitmapDEFAULT);

			bs->state=mstate;
			bs->flags=mflags;
			bs->alpha=malpha;
			bs->colorkey=mcolorkey;
		}
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float Amovie::getFPS()
{
	return fps;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amovie::open(char *filename)
{
	try
	{
		if(!(nMovies++))
		{
			CoInitialize(NULL);
			if(DirectDrawCreate(NULL, &pDD, NULL)!=DD_OK)
				return false;
			if(!pDD)
				return false;
			pDD->SetCooperativeLevel(GetDesktopWindow(), DDSCL_NORMAL);
		}

		IAMMultiMediaStream		*pAMStream = NULL;
		DDSURFACEDESC			ddsd;
		STREAM_TIME				mediaTime, frameTime;
		WCHAR					wFile[1024];
		HRESULT					hr;

		MultiByteToWideChar(CP_ACP, 0, filename, -1, wFile, sizeof(wFile)/sizeof(wFile[0]));

		hr=CoCreateInstance(CLSID_AMMultiMediaStream, NULL, CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, (void **)&pAMStream);
		switch(hr)
		{
			case REGDB_E_CLASSNOTREG:
			Alasterror="class not registred";
			return false;

			case CLASS_E_NOAGGREGATION:
			Alasterror="class with no aggregation";
			return false;

			case CO_E_NOTINITIALIZED:
			Alasterror="com interface not initialized.";
			return false;

			case S_OK:
			break;

			default:
			Alasterror="unknown error";
			return false;
		}

		SAFE_AMSTREAM(Initialize(STREAMTYPE_READ, 0, NULL));
		SAFE_AMSTREAM(AddMediaStream(pDD, &MSPID_PrimaryVideo, 0, NULL));
		SAFE_AMSTREAM(OpenFile(wFile, AMMSF_NOCLOCK|AMMSF_RENDERTOEXISTING));

		info->m_pMMStream=pAMStream;
		pAMStream->AddRef();
		
		hr=info->m_pMMStream->GetMediaStream(MSPID_PrimaryVideo, &info->m_pPrimaryVidStream);

		if(hr != S_OK)
		{
			RELEASEFALSE(info->m_pMMStream)
		}

		hr=info->m_pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&info->m_pDDStream);

		if(hr!=S_OK)
		{
			RELEASE(info->m_pMMStream);
			RELEASEFALSE(info->m_pPrimaryVidStream);
		}

		//---------------------------------------------------- GET ANIM INFO
		
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		hr = info->m_pDDStream->GetFormat(&ddsd, NULL, NULL, NULL);
		
		w=ddsd.dwWidth;
		h=ddsd.dwHeight;

		info->m_pMMStream->GetDuration(&mediaTime);
		info->m_pDDStream->GetTimePerFrame(&frameTime);

		if(frameTime)
		{
			info->fTime=frameTime;
			defaultFT=FALSE;
		}
		else
		{
			info->fTime=DEFAULTFTIME;
			defaultFT=TRUE;
		}
		info->duration=mediaTime;
		fps=(float)((double)MILLISECONDS*1000.0/(double)info->fTime);

		nbFrames = (int) (mediaTime / info->fTime);
		nbBits=32;

	//	*timeFrame = (float) (frameTime*(100*1e-9)*1000);
	/*
		if(pixelFmt != NULL)
		{
			MFXDDGetPixelFormatMFXFromDDPixelFormat(ddsd.ddpfPixelFormat, pixelFmt);
		}
	*/
		//---------------------------------------------------- CREATE ANIM SURFACE
		
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);

		ddsd.dwFlags		= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth		= w;
		ddsd.dwHeight		= h;
		
		ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

		ddsd.ddpfPixelFormat.dwFlags		    = DDPF_RGB; // | DDPF_ALPHAPIXELS; Ouch ! 
		ddsd.ddpfPixelFormat.dwRGBBitCount		= 32;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask  = 0xFF000000;
		ddsd.ddpfPixelFormat.dwRBitMask		    = 0x00FF0000;
		ddsd.ddpfPixelFormat.dwGBitMask		    = 0x0000FF00;
		ddsd.ddpfPixelFormat.dwBBitMask		    = 0x000000FF;

		hr = pDD->CreateSurface(&ddsd, &info->m_pDDSurfaceAnim, NULL);

		if(hr!=S_OK)
		{
			RELEASE(info->m_pMMStream);
			RELEASE(info->m_pPrimaryVidStream);
			RELEASEFALSE(info->m_pDDStream);
		}

		RECT r;
		r.top=0;
		r.left=0;
		r.right=w; 
		r.bottom=h;

		hr = info->m_pDDStream->CreateSample(info->m_pDDSurfaceAnim, &r, 0, &info->m_pSample);

		if (hr != S_OK)
		{
			RELEASE(info->m_pMMStream);
			RELEASE(info->m_pPrimaryVidStream);
			RELEASE(info->m_pDDStream);
			RELEASEFALSE(info->m_pDDSurfaceAnim);
		}

		//---------------------------------------------------- RUN MEDIA.

		hr = info->m_pMMStream->SetState(STREAMSTATE_RUN);

		if (hr != S_OK)
		{
			RELEASE(info->m_pMMStream);
			RELEASE(info->m_pPrimaryVidStream);
			RELEASE(info->m_pDDStream);
			RELEASE(info->m_pDDSurfaceAnim);
			RELEASEFALSE(info->m_pSample);
		}

		isLast=false;

		{
			HRESULT hr = info->m_pSample->Update(0, NULL, NULL, 0);
			if(hr==S_OK)
				return true;
		}
		return false;
	}
	catch(...)
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amovie::close()
{
    if(info->m_pMMStream != NULL)
		info->m_pMMStream->SetState(STREAMSTATE_STOP);

	RELEASEREF(info->m_pMMStream, 1);
	RELEASEREF(info->m_pSample, 0);
	RELEASEREF(info->m_pDDSurfaceAnim, 0);
	RELEASEREF(info->m_pDDStream, 1);
    RELEASEREF(info->m_pPrimaryVidStream, 1);
	RELEASE(info->m_pMMStream);
	RELEASE(info->m_pDDStream);
    RELEASE(info->m_pPrimaryVidStream);

	if(!(--nMovies))
	{
	    RELEASE(pDD);
		CoUninitialize();
	}

	isOK=false;
	w=0;
	h=0;
	nbBits=0;
	nbFrames=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Amovie::openFLI(char *filename)
{
	fliflc=load_fli(filename, FLI_DISK);
	if(fliflc)
	{
		w=fliflc->width;
		h=fliflc->height;
		fps=1024.f/(float)fliflc->speed;
		nbBits=8;
		nbFrames=fliflc->frames;
		bfli=new Abitmap(w, h, nbBits);
		{
			int	i;	
			for(i=0; i<256; i++)
				bfli->palette[i]=255-i;
		}
		isLast=false;
		select(NEXT);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Amovie::closeFLI()
{
	if(fliflc)
		close_fli(fliflc);
	if(bfli)
		delete(bfli);
	bfli=NULL;
	fliflc=NULL;
	isOK=false;
	w=0;
	h=0;
	nbBits=0;
	nbFrames=0;
	isOK=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
