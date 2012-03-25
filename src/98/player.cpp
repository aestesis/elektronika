// coded by Renan
// project : VIDEOASTESIS
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include					<windows.h>
#include					<math.h>
#include					<stdio.h>
#include					<stdlib.h>
#include					<io.h>
#include					<vfw.h>
#include					"resource.h"

//////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char		byte;
typedef unsigned short		word;
typedef unsigned long		dword;

#include					"player.h"

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define						E_NOTHING					0
#define						E_BLOCKED					1
#define						E_LOAD						2
#define						E_LOADED					3

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

CRITICAL_SECTION			critic;

static byte					table[65536*2*2];
static byte					sinus[256+64];
static byte					sinus_deph[256+64];
static byte					*cosin=sinus+64;
static byte					*cosin_deph=sinus_deph+64;

byte						*defbmp;

static HWND					hwindow=NULL;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

typedef struct SChoixInfo
{
	bool					pingpong;
	int						currentimage;	/* fix 16.16 */
	int						moux;
	int						mouy;
	int						dmoux;
	int						dmouy;
	HANDLE					hThread;
	FILE					*Fli_File;
	byte					*Buffer;

	PAVIFILE				pfile;
	PAVISTREAM				pavi;
	PGETFRAME				pgetframe;

	int						etat;		/* 0..nothing 1..blocked  2..load  3..loaded */
	int						nbimages;
	byte					*images;	/* nbimages * { 320x200 body + 4*256 palette ] */
} TChoixInfo;

typedef struct SVar
{
	int						samps[256*3];
	int						smooth[256*3];
	float					lray;
	float					langle;
	float					lamort;
	float					angle2;
	byte					deform[1024];	//deforma=deform+512
	byte					lsample[256];
	int						angle;
	int						cyc;
	int						decal1;
	int						decal2;
	int						decal3;
	int						decal4;
	int						oldchoix4;
	int						nChoixInfo;
	TChoixInfo				ChoixInfo[10];
	byte					im4roto[256*256];
	float					a1,a2,a3,a4,a5,a6,a7,a8;
	float					d1,d2,d3,d4,d5,d6,d7,d8;
} TVar;

typedef struct sVarThread
{
	TVar					*pVar;
	TParam					*pParam;
	char					name[256];
	int						nChoixInfo;
} TVarThread;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

//........................................................................
//
//     ROTO256: buf_dest , buf_src , struct s_4points * ,taille

//              320x200  aille*taille   16bit(entier)..16bit(d‚cimal)


//      taille =        0         rien        
//                      1         2x  2
//                      2         4x  4
//                      3         8x  8
//                      4        16x 16
//                      5        32x 32
//                      6        64x 64
//                      7       128x128
//                      8       256x256
//                      .
//                      .
//                      16      65536x65536

//.......................................................................
//.......................................................................
//.......................................................................


//s_4point :
//----------
//
//       x1,y1    x2,y2
//         ---------
//         !       !
//         !       !
//         !       !
//         !       !
//         ---------
//       x4,y4    x3,y3


typedef struct SRoto
{
	dword		x1;
	dword		y1;
	dword		x2;
	dword		y2;
	dword		x3;
	dword		y3;
	dword		x4;
	dword		y4;
} TRoto;

#pragma optimize("",off)
#pragma code_seg("automod")

void roto(byte *dest, byte *src, TRoto *p, dword n)
{
	static dword	y;
	__asm
	{
		mov		eax,dest
		mov		edx,src
		mov		ebx,p
		mov		ecx,n

		push	ebp

		lea		esi,cg_dest
		mov		[esi-4],eax

		lea     esi,c1g_src
		mov     [esi-4],edx
		lea     esi,c2g_src
		mov     [esi-4],edx
		lea     esi,c3g_src
		mov     [esi-4],edx
		lea     esi,c4g_src
		mov     [esi-4],edx
                                //calcul mask taille ‚cran source        
                                //-------------------------------
		mov     edx,1          
		shl     edx,cl
		dec     edx
		lea     esi,cg_mask_x1
		mov     [esi-4],edx
		lea     esi,cg_mask_x2
		mov     [esi-4],edx
		lea     esi,cg_mask_x3
		mov     [esi-4],edx
		lea     esi,cg_mask_x4
		mov     [esi-4],edx
		shl     edx,cl
		lea     esi,cg_mask_y1
		mov     [esi-4],edx
		lea     esi,cg_mask_y2
		mov     [esi-4],edx
		lea     esi,cg_mask_y3
		mov     [esi-4],edx
		lea     esi,cg_mask_y4
		mov     [esi-4],edx
		mov     al,16
		sub     al,cl
		lea     esi,cg_mask_rot1
		mov     [esi-1],al
		lea     esi,cg_mask_rot2
		mov     [esi-1],al
		lea     esi,cg_mask_rot3
		mov     [esi-1],al
		lea     esi,cg_mask_rot4
		mov     [esi-1],al


                                //calcul des dx200 et dy200 
                                //-------------------------

		mov     ecx,200

		mov     eax,[ebx]TRoto.x4    //calc dx1
		sub     eax,[ebx]TRoto.x1
		cdq
		idiv    ecx
		lea     esi,cg_dx1
		mov     [esi-4],eax

		mov     eax,[ebx]TRoto.x3    //calc dx2
		sub     eax,[ebx]TRoto.x2
		cdq
		idiv    ecx
		lea     esi,cg_dx2
		mov     [esi-4],eax

		mov     eax,[ebx]TRoto.y4    //calc dy1
		sub     eax,[ebx]TRoto.y1
		cdq
		idiv    ecx
		lea     esi,cg_dy1
		mov     [esi-4],eax

		mov     eax,[ebx]TRoto.y3    //calc dy2
		sub     eax,[ebx]TRoto.y2
		cdq
		idiv    ecx
		lea     esi,cg_dy2
		mov     [esi-4],eax

		mov     esi,[ebx]TRoto.x1	//coordonn‚es d‚part des lignes dans les lignes
		mov     edi,[ebx]TRoto.y1
		mov     eax,[ebx]TRoto.x2
		mov     ebp,[ebx]TRoto.y2
		mov     ebx,eax


                                //calcul des x_dep, y_dep , dx320 et dy320
                                //----------------------------------------

		mov     y,200
		mov     ecx,320

        
	boucy200:
		mov     eax,ebp         //calc dy320
		sub     eax,edi         //
		cdq                     //
		idiv    ecx             //
		push    eax             //push dy320

		mov     eax,ebx         //calc dx320
		sub     eax,esi         //
		cdq                     //
		idiv    ecx             //
		push    eax             //push dx320

		push    edi
		push    esi

		add     esi,012345678h
	cg_dx1:
		add     edi,012345678h
	cg_dy1:
		add     ebx,012345678h
	cg_dx2:
		add     ebp,012345678h
	cg_dy2:

		dec     y
		jnz     boucy200


                                //Affiche Roto
                                //------------

		mov     y,200
		mov     ebx,012345678h
	cg_dest:

	boucy:

		pop     esi                     //get x_dep
		pop     edi                     //get y_dep
		pop     eax                     //get dx320
		lea     ecx,c1g_dx320
		mov     [ecx-4],eax
		lea     ecx,c2g_dx320
		mov     [ecx-4],eax
		lea     ecx,c3g_dx320
		mov     [ecx-4],eax
		lea     ecx,c4g_dx320
		mov     [ecx-4],eax
		pop     eax                     //get dy320
		lea     ecx,c1g_dy320
		mov     [ecx-4],eax
		lea     ecx,c2g_dy320
		mov     [ecx-4],eax
		lea     ecx,c3g_dy320
		mov     [ecx-4],eax
		lea     ecx,c4g_dy320
		mov     [ecx-4],eax

		mov     ecx,320/4               //diviser par 4 pour aligner

	boucx:
		mov     ebp,esi                 //enlŠve partie d‚cimale 
		mov     edx,edi
		shr     ebp,16
		shr     edx,8                   //et calcul y*256
	cg_mask_rot1:
		and     ebp,012345678h
	cg_mask_x1:
		and     edx,012345678h
	cg_mask_y1:
		or      edx,ebp                 //edx=adr=x+y*256

		mov     al,[edx+012345678h]
	c1g_src:
		rol     eax,8

		add     esi,012345678h
	c1g_dx320:
		add     edi,012345678h
	c1g_dy320:

		mov     ebp,esi                 //enlŠve partie d‚cimale 
		mov     edx,edi
		shr     ebp,16
		shr     edx,8                   //et calcul y*256
	cg_mask_rot2:
		and     ebp,012345678h
	cg_mask_x2:
		and     edx,012345678h
	cg_mask_y2:
		or      edx,ebp                 //edx=adr=x+y*256

		mov     al,[edx+012345678h]
	c2g_src:
		rol     eax,8

		add     esi,012345678h
	c2g_dx320:
		add     edi,012345678h
	c2g_dy320:

		mov     ebp,esi                 //enlŠve partie d‚cimale 
		mov     edx,edi
		shr     ebp,16
		shr     edx,8                   //et calcul y*256
	cg_mask_rot3:
		and     ebp,012345678h
	cg_mask_x3:
		and     edx,012345678h
	cg_mask_y3:
		or      edx,ebp                 //edx=adr=x+y*256

		mov     al,[edx+012345678h]
	c3g_src:
		rol     eax,8

		add     esi,012345678h
	c3g_dx320:
		add     edi,012345678h
	c3g_dy320:

		mov     ebp,esi                 //enlŠve partie d‚cimale 
		mov     edx,edi
		shr     ebp,16
		shr     edx,8                   //et calcul y*256
	cg_mask_rot4:
		and     ebp,012345678h
	cg_mask_x4:
		and     edx,012345678h
	cg_mask_y4:
		or      edx,ebp                 //edx=adr=x+y*256

		mov     al,[edx+012345678h]
	c4g_src:

		add     esi,012345678h
	c4g_dx320:
		add     edi,012345678h
	c4g_dy320:

		bswap   eax
		mov     [ebx],eax
		add     ebx,4

		dec     ecx

        jnz     boucx

        dec     y
        jnz     boucy

        pop		ebp
	}
}


