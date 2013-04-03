/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	OMPP.H		ver 1.00		Renan Jegouzo [aka YoY]	/ www.aestesis.org								2006
//
//  OpenMediaPluginPlatform		www.sourceforge.net
//
//	supported by "Elektronika" live video studio software
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	distributed under the LGPL license. see license.txt
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _OPEN_MEDIA_PLUGIN_PLATFORM_H_
#define _OPEN_MEDIA_PLUGIN_PLATFORM_H_
#pragma warning( disable : 4995)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						<string.h>
#include						<malloc.h>
#include						<windows.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MPVERSION					0x00010000

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define 						MPSTRINGSIZE 				128
#define							MPPATHSIZE					1024
#define							MPBPMREF					(120.0)		// at BPMref movies are played at normal rate

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// group

#define							MPgroupGENERIC				0
#define							MPgroupINOUT				1
#define							MPgroupMIXER				2
#define							MPgroupPLAYER				3
#define							MPgroupEFFECT2D				4
#define							MPgroupEFFECT3D				5
#define							MPgroupEFFECTAUDIO			6
#define							MPgroupRENDER				7

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// priority

#define							MPpriorityIMPORT			1		// import media from device (ex: sound card capture)
#define							MPpriorityFILTER			2		// filter (process IN/OUT media) 
#define							MPpriorityEXPORT			3		// export media to device (ex: sound card playback)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// media types

// MASK
#define							MPmaskDIR					0xf0000000
#define							MPmaskTYPE					0x0fffffff

// DIRECTION

#define							MPmediaIN					0x10000000	
#define							MPmediaOUT					0x20000000

// TYPES
#define							MPmediaUNKNOWN				0x0000		// not defined "error"
#define							MPmediaFLOAT				0x0100		// float value (double)	- default range [0..1] 
#define							MPmediaINTEGER				0x0110		// integer value - default range [0..255] 

#define							MPmediaSELECT				0x0150		// select value (ex: gui listbox)
#define							MPmediaSELECTVIEW			0x0151		// select value with preview for each choice (ex: movie select)
									
#define							MPmediaBITMAP_XRGB			0x0200		// bitmap - 32 bits
#define							MPmediaBITMAP_ARGB			0x0201		// bitmap - 32 bits with alpha
#define							MPmediaBITMAP_XYUV			0x0202		// bitmap - 32 bits	
#define							MPmediaBITMAP_XHLS			0x0203		// bitmap - 32 bits
								
#define							MPmediaSAMPLE_PCM_MONO		0x0300		// sound 44,1k 16 bits mono			(signed short)
#define							MPmediaSAMPLE_PCM_STEREO	0x0301		// sound 44,1k 16 bits stereo		(signed short)

#define							MPmediaANSISTRING			0x0400		// an ansi zero terminal string		(BYTE characters)
#define							MPmediaUNICODESTRING		0x0401		// an unicode zero terminal string	(WORD characters)

#define							MPmediaOGL_TEXTURE			0x0500		// an openGL texture identifier

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MPmediaSELECT_MAXVALUE		(128)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// simples types

typedef unsigned __int64		MPqword;
typedef unsigned __int32		MPdword;
typedef unsigned __int16		MPword;
typedef unsigned __int8			MPbyte;

typedef signed __int64			MPsqword;
typedef signed __int32			MPsdword;
typedef signed __int16			MPsword;
typedef signed __int8			MPsbyte;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Media Info structure

class MediaInfo
{
public:

	int						type;						// media type
	MPqword					guid;						// the GUID of the media pin (64 bits unique identifier)
	char					name[MPSTRINGSIZE];			// name of the media
	char					help[MPSTRINGSIZE];			// a little help on the media usage (tooltips)
	class MediaBuffer		*defvalue;					// default value (can be null)
	class MediaBuffer		*minvalue;					// minimum value (can be null)
	class MediaBuffer		*maxvalue;					// maximum value (can be null)
	
	MediaInfo()
	{
		type=MPmediaUNKNOWN;
		strcpy(name, "UNKNOWN MEDIA");
		guid=0;
		help[0]=0;
		defvalue=NULL;
	}
	
