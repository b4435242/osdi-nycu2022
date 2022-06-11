#ifndef __LL__
#define __LL__

#include "stdlib.h"
#include "mm.h"
#include "uart.h"

typedef struct Page Page;
typedef struct Pool Pool;


Page* get_first_page(Page* head);
Page* remove_page(Page* head, Page* page); 
Page* add_page(Page* head, Page* page); 
void add_page_with_index(Page* head, unsigned int index);
void remove_page_with_index(Page* head, unsigned int index);

Pool* add_pool(Pool* head, Pool* node); 

#endif