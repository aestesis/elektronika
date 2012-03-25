// attention,			|	extern	|	intern	|
//						|----------	|----------	|
//						|	object	|	perso	|
//						|			|			|
//						|	item	|	object	|


#include	<windows.h>
#include	<assert.h>
#include	<stdio.h>	// "animat.c"
#include	<stdlib.h>	// renan@aestesis.org'97
#include	<math.h>
#include    <string.h>
#include	"main.h"
#include	"animat.h"
#include	"matrice.h"
#include	"resource.h" 
 

#define PERSOSDIR   ""
#define TEXTURESDIR ""

// remplace la partie IA
#define NB_ANIMS    26
char* animation_array[] = {
        "a00",
        "a01",
        "a02",
        "a03",
        "a04",
        "a05",
        "a06",
        "a07",
        "a08",
        "a09",
        "a10",
        "a11",
        "a12",
        "a13",
        "a14",
        "a15",
        "a16",
        "a17",
        "a18",
        "a19",
        "a20",
        "a21",
        "a22",
        "a23",
        "a24",
        "a25",
         };

#define MYPI    3.14159265359

////////////////////////////////////////////////
//
// gestion des erreurs					(begin)

#define ANIMATERROR	sprintf(animat_linerr, " @ %4d %s" , __LINE__ , __FILE__ )

static int			animat_error=0;
static char 		animat_linerr[64]="";
static char	        strerr[256];

int animat_getinterror()
{
	return animat_error;
}

char * animat_getstrerror()
{
	strcpy(strerr, "[animation] ");
	switch(animat_error)
	{
        case ANIMAT_NOERR:
        strcat(strerr, "no errors.");
		break;

        case ANIMAT_FILEOPENERR:
        strcat(strerr, "can't open file.");
		break;

        case ANIMAT_FILEREADERR:
        strcat(strerr, "can't read file.");
		break;

        case ANIMAT_ALLOCMEMERR:
        strcat(strerr, "can't allocate memory.");
		break;

        case ANIMAT_RENDERERR:
        strcat(strerr, "renderer error.");
		break;

        case ANIMAT_INVALIDPARAMERR:
        strcat(strerr, "invalid(s) parameter(s).");
		break;

        case ANIMAT_MAXIMUMERR:
        strcat(strerr, "maximum buffer error.");
		break;

        case ANIMAT_NAMEANIMERR:
        strcat(strerr, "an animation name in the bank don't exist in the game.");
		break;

	}
	strcat(strerr, animat_linerr);
    strcat(strerr, "\n");
	return (char *)strerr;
}

// gestion des erreurs					(end)
//
////////////////////////////////////////////////

////////////////////////////////////////////////
//
// structures fichier animation			(begin)

#define		TEXTLEN		128

#pragma pack(1)

typedef struct SE_OBJECT
{
	char			name[TEXTLEN];
	int				nbmaterials;
	int				nbtypevtx;
	int				nbvertices;
	int				nbfaces;
	int				nbnodes;
	int				nbanims;
} E_OBJECT;

typedef struct SE_MATERIAL
{
	char			name[TEXTLEN];
	unsigned char	ar,ag,ab,ap;
	unsigned char	dr,dg,db,dp;
	unsigned char	sr,sg,sb,sp;
	char			texture[TEXTLEN];
} E_MATERIAL;

typedef struct SE_TYPEVTX
{
	char			name[TEXTLEN];
	int				code;				// 0..normal   1..wilde inclus  2..wilde exclus
} E_TYPEVTX;

typedef struct SE_VERTICE
{
	int				type;
} E_VERTICE;

typedef struct SE_FACE
{
	int				vertice[3];
	unsigned char	u[3];
	unsigned char	v[3];
	unsigned short	mat;
} E_FACE;

typedef struct SE_NODE
{
	char			name[TEXTLEN];
	int				nbvertices;
	int				pere;
} E_NODE;

typedef struct SE_NODEVTX
{
	int				vertice;
    float           weight;
    float           x,y,z;
} E_NODEVTX;

typedef struct SE_ANIM
{
	char			name[TEXTLEN];
	int				nbkeys;
	int				nbmorphs;
} E_ANIM;

typedef struct SE_IMORPH
{
	int				vertice;
} E_IMORPH;

typedef struct SE_MORPH
{
    float           x,y,z;
} E_MORPH;

typedef struct SE_ANIMPOS
{
    float           rx,ry,rz;
    float           tx,ty,tz;
} E_ANIMPOS;

// structures fichier animation			(end)
//
////////////////////////////////////////////////



////////////////////////////////////////////////
//
// structures base de données animation	(begin)

enum
{
	NULLPERE_1,
	JNTJAMBEG_1,
	JNTJAMBEG_2,
	JNTPIEDG,
	JNTJAMBED_1,
	JNTJAMBED_2,
	JNTPIEDD,
	JNTTRONC,
	JNT1_1,
	JNTBRASD_1,
	JNTBRASD_2,
	JNTBRASG_1,
	JNTBRASG_2,
	CHNTETE,
	JNTTETE
};

#define		MAX_OBJECTBYPERSO	4

// perso

typedef struct S_VTX3
{
    float   x,y,z;
} VTX3;

typedef struct SA_PERSO
{
	int					nbobjects;
	int					nbfaces;
	int					nbvertices;
	int					nbnodes;
	int					nbnodesobj[MAX_OBJECTBYPERSO];
	struct SA_ANIM		**anims[MAX_OBJECTBYPERSO];
	struct SA_PFACE		*faces;
    VTX3                *vertices;
    VTX3                *vertigo;
	struct SA_OBJECT	*obj[MAX_OBJECTBYPERSO];
    VTX3                **tvertigo[MAX_OBJECTBYPERSO];  // tableau de correspondance vtx object 2 vtx perso
	struct SA_PNODE		*nodes;
    struct RMATRICE     *firstmat[MAX_OBJECTBYPERSO];  // pointeur sur la matrice du premier noeud de chaques parties
	// gestion de l'anim en cours
    int                 modeanim;   // (bit)  1..recal auto    2..skip static position  4..interupt
    float               starttime;
    float               rx,ry,rz;
    float               tx,ty,tz;
	struct SA_ANIM		*currentanim[MAX_OBJECTBYPERSO];
    float               nexttime;
    float               nrx,nry,nrz;
    float               ntx,nty,ntz;
    struct SA_ANIM      *nextanim[MAX_OBJECTBYPERSO];
} A_PERSO;

typedef struct SA_PFACE
{
    VTX3              *vertice[3];
	unsigned char		u[4],v[4];
	struct SA_MATERIAL	*material;
} A_PFACE;

typedef struct SA_PNODE
{
	int					nobj;		// numero de l'object dont provient le node
	int					nodeobj;	// numero du node dans l'objet dont il provient
	int					nbvertices;
	struct SA_PNODEVTX	*vertices;
	struct SA_PNODE		*pere;
    struct RMATRICE     matrice;
} A_PNODE;

typedef struct SA_PNODEVTX
{
    VTX3                 *vertice;
    float               weight;
    float               x,y,z;
} A_PNODEVTX;

// object

typedef struct SA_OBJECT
{
	char				name[ATEXTLEN];
	int					nbmaterials;
	int					nbtypevtx;
	int					nbvertices;
	int					nbfaces;
	int					nbnodes;
	int					nbanims;
	struct SA_MATERIAL	*materials;
	struct SA_TYPEVTX	*typevtx;
	struct SA_VTX2TYPE	*vertices;
	struct SA_FACE		*faces;
	struct SA_NODE		*nodes;
	struct SA_ANIM		**anims;
} A_OBJECT;

typedef struct SA_MATERIAL
{
	unsigned char	ar,ag,ab,aa;
	unsigned char	dr,dg,db,da;
	unsigned char	sr,sg,sb,sa;
	A_TEXTURE		*tex;
} A_MATERIAL;

typedef struct SA_TYPEVTX
{
	char				name[ATEXTLEN];
	int					code;				// 0..normal   1..wilde inclus  2..wilde exclus
} A_TYPEVTX;

typedef struct SA_VTX2TYPE
{
	int					type;
} A_VTX2TYPE;

typedef struct SA_FACE
{
	int					vertice[3];
	unsigned char		u[4],v[4];
	struct SA_MATERIAL	*material;
} A_FACE;

typedef struct SA_NODE
{
	int					nbvertices;
	struct SA_NODEVTX	*vertices;
	int					code;	// 1..root    2..vertebre   4..main/pied gauche   8..main/pied droit	(1..16)x256 le node
	int					pere;
} A_NODE;

typedef struct SA_NODEVTX
{
	int					vertice;
    float               weight;
    float               x,y,z;
} A_NODEVTX;

typedef struct SA_ANIM
{
    char                name[ATEXTLEN];
	int					nbkeys;
	int					nbmorphs;
    float               *time;
	struct SA_IMORPH	*imorphs;
	struct SA_MORPH		**morphs;
	struct SA_ANIMPOS	**animpos;
} A_ANIM;

typedef struct SA_IMORPH
{
    int                 vertice;
} A_IMORPH;

typedef struct SA_MORPH
{
    float               x,y,z;
} A_MORPH;

typedef struct SA_ANIMPOS
{
    float               rx,ry,rz;
    float               tx,ty,tz;
} A_ANIMPOS;

// structures base de données animation	(end)
//
////////////////////////////////////////////////

////////////////////////////////////////////////
//
// variables internes					(begin)


// variables internes					(begin)
//
////////////////////////////////////////////////


////////////////////////////////////////////////
//
// fonctions internes					(begin)


