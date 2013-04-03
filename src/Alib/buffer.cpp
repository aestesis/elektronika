/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BUFFER.CPP					(c)	YoY'02						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						"buffer.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						Abuffer::CI=ACI("Abuffer", GUID(0xAE57E515,0x00000410), &Afile::CI, 0, NULL); // countof(properties), properties);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abuffer::Abuffer(char *name, int bufsize) : Afile(name)
{
	section.enter(__FILE__,__LINE__);
	this->bufsize=bufsize;
	offset=size=0;
	buffer=(char *)malloc(bufsize);
	memset(buffer, 0, bufsize);
	section.leave();
}

ADLL Abuffer::~Abuffer()
{
	section.enter(__FILE__,__LINE__);
	free(buffer);
	buffer=NULL;
	offset=0;
	size=0;
	bufsize=0;
	section.leave();
}

ADLL int Abuffer::read(void *p, int s)
{
	int	r=0;
	section.enter(__FILE__,__LINE__);
	if(size>=s)
	{
		if(offset+s<=bufsize)
		{
			memcpy(p, &buffer[offset], s);
			size-=s;
			offset+=s;
			r=s;
		}
		else
		{
			sqword	n=bufsize-offset;
			memcpy(p, &buffer[offset], (int)n);
			memcpy(&((char *)p)[n], &buffer[0], (int)(s-n));
			size-=s;
			offset=s-n;
			r=s;
		}
	}
	section.leave();
	return r;
}

ADLL int Abuffer::peek(void *p, int s)
{
	int	r=0;
	section.enter(__FILE__,__LINE__);
	if(size>=s)
	{
		if(offset+s<=bufsize)
		{
			memcpy(p, &buffer[offset], s);
			r=s;
		}
		else
		{
			sqword	n=bufsize-offset;
			memcpy(p, &buffer[offset], (int)n);
			memcpy(&((char *)p)[n], &buffer[0], (int)(s-n));
			r=s;
		}
	}
	section.leave();
	return r;
}


ADLL int Abuffer::write(void *p, int s)
{
	int	r=0;
	section.enter(__FILE__,__LINE__);
	if(s<=bufsize-size)
	{
		sqword	w=(offset+size)%bufsize;
		if(w+s<=bufsize)
		{
			memcpy(&buffer[w], p, s);
			size+=s;
			r=s;
		}
		else
		{
			sqword	n=bufsize-w;
			memcpy(&buffer[w], p, (size_t)n);
			memcpy(&buffer[0], &((char *)p)[n], (size_t)(s-n));
			size+=s;
			r=s;
		}
	}
	section.leave();
	return r;
}

ADLL bool Abuffer::writeString(char *s)
{
	int					len=strlen(s);
	static const int	so=sizeof(len);
	bool				r=false;
	section.enter(__FILE__,__LINE__);
	if(size+len+so<=bufsize)
	{
		write(&len, sizeof(len));
		write(s, len);
		r=true;
	}
	section.leave();
	return r;
}

ADLL bool Abuffer::readString(char *s)
{
	bool	r=false;
	int		len;
	sqword	oldoffset;
	sqword	oldsize;
	section.enter(__FILE__,__LINE__);
	oldoffset=offset;
	oldsize=size;
	if(size>=4)
	{
		if(read(&len, sizeof(len))==sizeof(len))
		{
			if(read(s, len)==len)
			{
				s[len]=0;
				r=true;
			}
		}
	}
	if(!r)
	{
		offset=oldoffset;
		size=oldsize;
	}
	section.leave();
	return true;
}

ADLL bool Abuffer::readStringAlloc(char **s)
{
	bool	r=false;
	int		len;
	sqword	oldoffset;
	sqword	oldsize;
	section.enter(__FILE__,__LINE__);
	oldoffset=offset;
	oldsize=size;
	if(size>=4)
	{
		if(read(&len, sizeof(len))==sizeof(len))
		{
			char	*str=(char *)malloc(len+1);
			if(read(str, len)==len)
			{
				str[len]=0;
				*s=str;
				r=true;
			}
			else
			{
				free(str);
			}
		}
	}
	if(!r)
	{
		offset=oldoffset;
		size=oldsize;
	}
	section.leave();
	return true;
}

ADLL sqword Abuffer::getSize()
{
	section.enter(__FILE__,__LINE__);
	sqword	n=size;
	section.leave();
	return n;
}

ADLL bool Abuffer::seek(sqword n)
{
	size+=offset-n;
	offset=n%bufsize;
	return true;
}

ADLL void Abuffer::clear()
{
	section.enter(__FILE__,__LINE__);
	offset=0;
	size=0;
	section.leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
