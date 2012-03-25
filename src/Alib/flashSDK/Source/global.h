/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

/****************************************************************************
  Porting Version of Flash 4 SDK
  Middlesoft, Inc. October 1999
  Release 10/22/1999 Lee Thomason, lee@middlesoft.com 
****************************************************************************/

#ifndef GLOBAL_INCLUDED
#define GLOBAL_INCLUDED

#define NATIVE_DISPLAY		"./win32/_display.h"
#define NATIVE_PALETTE		"./win32/palette.h"
#define NATIVE_UTIL			"./win32/util.h"
#define NATIVE_PLAYERWND	"./win32/playerwnd.h"
#define NATIVE_FIXED		"./win32/fixed.h"
#define NATIVE_SOUND		"./win32/sound.h"
#define NATIVE_DEBUG		"./win32/debug.h"
#define NATIVE_VERSION		"./win32/version.h"

// The following are the #defines past in by the compiler for a full build:
// _DEBUG
// WIN32,_WINDOWS	(or whatever is appropriate)
// USE_JPEG
// FRAMESPERSEC
// SOUND
// EDITTEXT
// FLASHMP3

#if defined(_DEBUG) && !defined(DEBUG)
	#define DEBUG
#endif

/*! Expects wide characters from the cut and paste strings - otherwise
 *	uses ascii.
 */
#define EDITTEXT_CLIPBOARD_WIDE

/*! Replaces fixed point math routines with floating point.
 */
// #define FIXEDUSEFLOAT

/*! If ONE_SOUND_OBJECT this is defined, there will be only one NativeSoundMix 
 *  used by all the SPlayers. For a version with only one SPlayer, this is moot. 
 *  If not defined, every SPlayer will have its own NativeSoundMix.
 */
#define ONE_SOUND_OBJECT



//
// Global Types
//

#include NATIVE_DEBUG

typedef unsigned long	U32, *P_U32, **PP_U32;
typedef signed long		S32, *P_S32, **PP_S32;
typedef unsigned short	U16, *P_U16, **PP_U16;
typedef signed short	S16, *P_S16, **PP_S16;
typedef unsigned char	U8, *P_U8, **PP_U8;
typedef signed char		S8, *P_S8, **PP_S8;
typedef int				BOOL;

#ifdef DEBUG
	#define Dbg( x )	x
#else
	#define Dbg( x )	/* */
#endif 

#define Abs(v)			((v)<0?(-(v)):(v))
#define Max(a,b)		((a)>(b)?(a):(b))
#define Min(a,b)		((a)<(b)?(a):(b))
   
#define LowU16(dw)		((U16)(U32)(dw))
#define HighU16(dw)		((U16)((U32)(dw)>>16))
#define LowU8(w)		((U8)(w))
#define HighU8(w)		((U8)((U16)(w)>>8))
#define MakeU16(lb,hb)	(((U16)(hb)<<8)|(U16)(lb))
#define MakeU32(lw,hw)	(((U32)(hw)<<16)|(U32)(lw))

// Limits
#define maxU8			((U8)0xFF)
#define minS8			((S8)0x80)
#define maxS8			((S8)0x7F)
#define maxU16			((U16)0xFFFF)
#define minS16			((S16)0x8000)
#define maxS16			((S16)0x7FFF)
#define maxU32			((U32)0xFFFFFFFF)
#define minS32			((S32)0x80000000)
#define maxS32			((S32)0x7FFFFFFF)


//
// Global Utilities
//

#define Swap(a, b, type) { type swapTmp; swapTmp = a; a = b; b = swapTmp; }
#define PinToRange(min, value, max) \
			((min) > (value) ? (min) : ((max) < (value) ? (max) : (value)))
#define Sign(a) ((a) == 0 ? 0 : ((a) < 0 ? -1 : 1))
#define FlipU32(a)  { U8 *pa = (U8 *)&(a); Swap(pa[0], pa[3], U8); Swap(pa[1], pa[2], U8); }
#define FlipU16(a)  { U8 *pa = (U8 *)&(a); Swap(pa[0], pa[1], U8); }

//
// A Fast Memory Allocator for uniform sized objects
//

struct ChunkBlock;

class ChunkAlloc {
private:
	int itemSize;
	int blockN;					// the number of items to allocate in a block
	long tag;
	U16 blockId;				// the priority of the next block created
	BOOL noGarbageCollection;
	
	ChunkBlock* firstBlock;		// The list of chunk blocks
	ChunkBlock* lastBlock;

	ChunkBlock* firstFree;		// The lowest priority block that has free items

	#ifdef DEBUG
	int maxUsed;
	int nBlocks;
	int nAlloc;
	#endif
	
private:
	ChunkBlock* CreateChunk();
	void FreeChunk(ChunkBlock* b);
	
public:
	ChunkAlloc(int size, int n = 64, BOOL suppressGC = false, long tg = 0);
	~ChunkAlloc();

	void FreeEmpties();	// deallocate any empty chunks
	
	void* Alloc();
	void Free(void*);
};

inline void Spin() {}

// With the Unix port of the Flash Generator the Flash codebase must 
// now port to a number of different platforms using number of different 
// compilers.  The defines below try the determine whether the platform 
// uses big-endian integer values (Mac 68K, Mac PPC, and most RISC 
// systems) or whether it uses little-endian ingeteger values (Intel 
// x86 systems).  Add additional defines for different compilers as 
// needed.

#if defined(_MAC) || defined(_M_MPPC) || defined(__sparc) || defined(sparc) || defined(_AIX)
	#undef BIG_ENDIAN
	#undef LITTLE_ENDIAN
	
	#define BIG_ENDIAN
#elif defined(_M_IX86) || defined(__i386) || defined(i386) || defined(__linux)
	#undef BIG_ENDIAN
	#undef LITTLE_ENDIAN
	
	#define LITTLE_ENDIAN
#elif
	// Throw an error if our "endian-ness" not defined!
	@error@
#endif

#endif // SGLOBAL_INCLUDED
