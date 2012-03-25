/****************************************************************************
CONFIDENTIAL AND PROPRIETARY INFORMATION.  The entire contents of this file
is Copyright © Macromedia, Inc. 1993-1998, All Rights Reserved.  This
document is an unpublished trade secret of Macromedia, Inc. and may not be
viewed, copied or distributed by anyone, without the specific, written
permission of Macromedia, Inc. 
****************************************************************************/

#ifndef SPLAY_INCLUDED
#define SPLAY_INCLUDED

#ifndef SDISPLAY_INCLUDED
#include "sdisplay.h"
#endif
#ifndef sndmix_INCLUDED
#include "sndmix.h"
#endif

//
// Helpers for Parsing a Script
//

class CSoundChannel;
struct StreamData;

// The low level parser
class SParser {
public:
	U8 * script;
	S32 pos;

	// Bit Handling
	U32 bitBuf;
	int bitPos;

public:
	// Tag Parser
	S32 tagPos;		// the start pos of the tag
	S32 tagEnd;
	int tagCode;

	int GetTag(S32 len=0x1FFFFFFF);	// return -1 if the tag is not yet loaded given len data

public:
	SParser() { script = 0; pos = 0; };
	void Attach(U8 * s, S32 start, S32 e = (1L<<29)) { script = s; pos = start; tagEnd = e; };

	// Get data from the script
	U8 GetByte();
	U16 GetWord();
	U32 GetDWord();

	void SkipBytes(int n) { pos += n; }

	S32 GetData(void * data, S32 len);	// returns the actual number of bytes read could 
										//	be less than len if we hit the end of a tag

	void GetColor(RGBI*, BOOL alpha=false);
	SRGB GetColor(BOOL alpha=false);
	void GetRect(SRECT*);
	void GetMatrix(MATRIX*);
	void GetColorTransform(ColorTransform*, BOOL alpha=false);
	char* GetString();	// caller must free memory with a delete operator
	char* GetStringP();	// just return a pointer to the string data
	#ifdef SOUND
	void GetSoundInfo(CSoundChannel*);
	#endif

	// Routines for reading arbitrary sized bit fields from the stream
	//	always call start bits before gettings bits and do not intermix 
	//	this calls with GetByte, etc...	
	void InitBits();
	U32 GetBits(int n);		// get n bits from the stream
	S32 GetSBits(int n);	// extend the sign bit
};


// The Shape Edge parser
struct SLine {
	RColor* color;
	int thickness;
};

class ScriptPlayer;
class SPlayer;

class SShapeParser : public SParser {
public:
	DisplayList* display;
	ScriptPlayer* player;
	RColor* localColors;	// if the colors are not kept, colorList points to this list
	RColor** colorList;	   	// the colors are added to this list
	MATRIX mat;				// the edges are transformed by this matrix - defaults to identity
	ColorTransform cxform;	// the color transform
	S32 ratio;				// the morph ratio
	CRaster* raster;		// optional raster if you want the color caches built
	U32 layer;			// the base layer for the edges

	// Line and Fill Index
	int nLines, nFills;
	RColor** fillIndex;
	SLine* lineIndex;

	RColor* fillIndexMem[256];
	SLine lineIndexMem[256];

	int nFillBits, nLineBits;

	BOOL getAlpha;

	// Current State
	int line, fill[2];
	SPOINT curPt, curPtX;

public:
	void SetupColor(RColor*);

public:
	SShapeParser(ScriptPlayer* p, U8 * data, S32 start, MATRIX*);
	~SShapeParser();
	BOOL GetStyles();
	BOOL GetMorphStyles();

	int GetEdge(P_CURVE);	// return the edge flags
};

// lee@middlesoft moved this to the header - there was 2 copies, which created hell
// when they became slightly out of sync
//
// The Character Parser
//
// Takes the packed edge data and adds processed edges to the object.

class SCharacterParser : public SShapeParser {
public:
	BOOL useWinding;
	BOOL preclip;
	int fillRule;
	RColor* color1;
	RColor* color2;
	int depth;
	SObject* obj;

public:
	SCharacterParser(ScriptPlayer* p, U8* data, S32 start, MATRIX* m) : 
			SShapeParser(p, data, start, m) { useWinding = false; }

