/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"main.h"
#include						"resource.h"
#include						<math.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// elektronika SDK effect sample
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// if you want to use MFC/AFX
// add the MFC/AFX includes
// select "MFC  shared dll" in general tab of the project settings
// and remove the _USRDLL define in the project settings
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACI								AzoolInfo::CI	= ACI("AzoolInfo",	GUID(0x50000000,0x00000720), &AeffectInfo::CI, 0, NULL);
ACI								Azool::CI		= ACI("Azool",		GUID(0x50000000,0x00000721), &Aeffect::CI, 0, NULL);
ACI								AzoolFront::CI	= ACI("AzoolFront",	GUID(0x50000000,0x00000722), &AeffectFront::CI, 0, NULL);
ACI								AzoolBack::CI	= ACI("AzoolBack",	GUID(0x50000000,0x00000723), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
static Aresource				resdll=Aresource("zool", GetModuleHandle("zool.dll"));

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void init()
{
	if(count==0)
	{
		// make globals inits as somes tables
	}
	count++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void end()
{
	count--;
	if(count==0)
	{
		// free globals inits
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Azool::Azool(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init();	// global init

	front=new AzoolFront(qiid, "zool front", this, 50);
	front->setTooltips("zool 3d sample");
	back=new AzoolBack(qiid, "zool back", this, 50);
	back->setTooltips("zool 3d sample");

	w=getVideoWidth();
	h=getVideoHeight();

	ob=new AoglBitmap(w, h);
	initGL();
	ttime=0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Azool::~Azool()
{
	delete(ob);
	end();	// global free
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Azool::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Azool::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Azool::settings(bool emergency)
{
	if((w!=getVideoWidth())||(h!=getVideoHeight()))
	{
		delete(ob);
		w=getVideoWidth();
		h=getVideoHeight();
		ob=new AoglBitmap(w, h);
		initGL();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Azool::initGL()
{
	GLfloat			LightAmbient[]= { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat			LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat			LightPosition[]= { 0.0f, 100.0f, -2.0f, 1.0f };
	Abitmap			*tex=new Abitmap("d:\\elektro_logo.jpg");

	ob->makeCurrent();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, (GLdouble)w/(GLdouble)h, 1.0, 100.0);
/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
*/
	glGenTextures(2, texture);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	ob->texture(tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	ob->texture(tex);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glEnable(GL_TEXTURE_2D);						// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);						// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);			// Black Background
	glClearDepth(1.0f);								// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);						// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);							// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	

	ob->releaseCurrent();

	delete(tex);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Azool::action(double time, double dtime, double beat, double dbeat)
{
	static float	dx[]={1.f, -1.f, 1.f, -1.f};
	static float	dy[]={1.f, -1.f, -1.f, 1.f};
	static float	dz[]={1.f, 1.f, -1.f, -1.f};
	float			color0[]={1.f, 1.f, 1.f, 1.f};
	float			color1[]={1.f, 1.f, 0.f, 1.f};
	float			color2[]={0.f, 1.f, 1.f, 1.f};
	float			color3[]={1.f, 0.f, 1.f, 1.f};
	AzoolFront		*front=(AzoolFront *)this->front;
	AzoolBack		*back=(AzoolBack *)this->back;
	Avideo			*out=back->out;
	Avideo			*in1=back->in1;
	Avideo			*in2=back->in2;
	GLfloat			LightPosition[]= { 0.0f, 100.0f, -2.0f, 1.0f };
	float			speed=front->speed->get();
	float			xypow=front->xypow->get()*2.f;
	float			zpow=front->zpow->get();
	int				nbserp=(int)(front->nbserp->get()*9.f)+1;
	int				nbcube=(int)(front->nbcube->get()*9.f)+1;
	bool			ccbuf=front->ccbuf->get();
	bool			czbuf=front->czbuf->get();
	dword			col0=front->col0->get();
	dword			col1=front->col1->get();
	float			r0,g0,b0;
	float			r1,g1,b1;
	float			dr,dg,db;
	int				mirror=front->mirror->get()+1;
	int				src1=front->src1->get();
	int				src2=front->src2->get();
	if(mirror==3)
		mirror=4;

	{
		byte	r,g,b;
		float	dn=1.f/(float)nbcube;
		colorRGB(&r, &g, &b, col0);
		r0=(float)r/255.f;
		g0=(float)g/255.f;
		b0=(float)b/255.f;
		colorRGB(&r, &g, &b, col1);
		r1=(float)r/255.f;
		g1=(float)g/255.f;
		b1=(float)b/255.f;
		dr=(r1-r0)*dn;
		dg=(g1-g0)*dn;
		db=(b1-b0)*dn;
	}

	ob->makeCurrent();

	if((src1==0)||(src2==0))
	{
		in1->enter(__FILE__,__LINE__);
		{
			Abitmap	*b=in1->getBitmap();
			if(b)
			{
				glBindTexture(GL_TEXTURE_2D, texture[0]);
				ob->texture(b, (w-h)>>1, 0, h, h);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			}
		}
		in1->leave();
	}

	if((src1==1)||(src2==1))
	{
		in2->enter(__FILE__,__LINE__);
		{
			Abitmap	*b=in2->getBitmap();
			if(b)
			{
				glBindTexture(GL_TEXTURE_2D, texture[1]);
				ob->texture(b, (w-h)>>1, 0, h, h);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			}
		}
		in2->leave();
	}

/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	LightPosition[0]=(float)sin(time*0.003f)*50.f;
	LightPosition[1]=(float)sin(time*0.003515f)*50.f;
	LightPosition[2]=(float)sin(time*0.0035621f)*50.f;
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
*/

	if(!ccbuf)
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	if(czbuf)
	{
		glColor3f(r1, g1, b1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, -99.f);
		glBindTexture(GL_TEXTURE_2D, texture[src2]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-35.5f, -35.5f,  0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 35.5f, -35.5f,  0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 35.5f,  35.5f,  0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-35.5f,  35.5f,  0.0f);
		glEnd();
	}

	ttime+=dtime*(speed*5.f+0.1f);

	int	k;
	for(k=0; k<mirror; k++)
	{
		int	j;
		for(j=0; j<nbserp; j++)
		{
			double	v=ttime+(double)j*10000.f;
			int		i;
			float	r=r0;
			float	g=g0;
			float	b=b0;

			for(i=0; i<nbcube; i++)
			{
				GLfloat	rtri=(float)(v-i*50)/10.f+51.5616508515f;
				GLfloat	rquad=-(float)(v-i*40)/10.5f+815.51515511f;
				GLfloat	toto=(float)(v-i*50)/10.84451515f+51.515774742f;

				glColor3f(r, g, b);
				r+=dr;
				g+=dg;
				b+=db;

				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glTranslatef((float)sin(rtri*0.0144f)*(float)sin(toto*0.0144f)*5.f*dx[k]*(xypow+0.01f), (float)sin(rtri*0.0195451f)*(float)sin(toto*0.0195451f)*5.f*dy[k]*(xypow+0.01f), -5.f+-40.0f*(zpow+0.01f)+(float)sin(rtri*0.01f)*(float)sin(toto*0.01f)*40.f*(zpow+0.01f));				// Move Into The Screen And Left

				glRotatef(rtri,0.0f,dy[k],0.0f);
				glRotatef(rquad,0.0f,0.0f,dx[k]);
				glRotatef(rtri*1.6221f,dx[k],0.0f,0.0f);

				glBindTexture(GL_TEXTURE_2D, texture[src1]);
				glBegin(GL_QUADS);
					// Front Face
					glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
					glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
					glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// Top Right Of The Texture and Quad
					glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// Top Left Of The Texture and Quad
					// Back Face
					glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// Bottom Right Of The Texture and Quad
					glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// Top Right Of The Texture and Quad
					glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// Top Left Of The Texture and Quad
					glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// Bottom Left Of The Texture and Quad
					// Top Face
					glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// Top Left Of The Texture and Quad
					glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
					glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
					glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// Top Right Of The Texture and Quad
					// Bottom Face
					glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// Top Right Of The Texture and Quad
					glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// Top Left Of The Texture and Quad
					glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
					glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
					// Right face
					glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);	// Bottom Right Of The Texture and Quad
					glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);	// Top Right Of The Texture and Quad
					glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);	// Top Left Of The Texture and Quad
					glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);	// Bottom Left Of The Texture and Quad
					// Left Face
					glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);	// Bottom Left Of The Texture and Quad
					glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);	// Bottom Right Of The Texture and Quad
					glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);	// Top Right Of The Texture and Quad
					glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);	// Top Left Of The Texture and Quad
				glEnd();
			}
		}
	}

	glFlush();
	ob->releaseCurrent();

	out->enter(__FILE__,__LINE__);
	{
		Abitmap *b=out->getBitmap();
		if(b)
			ob->set2(b);
	}
	out->leave();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AzoolFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AzoolFront::AzoolFront(QIID qiid, char *name, Azool *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_MOONSTER_FRONT), "PNG");
	back=new Abitmap(&o);

	speed=new Apaddle(MKQIID(qiid, 0x87ac20fe3b405800), "speed", this, 121, 23, 7, 0xffffffff);
	speed->setTooltips("speed");
	speed->set(0.f);
	speed->show(TRUE);

	xypow=new Apaddle(MKQIID(qiid, 0x6d82ae1dce2fe0e0), "xy movement", this, 156, 14, 5, 0xffffffff);
	xypow->setTooltips("xy movement");
	xypow->set(0.f);
	xypow->show(TRUE);

	zpow=new Apaddle(MKQIID(qiid, 0x125d74e59c1b7900), "z movement", this, 183, 14, 5, 0xffffffff);
	zpow->setTooltips("z movement");
	zpow->set(0.f);
	zpow->show(TRUE);

	nbserp=new Apaddle(MKQIID(qiid, 0x142b7b089e89a341), "nb snake", this, 155, 36, 4, 0xff000000);
	nbserp->setTooltips("nb snake");
	nbserp->set(0.f);
	nbserp->show(TRUE);

	nbcube=new Apaddle(MKQIID(qiid, 0xd38bdf48249cfda0), "snake lenght", this, 182, 36, 4, 0xff000000);
	nbcube->setTooltips("snake lenght");
	nbcube->set(0.f);
	nbcube->show(TRUE);

	czbuf=new ActrlButton(MKQIID(qiid, 0x50ccc9b714f786b0), "background", this, 60, 10, 9, 9, &resdll.get(MAKEINTRESOURCE(PNG_BUTTON), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	czbuf->setTooltips("background");
	czbuf->set(true);
	czbuf->show(TRUE);

	ccbuf=new ActrlButton(MKQIID(qiid, 0x2d6799de1f702ba8), "freeze", this, 60, 30, 9, 9, &resdll.get(MAKEINTRESOURCE(PNG_BUTTON), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	ccbuf->setTooltips("freeze");
	ccbuf->set(false);
	ccbuf->show(TRUE);

	col0=new Aselcolor(MKQIID(qiid, 0x6f2fbde4a1d89a58), "color (foreground)", this, 80, 8);
	col0->setTooltips("color (foreground)");
	col0->set(0xffffffff);
	col0->show(true);

	col1=new Aselcolor(MKQIID(qiid, 0xbc5348e7e6f84000), "color (background)", this, 80, 26);
	col1->setTooltips("color (background)");
	col1->set(0xff808080);
	col1->show(true);

	mirror=new Aselect(MKQIID(qiid, 0xa7a33f761bcec180), "mirrors", this, 204, 19, 3, 1, &resdll.get(MAKEINTRESOURCE(PNG_SELECT), "PNG"), 12, 12);
	mirror->setTooltips("mirrors");
	mirror->set(0);
	mirror->show(TRUE);

	src1=new Aselect(MKQIID(qiid, 0xae24fa2c39ff8496), "sources (3D)", this, 10, 8, 1, 2, &resdll.get(MAKEINTRESOURCE(PNG_SOURCES), "PNG"), 16, 16);
	src1->setTooltips("sources (3D)");
	src1->set(0);
	src1->show(TRUE);

	src2=new Aselect(MKQIID(qiid, 0x64328e1a74d13538), "sources (background)", this, 30, 8, 1, 2, &resdll.get(MAKEINTRESOURCE(PNG_SOURCES), "PNG"), 16, 16);
	src2->setTooltips("sources (background)");
	src2->set(0);
	src2->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AzoolFront::~AzoolFront()
{
	delete(src1);
	delete(src2);
	delete(mirror);
	delete(col0);
	delete(col1);
	delete(ccbuf);
	delete(czbuf);
	delete(speed);
	delete(xypow);
	delete(zpow);
	delete(nbserp);
	delete(nbcube);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AzoolBack::AzoolBack(QIID qiid, char *name, Azool *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_MOONSTER_BACK), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0x65c220f388b9a240), "video output", this, pinOUT|pinNOGFX, pos.w-26, 12);
	out->setTooltips("video output");
	out->show(TRUE);

	in1=new Avideo(MKQIID(qiid, 0x539869476dffa3a0), "video input 1", this, pinIN|pinNOGFX, 16, 12);
	in1->setTooltips("video input 1");
	in1->show(TRUE);

	in2=new Avideo(MKQIID(qiid, 0xf69984f000e80200), "video input 2", this, pinIN|pinNOGFX, 35, 12);
	in2->setTooltips("video input 2");
	in2->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AzoolBack::~AzoolBack()
{
	delete(in1);
	delete(in2);
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AzoolBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AzoolInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Azool(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	DLLEXPORT class Aplugz * getPlugz()
	{
		return new AzoolInfo("zool", &Azool::CI, "zool - 3D sample", "zool - 3D sample");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
