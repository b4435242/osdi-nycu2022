#ifndef __LIB__
#define __LIB__

#define size_t unsigned int
size_t len(char * str);
int _strncmp(const char *s1, const char *s2, register int n);
void* _memset(void *b, int c, int len);
void* simple_malloc(size_t size);
unsigned int hexstr_2_dec(char field[], int size);
size_t aligned_on_n_bytes(int size, int n);

#endif