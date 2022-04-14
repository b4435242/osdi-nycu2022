#ifndef __MM__
#define __MM__

#define LOG

#include "stdlib.h"
#include "link_list.h"
#include "uart.h"
#include "fdt.h"
#include "cpio.h"

#define MAX_PAGE_ORDER 10
#define F -1
#define X -2
#define XH -3

#define KB 0x400
#define PAGE_SIZE 0x1000
#define MAX_OBJECT_ORDER 11

/* Initialized at fdt callback */
uint64_t MEM_BASE;
uint64_t MEM_LENGTH;
uint64_t FRAME_ARRAY_LENGTH;
extern uint64_t CPIO_BASE;
uint64_t DTB_BASE;

typedef struct Page Page;
typedef struct Pool Pool;
typedef struct fdt_header fdt_header;

struct Page{
    uint32_t index;
    Page* next;
};

struct Pool{
    char *record; // boolean array to keep track of pool
    uint32_t order; // size = 2^order, order is the index of pointer array
    uint32_t left; // FULL = (left==0)
    uint32_t base; // physical base address
    Pool* next;
};

/* highest level allocation API */
void* malloc(size_t size);
void free(void* p);

/* startup allocator */
void startup_alloc(uint64_t DTB_BASE);
void memory_reserve(uint64_t start, uint64_t end); // arg is addr // parse to fdt_traverse_rsvmap
void* heap_malloc(size_t size);
void heap_free(void *p);


/* buddy system */
uint32_t alloc_page(uint32_t req_num_page);
uint32_t free_page(uint32_t index); // return merged order

uint32_t get_physical_addr_from_frame(uint32_t index);
uint32_t get_frame_index(uint64_t addr);
uint32_t get_buddy_index(uint32_t index, uint32_t order);
void set_page_taken(uint32_t index, uint32_t order); 
void set_page_free(uint32_t index, uint32_t order); 


/* memory pool */
void* alloc_object(size_t size);
void free_object(void* p, Pool* pool);

void set_pool(Pool* pool, uint32_t order, char *record, uint32_t left, uint32_t base, Pool* next);
uint32_t get_physical_addr_from_pool(uint32_t base, uint32_t order, uint32_t index);
int is_memory_pool_full(Pool *object);
Pool* search_memory_pool(uint32_t frame_index);

uint32_t nearest_2_exp_order(uint32_t n);

#endif