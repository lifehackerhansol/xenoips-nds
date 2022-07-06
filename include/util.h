// This file is licensed CC0 by Xenon++.

#include <nds.h>

int filelength(int fd);

unsigned int read24be(const void *p);
unsigned short read16be(const void *p);
void write24be(void *p, const unsigned int n);
void write16be(void *p, const unsigned short n);
