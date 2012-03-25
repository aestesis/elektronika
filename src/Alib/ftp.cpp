/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FTP.CPP						(c)	YoY'05						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<winsock2.h>
#include						<stdio.h>
#include						"ftp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						AftpClient::CI=ACI("AftpClient", GUID(0xAE57E515,0x00000430), &Anode::CI, 0, NULL); // countof(properties), properties);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AftpSocket
{
public:
	SOCKET						control;
	SOCKET						data;

	AftpSocket()
	{
		control=INVALID_SOCKET;
		data=INVALID_SOCKET;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int ftpRcv(SOCKET s, char *data=null)
{
	char	sReceiveBuffer[1024];
	int		n=500;
	int		r=0;
	
	memset(sReceiveBuffer, 0, sizeof(sReceiveBuffer));

	while(n)
	{
		r=recv(s, (LPSTR)sReceiveBuffer, sizeof(sReceiveBuffer), 0);
		if(r!=SOCKET_ERROR)
			sReceiveBuffer[r] = '\0';
		if(r||(r==SOCKET_ERROR))
			break;
		Athread::sleep(1);
		n--;
	}
	
	if(r&&(r!=SOCKET_ERROR))
	{
		int		iLength=r; 
		int		tc=strchr(sReceiveBuffer, 13)?13:10;
		char	*s=sReceiveBuffer;
		char	*o=sReceiveBuffer;

		while(s)
		{
			char	*n=strchr(s+1, tc);
			if(n)
			{
				o=s;
				s=n;
			}
			else
				break;
		}
		while(*o&&((*o==13)||(*o==10)))
			o++;

		if(data)
			strcat(data, o);
#if _DEBUG
		OutputDebugString(o);
#endif
		return atoi(o);
	}

#if _DEBUG
	else if(r==SOCKET_ERROR)
		OutputDebugString("socket error\r\n");
	else
		OutputDebugString("socket time out\r\n");
#endif	

	return 999;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SOCKET pasvSocket(char *str)
{
	char	*s=strchr(str, '(')+1;
	if(s)
	{
		char	ip[1024];
		strcpy(ip, s);
		char	*sf=ip;

		{
			int	i;
			for(i=0; i<4; i++)
			{
				sf=strchr(sf+1, ',');
				if(!sf)
					break;
			}
		}

		if(sf)
		{
			*(sf++)=0;
			int	port=atoi(sf)<<8;
			sf=strchr(sf, ',');
			if(sf)
			{
				port+=atoi(sf+1);
				{
					char	*s=strchr(ip, ',');
					while(s&&*s)
					{
						*s='.',
						s=strchr(s, ',');
					}
				}
				SOCKADDR_IN		sain;
				memset(&sain, 0, sizeof(sain));
				sain.sin_family	= AF_INET;
				sain.sin_addr.S_un.S_addr	= inet_addr(ip);
				sain.sin_port	= htons(port);
				SOCKET	so=socket(AF_INET, SOCK_STREAM, 0);
				if(::connect(so, (SOCKADDR *)&sain, sizeof(sain))==0)
					return so;
				sain.sin_addr.S_un.S_addr	= inet_addr("127.0.0.1");
				if(::connect(so, (SOCKADDR *)&sain, sizeof(sain))==0)
					return so;
				closesocket(so);
			}
		}
	}
	return INVALID_SOCKET;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AftpClient::AftpClient(char *srv, char *log, char *pwd) : Anode(srv)
{
	status=statusNOTCONNECTED;
	this->srv=strdup(srv);
	{
		char	*p=strchr(this->srv, ':');
		if(p)
		{
			*p=0;
			p++;
			port=atoi(p);
		}
		else
			port=21;
	}
	this->log=strdup(log);
	this->pwd=strdup(pwd);
	buffer=new Abuffer("buffer", 16384);
	sock=new AftpSocket();
	connect();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AftpClient::~AftpClient()
{
	while(status&statusSTOR)
		sleep(1);
	disconnect();
	if(srv)
		free(srv);
	if(log)
		free(log);
	if(pwd)
		free(pwd);
	delete(sock);
	delete(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AftpClient::sendFile(char *filename, Afile *f)
{
	bool	r=false;
	section.enter(__FILE__,__LINE__);
	if(status==statusCONNECTED)
	{
		int		size=(int)f->getSize();
		if(buffer->bufsize<size)
		{
			delete(buffer);
			buffer=new Abuffer("buffer", (size&1023)+4096);
		}
		buffer->clear();
		{
			byte	data[1024];
			while(size)
			{
				int	s=mini(size, sizeof(data));
				f->read(data, s);
				buffer->write(data, s);
				size-=s;
			}
		}
		strcpy(this->filename, filename);
		status|=statusSTOR;
		start();
	}
	section.leave();
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AftpClient::run()
{
	char	cmd[2048];
	char	ret[2048];
	send(sock->control, "TYPE I\r\n", 8, 0);
	if(ftpRcv(sock->control, ret)==200)	// type set to I
	{
		send(sock->control, "PASV\r\n", 6, 0);
		Athread::sleep(10);
		if(ftpRcv(sock->control, ret)==227)	// passive mode
		{
			sock->data=pasvSocket(ret);
			if(sock->data!=INVALID_SOCKET)
			{
				strcpy(cmd, "STOR ");
				strcat(cmd, filename);
				strcat(cmd, "\r\n");
				send(sock->control, cmd, strlen(cmd), 0);
				if(ftpRcv(sock->control, ret)==150)	// opening binary mode for send file
				{
					char	data[1024];
					int		size=(int)buffer->getSize();
					while(size)
					{
						int	s=mini(size, sizeof(data));
						buffer->read(data, s);
						send(sock->data, data, s, 0);
						size-=s;
					}
					closesocket(sock->data);
					sock->data=INVALID_SOCKET;
				}
			}
		}
	}
	section.enter(__FILE__,__LINE__);
	status&=~statusSTOR;
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AftpClient::connect()
{
	WSADATA wd;
	if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
	{
		unsigned long		addr=inet_addr(srv);

		if(addr==INADDR_NONE)
		{		
			HOSTENT		*host=gethostbyname(srv);
			if(host)
				if(host->h_addrtype==AF_INET)
					memcpy(&addr, host->h_addr, mini(host->h_length, sizeof(addr)));
		}

		if(addr!=INADDR_NONE)
		{
			IN_ADDR	*inaddr=(IN_ADDR *)&addr;
			sock->control=socket(AF_INET, SOCK_STREAM, 0);
			if(sock->control!=SOCKET_ERROR)
			{
				SOCKADDR_IN		sain;
				sain.sin_family	= AF_INET;
				sain.sin_addr	= *inaddr;
				sain.sin_port	= htons(port);

				//srvip=addr;

				if(::connect(sock->control, (SOCKADDR *)&sain, sizeof(sain))==0)
				{
					char	cmd[1024];
					if(ftpRcv(sock->control)==220)	// ready to user
					{
						strcpy(cmd, "USER ");
						strcat(cmd, log);
						strcat(cmd, "\n");
						send(sock->control, cmd, strlen(cmd), 0);
						if(ftpRcv(sock->control)==331)	// user OK need password
						{
							strcpy(cmd, "PASS ");
							strcat(cmd, pwd);
							strcat(cmd, "\r\n");
							send(sock->control, cmd, strlen(cmd), 0);
							if(ftpRcv(sock->control)==230)	// connected
							{
								status=statusCONNECTED;
							}
						}
					}
				}
			}
		}
	}
	if(!status)
	{
		if(sock->control!=INVALID_SOCKET)
		{
			closesocket(sock->control);
			sock->control=INVALID_SOCKET;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AftpClient::disconnect()
{
	if(status)
		status=statusNOTCONNECTED;
	if(sock->control!=INVALID_SOCKET)
	{
		closesocket(sock->control);
		sock->control=INVALID_SOCKET;
	}
	WSACleanup();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool AftpClient::isOK()
{
	return status?true:false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL int AftpClient::getStatus()
{
	int	s=0;
	section.enter(__FILE__,__LINE__);
	s=status;
	section.leave();
	return s;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
