/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ELEKTRO.H					(c)	YoY'06						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef							_ELEKTRO_H_
#define							_ELEKTRO_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<interface.h>
#include						"table.h"
#include						"sequencer.h"
#include						"effect.h"
#include						"capsule.h"
#include						"transition.h"
#include						"effect3d.h"
#include						"pin.h"
#include						"connect.h"
#include						"simple.h"
#include						"sample.h"
#include						"video.h"
#include						"bpmTracker.h"
#include						"acontrol.h"
#include						"paddle.h"
#include						"selpad.h"
#include						"select.h"
#include						"selcolor.h"
#include						"slider.h"
#include						"ctrlButton.h"
#include						"trigger.h"
#include						"equalizer.h"
#include						"zone.h"
#include						"timeline.h"
#include						"selectView.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	If you use directly the variables in the class API, it's at your own risks. Prefer the methods...   

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ELEKTRO_QIID_DEFINED_
#define _ELEKTRO_QIID_DEFINED_
typedef qword					QIID;
#endif

__inline QIID MKQIID(QIID x, QIID y)
{
	return x^y;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//								Atable							0x00000000
//								AtableFront						0x00000010
//								AtableBack						0x00000020
//								Actbar							0x00000030
//								Aonoff							0x00000040

//								Asequencer						0x00000100
//								Atrack							0x00000110

//								AtcpRemote						0x00000200
//								AoscStream						0x00000210
//								AoscMessage						0x00000220

//								AoscData						0x00000230
//								AoscInteger						0x00000231
//								AoscFloat						0x00000232
//								AoscString						0x00000233
//								AoscColor						0x00000234

//								AbpmTracker						0x00000500

//								Acapsule						0x00008000

//								Aeffect							0x00009000
//								AeffectInfo						0x00009001
//								AeffectFront					0x00009005
//								AeffectBack						0x00009006

//								Atransition						0x00009010
//								AtransitionInfo					0x00009011

//								Aeffect3d						0x00009020
//								Aeffect3dInfo					0x00009021
//								Ainput3d						0x00009022
//								Ainput3dInfo					0x00009023
//								Aeffect3dSound					0x00009024

//								AoscNode						0x00009030

//								Apin							0x00010020
//								Aconnect						0x00010030

//								Acontrol						0x00010040
//								Amapping						0x00010041
//								Apaddle							0x00010050
//								Aselect							0x00010060
//								Aselcolor						0x00010070
//								Aslider							0x00010080
//								ActrlButton						0x00010090
//								Atrigger						0x00010100
//								ActrlTrigger					0x00010105
//								Aequalizer						0x00010110
//								Azone							0x00010120
//								Aselpad							0x00010130
//								Atimeline						0x00010140
//								AselectView						0x00010150

//								Asimple							0x00012000
//								Asample							0x00012010
//								Avideo							0x00012020

#define 						guidSIMPLES						GUID(0xE4EC7600, 0x10000000)

#define 						guidBYTE 						(guidSIMPLES+1)					// unsigned byte
#define 						guidWORD						(guidBYTE+1)					// unsigned word
#define 						guidDWORD						(guidWORD+1)					// unsigned dword
#define 						guidQWORD						(guidDWORD+1)					// unsigned qword

#define 						guidSBYTE						(guidQWORD+1)					// signed byte
#define 						guidSWORD						(guidSBYTE+1)					// signed word
#define 						guidSDWORD						(guidSWORD+1)					// signed dword
#define 						guidSQWORD						(guidSDWORD+1)					// signed qword

#define 						guidFLOAT						(guidSQWORD+1)					// float	range [0..1]
#define 						guidDOUBLE						(guidFLOAT+1)					// double	range [0..1]

#define 						guidSFLOAT						(guidDOUBLE+1)					// sfloat	range [-1..1]
#define 						guidSDOUBLE						(guidSFLOAT+1)					// sdouble	range [-1..1]

#define 						guidSTRING128					(guidSQWORD+1)					// string 128
#define 						guidSTRING256					(guidSTRING128+1)				// string 256
#define 						guidSTRINGP						(guidSTRING256+1)				// string with allocation

#define 						guidLASTSIMPLE					(guidSTRINGP+1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
								contextADDEFFECT				= contextELEKTRO,
								contextLASTEFFECT				= contextADDEFFECT+1024,
								contextDELEFFECT,
								contextMAPPING_NONEXIST_REMOVED,
								contextADDTRACK,
								contextDELTRACK,
								contextMONITOR,
								contextRANDOMIZE,
								contextSEQMUTEALL,
								contextSEQUNMUTEALL,
								contextSEQUNSOLOALL,
								contextCLEAR
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum magic
{
								magicTABLE=0xf4910000,	// WARNING !!!!!!!!
								magicEFFECTS,			// only append, no insert because of file version compatibility
								magicEFFECT,
								magicCONTROLS,
								magicCONTROL,
								magicCONNECT,
								magicONOFF,
								magicPRESET,
								magicPRESETTEXT,
								magicPROGRAMCHANNEL,
								magicPRESETFILENAME,
								magicCLOCKTYPE,
								magicIN3D,
								magicEF3D,
								magicTHREADING,
								magicLOOPTIME,
								magicMINMAXCTRL,
								magicSEQUENCER,
								magicPATTERN,
								magicTRACK,
								magicTRACKEFFECT,
								magicTRACKSUB,
								magicTRACKCONTROL,
								magicMODECTRL,
								magicELEKVERSION,
								magicRAMCOMPRESS,
								magicPATTERNLENGHT,
								magicPATTERNPRESET,
								magicPRESETPOS,
								magicEF3DPIX,
								magicMIDIIN,
								magicDSOUNDSIZE,
								magicEFFECTNAME,
								magicMPMEDIA,			// OMPP MEDIA TAG
								magicMASK3D
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
								nyBPMCHANGE=nySUBSYS1,
								nySELECTDROP,
								nyPINCLICK,
								nyPINENTER,
								nyPINLEAVE,
								nyADDEFFECT
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool magictest(Afile *f, dword magic)
{
	dword m;
	if(!f->read(&m, sizeof(m)))
		return false;
	if(m==magic)
		return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline bool safemagictest(Afile *f, dword magic)
{
	sqword	oset=f->offset;
	dword	m;
	if(!f->read(&m, sizeof(m)))
		return false;
	if(m==magic)
		return true;
	f->seek(oset);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
