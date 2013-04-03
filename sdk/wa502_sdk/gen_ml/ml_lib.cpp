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

#include <windows.h>
#include "ml.h"

void freeRecord(itemRecord *p) {
  free(p->title);
  free(p->artist);
  free(p->comment);
  free(p->album);
  free(p->genre);
  free(p->filename);
  if (p->extended_info)
  {
    int x=0;
    for (x = 0; p->extended_info[x]; x ++)
      free(p->extended_info[x]);
    free(p->extended_info);
  }
  memset(p,0,sizeof(itemRecord));
}

void freeRecordList(itemRecordList *obj)
{
  emptyRecordList(obj);
  free(obj->Items);
  obj->Items=0;
  obj->Alloc=obj->Size=0;
}

void emptyRecordList(itemRecordList *obj)
{
  itemRecord *p=obj->Items;
  while (obj->Size-->0)
  {
    freeRecord(p);
    p++;
  }
  obj->Size=0;
}

void allocRecordList(itemRecordList *obj, int newsize, int granularity)
{
  if (newsize < obj->Alloc || newsize < obj->Size) return;

  obj->Alloc=newsize+granularity;
  obj->Items=(itemRecord*)realloc(obj->Items,sizeof(itemRecord)*obj->Alloc);
  if (!obj->Items) obj->Alloc=0;
}

void copyRecord(itemRecord *out, itemRecord *in)
{
  int y;
#define COPYSTR(FOO) out->FOO = in->FOO ? strdup(in->FOO) : 0;
  COPYSTR(filename)
  COPYSTR(title)
  COPYSTR(album)
  COPYSTR(artist)
  COPYSTR(comment)
  COPYSTR(genre)
  out->year=in->year;
  out->track=in->track;
  out->length=in->length;
#undef COPYSTR
  out->extended_info=0;

  if (in->extended_info) for (y = 0; in->extended_info[y]; y ++)
  {
    char *p=in->extended_info[y];
    if (*p) setRecordExtendedItem(out,p,p+strlen(p)+1);
  }
}

void copyRecordList(itemRecordList *out, itemRecordList *in)
{
  int x;
  allocRecordList(out,out->Size+in->Size,0);
  if (!out->Items) return;
  for (x = 0; x < in->Size; x ++)
  {
    copyRecord(&out->Items[out->Size++],&in->Items[x]);
  }
}

char *getRecordExtendedItem(itemRecord *item, char *name)
{
  int x=0;
  if (item->extended_info) for (x = 0; item->extended_info[x]; x ++)
  {
    if (!stricmp(item->extended_info[x],name))
      return item->extended_info[x]+strlen(name)+1;
  }
  return NULL;
}

void setRecordExtendedItem(itemRecord *item, char *name, char *value)
{
  int x=0;
  if (item->extended_info) for (x = 0; item->extended_info[x]; x ++)
  {
    if (!stricmp(item->extended_info[x],name))
    {
      if (strlen(value)>strlen(item->extended_info[x])+strlen(name)+1)
      {
        free(item->extended_info[x]);
        item->extended_info[x]=(char*)malloc(strlen(name)+strlen(value)+2);
      }
      strcpy(item->extended_info[x],name);
      strcpy(item->extended_info[x]+strlen(name)+1,value);
      return;
    }
  }
  // x=number of valid items.
  item->extended_info=(char**)realloc(item->extended_info,sizeof(char*) * (x+2));
  if (item->extended_info)
  {
    item->extended_info[x]=(char*)malloc(strlen(name)+strlen(value)+2);
    strcpy(item->extended_info[x],name);
    strcpy(item->extended_info[x]+strlen(name)+1,value);

    item->extended_info[x+1]=0;
  }
}