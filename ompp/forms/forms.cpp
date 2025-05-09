/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<stdio.h>
#include						<string.h>
#include						"forms.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

//typedef unsigned __int32		dword;

/////////////////////////////////////////////////////////////////////////////////////////////////////

FormsPlugin::FormsPlugin(MediaHost *host) : MediaPlugin(host)
{
	memset(media, 0, sizeof(media));
}

FormsPlugin::~FormsPlugin()
{
}

bool FormsPlugin::process(double time, double beat)
{
	int	i;
	for(i=0; i<pinNB; i++)	// check if all pins are linked
		if(!media[i])
			return false;
	{
		int		r=(int)(((MediaFloat *)media[inRED])->getValue()*256.f*2.f);
		int		g=(int)(((MediaFloat *)media[inGREEN])->getValue()*256.f*2.f);
		int		b=(int)(((MediaFloat *)media[inBLUE])->getValue()*256.f*2.f);
		int		w=((MediaBitmap *)media[inVIDEO])->getWidth();
		int		h=((MediaBitmap *)media[inVIDEO])->getHeight();
		MPdword	*src=((MediaBitmap *)media[inVIDEO])->getPixels();
		MPdword	*dst=((MediaBitmap *)media[outVIDEO])->getPixels();
		int		sz=w*h;
		for(i=0; i<sz; i++)
		{
			MPbyte	rc,gc,bc;
			MPcolorRGB(&rc, &gc, &bc, *(src++));
			*(dst++)=MPsature32(((int)rc*r)>>8, ((int)gc*g)>>8, ((int)bc*b)>>8);
		}
	}
	return true;
}

void FormsPlugin::link(int pin, MediaBuffer *mb)
{
	media[pin]=mb;
}


void FormsPlugin::getError(char *error)
{
	strcpy(error, "no errors");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

FormsPluginInfo::FormsPluginInfo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

FormsPluginInfo::~FormsPluginInfo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
bool FormsPluginInfo::getName(char *name)
{
	strcpy(name, "forms");
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

MPqword FormsPluginInfo::getGUID()
{
	return 0x5186214795136574;	// a static 64bits number as GUID
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
int FormsPluginInfo::getGroup()
{
	return MPgroupEFFECT2D;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int	FormsPluginInfo::getPriority()
{
	return MPpriorityFILTER;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int FormsPluginInfo::getMediaCount()
{
	return pinNB;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

MediaInfo FormsPluginInfo::getMediaInfo(int pin)
{
	MediaInfo	*mn=0;
	switch(pin)
	{
		case inVIDEO:
		return MediaInfo(MPmediaIN|MPmediaBITMAP_XRGB, 0x2155201452102641, "video in", "video input");
		case inRED:
		return MediaInfo(MPmediaIN|MPmediaFLOAT, 0x4856214201698541, "red", "red color value", new MediaFloat(0.5f));
		case inGREEN:
		return MediaInfo(MPmediaIN|MPmediaFLOAT, 0x6321015400254521, "green", "green color value", new MediaFloat(0.5f));
		case inBLUE:
		return MediaInfo(MPmediaIN|MPmediaFLOAT, 0x6523102458975884, "blue", "blue color value", new MediaFloat(0.5f));
		case outVIDEO:
		return MediaInfo(MPmediaOUT|MPmediaBITMAP_XRGB, 0x6321558745623487, "video out", "video output");
	}
	return MediaInfo();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
MediaPlugin * FormsPluginInfo::getNewInstance(MediaHost *host)
{
	return (MediaPlugin *)new FormsPlugin(host);
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

	__declspec(dllexport) class FormsPluginInfo * getMediaPluginInfo()
	{
		return new FormsPluginInfo();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
