#ifndef _PASSWORD_H_
#define _PASSWORD_H_

// #define RESELLER	// build reseller version	// web registration check reselller.php

#include <string.h>

typedef struct { char *guid; int numname; } Tregfile;

extern Tregfile regfile[];
extern int		nregfile;


__inline int	calcN(char *str)
{
	int		v=0;
	int		i;
	char	*s=str;
	for(i=0; i<32; i++)
	{
		v^=*s;
		v<<=1;
		if(*s)
			s++;
		else
			s=str;
	}
	return v;
}

#ifdef RESELLER

#define RESELLVER	"PRO "

bool VerifyPassword(char *name, char *passwd);

#else

#define RESELLVER	""

__inline bool VerifyPassword(char *name, char *passwd)
{
	int		i;
	if(strlen(name)<5)
		return 0;
		
	for(i=0; i<nregfile; i++)
	{
		if(!strcmp(passwd, regfile[i].guid))
		{
			if(regfile[i].numname)
			{
				if(calcN(name)==regfile[i].numname)
					return (bool)-1;
			}
			else
				return (bool)-1;
			return 0;
		}
	}
	return 0;
}

#endif




#endif