#ifndef _ANIMAT_H_	// from "animat.c" 4 kulbuto
#define _ANIMAT_H_	// renan@calva.net'97

#include	<GL/gl.h>
#include	<GL/glu.h>
//#include	<GL/glaux.h>


// rajout pour la demo
int				getnbanim();
//extern int		persozb[300];
// fin rajout



// gestion des erreurs
#define     ANIMAT_NOERR                0
#define     ANIMAT_FILEOPENERR          1
#define     ANIMAT_FILEREADERR          2
#define     ANIMAT_ALLOCMEMERR          3
#define     ANIMAT_RENDERERR            4
#define     ANIMAT_INVALIDPARAMERR      5
#define     ANIMAT_MAXIMUMERR           6
#define     ANIMAT_NAMEANIMERR          7

// mode animation
#define     ANIMAT_RECALAUTO            1
#define     ANIMAT_SKIPSTATIC           2
#define     ANIMAT_INTERUPT             4

// MAX value
#define		MAX_PERSOS					2
#define		MAX_OBJECTS					10 
#define		MAX_TEXTURES				20
#define		ATEXTLEN					32

// typedef

typedef struct SA_PERSO		A_PERSO;
typedef struct SA_OBJECT	A_OBJECT;
typedef struct SA_TEXTURE	A_TEXTURE;

// struct
struct SA_TEXTURE
{
	char	name[ATEXTLEN];
	int		xdep,ydep,width,height;
	GLuint	gltex;
};

#define		NBTEXTURES					4

// class

class Aanimat
{
public:
					Aanimat					()	
					{
						nbtextures=0;
						nbobjects=0;
						nbpersos=0;
						animat_time=0.f;
						cam_tx=0.f;
						cam_ty=0.f;
						cam_tz=0.f;
						cam_rx=0.f;
						cam_ry=0.f;
						cam_rz=0.f;
					    animat_loadtexture();
						animat_init();
					}

	virtual			~Aanimat				()  { animat_clear(); glDeleteTextures(NBTEXTURES, gltex); }

	void			elekPerso				(int tete, int tronc, int jambes);
	void			elekPlay				(int anim);
	void			elekRender				();

	int				elektete;
	int				elektronc;
	int				elekjambes;

	int				animat_getinterror		();
	char *			animat_getstrerror		();
	int				animat_init				();
	void			animat_clear			();
	void			animat_update			();
	void			animat_render			(float avance);
	int				object_create3			(int *hp, int i1, int i2, int i3);
	int				object_clear			(int hp);
	int				object_play				(int hp, int anim, int mode, float tx, float ty, float tz, float rx, float ry, float rz);
	int				object_play3			(int hp, int anim1, int anim2, int anim3, int mode, float tx, float ty, float tz, float rx, float ry, float rz);
	int				item_load				(int *ip, int name);
	int				item_clear				(int ip);


	float			cam_tx,cam_ty,cam_tz,cam_rx,cam_ry,cam_rz;

// private
	int				perso_render			(A_PERSO *perso);
	void			animat_control			(int hp);
	A_OBJECT *		object_load				(int name);
	void			animat_loadtexture		();

	A_PERSO *		persos[MAX_PERSOS];
	A_OBJECT *		objects[MAX_OBJECTS];
	A_TEXTURE		textures[MAX_TEXTURES];
	int				nbtextures;
	int				nbobjects;
	int				nbpersos;

	GLuint			gltex[4];

	float			animat_time;

	float			sinus[2][8];
};
#endif