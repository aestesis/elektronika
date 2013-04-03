/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1997)
 *                        All Rights Reserved
 *
 *   filename: mpegbitstream.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-05
 *   contents/description: MPEG bitstream - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2003/12/18 10:12:07 $
 * $Header: /CVS/code/elektroSDK/src/Alib/flashSDK/Source/mpegbitstream.h,v 1.1.1.1 2003/12/18 10:12:07 yoy Exp $
 */

#ifndef _MPEGBITSTREAM_H
#define _MPEGBITSTREAM_H

/* ------------------------ includes --------------------------------------*/

#include "bitstream.h"
#include "mpegheader.h"
#include "mp3sscdef.h"

/*-------------------------- defines --------------------------------------*/

/*-------------------------------------------------------------------------*/

/* MPEG bitstream class.

  This object is derived from \Ref{CBitStream}. In addition to \Ref{CBitStream}
  this object is able to sync to the next ISO/MPEG header position.

*/
class CMpegBitStream : public CBitStream
{
public:
  CMpegBitStream(int cbSize);
  CMpegBitStream(unsigned char *pBuf, int cbSize, BOOL fDataValid = false);
  virtual ~CMpegBitStream();

  virtual void       Reset();

  SSC                DoSync();
  int                GetSyncPosition() const { return m_SyncPosition; }
  const CMpegHeader *GetHdr() const { return &m_Hdr; }

  enum 
    {
    // sync found
    SYNC_OK       = SSC_OK,
    // sync not found yet, not enough data
    SYNC_SEARCHED = SSC_W_MPGA_SYNCSEARCHED,
    // sync lost, next call will re-sync
    SYNC_LOST     = SSC_W_MPGA_SYNCLOST,
    // not enough data to check sync
    SYNC_NEEDDATA = SSC_W_MPGA_SYNCNEEDDATA,
    // end of stream reached, no more data will follow
    SYNC_EOF      = SSC_W_MPGA_SYNCEOF
    };

protected:

private:
  SSC DoSyncInitial();
  SSC DoSyncContinue();

  CMpegHeader   m_Hdr;
  unsigned long m_FirstHdr;
  int           m_SyncPosition;
  SSC           m_SyncState;
};

/*-------------------------------------------------------------------------*/
#endif