static void object_free(A_OBJECT *obj)
{
	int		i,j;
	if(obj!=NULL)
	{
		if(obj->materials!=NULL)
			free(obj->materials);

		if(obj->typevtx!=NULL)
			free(obj->typevtx);

		if(obj->vertices!=NULL)
			free(obj->vertices);

		if(obj->faces!=NULL)
			free(obj->faces);

		if(obj->nodes!=NULL)
		{
			A_NODE	*pnode=obj->nodes;
			for(i=0; i<obj->nbnodes; i++)
			{
				if(pnode->vertices!=NULL)
					free(pnode->vertices);
				pnode++;
			}
			free(obj->nodes);
		}

		if(obj->anims)
		{
			int		an;
			for(an=0; an<NB_ANIMS; an++)
				if(*(obj->anims+an))
				{
					A_ANIM	*panim=*(obj->anims+an);
					if(panim->time)
						free(panim->time);
					if(panim->imorphs)
						free(panim->imorphs);
					if(panim->morphs)
					{
						A_MORPH	**pmorph=panim->morphs;
						for(j=0; j<panim->nbkeys; j++)
						{
							if(*pmorph!=NULL)
								free(*pmorph);
							pmorph++;
						}
						free(panim->morphs);
					}
					if(panim->animpos!=NULL)
					{
						A_ANIMPOS	**panimpos=panim->animpos;
						for(j=0; j<panim->nbkeys; j++)
						{
							if(*panimpos!=NULL)
								free(*panimpos);
							panimpos++;
						}
						free(panim->animpos);
					}
				}
			free(obj->anims);
		}
		free(obj);
	}
}

void	*presource=NULL;

FILE *myfopen(int name, char *status)
{
    HRSRC               h=FindResource(GetModuleHandle("dancer"), MAKEINTRESOURCE(name), "KPR");;
	if(h)
	{
		presource=(void *)LockResource(LoadResource(GetModuleHandle("dancer"), h));
		if(presource)
			return	(FILE *)1;
	}
	return 0;
}

int	myfread(void *buffer, int nb, int size, FILE *fic)
{
	if(presource)
	{
		int	s=nb*size;
		memcpy(buffer, presource, s);
		presource=(void *)((char *)presource+s);
		return s;
	}
	return 0;
}

void	myfclose(FILE *fic)
{
	presource=NULL;
}



A_OBJECT * Aanimat::object_load(int name)
{
	FILE		*fic;
	A_OBJECT	*obj;
	A_TEXTURE	*mytex[30];

	{
		int		i;
		for(i=0; i<30; i++)
			mytex[i]=NULL;
	}

	fic=myfopen(name, "rb");
	if(fic==NULL)
	{
		ANIMATERROR;
        animat_error=ANIMAT_FILEOPENERR;
		return NULL;
	}

	obj=(A_OBJECT *) calloc(1, sizeof(A_OBJECT));
	if(obj==NULL)
	{
		ANIMATERROR;
        animat_error=ANIMAT_ALLOCMEMERR;
		myfclose(fic);
		return NULL;
	}

	{	// charge object
		E_OBJECT	eobj;
		if(myfread(&eobj, 1, sizeof(E_OBJECT), fic)!=sizeof(E_OBJECT))
		{
			ANIMATERROR;
            animat_error=ANIMAT_FILEREADERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		eobj.name[ATEXTLEN-1]=0;
		strcpy(obj->name, eobj.name);
		obj->nbfaces=eobj.nbfaces;
		obj->nbnodes=eobj.nbnodes;
		obj->nbanims=eobj.nbanims;
		obj->nbvertices=eobj.nbvertices;
		obj->nbmaterials=eobj.nbmaterials;
		obj->nbtypevtx=eobj.nbtypevtx;
	}

	if(obj->nbmaterials>0) // charge materials
	{
		if((obj->materials=(A_MATERIAL *)calloc(obj->nbmaterials, sizeof(A_MATERIAL)))==NULL)
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		{
			E_MATERIAL	emat;
			A_MATERIAL	*pmat=obj->materials;
			int			i;
			for(i=0; i<obj->nbmaterials; i++)
			{
				if(myfread(&emat, 1, sizeof(E_MATERIAL), fic)!=sizeof(E_MATERIAL))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_FILEREADERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
				}
				pmat->ar=emat.ar;
				pmat->ag=emat.ag;
				pmat->ab=emat.ab;
				pmat->dr=emat.dr;
				pmat->dg=emat.dg;
				pmat->db=emat.db;
				pmat->sr=emat.sr;
				pmat->sg=emat.sg;
				pmat->sb=emat.sb;
				pmat->tex=NULL;
				if(strlen(emat.texture)>0)
				{
					char	str[ATEXTLEN],*chr;
					int		j;
					strupr(emat.texture);
					for(j=0; j<nbtextures; j++)
					{
						strcpy(str, textures[j].name);
						strupr(str);
						chr=strchr(str, '.');
						if(chr)
						{
							*chr=0;
							if(*(chr-1)=='~')
								*(chr-1)=0;
						}
						if(strstr(emat.texture, str))
						{
							pmat->tex=&textures[j];
							mytex[i]=&textures[j];
							break;
						}
					}
					assert(pmat->tex);
				}
				pmat++;
			}
		}
	}

	if(obj->nbtypevtx>0)	// charge info de vertices
	{
		if((obj->typevtx=(A_TYPEVTX *)calloc(obj->nbtypevtx, sizeof(A_TYPEVTX)))==NULL)
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		{
			E_TYPEVTX	etvtx;
			A_TYPEVTX	*ptvtx=obj->typevtx;
			int			i;
			for(i=0; i<obj->nbtypevtx; i++)
			{
				if(myfread(&etvtx, 1, sizeof(E_TYPEVTX), fic)!=sizeof(E_TYPEVTX))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_FILEREADERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
				}
				etvtx.name[ATEXTLEN-1]=0;
				strcpy(ptvtx->name, etvtx.name);
				(ptvtx++)->code=etvtx.code;
			}
		}
	}

	if(obj->nbvertices>0)	// charge vertices
    {
		if((obj->vertices=(A_VTX2TYPE *)calloc(obj->nbvertices, sizeof(A_VTX2TYPE)))==NULL)
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		{
			int			i;
			E_VERTICE	evtx;
			A_VTX2TYPE	*pvtx=obj->vertices;
			for(i=0; i<obj->nbvertices; i++)
			{
				if(myfread(&evtx, 1, sizeof(E_VERTICE), fic)!=sizeof(E_VERTICE))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_FILEREADERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
				}
				(pvtx++)->type=evtx.type;
			}
		}
	}

	if(obj->nbfaces>0)	// charge faces
	{
		if((obj->faces=(A_FACE *)calloc(obj->nbfaces, sizeof(A_FACE)))==NULL)
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		{
			int			i;
			E_FACE		eface;
			A_FACE		*pface=obj->faces;
			for(i=0; i<obj->nbfaces; i++)
			{
				if(myfread(&eface, 1, sizeof(E_FACE), fic)!=sizeof(E_FACE))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_FILEREADERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
				}
				pface->vertice[0]=eface.vertice[0];
				pface->vertice[1]=eface.vertice[1];
				pface->vertice[2]=eface.vertice[2];
				if(mytex[eface.mat])
				{
					unsigned int	dx=mytex[eface.mat]->xdep;
					unsigned int	dy=mytex[eface.mat]->ydep;
					unsigned int	w=mytex[eface.mat]->width;
					unsigned int	h=mytex[eface.mat]->height;
					pface->u[0]=(unsigned char)(dx+(((unsigned int)eface.u[0]*w)>>8));
					pface->u[1]=(unsigned char)(dx+(((unsigned int)eface.u[1]*w)>>8));
					pface->u[2]=(unsigned char)(dx+(((unsigned int)eface.u[2]*w)>>8));
					pface->v[0]=(unsigned char)(dy+(((unsigned int)eface.v[0]*h)>>8));
					pface->v[1]=(unsigned char)(dy+(((unsigned int)eface.v[1]*h)>>8));
					pface->v[2]=(unsigned char)(dy+(((unsigned int)eface.v[2]*h)>>8));
				}
				else
				{
					pface->u[0]=0;
					pface->u[1]=0;
					pface->u[2]=0;
					pface->v[0]=0;
					pface->v[1]=0;
					pface->v[2]=0;
				}
				pface->material=&obj->materials[eface.mat];
				pface++;
			}
		}
	}

	if(obj->nbnodes>0)	// charge les nodes & les vertices correspondants en local
	{
		if((obj->nodes=(A_NODE *)calloc(obj->nbnodes, sizeof(A_NODE)))==NULL)
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		{
			E_NODE		enode;
			E_NODEVTX	envtx;
			A_NODE		*pnode=obj->nodes;
			int			i,j;
			for(i=0; i<obj->nbnodes; i++)
			{
				A_NODEVTX	*pnvtx;
				if(myfread(&enode, 1, sizeof(E_NODE), fic)!=sizeof(E_NODE))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_FILEREADERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
				}
				pnode->code=0;
				if(strstr(enode.name, "nullpere"))
					pnode->code|=3|(NULLPERE_1<<8);
				if(strstr(enode.name, "jntpiedg"))
					pnode->code|=4|(JNTPIEDG<<8);
				if(strstr(enode.name, "jntpiedd"))
					pnode->code|=8|(JNTPIEDD<<8);
				if(strstr(enode.name, "jnttronc"))
					pnode->code|=1|(JNTTRONC<<8);
				if(strstr(enode.name, "jntbrasg_2"))
					pnode->code|=4|(JNTBRASG_2<<8);
				if(strstr(enode.name, "jntbrasd_2"))
					pnode->code|=8|(JNTBRASD_2<<8);
				if(strstr(enode.name, "chntete"))
					pnode->code|=2|(CHNTETE<<8);
				if(strstr(enode.name, "jnttete"))
					pnode->code|=3|(JNTTETE<<8);

				if(strstr(enode.name, "jntjambeg_1"))
					pnode->code|=(JNTJAMBEG_1<<8);
				if(strstr(enode.name, "jntjambeg_2"))
					pnode->code|=(JNTJAMBEG_2<<8);
				if(strstr(enode.name, "jntjambed_1"))
					pnode->code|=(JNTJAMBED_1<<8);
				if(strstr(enode.name, "jntjambed_2"))
					pnode->code|=(JNTJAMBED_2<<8);
				if(strstr(enode.name, "jntbrasg_1"))
					pnode->code|=(JNTBRASG_1<<8);
				if(strstr(enode.name, "jntbrasd_1"))
					pnode->code|=(JNTBRASD_1<<8);
				if(strstr(enode.name, "jnt1_1"))
					pnode->code|=(JNT1_1<<8);


				pnode->nbvertices=enode.nbvertices;
				pnode->pere=enode.pere;
				if(pnode->nbvertices>0)
				{
					if((pnvtx=pnode->vertices=(A_NODEVTX *)calloc(pnode->nbvertices, sizeof(A_NODEVTX)))==NULL)
					{
						ANIMATERROR;
                        animat_error=ANIMAT_ALLOCMEMERR;
						object_free(obj);
						myfclose(fic);
						return NULL;
					}
					for(j=0; j<pnode->nbvertices; j++)
					{
						if(myfread(&envtx, 1, sizeof(E_NODEVTX), fic)!=sizeof(E_NODEVTX))
						{
							ANIMATERROR;
                            animat_error=ANIMAT_FILEREADERR;
							object_free(obj);
							myfclose(fic);
							return NULL;
						}
						pnvtx->vertice=envtx.vertice;
						pnvtx->weight=envtx.weight;
                        pnvtx->x=envtx.x;
                        pnvtx->y=envtx.y;
                        (pnvtx++)->z=envtx.z;
					}
				}
				pnode++;
			}
		}
	}

	if(obj->nbanims>0)	// charge les anims
	{
		if((obj->anims=(A_ANIM **)calloc(NB_ANIMS, sizeof(A_ANIM *)))==NULL)
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			object_free(obj);
			myfclose(fic);
			return NULL;
		}
		{
			int			i,j,k;
			E_ANIM		eanim;
			E_IMORPH	eimorph;
			E_MORPH		emorph;
			E_ANIMPOS	eanimpos;
			A_ANIM		*panim;
			for(i=0; i<obj->nbanims; i++)
			{
				A_IMORPH	*pimorph;
				if(myfread(&eanim, 1, sizeof(E_ANIM), fic)!=sizeof(E_ANIM))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_FILEREADERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
				}

				for(j=0; j<NB_ANIMS; j++)
					if(!(strcmp(eanim.name, animation_array[j])))
						break;

                if(j>=NB_ANIMS)
                {
					ANIMATERROR;
                    animat_error=ANIMAT_NAMEANIMERR;
					object_free(obj);
					myfclose(fic);
					return NULL;
                }

                if((*(obj->anims+j)=panim=(A_ANIM *)calloc(NB_ANIMS, sizeof(A_ANIM)))==NULL)
                {
                    ANIMATERROR;
                    animat_error=ANIMAT_ALLOCMEMERR;
                    object_free(obj);
                    myfclose(fic);
                    return NULL;
                }
                panim->nbkeys=eanim.nbkeys;
                panim->nbmorphs=eanim.nbmorphs;
                strcpy(panim->name, eanim.name);  // 4 debug
                if((panim->time=(float *)calloc(panim->nbkeys, sizeof(float)))==NULL)
                {
                    ANIMATERROR;
                    animat_error=ANIMAT_ALLOCMEMERR;
                    object_free(obj);
                    myfclose(fic);
                    return NULL;
                }
                {
                    float mt;
                    if(myfread(&mt, 1, sizeof(float), fic)!=sizeof(float))
                    {
                        ANIMATERROR;
                        animat_error=ANIMAT_FILEREADERR;
                        object_free(obj);
                        myfclose(fic);
                        return NULL;
                    }
                    *(panim->time)=0.f;
                    for(j=1; j<panim->nbkeys; j++)
                    {
                        float     t;
                        if(myfread(&t, 1, sizeof(float), fic)!=sizeof(float))
                        {
                            ANIMATERROR;
                            animat_error=ANIMAT_FILEREADERR;
                            object_free(obj);
                            myfclose(fic);
                            return NULL;
                        }
                        *(panim->time+j)=t-mt;
                    }
                }
                if(panim->nbmorphs>0)
                {
                    if((pimorph=panim->imorphs=(A_IMORPH *)calloc(panim->nbmorphs, sizeof(A_IMORPH)))==NULL)
                    {
                        ANIMATERROR;
                        animat_error=ANIMAT_ALLOCMEMERR;
                        object_free(obj);
                        myfclose(fic);
                        return NULL;
                    }
                    for(j=0; j<panim->nbmorphs; j++)
                    {
                        if(myfread(&eimorph, 1, sizeof(E_IMORPH), fic)!=sizeof(E_IMORPH))
                        {
                            ANIMATERROR;
                            animat_error=ANIMAT_FILEREADERR;
                            object_free(obj);
                            myfclose(fic);
                            return NULL;
                        }
                        (pimorph++)->vertice=eimorph.vertice;
                    }
                }

                if(panim->nbkeys>0)
                {
                    A_MORPH     **pmorph;
                    A_ANIMPOS   **panimpos;
                    if((pmorph=panim->morphs=(A_MORPH **)calloc(panim->nbkeys, sizeof(A_MORPH *)))==NULL)
                    {
                        ANIMATERROR;
                        animat_error=ANIMAT_ALLOCMEMERR;
                        object_free(obj);
                        myfclose(fic);
                        return NULL;
                    }
                    if((panimpos=panim->animpos=(A_ANIMPOS **)calloc(panim->nbkeys, sizeof(A_ANIMPOS *)))==NULL)
                    {
                        ANIMATERROR;
                        animat_error=ANIMAT_ALLOCMEMERR;
                        object_free(obj);
                        myfclose(fic);
                        return NULL;
                    }

                    for(j=0; j<panim->nbkeys; j++)
                    {
                        if(panim->nbmorphs>0)
                        {
                            A_MORPH     *ptrmorph;
                            if((ptrmorph=*(pmorph++)=(A_MORPH *)calloc(panim->nbmorphs, sizeof(A_MORPH)))==NULL)
                            {
                                ANIMATERROR;
                                animat_error=ANIMAT_ALLOCMEMERR;
                                object_free(obj);
                                myfclose(fic);
                                return NULL;
                            }
                            for(k=0; k<panim->nbmorphs; k++)
                            {
                                if(myfread(&emorph, 1, sizeof(E_MORPH), fic)!=sizeof(E_MORPH))
                                {
                                    ANIMATERROR;
                                    animat_error=ANIMAT_FILEREADERR;
                                    object_free(obj);
                                    myfclose(fic);
                                    return NULL;
                                }
                                ptrmorph->x=emorph.x;
                                ptrmorph->y=emorph.y;
                                (ptrmorph++)->z=emorph.z;
                            }
                        }

                        if(obj->nbnodes>0)
                        {
                            A_ANIMPOS   *ptranimpos;
                            if((ptranimpos=*(panimpos++)=(A_ANIMPOS *)calloc(obj->nbnodes, sizeof(A_ANIMPOS)))==NULL)
                            {
                                ANIMATERROR;
                                animat_error=ANIMAT_ALLOCMEMERR;
                                object_free(obj);
                                myfclose(fic);
                                return NULL;
                            }
                            for(k=0; k<obj->nbnodes; k++)
                            {
                                if(myfread(&eanimpos, 1, sizeof(E_ANIMPOS), fic)!=sizeof(E_ANIMPOS))
                                {
                                    ANIMATERROR;
                                    animat_error=ANIMAT_FILEREADERR;
                                    object_free(obj);
                                    myfclose(fic);
                                    return NULL;
                                }
                                ptranimpos->rx=eanimpos.rx;
                                ptranimpos->ry=eanimpos.ry;
                                ptranimpos->rz=eanimpos.rz;
                                ptranimpos->tx=eanimpos.tx;
                                ptranimpos->ty=eanimpos.ty;
                                (ptranimpos++)->tz=eanimpos.tz;
                            }
                        }
					}
				}
			}
		}
	}
	myfclose(fic);
	return obj;
}

