/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	WEBCLIENT.CPP				(c)	YoY'01						WEB: www.aestesis.org
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
#include						<assert.h>

#include						"webclient.h"
#include						"resource.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AwebclientInfo::CI		= ACI("AwebclientInfo",		GUID(0x11111112,0x00000110), &AeffectInfo::CI, 0, NULL);
ACI								Awebclient::CI			= ACI("Awebclient",			GUID(0x11111112,0x00000111), &Aeffect::CI, 0, NULL);
ACI								AwebclientFront::CI		= ACI("AwebclientFront",	GUID(0x11111112,0x00000112), &AeffectFront::CI, 0, NULL);
ACI								AwebclientBack::CI		= ACI("AwebclientBack",		GUID(0x11111112,0x00000113), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MAXPAGEBUFFER				65536*2

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
								stateSTEALIMAGE,
								stateSRVPUSH
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awebclient::Awebclient(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	webrun=false;
	webstop=true;
	strcpy(hostname, "localhost");
	port=80;
	state=stateSTEALIMAGE;
	front=new AwebclientFront(qiid, "webclient front", this, 50);
	front->setTooltips("web client");
	back=new AwebclientBack(qiid, "webclient back", this, 50);
	back->setTooltips("web client");
	snap320=new Abitmap(320, 240);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Awebclient::~Awebclient()
{
	stopClient();
	delete(snap320);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awebclient::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awebclient::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebclient::settings(bool emergency)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebclient::action(double time, double dtime, double beat, double dbeat)
{
	Avideo			*out=((AwebclientBack *)back)->out;
	section.enter(__FILE__,__LINE__);
	out->enter(__FILE__,__LINE__);
	{
		Abitmap	*b=out->getBitmap();
		if(b)
			b->set(0, 0, b->w, b->h, 0, 0, snap320->w, snap320->h, snap320, bitmapNORMAL, bitmapNORMAL);
	}
	out->leave();
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebclient::run()
{
	WSADATA wd;
	Abitmap	*b=new Abitmap(320, 240);

	webrun=true;
	webstop=false;

	if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
	{
		unsigned long		addr=inet_addr(hostname);

		if(addr==INADDR_NONE)
		{		
			HOSTENT		*host=gethostbyname(hostname);
			if(host)
				if(host->h_addrtype==AF_INET)
					memcpy(&addr, host->h_addr, mini(host->h_length, sizeof(addr)));
		}

		if(addr!=INADDR_NONE)
		{
			IN_ADDR	*inaddr=(IN_ADDR *)&addr;

			while(webrun)
			{
				SOCKET	s=socket(AF_INET, SOCK_STREAM, 0);

				if(s!=SOCKET_ERROR)
				{
					SOCKADDR_IN		sain;

					sain.sin_family	= AF_INET;
					sain.sin_addr	= *inaddr;
					sain.sin_port	= htons(port);

					if(connect(s, (SOCKADDR *)&sain, sizeof(sain))==0)
					{
						char	buffer[MAXPAGEBUFFER];
						int		length;

						switch(state)
						{
							case stateSRVPUSH:
							sprintf(buffer, "GET /serverpush320 HTTP/1.0\r\nHost: %s:%d\r\n", hostname, port);
							strcat (buffer, "User-Agent: elektronika web client\r\n");
							strcat (buffer, "\r\n");
							strcat (buffer, "\r\n");

							length = strlen(buffer);
							if(send(s, buffer, length, 0)==length)
							{
								int				st=0;
								int				r=-1;
								int				t=0;
								unsigned long	vcmd=1;
								int				timeout=200;
								int				read=0;
								int				scan=0;
								char			magical[256]="--magicalboundarystring";

								ioctlsocket(s, FIONBIO, &vcmd);

								while(webrun&&(timeout--)&&(st!=-1))
								{
									r=recv(s, buffer+t, sizeof(buffer)-t, 0);
									if(r==0)
										break;
									if(r!=SOCKET_ERROR)
									{
										int	toread=(t+=r)-read;
										timeout=200;

										switch(st)
										{
											case 0: // header
											if(toread>1000)
											{
												int		ok=0;
												char	*str=&buffer[read];
												int		len=0;
												char	*lstr=str;
												int		bt=0;
												while(str&&(toread>0))
												{
													if((str[0]=='\r')&&(str[1]=='\n'))
													{
														if(len==0)
														{
															if(bt==1)
																bt=2;
															else
															{
																st=-1;
																break;
															}
														}
														else
														{
															if(bt==2)
															{
																if(!strnicmp(magical, lstr, len))
																	st=1;
																str+=2;
																toread-=2;
																read+=2;
																scan=read;
																break;	
															}
															else if(!strnicmp(lstr, "Content-Type: multipart/x-mixed-replace", len))
																bt=1;
															else if(!strnicmp(lstr, "boundary=", 9))
															{
																int	l=mini(len-9, 128);
																strncpy(magical+2, lstr+9, l);
																magical[l+2]=0;
															}
														}
														len=0;
														lstr=str+2;
														str++;
														toread--;
														read++;
													}
													else
														len++;
													str++;
													toread--;
													read++;
												}
											}
											break;

											case 1:	// search content
											if(read>16384)
											{
												memcpy(buffer, &buffer[read], sizeof(buffer)-read);
												t-=read;
												scan-=read;
												read=0;
											}
											else
											{
												int	toscan=t-scan-128;
												while(toscan>0)
												{
													if(!strnicmp(&buffer[scan], "Content-Type: ", 14))
													{
														if(!strnicmp(&buffer[scan], "Content-Type: image/jpeg", 24))
															st=2;
														else
															st=3;
														break;
													}
													scan++;
													toscan--;
												}
											}
											break;

											case 2:	// search content begin
											if(read>16384)
											{
												memcpy(buffer, &buffer[read], sizeof(buffer)-read);
												t-=read;
												scan-=read;
												read=0;
											}
											else
											{
												if(toread>1000)
												{
													char	*str=&buffer[read];
													int		len=0;
													while(str&&(toread>0))
													{
														if((str[0]=='\r')&&(str[1]=='\n'))
														{
															if(len==0)
															{
																st=4;
																read+=2;
																scan=read;
																break;
															}
															len=0;
															str++;
															toread--;
															read++;
														}
														else
															len++;
														str++;
														toread--;
														read++;
													}
												}
											}
											break;

											case 3:	// skip content
											if(read>16384)
											{
												memcpy(buffer, &buffer[read], sizeof(buffer)-read);
												t-=read;
												scan-=read;
												read=0;
											}
											else
											{
												int	len=strlen(magical);
												int	toscan=t-scan-len-2;
												while(toscan>0)
												{
													if(!strnicmp(&buffer[scan], magical, len))
													{
														read=scan+len+2;
														scan=read;
														st=1;
														break;
													}
													toscan--;
													scan++;
												}
											}
											break;

											case 4:	// get jpeg
											if(read>16384)
											{
												memcpy(buffer, &buffer[read], sizeof(buffer)-read);
												t-=read;
												scan-=read;
												read=0;
											}
											else
											{
												int	len=strlen(magical);
												int	toscan=t-scan-len-2;
												while(toscan>0)
												{
													if(!strnicmp(&buffer[scan], magical, len))
													{
														int	l=(scan-2)-read;
														assert(l>0);
														{
															Afilemem	*fm=new Afilemem("jpg file", &buffer[read], sizeof(buffer)-read);

															b->load(fm);

															assert((b->w==320)&&(b->h==240));
															section.enter(__FILE__,__LINE__);
															snap320->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
															section.leave();

															read=scan+len+2;
															scan=read;
															
															st=1;

															break;
														}

													}
													toscan--;
													scan++;
												}
											}
											break;
										}
									}
									sleep(5);
								}
								vcmd=0;
								ioctlsocket(s, FIONBIO, &vcmd);
							}
							break;

							case stateSTEALIMAGE:
							sprintf(buffer, "GET /thumbcamjpg320 HTTP/1.0\r\nHost: %s:%d\r\n", hostname, port);
							strcat (buffer, "User-Agent: elektronika web client\r\n");
							strcat (buffer, "\r\n");
							strcat (buffer, "\r\n");

							length = strlen(buffer);
							if(send(s, buffer, length, 0)==length)
							{
								int				r=-1;
								int				t=0;
								unsigned long	vcmd=1;
								int				timeout=200;

								ioctlsocket(s, FIONBIO, &vcmd);

								while(webrun&&(timeout--))
								{
									r=recv(s, buffer+t, sizeof(buffer)-t, 0);
									if(r!=SOCKET_ERROR)
										t+=r;
									if((r==0)||(t>=MAXPAGEBUFFER))
										break;

									sleep(5);
								}

								if(r==0)
								{
									char	*str=buffer;
									int		len=0;
									bool	jpeg=false;
									while(str&&((str-buffer)<t))
									{
										if((str[0]=='\r')&&(str[1]=='\n'))
										{
											if(!strncmp(str+2, "Content-Type: image/jpeg", 24))
												jpeg=true;
											if(len==0)
												break;
											len=0;
											str++;
										}
										else if(str[0]!='\n')
											len++;
										str++;
									}
									
									len=t-(str-buffer);

									if(jpeg&&(len>0))
									{
										Afilemem	*fm=new Afilemem("jpg file", str, sizeof(buffer)-(str-buffer));

										b->load(fm);

										assert((b->w==320)&&(b->h==240));
										section.enter(__FILE__,__LINE__);
										snap320->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
										section.leave();
										
										delete(fm);
									}
									else
									{
										if(!jpeg)
											front->notify(this, nyERROR, (dword)"can't connect to elektronika web server");
									}
								}
								vcmd=0;
								ioctlsocket(s, FIONBIO, &vcmd);
							}
							break;
						}
					}
					else
						front->notify(this, nyERROR, (dword)"can't connect to host");

					closesocket(s);
				}
				else
					front->notify(this, nyERROR, (dword)"can't create socket");

				sleep(10);
			}
		}
		else
			front->notify(this, nyERROR, (dword)"can't resolve host name");
        WSACleanup();
	}
	else
		front->notify(this, nyERROR, (dword)"can't start winsock");

	delete(b);

	webstop=true;
	webrun=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Awebclient::startClient()
{
	start();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Awebclient::stopClient()
{
	webrun=false;
	while(!webstop)
		sleep(10);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AwebclientFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebclientFront::AwebclientFront(QIID qiid, char *name, Awebclient *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_WEBCLT), "PNG");
	back=new Abitmap(&o);
	ehostname=new Aedit("hostname", this, 16, (pos.h-14)>>1, 218, 14);
	ehostname->setTooltips("host name");
	ehostname->set("localhost");
	ehostname->show(true);
	eport=new Aedit("port", this, 240, (pos.h-14)>>1, 40, 14);
	eport->setTooltips("port");
	eport->set("80");
	eport->show(true);
	active=new Abutton("active", this, 300, (pos.h-14)>>1, 100, 14, "ACTIVE", Abutton::btCAPTION|Abutton::bt2STATES);
	active->setTooltips("active web server");
	active->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebclientFront::~AwebclientFront()
{
	delete(back);
	delete(ehostname);
	delete(eport);
	delete(active);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AwebclientFront::notify(Anode *o, int event, dword p)
{
	switch(event)
	{
		case nyCHANGE:
		if(o==ehostname)
		{
			((Awebclient *)effect)->stopClient();
			active->setChecked(false);
			ehostname->get(((Awebclient *)effect)->hostname, sizeof(((Awebclient *)effect)->hostname)-1);
		}
		else if(o==eport)
		{
			((Awebclient *)effect)->stopClient();
			active->setChecked(false);
			char	vp[10];
			eport->get(vp, 9);
			vp[9]=0;
			((Awebclient *)effect)->port=(word)atoi(vp);
		}
		else if(o==active)
		{
			if(active->isChecked())
				((Awebclient *)effect)->startClient();
			else
				((Awebclient *)effect)->stopClient();
		}
		break;

		case nyCLICK:
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AwebclientFront::pulse()
{
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebclientBack::AwebclientBack(QIID qiid, char *name, Awebclient *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_WEBCLT2), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0xe57da98ab43c4c40), "video output", this, pinOUT|pinNOGFX, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AwebclientBack::~AwebclientBack()
{
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AwebclientBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AwebclientInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Awebclient(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * webclientGetInfo()
{
	return new AwebclientInfo("webclientInfo", &Awebclient::CI, "web client", "web client module - import video/picture from the web or local");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
