// This file is licensed CC0 by Xenon++.

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <nds.h>
#include <fat.h>

#include "util.h"

int filelength(int fd){ //constant phrase
    struct stat st;
    fstat(fd,&st);
    return st.st_size;
}

unsigned int read24be(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[2]|(x[1]<<8)|(x[0]<<16);
}

unsigned short read16be(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[1]|(x[0]<<8);
}

void write24be(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[2]=n&0xff,x[1]=(n>>8)&0xff,x[0]=(n>>16)&0xff;
}

void write16be(void *p, const unsigned short n){
	unsigned char *x=(unsigned char*)p;
	x[1]=n&0xff,x[0]=(n>>8)&0xff;
}
