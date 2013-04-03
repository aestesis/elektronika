/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AVI.CPP						(c)	YoY'03						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<assert.h>
#include						"avi.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						AaviRead::CI=ACI("AaviRead", GUID(0xAE57E515,0x00000150), &Anode::CI, 0, NULL);
ADLL ACI						AaviWrite::CI=ACI("AaviWrite", GUID(0xAE57E515,0x00000151), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static __inline void dbgprint(char *s)
{
	FILE	*f=fopen("d:\\elek-Aavi.txt", "a");
	if(f)
	{
		fprintf(f, s);
		fprintf(f, "\r\n");
		fclose(f);
	}
}
*/

#define MAXDRIVERS		64
#define MAXFMTTAGS		256
#define MAXFORMATS		1024

typedef struct 
{
	HACMDRIVERID		hadid;
	ACMDRIVERDETAILS	details;
} ACMdriver;

typedef struct 
{
	int					idACMdriver;
	ACMFORMATTAGDETAILS details;
} ACMfmtTag;

typedef struct 
{
	int					idACMfmttag;
	ACMFORMATDETAILS	details;
} ACMformat;


static int		count=0;
ACMdriver		driver[MAXDRIVERS];
ACMfmtTag		fmttag[MAXFMTTAGS];
static int		ndriver=0;
static int		nfmttag=0;

static BOOL CALLBACK acmFormatTagEnumCallback(HACMDRIVERID hadid, LPACMFORMATTAGDETAILS paftd, DWORD dwInstance, DWORD fdwSupport)
{
	if(nfmttag<MAXFMTTAGS)
	{
		HACMDRIVER	had=(HACMDRIVER)dwInstance;
		fmttag[nfmttag].idACMdriver=dwInstance;
		fmttag[nfmttag].details=*paftd;
		//MMRESULT res=acmFormatEnum(had, details, acmFormatEnumCallback, 0, 0);
		nfmttag++;
	}
	return true;
}


static BOOL CALLBACK acmDriverEnumCallback(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{
	if(ndriver<MAXDRIVERS)
	{
		driver[ndriver].hadid=hadid;
		memset(&driver[ndriver].details, 0, sizeof(ACMDRIVERDETAILS));
		driver[ndriver].details.cbStruct=sizeof(ACMDRIVERDETAILS);
		MMRESULT	res=acmDriverDetails(hadid, &driver[ndriver].details, 0);
		if(!res)
		{
			HACMDRIVER	had;
			MMRESULT	res=acmDriverOpen(&had, hadid, 0);
			if(!res)
			{
				ACMFORMATTAGDETAILS acmftd;
				memset(&acmftd, 0, sizeof(acmftd));
				acmftd.cbStruct=sizeof(acmftd);
				res=acmFormatTagEnum(had, &acmftd, acmFormatTagEnumCallback, (dword)had, 0);
				if(!res)
					ndriver++;
				acmDriverClose(had, 0);
			}
		}
	}
	return true;
}

static void init()
{
	/*
	if(!count)
	{
		MMRESULT res=acmDriverEnum(acmDriverEnumCallback, 0, 0);
	}
	*/
	count++;
}

static void close()
{
	count--;
	/*
	if(!count)
	{
		ndriver=0;
		nfmttag=0;
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaviStream::AaviStream()
{
	memset(name, 0, sizeof(name));
	memset(codec, 0, sizeof(codec));
	memset(&header, 0, sizeof(header));
	memset(&audio, 0, sizeof(audio));
	memset(&video, 0, sizeof(video));
	memset(&generic, 0, sizeof(generic));
	data=NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AaviStream::~AaviStream()
{
	switch(header.fccType)
	{
		case MAKEFOURCC('v','i','d','s'):	// video
		if(video.bitmap)
			delete(video.bitmap);
		if(video.bits)
			free(video.bits);
		if(video.hVidsDecoder)
		{
			HIC		h=video.hVidsDecoder;
			video.hVidsDecoder=NULL;
			try
			{
				ICClose(h);
			}
			catch(...)
			{
				throw;
			}
		}
		break;

		case MAKEFOURCC('a','u','d','s'):	// audio
		if(audio.audioCmp)
			free(audio.audioCmp);
		if(audio.audioPcm)
			free(audio.audioPcm);
		if(audio.hAudsDecoder)
		{
			while(acmStreamClose(audio.hAudsDecoder, 0)==ACMERR_BUSY)
				Athread::sleep(100);
		}
		break;
		
		default:
		break;
	}
	if(generic.format)
		free(generic.format);
	if(data)
		free(data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MAKE2CC(x,y)					(x|(y<<8))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AaviRead::AaviRead(char *filename, int type) : Anode(filename)
{	
	init();
	this->type=type;
	
	force=false;
	
	isOK=false;
	strcpy(this->filename, filename);
	memset(&avihead, 0, sizeof(avihead));

	aie=NULL;
	nbaie=0;
	naie=0;
	aieOffset=0;
	file=NULL;
	audioEachFrame=false;
	error=null;

	memset(&movi, 0, sizeof(movi));
	nstream=0;
	svideo=-1;
	saudio=-1;

	frame=-1;
	nbframes=0;
	
	readBuffer=NULL;
	readBufferSize=0;
	
	isOK=open();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AaviRead::~AaviRead()
{
	if(readBuffer)
		free(readBuffer);
	if(aie)
		free(aie);
	if(file)
		delete(file);
	close();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int AaviRead::getWidth()
{
	if(isOK)
		return avihead.dwWidth;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int AaviRead::getHeight()
{
	if(isOK)
		return avihead.dwHeight;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int AaviRead::getNbBits()
{
	if(isOK&&(svideo!=-1))
		return stream[svideo].video.bitmap->nbBits;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AaviRead::open()
{
	bool	bok=false;
	char	fname[ALIB_PATH];
	strcpy(fname, filename);
	strupr(fname);
	if(strstr(fname, ".AVI"))
	{
		Tchunk	chunk[100];
		int		level=0;
		file=new Afilehd(filename, Afilehd::READ);
		
		memset(chunk, 0, sizeof(chunk));
		
		if(file->isOK())
		{
			while(file->read(&chunk[level].magic, sizeof(dword)))
			{
				if(file->read(&chunk[level].size, sizeof(dword)))
				{
					chunk[level].start=file->offset;
					chunk[level].next=file->offset+chunk[level].size;
					chunk[level].type=0;
					switch(chunk[level].magic)
					{
						case MAKEFOURCC('R','I','F','F'):
						if(file->read(&chunk[level].type, sizeof(dword)))
						{
							switch(chunk[level].type)
							{
								case MAKEFOURCC('A','V','I',' '):
								level++;
								break;
								
								default:
								file->seek(chunk[level].next);
								break;
							}
						}
						else goto fileError;
						break;
						
						case MAKEFOURCC('L','I','S','T'):
						if(file->read(&chunk[level].type, sizeof(dword)))
						{
							switch(chunk[level].type)
							{
								case MAKEFOURCC('h','d','r','l'):
								level++;
								break;
								
								case MAKEFOURCC('s','t','r','l'):
								level++;
								break;

								case MAKEFOURCC('m','o','v','i'):
								movi=chunk[level];
								file->seek(chunk[level].next);
								break;
					
								default:
								file->seek(chunk[level].next);
								break;
							}
						}
						else goto fileError;
						break;
						
						case MAKEFOURCC('a','v','i','h'):
						if(chunk[level].size>=sizeof(MainAVIHeader))
						{
							MainAVIHeader	head;
							if(file->read(&head, sizeof(head)))
							{
								avihead=head;
							}
							else goto fileError;
						}
						else if(!file->seek(chunk[level].next))
							goto fileError;
						break;
						
						case MAKEFOURCC('s','t','r','h'):
						if(chunk[level].size>=sizeof(myAVIStreamHeader))
						{
							myAVIStreamHeader	head;
							if(file->read(&head, sizeof(head)))
							{
								stream[nstream].header=head;
								Conv4cc(stream[nstream].codec, head.fccHandler);
								nstream++;
							}
							else goto fileError;
						}
						else if(!file->seek(chunk[level].next))
							goto fileError;
						break;

						case MAKEFOURCC('s','t','r','f'):	// stream format info
						assert(nstream>0);
						if(nstream>0)
						{
							stream[nstream-1].generic.fsize=chunk[level].size;
							stream[nstream-1].generic.format=malloc(chunk[level].size);
							if(!file->read(stream[nstream-1].generic.format, chunk[level].size))
								goto fileError;
						}
						else if(!file->seek(chunk[level].next))
							goto fileError;
						break;

						case MAKEFOURCC('s','t','r','d'):	// stream additional data
						assert(nstream>0);
						if(nstream>0)
						{
							stream[nstream-1].data=malloc(chunk[level].size);
							if(!file->read(stream[nstream-1].data, chunk[level].size))
								goto fileError;
						}
						else if(!file->seek(chunk[level].next))
							goto fileError;
						break;

						case MAKEFOURCC('s','t','r','n'):	// stream name
						assert(nstream>0);
						if(nstream>0)
						{
							assert(chunk[level].size<sizeof(stream[nstream-1].name));
							if(chunk[level].size<sizeof(stream[nstream-1].name))
							{
								if(!file->read(stream[nstream-1].name, chunk[level].size))
									goto fileError;
							}
							else if(!file->seek(chunk[level].next))
								goto fileError;
						}
						else if(!file->seek(chunk[level].next))
							goto fileError;
						break;

						case MAKEFOURCC('i','d','x','1'):
						nbaie=(int)((chunk[level].next-file->offset)/sizeof(AVIINDEXENTRY));
						if(nbaie)
						{
							aie=(AVIINDEXENTRY *)malloc(sizeof(AVIINDEXENTRY)*nbaie);
							if(file->read(aie, sizeof(AVIINDEXENTRY)*nbaie))
							{
								naie=0;
								checkIDX(file);
								file->seek(chunk[level].next);
								bok=initStreams();
								goto fileOK;
							}
							else goto fileError;
						}
						break;
						
						default:
						if(!file->seek(chunk[level].next))
							goto fileError;
						break;
					}
				}
				else goto fileError;
				
				if(chunk[level].magic)	// not level++
				{
					if(file->offset!=chunk[level].next)	// skip rest chunk's data
					{
						//assert(false);
						if(!file->seek(chunk[level].next))
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
	}
//	if(nstream)
//		bok=initStreams();
	fileError: ;
	fileOK: ;
	return bok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AaviRead::initStreams()
{
	int	i;
	//dbgprint("initstream-start");
	for(i=0; i<nstream; i++)
	{
		AaviStream	*s=&stream[i];
		switch(s->header.fccType)
		{
			case MAKEFOURCC('v','i','d','s'):	// video
			//dbgprint("initstream-vids-start");
			if((type&VIDEO)&&(svideo==-1))
			{
				bool	ok=false;
				svideo=i;
				switch(s->header.fccHandler)
				{
					case MAKEFOURCC('D','I','B',' '):
					case MAKEFOURCC('R','A','W',' '):
					case MAKEFOURCC('R','G','B',' '):
					s->header.fccHandler=0;
					break;
				}

				switch(s->video.bmi->bmiHeader.biCompression)
				{
					case BI_RGB:
					ok=true;
					break;
					
					default:
					switch(s->video.bmi->bmiHeader.biBitCount)
					{
						case 8:
						case 24:
						case 32:
						s->video.bmo.biBitCount=s->video.bmi->bmiHeader.biBitCount;
						break;
						
						default:
						s->video.bmo.biBitCount=24;
						break;
					}
					
					s->video.bmo.biCompression=BI_RGB;	// BI_BITFIELDS (avec alpha) ??
					s->video.bmo.biPlanes=1;
					s->video.bmo.biWidth=avihead.dwWidth;
					s->video.bmo.biHeight=avihead.dwHeight;
					s->video.bmo.biSizeImage=(s->video.bmo.biHeight*s->video.bmo.biWidth*s->video.bmo.biBitCount)>>3;
					
					//	hDecV=ICOpen(MAKEFOURCC('V', 'I', 'D', 'C'), vidshead.fccHandler, ICMODE_DECOMPRESS);
					s->video.hVidsDecoder=ICLocate(MAKEFOURCC('V', 'I', 'D', 'C'), s->header.fccHandler, (BITMAPINFOHEADER *)s->video.bmi, &s->video.bmo, ICMODE_DECOMPRESS); 
					
					if(s->video.hVidsDecoder)
					{
						/*
						if(ICDecompressGetFormat(hDecV, &bmi, &bmo)==ICERR_OK)	// do nothing !!
						{
							int	a=5;
							a++;
						}
						*/

						if(ICDecompressBegin(s->video.hVidsDecoder, (BITMAPINFOHEADER *)s->video.bmi, &s->video.bmo)!=ICERR_OK)
						{
							ICClose(s->video.hVidsDecoder);
							s->video.hVidsDecoder=NULL;
						}
					}

					if(s->video.hVidsDecoder)
					{
						s->video.bits=malloc(s->video.bmo.biSizeImage+65536);
						ok=true;
					}
					else
						memset(&s->video.bmo, 0, sizeof(s->video.bmo));
					break;
				}
				if(ok)
				{
					int nbBits=s->video.hVidsDecoder?s->video.bmo.biBitCount:s->video.bmi->bmiHeader.biBitCount;
					s->video.bitmap=new Abitmap(avihead.dwWidth, avihead.dwHeight, (nbBits<=8)?8:32);
					if(s->video.bitmap->nbBits==8)
					{
						int		nbcol=s->video.bmi->bmiHeader.biClrUsed?s->video.bmi->bmiHeader.biClrUsed:256;
						int		i;
						for(i=0; i<nbcol; i++)
						{
							RGBQUAD	*rgb=&s->video.bmi->bmiColors[i];
							s->video.bitmap->palette[i]=color32(rgb->rgbRed, rgb->rgbGreen, rgb->rgbBlue, rgb->rgbReserved);
						}
					}
				}
			}
			//dbgprint("initstream-vids-end");
			break;
			
			case MAKEFOURCC('a','u','d','s'):	// audio
			//dbgprint("initstream-auds-start");
			if((type&AUDIO)&&(saudio==-1))
			{
				saudio=i;
				s->audio.audioPcmUsed=0;
				switch(s->audio.wvi->wFormatTag)
				{
					case WAVE_FORMAT_PCM:
					//dbgprint("initstream-auds-pcm-start");
					s->audio.wvo.cbSize=sizeof(s->audio.wvo);
					//dbgprint("initstream-auds-pcm-01");
					{
						char	str[1024];
						sprintf(str, "initstream-auds-pcm-01-size=%d", s->audio.fsize);
						//dbgprint(str);
					}
					memcpy(&s->audio.wvo, s->audio.wvi, mini(s->audio.fsize, sizeof(s->audio.wvo)));	//s->audio.wvo=*s->audio.wvi;
					//dbgprint("initstream-auds-pcm-02");
					s->audio.audioPcmSize=s->header.dwSuggestedBufferSize;
					//dbgprint("initstream-auds-pcm-03");
					if(!s->audio.audioPcmSize)
					{
						//dbgprint("initstream-auds-pcm-03-test");
						s->audio.audioPcmSize=65536;
					}
					//dbgprint("initstream-auds-pcm-04");
					s->audio.audioPcm=malloc(s->audio.audioPcmSize);
					//dbgprint("initstream-auds-pcm-end");
					break;
					
					default:
					//dbgprint("initstream-auds-acm-start");
					{
						int		ret=0;
						memset(&s->audio.wvo, 0, sizeof(s->audio.wvo));
						s->audio.wvo.cbSize=sizeof(s->audio.wvo);
						s->audio.wvo.wFormatTag=WAVE_FORMAT_PCM;
						ret=acmFormatSuggest(NULL, s->audio.wvi, &s->audio.wvo, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
						if(!ret)
						{
							ret=acmStreamOpen(&s->audio.hAudsDecoder, NULL, s->audio.wvi, &s->audio.wvo, NULL, NULL, 0, 0);
							if(!ret)
							{
								s->audio.audioCmpSize=s->header.dwSuggestedBufferSize;
								if(!s->audio.audioCmpSize)
									s->audio.audioCmpSize=32768;
								s->audio.audioPcmSize=0;
								acmStreamSize(s->audio.hAudsDecoder, s->audio.audioCmpSize, (DWORD *)&s->audio.audioPcmSize, ACM_STREAMSIZEF_SOURCE);
								if(!s->audio.audioPcmSize)
									s->audio.audioPcmSize=65536;
									
								s->audio.audioCmp=malloc(s->audio.audioCmpSize);
								s->audio.audioPcm=malloc(s->audio.audioPcmSize);
								
								memset(&s->audio.ash, 0, sizeof(s->audio.ash));
								s->audio.ash.cbStruct=sizeof(s->audio.ash);
								s->audio.ash.fdwStatus=0;
								s->audio.ash.dwUser=0; 
								s->audio.ash.pbSrc=(LPBYTE)s->audio.audioCmp;
								s->audio.ash.cbSrcLength=s->audio.audioCmpSize;
								s->audio.ash.pbDst=(LPBYTE)s->audio.audioPcm;
								s->audio.ash.cbDstLength=s->audio.audioPcmSize;
								ret=acmStreamPrepareHeader(s->audio.hAudsDecoder ,&s->audio.ash, 0);
								if(ret)
								{
									acmStreamClose(s->audio.hAudsDecoder, 0);
									s->audio.hAudsDecoder=NULL;
								}
							}
						}
						if(ret)
						{
							saudio=-1;
							error="audio codec not found";
						}
					}
					//dbgprint("initstream-auds-acm-end");
					break;
				}
				if(!audioEachFrame)
				{
			
					saudio=-1;
					error="audio need to be interlaced every frame";
				}
			}
			//dbgprint("initstream-auds-end");
			break;
			
			case MAKEFOURCC('t','x','t','s'):	// text
			break;
			
			case MAKEFOURCC('m','i','d','s'):	// midi
			break;
		}
	}
	
	frame=-1;
	nbframes=avihead.dwTotalFrames-avihead.dwInitialFrames;
	
	//dbgprint("initstream-end\r\n");

	// 4test
	file->seek(movi.start+4);
	return read();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap * AaviRead::getFrame(int n)
{
	AaviStream	*s=&stream[svideo];
	if(svideo==-1)
		return NULL;
	if(!s->video.bitmap)
		return NULL;
	if(frame==n)
		return s->video.bitmap;
	if(n==frame+1)
	{
		if(read())
			return s->video.bitmap;
		else
			return NULL;
	}
	else
	{
		if(seek(n))
			return s->video.bitmap;
		else
			return NULL;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int AaviRead::getSample(int n, void **buffer)
{
	AaviStream	*s=&stream[saudio];
	*buffer=NULL;
	if(saudio==-1)
		return 0;
	if(!s->audio.audioPcm)
		return 0;
	if(frame==n)
	{
		*buffer=s->audio.audioPcm;
		return s->audio.audioPcmUsed;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int AaviRead::setSample44ks(int frame, void *buffer, int bufsize)
{
	void	*src=null;
	int		size=getSample(frame, &src);
	if(size)
	{
		AaviStream	*as=&stream[saudio];
		dword		fmtout=sizeof(sword)*2;
		int			nin=size/as->audio.wvo.nBlockAlign;
		int			nout=bufsize/fmtout;
		{
			switch(as->audio.wvo.nChannels)
			{
				case 1:
				switch(as->audio.wvo.wBitsPerSample)
				{
					case 16:
					if(as->audio.wvo.nSamplesPerSec<1000)
					{
						assert(false);
					}
					else if(as->audio.wvo.nSamplesPerSec<44100)
					{
						int		i;
						short	*s=(short *)src;
						short	*d=(short *)buffer;
						float	ps=0.f;
						float	dps=(float)as->audio.wvo.nSamplesPerSec/(float)44100;
						int		nfout=(int)((float)nin/dps);
						if(nfout<=nout)
						{
							for(i=0; i<nfout; i++)
							{
								int		si=(int)ps;
								float	a0=1.f-(ps-si);
								float	v0=a0*(float)s[si];
								if(si+1<nin)
								{
									float	a1=1.f-a0;
									v0+=a1*(float)s[si+1];
								}
								*(d++)=(short)v0;
								*(d++)=(short)v0;
								ps+=dps;
							}
							return nfout*fmtout;
						}
						else
							assert(false);
					}
					else if(as->audio.wvo.nSamplesPerSec>44100)
					{
						int		i,k;
						short	*s=(short *)src;
						short	*d=(short *)buffer;
						float	ps=0.f;
						float	dps=(float)as->audio.wvo.nSamplesPerSec/(float)44100;
						int		nfout=(int)((float)nin/dps);
						if(nfout<=nout)
						{
							for(i=0; i<nfout; i++)
							{
								float	psn=ps+dps;
								int		n0=(int)ps;
								int		n1=(int)psn;
								float	v=s[n0]*(1.f-(ps-n0));
								for(k=n0+1; k<n1; k++)
									v+=s[k];
								if(n1<nin)
									v+=s[n1]*(psn-n1);
								v/=dps;
								*(d++)=(short)dps;
								*(d++)=(short)dps;
								ps=psn;
							}						
							return nfout*fmtout;
						}
						else
							assert(false);
					}
					else
					{
						if(nin<=nout)
						{
							int		i;
							short	*s=(short *)src;
							short	*d=(short *)buffer;
							for(i=0; i<nin; i++)
							{
								*(d++)=*s;
								*(d++)=*s;
								s++;
							}
							return nin*fmtout;
						}
						else
							assert(false);
					}
					break;
					
					case 8:
					if(as->audio.wvo.nSamplesPerSec<1000)
					{
						assert(false);
					}
					else if(as->audio.wvo.nSamplesPerSec<44100)
					{
						int		i;
						byte	*s=(byte *)src;
						short	*d=(short *)buffer;
						float	ps=0.f;
						float	dps=(float)as->audio.wvo.nSamplesPerSec/(float)44100;
						int		nfout=(int)((float)nin/dps);
						if(nfout<=nout)
						{
							for(i=0; i<nfout; i++)
							{
								int		si=(int)ps;
								float	a0=1.f-(ps-si);
								float	v0=a0*(float)(((int)s[si]-128)<<8);
								if(si+1<nin)
								{
									float	a1=1.f-a0;
									v0+=a1*(float)(((int)s[si+1]-128)<<8);
								}
								*(d++)=(short)v0;
								*(d++)=(short)v0;
								ps+=dps;
							}
							return nfout*fmtout;
						}
						else
							assert(false);
					}
					else if(as->audio.wvo.nSamplesPerSec>44100)
					{
						int		i,k;
						byte	*s=(byte *)src;
						short	*d=(short *)buffer;
						float	ps=0.f;
						float	dps=(float)as->audio.wvo.nSamplesPerSec/(float)44100;
						int		nfout=(int)((float)nin/dps);
						if(nfout<=nout)
						{
							for(i=0; i<nfout; i++)
							{
								float	psn=ps+dps;
								int		n0=(int)ps;
								int		n1=(int)psn;
								float	v=(((int)s[n0]-128)<<8)*(1.f-(ps-n0));
								for(k=n0+1; k<n1; k++)
									v+=((int)s[k]<<8);
								if(n1<nin)
									v+=(((int)s[n1]-128)<<8)*(psn-n1);
								v/=dps;
								*(d++)=(short)dps;
								*(d++)=(short)dps;
								ps=psn;
							}						
							return nfout*fmtout;
						}
						else
							assert(false);
					}
					else
					{
						if(nin<=nout)
						{
							int		i;
							byte	*s=(byte *)src;
							short	*d=(short *)buffer;
							for(i=0; i<nin; i++)
							{
								*(d++)=(((int)*s-128)<<8);
								*(d++)=(((int)*s-128)<<8);
								s++;
							}
							return nin*fmtout;
						}
						else
							assert(false);
					}
					break;
					
					default:
					assert(false);
					break;
				}
				break;
				
				case 2:
				switch(as->audio.wvo.wBitsPerSample)
				{
					case 16:
					if(as->audio.wvo.nSamplesPerSec<1000)
					{
						assert(false);
					}
					else if(as->audio.wvo.nSamplesPerSec<44100)
					{
						int		i;
						short	*s=(short *)src;
						short	*d=(short *)buffer;
						float	ps=0.f;
						float	dps=(float)as->audio.wvo.nSamplesPerSec/(float)44100;
						int		nfout=(int)((float)nin/dps);
						if(nfout<=nout)
						{
							for(i=0; i<nfout; i++)
							{
								int		si=(int)ps;
								int		si2=si<<1;
								float	a0=1.f-(ps-si);
								float	v0=a0*(float)s[si2];
								float	v1=a0*(float)s[si2+1];
								if(si+1<nin)
								{
									float	a1=1.f-a0;
									v0+=a1*(float)s[si2+2];
									v1+=a1*(float)s[si2+3];
								}
								*(d++)=(short)v0;
								*(d++)=(short)v1;
								ps+=dps;
							}
							return nfout*fmtout;
						}
						else
							assert(false);
					}
					else if(as->audio.wvo.nSamplesPerSec>44100)
					{
						int		i,j,k;
						short	*s=(short *)src;
						short	*d=(short *)buffer;
						float	ps=0.f;
						float	dps=(float)as->audio.wvo.nSamplesPerSec/(float)44100;
						int		nfout=(int)((float)nin/dps);
						if(nfout<=nout)
						{
							for(i=0; i<nfout; i++)
							{
								float	psn=ps+dps;
								int		n0=(int)ps;
								int		n1=(int)psn;
								for(j=0; j<2; j++)
								{
									float	v=s[(n0<<1)+j]*(1.f-(ps-n0));
									for(k=n0+1; k<n1; k++)
										v+=s[(k<<1)+j];
									if(n1<nin)
										v+=s[(n1<<1)+j]*(psn-n1);
									*(d++)=(short)(v/dps);
								}
								ps=psn;
							}						
							return nfout*fmtout;
						}
						else
							assert(false);
					}
					else
					{
						if(nin<=nout)
						{
							memcpy(buffer, src, size);
							return size;
						}
						else
							assert(false);
					}
					break;
					
					case 8:
					break;
					
					default:
					assert(false);
					break;
				}
				break;
				
				default:
				assert(false);
				break;
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AaviRead::seek(int f)
{
	if(force)
	{
		if(f>frame)
		{
			int	n=frame+1;
			while(n++<f)
			{
				if(!read())
					return false;
			}
			return true;
		}
		else
		{
			int	n=0;
			frame=-1;
			file->seek(movi.start+4);
			while(n++<f)
			{
				if(!read())
					return false;
			}
			return true;
		}
	}
	else
	{
		if(f>frame)
		{
			if(f-frame>25)
				goIDX(f);
			if(!read())
				return false;
		}
		else
		{
			if(!goIDX(f))
			{
				frame=-1;
				file->seek(movi.start+4);
			}
			if(!read())
				return false;
		}
		if(frame==f)
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AaviRead::goIDX(int f)
{
	if(nbaie)
	{
		int				i;
		int				fvid=0;
		int				fvidkey=-1;
		AVIINDEXENTRY	key;
		
		for(i=0; i<nbaie; i++)
		{
			AVIINDEXENTRY	*ie=&aie[i];
			dword			id=ie->ckid;
			char			ccid[5];
			Conv4cc(ccid, id);
			switch(id>>16)
			{
				case MAKE2CC('w','b'):
				{
					int	ns=(ccid[0]-48)*10+(ccid[1]-48);
					if(ns==saudio)
					{
					}
				}
				break;
				
				case MAKE2CC('d','b'):
				case MAKE2CC('d','c'):
				{
					int	ns=(ccid[0]-48)*10+(ccid[1]-48);
					if(ns==svideo)
					{
						if(ie->dwFlags&AVIIF_KEYFRAME)
						{
							fvidkey=fvid;
							key=*ie;
						}
						fvid++;
						if(fvid>f)
							i=nbaie;
					}
				}
				break;
			}
		}
		
		if(fvidkey!=-1)
		{
			if((frame>=fvidkey)&&(frame<=f))
				return true;
			frame=fvidkey-1;
			file->seek(aieOffset+key.dwChunkOffset);
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AaviRead::testIDX(Afile *f, int test)
{
	bool	b=true;
	dword	id;
	char	ccid[5];
		
	if(0/*avihead.dwPaddingGranularity*/)
	{
		int	n=(int)(file->offset%avihead.dwPaddingGranularity);
		if(n)
			file->seek(file->offset+avihead.dwPaddingGranularity-n);
	}
	else
	{
		int	n=(int)(file->offset%2);
		if(n)
			file->seek(file->offset+2-n);
	}
		
	if(!file->read(&id, sizeof(id)))
		return false;

	Conv4cc(ccid, id);
	
	if((id>>16)==test)
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// detect type of AVI idx chunk (offset information)
//
// [dwOffset] microsoft documentation "AVIOLDINDEX Structure"
//
// Specifies the location of the data chunk in the file. 
// The value should be specified as an offset, in bytes, from the start of the 'movi' list; 
// however, in some AVI files it is given as an offset from the start of the file.

// test also if the interlace audio/video is ok: one audio buffer by video frame

void AaviRead::checkIDX(Afile *f)	
{
	sqword	offset=f->offset;	
	if(nbaie)
	{
		int				i;
		int				ntry=0;
		int				nbegin=0;
		int				nmovi=0;
		dword			oid=0;
		int				nlace=0;
		
		for(i=0; i<nbaie; i++)
		{
			AVIINDEXENTRY	*ie=&aie[i];
			dword			id=ie->ckid;
			char			ccid[5];
			Conv4cc(ccid, id);
			switch(id>>16)
			{
				case MAKE2CC('w','b'):
				case MAKE2CC('d','b'):
				case MAKE2CC('d','c'):
				{
					f->seek(ie->dwChunkOffset);
					if(testIDX(f, id>>16))
						nbegin++;
					f->seek(movi.start+ie->dwChunkOffset);
					if(testIDX(f, id>>16))
						nmovi++;
					if(ntry++>11)
						i=nbaie;
					nlace+=(id!=oid)?1:-1;
					oid=id;
				}
				break;
			}
		}
		aieOffset=(int)((nmovi>nbegin)?movi.start:0);
		audioEachFrame=(nlace>(ntry/2));
	}
	f->seek(offset);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AaviRead::read()
{
	bool	b=true;
	dword	id;
	dword	size;

	while(b)
	{
		char	ccid[5];
		
		if(avihead.dwPaddingGranularity)
		{
			int	n=(int)(file->offset%avihead.dwPaddingGranularity);
			if(n)
				file->seek(file->offset+avihead.dwPaddingGranularity-n);
		}
		else
		{
			int	n=(int)(file->offset%2);
			if(n)
				file->seek(file->offset+2-n);
		}
		
		if(!file->read(&id, sizeof(id)))
			return false;

		Conv4cc(ccid, id);
			
		if(!file->read(&size, sizeof(size)))
			return false;

		if(id==MAKEFOURCC('L','I','S','T'))
			return false;	// todo:

		if(!size)
			continue;
			
		if(size>(1024*65536))
			return false;
			
		if(readBufferSize<size)
		{
			if(readBuffer)
				free(readBuffer);
			readBuffer=malloc(readBufferSize=size);	
		}
		
		if(!file->read(readBuffer, size))
			return false;

		switch(id>>16)
		{
			case MAKE2CC('w','b'):
			{
				int	ns=(ccid[0]-48)*10+(ccid[1]-48);
				if(ns==saudio)
				{
					AaviStream	*s=&stream[saudio];
					switch(s->audio.wvi->wFormatTag)
					{
						case WAVE_FORMAT_PCM:
						if(size<=s->audio.audioPcmSize)
						{
							memcpy(s->audio.audioPcm, readBuffer, size);
							s->audio.audioPcmUsed=size;
						}
						else
						{
							assert(false);
						}
						break;
						
						default:
						{
							DWORD		flags=0;
							if(size==s->audio.audioCmpSize)
							{
								memcpy(s->audio.audioCmp, readBuffer, size);
								flags=ACM_STREAMCONVERTF_BLOCKALIGN;
							}
							else if(size<s->audio.audioCmpSize)
							{
								memcpy(s->audio.audioCmp, readBuffer, size);
							}
							else
							{
								assert(false);
							}
						
							if(acmStreamConvert(s->audio.hAudsDecoder, &s->audio.ash, flags))
								s->audio.audioPcmUsed=s->audio.ash.cbDstLengthUsed;
						}
						break;
					}
				}
			}
			continue;

			case MAKE2CC('d','b'):
			case MAKE2CC('d','c'):
			{
				int	ns=(ccid[0]-48)*10+(ccid[1]-48);
				if(ns==svideo)
				{
					AaviStream	*s=&stream[svideo];
					Abitmap		*bitmap=s->video.bitmap;
					if(bitmap)
					{
						int			width=bitmap->w;
						int			height=bitmap->h;
						char		bicc[5];
						Conv4cc(bicc, s->video.bmi->bmiHeader.biCompression);
						
						switch(s->video.bmi->bmiHeader.biCompression)
						{
							case BI_RGB:
							switch(s->video.bmi->bmiHeader.biBitCount)
							{
								case 8:
								{
									byte	*ss=(byte *)readBuffer;
									int		y;
									for(y=0; y<height; y++)
									{
										byte	*d=&bitmap->body8[bitmap->adr[height-y-1]];
										memcpy(d, ss, width);
										ss+=width;
									}
								}
								break;

								case 24:
								{
									byte	*ss=(byte *)readBuffer;
									int		x,y;
									for(y=0; y<height; y++)
									{
										dword	*d=&bitmap->body32[bitmap->adr[height-y-1]];
										for(x=0; x<width; x++)
										{
											byte	b=*(ss++);
											byte	g=*(ss++);
											byte	r=*(ss++);
											*(d++)=color32(r, g, b);
										}
									}
								}
								break;

								case 32:
								{
									byte	*ss=(byte *)readBuffer;
									int		y;
									int		len=width*4;
									for(y=0; y<height; y++)
									{
										dword	*d=&bitmap->body32[bitmap->adr[height-y-1]];
										memcpy(d, ss, len);
										ss+=len;
									}
								}
								bitmap->flags=bitmapALPHA;
								break;
								
								default:
								return false;
							}
							break;
							
							//case BI_BITFIELD:
							//break;

							default:
							if(s->video.hVidsDecoder)
							{
								try
								{
									// ICDECOMPRESS_NOTKEYFRAME
									if(ICDecompress(s->video.hVidsDecoder, ICDECOMPRESS_NOTKEYFRAME, (BITMAPINFOHEADER *)s->video.bmi, readBuffer, &s->video.bmo, s->video.bits)==ICERR_OK)
									{
										switch(s->video.bmo.biBitCount)
										{
											case 8:
											{
												byte	*ss=(byte *)s->video.bits;
												int		y;
												for(y=0; y<height; y++)
												{
													byte	*d=&bitmap->body8[bitmap->adr[height-y-1]];
													memcpy(d, ss, width);
													ss+=width;
												}
											}
											break;
											
											case 24:
											{
												byte	*ss=(byte *)s->video.bits;
												int		x,y;
												//int		nbdif=0;
												for(y=0; y<height; y++)
												{
													dword	*d=&bitmap->body32[bitmap->adr[height-y-1]];
													for(x=0; x<width; x++)
													{
														byte	b=*(ss++);
														byte	g=*(ss++);
														byte	r=*(ss++);
														dword v=color32(r, g, b);
														//if(v!=*d)
														//	nbdif++;
														*(d++)=v;
													}
												}
												//int z=nbdif;
											}
											break;

											case 32:
											{
												byte	*ss=(byte *)s->video.bits;
												int		y;
												int		len=width*4;
												for(y=0; y<height; y++)
												{
													dword	*d=&bitmap->body32[bitmap->adr[height-y-1]];
													memcpy(d, ss, len);
													ss+=len;
												}
											}
											bitmap->flags=bitmapALPHA;
											break;
											
											default:
											return false;
										}
									}
								}
								catch(...)
								{
									return false;
								}
							}
							else
								return false;
							break;
						}
					}
					else
						return false;
				}
			}
			break;
			
			default:
			continue;
		}
		b=false;
	}
	frame++;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL float AaviRead::getFPS()
{
	float	fps=0.f;
	AaviStream	*s=&stream[svideo];
	if(svideo!=-1)
	{
		fps=(float)s->header.dwRate/(float)s->header.dwScale;
		if((fps>100.f)||(fps<1.f))
			fps=25.f;
	}
	return fps;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Achunk
{
public:
							Achunk								(Afile *f, char *name);
							Achunk								(Achunk *c, char *name);

	virtual					~Achunk								();

	int						write								(void *buf, int bufsize);
	void					writeMagic							(char *magic);

	dword					magic;
	Achunk					*father;
	Afile					*f;
	int						start;
	int						size;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Achunk::Achunk(Afile *f, char *name)
{
	magic=MAKEFOURCC(name[0],name[1],name[2],name[3]);
	this->f=f;
	start=(int)f->offset;
	size=-8;
	father=null;
	writeMagic(name);
	write(&size, sizeof(size));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Achunk::Achunk(Achunk *c, char *name)
{
	magic=MAKEFOURCC(name[0],name[1],name[2],name[3]);
	f=c->f;
	start=(int)f->offset;
	size=-8;
	father=c;
	writeMagic(name);
	write(&size, sizeof(size));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Achunk::~Achunk()
{
	if(f->offset&1)
	{
		byte	b=0;
		write(&b, sizeof(b));
	}
	qword offset=f->offset;
	f->seek(start+4);
	f->write(&size, sizeof(size));
	f->seek(offset);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Achunk::write(void *buf, int bufsize)
{
	size+=bufsize;
	if(father)
		return father->write(buf, bufsize);
	else
		return f->write(buf, bufsize);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Achunk::writeMagic(char *name)
{
	dword	magic=MAKEFOURCC(name[0],name[1],name[2],name[3]);
	write(&magic, sizeof(magic));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FPS		(25)

ADLL AaviWrite::AaviWrite(char *filename, int width, int height, float fps, bool vdub)
{
	this->vdub=vdub;
	file=null;
	c_avi=null;
	c_mov=null;
	isOK=false;
	strcpy(this->filename, filename);
	image=new Abitmap(width, height);


	memset(&avihead, 0, sizeof(avihead));
	avihead.dwHeight=height;
	avihead.dwWidth=width;
	avihead.dwStreams=2;
	avihead.dwPaddingGranularity=0;
	avihead.dwFlags=AVIF_HASINDEX|AVIF_ISINTERLEAVED;
	avihead.dwMicroSecPerFrame=(int)(1000000.0/(double)fps);


	memset(&vidhead, 0, sizeof(vidhead));
	vidhead.fccType=MAKEFOURCC('v','i','d','s');
	
	vidhead.fccHandler= vdub?MAKEFOURCC('M','J','P','G'):MAKEFOURCC('D','I','B',' ');
	vidhead.dwSuggestedBufferSize=width*height*4;
	{
		int	i;
		for(i=1; i<10000; i++)
		{
			float	rate=(float)i*fps;
			if(((float)(int)rate)==rate)
			break;
		}
		vidhead.dwScale=i;
		vidhead.dwRate=(int)((float)vidhead.dwScale*fps);
	}

	memset(&audhead, 0, sizeof(audhead));
	audhead.fccType=MAKEFOURCC('a','u','d','s');
	audhead.dwInitialFrames=0;
	audhead.dwSampleSize=4;
	audhead.dwScale=4;
	audhead.dwRate=176400;	// 44100*dwScale
	audhead.dwSuggestedBufferSize=((int)(audhead.dwRate/fps));

	maxaie=1000;
	aie=(AVIINDEXENTRY *)malloc(sizeof(AVIINDEXENTRY)*maxaie);
	nbaie=0;

	this->fps=fps;

	audio=new Abuffer("audio", (int)(4.f*44100.f/(fps*0.1f)));
	open();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AaviWrite::~AaviWrite()
{
	this->close();
	delete(audio);
	delete(image);
	free(aie);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AaviWrite::open()
{
	file=new Afilehd(filename, Afilehd::WRITE);
	isOK=file->isOK();
	if(isOK)
	{
		c_avi=new Achunk(file, "RIFF");
		c_avi->writeMagic("AVI ");
		{
			Achunk	c_head=Achunk(c_avi, "LIST");
			c_head.writeMagic("hdrl");
			{	// avi header
				Achunk c_avih=Achunk(&c_head, "avih");
				o_avihead=(dword)c_avih.f->offset;
				c_avih.write(&avihead, sizeof(avihead));
			}
			{	// video
				Achunk c_vid=Achunk(&c_head, "LIST");
				c_vid.writeMagic("strl");
				{
					Achunk c_vhead=Achunk(&c_vid, "strh");
					o_vidhead=(dword)c_vhead.f->offset;
					c_vhead.write(&vidhead, sizeof(vidhead));
				}
				{
					Achunk				c_vfmt=Achunk(&c_vid, "strf");
					BITMAPINFOHEADER	bmh;
					memset(&bmh, 0, sizeof(bmh));
					bmh.biHeight=avihead.dwHeight;
					bmh.biWidth=avihead.dwWidth;
					bmh.biBitCount=0; //32;
					bmh.biPlanes=1;
					bmh.biSize=sizeof(bmh);
					bmh.biSizeImage=avihead.dwHeight*avihead.dwWidth*4;
					bmh.biCompression=vdub?MAKEFOURCC('M','J','P','G'):BI_RGB;
					c_vfmt.write(&bmh, sizeof(bmh));
				}
			}
			{	// audio
				Achunk c_aud=Achunk(&c_head, "LIST");
				c_aud.writeMagic("strl");
				{
					Achunk c_ahead=Achunk(&c_aud, "strh");
					o_audhead=(dword)c_ahead.f->offset;
					c_ahead.write(&audhead, sizeof(audhead));
				}
				{
					Achunk				c_afmt=Achunk(&c_aud, "strf");
					WAVEFORMATEX		wfmt;
					memset(&wfmt, 0, sizeof(wfmt));
					wfmt.cbSize=sizeof(wfmt);
					wfmt.nChannels=2;
					wfmt.nSamplesPerSec=44100;
					wfmt.wBitsPerSample=16;
					wfmt.wFormatTag=WAVE_FORMAT_PCM;
					wfmt.nBlockAlign=4;
					wfmt.nAvgBytesPerSec=wfmt.nSamplesPerSec*wfmt.nBlockAlign;
					c_afmt.write(&wfmt, sizeof(wfmt));
				}
			}
		}
		c_mov=new Achunk(c_avi, "LIST");
		c_mov->writeMagic("movi");
	}
	else
	{
		delete(file);
		file=null;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AaviWrite::close()
{
	if(isOK)
	{
		delete(c_mov);
		c_mov=null;
		{
			Achunk c_idx=Achunk(c_avi, "idx1");
			c_idx.write(aie, nbaie*sizeof(AVIINDEXENTRY));
		}
		delete(c_avi);
		c_avi=null;
		{
			sqword	offset=file->offset;
			file->seek(o_vidhead);
			file->write(&vidhead, sizeof(vidhead));
			file->seek(o_audhead);
			file->write(&audhead, sizeof(audhead));
			file->seek(o_avihead);
			file->write(&avihead, sizeof(avihead));
			file->seek(offset);
		}
		delete(file);
		file=null;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AaviWrite::setBitmap(Abitmap *b)
{
	image->set(0, 0, image->w, image->h, 0, 0, b->w, b->h, b, bitmapNORMAL, bitmapNORMAL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AaviWrite::setSample(void *buffer, int bufsize)
{
	audio->write(buffer, bufsize);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AaviWrite::writeFrame()
{
	bool	ok=true;
	{	// video
		Achunk	c_vid=Achunk(c_mov, "00db");
		if(vdub)
		{	// jpeg
			byte data[4*1024*1024];
			Afilemem buf=Afilemem("buffer", data, sizeof(data));
			image->save(&buf, bitmapJPG, 95);
			c_vid.write(data, buf.getSize());
		}
		else
		{	// normal
			int		y;
			int		ws=image->w*4;
			for(y=avihead.dwHeight-1; y>=0; y--)
			{
				if(c_vid.write(&image->body32[y*image->w], ws)!=ws)
					ok=false;
			}
		}
		vidhead.dwLength++;
		addIDX(&c_vid, true);	// get the real chunk size if here
	}
	{	// audio
		Achunk	c_aud=Achunk(c_mov, "01wb");
		int		need=((int)(44100.f/fps))*4;
		int		avaible=mini((int)audio->getSize(), need);
		int		dif=need-avaible;
		while(avaible)
		{
			byte	buf[1024];
			int		n=mini(avaible, 1024);
			audio->read(buf, n);
			if(!c_aud.write(buf, n)==n)
				ok=false;
			avaible-=n;
			audhead.dwLength+=n;
		}
		if(dif>0)
		{
			byte	b[1024];
			memset(b, 0, sizeof(b));
			while(dif>sizeof(b))
			{
				c_aud.write(&b, sizeof(b));
				dif-=sizeof(b);
			}
			if(dif>0)
				c_aud.write(&b, dif);
		}
		addIDX(&c_aud, false);	// get the real chunk size if here
	}
	avihead.dwTotalFrames++;
	return ok;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AaviWrite::addIDX(Achunk *c, bool key)
{
	if(nbaie>=maxaie)
	{
		int	max=maxaie;
		maxaie+=1000;
		AVIINDEXENTRY * ie=(AVIINDEXENTRY *)malloc(sizeof(AVIINDEXENTRY)*maxaie);
		memcpy(ie, aie, max*sizeof(AVIINDEXENTRY));
		free(aie);
		aie=ie;
	}
	{
		AVIINDEXENTRY	*e=&aie[nbaie++];
		e->ckid=c->magic;
		e->dwFlags=key?AVIIF_KEYFRAME:0;
		e->dwChunkOffset=c->start;
		e->dwChunkLength=c->size;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
