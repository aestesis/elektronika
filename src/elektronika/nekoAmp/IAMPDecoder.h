#ifndef f_AMPLIB_IAMPDECODER_H
#define f_AMPLIB_IAMPDECODER_H

class AMPStreamInfo {
public:
	long	lBitrate;			// average bits/second for this stream (0=unconstrained)
	long	lSamplingFreq;		// sampling frequency (Hz)
	char	nLayer;				// MPEG audio layer (1-3)
	char	nMPEGVer;			// MPEG version (1/2)
	char	fStereo;			// true: stereo, false: mono
};

class IAMPBitsource {
public:
	virtual int read(void *buffer, int bytes)=0;
};

class IAMPDecoder {
public:

	enum {
		ERR_NONE			= 0,
		ERR_EOF				= 1,
		ERR_READ			= 2,
		ERR_MPEG25			= 3,
/*		ERR_LAYER1			= 4,*/ /* Not applicable to NekoAmp1.4 */
		ERR_FREEFORM		= 5,
		ERR_SYNC			= 6,
		ERR_INTERNAL		= 7,
		ERR_INCOMPLETEFRAME	= 8,
	};

	virtual void	Destroy()=0;

	virtual char *	GetAmpVersionString()				=0;
	virtual void	Init()								=0;
	virtual void	setSource(IAMPBitsource *pSource)	=0;
	virtual void	setDestination(short *psDest)		=0;
	virtual long	getSampleCount()					=0;
	virtual void	getStreamInfo(AMPStreamInfo *pasi)	=0;
	virtual char *	getErrorString(int err)				=0;
	virtual void	Reset()								=0;
	virtual void	ReadHeader()						=0;
	virtual void	PrereadFrame()						=0;
	virtual bool	DecodeFrame()						=0;
};

extern IAMPDecoder *CreateAMPDecoder();

#endif

