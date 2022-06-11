#include "include/vm.h"


pte *walk(pagetable pagetable, uint64_t va, int granularity_level) {
    pte *pte;
    for (int level = 3; level > granularity_level; level--) {
        uint64_t offset = va >> (level*9+12) & 0x1ff;
        pte = pagetable + offset;
        if (*pte & 1) { // pte is a entry
            pagetable = *pte & ~0xfff;
            pagetable = KERNEL_SPACE(pagetable);
        } else {
            pagetable = malloc(PAGE_SIZE);
            *pte = (uint64_t)pagetable | PD_TABLE;
            pagetable = KERNEL_SPACE(pagetable);
            memset(pagetable, 0, PAGE_SIZE);
        }
    }
    uint64_t offset = va >> (granularity_level*9+12) & 0x1ff;
    pte = pagetable + offset;
    return pte;
}

void mappages(pagetable pagetable, uint64_t va, uint64_t size, uint64_t pa, int user_access){
    for (int i=0; i<size; i++){
        pte *pte = walk(pagetable, va, 0); // 4KB
        
        uint64_t attr = PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE<<2) | PD_TABLE;
        if (user_access) 
            attr |= PD_USER_ACCESS;
        *pte = pa | attr;
        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }
        
}

void mapblocks_2mb(pagetable pagetable, uint64_t va, uint64_t size, uint64_t pa, int user_access){
    for (int i=0; i<size; i++){
        pte *pte = walk(pagetable, va, 1); // 2MB
        
        uint64_t attr = PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE<<2) | PD_BLOCK;
        if (user_access) 
            attr |= PD_USER_ACCESS;
        *pte = pa | attr;
        va += BLOCK_2MB_SIZE;
        pa += BLOCK_2MB_SIZE;
    }
}

void* mmap(void* addr, size_t len, int prot, int flags, int fd, int file_offset){
    pte* pte = NULL;
    pagetable u_PGD = current_thread()->u_PGD;

    if (addr!=NULL){
        addr = align_n(addr, 12); // paged-align
    } else {
        addr = 0;
    }
    size_t page_len = len/PAGE_SIZE + (len%PAGE_SIZE>0?1:0); // round up page size
    size_t n;
    // find available addr with n consecutive empty page
    while (n<page_len && addr<VM_STACK_BOTTOM)
    {
        pte = walk(u_PGD, addr, 0);
        if (pte==0) n++; // empty
        else n = 0; // used
        
        addr += PAGE_SIZE;
    }

    if (addr==VM_STACK_BOTTOM) 
        return mmap(NULL, len, prot, flags, fd, file_offset);

    
    uint64_t attr = PD_USER_ACCESS | PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE<<2) | PD_TABLE;
    if (prot&PROT_NONE){
        attr &= ~PD_ACCESS;
    } 
    if (prot&PROT_READ==1 && prot&PROT_WRITE==0){
        attr |= PD_READ_ONLY;
    }
    if (prot&PROT_EXEC==0){
        attr |= PD_NONE_EXEC_EL0;
    }

    uint64_t pa = malloc(len);
    uint64_t va = addr;
    
    for(int i=0; i<page_len; i++){
        pte = walk(u_PGD, va, 0);
        *pte = attr;
        va += PAGE_SIZE;
        pa += PAGE_SIZE;
    }

    return addr;
}