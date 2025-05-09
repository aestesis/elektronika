/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TCPREMOTE.H					(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektrodef.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_TCPREMOTE_H_
#define							_TCPREMOTE_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<interface.h>
#include						"../elektro/elektro.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							MAXOSCSTREAM					(16)
#define							MAXOSCSTRING					(1024)
#define							MAXOSCMESSAGE					(512)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
								oscSETVALUE=0,
								oscFIRSTQUERY=1,
								oscTYPESIGNATURE=1,
								oscGETVALUE=2,
								oscGETMINVALUE=3,
								oscGETMAXVALUE=4,
								oscDOCUMENTATION=5,
								oscUISIGNATURE=6,
								oscLASTQUERY
};

extern EDLL char				*oscSTRING[];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscData : public Anode
{
public:
	ELIBOBJ

								AoscData() {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscFloat : public AoscData
{
public:
	ELIBOBJ

	float						value;

	EDLL						AoscFloat(float f);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscBitmap : public AoscData
{
public:
	ELIBOBJ

	Abitmap						*value;

	EDLL						AoscBitmap(Abitmap *b);
	EDLL virtual				~AoscBitmap();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscString : public AoscData
{
public:
	ELIBOBJ

	char						value[MAXOSCSTRING];

	EDLL						AoscString(char *str);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscColor : public AoscData
{
public:
	ELIBOBJ

	dword						value;

	EDLL						AoscColor(dword c);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscInteger : public AoscData
{
public:
	ELIBOBJ

	int							value;

	EDLL						AoscInteger(int i);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscMessage : public Anode
{
public:
	ELIBOBJ

	char						pattern[MAXOSCSTRING];
	char						format[MAXOSCSTRING];
	bool						bok;

	EDLL						AoscMessage						(byte *buffer, int size);
	EDLL						AoscMessage						();
	EDLL virtual				~AoscMessage					();

	bool						isOK							() { return bok; }

	EDLL virtual int			getBytes						(byte *data, int sizemax);

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AoscStream : public Anode, Athread
{
public:
	ELIBOBJ

	SOCKET						socket;
	class AtcpRemote			*tcpRemote;

	Asection					section;
	Abuffer						*buffer;

	int							nerror;

	AoscMessage					*sndmsg[MAXOSCMESSAGE];
	int							nsndmsg;
	AoscMessage					*rcvmsg[MAXOSCMESSAGE];
	int							nrcvmsg;

	bool						srvstop;
	bool						srvstopped;

	EDLL						AoscStream						(AtcpRemote *tcpRemote, SOCKET s);
	EDLL virtual				~AoscStream						();

	virtual void				start							() { Athread::start(); }
	EDLL virtual void			close							();	// call delete via run thread

	EDLL virtual void			reply							(AoscMessage *msg);

	EDLL virtual void			run								();
	EDLL virtual void			idleProcess						(class Atable *table);

};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AtcpRemote : public Anode, Athread, Aidle
{
public:
	ELIBOBJ

	EDLL						AtcpRemote						(class Atable *table);
	EDLL virtual				~AtcpRemote						();
	
	EDLL void					run								();
	EDLL void					idlePulse						();

	// OSC

	EDLL void					addStream						(SOCKET s);
	EDLL void					delStream						(class AoscStream *osc);

	Asection					section;
	Atable						*table;
	bool						srvrun;
	bool						srvstop;
	SOCKET						socket;
	AoscStream					*oscStream[MAXOSCSTREAM];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_TCPREMOTE_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
