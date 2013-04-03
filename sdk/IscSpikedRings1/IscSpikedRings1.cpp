///////////////////////////////////////////////////////////////////////////////////
// FreeFrameSample.cpp
//
// FreeFrame Open Video Plugin 
// C Version
//
// Implementation of the Free Frame sample plugin
//
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

///////////////////////////////////////////////////////////////////////////////////////////
//
// includes 
//

#include "IscSpikedRings1.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include "resource.h"

#include "math.h"

#define NUM_PARAMS 12
#define NUM_INPUTS 1

#ifndef M_PI 
#define M_PI 3.1415926535897932384626433832795
//#define M_PI 3.1416f
#endif

#ifndef M_PI2
#define M_PI2 M_PI * 2.0
#endif

#define MB_TEXSIZE_D 256.0
#define MB_TEXSIZE_DW 256


/////////////////////////////////////////////////////
//
// Plugin Globals
//
// these are defined globally in this sample plugin for simplicities sake
// Plugin developers should allocate memory from the heap for this stuff
//

PlugInfoStruct plugInfo;
PlugExtendedInfoStruct plugExtInfo;
ParamConstantsStruct paramConstants[12] = {{0.5f,"Rotation X"},{0.5f,"Rotation Y"},{0.5f,"Rotation Z"},{0.0f,"Ring 1 Rot X"},{0.0f,"Ring 1 Rot Y"},{0.0f,"Ring 1 Rot Z"},{0.0f,"Ring 2 Rot X"},{0.0f,"Ring 2 Rot Y"},{0.0f,"Ring 2 Rot Z"},{0.0f,"Ring 3 Rot X"},{0.0f,"Ring 3 Rot Y"},{0.0f,"Ring 3 Rot Z"}};
HINSTANCE hInstDTS;
DWORD DTSPosX;
DWORD DTSPosY;

WNDCLASSEX winClass; 


#include "wglext.h"

// WGL_ARB_extensions_string
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;

// WGL_ARB_pbuffer
PFNWGLCREATEPBUFFERARBPROC    wglCreatePbufferARB    = NULL;
PFNWGLGETPBUFFERDCARBPROC     wglGetPbufferDCARB     = NULL;
PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
PFNWGLDESTROYPBUFFERARBPROC   wglDestroyPbufferARB   = NULL;
PFNWGLQUERYPBUFFERARBPROC     wglQueryPbufferARB     = NULL;

// WGL_ARB_pixel_format
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

// WGL_ARB_render_texture
PFNWGLBINDTEXIMAGEARBPROC     wglBindTexImageARB     = NULL;
PFNWGLRELEASETEXIMAGEARBPROC  wglReleaseTexImageARB  = NULL;
PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttribARB = NULL;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
HWND   g_hWnd             = NULL;
HDC	   g_hDC              = NULL;
HGLRC  g_hRC              = NULL;
GLuint g_testTextureID    = -1;
GLuint g_dynamicTextureID = -1;

float  g_fSpinX_L = 0.0f;
float  g_fSpinY_L = 0.0f;
float  g_fSpinX_R = 0.0f;
float  g_fSpinY_R = 0.0f;



struct Vertex
{
    float tu, tv;
    float x, y, z;
};

struct Vertex2
{
    float tu, tv;
	float r,g,b,a;
	float nx,ny,nz;
    float x, y, z;
};



#include "sphere!!.txt"
#include "spikedring!!.txt"

#define SpikedRing1_VCnt 4494
#define SpikedRing2_VCnt 4494
#define SpikedRing3_VCnt 4446

#define BkgrSphere_VCnt 2880

Vertex2 SpikedRing1_Rotated[SpikedRing1_VCnt];
Vertex2 SpikedRing2_Rotated[SpikedRing2_VCnt];
Vertex2 SpikedRing3_Rotated[SpikedRing3_VCnt];

Vertex BkgrSphereRotated[BkgrSphere_VCnt];

// This little struct will help to organize our p-buffer's data
typedef struct {

    HPBUFFERARB hPBuffer;
    HDC         hDC;
    HGLRC       hRC;
    int         nWidth;
    int         nHeight;

} PBUFFER;

PBUFFER g_pbuffer;

const int PBUFFER_WIDTH  = 256;
const int PBUFFER_HEIGHT = 256;


//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
	static POINT ptLastMousePosit_L;
	static POINT ptCurrentMousePosit_L;
	static bool  bMousing_L;
	
	static POINT ptLastMousePosit_R;
	static POINT ptCurrentMousePosit_R;
	static bool  bMousing_R;

    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
			}
		}
        break;

		case WM_LBUTTONDOWN:
		{
			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x = LOWORD (lParam);
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y = HIWORD (lParam);
			bMousing_L = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing_L = false;
		}
		break;

		case WM_RBUTTONDOWN:
		{
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x = LOWORD (lParam);
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y = HIWORD (lParam);
			bMousing_R = true;
		}
		break;

		case WM_RBUTTONUP:
		{
			bMousing_R = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit_L.x = LOWORD (lParam);
			ptCurrentMousePosit_L.y = HIWORD (lParam);
			ptCurrentMousePosit_R.x = LOWORD (lParam);
			ptCurrentMousePosit_R.y = HIWORD (lParam);

			if( bMousing_L )
			{
				g_fSpinX_L -= (ptCurrentMousePosit_L.x - ptLastMousePosit_L.x);
				g_fSpinY_L -= (ptCurrentMousePosit_L.y - ptLastMousePosit_L.y);
			}
			
			if( bMousing_R )
			{
				g_fSpinX_R -= (ptCurrentMousePosit_R.x - ptLastMousePosit_R.x);
				g_fSpinY_R -= (ptCurrentMousePosit_R.y - ptLastMousePosit_R.y);
			}

			ptLastMousePosit_L.x = ptCurrentMousePosit_L.x;
            ptLastMousePosit_L.y = ptCurrentMousePosit_L.y;
			ptLastMousePosit_R.x = ptCurrentMousePosit_R.x;
            ptLastMousePosit_R.y = ptCurrentMousePosit_R.y;
		}
		break;
		
		case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 100.0);
		}
		break;
		
		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}
		break;

        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;
		
		default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////
//
// Sample Function implementation
//

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

PlugInfoStruct* getInfo() 
{
	plugInfo.APIMajorVersion = 1;		// number before decimal point in version nums
	plugInfo.APIMinorVersion = 000;		// this is the number after the decimal point
										// so version 0.511 has major num 0, minor num 501
	char ID[5] = "Iz2q";		 // this *must* be unique to your plugin 
								 // see www.freeframe.org for a list of ID's already taken
	char name[17] = "IscSpikedRings1";
	
	memcpy(plugInfo.uniqueID, ID, 4);
	memcpy(plugInfo.pluginName, name, 16);
	plugInfo.pluginType = FF_EFFECT;

	return &plugInfo;
}


///////////////////////////////////////////////////////////////////////////////////////
// initialise
//
// do nothing for now - plugin instantiate is where the init happens now


DWORD initialise()
{
	return FF_SUCCESS;
}

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

