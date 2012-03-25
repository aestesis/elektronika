///////////////////////////////////////////////////////////////////////////////////
// MemCopy.h
//
// MMX optimized memcopy
// C Version
//
// www.freeframe.org
// bluelive@xs4all.nl

/*

Copyright (c) 2003, Bart van der Werf www.freeframe.org
All rights reserved.

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

#ifndef __MEMCOPY_H__
#define __MEMCOPY_H__

#ifdef LINUX
extern "C" {
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#endif

//////////////////////////////////////////////////////////////////////////////////
//
// includes
//

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#elif defined LINUX

//typedefs for linux - in windows these are defined in files included by windows.h
typedef unsigned int DWORD;
typedef void * LPVOID;
typedef char BYTE;

#endif

#ifdef NOT_INTEL
#define NO_MEMCOPY
#endif
#ifdef LINUX
#define NO_MEMCOPY
#endif


#ifdef NO_MEMCOPY

#define memcopy(a, b, c) memcpy((a), (b), (c))

#else

///////////////////////////////////////////////////////////////////////////////////////
// Function prototypes
//

typedef void* (*MemCopy)(void *, const void *, size_t);

void *amd3dnowextmemcpy(void *dest, const void *src, size_t n);
void *testmemcpy(void *dest, const void *src, size_t n);
void *mmxmemcpy(void *dest, const void *src, size_t n);
void *defaultmemcpy(void *dest, const void *src, size_t n);

void *memcopy(void *dest, const void *src, size_t n);

#endif

#ifdef LINUX
}
#endif
#endif __MEMCOPY_H__