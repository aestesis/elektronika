/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef MEMCOP_INCLUDED
#define MEMCOP_INCLUDED

#include "global.h"

// These are all stubed out for the player since it should always be able to
//	handle a allocation failure

// The Memory Cop API

inline BOOL MemCopInit(int minPartition, int reserve) { return true; }
// Initialize the memory cop and check for a minimum partition size
//	returns false if the partition is too small

inline BOOL MemIsLow() { return false; }
// Check to see if memory is low and we should avoid any allocations

inline BOOL MemIsStressed() { return false; }
// Returns true if we have been purging code segments to free memory

inline void CheckMem() {}
// Raise an exception if we are low on memory

inline BOOL MemRebuildReserve() { return true; }
// Check to rebuild the reserve

inline void MemAllowFail(BOOL f) {}
// Set if the next memory allocation that we try should not cut 
//  into the memory reserve, these calls should never be nested

inline void MemForceSuccess(BOOL f) {FLASHASSERT(false);}
// Lower the out of memory threshold-this should only be used for
//	error recovery so that we can back out of 

inline void MemReduceReserve(BOOL f) {}
// Lower the out of memory threshold-this should only be used for
//	less strict than ForceSuccess, used when rendering the screen

inline void SetMemWarned() {}

inline void PreMemAllowFail() {}
inline void PostMemAllowFail(BOOL failed) {}

// #if defined(_MAC)
// extern BOOL allocFailed;  // true if a recent allocation failed
// extern BOOL allocFailedOnce; // true if an allocation has ever failed in this session
// 
// #if defined(PLAYER)
// BOOL TempMemSeemsLow(Size size);
// BOOL AppMemSeemsLow(Size size);
// #endif
// 
// #endif

#endif
