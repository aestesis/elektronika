/***************************************************************************\ 
 *
 *               (C) copyright Fraunhofer - IIS (1998)
 *                        All Rights Reserved
 *
 *   filename: mp3streaminfo.h
 *   project : MPEG Layer-3 Decoder
 *   author  : Martin Sieler
 *   date    : 1998-05-27
 *   contents/description: current bitstream parameters
 *
\***************************************************************************/

/*
 * $Date: 2003/12/18 10:12:07 $
 * $Header: /CVS/code/elektroSDK/src/Alib/flashSDK/Source/mp3streaminfo.h,v 1.1.1.1 2003/12/18 10:12:07 yoy Exp $
 */

#ifndef __MP3STREAMINFO_H__
#define __MP3STREAMINFO_H__

/*-------------------------------------------------------------------------*/

typedef struct
{
  int   m_Layer;              /* ISO/MPEG Layer                      */
  int   m_MpegVersion;        /* ISO/MPEG Version                    */
  int   m_Bitrate;            /* Bitrate (Bit/s)                     */
  int   m_BitrateIndex;       /* ISO/MPEG Bitrate index of frame     */
  int   m_Channels;           /* Number of Channels (as indicated)   */
  int   m_SFreq;              /* Sampling Frequency (as indicated)   */
  int   m_EffectiveChannels;  /* Number of effective output channels */
  int   m_EffectiveSFreq;     /* Effective Sampling Frequency        */
  int   m_BitsPerSample;      /* Number of bits per Output Sample    */
  int   m_BitsPerFrame;       /* Number of bits in frame             */
  float m_Duration;           /* Duration of frame in milli seconds  */
  int   m_CrcError;           /* Indication of CRC Errors            */
  int   m_NoMainData;         /* Indication of missing main data     */
  } MP3STREAMINFO;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus

/* Mp3 Streaminfo object.

  Object holding information on the last successfully decode frame.

*/
class CMp3StreamInfo : public MP3STREAMINFO
{
public:

  CMp3StreamInfo() { Reset(); }

  /** ISO/MPEG Layer
  */
  int   GetLayer()             const { return m_Layer; }

  /** ISO/MPEG Version
  */
  int   GetMpegVersion()       const { return m_MpegVersion; }

  /** Bitrate (Bit/s)
  */
  int   GetBitrate()           const { return m_Bitrate; }

  /** ISO/MPEG Bitrate index of frame
  */
  int   GetBitrateIndex()      const { return m_BitrateIndex; }

  /** Number of Channels (as indicated)
  */
  int   GetChannels()          const { return m_Channels; }

  /** Sampling Frequency (as indicated)
  */
  int   GetSFreq()             const { return m_SFreq; }

  /** Number of effective output channels
  */
  int   GetEffectiveChannels() const { return m_EffectiveChannels; }

  /** Effective Sampling Frequency
  */
  int   GetEffectiveSFreq()    const { return m_EffectiveSFreq; }

  /** Number of bits per Output Sample
  */
  int   GetBitsPerSample()     const { return m_BitsPerSample; }

  /** Number of bits in frame
  */
  int   GetBitsPerFrame()      const { return m_BitsPerFrame; }

  /** Duration of frame in milli seconds
  */
  float GetDuration()          const { return m_Duration; }

  /** Indication of CRC Errors
  */
  int   GetCrcError()          const { return m_CrcError; }
  int   GetNoMainData()        const { return m_NoMainData; }

  void SetLayer(int nValue)             { m_Layer = nValue; }
  void SetMpegVersion(int nValue)       { m_MpegVersion = nValue; }
  void SetBitrate(int nValue)           { m_Bitrate = nValue; }
  void SetBitrateIndex(int nValue)      { m_BitrateIndex = nValue; }
  void SetChannels(int nValue)          { m_Channels = nValue; }
  void SetSFreq(int nValue)             { m_SFreq = nValue; }
  void SetEffectiveChannels(int nValue) { m_EffectiveChannels = nValue; }
  void SetEffectiveSFreq(int nValue)    { m_EffectiveSFreq = nValue; }
  void SetBitsPerSample(int nValue)     { m_BitsPerSample = nValue; }
  void SetBitsPerFrame(int nValue)      { m_BitsPerFrame = nValue; }
  void SetDuration(float fValue)        { m_Duration = fValue; }
  void SetCrcError(int nValue)          { m_CrcError = nValue; }
  void SetNoMainData(int nValue)        { m_NoMainData = nValue; }

  void Reset()
    {
    m_Layer             = 0;
    m_MpegVersion       = 0;
    m_Bitrate           = 0;
    m_BitrateIndex      = 0;
    m_Channels          = 0;
    m_SFreq             = 0;
    m_EffectiveChannels = 0;
    m_EffectiveSFreq    = 0;
    m_BitsPerSample     = 0;
    m_BitsPerFrame      = 0;
    m_Duration          = 0.0f;
    m_CrcError          = 0;
    m_NoMainData        = 0;
    }
};

#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/
#endif
