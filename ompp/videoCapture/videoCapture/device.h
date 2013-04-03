#pragma once
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DEVICE.H					(c)	YoY'05						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include						<dshow.h>
#include						<xprtdefs.h>
#include						<dbt.h>
#include						<math.h>
#include						<assert.h>
#include						<Dvdmedia.h>
#include						"../../../src/alib/defSampleGrabber.h"
#include						"ompp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	CLSID						clsid;
	char						name[256];
	IMoniker					*moniker;
} Tdevice;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Asection
{
	CRITICAL_SECTION	cs;
public:

	Asection()
	{
		InitializeCriticalSection(&cs);
	}

	virtual ~Asection()
	{
		LeaveCriticalSection(&cs);
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Avidcap
{
public:


	enum
	{
								stateSTOP,
								statePAUSE,
								statePLAY,
								stateNODEVICE
	};
	
	enum
	{
								dialogSOURCE				=VfwCaptureDialog_Source,
								dialogFORMAT				=VfwCaptureDialog_Format,
								dialogDISPLAY				=VfwCaptureDialog_Display
	};

	enum
	{
								standardNONE,
								standardNTSC_M, 
								standardNTSC_M_J,
								standardNTSC_433,
								standardPAL_B,
								standardPAL_D,
								standardPAL_H,
								standardPAL_I,
								standardPAL_M,
								standardPAL_N,
								standardPAL_60,
								standardSECAM_B,
								standardSECAM_D,
								standardSECAM_G,
								standardSECAM_H,
								standardSECAM_K,
								standardSECAM_K1,
								standardSECAM_L,
								standardSECAM_L1,
								standardPAL_N_COMBO,
								standardMAXIMUM
	};	
	
	enum
	{
								analogBRIGHTNESS=VideoProcAmp_Brightness,
								analogCONTRAST=VideoProcAmp_Contrast,
								analogHUE=VideoProcAmp_Hue,
								analogSATURATION=VideoProcAmp_Saturation,
								analogSHARPNESS=VideoProcAmp_Sharpness,
								analogGAMMA=VideoProcAmp_Gamma,
								analogCOLOR=VideoProcAmp_ColorEnable,
								analogWHITE=VideoProcAmp_WhiteBalance,
								analogBACKLIGHT=VideoProcAmp_BacklightCompensation,
								analogGAIN=VideoProcAmp_Gain,
								analogMAXIMUM
	};


								Avidcap						();
								~Avidcap					();
								
	void						enumDevices					();
	void						clearEnumDevices			();

	IBaseFilter *				createSourceInstance		(int n);
 
	
	bool						init						(int n, int width, int height, double fps, int bits);
	void						release						();
	
	bool						getBitmap					(MediaBitmap *b, bool deinterlace);
	void						deinterlace					(MediaBitmap *b);
	
	int							getState					();
	bool						start						();
	void						stop						();
	
	bool						dialog						(HWND hw, int n);
	bool						isdialog					(int n);
	
	int							getAnalogVideoStandardInfo	();
	int							getAnalogVideoStandard		();
	void						setAnalogVideoStandard		(int f);
	
	int							getAnalogSourceInfo			(int in=-1, char *name=NULL);	// getAnalogSourceInfo() return the number of sources otherwise return the pin physical type (physicalVIDEO or physicalAUDIO)
	int							getAnalogSource				();
	bool						setAnalogSource				(int in);

	float						getAnalogSetting			(int set);
	bool						setAnalogSetting			(int set, float value);
	
	IGraphBuilder				*pGraph;
	ICaptureGraphBuilder2		*pBuild;
	IMediaControl				*pMC;
	class IcaptureCB			*captureCB;
	ISampleGrabber				*pGrabVideo;
	IMediaEvent					*pME;

//	IAMDroppedFrames			*pDF;
//	IAMExtTransport				*pET;
//	IAMCameraControl			*pCC;

	IAMVideoProcAmp				*pVPA;
	IAMAnalogVideoDecoder		*pVDEC;
	IAMCrossbar					*pCROSS;
	
	IAMVfwCaptureDialogs		*pDLG;
	
	Asection					section;
	Tdevice						devices[128];
	int							nbdevices;
	char						*error;
	bool						ok;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
