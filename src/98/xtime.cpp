// coded by Renan
// project : VIDEOASTESIS
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include					<windows.h>
#include					<math.h>
#include					"resource.h"

//////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

#include					"xtime.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define						MAX						1000
#define						VAL3					1

typedef struct Spoint
{
	struct Spoint          *suivant;
	int                     z;
	int                     y;
	int                     x;
	int                     couleur;
} TPoint;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static int					sinus[1024+256];
static int					*cosinus=sinus+256;
static word					tabzoom[320*200];
static byte					table[256];

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

typedef struct SVar
{
	int						lastspaceticks;
	int						ticks;
	int						tickslen;
	float					decal;
	int						smooth;
	TPoint					*start;
	int						tx[256];
	int						nb;
	int						calcpal_compteur;
	double					calcpal_alpha1;
	double					calcpal_alpha2;
	double					calcpal_alpha3;
	double					calcpoints_angle;
	int						calcpoints1_alpha2;
	int						calcpoints1_inc2;
	int						calcpoints2_alpha1;
	int						calcpoints2_alpha2;
	int						calcpoints2_inc1;
	int						calcpoints2_inc2;
	int						calcpoints3_alpha1;
	int						calcpoints3_alpha2;
	int						calcpoints3_inc1;
	int						calcpoints3_inc2;
	int						calcpoints3_depart;
	int						calcpoints4_alpha1;
	int						calcpoints4_alpha2;
	int						calcpoints4_inc1;
	int						calcpoints4_inc2;
	int						calcpoints4_depart;
	int						calcpoints5_alp1;
	int						calcpoints5_alp2;
	int						calcpoints5_alp3;
	int						calcpoints5_alp4;
	bool					mybool;
	int						action_numf;
	int						action_num;
	int						action_mybool;
	TPoint					lespoints[MAX];
	byte					image[384*240];
	byte					image2[64000];
	byte					image3[64000];
	byte					image4[64000];
	byte					image5[64000];
	byte					image6[64000];
	byte					memima[30][64000];
} TVar;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define mx	384

