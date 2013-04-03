/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SAMPLE.CPP					(c)	YoY'00						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						<math.h>
#include						"sample.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						Asample::CI=ACI("Asample", GUID(0xE4EC7600,0x00012010), &Apin::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Asample::Asample(QIID qiid, char *name, AeffectBack *eb, int type, int x, int y) : Apin(qiid, name, eb, type, x, y, 8, 8)
{
	bitmap->boxf(0, 0, 7, 7, 0xffc08000);
	memset(bas, 0, sizeof(bas));
	memset(med, 0, sizeof(med));
	ibas=0;
	imed=0;
	if((type&pinDIR)==pinOUT)
	{
		size=44100;
		sample=(sword *)malloc(size*2*sizeof(sword));
		bass=(sword *)malloc(size*2*sizeof(sword));
		medium=(sword *)malloc(size*2*sizeof(sword));
		treble=(sword *)malloc(size*2*sizeof(sword));
		memset(sample, 0, size*2*sizeof(sword));
		memset(bass, 0, size*2*sizeof(sword));
		memset(medium, 0, size*2*sizeof(sword));
		memset(treble, 0, size*2*sizeof(sword));
		bitmap->box(0, 0, 7, 7, 0xff000000);
	}
	else
	{
		size=0;
		sample=NULL;
		bass=NULL;
		medium=NULL;
		treble=NULL;
		bitmap->box(0, 0, 7, 7, 0xffffffff);
	}
	if(type&pinNOGFX)
	{
		bitmap->boxf(0, 0, bitmap->w, bitmap->h, 0x00000000);
		bitmap->flags=bitmapALPHA;
	}
	offset=0;
	time=0;
	vmaster=1.f;
	vbass=1.f;
	vmedium=1.f;
	vtreeble=1.f;
	sbl=sbr=sml=smr=0;
	mbas=0;
	mmed=0;
	mtre=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL Asample::~Asample()
{
	if(sample)
		free(sample);
	if(bass)
		free(bass);
	if(medium)
		free(medium);
	if(treble)
		free(treble);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asample::addConnect(Apin *pin)
{
	if((type&pinDIR)==pinIN)
	{
		Asample	*out=(Asample *)pin;
		out->enter(__FILE__,__LINE__);
		offset=out->offset;
		out->leave();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL bool Asample::connected(Aconnect *c)
{
	Asample	*out=(Asample *)getConnectedPin();
	if(out)
	{
		out->enter(__FILE__,__LINE__);
		offset=out->offset;
		out->leave();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Asample::calcFrequency(int offdep, int offend)
{
	int		i;
	int		mbass=(int)(vmaster*vbass*255.99f)<<8;
	int		mmedium=(int)(vmaster*vmedium*255.99f)<<8;
	int		mtreeble=(int)(vmaster*vtreeble*255.99f)<<8;
	int		ob=offdep-32-32;
	int		om=offdep-32-4;
	int		ot=offdep-32;
	int		obs=offdep;
	int		oms=offdep+4-32;
	int		obe,ome;
	int		otf;
	int		mR=0;
	int		mL=0;

	if(ob<0)
		ob+=size;
	if(om<0)
		om+=size;
	if(ot<0)
		ot+=size;
	if(oms<0)
		oms+=size;
		
	otf=ot;
	obe=ob;
	ome=om;

	for(i=offdep; i<offend; i++)
	{
		int	ib=ibas<<1;
		int	im=imed<<1;

		sbl+=(int)sample[obs<<1]-(int)bas[ib];
		bas[ib]=(int)sample[obs<<1];
		sbr+=(int)sample[(obs<<1)+1]-(int)bas[ib+1];
		bas[ib+1]=(int)sample[(obs<<1)+1];
		ibas=(ibas+1)&63;

		sml+=(int)sample[oms<<1]-(int)med[im];
		med[im]=sample[oms<<1];
		smr+=(int)sample[(oms<<1)+1]-(int)med[im+1];
		med[im+1]=sample[(oms<<1)+1];
		imed=(imed+1)&7;

		int	bl=sbl>>6;
		int	br=sbr>>6;
		int	ml=(sml>>3)-bl;
		int	mr=(smr>>3)-br;

		int	mb=mbas>>8;
		int	mm=mmed>>8;
		int	mt=mtre>>8;

		int	n0=ot<<1;
		int	n1=n0+1;

		bass[n0]=maxi(mini((mb*bl)>>8, 32767), -32767);
		bass[n1]=maxi(mini((mb*br)>>8, 32767), -32767);
		medium[n0]=maxi(mini((mm*ml)>>8, 32767), -32767);
		medium[n1]=maxi(mini((mm*mr)>>8, 32767), -32767);
		treble[n0]=maxi(mini((mt*((int)sample[n0]-(ml+bl)))>>8, 32767), -32767);
		treble[n1]=maxi(mini((mt*((int)sample[n1]-(mr+br)))>>8, 32767), -32767);

		obs=(obs+1)%size;
		oms=(oms+1)%size;
		obe=(obe+1)%size;
		ome=(ome+1)%size;

		mbas=((mbas*15)+mbass)>>4;
		mmed=((mmed*15)+mmedium)>>4;
		mtre=((mtre*15)+mtreeble)>>4;
		ot=(ot+1)%size;
	}
	for(i=offdep; i<offend; i++)
	{
		int	n0=otf<<1;
		int	n1=n0+1;
		int	v0=(int)bass[n0]+(int)medium[n0]+(int)treble[n0];
		int	v1=(int)bass[n1]+(int)medium[n1]+(int)treble[n1];
		sample[n0]=maxi(mini(v0, 32767), -32767);
		sample[n1]=maxi(mini(v1, 32767), -32767);
		mL=maxi(mL, abs(v0));
		mR=maxi(mR, abs(v1));
		otf=(otf+1)%size;
	}

	maxL=(float)mL/32767.f;
	maxR=(float)mR/32767.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void Asample::addBuffer(sword *buffer, int size)
{
	int	len=this->size-offset;
	int lastoffset=offset;

	size=mini(size, this->size);

	if(size<0)
		return;

	if(len>size)
	{
		memcpy(&sample[offset<<1], buffer, size<<2);
		offset+=size;
		calcFrequency(lastoffset, offset);
	}
	else
	{
		int	lr=size-len;
		memcpy(&sample[offset<<1], buffer, len<<2);
		memcpy(&sample[0], buffer+(len<<1), lr<<2);
		offset=lr;
		calcFrequency(lastoffset, lastoffset+len);
		calcFrequency(0, lr);
	}
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL int Asample::getBuffer(sword *buffer, int bufferSize, int dtime)
{
	if((type&pinDIR)==pinIN)
	{
		Asample	*out=(Asample *)getConnectedPin();
		if(out)
		{
			int	s=0;
			out->enter(__FILE__,__LINE__);
			{
				int	d=((offset<=out->offset)?(out->offset-offset):(out->offset+out->size-offset));
				s=mini(bufferSize, d);
				if(dtime>=0)
					s=mini(s, dtime*out->size/1000);
				if((offset+s)<=out->size)
				{
					memcpy(buffer, &out->sample[offset<<1], s<<2);
				}
				else
				{
					int	n=out->size-offset;
					int	n1=s-n;
					memcpy(buffer, &out->sample[offset<<1], n<<2);
					memcpy(&buffer[n<<1], &out->sample[0], n1<<2);
				}
				offset+=s;
				if(offset>out->size)
					offset-=out->size;
			}
			out->leave();
			return s;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL int Asample::getBuffer(sword *buffer, sword *bass, sword *medium, sword *treble, int bufferSize, int dtime)
{
	if((type&pinDIR)==pinIN)
	{
		Asample	*out=(Asample *)getConnectedPin();
		if(out)
		{
			int	s=0;
			out->enter(__FILE__,__LINE__);
			{
				int	d=((offset<=out->offset)?(out->offset-offset):(out->offset+out->size-offset))-32;
				s=maxi(mini(bufferSize, d), 0);
				if(dtime>=0)
					s=mini(s, dtime*out->size/1000);
				if((offset+s)<=out->size)
				{
					if(buffer)
						memcpy(buffer, &out->sample[offset<<1], s<<2);
					if(bass)
						memcpy(bass, &out->bass[offset<<1], s<<2);
					if(medium)
						memcpy(medium, &out->medium[offset<<1], s<<2);
					if(treble)
						memcpy(treble, &out->treble[offset<<1], s<<2);
				}
				else
				{
					int	n=out->size-offset;
					int	n1=s-n;
			
					if(buffer)
					{
						memcpy(buffer, &out->sample[offset<<1], n<<2);
						memcpy(&buffer[n<<1], &out->sample[0], n1<<2);
					}
					if(bass)
					{
						memcpy(bass, &out->bass[offset<<1], n<<2);
						memcpy(&bass[n<<1], &out->bass[0], n1<<2);
					}
					if(medium)
					{
						memcpy(medium, &out->medium[offset<<1], n<<2);
						memcpy(&medium[n<<1], &out->medium[0], n1<<2);
					}
					if(treble)
					{
						memcpy(treble, &out->treble[offset<<1], n<<2);
						memcpy(&treble[n<<1], &out->treble[0], n1<<2);
					}
				}
				offset+=s;
				if(offset>out->size)
					offset-=out->size;
			}
			out->leave();
			return s;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL int Asample::avaible()
{
	int		d=0;
	if((type&pinDIR)==pinIN)
	{
		Asample	*out=(Asample *)getConnectedPin();
		if(out)
		{
			out->enter(__FILE__,__LINE__);
			d=((offset<=out->offset)?(out->offset-offset):(out->offset+out->size-offset))-32;
			if(d<0)
				d=0;
			out->leave();
		}
	}
	else
	{
	}
	return d;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL int Asample::skip(int reloffset)
{
	Asample	*out=(Asample *)getConnectedPin();
	int		skipped=0;
	if(out)
	{
		out->enter(__FILE__,__LINE__);
		{
			int	d=(offset<=out->offset)?(out->offset-offset):(out->offset+out->size-offset);
			if(reloffset>=0)
			{
				if(reloffset>d)
				{
					offset=out->offset;
					skipped=d;
				}
				else
				{
					offset+=reloffset;
					if(offset>out->size)
						offset=offset-out->size;
					skipped=reloffset;
				}
			}
		}
		out->leave();
	}
	return skipped; 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   This computes an in-place complex-to-complex FFT 
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform 
*/

static void FFT(int dir, long  m, double *x, double *y)
{
	long	n,i,i1,j,k,i2,l,l1,l2;
	double	c1,c2,tx,ty,t1,t2,u1,u2,z;

	// Calculate the number of points 
	n = 1;
	for (i=0;i<m;i++) 
		n *= 2;
	
	// Do the bit reversal
	i2 = n >> 1;
	j = 0;
	for (i=0;i<n-1;i++) 
	{
		if (i < j) 
		{
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = tx;
			y[j] = ty;
		}
		k = i2;
		while (k <= j) 
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	// Compute the FFT
	c1 = -1.0; 
	c2 = 0.0;
	l2 = 1;
	for(l=0; l<m; l++) 
	{
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0; 
		u2 = 0.0;
		for (j=0;j<l1;j++) 
		{
			for (i=j;i<n;i+=l2) 
			{
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = x[i] - t1; 
				y[i1] = y[i] - t2;
				x[i] += t1;
				y[i] += t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0 - c1) / 2.0);
		if (dir == 1) 
			c2 = -c2;
		c1 = sqrt((1.0 + c1) / 2.0);
	}

/*
	// Scaling for forward transform
	if (dir == 1) 
	{
		double	nn=1.0/(double)n;	
		for (i=0; i<n; i++) 
		{
			x[i] *= nn;
			y[i] *= nn;
		}
	}
*/
}


void fft_1d(double array[2][1024], int n)
{
	double  u_r,u_i, w_r,w_i, t_r,t_i;
	int     ln, nv2, k, l, le, le1, j, ip, i;

	ln = (int)( log( (double)n )/log(2.0) + 0.5 );
	nv2 = n / 2;
	j = 1;
	
 	for (i = 1; i < n; i++ )
	{
		if (i < j)
		{
	    	t_r = array[0][i - 1];
	    	t_i = array[1][i - 1];
	    	array[0][i - 1] = array[0][j - 1];
	    	array[1][i - 1] = array[1][j - 1];
	    	array[0][j - 1] = t_r;
	    	array[1][j - 1] = t_i;
		}
		k = nv2;
		while (k < j)
		{
			j = j - k;
			k = k / 2;
		}
		j = j + k;
	}

 	for (l = 1; l <= ln; l++) /* loops thru stages */
	{	
		le = (int)(exp( (double)l * log(2.0) ) + 0.5 );
	  	le1 = le / 2;
		u_r = 1.0;
		u_i = 0.0;
		w_r =  cos( PI / (double)le1 );
		w_i = -sin( PI / (double)le1 );
		for (j = 1; j <= le1; j++) /* loops thru 1/2 twiddle values per stage */
		{
			for (i = j; i <= n; i += le) /* loops thru points per 1/2 twiddle */
			{
				ip = i + le1;
				t_r = array[0][ip - 1] * u_r - u_i * array[1][ip - 1];
				t_i = array[1][ip - 1] * u_r + u_i * array[0][ip - 1];

				array[0][ip - 1] = array[0][i - 1] - t_r;
				array[1][ip - 1] = array[1][i - 1] - t_i; 

				array[0][i - 1] =  array[0][i - 1] + t_r;
				array[1][i - 1] =  array[1][i - 1] + t_i;  
	    	} 
	    	t_r = u_r * w_r - w_i * u_i;
	    	u_i = w_r * u_i + w_i * u_r;
	    	u_r = t_r;
		} 
	}  
} /* end of FFT_1d */



#define M_PI 3.14159265358979323846

typedef struct complex			    /* complex pair */
{
    float           r;			    /* real part */
    float           i;			    /* imaginary part */
}               CMPLX;

void fft_c (CMPLX vec[], int ln)
{
	CMPLX           u, w, t;
	int             n, nv2, k, l, le, le1, j, ip, i;

    n = (int) pow (2.0, (double)ln);
    nv2 = n / 2;
    j = 1;
    
    for (i = 1; i < n; i++ ) /* reorders the input vector [bit reversal] */
    {
		if (i < j)
		{
			t.r = vec[i - 1].r;
			t.i = vec[i - 1].i;
			vec[i - 1].r = vec[j - 1].r;
			vec[i - 1].i = vec[j - 1].i;
			vec[j - 1].r = t.r;
			vec[j - 1].i = t.i;
		}
		k = nv2;
		while (k < j)
		{
			j = j - k;
			k = k / 2;
		}
		j = j + k;
    }

    for (l = 1; l <= ln; l++)
    {
	le = (int) pow (2.0, (double)l);
	le1 = le / 2;
	u.r = 1.0e0;
	u.i = 0.0e0;
	w.r = (float)( cos ((double) (M_PI / le1)) );
	w.i = (float)(-sin ((double) (M_PI / le1)) );
	for (j = 1; j <= le1; j++)
	{
	    for (i = j; i <= n; i = i + le)
	    {
		ip = i + le1;

/* t = vec[ip - 1] * u */
		t.r = vec[ip - 1].r * u.r - u.i * vec[ip - 1].i;
		t.i = u.r * vec[ip - 1].i + u.i * vec[ip - 1].r;
		vec[ip - 1].r = vec[i - 1].r - t.r;
		vec[ip - 1].i = vec[i - 1].i - t.i;
		vec[i - 1].r = t.r + vec[i - 1].r;
		vec[i - 1].i = t.i + vec[i - 1].i;
	    };
/* u = u * w */
	    t.r = u.r * w.r - w.i * u.i;
	    u.i = w.r * u.i + w.i * u.r;
	    u.r = t.r;
	};
    };
};


void Asample::calcFFT(sword *sample, double *freq)
{
	short	*s=(short *)sample;
	CMPLX	vec[1024];
	int		i;
	double	nd=1.0/32768.0;
	
	for(i=0; i<1024; i++)
	{
		vec[i].r=(float)((((double)*(s++))*nd));
		vec[i].i=0;
	}
	
	fft_c(vec, 10);
	
	for(i=0; i<1024; i++)
		freq[i]=fabs(vec[i].r);
}

/*

void Asample::calcFFT(sword *sample, double *freq)
{
	short	*s=(short *)sample;
	double	x[2][1024];
	int		i;
	double	nd=1.0/32768.0;
	
	for(i=0; i<1024; i++)
	{
		x[0][i]=(((double)*(s++))*nd);
		x[1][i]=0;
	}
	
	fft_1d(x, 1024);
	//FFT(1, 10, x, y);
	
	for(i=0; i<1024; i++)
		freq[i]=fabs(x[0][i]);
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