static void perso_free(A_PERSO * perso)
{
	if(perso)
	{
		int		i;
		if(perso->vertices)
			free(perso->vertices);
        if(perso->vertigo)
            free(perso->vertigo);
		if(perso->faces)
			free(perso->faces);
		for(i=0; i<MAX_OBJECTBYPERSO; i++)
		{
            if(perso->tvertigo[i])
                free(perso->tvertigo[i]);
		}
		if(perso->nodes)
		{
			struct SA_PNODE	*pnode=perso->nodes;
			for(i=0; i<perso->nbnodes; i++)
			{
				if(pnode->vertices)
					free(pnode->vertices);
				pnode++;
			}
			free(perso->nodes);
        }
	}
}

static A_PERSO * perso_create3(A_OBJECT *jambes, A_OBJECT *tronc, A_OBJECT *tete)
{
    A_PERSO     *perso;
    VTX3        *kvtx1[900],*kvtx2[900],*kvtx3[900];
	if(!(jambes&&tronc&&tete))
		return NULL;
	perso=(A_PERSO *)calloc(1, sizeof(A_PERSO));
	if(!perso)
		return NULL;

	perso->nbobjects=3;
	perso->obj[0]=jambes;
	perso->obj[1]=tronc;
	perso->obj[2]=tete;
	perso->anims[0]=jambes->anims;
	perso->anims[1]=tronc->anims;
	perso->anims[2]=tete->anims;
	perso->nbnodesobj[0]=jambes->nbnodes;
	perso->nbnodesobj[1]=tronc->nbnodes;
	perso->nbnodesobj[2]=tete->nbnodes;
    perso->tx=0.f;
    perso->ty=0.f;
    perso->tz=0.f;
    perso->rx=0.f;
    perso->ry=0.f;
    perso->rz=0.f;
    perso->ntx=0.f;
    perso->nty=0.f;
    perso->ntz=0.f;
    perso->nrx=0.f;
    perso->nry=0.f;
    perso->nrz=0.f;

	{	// prepare vertices
		int		i;
        if(!(perso->tvertigo[0]=(VTX3 **)calloc(jambes->nbvertices, sizeof(VTX3 *))))
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			perso_free(perso);
			return NULL;
		}
        if(!(perso->tvertigo[1]=(VTX3 **)calloc(tronc->nbvertices, sizeof(VTX3 *))))
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			perso_free(perso);
			return NULL;
		}
        if(!(perso->tvertigo[2]=(VTX3 **)calloc(tete->nbvertices, sizeof(VTX3 *))))
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			perso_free(perso);
			return NULL;
		}
		{
			int		j;
			int		last=-1;
			int		nbdvtx=0,idvtx=0;
            int     svtx1[900],svtx2[900],svtx3[900];
			char	str[100][ATEXTLEN];
			int		nbstr=0,mpstr[100];

			last=-1;
			for(i=0; i<jambes->nbvertices; i++)
			{
				if(last!=jambes->vertices[i].type)
				{
					last=jambes->vertices[i].type;
					for(j=0; j<nbstr; j++)
					{
						if(!strcmp(jambes->typevtx[last].name, str[j]))
							break;
					}
					if(j==nbstr)
					{
						strcpy(str[nbstr], jambes->typevtx[last].name);
						if(strstr(str[nbstr], "normal"))
							strcat(str[nbstr], "jambes");
						idvtx=nbdvtx;
						mpstr[nbstr]=idvtx;
						nbstr++;
					}
					else
						idvtx=mpstr[j];
				}
				svtx1[i]=idvtx++;
				if(idvtx>nbdvtx)
					nbdvtx++;
			}
			last=-1;
			for(i=0; i<tronc->nbvertices; i++)
			{
				if(last!=tronc->vertices[i].type)
				{
					last=tronc->vertices[i].type;
					for(j=0; j<nbstr; j++)
						if(!strcmp(tronc->typevtx[last].name, str[j]))
							break;
					if(j==nbstr)
					{
						strcpy(str[nbstr], tronc->typevtx[last].name);
						if(strstr(str[nbstr], "normal"))
							strcat(str[nbstr], "tronc");
						idvtx=nbdvtx;
						mpstr[nbstr]=idvtx;
						nbstr++;
					}
					else
						idvtx=mpstr[j];
				}
				svtx2[i]=idvtx++;
				if(idvtx>nbdvtx)
					nbdvtx++;
			}
			last=-1;
			for(i=0; i<tete->nbvertices; i++)
			{
				if(last!=tete->vertices[i].type)
				{
					last=tete->vertices[i].type;
					for(j=0; j<nbstr; j++)
						if(!strcmp(tete->typevtx[last].name, str[j]))
							break;
					if(j==nbstr)
					{
						strcpy(str[nbstr], tete->typevtx[last].name);
						if(strstr(str[nbstr], "normal"))
							strcat(str[nbstr], "tete");
						idvtx=nbdvtx;
						mpstr[nbstr]=idvtx;
						nbstr++;
					}
					else
						idvtx=mpstr[j];
				}
				svtx3[i]=idvtx++;
				if(idvtx>nbdvtx)
					nbdvtx++;
			}
            if(!(perso->vertices=(VTX3 *)calloc(nbdvtx, sizeof(VTX3))))
			{
				ANIMATERROR;
                animat_error=ANIMAT_ALLOCMEMERR;
				perso_free(perso);
				return NULL;
			}
            if(!(perso->vertigo=(VTX3 *)calloc(nbdvtx, sizeof(VTX3))))
			{
				ANIMATERROR;
                animat_error=ANIMAT_ALLOCMEMERR;
				perso_free(perso);
				return NULL;
			}
			for(i=0; i<jambes->nbvertices; i++)
                *(perso->tvertigo[0]+i)=perso->vertigo+svtx1[i];
			for(i=0; i<tronc->nbvertices; i++)
                *(perso->tvertigo[1]+i)=perso->vertigo+svtx2[i];
			for(i=0; i<tete->nbvertices; i++)
                *(perso->tvertigo[2]+i)=perso->vertigo+svtx3[i];
			for(i=0; i<jambes->nbvertices; i++)
                kvtx1[i]=perso->vertices+svtx1[i];
			for(i=0; i<tronc->nbvertices; i++)
                kvtx2[i]=perso->vertices+svtx2[i];
			for(i=0; i<tete->nbvertices; i++)
                kvtx3[i]=perso->vertices+svtx3[i];
			perso->nbvertices=nbdvtx;
		}
	}

	{	// prepare faces
		int			i;
		A_PFACE		*pdface;
		A_FACE		*psface;
		perso->nbfaces=jambes->nbfaces+tronc->nbfaces+tete->nbfaces;
		if(!(pdface=perso->faces=(A_PFACE *)calloc(perso->nbfaces, sizeof(A_PFACE))))
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			perso_free(perso);
			return NULL;
		}
		psface=jambes->faces;
		for(i=0; i<jambes->nbfaces; i++)
		{
            pdface->vertice[0]=kvtx1[psface->vertice[0]];
            pdface->vertice[1]=kvtx1[psface->vertice[1]];
            pdface->vertice[2]=kvtx1[psface->vertice[2]];
			pdface->u[0]=psface->u[0];
			pdface->u[1]=psface->u[1];
			pdface->u[2]=psface->u[2];
			pdface->v[0]=psface->v[0];
			pdface->v[1]=psface->v[1];
			pdface->v[2]=psface->v[2];
			pdface->material=psface->material;
			psface++;
			pdface++;
		}
		psface=tronc->faces;
		for(i=0; i<tronc->nbfaces; i++)
		{
            pdface->vertice[0]=kvtx2[psface->vertice[0]];
            pdface->vertice[1]=kvtx2[psface->vertice[1]];
            pdface->vertice[2]=kvtx2[psface->vertice[2]];
			pdface->u[0]=psface->u[0];
			pdface->u[1]=psface->u[1];
			pdface->u[2]=psface->u[2];
			pdface->v[0]=psface->v[0];
			pdface->v[1]=psface->v[1];
			pdface->v[2]=psface->v[2];
			pdface->material=psface->material;
			psface++;
			pdface++;
		}
		psface=tete->faces;
		for(i=0; i<tete->nbfaces; i++)
		{
            pdface->vertice[0]=kvtx3[psface->vertice[0]];
            pdface->vertice[1]=kvtx3[psface->vertice[1]];
            pdface->vertice[2]=kvtx3[psface->vertice[2]];
			pdface->u[0]=psface->u[0];
			pdface->u[1]=psface->u[1];
			pdface->u[2]=psface->u[2];
			pdface->v[0]=psface->v[0];
			pdface->v[1]=psface->v[1];
			pdface->v[2]=psface->v[2];
			pdface->material=psface->material;
			psface++;
			pdface++;
		}
	}

	{	// prepare nodes
		int			i,j;
		A_PNODE		*pere[100];
		A_PNODE		*pdnodepere=NULL;
		A_PNODE		*pdnode;
		A_NODE		*psnode;
		A_PNODEVTX	*pdvtx;
		A_NODEVTX	*psvtx;
		perso->nbnodes=jambes->nbnodes+tronc->nbnodes+tete->nbnodes;
		if(!(pdnode=perso->nodes=(A_PNODE *)calloc(perso->nbnodes, sizeof(A_PNODE))))
		{
			ANIMATERROR;
            animat_error=ANIMAT_ALLOCMEMERR;
			perso_free(perso);
			return NULL;
		}
		psnode=jambes->nodes;
        perso->firstmat[0]=&(pdnode->matrice);
		for(i=0; i<jambes->nbnodes; i++)
		{
			pdnode->nodeobj=i;
			pdnode->nobj=0;
			if(psnode->pere==-1)
				pdnode->pere=pdnodepere;
			else
				pdnode->pere=pere[psnode->pere];
			if(psnode->code&2)
				pdnodepere=pdnode;
			if(pdnode->nbvertices=psnode->nbvertices)
			{
				if(!(pdvtx=pdnode->vertices=(A_PNODEVTX *)calloc(psnode->nbvertices, sizeof(A_PNODEVTX))))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_ALLOCMEMERR;
					perso_free(perso);
					return NULL;
				}
				psvtx=psnode->vertices;
				for(j=0; j<psnode->nbvertices; j++)
				{
                    pdvtx->vertice=*(perso->tvertigo[0]+psvtx->vertice);
					pdvtx->weight=psvtx->weight;
                    pdvtx->x=psvtx->x;
                    pdvtx->y=psvtx->y;
                    pdvtx->z=psvtx->z;
					psvtx++;
					pdvtx++;
				}
			}
			psnode++;
			pere[i]=pdnode;
			pdnode++;
		}
		psnode=tronc->nodes;
        perso->firstmat[1]=&pdnode->matrice;
		for(i=0; i<tronc->nbnodes; i++)
		{
			pdnode->nodeobj=i;
			pdnode->nobj=1;
			if(psnode->pere==-1)
				pdnode->pere=pdnodepere;
			else
				pdnode->pere=pere[psnode->pere];
			if(psnode->code&2)
				pdnodepere=pdnode;
			if(pdnode->nbvertices=psnode->nbvertices)
			{
				if(!(pdvtx=pdnode->vertices=(A_PNODEVTX *)calloc(psnode->nbvertices, sizeof(A_PNODEVTX))))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_ALLOCMEMERR;
					perso_free(perso);
					return NULL;
				}
				psvtx=psnode->vertices;
				for(j=0; j<psnode->nbvertices; j++)
				{
                    pdvtx->vertice=*(perso->tvertigo[1]+psvtx->vertice);
					pdvtx->weight=psvtx->weight;
                    pdvtx->x=psvtx->x;
                    pdvtx->y=psvtx->y;
                    pdvtx->z=psvtx->z;
					psvtx++;
					pdvtx++;
				}
			}
			psnode++;
			pere[i]=pdnode;
			pdnode++;
		}
		psnode=tete->nodes;
        perso->firstmat[2]=&pdnode->matrice;
		for(i=0; i<tete->nbnodes; i++)
		{
			pdnode->nodeobj=i;
			pdnode->nobj=2;
			if(psnode->pere==-1)
				pdnode->pere=pdnodepere;
			else
				pdnode->pere=pere[psnode->pere];
			if(psnode->code&2)
				pdnodepere=pdnode;
			if(pdnode->nbvertices=psnode->nbvertices)
			{
				if(!(pdvtx=pdnode->vertices=(A_PNODEVTX *)calloc(psnode->nbvertices, sizeof(A_PNODEVTX))))
				{
					ANIMATERROR;
                    animat_error=ANIMAT_ALLOCMEMERR;
					perso_free(perso);
					return NULL;
				}
				psvtx=psnode->vertices;
				for(j=0; j<psnode->nbvertices; j++)
				{
                    pdvtx->vertice=*(perso->tvertigo[2]+psvtx->vertice);
					pdvtx->weight=psvtx->weight;
                    pdvtx->x=psvtx->x;
                    pdvtx->y=psvtx->y;
                    pdvtx->z=psvtx->z;
					psvtx++;
					pdvtx++;
				}
			}
			psnode++;
			pere[i]=pdnode;
			pdnode++;
		}
	}
	return perso;
}

