#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include		<interface.h>
#include		"..\\elektro\\elektro.h"
#include		"password.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							VERSION					RESELLVER "2.77"
#define							VERSION_NUMBER			(0x00025003)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
				contextBACKLINE=contextELEKTRONIKA,
				contextBACKLINE2,
				contextBACKDIAG,
				contextBACKPATCHLY,
				contextBACKBLACK,
				contextBACKORANGE,
				contextBACKTEXT01,
				contextBACKTEXT02,
				contextBACKTEXT03,
				contextBACKTEXT04,
				contextBACKTEXT05,
				contextBACKTEXT06,
				contextBACKTEXT07,
				contextABOUT,
				contextCOPYPATTERN,
				contextPASTEPATTERN,
				contextRANDPATTERN
};

__inline float frand()
{
	return (float)rand()/(float)RAND_MAX;
}


#endif
