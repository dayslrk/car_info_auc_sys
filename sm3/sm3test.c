#include <string.h>
#include <stdio.h>
#include "sm3.h"

int main( int argc, char *argv[] )
{
	unsigned char *input =(unsigned char *)argv[2];
	int ilen;
	ilen=strlen((char *)input);
	int klen;
 	unsigned char *key=(unsigned char *)argv[1];
	klen=strlen((char *)key);
	unsigned char output[32];
	int i;
        sm3_hmac(key,klen,input, ilen, output);
	for(i=0;i<32;i++){
	printf("%02x",output[i]);
                }


    //getch();	//VS2008 
}
