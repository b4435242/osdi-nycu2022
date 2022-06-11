#ifndef __VM__
#define __VM__

#include "stdlib.h"
#include "mm.h"

#define KERNEL_PGD_PA 0x1000

#define VM_KERNEL 0xffff000000000000

#define VM_STACK_BOTTOM 0xfffffffff000
#define VM_STACK_HEAD 0xffffffffb000
#define VM_TEXT 0

#define VM_HANDLER_STACK_BOTTOM 0xffffffffb000
#define VM_HANDLER_STACK_HEAD 0xffffffff7000


#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)
#define PD_USER_ACCESS (1<<6)
#define PD_READ_ONLY (1<<7)
#define PD_NONE_EXEC_EL0 (1<<54)


#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define KERNEL_SPACE(x) (uint64_t)x+VM_KERNEL
#define DE_KERNEL_SPACE(x) (uint64_t)x-VM_KERNEL
#define IS_IN_KERNEL_SPACE(x) (uint64_t)x&VM_KERNEL==VM_KERNEL


#define BLOCK_2MB_SIZE (1<<21) 

#define MAP_ANONYMOUS 0x20
#define MAP_POPULATE 0x8000

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

typedef uint64_t pte;
typedef uint64_t *pagetable;

pte *walk(pagetable pagetable, uint64_t va, int granularity_level);
void mappages(pagetable pagetable, uint64_t va, uint64_t size, uint64_t pa, int user_access);
void mapblocks_2mb(pagetable pagetable, uint64_t va, uint64_t size, uint64_t pa, int user_access);

void* mmap(void* addr, size_t len, int prot, int flags, int fd, int file_offset);

#endif