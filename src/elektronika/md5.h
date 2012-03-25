/***
    Implementation de MD5
    Fabien Pollet
***/

#ifndef MD5_H
#define MD5_H

typedef struct
{
  unsigned long A;
  unsigned long B;
  unsigned long C;
  unsigned long D;
} md5_regs;

typedef unsigned long md5_size;

// Calcul du digest MD5
unsigned char *md5(unsigned char *, md5_size, unsigned char *);

// Padding
void md5_padding(unsigned char *, md5_size*);

// Calcul de la taille en octets apres padding
md5_size md5_needed(unsigned char *, md5_size);

// Ajout de la taille
void md5_addsize(unsigned char *, md5_size *, md5_size);

// Rondes 1, 2, 3 et 4
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (~z & y))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | ~z))

#define rotate(w,s) ((w << s) | (w >> (32 - s)))

#define FF(a,b,c,d,x,s,t) (a = b + rotate((a + F(b,c,d) + x + t), s))
#define GG(a,b,c,d,x,s,t) (a = b + rotate((a + G(b,c,d) + x + t), s))
#define HH(a,b,c,d,x,s,t) (a = b + rotate((a + H(b,c,d) + x + t), s))
#define II(a,b,c,d,x,s,t) (a = b + rotate((a + I(b,c,d) + x + t), s))

void md5_calc(md5_regs *, md5_size, unsigned char *);

// Resultat
unsigned char *md5_result(md5_regs, unsigned char *);

int getMD5(char *mds, char *src);


#endif