#pragma optimize("",off)
#pragma code_seg("automod")
void projection(byte *image, TPoint *p, int nombre, TVar *pVar)
{
	static dword	start=(dword)pVar->start;
	static dword	debut[256];
	static dword	courant[256];
	static dword	taille_structure=sizeof(TPoint);
	static dword	table1[256];
	memset(image, 0, 384*240);
	__asm
	{	
		lea		esi,table1[0]
		lea		eax,l101
		lea		ebx,l102
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l103
		lea		ebx,l104
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l105
		lea		ebx,l106
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l107
		lea		ebx,l108
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l109
		lea		ebx,l110
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l111
		lea		ebx,l112
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l113
		lea		ebx,l114
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l115
		lea		ebx,l116
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4

		lea		eax,l201
		lea		ebx,l202
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l203
		lea		ebx,l204
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l205
		lea		ebx,l206
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l207
		lea		ebx,l208
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l209
		lea		ebx,l210
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l211
		lea		ebx,l212
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l213
		lea		ebx,l214
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l215
		lea		ebx,l216
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4

		lea		eax,l301
		lea		ebx,l302
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l303
		lea		ebx,l304
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l305
		lea		ebx,l306
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l307
		lea		ebx,l308
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l309
		lea		ebx,l310
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l311
		lea		ebx,l312
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l313
		lea		ebx,l314
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l315
		lea		ebx,l316
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4

		lea		eax,l401
		lea		ebx,l402
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l403
		lea		ebx,l404
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l405
		lea		ebx,l406
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l407
		lea		ebx,l408
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l409
		lea		ebx,l410
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l411
		lea		ebx,l412
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l413
		lea		ebx,l414
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,l415
		lea		ebx,l416
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
/////////////
		lea		eax,a101
		lea		ebx,a102
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a103
		lea		ebx,a104
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a105
		lea		ebx,a106
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a107
		lea		ebx,a108
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a109
		lea		ebx,a110
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a111
		lea		ebx,a112
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a113
		lea		ebx,a114
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a115
		lea		ebx,a116
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4

		lea		eax,a201
		lea		ebx,a202
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a203
		lea		ebx,a204
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a205
		lea		ebx,a206
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a207
		lea		ebx,a208
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a209
		lea		ebx,a210
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a211
		lea		ebx,a212
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a213
		lea		ebx,a214
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a215
		lea		ebx,a216
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4

		lea		eax,a301
		lea		ebx,a302
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a303
		lea		ebx,a304
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a305
		lea		ebx,a306
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a307
		lea		ebx,a308
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a309
		lea		ebx,a310
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a311
		lea		ebx,a312
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a313
		lea		ebx,a314
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a315
		lea		ebx,a316
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4

		lea		eax,a401
		lea		ebx,a402
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a403
		lea		ebx,a404
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a405
		lea		ebx,a406
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a407
		lea		ebx,a408
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a409
		lea		ebx,a410
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a411
		lea		ebx,a412
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a413
		lea		ebx,a414
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
		lea		eax,a415
		lea		ebx,a416
		mov		[esi],eax
		add		esi,4
		mov		[esi],ebx
		add		esi,4
///////////

		mov		eax,image
		mov		edx,p
		mov		ebx,taille_structure
		mov		ecx,nombre

		push	ebp

		lea     esi,cg_dest
		add     eax,mx*10+16
		mov     [esi-4],eax

		lea     esi,cg_struct1
		mov     [esi-4],edx

		lea     esi,cg_nb1
		mov     [esi-4],ecx

		lea     esi,cg_size
		mov     [esi-4],ebx

        
                                        // remplis pointeur courant pointe sur debut
        mov     ecx,256
        lea     edx,debut
        lea     ebx,courant
        mov     eax,-1
bouc1:
        mov     [ebx],edx
        mov     [edx],eax
        add     edx,4
        add     ebx,4
        dec     ecx
        jnz     bouc1

                                        // *** 1 Šre Passe *************************************

        mov     ebp,012345678h          // ebp = nombre de points
cg_nb1:
        mov     esi,012345678h          // esi = adresse structure ‚l‚ment
cg_struct1:
        xor     ecx,ecx

bouc2:        
        mov     eax,[esi+4]             // eax = z
        cmp     eax,0
        jle     tropetit                
        and     eax,0ffh
        lea     edx,[eax*4+courant]     //; edx = adresse de l'adresse du point dans table courant
        mov     edi,[edx]               //; edi = adresse point pr‚c‚dent
        mov     [edi],esi               //; point pr‚c‚dent pointe sur point encours
        mov     [edx],esi               //; courant pointe sur le point encours
        inc     ecx                     //; nb point ++
tropetit:        
        add     esi,012345678h
cg_size:

        dec     ebp
        jnz     bouc2

        cmp     ecx,0
        je      fin

        call    attache


                                        // remplis pointeur courant pointe sur debut
        mov     ebp,256
        lea     edx,debut
        lea     ebx,courant
        mov     eax,-1
bouc3:
        mov     [ebx],edx
        mov     [edx],eax
        add     edx,4
        add     ebx,4
        dec     ebp
        jnz     bouc3


                                        // *** 2Šme Passe ****************************************
        mov     ebp,ecx                 // ebp = nbpoint (z>0)
        mov     esi,start
        xor     eax,eax
bouc4:
        mov     al,[esi+5]              // al = 2 Šme octet de z

        lea     edx,[eax*4+courant]     // edx = adresse de l'adresse du point dans table courant
        mov     edi,[edx]               // edi = adresse point pr‚c‚dent
        mov     [edi],esi               // point pr‚c‚dent pointe sur point encours
        mov     [edx],esi               // courant pointe sur le point encours

        mov     esi,[esi]               // esi = suivant
        dec     ebp
        jnz     bouc4


        call    attache



                                        // *** Passe d'affichage ****************************************
        mov     esi,start
bouc5:
        mov     ebp,[esi+4]             //  z            
        
        mov     eax,[esi+8]             //  y
        sal     eax,8                   // * 256
        cdq
        idiv    ebp
        mov     ebx,110                 //maxy/2
        add     ebx,eax
        cmp     ebx,219                 //maxy
        ja      finy                    

        mov     eax,[esi+12]            //x
        sal     eax,8
        cdq
        idiv    ebp
        add     eax,176
        cmp     eax,351
        ja      finy

        shl     ebx,7
        lea     ebx,[ebx+ebx*2]         // ebx=ebx*384

        mov     edx,[esi+16]            //edx=couleur
        add     ebx,eax

        add     ebx,012345678h
cg_dest:
        

        mov     eax,ebx
        shr     ebp,5
        and     eax,3
        mov     edi,255
        shl     eax,4
        sub     edi,ebp
        mov     dh,dl
        shr     edi,4
        mov     ebp,edx
        or      edi,eax                 // edi = ( alignement * 16 + 0..15 (taille sprite ) )  alignement = 0..3
        bswap   ebp
        //add     edi,_xtimesmooth
        and     edi,63
        or      edx,ebp                 
        
        jmp     table1[edi*4]

        //mov     [ebx],dl
        
finy:
        mov     esi,[esi]               // esi = suivant
        dec     ecx
        jnz     bouc5
		jmp		fin


                                        // prc‚dure de cr‚ation des liaisons entrent les listes
attache:
        pushad

        mov     ecx,256
        lea     esi,debut				// le + 255 et sub ,4 pour esi et edi; font le tri … l'envers 
        lea     edi,courant
        add     esi,255*4
        add     edi,255*4
        lea     edx,start
        mov     ebp,-1

bcattache:
        cmp     [esi],ebp
        je      suite_att

        mov     eax,[esi]                
        mov     [edx],eax               // *(edx) = adr de debut encours
        mov     edx,[edi]               // edx    = adr du dernier de la liste

suite_att:
        sub     esi,4
        sub     edi,4
        dec     ecx
        jnz     bcattache

        popad
        ret

l101:
        mov     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l102:
        mov     [ebx+(-1* mx) + (-1) ],dl
        mov     [ebx+(-1* mx) + ( 0) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dl
        mov     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l103:
        mov     [ebx+(-1* mx) + (-1) ],dl
        mov     [ebx+(-1* mx) + ( 0) ],dx
        mov     [ebx+( 0* mx) + (-1) ],dl
        mov     [ebx+( 0* mx) + ( 0) ],dx
        mov     [ebx+( 1* mx) + (-1) ],dl
        mov     [ebx+( 1* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l104:
        mov     [ebx+(-2* mx) + (-1) ],dl
        mov     [ebx+(-2* mx) + ( 0) ],dl
        mov     [ebx+(-1* mx) + (-2) ],dx
        mov     [ebx+(-1* mx) + ( 0) ],dx
        mov     [ebx+( 0* mx) + (-2) ],dx
        mov     [ebx+( 0* mx) + ( 0) ],dx
        mov     [ebx+( 1* mx) + (-1) ],dl
        mov     [ebx+( 1* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l105:
        mov     [ebx+(-2* mx) + (-1) ],dl
        mov     [ebx+(-2* mx) + ( 0) ],dx
        mov     [ebx+(-1* mx) + (-2) ],dx
        mov     [ebx+(-1* mx) + ( 0) ],dx
        mov     [ebx+(-1* mx) + ( 2) ],dl
        mov     [ebx+( 0* mx) + (-2) ],dx
        mov     [ebx+( 0* mx) + ( 0) ],dx
        mov     [ebx+( 0* mx) + ( 2) ],dl
        mov     [ebx+( 1* mx) + (-2) ],dx
        mov     [ebx+( 1* mx) + ( 0) ],dx
        mov     [ebx+( 1* mx) + ( 2) ],dl
        mov     [ebx+( 2* mx) + (-1) ],dl
        mov     [ebx+( 2* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l106:
        mov     [ebx+(-3* mx) + (-2) ],dx
        mov     [ebx+(-3* mx) + ( 0) ],dx
        mov     [ebx+(-2* mx) + (-3) ],dl
        mov     [ebx+(-2* mx) + (-2) ],dx
        mov     [ebx+(-2* mx) + ( 0) ],dx
        mov     [ebx+(-2* mx) + ( 2) ],dl
        mov     [ebx+(-1* mx) + (-3) ],dl
        mov     [ebx+(-1* mx) + (-2) ],dx
        mov     [ebx+(-1* mx) + ( 0) ],dx
        mov     [ebx+(-1* mx) + ( 2) ],dl
        mov     [ebx+( 0* mx) + (-3) ],dl
        mov     [ebx+( 0* mx) + (-2) ],dx
        mov     [ebx+( 0* mx) + ( 0) ],dx
        mov     [ebx+( 0* mx) + ( 2) ],dl
        mov     [ebx+( 1* mx) + (-3) ],dl
        mov     [ebx+( 1* mx) + (-2) ],dx
        mov     [ebx+( 1* mx) + ( 0) ],dx
        mov     [ebx+( 1* mx) + ( 2) ],dl
        mov     [ebx+( 2* mx) + (-2) ],dx
        mov     [ebx+( 2* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l107:
        mov     [ebx+(-3* mx) + (-1) ],dl
        mov     [ebx+(-3* mx) + ( 0) ],dx
        mov     [ebx+(-2* mx) + (-2) ],dx
        mov     [ebx+(-2* mx) + ( 0) ],dx
        mov     [ebx+(-2* mx) + ( 2) ],dl
        mov     [ebx+(-1* mx) + (-3) ],dl
        mov     [ebx+(-1* mx) + (-2) ],dx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + (-3) ],dl
        mov     [ebx+( 0* mx) + (-2) ],dx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + (-3) ],dl
        mov     [ebx+( 1* mx) + (-2) ],dx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + (-2) ],dx
        mov     [ebx+( 2* mx) + ( 0) ],dx
        mov     [ebx+( 2* mx) + ( 2) ],dl
        mov     [ebx+( 3* mx) + (-1) ],dl
        mov     [ebx+( 3* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l108:
        mov     [ebx+(-4* mx) + (-2) ],dx
        mov     [ebx+(-4* mx) + ( 0) ],dx
        mov     [ebx+(-3* mx) + (-3) ],dl
        mov     [ebx+(-3* mx) + (-2) ],dx
        mov     [ebx+(-3* mx) + ( 0) ],dx
        mov     [ebx+(-3* mx) + ( 2) ],dl
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + (-3) ],dl
        mov     [ebx+( 2* mx) + (-2) ],dx
        mov     [ebx+( 2* mx) + ( 0) ],dx
        mov     [ebx+( 2* mx) + ( 2) ],dl
        mov     [ebx+( 3* mx) + (-2) ],dx
        mov     [ebx+( 3* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l109:
        mov     [ebx+(-4* mx) + (-2) ],dx
        mov     [ebx+(-4* mx) + ( 0) ],dx
        mov     [ebx+(-4* mx) + ( 2) ],dl
        mov     [ebx+(-3* mx) + (-3) ],dl
        mov     [ebx+(-3* mx) + (-2) ],dx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],dl
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],dl
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],dl
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],dl
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],dl
        mov     [ebx+( 3* mx) + (-3) ],dl
        mov     [ebx+( 3* mx) + (-2) ],dx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 4* mx) + (-2) ],dx
        mov     [ebx+( 4* mx) + ( 0) ],dx
        mov     [ebx+( 4* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l110:
        mov     [ebx+(-5* mx) + (-2) ],dx
        mov     [ebx+(-5* mx) + ( 0) ],dx
        mov     [ebx+(-4* mx) + (-4) ],edx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + (-5) ],dl
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],dl
        mov     [ebx+(-1* mx) + (-5) ],dl
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],dl
        mov     [ebx+( 0* mx) + (-5) ],dl
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],dl
        mov     [ebx+( 1* mx) + (-5) ],dl
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],dl
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 4* mx) + (-2) ],dx
        mov     [ebx+( 4* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l111:
        mov     [ebx+(-5* mx) + (-2) ],dx
        mov     [ebx+(-5* mx) + ( 0) ],dx
        mov     [ebx+(-5* mx) + ( 2) ],dl
        mov     [ebx+(-4* mx) + (-3) ],dl
        mov     [ebx+(-4* mx) + (-2) ],dx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + ( 4) ],dl
        mov     [ebx+(-2* mx) + (-5) ],dl
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],dx
        mov     [ebx+(-1* mx) + (-5) ],dl
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],dx
        mov     [ebx+( 0* mx) + (-5) ],dl
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],dx
        mov     [ebx+( 1* mx) + (-5) ],dl
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],dx
        mov     [ebx+( 2* mx) + (-5) ],dl
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],dx
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + ( 4) ],dl
        mov     [ebx+( 4* mx) + (-3) ],dl
        mov     [ebx+( 4* mx) + (-2) ],dx
        mov     [ebx+( 4* mx) + ( 0) ],edx
        mov     [ebx+( 5* mx) + (-2) ],dx
        mov     [ebx+( 5* mx) + ( 0) ],dx
        mov     [ebx+( 5* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l112:
        mov     [ebx+(-6* mx) + (-2) ],dx
        mov     [ebx+(-6* mx) + ( 0) ],dx
        mov     [ebx+(-5* mx) + (-4) ],edx
        mov     [ebx+(-5* mx) + ( 0) ],edx
        mov     [ebx+(-4* mx) + (-5) ],dl
        mov     [ebx+(-4* mx) + (-4) ],edx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-4* mx) + ( 4) ],dl
        mov     [ebx+(-3* mx) + (-5) ],dl
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + ( 4) ],dl
        mov     [ebx+(-2* mx) + (-6) ],dx
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],dx
        mov     [ebx+(-1* mx) + (-6) ],dx
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],dx
        mov     [ebx+( 0* mx) + (-6) ],dx
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],dx
        mov     [ebx+( 1* mx) + (-6) ],dx
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],dx
        mov     [ebx+( 2* mx) + (-5) ],dl
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],dl
        mov     [ebx+( 3* mx) + (-5) ],dl
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + ( 4) ],dl
        mov     [ebx+( 4* mx) + (-4) ],edx
        mov     [ebx+( 4* mx) + ( 0) ],edx
        mov     [ebx+( 5* mx) + (-2) ],dx
        mov     [ebx+( 5* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l113:
        mov     [ebx+(-6* mx) + (-2) ],dx
        mov     [ebx+(-6* mx) + ( 0) ],dx
        mov     [ebx+(-6* mx) + ( 2) ],dl
        mov     [ebx+(-5* mx) + (-4) ],edx
        mov     [ebx+(-5* mx) + ( 0) ],edx
        mov     [ebx+(-5* mx) + ( 4) ],dl
        mov     [ebx+(-4* mx) + (-5) ],dl
        mov     [ebx+(-4* mx) + (-4) ],edx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-4* mx) + ( 4) ],dx
        mov     [ebx+(-3* mx) + (-5) ],dl
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + ( 4) ],dx
        mov     [ebx+(-2* mx) + (-6) ],dx
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],dx
        mov     [ebx+(-2* mx) + ( 6) ],dl
        mov     [ebx+(-1* mx) + (-6) ],dx
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],dx
        mov     [ebx+(-1* mx) + ( 6) ],dl
        mov     [ebx+( 0* mx) + (-6) ],dx
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],dx
        mov     [ebx+( 0* mx) + ( 6) ],dl
        mov     [ebx+( 1* mx) + (-6) ],dx
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],dx
        mov     [ebx+( 1* mx) + ( 6) ],dl
        mov     [ebx+( 2* mx) + (-6) ],dx
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],dx
        mov     [ebx+( 2* mx) + ( 6) ],dl
        mov     [ebx+( 3* mx) + (-5) ],dl
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + ( 4) ],dx
        mov     [ebx+( 4* mx) + (-5) ],dl
        mov     [ebx+( 4* mx) + (-4) ],edx
        mov     [ebx+( 4* mx) + ( 0) ],edx
        mov     [ebx+( 4* mx) + ( 4) ],dx
        mov     [ebx+( 5* mx) + (-4) ],edx
        mov     [ebx+( 5* mx) + ( 0) ],edx
        mov     [ebx+( 5* mx) + ( 4) ],dl
        mov     [ebx+( 6* mx) + (-2) ],dx
        mov     [ebx+( 6* mx) + ( 0) ],dx
        mov     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l114:
        mov     [ebx+(-7* mx) + (-3) ],dl
        mov     [ebx+(-7* mx) + (-2) ],dx
        mov     [ebx+(-7* mx) + ( 0) ],dx
        mov     [ebx+(-7* mx) + ( 2) ],dl
        mov     [ebx+(-6* mx) + (-4) ],edx
        mov     [ebx+(-6* mx) + ( 0) ],edx
        mov     [ebx+(-5* mx) + (-5) ],dl
        mov     [ebx+(-5* mx) + (-4) ],edx
        mov     [ebx+(-5* mx) + ( 0) ],edx
        mov     [ebx+(-5* mx) + ( 4) ],dl
        mov     [ebx+(-4* mx) + (-6) ],dx
        mov     [ebx+(-4* mx) + (-4) ],edx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-4* mx) + ( 4) ],dx
        mov     [ebx+(-3* mx) + (-7) ],dl
        mov     [ebx+(-3* mx) + (-6) ],dx
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + ( 4) ],dx
        mov     [ebx+(-3* mx) + ( 6) ],dl
        mov     [ebx+(-2* mx) + (-7) ],dl
        mov     [ebx+(-2* mx) + (-6) ],dx
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],dx
        mov     [ebx+(-2* mx) + ( 6) ],dl
        mov     [ebx+(-1* mx) + (-7) ],dl
        mov     [ebx+(-1* mx) + (-6) ],dx
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],dx
        mov     [ebx+(-1* mx) + ( 6) ],dl
        mov     [ebx+( 0* mx) + (-7) ],dl
        mov     [ebx+( 0* mx) + (-6) ],dx
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],dx
        mov     [ebx+( 0* mx) + ( 6) ],dl
        mov     [ebx+( 1* mx) + (-7) ],dl
        mov     [ebx+( 1* mx) + (-6) ],dx
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],dx
        mov     [ebx+( 1* mx) + ( 6) ],dl
        mov     [ebx+( 2* mx) + (-7) ],dl
        mov     [ebx+( 2* mx) + (-6) ],dx
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],dx
        mov     [ebx+( 2* mx) + ( 6) ],dl
        mov     [ebx+( 3* mx) + (-6) ],dx
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + ( 4) ],dx
        mov     [ebx+( 4* mx) + (-5) ],dl
        mov     [ebx+( 4* mx) + (-4) ],edx
        mov     [ebx+( 4* mx) + ( 0) ],edx
        mov     [ebx+( 4* mx) + ( 4) ],dl
        mov     [ebx+( 5* mx) + (-4) ],edx
        mov     [ebx+( 5* mx) + ( 0) ],edx
        mov     [ebx+( 6* mx) + (-3) ],dl
        mov     [ebx+( 6* mx) + (-2) ],dx
        mov     [ebx+( 6* mx) + ( 0) ],dx
        mov     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l115:
        mov     [ebx+(-7* mx) + (-2) ],dx
        mov     [ebx+(-7* mx) + ( 0) ],dx
        mov     [ebx+(-7* mx) + ( 2) ],dl
        mov     [ebx+(-6* mx) + (-4) ],edx
        mov     [ebx+(-6* mx) + ( 0) ],edx
        mov     [ebx+(-6* mx) + ( 4) ],dl
        mov     [ebx+(-5* mx) + (-5) ],dl
        mov     [ebx+(-5* mx) + (-4) ],edx
        mov     [ebx+(-5* mx) + ( 0) ],edx
        mov     [ebx+(-5* mx) + ( 4) ],dx
        mov     [ebx+(-4* mx) + (-6) ],dx
        mov     [ebx+(-4* mx) + (-4) ],edx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-4* mx) + ( 4) ],dx
        mov     [ebx+(-4* mx) + ( 6) ],dl
        mov     [ebx+(-3* mx) + (-6) ],dx
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + ( 4) ],dx
        mov     [ebx+(-3* mx) + ( 6) ],dl
        mov     [ebx+(-2* mx) + (-7) ],dl
        mov     [ebx+(-2* mx) + (-6) ],dx
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],edx
        mov     [ebx+(-1* mx) + (-7) ],dl
        mov     [ebx+(-1* mx) + (-6) ],dx
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],edx
        mov     [ebx+( 0* mx) + (-7) ],dl
        mov     [ebx+( 0* mx) + (-6) ],dx
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],edx
        mov     [ebx+( 1* mx) + (-7) ],dl
        mov     [ebx+( 1* mx) + (-6) ],dx
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],edx
        mov     [ebx+( 2* mx) + (-7) ],dl
        mov     [ebx+( 2* mx) + (-6) ],dx
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],edx
        mov     [ebx+( 3* mx) + (-6) ],dx
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + ( 4) ],dx
        mov     [ebx+( 3* mx) + ( 6) ],dl
        mov     [ebx+( 4* mx) + (-6) ],dx
        mov     [ebx+( 4* mx) + (-4) ],edx
        mov     [ebx+( 4* mx) + ( 0) ],edx
        mov     [ebx+( 4* mx) + ( 4) ],dx
        mov     [ebx+( 4* mx) + ( 6) ],dl
        mov     [ebx+( 5* mx) + (-5) ],dl
        mov     [ebx+( 5* mx) + (-4) ],edx
        mov     [ebx+( 5* mx) + ( 0) ],edx
        mov     [ebx+( 5* mx) + ( 4) ],dx
        mov     [ebx+( 6* mx) + (-4) ],edx
        mov     [ebx+( 6* mx) + ( 0) ],edx
        mov     [ebx+( 6* mx) + ( 4) ],dl
        mov     [ebx+( 7* mx) + (-2) ],dx
        mov     [ebx+( 7* mx) + ( 0) ],dx
        mov     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l116:
        mov     [ebx+(-8* mx) + (-3) ],dl
        mov     [ebx+(-8* mx) + (-2) ],dx
        mov     [ebx+(-8* mx) + ( 0) ],dx
        mov     [ebx+(-8* mx) + ( 2) ],dl
        mov     [ebx+(-7* mx) + (-5) ],dl
        mov     [ebx+(-7* mx) + (-4) ],edx
        mov     [ebx+(-7* mx) + ( 0) ],edx
        mov     [ebx+(-7* mx) + ( 4) ],dl
        mov     [ebx+(-6* mx) + (-6) ],dx
        mov     [ebx+(-6* mx) + (-4) ],edx
        mov     [ebx+(-6* mx) + ( 0) ],edx
        mov     [ebx+(-6* mx) + ( 4) ],dx
        mov     [ebx+(-5* mx) + (-7) ],dl
        mov     [ebx+(-5* mx) + (-6) ],dx
        mov     [ebx+(-5* mx) + (-4) ],edx
        mov     [ebx+(-5* mx) + ( 0) ],edx
        mov     [ebx+(-5* mx) + ( 4) ],dx
        mov     [ebx+(-5* mx) + ( 6) ],dl
        mov     [ebx+(-4* mx) + (-7) ],dl
        mov     [ebx+(-4* mx) + (-6) ],dx
        mov     [ebx+(-4* mx) + (-4) ],edx
        mov     [ebx+(-4* mx) + ( 0) ],edx
        mov     [ebx+(-4* mx) + ( 4) ],dx
        mov     [ebx+(-4* mx) + ( 6) ],dl
        mov     [ebx+(-3* mx) + (-8) ],edx
        mov     [ebx+(-3* mx) + (-4) ],edx
        mov     [ebx+(-3* mx) + ( 0) ],edx
        mov     [ebx+(-3* mx) + ( 4) ],edx
        mov     [ebx+(-2* mx) + (-8) ],edx
        mov     [ebx+(-2* mx) + (-4) ],edx
        mov     [ebx+(-2* mx) + ( 0) ],edx
        mov     [ebx+(-2* mx) + ( 4) ],edx
        mov     [ebx+(-1* mx) + (-8) ],edx
        mov     [ebx+(-1* mx) + (-4) ],edx
        mov     [ebx+(-1* mx) + ( 0) ],edx
        mov     [ebx+(-1* mx) + ( 4) ],edx
        mov     [ebx+( 0* mx) + (-8) ],edx
        mov     [ebx+( 0* mx) + (-4) ],edx
        mov     [ebx+( 0* mx) + ( 0) ],edx
        mov     [ebx+( 0* mx) + ( 4) ],edx
        mov     [ebx+( 1* mx) + (-8) ],edx
        mov     [ebx+( 1* mx) + (-4) ],edx
        mov     [ebx+( 1* mx) + ( 0) ],edx
        mov     [ebx+( 1* mx) + ( 4) ],edx
        mov     [ebx+( 2* mx) + (-8) ],edx
        mov     [ebx+( 2* mx) + (-4) ],edx
        mov     [ebx+( 2* mx) + ( 0) ],edx
        mov     [ebx+( 2* mx) + ( 4) ],edx
        mov     [ebx+( 3* mx) + (-7) ],dl
        mov     [ebx+( 3* mx) + (-6) ],dx
        mov     [ebx+( 3* mx) + (-4) ],edx
        mov     [ebx+( 3* mx) + ( 0) ],edx
        mov     [ebx+( 3* mx) + ( 4) ],dx
        mov     [ebx+( 3* mx) + ( 6) ],dl
        mov     [ebx+( 4* mx) + (-7) ],dl
        mov     [ebx+( 4* mx) + (-6) ],dx
        mov     [ebx+( 4* mx) + (-4) ],edx
        mov     [ebx+( 4* mx) + ( 0) ],edx
        mov     [ebx+( 4* mx) + ( 4) ],dx
        mov     [ebx+( 4* mx) + ( 6) ],dl
        mov     [ebx+( 5* mx) + (-6) ],dx
        mov     [ebx+( 5* mx) + (-4) ],edx
        mov     [ebx+( 5* mx) + ( 0) ],edx
        mov     [ebx+( 5* mx) + ( 4) ],dx
        mov     [ebx+( 6* mx) + (-5) ],dl
        mov     [ebx+( 6* mx) + (-4) ],edx
        mov     [ebx+( 6* mx) + ( 0) ],edx
        mov     [ebx+( 6* mx) + ( 4) ],dl
        mov     [ebx+( 7* mx) + (-3) ],dl
        mov     [ebx+( 7* mx) + (-2) ],dx
        mov     [ebx+( 7* mx) + ( 0) ],dx
        mov     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l201:
        mov     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l202:
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l203:
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + ( 1) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + ( 1) ],dl
        mov     [ebx+( 1* mx) + (-1) ],dx
        mov     [ebx+( 1* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l204:
        mov     [ebx+(-2* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + (-2) ],dl
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + ( 1) ],dl
        mov     [ebx+( 0* mx) + (-2) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + ( 1) ],dl
        mov     [ebx+( 1* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l205:
        mov     [ebx+(-2* mx) + (-1) ],dx
        mov     [ebx+(-2* mx) + ( 1) ],dl
        mov     [ebx+(-1* mx) + (-2) ],dl
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + (-2) ],dl
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + (-2) ],dl
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + (-1) ],dx
        mov     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l206:
        mov     [ebx+(-3* mx) + (-2) ],dl
        mov     [ebx+(-3* mx) + (-1) ],dx
        mov     [ebx+(-3* mx) + ( 1) ],dl
        mov     [ebx+(-2* mx) + (-3) ],dx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + (-3) ],dx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + (-3) ],dx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + (-3) ],dx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + (-2) ],dl
        mov     [ebx+( 2* mx) + (-1) ],dx
        mov     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l207:
        mov     [ebx+(-3* mx) + (-1) ],dx
        mov     [ebx+(-3* mx) + ( 1) ],dl
        mov     [ebx+(-2* mx) + (-2) ],dl
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + (-3) ],dx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],dl
        mov     [ebx+( 0* mx) + (-3) ],dx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],dl
        mov     [ebx+( 1* mx) + (-3) ],dx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],dl
        mov     [ebx+( 2* mx) + (-2) ],dl
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + (-1) ],dx
        mov     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l208:
        mov     [ebx+(-4* mx) + (-2) ],dl
        mov     [ebx+(-4* mx) + (-1) ],dx
        mov     [ebx+(-4* mx) + ( 1) ],dl
        mov     [ebx+(-3* mx) + (-3) ],dx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + (-4) ],dl
        mov     [ebx+(-2* mx) + (-3) ],dx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],dl
        mov     [ebx+(-1* mx) + (-4) ],dl
        mov     [ebx+(-1* mx) + (-3) ],dx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],dl
        mov     [ebx+( 0* mx) + (-4) ],dl
        mov     [ebx+( 0* mx) + (-3) ],dx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],dl
        mov     [ebx+( 1* mx) + (-4) ],dl
        mov     [ebx+( 1* mx) + (-3) ],dx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],dl
        mov     [ebx+( 2* mx) + (-3) ],dx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + (-2) ],dl
        mov     [ebx+( 3* mx) + (-1) ],dx
        mov     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l209:
        mov     [ebx+(-4* mx) + (-2) ],dl
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + (-3) ],dx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],dl
        mov     [ebx+(-2* mx) + (-4) ],dl
        mov     [ebx+(-2* mx) + (-3) ],dx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],dx
        mov     [ebx+(-1* mx) + (-4) ],dl
        mov     [ebx+(-1* mx) + (-3) ],dx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],dx
        mov     [ebx+( 0* mx) + (-4) ],dl
        mov     [ebx+( 0* mx) + (-3) ],dx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],dx
        mov     [ebx+( 1* mx) + (-4) ],dl
        mov     [ebx+( 1* mx) + (-3) ],dx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],dx
        mov     [ebx+( 2* mx) + (-4) ],dl
        mov     [ebx+( 2* mx) + (-3) ],dx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],dx
        mov     [ebx+( 3* mx) + (-3) ],dx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],dl
        mov     [ebx+( 4* mx) + (-2) ],dl
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l210:
        mov     [ebx+(-5* mx) + (-2) ],dl
        mov     [ebx+(-5* mx) + (-1) ],dx
        mov     [ebx+(-5* mx) + ( 1) ],dl
        mov     [ebx+(-4* mx) + (-4) ],dl
        mov     [ebx+(-4* mx) + (-3) ],dx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],dl
        mov     [ebx+(-3* mx) + (-4) ],dl
        mov     [ebx+(-3* mx) + (-3) ],dx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],dl
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],dx
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],dx
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],dx
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],dx
        mov     [ebx+( 2* mx) + (-4) ],dl
        mov     [ebx+( 2* mx) + (-3) ],dx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],dl
        mov     [ebx+( 3* mx) + (-4) ],dl
        mov     [ebx+( 3* mx) + (-3) ],dx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],dl
        mov     [ebx+( 4* mx) + (-2) ],dl
        mov     [ebx+( 4* mx) + (-1) ],dx
        mov     [ebx+( 4* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l211:
        mov     [ebx+(-5* mx) + (-2) ],dl
        mov     [ebx+(-5* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + (-3) ],dx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],dl
        mov     [ebx+(-3* mx) + (-4) ],dl
        mov     [ebx+(-3* mx) + (-3) ],dx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],dx
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],dx
        mov     [ebx+(-2* mx) + ( 5) ],dl
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],dx
        mov     [ebx+(-1* mx) + ( 5) ],dl
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],dx
        mov     [ebx+( 0* mx) + ( 5) ],dl
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],dx
        mov     [ebx+( 1* mx) + ( 5) ],dl
        mov     [ebx+( 2* mx) + (-5) ],edx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],dx
        mov     [ebx+( 2* mx) + ( 5) ],dl
        mov     [ebx+( 3* mx) + (-4) ],dl
        mov     [ebx+( 3* mx) + (-3) ],dx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],dx
        mov     [ebx+( 4* mx) + (-3) ],dx
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     [ebx+( 4* mx) + ( 3) ],dl
        mov     [ebx+( 5* mx) + (-2) ],dl
        mov     [ebx+( 5* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l212:
        mov     [ebx+(-6* mx) + (-2) ],dl
        mov     [ebx+(-6* mx) + (-1) ],dx
        mov     [ebx+(-6* mx) + ( 1) ],dl
        mov     [ebx+(-5* mx) + (-4) ],dl
        mov     [ebx+(-5* mx) + (-3) ],dx
        mov     [ebx+(-5* mx) + (-1) ],edx
        mov     [ebx+(-5* mx) + ( 3) ],dl
        mov     [ebx+(-4* mx) + (-5) ],edx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],dx
        mov     [ebx+(-3* mx) + (-5) ],edx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],dx
        mov     [ebx+(-2* mx) + (-6) ],dl
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],dx
        mov     [ebx+(-2* mx) + ( 5) ],dl
        mov     [ebx+(-1* mx) + (-6) ],dl
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],dx
        mov     [ebx+(-1* mx) + ( 5) ],dl
        mov     [ebx+( 0* mx) + (-6) ],dl
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],dx
        mov     [ebx+( 0* mx) + ( 5) ],dl
        mov     [ebx+( 1* mx) + (-6) ],dl
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],dx
        mov     [ebx+( 1* mx) + ( 5) ],dl
        mov     [ebx+( 2* mx) + (-5) ],edx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],dx
        mov     [ebx+( 3* mx) + (-5) ],edx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],dx
        mov     [ebx+( 4* mx) + (-4) ],dl
        mov     [ebx+( 4* mx) + (-3) ],dx
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     [ebx+( 4* mx) + ( 3) ],dl
        mov     [ebx+( 5* mx) + (-2) ],dl
        mov     [ebx+( 5* mx) + (-1) ],dx
        mov     [ebx+( 5* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l213:
        mov     [ebx+(-6* mx) + (-2) ],dl
        mov     [ebx+(-6* mx) + (-1) ],edx
        mov     [ebx+(-5* mx) + (-4) ],dl
        mov     [ebx+(-5* mx) + (-3) ],dx
        mov     [ebx+(-5* mx) + (-1) ],edx
        mov     [ebx+(-5* mx) + ( 3) ],dx
        mov     [ebx+(-4* mx) + (-5) ],edx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],dx
        mov     [ebx+(-4* mx) + ( 5) ],dl
        mov     [ebx+(-3* mx) + (-5) ],edx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],dx
        mov     [ebx+(-3* mx) + ( 5) ],dl
        mov     [ebx+(-2* mx) + (-6) ],dl
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],edx
        mov     [ebx+(-1* mx) + (-6) ],dl
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],edx
        mov     [ebx+( 0* mx) + (-6) ],dl
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],edx
        mov     [ebx+( 1* mx) + (-6) ],dl
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],edx
        mov     [ebx+( 2* mx) + (-6) ],dl
        mov     [ebx+( 2* mx) + (-5) ],edx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],edx
        mov     [ebx+( 3* mx) + (-5) ],edx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],dx
        mov     [ebx+( 3* mx) + ( 5) ],dl
        mov     [ebx+( 4* mx) + (-5) ],edx
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     [ebx+( 4* mx) + ( 3) ],dx
        mov     [ebx+( 4* mx) + ( 5) ],dl
        mov     [ebx+( 5* mx) + (-4) ],dl
        mov     [ebx+( 5* mx) + (-3) ],dx
        mov     [ebx+( 5* mx) + (-1) ],edx
        mov     [ebx+( 5* mx) + ( 3) ],dx
        mov     [ebx+( 6* mx) + (-2) ],dl
        mov     [ebx+( 6* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l214:
        mov     [ebx+(-7* mx) + (-3) ],dx
        mov     [ebx+(-7* mx) + (-1) ],edx
        mov     [ebx+(-6* mx) + (-4) ],dl
        mov     [ebx+(-6* mx) + (-3) ],dx
        mov     [ebx+(-6* mx) + (-1) ],edx
        mov     [ebx+(-6* mx) + ( 3) ],dl
        mov     [ebx+(-5* mx) + (-5) ],edx
        mov     [ebx+(-5* mx) + (-1) ],edx
        mov     [ebx+(-5* mx) + ( 3) ],dx
        mov     [ebx+(-4* mx) + (-6) ],dl
        mov     [ebx+(-4* mx) + (-5) ],edx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],dx
        mov     [ebx+(-4* mx) + ( 5) ],dl
        mov     [ebx+(-3* mx) + (-7) ],dx
        mov     [ebx+(-3* mx) + (-5) ],edx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],edx
        mov     [ebx+(-2* mx) + (-7) ],dx
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],edx
        mov     [ebx+(-1* mx) + (-7) ],dx
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],edx
        mov     [ebx+( 0* mx) + (-7) ],dx
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],edx
        mov     [ebx+( 1* mx) + (-7) ],dx
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],edx
        mov     [ebx+( 2* mx) + (-7) ],dx
        mov     [ebx+( 2* mx) + (-5) ],edx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],edx
        mov     [ebx+( 3* mx) + (-6) ],dl
        mov     [ebx+( 3* mx) + (-5) ],edx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],dx
        mov     [ebx+( 3* mx) + ( 5) ],dl
        mov     [ebx+( 4* mx) + (-5) ],edx
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     [ebx+( 4* mx) + ( 3) ],dx
        mov     [ebx+( 5* mx) + (-4) ],dl
        mov     [ebx+( 5* mx) + (-3) ],dx
        mov     [ebx+( 5* mx) + (-1) ],edx
        mov     [ebx+( 5* mx) + ( 3) ],dl
        mov     [ebx+( 6* mx) + (-3) ],dx
        mov     [ebx+( 6* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l215:
        mov     [ebx+(-7* mx) + (-2) ],dl
        mov     [ebx+(-7* mx) + (-1) ],edx
        mov     [ebx+(-6* mx) + (-4) ],dl
        mov     [ebx+(-6* mx) + (-3) ],dx
        mov     [ebx+(-6* mx) + (-1) ],edx
        mov     [ebx+(-6* mx) + ( 3) ],dx
        mov     [ebx+(-5* mx) + (-5) ],edx
        mov     [ebx+(-5* mx) + (-1) ],edx
        mov     [ebx+(-5* mx) + ( 3) ],dx
        mov     [ebx+(-5* mx) + ( 5) ],dl
        mov     [ebx+(-4* mx) + (-6) ],dl
        mov     [ebx+(-4* mx) + (-5) ],edx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],edx
        mov     [ebx+(-3* mx) + (-6) ],dl
        mov     [ebx+(-3* mx) + (-5) ],edx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],edx
        mov     [ebx+(-2* mx) + (-7) ],dx
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],edx
        mov     [ebx+(-2* mx) + ( 7) ],dl
        mov     [ebx+(-1* mx) + (-7) ],dx
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],edx
        mov     [ebx+(-1* mx) + ( 7) ],dl
        mov     [ebx+( 0* mx) + (-7) ],dx
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],edx
        mov     [ebx+( 0* mx) + ( 7) ],dl
        mov     [ebx+( 1* mx) + (-7) ],dx
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],edx
        mov     [ebx+( 1* mx) + ( 7) ],dl
        mov     [ebx+( 2* mx) + (-7) ],dx
        mov     [ebx+( 2* mx) + (-5) ],edx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],edx
        mov     [ebx+( 2* mx) + ( 7) ],dl
        mov     [ebx+( 3* mx) + (-6) ],dl
        mov     [ebx+( 3* mx) + (-5) ],edx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],edx
        mov     [ebx+( 4* mx) + (-6) ],dl
        mov     [ebx+( 4* mx) + (-5) ],edx
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     [ebx+( 4* mx) + ( 3) ],edx
        mov     [ebx+( 5* mx) + (-5) ],edx
        mov     [ebx+( 5* mx) + (-1) ],edx
        mov     [ebx+( 5* mx) + ( 3) ],dx
        mov     [ebx+( 5* mx) + ( 5) ],dl
        mov     [ebx+( 6* mx) + (-4) ],dl
        mov     [ebx+( 6* mx) + (-3) ],dx
        mov     [ebx+( 6* mx) + (-1) ],edx
        mov     [ebx+( 6* mx) + ( 3) ],dx
        mov     [ebx+( 7* mx) + (-2) ],dl
        mov     [ebx+( 7* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l216:
        mov     [ebx+(-8* mx) + (-3) ],dx
        mov     [ebx+(-8* mx) + (-1) ],edx
        mov     [ebx+(-7* mx) + (-5) ],edx
        mov     [ebx+(-7* mx) + (-1) ],edx
        mov     [ebx+(-7* mx) + ( 3) ],dx
        mov     [ebx+(-6* mx) + (-6) ],dl
        mov     [ebx+(-6* mx) + (-5) ],edx
        mov     [ebx+(-6* mx) + (-1) ],edx
        mov     [ebx+(-6* mx) + ( 3) ],dx
        mov     [ebx+(-6* mx) + ( 5) ],dl
        mov     [ebx+(-5* mx) + (-7) ],dx
        mov     [ebx+(-5* mx) + (-5) ],edx
        mov     [ebx+(-5* mx) + (-1) ],edx
        mov     [ebx+(-5* mx) + ( 3) ],edx
        mov     [ebx+(-4* mx) + (-7) ],dx
        mov     [ebx+(-4* mx) + (-5) ],edx
        mov     [ebx+(-4* mx) + (-1) ],edx
        mov     [ebx+(-4* mx) + ( 3) ],edx
        mov     [ebx+(-3* mx) + (-8) ],dl
        mov     [ebx+(-3* mx) + (-7) ],dx
        mov     [ebx+(-3* mx) + (-5) ],edx
        mov     [ebx+(-3* mx) + (-1) ],edx
        mov     [ebx+(-3* mx) + ( 3) ],edx
        mov     [ebx+(-3* mx) + ( 7) ],dl
        mov     [ebx+(-2* mx) + (-8) ],dl
        mov     [ebx+(-2* mx) + (-7) ],dx
        mov     [ebx+(-2* mx) + (-5) ],edx
        mov     [ebx+(-2* mx) + (-1) ],edx
        mov     [ebx+(-2* mx) + ( 3) ],edx
        mov     [ebx+(-2* mx) + ( 7) ],dl
        mov     [ebx+(-1* mx) + (-8) ],dl
        mov     [ebx+(-1* mx) + (-7) ],dx
        mov     [ebx+(-1* mx) + (-5) ],edx
        mov     [ebx+(-1* mx) + (-1) ],edx
        mov     [ebx+(-1* mx) + ( 3) ],edx
        mov     [ebx+(-1* mx) + ( 7) ],dl
        mov     [ebx+( 0* mx) + (-8) ],dl
        mov     [ebx+( 0* mx) + (-7) ],dx
        mov     [ebx+( 0* mx) + (-5) ],edx
        mov     [ebx+( 0* mx) + (-1) ],edx
        mov     [ebx+( 0* mx) + ( 3) ],edx
        mov     [ebx+( 0* mx) + ( 7) ],dl
        mov     [ebx+( 1* mx) + (-8) ],dl
        mov     [ebx+( 1* mx) + (-7) ],dx
        mov     [ebx+( 1* mx) + (-5) ],edx
        mov     [ebx+( 1* mx) + (-1) ],edx
        mov     [ebx+( 1* mx) + ( 3) ],edx
        mov     [ebx+( 1* mx) + ( 7) ],dl
        mov     [ebx+( 2* mx) + (-8) ],dl
        mov     [ebx+( 2* mx) + (-7) ],dx
        mov     [ebx+( 2* mx) + (-5) ],edx
        mov     [ebx+( 2* mx) + (-1) ],edx
        mov     [ebx+( 2* mx) + ( 3) ],edx
        mov     [ebx+( 2* mx) + ( 7) ],dl
        mov     [ebx+( 3* mx) + (-7) ],dx
        mov     [ebx+( 3* mx) + (-5) ],edx
        mov     [ebx+( 3* mx) + (-1) ],edx
        mov     [ebx+( 3* mx) + ( 3) ],edx
        mov     [ebx+( 4* mx) + (-7) ],dx
        mov     [ebx+( 4* mx) + (-5) ],edx
        mov     [ebx+( 4* mx) + (-1) ],edx
        mov     [ebx+( 4* mx) + ( 3) ],edx
        mov     [ebx+( 5* mx) + (-6) ],dl
        mov     [ebx+( 5* mx) + (-5) ],edx
        mov     [ebx+( 5* mx) + (-1) ],edx
        mov     [ebx+( 5* mx) + ( 3) ],dx
        mov     [ebx+( 5* mx) + ( 5) ],dl
        mov     [ebx+( 6* mx) + (-5) ],edx
        mov     [ebx+( 6* mx) + (-1) ],edx
        mov     [ebx+( 6* mx) + ( 3) ],dx
        mov     [ebx+( 7* mx) + (-3) ],dx
        mov     [ebx+( 7* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l301:
        mov     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l302:
        mov     [ebx+(-1* mx) + (-1) ],dl
        mov     [ebx+(-1* mx) + ( 0) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dl
        mov     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l303:
        mov     [ebx+(-1* mx) + (-1) ],dl
        mov     [ebx+(-1* mx) + ( 0) ],dx
        mov     [ebx+( 0* mx) + (-1) ],dl
        mov     [ebx+( 0* mx) + ( 0) ],dx
        mov     [ebx+( 1* mx) + (-1) ],dl
        mov     [ebx+( 1* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l304:
        mov     [ebx+(-2* mx) + (-1) ],dl
        mov     [ebx+(-2* mx) + ( 0) ],dl
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + (-1) ],dl
        mov     [ebx+( 1* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l305:
        mov     [ebx+(-2* mx) + (-1) ],dl
        mov     [ebx+(-2* mx) + ( 0) ],dx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],dl
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],dl
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],dl
        mov     [ebx+( 2* mx) + (-1) ],dl
        mov     [ebx+( 2* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l306:
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + (-3) ],dl
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],dl
        mov     [ebx+(-1* mx) + (-3) ],dl
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],dl
        mov     [ebx+( 0* mx) + (-3) ],dl
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],dl
        mov     [ebx+( 1* mx) + (-3) ],dl
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],dl
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l307:
        mov     [ebx+(-3* mx) + (-1) ],dl
        mov     [ebx+(-3* mx) + ( 0) ],dx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],dl
        mov     [ebx+(-1* mx) + (-3) ],dl
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],dx
        mov     [ebx+( 0* mx) + (-3) ],dl
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],dx
        mov     [ebx+( 1* mx) + (-3) ],dl
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],dx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],dl
        mov     [ebx+( 3* mx) + (-1) ],dl
        mov     [ebx+( 3* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l308:
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + (-3) ],dl
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],dl
        mov     [ebx+(-2* mx) + (-4) ],dx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],dx
        mov     [ebx+(-1* mx) + (-4) ],dx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],dx
        mov     [ebx+( 0* mx) + (-4) ],dx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],dx
        mov     [ebx+( 1* mx) + (-4) ],dx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],dx
        mov     [ebx+( 2* mx) + (-3) ],dl
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],dl
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l309:
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],dl
        mov     [ebx+(-3* mx) + (-3) ],dl
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],dx
        mov     [ebx+(-2* mx) + (-4) ],dx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],dx
        mov     [ebx+(-2* mx) + ( 4) ],dl
        mov     [ebx+(-1* mx) + (-4) ],dx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],dx
        mov     [ebx+(-1* mx) + ( 4) ],dl
        mov     [ebx+( 0* mx) + (-4) ],dx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],dx
        mov     [ebx+( 0* mx) + ( 4) ],dl
        mov     [ebx+( 1* mx) + (-4) ],dx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],dx
        mov     [ebx+( 1* mx) + ( 4) ],dl
        mov     [ebx+( 2* mx) + (-4) ],dx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],dx
        mov     [ebx+( 2* mx) + ( 4) ],dl
        mov     [ebx+( 3* mx) + (-3) ],dl
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],dx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l310:
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + (-4) ],dx
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],dx
        mov     [ebx+(-3* mx) + (-4) ],dx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],dx
        mov     [ebx+(-2* mx) + (-5) ],dl
        mov     [ebx+(-2* mx) + (-4) ],dx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],dx
        mov     [ebx+(-2* mx) + ( 4) ],dl
        mov     [ebx+(-1* mx) + (-5) ],dl
        mov     [ebx+(-1* mx) + (-4) ],dx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],dx
        mov     [ebx+(-1* mx) + ( 4) ],dl
        mov     [ebx+( 0* mx) + (-5) ],dl
        mov     [ebx+( 0* mx) + (-4) ],dx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],dx
        mov     [ebx+( 0* mx) + ( 4) ],dl
        mov     [ebx+( 1* mx) + (-5) ],dl
        mov     [ebx+( 1* mx) + (-4) ],dx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],dx
        mov     [ebx+( 1* mx) + ( 4) ],dl
        mov     [ebx+( 2* mx) + (-4) ],dx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],dx
        mov     [ebx+( 3* mx) + (-4) ],dx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],dx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l311:
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + ( 2) ],dl
        mov     [ebx+(-4* mx) + (-3) ],dl
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],dx
        mov     [ebx+(-3* mx) + (-4) ],dx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],dx
        mov     [ebx+(-3* mx) + ( 4) ],dl
        mov     [ebx+(-2* mx) + (-5) ],dl
        mov     [ebx+(-2* mx) + (-4) ],dx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],edx
        mov     [ebx+(-1* mx) + (-5) ],dl
        mov     [ebx+(-1* mx) + (-4) ],dx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],edx
        mov     [ebx+( 0* mx) + (-5) ],dl
        mov     [ebx+( 0* mx) + (-4) ],dx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],edx
        mov     [ebx+( 1* mx) + (-5) ],dl
        mov     [ebx+( 1* mx) + (-4) ],dx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],edx
        mov     [ebx+( 2* mx) + (-5) ],dl
        mov     [ebx+( 2* mx) + (-4) ],dx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],edx
        mov     [ebx+( 3* mx) + (-4) ],dx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],dx
        mov     [ebx+( 3* mx) + ( 4) ],dl
        mov     [ebx+( 4* mx) + (-3) ],dl
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],dx
        mov     [ebx+( 5* mx) + (-2) ],edx
        mov     [ebx+( 5* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l312:
        mov     [ebx+(-6* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + (-4) ],dx
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + ( 2) ],dx
        mov     [ebx+(-4* mx) + (-5) ],dl
        mov     [ebx+(-4* mx) + (-4) ],dx
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],dx
        mov     [ebx+(-4* mx) + ( 4) ],dl
        mov     [ebx+(-3* mx) + (-5) ],dl
        mov     [ebx+(-3* mx) + (-4) ],dx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],dx
        mov     [ebx+(-3* mx) + ( 4) ],dl
        mov     [ebx+(-2* mx) + (-6) ],edx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],edx
        mov     [ebx+(-1* mx) + (-6) ],edx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],edx
        mov     [ebx+( 0* mx) + (-6) ],edx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],edx
        mov     [ebx+( 1* mx) + (-6) ],edx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],edx
        mov     [ebx+( 2* mx) + (-5) ],dl
        mov     [ebx+( 2* mx) + (-4) ],dx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],dx
        mov     [ebx+( 2* mx) + ( 4) ],dl
        mov     [ebx+( 3* mx) + (-5) ],dl
        mov     [ebx+( 3* mx) + (-4) ],dx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],dx
        mov     [ebx+( 3* mx) + ( 4) ],dl
        mov     [ebx+( 4* mx) + (-4) ],dx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],dx
        mov     [ebx+( 5* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l313:
        mov     [ebx+(-6* mx) + (-2) ],edx
        mov     [ebx+(-6* mx) + ( 2) ],dl
        mov     [ebx+(-5* mx) + (-4) ],dx
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + ( 2) ],dx
        mov     [ebx+(-5* mx) + ( 4) ],dl
        mov     [ebx+(-4* mx) + (-5) ],dl
        mov     [ebx+(-4* mx) + (-4) ],dx
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],edx
        mov     [ebx+(-3* mx) + (-5) ],dl
        mov     [ebx+(-3* mx) + (-4) ],dx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],edx
        mov     [ebx+(-2* mx) + (-6) ],edx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],edx
        mov     [ebx+(-2* mx) + ( 6) ],dl
        mov     [ebx+(-1* mx) + (-6) ],edx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],edx
        mov     [ebx+(-1* mx) + ( 6) ],dl
        mov     [ebx+( 0* mx) + (-6) ],edx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],edx
        mov     [ebx+( 0* mx) + ( 6) ],dl
        mov     [ebx+( 1* mx) + (-6) ],edx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],edx
        mov     [ebx+( 1* mx) + ( 6) ],dl
        mov     [ebx+( 2* mx) + (-6) ],edx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],edx
        mov     [ebx+( 2* mx) + ( 6) ],dl
        mov     [ebx+( 3* mx) + (-5) ],dl
        mov     [ebx+( 3* mx) + (-4) ],dx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],edx
        mov     [ebx+( 4* mx) + (-5) ],dl
        mov     [ebx+( 4* mx) + (-4) ],dx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],edx
        mov     [ebx+( 5* mx) + (-4) ],dx
        mov     [ebx+( 5* mx) + (-2) ],edx
        mov     [ebx+( 5* mx) + ( 2) ],dx
        mov     [ebx+( 5* mx) + ( 4) ],dl
        mov     [ebx+( 6* mx) + (-2) ],edx
        mov     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l314:
        mov     [ebx+(-7* mx) + (-3) ],dl
        mov     [ebx+(-7* mx) + (-2) ],edx
        mov     [ebx+(-7* mx) + ( 2) ],dl
        mov     [ebx+(-6* mx) + (-4) ],dx
        mov     [ebx+(-6* mx) + (-2) ],edx
        mov     [ebx+(-6* mx) + ( 2) ],dx
        mov     [ebx+(-5* mx) + (-5) ],dl
        mov     [ebx+(-5* mx) + (-4) ],dx
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + ( 2) ],dx
        mov     [ebx+(-5* mx) + ( 4) ],dl
        mov     [ebx+(-4* mx) + (-6) ],edx
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],edx
        mov     [ebx+(-3* mx) + (-7) ],dl
        mov     [ebx+(-3* mx) + (-6) ],edx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],edx
        mov     [ebx+(-3* mx) + ( 6) ],dl
        mov     [ebx+(-2* mx) + (-7) ],dl
        mov     [ebx+(-2* mx) + (-6) ],edx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],edx
        mov     [ebx+(-2* mx) + ( 6) ],dl
        mov     [ebx+(-1* mx) + (-7) ],dl
        mov     [ebx+(-1* mx) + (-6) ],edx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],edx
        mov     [ebx+(-1* mx) + ( 6) ],dl
        mov     [ebx+( 0* mx) + (-7) ],dl
        mov     [ebx+( 0* mx) + (-6) ],edx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],edx
        mov     [ebx+( 0* mx) + ( 6) ],dl
        mov     [ebx+( 1* mx) + (-7) ],dl
        mov     [ebx+( 1* mx) + (-6) ],edx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],edx
        mov     [ebx+( 1* mx) + ( 6) ],dl
        mov     [ebx+( 2* mx) + (-7) ],dl
        mov     [ebx+( 2* mx) + (-6) ],edx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],edx
        mov     [ebx+( 2* mx) + ( 6) ],dl
        mov     [ebx+( 3* mx) + (-6) ],edx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],edx
        mov     [ebx+( 4* mx) + (-5) ],dl
        mov     [ebx+( 4* mx) + (-4) ],dx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],dx
        mov     [ebx+( 4* mx) + ( 4) ],dl
        mov     [ebx+( 5* mx) + (-4) ],dx
        mov     [ebx+( 5* mx) + (-2) ],edx
        mov     [ebx+( 5* mx) + ( 2) ],dx
        mov     [ebx+( 6* mx) + (-3) ],dl
        mov     [ebx+( 6* mx) + (-2) ],edx
        mov     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l315:
        mov     [ebx+(-7* mx) + (-2) ],edx
        mov     [ebx+(-7* mx) + ( 2) ],dl
        mov     [ebx+(-6* mx) + (-4) ],dx
        mov     [ebx+(-6* mx) + (-2) ],edx
        mov     [ebx+(-6* mx) + ( 2) ],dx
        mov     [ebx+(-6* mx) + ( 4) ],dl
        mov     [ebx+(-5* mx) + (-5) ],dl
        mov     [ebx+(-5* mx) + (-4) ],dx
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + ( 2) ],edx
        mov     [ebx+(-4* mx) + (-6) ],edx
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],edx
        mov     [ebx+(-4* mx) + ( 6) ],dl
        mov     [ebx+(-3* mx) + (-6) ],edx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],edx
        mov     [ebx+(-3* mx) + ( 6) ],dl
        mov     [ebx+(-2* mx) + (-7) ],dl
        mov     [ebx+(-2* mx) + (-6) ],edx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],edx
        mov     [ebx+(-2* mx) + ( 6) ],dx
        mov     [ebx+(-1* mx) + (-7) ],dl
        mov     [ebx+(-1* mx) + (-6) ],edx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],edx
        mov     [ebx+(-1* mx) + ( 6) ],dx
        mov     [ebx+( 0* mx) + (-7) ],dl
        mov     [ebx+( 0* mx) + (-6) ],edx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],edx
        mov     [ebx+( 0* mx) + ( 6) ],dx
        mov     [ebx+( 1* mx) + (-7) ],dl
        mov     [ebx+( 1* mx) + (-6) ],edx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],edx
        mov     [ebx+( 1* mx) + ( 6) ],dx
        mov     [ebx+( 2* mx) + (-7) ],dl
        mov     [ebx+( 2* mx) + (-6) ],edx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],edx
        mov     [ebx+( 2* mx) + ( 6) ],dx
        mov     [ebx+( 3* mx) + (-6) ],edx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],edx
        mov     [ebx+( 3* mx) + ( 6) ],dl
        mov     [ebx+( 4* mx) + (-6) ],edx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],edx
        mov     [ebx+( 4* mx) + ( 6) ],dl
        mov     [ebx+( 5* mx) + (-5) ],dl
        mov     [ebx+( 5* mx) + (-4) ],dx
        mov     [ebx+( 5* mx) + (-2) ],edx
        mov     [ebx+( 5* mx) + ( 2) ],edx
        mov     [ebx+( 6* mx) + (-4) ],dx
        mov     [ebx+( 6* mx) + (-2) ],edx
        mov     [ebx+( 6* mx) + ( 2) ],dx
        mov     [ebx+( 6* mx) + ( 4) ],dl
        mov     [ebx+( 7* mx) + (-2) ],edx
        mov     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l316:
        mov     [ebx+(-8* mx) + (-3) ],dl
        mov     [ebx+(-8* mx) + (-2) ],edx
        mov     [ebx+(-8* mx) + ( 2) ],dl
        mov     [ebx+(-7* mx) + (-5) ],dl
        mov     [ebx+(-7* mx) + (-4) ],dx
        mov     [ebx+(-7* mx) + (-2) ],edx
        mov     [ebx+(-7* mx) + ( 2) ],dx
        mov     [ebx+(-7* mx) + ( 4) ],dl
        mov     [ebx+(-6* mx) + (-6) ],edx
        mov     [ebx+(-6* mx) + (-2) ],edx
        mov     [ebx+(-6* mx) + ( 2) ],edx
        mov     [ebx+(-5* mx) + (-7) ],dl
        mov     [ebx+(-5* mx) + (-6) ],edx
        mov     [ebx+(-5* mx) + (-2) ],edx
        mov     [ebx+(-5* mx) + ( 2) ],edx
        mov     [ebx+(-5* mx) + ( 6) ],dl
        mov     [ebx+(-4* mx) + (-7) ],dl
        mov     [ebx+(-4* mx) + (-6) ],edx
        mov     [ebx+(-4* mx) + (-2) ],edx
        mov     [ebx+(-4* mx) + ( 2) ],edx
        mov     [ebx+(-4* mx) + ( 6) ],dl
        mov     [ebx+(-3* mx) + (-8) ],dx
        mov     [ebx+(-3* mx) + (-6) ],edx
        mov     [ebx+(-3* mx) + (-2) ],edx
        mov     [ebx+(-3* mx) + ( 2) ],edx
        mov     [ebx+(-3* mx) + ( 6) ],dx
        mov     [ebx+(-2* mx) + (-8) ],dx
        mov     [ebx+(-2* mx) + (-6) ],edx
        mov     [ebx+(-2* mx) + (-2) ],edx
        mov     [ebx+(-2* mx) + ( 2) ],edx
        mov     [ebx+(-2* mx) + ( 6) ],dx
        mov     [ebx+(-1* mx) + (-8) ],dx
        mov     [ebx+(-1* mx) + (-6) ],edx
        mov     [ebx+(-1* mx) + (-2) ],edx
        mov     [ebx+(-1* mx) + ( 2) ],edx
        mov     [ebx+(-1* mx) + ( 6) ],dx
        mov     [ebx+( 0* mx) + (-8) ],dx
        mov     [ebx+( 0* mx) + (-6) ],edx
        mov     [ebx+( 0* mx) + (-2) ],edx
        mov     [ebx+( 0* mx) + ( 2) ],edx
        mov     [ebx+( 0* mx) + ( 6) ],dx
        mov     [ebx+( 1* mx) + (-8) ],dx
        mov     [ebx+( 1* mx) + (-6) ],edx
        mov     [ebx+( 1* mx) + (-2) ],edx
        mov     [ebx+( 1* mx) + ( 2) ],edx
        mov     [ebx+( 1* mx) + ( 6) ],dx
        mov     [ebx+( 2* mx) + (-8) ],dx
        mov     [ebx+( 2* mx) + (-6) ],edx
        mov     [ebx+( 2* mx) + (-2) ],edx
        mov     [ebx+( 2* mx) + ( 2) ],edx
        mov     [ebx+( 2* mx) + ( 6) ],dx
        mov     [ebx+( 3* mx) + (-7) ],dl
        mov     [ebx+( 3* mx) + (-6) ],edx
        mov     [ebx+( 3* mx) + (-2) ],edx
        mov     [ebx+( 3* mx) + ( 2) ],edx
        mov     [ebx+( 3* mx) + ( 6) ],dl
        mov     [ebx+( 4* mx) + (-7) ],dl
        mov     [ebx+( 4* mx) + (-6) ],edx
        mov     [ebx+( 4* mx) + (-2) ],edx
        mov     [ebx+( 4* mx) + ( 2) ],edx
        mov     [ebx+( 4* mx) + ( 6) ],dl
        mov     [ebx+( 5* mx) + (-6) ],edx
        mov     [ebx+( 5* mx) + (-2) ],edx
        mov     [ebx+( 5* mx) + ( 2) ],edx
        mov     [ebx+( 6* mx) + (-5) ],dl
        mov     [ebx+( 6* mx) + (-4) ],dx
        mov     [ebx+( 6* mx) + (-2) ],edx
        mov     [ebx+( 6* mx) + ( 2) ],dx
        mov     [ebx+( 6* mx) + ( 4) ],dl
        mov     [ebx+( 7* mx) + (-3) ],dl
        mov     [ebx+( 7* mx) + (-2) ],edx
        mov     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l401:
        mov     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l402:
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l403:
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + ( 1) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + ( 1) ],dl
        mov     [ebx+( 1* mx) + (-1) ],dx
        mov     [ebx+( 1* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l404:
        mov     [ebx+(-2* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + (-2) ],dl
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + ( 1) ],dl
        mov     [ebx+( 0* mx) + (-2) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + ( 1) ],dl
        mov     [ebx+( 1* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l405:
        mov     [ebx+(-2* mx) + (-1) ],dx
        mov     [ebx+(-2* mx) + ( 1) ],dl
        mov     [ebx+(-1* mx) + (-2) ],dl
        mov     [ebx+(-1* mx) + (-1) ],dx
        mov     [ebx+(-1* mx) + ( 1) ],dx
        mov     [ebx+( 0* mx) + (-2) ],dl
        mov     [ebx+( 0* mx) + (-1) ],dx
        mov     [ebx+( 0* mx) + ( 1) ],dx
        mov     [ebx+( 1* mx) + (-2) ],dl
        mov     [ebx+( 1* mx) + (-1) ],dx
        mov     [ebx+( 1* mx) + ( 1) ],dx
        mov     [ebx+( 2* mx) + (-1) ],dx
        mov     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l406:
        mov     [ebx+(-3* mx) + (-2) ],dl
        mov     [ebx+(-3* mx) + (-1) ],dx
        mov     [ebx+(-3* mx) + ( 1) ],dl
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],dx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],dx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],dx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],dx
        mov     [ebx+( 2* mx) + (-2) ],dl
        mov     [ebx+( 2* mx) + (-1) ],dx
        mov     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l407:
        mov     [ebx+(-3* mx) + (-1) ],dx
        mov     [ebx+(-3* mx) + ( 1) ],dl
        mov     [ebx+(-2* mx) + (-2) ],dl
        mov     [ebx+(-2* mx) + (-1) ],dx
        mov     [ebx+(-2* mx) + ( 1) ],dx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],dx
        mov     [ebx+(-1* mx) + ( 3) ],dl
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],dx
        mov     [ebx+( 0* mx) + ( 3) ],dl
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],dx
        mov     [ebx+( 1* mx) + ( 3) ],dl
        mov     [ebx+( 2* mx) + (-2) ],dl
        mov     [ebx+( 2* mx) + (-1) ],dx
        mov     [ebx+( 2* mx) + ( 1) ],dx
        mov     [ebx+( 3* mx) + (-1) ],dx
        mov     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l408:
        mov     [ebx+(-4* mx) + (-2) ],dl
        mov     [ebx+(-4* mx) + (-1) ],dx
        mov     [ebx+(-4* mx) + ( 1) ],dl
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],dx
        mov     [ebx+(-2* mx) + (-4) ],dl
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],dx
        mov     [ebx+(-2* mx) + ( 3) ],dl
        mov     [ebx+(-1* mx) + (-4) ],dl
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],dx
        mov     [ebx+(-1* mx) + ( 3) ],dl
        mov     [ebx+( 0* mx) + (-4) ],dl
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],dx
        mov     [ebx+( 0* mx) + ( 3) ],dl
        mov     [ebx+( 1* mx) + (-4) ],dl
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],dx
        mov     [ebx+( 1* mx) + ( 3) ],dl
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],dx
        mov     [ebx+( 3* mx) + (-2) ],dl
        mov     [ebx+( 3* mx) + (-1) ],dx
        mov     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l409:
        mov     [ebx+(-4* mx) + (-2) ],dl
        mov     [ebx+(-4* mx) + (-1) ],dx
        mov     [ebx+(-4* mx) + ( 1) ],dx
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],dx
        mov     [ebx+(-3* mx) + ( 3) ],dl
        mov     [ebx+(-2* mx) + (-4) ],dl
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + (-4) ],dl
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + (-4) ],dl
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + (-4) ],dl
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + (-4) ],dl
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],dx
        mov     [ebx+( 3* mx) + ( 3) ],dl
        mov     [ebx+( 4* mx) + (-2) ],dl
        mov     [ebx+( 4* mx) + (-1) ],dx
        mov     [ebx+( 4* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l410:
        mov     [ebx+(-5* mx) + (-2) ],dl
        mov     [ebx+(-5* mx) + (-1) ],dx
        mov     [ebx+(-5* mx) + ( 1) ],dl
        mov     [ebx+(-4* mx) + (-4) ],dl
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],dx
        mov     [ebx+(-4* mx) + ( 3) ],dl
        mov     [ebx+(-3* mx) + (-4) ],dl
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],dx
        mov     [ebx+(-3* mx) + ( 3) ],dl
        mov     [ebx+(-2* mx) + (-5) ],dx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + (-5) ],dx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + (-5) ],dx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + (-5) ],dx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + (-4) ],dl
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],dx
        mov     [ebx+( 2* mx) + ( 3) ],dl
        mov     [ebx+( 3* mx) + (-4) ],dl
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],dx
        mov     [ebx+( 3* mx) + ( 3) ],dl
        mov     [ebx+( 4* mx) + (-2) ],dl
        mov     [ebx+( 4* mx) + (-1) ],dx
        mov     [ebx+( 4* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l411:
        mov     [ebx+(-5* mx) + (-2) ],dl
        mov     [ebx+(-5* mx) + (-1) ],dx
        mov     [ebx+(-5* mx) + ( 1) ],dx
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],dx
        mov     [ebx+(-4* mx) + ( 3) ],dl
        mov     [ebx+(-3* mx) + (-4) ],dl
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + (-5) ],dx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + ( 5) ],dl
        mov     [ebx+(-1* mx) + (-5) ],dx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + ( 5) ],dl
        mov     [ebx+( 0* mx) + (-5) ],dx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + ( 5) ],dl
        mov     [ebx+( 1* mx) + (-5) ],dx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + ( 5) ],dl
        mov     [ebx+( 2* mx) + (-5) ],dx
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + ( 5) ],dl
        mov     [ebx+( 3* mx) + (-4) ],dl
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],edx
        mov     [ebx+( 4* mx) + (-3) ],edx
        mov     [ebx+( 4* mx) + ( 1) ],dx
        mov     [ebx+( 4* mx) + ( 3) ],dl
        mov     [ebx+( 5* mx) + (-2) ],dl
        mov     [ebx+( 5* mx) + (-1) ],dx
        mov     [ebx+( 5* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l412:
        mov     [ebx+(-6* mx) + (-2) ],dl
        mov     [ebx+(-6* mx) + (-1) ],dx
        mov     [ebx+(-6* mx) + ( 1) ],dl
        mov     [ebx+(-5* mx) + (-4) ],dl
        mov     [ebx+(-5* mx) + (-3) ],edx
        mov     [ebx+(-5* mx) + ( 1) ],dx
        mov     [ebx+(-5* mx) + ( 3) ],dl
        mov     [ebx+(-4* mx) + (-5) ],dx
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],edx
        mov     [ebx+(-3* mx) + (-5) ],dx
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + (-6) ],dl
        mov     [ebx+(-2* mx) + (-5) ],dx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + ( 5) ],dl
        mov     [ebx+(-1* mx) + (-6) ],dl
        mov     [ebx+(-1* mx) + (-5) ],dx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + ( 5) ],dl
        mov     [ebx+( 0* mx) + (-6) ],dl
        mov     [ebx+( 0* mx) + (-5) ],dx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + ( 5) ],dl
        mov     [ebx+( 1* mx) + (-6) ],dl
        mov     [ebx+( 1* mx) + (-5) ],dx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + ( 5) ],dl
        mov     [ebx+( 2* mx) + (-5) ],dx
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 3* mx) + (-5) ],dx
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],edx
        mov     [ebx+( 4* mx) + (-4) ],dl
        mov     [ebx+( 4* mx) + (-3) ],edx
        mov     [ebx+( 4* mx) + ( 1) ],dx
        mov     [ebx+( 4* mx) + ( 3) ],dl
        mov     [ebx+( 5* mx) + (-2) ],dl
        mov     [ebx+( 5* mx) + (-1) ],dx
        mov     [ebx+( 5* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l413:
        mov     [ebx+(-6* mx) + (-2) ],dl
        mov     [ebx+(-6* mx) + (-1) ],dx
        mov     [ebx+(-6* mx) + ( 1) ],dx
        mov     [ebx+(-5* mx) + (-4) ],dl
        mov     [ebx+(-5* mx) + (-3) ],edx
        mov     [ebx+(-5* mx) + ( 1) ],edx
        mov     [ebx+(-4* mx) + (-5) ],dx
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],edx
        mov     [ebx+(-4* mx) + ( 5) ],dl
        mov     [ebx+(-3* mx) + (-5) ],dx
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],edx
        mov     [ebx+(-3* mx) + ( 5) ],dl
        mov     [ebx+(-2* mx) + (-6) ],dl
        mov     [ebx+(-2* mx) + (-5) ],dx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + ( 5) ],dx
        mov     [ebx+(-1* mx) + (-6) ],dl
        mov     [ebx+(-1* mx) + (-5) ],dx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + ( 5) ],dx
        mov     [ebx+( 0* mx) + (-6) ],dl
        mov     [ebx+( 0* mx) + (-5) ],dx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + ( 5) ],dx
        mov     [ebx+( 1* mx) + (-6) ],dl
        mov     [ebx+( 1* mx) + (-5) ],dx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + ( 5) ],dx
        mov     [ebx+( 2* mx) + (-6) ],dl
        mov     [ebx+( 2* mx) + (-5) ],dx
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + ( 5) ],dx
        mov     [ebx+( 3* mx) + (-5) ],dx
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],edx
        mov     [ebx+( 3* mx) + ( 5) ],dl
        mov     [ebx+( 4* mx) + (-5) ],dx
        mov     [ebx+( 4* mx) + (-3) ],edx
        mov     [ebx+( 4* mx) + ( 1) ],edx
        mov     [ebx+( 4* mx) + ( 5) ],dl
        mov     [ebx+( 5* mx) + (-4) ],dl
        mov     [ebx+( 5* mx) + (-3) ],edx
        mov     [ebx+( 5* mx) + ( 1) ],edx
        mov     [ebx+( 6* mx) + (-2) ],dl
        mov     [ebx+( 6* mx) + (-1) ],dx
        mov     [ebx+( 6* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l414:
        mov     [ebx+(-7* mx) + (-3) ],edx
        mov     [ebx+(-7* mx) + ( 1) ],dx
        mov     [ebx+(-6* mx) + (-4) ],dl
        mov     [ebx+(-6* mx) + (-3) ],edx
        mov     [ebx+(-6* mx) + ( 1) ],dx
        mov     [ebx+(-6* mx) + ( 3) ],dl
        mov     [ebx+(-5* mx) + (-5) ],dx
        mov     [ebx+(-5* mx) + (-3) ],edx
        mov     [ebx+(-5* mx) + ( 1) ],edx
        mov     [ebx+(-4* mx) + (-6) ],dl
        mov     [ebx+(-4* mx) + (-5) ],dx
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],edx
        mov     [ebx+(-4* mx) + ( 5) ],dl
        mov     [ebx+(-3* mx) + (-7) ],edx
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],edx
        mov     [ebx+(-3* mx) + ( 5) ],dx
        mov     [ebx+(-2* mx) + (-7) ],edx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + ( 5) ],dx
        mov     [ebx+(-1* mx) + (-7) ],edx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + ( 5) ],dx
        mov     [ebx+( 0* mx) + (-7) ],edx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + ( 5) ],dx
        mov     [ebx+( 1* mx) + (-7) ],edx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + ( 5) ],dx
        mov     [ebx+( 2* mx) + (-7) ],edx
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + ( 5) ],dx
        mov     [ebx+( 3* mx) + (-6) ],dl
        mov     [ebx+( 3* mx) + (-5) ],dx
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],edx
        mov     [ebx+( 3* mx) + ( 5) ],dl
        mov     [ebx+( 4* mx) + (-5) ],dx
        mov     [ebx+( 4* mx) + (-3) ],edx
        mov     [ebx+( 4* mx) + ( 1) ],edx
        mov     [ebx+( 5* mx) + (-4) ],dl
        mov     [ebx+( 5* mx) + (-3) ],edx
        mov     [ebx+( 5* mx) + ( 1) ],dx
        mov     [ebx+( 5* mx) + ( 3) ],dl
        mov     [ebx+( 6* mx) + (-3) ],edx
        mov     [ebx+( 6* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l415:
        mov     [ebx+(-7* mx) + (-2) ],dl
        mov     [ebx+(-7* mx) + (-1) ],dx
        mov     [ebx+(-7* mx) + ( 1) ],dx
        mov     [ebx+(-6* mx) + (-4) ],dl
        mov     [ebx+(-6* mx) + (-3) ],edx
        mov     [ebx+(-6* mx) + ( 1) ],edx
        mov     [ebx+(-5* mx) + (-5) ],dx
        mov     [ebx+(-5* mx) + (-3) ],edx
        mov     [ebx+(-5* mx) + ( 1) ],edx
        mov     [ebx+(-5* mx) + ( 5) ],dl
        mov     [ebx+(-4* mx) + (-6) ],dl
        mov     [ebx+(-4* mx) + (-5) ],dx
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],edx
        mov     [ebx+(-4* mx) + ( 5) ],dx
        mov     [ebx+(-3* mx) + (-6) ],dl
        mov     [ebx+(-3* mx) + (-5) ],dx
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],edx
        mov     [ebx+(-3* mx) + ( 5) ],dx
        mov     [ebx+(-2* mx) + (-7) ],edx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + ( 5) ],dx
        mov     [ebx+(-2* mx) + ( 7) ],dl
        mov     [ebx+(-1* mx) + (-7) ],edx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + ( 5) ],dx
        mov     [ebx+(-1* mx) + ( 7) ],dl
        mov     [ebx+( 0* mx) + (-7) ],edx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + ( 5) ],dx
        mov     [ebx+( 0* mx) + ( 7) ],dl
        mov     [ebx+( 1* mx) + (-7) ],edx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + ( 5) ],dx
        mov     [ebx+( 1* mx) + ( 7) ],dl
        mov     [ebx+( 2* mx) + (-7) ],edx
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + ( 5) ],dx
        mov     [ebx+( 2* mx) + ( 7) ],dl
        mov     [ebx+( 3* mx) + (-6) ],dl
        mov     [ebx+( 3* mx) + (-5) ],dx
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],edx
        mov     [ebx+( 3* mx) + ( 5) ],dx
        mov     [ebx+( 4* mx) + (-6) ],dl
        mov     [ebx+( 4* mx) + (-5) ],dx
        mov     [ebx+( 4* mx) + (-3) ],edx
        mov     [ebx+( 4* mx) + ( 1) ],edx
        mov     [ebx+( 4* mx) + ( 5) ],dx
        mov     [ebx+( 5* mx) + (-5) ],dx
        mov     [ebx+( 5* mx) + (-3) ],edx
        mov     [ebx+( 5* mx) + ( 1) ],edx
        mov     [ebx+( 5* mx) + ( 5) ],dl
        mov     [ebx+( 6* mx) + (-4) ],dl
        mov     [ebx+( 6* mx) + (-3) ],edx
        mov     [ebx+( 6* mx) + ( 1) ],edx
        mov     [ebx+( 7* mx) + (-2) ],dl
        mov     [ebx+( 7* mx) + (-1) ],dx
        mov     [ebx+( 7* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
l416:
        mov     [ebx+(-8* mx) + (-3) ],edx
        mov     [ebx+(-8* mx) + ( 1) ],dx
        mov     [ebx+(-7* mx) + (-5) ],dx
        mov     [ebx+(-7* mx) + (-3) ],edx
        mov     [ebx+(-7* mx) + ( 1) ],edx
        mov     [ebx+(-6* mx) + (-6) ],dl
        mov     [ebx+(-6* mx) + (-5) ],dx
        mov     [ebx+(-6* mx) + (-3) ],edx
        mov     [ebx+(-6* mx) + ( 1) ],edx
        mov     [ebx+(-6* mx) + ( 5) ],dl
        mov     [ebx+(-5* mx) + (-7) ],edx
        mov     [ebx+(-5* mx) + (-3) ],edx
        mov     [ebx+(-5* mx) + ( 1) ],edx
        mov     [ebx+(-5* mx) + ( 5) ],dx
        mov     [ebx+(-4* mx) + (-7) ],edx
        mov     [ebx+(-4* mx) + (-3) ],edx
        mov     [ebx+(-4* mx) + ( 1) ],edx
        mov     [ebx+(-4* mx) + ( 5) ],dx
        mov     [ebx+(-3* mx) + (-8) ],dl
        mov     [ebx+(-3* mx) + (-7) ],edx
        mov     [ebx+(-3* mx) + (-3) ],edx
        mov     [ebx+(-3* mx) + ( 1) ],edx
        mov     [ebx+(-3* mx) + ( 5) ],dx
        mov     [ebx+(-3* mx) + ( 7) ],dl
        mov     [ebx+(-2* mx) + (-8) ],dl
        mov     [ebx+(-2* mx) + (-7) ],edx
        mov     [ebx+(-2* mx) + (-3) ],edx
        mov     [ebx+(-2* mx) + ( 1) ],edx
        mov     [ebx+(-2* mx) + ( 5) ],dx
        mov     [ebx+(-2* mx) + ( 7) ],dl
        mov     [ebx+(-1* mx) + (-8) ],dl
        mov     [ebx+(-1* mx) + (-7) ],edx
        mov     [ebx+(-1* mx) + (-3) ],edx
        mov     [ebx+(-1* mx) + ( 1) ],edx
        mov     [ebx+(-1* mx) + ( 5) ],dx
        mov     [ebx+(-1* mx) + ( 7) ],dl
        mov     [ebx+( 0* mx) + (-8) ],dl
        mov     [ebx+( 0* mx) + (-7) ],edx
        mov     [ebx+( 0* mx) + (-3) ],edx
        mov     [ebx+( 0* mx) + ( 1) ],edx
        mov     [ebx+( 0* mx) + ( 5) ],dx
        mov     [ebx+( 0* mx) + ( 7) ],dl
        mov     [ebx+( 1* mx) + (-8) ],dl
        mov     [ebx+( 1* mx) + (-7) ],edx
        mov     [ebx+( 1* mx) + (-3) ],edx
        mov     [ebx+( 1* mx) + ( 1) ],edx
        mov     [ebx+( 1* mx) + ( 5) ],dx
        mov     [ebx+( 1* mx) + ( 7) ],dl
        mov     [ebx+( 2* mx) + (-8) ],dl
        mov     [ebx+( 2* mx) + (-7) ],edx
        mov     [ebx+( 2* mx) + (-3) ],edx
        mov     [ebx+( 2* mx) + ( 1) ],edx
        mov     [ebx+( 2* mx) + ( 5) ],dx
        mov     [ebx+( 2* mx) + ( 7) ],dl
        mov     [ebx+( 3* mx) + (-7) ],edx
        mov     [ebx+( 3* mx) + (-3) ],edx
        mov     [ebx+( 3* mx) + ( 1) ],edx
        mov     [ebx+( 3* mx) + ( 5) ],dx
        mov     [ebx+( 4* mx) + (-7) ],edx
        mov     [ebx+( 4* mx) + (-3) ],edx
        mov     [ebx+( 4* mx) + ( 1) ],edx
        mov     [ebx+( 4* mx) + ( 5) ],dx
        mov     [ebx+( 5* mx) + (-6) ],dl
        mov     [ebx+( 5* mx) + (-5) ],dx
        mov     [ebx+( 5* mx) + (-3) ],edx
        mov     [ebx+( 5* mx) + ( 1) ],edx
        mov     [ebx+( 5* mx) + ( 5) ],dl
        mov     [ebx+( 6* mx) + (-5) ],dx
        mov     [ebx+( 6* mx) + (-3) ],edx
        mov     [ebx+( 6* mx) + ( 1) ],edx
        mov     [ebx+( 7* mx) + (-3) ],edx
        mov     [ebx+( 7* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a101:
        add     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a102:
        add     [ebx+(-1* mx) + (-1) ],dl
        add     [ebx+(-1* mx) + ( 0) ],dl
        add     [ebx+( 0* mx) + (-1) ],dl
        add     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a103:
        add     [ebx+(-1* mx) + (-1) ],dl
        add     [ebx+(-1* mx) + ( 0) ],dx
        add     [ebx+( 0* mx) + (-1) ],dl
        add     [ebx+( 0* mx) + ( 0) ],dx
        add     [ebx+( 1* mx) + (-1) ],dl
        add     [ebx+( 1* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a104:
        add     [ebx+(-2* mx) + (-1) ],dl
        add     [ebx+(-2* mx) + ( 0) ],dl
        add     [ebx+(-1* mx) + (-2) ],dx
        add     [ebx+(-1* mx) + ( 0) ],dx
        add     [ebx+( 0* mx) + (-2) ],dx
        add     [ebx+( 0* mx) + ( 0) ],dx
        add     [ebx+( 1* mx) + (-1) ],dl
        add     [ebx+( 1* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a105:
        add     [ebx+(-2* mx) + (-1) ],dl
        add     [ebx+(-2* mx) + ( 0) ],dx
        add     [ebx+(-1* mx) + (-2) ],dx
        add     [ebx+(-1* mx) + ( 0) ],dx
        add     [ebx+(-1* mx) + ( 2) ],dl
        add     [ebx+( 0* mx) + (-2) ],dx
        add     [ebx+( 0* mx) + ( 0) ],dx
        add     [ebx+( 0* mx) + ( 2) ],dl
        add     [ebx+( 1* mx) + (-2) ],dx
        add     [ebx+( 1* mx) + ( 0) ],dx
        add     [ebx+( 1* mx) + ( 2) ],dl
        add     [ebx+( 2* mx) + (-1) ],dl
        add     [ebx+( 2* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a106:
        add     [ebx+(-3* mx) + (-2) ],dx
        add     [ebx+(-3* mx) + ( 0) ],dx
        add     [ebx+(-2* mx) + (-3) ],dl
        add     [ebx+(-2* mx) + (-2) ],dx
        add     [ebx+(-2* mx) + ( 0) ],dx
        add     [ebx+(-2* mx) + ( 2) ],dl
        add     [ebx+(-1* mx) + (-3) ],dl
        add     [ebx+(-1* mx) + (-2) ],dx
        add     [ebx+(-1* mx) + ( 0) ],dx
        add     [ebx+(-1* mx) + ( 2) ],dl
        add     [ebx+( 0* mx) + (-3) ],dl
        add     [ebx+( 0* mx) + (-2) ],dx
        add     [ebx+( 0* mx) + ( 0) ],dx
        add     [ebx+( 0* mx) + ( 2) ],dl
        add     [ebx+( 1* mx) + (-3) ],dl
        add     [ebx+( 1* mx) + (-2) ],dx
        add     [ebx+( 1* mx) + ( 0) ],dx
        add     [ebx+( 1* mx) + ( 2) ],dl
        add     [ebx+( 2* mx) + (-2) ],dx
        add     [ebx+( 2* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a107:
        add     [ebx+(-3* mx) + (-1) ],dl
        add     [ebx+(-3* mx) + ( 0) ],dx
        add     [ebx+(-2* mx) + (-2) ],dx
        add     [ebx+(-2* mx) + ( 0) ],dx
        add     [ebx+(-2* mx) + ( 2) ],dl
        add     [ebx+(-1* mx) + (-3) ],dl
        add     [ebx+(-1* mx) + (-2) ],dx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + (-3) ],dl
        add     [ebx+( 0* mx) + (-2) ],dx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + (-3) ],dl
        add     [ebx+( 1* mx) + (-2) ],dx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + (-2) ],dx
        add     [ebx+( 2* mx) + ( 0) ],dx
        add     [ebx+( 2* mx) + ( 2) ],dl
        add     [ebx+( 3* mx) + (-1) ],dl
        add     [ebx+( 3* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a108:
        add     [ebx+(-4* mx) + (-2) ],dx
        add     [ebx+(-4* mx) + ( 0) ],dx
        add     [ebx+(-3* mx) + (-3) ],dl
        add     [ebx+(-3* mx) + (-2) ],dx
        add     [ebx+(-3* mx) + ( 0) ],dx
        add     [ebx+(-3* mx) + ( 2) ],dl
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + (-3) ],dl
        add     [ebx+( 2* mx) + (-2) ],dx
        add     [ebx+( 2* mx) + ( 0) ],dx
        add     [ebx+( 2* mx) + ( 2) ],dl
        add     [ebx+( 3* mx) + (-2) ],dx
        add     [ebx+( 3* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a109:
        add     [ebx+(-4* mx) + (-2) ],dx
        add     [ebx+(-4* mx) + ( 0) ],dx
        add     [ebx+(-4* mx) + ( 2) ],dl
        add     [ebx+(-3* mx) + (-3) ],dl
        add     [ebx+(-3* mx) + (-2) ],dx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],dl
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],dl
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],dl
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],dl
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],dl
        add     [ebx+( 3* mx) + (-3) ],dl
        add     [ebx+( 3* mx) + (-2) ],dx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 4* mx) + (-2) ],dx
        add     [ebx+( 4* mx) + ( 0) ],dx
        add     [ebx+( 4* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a110:
        add     [ebx+(-5* mx) + (-2) ],dx
        add     [ebx+(-5* mx) + ( 0) ],dx
        add     [ebx+(-4* mx) + (-4) ],edx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + (-5) ],dl
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],dl
        add     [ebx+(-1* mx) + (-5) ],dl
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],dl
        add     [ebx+( 0* mx) + (-5) ],dl
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],dl
        add     [ebx+( 1* mx) + (-5) ],dl
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],dl
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 4* mx) + (-2) ],dx
        add     [ebx+( 4* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a111:
        add     [ebx+(-5* mx) + (-2) ],dx
        add     [ebx+(-5* mx) + ( 0) ],dx
        add     [ebx+(-5* mx) + ( 2) ],dl
        add     [ebx+(-4* mx) + (-3) ],dl
        add     [ebx+(-4* mx) + (-2) ],dx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + ( 4) ],dl
        add     [ebx+(-2* mx) + (-5) ],dl
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],dx
        add     [ebx+(-1* mx) + (-5) ],dl
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],dx
        add     [ebx+( 0* mx) + (-5) ],dl
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],dx
        add     [ebx+( 1* mx) + (-5) ],dl
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],dx
        add     [ebx+( 2* mx) + (-5) ],dl
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],dx
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + ( 4) ],dl
        add     [ebx+( 4* mx) + (-3) ],dl
        add     [ebx+( 4* mx) + (-2) ],dx
        add     [ebx+( 4* mx) + ( 0) ],edx
        add     [ebx+( 5* mx) + (-2) ],dx
        add     [ebx+( 5* mx) + ( 0) ],dx
        add     [ebx+( 5* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a112:
        add     [ebx+(-6* mx) + (-2) ],dx
        add     [ebx+(-6* mx) + ( 0) ],dx
        add     [ebx+(-5* mx) + (-4) ],edx
        add     [ebx+(-5* mx) + ( 0) ],edx
        add     [ebx+(-4* mx) + (-5) ],dl
        add     [ebx+(-4* mx) + (-4) ],edx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-4* mx) + ( 4) ],dl
        add     [ebx+(-3* mx) + (-5) ],dl
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + ( 4) ],dl
        add     [ebx+(-2* mx) + (-6) ],dx
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],dx
        add     [ebx+(-1* mx) + (-6) ],dx
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],dx
        add     [ebx+( 0* mx) + (-6) ],dx
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],dx
        add     [ebx+( 1* mx) + (-6) ],dx
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],dx
        add     [ebx+( 2* mx) + (-5) ],dl
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],dl
        add     [ebx+( 3* mx) + (-5) ],dl
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + ( 4) ],dl
        add     [ebx+( 4* mx) + (-4) ],edx
        add     [ebx+( 4* mx) + ( 0) ],edx
        add     [ebx+( 5* mx) + (-2) ],dx
        add     [ebx+( 5* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a113:
        add     [ebx+(-6* mx) + (-2) ],dx
        add     [ebx+(-6* mx) + ( 0) ],dx
        add     [ebx+(-6* mx) + ( 2) ],dl
        add     [ebx+(-5* mx) + (-4) ],edx
        add     [ebx+(-5* mx) + ( 0) ],edx
        add     [ebx+(-5* mx) + ( 4) ],dl
        add     [ebx+(-4* mx) + (-5) ],dl
        add     [ebx+(-4* mx) + (-4) ],edx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-4* mx) + ( 4) ],dx
        add     [ebx+(-3* mx) + (-5) ],dl
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + ( 4) ],dx
        add     [ebx+(-2* mx) + (-6) ],dx
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],dx
        add     [ebx+(-2* mx) + ( 6) ],dl
        add     [ebx+(-1* mx) + (-6) ],dx
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],dx
        add     [ebx+(-1* mx) + ( 6) ],dl
        add     [ebx+( 0* mx) + (-6) ],dx
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],dx
        add     [ebx+( 0* mx) + ( 6) ],dl
        add     [ebx+( 1* mx) + (-6) ],dx
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],dx
        add     [ebx+( 1* mx) + ( 6) ],dl
        add     [ebx+( 2* mx) + (-6) ],dx
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],dx
        add     [ebx+( 2* mx) + ( 6) ],dl
        add     [ebx+( 3* mx) + (-5) ],dl
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + ( 4) ],dx
        add     [ebx+( 4* mx) + (-5) ],dl
        add     [ebx+( 4* mx) + (-4) ],edx
        add     [ebx+( 4* mx) + ( 0) ],edx
        add     [ebx+( 4* mx) + ( 4) ],dx
        add     [ebx+( 5* mx) + (-4) ],edx
        add     [ebx+( 5* mx) + ( 0) ],edx
        add     [ebx+( 5* mx) + ( 4) ],dl
        add     [ebx+( 6* mx) + (-2) ],dx
        add     [ebx+( 6* mx) + ( 0) ],dx
        add     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a114:
        add     [ebx+(-7* mx) + (-3) ],dl
        add     [ebx+(-7* mx) + (-2) ],dx
        add     [ebx+(-7* mx) + ( 0) ],dx
        add     [ebx+(-7* mx) + ( 2) ],dl
        add     [ebx+(-6* mx) + (-4) ],edx
        add     [ebx+(-6* mx) + ( 0) ],edx
        add     [ebx+(-5* mx) + (-5) ],dl
        add     [ebx+(-5* mx) + (-4) ],edx
        add     [ebx+(-5* mx) + ( 0) ],edx
        add     [ebx+(-5* mx) + ( 4) ],dl
        add     [ebx+(-4* mx) + (-6) ],dx
        add     [ebx+(-4* mx) + (-4) ],edx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-4* mx) + ( 4) ],dx
        add     [ebx+(-3* mx) + (-7) ],dl
        add     [ebx+(-3* mx) + (-6) ],dx
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + ( 4) ],dx
        add     [ebx+(-3* mx) + ( 6) ],dl
        add     [ebx+(-2* mx) + (-7) ],dl
        add     [ebx+(-2* mx) + (-6) ],dx
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],dx
        add     [ebx+(-2* mx) + ( 6) ],dl
        add     [ebx+(-1* mx) + (-7) ],dl
        add     [ebx+(-1* mx) + (-6) ],dx
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],dx
        add     [ebx+(-1* mx) + ( 6) ],dl
        add     [ebx+( 0* mx) + (-7) ],dl
        add     [ebx+( 0* mx) + (-6) ],dx
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],dx
        add     [ebx+( 0* mx) + ( 6) ],dl
        add     [ebx+( 1* mx) + (-7) ],dl
        add     [ebx+( 1* mx) + (-6) ],dx
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],dx
        add     [ebx+( 1* mx) + ( 6) ],dl
        add     [ebx+( 2* mx) + (-7) ],dl
        add     [ebx+( 2* mx) + (-6) ],dx
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],dx
        add     [ebx+( 2* mx) + ( 6) ],dl
        add     [ebx+( 3* mx) + (-6) ],dx
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + ( 4) ],dx
        add     [ebx+( 4* mx) + (-5) ],dl
        add     [ebx+( 4* mx) + (-4) ],edx
        add     [ebx+( 4* mx) + ( 0) ],edx
        add     [ebx+( 4* mx) + ( 4) ],dl
        add     [ebx+( 5* mx) + (-4) ],edx
        add     [ebx+( 5* mx) + ( 0) ],edx
        add     [ebx+( 6* mx) + (-3) ],dl
        add     [ebx+( 6* mx) + (-2) ],dx
        add     [ebx+( 6* mx) + ( 0) ],dx
        add     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a115:
        add     [ebx+(-7* mx) + (-2) ],dx
        add     [ebx+(-7* mx) + ( 0) ],dx
        add     [ebx+(-7* mx) + ( 2) ],dl
        add     [ebx+(-6* mx) + (-4) ],edx
        add     [ebx+(-6* mx) + ( 0) ],edx
        add     [ebx+(-6* mx) + ( 4) ],dl
        add     [ebx+(-5* mx) + (-5) ],dl
        add     [ebx+(-5* mx) + (-4) ],edx
        add     [ebx+(-5* mx) + ( 0) ],edx
        add     [ebx+(-5* mx) + ( 4) ],dx
        add     [ebx+(-4* mx) + (-6) ],dx
        add     [ebx+(-4* mx) + (-4) ],edx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-4* mx) + ( 4) ],dx
        add     [ebx+(-4* mx) + ( 6) ],dl
        add     [ebx+(-3* mx) + (-6) ],dx
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + ( 4) ],dx
        add     [ebx+(-3* mx) + ( 6) ],dl
        add     [ebx+(-2* mx) + (-7) ],dl
        add     [ebx+(-2* mx) + (-6) ],dx
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],edx
        add     [ebx+(-1* mx) + (-7) ],dl
        add     [ebx+(-1* mx) + (-6) ],dx
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],edx
        add     [ebx+( 0* mx) + (-7) ],dl
        add     [ebx+( 0* mx) + (-6) ],dx
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],edx
        add     [ebx+( 1* mx) + (-7) ],dl
        add     [ebx+( 1* mx) + (-6) ],dx
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],edx
        add     [ebx+( 2* mx) + (-7) ],dl
        add     [ebx+( 2* mx) + (-6) ],dx
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],edx
        add     [ebx+( 3* mx) + (-6) ],dx
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + ( 4) ],dx
        add     [ebx+( 3* mx) + ( 6) ],dl
        add     [ebx+( 4* mx) + (-6) ],dx
        add     [ebx+( 4* mx) + (-4) ],edx
        add     [ebx+( 4* mx) + ( 0) ],edx
        add     [ebx+( 4* mx) + ( 4) ],dx
        add     [ebx+( 4* mx) + ( 6) ],dl
        add     [ebx+( 5* mx) + (-5) ],dl
        add     [ebx+( 5* mx) + (-4) ],edx
        add     [ebx+( 5* mx) + ( 0) ],edx
        add     [ebx+( 5* mx) + ( 4) ],dx
        add     [ebx+( 6* mx) + (-4) ],edx
        add     [ebx+( 6* mx) + ( 0) ],edx
        add     [ebx+( 6* mx) + ( 4) ],dl
        add     [ebx+( 7* mx) + (-2) ],dx
        add     [ebx+( 7* mx) + ( 0) ],dx
        add     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a116:
        add     [ebx+(-8* mx) + (-3) ],dl
        add     [ebx+(-8* mx) + (-2) ],dx
        add     [ebx+(-8* mx) + ( 0) ],dx
        add     [ebx+(-8* mx) + ( 2) ],dl
        add     [ebx+(-7* mx) + (-5) ],dl
        add     [ebx+(-7* mx) + (-4) ],edx
        add     [ebx+(-7* mx) + ( 0) ],edx
        add     [ebx+(-7* mx) + ( 4) ],dl
        add     [ebx+(-6* mx) + (-6) ],dx
        add     [ebx+(-6* mx) + (-4) ],edx
        add     [ebx+(-6* mx) + ( 0) ],edx
        add     [ebx+(-6* mx) + ( 4) ],dx
        add     [ebx+(-5* mx) + (-7) ],dl
        add     [ebx+(-5* mx) + (-6) ],dx
        add     [ebx+(-5* mx) + (-4) ],edx
        add     [ebx+(-5* mx) + ( 0) ],edx
        add     [ebx+(-5* mx) + ( 4) ],dx
        add     [ebx+(-5* mx) + ( 6) ],dl
        add     [ebx+(-4* mx) + (-7) ],dl
        add     [ebx+(-4* mx) + (-6) ],dx
        add     [ebx+(-4* mx) + (-4) ],edx
        add     [ebx+(-4* mx) + ( 0) ],edx
        add     [ebx+(-4* mx) + ( 4) ],dx
        add     [ebx+(-4* mx) + ( 6) ],dl
        add     [ebx+(-3* mx) + (-8) ],edx
        add     [ebx+(-3* mx) + (-4) ],edx
        add     [ebx+(-3* mx) + ( 0) ],edx
        add     [ebx+(-3* mx) + ( 4) ],edx
        add     [ebx+(-2* mx) + (-8) ],edx
        add     [ebx+(-2* mx) + (-4) ],edx
        add     [ebx+(-2* mx) + ( 0) ],edx
        add     [ebx+(-2* mx) + ( 4) ],edx
        add     [ebx+(-1* mx) + (-8) ],edx
        add     [ebx+(-1* mx) + (-4) ],edx
        add     [ebx+(-1* mx) + ( 0) ],edx
        add     [ebx+(-1* mx) + ( 4) ],edx
        add     [ebx+( 0* mx) + (-8) ],edx
        add     [ebx+( 0* mx) + (-4) ],edx
        add     [ebx+( 0* mx) + ( 0) ],edx
        add     [ebx+( 0* mx) + ( 4) ],edx
        add     [ebx+( 1* mx) + (-8) ],edx
        add     [ebx+( 1* mx) + (-4) ],edx
        add     [ebx+( 1* mx) + ( 0) ],edx
        add     [ebx+( 1* mx) + ( 4) ],edx
        add     [ebx+( 2* mx) + (-8) ],edx
        add     [ebx+( 2* mx) + (-4) ],edx
        add     [ebx+( 2* mx) + ( 0) ],edx
        add     [ebx+( 2* mx) + ( 4) ],edx
        add     [ebx+( 3* mx) + (-7) ],dl
        add     [ebx+( 3* mx) + (-6) ],dx
        add     [ebx+( 3* mx) + (-4) ],edx
        add     [ebx+( 3* mx) + ( 0) ],edx
        add     [ebx+( 3* mx) + ( 4) ],dx
        add     [ebx+( 3* mx) + ( 6) ],dl
        add     [ebx+( 4* mx) + (-7) ],dl
        add     [ebx+( 4* mx) + (-6) ],dx
        add     [ebx+( 4* mx) + (-4) ],edx
        add     [ebx+( 4* mx) + ( 0) ],edx
        add     [ebx+( 4* mx) + ( 4) ],dx
        add     [ebx+( 4* mx) + ( 6) ],dl
        add     [ebx+( 5* mx) + (-6) ],dx
        add     [ebx+( 5* mx) + (-4) ],edx
        add     [ebx+( 5* mx) + ( 0) ],edx
        add     [ebx+( 5* mx) + ( 4) ],dx
        add     [ebx+( 6* mx) + (-5) ],dl
        add     [ebx+( 6* mx) + (-4) ],edx
        add     [ebx+( 6* mx) + ( 0) ],edx
        add     [ebx+( 6* mx) + ( 4) ],dl
        add     [ebx+( 7* mx) + (-3) ],dl
        add     [ebx+( 7* mx) + (-2) ],dx
        add     [ebx+( 7* mx) + ( 0) ],dx
        add     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a201:
        add     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a202:
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a203:
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + ( 1) ],dl
        add     [ebx+( 0* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + ( 1) ],dl
        add     [ebx+( 1* mx) + (-1) ],dx
        add     [ebx+( 1* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a204:
        add     [ebx+(-2* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + (-2) ],dl
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + ( 1) ],dl
        add     [ebx+( 0* mx) + (-2) ],dl
        add     [ebx+( 0* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + ( 1) ],dl
        add     [ebx+( 1* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a205:
        add     [ebx+(-2* mx) + (-1) ],dx
        add     [ebx+(-2* mx) + ( 1) ],dl
        add     [ebx+(-1* mx) + (-2) ],dl
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + (-2) ],dl
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + (-2) ],dl
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + (-1) ],dx
        add     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a206:
        add     [ebx+(-3* mx) + (-2) ],dl
        add     [ebx+(-3* mx) + (-1) ],dx
        add     [ebx+(-3* mx) + ( 1) ],dl
        add     [ebx+(-2* mx) + (-3) ],dx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + (-3) ],dx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + (-3) ],dx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + (-3) ],dx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + (-2) ],dl
        add     [ebx+( 2* mx) + (-1) ],dx
        add     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a207:
        add     [ebx+(-3* mx) + (-1) ],dx
        add     [ebx+(-3* mx) + ( 1) ],dl
        add     [ebx+(-2* mx) + (-2) ],dl
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + (-3) ],dx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],dl
        add     [ebx+( 0* mx) + (-3) ],dx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],dl
        add     [ebx+( 1* mx) + (-3) ],dx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],dl
        add     [ebx+( 2* mx) + (-2) ],dl
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + (-1) ],dx
        add     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a208:
        add     [ebx+(-4* mx) + (-2) ],dl
        add     [ebx+(-4* mx) + (-1) ],dx
        add     [ebx+(-4* mx) + ( 1) ],dl
        add     [ebx+(-3* mx) + (-3) ],dx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + (-4) ],dl
        add     [ebx+(-2* mx) + (-3) ],dx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],dl
        add     [ebx+(-1* mx) + (-4) ],dl
        add     [ebx+(-1* mx) + (-3) ],dx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],dl
        add     [ebx+( 0* mx) + (-4) ],dl
        add     [ebx+( 0* mx) + (-3) ],dx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],dl
        add     [ebx+( 1* mx) + (-4) ],dl
        add     [ebx+( 1* mx) + (-3) ],dx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],dl
        add     [ebx+( 2* mx) + (-3) ],dx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + (-2) ],dl
        add     [ebx+( 3* mx) + (-1) ],dx
        add     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a209:
        add     [ebx+(-4* mx) + (-2) ],dl
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + (-3) ],dx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],dl
        add     [ebx+(-2* mx) + (-4) ],dl
        add     [ebx+(-2* mx) + (-3) ],dx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],dx
        add     [ebx+(-1* mx) + (-4) ],dl
        add     [ebx+(-1* mx) + (-3) ],dx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],dx
        add     [ebx+( 0* mx) + (-4) ],dl
        add     [ebx+( 0* mx) + (-3) ],dx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],dx
        add     [ebx+( 1* mx) + (-4) ],dl
        add     [ebx+( 1* mx) + (-3) ],dx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],dx
        add     [ebx+( 2* mx) + (-4) ],dl
        add     [ebx+( 2* mx) + (-3) ],dx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],dx
        add     [ebx+( 3* mx) + (-3) ],dx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],dl
        add     [ebx+( 4* mx) + (-2) ],dl
        add     [ebx+( 4* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a210:
        add     [ebx+(-5* mx) + (-2) ],dl
        add     [ebx+(-5* mx) + (-1) ],dx
        add     [ebx+(-5* mx) + ( 1) ],dl
        add     [ebx+(-4* mx) + (-4) ],dl
        add     [ebx+(-4* mx) + (-3) ],dx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],dl
        add     [ebx+(-3* mx) + (-4) ],dl
        add     [ebx+(-3* mx) + (-3) ],dx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],dl
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],dx
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],dx
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],dx
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],dx
        add     [ebx+( 2* mx) + (-4) ],dl
        add     [ebx+( 2* mx) + (-3) ],dx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],dl
        add     [ebx+( 3* mx) + (-4) ],dl
        add     [ebx+( 3* mx) + (-3) ],dx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],dl
        add     [ebx+( 4* mx) + (-2) ],dl
        add     [ebx+( 4* mx) + (-1) ],dx
        add     [ebx+( 4* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a211:
        add     [ebx+(-5* mx) + (-2) ],dl
        add     [ebx+(-5* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + (-3) ],dx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],dl
        add     [ebx+(-3* mx) + (-4) ],dl
        add     [ebx+(-3* mx) + (-3) ],dx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],dx
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],dx
        add     [ebx+(-2* mx) + ( 5) ],dl
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],dx
        add     [ebx+(-1* mx) + ( 5) ],dl
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],dx
        add     [ebx+( 0* mx) + ( 5) ],dl
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],dx
        add     [ebx+( 1* mx) + ( 5) ],dl
        add     [ebx+( 2* mx) + (-5) ],edx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],dx
        add     [ebx+( 2* mx) + ( 5) ],dl
        add     [ebx+( 3* mx) + (-4) ],dl
        add     [ebx+( 3* mx) + (-3) ],dx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],dx
        add     [ebx+( 4* mx) + (-3) ],dx
        add     [ebx+( 4* mx) + (-1) ],edx
        add     [ebx+( 4* mx) + ( 3) ],dl
        add     [ebx+( 5* mx) + (-2) ],dl
        add     [ebx+( 5* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a212:
        add     [ebx+(-6* mx) + (-2) ],dl
        add     [ebx+(-6* mx) + (-1) ],dx
        add     [ebx+(-6* mx) + ( 1) ],dl
        add     [ebx+(-5* mx) + (-4) ],dl
        add     [ebx+(-5* mx) + (-3) ],dx
        add     [ebx+(-5* mx) + (-1) ],edx
        add     [ebx+(-5* mx) + ( 3) ],dl
        add     [ebx+(-4* mx) + (-5) ],edx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],dx
        add     [ebx+(-3* mx) + (-5) ],edx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],dx
        add     [ebx+(-2* mx) + (-6) ],dl
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],dx
        add     [ebx+(-2* mx) + ( 5) ],dl
        add     [ebx+(-1* mx) + (-6) ],dl
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],dx
        add     [ebx+(-1* mx) + ( 5) ],dl
        add     [ebx+( 0* mx) + (-6) ],dl
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],dx
        add     [ebx+( 0* mx) + ( 5) ],dl
        add     [ebx+( 1* mx) + (-6) ],dl
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],dx
        add     [ebx+( 1* mx) + ( 5) ],dl
        add     [ebx+( 2* mx) + (-5) ],edx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],dx
        add     [ebx+( 3* mx) + (-5) ],edx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],dx
        add     [ebx+( 4* mx) + (-4) ],dl
        add     [ebx+( 4* mx) + (-3) ],dx
        add     [ebx+( 4* mx) + (-1) ],edx
        add     [ebx+( 4* mx) + ( 3) ],dl
        add     [ebx+( 5* mx) + (-2) ],dl
        add     [ebx+( 5* mx) + (-1) ],dx
        add     [ebx+( 5* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a213:
        add     [ebx+(-6* mx) + (-2) ],dl
        add     [ebx+(-6* mx) + (-1) ],edx
        add     [ebx+(-5* mx) + (-4) ],dl
        add     [ebx+(-5* mx) + (-3) ],dx
        add     [ebx+(-5* mx) + (-1) ],edx
        add     [ebx+(-5* mx) + ( 3) ],dx
        add     [ebx+(-4* mx) + (-5) ],edx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],dx
        add     [ebx+(-4* mx) + ( 5) ],dl
        add     [ebx+(-3* mx) + (-5) ],edx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],dx
        add     [ebx+(-3* mx) + ( 5) ],dl
        add     [ebx+(-2* mx) + (-6) ],dl
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],edx
        add     [ebx+(-1* mx) + (-6) ],dl
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],edx
        add     [ebx+( 0* mx) + (-6) ],dl
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],edx
        add     [ebx+( 1* mx) + (-6) ],dl
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],edx
        add     [ebx+( 2* mx) + (-6) ],dl
        add     [ebx+( 2* mx) + (-5) ],edx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],edx
        add     [ebx+( 3* mx) + (-5) ],edx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],dx
        add     [ebx+( 3* mx) + ( 5) ],dl
        add     [ebx+( 4* mx) + (-5) ],edx
        add     [ebx+( 4* mx) + (-1) ],edx
        add     [ebx+( 4* mx) + ( 3) ],dx
        add     [ebx+( 4* mx) + ( 5) ],dl
        add     [ebx+( 5* mx) + (-4) ],dl
        add     [ebx+( 5* mx) + (-3) ],dx
        add     [ebx+( 5* mx) + (-1) ],edx
        add     [ebx+( 5* mx) + ( 3) ],dx
        add     [ebx+( 6* mx) + (-2) ],dl
        add     [ebx+( 6* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a214:
        add     [ebx+(-7* mx) + (-3) ],dx
        add     [ebx+(-7* mx) + (-1) ],edx
        add     [ebx+(-6* mx) + (-4) ],dl
        add     [ebx+(-6* mx) + (-3) ],dx
        add     [ebx+(-6* mx) + (-1) ],edx
        add     [ebx+(-6* mx) + ( 3) ],dl
        add     [ebx+(-5* mx) + (-5) ],edx
        add     [ebx+(-5* mx) + (-1) ],edx
        add     [ebx+(-5* mx) + ( 3) ],dx
        add     [ebx+(-4* mx) + (-6) ],dl
        add     [ebx+(-4* mx) + (-5) ],edx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],dx
        add     [ebx+(-4* mx) + ( 5) ],dl
        add     [ebx+(-3* mx) + (-7) ],dx
        add     [ebx+(-3* mx) + (-5) ],edx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],edx
        add     [ebx+(-2* mx) + (-7) ],dx
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],edx
        add     [ebx+(-1* mx) + (-7) ],dx
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],edx
        add     [ebx+( 0* mx) + (-7) ],dx
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],edx
        add     [ebx+( 1* mx) + (-7) ],dx
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],edx
        add     [ebx+( 2* mx) + (-7) ],dx
        add     [ebx+( 2* mx) + (-5) ],edx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],edx
        add     [ebx+( 3* mx) + (-6) ],dl
        add     [ebx+( 3* mx) + (-5) ],edx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],dx
        add     [ebx+( 3* mx) + ( 5) ],dl
        add     [ebx+( 4* mx) + (-5) ],edx
        add     [ebx+( 4* mx) + (-1) ],edx
        add     [ebx+( 4* mx) + ( 3) ],dx
        add     [ebx+( 5* mx) + (-4) ],dl
        add     [ebx+( 5* mx) + (-3) ],dx
        add     [ebx+( 5* mx) + (-1) ],edx
        add     [ebx+( 5* mx) + ( 3) ],dl
        add     [ebx+( 6* mx) + (-3) ],dx
        add     [ebx+( 6* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a215:
        add     [ebx+(-7* mx) + (-2) ],dl
        add     [ebx+(-7* mx) + (-1) ],edx
        add     [ebx+(-6* mx) + (-4) ],dl
        add     [ebx+(-6* mx) + (-3) ],dx
        add     [ebx+(-6* mx) + (-1) ],edx
        add     [ebx+(-6* mx) + ( 3) ],dx
        add     [ebx+(-5* mx) + (-5) ],edx
        add     [ebx+(-5* mx) + (-1) ],edx
        add     [ebx+(-5* mx) + ( 3) ],dx
        add     [ebx+(-5* mx) + ( 5) ],dl
        add     [ebx+(-4* mx) + (-6) ],dl
        add     [ebx+(-4* mx) + (-5) ],edx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],edx
        add     [ebx+(-3* mx) + (-6) ],dl
        add     [ebx+(-3* mx) + (-5) ],edx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],edx
        add     [ebx+(-2* mx) + (-7) ],dx
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],edx
        add     [ebx+(-2* mx) + ( 7) ],dl
        add     [ebx+(-1* mx) + (-7) ],dx
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],edx
        add     [ebx+(-1* mx) + ( 7) ],dl
        add     [ebx+( 0* mx) + (-7) ],dx
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],edx
        add     [ebx+( 0* mx) + ( 7) ],dl
        add     [ebx+( 1* mx) + (-7) ],dx
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],edx
        add     [ebx+( 1* mx) + ( 7) ],dl
        add     [ebx+( 2* mx) + (-7) ],dx
        add     [ebx+( 2* mx) + (-5) ],edx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],edx
        add     [ebx+( 2* mx) + ( 7) ],dl
        add     [ebx+( 3* mx) + (-6) ],dl
        add     [ebx+( 3* mx) + (-5) ],edx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],edx
        add     [ebx+( 4* mx) + (-6) ],dl
        add     [ebx+( 4* mx) + (-5) ],edx
        add     [ebx+( 4* mx) + (-1) ],edx
        add     [ebx+( 4* mx) + ( 3) ],edx
        add     [ebx+( 5* mx) + (-5) ],edx
        add     [ebx+( 5* mx) + (-1) ],edx
        add     [ebx+( 5* mx) + ( 3) ],dx
        add     [ebx+( 5* mx) + ( 5) ],dl
        add     [ebx+( 6* mx) + (-4) ],dl
        add     [ebx+( 6* mx) + (-3) ],dx
        add     [ebx+( 6* mx) + (-1) ],edx
        add     [ebx+( 6* mx) + ( 3) ],dx
        add     [ebx+( 7* mx) + (-2) ],dl
        add     [ebx+( 7* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a216:
        add     [ebx+(-8* mx) + (-3) ],dx
        add     [ebx+(-8* mx) + (-1) ],edx
        add     [ebx+(-7* mx) + (-5) ],edx
        add     [ebx+(-7* mx) + (-1) ],edx
        add     [ebx+(-7* mx) + ( 3) ],dx
        add     [ebx+(-6* mx) + (-6) ],dl
        add     [ebx+(-6* mx) + (-5) ],edx
        add     [ebx+(-6* mx) + (-1) ],edx
        add     [ebx+(-6* mx) + ( 3) ],dx
        add     [ebx+(-6* mx) + ( 5) ],dl
        add     [ebx+(-5* mx) + (-7) ],dx
        add     [ebx+(-5* mx) + (-5) ],edx
        add     [ebx+(-5* mx) + (-1) ],edx
        add     [ebx+(-5* mx) + ( 3) ],edx
        add     [ebx+(-4* mx) + (-7) ],dx
        add     [ebx+(-4* mx) + (-5) ],edx
        add     [ebx+(-4* mx) + (-1) ],edx
        add     [ebx+(-4* mx) + ( 3) ],edx
        add     [ebx+(-3* mx) + (-8) ],dl
        add     [ebx+(-3* mx) + (-7) ],dx
        add     [ebx+(-3* mx) + (-5) ],edx
        add     [ebx+(-3* mx) + (-1) ],edx
        add     [ebx+(-3* mx) + ( 3) ],edx
        add     [ebx+(-3* mx) + ( 7) ],dl
        add     [ebx+(-2* mx) + (-8) ],dl
        add     [ebx+(-2* mx) + (-7) ],dx
        add     [ebx+(-2* mx) + (-5) ],edx
        add     [ebx+(-2* mx) + (-1) ],edx
        add     [ebx+(-2* mx) + ( 3) ],edx
        add     [ebx+(-2* mx) + ( 7) ],dl
        add     [ebx+(-1* mx) + (-8) ],dl
        add     [ebx+(-1* mx) + (-7) ],dx
        add     [ebx+(-1* mx) + (-5) ],edx
        add     [ebx+(-1* mx) + (-1) ],edx
        add     [ebx+(-1* mx) + ( 3) ],edx
        add     [ebx+(-1* mx) + ( 7) ],dl
        add     [ebx+( 0* mx) + (-8) ],dl
        add     [ebx+( 0* mx) + (-7) ],dx
        add     [ebx+( 0* mx) + (-5) ],edx
        add     [ebx+( 0* mx) + (-1) ],edx
        add     [ebx+( 0* mx) + ( 3) ],edx
        add     [ebx+( 0* mx) + ( 7) ],dl
        add     [ebx+( 1* mx) + (-8) ],dl
        add     [ebx+( 1* mx) + (-7) ],dx
        add     [ebx+( 1* mx) + (-5) ],edx
        add     [ebx+( 1* mx) + (-1) ],edx
        add     [ebx+( 1* mx) + ( 3) ],edx
        add     [ebx+( 1* mx) + ( 7) ],dl
        add     [ebx+( 2* mx) + (-8) ],dl
        add     [ebx+( 2* mx) + (-7) ],dx
        add     [ebx+( 2* mx) + (-5) ],edx
        add     [ebx+( 2* mx) + (-1) ],edx
        add     [ebx+( 2* mx) + ( 3) ],edx
        add     [ebx+( 2* mx) + ( 7) ],dl
        add     [ebx+( 3* mx) + (-7) ],dx
        add     [ebx+( 3* mx) + (-5) ],edx
        add     [ebx+( 3* mx) + (-1) ],edx
        add     [ebx+( 3* mx) + ( 3) ],edx
        add     [ebx+( 4* mx) + (-7) ],dx
        add     [ebx+( 4* mx) + (-5) ],edx
        add     [ebx+( 4* mx) + (-1) ],edx
        add     [ebx+( 4* mx) + ( 3) ],edx
        add     [ebx+( 5* mx) + (-6) ],dl
        add     [ebx+( 5* mx) + (-5) ],edx
        add     [ebx+( 5* mx) + (-1) ],edx
        add     [ebx+( 5* mx) + ( 3) ],dx
        add     [ebx+( 5* mx) + ( 5) ],dl
        add     [ebx+( 6* mx) + (-5) ],edx
        add     [ebx+( 6* mx) + (-1) ],edx
        add     [ebx+( 6* mx) + ( 3) ],dx
        add     [ebx+( 7* mx) + (-3) ],dx
        add     [ebx+( 7* mx) + (-1) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a301:
        add     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a302:
        add     [ebx+(-1* mx) + (-1) ],dl
        add     [ebx+(-1* mx) + ( 0) ],dl
        add     [ebx+( 0* mx) + (-1) ],dl
        add     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a303:
        add     [ebx+(-1* mx) + (-1) ],dl
        add     [ebx+(-1* mx) + ( 0) ],dx
        add     [ebx+( 0* mx) + (-1) ],dl
        add     [ebx+( 0* mx) + ( 0) ],dx
        add     [ebx+( 1* mx) + (-1) ],dl
        add     [ebx+( 1* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a304:
        add     [ebx+(-2* mx) + (-1) ],dl
        add     [ebx+(-2* mx) + ( 0) ],dl
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + (-1) ],dl
        add     [ebx+( 1* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a305:
        add     [ebx+(-2* mx) + (-1) ],dl
        add     [ebx+(-2* mx) + ( 0) ],dx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],dl
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],dl
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],dl
        add     [ebx+( 2* mx) + (-1) ],dl
        add     [ebx+( 2* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a306:
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + (-3) ],dl
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],dl
        add     [ebx+(-1* mx) + (-3) ],dl
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],dl
        add     [ebx+( 0* mx) + (-3) ],dl
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],dl
        add     [ebx+( 1* mx) + (-3) ],dl
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],dl
        add     [ebx+( 2* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a307:
        add     [ebx+(-3* mx) + (-1) ],dl
        add     [ebx+(-3* mx) + ( 0) ],dx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],dl
        add     [ebx+(-1* mx) + (-3) ],dl
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],dx
        add     [ebx+( 0* mx) + (-3) ],dl
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],dx
        add     [ebx+( 1* mx) + (-3) ],dl
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],dx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],dl
        add     [ebx+( 3* mx) + (-1) ],dl
        add     [ebx+( 3* mx) + ( 0) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a308:
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + (-3) ],dl
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],dl
        add     [ebx+(-2* mx) + (-4) ],dx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],dx
        add     [ebx+(-1* mx) + (-4) ],dx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],dx
        add     [ebx+( 0* mx) + (-4) ],dx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],dx
        add     [ebx+( 1* mx) + (-4) ],dx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],dx
        add     [ebx+( 2* mx) + (-3) ],dl
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],dl
        add     [ebx+( 3* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a309:
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],dl
        add     [ebx+(-3* mx) + (-3) ],dl
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],dx
        add     [ebx+(-2* mx) + (-4) ],dx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],dx
        add     [ebx+(-2* mx) + ( 4) ],dl
        add     [ebx+(-1* mx) + (-4) ],dx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],dx
        add     [ebx+(-1* mx) + ( 4) ],dl
        add     [ebx+( 0* mx) + (-4) ],dx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],dx
        add     [ebx+( 0* mx) + ( 4) ],dl
        add     [ebx+( 1* mx) + (-4) ],dx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],dx
        add     [ebx+( 1* mx) + ( 4) ],dl
        add     [ebx+( 2* mx) + (-4) ],dx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],dx
        add     [ebx+( 2* mx) + ( 4) ],dl
        add     [ebx+( 3* mx) + (-3) ],dl
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],dx
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a310:
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + (-4) ],dx
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],dx
        add     [ebx+(-3* mx) + (-4) ],dx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],dx
        add     [ebx+(-2* mx) + (-5) ],dl
        add     [ebx+(-2* mx) + (-4) ],dx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],dx
        add     [ebx+(-2* mx) + ( 4) ],dl
        add     [ebx+(-1* mx) + (-5) ],dl
        add     [ebx+(-1* mx) + (-4) ],dx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],dx
        add     [ebx+(-1* mx) + ( 4) ],dl
        add     [ebx+( 0* mx) + (-5) ],dl
        add     [ebx+( 0* mx) + (-4) ],dx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],dx
        add     [ebx+( 0* mx) + ( 4) ],dl
        add     [ebx+( 1* mx) + (-5) ],dl
        add     [ebx+( 1* mx) + (-4) ],dx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],dx
        add     [ebx+( 1* mx) + ( 4) ],dl
        add     [ebx+( 2* mx) + (-4) ],dx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],dx
        add     [ebx+( 3* mx) + (-4) ],dx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],dx
        add     [ebx+( 4* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a311:
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + ( 2) ],dl
        add     [ebx+(-4* mx) + (-3) ],dl
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],dx
        add     [ebx+(-3* mx) + (-4) ],dx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],dx
        add     [ebx+(-3* mx) + ( 4) ],dl
        add     [ebx+(-2* mx) + (-5) ],dl
        add     [ebx+(-2* mx) + (-4) ],dx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],edx
        add     [ebx+(-1* mx) + (-5) ],dl
        add     [ebx+(-1* mx) + (-4) ],dx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],edx
        add     [ebx+( 0* mx) + (-5) ],dl
        add     [ebx+( 0* mx) + (-4) ],dx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],edx
        add     [ebx+( 1* mx) + (-5) ],dl
        add     [ebx+( 1* mx) + (-4) ],dx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],edx
        add     [ebx+( 2* mx) + (-5) ],dl
        add     [ebx+( 2* mx) + (-4) ],dx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],edx
        add     [ebx+( 3* mx) + (-4) ],dx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],dx
        add     [ebx+( 3* mx) + ( 4) ],dl
        add     [ebx+( 4* mx) + (-3) ],dl
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],dx
        add     [ebx+( 5* mx) + (-2) ],edx
        add     [ebx+( 5* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a312:
        add     [ebx+(-6* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + (-4) ],dx
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + ( 2) ],dx
        add     [ebx+(-4* mx) + (-5) ],dl
        add     [ebx+(-4* mx) + (-4) ],dx
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],dx
        add     [ebx+(-4* mx) + ( 4) ],dl
        add     [ebx+(-3* mx) + (-5) ],dl
        add     [ebx+(-3* mx) + (-4) ],dx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],dx
        add     [ebx+(-3* mx) + ( 4) ],dl
        add     [ebx+(-2* mx) + (-6) ],edx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],edx
        add     [ebx+(-1* mx) + (-6) ],edx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],edx
        add     [ebx+( 0* mx) + (-6) ],edx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],edx
        add     [ebx+( 1* mx) + (-6) ],edx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],edx
        add     [ebx+( 2* mx) + (-5) ],dl
        add     [ebx+( 2* mx) + (-4) ],dx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],dx
        add     [ebx+( 2* mx) + ( 4) ],dl
        add     [ebx+( 3* mx) + (-5) ],dl
        add     [ebx+( 3* mx) + (-4) ],dx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],dx
        add     [ebx+( 3* mx) + ( 4) ],dl
        add     [ebx+( 4* mx) + (-4) ],dx
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],dx
        add     [ebx+( 5* mx) + (-2) ],edx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a313:
        add     [ebx+(-6* mx) + (-2) ],edx
        add     [ebx+(-6* mx) + ( 2) ],dl
        add     [ebx+(-5* mx) + (-4) ],dx
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + ( 2) ],dx
        add     [ebx+(-5* mx) + ( 4) ],dl
        add     [ebx+(-4* mx) + (-5) ],dl
        add     [ebx+(-4* mx) + (-4) ],dx
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],edx
        add     [ebx+(-3* mx) + (-5) ],dl
        add     [ebx+(-3* mx) + (-4) ],dx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],edx
        add     [ebx+(-2* mx) + (-6) ],edx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],edx
        add     [ebx+(-2* mx) + ( 6) ],dl
        add     [ebx+(-1* mx) + (-6) ],edx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],edx
        add     [ebx+(-1* mx) + ( 6) ],dl
        add     [ebx+( 0* mx) + (-6) ],edx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],edx
        add     [ebx+( 0* mx) + ( 6) ],dl
        add     [ebx+( 1* mx) + (-6) ],edx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],edx
        add     [ebx+( 1* mx) + ( 6) ],dl
        add     [ebx+( 2* mx) + (-6) ],edx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],edx
        add     [ebx+( 2* mx) + ( 6) ],dl
        add     [ebx+( 3* mx) + (-5) ],dl
        add     [ebx+( 3* mx) + (-4) ],dx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],edx
        add     [ebx+( 4* mx) + (-5) ],dl
        add     [ebx+( 4* mx) + (-4) ],dx
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],edx
        add     [ebx+( 5* mx) + (-4) ],dx
        add     [ebx+( 5* mx) + (-2) ],edx
        add     [ebx+( 5* mx) + ( 2) ],dx
        add     [ebx+( 5* mx) + ( 4) ],dl
        add     [ebx+( 6* mx) + (-2) ],edx
        add     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a314:
        add     [ebx+(-7* mx) + (-3) ],dl
        add     [ebx+(-7* mx) + (-2) ],edx
        add     [ebx+(-7* mx) + ( 2) ],dl
        add     [ebx+(-6* mx) + (-4) ],dx
        add     [ebx+(-6* mx) + (-2) ],edx
        add     [ebx+(-6* mx) + ( 2) ],dx
        add     [ebx+(-5* mx) + (-5) ],dl
        add     [ebx+(-5* mx) + (-4) ],dx
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + ( 2) ],dx
        add     [ebx+(-5* mx) + ( 4) ],dl
        add     [ebx+(-4* mx) + (-6) ],edx
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],edx
        add     [ebx+(-3* mx) + (-7) ],dl
        add     [ebx+(-3* mx) + (-6) ],edx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],edx
        add     [ebx+(-3* mx) + ( 6) ],dl
        add     [ebx+(-2* mx) + (-7) ],dl
        add     [ebx+(-2* mx) + (-6) ],edx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],edx
        add     [ebx+(-2* mx) + ( 6) ],dl
        add     [ebx+(-1* mx) + (-7) ],dl
        add     [ebx+(-1* mx) + (-6) ],edx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],edx
        add     [ebx+(-1* mx) + ( 6) ],dl
        add     [ebx+( 0* mx) + (-7) ],dl
        add     [ebx+( 0* mx) + (-6) ],edx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],edx
        add     [ebx+( 0* mx) + ( 6) ],dl
        add     [ebx+( 1* mx) + (-7) ],dl
        add     [ebx+( 1* mx) + (-6) ],edx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],edx
        add     [ebx+( 1* mx) + ( 6) ],dl
        add     [ebx+( 2* mx) + (-7) ],dl
        add     [ebx+( 2* mx) + (-6) ],edx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],edx
        add     [ebx+( 2* mx) + ( 6) ],dl
        add     [ebx+( 3* mx) + (-6) ],edx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],edx
        add     [ebx+( 4* mx) + (-5) ],dl
        add     [ebx+( 4* mx) + (-4) ],dx
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],dx
        add     [ebx+( 4* mx) + ( 4) ],dl
        add     [ebx+( 5* mx) + (-4) ],dx
        add     [ebx+( 5* mx) + (-2) ],edx
        add     [ebx+( 5* mx) + ( 2) ],dx
        add     [ebx+( 6* mx) + (-3) ],dl
        add     [ebx+( 6* mx) + (-2) ],edx
        add     [ebx+( 6* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a315:
        add     [ebx+(-7* mx) + (-2) ],edx
        add     [ebx+(-7* mx) + ( 2) ],dl
        add     [ebx+(-6* mx) + (-4) ],dx
        add     [ebx+(-6* mx) + (-2) ],edx
        add     [ebx+(-6* mx) + ( 2) ],dx
        add     [ebx+(-6* mx) + ( 4) ],dl
        add     [ebx+(-5* mx) + (-5) ],dl
        add     [ebx+(-5* mx) + (-4) ],dx
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + ( 2) ],edx
        add     [ebx+(-4* mx) + (-6) ],edx
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],edx
        add     [ebx+(-4* mx) + ( 6) ],dl
        add     [ebx+(-3* mx) + (-6) ],edx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],edx
        add     [ebx+(-3* mx) + ( 6) ],dl
        add     [ebx+(-2* mx) + (-7) ],dl
        add     [ebx+(-2* mx) + (-6) ],edx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],edx
        add     [ebx+(-2* mx) + ( 6) ],dx
        add     [ebx+(-1* mx) + (-7) ],dl
        add     [ebx+(-1* mx) + (-6) ],edx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],edx
        add     [ebx+(-1* mx) + ( 6) ],dx
        add     [ebx+( 0* mx) + (-7) ],dl
        add     [ebx+( 0* mx) + (-6) ],edx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],edx
        add     [ebx+( 0* mx) + ( 6) ],dx
        add     [ebx+( 1* mx) + (-7) ],dl
        add     [ebx+( 1* mx) + (-6) ],edx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],edx
        add     [ebx+( 1* mx) + ( 6) ],dx
        add     [ebx+( 2* mx) + (-7) ],dl
        add     [ebx+( 2* mx) + (-6) ],edx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],edx
        add     [ebx+( 2* mx) + ( 6) ],dx
        add     [ebx+( 3* mx) + (-6) ],edx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],edx
        add     [ebx+( 3* mx) + ( 6) ],dl
        add     [ebx+( 4* mx) + (-6) ],edx
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],edx
        add     [ebx+( 4* mx) + ( 6) ],dl
        add     [ebx+( 5* mx) + (-5) ],dl
        add     [ebx+( 5* mx) + (-4) ],dx
        add     [ebx+( 5* mx) + (-2) ],edx
        add     [ebx+( 5* mx) + ( 2) ],edx
        add     [ebx+( 6* mx) + (-4) ],dx
        add     [ebx+( 6* mx) + (-2) ],edx
        add     [ebx+( 6* mx) + ( 2) ],dx
        add     [ebx+( 6* mx) + ( 4) ],dl
        add     [ebx+( 7* mx) + (-2) ],edx
        add     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a316:
        add     [ebx+(-8* mx) + (-3) ],dl
        add     [ebx+(-8* mx) + (-2) ],edx
        add     [ebx+(-8* mx) + ( 2) ],dl
        add     [ebx+(-7* mx) + (-5) ],dl
        add     [ebx+(-7* mx) + (-4) ],dx
        add     [ebx+(-7* mx) + (-2) ],edx
        add     [ebx+(-7* mx) + ( 2) ],dx
        add     [ebx+(-7* mx) + ( 4) ],dl
        add     [ebx+(-6* mx) + (-6) ],edx
        add     [ebx+(-6* mx) + (-2) ],edx
        add     [ebx+(-6* mx) + ( 2) ],edx
        add     [ebx+(-5* mx) + (-7) ],dl
        add     [ebx+(-5* mx) + (-6) ],edx
        add     [ebx+(-5* mx) + (-2) ],edx
        add     [ebx+(-5* mx) + ( 2) ],edx
        add     [ebx+(-5* mx) + ( 6) ],dl
        add     [ebx+(-4* mx) + (-7) ],dl
        add     [ebx+(-4* mx) + (-6) ],edx
        add     [ebx+(-4* mx) + (-2) ],edx
        add     [ebx+(-4* mx) + ( 2) ],edx
        add     [ebx+(-4* mx) + ( 6) ],dl
        add     [ebx+(-3* mx) + (-8) ],dx
        add     [ebx+(-3* mx) + (-6) ],edx
        add     [ebx+(-3* mx) + (-2) ],edx
        add     [ebx+(-3* mx) + ( 2) ],edx
        add     [ebx+(-3* mx) + ( 6) ],dx
        add     [ebx+(-2* mx) + (-8) ],dx
        add     [ebx+(-2* mx) + (-6) ],edx
        add     [ebx+(-2* mx) + (-2) ],edx
        add     [ebx+(-2* mx) + ( 2) ],edx
        add     [ebx+(-2* mx) + ( 6) ],dx
        add     [ebx+(-1* mx) + (-8) ],dx
        add     [ebx+(-1* mx) + (-6) ],edx
        add     [ebx+(-1* mx) + (-2) ],edx
        add     [ebx+(-1* mx) + ( 2) ],edx
        add     [ebx+(-1* mx) + ( 6) ],dx
        add     [ebx+( 0* mx) + (-8) ],dx
        add     [ebx+( 0* mx) + (-6) ],edx
        add     [ebx+( 0* mx) + (-2) ],edx
        add     [ebx+( 0* mx) + ( 2) ],edx
        add     [ebx+( 0* mx) + ( 6) ],dx
        add     [ebx+( 1* mx) + (-8) ],dx
        add     [ebx+( 1* mx) + (-6) ],edx
        add     [ebx+( 1* mx) + (-2) ],edx
        add     [ebx+( 1* mx) + ( 2) ],edx
        add     [ebx+( 1* mx) + ( 6) ],dx
        add     [ebx+( 2* mx) + (-8) ],dx
        add     [ebx+( 2* mx) + (-6) ],edx
        add     [ebx+( 2* mx) + (-2) ],edx
        add     [ebx+( 2* mx) + ( 2) ],edx
        add     [ebx+( 2* mx) + ( 6) ],dx
        add     [ebx+( 3* mx) + (-7) ],dl
        add     [ebx+( 3* mx) + (-6) ],edx
        add     [ebx+( 3* mx) + (-2) ],edx
        add     [ebx+( 3* mx) + ( 2) ],edx
        add     [ebx+( 3* mx) + ( 6) ],dl
        add     [ebx+( 4* mx) + (-7) ],dl
        add     [ebx+( 4* mx) + (-6) ],edx
        add     [ebx+( 4* mx) + (-2) ],edx
        add     [ebx+( 4* mx) + ( 2) ],edx
        add     [ebx+( 4* mx) + ( 6) ],dl
        add     [ebx+( 5* mx) + (-6) ],edx
        add     [ebx+( 5* mx) + (-2) ],edx
        add     [ebx+( 5* mx) + ( 2) ],edx
        add     [ebx+( 6* mx) + (-5) ],dl
        add     [ebx+( 6* mx) + (-4) ],dx
        add     [ebx+( 6* mx) + (-2) ],edx
        add     [ebx+( 6* mx) + ( 2) ],dx
        add     [ebx+( 6* mx) + ( 4) ],dl
        add     [ebx+( 7* mx) + (-3) ],dl
        add     [ebx+( 7* mx) + (-2) ],edx
        add     [ebx+( 7* mx) + ( 2) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a401:
        add     [ebx+( 0* mx) + ( 0) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a402:
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a403:
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + ( 1) ],dl
        add     [ebx+( 0* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + ( 1) ],dl
        add     [ebx+( 1* mx) + (-1) ],dx
        add     [ebx+( 1* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a404:
        add     [ebx+(-2* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + (-2) ],dl
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + ( 1) ],dl
        add     [ebx+( 0* mx) + (-2) ],dl
        add     [ebx+( 0* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + ( 1) ],dl
        add     [ebx+( 1* mx) + (-1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a405:
        add     [ebx+(-2* mx) + (-1) ],dx
        add     [ebx+(-2* mx) + ( 1) ],dl
        add     [ebx+(-1* mx) + (-2) ],dl
        add     [ebx+(-1* mx) + (-1) ],dx
        add     [ebx+(-1* mx) + ( 1) ],dx
        add     [ebx+( 0* mx) + (-2) ],dl
        add     [ebx+( 0* mx) + (-1) ],dx
        add     [ebx+( 0* mx) + ( 1) ],dx
        add     [ebx+( 1* mx) + (-2) ],dl
        add     [ebx+( 1* mx) + (-1) ],dx
        add     [ebx+( 1* mx) + ( 1) ],dx
        add     [ebx+( 2* mx) + (-1) ],dx
        add     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a406:
        add     [ebx+(-3* mx) + (-2) ],dl
        add     [ebx+(-3* mx) + (-1) ],dx
        add     [ebx+(-3* mx) + ( 1) ],dl
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],dx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],dx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],dx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],dx
        add     [ebx+( 2* mx) + (-2) ],dl
        add     [ebx+( 2* mx) + (-1) ],dx
        add     [ebx+( 2* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a407:
        add     [ebx+(-3* mx) + (-1) ],dx
        add     [ebx+(-3* mx) + ( 1) ],dl
        add     [ebx+(-2* mx) + (-2) ],dl
        add     [ebx+(-2* mx) + (-1) ],dx
        add     [ebx+(-2* mx) + ( 1) ],dx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],dx
        add     [ebx+(-1* mx) + ( 3) ],dl
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],dx
        add     [ebx+( 0* mx) + ( 3) ],dl
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],dx
        add     [ebx+( 1* mx) + ( 3) ],dl
        add     [ebx+( 2* mx) + (-2) ],dl
        add     [ebx+( 2* mx) + (-1) ],dx
        add     [ebx+( 2* mx) + ( 1) ],dx
        add     [ebx+( 3* mx) + (-1) ],dx
        add     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a408:
        add     [ebx+(-4* mx) + (-2) ],dl
        add     [ebx+(-4* mx) + (-1) ],dx
        add     [ebx+(-4* mx) + ( 1) ],dl
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],dx
        add     [ebx+(-2* mx) + (-4) ],dl
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],dx
        add     [ebx+(-2* mx) + ( 3) ],dl
        add     [ebx+(-1* mx) + (-4) ],dl
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],dx
        add     [ebx+(-1* mx) + ( 3) ],dl
        add     [ebx+( 0* mx) + (-4) ],dl
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],dx
        add     [ebx+( 0* mx) + ( 3) ],dl
        add     [ebx+( 1* mx) + (-4) ],dl
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],dx
        add     [ebx+( 1* mx) + ( 3) ],dl
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],dx
        add     [ebx+( 3* mx) + (-2) ],dl
        add     [ebx+( 3* mx) + (-1) ],dx
        add     [ebx+( 3* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a409:
        add     [ebx+(-4* mx) + (-2) ],dl
        add     [ebx+(-4* mx) + (-1) ],dx
        add     [ebx+(-4* mx) + ( 1) ],dx
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],dx
        add     [ebx+(-3* mx) + ( 3) ],dl
        add     [ebx+(-2* mx) + (-4) ],dl
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + (-4) ],dl
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + (-4) ],dl
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + (-4) ],dl
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + (-4) ],dl
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],dx
        add     [ebx+( 3* mx) + ( 3) ],dl
        add     [ebx+( 4* mx) + (-2) ],dl
        add     [ebx+( 4* mx) + (-1) ],dx
        add     [ebx+( 4* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a410:
        add     [ebx+(-5* mx) + (-2) ],dl
        add     [ebx+(-5* mx) + (-1) ],dx
        add     [ebx+(-5* mx) + ( 1) ],dl
        add     [ebx+(-4* mx) + (-4) ],dl
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],dx
        add     [ebx+(-4* mx) + ( 3) ],dl
        add     [ebx+(-3* mx) + (-4) ],dl
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],dx
        add     [ebx+(-3* mx) + ( 3) ],dl
        add     [ebx+(-2* mx) + (-5) ],dx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + (-5) ],dx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + (-5) ],dx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + (-5) ],dx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + (-4) ],dl
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],dx
        add     [ebx+( 2* mx) + ( 3) ],dl
        add     [ebx+( 3* mx) + (-4) ],dl
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],dx
        add     [ebx+( 3* mx) + ( 3) ],dl
        add     [ebx+( 4* mx) + (-2) ],dl
        add     [ebx+( 4* mx) + (-1) ],dx
        add     [ebx+( 4* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a411:
        add     [ebx+(-5* mx) + (-2) ],dl
        add     [ebx+(-5* mx) + (-1) ],dx
        add     [ebx+(-5* mx) + ( 1) ],dx
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],dx
        add     [ebx+(-4* mx) + ( 3) ],dl
        add     [ebx+(-3* mx) + (-4) ],dl
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + (-5) ],dx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + ( 5) ],dl
        add     [ebx+(-1* mx) + (-5) ],dx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + ( 5) ],dl
        add     [ebx+( 0* mx) + (-5) ],dx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + ( 5) ],dl
        add     [ebx+( 1* mx) + (-5) ],dx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + ( 5) ],dl
        add     [ebx+( 2* mx) + (-5) ],dx
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + ( 5) ],dl
        add     [ebx+( 3* mx) + (-4) ],dl
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],edx
        add     [ebx+( 4* mx) + (-3) ],edx
        add     [ebx+( 4* mx) + ( 1) ],dx
        add     [ebx+( 4* mx) + ( 3) ],dl
        add     [ebx+( 5* mx) + (-2) ],dl
        add     [ebx+( 5* mx) + (-1) ],dx
        add     [ebx+( 5* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a412:
        add     [ebx+(-6* mx) + (-2) ],dl
        add     [ebx+(-6* mx) + (-1) ],dx
        add     [ebx+(-6* mx) + ( 1) ],dl
        add     [ebx+(-5* mx) + (-4) ],dl
        add     [ebx+(-5* mx) + (-3) ],edx
        add     [ebx+(-5* mx) + ( 1) ],dx
        add     [ebx+(-5* mx) + ( 3) ],dl
        add     [ebx+(-4* mx) + (-5) ],dx
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],edx
        add     [ebx+(-3* mx) + (-5) ],dx
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + (-6) ],dl
        add     [ebx+(-2* mx) + (-5) ],dx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + ( 5) ],dl
        add     [ebx+(-1* mx) + (-6) ],dl
        add     [ebx+(-1* mx) + (-5) ],dx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + ( 5) ],dl
        add     [ebx+( 0* mx) + (-6) ],dl
        add     [ebx+( 0* mx) + (-5) ],dx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + ( 5) ],dl
        add     [ebx+( 1* mx) + (-6) ],dl
        add     [ebx+( 1* mx) + (-5) ],dx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + ( 5) ],dl
        add     [ebx+( 2* mx) + (-5) ],dx
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 3* mx) + (-5) ],dx
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],edx
        add     [ebx+( 4* mx) + (-4) ],dl
        add     [ebx+( 4* mx) + (-3) ],edx
        add     [ebx+( 4* mx) + ( 1) ],dx
        add     [ebx+( 4* mx) + ( 3) ],dl
        add     [ebx+( 5* mx) + (-2) ],dl
        add     [ebx+( 5* mx) + (-1) ],dx
        add     [ebx+( 5* mx) + ( 1) ],dl
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a413:
        add     [ebx+(-6* mx) + (-2) ],dl
        add     [ebx+(-6* mx) + (-1) ],dx
        add     [ebx+(-6* mx) + ( 1) ],dx
        add     [ebx+(-5* mx) + (-4) ],dl
        add     [ebx+(-5* mx) + (-3) ],edx
        add     [ebx+(-5* mx) + ( 1) ],edx
        add     [ebx+(-4* mx) + (-5) ],dx
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],edx
        add     [ebx+(-4* mx) + ( 5) ],dl
        add     [ebx+(-3* mx) + (-5) ],dx
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],edx
        add     [ebx+(-3* mx) + ( 5) ],dl
        add     [ebx+(-2* mx) + (-6) ],dl
        add     [ebx+(-2* mx) + (-5) ],dx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + ( 5) ],dx
        add     [ebx+(-1* mx) + (-6) ],dl
        add     [ebx+(-1* mx) + (-5) ],dx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + ( 5) ],dx
        add     [ebx+( 0* mx) + (-6) ],dl
        add     [ebx+( 0* mx) + (-5) ],dx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + ( 5) ],dx
        add     [ebx+( 1* mx) + (-6) ],dl
        add     [ebx+( 1* mx) + (-5) ],dx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + ( 5) ],dx
        add     [ebx+( 2* mx) + (-6) ],dl
        add     [ebx+( 2* mx) + (-5) ],dx
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + ( 5) ],dx
        add     [ebx+( 3* mx) + (-5) ],dx
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],edx
        add     [ebx+( 3* mx) + ( 5) ],dl
        add     [ebx+( 4* mx) + (-5) ],dx
        add     [ebx+( 4* mx) + (-3) ],edx
        add     [ebx+( 4* mx) + ( 1) ],edx
        add     [ebx+( 4* mx) + ( 5) ],dl
        add     [ebx+( 5* mx) + (-4) ],dl
        add     [ebx+( 5* mx) + (-3) ],edx
        add     [ebx+( 5* mx) + ( 1) ],edx
        add     [ebx+( 6* mx) + (-2) ],dl
        add     [ebx+( 6* mx) + (-1) ],dx
        add     [ebx+( 6* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
a414:
        add     [ebx+(-7* mx) + (-3) ],edx
        add     [ebx+(-7* mx) + ( 1) ],dx
        add     [ebx+(-6* mx) + (-4) ],dl
        add     [ebx+(-6* mx) + (-3) ],edx
        add     [ebx+(-6* mx) + ( 1) ],dx
        add     [ebx+(-6* mx) + ( 3) ],dl
        add     [ebx+(-5* mx) + (-5) ],dx
        add     [ebx+(-5* mx) + (-3) ],edx
        add     [ebx+(-5* mx) + ( 1) ],edx
        add     [ebx+(-4* mx) + (-6) ],dl
        add     [ebx+(-4* mx) + (-5) ],dx
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],edx
        add     [ebx+(-4* mx) + ( 5) ],dl
        add     [ebx+(-3* mx) + (-7) ],edx
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],edx
        add     [ebx+(-3* mx) + ( 5) ],dx
        add     [ebx+(-2* mx) + (-7) ],edx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + ( 5) ],dx
        add     [ebx+(-1* mx) + (-7) ],edx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + ( 5) ],dx
        add     [ebx+( 0* mx) + (-7) ],edx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + ( 5) ],dx
        add     [ebx+( 1* mx) + (-7) ],edx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + ( 5) ],dx
        add     [ebx+( 2* mx) + (-7) ],edx
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + ( 5) ],dx
        add     [ebx+( 3* mx) + (-6) ],dl
        add     [ebx+( 3* mx) + (-5) ],dx
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],edx
        add     [ebx+( 3* mx) + ( 5) ],dl
        add     [ebx+( 4* mx) + (-5) ],dx
        add     [ebx+( 4* mx) + (-3) ],edx
        add     [ebx+( 4* mx) + ( 1) ],edx
        add     [ebx+( 5* mx) + (-4) ],dl
        add     [ebx+( 5* mx) + (-3) ],edx
        add     [ebx+( 5* mx) + ( 1) ],dx
        add     [ebx+( 5* mx) + ( 3) ],dl
        add     [ebx+( 6* mx) + (-3) ],edx
        add     [ebx+( 6* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
	a415:
        add     [ebx+(-7* mx) + (-2) ],dl
        add     [ebx+(-7* mx) + (-1) ],dx
        add     [ebx+(-7* mx) + ( 1) ],dx
        add     [ebx+(-6* mx) + (-4) ],dl
        add     [ebx+(-6* mx) + (-3) ],edx
        add     [ebx+(-6* mx) + ( 1) ],edx
        add     [ebx+(-5* mx) + (-5) ],dx
        add     [ebx+(-5* mx) + (-3) ],edx
        add     [ebx+(-5* mx) + ( 1) ],edx
        add     [ebx+(-5* mx) + ( 5) ],dl
        add     [ebx+(-4* mx) + (-6) ],dl
        add     [ebx+(-4* mx) + (-5) ],dx
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],edx
        add     [ebx+(-4* mx) + ( 5) ],dx
        add     [ebx+(-3* mx) + (-6) ],dl
        add     [ebx+(-3* mx) + (-5) ],dx
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],edx
        add     [ebx+(-3* mx) + ( 5) ],dx
        add     [ebx+(-2* mx) + (-7) ],edx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + ( 5) ],dx
        add     [ebx+(-2* mx) + ( 7) ],dl
        add     [ebx+(-1* mx) + (-7) ],edx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + ( 5) ],dx
        add     [ebx+(-1* mx) + ( 7) ],dl
        add     [ebx+( 0* mx) + (-7) ],edx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + ( 5) ],dx
        add     [ebx+( 0* mx) + ( 7) ],dl
        add     [ebx+( 1* mx) + (-7) ],edx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + ( 5) ],dx
        add     [ebx+( 1* mx) + ( 7) ],dl
        add     [ebx+( 2* mx) + (-7) ],edx
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + ( 5) ],dx
        add     [ebx+( 2* mx) + ( 7) ],dl
        add     [ebx+( 3* mx) + (-6) ],dl
        add     [ebx+( 3* mx) + (-5) ],dx
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],edx
        add     [ebx+( 3* mx) + ( 5) ],dx
        add     [ebx+( 4* mx) + (-6) ],dl
        add     [ebx+( 4* mx) + (-5) ],dx
        add     [ebx+( 4* mx) + (-3) ],edx
        add     [ebx+( 4* mx) + ( 1) ],edx
        add     [ebx+( 4* mx) + ( 5) ],dx
        add     [ebx+( 5* mx) + (-5) ],dx
        add     [ebx+( 5* mx) + (-3) ],edx
        add     [ebx+( 5* mx) + ( 1) ],edx
        add     [ebx+( 5* mx) + ( 5) ],dl
        add     [ebx+( 6* mx) + (-4) ],dl
        add     [ebx+( 6* mx) + (-3) ],edx
        add     [ebx+( 6* mx) + ( 1) ],edx
        add     [ebx+( 7* mx) + (-2) ],dl
        add     [ebx+( 7* mx) + (-1) ],dx
        add     [ebx+( 7* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
        jmp     fin
	a416:
        add     [ebx+(-8* mx) + (-3) ],edx
        add     [ebx+(-8* mx) + ( 1) ],dx
        add     [ebx+(-7* mx) + (-5) ],dx
        add     [ebx+(-7* mx) + (-3) ],edx
        add     [ebx+(-7* mx) + ( 1) ],edx
        add     [ebx+(-6* mx) + (-6) ],dl
        add     [ebx+(-6* mx) + (-5) ],dx
        add     [ebx+(-6* mx) + (-3) ],edx
        add     [ebx+(-6* mx) + ( 1) ],edx
        add     [ebx+(-6* mx) + ( 5) ],dl
        add     [ebx+(-5* mx) + (-7) ],edx
        add     [ebx+(-5* mx) + (-3) ],edx
        add     [ebx+(-5* mx) + ( 1) ],edx
        add     [ebx+(-5* mx) + ( 5) ],dx
        add     [ebx+(-4* mx) + (-7) ],edx
        add     [ebx+(-4* mx) + (-3) ],edx
        add     [ebx+(-4* mx) + ( 1) ],edx
        add     [ebx+(-4* mx) + ( 5) ],dx
        add     [ebx+(-3* mx) + (-8) ],dl
        add     [ebx+(-3* mx) + (-7) ],edx
        add     [ebx+(-3* mx) + (-3) ],edx
        add     [ebx+(-3* mx) + ( 1) ],edx
        add     [ebx+(-3* mx) + ( 5) ],dx
        add     [ebx+(-3* mx) + ( 7) ],dl
        add     [ebx+(-2* mx) + (-8) ],dl
        add     [ebx+(-2* mx) + (-7) ],edx
        add     [ebx+(-2* mx) + (-3) ],edx
        add     [ebx+(-2* mx) + ( 1) ],edx
        add     [ebx+(-2* mx) + ( 5) ],dx
        add     [ebx+(-2* mx) + ( 7) ],dl
        add     [ebx+(-1* mx) + (-8) ],dl
        add     [ebx+(-1* mx) + (-7) ],edx
        add     [ebx+(-1* mx) + (-3) ],edx
        add     [ebx+(-1* mx) + ( 1) ],edx
        add     [ebx+(-1* mx) + ( 5) ],dx
        add     [ebx+(-1* mx) + ( 7) ],dl
        add     [ebx+( 0* mx) + (-8) ],dl
        add     [ebx+( 0* mx) + (-7) ],edx
        add     [ebx+( 0* mx) + (-3) ],edx
        add     [ebx+( 0* mx) + ( 1) ],edx
        add     [ebx+( 0* mx) + ( 5) ],dx
        add     [ebx+( 0* mx) + ( 7) ],dl
        add     [ebx+( 1* mx) + (-8) ],dl
        add     [ebx+( 1* mx) + (-7) ],edx
        add     [ebx+( 1* mx) + (-3) ],edx
        add     [ebx+( 1* mx) + ( 1) ],edx
        add     [ebx+( 1* mx) + ( 5) ],dx
        add     [ebx+( 1* mx) + ( 7) ],dl
        add     [ebx+( 2* mx) + (-8) ],dl
        add     [ebx+( 2* mx) + (-7) ],edx
        add     [ebx+( 2* mx) + (-3) ],edx
        add     [ebx+( 2* mx) + ( 1) ],edx
        add     [ebx+( 2* mx) + ( 5) ],dx
        add     [ebx+( 2* mx) + ( 7) ],dl
        add     [ebx+( 3* mx) + (-7) ],edx
        add     [ebx+( 3* mx) + (-3) ],edx
        add     [ebx+( 3* mx) + ( 1) ],edx
        add     [ebx+( 3* mx) + ( 5) ],dx
        add     [ebx+( 4* mx) + (-7) ],edx
        add     [ebx+( 4* mx) + (-3) ],edx
        add     [ebx+( 4* mx) + ( 1) ],edx
        add     [ebx+( 4* mx) + ( 5) ],dx
        add     [ebx+( 5* mx) + (-6) ],dl
        add     [ebx+( 5* mx) + (-5) ],dx
        add     [ebx+( 5* mx) + (-3) ],edx
        add     [ebx+( 5* mx) + ( 1) ],edx
        add     [ebx+( 5* mx) + ( 5) ],dl
        add     [ebx+( 6* mx) + (-5) ],dx
        add     [ebx+( 6* mx) + (-3) ],edx
        add     [ebx+( 6* mx) + ( 1) ],edx
        add     [ebx+( 7* mx) + (-3) ],edx
        add     [ebx+( 7* mx) + ( 1) ],dx
        mov     esi,[esi]
        dec     ecx
        jnz     bouc5
	fin:
		pop		ebp
	}
}

