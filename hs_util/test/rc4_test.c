#include "hs_rc4.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	unsigned char s[256] = {0}, s2[256] = {0}; //S-box
	char key[256] = { "justfortest" };
	char pData[512] = "这是一个用来加密的数据Data";
	unsigned long len = strlen(pData);
	int i;
	 
	printf("pData=%s\n", pData);
	printf("key=%s,length=%d\n\n", key, strlen(key));
	hs_rc4_init(s, (unsigned char*)key, strlen(key));//已经完成了初始化
	printf("完成对S[i]的初始化，如下：\n\n");
	for (i=0; i<256; i++) {
		printf("%02X", s[i]);
		if (i && (i+1)%16 == 0)
			putchar('\n');
	}
	printf("\n\n");

	//用s2[i]暂时保留经过初始化的s[i]，很重要的！！！
	for (i=0; i<256; i++) {
		s2[i] = s[i];
	}
	printf("已经初始化，现在加密:\n\n");
	hs_rc4_crypt(s, (unsigned char*)pData, len);//加密
	printf("pData=%s\n\n",pData);
	printf("已经加密，现在解密:\n\n");
	hs_rc4_crypt(s2, (unsigned char*)pData, len);//解密
	printf("pData=%s\n\n", pData);
	return 0;
}
