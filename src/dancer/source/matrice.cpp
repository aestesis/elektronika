#include <windows.h>
#include <math.h>
#include "matrice.h"

#define car(x) ((x)*(x))


struct RMATRICE matrice_identity={	(float)1, (float)0, (float)0, (float)0  ,
									(float)0, (float)1, (float)0, (float)0  ,
									(float)0, (float)0, (float)1, (float)0  ,
									(float)0, (float)0, (float)0, (float)1	};


struct RMATRICE matrice_multiply(struct RMATRICE m1, struct RMATRICE m2)
{
	struct RMATRICE m;
	m._11 = m1._11*m2._11 + m1._12*m2._21 + m1._13*m2._31 + m1._14*m2._41;
	m._12 = m1._11*m2._12 + m1._12*m2._22 + m1._13*m2._32 + m1._14*m2._42;
	m._13 = m1._11*m2._13 + m1._12*m2._23 + m1._13*m2._33 + m1._14*m2._43;
	m._14 = m1._11*m2._14 + m1._12*m2._24 + m1._13*m2._34 + m1._14*m2._44;
	m._21 = m1._21*m2._11 + m1._22*m2._21 + m1._23*m2._31 + m1._24*m2._41;
	m._22 = m1._21*m2._12 + m1._22*m2._22 + m1._23*m2._32 + m1._24*m2._42;
	m._23 = m1._21*m2._13 + m1._22*m2._23 + m1._23*m2._33 + m1._24*m2._43;
	m._24 = m1._21*m2._14 + m1._22*m2._24 + m1._23*m2._34 + m1._24*m2._44;
	m._31 = m1._31*m2._11 + m1._32*m2._21 + m1._33*m2._31 + m1._34*m2._41;
	m._32 = m1._31*m2._12 + m1._32*m2._22 + m1._33*m2._32 + m1._34*m2._42;
	m._33 = m1._31*m2._13 + m1._32*m2._23 + m1._33*m2._33 + m1._34*m2._43;
	m._34 = m1._31*m2._14 + m1._32*m2._24 + m1._33*m2._34 + m1._34*m2._44;
	m._41 = m1._41*m2._11 + m1._42*m2._21 + m1._43*m2._31 + m1._44*m2._41;
	m._42 = m1._41*m2._12 + m1._42*m2._22 + m1._43*m2._32 + m1._44*m2._42;
	m._43 = m1._41*m2._13 + m1._42*m2._23 + m1._43*m2._33 + m1._44*m2._43;
	m._44 = m1._41*m2._14 + m1._42*m2._24 + m1._43*m2._34 + m1._44*m2._44;
	return m;
}

void matrice_multiply(struct RMATRICE *m1, float n)
{
	m1->_11*=n;
	m1->_12*=n;
	m1->_13*=n;
	m1->_14*=n;
	m1->_21*=n;
	m1->_22*=n;
	m1->_23*=n;
	m1->_24*=n;
	m1->_31*=n;
	m1->_32*=n;
	m1->_33*=n;
	m1->_34*=n;
	m1->_41*=n;
	m1->_42*=n;
	m1->_43*=n;
	m1->_44*=n;
}

void matrice_transpose(struct RMATRICE *m1)
{
	struct RMATRICE m;
	m._11=m1->_11;
	m._12=m1->_21;
	m._13=m1->_31;
	m._14=m1->_41;
	m._21=m1->_12;
	m._22=m1->_22;
	m._23=m1->_32;
	m._24=m1->_42;
	m._31=m1->_13;
	m._32=m1->_23;
	m._33=m1->_33;
	m._34=m1->_43;
	m._41=m1->_14;
	m._42=m1->_24;
	m._43=m1->_34;
	m._44=m1->_44;
	*(m1)=m;
}

float matrice_det(struct RMATRICE2 *m1, int niveau)
{
	if(niveau==2)
	{
		return m1->v[0][0]*m1->v[1][1]-m1->v[1][0]*m1->v[0][1];
	}
	else
	{
		int					i,a,b,c;
		struct RMATRICE2	m;
		float				det=(float)0;

		for(i=0; i<niveau; i++)
		{
			c=0;
			for(a=0; a<niveau; a++)
				if(a!=i)
				{
					for(b=1; b<niveau; b++)
						m.v[b-1][c]=m1->v[b][a];
					c++;
				}
			
			if((i&1)==0)
				det+=m1->v[0][i]*matrice_det(&m, niveau-1);
			else
				det-=m1->v[0][i]*matrice_det(&m, niveau-1);
		}
		return det;
	}
}

