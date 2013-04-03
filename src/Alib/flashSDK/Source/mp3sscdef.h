/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   filename: mp3sscdef.h
 *   project : ---
 *   author  : Martin Sieler
 *   date    : 1998-02-16
 *   contents/description: ssc definitions (Structured Status Code)
 *
 *
\***************************************************************************/

/*
 * $Date: 2003/12/18 10:12:07 $
 * $Header: /CVS/code/elektroSDK/src/Alib/flashSDK/Source/mp3sscdef.h,v 1.1.1.1 2003/12/18 10:12:07 yoy Exp $
 */

#ifndef _MP3SSCDEF_H
#define _MP3SSCDEF_H

/*
 * this file needs to be re-written (re-structured)
 * for now it should work
 */

/* ------------------------ includes --------------------------------------*/

/*-------------------------- defines --------------------------------------*/

typedef unsigned long SSC;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus

/* Helper class for more information about SSC codes.
*/
class CMp3Ssc
{
public:
  /** Object constructor

    @param An SSC staus code to initialize the object with.

  */
  CMp3Ssc(SSC _ssc = 0xffffffff);
  ~CMp3Ssc() {}

  /** Operator for conversion to a text string.

    @return Textual description.

  */
  operator const char*();

  /** Get textual description for an SSC status code.
  
    @return Address of a buffer containing the textual description.
    
    @param _ssc An SSC status code.

  */
  const char *GetText(SSC _ssc);

private:
  SSC  ssc;
  char szText[100];
};

#endif /* ifdef __cplusplus */

/*-------------------------------------------------------------------------*/

/*
 *
 *  Standard error/return values are 32 bit values layed out as follows:
 *
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +---+-+-+-----------------------+-------------------------------+
 *  |Sev|C|R|     Handler           |               Code            |
 *  +---+-+-+-----------------------+-------------------------------+
 *
 *  where
 *
 *      Sev - is the severity code
 *
 *          00 - Success
 *          01 - Informational
 *          10 - Warning
 *          11 - Error
 *
 *      C       - is the Customer code flag
 *
 *      R       - is a reserved bit
 *
 *      Handler - is the handler code
 *
 *      Code    - is the facility's status code
 *
 */

/*
 * define the Severity codes
 */

#define SSC_SEV_SUCCESS  0x00000000
#define SSC_SEV_INFO     0x40000000
#define SSC_SEV_WARNING  0x80000000
#define SSC_SEV_ERROR    0xc0000000

/*
 * define masks to extract the fields
 */

#define SSC_MASK_SEVERITY 0xc0000000
#define SSC_MASK_HANDLER  0x0fff0000
#define SSC_MASK_CODE     0x0000ffff

/*
 * define MACROS to test an error/return code
 */

#define SSC_GETSEV(x) ( (x) & SSC_MASK_SEVERITY )

/** @name SSC Macros */

/*@{*/

/** Check, if an SSC indicates success.
*/
#define SSC_SUCCESS(x) (((SSC_GETSEV(x)==SSC_SEV_SUCCESS)||(SSC_GETSEV(x)==SSC_SEV_INFO))?1:0)

/** Check, if an SSC indicates an information.
*/
#define SSC_INFO(x)    ((SSC_GETSEV(x)==SSC_SEV_INFO)?1:0)

/** Check, if an SSC indicates a warning.
*/
#define SSC_WARNING(x) ((SSC_GETSEV(x)==SSC_SEV_WARNING)?1:0)

/** Check, if an SSC indicates an error.
*/
#define SSC_ERROR(x)   ((SSC_GETSEV(x)==SSC_SEV_ERROR)?1:0)

/*@}*/

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

#define SSC_OK                  0x00000000

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

#define SSC_HANDLER_GEN 0x00000000

#define SSC_I_GEN (SSC_SEV_INFO    | SSC_HANDLER_GEN)
#define SSC_W_GEN (SSC_SEV_WARNING | SSC_HANDLER_GEN)
#define SSC_E_GEN (SSC_SEV_ERROR   | SSC_HANDLER_GEN)


#define SSC_E_WRONGPARAMETER        (SSC_E_GEN | 1)
#define SSC_E_OUTOFMEMORY           (SSC_E_GEN | 2)
#define SSC_E_INVALIDHANDLE         (SSC_E_GEN | 3)

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

#define SSC_HANDLER_IO 0x00010000

#define SSC_I_IO (SSC_SEV_INFO    | SSC_HANDLER_IO)
#define SSC_W_IO (SSC_SEV_WARNING | SSC_HANDLER_IO)
#define SSC_E_IO (SSC_SEV_ERROR   | SSC_HANDLER_IO)

#define SSC_E_IO_GENERIC     (SSC_W_IO | 1)
#define SSC_E_IO_OPENFAILED  (SSC_W_IO | 2)
#define SSC_E_IO_CLOSEFAILED (SSC_W_IO | 3)
#define SSC_E_IO_READFAILED  (SSC_W_IO | 4)

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

#define SSC_HANDLER_MPGA 0x01010000

#define SSC_I_MPGA (SSC_SEV_INFO    | SSC_HANDLER_MPGA)
#define SSC_W_MPGA (SSC_SEV_WARNING | SSC_HANDLER_MPGA)
#define SSC_E_MPGA (SSC_SEV_ERROR   | SSC_HANDLER_MPGA)

#define SSC_I_MPGA_CRCERROR       (SSC_I_MPGA |  1)
#define SSC_I_MPGA_NOMAINDATA     (SSC_I_MPGA |  2)

#define SSC_E_MPGA_GENERIC        (SSC_E_MPGA |  1)
#define SSC_E_MPGA_WRONGLAYER     (SSC_E_MPGA |  2)
#define SSC_E_MPGA_BUFFERTOOSMALL (SSC_E_MPGA |  3)

#define SSC_W_MPGA_SYNCSEARCHED   (SSC_W_MPGA |  1)
#define SSC_W_MPGA_SYNCLOST       (SSC_W_MPGA |  2)
#define SSC_W_MPGA_SYNCNEEDDATA   (SSC_W_MPGA |  3)
#define SSC_W_MPGA_SYNCEOF        (SSC_W_MPGA |  4)

/*-------------------------------------------------------------------------*/
#endif