void stretch(byte *dest, byte *src)   // 320x200 to 256x256
{
	__asm
	{
		mov		esi,src
		mov		edi,dest
		xor		ebx,ebx

		mov		edx,256
		push	ebx

	boucy:
		pop		ebx
		dec		edx
		js		theend

		add		ebx,199
		push	ebx
		shr		ebx,2
		mov		ecx,256
		and		ebx,0xfffffc0
		lea		ebx,[ebx*4+ebx]
		shl		ebx,8

	boucx:
		mov		eax,ebx
		dec		ecx
		js		boucy

		shr		eax,8
		mov		al,[eax+esi]
		add		ebx,319
		mov		[edi],al
		inc		edi
		jmp		boucx

	theend:
	}
}
#pragma code_seg()
#pragma optimize("",on)


void rotozoom(byte *ecran_dest, byte *ecran_src, TVar *pVar, TParam *pParam)
{
    TRoto		p;
    double		ang1=sin((double)pParam->amouy[2]/5000.0)*10;
    double		ang2=sin((double)pParam->amoux[2]/5000.0);
    double		v1=sin(ang2)*ang1;
    double		v2=sin(ang2*2)*ang1;
    double		v3=sin(ang2*3)*ang1;
    double		v4=sin(ang2*4)*ang1;
	float		dangle,zoom,px,py;

    pVar->langle+=((float)pParam->amoux[1])/300000;
    pVar->lray=(((float)pParam->amouy[1]*(float)pParam->amouy[1]/4000)+1);

    px=(float)(128.f+cos(pVar->langle)*pVar->lray);
    py=(float)(128.f+sin(pVar->langle)*pVar->lray);

    dangle=((float)pParam->amoux[3])/30000;
    zoom=(((float)pParam->amouy[3]*(float)pParam->amouy[3]/4000)+1);
    pVar->angle2+=dangle;

    p.x1=(int)((px+zoom*cos(v1+pVar->angle2-0.5585993153))*65536);
    p.y1=(int)((py+zoom*sin(v1+pVar->angle2-0.5585993153))*65536);
        
    p.x2=(int)((px+zoom*cos(v2+pVar->angle2-2.5829933382))*65536);
    p.y2=(int)((py+zoom*sin(v2+pVar->angle2-2.5829933382))*65536);

    p.x3=(int)((px+zoom*cos(v3+pVar->angle2+2.5829933382))*65536);
    p.y3=(int)((py+zoom*sin(v3+pVar->angle2+2.5829933382))*65536);

    p.x4=(int)((px+zoom*cos(v4+pVar->angle2+0.5585993153))*65536);
    p.y4=(int)((py+zoom*sin(v4+pVar->angle2+0.5585993153))*65536);

	stretch(pVar->im4roto, ecran_src);
    roto(ecran_dest, pVar->im4roto, &p, 8);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

byte *loadbmp(char *filename)
{
	if(filename>(char *)0x100000)
	{
		FILE				*f=fopen(filename, "rb");
		byte				*image;
		BITMAPFILEHEADER	bf;
		BITMAPINFOHEADER	bi;
		int					size;
		int					nbc;
		if(!f)
		{
			return NULL;
		}
		if(fread(&bf, sizeof(bf), 1, f)!=1)
		{
			fclose(f);
			return NULL;
		}
		if(fread(&bi, sizeof(bi), 1, f)!=1)
		{
			fclose(f);
			return NULL;
		}
		nbc=(bi.biClrImportant==0)?256:bi.biClrImportant;
		size=bi.biWidth*bi.biHeight;
		if((bi.biBitCount!=8)||(bi.biWidth!=320)||(bi.biHeight!=200))
		{
			fclose(f);
			return NULL;
		}
		image=(byte *)malloc(320*200+256*4);
		if(!image)
		{
			fclose(f);
			return NULL;
		}
		if(fread(image+320*200, nbc*4, 1, f)!=1)
		{
			free(image);
			fclose(f);
			return NULL;
		}
		{
			int	y;
			for(y=0; y<200; y++)
			{
				if(fread(image+(199-y)*320, 320, 1, f)!=1)
				{
					free(image);
					fclose(f);
					return NULL;
				}
			}
		}
		fclose(f);
		return image;
	}
	else
	{
		HINSTANCE			hinst=GetModuleHandle("98.dll");
        HRSRC				h=FindResource(hinst, filename, RT_BITMAP);
        BITMAPINFOHEADER	*pbi=(BITMAPINFOHEADER *)LockResource(LoadResource(hinst, h));
		if(pbi)
		{
			int					size=pbi->biWidth*pbi->biHeight*4;
			void				*body=(void *)(pbi+1);
			byte				*image;
			if((pbi->biBitCount!=8)&&(pbi->biWidth!=320)&&(pbi->biHeight!=200))
				return NULL;
			image=(byte *)malloc(320*200+256*4);
			if(!image)
				return NULL;
			memcpy(image+320*200, body, 256*4);
			{
				int	y;
				for(y=0; y<200; y++)
					memcpy(image+y*320, (byte *)body+256*4+(199-y)*320, 320);
			}
			return image;
		}
    }
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define HEADER_SIZE 128

typedef struct  
{
	unsigned long	size;
	unsigned short	type;
	unsigned short	nb_frame;
	unsigned short	width;
	unsigned short	height;
	unsigned short	bits_a_pixel;
	short			flags;
	short			speed;
	long			next_head;
	long			frames_in_table;
	char			expand[102];       
} fli_head;

typedef struct
{
	unsigned long	size;
	unsigned short	type;    
	short			chunks;
	char			expand[8];
} fli_frame;

typedef struct  
{
	unsigned long	size;
	short			type;
} fli_chunk;


static void fli_color(byte *p, byte	*palette)
{
	__asm
	{
		mov     esi,p
		mov     bh,[esi]			//bh=nombre de paquets
		add     esi,2
                
		mov     bl,0				//pointeur de couleur
	other_paquet:
		mov     al,[esi]			//al=nombre de couleur … sauter
		add     bl,al

		mov		edi,ebx
		and		edi,0ffh
		shl		edi,2
		add		edi,palette
        
		inc     esi
		mov     cl,[esi]			//cl=nombre de couleur a changer
        
		inc     esi					//esi pointe sur les composantes

	all_compo_p:
        mov     al,[esi+0]
		shl		al,2
        mov     [edi+2],al
        mov     al,[esi+1]
		shl		al,2
        mov     [edi+1],al
        mov     al,[esi+2]
		shl		al,2
        mov     [edi+0],al
		add		esi,3
		add		edi,4
        dec     cl
        jnz     all_compo_p      
        
        dec     bh
        jnz     other_paquet
	}
}

static void fli_lc(byte *p, byte *image, int width)
{
	__asm
	{
        mov     edi,image
        
        mov     ecx,width			//edx pointe a la fin du buffer     
        mov     esi,p				//esi pointe sur le compactage
		push	ebp
		mov		ebp,ecx
        
        movsx   eax,word ptr [esi]
        add     esi,2
        mul     ebp
        add     edi,eax				//edi depart du decompactage
        
        movsx   edx,word ptr [esi]
        add     esi,2
        
	all_compressed_lines:
        call    line_comp_fli
        
        dec     edx
        jnz     all_compressed_lines
		jmp		theend

	line_comp_fli:
        push    edi
        mov     bl,[esi]			//bl=nombre de paquets
        inc     esi
        mov     bh,0
        
	all_paquet_lc:
        cmp     bh,bl
        jz      end_line_lc
        movzx   eax,byte ptr [esi]	//saut
        inc     esi
        add     edi,eax				//fixer
        
		movsx   ecx,byte ptr [esi]
		inc     esi
		or      ecx,ecx
		js      neg_lc

	//pos_lc: 
		rep     movsb       
		inc     bh
		jmp     all_paquet_lc

	neg_lc: 
		neg     ecx
		mov     al,[esi]
		inc     bh
		rep     stosb
		inc     esi 
		jmp     all_paquet_lc

	end_line_lc:
		pop     edi
		add     edi,ebp
		ret

	theend:
		pop		ebp
	}
}

static void fli_black(byte *image, int n)
{
	__asm
	{
		mov		ecx,n
        mov     edi,image
        shr     ecx,2
        sub     eax,eax
        rep     stosd
	}
}

static void fli_brun(byte *p, byte *image, int n)
{
	__asm
	{
        mov     esi,p				//esi pointe sur le compactage
        mov     edi,image
		mov		ebx,n
		push	ebp
        sub     ebp,ebp				//ebp=compteur de ligne
        
	other_line:
        cmp     ebx,ebp
        jz      end_fli_brun
        inc     ebp
        
        mov     dl,[esi]			//bl=nb de paquet de la ligne
        inc     esi
        mov     dh,0

	all_fb:        
        cmp     dh,dl
        jz      other_line
        inc     dh
        movsx   ecx,byte ptr[esi]	//nb d'octets … copier
        inc     esi
        or      ecx,ecx
        js      neg_fb
        
	//pos_fb:						//positif donc on copie le meme octet cl fois
        mov     al,[esi]
        rep     stosb        
        inc     esi
        jmp     all_fb

	neg_fb:
		neg     ecx
        rep     movsb
        jmp     all_fb
        
	end_fli_brun:
		pop		ebp
	}
}

static void fli_copy(byte *p, byte *image, int n)
{
	__asm
	{
        mov     esi,p
        mov     edi,image
        mov     ecx,n
        shr     ecx,2
        rep     movsd
	}
}

static dword __stdcall loadfli(void *p)
{
	if(p)
	{
		TVarThread			*pVarThread=(TVarThread *)p;
		TVar				*pVar=pVarThread->pVar;
		TParam				*pParam=pVarThread->pParam;
		TChoixInfo			*pCI=&pVar->ChoixInfo[pVarThread->nChoixInfo];
		long				frame_ptr=0;
		long				chunk_ptr=0;
		int					fi=0;
		int					fj=0;
		fli_head			*p_fh=0;
		fli_frame			*p_ff=0;
		fli_chunk			*p_fc=0;
		unsigned long		Taille_Fichier;
		int					Handle_Fichier;

		EnterCriticalSection(&critic);
		pCI->etat=E_LOAD;
		LeaveCriticalSection(&critic);

		pCI->Fli_File=fopen(pVarThread->name, "rb");
		free(pVarThread);
		if(pCI->Fli_File==NULL)
		{
			EnterCriticalSection(&critic);
			pCI->etat=E_NOTHING;
			pCI->hThread=NULL;
			LeaveCriticalSection(&critic);
			MessageBox(NULL, "     error in FLI file.     ", "aestesis'98 - player effect", MB_OK);
			return 1;
		}

		Handle_Fichier = _fileno(pCI->Fli_File);
		Taille_Fichier = _filelength(Handle_Fichier);

		if(Taille_Fichier==(dword)-1L)
		{
			EnterCriticalSection(&critic);
			fclose(pCI->Fli_File);
			pCI->Fli_File=NULL;
			pCI->etat=E_NOTHING;
			pCI->hThread=NULL;
			LeaveCriticalSection(&critic);
			MessageBox(NULL, "     error in FLI file.     ", "aestesis'98 - player effect", MB_OK);
			return 2;
		}

		pCI->Buffer=(byte *)malloc(Taille_Fichier);
		if(!pCI->Buffer)
		{
			EnterCriticalSection(&critic);
			fclose(pCI->Fli_File);
			pCI->Fli_File=NULL;
			pCI->etat=E_NOTHING;
			pCI->hThread=NULL;
			LeaveCriticalSection(&critic);
			MessageBox(NULL, "     error loading FLI file, not enought memory.     ", "aestesis'98 - player effect", MB_OK);
			return 3;
		}

		{
			byte	*p=pCI->Buffer;
			while(1)
			{
				if(Taille_Fichier>4096)
				{
					if(fread(p,4096,1,pCI->Fli_File)!=1)
					{
						EnterCriticalSection(&critic);
						fclose(pCI->Fli_File);
						free(pCI->Buffer);
						pCI->etat=E_NOTHING;
						pCI->Buffer=NULL;
						pCI->Fli_File=NULL;
						pCI->hThread=NULL;
						LeaveCriticalSection(&critic);
						MessageBox(NULL, "     read error when loading FLI file.     ", "aestesis'98 - player effect", MB_OK);
						return 4;
					}
					Taille_Fichier-=4096;
					p+=4096;
				}
				else
				{
					if(fread(p,Taille_Fichier,1,pCI->Fli_File)!=1)
					{
						EnterCriticalSection(&critic);
						fclose(pCI->Fli_File);
						free(pCI->Buffer);
						pCI->etat=E_NOTHING;
						pCI->Buffer=NULL;
						pCI->Fli_File=NULL;
						pCI->hThread=NULL;
						LeaveCriticalSection(&critic);
						MessageBox(NULL, "     read error when loading FLI file.     ", "aestesis'98 - player effect", MB_OK);
						return 4;
					}
					break;
				}
				Sleep(10);
			}
		}

		p_fh=(fli_head*)(pCI->Buffer);
		frame_ptr=HEADER_SIZE;
		p_ff=(fli_frame*)(pCI->Buffer+frame_ptr);
		p_fc=(fli_chunk*)(pCI->Buffer+frame_ptr+16);
		fclose(pCI->Fli_File);
		pCI->Fli_File=NULL;

		pCI->images=(byte *)malloc(p_fh->nb_frame*(320*200+256*4));
		if(!pCI->images)
		{
			EnterCriticalSection(&critic);
			free(pCI->Buffer);
			pCI->Buffer=NULL;
			pCI->etat=E_NOTHING;
			pCI->hThread=NULL;
			LeaveCriticalSection(&critic);
			MessageBox(NULL, "     error loading FLI file, not enought memory.     ", "aestesis'98 - player effect", MB_OK);
			return 5;
		}

		{
			byte	palette[256*4];
			byte	image[320*200];
			memset(palette, 0, 256*4);
			memset(image, 0, 320*200);
			for(fi=0; fi<p_fh->nb_frame; fi++)
			{
				chunk_ptr=frame_ptr+16;
				if (p_ff->chunks!=0) 
				{
					for (fj=0;fj<p_ff->chunks;fj++)
					{
						p_fc=(fli_chunk*)(pCI->Buffer+chunk_ptr);
						switch (p_fc->type)
						{
							case 11:
							fli_color(pCI->Buffer+chunk_ptr+6, palette);
							break;

							case 12:
							fli_lc(pCI->Buffer+chunk_ptr+6,image,p_fh->width);
							break;

							case 13:
							fli_black(image,p_fh->width*p_fh->height);
							break;

							case 15:
							fli_brun(pCI->Buffer+chunk_ptr+6,image,p_fh->height);
							break;

							case 16:
							fli_copy(pCI->Buffer+chunk_ptr+6,image,(p_fc->size)-6);
							break;
						}
						chunk_ptr=chunk_ptr+p_fc->size;
					}
				}
				frame_ptr+=p_ff->size;
				p_ff=(fli_frame*)(pCI->Buffer+frame_ptr);
				{
					byte	*img=pCI->images+((320*200+256*4)*fi); 
					byte	*pal=img+320*200;
					memcpy(img, image, 320*200);
					memcpy(pal, palette, 256*4);
				}
			}
		}
		EnterCriticalSection(&critic);
		pCI->nbimages=p_fh->nb_frame;
		free(pCI->Buffer);
		pCI->Buffer=NULL;
		pCI->etat=E_LOADED;
		pCI->hThread=NULL;
		LeaveCriticalSection(&critic);
	}
	return	0;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void avistretch(byte *dest, byte *src, int width, int height)
{
	int		ddx=(width<<8)/320;
	int		ddy=(height<<8)/200;
	__asm
	{
		mov		eax,width
		mov		ecx,ddy
		lea		esi,cg_width
		lea		edx,cg_dy
		mov		[esi-4],eax
		mov		[edx-4],ecx

		mov		eax,src
		mov		ebx,ddx
		lea		esi,cg_src
		lea		edi,cg_dx1
		lea		edx,cg_dx2
		mov		[esi-4],eax
		mov		[edi-4],ebx
		mov		[edx-4],ebx

		mov		edi,dest
		add		edi,320*201
		xor		edx,edx

		push	ebp
		mov		ebp,200
	boucy:
		sub		edi,320*2
		dec		ebp
		js		theend

		mov		eax,edx
		add		edx,012345678h
	cg_dy:
		shr		eax,8
		mov		ebx,012345678h
	cg_width:
		imul	eax,ebx
		lea		esi,cg_ligne1
		lea		ebx,cg_ligne2
		add		eax,012345678h
	cg_src:
		mov		ecx,320/2
		mov		[esi-4],eax
		mov		[ebx-4],eax
		xor		esi,esi

	boucx:
		dec		ecx
		js		boucy

		mov		eax,esi
		add		esi,012345678h
	cg_dx1:
		mov		ebx,esi
		add		esi,012345678h
	cg_dx2:
		shr		eax,8
		shr		ebx,8
		mov		al,[012345678h+eax]
	cg_ligne1:
		mov		ah,[012345678h+ebx]
	cg_ligne2:
		mov		[edi],ax
		add		edi,2
		jmp		boucx

	theend:
		pop		ebp
	}
}

static dword __stdcall loadavi(void *p)
{
	if(p)
	{
		TVarThread			*pVarThread=(TVarThread *)p;
		TVar				*pVar=pVarThread->pVar;
		TParam				*pParam=pVarThread->pParam;
		TChoixInfo			*pCI=&pVar->ChoixInfo[pVarThread->nChoixInfo];
		HRESULT				res;
		AVISTREAMINFO		avi;
		BITMAPINFOHEADER	*pbi=NULL;
		dword				error=0;
		int					aviwidth;
		int					aviheight;
		int					aviframes;
		int					etat=E_NOTHING;
		int					nbimages=0;

		EnterCriticalSection(&critic);
		pCI->etat=E_LOAD;
		LeaveCriticalSection(&critic);
		res=AVIFileOpen(&pCI->pfile, pVarThread->name, OF_READ, 0L);
		free(pVarThread);
		if(res) 
		{
			switch (res) 
			{
				case AVIERR_BADFORMAT:
				{
					int	a=5;
					a++;
				}
				break;
				case AVIERR_MEMORY:
				{
					int	a=5;
					a++;
				}
				break;
				case AVIERR_FILEREAD:
				{
					int	a=5;
					a++;
				}
				break;
				case AVIERR_FILEOPEN:
				{
					int	a=5;
					a++;
				}
				break;
				case REGDB_E_CLASSNOTREG:
				{
					int	a=5;
					a++;
				}
				break;
				default:
				break;
			}
			error=1;
			MessageBox(NULL, "     error in AVI file.     ", "aestesis'98 - player effect", MB_OK);
			goto theend;
		}
		res=AVIFileGetStream(pCI->pfile, &pCI->pavi, streamtypeVIDEO, 0);
		if(res) 
		{
			error=2;
			MessageBox(NULL, "     error in AVI file.     ", "aestesis'98 - player effect", MB_OK);
			goto theend;
		}
	
		res=AVIStreamInfo(pCI->pavi, &avi, sizeof(AVISTREAMINFO));
		if(res)
		{
			error=3;
			MessageBox(NULL, "     error in AVI file.     ", "aestesis'98 - player effect", MB_OK);
			goto theend;
		}
	

		aviwidth=(WORD)(avi.rcFrame.right-avi.rcFrame.left);
		aviheight=(WORD)(avi.rcFrame.bottom-avi.rcFrame.top);
		aviframes=avi.dwLength;
		  
		//-- Load Frame --------------------------------------
	
		pCI->pgetframe = AVIStreamGetFrameOpen(pCI->pavi,NULL);
		if (!pCI->pgetframe) 
		{
			error=4;
			MessageBox(NULL, "     error in AVI file.     ", "aestesis'98 - player effect", MB_OK);
			goto theend;
		}

		pCI->images=(byte *)malloc(aviframes*(320*200+256*4));
		if(!pCI->images)
		{
			error=5;
			MessageBox(NULL, "     error in AVI file.     ", "aestesis'98 - player effect", MB_OK);
			goto theend;
		}

		int		frame;
		for(frame=0; frame<aviframes; frame++)
		{
			pbi=(BITMAPINFOHEADER *)AVIStreamGetFrame(pCI->pgetframe,frame);
			if(!pbi)
			{
				error=6;
				MessageBox(NULL, "     error in AVI file.     ", "aestesis'98 - player effect", MB_OK);
				goto theend;
			}
	
			switch(pbi->biBitCount)
			{
				case 8:
				if((aviwidth==320)&&(aviheight==200))
				{
					byte	*img=pCI->images+((320*200+256*4)*frame); 
					byte	*pal=img+320*200;
					memcpy(pal, pbi+1, 256*4);
					{
						int		y;
						byte	*d=img;
						byte	*s=(byte *)(pbi+1)+256*4;
						s+=320*199;
						for(y=0; y<200; y++)
						{
							memcpy(d, s, 320);
							s-=320;
							d+=320;
						}
					}
				}
				else
				{
					byte	*img=pCI->images+((320*200+256*4)*frame); 
					byte	*pal=img+320*200;
					memcpy(pal, pbi+1, 256*4);
					avistretch(img, (byte *)(pbi+1)+256*4, aviwidth, aviheight);
				}
				break;

				default:
				error=7;
				MessageBox(hwindow, "    Player load only 256 colors AVI files.    ", "aestesis'98 - player effect", MB_OK);
				goto theend;
			}
			Sleep(0);
		}
		etat=E_LOADED;
		nbimages=aviframes;

	theend:
		EnterCriticalSection(&critic);
		if(pCI->pgetframe)   
		{
			AVIStreamGetFrameClose(pCI->pgetframe);
			pCI->pgetframe=NULL;
		}
		if(pCI->pavi)
		{
			AVIStreamRelease(pCI->pavi);
			pCI->pavi=NULL;
		}
		if(pCI->pfile)
		{
			AVIFileRelease(pCI->pfile);
			pCI->pfile=NULL;
		}
		pCI->Fli_File=NULL;
		pCI->hThread=NULL;
		pCI->etat=etat;
		pCI->nbimages=nbimages;
		LeaveCriticalSection(&critic);
		return error;
	}
	return 10;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void affiche(byte *dest, byte *source, TVar *pVar)
{
	dword	deformation=(dword)&pVar->deform[0];
	__asm
	{
		mov		eax,dest
		mov		edx,source
        mov     edi,deformation
        push    ebp
        mov     ebp,edx						// adresse source dans ebp
        mov     edx,eax						// adresse destination dans edx
        add     edx,64000-1
        lea     esi,table[0]

        mov     ecx,64000
        xor     ebx,ebx
        xor     eax,eax

	mybouc4:
        xor     eax,eax
        mov     bx,[esi+ecx*2-2]			// met dans bx coord pol du point (cx)

        shl     ecx,16

        mov     al,bl						// calcul les d‚formation
        mov     cx,bx
        add     bx,[edi+eax*2]
        mov     al,ch
        add     bx,[edi+eax*2+512]

        shr     ecx,16

        mov     ax,[esi+ebx*2+(65536*2)]	// met dans ax coord cart
        
        mov     al,[eax+ebp]				// prend la couleur du point
        mov     [edx],al					// affiche le point

        dec     edx
        dec     ecx
        jnz     mybouc4

        pop     ebp
	}
}

static void affiche2(byte *dest, byte *source)
{
}

static void calc_table(void)
{
	{
		int x;
		int y;
		byte   *ptr=table;
		double src_x;
		double src_y;
		double r;
		double teta;
		for(y=0; y<200; y++)
			for(x=0; x<320; x++)
			{
				src_x=160-x;
				src_y=100-y;
				r=sqrt(src_x*src_x+src_y*src_y);
				if(r!=0)
				{
					if(src_y>0)
						teta=asin(src_y/r);
					else
						teta=asin(-src_y/r);
					if(src_y>0)
					{
						if(src_x<0)
							teta=PI+teta;
						else
							teta=PI*2-teta;
					}      
					else if(src_x<0)
						teta=PI-teta;
                        
				} 
				*(ptr++)=(byte)((teta*128)/PI);
				*(ptr++)=(byte)r;
			}
	}
	{
		int           teta;
		int           r;
		int           x;
		int           y;
		word          *mybuf=(word *)(table+65536*2);
		for(r=0; r<256; r++)
			for(teta=0; teta<256; teta++)
			{
				x=(int) (-cos((double)2*(double)PI*(double)teta/(double)256)*(double)r+(double)160);
				y=(int) (sin((double)2*(double)PI*(double)teta/(double)256)*(double)r+(double)100);
				if ((x>=0) && (x<=319) && (y>=0) && (y<=199))
					*(mybuf+r*256+teta) = x+y*320;
				else
					*(mybuf+r*256+teta) = 0;
			}
	}
}

static void calc_sinus()
{
	int i;
	for (i=0; i<(256+64); i++)
	{
		sinus[i]      = (byte) ((float)sin((2*PI*(double)i)/256)*127+128);
		sinus_deph[i] = (byte) ((float)sin((2*PI*(double)i)/256)*60+128);
	}
}


void calc_samps(int *samps, byte *sample)
{
    int     i;
    int     s1=0;
    int     s2=0;
    int     smp[256];

	{
		int				*dest=smp;
		unsigned char	*src=sample;
		for(i=0; i<256; i++)
			*(dest++)=*(src++)-127;
	}

    for(i=-31; i<32; i++)
    {
        s1+=smp[i&255];
    }

	{
		int				*dest=samps;
		for(i=0; i<256; i++)
		{
			int     d=i-31;
			int     f=i+32;
			d=smp[d&255];
			f=smp[f&255];
			s1+=f-d;
			*(dest++)=s1>>5;
		}
	}

    for(i=0; i<256; i++)
        smp[i]-=samps[i];


    for(i=-4; i<5; i++)
    {
        s2+=smp[i&255];
    }

    for(i=0; i<256; i++)
    {
        int     d=i-4;
        int     f=i+5;
        d=smp[d&255];
        f=smp[f&255];
        s2+=f-d;
        samps[i+256]=s2>>3;
    }

    for(i=0; i<256; i++)
        samps[i+512]=smp[i]-samps[i+256];

    for(i=0; i<256; i++)
    {
         int     a;
    
         a=(samps[i]*samps[i])>>4;
         if(a>127) a=127;
         samps[i]=a;
     
         a=(samps[i+256]*samps[i+256])>>4;
         if(a>127) a=127;
         samps[i+256]=a;
     
         a=(samps[i+512]*samps[i+512])>>4;
         if(a>127) a=127;
         samps[i+512]=a;
    }
}


static void calc_pal(byte *image, TVar *pVar, TParam *pParam)
{
	switch(pParam->choix3)
	{
		case 2:
		calc_samps(pVar->samps, pParam->sample);
		{
			int		i;
			int		*d1=pVar->smooth;
			int		*d2=pVar->smooth+256;
			int		*d3=pVar->smooth+512;
			int		*s1=pVar->samps;
			int		*s2=pVar->samps+256;
			int		*s3=pVar->samps+512;
			for(i=0; i<256; i++)
			{
				*(d1)=((*d1)*3+*(s1++))>>2;
				d1++;
				*(d2)=((*d2)*3+*(s2++))>>2;
				d2++;
				*(d3)=((*d3)*3+*(s3++))>>2;
				d3++;
			}
		}
		{
			int		i;
			byte	*si=image+320*200;
			byte	*p=pParam->palette;
			int		*s1=pVar->smooth;
			int		*s2=pVar->smooth+256;
			int		*s3=pVar->smooth+512;
			for(i=0; i<256; i++)
			{
				*(p++)=(*(s3++)* *(si+0))>>7;
				*(p++)=(*(s2++)* *(si+1))>>7;
				*(p++)=(*(s1++)* *(si+2))>>7;
				p++;
				si+=4;
			}
		}
		break;

		case 3:
		{
			int		i;
			byte	*si=image+320*200;
			byte	*p=pParam->palette;
			int		*s1=pVar->samps;
			int		*s2=pVar->samps+256;
			int		*s3=pVar->samps+512;
			calc_samps(pVar->samps, pParam->sample);
			for(i=0; i<256; i++)
			{
				*(p++)=(*(s3++)* *(si+0))>>7;
				*(p++)=(*(s2++)* *(si+1))>>7;
				*(p++)=(*(s1++)* *(si+2))>>7;
				p++;
				si+=4;
			}
		}
		break;

		case 4:
		calc_samps(pVar->samps, pParam->sample);
		{
			int		i;
			int		*d1=pVar->smooth;
			int		*d2=pVar->smooth+256;
			int		*d3=pVar->smooth+512;
			int		*s1=pVar->samps;
			int		*s2=pVar->samps+256;
			int		*s3=pVar->samps+512;
			for(i=0; i<256; i++)
			{
				*(d1)=((*d1)*3+*(s1++))>>2;
				d1++;
				*(d2)=((*d2)*3+*(s2++))>>2;
				d2++;
				*(d3)=((*d3)*3+*(s3++))>>2;
				d3++;
			}
		}
		{
			int		i;
			byte	*p=pParam->palette;
			int		*s1=pVar->smooth;
			int		*s2=pVar->smooth+256;
			int		*s3=pVar->smooth+512;
			/*
			for(i=0; i<64; i++)
			{
				*(p++)=(*(s1++)*i)>>6;
				*(p++)=(*(s2++)*i)>>6;
				*(p++)=(*(s3++)*i)>>6;
				p++;
			}
			*/
			for(i=0; i<256; i++)
			{
				*(p++)=*(s3++);
				*(p++)=*(s2++);
				*(p++)=*(s1++);
				p++;
			}
		}
		break;

		case 5:
		{
			int		i;
			byte	*p=pParam->palette;
			int		*s1=pVar->samps;
			int		*s2=pVar->samps+256;
			int		*s3=pVar->samps+512;
			calc_samps(pVar->samps, pParam->sample);
			for(i=0; i<256; i++)
			{
				*(p++)=*(s3++);
				*(p++)=*(s2++);
				*(p++)=*(s1++);
				p++;
			}
		}
		break;

		case 6:
		{
			int		i;
			byte	*p=pParam->palette;
			float	a1=(float)sin(pVar->a1)*3.f;
			float	a2=(float)sin(pVar->a2)*3.f;
			float	a3=(float)sin(pVar->a3)*3.f;
			float	a4=(float)sin(pVar->a7)*3.f;
			float	d1=(float)sin(pVar->a4)*0.1f+0.1f;
			float	d2=(float)sin(pVar->a5)*0.1f+0.1f;
			float	d3=(float)sin(pVar->a6)*0.1f+0.1f;
			float	d4=(float)sin(pVar->a8)*0.1f+0.1f;
			for(i=0 ; i<256; i++)
			{
				float	v=(float)(sin(a4)*0.5f+0.5f);
				*(p++) = (byte) ((sin(a1+v    )*0.5f+0.5f)*(float)i);
				*(p++) = (byte) ((sin(a2+v*2.f)*0.5f+0.5f)*(float)i);
				*(p++) = (byte) ((sin(a3+v*3.f)*0.5f+0.5f)*(float)i);
				a1+=d1;
				p++;
				a2+=d2;
				a3+=d3;
				a4+=d4;
			}
			pVar->a1+=pVar->d1;
			pVar->a2+=pVar->d2;
			pVar->a3+=pVar->d3;
			pVar->a4+=pVar->d4;
			pVar->a5+=pVar->d5;
			pVar->a6+=pVar->d6;
		}
		break; 

		case 7:
		calc_samps(pVar->samps, pParam->sample);
		{
			int		i;
			int		*d1=pVar->smooth;
			int		*d2=pVar->smooth+256;
			int		*d3=pVar->smooth+512;
			int		*s1=pVar->samps;
			int		*s2=pVar->samps+256;
			int		*s3=pVar->samps+512;
			for(i=0; i<256; i++)
			{
				*(d1)=((*d1)*3+*(s1++))>>2;
				d1++;
				*(d2)=((*d2)*3+*(s2++))>>2;
				d2++;
				*(d3)=((*d3)*3+*(s3++))>>2;
				d3++;
			}
		}
		{
			int		i;
			byte	*s=pParam->palette+253*4;
			byte	*d=pParam->palette+255*4;
			for(i=0; i<254; i++)
			{
				*(d+0)=*(s+0);
				*(d+1)=*(s+1);
				*(d+2)=*(s+2);
				d-=4;
				s-=4;
			}
		}
		{
			int		i;
			byte	*p=pParam->palette;
			int		*s1=pVar->smooth;
			int		*s2=pVar->smooth+256;
			int		*s3=pVar->smooth+512;
			for(i=0; i<2; i++)
			{
				*(p++)=*(s3++);
				*(p++)=*(s2++);
				*(p++)=*(s1++);
				p++;
			}
		}
		break; 

		case 8:
		calc_samps(pVar->samps, pParam->sample);
		{
			int		i;
			int		*d1=pVar->smooth;
			int		*d2=pVar->smooth+256;
			int		*d3=pVar->smooth+512;
			int		*s1=pVar->samps;
			int		*s2=pVar->samps+256;
			int		*s3=pVar->samps+512;
			for(i=0; i<256; i++)
			{
				*(d1)=((*d1)*3+*(s1++))>>2;
				d1++;
				*(d2)=((*d2)*3+*(s2++))>>2;
				d2++;
				*(d3)=((*d3)*3+*(s3++))>>2;
				d3++;
			}
		}
		{
			int		i;
			byte	*s=pParam->palette+245*4;
			byte	*d=pParam->palette+255*4;
			for(i=0; i<245; i++)
			{
				*(d+0)=*(s+0);
				*(d+1)=*(s+1);
				*(d+2)=*(s+2);
				d-=4;
				s-=4;
			}
		}
		{
			int		i;
			byte	*p=pParam->palette;
			int		*s1=pVar->smooth;
			int		*s2=pVar->smooth+256;
			int		*s3=pVar->smooth+512;
			for(i=0; i<11; i++)
			{
				*(p++)=*(s3++);
				*(p++)=*(s2++);
				*(p++)=*(s1++);
				p++;
			}
		}
		break; 


		default:
		memcpy(pParam->palette, image+320*200, 256*4);
		break;
	}
}

static void calcul_deform(TVar *pVar, TParam *pParam)
{
	int		i;
	byte	*mybuf=pVar->deform;
	byte	*mybuf2=pVar->deform+511;
	byte	*mybufa=pVar->deform+512;
	byte	*touche=&pParam->touche[0];
	byte	*spectre=&pParam->spectre[0];
	int		decal1=pVar->decal1;
	int		decal2=pVar->decal2;
	int		decal3=pVar->decal3;
	int		decal4=pVar->decal4;
	byte	*lsample=pVar->lsample;
	switch(pParam->choix)
	{
		case 1:
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = 0; // rayon/angle

			*(mybufa++) = 0; // angle/rayon
			*(mybufa++) = decal2+sinus[(i*2+decal3)&255]+sinus[(i+decal4)&255]; // rayon/rayon
        }
        break;

		case 2:
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = (sinus[(i*4+decal2*2)&255]/16)+touche[0]*spectre[0]/8; // rayon/angle

			*(mybufa++) = decal1+sinus[decal2&255]/16 ;  // angle/rayon
			*(mybufa++) = -(i*i/256)/4 ; // rayon/rayon
        }
        break;

		case 3:
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = 0; // rayon/angle

			*(mybufa++) = ( sinus[decal2&255]*(255-i)/256) ;  // angle/rayon
			*(mybufa++) = (touche[0]*(i)*spectre[0]/64) ; // rayon/rayon
        }
        break;

		case 4:
		for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = 0; // rayon/angle

			*(mybufa++) = decal1+sinus[(i*2+decal3)&255]/18; // angle/rayon
			*(mybufa++) = decal2+sinus[(i+decal4)&255]/10; // rayon/rayon
        }
        break;

		case 5:
		for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = (sinus[(i*4+decal2*2)&255]/16); //+spectre[i]; // angle/angle
			*(mybuf++)  = 0; // rayon/angle

			*(mybufa++) = decal1+sinus[(i*2+decal3)&255]/8; // angle/rayon
			*(mybufa++) = decal2+sinus[(i+decal4)&255]; // rayon/rayon
        }
        break;

		case 6:
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = 0; // rayon/angle

			*(mybufa++) = decal3+sinus[(i*2+decal3)&255]/18; // angle/rayon
			*(mybufa++) = -decal2+sinus[(i+decal4)&255]/10; // rayon/rayon
        }
        break;

		case 7:
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = -decal1+i; // angle/angle
			*(mybuf++)  = -decal2+i; // rayon/angle

			*(mybufa++) = decal3+i; // angle/rayon
			*(mybufa++) = decal4+i; // rayon/rayon
        }
        break;


		case 8:
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = decal1*touche[1]+sinus[(i+decal2)&255]*touche[4]+(decal1+sinus[(i+decal3)&255]/10)*touche[5]; // angle/angle
			*(mybuf++)  = decal2*touche[2]+sinus[(i+decal3)&255]*touche[7]; // rayon/angle
			*(mybufa++) = decal3*touche[3]+sinus[(i+decal4)&255]*touche[8]; // angle/rayon
			*(mybufa++) = decal4*touche[6]+sinus[(i+decal1)&255]*touche[9]+((lsample[(255-i)]-128)*i/64)*touche[0]; // rayon/rayon
        }
        break;
