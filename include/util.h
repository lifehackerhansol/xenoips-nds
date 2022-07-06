// This file is licensed CC0 by Xenon++.

#include <nds.h>

int filelength(int fd);

unsigned long long int read64(const void *p);
unsigned int read32(const void *p);
unsigned int read24(const void *p);
unsigned short read16(const void *p);
void write64(void *p, const unsigned long long int n);
void write32(void *p, const unsigned int n);
void write24(void *p, const unsigned int n);
void write16(void *p, const unsigned short n);

unsigned int read32be(const void *p);
unsigned int read24be(const void *p);
unsigned short read16be(const void *p);
void write32be(void *p, const unsigned int n);
void write24be(void *p, const unsigned int n);
void write16be(void *p, const unsigned short n);

char* myfgets(char *buf,int n,FILE *fp);
void msleep(int msec);

unsigned short crc16(unsigned short crc, const unsigned char *p, unsigned int size);
unsigned int crc32_left(unsigned int crc, const unsigned char *p, unsigned int size);

typedef void (*type_u32p)(u32*);

int strchrindex(const char *s, const int c, const int idx);
size_t _FAT_directory_mbstoucs2(unsigned short* dst, const unsigned char* src, size_t len);
u32 mbstoucs2(unsigned short* dst, const unsigned char* src);
size_t _FAT_directory_ucs2tombs(unsigned char* dst, const unsigned short* src, size_t len);
u32 ucs2tombs(unsigned char* dst, const unsigned short* src);
void NullMemory(void* buf, unsigned int n);

int memcmp_fast(const void *x,const void *y,unsigned int len);
