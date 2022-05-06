#include "include/process.h"

// keep init mem layout for fork
Process proc_table[MAX_THREAD]; 


int load_new_image(char *name, char *const argv[], uint64_t* lr, uint64_t* sp){


    extraction* image = cpio_search(name);
    if (image==NULL) 
        return -1;

    char *program_text = malloc(image->filesize);
    uart_puts("malloc-1 fin ");


    memcpy(program_text, image->file, image->filesize);


    uart_puts("copy fin");

    uint64_t u_stack = malloc(THREAD_SP_SIZE) + THREAD_SP_SIZE;
    
    *lr = (uint64_t)program_text;
    *sp = u_stack;
    //uart_puts("malloc fin");
    // thread data update
    thread* cur_t = current_thread();
    free(cur_t->u_stack);
    cur_t->u_stack = u_stack;
    //uart_puts("thread fin");

    // proc data update
    int curr = cur_t->tpid;
    Process *proc = &proc_table[curr];
    proc->start = (uint64_t)program_text, 
    proc->size = image->filesize;
    //uart_puts("proc fin");

    return 0;

}


int fork_process(uint64_t k_sp){
    uint64_t k_lr;
    asm volatile("mov %0, lr":"=r"(k_lr));

    // get parent proc memory layout
    Process *p_proc = get_current_proc();

    thread* parent = current_thread();

    thread* child = thread_create(k_lr, NULL, k_sp, parent->priority); // cp kernel stack
    int c_tpid = child->tpid;

    
    // eret to same elr
    sys_reg *p_sysreg = get_current_sysreg();
    sys_reg *c_sysreg = get_sysreg(c_tpid);
    

    // user stack copy
    uint64_t c_u_sp = cp_stack(child->u_stack, parent->u_stack, p_sysreg->sp_el0); 

    cp_sig_handler(parent->tpid, child->tpid);

    // setup el0 sys reg
    c_sysreg->elr = p_sysreg->elr;
    c_sysreg->spsr = p_sysreg->spsr;
    c_sysreg->sp_el0 = c_u_sp;
    
    // Internel data
    Process child_proc = {
        .start = (uint64_t)p_proc->start, // run same text section
        .size = p_proc->size,
    };
    proc_table[c_tpid] = child_proc;

    return c_tpid;
}

Process* get_current_proc(){
    Process *current_proc = &(proc_table[current_thread()->tpid]);
    return current_proc;
}

Process* get_init_proc(){
    Process *current_proc = &proc_table[0];
    return current_proc;
}

void from_el1_to_el0(uint64_t lr, uint64_t sp){
    if (lr==0)
        asm volatile("mov %0, lr":"=r"(lr));
    asm volatile("msr elr_el1, %0"::"r"(lr));

    asm volatile("msr sp_el0, %0"::"r"(sp));
    asm volatile("msr spsr_el1, %0"::"r"(0x340)); // clear IRQ mask, el0
    
    asm volatile("eret");
}