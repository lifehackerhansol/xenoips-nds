// This file is licensed CC0 by Xenon++.

/*
 * XenoPatch driver
 * Now supports IPS/PPF/UPF/WIPS
 * xdelta and bsdiff are delta format, so they won't be supported.
 * Moreover, xdelta.exe is much more reliable than ips patchers.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <nds/ndstypes.h>

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

int filelength(int fd){ //constant phrase
    struct stat st;
    fstat(fd,&st);
    return st.st_size;
}

unsigned int read24be(const void *p) {
	const unsigned char *x = (const unsigned char*)p;
	return x[2] | (x[1] << 8) | (x[0] << 16);
}

unsigned short read16be(const void *p) {
	const unsigned char *x = (const unsigned char*)p;
	return x[1] | (x[0] << 8);
}

int ipspatch(u8 *pfile, unsigned int *sizfile, const u8 *pips, const unsigned int sizips) { //pass pfile=NULL to get sizfile.
	unsigned int offset = 5, address = 0, size = -1, u;
	if(sizips < 8 || memcmp(pips, "PATCH", 5)) return 2;
	if(!pfile) *sizfile=0;
	while(1) {
		if(offset + 3 > sizips) return 1;
        address = read24be(pips + offset);
        offset += 3;
		if(address == 0x454f46 && offset == sizips) break;
		if(offset + 2 > sizips) return 1;
        size = read16be(pips + offset);
        offset += 2;
		if(size) {
			if(offset + size > sizips) return 1;
			if(!pfile) {
				if(*sizfile < address + size) *sizfile = address + size;
				offset += size;
				continue;
			}
			if(address + size > *sizfile) return 1;
			iprintf("write 0x%06x %dbytes\n", address, size);
			memcpy(pfile + address, pips + offset, size);
			offset += size;
		} else {
			if(offset + 3 > sizips) return 1;
            size = read16be(pips + offset);
            offset += 2;
			if(!pfile){
				if(*sizfile < address + size) *sizfile = address + size;
				offset++;
				continue;
			}
			if(address + size > *sizfile) return 1;
			iprintf("fill  0x%06x %dbytes\n", address, size);
			for(u = address; u < address + size; u++) pfile[u] = pips[offset];
			offset++;
		}
	}
	return 0;
}

//bootstrap
int _ipspatch(FILE *file, FILE *ips) {
	unsigned int sizfile, sizips;

	sizips = filelength(fileno(ips));
	u8* pips = (u8*)malloc(sizips);
	if(!pips) {
		iprintf("cannot allocate memory\n");
        return 4;
	}
	fread(pips, 1, sizips, ips);

	iprintf("(IPS %u bytes) ", sizips);
	int ret = ipspatch(NULL, &sizfile, pips, sizips);
	switch(ret) {
		case 0:
			if(!sizfile) {
				free(pips);
				iprintf("ips empty\n");
                return 5;
			}
			iprintf("allocfilesize=%u\n", sizfile);
			u8* pfile = (u8*)malloc(sizfile);
			if(!pfile) {
				free(pips);
				iprintf("cannot allocate memory\n");return 4;
			}
			fread(pfile, 1, sizfile, file); //might occur error, but OK
			rewind(file);
			ipspatch(pfile, &sizfile, pips, sizips);
			fwrite(pfile, 1, sizfile, file);
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