void Aanimat::animat_loadtexture()
{
	static char		*ltex[]={	MAKEINTRESOURCE(PNG_BEN),
								MAKEINTRESOURCE(PNG_GRADE),
								MAKEINTRESOURCE(PNG_HELMUT),
								MAKEINTRESOURCE(PNG_JACK) };
	static char		*stex[]={	"ben",
								"grade",
								"helmut",
								"jack" };
	int				i;

	glGenTextures(NBTEXTURES, gltex);

	for(i=0; i<NBTEXTURES; i++)
	{
		Aresobj	o=resdll.get(ltex[i], "PNG");
		Abitmap	*tex=new Abitmap(&o);

		glBindTexture(GL_TEXTURE_2D, gltex[i]);
		AoglBitmap::texture(tex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		strcpy(textures[nbtextures].name, stex[i]);

		textures[nbtextures].xdep=0;
		textures[nbtextures].ydep=0;
		textures[nbtextures].width=tex->w;
		textures[nbtextures].height=tex->h;

		textures[nbtextures].gltex=gltex[i];

		nbtextures++;
		delete(tex);
	}
}

static int face_oriente(A_PFACE *face)
{
	VTX3			*v0=face->vertice[0];
	VTX3			*v1=face->vertice[1];
	VTX3			*v2=face->vertice[2];
	register float	n0,n1,n2;

	n0=(v0->y*65536)+v0->x;
	n1=(v1->y*65536)+v1->x;
	n2=(v2->y*65536)+v2->x;

	if((n0<=n1)&&(n0<=n2))
		return TRUE;

	n2=n1;
	n1=n0;
	n0=n2;
	if((n0<=n1)&&(n0<=n2))
	{
		face->vertice[2]=v0;
		face->vertice[0]=v1;
		face->vertice[1]=v2;
		return TRUE;
	}

	n2=n1;
	n1=n0;
	n0=n2;
	if((n0<=n1)&&(n0<=n2))
	{
		face->vertice[1]=v0;
		face->vertice[2]=v1;
		face->vertice[0]=v2;
		return TRUE;
	}

	return FALSE;
}

static void perso_projete(A_PNODE *pnode)
{
    struct RMATRICE     m1=pnode->matrice;
    int                 j;
    A_PNODEVTX          *pnodevtx=pnode->vertices;
	for(j=0; j<pnode->nbvertices; j++)
	{
        VTX3         *pdvtx=pnodevtx->vertice;
        float       sx=pnodevtx->x;
        float       sy=pnodevtx->y;
        float       sz=pnodevtx->z;
        float       we=pnodevtx->weight;
        pdvtx->x+=(m1._11*sx+m1._21*sy+m1._31*sz+m1._41)*we;
        pdvtx->y+=(m1._12*sx+m1._22*sy+m1._32*sz+m1._42)*we;
        pdvtx->z+=(m1._13*sx+m1._23*sy+m1._33*sz+m1._43)*we;
		pnodevtx++;
	}
}

static int perso_sendface(A_PERSO *perso)
{
    int         i;
    A_PFACE     *face=perso->faces;
    VTX3        *pdvtx=perso->vertices;
    VTX3        *psvtx=perso->vertigo;

    for(i=0; i<perso->nbvertices; i++)
    {
	/*
		int		ix=(int)(400.f*psvtx->x/(psvtx->z+256.f)+160.f);
		int		iy=(int)(-400.f*psvtx->y/(psvtx->z+256.f)+100.f);
		int		iz=(int)psvtx->z;
		if(iz>=0&&iz<65000)
			//fastsprite(theimage, persozb[i], 10, ix, iy, iz);
		psvtx++;
	*/

        pdvtx->x=psvtx->x*0.02f;//(256.f*dx/(dz+256.f)+200.f);
        pdvtx->y=-psvtx->y*0.02f;//(256.f*dy/(dz+256.f)+200.f);
        pdvtx->z=psvtx->z*0.02f;//dz;
        psvtx++;
        pdvtx++;
    }

    for(i=0; i<perso->nbfaces; i++)
	{
		//if(face_oriente(face))
		{
            A_MATERIAL	*mat=face->material;
			VTX3		*vtx;

			if(mat->tex)
			{
				glBindTexture(GL_TEXTURE_2D, mat->tex->gltex);
			}

			glBegin(GL_TRIANGLES);
				{
					VTX3	*vtx0=face->vertice[0];
					VTX3	*vtx1=face->vertice[1];
					VTX3	*vtx2=face->vertice[2];
					float	u0=vtx1->x-vtx0->x;
					float	u1=vtx1->y-vtx0->y;
					float	u2=vtx1->z-vtx0->z;
					float	v0=vtx2->x-vtx0->x;
					float	v1=vtx2->y-vtx0->y;
					float	v2=vtx2->z-vtx0->z;

					float	x=u1*v2-u2*v1;
					float	y=u2*v0-u0*v2;
					float	z=u0*v1-u1*v0;

					float	d=1.f/(float)sqrt(x*x+y*y+z*z);
					x*=d;
					y*=d;
					z*=d;

					glNormal3f(x, y, z);
				}



				if(mat->tex)
				{
					float	iw=1.f/(float)mat->tex->width;
					float	ih=1.f/(float)mat->tex->height;


					GLfloat		v[4];
					float		m=1.f/255.f;

					v[0]=(float)0.2f;
					v[1]=(float)0.2f;
					v[2]=(float)0.2f;
					v[3]=(float)1.f;
					glMaterialfv(GL_FRONT, GL_AMBIENT, v);

					v[0]=(float)1.f;
					v[1]=(float)1.f;
					v[2]=(float)1.f;
					v[3]=(float)1.f;
					glMaterialfv(GL_FRONT, GL_DIFFUSE, v);

					v[0]=(float)1.f;
					v[1]=(float)1.f;
					v[2]=(float)1.f;
					v[3]=(float)1.f;
					glMaterialfv(GL_FRONT, GL_SPECULAR, v);
					//glColor3f(1.f, 1.f, 1.f);

					glTexCoord2f((float)face->u[0]*iw, 1.f-(float)face->v[0]*ih);
					vtx=face->vertice[0];
					glVertex3f(vtx->x, vtx->y,  vtx->z);

					glTexCoord2f((float)face->u[1]*iw, 1.f-(float)face->v[1]*ih);
					vtx=face->vertice[1];
					glVertex3f(vtx->x, vtx->y,  vtx->z);

					glTexCoord2f((float)face->u[2]*iw, 1.f-(float)face->v[2]*ih);
					vtx=face->vertice[2];
					glVertex3f(vtx->x, vtx->y,  vtx->z);
				}
				else
				{

					GLfloat		v[4];
					float		m=1.f/255.f;

					v[0]=(float)mat->ar*m;
					v[1]=(float)mat->ag*m;
					v[2]=(float)mat->ab*m;
					v[3]=(float)mat->aa*m;
					glMaterialfv(GL_FRONT, GL_AMBIENT, v);

					v[0]=(float)mat->dr*m;
					v[1]=(float)mat->dg*m;
					v[2]=(float)mat->db*m;
					v[3]=(float)mat->da*m;
					glMaterialfv(GL_FRONT, GL_DIFFUSE, v);

					v[0]=(float)mat->sr*m;
					v[1]=(float)mat->sg*m;
					v[2]=(float)mat->sb*m;
					v[3]=(float)mat->sa*m;
					glMaterialfv(GL_FRONT, GL_SPECULAR, v);

					//glColor3f((float)mat->dr/255.f, (float)mat->dg/255.f, (float)mat->db/255.f);
					vtx=face->vertice[0];
					glVertex3f(vtx->x, vtx->y,  vtx->z);

					//glColor3f(1.f, 0.18f, 0.18f);
					vtx=face->vertice[1];
					glVertex3f(vtx->x, vtx->y,  vtx->z);

					//glColor3f(0.18f, 1.f, 0.18f);
					vtx=face->vertice[2];
					glVertex3f(vtx->x, vtx->y,  vtx->z);
				}

			glEnd();
			

			/*
			if(mat->bitmap)
			{
				poly->p0=face->vertice[0];
				poly->p1=face->vertice[1];
				poly->p2=face->vertice[2];
				poly->u0=face->u[0];
				poly->u1=face->u[1];
				poly->u2=face->u[2];
				poly->v0=face->v[0];
				poly->v1=face->v[1];
				poly->v2=face->v[2];
				poly->texture=mat->bitmap;
				poly->palette=(CVECTOR *)((ulong)(GetCuePalette(10,mat->palette)));
				RenderAddPrim((POLY *)poly, poly->p0->vy);
			}
			else
            {
                GrVertex vtx[4];
                vtx[0].tmuvtx[0].sow = 0.f;
                vtx[0].tmuvtx[0].tow = 255.f;
                vtx[0].oow = 1.f;
                vtx[0].r = 45.f;
                vtx[0].g = 45.f;
                vtx[0].b = 245.f;
                vtx[0].a = 255.f;
                vtx[1].tmuvtx[0].sow = 0.f;
                vtx[1].tmuvtx[0].tow = 255.f;
                vtx[1].oow = 1.f;
                vtx[1].r = 245.f;
                vtx[1].g = 45.f;
                vtx[1].b = 45.f;
                vtx[1].a = 255.f;
                vtx[2].tmuvtx[0].sow = 0.f;
                vtx[2].tmuvtx[0].tow = 255.f;
                vtx[2].oow = 1.f;
                vtx[2].r = 45.f;
                vtx[2].g = 245.f;
                vtx[2].b = 45.f;
                vtx[2].a = 255.f;
                vtx[0].x=face->vertice[0]->x;
                vtx[0].y=face->vertice[0]->y;
                vtx[1].x=face->vertice[1]->x;
                vtx[1].y=face->vertice[1]->y;
                vtx[2].x=face->vertice[2]->x;
                vtx[2].y=face->vertice[2]->y;
                vtx[0].z=0;
                vtx[1].z=0;
                vtx[2].z=0;
                grAADrawLine(&vtx[0],  &vtx[1]);
                //grAADrawTriangle(&vtx[0],  &vtx[1], &vtx[2], FXTRUE, FXTRUE, FXTRUE);
                //guDrawTriangleWithClip(&vtx[0], &vtx[1], &vtx[2]);
            }
			*/
			/*
            {
                line((int)face->vertice[0]->x, (int)face->vertice[0]->y,
                     (int)face->vertice[1]->x, (int)face->vertice[1]->y, 200);
                line((int)face->vertice[2]->x, (int)face->vertice[2]->y,
                     (int)face->vertice[1]->x, (int)face->vertice[1]->y, 200);
                line((int)face->vertice[0]->x, (int)face->vertice[0]->y,
                     (int)face->vertice[2]->x, (int)face->vertice[2]->y, 200);
            }
			*/
        }
		face++;
	}
    return TRUE;
}

int Aanimat::perso_render(A_PERSO *perso)
{
    int     i;
	if(!perso)
		return FALSE;
    {
        struct RMATRICE monde=matrice_identity;
        memset(perso->vertigo, 0, perso->nbvertices*sizeof(VTX3));
        {
            struct RMATRICE m=matrice_identity;
            matrice_setrotation(&m, cam_rx, cam_ry, cam_rz);
            matrice_settranslation(&monde, cam_tx, cam_ty, cam_tz);
            monde=matrice_multiply(monde, m);
        }
        if(perso->nextanim[0])
        {
			if(perso->currentanim[0])
            {   //  nouvelle et ancienne anims, gestion des transitions  (a faire $$$$$$$$$$$$$$$$$$$$$$$)
                float      t=animat_time-perso->starttime;
                if(t>=(perso->currentanim[0]->time[perso->currentanim[0]->nbkeys-1]))
                {
                    perso->starttime+=perso->currentanim[0]->time[perso->currentanim[0]->nbkeys-1];
                    perso->tx=perso->ntx;
                    perso->ty=perso->nty;
                    perso->tz=perso->ntz;
                    perso->rx=perso->nrx;
                    perso->ry=perso->nry;
                    perso->rz=perso->nrz;
                    perso->ntx=0.f;
                    perso->nty=0.f;
                    perso->ntz=0.f;
                    perso->nrx=0.f;
                    perso->nry=0.f;
                    perso->nrz=0.f;
                    for(i=0; i<MAX_OBJECTBYPERSO; i++)
                    {
                        perso->currentanim[i]=perso->nextanim[i];
                        perso->nextanim[i]=0;
                    }
                    goto anim_norm;
                }
                else if(t>=(perso->currentanim[0]->time[perso->currentanim[0]->nbkeys-2]))
                {
                    int             lkey[MAX_OBJECTBYPERSO],rkey[MAX_OBJECTBYPERSO];
                    float          w[MAX_OBJECTBYPERSO];
                    float          t=animat_time-perso->starttime;
                    float          tt;
                    A_PNODE         *pnode=perso->nodes;
                    {
                        int     j;
                        A_ANIM  *panim=perso->currentanim[0];
                        tt=t/panim->time[panim->nbkeys-1];
                        for(j=1; j<panim->nbkeys; j++)
                            if(panim->time[j]>t)
                                break;
                        lkey[0]=j-1;
                        rkey[0]=j;
                        w[0]=1-(t-panim->time[j-1])/(panim->time[j]-panim->time[j-1]);
                    }
                    for(i=1; i<perso->nbobjects; i++)
                    {
                        int     j;
                        A_ANIM  *panim=perso->currentanim[i];
                        float ttt=tt*panim->time[panim->nbkeys-1];
                        for(j=1; j<panim->nbkeys; j++)
                            if(panim->time[j]>ttt)
                                break;
                        lkey[i]=j-1;
                        rkey[i]=j;
                        w[i]=1-(ttt-panim->time[j-1])/(panim->time[j]-panim->time[j-1]);
                    }
                    for(i=0; i<perso->nbnodes; i++)
                    {
                        A_ANIM  *panim=perso->currentanim[pnode->nobj];
                        A_ANIM  *panim2=perso->nextanim[pnode->nobj];
                        if(panim)
                        if((panim->nbkeys-2)<=lkey[pnode->nobj])
                        {
                            struct RMATRICE m=matrice_identity;
                            A_ANIMPOS       *panimposl=*(panim->animpos+lkey[pnode->nobj])+pnode->nodeobj;
                            A_ANIMPOS       *panimposr=*(panim2->animpos)+pnode->nodeobj;
                            float          ww=w[pnode->nobj];
                            float          wm=1-ww;
                            matrice_setrotation(&m, panimposl->rx*ww+panimposr->rx*wm, panimposl->ry*ww+panimposr->ry*wm, panimposl->rz*ww+panimposr->rz*wm);
                            matrice_settranslation(&m, panimposl->tx*ww+panimposr->tx*wm, panimposl->ty*ww+panimposr->ty*wm, panimposl->tz*ww+panimposr->tz*wm);
                            if(pnode->pere)
                            {
                                pnode->matrice=matrice_multiply(m, pnode->pere->matrice);
                            }
                            else
                            {
                                struct RMATRICE m2=matrice_identity;
                                matrice_settranslation(&m2, perso->tx*ww+perso->ntx*wm, perso->ty*ww+perso->nty*wm, perso->tz*ww+perso->ntz*wm);
                                matrice_setrotation(&m2, perso->rx*ww+perso->nrx*wm, perso->ry*ww+perso->nry*wm, perso->rz*ww+perso->nrz*wm);
                                monde=matrice_multiply(m2, monde);
                                pnode->matrice=matrice_multiply(m, monde);
                            }
                            perso_projete(pnode);
                        }
                        else
                        {
                            struct RMATRICE m=matrice_identity;
                            A_ANIMPOS       *panimposl=*(panim->animpos+lkey[pnode->nobj])+pnode->nodeobj;
                            A_ANIMPOS       *panimposr=*(panim->animpos+rkey[pnode->nobj])+pnode->nodeobj;
                            float           ww=w[pnode->nobj];
                            float           wm=1-ww;
                            matrice_setrotation(&m, panimposl->rx*ww+panimposr->rx*wm, panimposl->ry*ww+panimposr->ry*wm, panimposl->rz*ww+panimposr->rz*wm);
                            matrice_settranslation(&m, panimposl->tx*ww+panimposr->tx*wm, panimposl->ty*ww+panimposr->ty*wm, panimposl->tz*ww+panimposr->tz*wm);
                            if(pnode->pere)
                            {
                                pnode->matrice=matrice_multiply(m, pnode->pere->matrice);
                            }
                            else
                            {
                                struct RMATRICE m2=matrice_identity;
                                matrice_settranslation(&m2, perso->tx, perso->ty, perso->tz);
                                matrice_setrotation(&m2, perso->rx, perso->ry, perso->rz);
                                monde=matrice_multiply(m2, monde);
                                pnode->matrice=matrice_multiply(m, monde);
                            }
                            perso_projete(pnode);
                        }
                        pnode++;
                    }

                    {
                        for(i=0; i<perso->nbobjects; i++)
                        {
                            A_ANIM      *paniml=perso->currentanim[i];
                            A_ANIM      *panimr=perso->nextanim[i];
                            A_IMORPH    *pimorphl=paniml->imorphs;
                            A_IMORPH    *pimorphr=panimr->imorphs;
                            A_MORPH     *pmorphl=paniml->morphs[lkey[i]];
                            A_MORPH     *pmorphr=panimr->morphs[0];
                            int         nl=paniml->nbmorphs;
                            int         nr=panimr->nbmorphs;
                            float       we=w[i];
                            float       web=1-we;
                            while(nr&&nl)
                            {
                                if(pimorphl->vertice==pimorphr->vertice)
                                {
                                    {
                                        VTX3                *pdvtx=*(perso->tvertigo[i]+pimorphl->vertice);
                                        struct RMATRICE     *mat=perso->firstmat[i];
                                        float               x=pmorphl->x*we+pmorphr->x*web;
                                        float               y=pmorphl->y*we+pmorphr->y*web;
                                        float               z=pmorphl->z*we+pmorphr->z*web;
                                        pdvtx->x=mat->_11*x+mat->_21*y+mat->_31*z+mat->_41;
                                        pdvtx->y=mat->_12*x+mat->_22*y+mat->_32*z+mat->_42;
                                        pdvtx->z=mat->_13*x+mat->_23*y+mat->_33*z+mat->_43;
                                    }
                                    pmorphl++;
                                    pmorphr++;
                                    pimorphl++;
                                    pimorphr++;
                                    nl--;
                                    nr--;
                                }
                                else if(pimorphl->vertice>pimorphr->vertice)
                                {
                                    {
                                        VTX3                *pdvtx=*(perso->tvertigo[i]+pimorphr->vertice);
                                        struct RMATRICE     *mat=perso->firstmat[i];
                                        float               x=pmorphr->x;
                                        float               y=pmorphr->y;
                                        float               z=pmorphr->z;
                                        pdvtx->x=(mat->_11*x+mat->_21*y+mat->_31*z+mat->_41)*web+pdvtx->x*we;
                                        pdvtx->y=(mat->_12*x+mat->_22*y+mat->_32*z+mat->_42)*web+pdvtx->y*we;
                                        pdvtx->z=(mat->_13*x+mat->_23*y+mat->_33*z+mat->_43)*web+pdvtx->z*we;
                                    }
                                    pmorphr++;
                                    pimorphr++;
                                    nr--;
                                }
                                else
                                {
                                    {
                                        VTX3                *pdvtx=*(perso->tvertigo[i]+pimorphl->vertice);
                                        struct RMATRICE     *mat=perso->firstmat[i];
                                        float               x=pmorphl->x;
                                        float               y=pmorphl->y;
                                        float               z=pmorphl->z;
                                        pdvtx->x=(mat->_11*x+mat->_21*y+mat->_31*z+mat->_41)*we+pdvtx->x*web;
                                        pdvtx->y=(mat->_12*x+mat->_22*y+mat->_32*z+mat->_42)*we+pdvtx->y*web;
                                        pdvtx->z=(mat->_13*x+mat->_23*y+mat->_33*z+mat->_43)*we+pdvtx->z*web;
                                    }
                                    pmorphl++;
                                    pimorphl++;
                                    nl--;
                                }
                            }
                            while(nr--)
                            {
                                {
                                    VTX3                *pdvtx=*(perso->tvertigo[i]+pimorphr->vertice);
                                    struct RMATRICE     *mat=perso->firstmat[i];
                                    float               x=pmorphr->x;
                                    float               y=pmorphr->y;
                                    float               z=pmorphr->z;
                                    pdvtx->x=(mat->_11*x+mat->_21*y+mat->_31*z+mat->_41)*web+pdvtx->x*we;
                                    pdvtx->y=(mat->_12*x+mat->_22*y+mat->_32*z+mat->_42)*web+pdvtx->y*we;
                                    pdvtx->z=(mat->_13*x+mat->_23*y+mat->_33*z+mat->_43)*web+pdvtx->z*we;
                                }
                                pmorphr++;
                                pimorphr++;
                            }
                            while(nl--)
                            {
                                {
                                    VTX3                *pdvtx=*(perso->tvertigo[i]+pimorphl->vertice);
                                    struct RMATRICE     *mat=perso->firstmat[i];
                                    float               x=pmorphl->x;
                                    float               y=pmorphl->y;
                                    float               z=pmorphl->z;
                                    pdvtx->x=(mat->_11*x+mat->_21*y+mat->_31*z+mat->_41)*we+pdvtx->x*web;
                                    pdvtx->y=(mat->_12*x+mat->_22*y+mat->_32*z+mat->_42)*we+pdvtx->y*web;
                                    pdvtx->z=(mat->_13*x+mat->_23*y+mat->_33*z+mat->_43)*we+pdvtx->z*web;
                                }
                                pmorphl++;
                                pimorphl++;
                            }
                        }
                    }
                    perso_sendface(perso);
                }
                else
                {
                    goto anim_norm;
                }
			}
            else
            {   // nouvelle anim, c'est une premiere, pas d'anciene pas de transistion
                for(i=0; i<perso->nbobjects; i++)
                {
                    perso->currentanim[i]=perso->nextanim[i];
                    perso->nextanim[i]=NULL;
                }
                perso->starttime=perso->nexttime;
                perso->nexttime=0.f;
                {
                    A_PNODE     *pnode=perso->nodes;
                    for(i=0; i<perso->nbnodes; i++)
                    {
                        A_ANIM  *panim=perso->currentanim[pnode->nobj];
                        if(panim)
                        {
                            struct RMATRICE m=matrice_identity;
                            A_ANIMPOS       *panimpos=*(panim->animpos)+pnode->nodeobj;
                            matrice_setrotation(&m, panimpos->rx, panimpos->ry, panimpos->rz);
                            matrice_settranslation(&m, panimpos->tx, panimpos->ty, panimpos->tz);
                            if(pnode->pere)
                                pnode->matrice=matrice_multiply(m, pnode->pere->matrice);
                            else
                            {
                                struct RMATRICE m2=matrice_identity;
                                matrice_settranslation(&m2, perso->tx, perso->ty, perso->tz);
                                matrice_setrotation(&m2, perso->rx, perso->ry, perso->rz);
                                monde=matrice_multiply(m2, monde);
                                pnode->matrice=matrice_multiply(m, monde);
                            }
                            perso_projete(pnode);
                        }
                        pnode++;
                    }
                    perso_sendface(perso);
                }
            }
		}
		else
		{
			if(perso->currentanim[0])
            {
                if((perso->currentanim[0]->time[perso->currentanim[0]->nbkeys-1])<=(animat_time-perso->starttime))
                {   // anim fini et pas de nouvelle envoie la derniere keyframe au rendu
	                A_PNODE     *pnode=perso->nodes;
                    for(i=0; i<perso->nbnodes; i++)
                    {
                        A_ANIM  *panim=perso->currentanim[pnode->nobj];
                        if(panim)
                        {
                            struct RMATRICE m=matrice_identity;
                            A_ANIMPOS       *panimpos;
                            panimpos=*(panim->animpos+panim->nbkeys-1)+pnode->nodeobj;
                            matrice_setrotation(&m, panimpos->rx, panimpos->ry, panimpos->rz);
                            matrice_settranslation(&m, panimpos->tx, panimpos->ty, panimpos->tz);
                            if(pnode->pere)
                                pnode->matrice=matrice_multiply(m, pnode->pere->matrice);
                            else
                            {
                                struct RMATRICE m2=matrice_identity;
                                matrice_settranslation(&m2, perso->tx, perso->ty, perso->tz);
                                matrice_setrotation(&m2, perso->rx, perso->ty, perso->rz);
                                monde=matrice_multiply(m2, monde);
                                pnode->matrice=matrice_multiply(m, monde);
                            }
                            perso_projete(pnode);
                        }
                        pnode++;
                    }
                    perso_sendface(perso);
                }
                else
                anim_norm:
                {   //  milieu d'anim, pas encore de nouvelle            (a faire $$$$$$$$$$$$$$$$$$$$$$$)
                    int       lkey[MAX_OBJECTBYPERSO],rkey[MAX_OBJECTBYPERSO];
                    float     w[MAX_OBJECTBYPERSO];
                    float     t=animat_time-perso->starttime;
                    float     tt;
                    A_PNODE    *pnode=perso->nodes;
                    {
                        int     j;
                        A_ANIM  *panim=perso->currentanim[0];
                        tt=t/panim->time[panim->nbkeys-1];
                        for(j=1; j<panim->nbkeys; j++)
                            if(panim->time[j]>t)
                                break;
                        lkey[0]=j-1;
                        rkey[0]=j;
                        w[0]=1-(t-panim->time[j-1])/(panim->time[j]-panim->time[j-1]);
                    }
                    for(i=1; i<perso->nbobjects; i++)
                    {
                        int     j;
                        A_ANIM  *panim=perso->currentanim[i];
                        float ttt=tt*panim->time[panim->nbkeys-1];
                        for(j=1; j<panim->nbkeys; j++)
                            if(panim->time[j]>ttt)
                                break;
                        lkey[i]=j-1;
                        rkey[i]=j;
                        w[i]=1-(ttt-panim->time[j-1])/(panim->time[j]-panim->time[j-1]);
                    }
                    for(i=0; i<perso->nbnodes; i++)
                    {
                        A_ANIM  *panim=perso->currentanim[pnode->nobj];
                        if(panim)
                        {
                            struct RMATRICE m=matrice_identity;
                            A_ANIMPOS       *panimposl=*(panim->animpos+lkey[pnode->nobj])+pnode->nodeobj;
                            A_ANIMPOS       *panimposr=*(panim->animpos+rkey[pnode->nobj])+pnode->nodeobj;
                            float          ww=w[pnode->nobj];
                            float          wm=1-ww;
                            matrice_setrotation(&m, panimposl->rx*ww+panimposr->rx*wm, panimposl->ry*ww+panimposr->ry*wm, panimposl->rz*ww+panimposr->rz*wm);
                            matrice_settranslation(&m, panimposl->tx*ww+panimposr->tx*wm, panimposl->ty*ww+panimposr->ty*wm, panimposl->tz*ww+panimposr->tz*wm);
                            if(pnode->pere)
                            {
                                pnode->matrice=matrice_multiply(m, pnode->pere->matrice);
                            }
                            else
                            {
                                struct RMATRICE m2=matrice_identity;
                                matrice_settranslation(&m2, perso->tx, perso->ty, perso->tz);
                                matrice_setrotation(&m2, perso->rx, perso->ry, perso->rz);
                                monde=matrice_multiply(m2, monde);
                                pnode->matrice=matrice_multiply(m, monde);
                            }
                            perso_projete(pnode);
                        }
                        pnode++;
                    }

                    {
                        int     j;
                        for(i=0; i<perso->nbobjects; i++)
                        {
                            A_ANIM      *panim=perso->currentanim[i];
                            A_IMORPH    *pimorph=panim->imorphs;
                            A_MORPH     *pmorphl=panim->morphs[lkey[i]];
                            A_MORPH     *pmorphr=panim->morphs[rkey[i]];
                            float       we=w[i];
                            float       web=1-we;
                            for(j=0; j<panim->nbmorphs; j++)
                            {
                                VTX3                *pdvtx=*(perso->tvertigo[i]+pimorph->vertice);
                                struct RMATRICE     *mat=perso->firstmat[i];
                                float               x=pmorphl->x*we+pmorphr->x*web;
                                float               y=pmorphl->y*we+pmorphr->y*web;
                                float               z=pmorphl->z*we+pmorphr->z*web;
                                pdvtx->x=mat->_11*x+mat->_21*y+mat->_31*z+mat->_41;
                                pdvtx->y=mat->_12*x+mat->_22*y+mat->_32*z+mat->_42;
                                pdvtx->z=mat->_13*x+mat->_23*y+mat->_33*z+mat->_43;
                                pimorph++;
                                pmorphl++;
                                pmorphr++;
                            }
                        }
                    }
					perso_sendface(perso);
                }
			}
		}

	}
	return TRUE;
}

// fonctions internes					(end)
//
////////////////////////////////////////////////


////////////////////////////////////////////////
//
// fonctions externes					(begin)

void Aanimat::animat_clear()
{
	int	i;
	for(i=0; i<MAX_OBJECTS; i++)
		if(objects[i])
		{
			object_free(objects[i]);
			objects[i]=NULL;
		}
	for(i=0; i<MAX_PERSOS; i++)
		if(persos[i])
		{
			perso_free(persos[i]);
			persos[i]=NULL;
		}
	nbobjects=0;
	nbpersos=0;
}

int Aanimat::object_create3(int *hp, int i1, int i2, int i3)
{
	int		i;
	if((i1<0)||(i1>=MAX_OBJECTS)||(i2<0||i2>=MAX_OBJECTS)||(i3<0||i3>=MAX_OBJECTS))
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		*hp=-1;
		return FALSE;
	}
	if(!(objects[i1]&&objects[i2]&&objects[i3]))
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		*hp=-1;
		return FALSE;
	}
	for(i=0; i<MAX_PERSOS; i++)
		if(!persos[i])
			break;
	if(i==MAX_PERSOS)
	{
		ANIMATERROR;
        animat_error=ANIMAT_MAXIMUMERR;
		*hp=-1;
		return FALSE;
	}
	if(!(persos[i]=perso_create3(objects[i1], objects[i2], objects[i3])))
	{
		*hp=-1;
		return FALSE;
	}
	*hp=i;
	nbpersos++;
	return TRUE;
}

