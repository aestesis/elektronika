/*
 * PROJECT:             FLI/FLC animation play routines V2.4
 * 
 * AUTHOR:              Ivo Bosticky 
 *
 * PROJECT STARTED ON:  17-7-1994
 *
 * LATEST UPDATE:       15-11-95
 *
 * COMPILE NOTES:       Compile with any options, remember that the fli/flc
 *                      file structures are byte packed, if you decide to 
 *                      use them elsewhere, use the #PRAGMA PACK(1); option
 *                      before the struct and #PRAGMA PACK(); after it. I 
 *          			tested wflilib with both dos4gw and pmode, and used
 *          			it in many of my applications, so it seems to be
 *			            reliable.
 *
 * UPDATES:
 *      8-10-94 Extended to play flc files, only support flcs that have 
 *              sizex*sizey < 64000
 *      22-1-95 Added access to the color palette from external modules
 *      20-3-95 Modified for use in 32-bit flat memory compiler (WATCOM)
 *              Set the colors using VGA ports, used to be done by BIOS (slow)
 *      23-3-95 Finilised modifications for 32-bit flat memory, this will 
 *              allow the playing of flc's of any size
 *      24-4-95 Fixed a frame playing bug, for the FLI_RAM option only. It used
 *              to play all the animation frames and then wrap to the first
 *              frame. Now it plays all the frames plust the wrap-around
 *              frame and wraps to the second frame. This might make some
 *              fli/flcs play faster. The DISK option is still bugged but the
 *              animation looks the same anyway.
 *      29-4-95 Modified the code so that you don't have to compile flilib
 *              with the -zp1 option anymore.
 *     15-11-95 Fixed the DISK play bug.
 *  
 *
 * ABOUT:	For more on the wflilib, see the header file.
 */
 

#include <io.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include "wflilib.h"


// fli file header, have to use byte structure packing


#pragma pack(1)

struct fli_header
	{
	unsigned long size;
	unsigned short int magic;
	unsigned short int frames;
	unsigned short int width;
	unsigned short int height;
	unsigned short int depth;
	unsigned short int flags;
	unsigned short int speed;
	unsigned long    next;
	unsigned long    frit;
	char    reserved[102];
	};

// flc file header
struct flc_header
	{
	unsigned long size;
	unsigned short int magic;
	unsigned short int frames;
	unsigned short int width;
	unsigned short int height;
	unsigned short int depth;
	unsigned short int flags;
	unsigned long speed;
	char    reserved0[2];
	long    created;
	char    creator[4];
	long    updated;
	char    updater[4];
	short int     aspectx;
	short int     aspecty;
	char    reserved1[38];
	long    oframe1;
	long    oframe2;
	char    reserved[40];
	};

struct  frame_header
	{
	unsigned long   size;
	unsigned short int magic;
	unsigned short int     chunks;
	char    expand[8];
	};

struct  chunk_header
	{
	unsigned long size;
	unsigned short int     type;
	};

#pragma pack() //use the default packing method

#define fli_magic       0xAF11
#define flc_magic       0xaf12
#define prefix_id       0xf100
#define frame_magic     0xF1Fa

#define FLI_COLOR256    4
#define FLI_SS2         7
#define FLI_COLOR       11
#define FLI_LC          12
#define FLI_BLACK       13
#define FLI_BRUN        15
#define FLI_COPY        16
#define FLI_PSTAMP      18

