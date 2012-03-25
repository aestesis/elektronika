/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1997)
 *                        All Rights Reserved
 *
 *   filename: bitstream.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1997-12-05
 *   contents/description: generic bitbuffer - HEADER
 *
 *
\***************************************************************************/

/*
 * $Date: 2003/12/18 10:12:06 $
 * $Header: /CVS/code/elektroSDK/src/Alib/flashSDK/Source/bitstream.h,v 1.1.1.1 2003/12/18 10:12:06 yoy Exp $
 */

#ifndef _BITSTREAM_H
#define _BITSTREAM_H

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

class CGioBase;

/*-------------------------------------------------------------------------*/

/* Bitstream input class.

    This class defines the interface that the mp3 decoder object will
    read all of its bitstream input data from.

*/
class CBitStream
{
public:

  CBitStream(int cbSize);
  CBitStream(unsigned char *pBuf, int cbSize, BOOL fDataValid = false);
  virtual ~CBitStream();

  virtual void   Reset();

  void           Connect(CGioBase *_pGB);

  void           ResetBitCnt()     { m_BitCnt = 0;    }
  int            GetBitCnt() const { return m_BitCnt; }

  unsigned int   GetBits(unsigned int nBits);
  unsigned long  Get32Bits();

  int            Ff(int nBits);
  int            Rewind(int nBits);
  int            Seek(int nBits)
    {  
    m_BitCnt    += nBits;
    m_ValidBits -= nBits;
    m_BitNdx     = (m_BitNdx+nBits) & (m_nBits-1);
    return 1;
    }

  int            GetValidBits() const { return m_ValidBits; }
  int            GetFree()      const;

  void           SetEof();
  int            Fill(const unsigned char *pBuf, int cbSize);
  int            Fill(CBitStream &Bs, int cbSize);

protected:

  int            Refill();
  BOOL           IsEof()       const;
  BOOL           IsConnected() const;

private:

  CGioBase      *m_pGB;           // I/O object
  int            m_nBytes;        // size of buffer in bytes
  int            m_nBits;         // size of buffer in bits
  int            m_ValidBits;     // valid bits in buffer
  int            m_ReadOffset;    // where to write next
  int            m_BitCnt;        // bit counter
  int            m_BitNdx;        // position of next bit in byte
  BOOL           m_fEof;          // indication of input eof
  unsigned char *m_Buf;           // the buffer
  BOOL           m_fBufferIntern; // did we allocate the buffer ourselves
};

/*-------------------------------------------------------------------------*/
#endif
