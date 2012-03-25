#ifndef _MATRICE_H_
#define _MATRICE_H_

#ifndef PI
#define PI	3.14159265359
#endif



struct RMATRICE
{
	float	_11,_12,_13,_14;
	float	_21,_22,_23,_24;
	float	_31,_32,_33,_34;
	float	_41,_42,_43,_44;
};

struct RMATRICE2
{
	float	v[4][4];
};

extern struct RMATRICE matrice_identity;

struct RMATRICE matrice_multiply(struct RMATRICE m1, struct RMATRICE m2);
void	matrice_comatrice		(struct RMATRICE *m1);
void	matrice_transpose		(struct RMATRICE *m1);
float	matrice_det				(struct RMATRICE2 *m1, int niveau);
void	matrice_multiply		(struct RMATRICE *m1, float n);
void	matrice_inverse			(struct RMATRICE *m1);
void	matrice_setscale		(struct RMATRICE *matrice, float x, float y, float z, float ax, float ay, float az);
void	matrice_setrotation		(struct RMATRICE *matrice, float ax, float ay, float az);
void	matrice_settranslation	(struct RMATRICE *matrice, float x, float y, float z);
void	matrice_gettranslation	(struct RMATRICE matrice, float *x, float *y, float *z);
void	matrice_getrotation		(struct RMATRICE matrice, float *x, float *y, float *z);
int		matrice_getrotationwc	(struct RMATRICE mat, float *x, float *y, float *z);

#define matrice_determinant(x) matrice_det((struct RMATRICE2 *)x, 4)

#endif