fli *   load_fli(char *filename,fli_source playtype) {

	short int handle;
	fli *ptr; 
	char *ptr1;
	struct fli_header header;
	//frame_header pfix;
	flc_header *flc;

	// load the 128 byte fli/flc header
	if((handle = _open(filename,O_RDONLY | O_BINARY))==-1) 
		return NULL;
	_read(handle,&header,sizeof(fli_header));
	flc=(flc_header *)&header;
	switch (header.magic) {
		case fli_magic:
			break;
		case flc_magic:
			_lseek(handle,flc->oframe1,SEEK_SET);
			break;
		default       : 
			_close(handle);
			return (NULL);
		}
	if(playtype!=FLI_DISK) 
	{
	// try to allocate ram for the entire fli
		if((ptr=(fli *)malloc(header.size))==NULL)
		{
			if(playtype==FLI_RAM) 
			{
				_close(handle);
				return NULL;
			}
			playtype=FLI_DISK;
		} 
		else 
		{
			memset(ptr, 0, sizeof(header.size));
			playtype=FLI_RAM;
		}
	}

	// try to allocate ram just for the fli descriptor
	if(playtype==FLI_DISK)
		if((ptr=(fli *)malloc(sizeof(fli)))==NULL) 
		{
			_close(handle);
			return NULL;
		}
		else
			memset(ptr, 0, sizeof(fli));

	// initialise the fli descriptor
	if (header.magic==fli_magic) {
		ptr->frames=header.frames;
		ptr->width=header.width;
		ptr->height=header.height;
		ptr->speed=header.speed;
	} else {
		ptr->frames=flc->frames;
		ptr->width=flc->width;
		ptr->height=flc->height;
		ptr->speed=(unsigned short)(flc->speed*7/100);
		if (ptr->speed<1)
			ptr->speed=1;
	}
	ptr->palette=(unsigned char *)malloc(256*4);
	ptr->current=0;
	ptr->crntoff=(long)sizeof(fli);

	if (playtype==FLI_RAM)
		ptr->handle=0;
	else 
		ptr->handle=handle;

	if (playtype==FLI_RAM) {
	// load the entire fli into ram
		ptr1 = (char *)ptr + sizeof(fli);
		while(_read(handle, ptr1, 32700) == 32700) 
			ptr1 += 32700;
		_close(handle);
		}

	// return the pointer to the fli
	return (fli *)ptr;
	}

void    flicolor(fli *fliptr, char *data)
{
       short int i, j, k, l, change;
	unsigned char current = 0;
	unsigned char *fli_pal=fliptr->palette;
	i = *data;
	data += 2;
	{
		for (j=0; j<i; j++) 
		{
			current += *data++;
			change = *data++;
			if (!change)
				change = 256;
			for (k = 0; k < change ; k++)
			{
				int	c=((int)current+k)*4;

				for(l=0; l<3; l++)
				{
					int	v=*(data++);
					fli_pal[c+(2-l)] = v<<2;
				}
				fli_pal[c + 3] = 255;
			}
		}
	} 
	fliptr->paletteChange=true;
}

void    flicolor256(fli *fliptr, char *data)
{
    short int i, j, k, l, change;
	unsigned char current = 0;
	unsigned char *fli_pal=fliptr->palette;
	i = *data;
	data += 2;
	{
		for (j=0; j<i; j++) 
		{
			current += *data++;
			change = *data++;
			if (!change)
				change = 256;
			for (k = 0; k < change ; k++)
			{
				int	c=((int)current+k)*4;

				for(l=0; l<3; l++)
				{
					int	v=*(data++);
					fli_pal[c+(2-l)] = v;
				}
				fli_pal[c + 3] = 255;
			}
		}
	} 
	fliptr->paletteChange=true;
}


void    flicolor256to64(char *data) {
// this just converts from 0 to 255 range to 0 to 64 color range for each
// r,g,b color
	short int i, j, k, change;
	i = *data;
	data += 2;
	for (j = 0; j < i; j++) {
		data++;
		change = *data++;
		if (!change)
			change=256;
		for (k = 0; k < change * 3; k++)
			*(data++) /= 4;
	}
}

void flibrun(char *data, char *dest, short int xsize, short int ysize) 
{
	char	*ecran=dest;
	short int i, j, k;
	char ppl, dt, *dest1;
	signed char sc;
	for (i = 0; i < ysize; i++) {
		ppl = *(data++);
		dest1 = dest;
		for (j = 0; j < ppl; j++) {
			sc = *(data++);
			if (sc < 0)
				for(k = 0; k < -sc; k++)
					*(dest++) = *(data++);
			else {  
				dt = *(data++);
				for(k = 0; k < sc; k++)
					*(dest++) = dt;
			}
		}
		dest = dest1 + xsize;
	}
}

void flilc(char *data,char *dest,short int xsize,char *finaldata) 
{
	short int j,k,l,m;
	char dt,*dest1;
	char	*ecran=dest;
	signed char sc;
	unsigned char ppl;
	do {
	    j = *(short int *)(data);
	    data += 2;
	    dest += j * xsize;
	    m = *(short int *)(data);
	    data += 2;
	    for (l = 0; l < m; l++) {
		ppl = *(data++);
		dest1 = dest;
		for (j = 0; j < ppl; j++) {
			dest += (unsigned char)*(data++);
			sc = *(data++);
			if (sc > 0) 
				for(k = 0; k < sc; k++)
					*(dest++) = *(data++);
			if (sc < 0) {  
				dt = *(data++);
				for(k = 0; k < -sc; k++)
					*(dest++) = dt;
				}
			if (data >= finaldata)
				return;
			}
		dest = dest1 + xsize;
		}
	}while(data < finaldata);
}

