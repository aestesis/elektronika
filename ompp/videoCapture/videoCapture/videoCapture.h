// forms.h : main header file for the forms DLL
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"device.h"
#include						"ompp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	outVIDEO=0,
	inDEVICE,
	inSOURCE,
	pinNB
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

class VideoCapture : MediaPlugin  
{
public:
								VideoCapture				(MediaHost *mhost);					// module constructor
	virtual						~VideoCapture				();									// module destructor

	virtual bool				process						(double time, double beat);			// process, time is in seconds, beat in count of beats from the beginning
	virtual bool				start						();
	virtual void				stop						();

	// the MediaBuffers are allocated by the host, the client can store the MediaBuffer pointer

	virtual void				link						(int pin, MediaBuffer *media);		// link media to pin, the client must store the pointer on the MediaBuffer class allocated by the host

	// if a funtion return false the server can call the getError function to fill the error string
	virtual void				getError					(char *error);						// return an error string (error zero terminal string, max size MPSTRINGSIZE)

	// helpers

	void						initDeviceList				();

	// data
	
	MediaBuffer					*media[pinNB];
	class Avidcap				*vidcap;
	double						fps;
	int							ndevice;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

class VideoCaptureInfo : MediaPluginInfo
{
public:
								VideoCaptureInfo			();
	virtual						~VideoCaptureInfo			();
	
	virtual bool				getName						(char *name);						// return plugin name
	virtual MPqword				getGUID						();									// return the GUID (64 bits)
	
	virtual int					getGroup					();									// return plugin group
	virtual int					getPriority					();									// module priority in the modules chain

	virtual int					getMediaCount				();									// return the number of IN parameter
	virtual MediaInfo			getMediaInfo				(int n);							// return MediaInfo
	
	virtual MediaPlugin *		getNewInstance				(MediaHost *mhost);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
