/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  ns-eel.h: main application interface header

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <windows.h>
#include "../ns-eel/ns-eel.h"
#include "../ns-eel/ns-eel-int.h"
#include "megabuf.h"

void megabuf_ppproc(void *data, int data_size, void **userfunc_data)
{
  if (data_size > 5 && *(int*)((char *)data+1) == 0xFFFFFFFF)
  {
    *(int*)((char *)data+1) = (int) (userfunc_data+0);
  }
}

void megabuf_cleanup(NSEEL_VMCTX ctx)
{
  if (ctx)
  {
    compileContext *c=(compileContext*)ctx;
    if (c->userfunc_data[0])
    {
      double **blocks = (double **)c->userfunc_data[0];
      int x;
      for (x = 0; x < MEGABUF_BLOCKS; x ++)
      {
        if (blocks[x]) GlobalFree(blocks[x]);
        blocks[x]=0;
      }
      GlobalFree((HGLOBAL)blocks);
      c->userfunc_data[0]=0;
    }
  }
}

static double * NSEEL_CGEN_CALL megabuf_(double ***blocks, double *which)
{
  static double error;
  int w=(int)(*which + 0.0001);
  int whichblock = w/MEGABUF_ITEMSPERBLOCK;

  if (!*blocks)
  {
    *blocks = (double **)GlobalAlloc(GPTR,sizeof(double *)*MEGABUF_BLOCKS);
  }
  if (!*blocks) return &error;

  if (w >= 0 && whichblock >= 0 && whichblock < MEGABUF_BLOCKS)
  {
    int whichentry = w%MEGABUF_ITEMSPERBLOCK;
    if (!(*blocks)[whichblock])
    {
      (*blocks)[whichblock]=(double *)GlobalAlloc(GPTR,sizeof(double)*MEGABUF_ITEMSPERBLOCK);
    }
    if ((*blocks)[whichblock])
      return &(*blocks)[whichblock][whichentry];
  }

  return &error;
}

static double * (NSEEL_CGEN_CALL *__megabuf)(double ***,double *) = &megabuf_;
__declspec ( naked ) void _asm_megabuf(void)
{
  double ***my_ctx;
  double *parm_a, *__nextBlock;
  __asm { mov edx, 0xFFFFFFFF }
  __asm { mov ebp, esp }
  __asm { sub esp, __LOCAL_SIZE }
  __asm { mov dword ptr my_ctx, edx }
  __asm { mov dword ptr parm_a, eax }
  
  __nextBlock = __megabuf(my_ctx,parm_a);

  __asm { mov eax, __nextBlock } // this is custom, returning pointer
  __asm { mov esp, ebp }
}
__declspec ( naked ) void _asm_megabuf_end(void) {}
