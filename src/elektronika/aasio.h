#pragma once
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aasio						An Asio MultipleInstance C++ Wrapper
// developer:					renan jegouzo 
// email:						renan@aestesis.org
// web:							aestesis.org 
// (C) 2005 AESTESIS.ORG
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include						<windows.h>
#include						<interface.h>
#include						"../../sdk/steinberg/ASIO SDK/asiosdk2/common/asiosys.h"
#include						"../../sdk/steinberg/ASIO SDK/asiosdk2/common/asio.h"
#include						"../../sdk/steinberg/ASIO SDK/asiosdk2/host/pc/asiolist.h"
#include						"../../sdk/steinberg/ASIO SDK/asiosdk2/common/iasiodrv.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MAXASIOINSTANCE					16
#define							MAXASIOCHANNEL					256

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AasioClient
{
public:
								AasioClient						();
	virtual						~AasioClient					();

	bool						asioCreate						(int device);
	void						asioRelease						();

	int							asioGetNumDev					();
	void						asioGetDriverName				(int index, char *name, int size);

	int							asioGetNbInput					();
	int							asioGetNbOutput					();
	int							asioGetFirstInput				();
	int							asioGetFirstOutput				();
	char *						asioGetChannelName				(int channel);

	bool						asioUseChannel					(int left, int right);
	void						asioReleaseChannel				();

	void						asioAddBuffer					(sword *buffer, int nsample);	// 16 bits 44100 hz interlaced stereo
	int							asioGetBufferSize				();

	void						asioControlPanel				();

	class AasioInstance			*asioDriver;
	int							asioLeftChannel;
	int							asioRightChannel;
	short						asioMemoBuf[2];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct AasioQuality
{
	int							mBufSize;
	int							mUsed;
	float						ratio;
	bool						stopped;
} AasioQuality;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct AasioInfo
{
	ASIODriverInfo				driverInfo;

	ASIOSampleRate				sampleRate;

	ASIOCallbacks				callbacks;

	long						nbInput;
	long						nbOutput;
	long						nbChannel;		// input+output

	ASIOChannelInfo				channelInfo[MAXASIOCHANNEL];
	ASIOBufferInfo				bufferInfo[MAXASIOCHANNEL];

	AasioQuality				quality[MAXASIOCHANNEL];

	// buffer
	long						minSize;
	long						maxSize;
	long						preferredSize;
	long						granularity;

	bool						postOutput;

	long						inputLatency;
	long						outputLatency;

	ASIOTime					time;

	double						nano;
	double						samples;
	double						tcSamples;

	unsigned long				sysRefTime;
} AasioInfo;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AasioInstance
{
public:

								AasioInstance					();
								~AasioInstance					();

	bool						create							(int device);
	void						release							(AasioClient *client);

	bool						useChannel						(AasioClient *client, int left, int right);
	void						releaseChannel					(AasioClient *client);

	void						addBuffer						(int left, int right, sword *buffer, int nsample, short *membuf);
	int							getBufferSize					(int channel);

	void						controlPanel					();

	// private

	void						stretchAudio					(sword *out, int nout, sword *in, int nin, short *membuf);
	void						setCallback						();

	void						bufferSwitch					(long index, ASIOBool processNow);
	ASIOTime *					bufferSwitchTimeInfo			(ASIOTime *timeInfo, long index, ASIOBool processNow);
	void						sampleRateChanged				(ASIOSampleRate sRate);
	long						asioMessages					(long selector, long value, void* message, double* opt);

	Asection					section;

	int							nbClient;

	int							device;
	IASIO 						*driver;
	AasioInfo					info;

	Abuffer						*channel[MAXASIOCHANNEL];
	AasioClient					*channelClient[MAXASIOCHANNEL];
	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
