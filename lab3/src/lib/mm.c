#include "include/mm.h"

signed char *frame_array;
Page* free_list[MAX_PAGE_ORDER+1]; // HEAD remain unchanged
Pool* memory_pools[MAX_OBJECT_ORDER+1];


/* highest level allocation API */

void* malloc(size_t size){ 
    size_t page = size/PAGE_SIZE; 
    if (page>0){ // page frame
        page += (size%PAGE_SIZE==0?0:1); // ceiling of page
        uint32_t index = alloc_page(page);
        return (void*)get_physical_addr_from_frame(index);
    } else { // allocate from memory pool
        return alloc_object(size);
    }
}

void free(void* p){
    uint32_t addr = (uint32_t)p;
    uint32_t index = get_frame_index(addr);
    // use index to decide whether object or page is going to be free
    Pool* pool = search_memory_pool(index);  // iterate memory_pools to compare address base
    if (pool==NULL){
        free_page(index);
    } else {
        free_object(p, pool);
    }
}

/* startup allocator */

void startup_alloc(uint64_t DTB_BASE){
    

    // init head of all size of pages
    for(int i=0; i<=MAX_PAGE_ORDER; i++){
        free_list[i] = heap_malloc(sizeof(Page));
        free_list[i]->next = NULL;
    }
    // init head of all size of memory pools 
    // 2^0 ~ 2^11
    for(int i=0; i<=MAX_OBJECT_ORDER; i++){
        memory_pools[i] = heap_malloc(sizeof(Pool));
        memory_pools[i]->next = NULL;
    }

    /* dynamically setup frame_array with usable memory len from DTB */
    FRAME_ARRAY_LENGTH = MEM_LENGTH/PAGE_SIZE;
    frame_array = heap_malloc(FRAME_ARRAY_LENGTH*sizeof(signed char));
    // initialize frame_array to X
    for(int i=0; i<FRAME_ARRAY_LENGTH; i++) frame_array[i] = X;

    /* reserve memory */
    fdt_traverse_rsvmap(DTB_BASE, memory_reserve);
    memory_reserve(0x100000, 0x100400); // user program code

    /* merge pages */ 
    uint32_t index = 0;
    while (index<FRAME_ARRAY_LENGTH)
    {
        if (frame_array[index]==X){
            uint32_t order = free_page(index);
            index += (1<<order);
        } else { // reserve memory
            index++;
        }
        
    }
}

void memory_reserve(uint64_t start, uint64_t end){ // arg is addr
    uint32_t si = get_frame_index(start);
    uint32_t ei = get_frame_index(end);
    uint32_t offset = (end-start)%PAGE_SIZE;
    if (offset>0) ei++; // reserve till next page
    uint32_t index = si;
    while (index<ei) // set taken page by page
    {
        set_page_taken(index++, 0);
    }
    
}


void* heap_malloc(size_t size){
  static void *heap_top = 0x10000;
  void *old = heap_top;
  heap_top += cpio_align(size);
  return old;
}

void heap_free(void *p){
    // NOT IMPLEMENTED
}


/* buddy system */

uint32_t alloc_page(uint32_t req_num_page){
    
    // get nearest 2 exp
    uint32_t req_page_order = nearest_2_exp_order(req_num_page);
    req_num_page = 1<<req_page_order;

    // search first available page of the certain size 
    // if not exist, iterate to larger page
    // Assume there is must be a page, no case of out-of-memory
    // available_page != NULL
    uint32_t alloc_page_order = req_page_order;
    Page *available_page;
    while (alloc_page_order<=MAX_PAGE_ORDER)
    {
        uart_puts("order");
        uart_hex(alloc_page_order);
        uart_puts("\r\n");
        available_page = get_first_page(free_list[alloc_page_order]);
        if (available_page!=NULL) break;
        else alloc_page_order++;
    }


    if (available_page==NULL) return -1;
    uart_puts("index is");
    uart_hex(&(available_page->index));

    uart_puts("\r\n");
    // delete available_page with alloc_page_order
    Page *head_available = free_list[alloc_page_order];
    uint32_t index = available_page->index;
    remove_page(head_available, available_page); // del first node
    set_page_taken(index, alloc_page_order); // mark as taken

    // put redundant pages back
    size_t redundant_page_order = alloc_page_order;
    while(redundant_page_order>req_page_order){
        redundant_page_order--;
        size_t redundant_page_index = available_page->index + (1<<redundant_page_order);
        // update free list
        add_page_with_index(free_list[redundant_page_order], redundant_page_index); // put half back to list
        // update frame array
        set_page_free(redundant_page_index, redundant_page_order);
    }

    

    return index;
    
}