	virtual ~MediaInfo();
	
	MediaInfo(int type, MPqword guid, char *name, char *help, class MediaBuffer *defvalue=NULL, class MediaBuffer *minvalue=NULL, class MediaBuffer *maxvalue=NULL)
	{
		this->type=type;
		this->guid=guid;
		if(name)
			strcpy(this->name, name);
		else
			this->name[0]=0;
		if(help)
			strcpy(this->help, help);
		else
			this->help[0]=0;
		this->defvalue=defvalue;
		this->minvalue=minvalue;
		this->maxvalue=maxvalue;
	}

	MediaInfo(MediaInfo &mi);
	void operator=(MediaInfo &mi);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MediaLock

class MediaLock	// generic multi threaded lock/unlock mechanism // on windows, use critical section
{
	CRITICAL_SECTION	cs;
	char dbginfo[1024];
public:

	MediaLock()
	{
		memset(dbginfo, 0, sizeof(dbginfo));
		InitializeCriticalSection(&cs);
	}

	virtual ~MediaLock()
	{
		DeleteCriticalSection(&cs);
	}

	void lock(char *dbgfile, int dbgline)
	{
		EnterCriticalSection(&cs);
		sprintf(dbginfo, "%s line:%d", dbgfile, dbgline);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
		memset(dbginfo, 0, sizeof(dbginfo));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Media Buffers

class MediaBuffer : public MediaLock	// generic Media Buffer Class
{
	int	m_type;
	int m_focus;

public:

	bool host_updated;		// host has updated value
	bool plug_updated;		// plugin has updated value

	enum 				// set and get if the host or the plugz put a focus on an IN|OUT buffer
	{
		focusNONE,		// no focus, host and plugin can read and write the value
		focusHOST,		// host focus, the plugin must only read the value
		focusPLUGIN,	// plugin focus, the host must only read the value
		focusQUERY		// query current focus
	};
	

	MediaBuffer(int type=MPmediaUNKNOWN)
	{ 
		m_type=type; 
		m_focus=focusNONE;
		host_updated=false;
		plug_updated=false;
	}

	virtual ~MediaBuffer()
	{
	}
								
	int	getType()
	{ 
		return m_type; 
	}

	int focus(int ifocus=focusQUERY)
	{
		if(ifocus!=focusQUERY)
			m_focus=ifocus;
		return m_focus;
	}
	
	virtual MediaBuffer * clone()
	{
		return new MediaBuffer(MPmediaUNKNOWN);
	}
	
	virtual bool copyTo(MediaBuffer *media)
	{ 
		if((m_type&MPmaskTYPE)!=(media->m_type&MPmaskTYPE))
			return false; 
		return true; 
	}

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MediaInteger

class MediaInteger : public MediaBuffer
{
	int	m_value;

public:
	MediaInteger() : MediaBuffer(MPmediaINTEGER)
	{ 
		m_value=0;
	}
	
	MediaInteger(int value) : MediaBuffer(MPmediaINTEGER)
	{
		m_value=value;
	}
	
	MediaInteger(MediaInfo mi) : MediaBuffer(MPmediaINTEGER)
	{
		m_value=((MediaInteger *)mi.defvalue)->getValue();
	}
	
	int getValue()
	{ 
		return m_value; 
	}
	
	void setValue(int value)
	{ 
		m_value=value; 
	}

	virtual MediaBuffer * clone()
	{
		return new MediaInteger(m_value);
	}
	
	virtual bool copyTo(MediaBuffer *media)
	{ 
		if(!MediaBuffer::copyTo(media)) 
			return false; 
		((MediaInteger *)media)->m_value=m_value; 
		return true; 
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MediaFloat

class MediaFloat : public MediaBuffer
{
	double	m_value;

public:
	MediaFloat() : MediaBuffer(MPmediaFLOAT)
	{ 
		m_value=0.f;
	}
	
	MediaFloat(double value) : MediaBuffer(MPmediaFLOAT)
	{
		m_value=value;
	}
	
	MediaFloat(MediaInfo mi) : MediaBuffer(MPmediaFLOAT)
	{
		m_value=((MediaFloat *)mi.defvalue)->getValue();
	}

	double getValue()
	{ 
		return m_value; 
	}
	
	void setValue(double value)
	{ 
		m_value=value; 
	}

	virtual MediaBuffer * clone()
	{
		return new MediaFloat(m_value);
	}
	
	virtual bool copyTo(MediaBuffer *media)
	{ 
		if(!MediaBuffer::copyTo(media)) 
			return false; 
		((MediaFloat *)media)->m_value=m_value; 
		return true; 
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MediaBitmap

class MediaBitmap : public MediaBuffer
{
	MPdword	*m_pixels;
	int		m_width;
	int		m_height;
	float	m_ratio;

public:
	
	MediaBitmap(int width, int height, float ratio=1.f, int type=MPmediaBITMAP_XRGB) : MediaBuffer(type)
	{ 
		m_ratio=ratio; 
		m_pixels=NULL; 
		size(width, height); 
	}
	
	virtual	~MediaBitmap()	
	{ 
		if(m_pixels) 
			free(m_pixels); 
	}
	
	int	getWidth()	
	{ 
		return m_width; 
	}
	
	int	getHeight()
	{ 
		return m_height;
	}
	
	float getRatio()
	{
		return m_ratio; 
	}
	
	MPdword * getPixels()
	{
		return m_pixels;
	}
	
	void setRatio(float ratio)
	{
		m_ratio=ratio;
	}
	
	void size(int width, int height)
	{ 
		int	sz=sizeof(MPdword)*width*height;
		m_width=width; 
		m_height=height; 
		if(m_pixels) 
			free(m_pixels); 
		m_pixels=(MPdword *)malloc(sz);
		memset(m_pixels, 0, sz);
	}

	virtual MediaBuffer * clone()
	{
		MediaBitmap *mb=new MediaBitmap(m_width, m_height, m_ratio, getType());
		copyTo(mb);
		return mb;
	}

	virtual bool copyTo(MediaBuffer *media)
	{ 
		if(!MediaBuffer::copyTo(media)) 
			return false; 
		if(!((((MediaBitmap *)media)->m_width==m_width)&&(((MediaBitmap *)media)->m_width==m_width))) 
			return false; 
		memcpy(((MediaBitmap *)media)->m_pixels, m_pixels, m_width*m_height*sizeof(int));
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MediaSelect

class MediaSelect : public MediaBuffer
{
	int		m_value;	// m_value range [0..m_maxvalue-1]
	int		m_maxvalue;	
	char	*m_desc[MPmediaSELECT_MAXVALUE];

public:
	MediaSelect(int media=MPmediaSELECT) : MediaBuffer(media)
	{ 
		m_value=0;
		m_maxvalue=0;
		memset(m_desc, 0, sizeof(m_desc));
	}
	
	MediaSelect(int value, int maxvalue, int media=MPmediaSELECT) : MediaBuffer(media)
	{
		m_value=value;
		m_maxvalue=maxvalue;
		memset(m_desc, 0, sizeof(m_desc));
	}

	MediaSelect(MediaInfo mi) : MediaBuffer(mi.type&MPmaskTYPE)
	{
		m_value=((MediaInteger *)mi.defvalue)->getValue();
		m_maxvalue=((MediaInteger *)mi.maxvalue)->getValue();
		memset(m_desc, 0, sizeof(m_desc));
	}

	virtual ~MediaSelect()
	{
		int i;
		for(i=0; i<MPmediaSELECT_MAXVALUE; i++)
		{
			if(m_desc[i])
				free(m_desc[i]);
		}
	}
	
	int getValue()
	{ 
		return m_value; 
	}
	
	void setValue(int value)
	{ 
		m_value=value; 
	}

	virtual MediaBuffer * clone()
	{
		MediaSelect	*ms=new MediaSelect();
		if(copyTo(ms))
			return ms;
		delete(ms);
		return NULL;
	}
	
	virtual bool copyTo(MediaBuffer *media)
	{ 
		if(!MediaBuffer::copyTo(media)) 
			return false; 
		((MediaSelect *)media)->m_value=m_value; 
		((MediaSelect *)media)->m_maxvalue=m_maxvalue;
		{
			int i;
			for(i=0; i<MPmediaSELECT_MAXVALUE; i++)
				((MediaSelect *)media)->setDesc(i, m_desc[i]);
		}
		return true; 
	}

	void setDesc(int n, char *desc)
	{
		if(m_desc[n])
		{
			free(m_desc[n]);
			m_desc[n]=NULL;
		}
		if(desc)
			m_desc[n]=_strdup(desc);
	}

	char * getDesc(int n)
	{
		return m_desc[n];
	}

	int getMaxValue()
	{
		return m_maxvalue;
	}

	void setMaxValue(int mv)
	{
		m_maxvalue=mv;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MediaSelectView

class MediaSelectView : public MediaSelect
{
	MediaBitmap *m_view[MPmediaSELECT_MAXVALUE];

public:
	MediaSelectView(int media=MPmediaSELECTVIEW) : MediaSelect(media)
	{
		memset(m_view, 0, sizeof(m_view));
	}
	
	MediaSelectView(int value, int maxvalue, int media=MPmediaSELECTVIEW) : MediaSelect(value, maxvalue, media)
	{
		memset(m_view, 0, sizeof(m_view));
	}

	MediaSelectView(int value, int maxvalue, int width, int height, int media=MPmediaSELECTVIEW) : MediaSelect(value, maxvalue, media)
	{
		int i;
		memset(m_view, 0, sizeof(m_view));
		for(i=0; i<maxvalue+1; i++)
			m_view[i]=new MediaBitmap(width, height);
	}

	virtual ~MediaSelectView()
	{
		int i;
		for(i=0; i<MPmediaSELECT_MAXVALUE; i++)
		{
			if(m_view[i])
				delete(m_view[i]);
		}
	}
	
	virtual MediaBuffer * clone()
	{
		MediaSelectView	*ms=new MediaSelectView();
		if(copyTo(ms))
			return ms;
		delete(ms);
		return NULL;
	}
	
	virtual bool copyTo(MediaBuffer *media)
	{ 
		if(!MediaSelect::copyTo(media))
			return false;
		{
			int i;
			for(i=0; i<MPmediaSELECT_MAXVALUE; i++)
				((MediaSelectView *)media)->setView(i, m_view[i]);
		}
		return true; 
	}

	void setView(int n, MediaBitmap *src)
	{
		if(m_view[n])
		{
			if(!src->copyTo(m_view[n]))
			{
				delete(m_view[n]);
				m_view[n]=(MediaBitmap *)src->clone();
			}
		}
		else 
			m_view[n]=(MediaBitmap *)src->clone();
	}

	MediaBitmap * getView(int n)
	{
		return m_view[n];
	}

	void size(int width, int height)
	{
		int i;
		int max=getMaxValue()+1;
		for(i=0; i<max; i++)
			if(m_view[i])
			{
				m_view[i]->lock(__FILE__,__LINE__);
				m_view[i]->size(width, height);
				m_view[i]->unlock();
			}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// plugin interface

class MediaHost
{
public:
								MediaHost					() {}
	virtual						~MediaHost					() {}

	virtual double				getBPM						() { return MPBPMREF; }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// plugin interface

class MediaPlugin
{
public:
	MediaHost					*host;

								MediaPlugin					(MediaHost *host) { this->host=host; }								// module constructor
	virtual						~MediaPlugin				() {}								// module destructor

	virtual bool				start						() { return true; }					// run the plugin, called after all links have been done, return true if successfully started
	virtual bool				process						(double time, double beat)=0;		// process, time is in seconds, beat in count of beats from the beginning, host can seek in time
	virtual void				stop						() { }								// stop the plugin

	// the MediaBuffers are allocated by the host, the plugin must store the MediaBuffer pointer

	virtual void				link						(int pin, MediaBuffer *buffer)=0;	

	// if a funtion return false the host can call the getError function to fill the error string
	virtual void				getError					(char *error)=0;					// return an error string (error: zero terminal string, max size MPSTRINGSIZE)
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// plugin info interface

class MediaPluginInfo
{
public:
								MediaPluginInfo				() {}
	virtual						~MediaPluginInfo			() {}
	
	virtual bool				getName						(char *name)=0;						// return plugin name (lenght max: MPSTRINGSIZE)
	virtual MPqword				getGUID						()=0;								// return the GUID of the plugin module (64 bits)
	
	virtual int					getGroup					()=0;								// return plugin group
	virtual int					getPriority					()=0;								// module priority in the modules chain

	virtual int					getMediaCount				()=0;								// return the number of media pin
	virtual MediaInfo			getMediaInfo				(int pin)=0;						// return a MediaInfo structure

	virtual MediaPlugin *		getNewInstance				(MediaHost *host)=0;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DLL entry point

typedef int						MPgetMediaPluginVersion		();									// return the OMPP version

// use one of the two entries points (the first if there is only one plugin in the DLL, 
// the second if there are more plugins) you can use the second entry to develop a wrapper
// on other media plugin system

typedef MediaPluginInfo *		MPgetMediaPluginInfo		();
typedef int						MPgetMediaPluginInfos		(MediaPluginInfo **ptr);			// NULL as param return nb PluginInfo otherwise return an array of PluginInfo pointers


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// inlines

__inline MediaInfo::MediaInfo(MediaInfo &mi)
{
	type=mi.type;
	guid=mi.guid;
	memcpy(name, mi.name, sizeof(name));
	memcpy(help, mi.help, sizeof(help));
	defvalue=NULL;
	minvalue=NULL;
	maxvalue=NULL;
	if(mi.defvalue)
		defvalue=mi.defvalue->clone();
	if(mi.minvalue)
		minvalue=mi.minvalue->clone();
	if(mi.maxvalue)
		maxvalue=mi.maxvalue->clone();
}

__inline void MediaInfo::operator=(MediaInfo &mi)
{
	type=mi.type;
	guid=mi.guid;
	memcpy(name, mi.name, sizeof(name));
	memcpy(help, mi.help, sizeof(help));
	defvalue=NULL;
	minvalue=NULL;
	maxvalue=NULL;
	if(mi.defvalue)
		defvalue=mi.defvalue->clone();
	if(mi.minvalue)
		minvalue=mi.minvalue->clone();
	if(mi.maxvalue)
		maxvalue=mi.maxvalue->clone();
}

__inline MediaInfo::~MediaInfo()
{
	if(defvalue)
		delete(defvalue);
	if(minvalue)
		delete(minvalue);
	if(maxvalue)
		delete(maxvalue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helpers

__inline void MPcolorRGB(MPbyte *r, MPbyte *g, MPbyte *b, MPdword color)
{
	*r=(MPbyte)(color>>16);
	*g=(MPbyte)(color>>8);
	*b=(MPbyte)(color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline void MPcolorRGBA(MPbyte *r, MPbyte *g, MPbyte *b, MPbyte *a, MPdword color)
{
	*a=(MPbyte)(color>>24);
	*r=(MPbyte)(color>>16);
	*g=(MPbyte)(color>>8);
	*b=(MPbyte)(color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline MPdword MPcolor32(MPbyte r, MPbyte g, MPbyte b, MPbyte a=0xff)
{
	return ((MPdword)a<<24)|((MPdword)r<<16)|((MPdword)g<<8)|(MPdword)b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline MPbyte MPsature8(int v)
{
	if(v<0)
		return 0;
	if(v>255)
		return 255;
	return v;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline MPdword MPsature32(int r, int g, int b, int a=255)
{
	return MPcolor32(MPsature8(r), MPsature8(g), MPsature8(b), MPsature8(a));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif	// _OPEN_MEDIA_PLUGIN_PLATFORM_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