int Aanimat::object_clear(int hp)
{
	if(!persos[hp])
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
	perso_free(persos[hp]);
	persos[hp]=NULL;
	nbpersos--;
	return TRUE;
}

int Aanimat::object_play3(int hp, int anim1, int anim2, int anim3, int mode, float tx, float ty, float tz, float rx, float ry, float rz)
{
    A_PERSO *perso=persos[hp];
    if((hp<0)||(hp>MAX_PERSOS)||(anim1<0)||(anim1>=NB_ANIMS)||(anim2<0)||(anim2>=NB_ANIMS)||(anim3<0)||(anim3>=NB_ANIMS))
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
	if(!perso)
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
    if(perso->nbobjects!=3)
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
    perso->nextanim[0]=*(perso->obj[0]->anims+anim1);
    perso->nextanim[1]=*(perso->obj[1]->anims+anim2);
    perso->nextanim[2]=*(perso->obj[2]->anims+anim3);
    perso->nexttime=animat_time;
    perso->modeanim=mode;
    perso->ntx=tx;
    perso->nty=ty;
    perso->ntz=tz;
    perso->nrx=rx;
    perso->nry=ry;
    perso->nrz=rz;
	return TRUE;
}

int Aanimat::object_play(int hp, int anim, int mode, float tx, float ty, float tz, float rx, float ry, float rz)
{
    int     i;
    A_PERSO *perso=persos[hp];
	if((hp<0)||(hp>MAX_PERSOS)||(anim<0)||(anim>=NB_ANIMS))
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
	if(!perso)
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
    /*
    for(i=0; i<perso->nbobjects; i++)   // 4 debug
        perso->currentanim[i]=NULL;     // 4 debug
    perso->starttime=animat_time;       // 4 debug
    perso->tx=tx;                       // 4 debug
    perso->ty=ty;                       // 4 debug
    perso->tz=tz;                       // 4 debug
    perso->rx=rx;                       // 4 debug
    perso->ry=ry;                       // 4 debug
    perso->rz=rz;                       // 4 debug
    */

    for(i=0; i<perso->nbobjects; i++)
        perso->nextanim[i]=*(perso->obj[i]->anims+anim);
    perso->nexttime=animat_time;
    perso->modeanim=mode;
    if(perso->modeanim&ANIMAT_RECALAUTO)
    {
        if(perso->currentanim[0])
        {
            A_ANIMPOS   *p1=perso->currentanim[0]->animpos[0];
            A_ANIMPOS   *p2=perso->currentanim[0]->animpos[perso->currentanim[0]->nbkeys-1];
            perso->ntx=perso->tx+p2->tx-p1->tx;
            perso->nty=perso->ty+p2->ty-p1->ty;
            perso->ntz=perso->tz+p2->tz-p1->tz;
            perso->nrx=perso->rx+p2->rx-p1->rx;
            perso->nry=perso->ry+p2->ry-p1->ry;
            perso->nrz=perso->rz+p2->rz-p1->rz;
        }
        else
        {
            perso->ntx=tx;
            perso->nty=ty;
            perso->ntz=tz;
            perso->nrx=rx;
            perso->nry=ry;
            perso->nrz=rz;
        }
    }
    else
    {
        perso->ntx=tx;
        perso->nty=ty;
        perso->ntz=tz;
        perso->nrx=rx;
        perso->nry=ry;
        perso->nrz=rz;
    }
	return TRUE;
}