#define LINE_SKIP 0xc000        

void fliss2(char *data,char *dest,short int xsize) {
	short int k, cl, l, p, ppl, ppl1, dt;	// , j, m
	char *dest1;
	signed char sc;

	cl = *(short int *)data;
	data += 2;
	for (l = 0; l < cl; l++) {
		ppl = *(short int *)data;
		data += 2;
		while ((ppl & LINE_SKIP) == LINE_SKIP) {
			dest -= xsize * ppl;
			ppl = *(short int *)data;
			data += 2;
		}
		dest1 = dest;
		ppl1 = ppl;
		if (ppl < 0) {
			ppl1 = *(short int *)data;
			data += 2;
		}
		for (p = 0; p < ppl1; p++) {
			dest += *(data++);
			sc = *(data++);
			if (sc > 0)
				for(k = 0; k < sc; k++) {
					*(short int *)dest =
						*(short int *)data;
					dest += 2;
					data += 2;
				}
			if (sc < 0) {
				dt = *(short int *)data;
				data += 2;
				for(k = 0; k < -sc; k++) {
					*(short int *)dest = dt;
					dest += 2;
				}
			}
		}
		if (ppl < 0)
			*dest = (char)ppl;	// renan@aestesis.org change // (cast short to char)
		dest = dest1 + xsize;
	}
}

short int     play_frame(fli *fliptr,char *dest,short int sizex){
	frame_header *frame,frame1;
	chunk_header *chunk, *chunk1;
	short int     i;

	fliptr->paletteChange=false;

	if (fliptr->handle==0) {
	// if playing from ram, setup pointers for the next frame
		frame=(frame_header *)((char *)fliptr+fliptr->crntoff);
		chunk=(chunk_header *)((char *)frame+sizeof(frame_header));
		fliptr->current++;
	}
	else {
	// if playing from disk, read the next frame
		_read(fliptr->handle,&frame1,sizeof(frame_header));
		frame=&frame1;
		if (frame1.chunks>0) {
			// if there are chunks to load, load em
			if((chunk=(chunk_header *)malloc(frame1.size-sizeof(frame_header)))==NULL)
                return 1;
            chunk1 = chunk;
			_read(fliptr->handle,chunk,frame1.size-sizeof(frame_header));
			}
		fliptr->current++;
		if (fliptr->current==fliptr->frames) {
			fliptr->current=0;
			if (_lseek(fliptr->handle,sizeof(fli_header),SEEK_SET)==-1) return 1;
			}
	}

	// play the current frame into the destination buffer
	if (frame->magic == frame_magic)
	   for (i=0; i<frame->chunks;i++) {
		switch(chunk->type) {
		case FLI_COLOR256:
			// this fixes up the color pal and if playing from ram
			// this is done only once
			//flicolor256to64((char *)chunk+sizeof(chunk_header));
			//chunk->type = FLI_COLOR;
			flicolor256(fliptr, (char *)chunk+sizeof(chunk_header));
			break;
		case FLI_SS2:
			fliss2((char *)chunk+sizeof(chunk_header),dest,sizex);
			break;
		case FLI_COLOR:
			flicolor(fliptr, (char *)chunk+sizeof(chunk_header));
			break;
		case FLI_LC:
			flilc((char *)chunk+sizeof(chunk_header),dest,sizex,(char *)chunk+chunk->size-1);
			break;
		case FLI_BLACK:
			{
				int	a=5;
				a++;
			}
			break; // have to make this one day
		case FLI_BRUN:
			flibrun((char *)chunk+sizeof(chunk_header),dest,sizex,fliptr->height);
			break;
		case FLI_COPY:
			{
				int	a=5;
				a++;
			}
			break; // have to make this one day
		case FLI_PSTAMP:
			break; // ignore the stamps
		default:
			// undefined chunk type, ignore it
			break;
		}
		chunk = (chunk_header *)((char *)chunk + chunk->size);
	   }

	// if playing from the disk, free the previous frame data
	if (fliptr->handle!=0) {
		if (frame1.chunks!=0) 
			free(chunk1);
	} else {
		if (fliptr->current > fliptr->frames) {
			fliptr->current = 1;
			fliptr->crntoff = (long)sizeof(fli);
			frame=(frame_header *)((char *)fliptr+fliptr->crntoff);
		} 
		fliptr->crntoff+=frame->size;
	}
	return 0;
}


void    close_fli(fli *fliptr) {
	if (fliptr->handle!=0) 
		_close(fliptr->handle);
	if(fliptr->palette)
		free(fliptr->palette);
	free(fliptr);
	}