void matrice_comatrice(struct RMATRICE *m1)
{
	int					i,j,a,b,c,d;
	struct	RMATRICE2	m,m2,*p1;

	p1=(struct RMATRICE2 *) m1;
	for(j=0; j<4; j++)
	for(i=0; i<4; i++)
	{
		d=0;
		for(b=0; b<4; b++)
			if(b!=j)
			{
				c=0;
				for(a=0; a<4; a++)
					if(a!=i)
					{
						m.v[d][c]=p1->v[b][a];
						c++;
					}
				d++;
			}
		if(((i+j)&1)==0)
			m2.v[j][i]=matrice_det(&m, 3);
		else
			m2.v[j][i]=-matrice_det(&m, 3);
	}
	*(p1)=m2;
}

void matrice_inverse(struct RMATRICE *m1)
{
	struct RMATRICE		m;
	float				n;
	m=*(m1);
	matrice_comatrice(&m);
	matrice_transpose(&m);
	n=1/matrice_determinant(m1);
	matrice_multiply(&m, n);
	*(m1)=m;
}

void matrice_setscale(struct RMATRICE *matrice, float x, float y, float z)
{
	float	d;
	d=(float) sqrt(car((double)matrice->_11)+car((double)matrice->_12)+car((double)matrice->_13));
	d=x/d;
	matrice->_11*=d;
	matrice->_12*=d;
	matrice->_13*=d;
	d=(float) sqrt(car((double)matrice->_21)+car((double)matrice->_22)+car((double)matrice->_23));
	d=y/d;
	matrice->_21*=d;
	matrice->_22*=d;
	matrice->_23*=d;
	d=(float) sqrt(car((double)matrice->_31)+car((double)matrice->_32)+car((double)matrice->_33));
	d=z/d;
	matrice->_31*=d;
	matrice->_32*=d;
	matrice->_33*=d;
}

void matrice_setrotation(struct RMATRICE *matrice, float ax, float ay, float az)
{
	// de type -> rot=rotz.roty.rotx  (format softimage)
	double	cx,cy,cz,sx,sy,sz;
	cx=cos((double)ax);
	cy=cos((double)ay);
	cz=cos((double)az);
	sx=sin((double)ax);
	sy=sin((double)ay);
	sz=sin((double)az);
	matrice->_11=(float)(cy*cz);
	matrice->_12=(float)(cy*sz);
	matrice->_13=(float)(-sy);
	matrice->_21=(float)(sx*sy*cz-sz*cx);
	matrice->_22=(float)(sx*sy*sz+cx*cz);
	matrice->_23=(float)(sx*cy);
	matrice->_31=(float)(sx*sz+cx*cz*sy);
	matrice->_32=(float)(cx*sy*sz-sx*cz);
	matrice->_33=(float)(cx*cy);
}

void matrice_settranslation(struct RMATRICE *matrice, float x, float y, float z)
{
	matrice->_41=x;
	matrice->_42=y;
	matrice->_43=z;
}

void matrice_gettranslation(struct RMATRICE matrice, float *x, float *y, float *z)
{
	*x=matrice._41;
	*y=matrice._42;
	*z=matrice._43;
}

#define egal(x,y)   (fabs((double)(x-y))<(double)0.05)
void matrice_getrotation(struct RMATRICE mat, float *x, float *y, float *z)
{
	struct RMATRICE		m;
	int		i,j,k;

	if(egal(mat._12,0)&&egal(mat._13,0))
	{
		for(i=0; i<2; i++)
		for(j=0; j<2; j++)
		for(k=0; k<2; k++)
		{
			*z=(float)((double)i*(double)PI);
			*y=(float)((double)j*(double)PI);
			*x=(float)(atan(mat._23/ mat._33)+(double)k*(double)PI);
			m=matrice_identity;
			matrice_setrotation(&m, *x, *y, *z);
			if(	egal(m._11, mat._11)&&
				egal(m._12, mat._12)&&
				egal(m._13, mat._13)&&
				egal(m._21, mat._21)&&
				egal(m._22, mat._22)&&
				egal(m._23, mat._23)&&
				egal(m._31, mat._31)&&
				egal(m._32, mat._32)&&
				egal(m._33, mat._33) )
				return;
		}
	}
	else
	{
		if(mat._12==0)
		{
			for(i=0; i<2; i++)
			for(j=0; j<2; j++)
			for(k=0; k<2; k++)
			{
				*z=(float)(atan(mat._12/ mat._11)+(double)i*(double)PI);
				*y=(float)(atan(-mat._13*cos((double)*z)/ mat._11)+(double)j*(double)PI);
				*x=(float)(atan(mat._23/ mat._33)+(double)k*(double)PI);

				m=matrice_identity;
				matrice_setrotation(&m, *x, *y, *z);
				if(	egal(m._11, mat._11)&&
					egal(m._12, mat._12)&&
					egal(m._13, mat._13)&&
					egal(m._21, mat._21)&&
					egal(m._22, mat._22)&&
					egal(m._23, mat._23)&&
					egal(m._31, mat._31)&&
					egal(m._32, mat._32)&&
					egal(m._33, mat._33) )
					return;
			}
		}
		else
		{
			for(i=0; i<2; i++)
			for(j=0; j<2; j++)
			for(k=0; k<2; k++)
			{
				*z=(float)(atan(mat._12/ mat._11)+(double)i*(double)PI);
				*y=(float)(atan(-mat._13*sin((double)*z)/ mat._12)+(double)j*(double)PI);
				*x=(float)(atan(mat._23/ mat._33)+(double)k*(double)PI);

				m=matrice_identity;
				matrice_setrotation(&m, *x, *y, *z);
				if(	egal(m._11, mat._11)&&
					egal(m._12, mat._12)&&
					egal(m._13, mat._13)&&
					egal(m._21, mat._21)&&
					egal(m._22, mat._22)&&
					egal(m._23, mat._23)&&
					egal(m._31, mat._31)&&
					egal(m._32, mat._32)&&
					egal(m._33, mat._33) )
					return;
			}
		}
	}
	MessageBox(NULL, "Error matrice_getrotation, probably use of scale in the animation...", "zen of animation", MB_OK);
}

