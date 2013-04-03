/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"export.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FILE.CPP					(c)	YoY'99						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<windows.h>
#include						<GL/gl.h>
#include						<GL/glu.h>
#include						<stdio.h>
#include						"bitmap.h"
#include						"oglbitmap.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL ACI						AoglBitmap::CI	= ACI("AgloutBack",		GUID(0xAE57E515,0x00000140), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AoglBitmap::AoglBitmap(int w, int h) : Anode("an AoglBitmap")
{
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof (PIXELFORMATDESCRIPTOR), // strcut size 
		1,                              // Version number
		PFD_DRAW_TO_BITMAP |    // Flags, draw to a window,
            PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED , // use OpenGL
		PFD_TYPE_RGBA,          // RGBA pixel values
        32,                     // 24-bit color
        0, 0, 0,                // RGB bits & shift sizes.
        0, 0, 0,                // Don't care about them
        0, 0,                   // No alpha buffer info
        0, 0, 0, 0, 0,          // No accumulation buffer
        32,                     // 32-bit depth buffer
        0,                      // No stencil buffer
        0,                      // No auxiliary buffers
        PFD_MAIN_PLANE,         // Layer type
        0,                      // Reserved (must be 0)
        0,                      // No layer mask
        0,                      // No visible mask
        0                       // No damage mask
    };
    int		nMyPixelFormatID;
//	DWORD	err;

	this->w=w;
	this->h=h;

	memset(&bih, 0, sizeof(bih));
	bih.biSize=sizeof(bih);
	bih.biPlanes=1;
	bih.biWidth=w;
	bih.biHeight=h;
	bih.biBitCount=32;
	bih.biCompression=BI_RGB;

	hbmp=CreateDIBSection(hdc, (BITMAPINFO *)&bih, DIB_RGB_COLORS, (void **)&body32, NULL, 0);
    hdc=CreateCompatibleDC(NULL);
	ohbmp=(HBITMAP)SelectObject(hdc, hbmp);
/*
    rdc=CreateCompatibleDC(NULL);
	rbmp=CreateCompatibleBitmap(rdc, w, h);
	orbmp=(HBITMAP)SelectObject(rdc, rbmp);
*/


    nMyPixelFormatID=ChoosePixelFormat(hdc, &pfd);
	isOK=nMyPixelFormatID?true:false;

    // catch errors here.
    // If nMyPixelFormat is zero, then there's
    // something wrong... most likely the window's
    // style bits are incorrect (in CreateWindow() )
    // or OpenGl isn't installed on this machine
	/*
	DescribePixelFormat(hdc, GetPixelFormat(hdc), sizeof(PIXELFORMATDESCRIPTOR), &pfd); 
	if(pfd.dwFlags&PFD_DRAW_TO_WINDOW)
		OutputDebugString("draw to window\n");
	if(pfd.dwFlags&PFD_DRAW_TO_BITMAP)
		OutputDebugString("draw to bitmap\n");
	if(pfd.dwFlags&PFD_SUPPORT_OPENGL)
		OutputDebugString("support open gl\n");
	if(pfd.dwFlags&PFD_GENERIC_ACCELERATED)
		OutputDebugString("accelerated render\n");
	if(pfd.dwFlags&PFD_GENERIC_FORMAT)
		OutputDebugString("software render\n");
	*/

	SetPixelFormat(hdc, nMyPixelFormatID, &pfd);
	glrc=wglCreateContext(hdc);

//	err=GetLastError();

	makeCurrent();
	glViewport(0, 0, w, h);
	releaseCurrent();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL AoglBitmap::~AoglBitmap()
{
	SelectObject(hdc, ohbmp);
	//SelectObject(rdc, orbmp);
	wglDeleteContext(glrc); 
	DeleteDC(hdc);
	DeleteObject(hbmp);
	//DeleteDC(rdc);
	//DeleteObject(rbmp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AoglBitmap::makeCurrent()
{
	wglMakeCurrent(hdc, glrc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AoglBitmap::releaseCurrent()
{
	wglMakeCurrent(NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ADLL void AoglBitmap::set2(Abitmap *b)
{
//	BitBlt(hdc, 0, 0, w, h, rdc, 0, 0, SRCCOPY);
	if(b->nbBits==32)
	{
		int		ww=mini(w, b->w);
		int		hh=mini(h, b->h);
		int		i;
		int		yd=0;
		int		ys=0;
		for(i=0; i<hh; i++)
		{
			memcpy(&b->body32[yd], &body32[ys], ww<<2);
			ys+=b->w;
			yd+=w;
		}		
	}
}

ADLL void AoglBitmap::texture(Abitmap *b, int level)
{
	texture(b, 0, 0, b->w, b->h, level);
}

ADLL void AoglBitmap::texture(Abitmap *b, int x, int y, int w, int h, int level)
{
	Abitmap	*tex;
	int		n=1;
	int		i;
	for(i=0; i<8; i++)
	{
		if((b->w<=n)&&(b->h<=n))
			break;
		n<<=1;
	}
	tex=new Abitmap(n, n);
	tex->set(0, 0, n, n, x, y, w, h, b, bitmapNORMAL, 0);

	{
		int		size=n*n;
		dword	*s=tex->body32;
		for(i=0; i<size; i++)
		{
			byte	a,r,g,b;
			colorRGBA(&r, &g, &b, &a, *s);
			*(s++)=color32(b, g, r, a);
		}
	}

	glTexImage2D(GL_TEXTURE_2D, level, 4, tex->w, tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->body32);

	delete(tex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