void transfert(byte *dest , byte *src)
{
	__asm
	{
        mov     edi,dest
        mov     esi,src
        add     esi,mx*20+32
        mov     edx,200
	trboucy:
        mov     ecx,320/4
        rep     movsd
        add     esi,64
        dec     edx
        jnz     trboucy
	}
}

void transfert_shade(byte *dest, byte *src)
{
	__asm
	{
        mov     edi,dest
        mov     esi,src
        add     esi,mx*20+32
        mov     edx,200
	strboucy:
        mov     ecx,320/4
	strboucx:
        mov     eax,[esi]
        add     esi,4
        add     [edi],eax
        add     edi,4
        dec     ecx
        jnz     strboucx

        add     esi,64
        dec     edx
        jnz     strboucy
	}
}

#pragma optimize("",off)
#pragma code_seg("automod")
void plasma(byte *dest, byte *src, byte *table)
{
	__asm
	{
		mov			ebx,table
        mov         esi,src
        mov         edi,dest
		push		ebp
        lea         edx,plasma_cg_table
        mov         [edx-4],ebx
        add         edi,320+1
        add         esi,320+1
        
        mov         ebp,200-2

	plasma_boucy:

        mov         ecx,320-2
        xor         eax,eax
        xor         ebx,ebx

	plasma_boucx:
        movzx       ebx,byte ptr[esi-321]
        mov         al,[esi-320]
        add         ebx,eax
        mov         al,[esi-319]
        add         ebx,eax
        mov         al,[esi-1]
        add         ebx,eax
        mov         al,[esi+1]
        add         ebx,eax
        mov         al,[esi+319]
        add         ebx,eax
        mov         al,[esi+320]
        add         ebx,eax
        mov         al,[esi+321]
        inc         esi
        add         ebx,eax
        shr         ebx,3
        mov         al,[ebx+012345678h]
	plasma_cg_table:
        mov         [edi],al
        inc         edi

        dec         ecx
        jnz         plasma_boucx

        dec         ebp
        jnz         plasma_boucy

		pop			ebp
	}
}
#pragma code_seg()
#pragma optimize("",on)

