#ifndef __CPIO__
#define __CPIO__

#include "uart.h"
#include "stdlib.h"

typedef struct cpio_newc_header{
    char	   c_magic[6];
    char	   c_ino[8];
    char	   c_mode[8];
    char	   c_uid[8];
    char	   c_gid[8];
    char	   c_nlink[8];
    char	   c_mtime[8];
    char	   c_filesize[8];
    char	   c_devmajor[8];
    char	   c_devminor[8];
    char	   c_rdevmajor[8];
    char	   c_rdevminor[8];
    char	   c_namesize[8];
    char	   c_check[8];
} cpio_newc_header;

typedef struct extraction{
    char *name;
    char *file;
    unsigned int namesize;
    unsigned int filesize;
    cpio_newc_header* next_header;
} extraction;

uint64_t CPIO_BASE;

extraction cpio_search(char *name);
void cpio_ls();
void cpio_cat();
void parse_cpio_header(cpio_newc_header* header, extraction *info); 
unsigned long cpio_align (unsigned long v);

#endif