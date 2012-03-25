/***
    Implementation de MD5
    Fabien Pollet
***/

#include "md5.h"
#include <stdio.h>
#include <string.h>

unsigned char *md5(unsigned char *buffer, md5_size len, unsigned char *digest)
{
  md5_size oldlen = len;
  md5_regs regs;

  // *** Initialisation
  regs.A = 0x67452301;
  regs.B = 0xefcdab89;
  regs.C = 0x98badcfe;
  regs.D = 0x10325476;

  // *** Padding
  md5_padding(buffer, &len);

  // *** Ajout de la taille
  md5_addsize(buffer, &len, oldlen);

  // *** Calcul du Message Digest
  md5_calc(&regs, len, buffer);

  // *** Resultat
  return md5_result(regs, digest);
}


/* Padding */
void md5_padding(unsigned char *buffer, md5_size *len)
{
  if((((*len)*8) % 512) != 448)
    buffer[(*len)++] = 128;
  while((((*len)*8) % 512) != 448)
    buffer[(*len)++] = 0;
}

/* Calcul de la taille necessaire apres padding et ajout de taille en octets */
md5_size md5_needed(unsigned char *buffer, md5_size len)
{
  while(((len*8) % 512) != 448)
    len ++;        // calcul du padding

  return (len + 8); // ajout de la taille
}

/* Ajout de la taille */
void md5_addsize(unsigned char *buffer, md5_size *len, md5_size oldlen)
{
  /*     a * 8 <=> a << 3     */
  buffer[(*len)++]=(oldlen<<3)&255;
  buffer[(*len)++]=(oldlen>>5)&255;
  buffer[(*len)++]=(oldlen>>13)&255;
  buffer[(*len)++]=(oldlen>>21)&255;
  buffer[(*len)++]=(oldlen>>29)&255;
  buffer[(*len)++]=(oldlen>>37)&255;
  buffer[(*len)++]=(oldlen>>45)&255;
  buffer[(*len)++]=(oldlen>>53)&255;
}