	void AddCurve(P_CURVE);
	void MakeClipper();
	void BuildEdges(BOOL getStyles, SRECT*);
	void BuildMorphEdges(SRECT*);
};


class ScriptVariable {
public:
	ScriptVariable* next;
	char* name;
	char* value;
};


//
// The ScriptThread handles the flow of control and parsing for a timeline
//

class ScriptThread : public SParser {
public:
	enum { 
			noErr = 0, 
			badHeaderErr = -1, 	// The header tag was missing
			noMemErr = -2,		// There was not enough memory available
			noScriptErr = -3,	// There was not enough memory available
			playOK = 0, 		// The frame was drawn properly
			playNeedData = 1,	// The player needs more data to finish the frame
			playAtEnd = 2		// The player is at the end of the script
		 };

public:
	ScriptPlayer* player;		// The player that has the character dictionary for this thread
	ScriptThread* next;			// for the display list to use
 	SPlayer* splayer;			// Needed to support multiple sound objects in ScriptThread - else
								// this could be deprectated to ScriptPlayer. lee@middlesoft

	// The script stream/data
	S32 startPos;	// the first tag in the script data
	S32 len;		// the length of the data we have so far...

	BOOL atEnd;
	int curFrame;
	BOOL seeking;
    BOOL justSeeked;            // used for mp3 streaming
    BOOL justSeekedNegative;    // used for mp3 streaming

	BOOL playing;		// set if the movie should be playing
	BOOL loop;			// set if the movie should loop at the end
	
	int numFrames;	// the number of frames
	int scriptErr;
	U16 layerDepth;	// the depth to add to all place/remove objects for multiple layer support

	char* currentLabel;			// a pointer to the most recent frame label encountered to track state information

	DisplayList* display;		// The display list we are attached to
	SObject* rootObject;		// The root object we have created on the display list for this thread

	#ifdef SOUND
	// Sound Stream State
	// Note the sound thread runs a bit ahead of the main draw thread to solve latency problems
	BOOL needBuffer;			// true if we should prebuffer some frames before starting the sound
	BOOL sndAtEnd;				// true if we hit the end of the movie for the stream
	int mixFormat;				// the suggested format for the global sound mixer
	CSound snd;					// the format of all streaming sound in the file
	CSoundChannel* sndChannel;	// the sound channel for the streaming sound, nil if not streaming...
	int sndFrame;				// the current frame number for the sound thread
	S32 sndPos;					// the position of the sound thread
	#endif

public:	
	void PlaceObject();
	void PlaceObject2();
    void CloneSprite(ScriptThread *thread, char *name, int depth);
    void RemoveSprite(ScriptThread *thread, BOOL createPlaceholder = true);
	void RemoveObject();
	void DefineShape();
	void DefineMorphShape();
	void DefineButton();
	void DefineButtonExtra(int);
	void DefineSprite();
	void DefineBits();
	void DefineFont();
	void DefineFont2();
	void DefineFontInfo();
	void DefineText();

#ifdef EDITTEXT
	void DefineEditText();
#endif

#ifdef USE_VIDEO
	void DefineEditText();
#endif

	void DefineJPEGTables();

	#ifdef SOUND
	void DefineSound();
	void StartSound();

	void SoundStreamHead();
	void SoundStreamBlock();
    void AdjustMp3Streaming();

	void StopStream();
	BOOL DoSoundFrame();
	void PlayButtonSound(SObject* btn, int newState);
	#endif
	
	#ifdef _CELANIMATOR
	BOOL protect;			// set if this file should not be imported
	BOOL pathsArePostScript;	// set if paths should be treated as PostScript
	void DefineBitsPtr();
	#endif

	#ifdef FSSERVER
	U8 * AttachData(S32 start, S32 end);
	void ReleaseData(U8 *&);
	#else
	U8 * AttachData(S32 start, S32 end) { return script+start; }
	void ReleaseData(U8 *&) {}
	#endif

	int DoTag();
	int DoTags(int);

	void BuildBits(SCharacter*);

public:
	// Support for Variables
	ScriptVariable *firstVariable, *lastVariable;

	ScriptVariable* FindVariable(char* name);
	void SetVariable(char* name, char* value, BOOL updateFocus = true);

public:
	ScriptThread();
	~ScriptThread();

