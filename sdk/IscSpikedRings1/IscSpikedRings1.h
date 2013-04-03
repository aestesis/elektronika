///////////////////////////////////////////////////////////////////////////////////
// FreeFrame.h
//
// FreeFrame Open Video Plugin Prototype
// ANSI C Version

// www.freeframe.org
// marcus@freeframe.org

/*

Copyright (c) 2002, Marcus Clements www.freeframe.org
All rights reserved.

FreeFrame 1.0 upgrade by Russell Blakeborough
email: boblists@brightonart.org

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of FreeFrame nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __IscSpikedRings1_H__
#define __IscSpikedRings1_H__

//////////////////////////////////////////////////////////////////////////////////
//
// includes
//

#include "FreeFrame.h"
//#include "Metaballs.h"
#include "timer.h"
//#include "MarchingCubes.h"
//#include "graphics.h"
//#include "window.h"

///////////////////////////////////////////////////////////////////////////////////////////
//
// implementation specific definitions
//


typedef struct ParamConstsStructTag {
	float defaultValue;
	char name[16];
} ParamConstantsStruct;

typedef struct ParamDynamicDataStructTag {
	float value;
	char displayValue[16];
} ParamDynamicDataStruct;

typedef struct VideoPixel24bitTag {
	BYTE red;
	BYTE green;
	BYTE blue;
} VideoPixel24bit;

typedef struct VideoPixel16bitTag {
	BYTE fb;
	BYTE sb;
} VideoPixel16bit;

typedef struct VideoPixel32bitTag {
	BYTE blue;
	BYTE green;
	BYTE red;
	BYTE alpha;
} VideoPixel32bit;

typedef struct PixelSrcStructTag {
	double SrcX;
	double SrcY;
	float Shade; // for shadows
} PixelSrcStruct;


// Russell - PluginInstance Object - these calls relate to instances of plugObj
// created by FF_INSTANTIATE

class plugClass {

public:

	
	///////////////////////////////////////////////////////////////////////////////////////
	//
	// Function prototypes - Instance Specific Plugin Functions inside the instance object


	///////////////////////////////////////////////////////////////////////////////////////
	// getParameterDisplay
	//
	// returns pointer to array of 16 char containing a string to display as the value of
	// parameter index
	//
	// parameters:
	// DWORD index - index of parameter 
	//
	// return values (32-bit pointer to char):
	// 32-bit pointer to array of char
	// FF_FAIL on error
	//

	char*	getParameterDisplay(DWORD index);			

	///////////////////////////////////////////////////////////////////////////////////////
	// setParameter
	//
	// Sets the value of parameter specified by index
	// value is a 32-bit float 0<=value<=1
	//
	// parameters:
	// DWORD index - index of parameter 
	// 32-bit float value
	//
	// return values (DWORD):
	// FF_FAIL on error
	//

	DWORD	setParameter(SetParameterStruct* pParam);		

	///////////////////////////////////////////////////////////////////////////////////////
	// getParameter
	//
	// returns value of parameter specified by index as 32-bit float 0<=value<=1
	//
	// parameters:
	// DWORD index - index of parameter 
	//
	// return values (32-bit float):
	// 32-bit float value
	// FF_FAIL on error
	//

	float	getParameter(DWORD index);					

	///////////////////////////////////////////////////////////////////////////////////////
	// processFrame
	//
	// process a frame of video 'in place' 
	//
	// parameters:
	// 32-bit pointer to byte array containing frame of video
	//
	// return values (DWORD):
	// FF_SUCCESS
	// FF_FAIL on error
	//
	// HOST: pFrame needs to be a valid pointer throughout this call as the plugin processes
	// the frame 'in place'.

	DWORD	processFrame(LPVOID pFrame);
	DWORD	processFrame24Bit(LPVOID pFrame);
	DWORD	processFrame32Bit(LPVOID pFrame);

	///////////////////////////////////////////////////////////////////////////////////////
	// processFrameCopy
	//
	// processes a frame of video from one buffer to another 
	//
	// parameters:
	// 32-bit pointer to a structure containing a pointer to an array of input
	// buffers, the number of input buffers, and a pointer to an output frame
	// buffer
	//
	// return values (DWORD):
	// FF_SUCCESS
	// FF_FAIL on error
	//
	DWORD	processFrameCopy(ProcessFrameCopyStruct* pFrameData);
	DWORD	processFrameCopy24Bit(ProcessFrameCopyStruct* pFrameData);
	DWORD	processFrameCopy32Bit(ProcessFrameCopyStruct* pFrameData);

	
	///////////////////////////////////////////////////////////////////////////////////////
	// parameters dynamic data 

	ParamDynamicDataStruct paramDynamicData[12];   // should be NUM_PARAMS - scope prob. presumably
	VideoPixel24bit* DestPix24;
	VideoPixel32bit* DestPix32;
//	PixelSrcStruct* WormHoleMap;
	double CosX1;
	double CosX2;
	double SinY1;
	double SinY2;
	double TransX;
	double TransY;
	VideoInfoStruct videoInfo;
	int vidmode;

	VideoPixel32bit *RenderedBuffer1;
	VideoPixel32bit *RenderedBuffer2;
	bool FirstPass;

	bool ShowBkgrSphere;

	int RotX;
	int RotY;
	int RotZ;

	int BSRotX;
	int BSRotY;
	int BSRotZ;

	int DelayedList[100][3];

	int *SinCosTab;

//	CMetaballs *m_pMetaballs;
	CTimer     *m_pTimer;
//	IDirect3DTexture8 *m_pTexture;
//	IDirect3DTexture8 *m_pDestTexture;
//	IDirect3DTexture8 *m_pCopiedTexture;
//	CWindow *m_pWindow;
//	CGraphics *pGfx;
//	IDirect3DSurface8* DTSSurf;
//	IDirect3DSurface8* DTSSurf2;
//	IDirect3DSurface8* DTSSurfCopied;

private:

	//////////////////////////////////////////////////////////////////////////////////
	// plugin instance data storage
	//
	// all variables for stream processing should be declared here
	// so that they automatically run instances of themselves withing the plugin object



};

///////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes - Global Plugin Functions that lie outside the instance object

///////////////////////////////////////////////////////////////////////////////////////
// getInfo
//
// gets information about the plugin - version, unique id, short name and type 
// This function should be identical in all future versions of the FreeFrame API
//  
// return values (32-bit pointer to PlugInfoStruct)
// FF_FAIL on error
// 32-bit pointer to PlugInfoStruct
//
// HOST: call this function first to get version information
//       the version defines the other fucntion codes that are supported
//       supported function codes are listed in the documentation www.freeframe.org

PlugInfoStruct*	getInfo();							

///////////////////////////////////////////////////////////////////////////////////////
// initialise
//
// Prepare the Plug-in for processing.  
// Set default values, allocate memory
// When the plug-in returns from this function it must be ready to proces a frame
//
// return values (DWORD)
// FF_SUCCESS - success
// non-zero - fail (error values to be defined)
//
// HOST:	This function *must* return before a call to instantiate.
// PLUGIN:  make a copy of the videoInfoStruct locally as pointer may not be valid after
//			function returns

DWORD	initialise();								

///////////////////////////////////////////////////////////////////////////////////////
// deinitialise
//
// Tidy up   
// Deallocate memory
//
// return values (DWORD)
// FF_SUCCESS - success
// non-zero - fail (error values to be defined)
//
// HOST: This *must* be the last function called on the plugin

DWORD	deInitialise();								

///////////////////////////////////////////////////////////////////////////////////////
// getNumParameters 
//
// returns number of parameters in plugin
// 
// return values (DWORD)
// number of parameters
// FF_FAIL on error
//

DWORD	getNumParameters();							

///////////////////////////////////////////////////////////////////////////////////////
// getParameterName
//
// returns pointer to 16 byte char array containing the name of parameter specified by index
//
// parameters:
// DWORD index - index of parameter 
//
// return values (32-bit pointer to char):
// 32-bit pointer to array of char
// FF_FAIL on error
//

char*	getParameterName(DWORD index);				

///////////////////////////////////////////////////////////////////////////////////////
// getParameterDefault
//
// returns default value of parameter specified by index as 32-bit float 0<=value<=1
//
// parameters:
// DWORD index - index of parameter 
//
// return values (32-bit float):
// 32-bit float value
// FF_FAIL on error
//

float	getParameterDefault(DWORD index);			


///////////////////////////////////////////////////////////////////////////////////////
// getpluginCaps
//
// returns true pof false to indicate whether cappable of feature specified by index
//
// parameters:
// DWORD index - index of parameter 
// allowed values:
// 0 - 16 bit video
// 1 - 24 bit video
// 2 - 32 bit video
//
// return values (DWORD):
// FF_TRUE
// FF_FALSE
//

DWORD	getPluginCaps(DWORD index);	

// Russell - new calls for FF 1.0 ......

///////////////////////////////////////////////////////////////////////////////////////
// instantiate
//
// Run up plugin instance - plugObj
// Prepare the Plug-in instance for processing.  
// Set default values, allocate memory
// When the plug-in returns from this function it must be ready to proces a frame
// Pointer to videoInfoStruct *must* be valid until function returns
//
// return values (pointer to a plugObj - FF Dword Instance ID)
// FF_FAIL
//
// HOST: This function *must* return before a call to processFrame

LPVOID instantiate(VideoInfoStruct* pVideoInfo);

///////////////////////////////////////////////////////////////////////////////////////
// deInstantiate
//
// Run down plugin instance
//
// Deallocate memory used by this instance

DWORD deInstantiate(LPVOID instanceID);	

///////////////////////////////////////////////////////////////////////////////////////
// getExtendedInfo
//

LPVOID getExtendedInfo();		

DWORD setDTSWin(HINSTANCE* hI);

#endif
