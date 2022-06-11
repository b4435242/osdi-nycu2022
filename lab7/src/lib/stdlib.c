#include "include/stdlib.h"

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

int strcmp(char string1[], char string2[] )
{
    for (int i = 0; ; i++)
    {
        if (string1[i] != string2[i])
        {
            return string1[i] < string2[i] ? -1 : 1;
        }

        if (string1[i] == '\0')
        {
            return 0;
        }
    }
}

void* memset(void *b, int c, int len)
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



unsigned int hexstr_2_dec(char* s, int size){
    unsigned int val = 0;
    for (int i=0; i<size; i++) {
        val *= 16;
        char c = *(s+i);
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

char*  strtok_r(char* string_org,const char* demial) {
	static unsigned char* last; 
	unsigned char* str;         
	const unsigned char* ctrl = (const unsigned char*)demial;
	unsigned char map[32]; 
	int count;
	
	for (count =0; count <32; count++){
		map[count] = 0;
	}   
	do {
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);     
	if (string_org){
		str = (unsigned char*)string_org;
	} else{
		str = last;
	}
	while ((map[*str >> 3] & (1 << (*str & 7)))  && *str){
		str++;
	} 
	string_org = (char*)str; 
	for (;*str; str++){
		if ( map[*str >> 3] & (1 << (*str & 7))){
			*str++ = '\0';
			break;         
		}         
	}    
	last =str;    
	if (string_org == (char*)str){
		return NULL; 
	}else{
		return string_org;
	}
}

int atoi(char* str)
{
    // Initialize result
    int res = 0;
 
    // Iterate through all characters
    // of input string and update result
    // take ASCII character of corresponding digit and
    // subtract the code from '0' to get numerical
    // value and multiply res by 10 to shuffle
    // digits left to update running total
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
 
    // return result.
    return res;
}

uint64_t atoull(char* str)
{
    // Initialize result
    uint64_t res = 0;
 
    // Iterate through all characters
    // of input string and update result
    // take ASCII character of corresponding digit and
    // subtract the code from '0' to get numerical
    // value and multiply res by 10 to shuffle
    // digits left to update running total
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
 
    // return result.
    return res;
}

void strcpy(char *dest, char *src){
  if (dest==NULL||src==NULL) 
    return;
    
  while(*src!=NULL){
    *dest = *src;
    dest++;
    src++;
  }
}

void* memcpy(void *dest, const void *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

int ceil(float num){ // positive number
  int inum = (int)num;
    if (num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

size_t min(size_t a, size_t b){
  return a<b?a:b;
}

uint64_t align_n(uint64_t num, int n){
  uint64_t mask = 0;
  for(int i=0; i<n; i++) mask |= (1<<i);
  uint64_t offset = num & mask;
  if (offset!=0){
    num -= offset;
    num += mask+1;
  }
  return num;
}

char* strchr (register const char *s, char c)
{
  do {
    if (*s == c)
      {
        return (char*)s;
      }
  } while (*s++);
  return (0);
}

char* strrchr (register const char *s, char c)
{
  char *rtnval = 0;

  do {
    if (*s == c)
      rtnval = (char*) s;
  } while (*s++);
  return (rtnval);
}