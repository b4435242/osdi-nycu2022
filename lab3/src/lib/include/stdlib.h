#ifndef __STDLIB__
#define __STDLIB__

#define size_t unsigned int
#define NULL ((void *)0)

typedef void (*callback_ptr)(void*);


size_t len(char * str);
int _strncmp(const char *s1, const char *s2, register int n);
void* _memset(void *b, int c, int len);
void* simple_malloc(size_t size);
unsigned int hexstr_2_dec(char* s, int size);
size_t aligned_on_n_bytes(int size, int n);
char*  strtok_r(char* string_org, const char* demial);
int atoi(char* str);
void _strcpy(char* dest, char *src);
void *memcpy(void *dest, const void *src, size_t n);

#endif