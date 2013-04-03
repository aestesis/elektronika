#include <windows.h>


static char app_name[] = "WAV Writing Output Driver";
static char INI_FILE[MAX_PATH];
extern char config_waveoutdir[MAX_PATH];

static void _r_s(char *name,char *data, int mlen)
{
	char buf[2048];
	strcpy(buf,data);
	GetPrivateProfileString(app_name,name,buf,data,mlen,INI_FILE);
}
#define RS(x) (_r_s(#x,x,sizeof(x)))
#define WS(x) (WritePrivateProfileString(app_name,#x,x,INI_FILE))



static void config_init()
{
	char *p=INI_FILE;
	GetModuleFileName(NULL,INI_FILE,sizeof(INI_FILE));
	while (*p) p++;
	while (p >= INI_FILE && *p != '.') p--;
	strcpy(p+1,"ini");
}

void config_read()
{
	config_init();
	RS(config_waveoutdir);
}

void config_write()
{
	WS(config_waveoutdir);
}