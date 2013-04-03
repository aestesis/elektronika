#include "main.h"
#include "gaystring.h"

GayString::GayString(const char *initial)
{
  m_buf=NULL;
  m_alloc=0;
  if (initial) Set(initial);
}

GayString::~GayString()
{
  free(m_buf);
}

void GayString::Set(const char *value)
{
  Grow(strlen(value)+1);
  strcpy(m_buf,value);
}
char *GayString::Get() { return m_buf?m_buf:(char*)""; }

void GayString::Append(const char *append)
{
  int oldsize=m_buf ? strlen(m_buf) : 0;
  Grow(oldsize + strlen(append) + 1);
  strcpy(m_buf+oldsize,append);
}


void GayString::Grow(int newsize)
{
  if (m_alloc < newsize)
  {
    m_alloc=newsize+512;
    m_buf=(char*)realloc(m_buf,m_alloc);
  }
}
void GayString::Compact()
{
  if (m_buf)
  {
    m_alloc=strlen(m_buf)+1;
    m_buf=(char*)realloc(m_buf,m_alloc);
  }
}