/* Calcul */
void md5_calc(md5_regs *regs, md5_size len, unsigned char *buffer)
{
  unsigned long A=regs->A, B=regs->B, C=regs->C, D=regs->D;
  unsigned long AA, BB, CC, DD;
  unsigned long i,j;
  unsigned long X[16];
  unsigned long *M=(unsigned long *)buffer;

  for(i = 0 ; i < len/64 ; i++)
    {
      for(j = 0 ; j < 16 ; j++)
	X[j]=M[i*16 + j];
      AA=A; BB=B; CC=C; DD=D;

      /* Ronde 1 */
      FF (A, B, C, D,  X[0],  7, 0xd76aa478);
      FF (D, A, B, C,  X[1], 12, 0xe8c7b756);
      FF (C, D, A, B,  X[2], 17, 0x242070db);
      FF (B, C, D, A,  X[3], 22, 0xc1bdceee);
      FF (A, B, C, D,  X[4],  7, 0xf57c0faf);
      FF (D, A, B, C,  X[5], 12, 0x4787c62a);
      FF (C, D, A, B,  X[6], 17, 0xa8304613);
      FF (B, C, D, A,  X[7], 22, 0xfd469501);
      FF (A, B, C, D,  X[8],  7, 0x698098d8);
      FF (D, A, B, C,  X[9], 12, 0x8b44f7af);
      FF (C, D, A, B, X[10], 17, 0xffff5bb1);
      FF (B, C, D, A, X[11], 22, 0x895cd7be);
      FF (A, B, C, D, X[12],  7, 0x6b901122);
      FF (D, A, B, C, X[13], 12, 0xfd987193);
      FF (C, D, A, B, X[14], 17, 0xa679438e);
      FF (B, C, D, A, X[15], 22, 0x49b40821);

      /* Ronde 2 */
      GG (A, B, C, D,  X[1],  5, 0xf61e2562);
      GG (D, A, B, C,  X[6],  9, 0xc040b340);
      GG (C, D, A, B, X[11], 14, 0x265e5a51);
      GG (B, C, D, A,  X[0], 20, 0xe9b6c7aa);
      GG (A, B, C, D,  X[5],  5, 0xd62f105d);
      GG (D, A, B, C, X[10],  9, 0x02441453);
      GG (C, D, A, B, X[15], 14, 0xd8a1e681);
      GG (B, C, D, A,  X[4], 20, 0xe7d3fbc8);
      GG (A, B, C, D,  X[9],  5, 0x21e1cde6);
      GG (D, A, B, C, X[14],  9, 0xc33707d6);
      GG (C, D, A, B,  X[3], 14, 0xf4d50d87);
      GG (B, C, D, A,  X[8], 20, 0x455a14ed);
      GG (A, B, C, D, X[13],  5, 0xa9e3e905);
      GG (D, A, B, C,  X[2],  9, 0xfcefa3f8);
      GG (C, D, A, B,  X[7], 14, 0x676f02d9);
      GG (B, C, D, A, X[12], 20, 0x8d2a4c8a);

      /* Ronde 3 */
      HH (A, B, C, D,  X[5],  4, 0xfffa3942);
      HH (D, A, B, C,  X[8], 11, 0x8771f681);
      HH (C, D, A, B, X[11], 16, 0x6d9d6122);
      HH (B, C, D, A, X[14], 23, 0xfde5380c);
      HH (A, B, C, D,  X[1],  4, 0xa4beea44);
      HH (D, A, B, C,  X[4], 11, 0x4bdecfa9);
      HH (C, D, A, B,  X[7], 16, 0xf6bb4b60);
      HH (B, C, D, A, X[10], 23, 0xbebfbc70);
      HH (A, B, C, D, X[13],  4, 0x289b7ec6);
      HH (D, A, B, C,  X[0], 11, 0xeaa127fa);
      HH (C, D, A, B,  X[3], 16, 0xd4ef3085);
      HH (B, C, D, A,  X[6], 23, 0x04881d05);
      HH (A, B, C, D,  X[9],  4, 0xd9d4d039);
      HH (D, A, B, C, X[12], 11, 0xe6db99e5);
      HH (C, D, A, B, X[15], 16, 0x1fa27cf8);
      HH (B, C, D, A,  X[2], 23, 0xc4ac5665);

      /* Ronde 4 */
      II (A, B, C, D,  X[0],  6, 0xf4292244);
      II (D, A, B, C,  X[7], 10, 0x432aff97);
      II (C, D, A, B, X[14], 15, 0xab9423a7);
      II (B, C, D, A,  X[5], 21, 0xfc93a039);
      II (A, B, C, D, X[12],  6, 0x655b59c3);
      II (D, A, B, C,  X[3], 10, 0x8f0ccc92);
      II (C, D, A, B, X[10], 15, 0xffeff47d);
      II (B, C, D, A,  X[1], 21, 0x85845dd1);
      II (A, B, C, D,  X[8],  6, 0x6fa87e4f);
      II (D, A, B, C, X[15], 10, 0xfe2ce6e0);
      II (C, D, A, B,  X[6], 15, 0xa3014314);
      II (B, C, D, A, X[13], 21, 0x4e0811a1);
      II (A, B, C, D,  X[4],  6, 0xf7537e82);
      II (D, A, B, C, X[11], 10, 0xbd3af235);
      II (C, D, A, B,  X[2], 15, 0x2ad7d2bb);
      II (B, C, D, A,  X[9], 21, 0xeb86d391);

      A+=AA; B+=BB; C+=CC; D+=DD;
    }
  regs->A=A; regs->B=B; regs->C=C; regs->D=D;
}

unsigned char *md5_result(md5_regs regs, unsigned char *digest)
{
  int c;

  for(c=0;c<4;c++)
    digest[c] = (regs.A >> (c*8)) & 255;
  for(;c<8;c++)
    digest[c] = (regs.B >> ((c-4)*8)) & 255;
  for(;c<12;c++)
    digest[c] = (regs.C >> ((c-8)*8)) & 255;
  for(;c<16;c++)
    digest[c] = (regs.D >> ((c-12)*8)) & 255;

  return digest;
}

int getMD5(char *mds, char *src)
{
	mds[0]=0;
	{
		char	md[16];
		int		i;
		md5((unsigned char *)src, strlen(src), (unsigned char *)md);
		for(i=0; i<16; i++)
		{
			char	s[1024];
			sprintf(s, "%2X", (int)(unsigned char)md[i]);
			strcat(mds, s);
		}
	}
	{
		char	*s=mds;
		while(*s)
		{
			if(*s==' ')
				*s='0';
			s++;
		}
	}
	return 0;
}