	void ClearState();
	void ClearVariables();

	void SetDisplay(DisplayList* d) 
	{ 
		FLASHASSERT(!display);
		display = d;
		display->AddThread(this);
	}

	// Play the script
	int GetFrame() { return curFrame; }
	BOOL AtEnd() { return atEnd; }

	int DrawFrame(int fnum, BOOL seek);	// advance or seek to the specified frame number
	void DoFrame(int frameDelta = 1, BOOL bForceRebuild = false); // handle a clock tick for the thread

	void Play(BOOL rewind=true);
	void StopPlay();
	void Seek(int f);
	int FindLabel(char*);
// #ifdef SPRITE_XTRA
// 	BOOL GetFrameLabel(int frameNum, char* label);
// #endif
};


//
// The ScriptPlayer handles a stream header, the character dictionary and streaming
//

// Special reserved character tag ids
#define	ctagBitmapBug           65535,	// reserved as a workaround for an old export bug
#define ctagThreadRoot          65534
#define ctagExternalThreadRoot  65533
#define ctagSpritePlaceholder   65532

struct JPEGPlayState;

typedef void (*LostCharacterProc)(void* data, U16 tag);

class ScriptPlayer : public ScriptThread {
public:
	// The URL this script thread was downloaded from
	char *url;

	// The script
	S32 scriptLen;	// the length of the script from the header
    int *scriptRefCount;  // used for cloning external sprites

	S32 headerLen;	// temp data for holding a partial header
	U8 headerBuf[8];

	BOOL gotHeader;

	int numFramesComplete;	// info for tracking the numbers of frames loaded
	U32 numFramesCompletePos;

	// The header info
	SRECT frame;	// the picture frame
	SFIXED frameRate;// the number of frames to show per second
	S32 frameDelay;	// the number of milliseconds to between frames given the frame rate
	int version;	// the version number from the header
	
	JPEGPlayState* jpegInfo;

	// For handling layers and streaming in SPlayer
// 	SPlayer* splayer;				// moved to ScriptThread in order to support multiple sound objects
 	StreamData* stream;			    // a slot for the stream object to store data
	
	ScriptPlayer* nextLayer;

	#ifdef SOUND
	BOOL mute;					// don't play any sounds
	#endif

	// These members are used for Load Variable actions
	BOOL variableLoader;
	int variableLayer;
	char *variableSprite;
#define VAR_INCR 2048

public:	
	// The Character list
	enum { charIndexSize = 128, charIndexMask = 127 };
	SCharacter* charIndex[charIndexSize];	// a hash table sorted by character tag values

	// Memory Allocators
	ChunkAlloc characterAlloc;

    // add one to the refcount and return the new value
    inline int ScriptAddRefCount()
    {
        if (!scriptRefCount)
        {
            scriptRefCount = new int;
            *scriptRefCount = 0;
        }

        *scriptRefCount += 1;
        return *scriptRefCount;
    }
    
    // sub one from the refcount and return the new value
    inline int ScriptSubRefCount()
    {
        if (scriptRefCount)
        {
            *scriptRefCount -= 1;
            int result = *scriptRefCount;
            if (result <= 0) 
            {
                delete scriptRefCount;
                scriptRefCount = 0;
            }
            return result;
        }
        else
            return 0;
    }
    
    
	inline SCharacter* CreateCharacter() { return (SCharacter*)(characterAlloc.Alloc()); }
	void FreeCharacter(SCharacter* c);

	void FreeAll();
	void FreeCache();

public:	
	// Internal Methods
	SCharacter* CreateCharacter(U16 tag);
	SCharacter* FindCharacter(U16 tag);
	void FreeCharacter(U16 tag);

public:
	ScriptPlayer();
	~ScriptPlayer();

	void ClearScript();

	void PushDataBuf(U8* data, S32 chunkLen);	// push a partial chunk of data
	void PushDataComplete();					// the current chunk of data is complete, can execute actions and abort a stream...
	void SetDataComplete();						// the stream is complete, close it

	// This is a deprecated method - it is not safe to call this in a loop since PushDataComplete can trigger actions and potentially clear the stream we are writing to
	void PushData(U8* data, S32 chunkLen) { PushDataBuf(data, chunkLen); PushDataComplete(); }

