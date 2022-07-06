// This file is licensed CC0 by Xenon++.

/*
 * XenoPatch driver
 * Now supports IPS/PPF/UPF/WIPS
 * xdelta and bsdiff are delta format, so they won't be supported.
 * Moreover, xdelta.exe is much more reliable than ips patchers.
*/

#include <stdio.h>

#include <nds.h>

#include "util.h"

#define BUFLEN (1<<19)
unsigned char buf[BUFLEN];

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

int RLE2(const u8 *p, const unsigned int size, const unsigned int check, unsigned int *length){ //if>8bytes, fill operation will be used.
	int i=0,j;
	if(size < 3 || check < 3) return -1;
	for(; i < size - check; i++) {
		for(j = i + 1; j < size; j++) if(p[i] != p[j]) break;
		if(j >= i + check) {
            *length = j - i;
            return i;
        }
	}
	return -1;
}

void ipswrite(const u8 *p, const unsigned int address, const unsigned int _size, FILE *ips){
	u8 head[5];
	unsigned int size = _size, patchofs = 0;
	while(size) {
		iprintf("write 0x%06x %dbytes\n", address + patchofs, min(size, 65535));
		write24be(head, address + patchofs);
		write16be(head + 3, min(size, 65535));
		fwrite(head, 1, 5, ips);
		fwrite(p + address + patchofs, 1, min(size, 65535), ips);
		patchofs += min(size, 65535);
		size -= min(size, 65535);
	}
}

void ipsfill(const u8 p, const unsigned int address, const unsigned int _size, FILE *ips){
	u8 head[7];
	unsigned int size = _size, patchofs = 0;
	while(size) {
		iprintf("fill  0x%06x %dbytes\n",address+patchofs,min(size,65535));
		write24be(head,address+patchofs);
		head[3]=head[4]=0;
		write16be(head+5,min(size,65535));
		fwrite(head,1,7,ips);
		fwrite(&p,1,1,ips);
		patchofs+=min(size,65535);
		size-=min(size,65535);
	}
}

//libxenoips
int ipsmake(const u8 *pfile, const unsigned int sizfile, const u8 *pnewfile, const unsigned int siznewfile, FILE *ips){
	unsigned int i=0,address=0,final;
	unsigned int size,_size,patchofs;
	fwrite("PATCH",1,5,ips);
	for(;i<min(sizfile,siznewfile);i++){
		if(pfile[i]!=pnewfile[i]){
			address=final=i;
			//for(gap=0;gap<6&&i-1<min(sizfile,siznewfile);gap++){//Up to 5 bytes can be gap. To make small IPS.
			//	i++;
			//	if(pfile[i]!=pnewfile[i])gap=0,final=i+1; //if !memcmp(pfile+i,pnewfile+i,5) then break
			//}
			for(;memcmp(pfile+final,pnewfile+final,min(5,min(sizfile,siznewfile)-final));final++)i++;

			_size=size=final-address;
			patchofs=0;
			while(size){
				unsigned int rleaddr=-1,rlelength=0;
				//OK Let's consume the buffer (do I have to use "fill"?)
				if(size>3)rleaddr=RLE2(pnewfile+address+patchofs,_size-patchofs,min(size,7),&rlelength);
				if(rleaddr!=-1){
					if(rleaddr)ipswrite(pnewfile,address+patchofs,rleaddr,ips);
					ipsfill(pnewfile[address+patchofs+rleaddr],address+patchofs+rleaddr,rlelength,ips);
					size-=rleaddr+rlelength;
					patchofs+=rleaddr+rlelength;
					continue;
				}
				ipswrite(pnewfile,address+patchofs,size,ips);
				break; //
			}
		}
	}

	if(siznewfile > sizfile){
		address = sizfile;
		_size = size = siznewfile - sizfile;
		patchofs = 0;
			while(size){
				unsigned int rleaddr = -1, rlelength = 0;
				//OK Let's consume the buffer (do I have to use "fill"?)
				rleaddr = RLE2(pnewfile + address + patchofs, _size - patchofs, size > 3 ? min(size, 7) : 3 , &rlelength);
				if(rleaddr != -1){
					if(rleaddr) ipswrite(pnewfile, address + patchofs, rleaddr, ips);
					ipsfill(pnewfile[address + patchofs + rleaddr], address + patchofs + rleaddr, rlelength, ips);
					size-=rleaddr+rlelength;
					patchofs+=rleaddr+rlelength;
					continue;
				}
				ipswrite(pnewfile,address+patchofs,size,ips);
				break; //
			}
	}

	fwrite("EOF",1,3,ips);
	fflush(ips);
	return 0;
}