int Aanimat::item_load(int *ip, int name)
{
	int		i;
	//char	str[256];
	for(i=0; i<MAX_OBJECTS; i++)
		if(!objects[i])
			break;
	if(i==MAX_OBJECTS)
	{
		ANIMATERROR;
        animat_error=ANIMAT_MAXIMUMERR;
		*ip=-1;
		return FALSE;
	}
	//strcpy(str, PERSOSDIR);
	//strcpy(str, name);
	if(!(objects[i]=object_load(name)))
	{
		*ip=-1;
		return FALSE;
	}
	*ip=i;
	return TRUE;
}

int Aanimat::item_clear(int ip)
{
	A_OBJECT	*item=objects[ip];
	if((ip<0)||(ip>=MAX_OBJECTS))
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
	if(!item)
	{
		ANIMATERROR;
        animat_error=ANIMAT_INVALIDPARAMERR;
		return FALSE;
	}
	object_free(item);
	objects[ip]=NULL;
	return TRUE;
}

static int debuganim;

void Aanimat::animat_control(int hp)
{
    static  int n=-1;
	static	int nb=0;
    while(1)
    {
		switch(n)
		{
			case -1: n=7;  nb=0; break;
			case  7: 
			{
				nb++;
				if(nb==10)
					n=3;  
			}
			break;
			case  3: n=1;  break;
			case  1: n=10; break;
			case 10: n=12; break;
			case 12: n=7;  nb=0; break;
		}
		n=0;
        if(objects[0]->anims[n])
        {
            object_play(hp, n, 0, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f); // 0 remplace ANIMAT_RECALAUTO
			debuganim=n;
            break;
        }
    }
}