void soustract(byte *dest, byte *src)
{
	__asm
	{
        mov     esi,src
        mov     edi,dest
        mov     ecx,320*200/4

	soustract_boucle:
        mov     eax,[esi]
        add     esi,4
        sub     [edi],eax
        add     edi,4
        dec     ecx
        jnz     soustract_boucle
	}
}

void addition(byte *dest, byte *src)
{
	__asm
	{
        mov     esi,src
        mov     edi,dest
        mov     ecx,320*200/4

	addition_boucle:
        mov     eax,[esi]
        add     esi,4
        add     [edi],eax
        add     edi,4
        dec     ecx
        jnz     addition_boucle
	}
}

#pragma optimize("",off)
#pragma code_seg("automod")
void		myzoom			(byte *dest, byte *src)
{
	__asm
	{
		mov		edi,dest
		mov		esi,src
		lea		ebx,tabzoom[0]
        lea     edx,cg_imsrc
        mov     [edx-4],esi
		push	ebp
        mov     ebp,64000
    myzoom_bc:
        movzx   eax,byte ptr[esi]
        inc     esi
        movzx   edx,word ptr[ebx]
        add     ebx,2
        movzx   edx,byte ptr[edx+012345678h]
    cg_imsrc:
        add     eax,edx
        shr     eax,1
        mov     [edi],al
        inc     edi
        dec     ebp
        jnz     myzoom_bc
		pop		ebp
	}
}
#pragma code_seg()
#pragma optimize("",on)

