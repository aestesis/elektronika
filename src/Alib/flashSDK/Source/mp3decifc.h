/***************************************************************************\
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   filename: mp3decifc.h
 *   project : MPEG Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-28
 *   contents/description: Mp3 Decoder interface (C-style)
 *
 *
\***************************************************************************/

/*
 * $Date: 2003/12/18 10:12:07 $
 * $Header: /CVS/code/elektroSDK/src/Alib/flashSDK/Source/mp3decifc.h,v 1.1.1.1 2003/12/18 10:12:07 yoy Exp $
 */

#ifndef __MP3DECIFC_H__
#define __MP3DECIFC_H__

/* ------------------------ includes --------------------------------------*/

#include "mp3sscdef.h"
#include "mp3streaminfo.h"

/*-------------------------- defines --------------------------------------*/

/* define WINAPI for non Windows platforms */
#ifndef WINAPI
  #define WINAPI
#endif

/*-------------------------------------------------------------------------*/

typedef void *MP3DEC_HANDLE;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
  extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/** Open an instance of the mp3 decoder.

  @return An SSC status code (see mp3sscdef.h)

  @param handle        Address filled with a handle identifying the open mp3 
                       decoder instance. Use the handle to identify the mp3
                       decoder instance when calling other mp3 decoder 
                       functions.

  @param Quality       Select what parts of the spectrum should be decoded.
                       Set to '0' for best quality
                       (0: full, 1: half, 2: quarter).

  @param Resolution    Selects 8 bit and 16 bit PCM output. Set to '0' for
                       best quality
                       (0: 16 bit, 1: 8 bit).

  @param Downmix       Select downmix mode. Set to '0' for best quality
                       (0: no downmix, 1: downmix).

  @param IntegerWindow Set polyphase windowing mode. Should run faster on 
                       80486 CPUs when set to '1'. Set to '0' otherwise
                       (0: float, 1: integer).

*/

SSC WINAPI mp3decOpen
    (
    MP3DEC_HANDLE *handle,
    int            Quality,
    int            Resolution,
    int            Downmix,
    int            IntegerWindow
    );

/*-------------------------------------------------------------------------*/
/** Close an open instance of the mp3 decoder.

  @return An SSC status code (see mp3sscdef.h)

  @param handle Handle of an open mp3 decoder instance. If the function
                succeeds, the handle is no longer valid after this call.

*/

SSC WINAPI mp3decClose(MP3DEC_HANDLE handle);

/*-------------------------------------------------------------------------*/
/** Reset an open instance of the mp3 decoder.

  @return An SSC status code (see mp3sscdef.h)

  @param handle Handle of an open mp3 decoder instance.

*/

SSC WINAPI mp3decReset(MP3DEC_HANDLE handle);

/*-------------------------------------------------------------------------*/
/** Decode one mp3 frame.

  @return An SSC status code (see mp3sscdef.h)

  @param handle  Handle of an open mp3 decoder instance.

  @param pPcm    Address of a buffer receiving the decoded PCM data.

  @param cbPcm   Size of buffer in bytes pointed to by pPcm.

  @param pcbUsed Address of an int receiving the number of bytes written to
                 buffer pointed to by pPcm.

*/

SSC WINAPI mp3decDecode
    (
    MP3DEC_HANDLE  handle, 
    unsigned char *pPcm, 
    int            cbPcm, 
    int           *pcbUsed
    );

/*-------------------------------------------------------------------------*/
/** Get information about the last successfully decoded frame.

  @return Pointer to an MP3STREAMINFO structure (see mp3streaminfo.h)

  @param handle Handle of an open mp3 decoder instance.

*/

const MP3STREAMINFO * WINAPI mp3decGetStreamInfo(MP3DEC_HANDLE handle);

/*-------------------------------------------------------------------------*/
/** Provide mp3 data to the mp3 decoder.

  @return An SSC status code (see mp3sscdef.h)

  @param handle    Handle of an open mp3 decoder instance.

  @param pBuffer   Address of a buffer containing the mp3 data.

  @param cbBuffer  Size of buffer in bytes pointed to by pBuffer.

  @param pcbCopied Address of an int receiving the number of bytes copied
                   to the mp3 decoder input buffer.
*/

SSC WINAPI mp3decFill
    (
    MP3DEC_HANDLE        handle,
    const unsigned char *pBuffer,
    int                  cbBuffer,
    int                 *pcbCopied
    );

/*-------------------------------------------------------------------------*/
/** Check, how many bytes could be provided to the mp3 decoder.

  @return An SSC status code (see mp3sscdef.h)

  @param handle Handle of an open mp3 decoder instance.

  @param pValue Address of an int to receive number of bytes that
                could be provided to the mp3 decoder by \Ref{mpgadecFill}.

*/

SSC WINAPI mp3decGetInputFree(MP3DEC_HANDLE handle, int *pValue);

/*-------------------------------------------------------------------------*/
/** Check, how many bytes are left in the mp3 decoder input buffer.

  @return An SSC status code (see mp3sscdef.h)

  @param handle Handle of an open mp3 decoder instance.

  @param pValue Address of an int to receive number of bytes left
                in the mp3 decoder input buffer.

*/

SSC WINAPI mp3decGetInputLeft(MP3DEC_HANDLE handle, int *pValue);

/*-------------------------------------------------------------------------*/
/** Indicate EOF (end-of-file) to the mp3 decoder.

  @return An SSC status code (see mp3sscdef.h)

  @param handle Handle of an open mp3 decoder instance.

*/

SSC WINAPI mp3decSetInputEof(MP3DEC_HANDLE handle);

/*-------------------------------------------------------------------------*/
/** Check if the mp3 decoder reached EOF.

  @return '0': EOF not yet reached, else: EOF reached.

  @param handle Handle of an open mp3 decoder instance.

*/

int WINAPI mp3decIsEof(MP3DEC_HANDLE handle);

/*-------------------------------------------------------------------------*/
/** Get a textual description of an SSC status code.

  @return Address of a buffer containing the textual description.

  @param handle Handle of an open mp3 decoder instance.

  @param ssc    SSC status code to get the textual description for.
*/

const char * WINAPI mp3decGetErrorText(MP3DEC_HANDLE handle, SSC ssc);

#ifdef __cplusplus
  }
#endif

/*-------------------------------------------------------------------------*/
#endif