void Aanimat::animat_update()
{
	int		n=nbpersos;
	int		hp=0;
    while(n)
	{
		A_PERSO *perso=persos[hp];
		if(perso)
		{
            if(!perso->nextanim[0])
            {
                if(!perso->currentanim[0])
                    animat_control(hp);
                else
                {
                    if(*(perso->currentanim[0]->time+perso->currentanim[0]->nbkeys-2)<animat_time-perso->starttime)
                        animat_control(hp);
                }
            }
			n--;
		}
		hp++;
	}
}

void Aanimat::animat_render(float avance)
{
    animat_time+=avance*0.68f;
	animat_update();
	if(1)
		elekRender();
	else
		perso_render(persos[0]);
}

int Aanimat::animat_init()
{
    {
        int     i;
        for(i=0; i<MAX_PERSOS; i++)
            persos[i]=NULL;
        for(i=0; i<MAX_OBJECTS; i++)
            objects[i]=NULL;
    }
	elekPerso(0, 0, 0);
	/*
    objects[0]=object_load(KPR_JACK_JAMBES);
    objects[1]=object_load(KPR_JACK_TRONC);
    objects[2]=object_load(KPR_JACK_TETE);
	persos[0]=perso_create3(objects[0], objects[1], objects[2]);
	nbobjects=3;
	nbpersos=1;
	*/
	return TRUE;
}