	// return true if the script data for a given frame is loaded
	BOOL FrameComplete(int f) { return numFramesComplete >= f; }
	BOOL ScriptComplete() { return script && len >= scriptLen; }

	BOOL ParseHeader() { return scriptErr == noErr && gotHeader; } // return true if we have a valid header
};

/*! \page STRINGUTILITY String Utilities.
 *	The string utilities are defined in splay.h and provide general
 *	functions for working with Flash strings.<p>
 *  <b>BOOL ConvertStringToInteger(const char *s, int *value)</b><br>
 *	Converts a string to an integer value. Returns true if it succeeds, false if it fails.<p>
 *	<b>char* SkipPrefix( char* str, char* pre )</b><br>
 *  If the prefix pre is found, returns a pointer
 *	past the prefix. If not found, returns null.<p>
 *	<b>BOOL StrEqual( const char* a, const char* b)</b><br>
 *  Returns true if both strings equal.<p>
 *	<b>int ToInt(const char* c)</b><br>
 *  String to integer conversion.<p>
 *	<b>void FreeStr(char* s)</b><br>
 *  Free's memory created with CreateStr.<p>
 *	<b>char* CreateStr( const char* s)</b><br>
 *  Creates a string (alloctating memory)
 *	by copying the string passed in. Returns a pointer to 
 *	the created string.<p>
 *	<b>char *CreateStr( const char *s, int count)</b><br>
 *  String utility. Creates a string (alloctating memory)
 *	by copying the string passed in. Count specifies the
 *	amount of memory allocated and must be sufficient.<p>
 *	<b>BOOL IsTrue( const char* )</b><br>
 *  Returns true if the string passed in
 *	a string representation of a non zero number, the
 *  word "true" or the word "yes".<br>
 */

/*!
 *  String utility. Converts a string to an integer value. Returns
 *	true if it succeeds, false if it fails.
 */
BOOL ConvertStringToInteger(const char *s, int *value);

/*! String utility. If the prefix pre is found, returns a pointer
 *	past the prefix. If not found, returns null.
 */	
char* SkipPrefix( char* str, char* pre );

/*! String utility. True if both strings equal.
 */
BOOL StrEqual( const char* a, const char* b);

/*! String utility. String to integer conversion.
 */
int ToInt(const char* c);

/*! String utility. Free's memory created with CreateStr.
 */
void FreeStr(char* s);

/*! String utility. Creates a string (alloctating memory)
 *	by copying the string passed in. Returns a pointer to 
 *	the created string.
 */
char* CreateStr( const char* s);

/*! String utility. Creates a string (alloctating memory)
 *	by copying the string passed in. Count specifies the
 *	amount of memory allocated and must be sufficient.
 */
char *CreateStr( const char *s, int count);

/*! String utility. Returns true if the string passed in
 *	a string representation of a non zero number, the
 *  word "true" or the word "yes"
 */
BOOL IsTrue( const char* );

//
// Inlines for our basic parser
//

inline U8 SParser::GetByte() 
{
	//if ( pos >= end ) return 0;
	return script[pos++];
}

inline U16 SParser::GetWord()
{
	U8 * s = script+pos;
	pos+=2;
	//if ( pos > end ) return 0;
	return (U16)s[0] | ((U16)s[1]<<8);
}

inline U32 SParser::GetDWord()
{
	U8 * s = script+pos;
	pos+=4;
	//if ( pos > end ) return 0;
	return (U32)s[0] | ((U32)s[1]<<8) | ((U32)s[2]<<16) | ((U32)s[3]<<24);
}

//
// LoadInfo
//
// This structure is used for Load Sprite and Load Variable actions
//

struct LoadInfo
{
	int layer;
	char *spriteName;
	BOOL loadVars;
};

/*! The structure associated with a download stream. The structure needs to be allocated and
 *  attached to the stream, but the Flash core will do all the writing that is needed.
 */
struct StreamData
{
	char*			url;			//!< The URL of the stream.
	ScriptPlayer*	scriptPlayer;	//!< Pointer to the ScriptPlayer associated with this download stream.
	BOOL			resize;			//!< Should the window be resized to this stream dimension?
};
		

#endif



