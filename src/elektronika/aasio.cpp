/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aasio						An Asio MultipleInstance C++ Wrapper
// developer:					renan jegouzo 
// email:						renan@aestesis.org
// web:							aestesis.org 
// (C) 2005 AESTESIS.ORG
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"aasio.h"
#include						<stdio.h>
#include						<string.h>
#include						<assert.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
	#define ASIO64toDouble(a)  (a)
#else
	const double twoRaisedTo32 = 4294967296.;
	#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aasio
{
public:
								Aasio							();

	AsioDriverList				list;
	AasioInstance				inst[MAXASIOINSTANCE];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Aasio					asio;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aasio::Aasio()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AasioInstance::AasioInstance()
{
	driver=NULL;
	device=-1;
	memset(&info, 0, sizeof(info));
	memset(&channel, 0, sizeof(channel));
	memset(&channelClient, 0, sizeof(channelClient));
	nbClient=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AasioInstance::~AasioInstance()
{
	release(null);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AasioInstance::create(int device)
{
	if(nbClient)
		return true;
	nbClient++;

	bool	b=false;
	this->device=device;
	if(asio.list.asioOpenDriver(device, (void **)&driver)==S_OK)
	{
		if(driver->init(&info.driverInfo.sysRef))
		{
			driver->getDriverName(info.driverInfo.name);
			info.driverInfo.driverVersion = driver->getDriverVersion();

			if(driver->getChannels(&info.nbInput, &info.nbOutput)==ASE_OK)
			{
				info.nbChannel=info.nbInput+info.nbOutput;

				if(driver->getSampleRate(&info.sampleRate)==ASE_OK)
				{
					//if((info.sampleRate<=0.0)||(info.sampleRate>96000.0))
					{
						if(driver->setSampleRate(44100.0) == ASE_OK)
							driver->getSampleRate(&info.sampleRate);
					}
				}

				if((info.sampleRate>0.0)&&(info.sampleRate<=96000.0))
				{
					if(driver->getBufferSize(&info.minSize, &info.maxSize, &info.preferredSize, &info.granularity) == ASE_OK)
					{
						if(driver->outputReady()==ASE_OK)
							info.postOutput = true;
						else
							info.postOutput = false;

						{
							ASIOBufferInfo	*inf = info.bufferInfo;
							int				i;

							for(i = 0; i < info.nbInput; i++)
							{
								inf->isInput = ASIOTrue;
								inf->channelNum = i;
								inf->buffers[0] = inf->buffers[1] = 0;
								inf++;
							}

							for(i= 0; i<info.nbOutput; i++)
							{
								inf->isInput = ASIOFalse;
								inf->channelNum = i;
								inf->buffers[0] = inf->buffers[1] = 0;
								inf++;
							}
						}

						setCallback();

						{	// create and activate buffers
							long result = driver->createBuffers(info.bufferInfo, info.nbChannel, info.preferredSize, &info.callbacks);
							if (result == ASE_OK)
							{
								int	i;
								for (i = 0; i < info.nbChannel; i++)
								{
									info.channelInfo[i].channel = info.bufferInfo[i].channelNum;
									info.channelInfo[i].isInput = info.bufferInfo[i].isInput;
									result = driver->getChannelInfo(&info.channelInfo[i]);
									if (result != ASE_OK)
										break;
								}

								if (result == ASE_OK)
								{
									result=driver->getLatencies(&info.inputLatency, &info.outputLatency);
									driver->start();
									b=true;
								}
							}
						}
					}
				}
			}
		}
	}
	if(!b)
		release(null);
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::release(AasioClient *client)
{
	if(client)
		releaseChannel(client);
	if((--nbClient<=0)||(!client))
	{
		if(driver)
		{
			int	i;
			driver->stop();
			driver->disposeBuffers();
			asio.list.asioCloseDriver(device);
			driver=NULL;
			device=-1;
			nbClient=0;
			memset(&info, 0, sizeof(info));
			for(i=0; i<MAXASIOCHANNEL; i++)
			{
				assert(channelClient[i]==null);
				channelClient[i]=null;
				if(channel[i])
				{
					delete(channel[i]);
					channel[i]=null;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::controlPanel()
{
	if(driver)
		driver->controlPanel();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AasioInstance::useChannel(AasioClient *client, int left, int right)
{
	bool	b=false;
	section.enter(__FILE__,__LINE__);
	if((channelClient[left]==null)&&(channelClient[right]==null))
	{
		channelClient[left]=client;
		channelClient[right]=client;
		channel[left]=new Abuffer("channel", 65536);
		channel[right]=new Abuffer("channel", 65536);
		b=true;
	}
	section.leave();
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::releaseChannel(AasioClient *client)
{
	int	i;
	section.enter(__FILE__,__LINE__);
	for(i=0; i<MAXASIOCHANNEL; i++)
	{
		if(channelClient[i]==client)
		{
			channelClient[i]=null;
			if(channel[i])
			{
				delete(channel[i]);
				channel[i]=null;
			}
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioInstance::getBufferSize(int nchannel)
{
	int	v=0;
	section.enter(__FILE__,__LINE__);
	v=(int)((double)channel[nchannel]->getSize()*44100.0/info.sampleRate);
	section.leave();
	return v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::stretchAudio(sword *out, int nout, sword *in, int nin, short *membuf)
{
	int		nsmp=nin;
	sword	*outbuf=out;
	sword	*buf=in;

	if(nout>nsmp)	// slow play
	{
		double	s=0.f;
		double	ds=(float)(nsmp)/(float)nout;
		int		i;
		int		di=0;
		for(i=0; i<nout; i++)
		{
			int		si=(int)s;
			float	a1=(float)(s-(float)si);
			float	a0=1.f-a1;
			float	v0,v1;
			int		di=si;
			si--;
			int		si2=si<<1;
			int		di2=di<<1;
			int		i2=i<<1;
			if(si<0)
			{
				v0=(float)membuf[0]*a0;
				v1=(float)membuf[1]*a0;
			}
			else
			{
				v0=(float)buf[si2]*a0;
				v1=(float)buf[si2+1]*a0;
			}
			v0+=(float)buf[di2]*a1;
			v1+=(float)buf[di2+1]*a1;
			outbuf[i2]=(sword)v0;
			outbuf[i2+1]=(sword)v1;
			s+=ds;
		}
	}
	else	// nout<nsmp // speed play
	{
		double	s=0.f;
		double	ds=(float)nsmp/(float)nout;
		int		i;
		for(i=0; i<nout; i++)
		{
			double	sn=s+ds;
			int		n0=(int)s;
			int		n1=(int)sn;
			float	a0=(float)(1.0-(s-n0));
			int		n02=n0<<1;
			int		n12=n1<<1;
			float	v0=buf[n02]*a0;
			float	v1=buf[n02+1]*a0;
			int		i2=i<<1;
			int		k;
			for(k=n0+1; k<n1; k++)
			{
				int	k2=k<<1;
				v0+=buf[k2];
				v1+=buf[k2+1];
			}
			if(n1<nsmp)
			{
				float	a1=(float)(sn-n1);
				v0+=buf[n12]*a1;
				v1+=buf[n12+1]*a1;
			}
			outbuf[i2]=(sword)(v0/ds);
			outbuf[i2+1]=(sword)(v1/ds);
			s=sn;
		}
	}
	membuf[0]=((sword *)buf)[(nsmp-1)<<1];
	membuf[1]=((sword *)buf)[((nsmp-1)<<1)+1];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::addBuffer(int left, int right, sword *buffer, int nsample, short *membuf)
{
	section.enter(__FILE__,__LINE__);

	assert(channel[left]&&channel[right]);

	{
		sword	outbuf[65536*2*3];

		int		nout=(int)((double)nsample*info.sampleRate/44100.0);

		assert(nsample<65536);
		assert(nout<65536*3);

		if(nout>0)
		{
			sword	*s;
			if(nsample==nout)
			{
				s=buffer;
				membuf[0]=buffer[((nout-1)<<1)];
				membuf[1]=buffer[((nout-1)<<1)+1];
			}
			else
			{
				stretchAudio(outbuf, nout, buffer, nsample, membuf);
				s=outbuf;
			}

			int		i;

			switch (info.channelInfo[left].type)
			{
				case ASIOSTInt16LSB:
				for(i=0; i<nout; i++)
				{	
					channel[left]->write(s++, sizeof(sword));
					channel[right]->write(s++, sizeof(sword));
				}
				break;
				case ASIOSTInt24LSB:		// used for 20 bits as well
				for(i=0; i<nout; i++)
				{	
					int	v=*(s++)<<8;
					channel[left]->write(&v, 3);
					v=*(s++)<<8;
					channel[right]->write(&v, 3);
				}
				break;
				case ASIOSTInt32LSB:
				for(i=0; i<nout; i++)
				{	
					int	v=*(s++)<<16;
					channel[left]->write(&v, sizeof(v));
					v=*(s++)<<16;
					channel[right]->write(&v, sizeof(v));
				}
				break;
				case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				for(i=0; i<nout; i++)
				{	
					float	v=(float)*(s++)/32768.f;
					channel[left]->write(&v, sizeof(v));
					v=(float)*(s++)/32768.f;
					channel[right]->write(&v, sizeof(v));
				}
				break;
				case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				for(i=0; i<nout; i++)
				{	
					double	v=(double)*(s++)/32768.0;
					channel[left]->write(&v, sizeof(v));
					v=(double)*(s++)/32768.0;
					channel[right]->write(&v, sizeof(v));
				}
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32LSB16:		// 32 bit data with 16 bit alignment
				for(i=0; i<nout; i++)
				{	
					int	v=*(s++);
					channel[left]->write(&v, sizeof(v));
					v=*(s++);
					channel[right]->write(&v, sizeof(v));
				}
				break;
				case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
				for(i=0; i<nout; i++)
				{	
					int	v=*(s++)<<2;
					channel[left]->write(&v, sizeof(v));
					v=*(s++)<<2;
					channel[right]->write(&v, sizeof(v));
				}
				break;
				case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
				for(i=0; i<nout; i++)
				{	
					int	v=*(s++)<<4;
					channel[left]->write(&v, sizeof(v));
					v=*(s++)<<4;
					channel[right]->write(&v, sizeof(v));
				}
				break;
				case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				for(i=0; i<nout; i++)
				{	
					int	v=*(s++)<<8;
					channel[left]->write(&v, sizeof(v));
					v=*(s++)<<8;
					channel[right]->write(&v, sizeof(v));
				}
				break;

				// mac (little endian - big endian - switch)

				case ASIOSTInt16MSB:
				//memset (info.bufferInfo[i].buffers[index], 0, buffSize * 2);
				break;
				case ASIOSTInt24MSB:		// used for 20 bits as well
				//memset (info.bufferInfo[i].buffers[index], 0, buffSize * 3);
				break;
				case ASIOSTInt32MSB:
				//memset (info.bufferInfo[i].buffers[index], 0, buffSize * 4);
				break;
				case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				//memset (info.bufferInfo[i].buffers[index], 0, buffSize * 4);
				break;
				case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				//memset (info.bufferInfo[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				//memset (info.bufferInfo[i].buffers[index], 0, buffSize * 4);
				break;
			}
		}
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ASIOTime * AasioInstance::bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.
	static int processedSamples = 0;

	// store the timeInfo for later use
	info.time = *timeInfo;

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (timeInfo->timeInfo.flags & kSystemTimeValid)
		info.nano = ASIO64toDouble(timeInfo->timeInfo.systemTime);
	else
		info.nano = 0;

	if (timeInfo->timeInfo.flags & kSamplePositionValid)
		info.samples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
	else
		info.samples = 0;

	if (timeInfo->timeCode.flags & kTcValid)
		info.tcSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
	else
		info.tcSamples = 0;

	// get the system reference time
	info.sysRefTime = timeGetTime();

/*
#if WINDOWS && _DEBUG
	// a few debug messages for the Windows device driver developer
	// tells you the time when driver got its interrupt and the delay until the app receives
	// the event notification.
	static double last_samples = 0;
	char tmp[128];
	sprintf (tmp, "diff: %d / %d ms / %d ms / %d samples                 \n", info.sysRefTime - (long)(info.nano / 1000000.0), info.sysRefTime, (long)(info.nano / 1000000.0), (long)(info.samples - last_samples));
	OutputDebugString (tmp);
	last_samples = info.samples;
#endif
*/

	// buffer size in samples
	long buffSize = info.preferredSize;

	section.enter(__FILE__,__LINE__);
	// perform the processing
	for (int i = 0; i < info.nbChannel; i++)
	{
		if (info.bufferInfo[i].isInput == false)
		{
			// OK do processing for the outputs only
			int	realSize=0;
			int	align;
			switch (info.channelInfo[i].type)
			{
				case ASIOSTInt16LSB:
				realSize=buffSize * 2;
				align=2;
				break;
				case ASIOSTInt24LSB:		// used for 20 bits as well
				realSize=buffSize * 3;
				align=3;
				break;
				case ASIOSTInt32LSB:
				realSize=buffSize * 4;
				align=4;
				break;
				case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				realSize=buffSize * 4;
				align=4;
				break;
				case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				realSize=buffSize * 8;
				align=8;
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				realSize=buffSize * 4;
				align=4;
				break;

				case ASIOSTInt16MSB:
				realSize=buffSize * 2;
				align=2;
				break;
				case ASIOSTInt24MSB:		// used for 20 bits as well
				realSize=buffSize * 3;
				align=3;
				break;
				case ASIOSTInt32MSB:
				realSize=buffSize * 4;
				align=4;
				break;
				case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				realSize=buffSize * 4;
				align=4;
				break;
				case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				realSize=buffSize * 8;
				align=8;
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
				case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
				case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
				case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				realSize=buffSize * 4;
				align=4;
				break;
			}
			if(realSize)
			{
				bool	ok=false;
				if(channel[i])
				{
					int	n=(int)channel[i]->getSize();
					if(!info.quality[i].stopped)
					{
						if(n>realSize&&info.quality)
						{
							channel[i]->read(info.bufferInfo[i].buffers[index], realSize);
							ok=true;
						}
						else
						{
							info.quality[i].stopped=true;
						}
					}
					if(n>info.quality[i].mBufSize)	// new sample added to buffer
					{
						info.quality[i].ratio=(float)n/(float)info.quality[i].mUsed;
						if(info.quality[i].stopped)
						{
							if(info.quality[i].ratio>1.5f)
								info.quality[i].stopped=false;
						}
						else
						{
							if(info.quality[i].ratio>3.f)
							{
								int	ns=(int)((float)(n-realSize)-(float)info.quality[i].mUsed*1.5f);
								channel[i]->seek(channel[i]->offset+(align*(ns/align)));	// alligned on sample size
							}
						}
						info.quality[i].mUsed=0;
					}
					info.quality[i].mBufSize=n;
					info.quality[i].mUsed+=realSize;
				}
				else
				{
					info.quality[i].mBufSize=0;
					info.quality[i].mUsed=realSize*20;
					info.quality[i].stopped=false;
				}

				if(!ok)
					memset (info.bufferInfo[i].buffers[index], 0, realSize);
			}
		}
	}
	section.leave();

	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	if (info.postOutput)
		driver->outputReady();

	return 0L;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::bufferSwitch(long index, ASIOBool processNow)
{	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset (&timeInfo, 0, sizeof (timeInfo));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if(driver->getSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo (&timeInfo, index, processNow);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::sampleRateChanged(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
	section.enter(__FILE__,__LINE__);
	info.sampleRate=sRate;
	/*
	driver->stop();
	driver->disposeBuffers();
	{	// create and activate buffers
		long result = driver->createBuffers(info.bufferInfo, info.nbChannel, info.preferredSize, &info.callbacks);
		if (result == ASE_OK)
		{
			int	i;
			for (i = 0; i < info.nbChannel; i++)
			{
				info.channelInfo[i].channel = info.bufferInfo[i].channelNum;
				info.channelInfo[i].isInput = info.bufferInfo[i].isInput;
				result = driver->getChannelInfo(&info.channelInfo[i]);
				if (result != ASE_OK)
					break;
			}

			if (result == ASE_OK)
			{
				result=driver->getLatencies(&info.inputLatency, &info.outputLatency);
				driver->start();
				b=true;
			}
		}
	}
	*/
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long AasioInstance::asioMessages(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
			if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			//asioDriverInfo.stopped;  // In this sample the processing will just stop
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define genBufferSwitch(x) \
	void bufferSwitch##x (long index, ASIOBool processNow)\
	{\
		asio.inst[x].bufferSwitch(index, processNow);\
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define genBufferSwitchTimeInfo(x) \
	ASIOTime* bufferSwitchTimeInfo##x (ASIOTime *timeInfo, long index, ASIOBool processNow)\
	{\
		return asio.inst[x].bufferSwitchTimeInfo(timeInfo, index, processNow);\
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define genSampleRateChanged(x) \
	void sampleRateChanged##x (ASIOSampleRate sRate)\
	{\
		asio.inst[x].sampleRateChanged(sRate);\
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define genAsioMessages(x) \
	long asioMessages##x (long selector, long value, void* message, double* opt)\
	{\
		return asio.inst[x].asioMessages(selector, value, message, opt);\
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

genBufferSwitch(0)
genBufferSwitch(1)
genBufferSwitch(2)
genBufferSwitch(3)
genBufferSwitch(4)
genBufferSwitch(5)
genBufferSwitch(6)
genBufferSwitch(7)
genBufferSwitch(8)
genBufferSwitch(9)
genBufferSwitch(10)
genBufferSwitch(11)
genBufferSwitch(12)
genBufferSwitch(13)
genBufferSwitch(14)
genBufferSwitch(15)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

genBufferSwitchTimeInfo(0)
genBufferSwitchTimeInfo(1)
genBufferSwitchTimeInfo(2)
genBufferSwitchTimeInfo(3)
genBufferSwitchTimeInfo(4)
genBufferSwitchTimeInfo(5)
genBufferSwitchTimeInfo(6)
genBufferSwitchTimeInfo(7)
genBufferSwitchTimeInfo(8)
genBufferSwitchTimeInfo(9)
genBufferSwitchTimeInfo(10)
genBufferSwitchTimeInfo(11)
genBufferSwitchTimeInfo(12)
genBufferSwitchTimeInfo(13)
genBufferSwitchTimeInfo(14)
genBufferSwitchTimeInfo(15)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

genSampleRateChanged(0)
genSampleRateChanged(1)
genSampleRateChanged(2)
genSampleRateChanged(3)
genSampleRateChanged(4)
genSampleRateChanged(5)
genSampleRateChanged(6)
genSampleRateChanged(7)
genSampleRateChanged(8)
genSampleRateChanged(9)
genSampleRateChanged(10)
genSampleRateChanged(11)
genSampleRateChanged(12)
genSampleRateChanged(13)
genSampleRateChanged(14)
genSampleRateChanged(15)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

genAsioMessages(0)
genAsioMessages(1)
genAsioMessages(2)
genAsioMessages(3)
genAsioMessages(4)
genAsioMessages(5)
genAsioMessages(6)
genAsioMessages(7)
genAsioMessages(8)
genAsioMessages(9)
genAsioMessages(10)
genAsioMessages(11)
genAsioMessages(12)
genAsioMessages(13)
genAsioMessages(14)
genAsioMessages(15)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define genCallback(x) \
	case x:\
	info.callbacks.bufferSwitch=bufferSwitch##x;\
	info.callbacks.bufferSwitchTimeInfo=bufferSwitchTimeInfo##x;\
	info.callbacks.sampleRateDidChange=sampleRateChanged##x;\
	info.callbacks.asioMessage=asioMessages##x;\
	break;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioInstance::setCallback()
{
	switch(device)
	{
		genCallback(0)
		genCallback(1)
		genCallback(2)
		genCallback(3)
		genCallback(4)
		genCallback(5)
		genCallback(6)
		genCallback(7)
		genCallback(8)
		genCallback(9)
		genCallback(10)
		genCallback(11)
		genCallback(12)
		genCallback(13)
		genCallback(14)
		genCallback(15)
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AasioClient::AasioClient()
{
	asioDriver=null;
	asioLeftChannel=-1;
	asioRightChannel=-1;
	asioMemoBuf[0]=asioMemoBuf[1]=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AasioClient::~AasioClient()
{
	asioRelease();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioClient::asioGetNumDev()
{
	return asio.list.asioGetNumDev();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioClient::asioGetDriverName(int index, char *name, int size)
{
	asio.list.asioGetDriverName(index, name, size);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AasioClient::asioCreate(int device)
{
	asio.inst[device].create(device);
	if(asio.inst[device].driver)
	{
		asioDriver=&asio.inst[device];
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioClient::asioRelease()
{
	if(asioDriver)
		asioDriver->release(this);
	asioDriver=null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AasioClient::asioUseChannel(int left, int right)
{
	bool b=false;
	if(asioDriver)
	{
		b=asioDriver->useChannel(this, left, right);
		if(b)
		{
			asioLeftChannel=left;
			asioRightChannel=right;
		}
	}
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioClient::asioReleaseChannel()
{
	asioLeftChannel=-1;
	asioRightChannel=-1;
	if(asioDriver)
		return asioDriver->releaseChannel(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioClient::asioGetNbInput()
{
	if(asioDriver)
		return asioDriver->info.nbInput;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioClient::asioGetNbOutput()
{
	if(asioDriver)
		return asioDriver->info.nbOutput;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioClient::asioGetFirstInput()
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioClient::asioGetFirstOutput()
{
	if(asioDriver)
		return asioDriver->info.nbInput;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char * AasioClient::asioGetChannelName(int channel)
{
	if(asioDriver)
		return asioDriver->info.channelInfo[channel].name;
	return null;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioClient::asioAddBuffer(sword *buffer, int nsample)
{
	assert(asioDriver&&(asioLeftChannel!=-1)&&(asioRightChannel!=-1));
	if(asioDriver&&(asioLeftChannel!=-1)&&(asioRightChannel!=-1))
		asioDriver->addBuffer(asioLeftChannel, asioRightChannel, buffer, nsample, asioMemoBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int AasioClient::asioGetBufferSize()
{
	return asioDriver->getBufferSize(asioLeftChannel);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AasioClient::asioControlPanel()
{
	if(asioDriver)
		asioDriver->controlPanel();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