uint32_t free_page(uint32_t index){
    
    // Buddy system
    if (index>=0 && index<FRAME_ARRAY_LENGTH){ // index falls in the range of buddy system
        /* Merge iteratively */ 

        uint32_t order = 0;
        while (order<MAX_PAGE_ORDER)
        {
            // try to merge the buddy with order
            size_t buddy = get_buddy_index(index, order);
            // In case of the memory that is going to be free
            // And the memory that is already free before this operation
            if (frame_array[buddy]==X || frame_array[buddy]==order){
                if (frame_array[buddy]==order)
                    remove_page_with_index(free_list[order], buddy);
                #ifdef LOG
                    uart_puts("[mm] Index ");
                    uart_hex(index);
                    uart_puts(" Successfully Merge The buddy ");
                    uart_hex(buddy);
                    uart_puts(" with the order ");
                    uart_hex(order);
                    uart_puts("\r\n");
                #endif
                order++; // Merge next order

            } else {
                break;
            }
        }

        // update frame array
        set_page_free(index, order);
        // update free list
        add_page_with_index(free_list[order], index);
        return order;
    }
    return -1;
}


/* memory pool */

void* alloc_object(size_t size){
    int alloc_object_order = nearest_2_exp_order(size);

    // In case of NO memory pool or the memory pool for certain object size is full 
    // Register a page as a new memory pool
    if (memory_pools[alloc_object_order]==NULL || is_memory_pool_full(memory_pools[alloc_object_order])){
        // alloc a new page frame
        size_t index = alloc_page(1);
        set_page_taken(index, 0); // order 0 = size 1
        remove_page_with_index(free_list[0], index);

        // new memory_pool
        uint32_t base = get_physical_addr_from_frame(index);
        size_t left = PAGE_SIZE/(1<<alloc_object_order); // left is initiated as the total count of objects in memory pool
        Pool *pool = heap_malloc(sizeof(Pool));
        pool->next = NULL;
        char *record = heap_malloc(left*sizeof(char));
        set_pool(pool, alloc_object_order, record, left, base, NULL);

        // add object header to pointer array, memory_pools
        add_pool(memory_pools[alloc_object_order], pool);
    }

    // select a NON-FULL memory pool
    // traverse memory_pools
    Pool *pool = memory_pools[alloc_object_order];
    while(pool!=NULL && is_memory_pool_full(pool)) pool = pool->next; 

    // return object address from memory pool
    size_t object_size = 1<<alloc_object_order;
    int record_len = PAGE_SIZE / object_size;
    size_t index = 0;
    while(index<record_len && pool->record[index]!=0) index++; // 0 represents free
    pool->record[index] = 1; // 1 represents taken
    pool->left--;
    
    return get_physical_addr_from_pool(pool->base, alloc_object_order, index);

}

void free_object(void* p, Pool* pool){ // NO IMPLEMENTATION of free page
    uint32_t addr = (uint32_t)p;
    // get index of memory pool
    size_t object_size = 1 << pool->order;
    size_t index = (addr - pool->base) / object_size; // offset div size
    pool->record[index] = 0; // 0 represents free
    pool->left++;
}

/* util */

void set_pool(Pool* pool, uint32_t order, char *record, uint32_t left, uint32_t base, Pool* next){
    for(int i=0; i<left; i++) record[i] = 0;
    pool->record = record;
    pool->order = order;
    pool->left = left;
    pool->base = base;
    pool->next = next;
}

uint32_t get_physical_addr_from_frame(uint32_t index){
    return MEM_BASE + (index*PAGE_SIZE);
}

uint32_t get_physical_addr_from_pool(uint32_t base, uint32_t order, uint32_t index){
    return base + (index*(1<<order));
}

uint32_t get_frame_index(uint64_t addr){
    return (addr - MEM_BASE) / PAGE_SIZE;
}

uint32_t get_buddy_index(uint32_t index, uint32_t order){
    return index ^ (1<<order);
}

void set_page_taken(uint32_t index, uint32_t order){
    // first element represents the head of continuous memory
    // XH won't be merged when another memory is being free
    frame_array[index] = XH;
    // set the following to X
    int len = 1<<order;
    for(int i=1; i<len; i++){
        frame_array[index+i] = X;
    }
}

void set_page_free(uint32_t index, uint32_t order){
    // first element represents order of continuous memory
    frame_array[index] = order;
    // set the following to F
    int len = 1<<order;
    for(int i=1; i<len; i++){
        frame_array[index+i] = F;
    }
}

uint32_t nearest_2_exp_order(uint32_t n){
    int order = 0;
    while((1<<order)<n) order++;
    return order;
}

int is_memory_pool_full(Pool *object){
    return object->left == 0;
}

Pool* search_memory_pool(uint32_t frame_index){
    uint32_t base = get_physical_addr_from_frame(frame_index);
    for(int i=0; i<=MAX_OBJECT_ORDER; i++){
        Pool *p = memory_pools[i];
        while(p!=NULL){
            if (p->base==base)
                return p;
            p = p->next;
        }
    }
    return NULL; // NOT FOUND
}



