/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN.CPP					(c)	YoY'01						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<math.h>
#include						"main.h"
#include						"animat.h"
#include						"matrice.h"
#include						"resource.h"
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

ACI								AdancerInfo::CI		= ACI("AdancerInfo",	GUID(0x50000000,0x00000750), &AeffectInfo::CI, 0, NULL);
ACI								Adancer::CI			= ACI("Adancer",		GUID(0x50000000,0x00000751), &Aeffect::CI, 0, NULL);
ACI								AdancerFront::CI	= ACI("AdancerFront",	GUID(0x50000000,0x00000752), &AeffectFront::CI, 0, NULL);
ACI								AdancerBack::CI		= ACI("AdancerBack",	GUID(0x50000000,0x00000753), &AeffectBack::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int						count=0;
Aresource						resdll=Aresource("dancer", GetModuleHandle("dancer.dll"));

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

Adancer::Adancer(QIID qiid, char *name, AeffectInfo *info, Acapsule *capsule) : Aeffect(qiid, name, info, capsule)
{
	init();	// global init

	front=new AdancerFront(qiid, "dancer front", this, 50);
	front->setTooltips("dancer module");
	back=new AdancerBack(qiid, "dancer back", this, 50);
	back->setTooltips("dancer module");

	w=getVideoWidth();
	h=getVideoHeight();

	ob=new AoglBitmap(w, h);
	initGL();
	ttime=0;

	ob->makeCurrent();
	animat=new Aanimat();
	ob->releaseCurrent();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Adancer::~Adancer()
{
	ob->makeCurrent();
	glDeleteTextures(1, texture);
	ob->releaseCurrent();
	delete(animat);
	delete(ob);
	end();	// global free
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Adancer::save(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Adancer::load(class Afile *f)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Adancer::settings(bool emergency)
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

void Adancer::initGL()
{
	GLfloat			LightAmbient1[]= { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat			LightDiffuse1[]= { 1.0f, 1.0f, 1.f, 1.0f };
	GLfloat			LightAmbient2[]= { 0.f, 0.5f, 0.5f, 1.0f };
	GLfloat			LightDiffuse2[]= { 0.f, 1.0f, 1.0f, 1.0f };
	GLfloat			LightAmbient3[]= { 0.5f, 0.f, 0.5f, 1.0f };
	GLfloat			LightDiffuse3[]= { 1.0f, 0.f, 1.0f, 1.0f };
	GLfloat			LightPosition[]= { 0.0f, 100.0f, -2.0f, 1.0f };
	Abitmap			*tex=new Abitmap("d:\\elektro_logo.jpg");

	ob->makeCurrent();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, (GLdouble)w/(GLdouble)h, 1.f, 1000.f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse1);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse2);
	glLightfv(GL_LIGHT2, GL_POSITION, LightPosition);
	glLightfv(GL_LIGHT3, GL_AMBIENT, LightAmbient3);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, LightDiffuse3);
	glLightfv(GL_LIGHT3, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	glEnable(GL_LIGHTING);


	glGenTextures(1, texture);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	AoglBitmap::texture(tex);
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

void Adancer::action(double time, double dtime, double beat, double dbeat)
{
	static float	dx[]={1.f, -1.f, 1.f, -1.f};
	static float	dy[]={1.f, -1.f, -1.f, 1.f};
	static float	dz[]={1.f, 1.f, -1.f, -1.f};
	float			color0[]={1.f, 1.f, 1.f, 1.f};
	float			color1[]={1.f, 1.f, 0.f, 1.f};
	float			color2[]={0.f, 1.f, 1.f, 1.f};
	float			color3[]={1.f, 0.f, 1.f, 1.f};
	AdancerFront	*front=(AdancerFront *)this->front;
	AdancerBack		*back=(AdancerBack *)this->back;
	Avideo			*out=back->out;
	Avideo			*in1=back->in1;
	Avideo			*in2=back->in2;
	GLfloat			LightPosition[]= { 0.0f, 100.0f, -2.0f, 1.0f };
	float			speed=front->speed->get();
	float			xypow=front->xypow->get();
	float			zpow=front->zpow->get();
	float			nbserp=front->nbserp->get();
	float			nbcube=front->nbcube->get();
	bool			ccbuf=front->ccbuf->get();
	bool			czbuf=front->czbuf->get();
	dword			col0=front->col0->get();
	dword			col1=front->col1->get();
	float			r0,g0,b0;
	float			r1,g1,b1;
	float			dr,dg,db;
	int				selbody[2];
	int				src1=front->src1->get();
	int				src2=front->src2->get();

	selbody[0]=front->selbody[0]->get();
	selbody[1]=front->selbody[1]->get();
	selbody[2]=front->selbody[2]->get();

	if((selbody[0]!=animat->elektete)||(selbody[1]!=animat->elektronc)||(selbody[2]!=animat->elekjambes))
		animat->elekPerso(selbody[0], selbody[1], selbody[2]);

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

	if(src1==0)
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

	if(src1==1)
	{
		in2->enter(__FILE__,__LINE__);
		{
			Abitmap	*b=in2->getBitmap();
			if(b)
			{
				glBindTexture(GL_TEXTURE_2D, texture[0]);
				ob->texture(b, (w-h)>>1, 0, h, h);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			}
		}
		in2->leave();
	}


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	LightPosition[0]=(float)sin(time*0.003f)*50.f;
	LightPosition[1]=(float)sin(time*0.003515f)*50.f;
	LightPosition[2]=(float)sin(time*0.0035621f)*50.f;
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	LightPosition[0]=(float)sin(time*0.0034515f)*50.f;
	LightPosition[1]=(float)sin(time*0.0035121f)*50.f;
	LightPosition[2]=(float)sin(time*0.0034123f)*50.f;
	glLightfv(GL_LIGHT2, GL_POSITION, LightPosition);
	LightPosition[0]=(float)sin(time*0.00358632f)*50.f;
	LightPosition[1]=(float)sin(time*0.0034112f)*50.f;
	LightPosition[2]=(float)sin(time*0.00372239f)*50.f;
	glLightfv(GL_LIGHT3, GL_POSITION, LightPosition);

	if(!ccbuf)
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


	if(czbuf)
	{
		glColor3f(r1, g1, b1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, -99.f);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-35.5f, -35.5f,  0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 35.5f, -35.5f,  0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 35.5f,  35.5f,  0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-35.5f,  35.5f,  0.0f);
		glEnd();
	}

	//ttime+=dtime*(speed*5.f+0.1f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.f, 1.3f+(nbcube-0.5f)*3.f, -1.7f-10.f*zpow);
	glRotatef((xypow-0.5f)*360.f, 0.0f, 1.f, 0.0f);
	glRotatef((nbserp-0.5f)*360.f, 1.0f, 0.f, 0.0f);
	animat->animat_render((float)(dtime*(speed*5.f+0.1f)*0.001f));
/*
	int	k;
	for(k=0; k<mirror; k++)
	{
		int	j;
		for(j=0; j<nbserp; j++)
		{
			float	v=ttime+(float)j*10000.f;
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
*/
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

void AdancerFront::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QIID qiidbody[]={ 0x70d791b4b776a520, 0xce8dcd12abf5ab20, 0x27666d7c74c62000}; 

AdancerFront::AdancerFront(QIID qiid, char *name, Adancer *e, int h) : AeffectFront(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_FRONT), "PNG");
	back=new Abitmap(&o);

	speed=new Apaddle(MKQIID(qiid, 0xf3c6f20c50378800), "speed", this, 106, 8, paddleYOLI32);
	speed->setTooltips("speed");
	speed->set(0.0f);
	speed->show(TRUE);

	xypow=new Apaddle(MKQIID(qiid, 0xc619df371ebcb400), "xy movement", this, 144, 4, paddleYOLI24);
	xypow->setTooltips("xy movement");
	xypow->set(0.5f);
	xypow->show(TRUE);

	zpow=new Apaddle(MKQIID(qiid, 0x6fbe2bd04f9a5328), "z movement", this, 170, 4, paddleYOLI24);
	zpow->setTooltips("z movement");
	zpow->set(0.5f);
	zpow->show(TRUE);

	nbserp=new Apaddle(MKQIID(qiid, 0x981bb4f47ca45a00), "nb snake", this, 148, 30, paddleYOLI16);
	nbserp->setTooltips("nb snake");
	nbserp->set(0.5f);
	nbserp->show(TRUE);

	nbcube=new Apaddle(MKQIID(qiid, 0x978845beb0f6caa0), "snake lenght", this, 174, 30, paddleYOLI16);
	nbcube->setTooltips("snake lenght");
	nbcube->set(0.5f);
	nbcube->show(TRUE);

	czbuf=new ActrlButton(MKQIID(qiid, 0x551958d3499bc860), "background", this, 60, 10, 9, 9, &resdll.get(MAKEINTRESOURCE(PNG_BUTTON), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	czbuf->setTooltips("background");
	czbuf->set(true);
	czbuf->show(TRUE);

	ccbuf=new ActrlButton(MKQIID(qiid, 0xbc155f024554d044), "freeze", this, 60, 30, 9, 9, &resdll.get(MAKEINTRESOURCE(PNG_BUTTON), "PNG"), Abutton::bt2STATES|Abutton::btBITMAP);
	ccbuf->setTooltips("freeze");
	ccbuf->set(false);
	ccbuf->show(TRUE);

	col0=new Aselcolor(MKQIID(qiid, 0xb36f11d95429fec4), "color (foreground)", this, 80, 8);
	col0->setTooltips("color (foreground)");
	col0->set(0xffffffff);
	col0->show(true);

	col1=new Aselcolor(MKQIID(qiid, 0xa5880965d7421a80), "color (background)", this, 80, 26);
	col1->setTooltips("color (background)");
	col1->set(0xff808080);
	col1->show(true);

	{
		int	i;
		for(i=0; i<3; i++)
		{
			selbody[i]=new Aselect(MKQIID(qiid, qiidbody[i]), "select bodies", this, 204+i*16, 4, 1, 3, &resdll.get(MAKEINTRESOURCE(PNG_SELECT), "PNG"), 12, 12);
			selbody[i]->setTooltips("select bodies");
			selbody[i]->set(0);
			selbody[i]->show(TRUE);
		}
	}

	src1=new Aselect(MKQIID(qiid, 0x09cd65260e7ca8e2), "sources (3D)", this, 10, 8, 1, 2, &resdll.get(MAKEINTRESOURCE(PNG_SOURCES), "PNG"), 16, 16);
	src1->setTooltips("sources (3D)");
	src1->set(0);
	src1->show(TRUE);

	src2=new Aselect(MKQIID(qiid, 0xe517972503b14000), "sources (background)", this, 30, 8, 1, 2, &resdll.get(MAKEINTRESOURCE(PNG_SOURCES), "PNG"), 16, 16);
	src2->setTooltips("sources (background)");
	src2->set(0);
	src2->show(TRUE);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdancerFront::~AdancerFront()
{
	delete(src1);
	delete(src2);
	delete(selbody[0]);
	delete(selbody[1]);
	delete(selbody[2]);
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

AdancerBack::AdancerBack(QIID qiid, char *name, Adancer *e, int h) : AeffectBack(qiid, name, e, h)
{
	Aresobj	o=resdll.get(MAKEINTRESOURCE(PNG_BACK), "PNG");
	back=new Abitmap(&o);

	out=new Avideo(MKQIID(qiid, 0xd331dfbdd604f320),  "video output", this, pinOUT, pos.w-18, 10);
	out->setTooltips("video output");
	out->show(TRUE);

	in1=new Avideo(MKQIID(qiid, 0x8cf0f07ef3dc6cc0),  "video input 1", this, pinIN, 10, 10);
	in1->setTooltips("video input 1");
	in1->show(TRUE);

	in2=new Avideo(MKQIID(qiid, 0x73f9c0b50f86249d),  "video input 2", this, pinIN, 30, 10);
	in2->setTooltips("video input 2");
	in2->show(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AdancerBack::~AdancerBack()
{
	delete(in1);
	delete(in2);
	delete(out);
	delete(back);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdancerBack::paint(Abitmap *b)
{
	b->set(0, 0, back, bitmapDEFAULT, bitmapDEFAULT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Aeffect * AdancerInfo::create(QIID qiid, char *name, Acapsule *capsule)
{
	return new Adancer(qiid, name, this, capsule);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	DLLEXPORT class Aplugz * getPlugz()
	{
		return new AdancerInfo("dancer", &Adancer::CI, "dancer", "dancer");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