int getnbanim()
{
	return debuganim;
}

// fonctions externes					(end)
//
////////////////////////////////////////////////

////////////////////////////////////////////////
// elektro special functions
//

static int itete[]={KPR_JACK_TETE, KPR_HELMUT_TETE, KPR_GRADE_TETE};
static int itronc[]={KPR_JACK_TRONC, KPR_HELMUT_TRONC, KPR_GRADE_TRONC};
static int ijambes[]={KPR_JACK_JAMBES, KPR_HELMUT_JAMBES, KPR_GRADE_JAMBES};

void Aanimat::elekPerso(int tete, int tronc, int jambes)
{
	animat_clear();
    objects[0]=object_load(ijambes[jambes]);
    objects[1]=object_load(itronc[tronc]);
    objects[2]=object_load(itete[tete]);
	persos[0]=perso_create3(objects[0], objects[1], objects[2]);
	nbobjects=3;
	nbpersos=1;
	animat_time=0.f;
	elektete=tete;
	elektronc=tronc;
	elekjambes=jambes;
}

void Aanimat::elekPlay(int anim)
{
}

void Aanimat::elekRender()
{
	A_PERSO *perso=persos[0];
	float	rx[16],ry[16],rz[16];
	{
		int	i;
		for(i=0; i<16; i++)
		{
			rx[i]=0.f;
			ry[i]=0.f;
			rz[i]=0.f;
		}
	}

	ry[NULLPERE_1]=(float)sin(animat_time*2.f)*0.5f;

	ry[JNTTRONC]=((float)sin(animat_time*4.f)-0.5f)*0.5f;
	rx[JNTTRONC]=(float)sin(animat_time*2.f);

	rz[JNTJAMBEG_2]=((float)sin(animat_time*4.f)+1.f)*0.25f;
	rz[JNTJAMBED_2]=(-(float)sin(animat_time*4.f)+1.f)*0.25f;

	ry[JNTBRASD_1]=(float)sin(animat_time*2.f)-1.f;
	ry[JNTBRASG_1]=(float)sin(animat_time*2.f)+1.f;

	ry[JNTBRASD_1]=(float)sin(animat_time*4.f)*0.5f;
	ry[JNTBRASG_1]=(float)sin(animat_time*4.f)*0.5f;

	ry[CHNTETE]=(float)sin(animat_time);
	rx[CHNTETE]=(float)sin(animat_time*2.f)*0.5f;


	rx[JNTBRASD_2]=(float)sin(animat_time*2.f)*0.5f;
	rx[JNTBRASG_2]=(float)sin(animat_time*2.f)*0.5f;

	rz[JNTBRASD_2]=(float)sin(animat_time*4.f)*0.5f;
	rz[JNTBRASG_2]=-(float)sin(animat_time*4.f)*0.5f;

	if(perso)
    {
        struct RMATRICE monde=matrice_identity;
        memset(perso->vertigo, 0, perso->nbvertices*sizeof(VTX3));
        {
            struct RMATRICE m=matrice_identity;
            matrice_setrotation(&m, cam_rx, cam_ry, cam_rz);
            matrice_settranslation(&monde, cam_tx, cam_ty, cam_tz);
            monde=matrice_multiply(monde, m);
        }
		{
	        A_PNODE     *pnode=perso->nodes;
			int			i;
            for(i=0; i<perso->nbnodes; i++)
            {
				int		anim=0;
                A_ANIM  *panim=*(perso->obj[pnode->nobj]->anims+anim);
				
                if(panim)
                {
                    struct RMATRICE m=matrice_identity;
                    A_ANIMPOS       *panimpos;
					int				code=perso->obj[pnode->nobj]->nodes[pnode->nodeobj].code>>8;
                    panimpos=*panim->animpos+pnode->nodeobj;
                    matrice_setrotation(&m, panimpos->rx+rx[code], panimpos->ry+ry[code], panimpos->rz+rz[code]);
                    matrice_settranslation(&m, panimpos->tx, panimpos->ty, panimpos->tz);
                    if(pnode->pere)
                        pnode->matrice=matrice_multiply(m, pnode->pere->matrice);
                    else
                    {
                        struct RMATRICE m2=matrice_identity;
                        matrice_settranslation(&m2, perso->tx, perso->ty, perso->tz);
                        matrice_setrotation(&m2, perso->rx, perso->ty, perso->rz);
                        monde=matrice_multiply(m2, monde);
                        pnode->matrice=matrice_multiply(m, monde);
                    }
                    perso_projete(pnode);
                }
                pnode++;
            }
            perso_sendface(perso);
		}
	}
}