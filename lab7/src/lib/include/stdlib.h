#ifndef __STDLIB__
#define __STDLIB__

//#define size_t unsigned int
#define NULL ((void *)0)
#define true 1
#define false 0

typedef void (*callback_ptr)(void*);

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;
typedef unsigned char bool;

size_t len(char * str);
int _strncmp(const char *s1, const char *s2, register int n);
int strcmp(char string1[], char string2[] );
void* memset(void *b, int c, int len);
unsigned int hexstr_2_dec(char* s, int size);
size_t aligned_on_n_bytes(int size, int n);
char*  strtok_r(char* string_org, const char* demial);
int atoi(char* str);
uint64_t atoull(char* str);
void strcpy(char* dest, char *src);
void *memcpy(void *dest, const void *src, size_t n);
int ceil(float n);
size_t min(size_t a, size_t b);
uint64_t align_n(uint64_t num, int n);
char* strchr (register const char *s, char c);
char* strrchr (register const char *s, char c);

#endif