static void calc_tabzoom()
{
	word *p=tabzoom;
	int x,y;
	for(y=0; y<200; y++)
		for(x=0; x<320; x++)
			*(p++)=(y-((y-100)>>3))*320+(x-((x-160)>>3));
}

static void cadre_noir(byte * image)
{
    int     i;
	{
		byte *ptr1=image;
		byte *ptr2=image+320*197;
		byte *ptr3=image+320*198;
		byte *ptr4=image+320*199;
		for(i=0; i<320; i++)
			*(ptr1++)=*(ptr2++)=*(ptr3++)=*(ptr4++)=0;
	}
	{
		byte *ptr1=image;
		byte *ptr2=image+319;
		for(i=0; i<200; i++)
			*(ptr1++)=*(ptr2++)=0;
	}
}

static void calc_pal(TParam *pParam, TVar *pVar)
{
	int				i;
	byte			*palette=pParam->palette;
	double			alpha1=pVar->calcpal_alpha1;
	double			alpha2=pVar->calcpal_alpha2;
	double			alpha3=pVar->calcpal_alpha3;

	switch(pParam->choix2)
	{
		case 1:
		{
			int	n=0;
			for(i=0;i<32;i++)
			{
				int nn=(((pVar->calcpal_compteur>>8)+n)&255);
				palette[nn*4+0]=(int) (   ((sin(alpha1+(double)i/48+32)*31+32)*(double)i)/32    );
				palette[nn*4+1]=(int) (   ((sin(alpha2+(double)i/21+42)*31+32)*(double)i)/32    );
				palette[nn*4+2]=(int) (   ((sin(alpha3+(double)i/71+75)*31+32)*(double)i)/32    );
				n++;
			}
			for(i=32;i<224;i++)
			{
				int nn=(((pVar->calcpal_compteur>>8)+n)&255);
				palette[nn*4+0]=(int) (   (sin(alpha1+(double)i/48+32)*31+32)    );
				palette[nn*4+1]=(int) (   (sin(alpha2+(double)i/21+42)*31+32)    );
				palette[nn*4+2]=(int) (   (sin(alpha3+(double)i/71+75)*31+32)    );
				n++;
			}
			for(i=224;i<256;i++)
			{
				int nn=(((pVar->calcpal_compteur>>8)+n)&255);
				palette[nn*4+0]=(int) (   (sin(alpha1+(double)i/48+32)*31+32)*(double)(256-i)/32    );
				palette[nn*4+1]=(int) (   (sin(alpha2+(double)i/21+42)*31+32)*(double)(256-i)/32    );
				palette[nn*4+2]=(int) (   (sin(alpha3+(double)i/71+75)*31+32)*(double)(256-i)/32    );
				n++;
			}
		}
		break;

		case 2:
		memset(pParam->palette, 0, 256*4);
		for(i=0;i<32;i++)
		{
			int n=(((pVar->calcpal_compteur>>8)+i)&255);
			palette[n*4  ]=i*2;
			palette[n*4+1]=i*2;
			palette[n*4+2]=i*2;
		}
		for(i=0;i<32;i++)
		{
			int n=(((pVar->calcpal_compteur>>8)+32+i)&255);
			palette[n*4  ]=63-i*2;
			palette[n*4+1]=63-i*2;
			palette[n*4+2]=63-i*2;
		}
		break;

		case 3:
		{
			int     j;
			double  v=sin((double)pParam->amouy[2]/500);
			double  zt1=10.0+25.0*(sin((double)pParam->amoux[2]/500.0+v)+1);
			double  zt2=10.0+25.0*(sin((double)pParam->amoux[2]/500.0+v*2)+1);
			double  zt3=10.0+25.0*(sin((double)pParam->amoux[2]/500.0+v*3)+1);
			double  dec1=sin((double)pParam->amoux[1]/500.0)*2;
			double  dec2=dec1*2;
			double  dec3=dec1*3;
			int     n=0;
			for(j=0 ; j<4 ; j++)
			{
				for(i=0;i<32;i++)
				{
					int nn=(((pVar->calcpal_compteur>>8)+n)&255);
					palette[nn*4+0]=(int) (((sin((double)n/zt1+dec1)*31+32)*(double)i)/32);
					palette[nn*4+1]=(int) (((sin((double)n/zt2+dec2)*31+32)*(double)i)/32);
					palette[nn*4+2]=(int) (((sin((double)n/zt3+dec3)*31+32)*(double)i)/32);
					n++;
				}
				for(i=31;i>=0;i--)
				{
					int nn=(((pVar->calcpal_compteur>>8)+n)&255);
					palette[nn*4+0]=(int)(((sin((double)n/zt1+dec1)*31+32)*(double)i)/32);
					palette[nn*4+1]=(int)(((sin((double)n/zt2+dec2)*31+32)*(double)i)/32);
					palette[nn*4+2]=(int)(((sin((double)n/zt3+dec3)*31+32)*(double)i)/32);
					n++;
				}
			}
		}
		break;
	}
	pVar->calcpal_alpha1-=0.00011253355*((double)pParam->amouy[2]);
	pVar->calcpal_alpha2-=0.00015265422*((double)pParam->amouy[2]);
	pVar->calcpal_alpha3-=0.00014052142*((double)pParam->amouy[2]);
	pVar->calcpal_compteur+=pParam->amouy[1];
}

