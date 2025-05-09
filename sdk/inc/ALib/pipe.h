/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PIPE.H						(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_PIPE_H_
#define							_PIPE_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						"types.h"
#include						"file.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							guidPIPE						(Apipe::CI.guid)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! a class that manage PIPE operation (local and network)
class Apipe : public Afile
{
public:
	ALIBOBJ

	enum
	{
								SEPARATOR	= '\\'
	};

	enum
	{
								READ=1,
								WRITE=2,
								SERVER=4,
								CLIENT=8
	};


	ADLL						Apipe							(char *name, int mode);
	ADLL virtual				~Apipe							();
	
	ADLL virtual int			avaible							();	// return nb avaible bytes to read

	ADLL virtual int			read							(void *p, int s);
	ADLL virtual int			write							(void *p, int s);

	ADLL virtual bool			writeString						(char *s);
	ADLL virtual bool			readString						(char *s);
	ADLL virtual bool			readStringAlloc					(char **s);

	ADLL virtual bool			isOK							();

	ADLL virtual bool			flush							();
	ADLL virtual bool			isConnected						();	// server
	ADLL virtual bool			waitConnection					();	// server

	HANDLE						hpipe;
	int							mode;
	int							timeout;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif							//_PIPE_H_
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
