/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PIPE.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"pipe.h"
#include						"thread.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Apipe::CI=ACI("Apipe", GUID(0xAE57E515,0x00000403), &Afile::CI, 0, NULL); // countof(properties), properties);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Apipe::Apipe(char *name, int mode) : Afile(name)
{
	this->mode=mode;
	timeout=300;
	if(mode&SERVER)
	{
		DWORD	omode=0;
		DWORD	otype=0;
		if((mode&READ)&&(mode&WRITE))
		{
			omode=PIPE_ACCESS_DUPLEX; // |FILE_FLAG_WRITE_THROUGH
			otype=PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_NOWAIT;
		}
		else if(mode&READ)
		{
			omode=PIPE_ACCESS_INBOUND;
			otype=PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_NOWAIT;
		}
		else if(mode&WRITE) 
		{
			omode=PIPE_ACCESS_OUTBOUND; // |FILE_FLAG_WRITE_THROUGH
			otype=PIPE_TYPE_BYTE|PIPE_NOWAIT;
		}
		hpipe=CreateNamedPipe(name, omode, otype, PIPE_UNLIMITED_INSTANCES, 0, 0, 0, 0);
	}
	else
	{	
		DWORD	omode=0;
		if(mode&READ)
			omode|=GENERIC_READ;
		if(mode&WRITE) 
			omode|=GENERIC_WRITE;
		hpipe=CreateFile(name, omode, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}
}

ADLL Apipe::~Apipe()
{
	if(isOK())
	{
		if(mode&SERVER)
		{
			DisconnectNamedPipe(hpipe);
			CloseHandle(hpipe);
		}
		else
			CloseHandle(hpipe);
	}
}

ADLL bool Apipe::isConnected()
{
	if(isOK())
	{
		if(ConnectNamedPipe(hpipe, NULL))
			return true;
		else if(GetLastError()==ERROR_PIPE_CONNECTED)
			return true;
	}
	return false;
}

ADLL bool Apipe::waitConnection()
{
	if(isOK())
	{
		int t=Athread::getTickCount();
		while(Athread::getTickCount()-t<timeout)
		{
			if(ConnectNamedPipe(hpipe, NULL))
				return true;
			else if(GetLastError()==ERROR_PIPE_CONNECTED)
				return true;
			Athread::sleep(1);
		}
	}
	return false;
}

ADLL bool Apipe::flush()
{
	if(isOK())
		return FlushFileBuffers(hpipe)?true:false;
	return false;
}

ADLL int Apipe::avaible()
{
	DWORD	nb=0;
	if(isOK()&&(mode&READ))
		PeekNamedPipe(hpipe, NULL, 0, NULL, &nb, NULL);
	return (int)nb;
}

ADLL int Apipe::read(void *p, int s)
{
	DWORD	nb=0;
	if(isOK()&&(mode&READ))
	{
		int	time=Athread::getTickCount();
		while(s)
		{
			DWORD	n=0;
			ReadFile(hpipe, p, s, &n, NULL);
			s-=n;
			p=(void *)((char *)p+n);
			nb+=n;
			offset+=n;
			if(s)
			{
				Athread::sleep(1);
				if((int)Athread::getTickCount()-time>timeout)
					break;
			}
		}
	}
	return nb;
}

ADLL int Apipe::write(void *p, int s)
{
	DWORD	nb=0;
	int	time=Athread::getTickCount();
	if(isOK()&&(mode&WRITE))
	{
		while(s)
		{
			DWORD	n=0;
			WriteFile(hpipe, p, s, &n, NULL);
			s-=n;
			p=(void *)((char *)p+n);
			nb+=n;
			offset+=n;
			if(s)
			{
				Athread::sleep(1);
				if((int)Athread::getTickCount()-time>timeout)
					break;
			}
		}
	}
	return nb;
}

ADLL bool Apipe::writeString(char *s)
{
	int	len=strlen(s);
	if(!write(&len, sizeof(len)))
		return false;
	if(!write(s, len))
		return false;
	return true;
}

ADLL bool Apipe::readString(char *s)
{
	int	len;
	if(read(&len, sizeof(len))!=sizeof(len))
		return false;
	if(read(s, len)!=len)
		return false;
	s[len]=0;
	return true;
}

ADLL bool Apipe::readStringAlloc(char **s)
{
	int	len;
	if(read(&len, sizeof(len))!=sizeof(len))
		return false;
	*s=(char *)malloc(len+1);
	if(read(*s, len)!=len)
		return false;
	(*s)[len]=0;
	return true;
}

ADLL bool Apipe::isOK()
{
	return (hpipe!=INVALID_HANDLE_VALUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
