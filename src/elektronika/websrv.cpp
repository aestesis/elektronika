/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	WEBSRV.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include						<windows.h>
//#include						<process.h>
//#include						<iostream.h>
//#include						<fstream.h>
#include						<stdio.h>
//#include						<math.h>
#include						<winsock2.h>

#include						"websrv.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AwebsrvInfo::CI		= ACI("AwebsrvInfo",	GUID(0x11111112,0x00000060), &AeffectInfo::CI, 0, NULL);
ACI								Awebsrv::CI			= ACI("Awebsrv",		GUID(0x11111112,0x00000061), &Aeffect::CI, 0, NULL);
ACI								AwebsrvFront::CI	= ACI("AwebsrvFront",	GUID(0x11111112,0x00000062), &AeffectFront::CI, 0, NULL);
ACI								AwebsrvBack::CI		= ACI("AwebsrvBack",	GUID(0x11111112,0x00000063), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
static bool						initOK=false;
static char						wwwroot[ALIB_PATH];
static char						hostname[1024]="";
static word						port=80;
static int						jpegquality=80;

static bool						useftp=false;
static char						ftpsrv[1024]="";
static char						ftplog[1024]="";
static char						ftppwd[1024]="";
static char						ftpfile[1024]="";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							COMM_BUFFER_SIZE	1024
#define							SMALL_BUFFER_SIZE	10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct HTTPRequestHeader
{
	char						method[SMALL_BUFFER_SIZE];
	char						url[ALIB_PATH];
	char						filepathname[ALIB_PATH];
	char						httpversion[SMALL_BUFFER_SIZE];
	IN_ADDR						client_ip;
};

struct MimeAssociation
{
	char						*file_ext;
	char						*mime;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MimeAssociation mimetypes[] =
{
        { ".txt",	"text/plain"	},
        { ".html",	"text/html"		},
        { ".htm",	"text/html"		},
        { ".gif",	"image/gif"		},
        { ".jpg",	"image/jpeg"	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const int nbmimetype=sizeof(mimetypes)/sizeof(MimeAssociation);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SOCKET StartWebServer()
{
	SOCKET s;
	s=socket(AF_INET, SOCK_STREAM, 0);
	if(s!=INVALID_SOCKET)
	{
		SOCKADDR_IN si;
		si.sin_family=AF_INET;
		si.sin_port=htons(port);
		si.sin_addr.s_addr=htonl(INADDR_ANY);
		if(bind(s,(struct sockaddr *) &si,sizeof(SOCKADDR_IN))!=SOCKET_ERROR)
			return s;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int SocketRead(SOCKET client_socket, char *receivebuffer, int buffersize)
{
	int size=0;
	int	totalsize=0;

	do
	{
		size=recv(client_socket, receivebuffer+totalsize, buffersize-totalsize, 0);
		if((size!=0)&&(size!=SOCKET_ERROR))
		{
			totalsize+=size;

			// are we done reading the http header?
			if(strstr(receivebuffer,"\r\n\r\n"))
				break;
		}
		else
			return size;	// error state
	} 
	while((size!=0)&&(size!=SOCKET_ERROR));

	return totalsize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool ParseHTTPHeader(char *receivebuffer, HTTPRequestHeader &requestheader)
{
	// http request header format
	// method uri httpversion
	char *pos=strtok(receivebuffer," ");;

	if(pos==NULL )
		return FALSE;
	strncpy(requestheader.method, pos, SMALL_BUFFER_SIZE);

	pos=strtok(NULL, " ");
	if(pos==NULL)
		return FALSE;
	strncpy(requestheader.url, pos, ALIB_PATH);

	pos=strtok(NULL, "\r");
	if(pos==NULL )
		return FALSE;
	strncpy(requestheader.httpversion, pos, SMALL_BUFFER_SIZE);

	// based on the url lets figure out the filename + path
	strncpy(requestheader.filepathname, wwwroot, ALIB_PATH);
	strncat(requestheader.filepathname, requestheader.url, ALIB_PATH);

	// because the filepathname can have relative references  ../ ./=20
	// call _fullpath to get the absolute 'real' filepath
	// _fullpath seems to handle '/' and '\'=20

	_fullpath(requestheader.filepathname, requestheader.filepathname, ALIB_PATH);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int findMimeType(const char *filename)
{
	const char	*pos=strrchr(filename,'.');
	if(pos)
	{
		int	x;
		for(x=0; x<nbmimetype; x++)
		{
			if(stricmp(mimetypes[x].file_ext,pos)==0)
				return x;
        }
	}
	return 0;  // return default mimetype  'text/plain'=20
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void OutputHTTPError(SOCKET client_socket, int statuscode)
{
	char headerbuffer[COMM_BUFFER_SIZE];
	char htmlbuffer[COMM_BUFFER_SIZE];

	sprintf(headerbuffer,"HTTP/1.0 %d\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n",statuscode,strlen(htmlbuffer));
	sprintf(htmlbuffer,"<html><body><h3>elektronika webServer<br><br><h2>Error: %d</h2></body></html>",statuscode);

	send(client_socket,headerbuffer,strlen(headerbuffer),0);
	send(client_socket,htmlbuffer,strlen(htmlbuffer),0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void OutputHTTPRedirect(SOCKET client_socket, const char *defaulturl)
{
	char headerbuffer[COMM_BUFFER_SIZE];
	char htmlbuffer[COMM_BUFFER_SIZE];
	char hosturl[ALIB_PATH];

	if(port==80)
		sprintf(hosturl,"http://%s",hostname);
	else
		sprintf(hosturl,"http://%s:%d",hostname, port);
	strncat(hosturl,defaulturl,COMM_BUFFER_SIZE);

	if(hosturl[strlen(hosturl)-1]!='/')
		strncat(hosturl,"/",ALIB_PATH);

	strncat(hosturl,"index.html",ALIB_PATH);

	sprintf(htmlbuffer,"<html><body><a href=3D\"%s\">%s</a></body></html>",hosturl,hosturl);
	sprintf(headerbuffer,"HTTP/1.0 301\r\nContent-Type: text/html\r\nContent-Length: %ld\r\nLocation: %s\r\n\r\n",strlen(htmlbuffer),hosturl);

	send(client_socket,headerbuffer,strlen(headerbuffer),0);
	send(client_socket,htmlbuffer,strlen(htmlbuffer),0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init(char *rootdir)
{
	if(!count)
	{
		WSADATA wd;
		if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
		{
			strcpy(wwwroot, rootdir);
			strcat(wwwroot, "\\wwwroot");
			{
				char	fn[ALIB_PATH];
				strcpy(fn, rootdir);
				strcat(fn, "\\www.ini");
				{
					FILE	*f=fopen(fn, "r");
					char	str[1024];
					hostname[0]=0;
					if(f)
					{
						while(fgets(str, sizeof(str)-1, f))
						{
							{
								char	*s=str;
								while(*s)
								{
									if((*s==10)||(*s==13))
									{
										*s=0;
										break;
									}
									s++;
								}
							}
							switch(str[0])
							{
								case '#':
								break;

								default:
								if(strncmp("hostname=", str, 9)==0)
								{
									strcpy(hostname, str+9);
								}
								else if(strncmp("useftp=true", str, 11)==0)
								{
									useftp=true;
								}
								else if(strncmp("ftpsrv=", str, 7)==0)
								{
									strcpy(ftpsrv, str+7);
								}
								else if(strncmp("ftplog=", str, 7)==0)
								{
									strcpy(ftplog, str+7);
								}
								else if(strncmp("ftppwd=", str, 7)==0)
								{
									strcpy(ftppwd, str+7);
								}
								else if(strncmp("ftpfile=", str, 8)==0)
								{
									strcpy(ftpfile, str+8);
								}
								break;
							}
						}
						fclose(f);
					}
					if(!hostname[0])
						gethostname(hostname, 1023);
				}
			}
			initOK=true;
		}
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		if(initOK)
		{
	        WSACleanup();
			initOK=false;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aclient : public Athread
{
public:
								Aclient							(Awebsrv *websrv, SOCKET socket, IN_ADDR ip);
	virtual						~Aclient						();

	virtual void				run								();

	Awebsrv						*websrv;
	SOCKET						socket;
	IN_ADDR						ip;

	bool						bRun;
	bool						bStop;

	Aclient						*next;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aclient::Aclient(Awebsrv *websrv, SOCKET socket, IN_ADDR ip)
{
	this->websrv=websrv;
	this->socket=socket;
	this->ip=ip;
	next=NULL;
	bStop=false;
	bRun=true;
	start();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aclient::~Aclient()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aclient::run()
{
	{
        HTTPRequestHeader	requestheader;
        int					size;
        char				receivebuffer[COMM_BUFFER_SIZE];
        char				sendbuffer[COMM_BUFFER_SIZE];

        requestheader.client_ip=ip;

        size=SocketRead(socket, receivebuffer, COMM_BUFFER_SIZE);
        if((size!=SOCKET_ERROR )&&(size!=0))
        {
			receivebuffer[size]=NULL;
			if(ParseHTTPHeader(receivebuffer, requestheader))
			{
				if(strstr(requestheader.method, "GET"))
				{
					if(strnicmp(requestheader.filepathname, wwwroot, strlen(wwwroot))==0)  // else security violation!
					{
						char	*pt=requestheader.filepathname+strlen(wwwroot);

						if(strncmp(pt, "\\serverpush320", 14)==0)
						{
							byte		buffer[128000];
							int			size,r;

							strcpy(sendbuffer, "HTTP/1.0 200 OK\r\n");
							strncat(sendbuffer, "Content-Type: multipart/x-mixed-replace\r\n", COMM_BUFFER_SIZE);
							strncat(sendbuffer, "boundary=magicalboundarystring\r\n", COMM_BUFFER_SIZE);
							strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);
							strncat(sendbuffer, "--magicalboundarystring\r\n", COMM_BUFFER_SIZE);

							size=strlen(sendbuffer);
							r=send(socket, sendbuffer, size, 0);

							if(r==size)
							{
								while(!bStop)
								{
									Afilemem	*fm=new Afilemem("jpg file", buffer, sizeof(buffer));

									websrv->section.enter(__FILE__,__LINE__);
									websrv->snap320->save(fm, bitmapJPG, jpegquality);
									websrv->section.leave();

									strcpy(sendbuffer, "Content-type: image/jpeg\r\n");
									strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);
									size=strlen(sendbuffer);
									r=send(socket, sendbuffer, size, 0);
									if(r==size)
									{
										int		p=0;
										int		filesize=(int)fm->getSize();
										while((!bStop)&&filesize)
										{
											int n=mini(filesize, 2048);
											int	r=send(socket, (char *)&buffer[p], n, 0);
											if(r!=n)
											{
												bStop=true;
												break;
											}
											p+=n;
											filesize-=n;
										}
									}
									else
										bStop=true;

									strcpy(sendbuffer, "\r\n--magicalboundarystring\r\n");
									size=strlen(sendbuffer);
									r=send(socket, sendbuffer, size, 0);

									delete(fm);

									sleep(100);
								}
							}
						}
						else if(strncmp(pt, "\\serverpush160", 14)==0)
						{
							byte		buffer[128000];
							int			size,r;

							strcpy(sendbuffer, "HTTP/1.0 200 OK\r\n");
							strncat(sendbuffer, "Content-Type: multipart/x-mixed-replace\r\n", COMM_BUFFER_SIZE);
							strncat(sendbuffer, "boundary=magicalboundarystring\r\n", COMM_BUFFER_SIZE);
							strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);
							strncat(sendbuffer, "--magicalboundarystring\r\n", COMM_BUFFER_SIZE);

							size=strlen(sendbuffer);
							r=send(socket, sendbuffer, size, 0);

							if(r==size)
							{
								while(!bStop)
								{
									Afilemem	*fm=new Afilemem("jpg file", buffer, sizeof(buffer));

									websrv->section.enter(__FILE__,__LINE__);
									websrv->snap160->save(fm, bitmapJPG, jpegquality);
									websrv->section.leave();

									strcpy(sendbuffer, "Content-type: image/jpeg\r\n");
									strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);
									size=strlen(sendbuffer);
									r=send(socket, sendbuffer, size, 0);
									if(r==size)
									{
										int		p=0;
										int		filesize=(int)fm->getSize();
										while((!bStop)&&filesize)
										{
											int n=mini(filesize, 2048);
											int	r=send(socket, (char *)&buffer[p], n, 0);
											if(r!=n)
											{
												bStop=true;
												break;
											}
											p+=n;
											filesize-=n;
										}
									}
									else
										bStop=true;

									strcpy(sendbuffer, "\r\n--magicalboundarystring\r\n");
									size=strlen(sendbuffer);
									r=send(socket, sendbuffer, size, 0);

									delete(fm);

									sleep(100);
								}
							}
						}
						else if(strncmp(pt, "\\thumbcamjpg320", 15)==0)
						{
							byte		buffer[128000];
							Afilemem	*fm=new Afilemem("jpg file", buffer, sizeof(buffer));

							websrv->section.enter(__FILE__,__LINE__);
							websrv->snap320->save(fm, bitmapJPG, jpegquality);
							websrv->section.leave();
							
							strcpy(sendbuffer, "HTTP/1.0 200 OK\r\n");
							strncat(sendbuffer, "Content-Type: ", COMM_BUFFER_SIZE);
							strncat(sendbuffer, "image/jpeg", COMM_BUFFER_SIZE);
							sprintf(sendbuffer+strlen(sendbuffer), "\r\nContent-Length: %ld\r\n", fm->getSize());
							strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);

							send(socket, sendbuffer, strlen(sendbuffer), 0);

							{
								int		p=0;
								int		filesize=(int)fm->getSize();
								while((!bStop)&&filesize)
								{
									int n=mini(filesize, 2048);
									send(socket, (char *)&buffer[p], n, 0);
									p+=n;
									filesize-=n;
								}
							}

							delete(fm);
						}
						else if(strncmp(pt, "\\thumbcamjpg160", 15)==0)
						{
							byte		buffer[64000];
							Afilemem	*fm=new Afilemem("jpg file", buffer, sizeof(buffer));

							websrv->section.enter(__FILE__,__LINE__);
							websrv->snap160->save(fm, bitmapJPG, jpegquality);
							websrv->section.leave();
							
							strcpy(sendbuffer, "HTTP/1.0 200 OK\r\n");
							strncat(sendbuffer, "Content-Type: ", COMM_BUFFER_SIZE);
							strncat(sendbuffer, "image/jpeg", COMM_BUFFER_SIZE);
							sprintf(sendbuffer+strlen(sendbuffer), "\r\nContent-Length: %ld\r\n", fm->getSize());
							strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);

							send(socket, sendbuffer, strlen(sendbuffer), 0);

							{
								int		p=0;
								int		filesize=(int)fm->getSize();
								while((!bStop)&&filesize)
								{
									int n=mini(filesize, 2048);
									send(socket, (char *)&buffer[p], n, 0);
									p+=n;
									filesize-=n;
								}
							}

							delete(fm);
						}
						else
						{
							long	filesize;
							DWORD	fileattrib=GetFileAttributes(requestheader.filepathname);

							if(!((fileattrib!=-1)&&(fileattrib&FILE_ATTRIBUTE_DIRECTORY)))
							{
								FILE	*in=fopen(requestheader.filepathname,"rb");  // read binary
								if(!in)
								{
									OutputHTTPError(socket, 404);   // 404 - not found
								}
								else
								{
									// determine file size
									fseek(in, 0, SEEK_END);
									filesize=ftell(in);
									fseek(in, 0, SEEK_SET);


									// send the http header and the file contents to the browser
									strcpy(sendbuffer, "HTTP/1.0 200 OK\r\n");
									strncat(sendbuffer, "Content-Type: ", COMM_BUFFER_SIZE);
									strncat(sendbuffer, mimetypes[findMimeType(requestheader.filepathname)].mime, COMM_BUFFER_SIZE);
									sprintf(sendbuffer+strlen(sendbuffer), "\r\nContent-Length: %ld\r\n", filesize);
									strncat(sendbuffer,"\r\n", COMM_BUFFER_SIZE);

									send(socket, sendbuffer, strlen(sendbuffer), 0);

									{
										byte	filebuffer[2048];
										int		n=1;
										while((!bStop)&&filesize&&n)
										{
											n=fread((char *)filebuffer, 1, mini(2048, filesize), in);
											send(socket, (char *)filebuffer, n, 0);
											filesize-=n;
										}
									}
									fclose(in);

									// log line
								}
							}
							else
							{
								OutputHTTPRedirect(socket, requestheader.url);
							}
						}
					}
					else
					{
						OutputHTTPError(socket, 403);    // 403 - forbidden
					}
				}
				else
				{
					OutputHTTPError(socket, 501);   // 501 not implemented
				}
			}
			else
			{
				OutputHTTPError(socket, 400);   // 400 - bad request
			}
		}
        closesocket(socket);
	}
	bRun=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awebsrv::Awebsrv(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init(getRootPath());
	webrun=false;
	clients=NULL;
	front=new AwebsrvFront(qiid, "webserver front", this, 50);
	front->setTooltips("webserver");
	back=new AwebsrvBack(qiid, "websrv back", this, 50);
	back->setTooltips("webserver");
	snap320=new Abitmap(320, 240);
	snap160=new Abitmap(160, 120);
	if(!initOK)
		front->notify(this, nyERROR, (dword)"can't initialize winsock 2.2");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awebsrv::~Awebsrv()
{
	stopServer();
	end();
	delete(snap320);
	delete(snap160);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebsrv::stopServer()
{
	webstop=true;
	closesocket(socket);
	while(webrun)
		sleep(10);
	section.enter(__FILE__,__LINE__);
	stop();
	{
		Aclient	*c=clients;
		while(c)
		{
			Aclient	*cn=c->next;
			c->bStop=true;
			while(c->bRun)
				sleep(10);
			delete(c);
			c=cn;
		}
		clients=NULL;
	}
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awebsrv::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awebsrv::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebsrv::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebsrv::action(double time, double dtime, double beat, double dbeat)
{
	Avideo			*in=((AwebsrvBack *)back)->in;
	section.enter(__FILE__,__LINE__);
	in->enter(__FILE__,__LINE__);
	{
		Abitmap	*b=in->getBitmap();
		if(b)
		{
			snap320->set(0, 0, snap320->w, snap320->h, 0, 0, b->w, b->h, b, bitmapDEFAULT, bitmapDEFAULT);
			snap160->set(0, 0, snap160->w, snap160->h, 0, 0, b->w, b->h, b, bitmapDEFAULT, bitmapDEFAULT);
			nsnap++;
		}
	}
	in->leave();
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebsrv::run()
{
	webstop=false;
	if(useftp)
	{
		Abuffer	*f=new Abuffer("image", snap320->w*snap320->h*4+1024);	// size max
		ftp=new AftpClient(ftpsrv, ftplog, ftppwd);
		webrun=true;
		while(!webstop)
		{
			if(nsnap)
			{
				if(ftp->getStatus()==AftpClient::statusCONNECTED)  // et pas AftpClient::statusSTOR
				{
					section.enter(__FILE__,__LINE__);
					{
						f->clear();
						snap320->save(f, bitmapJPG, jpegquality);
						ftp->sendFile(ftpfile, f);
					}
					nsnap=0;
					section.leave();
				}
			}
			sleep(10);
		}
		delete(ftp);
	}
	else
	{
		socket=StartWebServer();
		if(socket)
		{
			int		ciplen=sizeof(SOCKADDR_IN);
			if(listen(socket,SOMAXCONN)!=SOCKET_ERROR)
			{
				webrun=true;
				while(!webstop)
				{
					SOCKADDR_IN	cip;
					SOCKET		csock=accept(socket,(struct sockaddr *)&cip, &ciplen);
					if(csock!=INVALID_SOCKET)
					{
						IN_ADDR	ia;
						memcpy(&ia, &cip.sin_addr.s_addr, 4);
						Aclient	*c=new Aclient(this, csock, ia);
						section.enter(__FILE__,__LINE__);
						c->next=clients;
						clients=c;
						section.leave();
					}
					section.enter(__FILE__,__LINE__);
					{
						Aclient	*c=clients;
						Aclient	*oc=NULL;
						while(c)
						{
							Aclient	*nc=c->next;
							if(!c->bRun)
							{
								if(oc==NULL)
									clients=c->next;
								else
									oc->next=c->next;
								delete(c);
							}
							else
								oc=c;
							c=nc;
						}
					}
					section.leave();
					sleep(1);
				}
			}
			else
				front->notify(this, nyERROR, (dword)"can't start web server (can't listen)");
			section.enter(__FILE__,__LINE__);
			{
				Aclient	*c=clients;
				while(c)
				{
					Aclient	*nc=c->next;
					c->bStop=true;
					while(c->bRun)
						sleep(10);
					delete(c);
					c=nc;
				}
				clients=NULL;
			}
			section.leave();
			closesocket(socket);
		}
		else
			front->notify(this, nyERROR, (dword)"can't start web server");
	}
	webrun=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AwebsrvFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebsrvFront::AwebsrvFront(QIID qiid, char *name, Awebsrv *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_WEBSRV), "PNG");
	back=new Abitmap(&o);
	ehostname=new Aedit("hostname", this, 16, (pos.h-14)>>1, 218, 14);
	ehostname->setTooltips("host name");
	ehostname->set(hostname);
	ehostname->show(true);
	eport=new Aedit("port", this, 240, (pos.h-14)>>1, 40, 14);
	eport->setTooltips("port");
	eport->set("80");
	eport->show(true);
	active=new Abutton("active", this, 300, (pos.h-14)>>1, 100, 14, "ACTIVE", Abutton::btCAPTION|Abutton::bt2STATES);
	active->setTooltips("active web server");
	active->show(TRUE);
	jpegq=new Asegment("jpegquality", this, 420, (pos.h-14)>>1, 3, 1, 100, alib.getFont(fontSEGMENT10), 0xff000000, 0.5f);
	jpegq->setTooltips("jpeg quality");
	jpegq->set(80);
	jpegq->show(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebsrvFront::~AwebsrvFront()
{
	delete(back);
	delete(ehostname);
	delete(eport);
	delete(active);
	delete(jpegq);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AwebsrvFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==ehostname)
		{
			((Awebsrv *)effect)->stopServer();
			active->setChecked(false);
			ehostname->get(hostname, sizeof(hostname)-1);
		}
		else if(o==eport)
		{
			((Awebsrv *)effect)->stopServer();
			active->setChecked(false);
			char	vp[10];
			eport->get(vp, 9);
			vp[9]=0;
			port=(word)atoi(vp);
		}
		else if(o==active)
		{
			if(initOK)
			{
				if(active->isChecked())
					((Awebsrv *)effect)->start();
				else
					((Awebsrv *)effect)->stopServer();
			}
			else
			{
				active->setChecked(false);
			}
		}
		else if(o==jpegq)
		{
			jpegquality=jpegq->get();
		}
		break;

		case nyCLICK:
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AwebsrvFront::pulse()
{
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebsrvBack::AwebsrvBack(QIID qiid, char *name, Awebsrv *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_WEBSRV2), "PNG");
	back=new Abitmap(&o);

	in=new Avideo(MKQIID(qiid, 0x75108d5c3cc47f00), "video input", this, pinIN|pinNOGFX, 16, 12);
	in->setTooltips("video input");
	in->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebsrvBack::~AwebsrvBack()
{
	delete(in);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AwebsrvBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AwebsrvInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Awebsrv(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * websrvGetInfo()
{
	return new AwebsrvInfo("websrvInfo", &Awebsrv::CI, "web server", "web server module - export video/picture for the web");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
