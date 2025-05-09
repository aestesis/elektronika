/*****************************************************************************
 *
 * gcc version of the MMX optimized routines. All MMX optimized functions
 * have been gathered into this single source code file, regardless to their 
 * class or original source code file, in order to ease porting the library
 * to other compiler and processor platforms.
 *
 * This file is to be compiled on any platform with the GNU C compiler.
 * Compiler. Please see 'mmx_win.cpp' for the x86 Windows version of this
 * file.
 *
 * Author          : Copyright (c) Olli Parviainen
 * Author e-mail   : oparviai @ iki.fi
 * File created    : 13-Jan-2002
 *
 * Last changed  : $Date: 2003/11/05 18:19:40 $
 * File revision : $Revision: 1.3 $
 *
 * $Id: cpu_detect_x86_gcc.cpp,v 1.3 2003/11/05 18:19:40 Olli Exp $
 *
 * Acknowledgements:
 * Adopted for gcc : Stuart Lamble <sjl @ debian.lib.monash.edu.au>
 * Adopted for gcc3: Shachar Raindel <shacharr @ users.sourceforge.net>
 *
 * License :
 * 
 *  SoundTouch sound processing library
 *  Copyright (c) Olli Parviainen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/

#include <stdexcept>
#include <string>
#include "cpu_detect.h"

#ifndef __GNUC__
#error wrong platform - this source code file is for the GNU C compiler.
#endif

using namespace std;

#include <stdio.h>
//////////////////////////////////////////////////////////////////////////////
//
// processor instructions extension detection routines
//
//////////////////////////////////////////////////////////////////////////////


// Flag variable indicating whick ISA extensions are disabled (for debugging)
static uint _dwDisabledISA = 0x00;      // 0xffffffff; //<- use this to disable all extensions

// Disables given set of instruction extensions. See SUPPORT_... defines.
void disableExtensions(uint dwDisableMask)
{
    _dwDisabledISA = dwDisableMask;
}



/// Checks which instruction set extensions are supported by the CPU.
uint detectCPUextensions(void)
{
#ifndef __i386__
    return 0; // always disable extensions on non-x86 platforms.
#else
    uint res = 0;

    if (_dwDisabledISA == 0xffffffff) return 0;

    asm volatile(
        "\n\txor     %%esi, %%esi"       // clear %%esi = result register
        // check if 'cpuid' instructions is available by toggling eflags bit 21

        "\n\tpushf"                      // save eflags to stack
        "\n\tpop     %%eax"              // load eax from stack (with eflags)
        "\n\tmovl    %%eax, %%ecx"       // save the original eflags values to ecx
        "\n\txor     $0x00200000, %%eax" // toggle bit 21
        "\n\tpush    %%eax"              // store toggled eflags to stack
        "\n\tpopf"                       // load eflags from stack
        "\n\tpushf"                      // save updated eflags to stack
        "\n\tpop     %%eax"              // load from stack
        "\n\txor     %%edx, %%edx"       // clear edx for defaulting no mmx
        "\n\tcmp     %%eax, %%ecx"       // compare to original eflags values
        "\n\tjz      end"                // jumps to 'end' if cpuid not present

        // cpuid instruction available, test for presence of mmx instructions

        "\n\tmovl    $1, %%eax"
        "\n\tcpuid"
//        movl       $0x00800000, %edx   // force enable MMX
        "\n\ttest    $0x00800000, %%edx"
        "\n\tjz      end"                // branch if MMX not available

        "\n\tor      $0x01, %%esi"       // otherwise add MMX support bit

        "\n\ttest    $0x02000000, %%edx"
        "\n\tjz      test3DNow"          // branch if SSE not available

        "\n\tor      $0x08, %%esi"       // otherwise add SSE support bit

    "\n\ttest3DNow:"
        // test for precense of AMD extensions
        "\n\tmov     $0x80000000, %%eax"
        "\n\tcpuid"
        "\n\tcmp     $0x80000000, %%eax"
        "\n\tjbe     end"                 // branch if no AMD extensions detected

        // test for precense of 3DNow! extension
        "\n\tmov     $0x80000001, %%eax"
        "\n\tcpuid"
        "\n\ttest    $0x80000000, %%edx"
        "\n\tjz      end"                  // branch if 3DNow! not detected

        "\n\tor      $0x02, %%esi"         // otherwise add 3DNow support bit

    "\n\tend:"

        "\n\tmov     %%esi, %0"

      : "=rm" (res)
      : /* no inputs */
      : "%edx", "%eax", "%ecx", "%ebx", "%esi" );
      
    return res & ~_dwDisabledISA;
#endif
}
