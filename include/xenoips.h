// This file is licensed CC0 by Xenon++.

#include <stdio.h>
#include <nds.h>

int RLE2(const u8 *p, const unsigned int size, const unsigned int check, unsigned int *length);
void ipswrite(const u8 *p, const unsigned int address, const unsigned int _size, FILE *ips);
void ipsfill(const u8 p, const unsigned int address, const unsigned int _size, FILE *ips);
int ipsmake(const u8 *pfile, const unsigned int sizfile, const u8 *pnewfile, const unsigned int siznewfile, FILE *ips);
int ipspatch(u8 *pfile, unsigned int *sizfile, const u8 *pips, const unsigned int sizips);
int _ipsmake(FILE *file, FILE *newfile, FILE *ips);
int _ipspatch(FILE *file, FILE *ips);
