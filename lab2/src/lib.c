#include "lib.h"

// do not count '\0'
size_t len(char * str){
  int length = 0;
  while (str[length]!='\0')
    length++;
  return length;
}




int _strncmp(const char *s1, const char *s2, register int n)
{
  register unsigned char u1, u2;
  while (n-- > 0)
    {
      u1 = (unsigned char) *s1++;
      u2 = (unsigned char) *s2++;
      if (u1 != u2)
        return u1 - u2;
      if (u1 == '\0')
        return 0;
    }
  return 0;
}

void* _memset(void *b, int c, int len)
{
  int           i;
  unsigned char *p = b;
  i = 0;
  while(len > 0)
    {
      *p = c;
      p++;
      len--;
    }
}

void* simple_malloc(size_t size){
  static void *heap_top = 0;
  void *old = heap_top;
  heap_top += size;
  return old;
}

unsigned int hexstr_2_dec(char field[], int size){
    unsigned int val = 0;
    int i = 0;
    while (i<size) //specify size
    {
        val *= 16;
        char c = field[i++];
        //if (size==-1&&c=='\0') // not specify size
        //  break;
        if(c>='0'&&c<='9') 
            val += c-'0'; 
        else if (c>='a'&&c<='f')
            val += c-'a'+10;
        else if (c>='A'&&c<='F')
            val += c-'A'+10;
    }
    return val;
}

size_t aligned_on_n_bytes(int size, int n){
  return size%n==0?size:((size/n)+1)*n;
}