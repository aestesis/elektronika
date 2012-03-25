// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OMPPCHOST_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OMPPCHOST_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef OMPPCHOST_EXPORTS
#define OMPPCHOST_API extern "C" __declspec(dllexport) 
#else
#define OMPPCHOST_API extern "C" __declspec(dllimport)
#endif

#include "ompp.h"


OMPPCHOST_API void* __stdcall	omppInit(char *name, int width, int height);	// return handle
OMPPCHOST_API void __stdcall	omppFree(void * handle);

// start/stop/process

OMPPCHOST_API int __stdcall		omppStart(void *handle);
OMPPCHOST_API int __stdcall		omppProcessFrame(void *handle, double time, double beat);
OMPPCHOST_API void __stdcall	omppStop(void *handle);

// Media access
OMPPCHOST_API int __stdcall		omppGetNbMedia(void *handle);
OMPPCHOST_API int __stdcall		ommpGetMediaType(void *handle, int n);

// MediaBitmap access
OMPPCHOST_API void * __stdcall	omppBitmapGetBits(void *handle, int n);

// MediaSelect access
OMPPCHOST_API int __stdcall		omppSelectGetNbDesc(void *handle, int nmedia);
OMPPCHOST_API char * __stdcall	omppSelectGetDesc(void *handle, int nmedia, int n);
OMPPCHOST_API int __stdcall		omppSelectGetValue(void *handle, int nmedia);
OMPPCHOST_API int __stdcall		omppSelectSetValue(void *handle, int nmedia, int n);


