#include "include/process.h"



int load_new_image(char *name, char *const argv[]){


    cpio_ext* image = cpio_search(name);
    if (image==NULL) 
        return -1;


    void *text_pa = malloc(image->filesize);

    int size = image->filesize/PAGE_SIZE + (image->filesize%PAGE_SIZE>0?1:0);

    thread* exec_thread = current_thread();
    pagetable u_PGD = exec_thread->u_PGD;
    mappages(u_PGD, VM_TEXT, (uint64_t)size, (uint64_t)text_pa, true);
    memcpy(KERNEL_SPACE(text_pa), image->file, image->filesize);


    exec_thread->proc_size = image->filesize;
    
    free(exec_thread->u_text_pa);
    exec_thread->u_text_pa = text_pa;

    return 0;

}


int fork_process(uint64_t k_sp){
    uint64_t k_lr;
    asm volatile("mov %0, lr":"=r"(k_lr));


    thread* parent = current_thread();

    uint64_t sp_off = (parent->k_stack_pa+STACK_SIZE)-(k_sp-VM_KERNEL);
    thread* child = thread_create(k_lr, NULL, sp_off, parent->priority); 
    
    // copy thread structure    
    memcpy(&(child->sys_reg), &(parent->sys_reg), sizeof(sys_reg));
    memcpy(child->registered_handler, parent->registered_handler, sizeof(child->registered_handler));

    // copy kernel stack
    memcpy(KERNEL_SPACE(child->k_stack_pa), KERNEL_SPACE(parent->k_stack_pa), STACK_SIZE);
    // copy user stack
    memcpy(KERNEL_SPACE(child->u_stack_pa), KERNEL_SPACE(parent->u_stack_pa), STACK_SIZE);

    // copy text
    void *text_pa = malloc(parent->proc_size);
    memcpy(KERNEL_SPACE(text_pa), VM_TEXT, parent->proc_size);
    int size = parent->proc_size/PAGE_SIZE + (parent->proc_size%PAGE_SIZE>0?1:0);
    mappages(child->u_PGD, VM_TEXT, size, text_pa, true);

    memcpy(child->fd_table, current_thread()->fd_table, sizeof(child->fd_table)); // cp fd_table

    
    thread_enqueue_run_queue(child);
    return  child->tpid;
}


void from_el1_to_el0(){
    asm volatile("msr elr_el1, %0"::"r"(VM_TEXT));
    asm volatile("msr sp_el0, %0"::"r"(VM_STACK_BOTTOM));
    asm volatile("msr spsr_el1, %0"::"r"(0x340)); // clear IRQ mask, el0
    asm volatile("eret");
}