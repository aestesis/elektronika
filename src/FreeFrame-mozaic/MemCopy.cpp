///////////////////////////////////////////////////////////////////////////////////
// MemCopy.cpp
//
// Optimized MemCopy, currently supports mmx and 3dnowext
// C Version
//
// Implementation of a plugin interface for the FreeFrame API
//
// www.freeframe.org
// bluelive@xs4all.nl

///////////////////////////////////////////////////////////////////////////////////
// TODO:
// will gcc eat this inline asm ?

/*

Copyright (c) 2002, Bart van der Werf www.freeframe.org
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

///////////////////////////////////////////////////////////////////////////////////////////
//
// includes 
//

#include "MemCopy.h"

#ifndef NO_MEMCOPY

/////////////////////////////////////////////////////////////////////////////////////////
// #defines

//CPU_ID instruction
#define CPU_ID _asm _emit 0x0f _asm _emit 0xa2

MemCopy memcopyinstance=testmemcpy;

void *memcopy(void *dest, const void *src, size_t n)
{
	return memcopyinstance(dest, src, n);
}

void *testmemcpy(void *dest, const void *src, size_t n)
{
	__int32 cputype1 = 0;
	__int32 cputype2 = 0;

	_asm
	{
		xor eax, eax;
		CPU_ID;
		jnz to_end;

		CPU_ID;
		mov dword ptr cputype1, edx;

		and eax, eax;
		jz to_end;

		mov eax, 80000001h;
		CPU_ID;
		mov dword ptr cputype2, edx;
to_end:

		EMMS
	}

	if (cputype2 &0x80000000)
		memcopyinstance=amd3dnowextmemcpy;
	else
		if (cputype1 &0x800000)
			memcopyinstance=mmxmemcpy;
		else
			memcopyinstance=defaultmemcpy;
	return memcopy(dest, src, n);
}

void *defaultmemcpy(void *dest, const void *src, size_t n)
{
	//could be a macro, we need a function
	return memcpy(dest, src, n);
}

void *amd3dnowextmemcpy(void *dest, const void *src, size_t n)
{
	_asm
	{
		emms;

		mov edi, dest;
		mov esi, src;
	
		xor edx, edx;
		mov eax, n;
		mov ebx, 16;
		div ebx;
		mov ecx, eax
		//move them mmx bytes (16 at a time :)
lx:
		movq mm0,[esi];
		movq mm1,[esi+8];

		lea esi,[esi+16];
		movntq [edi],mm0;

		prefetcht0 [esi+768];
		movntq [edi+8],mm1;

		lea edi,[edi+16];
		dec ecx;

		//gives the loop an even number of operations, which makes the stuff a bit faster
		nop;
		jnz lx;

		//move them last bytes, ugly slow, but its the only way to move odd amoun of bytes
		//the count is 15 or less o it doesnt matter that much
		mov ecx, edx;
		cld;
		rep movsb;

		sfence;
		emms;
	}
	return dest;
}

void *mmxmemcpy(void *dest, const void *src, size_t n)
{
	_asm
	{
		emms;

		mov edi, dest;
		mov esi, src;
	
		xor edx, edx;
		mov eax, n;
		mov ebx, 16;
		div ebx;
		mov ecx, eax
		//move them mmx bytes (16 at a time :)
lx:
		movq mm0,[esi];
		movq mm1,[esi+8];

		lea esi,[esi+16];
		movq [edi],mm0;

		movq [edi+8],mm1;
		lea edi,[edi+16];

		dec ecx;
		jnz lx;

		//move them last bytes, ugly slow, but its the only way to move odd amoun of bytes
		//the count is 15 or less o it doesnt matter that much
		mov ecx, edx;
		cld;
		rep movsb;

		emms;
	}
	return dest;
}

#endif