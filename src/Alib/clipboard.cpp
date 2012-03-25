/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLIPBOARD.CPP				(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						"clipboard.h"
#include						"bitmap.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL char * Aclipboard::getText()
{
	char *text=NULL;
	if(OpenClipboard(NULL)) 
	{
		HANDLE	hData=GetClipboardData(CF_TEXT);
		if(hData)
		{
			char	*buffer=(char*)GlobalLock(hData);
			if(buffer)
			{
				text=strdup(buffer);
				GlobalUnlock(hData);
			}
		}
		CloseClipboard();
	}
	return text;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aclipboard::setText(char *s)
{
	bool	b=false;
	if(s&&OpenClipboard(NULL))
	{
		HGLOBAL	clipbuffer;
		char	*buffer;
		EmptyClipboard();
		clipbuffer=GlobalAlloc(GMEM_DDESHARE, strlen(s)+1);
		if(clipbuffer)
		{
			buffer=(char*)GlobalLock(clipbuffer);
			if(buffer)
			{
				strcpy(buffer, LPCSTR(s));
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_TEXT, clipbuffer);
			}
		}
		CloseClipboard();
	}
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aclipboard::setText(char *s, int f, int l)
{
	bool	b=false;
	if(s&&OpenClipboard(NULL))
	{
		HGLOBAL	clipbuffer;
		char	*buffer;
		EmptyClipboard();
		clipbuffer=GlobalAlloc(GMEM_DDESHARE, (l-f)+1);
		if(clipbuffer)
		{
			buffer=(char*)GlobalLock(clipbuffer);
			if(buffer)
			{
				strncpy(buffer, LPCSTR(s+f), l-f);
				((char *)buffer)[l-f]=0;
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_TEXT, clipbuffer);
			}
		}
		CloseClipboard();
	}
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL Abitmap * Aclipboard::getBitmap()
{
	Abitmap	*obmp=NULL;
	if(OpenClipboard(NULL))
	{
		HANDLE	h=GetClipboardData(CF_DIB);
		if(h)
		{
			void	*p=GlobalLock(h); 
			if(p)
			{
				BITMAPINFOHEADER	*pbmi=(BITMAPINFOHEADER *)p;
				if(pbmi->biCompression==BI_BITFIELDS)
				{
					dword				*body=(dword *)(pbmi+1);
					obmp=new Abitmap(pbmi->biWidth, pbmi->biHeight, (pbmi->biBitCount==8)?8:32);
					switch(pbmi->biBitCount)
					{
						case 8:
						{
							int		n=pbmi->biWidth*pbmi->biHeight;
							int		npal=pbmi->biClrUsed?pbmi->biClrUsed:256;
							dword	*body2=body+npal;
							byte	*pal=(byte *)body;
							memcpy(obmp->palette, pal, sizeof(dword)*npal);
							memcpy(obmp->body, body2, n);
						}
						break;

						case 16:
						{
						}
						break;

						case 24:
						{
							int		n=pbmi->biWidth*pbmi->biHeight;
							byte	*d=(byte *)obmp->body32;
							byte	*s=(byte *)body;
							while(--n)
							{
								*(d++)=*(s++);
								*(d++)=*(s++);
								*(d++)=*(s++);
								d++;
							}
						}
						break;

						case 32:
						{
							int		n=pbmi->biWidth*pbmi->biHeight;
							memcpy(obmp->body32, body, sizeof(dword)*n);
						}
						break;
					}
				}
				GlobalUnlock(h);
			}
		}
		CloseClipboard();
	}
	return obmp;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL bool Aclipboard::setBitmap(Abitmap *b)
{
	bool	btest=false;
	if(b&&OpenClipboard(NULL))
	{
		HGLOBAL	clipbuffer;
		char	*buffer;
		EmptyClipboard();
		clipbuffer=GlobalAlloc(GMEM_DDESHARE, ((b->nbBits==8)?(256*4):0)+b->bodysize+sizeof(BITMAPINFOHEADER));
		if(clipbuffer)
		{
			buffer=(char*)GlobalLock(clipbuffer);
			if(buffer)
			{
				switch(b->nbBits)
				{
					case 8:
					{
						BITMAPINFOHEADER	*pbmi=(BITMAPINFOHEADER *)buffer;
						dword				*body=(dword *)(pbmi+1);
						memset(pbmi, 0, sizeof(BITMAPINFOHEADER));
						pbmi->biSize=sizeof(BITMAPINFOHEADER);
						pbmi->biCompression=BI_BITFIELDS;
						pbmi->biPlanes=1;
						pbmi->biWidth=b->w;
						pbmi->biHeight=b->h;
						pbmi->biBitCount=8;
						memcpy(body, b->palette, sizeof(dword)*256);
						memcpy(body+256, b->body, b->bodysize);
						btest=true;
					}
					break;

					case 32:
					{
						BITMAPINFOHEADER	*pbmi=(BITMAPINFOHEADER *)buffer;
						dword				*body=(dword *)(pbmi+1);
						memset(pbmi, 0, sizeof(BITMAPINFOHEADER));
						pbmi->biSize=sizeof(BITMAPINFOHEADER);
						pbmi->biCompression=BI_BITFIELDS;
						pbmi->biPlanes=1;
						pbmi->biWidth=b->w;
						pbmi->biHeight=b->h;
						pbmi->biBitCount=32;
						memcpy(body, b->body, b->bodysize);
						btest=true;
					}
					break;
				}
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_DIB, clipbuffer);
			}
		}
		CloseClipboard();
	}
	return btest;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
