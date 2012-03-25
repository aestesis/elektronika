/*
 * PROJECT:     FLI/FLC animation play routines V2.4
 *
 * AUTHOR:      Ivo Bosticky 
 *
 * ABOUT:	Well when I started working on this I decided to create my
 *		own fli header to reference the individual fli/flc files
 *		that are being played. The reason being that the original 
 *		fli files had a huge and ugly file header i didn't feel like
 *		keeping in memory, later on this made it easier for flcs.
 *		So the header is the link you have to the fli/flc file once
 *		it is loaded, it has all the info about the fli file you
 *		need. The various structures and functions that wflilib 
 *		supports are described below.
 */

#ifndef flilib_h
#define flilib_h

struct  fli
	{
	unsigned short int		frames;
	unsigned short int		width;
	unsigned short int		height;
	unsigned short int		speed;
	unsigned short int		current;
	unsigned long			crntoff;
	unsigned short int		handle;
	unsigned char			*palette;
	bool					paletteChange;
	};

/*	description of a fli structure
	Any of these variables can be used directly, by fliptr->variable
	where fliptr is the pointer returned by the load_fli function.
	To go to the first frame execute the following code:
		fliptr->current=0;
		fliptr->crntoff=(long)sizeof(fli);
	(this only works if all of the fli was loaded into the ram)

	The data can only be used if the fli was loaded with the FLI_RAM 
    or possibly	the FLI_AUTO option.

	Handle is equal to 0 if all of the fli was loaded into the ram.

byte offset     size    name    meaning
0               2       frames  Number of frames in fly, a maximum of 4000
2               2       width   Screen width (320)
4               2       height  Screen height (200)
6               2       speed   Number of video ticks between frames
8               2       current Current frame (=0 initilly)
10              4       crntoff offset from begining to current frame (=14 initially)
14              2       handle  handle of the file if playing from disk
16              ?       data    data for the frames (only if handle>1)

*/

enum fli_source { FLI_RAM, FLI_DISK, FLI_AUTO };
// FLI_RAM forces load_fli to load the entire fli to memory and play it from ram
// FLI_DISK forces play from disk, (uses from 128 bytes up to just over 64Kb of ram)
// FLI_AUTO first tries to load file into memory then tries the disk option

fli * load_fli(char *filename,fli_source playtype);
// returns pointer to the fli in memory
// or returns NULL if it didn't find the file or out of memory
// the file can be either a fli or flc

short int play_frame(fli *fliptr,char *dest,short int sizex);
// plays a single frame and if it is the last frame, automatically
// sets the current and crntoff to the first frame
// fliptr is the pointer that load_fli returns
// dest is the memory buffer to play into, can be the screen of mode 0x13
// sizex is the width of the memory buffer (320)

void close_fli(fli *fliptr);
// closes the handle used if playing from DISK
// frees up all the memory used by the fli


#endif //flilib_H
