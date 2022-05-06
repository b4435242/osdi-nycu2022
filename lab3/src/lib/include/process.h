#ifndef __PROCESS__
#define __PROCESS__

#include "cpio.h"
#include "stdlib.h"
#include "mm.h"
#include "thread.h"
#include "sys_handler.h"

typedef struct Process Process;


struct Process
{
    uint64_t start;
    uint32_t size;
};





int load_new_image(char *name, char *const argv[], uint64_t* lr, uint64_t* sp);
int fork_process(uint64_t k_sp);
Process* get_current_proc();
Process* get_init_proc();
void from_el1_to_el0(uint64_t lr, uint64_t sp);

#endif