int ipspatch(u8 *pfile, unsigned int *sizfile, const u8 *pips, const unsigned int sizips){ //pass pfile=NULL to get sizfile.
	unsigned int offset=5,address=0,size=-1,u;
	if(sizips<8||memcmp(pips,"PATCH",5))return 2;
	if(!pfile)*sizfile=0;
	while(1){
		if(offset + 3 > sizips) return 1;
        address = read24be(pips + offset);
        offset += 3;
		if(address == 0x454f46 && offset == sizips) break;
		if(offset + 2 > sizips) return 1;
        size = read16be(pips + offset);
        offset += 2;
		if(size){
			if(offset+size>sizips)return 1;
			if(!pfile){
				if(*sizfile<address+size)*sizfile=address+size;
				offset+=size;
				continue;
			}
			if(address+size>*sizfile)return 1;
			iprintf("write 0x%06x %dbytes\n",address,size);
			memcpy(pfile+address,pips+offset,size);
			offset+=size;
		}else{
			if(offset + 3 > sizips) return 1;
            size = read16be(pips + offset);
            offset += 2;
			if(!pfile){
				if(*sizfile<address+size)*sizfile=address+size;
				offset++;
				continue;
			}
			if(address+size>*sizfile)return 1;
			iprintf("fill  0x%06x %dbytes\n",address,size);
			for(u=address;u<address+size;u++)pfile[u]=pips[offset];
			offset++;
		}
	}
	return 0;
}

//bootstrap
int _ipsmake(FILE *file, FILE *newfile, FILE *ips){
	u8 *pfile=NULL,*pnewfile=NULL;
	unsigned int sizfile,siznewfile;
	//int ret;

	if((sizfile=filelength(fileno(file)))>0x1000000||(siznewfile=filelength(fileno(newfile)))>0x1000000){
		iprintf("file is too big(16MB)\n");return 1;
	}
	pfile=(u8*)malloc(sizfile);
	pnewfile=(u8*)malloc(siznewfile);
	if(!pfile||!pnewfile){
		if(pfile)free(pfile);
		if(pnewfile)free(pnewfile);
		iprintf("cannot allocate memory\n");return 2;
	}
	memset(pfile,0,sizfile);
	fread(pfile,1,sizfile,file);
	memset(pnewfile,0,siznewfile);
	fread(pnewfile,1,siznewfile,newfile);

	iprintf("(%u bytes / %u bytes)\n",sizfile,siznewfile);
	ipsmake(pfile,sizfile,pnewfile,siznewfile,ips);

	iprintf("Made successfully\n");
	free(pfile);free(pnewfile);
	return 0;
}

int _ipspatch(FILE *file, FILE *ips){
	u8 *pfile=NULL,*pips=NULL;
	unsigned int sizfile,sizips;
	int ret;

	sizips=filelength(fileno(ips));
	pips=(u8*)malloc(sizips);
	if(!pips){
		iprintf("cannot allocate memory\n");return 4;
	}
	fread(pips,1,sizips,ips);

	iprintf("(IPS %u bytes) ",sizips);
	ret=ipspatch(NULL,&sizfile,pips,sizips);
	switch(ret){
		case 0:
			if(!sizfile){
				free(pips);
				iprintf("ips empty\n");return 5;
			}
			iprintf("allocfilesize=%u\n",sizfile);
			pfile=(u8*)malloc(sizfile);
			if(!pfile){
				free(pips);
				iprintf("cannot allocate memory\n");return 4;
			}
			fread(pfile,1,sizfile,file); //might occur error, but OK
			rewind(file);
			ipspatch(pfile,&sizfile,pips,sizips);
			fwrite(pfile,1,sizfile,file);
			free(pfile);
			iprintf("Patched successfully\n");
			break;
		case 2:
			iprintf("\nips not valid\n");
			break;
		case 1:
			iprintf("\nPatch failed (corrupted / truncated)\n");
			break;
	}
	free(pips);
	return ret;
}