/*
		case 10:	// old on sound
        for (i=0 ; i<256 ; i++ )
        {
			*(mybuf++)  = 0; // angle/angle
			*(mybuf++)  = 0; // rayon/angle                                                   

			*(mybufa++) = (byte)(decal3*touche[5]+sinus[((lsample[i]-128)+i*2+decal3)&255]*(255-i)/2048*touche[2]); // angle/rayon
			*(mybufa++) = (byte)(8*decal4*touche[3]*(touche[4]-0.5)  +  (lsample[(255-i)]-128)*i/64*touche[1]); // rayon/rayon
        }
        break;
*/
	}
	pVar->decal1-=3;
	pVar->decal2+=5;
	pVar->decal3++;
	pVar->decal4-=2;
}

static void lissage(TVar *pVar, TParam *pParam)
{
	int		i;
	byte	*sample=pParam->sample;
	byte	*lsample=pVar->lsample;
	for(i=4;i<256;i++)
	{
		lsample[i]=((lsample[i]+(sample[i]-lsample[i])/5)+lsample[i-1]+lsample[i-2]+lsample[i-3])/4;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static char *Load(char *filename, void *p, TParam *pParam)
{
	TVar	*pVar=(TVar *)p;
	char	*name=_strdup(filename);
	int		number;
	if(!name)
		return "not enought memory.";
	_strlwr(name);
	{
		int		n=10;
		EnterCriticalSection(&critic);
		while((n>=10)&&(n<30))
		{
			switch(pVar->ChoixInfo[pVar->nChoixInfo].etat)
			{
				case E_LOADED:
				if(n>19)
					n=E_LOADED;
				else
				{
					pVar->nChoixInfo=(pVar->nChoixInfo+1)%10;
					n++;
				}
				break;

				case E_NOTHING:
				n=E_NOTHING;
				break;

				case E_BLOCKED:
				case E_LOAD:
				pVar->nChoixInfo=(pVar->nChoixInfo+1)%10;

				default:
				n++;
				break;

			}
		}
		switch(n)
		{
			case E_LOADED:
			pVar->ChoixInfo[pVar->nChoixInfo].etat=E_BLOCKED;
			if(pVar->ChoixInfo[pVar->nChoixInfo].images)
			{
				free(pVar->ChoixInfo[pVar->nChoixInfo].images);
				pVar->ChoixInfo[pVar->nChoixInfo].nbimages=0;
				pVar->ChoixInfo[pVar->nChoixInfo].images=NULL;
			}
			break;

			case E_NOTHING:
			pVar->ChoixInfo[pVar->nChoixInfo].etat=E_BLOCKED;
			break;

			default:
			free(name);
			LeaveCriticalSection(&critic);
			return "not enought spaces, for loading.";
		}
		number=pVar->nChoixInfo;
		LeaveCriticalSection(&critic);
	}
	if(strstr(name, ".bmp"))
	{
		TChoixInfo	*pCI=&pVar->ChoixInfo[number];
		EnterCriticalSection(&critic);
		pCI->images=loadbmp(name);
		if(pCI->images)
		{
			pCI->nbimages=1;
			pCI->etat=E_LOADED;
		}
		else
		{
			pCI->etat=E_NOTHING;
			free(name);
			LeaveCriticalSection(&critic);
			return "load only 'bmp' 320x200 256 colors.";
		}
		free(name);
		LeaveCriticalSection(&critic);
		return NULL;
	}
	else if(strstr(name, ".fli"))
	{
		TVarThread		*pVarThread=(TVarThread *)malloc(sizeof(TVarThread));
		dword			id;
		if(pVarThread)
		{
			pVarThread->pVar=pVar;
			pVarThread->pParam=pParam;
			strcpy(pVarThread->name, name);
			pVarThread->nChoixInfo=number;
			if(strstr(name, "!"))
				pVar->ChoixInfo[number].pingpong=TRUE;
			else
				pVar->ChoixInfo[number].pingpong=FALSE;
			pVar->ChoixInfo[number].hThread=CreateThread(NULL, 4096, loadfli, pVarThread, 0, &id);
			if(pVar->ChoixInfo[number].hThread)
			{
				dword ecode;
				GetExitCodeThread(pVar->ChoixInfo[number].hThread, &ecode);
				if(ecode!=STILL_ACTIVE)
				{
					pVar->ChoixInfo[number].hThread=NULL;
				}
			}
		}
		free(name);
		return NULL;
	}
	else if(strstr(name, ".avi"))
	{
		TVarThread		*pVarThread=(TVarThread *)malloc(sizeof(TVarThread));
		dword			id;
		if(pVarThread)
		{
			pVarThread->pVar=pVar;
			pVarThread->pParam=pParam;
			strcpy(pVarThread->name, name);
			pVarThread->nChoixInfo=number;
			if(strstr(name, "!"))
				pVar->ChoixInfo[number].pingpong=TRUE;
			else
				pVar->ChoixInfo[number].pingpong=FALSE;
			pVar->ChoixInfo[number].hThread=CreateThread(NULL, 4096, loadavi, pVarThread, 0, &id);
			if(pVar->ChoixInfo[number].hThread)
			{
				dword ecode;
				GetExitCodeThread(pVar->ChoixInfo[number].hThread, &ecode);
				if(ecode!=STILL_ACTIVE)
					pVar->ChoixInfo[number].hThread=NULL;
			}
		}
		free(name);
		return NULL;
	}
	free(name);
	return "load only 'bmp','fli' and 'avi' files.";
}

static int Action(void *p, TParam *pParam)
{
	TVar	*pVar=(TVar *)p;
	EnterCriticalSection(&critic);
	if(pVar->oldchoix4!=pParam->choix4)
	{
		if(pVar->oldchoix4>0)
		{
			TChoixInfo	*pCI=&pVar->ChoixInfo[pVar->oldchoix4-1];
			pCI->dmoux=pParam->moux[0]-pCI->moux;
			pCI->dmouy=pParam->mouy[0]-pCI->mouy;
		}
		{
			TChoixInfo	*pCI=&pVar->ChoixInfo[pParam->choix4-1];
			pCI->moux=pParam->moux[0]-pCI->dmoux;
			pCI->mouy=pParam->mouy[0]-pCI->dmouy;
		}
		pVar->oldchoix4=pParam->choix4;
	}
	{
		TChoixInfo	*pCI=&pVar->ChoixInfo[pParam->choix4-1];
		byte		*image=defbmp;
		if(pCI->etat==E_LOADED)
		{
			if(pCI->nbimages==1)
				image=pCI->images;
			else if(pCI->nbimages>1)
			{
				int	n;
				pCI->currentimage=(pCI->currentimage+((pParam->mouy[0]-pCI->mouy)<<8));
				if(pCI->pingpong)
				{
					n=((pCI->currentimage+((pParam->moux[0]-pCI->moux)<<12))>>16)%(pCI->nbimages*2);
					if(n<0)
						n+=pCI->nbimages*2;
					if(n>=pCI->nbimages)
						n=pCI->nbimages*2-n-1;
				}
				else
				{
					n=((pCI->currentimage+((pParam->moux[0]-pCI->moux)<<12))>>16)%pCI->nbimages;
					if(n<0)
						n+=pCI->nbimages;
				}
				image=pCI->images+(n*(320*200+256*4));
			}
		}
		pVar->angle-=10;
		lissage(pVar, pParam);
		switch(pParam->choix2)
		{
			case 1: 
			memcpy(pParam->ecran, image , 64000);
			break;

			case 2: 
			calcul_deform(pVar, pParam);
			affiche(pParam->ecran, image, pVar);
			break;

			case 3:
			rotozoom(pParam->ecran, image, pVar, pParam);
			break;

			case 4: 
			{
				byte	im2[64000];
				calcul_deform(pVar, pParam);
				affiche(im2, image, pVar);
				rotozoom(pParam->ecran, im2, pVar, pParam);
			}
			break;
		}
		calc_pal(image, pVar, pParam);
	}
	LeaveCriticalSection(&critic);
	return AEE_NOERROR;
}
	
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define FRAND	((float)rand()/(float)RAND_MAX)

static void *Init()
{
	TVar	*pVar=(TVar *)malloc(sizeof(TVar));
	if(pVar)
		memset(pVar, 0, sizeof(TVar));
	pVar->a1=FRAND*10.f;
	pVar->a2=FRAND*10.f;
	pVar->a3=FRAND*10.f;
	pVar->a4=FRAND*10.f;
	pVar->a5=FRAND*10.f;
	pVar->a6=FRAND*10.f;
	pVar->a7=FRAND*10.f;
	pVar->a8=FRAND*10.f;
	pVar->d1=FRAND*0.01f;
	pVar->d2=FRAND*0.01f;
	pVar->d3=FRAND*0.01f;
	pVar->d4=FRAND*0.01f;
	pVar->d5=FRAND*0.01f;
	pVar->d6=FRAND*0.01f;
	pVar->d7=FRAND*0.01f;
	pVar->d8=FRAND*0.01f;
	return (void *)pVar;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void Free(void *p)
{
	TVar	*pVar=(TVar *)p;
	int		i;
	EnterCriticalSection(&critic);
	for(i=0; i<10; i++)
	{
		TChoixInfo	*pCI=&pVar->ChoixInfo[i];
		switch(pCI->etat)
		{
			case E_LOADED:
			case E_LOAD:
			if(pCI->hThread)
			{
				if(!TerminateThread(pCI->hThread, 0))
				{
					Sleep(100);
					TerminateThread(pCI->hThread, 0);
				}
				pCI->hThread=NULL;
				Sleep(100);
			}
			if(pCI->Buffer)
			{
				free(pCI->Buffer);
				pCI->Buffer=NULL;
			}
			if(pCI->Fli_File)
			{
				fclose(pCI->Fli_File);
				pCI->Fli_File=NULL;
			}
			if(pCI->pgetframe)   
			{
				AVIStreamGetFrameClose(pCI->pgetframe);
				pCI->pgetframe=NULL;
			}
			if(pCI->pavi)
			{
				AVIStreamRelease(pCI->pavi);
				pCI->pavi=NULL;
			}
			if(pCI->pfile)
			{
				AVIFileRelease(pCI->pfile);
				pCI->pfile=NULL;
			}

			case E_NOTHING:
			case E_BLOCKED:
			if(pCI->images)
			{
				free(pCI->images);
				pCI->images=NULL;
			}
			pCI->nbimages=0;
			pCI->etat=E_NOTHING;
			break;
		}
	}
	LeaveCriticalSection(&critic);
	if(p)
		free(p);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static void Quit()
{
	if(defbmp)
	{
		free(defbmp);
		defbmp=NULL;
	}
	AVIFileExit();
	DeleteCriticalSection(&critic);
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

static TInfo	info=
{
	AET_INFO,
	AET_ECRAN_OLDP,
	MAKEINTRESOURCE(IDB_PLAYER),
	"Player",
	"player",
	"player properties:"RET\
	"why ???",
	"code by renan",
    "an aestesis product",
    "aestesis'98",
    "renan@aestesis.org",
    "http://www.aestesis.org/",
	Load,
	Action,
	Init,
	Free,
	Quit,
	8,
	4,
	8,
	10,
	{TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE},
	{TRUE, TRUE, TRUE, TRUE},
};

static TTooltips	tooltips=
{
/* choix11		*/	"deform: circular wave",
/* choix12		*/	"deform: left squizy rotate",
/* choix13		*/	"deform: pendular rotate",
/* choix14		*/	"deform: to",
/* choix15		*/	"deform: from",
/* choix16		*/	"deform: squizy from",
/* choix17		*/	"deform: spiral",
/* choix18		*/	"deform: active toggle (C18)",
/* choix19		*/	NULL,
/* choix10		*/	NULL,

/* choix21		*/	"mode: normal",
/* choix22		*/	"mode: deform active",
/* choix23		*/	"mode: rotozoom active",
/* choix24		*/	"mode: deform rotozoom active",
/* choix25		*/	NULL,
/* choix26		*/	NULL,
/* choix27		*/	NULL,
/* choix28		*/	NULL,
/* choix29		*/	NULL,
/* choix20		*/	NULL,

/* choix31		*/	"color: normal",
/* choix32		*/	"color: smooth on sound (respect original)",
/* choix33		*/	"color: on sound (respect original)",
/* choix34		*/	"color: smooth on sound",
/* choix35		*/	"color: on sound",
/* choix36		*/	"color: cycling",
/* choix37		*/	NULL,
/* choix38		*/	NULL,
/* choix39		*/	NULL,
/* choix30		*/	NULL,

/* choix41		*/	"bank: 1",
/* choix42		*/	"bank: 2",
/* choix43		*/	"bank: 3",
/* choix44		*/	"bank: 4",
/* choix45		*/	"bank: 5",
/* choix46		*/	"bank: 6",
/* choix47		*/	"bank: 7",
/* choix48		*/	"bank: 8",
/* choix49		*/	"bank: 9",
/* choix40		*/	"bank: 10",

/* vb1			*/	"fli: up/down speed, right/left scratch",
/* vb2			*/	"roto: sinus amplitude position",
/* vb3			*/	"roto: crazy",
/* vb4			*/	"roto: zoom and speed rotation",

/* t0			*/	"toggle: sound punch    (activated by C18)",
/* t1			*/	"toggle: left rotate    (activated by C18)",
/* t2			*/	"toggle: to             (activated by C18)",
/* t3			*/	"toggle: right rotate   (activated by C18)",
/* t4			*/	"toggle: super squizy   (activated by C18)",
/* t5			*/	"toggle: deform rotate  (activated by C18)",
/* t6			*/	"toggle: from           (activated by C18)",
/* t7			*/	"toggle: deform         (activated by C18)",
/* t8			*/	"toggle: twirl          (activated by C18)",
/* t9			*/	"toggle: punch          (activated by C18)"
};

int InitializePlayer(TInfo **ppInfo, TTooltips **ppTooltips)
{
	// set info
	*ppInfo=&info;
	*ppTooltips=&tooltips;
	
	// init global
	calc_sinus();
	calc_table();
	InitializeCriticalSection(&critic);
	defbmp=loadbmp(MAKEINTRESOURCE(IDB_DEFPLAYER));
	AVIFileInit();
	return AEE_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
