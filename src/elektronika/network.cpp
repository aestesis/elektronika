/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	NETWORK.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						<assert.h>

#include						"network.h"
#include						"resource.h"
#include						"audiovideo.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AnetworkInfo::CI	= ACI("AnetworkInfo",	GUID(0x11111112,0x00000270), &AeffectInfo::CI, 0, NULL);
ACI								Anetwork::CI		= ACI("Anetwork",		GUID(0x11111112,0x00000271), &Aeffect::CI, 0, NULL);
ACI								AnetworkFront::CI	= ACI("AnetworkFront",	GUID(0x11111112,0x00000272), &AeffectFront::CI, 0, NULL);
ACI								AnetworkBack::CI	= ACI("AnetworkBack",	GUID(0x11111112,0x00000273), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
static bool						used[23];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(!count)
	{
		memset(used, 0, sizeof(used));
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Anetwork::Anetwork(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init();
	bufjpeg=NULL;
	pipe=NULL;
	mode=SERVER;
	bout=true;
	brun=false;
	berror=false;
	port=1;
	image=NULL;
	frame=0;
	bvideo=false;
	baudio=false;
	memset(srvname, 0, sizeof(srvname));
	audio=new Abuffer("audio", 44100*4);
	front=new AnetworkFront(qiid, "network front", this, 50);
	front->setTooltips("network module");
	back=new AnetworkBack(qiid, "network back", this, 50);
	back->setTooltips("network module");
	settings(false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Anetwork::~Anetwork()
{
	change(Anetwork::NONE, 0);
	delete(audio);
	if(bufjpeg)
		free(bufjpeg);
	if(image)
		delete(image);
	end();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Anetwork::save(class Afile *f)
{
	AnetworkFront	*front=(AnetworkFront *)this->front;
	bool			bclient=front->client->isChecked();
	int				port=front->port->get();
	bool			bst=front->power->isChecked();
	f->writeString(srvname);
	f->write(&bclient, sizeof(bclient));
	f->write(&port, sizeof(port));
	f->write(&bst, sizeof(bst));
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Anetwork::load(class Afile *f)
{
	AnetworkFront	*front=(AnetworkFront *)this->front;
	bool			bclient;
	int				port;
	bool			bst=false;
	f->readString(srvname);
	f->read(&bclient, sizeof(bclient));
	f->read(&port, sizeof(port));
	f->read(&bst, sizeof(bst));
	front->client->setChecked(bclient);
	front->port->set(port);
	front->power->setChecked(bst);
	if(bst)
		change(bclient?CLIENT:SERVER, port);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Anetwork::settings(bool emergency)
{
	simage.enter(__FILE__,__LINE__);
	if(image)
		delete(image);
	image=new Abitmap(getVideoWidth(), getVideoHeight());
	if(bufjpeg)
		free(bufjpeg);
	bufjpeg=malloc(getVideoWidth()*getVideoHeight()*3);
	simage.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Anetwork::actionStop()
{
	baudio=bvideo=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Anetwork::action(double time, double dtime, double beat, double dbeat)
{
	AnetworkFront	*front=(AnetworkFront *)this->front;
	AnetworkBack	*back=(AnetworkBack *)this->back;

	switch(mode)
	{
		case CLIENT:
		simage.enter(__FILE__,__LINE__);
		{
			Abitmap	*b=back->vout->getBitmap();
			if(b)
				b->set(0, 0, b->w, b->h, 0, 0, image->w, image->h, image, bitmapNORMAL, bitmapNORMAL);
		}
		{
			byte	smp[16384];
			Asample	*sp=back->aout;
			int		a=(int)audio->getSize();
			if(a>fillsize)
			{
				fillsize=0;
				while(a)
				{
					int	n=mini(sizeof(smp), a);
					int	r=audio->read(smp, n);
					sp->vmaster=front->master->get()*2.f;
					sp->vbass=front->bass->get()*2.f;
					sp->vmedium=front->medium->get()*2.f;
					sp->vtreeble=front->treble->get()*2.f;
					sp->addBuffer((sword *)smp, r>>2);
					a-=n;
				}
			}
		}
		simage.leave();
		break;

		case SERVER:
		simage.enter(__FILE__,__LINE__);
		{
			Abitmap	*b=back->vin->getBitmap();
			if(b)
				image->set(0, 0, b, bitmapNORMAL, bitmapNORMAL);
			bvideo=b?true:false;
		}
		baudio=back->ain->isConnected();
		if(baudio)
		{
			byte	smp[16384];
			Asample	*sp=back->ain;
			int		a=sp->avaible()*4;
			while(a)
			{
				int	n=mini(sizeof(smp), a);
				int	r=sp->getBuffer((sword *)smp, NULL, NULL, NULL, n>>2);
				audio->write(smp, r<<2);
				a-=n;
			}
		}
		simage.leave();
		break;
	}
	frame++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Anetwork::notify(Anode *n, int event, dword p)
{
	switch(event)
	{
		case nyRESET:
		brun=false;
		while(!bout)
			sleep(10);
		stop();
		return true;
	}
	return Anode::notify(n, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Anetwork::change(int mode, int port)
{
	brun=false;
	while(!bout)
		sleep(10);
	stop();
	this->mode=mode;
	if(mode!=NONE)
	{
		this->port=port;
		start();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Anetwork::run()
{
	bout=false;
	brun=true;
	berror=false;

	{
		char	name[256];
		switch(mode)
		{
			case CLIENT:
			aePipeName(name, port, srvname);
			pipe=new Apipe(name, Apipe::READ|Apipe::CLIENT);
			break;

			case SERVER:
			if(!used[port])
			{
				aePipeName(name, port);
				pipe=new Apipe(name, Apipe::WRITE|Apipe::SERVER);
				used[port]=true;
			}
			break;
		}
	}

	fillsize=17640;

	if(pipe&&pipe->isOK())
	{
		int		mframe=frame;
		bool	bmagic=false;
		while(brun)
		{
			if(mode==SERVER)
			{
				while(brun)
				{
					if(pipe->waitConnection())
						break;
					sleep(30);
				}
			}
			while(brun)
			{
				switch(mode)
				{
					case CLIENT:
					{
						TAEAV_CHUNK	chunk;
						qword		magic;

						if(bmagic||((pipe->read(&magic, sizeof(magic))==sizeof(magic))&&(magic==AEAV_MAGIC_CHUNK)))
						{
							bmagic=false;
							if(pipe->read(&chunk, sizeof(chunk))==sizeof(TAEAV_CHUNK))
							{
								switch(chunk.magic)
								{
									case AEAV_MAGIC_ENDSTREAM:
									brun=false;
									break;

									case AEAV_MAGIC_PING:
									break;

									case AEAV_MAGIC_FRAME:
									{
										TAEAV_FRAME	f;
										if(pipe->read(&f, sizeof(f))==sizeof(f))
											frame=f.frame;	// 4 nothing 	// todo: error catch
									}
									break;

									case AEAV_MAGIC_AUDIOPCM:
									{
										TAEAV_AUDIOPCM	a;
										if(pipe->read(&a, sizeof(a))==sizeof(a))
										{
											byte	smp[16384];
											int		size=(a.nbsamples*a.nbchannel*a.nbbits)>>3;
											while(size)
											{
												int	tr=mini(size, sizeof(smp));
												if(pipe->read(smp, tr)!=tr)
												{
													pipe->timeout=10;
													asyncNotify(this, nyRESET, 0);
													break;											
												}
												simage.enter(__FILE__,__LINE__);
												if(!audio->write(smp, tr))
												{
													audio->clear();
													audio->write(smp, tr);
												}
												simage.leave();
												size-=tr;
											}
										}
									}
									break;

									case AEAV_MAGIC_VIDEOJPEG:
									simage.enter(__FILE__,__LINE__);
									{
										Afilemem	*fm=new Afilemem("fm", bufjpeg, chunk.size);
										int			z,a=0,v,o=(int)pipe->offset;
										pipe->read(bufjpeg, chunk.size);
										image->load(fm, bitmapJPG);
										delete(fm);
										v=(int)pipe->offset;
										a=(int)pipe->offset-o;
										z=chunk.size-a;
										a++;
									}
									simage.leave();
									break;

									default:
									assert(false);
									break;
								}
							}
						}
						else
						{
							bool	r=true;
							int		ndbg=0;
							while(r)
							{
								int		n=0;
								int		i;
								byte	b;
								qword	magic=AEAV_MAGIC_CHUNK;
								for(i=0; i<8; i++)
								{
									if(pipe->read(&b, sizeof(b))==sizeof(b))
									{
										byte	v=(byte)(magic>>n);
										ndbg++;
										if(b==v)
											n+=8;
										else
											break;
									}
									else
									{
										r=false;
										pipe->timeout=10;
										asyncNotify(this, nyRESET, 0);
										break;											
									}
								}
								if(i==8)
								{
									bmagic=true;
									break;
								}
							}
						}
					}
					break;

					case SERVER:
					if(frame!=mframe)
					{
						TAEAV_CHUNK	c;
						qword		magicsync=AEAV_MAGIC_CHUNK;
						mframe=frame;
						{
							TAEAV_FRAME	f;
							c.magic=AEAV_MAGIC_FRAME;
							c.size=sizeof(TAEAV_FRAME);
							f.frame=mframe;
							f.timestamp=(dword)getTable()->getTime();
							if(pipe->write(&magicsync, sizeof(magicsync))!=sizeof(magicsync))
								goto serverError;
							pipe->write(&c, sizeof(c));
							pipe->write(&f, sizeof(f));
						}
						simage.enter(__FILE__,__LINE__);
						if(bvideo)
						{
							Afilemem	*fm=new Afilemem("bufjpeg", bufjpeg, getVideoWidth()*getVideoHeight()*3);
							image->save(fm, bitmapJPG, 80);
							c.magic=AEAV_MAGIC_VIDEOJPEG;
							c.size=(int)fm->getSize();
							if(pipe->write(&magicsync, sizeof(magicsync))!=sizeof(magicsync))
							{
								simage.leave();
								goto serverError;
							}
							pipe->write(&c, sizeof(c));
							pipe->write(bufjpeg, c.size);
							delete(fm);
						}
						if(baudio)
						{
							int	s=(int)audio->getSize();
							if(s)
							{
								TAEAV_AUDIOPCM	pcm;
								byte			smp[16384];
								c.magic=AEAV_MAGIC_AUDIOPCM;
								c.size=s+sizeof(pcm);
								if(pipe->write(&magicsync, sizeof(magicsync))!=sizeof(magicsync))
								{
									simage.leave();
									goto serverError;
								}
								pipe->write(&c, sizeof(c));
								pcm.frequency=44100;
								pcm.nbbits=16;
								pcm.nbchannel=2;
								pcm.nbsamples=s>>2;
								pipe->write(&pcm, sizeof(pcm));
								while(s)
								{
									int	sx=mini(sizeof(smp), s);
									audio->read(smp, sx);
									if(pipe->write(smp, sx)!=sx)
										break;
									s-=sx;
								}
							}
						}
						simage.leave();
					}
					else
					{
						TAEAV_CHUNK	c;
						qword		magicsync=AEAV_MAGIC_CHUNK;
						c.magic=AEAV_MAGIC_PING;
						c.size=0;
						if(pipe->write(&magicsync, sizeof(magicsync))!=sizeof(magicsync))
							goto serverError;
						pipe->write(&c, sizeof(c));
					}
					break;
				}
				sleep(30);
			}
			continue;
		serverError:
			brun=false;
			berror=true;
		}

	}
	else
		front->notify(this, nyERROR, (dword)"can't open network connection");

	if(pipe&&(mode==SERVER))
	{
		qword		magicsync=AEAV_MAGIC_CHUNK;
		TAEAV_CHUNK	c;
		c.magic=AEAV_MAGIC_ENDSTREAM;
		c.size=0;
		pipe->write(&magicsync, sizeof(magicsync));
		pipe->write(&c, sizeof(c));
		pipe->flush();
		used[port]=false;
	}
	if(pipe)
	{
		delete(pipe);
		pipe=NULL;
	}
	brun=false;
	bout=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AnetworkFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnetworkFront::AnetworkFront(QIID qiid, char *name, Anetwork *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_NETWORK), "PNG");
	back=new Abitmap(&o);

	server=new Aedit("server", this, 124, (pos.h-14)>>1, 100, 14);
	server->colorBorder=0xff606060;
	server->colorText=0xffffaa00;
	server->colorBackground=0xff606060;
	server->setTooltips("network computer");
	{
		WSADATA wd;
		if(WSAStartup(MAKEWORD(2, 2), &wd)==0)
		{
			char	hostname[ALIB_PATH];
			memset(hostname, 0, sizeof(hostname));
			gethostname(hostname, 128);
			server->set(hostname);
			WSACleanup();
			strcpy(e->srvname, hostname);
		}
	}
	server->show(true);

	port=new Asegment("port", this, 264, 17, 2, 1, 23, alib.getFont(fontSEGMENT10), 0xff404040, 0.3f);
	port->setTooltips("eleknet port");
	port->set(1);
	port->show(true);

	browse=new Abutton("browse network", this, 88, 13, 24, 24, &resource.get(MAKEINTRESOURCE(PNG_NETWORK_BROWSE), "PNG"));
	browse->setTooltips("browse network [computer near me]");
	browse->show(true);

	client=new Abutton("client", this, 237, 21, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	client->setTooltips("receive mode [client]");
	client->setChecked(false);
	client->show(TRUE);

	power=new Abutton("power", this, 302, 21, 9, 9, &resource.get(MAKEINTRESOURCE(PNG_LOOP_SHAPE), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	power->setTooltips("power - switch on/off network stream");
	power->setChecked(false);
	power->show(TRUE);

	master=new Apaddle(MKQIID(qiid, 0x3b597fc13768c7c0), "master", this, 440, 13, paddleELEK24);
	master->setTooltips("master");
	master->set(0.5f);
	master->show(TRUE);

	bass=new Apaddle(MKQIID(qiid, 0xf4323ac5405c2110), "bass", this, 370, 20, paddleELEK16);
	bass->setTooltips("bass");
	bass->set(0.5f);
	bass->show(TRUE);

	medium=new Apaddle(MKQIID(qiid, 0xd4fbcc0383b3d000), "medium", this, 390, 20, paddleELEK16);
	medium->setTooltips("medium");
	medium->set(0.5f);
	medium->show(TRUE);

	treble=new Apaddle(MKQIID(qiid, 0xe9c32b0c92e625b0), "treble", this, 410, 20, paddleELEK16);
	treble->setTooltips("treble");
	treble->set(0.5f);
	treble->show(TRUE);

	timer(100);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnetworkFront::~AnetworkFront()
{
	delete(master);
	delete(bass);
	delete(medium);
	delete(treble);
	delete(power);
	delete(client);
	delete(browse);
	delete(port);
	delete(server);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AnetworkFront::pulse()
{
	Anetwork	*network=(Anetwork *)effect;
	bool		b=power->isChecked();
	power->setChecked(network->brun);
	if(network->berror&&network->bout)
		network->change(Anetwork::SERVER, network->port);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AnetworkFront::notify(Anode *o, int event, dword p)
{
	Anetwork	*network=(Anetwork *)effect;
	switch(event)
	{
		case nyCHANGE:
		if(o==power)
		{
			if(power->isChecked())
				network->change(client->isChecked()?Anetwork::CLIENT:Anetwork::SERVER, port->get());
			else
				network->change(Anetwork::NONE, 0);

		}
		else if(o==client)
		{
			network->change(Anetwork::NONE, 0);
		}
		else if(o==port)
		{
			network->change(Anetwork::NONE, 0);
		}
		else if(o==server)
		{
			server->get(network->srvname, sizeof(network->srvname));
			network->change(Anetwork::NONE, 0);
		}
		break;

		case nyCLICK:
		if(o==browse)
		{
			char		filename[ALIB_PATH];
			AfileDlg	fdlg=AfileDlg("network computer", this, "directory\0*.\0", NULL, AfileDlg::COMPUTER);
			if(fdlg.getFile(filename))
			{
				strcpy(network->srvname, filename);
				server->set(filename);
				network->change(Anetwork::NONE, 0);
			}
		}
		break;
	}
	return AeffectFront::notify(o, event, p);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnetworkBack::AnetworkBack(QIID qiid, char *name, Anetwork *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resource.get(MAKEINTRESOURCE(PNG_NETWORK2), "PNG");
	back=new Abitmap(&o);

	vin=new Avideo(MKQIID(qiid, 0x56d01a6a14f8c000), "video in", this, pinIN, 10, 10);
	vin->setTooltips("video in");
	vin->show(TRUE);

	vout=new Avideo(MKQIID(qiid, 0x97a56fadb7706008), "video out", this, pinOUT, pos.w-18, 10);
	vout->setTooltips("video out");
	vout->show(TRUE);

	ain=new Asample(MKQIID(qiid, 0x747973ae76c21e98), "audio in", this, pinIN, 10, 30);
	ain->setTooltips("audio in");
	ain->show(TRUE);

	aout=new Asample(MKQIID(qiid, 0xb03207b143b90d18), "audio out", this, pinOUT, pos.w-18, 30);
	aout->setTooltips("audio out");
	aout->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnetworkBack::~AnetworkBack()
{
	delete(back);
	delete(vin);
	delete(vout);
	delete(ain);
	delete(aout);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AnetworkBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AnetworkInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Anetwork(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Aplugz * networkGetInfo()
{
	return new AnetworkInfo("networkInfo", &Anetwork::CI, "network 23", "network 23 module - stream elektronika video between computers");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