DWORD deInitialise()
{
	return FF_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////
// getNumParameters 
//
// returns number of parameters in plugin
// 
// return values (DWORD)
// number of parameters
// FF_FAIL on error
//

DWORD getNumParameters()
{
	return NUM_PARAMS;  
}

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

char* getParameterName(DWORD index)
{
	return paramConstants[index].name;
}


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

float getParameterDefault(DWORD index)
{
	return paramConstants[index].defaultValue;
}

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

char* plugClass::getParameterDisplay(DWORD index)
{
	// fill the array with spaces first
	for (int n=0; n<16; n++) {
		paramDynamicData[index].displayValue[n] = ' ';
	}
	sprintf(paramDynamicData[index].displayValue, "%f",paramDynamicData[index].value);
	return paramDynamicData[index].displayValue;
}

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

DWORD plugClass::setParameter(SetParameterStruct* pParam)
{
	paramDynamicData[pParam->index].value = pParam->value;
	return FF_SUCCESS;
}

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

float plugClass::getParameter(DWORD index)
{
	return paramDynamicData[index].value;
}

///////////////////////////////////////////////////////////////////////////////////////
// processFrame
//
// process a frame of video
//
// parameters:
// 32-bit pointer to byte array containing frame of video
//
// return values (DWORD):
// FF_SUCCESS
// FF_FAIL on error
//

DWORD plugClass::processFrame(LPVOID pFrame)
{
	switch (videoInfo.bitDepth) {
		case 1:
			return processFrame24Bit(pFrame);
		case 2:
			return processFrame32Bit(pFrame);
		default:
			return FF_FAIL;
	}

}

///////////////////////////////////////////////////////////////////////////////////////
// processFrame24Bit
//
// process a frame of 24 bit video
//
// parameters:
// 32-bit pointer to byte array containing frame of video
//
// return values (DWORD):
// FF_SUCCESS
// FF_FAIL on error
//

DWORD plugClass::processFrame24Bit(LPVOID pFrame)
{
	VideoPixel24bit* pPixel = (VideoPixel24bit*) pFrame;
	VideoPixel24bit* pPixelDest = (VideoPixel24bit*) DestPix24;

	return FF_SUCCESS;
}







///////////////////////////////////////////////////////////////////////////////////////
// processFrame32Bit
//
// process a frame of 32 bit video
//
// parameters:
// 32-bit pointer to byte array containing frame of video
//
// return values (DWORD):
// FF_SUCCESS
// FF_FAIL on error
//

DWORD plugClass::processFrame32Bit(LPVOID pFrame)
{
	VideoPixel32bit* pPixel = (VideoPixel32bit*) pFrame;
	//VideoPixel32bit* pPixelDest = (VideoPixel32bit*) DestPix32;

	int fW = (int)videoInfo.frameWidth;
	int fH = (int)videoInfo.frameHeight;
		
	DWORD dfW = (DWORD)fW;
	DWORD dfH = (DWORD)fH;

	double PosW = 0;
	double PosH = 0;

	//g_fSpinY_R -= 0.3f;
	//g_fSpinX_R -= 0.4f;

	int SizeX = 256;
	int SizeY = 256;

	int StepX = (fW << 16) / SizeX;
	int StepY = (fH << 16) / SizeY;

	int OffsY = 0;
	for (int py = 0; py < SizeY; py++) {
		int OffsX = 0;
		for (int px = 0; px < SizeX; px++) {
			int POffsX = OffsX >> 16;
			int POffsY = OffsY >> 16;
			RenderedBuffer1[(py * SizeX) + px] = pPixel[(POffsY * fW) + POffsX];
			OffsX += StepX;
		}
		OffsY += StepY;
	}

	// ------ UPDATE ROTATION ALL 3 RINGS

	// GLOBAL ROTATION:
	RotX += ((int)((paramDynamicData[0].value - 0.5f) * 200.0f) << 16) / 3600;
	RotY += ((int)((paramDynamicData[1].value - 0.5f) * 200.0f) << 16) / 3600;
	RotZ += ((int)((paramDynamicData[2].value - 0.5f) * 200.0f) << 16) / 3600;

	// RING 1 ROTATION
	int Rot1X = RotX + (int)((paramDynamicData[3].value - 0.5f) * 65536.0f);
	int Rot1Y = RotY + (int)((paramDynamicData[4].value - 0.5f) * 65536.0f);
	int Rot1Z = RotZ + (int)((paramDynamicData[5].value - 0.5f) * 65536.0f);

	while (Rot1X > 65535) {
		Rot1X -= 65536;
	}
	while (Rot1X < 0) {
		Rot1X += 65536;
	}
	while (Rot1Y > 65535) {
		Rot1Y -= 65536;
	}
	while (Rot1Y < 0) {
		Rot1Y += 65536;
	}
	while (Rot1Z > 65535) {
		Rot1Z -= 65536;
	}
	while (Rot1Z < 0) {
		Rot1Z += 65536;
	}

	// RING 2 ROTATION
	int Rot2X = RotX + (int)((paramDynamicData[6].value - 0.5f) * 65536.0f);
	int Rot2Y = RotY + (int)((paramDynamicData[7].value - 0.5f) * 65536.0f);
	int Rot2Z = RotZ + (int)((paramDynamicData[8].value - 0.5f) * 65536.0f);

	while (Rot2X > 65535) {
		Rot2X -= 65536;
	}
	while (Rot2X < 0) {
		Rot2X += 65536;
	}
	while (Rot2Y > 65535) {
		Rot2Y -= 65536;
	}
	while (Rot2Y < 0) {
		Rot2Y += 65536;
	}
	while (Rot2Z > 65535) {
		Rot2Z -= 65536;
	}
	while (Rot2Z < 0) {
		Rot2Z += 65536;
	}

	// RING 3 ROTATION
	int Rot3X = RotX + (int)((paramDynamicData[9].value - 0.5f) * 65536.0f);
	int Rot3Y = RotY + (int)((paramDynamicData[10].value - 0.5f) * 65536.0f);
	int Rot3Z = RotZ + (int)((paramDynamicData[11].value - 0.5f) * 65536.0f);

	while (Rot3X > 65535) {
		Rot3X -= 65536;
	}
	while (Rot3X < 0) {
		Rot3X += 65536;
	}
	while (Rot3Y > 65535) {
		Rot3Y -= 65536;
	}
	while (Rot3Y < 0) {
		Rot3Y += 65536;
	}
	while (Rot3Z > 65535) {
		Rot3Z -= 65536;
	}
	while (Rot3Z < 0) {
		Rot3Z += 65536;
	}

	// RING 1 ROTATE!!

	int AngX = Rot1X;
	int AngY = Rot1Y;
	int AngZ = Rot1Z;

	// rotate and morph coordinates
	for (int i = 0; i < SpikedRing1_VCnt; i++) {
		int px = (int)(SpikedRing[i].x * 1000.0f);
		int py = (int)(SpikedRing[i].y * 1000.0f);
		int pz = (int)(SpikedRing[i].z * 1000.0f);
		int pnx = (int)(SpikedRing[i].nx * 1000.0f);
		int pny = (int)(SpikedRing[i].ny * 1000.0f);
		int pnz = (int)(SpikedRing[i].nz * 1000.0f);

		int SinX = SinCosTab[(static_cast<WORD>(AngX) << 1)];
		int CosX = SinCosTab[(static_cast<WORD>(AngX) << 1) + 1];
		int SinY = SinCosTab[(static_cast<WORD>(AngY) << 1)];
		int CosY = SinCosTab[(static_cast<WORD>(AngY) << 1) + 1];
		int SinZ = SinCosTab[(static_cast<WORD>(AngZ) << 1)];
		int CosZ = SinCosTab[(static_cast<WORD>(AngZ) << 1) + 1];
		{
			// -------------------------------- calc new x rotation:
			//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
			//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
			register int Z_SinX = (SinX * pz) >> 16; // Y * Sin(Z angle)
			register int Y_SinX = (SinX * py) >> 16; // X * Sin(Z angle)
			register int Y_CosX = (CosX * py) >> 16; // X * Cos(Z angle)
			register int Z_CosX = (CosX * pz) >> 16; // Y * Cos(Z angle)
			py = (Y_CosX - Z_SinX);
			pz = (Z_CosX + Y_SinX);
		
		}
		{
			// -------------------------------- calc new y rotation:
			// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
			// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
			register int X_SinY = (SinY * px) >> 16; // Y * Sin(Z angle)
			register int Z_SinY = (SinY * pz) >> 16; // X * Sin(Z angle)
			register int Z_CosY = (CosY * pz) >> 16; // X * Cos(Z angle)
			register int X_CosY = (CosY * px) >> 16; // Y * Cos(Z angle)
			pz = (Z_CosY - X_SinY);
			px = (X_CosY + Z_SinY);

		}
		{
			// -------------------------------- calc new z rotation:
			// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
			// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
			register int Y_SinZ = (SinZ * py) >> 16; // Y * Sin(Z angle)
			register int X_SinZ = (SinZ * px) >> 16; // X * Sin(Z angle)
			register int X_CosZ = (CosZ * px) >> 16; // X * Cos(Z angle)
			register int Y_CosZ = (CosZ * py) >> 16; // Y * Cos(Z angle)
			px = (X_CosZ - Y_SinZ);
			py = (Y_CosZ + X_SinZ);
		}
		// NOW ROTATE NORMALS TOO:
		{
			// -------------------------------- calc new x rotation:
			//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
			//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
			register int Z_SinX = (SinX * pnz) >> 16; // Y * Sin(Z angle)
			register int Y_SinX = (SinX * pny) >> 16; // X * Sin(Z angle)
			register int Y_CosX = (CosX * pny) >> 16; // X * Cos(Z angle)
			register int Z_CosX = (CosX * pnz) >> 16; // Y * Cos(Z angle)
			pny = (Y_CosX - Z_SinX);
			pnz = (Z_CosX + Y_SinX);
		
		}
		{
			// -------------------------------- calc new y rotation:
			// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
			// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
			register int X_SinY = (SinY * pnx) >> 16; // Y * Sin(Z angle)
			register int Z_SinY = (SinY * pnz) >> 16; // X * Sin(Z angle)
			register int Z_CosY = (CosY * pnz) >> 16; // X * Cos(Z angle)
			register int X_CosY = (CosY * pnx) >> 16; // Y * Cos(Z angle)
			pnz = (Z_CosY - X_SinY);
			pnx = (X_CosY + Z_SinY);

		}
		{
			// -------------------------------- calc new z rotation:
			// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
			// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
			register int Y_SinZ = (SinZ * pny) >> 16; // Y * Sin(Z angle)
			register int X_SinZ = (SinZ * pnx) >> 16; // X * Sin(Z angle)
			register int X_CosZ = (CosZ * pnx) >> 16; // X * Cos(Z angle)
			register int Y_CosZ = (CosZ * pny) >> 16; // Y * Cos(Z angle)
			pnx = (X_CosZ - Y_SinZ);
			pny = (Y_CosZ + X_SinZ);
		}

		SpikedRing1_Rotated[i].tu = SpikedRing[i].tu;
		SpikedRing1_Rotated[i].tv = SpikedRing[i].tv;
		SpikedRing1_Rotated[i].nx = (float)pnx / 1000.0f;
		SpikedRing1_Rotated[i].ny = (float)pny / 1000.0f;
		SpikedRing1_Rotated[i].nz = (float)pnz / 1000.0f;
		SpikedRing1_Rotated[i].x = (float)px / 1000.0f;
		SpikedRing1_Rotated[i].y = (float)py / 1000.0f;
		SpikedRing1_Rotated[i].z = (float)pz / 1000.0f;
	}

	// RING 2 ROTATE!!:

	AngX = Rot2X;
	AngY = Rot2Y;
	AngZ = Rot2Z;

	// rotate and morph coordinates
	for (i = 0; i < SpikedRing2_VCnt; i++) {
		int px = (int)(SpikedRing[i].x * 700.0f);
		int py = (int)(SpikedRing[i].y * 700.0f);
		int pz = (int)(SpikedRing[i].z * 1250.0f);
		int pnx = (int)(SpikedRing[i].nx * 1000.0f);
		int pny = (int)(SpikedRing[i].ny * 1000.0f);
		int pnz = (int)(SpikedRing[i].nz * 1000.0f);

		int SinX = SinCosTab[(static_cast<WORD>(AngX) << 1)];
		int CosX = SinCosTab[(static_cast<WORD>(AngX) << 1) + 1];
		int SinY = SinCosTab[(static_cast<WORD>(AngY) << 1)];
		int CosY = SinCosTab[(static_cast<WORD>(AngY) << 1) + 1];
		int SinZ = SinCosTab[(static_cast<WORD>(AngZ) << 1)];
		int CosZ = SinCosTab[(static_cast<WORD>(AngZ) << 1) + 1];
		{
			// -------------------------------- calc new x rotation:
			//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
			//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
			register int Z_SinX = (SinX * pz) >> 16; // Y * Sin(Z angle)
			register int Y_SinX = (SinX * py) >> 16; // X * Sin(Z angle)
			register int Y_CosX = (CosX * py) >> 16; // X * Cos(Z angle)
			register int Z_CosX = (CosX * pz) >> 16; // Y * Cos(Z angle)
			py = (Y_CosX - Z_SinX);
			pz = (Z_CosX + Y_SinX);
		
		}
		{
			// -------------------------------- calc new y rotation:
			// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
			// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
			register int X_SinY = (SinY * px) >> 16; // Y * Sin(Z angle)
			register int Z_SinY = (SinY * pz) >> 16; // X * Sin(Z angle)
			register int Z_CosY = (CosY * pz) >> 16; // X * Cos(Z angle)
			register int X_CosY = (CosY * px) >> 16; // Y * Cos(Z angle)
			pz = (Z_CosY - X_SinY);
			px = (X_CosY + Z_SinY);

		}
		{
			// -------------------------------- calc new z rotation:
			// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
			// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
			register int Y_SinZ = (SinZ * py) >> 16; // Y * Sin(Z angle)
			register int X_SinZ = (SinZ * px) >> 16; // X * Sin(Z angle)
			register int X_CosZ = (CosZ * px) >> 16; // X * Cos(Z angle)
			register int Y_CosZ = (CosZ * py) >> 16; // Y * Cos(Z angle)
			px = (X_CosZ - Y_SinZ);
			py = (Y_CosZ + X_SinZ);
		}
		// NOW ROTATE NORMALS TOO:
		{
			// -------------------------------- calc new x rotation:
			//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
			//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
			register int Z_SinX = (SinX * pnz) >> 16; // Y * Sin(Z angle)
			register int Y_SinX = (SinX * pny) >> 16; // X * Sin(Z angle)
			register int Y_CosX = (CosX * pny) >> 16; // X * Cos(Z angle)
			register int Z_CosX = (CosX * pnz) >> 16; // Y * Cos(Z angle)
			pny = (Y_CosX - Z_SinX);
			pnz = (Z_CosX + Y_SinX);
		
		}
		{
			// -------------------------------- calc new y rotation:
			// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
			// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
			register int X_SinY = (SinY * pnx) >> 16; // Y * Sin(Z angle)
			register int Z_SinY = (SinY * pnz) >> 16; // X * Sin(Z angle)
			register int Z_CosY = (CosY * pnz) >> 16; // X * Cos(Z angle)
			register int X_CosY = (CosY * pnx) >> 16; // Y * Cos(Z angle)
			pnz = (Z_CosY - X_SinY);
			pnx = (X_CosY + Z_SinY);

		}
		{
			// -------------------------------- calc new z rotation:
			// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
			// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
			register int Y_SinZ = (SinZ * pny) >> 16; // Y * Sin(Z angle)
			register int X_SinZ = (SinZ * pnx) >> 16; // X * Sin(Z angle)
			register int X_CosZ = (CosZ * pnx) >> 16; // X * Cos(Z angle)
			register int Y_CosZ = (CosZ * pny) >> 16; // Y * Cos(Z angle)
			pnx = (X_CosZ - Y_SinZ);
			pny = (Y_CosZ + X_SinZ);
		}

		SpikedRing2_Rotated[i].tu = SpikedRing[i].tu;
		SpikedRing2_Rotated[i].tv = SpikedRing[i].tv;
		SpikedRing2_Rotated[i].nx = (float)pnx / 1000.0f;
		SpikedRing2_Rotated[i].ny = (float)pny / 1000.0f;
		SpikedRing2_Rotated[i].nz = (float)pnz / 1000.0f;
		SpikedRing2_Rotated[i].x = (float)px / 1000.0f;
		SpikedRing2_Rotated[i].y = (float)py / 1000.0f;
		SpikedRing2_Rotated[i].z = (float)pz / 1000.0f;
	}

	// RING 3 ROTATE!!:

	AngX = Rot3X;
	AngY = Rot3Y;
	AngZ = Rot3Z;

	// rotate and morph coordinates
	for (i = 0; i < SpikedRing3_VCnt; i++) {
		int px = (int)(SpikedRing[i].x * 500.0f);
		int py = (int)(SpikedRing[i].y * 500.0f);
		int pz = (int)(SpikedRing[i].z * 1500.0f);
		int pnx = (int)(SpikedRing[i].nx * 1000.0f);
		int pny = (int)(SpikedRing[i].ny * 1000.0f);
		int pnz = (int)(SpikedRing[i].nz * 1000.0f);

		int SinX = SinCosTab[(static_cast<WORD>(AngX) << 1)];
		int CosX = SinCosTab[(static_cast<WORD>(AngX) << 1) + 1];
		int SinY = SinCosTab[(static_cast<WORD>(AngY) << 1)];
		int CosY = SinCosTab[(static_cast<WORD>(AngY) << 1) + 1];
		int SinZ = SinCosTab[(static_cast<WORD>(AngZ) << 1)];
		int CosZ = SinCosTab[(static_cast<WORD>(AngZ) << 1) + 1];
		{
			// -------------------------------- calc new x rotation:
			//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
			//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
			register int Z_SinX = (SinX * pz) >> 16; // Y * Sin(Z angle)
			register int Y_SinX = (SinX * py) >> 16; // X * Sin(Z angle)
			register int Y_CosX = (CosX * py) >> 16; // X * Cos(Z angle)
			register int Z_CosX = (CosX * pz) >> 16; // Y * Cos(Z angle)
			py = (Y_CosX - Z_SinX);
			pz = (Z_CosX + Y_SinX);
		
		}
		{
			// -------------------------------- calc new y rotation:
			// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
			// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
			register int X_SinY = (SinY * px) >> 16; // Y * Sin(Z angle)
			register int Z_SinY = (SinY * pz) >> 16; // X * Sin(Z angle)
			register int Z_CosY = (CosY * pz) >> 16; // X * Cos(Z angle)
			register int X_CosY = (CosY * px) >> 16; // Y * Cos(Z angle)
			pz = (Z_CosY - X_SinY);
			px = (X_CosY + Z_SinY);

		}
		{
			// -------------------------------- calc new z rotation:
			// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
			// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
			register int Y_SinZ = (SinZ * py) >> 16; // Y * Sin(Z angle)
			register int X_SinZ = (SinZ * px) >> 16; // X * Sin(Z angle)
			register int X_CosZ = (CosZ * px) >> 16; // X * Cos(Z angle)
			register int Y_CosZ = (CosZ * py) >> 16; // Y * Cos(Z angle)
			px = (X_CosZ - Y_SinZ);
			py = (Y_CosZ + X_SinZ);
		}
		// NOW ROTATE NORMALS TOO:
		{
			// -------------------------------- calc new x rotation:
			//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
			//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
			register int Z_SinX = (SinX * pnz) >> 16; // Y * Sin(Z angle)
			register int Y_SinX = (SinX * pny) >> 16; // X * Sin(Z angle)
			register int Y_CosX = (CosX * pny) >> 16; // X * Cos(Z angle)
			register int Z_CosX = (CosX * pnz) >> 16; // Y * Cos(Z angle)
			pny = (Y_CosX - Z_SinX);
			pnz = (Z_CosX + Y_SinX);
		
		}
		{
			// -------------------------------- calc new y rotation:
			// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
			// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
			register int X_SinY = (SinY * pnx) >> 16; // Y * Sin(Z angle)
			register int Z_SinY = (SinY * pnz) >> 16; // X * Sin(Z angle)
			register int Z_CosY = (CosY * pnz) >> 16; // X * Cos(Z angle)
			register int X_CosY = (CosY * pnx) >> 16; // Y * Cos(Z angle)
			pnz = (Z_CosY - X_SinY);
			pnx = (X_CosY + Z_SinY);

		}
		{
			// -------------------------------- calc new z rotation:
			// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
			// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
			register int Y_SinZ = (SinZ * pny) >> 16; // Y * Sin(Z angle)
			register int X_SinZ = (SinZ * pnx) >> 16; // X * Sin(Z angle)
			register int X_CosZ = (CosZ * pnx) >> 16; // X * Cos(Z angle)
			register int Y_CosZ = (CosZ * pny) >> 16; // Y * Cos(Z angle)
			pnx = (X_CosZ - Y_SinZ);
			pny = (Y_CosZ + X_SinZ);
		}

		SpikedRing3_Rotated[i].tu = SpikedRing[i].tu;
		SpikedRing3_Rotated[i].tv = SpikedRing[i].tv;
		SpikedRing3_Rotated[i].nx = (float)pnx / 1000.0f;
		SpikedRing3_Rotated[i].ny = (float)pny / 1000.0f;
		SpikedRing3_Rotated[i].nz = (float)pnz / 1000.0f;
		SpikedRing3_Rotated[i].x = (float)px / 1000.0f;
		SpikedRing3_Rotated[i].y = (float)py / 1000.0f;
		SpikedRing3_Rotated[i].z = (float)pz / 1000.0f;
	}

	ShowBkgrSphere = true; //(paramDynamicData[9].value > 0.5f);


	// ------ UPDATE ROTATION BACKGROUND SPHERE

	BSRotX += ((int)(paramDynamicData[1].value * -17.0f) << 16) / 3600;
	BSRotY += ((int)(paramDynamicData[2].value * -13.0f) << 16) / 3600;
	BSRotZ += ((int)(paramDynamicData[0].value * -8.0f) << 16) / 3600;

	if (BSRotX > 65535) {
		BSRotX -= 65536;
	}
	if (BSRotX < 0) {
		BSRotX += 65536;
	}
	if (BSRotY > 65535) {
		BSRotY -= 65536;
	}
	if (BSRotY < 0) {
		BSRotY += 65536;
	}
	if (BSRotZ > 65535) {
		BSRotZ -= 65536;
	}
	if (BSRotZ < 0) {
		BSRotZ += 65536;
	}

	if (ShowBkgrSphere) {

		int AngX = BSRotX;
		int AngY = BSRotY;
		int AngZ = BSRotZ;

		// rotate and morph coordinates
		for (int i = 0; i < BkgrSphere_VCnt; i++) {
			int px = (int)(BkgrSphere[i].x * 1000.0f);
			int py = (int)(BkgrSphere[i].y * 1000.0f);
			int pz = (int)(BkgrSphere[i].z * 1000.0f);

			int SinX = SinCosTab[(static_cast<WORD>(AngX) << 1)];
			int CosX = SinCosTab[(static_cast<WORD>(AngX) << 1) + 1];
			int SinY = SinCosTab[(static_cast<WORD>(AngY) << 1)];
			int CosY = SinCosTab[(static_cast<WORD>(AngY) << 1) + 1];
			int SinZ = SinCosTab[(static_cast<WORD>(AngZ) << 1)];
			int CosZ = SinCosTab[(static_cast<WORD>(AngZ) << 1) + 1];
			{
				// -------------------------------- calc new x rotation:
				//	NewY = (OldY*Cos(ThetaX)) - (OldZ*Sin(ThetaX))
				//	NewZ = (OldZ*Cos(ThetaX)) + (OldY*Sin(ThetaX))
				register int Z_SinX = (SinX * pz) >> 16; // Y * Sin(Z angle)
				register int Y_SinX = (SinX * py) >> 16; // X * Sin(Z angle)
				register int Y_CosX = (CosX * py) >> 16; // X * Cos(Z angle)
				register int Z_CosX = (CosX * pz) >> 16; // Y * Cos(Z angle)
				py = (Y_CosX - Z_SinX);
				pz = (Z_CosX + Y_SinX);
			
			}
			{
				// -------------------------------- calc new y rotation:
				// NewZ = (OldZ*Cos(ThetaY)) - (OldX*Sin(ThetaY))
				// NewX = (OldX*Cos(ThetaY)) + (OldZ*Sin(ThetaY))
				register int X_SinY = (SinY * px) >> 16; // Y * Sin(Z angle)
				register int Z_SinY = (SinY * pz) >> 16; // X * Sin(Z angle)
				register int Z_CosY = (CosY * pz) >> 16; // X * Cos(Z angle)
				register int X_CosY = (CosY * px) >> 16; // Y * Cos(Z angle)
				pz = (Z_CosY - X_SinY);
				px = (X_CosY + Z_SinY);

			}
			{
				// -------------------------------- calc new z rotation:
				// NewX = (OldX*Cos(ThetaZ)) - (OldY*Sin(ThetaZ))
				// NewY = (OldY*Cos(ThetaZ)) + (OldX*Sin(ThetaZ))
				register int Y_SinZ = (SinZ * py) >> 16; // Y * Sin(Z angle)
				register int X_SinZ = (SinZ * px) >> 16; // X * Sin(Z angle)
				register int X_CosZ = (CosZ * px) >> 16; // X * Cos(Z angle)
				register int Y_CosZ = (CosZ * py) >> 16; // Y * Cos(Z angle)
				px = (X_CosZ - Y_SinZ);
				py = (Y_CosZ + X_SinZ);
			}
			BkgrSphereRotated[i].tu = BkgrSphere[i].tu;
			BkgrSphereRotated[i].tv = BkgrSphere[i].tv;
			BkgrSphereRotated[i].x = (float)px / 1000.0f;
			BkgrSphereRotated[i].y = (float)py / 1000.0f;
			BkgrSphereRotated[i].z = (float)pz / 1000.0f;
		}
	}

	// ------ START RENDER

    int flag = 0;
    wglQueryPbufferARB( g_pbuffer.hPBuffer, WGL_PBUFFER_LOST_ARB, &flag );

    if( flag != 0 )
    {
        MessageBox(NULL,"The p-buffer was lost!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        //exit(-1);
    }

	if( !wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC ) )
	{
		MessageBox(NULL,"Could not make the p-buffer's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        //exit(-1);
    }

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//
	// Let the user spin the cube about with the right mouse button, so our 
	// dynamic texture will show motion.
	//

	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -5.0f );
    glRotatef( -g_fSpinY_R, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX_R, 0.0f, 1.0f, 0.0f );

	//
	// Now the render the cube to the p-buffer just like you we would have 
	// done with the regular window.
	//

    glBindTexture( GL_TEXTURE_2D, g_testTextureID );

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	glTexImage2D( GL_TEXTURE_2D, 0, 4, SizeX, SizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, (BYTE*)RenderedBuffer1);

	if (ShowBkgrSphere) {
		glInterleavedArrays( GL_T2F_V3F, 0, BkgrSphereRotated );
		glDrawArrays( GL_TRIANGLES, 0, BkgrSphere_VCnt );
	}

	glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, SpikedRing3_Rotated );
    glDrawArrays( GL_TRIANGLES, 0, SpikedRing3_VCnt );

	glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, SpikedRing2_Rotated );
    glDrawArrays( GL_TRIANGLES, 0, SpikedRing2_VCnt );

	glInterleavedArrays( GL_T2F_C4F_N3F_V3F, 0, SpikedRing1_Rotated );
    glDrawArrays( GL_TRIANGLES, 0, SpikedRing1_VCnt );

	//-------------------------------------------------------------------------
	// Now, make the window's context current for regular rendering...
	//-------------------------------------------------------------------------

	if( !wglMakeCurrent( g_hDC, g_hRC ) )
	{
		MessageBox(NULL,"Could not make the window's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
        //exit(-1);
    }

    //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	//
	// Let the user spin the cube about with the left mouse button.
	//

	//glMatrixMode( GL_MODELVIEW );
    //glLoadIdentity();
	//glTranslatef( 0.0f, 0.0f, -5.0f );
    //glRotatef( -g_fSpinY_L, 1.0f, 0.0f, 0.0f );
    //glRotatef( -g_fSpinX_L, 0.0f, 1.0f, 0.0f );

    //
    // Finally, we'll use the dynamic texture like a regular static texture.
    //

	//
	// Bind the dynamic texture like you normally would, then bind the
	// p-buffer to it.
	//
    glBindTexture( GL_TEXTURE_2D, g_dynamicTextureID );

	if( !wglBindTexImageARB( g_pbuffer.hPBuffer, WGL_FRONT_LEFT_ARB ) )
	{
		MessageBox(NULL,"Could not bind p-buffer to render texture!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}

    //glInterleavedArrays( GL_T2F_V3F, 0, g_cubeVertices );
    //glDrawArrays( GL_QUADS, 0, 24 );

    //
    // Before we forget, we need to make sure that the p-buffer has been 
    // released from the dynamic "render-to" texture.
    //
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (BYTE*)RenderedBuffer2);

    if( !wglReleaseTexImageARB( g_pbuffer.hPBuffer, WGL_FRONT_LEFT_ARB ) )
    {
        MessageBox(NULL,"Could not release p-buffer from render texture!",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        //exit(-1);
    }


	SwapBuffers( g_hDC );

	StepX = (256 << 16) / fW;
	StepY = ((int)(256.0 * 0.75) << 16) / fH;

	OffsY = (((256 - (int)(256.0 * 0.75)) >> 1) << 16);
	for (py = 0; py < fH; py++) {
		int OffsX = 0;
		for (int px = 0; px < fW; px++) {
			int POffsX = OffsX >> 16;
			int POffsY = OffsY >> 16;
			pPixel[(py * fW) + px] = RenderedBuffer2[(POffsY << 8) + POffsX];
			OffsX += StepX;
		}
		OffsY += StepY;
	}
	
	// ----------- END RENDER

	return FF_SUCCESS;
}

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

DWORD plugClass::processFrameCopy(ProcessFrameCopyStruct* pFrameData)
{
	if (pFrameData->numInputFrames<NUM_INPUTS) {
		return FF_FAIL;
	}

	switch (videoInfo.bitDepth) {
		case 1:
			return processFrameCopy24Bit(pFrameData);
		case 2:
			return processFrameCopy32Bit(pFrameData);
		default:
			return FF_FAIL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// processFrameCopy24Bit
//
// process a frame of 24 bit video
//
// parameters:
// 32-bit pointer to byte array containing frame of video
//
// return values (DWORD):
// FF_SUCCESS
// FF_FAIL on error
//

DWORD plugClass::processFrameCopy24Bit(ProcessFrameCopyStruct* pFrameData)
{
	VideoPixel24bit* pInputPixel = (VideoPixel24bit*) pFrameData->InputFrames[0];
	VideoPixel24bit* pOutputPixel = (VideoPixel24bit*) pFrameData->OutputFrame;
	VideoPixel24bit* pPixelDest = (VideoPixel24bit*) DestPix24;

	
	return FF_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////
// processFrameCopy32Bit
//
// process a frame of 32 bit video
//
// parameters:
// 32-bit pointer to byte array containing frame of video
//
// return values (DWORD):
// FF_SUCCESS
// FF_FAIL on error
//

DWORD plugClass::processFrameCopy32Bit(ProcessFrameCopyStruct* pFrameData)
{
	VideoPixel32bit* pInputPixel = (VideoPixel32bit*) pFrameData->InputFrames[0];
	VideoPixel32bit* pOutputPixel = (VideoPixel32bit*) pFrameData->OutputFrame;
	VideoPixel32bit* pPixelDest = (VideoPixel32bit*) DestPix32;

	return FF_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////
// getpluginCaps
//
// returns true or false to indicate whether cappable of feature specified by index
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

DWORD getPluginCaps(DWORD index)
{
	switch (index) {

	case FF_CAP_16BITVIDEO:
		return FF_FALSE;

	case FF_CAP_24BITVIDEO:
		return FF_FALSE;

	case FF_CAP_32BITVIDEO:
		return FF_TRUE;

	case FF_CAP_PROCESSFRAMECOPY:
		return FF_FALSE;

	case FF_CAP_MINIMUMINPUTFRAMES:
		return NUM_INPUTS;

	case FF_CAP_MAXIMUMINPUTFRAMES:
		return NUM_INPUTS;

	case FF_CAP_COPYORINPLACE:
		return FF_FALSE;

	default:
		return FF_FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// instantiate
//
// Run up plugin instance - plugObj
// Prepare the Plug-in instance for processing.  
// Set default values, allocate memory
// When the plug-in returns from this function it must be ready to proces a frame
//
// return values (pointer to a plugObj - FF Dword Instance ID)
// FF_FAIL
//
// HOST: This function *must* return before a call to processFrame


LPVOID instantiate(VideoInfoStruct* pVideoInfo)
{

	// Create local pointer to plugObject
	int Fail = 0;
	plugClass *pPlugObj;
	// create new instance of plugClass
	pPlugObj = new plugClass;
	DWORD Siz = pVideoInfo->frameWidth * pVideoInfo->frameHeight;
	if (pVideoInfo->bitDepth == 1) {
		pPlugObj->DestPix24 = new VideoPixel24bit[Siz];
		for (DWORD i = 0; i < Siz; i++) {
			pPlugObj->DestPix24[i].red = (BYTE)0;
			pPlugObj->DestPix24[i].green = (BYTE)0;
			pPlugObj->DestPix24[i].blue = (BYTE)0;
		}
	} else {
		if (pVideoInfo->bitDepth == 2) {
		pPlugObj->DestPix32 = new VideoPixel32bit[Siz];
			for (DWORD i = 0; i < Siz; i++) {
				pPlugObj->DestPix32[i].red = (BYTE)0;
				pPlugObj->DestPix32[i].green = (BYTE)0;
				pPlugObj->DestPix32[i].blue = (BYTE)0;
				pPlugObj->DestPix32[i].alpha = (BYTE)0xff;
			}
		}
	}

	// make a copy of the VideoInfoStruct
	pPlugObj->videoInfo.frameWidth = pVideoInfo->frameWidth;
	pPlugObj->videoInfo.frameHeight = pVideoInfo->frameHeight;
	pPlugObj->videoInfo.bitDepth = pVideoInfo->bitDepth;

	// this shouldn't happen if the host is checking the capabilities properly
	pPlugObj->vidmode = pPlugObj->videoInfo.bitDepth;
	if (pPlugObj->vidmode > 2 || pPlugObj->vidmode < 0) {
	  return (LPVOID) FF_FAIL;
	}

	pPlugObj->paramDynamicData[0].value = paramConstants[0].defaultValue;
	pPlugObj->paramDynamicData[1].value = paramConstants[1].defaultValue;
	pPlugObj->paramDynamicData[2].value = paramConstants[2].defaultValue;
	pPlugObj->paramDynamicData[3].value = paramConstants[3].defaultValue;
	pPlugObj->paramDynamicData[4].value = paramConstants[4].defaultValue;
	pPlugObj->paramDynamicData[5].value = paramConstants[5].defaultValue;
	pPlugObj->paramDynamicData[6].value = paramConstants[6].defaultValue;
	pPlugObj->paramDynamicData[7].value = paramConstants[7].defaultValue;
	pPlugObj->paramDynamicData[8].value = paramConstants[8].defaultValue;
	pPlugObj->paramDynamicData[9].value = paramConstants[9].defaultValue;
	pPlugObj->paramDynamicData[10].value = paramConstants[10].defaultValue;
	pPlugObj->paramDynamicData[11].value = paramConstants[11].defaultValue;

	pPlugObj->m_pTimer = new CTimer();

	//-------------------------------------------------------------------------
	// Setup OpenGL

/*	// Create the window
	CWindow::SParameters Params;

	pPlugObj->m_pWindow = new CWindow();

	pPlugObj->m_pWindow->SetDefault(&Params);

	Params.m_hInstance = hInstDTS;
	Params.m_sTitle = "OpenGL Window";
	Params.m_nWidth = (int)pVideoInfo->frameWidth;
	Params.m_nHeight = (int)pVideoInfo->frameHeight;

	HRESULT hr = pPlugObj->m_pWindow->Initialize(Params);
	if( !FAILED(hr) ) {*/


	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName = "MY_WIN_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstDTS;
    winClass.hIcon	       = LoadIcon(NULL, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm	   = LoadIcon(NULL, (LPCTSTR)IDI_OPENGL_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
//	if( !RegisterClassEx(&winClass) )
//		return E_FAIL;

	DWORD ExStyle, Style;
	ExStyle = WS_EX_TOPMOST; // This lets our window cover the whole screen
	Style   = WS_POPUP;      // We don't need caption and system 


	g_hWnd = CreateWindowEx( ExStyle, "MY_WINDOWS_CLASS", 
		                     "OpenGL - Off-Screen Rendering Using P-Buffers",
						     Style,
					         0, 0, 640, 480, NULL, NULL, hInstDTS, NULL ); //hInstance, NULL );
	

//    ShowWindow( g_hWnd, SW_SHOW);
//    UpdateWindow( g_hWnd );

	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat(g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, 640.0f / 480.0f, 0.1f, 100.0f);

	//
	// This is our dynamic texture, which will be loaded with new pixel data
	// after we're finshed rendering to the p-buffer.
	//

	glGenTextures( 1, &g_dynamicTextureID );
	glBindTexture( GL_TEXTURE_2D, g_dynamicTextureID );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, PBUFFER_WIDTH, PBUFFER_HEIGHT, 0, GL_RGB, GL_FLOAT, 0 );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );

	//
	// If the required extensions are present, get the addresses for the
	// functions that we wish to use...
	//

	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	char *ext = NULL;
	
	if( wglGetExtensionsStringARB )
		ext = (char*)wglGetExtensionsStringARB( wglGetCurrentDC() );
	else
	{
		MessageBox(NULL,"Unable to get address for wglGetExtensionsStringARB!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
//			exit(-1);
	}

	//
	// WGL_ARB_pbuffer
	//

	if( strstr( ext, "WGL_ARB_pbuffer" ) == NULL )
	{
		MessageBox(NULL,"WGL_ARB_pbuffer extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}
	else
	{
		wglCreatePbufferARB    = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
		wglGetPbufferDCARB     = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
		wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
		wglDestroyPbufferARB   = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
		wglQueryPbufferARB     = (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB");

		if( !wglCreatePbufferARB || !wglGetPbufferDCARB || !wglReleasePbufferDCARB ||
			!wglDestroyPbufferARB || !wglQueryPbufferARB )
		{
			MessageBox(NULL,"One or more WGL_ARB_pbuffer functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			//exit(-1);
		}
	}

	//
	// WGL_ARB_pixel_format
	//

	if( strstr( ext, "WGL_ARB_pixel_format" ) == NULL )
	{
		MessageBox(NULL,"WGL_ARB_pixel_format extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return (void*)FF_FAIL;
	}
	else
	{
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

		if( !wglChoosePixelFormatARB )
		{
			MessageBox(NULL,"One or more WGL_ARB_pixel_format functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			//exit(-1);
		}
	}

	//
	// WGL_ARB_render_texture
	//

	if( strstr( ext, "WGL_ARB_render_texture" ) == NULL )
	{
		MessageBox(NULL,"WGL_ARB_render_texture extension was not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}
	else
	{
		wglBindTexImageARB     = (PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
		wglReleaseTexImageARB  = (PFNWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress("wglReleaseTexImageARB");
		wglSetPbufferAttribARB = (PFNWGLSETPBUFFERATTRIBARBPROC)wglGetProcAddress("wglSetPbufferAttribARB");

		if( !wglBindTexImageARB || !wglReleaseTexImageARB || !wglSetPbufferAttribARB )
		{
			MessageBox(NULL,"One or more WGL_ARB_render_texture functions were not found",
				"ERROR",MB_OK|MB_ICONEXCLAMATION);
			//exit(-1);
		}
	}

	//-------------------------------------------------------------------------
	// Create a p-buffer for off-screen rendering.
	//-------------------------------------------------------------------------

	g_pbuffer.hPBuffer = NULL;
	g_pbuffer.nWidth   = PBUFFER_WIDTH;
	g_pbuffer.nHeight  = PBUFFER_HEIGHT;

	//
	// Define the minimum pixel format requirements we will need for our 
	// p-buffer. A p-buffer is just like a frame buffer, it can have a depth 
	// buffer associated with it and it can be double buffered.
	//

	int pf_attr[] =
	{
		WGL_SUPPORT_OPENGL_ARB, TRUE,       // P-buffer will be used with OpenGL
		WGL_DRAW_TO_PBUFFER_ARB, TRUE,      // Enable render to p-buffer
		WGL_BIND_TO_TEXTURE_RGBA_ARB, TRUE, // P-buffer will be used as a texture
		WGL_RED_BITS_ARB, 8,                // At least 8 bits for RED channel
		WGL_GREEN_BITS_ARB, 8,              // At least 8 bits for GREEN channel
		WGL_BLUE_BITS_ARB, 8,               // At least 8 bits for BLUE channel
		WGL_ALPHA_BITS_ARB, 8,              // At least 8 bits for ALPHA channel
		WGL_DEPTH_BITS_ARB, 16,             // At least 16 bits for depth buffer
		WGL_DOUBLE_BUFFER_ARB, FALSE,       // We don't require double buffering
		0                                   // Zero terminates the list
	};

	unsigned int count = 0;
	int pixelFormat;
	wglChoosePixelFormatARB( g_hDC,(const int*)pf_attr, NULL, 1, &pixelFormat, &count);

	if( count == 0 )
	{
		MessageBox(NULL,"Could not find an acceptable pixel format!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}

	//
	// Set some p-buffer attributes so that we can use this p-buffer as a
	// 2D RGBA texture target.
	//

	int pb_attr[] =
	{
		WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGBA_ARB, // Our p-buffer will have a texture format of RGBA
		WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,   // Of texture target will be GL_TEXTURE_2D
		0                                             // Zero terminates the list
	};

	//
	// Create the p-buffer...
	//

	g_pbuffer.hPBuffer = wglCreatePbufferARB( g_hDC, pixelFormat, g_pbuffer.nWidth, g_pbuffer.nHeight, pb_attr );
	g_pbuffer.hDC      = wglGetPbufferDCARB( g_pbuffer.hPBuffer );
	g_pbuffer.hRC      = wglCreateContext( g_pbuffer.hDC );

	if( !g_pbuffer.hPBuffer )
	{
		MessageBox(NULL,"Could not create the p-buffer",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}

	int h;
	int w;
	wglQueryPbufferARB( g_pbuffer.hPBuffer, WGL_PBUFFER_WIDTH_ARB, &h );
	wglQueryPbufferARB( g_pbuffer.hPBuffer, WGL_PBUFFER_WIDTH_ARB, &w );

	if( h != g_pbuffer.nHeight || w != g_pbuffer.nWidth )
	{
		MessageBox(NULL,"The width and height of the created p-buffer don't match the requirements!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}

	//
	// We were successful in creating a p-buffer. We can now make its context 
	// current and set it up just like we would a regular context 
	// attached to a window.
	//

	if( !wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC ) ) 
	{
		MessageBox(NULL,"Could not make the p-buffer's context current!",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
		//exit(-1);
	}

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
//	glEnable(GL_AUTO_NORMAL);
	glEnable( GL_LIGHTING );
	glShadeModel(GL_SMOOTH);
	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHT1 );

	// ------------------------------- LIGHTING

	GLfloat mat_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv( GL_FRONT, GL_DIFFUSE, mat_diffuse );
	glMaterialfv( GL_FRONT, GL_AMBIENT, mat_ambient );   
 
	// Set light 0 to be a simple, bright directional light to use 
    // on the mesh that will represent light 2
/*	GLfloat diffuse_light0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat position_light0[] = { 0.5f, -0.5f, -0.5f, 0.0f };
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse_light0 );
	glLightfv( GL_LIGHT0, GL_POSITION, position_light0 );

	// Set light 1 to be a simple, faint grey directional light so 
    // the walls and floor are slightly different shades of grey
	GLfloat diffuse_light1[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	GLfloat position_light1[] = { 0.0f, 1.5f, -0.5f, 0.0f };
	//GLfloat position_light1[] = { 0.3f, -0.5f, -0.2f, 0.0f };
	glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse_light1 );
	glLightfv( GL_LIGHT1, GL_POSITION, position_light1 );

	// Light #2 will be the demo light used to light the floor and walls. 
	// It will be set up in render() since its type can be changed at 
    // run-time.

	// Enable some dim, grey ambient lighting so objects that are not lit 
    // by the other lights are not completely black.*/
	GLfloat ambient_lightModel[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

	// -----------------------------------------------

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, PBUFFER_WIDTH / PBUFFER_HEIGHT, 0.1f, 10.0f );

	pPlugObj->CosX1 = 0.0;
	pPlugObj->SinY1 = 0.0;
	pPlugObj->CosX2 = 0.0;
	pPlugObj->SinY2 = 0.0;

	pPlugObj->TransX = 0.0;
	pPlugObj->TransY = 0.0;

	pPlugObj->m_pTimer->Start();

	pPlugObj->RenderedBuffer1 = new VideoPixel32bit[1024 * 1024];
	pPlugObj->RenderedBuffer2 = new VideoPixel32bit[1024 * 1024];

/*	if (Fail) {
		return (LPVOID)FF_FAIL;
	}*/

	glGenTextures( 1, &g_testTextureID );

	// rotation of SpikedRings
	pPlugObj->RotX = 0;
	pPlugObj->RotY = 0;
	pPlugObj->RotZ = 0;

	// rotation of background Sphere
	pPlugObj->BSRotX = 0;
	pPlugObj->BSRotY = 0;
	pPlugObj->BSRotZ = 0;

	pPlugObj->ShowBkgrSphere = false;

	pPlugObj->SinCosTab = new int[65536 << 1];

	double Ang = 0.0;

	for (int sini = 0; sini < 65536; sini++) {
		pPlugObj->SinCosTab[(sini << 1)] = (int)(sin(Ang / 65536.0) * 65536.0);
		pPlugObj->SinCosTab[(sini << 1) + 1] = (int)(cos(Ang / 65536.0) * 65536.0);
		Ang += M_PI2;
	}

	pPlugObj->FirstPass = true;
	pPlugObj->ShowBkgrSphere = false;

	for (int ix = 0; ix < SpikedRing1_VCnt; ix++) {
		SpikedRing1_Rotated[ix].r = 1.0f;
		SpikedRing1_Rotated[ix].g = 1.0f;
		SpikedRing1_Rotated[ix].b = 1.0f;
	}

	for (ix = 0; ix < SpikedRing2_VCnt; ix++) {
		SpikedRing2_Rotated[ix].r = 1.0f;
		SpikedRing2_Rotated[ix].g = 1.0f;
		SpikedRing2_Rotated[ix].b = 1.0f;
	}

	for (ix = 0; ix < SpikedRing3_VCnt; ix++) {
		SpikedRing3_Rotated[ix].r = 1.0f;
		SpikedRing3_Rotated[ix].g = 1.0f;
		SpikedRing3_Rotated[ix].b = 1.0f;
	}

	return (LPVOID) pPlugObj;
}

///////////////////////////////////////////////////////////////////////////////////////
// deInstantiate
//
// Run down plugin instance
//
// Deallocate memory used by this instance

DWORD deInstantiate(LPVOID instanceID)
{

    glDeleteTextures( 1, &g_dynamicTextureID );
    glDeleteTextures( 2, &g_testTextureID );

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}

	//
	// Don't forget to clean up after our p-buffer...
	//

	if( g_pbuffer.hRC != NULL )
	{
		wglMakeCurrent( g_pbuffer.hDC, g_pbuffer.hRC );
		wglDeleteContext( g_pbuffer.hRC );
		wglReleasePbufferDCARB( g_pbuffer.hPBuffer, g_pbuffer.hDC );
		wglDestroyPbufferARB( g_pbuffer.hPBuffer );
		g_pbuffer.hRC = NULL;
	}

	if( g_pbuffer.hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_pbuffer.hDC );
		g_pbuffer.hDC = NULL;
	}

    UnregisterClass( "MY_WIN_CLASS", winClass.hInstance);


	// declare pPlugObj - pointer to this instance
	plugClass *pPlugObj;

	// typecast LPVOID into pointer to a plugClass
	pPlugObj = (plugClass*) instanceID;
	if (pPlugObj->videoInfo.bitDepth == 1) {
		delete []pPlugObj->DestPix24;
	} else {
		if (pPlugObj->videoInfo.bitDepth == 2) {
			delete []pPlugObj->DestPix32;
		}
	}

	delete []pPlugObj->RenderedBuffer1;
	delete []pPlugObj->RenderedBuffer2;
	delete []pPlugObj->SinCosTab;

	delete pPlugObj; // todo: ? success / fail?

	return FF_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////
// getExtendedInfo
//

LPVOID getExtendedInfo()
{

	plugExtInfo.PluginMajorVersion = 1;
	plugExtInfo.PluginMinorVersion = 10;

	// I'm just passing null for description etc for now
	// todo: send through description and about
	plugExtInfo.Description = NULL;
	plugExtInfo.About = NULL;

	// FF extended data block is not in use by the API yet
	// we will define this later if we want to
	plugExtInfo.FreeFrameExtendedDataSize = 0;
	plugExtInfo.FreeFrameExtendedDataBlock = NULL;

	return (LPVOID) &plugExtInfo;

	////////////////////////////////////////////////////

}


DWORD setDTSWin(HINSTANCE* hI)
{
	hInstDTS = *hI;
	return FF_SUCCESS;
}


/*DWORD setDTSPosX(DWORD DPos)
{
	DTSPosX = DPos;
	return FF_SUCCESS;
}


DWORD setDTSPosX(DWORD DPos)
{
	DTSPosX = DPos;
	return FF_SUCCESS;
}*/