int matrice_getrotationwc(struct RMATRICE mat, float *x, float *y, float *z)
{
    int                 bbool=0;
    struct RMATRICE     m;
    int                 i,j,k;
    float				ax,ay,az,max=(float)1000000,nn;
    float				rx=(float)0,ry=(float)0,rz=(float)0;
    if(egal(mat._12,0)&&egal(mat._13,0))
    {
        for(i=-4; i<4; i++)
        for(j=-4; j<4; j++)
        for(k=-4; k<4; k++)
        {
            az=(float)((float)i*(float)PI);
            ay=(float)((float)j*(float)PI);
            ax=(float)(atan(mat._23/ mat._33)+(float)k*(float)PI);
            m=matrice_identity;
            matrice_setrotation(&m, ax, ay, az);
            if( egal(m._11, mat._11)&&
                egal(m._12, mat._12)&&
                egal(m._13, mat._13)&&
                egal(m._21, mat._21)&&
                egal(m._22, mat._22)&&
                egal(m._23, mat._23)&&
                egal(m._31, mat._31)&&
                egal(m._32, mat._32)&&
                egal(m._33, mat._33) )
            {
                nn=(float)sqrt((ax-*x)*(ax-*x)+(ay-*y)*(ay-*y)+(az-*z)*(az-*z));
                if(nn<max)
                {
                    max=nn;
                    rx=ax;
                    ry=ay;
                    rz=az;
                    bbool=-1;
                }
            }
        }
    }
    else
    {
		if(mat._12==0)
		{
			for(i=-4; i<4; i++)
			for(j=-4; j<4; j++)
			for(k=-4; k<4; k++)
			{
				az=(float)(atan(mat._12/ mat._11)+(float)i*(float)PI);
				ay=(float)(atan(-mat._13*cos((float)az)/ mat._11)+(float)j*(float)PI);
				ax=(float)(atan(mat._23/ mat._33)+(float)k*(float)PI);
				m=matrice_identity;
				matrice_setrotation(&m, ax, ay, az);
				if( egal(m._11, mat._11)&&
					egal(m._12, mat._12)&&
					egal(m._13, mat._13)&&
					egal(m._21, mat._21)&&
					egal(m._22, mat._22)&&
					egal(m._23, mat._23)&&
					egal(m._31, mat._31)&&
					egal(m._32, mat._32)&&
					egal(m._33, mat._33) )
				{
					nn=(float)sqrt((ax-*x)*(ax-*x)+(ay-*y)*(ay-*y)+(az-*z)*(az-*z));
					if(nn<max)
					{
						max=nn;
						rx=ax;
						ry=ay;
						rz=az;
						bbool=-1;
					}
				}
			}
		}
		else
		{
			for(i=-4; i<4; i++)
			for(j=-4; j<4; j++)
			for(k=-4; k<4; k++)
			{
				az=(float)(atan(mat._12/ mat._11)+(float)i*(float)PI);
				ay=(float)(atan(-mat._13*sin((float)az)/ mat._12)+(float)j*(float)PI);
				ax=(float)(atan(mat._23/ mat._33)+(float)k*(float)PI);
				m=matrice_identity;
				matrice_setrotation(&m, ax, ay, az);
				if( egal(m._11, mat._11)&&
					egal(m._12, mat._12)&&
					egal(m._13, mat._13)&&
					egal(m._21, mat._21)&&
					egal(m._22, mat._22)&&
					egal(m._23, mat._23)&&
					egal(m._31, mat._31)&&
					egal(m._32, mat._32)&&
					egal(m._33, mat._33) )
				{
					nn=(float)sqrt((ax-*x)*(ax-*x)+(ay-*y)*(ay-*y)+(az-*z)*(az-*z));
					if(nn<max)
					{
						max=nn;
						rx=ax;
						ry=ay;
						rz=az;
						bbool=-1;
					}
				}
			}
        }
    }
    *x=rx;
    *y=ry;
    *z=rz;
    return bbool;
}