static void calc_sinus()
{
	int     i;
	int     *mybuf=sinus;
	for(i=0;i<(1024+256);i++)
		*(mybuf++)=(int) (sin(2*PI*((double)i)/1024)*256);
}

void calc_points(TPoint *mybuf,TParam *pParam, TVar *pVar)
{
	float			decal=pVar->decal;
	int				ticks=GetTickCount();
	if(pParam->spacebar) 
	{
		int	len=ticks-pVar->lastspaceticks;
		if(len<30000)
		{
			pVar->tickslen=len;
			pVar->lastspaceticks=ticks;
		}
		else
		{
			pVar->lastspaceticks=ticks;
			pVar->tickslen=5000;
		}
	}
	else if(pVar->tickslen)
	{
		pVar->calcpoints_angle=PI+(PI*(double)(ticks-pVar->lastspaceticks)/((double)pVar->tickslen) );
	}
	pVar->smooth=0;
	switch(pParam->choix)
	{
		case 1:
		{
			int			i;
			int			inc3=(5000*65536)/MAX;
			int			z=5000*65536;
	        pVar->calcpoints1_alpha2=0;
			for(i=0;i<MAX;i++)
			{
				mybuf->x= ( cosinus[ (((int)decal+pVar->calcpoints1_alpha2)>>8) & 1023 ] + (pParam->amoux[0]>>4) );
				mybuf->y= (   sinus[ (((int)decal+pVar->calcpoints1_alpha2)>>8) & 1023 ] + (pParam->amouy[0]>>4) );
				mybuf->z= ( z>>16 ) ;
				switch(pParam->choix3)
				{
					case 1:
					mybuf->couleur=30;
					break;

					case 2:
					mybuf->couleur=30;
					break;

					case 3:
					mybuf->couleur=VAL3;
					break;
				}
				mybuf++;
				z-=inc3;
				pVar->calcpoints1_alpha2+=pVar->calcpoints1_inc2;
			}
			pVar->calcpoints1_inc2+=5;
			pVar->decal+=pParam->amouy[1];
		}
		break;

		case 2:
		{
			int				i,z,inc3;
			inc3=(5000*65536)/MAX;
			z=5000*65536;
			pVar->calcpoints2_alpha2=0;
			pVar->calcpoints2_alpha1=(-(int)(pVar->calcpoints_angle*1024/(2*PI))*256);
			for(i=0;i<MAX;i++)
			{
				mybuf->x= ( cosinus[ (((int)decal+pVar->calcpoints2_alpha2)>>8) & 1023 ] * sinus[ (pVar->calcpoints2_alpha1>>8) & 1023 ] + (pParam->amoux[0]<<4) ) >> 8  ;
				mybuf->y= (   sinus[ (((int)decal+pVar->calcpoints2_alpha2)>>8) & 1023 ] * sinus[ (pVar->calcpoints2_alpha1>>8) & 1023 ] + (pParam->amouy[0]<<4) ) >> 8  ;
				mybuf->z= ( z>>16 ) ;
				switch(pParam->choix3)
				{
					case 1:
					mybuf->couleur=30;
					break;

                    case 2:
					mybuf->couleur=30;
					break;

					case 3:
					mybuf->couleur=VAL3;
					break;
				}
				mybuf++;
				z-=inc3;
				pVar->calcpoints2_alpha1+=500;
				pVar->calcpoints2_alpha2+=pVar->calcpoints2_inc2;
			}
			pVar->calcpoints2_inc2+=5;
			if(pParam->spacebar)
				pVar->calcpoints2_inc2=rand()*100-16000;
		}
		break;

		case 3:
		{
			int			i,z,inc3;
			inc3=(5000*65536)/MAX;
			z=5000*65536;
			pVar->calcpoints3_alpha2=0;
			pVar->calcpoints3_alpha1=pVar->calcpoints3_depart;
			for(i=0;i<MAX;i++)
			{
				mybuf->x= ( cosinus[ (((int)decal+pVar->calcpoints3_alpha2)>>8) & 1023 ] * sinus[ (pVar->calcpoints3_alpha1>>8) & 1023 ] * (int)pParam->spectre[127-(126*i)/MAX] + (pParam->amoux[0]<<4) ) >> 8;
				mybuf->y= (   sinus[ (((int)decal+pVar->calcpoints3_alpha2)>>8) & 1023 ] * sinus[ (pVar->calcpoints3_alpha1>>8) & 1023 ] * (int)pParam->spectre[127-(126*i)/MAX] + (pParam->amouy[0]<<4) ) >> 8;
				mybuf->z= ( z>>16 ) ;
				switch(pParam->choix3)
				{
					case 1:
					mybuf->couleur=30;
					break;

					case 2:
					mybuf->couleur=30;
					break;

                    case 3:
					mybuf->couleur=VAL3;
					break;
				}
				mybuf++;
				z-=inc3;
				pVar->calcpoints3_alpha1+=500;
				pVar->calcpoints3_alpha2+=pVar->calcpoints3_inc2;
			}
			pVar->calcpoints3_inc2+=5;
			pVar->calcpoints3_depart-=1000;
			pVar->decal+=pParam->amouy[1];
			if(pParam->spacebar)
			{
				pVar->calcpoints3_inc2=rand()*100-16000;
				pVar->calcpoints3_depart=rand()*100-16000;
			}
		}
		break;

		case 4:
		{
			int			i,z,inc3;
			inc3=(5000*65536)/MAX;
			z=5000*65536;
			pVar->calcpoints4_alpha2=0;
			pVar->calcpoints4_alpha1=pVar->calcpoints4_depart;
			for(i=0;i<MAX;i++)
			{
				mybuf->x= ( cosinus[ (((int)decal+pVar->calcpoints4_alpha2)>>8) & 1023 ] * sinus[ (pVar->calcpoints4_alpha1>>8) & 1023 ] * (int)pParam->spectre[(126*i)/MAX] + (pParam->amoux[0]<<4) ) >> 8  ;
				mybuf->y= (   sinus[ (((int)decal+pVar->calcpoints4_alpha2)>>8) & 1023 ] * sinus[ (pVar->calcpoints4_alpha1>>8) & 1023 ] * (int)pParam->spectre[(126*i)/MAX] + (pParam->amouy[0]<<4) ) >> 8  ;
				mybuf->z= ( z>>16 ) ;
				switch(pParam->choix3)
				{
					case 1:
					mybuf->couleur=30;
					break;

					case 2:
					mybuf->couleur=30;
					break;

					case 3:
					mybuf->couleur=VAL3;
					break;
				}
				mybuf++;
				z-=inc3;
				pVar->calcpoints4_alpha1+=500;
				pVar->calcpoints4_alpha2+=pVar->calcpoints4_inc2;
			}
			pVar->calcpoints4_inc2+=5;
			pVar->calcpoints4_depart-=1000;
			pVar->decal+=pParam->amouy[1];
			if(pParam->spacebar)
			{
				pVar->calcpoints4_inc2=rand()*100-16000;
				pVar->calcpoints4_depart=rand()*100-16000;
			}
		}
		break;

		case 5:
		{
			int         i,i1=pVar->calcpoints5_alp1+pVar->calcpoints5_alp3,i2=pVar->calcpoints5_alp2+pVar->calcpoints5_alp3,i4=pVar->calcpoints5_alp4;
			for(i=0;i<256;i++)
			{
				mybuf->x=(cosinus[i1&1023]*sinus[i4&1023]+(pParam->amoux[0]<<4))>>8; 
				i1+=25;
				mybuf->y=(  sinus[i2&1023]*sinus[i4&1023]+(pParam->amouy[0]<<4))>>8; 
				i2+=21;
				i4+=11;
				mybuf->z=500+i; 
				switch(pParam->choix3)
				{
					case 1:
					mybuf->couleur=30;
					break;

					case 2:
					mybuf->couleur=30;
					break;

					case 3:
					mybuf->couleur=VAL3;
					break;
				}
				mybuf++;
			}
			pVar->calcpoints5_alp1+=15;
			pVar->calcpoints5_alp2+=8;
			pVar->calcpoints5_alp3+=3;
			pVar->calcpoints5_alp4+=2;
			for(i=256;i<MAX;i++)
			{
				mybuf->z=-1;
				mybuf++;
			}
		}
		break;
	}
	pVar->decal+=0.125634f;
	projection(pVar->image, pVar->lespoints, MAX, pVar);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static int Action(void *p, TParam *pParam)
{
	TVar	*pVar=(TVar *)p;

	calc_pal(pParam, pVar);
	calc_points(pVar->lespoints, pParam, pVar);

	switch(pParam->choix3)
	{
		case 1:
		{
			pVar->mybool=!pVar->mybool;
			if(pVar->mybool)
			{
				transfert_shade(pVar->image3, pVar->image);
				cadre_noir(pVar->image3);
				plasma(pVar->image4, pVar->image3, table);
				memcpy(pParam->ecran, pVar->image4, 64000);
			}
			else
			{
				transfert_shade(pVar->image4, pVar->image);
				cadre_noir(pVar->image4);
				plasma(pVar->image3, pVar->image4, table);
				memcpy(pParam->ecran, pVar->image3, 64000);
			}
		}
		break;

		case 2:
		{
			transfert_shade(pVar->image3, pVar->image);
			cadre_noir(pVar->image3);
			plasma(pVar->image4, pVar->image3, table);
			myzoom(pVar->image3, pVar->image4);
			memcpy(pParam->ecran, pVar->image3, 64000);
		}
		break;

		case 3:
		{
			pVar->action_mybool=!pVar->action_mybool;
			transfert(pVar->memima[pVar->action_num], pVar->image);
			transfert_shade(pVar->image2, pVar->image);
			soustract(pVar->image2, pVar->memima[pVar->action_numf]);
			if(pVar->action_mybool)
			{
				addition(pVar->image5, pVar->image2);
				cadre_noir(pVar->image5);
				plasma(pVar->image6, pVar->image5, table);
				memcpy(pParam->ecran, pVar->image6, 64000);
			}
			else
			{
				addition(pVar->image6, pVar->image2);
				cadre_noir(pVar->image6);
				plasma(pVar->image5, pVar->image6, table);
				memcpy(pParam->ecran, pVar->image5, 64000);
			}
			pVar->action_num =(pVar->action_num +1)%30;
			pVar->action_numf=(pVar->action_numf+1)%30;
		}
		break;
	}

	return AEE_NOERROR;
}
	
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void *Init()
{
	TVar	*pVar=(TVar *)malloc(sizeof(TVar));
	if(pVar)
	{
		memset(pVar, 0, sizeof(TVar));
		pVar->action_numf=1;
		pVar->action_num=0;
	}
	return (void *)pVar;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void Free(void *p)
{
	if(p)
		free(p);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static TInfo	info=
{
	AET_INFO,
	AET_ECRAN_OLD,
	MAKEINTRESOURCE(IDB_XTIME),
	"Xtime",
	"Xtime",
	"You can use the space bar to\r\nsynced the twisted form on the sound.\r\n",
	"code by renan",
    "an aestesis product",
    "aestesis'98",
    "renan@aestesis.org",
    "http://www.aestesis.org/",
	NULL,
	Action,
	Init,
	Free,
	NULL,
	5,
	3,
	3,
	0,
	{FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE},
	{TRUE, TRUE, TRUE, FALSE},
};

static TTooltips	tooltips=
{
/* choix11		*/	"form: simple",
/* choix12		*/	"form: twisted",
/* choix13		*/	"form: on sound #1",
/* choix14		*/	"form: on sound #2",
/* choix15		*/	"form: sinus",
/* choix16		*/	NULL,
/* choix17		*/	NULL,
/* choix18		*/	NULL,
/* choix19		*/	NULL,
/* choix10		*/	NULL,

/* choix21		*/	"color: normal",
/* choix22		*/	"color: black & white",
/* choix23		*/	"color: vb color",
/* choix24		*/	NULL,
/* choix25		*/	NULL,
/* choix26		*/	NULL,
/* choix27		*/	NULL,
/* choix28		*/	NULL,
/* choix29		*/	NULL,
/* choix20		*/	NULL,

/* choix31		*/	"mode: normal",
/* choix32		*/	"mode: zoomed",
/* choix33		*/	"mode: smoothed",
/* choix34		*/	NULL,
/* choix35		*/	NULL,
/* choix36		*/	NULL,
/* choix37		*/	NULL,
/* choix38		*/	NULL,
/* choix39		*/	NULL,
/* choix30		*/	NULL,

/* choix41		*/	NULL,
/* choix42		*/	NULL,
/* choix43		*/	NULL,
/* choix44		*/	NULL,
/* choix45		*/	NULL,
/* choix46		*/	NULL,
/* choix47		*/	NULL,
/* choix48		*/	NULL,
/* choix49		*/	NULL,
/* choix40		*/	NULL,

/* vb1			*/	"position",
/* vb2			*/	"up/down cycling speed, right/left pseudo saturation (active on vb color)",
/* vb3			*/	"change color (active on vb color)",
/* vb4			*/	NULL,

/* t0			*/	NULL,
/* t1			*/	NULL,
/* t2			*/	NULL,
/* t3			*/	NULL,
/* t4			*/	NULL,
/* t5			*/	NULL,
/* t6			*/	NULL,
/* t7			*/	NULL,
/* t8			*/	NULL,
/* t9			*/	NULL
};

int InitializeXtime(TInfo **ppInfo, TTooltips **ppTooltips)
{
	// set info
	*ppInfo=&info;
	*ppTooltips=&tooltips;
	
	// init global
	{
		int	i;
		for(i=0; i<256 ; i++)
			table[i]=(i*9)/10;
	}
	calc_sinus();
	calc_tabzoom();

	return AEE_NOERROR;		
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
