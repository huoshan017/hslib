#include "hs_rc4.h"


/*初始化函数*/
void hs_rc4_init(unsigned char *s,unsigned char* key, unsigned long Len)
{
	int i=0, j=0;
	char k[256] = {0};
	unsigned char tmp = 0;
	for (i=0; i<256; i++) {
		s[i] = i;
		k[i] = key[i%Len];
	}
	for (i=0; i<256; i++) {
		j = (j+s[i]+k[i])%256;
		tmp = s[i];
		s[i] = s[j];//交换s[i]和s[j]
		s[j] = tmp;
	}
}

/*加解密*/
void hs_rc4_crypt(unsigned char* s, unsigned char* Data, unsigned long Len)
{
	int i = 0, j = 0, t = 0;
	unsigned long k = 0;
	unsigned char tmp;
	for (k=0; k<Len; k++) {
		i = (i+1)%256;
		j = (j+s[i])%256;
		tmp = s[i];
		s[i] = s[j];//交换s[x]和s[y]
		s[j] = tmp;
		t = (s[i]+s[j])%256;
		Data[k] ^= s[t];
	}
}
