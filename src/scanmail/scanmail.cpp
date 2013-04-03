// scanmail.cpp : Defines the entry point for the console application.
//

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<io.h>
#include	"stdafx.h"


#define PATH	"c:\\_data\\registration\\aestesis"


int main(int argc, char* argv[])
{
	FILE					*fo=fopen("d:\\aestesis register email.txt", "w");
	struct _finddata_t		fd;
	char					path[_MAX_PATH];
	strcpy(path, PATH);
	strcat(path, "\\*.txt");
	long					h=_findfirst(path, &fd);
	if(h!=-1)
	{
		int		r=0;
		while(r!=-1)
		{
			char		str[256];
			sprintf(str, "%s\\%s", PATH, fd.name);
			{
				FILE	*fi=fopen(str, "r");
				if(fi)
				{
					char	txt[1024];
					while(fgets(txt, 1023, fi))
					{
						strlwr(txt);
						if(!strncmp("customer e-mail:   ", txt, 19))
						{
							fprintf(fo, "%s", txt+19);
							printf("%s", txt+19);
						}
						else if(!strncmp("email du client     : ", txt, 22))
						{
							fprintf(fo, "%s", txt+22);
							printf("%s", txt+22);
						}
					}
					fclose(fi);
				}
			}
			r=_findnext(h, &fd);
		}
		_findclose(h);
	}
	fclose(fo);
	return 0;
}

