/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<stdio.h>
#include						<string.h>
#include						"videoPlayer.h"
#include						"QThelpers.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned __int32		dword;

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

__inline float frand()
{
	return (float)rand()/(float)RAND_MAX;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

VideoPlayer::VideoPlayer(MediaHost *host) : MediaPlugin(host)
{
	memset(media, 0, sizeof(media));
	memset(qt, 0, sizeof(qt));
	m_cur=-1;
	m_random_beat=0;
}

VideoPlayer::~VideoPlayer()
{
	int i;
	for(i=0; i<MAXQT; i++)
		if(qt[i])
		{
			qtHost.delQT(qt[i]);
			qt[i]=null;
		}
}

void VideoPlayer::stop()
{
	MediaSelectView *view=(MediaSelectView *)media[pinSELECTMOVIE];
	int				max=view->getMaxValue()+1;
	int				i;
	for(i=0; i<max;i++)
	{
		if(qt[i])
			qt[i]->play=false;
	}
}

bool VideoPlayer::process(double time, double beat)
{
	float bpmdif=(float)(host->getBPM()/MPBPMREF);
	int	i;
	for(i=0; i<pinNB; i++)	// check if all pins are linked
		if(!media[i])
			return false;

	MediaSelectView *view=(MediaSelectView *)media[pinSELECTMOVIE];

	view->lock(__FILE__,__LINE__);

	int cur=view->getValue();
	int max=view->getMaxValue()+1;

	if(m_cur!=cur)
		for(i=0; i<max; i++)
			if((i!=cur)&&qt[i])
				qt[i]->play=false;

	for(i=0; i<max; i++)
	{
		char *desc=view->getDesc(i);
		if(desc)
		{
			bool bload=false;
			if(!qt[i])
			{
				bload=true;
			}
			else if(strcmp(desc, qt[i]->getFilename()))	// different
			{
				qtHost.delQT(qt[i]);
				qt[i]=null;
				bload=true;
			}
			if(bload)
			{
				MediaBitmap *mb=((MediaSelectView *)media[pinSELECTMOVIE])->getView(i);
				mb->lock(__FILE__,__LINE__);
				memset(mb->getPixels(), 0, mb->getWidth()*mb->getHeight()*sizeof(MPdword));
				mb->plug_updated=true;
				mb->unlock();
				qt[i]=qtHost.newQT(desc);
				qt[i]->link(mb);
			}
		}
	} 

	view->unlock();

	if(cur!=m_cur)
	{
		((MediaFloat *)media[pinLOOPSTART])->setValue(memo[cur].start);
		media[pinLOOPSTART]->plug_updated=true;
		((MediaFloat *)media[pinLOOPLENGTH])->setValue(memo[cur].length);
		media[pinLOOPLENGTH]->plug_updated=true;
	}

	if(qt[cur])
	{
		memo[cur].start=qt[cur]->loopstart=(float)((MediaFloat *)media[pinLOOPSTART])->getValue();
		media[pinLOOPSTART]->host_updated=false;
		memo[cur].length=qt[cur]->looplength=(float)((MediaFloat *)media[pinLOOPLENGTH])->getValue();
		media[pinLOOPLENGTH]->host_updated=false;

		float f_revers=( ((MediaInteger *)media[pinREVERS])->getValue() != 0)?-1.f:1.f;
		qt[cur]->rate=(float)(((MediaFloat *)media[pinRATE])->getValue()-0.5f)*10.f*f_revers*bpmdif;
		media[pinRATE]->host_updated=false;

		qt[cur]->playmode=((MediaInteger *)media[pinMODE])->getValue();

		qt[cur]->play=((MediaInteger *)media[pinPLAY])->getValue()?true:false;
		media[pinPLAY]->host_updated=false;
		qt[cur]->sel=((MediaInteger *)media[pinLOOP])->getValue()?true:false;
		media[pinLOOP]->host_updated=false;
		qt[cur]->palindrome=((MediaInteger *)media[pinPALINDROME])->getValue()?true:false;
		media[pinPALINDROME]->host_updated=false;

		float	f_random=(float)((MediaFloat *)media[pinRANDOM])->getValue();
		bool	b_random=false;

		if(f_random>0.f)
		{
			float dbeat;
			if(f_random<0.5f)
				dbeat=(0.5f-f_random)*20+1.f;
			else
				dbeat=1.f-(f_random-0.5f)*2.f;
			if(beat>m_random_beat+dbeat)
			{
				b_random=true;
				m_random_beat=beat;
			}
		}

		qt[cur]->posfocus=(((MediaFloat *)media[pinPOS])->focus()==MediaFloat::focusHOST)||media[pinPOS]->host_updated;

		if(b_random&&!qt[cur]->posfocus)
		{
			float p=(qt[cur]->sel)?(memo[cur].length*frand()+memo[cur].start):frand();
			qt[cur]->posfocus=true;
			qt[cur]->pos=p;
			((MediaFloat *)media[pinPOS])->setValue(p);
		}
		else
		{
			if(qt[cur]->posfocus)
				qt[cur]->pos=(float)((MediaFloat *)media[pinPOS])->getValue();
			else
				((MediaFloat *)media[pinPOS])->setValue(qt[cur]->pos);
		}



		media[pinPOS]->host_updated=false;

		view->lock(__FILE__,__LINE__);
		{
			MediaBitmap *dst=(MediaBitmap *)media[pinVIDEO];
			MediaBitmap *src=view->getView((m_cur>=0)?m_cur:cur);	// wait one frame start display
			src->lock(__FILE__,__LINE__);
			dst->lock(__FILE__,__LINE__);
			src->copyTo(dst);
			dst->unlock();
			src->unlock();
		}
		view->unlock();
	}

	m_cur=cur;

	return true;
}

void VideoPlayer::link(int pin, MediaBuffer *mb)
{
	media[pin]=mb;
}

void VideoPlayer::getError(char *error)
{
	strcpy(error, "no errors");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

VideoPlayerInfo::VideoPlayerInfo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

VideoPlayerInfo::~VideoPlayerInfo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
bool VideoPlayerInfo::getName(char *name)
{
	strcpy(name, "QT player");
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPqword VideoPlayerInfo::getGUID()
{
	return 0x5186567899836574;	// a static 64bits number as GUID
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
int VideoPlayerInfo::getGroup()
{
	return MPgroupPLAYER;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int	VideoPlayerInfo::getPriority()
{
	return MPpriorityIMPORT;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int VideoPlayerInfo::getMediaCount()
{
	return pinNB;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

MediaInfo VideoPlayerInfo::getMediaInfo(int pin)
{
	MediaInfo	*mn=0;
	switch(pin)
	{
		case pinVIDEO:
		return MediaInfo(MPmediaOUT|MPmediaBITMAP_XRGB, 0x2156788852102641, "video out", "video output");
		case pinPLAY:
		return MediaInfo(MPmediaIN|MPmediaINTEGER, 0x6523567899875884, "play", "play/pause", new MediaInteger(0), new MediaInteger(0), new MediaInteger(1));
		case pinPOS:
		return MediaInfo(MPmediaIN|MPmediaOUT|MPmediaFLOAT, 0x6523345678975884, "position", "current play position", new MediaFloat(0.0f));
		case pinRATE:
		return MediaInfo(MPmediaIN|MPmediaFLOAT, 0x6545678995398884, "rate", "movie play rate", new MediaFloat(0.6f));
		case pinLOOP:
		return MediaInfo(MPmediaIN|MPmediaINTEGER, 0x6594795378975884, "selection", "use selection", new MediaInteger(0), new MediaInteger(0), new MediaInteger(1));
		case pinPALINDROME:
		return MediaInfo(MPmediaIN|MPmediaINTEGER, 0x6594577978975884, "palindrome", "palindrome loop mode", new MediaInteger(0), new MediaInteger(0), new MediaInteger(1));
		case pinLOOPSTART:
		return MediaInfo(MPmediaIN|MPmediaOUT|MPmediaFLOAT, 0x4859872689954741, "loop start", "loop start point", new MediaFloat(0.0f));
		case pinLOOPLENGTH:
		return MediaInfo(MPmediaIN|MPmediaOUT|MPmediaFLOAT, 0x6335679987546721, "loop length", "loop length", new MediaFloat(1.0f));
		case pinSELECTMOVIE:
		return MediaInfo(MPmediaIN|MPmediaSELECTVIEW, 0x6335446789876721, "movies", "movies", new MediaInteger(0), new MediaInteger(0), new MediaInteger(MAXQT-1));
		case pinREVERS:
		return MediaInfo(MPmediaIN|MPmediaINTEGER, 0x6523564688985884, "revers", "revers mode", new MediaInteger(0), new MediaInteger(0), new MediaInteger(1));
		case pinRANDOM:
		return MediaInfo(MPmediaIN|MPmediaFLOAT, 0x6546abcdfff77884, "random", "random mode", new MediaFloat(0.f));
		case pinMODE:
		return MediaInfo(MPmediaIN|MPmediaINTEGER, 0x65567abcd8798432, "mode", "play mode", new MediaInteger(1), new MediaInteger(0), new MediaInteger(1));
	}
	return MediaInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
MediaPlugin * VideoPlayerInfo::getNewInstance(MediaHost *host)
{
	return (MediaPlugin *)new VideoPlayer(host);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	__declspec(dllexport) int getMediaPluginVersion()
	{
		return MPVERSION;
	}

	__declspec(dllexport) class VideoPlayerInfo * getMediaPluginInfo()
	{
		return new VideoPlayerInfo();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
