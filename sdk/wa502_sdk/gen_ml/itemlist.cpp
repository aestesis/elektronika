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
#include "itemlist.h"

C_ItemList::C_ItemList()
{
  m_size=0;
  m_list=NULL;
}
C_ItemList::~C_ItemList()
{
  if (m_list) ::free(m_list);
}

void *C_ItemList::Add(void *i)
{
  if (!m_list || !(m_size&31))
  {
    m_list=(void**)::realloc(m_list,sizeof(void*)*(m_size+32));
  }
  m_list[m_size++]=i;
  return i;
}

void C_ItemList::Set(int w, void *newv) { if (w >= 0 && w < m_size) m_list[w]=newv; }

void *C_ItemList::Get(int w) { if (w >= 0 && w < m_size) return m_list[w]; return NULL; }

void C_ItemList::Del(int idx)
{
  if (m_list && idx >= 0 && idx < m_size)
  {
    m_size--;
    if (idx != m_size) ::memcpy(m_list+idx,m_list+idx+1,sizeof(void *)*(m_size-idx));
    if (!(m_size&31)&&m_size) // resize down
    {
      m_list=(void**)::realloc(m_list,sizeof(void*)*m_size);
    }
  }
}

void *C_ItemList::Insert(void *i, int pos) 
{
  if (!m_list || !(m_size&31))
  {
    m_list=(void**)::realloc(m_list,sizeof(void*)*(m_size+32));
  }
  m_size++;
  for(int j=m_size-1;j>pos;j--) 
    m_list[j]=m_list[j-1];
  m_list[pos]=i;
  return i;
}
