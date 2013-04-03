/*
** Copyright (C) 2003 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#ifndef _ML_H_
#define _ML_H_


#define MLHDR_VER 0x15

typedef struct {
	int version;
	char *description;
	int (*init)();
	void (*quit)();


  // return NONZERO if you accept this message as yours, otherwise 0 to pass it 
  // to other plugins
  int (*MessageProc)(int message_type, int param1, int param2, int param3); 

  //all the following data is filled in by the library
	HWND hwndWinampParent;
	HWND hwndLibraryParent; // send this any of the WM_ML_IPC messages
	HINSTANCE hDllInstance;
} winampMediaLibraryPlugin;


// messages your plugin may receive on MessageProc()

#define ML_MSG_TREE_BEGIN 0x100
  #define ML_MSG_TREE_ONCREATEVIEW 0x100 // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us

  #define ML_MSG_TREE_ONCLICK  0x101 // param1 = param of tree item, param2 = action type (below), param3 = HWND of main window
    #define ML_ACTION_RCLICK 0    // return value should be nonzero if ours
    #define ML_ACTION_DBLCLICK 1
    #define ML_ACTION_ENTER 2

  #define ML_MSG_TREE_ONDROPTARGET 0x102 // param1 = param of tree item, param2 = type of drop (ML_TYPE_*), param3 = pointer to data (or NULL if querying).
                                      // return -1 if not allowed, 1 if allowed, or 0 if not our tree item

  #define ML_MSG_TREE_ONDRAG 0x103 // param1 = param of tree item, param2 = POINT * to the mouse position, and param3 = (int *) to the type
                                  // set *(int*)param3 to the ML_TYPE you want and return 1, if you support drag&drop, or -1 to prevent d&d.

  #define ML_MSG_TREE_ONDROP 0x104 // param1 = param of tree item, param2 = POINT * to the mouse position
                                  // if you support dropping, send the appropriate ML_IPC_HANDLEDROP using SendMessage() and return 1, otherwise return -1.

#define ML_MSG_TREE_END 0x1FF // end of tree specific messages



#define ML_MSG_ONSENDTOBUILD 0x300 // you get sent this when the sendto menu gets built
// param1 = type of source, param2 param to pass as context to ML_IPC_ADDTOSENDTO
// be sure to return 0 to allow other plugins to add their context menus


// if your sendto item is selected, you will get this with your param3 == your user32 (preferably some
// unique identifier (like your plugin message proc). See ML_IPC_ADDTOSENDTO
#define ML_MSG_ONSENDTOSELECT 0x301
// param1 = type of source, param2 = data, param3 = user32

// return TRUE and do a config dialog using param1 as a HWND parent for this one
#define ML_MSG_CONFIG 0x400

// types for drag and drop

#define ML_TYPE_ITEMRECORDLIST 0 // if this, cast obj to itemRecordList
#define ML_TYPE_FILENAMES 1 // double NULL terminated char * to files, URLS, or playlists
#define ML_TYPE_STREAMNAMES 2 // double NULL terminated char * to URLS, or playlists ( effectively the same as ML_TYPE_FILENAMES, but not for files)
#define ML_TYPE_CDTRACKS 3 // if this, cast obj to itemRecordList (CD tracks) -- filenames should be cda://<drive letter>,<track index>. artist/album/title might be valid (CDDB)
#define ML_TYPE_QUERYSTRING 4 // char * to a query string
#define ML_TYPE_TREEITEM 69 // uhh?


// if you wish to put your tree items under "devices", use this constant for ML_IPC_ADDTREEITEM
#define ML_TREEVIEW_ID_DEVICES 10000



// children communicate back to the media library by SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,param,ML_IPC_X);
#define WM_ML_IPC WM_USER+0x1000

#define ML_IPC_ADDTREEITEM 0x0101 // pass mlAddTreeItemStruct as the param
#define ML_IPC_SETTREEITEM 0x0102 // pass mlAddTreeItemStruct with this_id valid
#define ML_IPC_DELTREEITEM 0x0103 // pass param of tree item to remove
#define ML_IPC_GETCURTREEITEM 0x0104 // returns current tree item param or 0 if none
#define ML_IPC_SETCURTREEITEM 0x0105 // selects the tree item passed, returns 1 if found, 0 if not
#define ML_IPC_GETTREE 0x106 //returns a HMENU with all the tree items. the caller needs to delete the returned handle! pass mlGetTreeStruct as the param

typedef struct {
  int item_start;   // TREE_PLAYLISTS, TREE_DEVICES...
  int cmd_offset;   // menu command offset if you need to make a command proxy, 0 otherwise
  int max_numitems; // maximum number of items you wish to insert or -1 for no limit
} mlGetTreeStruct;

#define ML_IPC_NEWPLAYLIST           0x107 // pass hwnd for dialog parent as param
#define ML_IPC_IMPORTPLAYLIST        0x108 // pass hwnd for dialog parent as param
#define ML_IPC_IMPORTCURRENTPLAYLIST 0x109
#define ML_IPC_GETPLAYLISTWND        0x10A
#define ML_IPC_SAVEPLAYLIST          0x10B // pass hwnd for dialog parent as param
#define ML_IPC_OPENPREFS             0x10C

#define ML_IPC_PLAY_PLAYLIST 0x010D // plays the playlist pointed to by the tree item passed, returns 1 if found, 0 if not
#define ML_IPC_LOAD_PLAYLIST 0x010E // loads the playlist pointed to by the tree item passed into the playlist editor, returns 1 if found, 0 if not

typedef struct {
  int parent_id; //0=root, or ML_TREEVIEW_ID_*
  char *title;
  int has_children;

  int this_id; //filled in by the library on ML_IPC_ADDTREEITEM
} mlAddTreeItemStruct;

#define ML_IPC_GETFILEINFO 0x0200 // pass it a &itemRecord with a valid filename (and all other fields NULL), and it will try to fill in the rest
#define ML_IPC_FREEFILEINFO 0x0201 // pass it a &itemRecord tha twas filled by getfileinfo, it will free the strings it allocated

#define ML_IPC_HANDLEDRAG 0x0300 // pass it a &mlDropItemStruct it will handle cursors etc (unless flags has the lowest bit set), and it will set result appropriately:
#define ML_IPC_HANDLEDROP 0x0301 // pass it a &mlDropItemStruct with data on drop:

#define ML_IPC_SENDTOWINAMP 0x302 // send with a mlSendToWinampStruct:
typedef struct {
  int type; //ML_TYPE_ITEMRECORDLIST, etc
  void *data; // object to play

  int enqueue; // low bit set specifies enqueuing, and second bit NOT set specifies that 
               // the media library should use its default behavior as the user configured it (if 
               // enqueue is the default, the low bit will be flipped by the library)
} mlSendToWinampStruct;


typedef struct {
  char *desc; // str
  int context; // context passed by ML_MSG_ONSENDTOBUILD
  int user32; // use some unique ptr in memory, you will get it back in ML_MSG_ONSENDTOSELECT...
} mlAddToSendToStruct;
#define ML_IPC_ADDTOSENDTO 0x0400


#define ML_IPC_HOOKTITLE 0x0440 // this is like winamp's IPC_HOOK_TITLES... :) param1 is waHookTitleStruct
#define ML_IPC_HOOKEXTINFO 0x441 // called on IPC_GET_EXTENDED_FILE_INFO_HOOKABLE, param1 is extendedFileInfoStruct

#define ML_HANDLEDRAG_FLAG_NOCURSOR 1
#define ML_HANDLEDRAG_FLAG_NAME 2

typedef struct {
  int type; //ML_TYPE_ITEMRECORDLIST, etc
  void *data; // NULL if just querying

  int result; // filled in by client: -1 if dont allow, 0 if dont know, 1 if allow.

  POINT p; // cursor pos in screen coordinates
  int flags; 

  char *name; // only valid if ML_HANDLEDRAG_FLAG_NAME

} mlDropItemStruct;


#define ML_IPC_SKIN_LISTVIEW   0x0500 // pass the hwnd of your listview. returns a handle to use with ML_IPC_UNSKIN_LISTVIEW
#define ML_IPC_UNSKIN_LISTVIEW 0x0501 // pass the handle you got from ML_IPC_SKIN_LISTVIEW
#define ML_IPC_LISTVIEW_UPDATE 0x0502 // pass the handle you got from ML_IPC_SKIN_LISTVIEW
#define ML_IPC_LISTVIEW_DISABLEHSCROLL 0x0503 // pass the handle you got from ML_IPC_SKIN_LISTVIEW

#define ML_IPC_SKIN_COMBOBOX   0x0508 // pass the hwnd of your combobox to skin, returns a ahndle to use with ML_IPC_UNSKIN_COMBOBOX
#define ML_IPC_UNSKIN_COMBOBOX 0x0509 // pass the handle from ML_IPC_SKIN_COMBOBOX


#define ML_IPC_SKIN_WADLG_GETFUNC 0x0600 
    // 1: return int (*WADlg_getColor)(int idx); // pass this an index, returns a RGB value (passing 0 or > 3 returns NULL)
    // 2: return int (*WADlg_handleDialogMsgs)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); 
    // 3: return void (*WADlg_DrawChildWindowBorders)(HWND hwndDlg, int *tab, int tabsize); // each entry in tab would be the id | DCW_*
    // 32: return void (*childresize_init)(HWND hwndDlg, ChildWndResizeItem *list, int num);
    // 33: return void (*childresize_resize)(HWND hwndDlg, ChildWndResizeItem *list, int num);
    // 66: return (HFONT) font to use for dialog elements, if desired (0 otherwise)
   
// itemRecord type for use with ML_TYPE_ITEMRECORDLIST, as well as many other functions
typedef struct
{
  char *filename;
  char *title;
  char *album;
  char *artist;
  char *comment;
  char *genre;
  int year;
  int track;
  int length;
  char **extended_info;
  // currently defined extended columns (while they are stored internally as integers
  // they are passed using extended_info as strings):
  // use getRecordExtendedItem and setRecordExtendedItem to get/set.
  // for your own internal use, you can set other things, but the following values
  // are what we use at the moment. Note that setting other things will be ignored
  // by ML_IPC_DB*.
  // 
  //"RATING" file rating. can be 1-5, or 0 or empty for undefined
  //"PLAYCOUNT" number of file plays.
  //"LASTPLAY" last time played, in standard time_t format
  //"LASTUPD" last time updated in library, in standard time_t format
  //"FILETIME" last known file time of file, in standard time_t format
  //"FILESIZE" last known file size, in kilobytes.
  //"BITRATE" file bitrate, in kbps

} itemRecord;

typedef struct 
{
  itemRecord *Items;
  int Size;
  int Alloc;
} itemRecordList;


//
// all return 1 on success, -1 on error. or 0 if not supported, maybe?

// pass these a mlQueryStruct
// results should be zeroed out before running a query, but if you wish you can run multiple queries and 
// have it concatenate the results. tho it would be more efficient to just make one query that contains both,
// as running multiple queries might have duplicates etc.
// in general, though, if you need to treat "results" as if they are native, you should use
// copyRecordList to save a copy, then free the results using ML_IPC_DB_FREEQUERYRESULTS.
// if you need to keep an exact copy that you will only read (and will not modify), then you can
// use the one in the mlQueryStruct.

#define ML_IPC_DB_RUNQUERY 0x0700 
#define ML_IPC_DB_RUNQUERY_SEARCH 0x0701 // "query" should be interpreted as keyword search instead of query string
#define ML_IPC_DB_RUNQUERY_FILENAME 0x0702 // searches for one exact filename match of "query"
#define ML_IPC_DB_RUNQUERY_INDEX 0x703 // retrieves item #(int)query

#define ML_IPC_DB_FREEQUERYRESULTS 0x0705 // frees memory allocated by ML_IPC_RUNQUERY (empties results)
typedef struct 
{
  char *query;
  int max_results;      // can be 0 for unlimited
  itemRecordList results;
} mlQueryStruct;

// pass these an (itemRecord *) to add/update.
// note that any NULL fields in the itemRecord won't be updated, 
// and year, track, or length of -1 prevents updating as well.
#define ML_IPC_DB_UPDATEITEM 0x0706    // returns -2 if file not found in db
#define ML_IPC_DB_ADDORUPDATEITEM 0x0707
#define ML_IPC_DB_REMOVEITEM 0x0708 // pass a char * to the filename to remove. returns -2 if file not found in db.

#define ML_IPC_DB_SYNCDB 0x0709 // sync db if dirty flags are set. good to do after a batch of updates.


// these return 0 if unsupported, -1 if failed, 1 if succeeded

// pass a winampMediaLibraryPlugin *. Will not call plugin's init() func. 
// YOU MUST set winampMediaLibraryPlugin->hDllInstance to NULL, and version to MLHDR_VER
#define ML_IPC_ADD_PLUGIN 0x0750 
#define ML_IPC_REMOVE_PLUGIN 0x0751 // winampMediaLibraryPlugin * of plugin to remove. Will not call plugin's quit() func

// this gets sent to any child windows of the library windows, and then (if not
// handled) the library window itself

#define WM_ML_CHILDIPC WM_APP+0x800 // avoids conflicts with any windows controls
#define ML_CHILDIPC_DROPITEM 0x100         // lParam = 100, wParam = &mlDropItemStruct




// utility functions in ml_lib.cpp
void freeRecordList(itemRecordList *obj);
void emptyRecordList(itemRecordList *obj); // does not free Items
void freeRecord(itemRecord *p);

// if out has items in it copyRecordList will append "in" to "out".
void copyRecordList(itemRecordList *out, itemRecordList *in);
//copies a record
void copyRecord(itemRecord *out, itemRecord *in);

void allocRecordList(itemRecordList *obj, int newsize, int granularity
#ifdef __cplusplus
=1024
#endif
);

char *getRecordExtendedItem(itemRecord *item, char *name);
void setRecordExtendedItem(itemRecord *item, char *name, char *value);



#endif//_ML_H_