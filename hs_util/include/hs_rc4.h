#ifndef __HS_RC4_H__
#define __HS_RC4_H__

void
hs_rc4_init(unsigned char* s, unsigned char* key, unsigned long Len);

void
hs_rc4_crypt(unsigned char* s, unsigned char* Data, unsigned long Len);

#endif
