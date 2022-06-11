#include "include/thread.h"

thread dead_queue;
thread *exec_thread;
thread run_queue;
thread wait_queue;
char tpid_table[MAX_THREAD]; // 0 ~ MAX_THREAD-1
thread* threads[MAX_THREAD];

#define TPID_OFFS(x) KERNEL_SPACE(x * THREAD_BLOCK_SIZE)

void thread_init(){ // Call by kernel
    memset(tpid_table, PID_FREE, sizeof(tpid_table));
    tpid_table[0] = PID_TAKEN; // tpid 0 is taken by current

    //uint64_t k_stack = (uint64_t)malloc(STACK_SIZE);
    uint64_t k_stack_pa;
    asm volatile("ldr %0, =_boot":"=r"(k_stack_pa));
    k_stack_pa -= STACK_SIZE;
    k_stack_pa -= VM_KERNEL;
    
    uint64_t u_stack = (uint64_t)malloc(STACK_SIZE);

    uint64_t k_PGD = KERNEL_SPACE(KERNEL_PGD_PA);
    pagetable u_PGD = k_calloc(PAGE_SIZE);

    //mappages((pagetable)k_PGD, VM_STACK_HEAD, 4, k_stack_pa, false);
    mappages((pagetable)u_PGD, VM_STACK_HEAD, 4, u_stack, true);
    // map sp to base 0xfffffffff000
    /*uint64_t old_sp;
    asm volatile("mov %0, sp":"=r"(old_sp));
    uint64_t new_sp = KERNEL_SPACE(VM_STACK_HEAD+(old_sp-VM_KERNEL-k_stack_pa));
    asm volatile("mov sp, %0"::"r"(new_sp));*/
   

    exec_thread = Thread(0, 0, k_stack_pa, u_stack, NULL, (pagetable)k_PGD, u_PGD);
    PGD0_switch(exec_thread);
    threads[0] = exec_thread;
}




thread* thread_create(thread_callback callback, char **argv, uint64_t k_sp_off, uint32_t priority){


    int tpid = get_free_tpid();
    
    pagetable u_PGD = k_calloc(PAGE_SIZE);
    uint64_t u_stack = (uint64_t)malloc(STACK_SIZE);
    uint64_t k_stack = (uint64_t)malloc(STACK_SIZE);

    memset(u_PGD, 0, PAGE_SIZE);
    mappages(u_PGD, VM_STACK_HEAD, 4, u_stack, true);
    mapblocks_2mb(u_PGD, 0x3c000000, 8, 0x3c000000, true); // Identity Paging for Video Core, 0x3c000000~0x3f000000, 16MB

    uint64_t c_k_sp = KERNEL_SPACE(k_stack + STACK_SIZE - k_sp_off);

    uint64_t lr = (uint64_t)callback;
    uint64_t tpid_offset = TPID_OFFS(tpid);
    asm volatile("mov x10, %0"::"r"(tpid_offset));
    asm volatile("str %0, [x10, 16 * 5]"::"r"(c_k_sp));
    asm volatile("str %0, [x10, 16*5 + 8]"::"r"(lr)); 
    asm volatile("str %0, [x10, 16 * 6]"::"r"(c_k_sp)); // switch_to sp+0x10
    

    thread* t = Thread(tpid, priority, k_stack, u_stack, NULL, current_thread()->k_PGD, u_PGD);
    memcpy(t->fd_table, current_thread()->fd_table, 3*sizeof(file*)); // cp 0 1 2
    threads[tpid] = t;

    return t;
}



void thread_exit(){
    thread_enqueue(&dead_queue, exec_thread); // wait for idle thread to free
    exec_thread = NULL; // DO NOT put back to run queue
    schedule();
}

void kill_thread(int tpid){
    thread* del = thread_dequeue_by_id(&run_queue, tpid);
    if (del==NULL)
        del = thread_dequeue_by_id(&wait_queue, tpid);
}

void periodical_schedule(){
    int freq;
    asm volatile("mrs %0, cntfrq_el0":"=r"(freq));
    add_timer(periodical_schedule, NULL, freq>>5);
    schedule();
}

void schedule(){
    uint64_t prev_offset = 0;
    if (exec_thread!=NULL)
        prev_offset = TPID_OFFS(exec_thread->tpid);
    thread* first = thread_dequeue(&run_queue);
    if (first!=NULL) {
        if (exec_thread!=NULL)
            thread_enqueue(&run_queue, exec_thread);
        exec_thread = first;
        uint64_t next_offset = TPID_OFFS(first->tpid);
        
        uint64_t sp;
        asm volatile("mov %0, sp":"=r"(sp));
        switch_to(prev_offset, next_offset, sp);
    } else { // When there are no other runnable threads, pick idle thread
        thread* t = thread_create(idle, NULL, 0, 99); // lowest priority
        thread_enqueue_run_queue(t);
        schedule();
    }
}

void thread_enqueue_run_queue(thread* t){
    thread_enqueue(&run_queue, t);
}

__attribute__ ((always_inline)) void PGD1_switch(thread* exec){
    asm volatile("mov x0, %0"::"r"(exec->k_PGD));
    asm volatile(
        "dsb ish;"  // ensure write has completed
        "msr ttbr1_el1, x0;"  // switch translation based address.
        "tlbi vmalle1is;" // invalidate all TLB entries
        "dsb ish;" // ensure completion of TLB invalidatation
        "isb;"  // clear pipeline
    );
}

__attribute__ ((always_inline)) void PGD0_switch(thread* exec){
    asm volatile("mov x0, %0"::"r"(exec->u_PGD));
    asm volatile(
        "dsb ish;"  // ensure write has completed
        "msr ttbr0_el1, x0;"  // switch translation based address.
        "tlbi vmalle1is;" // invalidate all TLB entries
        "dsb ish;" // ensure completion of TLB invalidatation
        "isb;"  // clear pipeline
    );
}

void switch_to(uint64_t prev, uint64_t next, uint64_t k_sp){ // tpid_offset
    asm volatile("mov x10, %0"::"r"(prev));
    asm volatile("mov x11, %0"::"r"(next));
    asm volatile("mov x9, %0"::"r"(k_sp));
    if (prev!=0)
        asm volatile(
            "stp x19, x20, [x10, 16 * 0];"
            "stp x21, x22, [x10, 16 * 1];"
            "stp x23, x24, [x10, 16 * 2];"
            "stp x25, x26, [x10, 16 * 3];"
            "stp x27, x28, [x10, 16 * 4];"
            "stp fp, lr, [x10, 16 * 5];"
            "str x9, [x10, 16 * 6];"
        );
    
    PGD0_switch(exec_thread); 

    /* NO STACK DATA CAN BE USED AFTER */
    
    asm volatile(
        "ldp x19, x20, [x11, 16 * 0];"
        "ldp x21, x22, [x11, 16 * 1];"
        "ldp x23, x24, [x11, 16 * 2];"
        "ldp x25, x26, [x11, 16 * 3];"
        "ldp x27, x28, [x11, 16 * 4];"
        "ldp fp, lr, [x11, 16 * 5];"
        "ldr x9, [x11, 16 * 6];"
        "mov sp,  x9;"
        "msr tpidr_el1, x1;"
        "ret"
    );
}



thread* Thread(int tpid, uint32_t priority, uint64_t k_stack_pa, uint64_t u_stack_pa, uint64_t u_text_pa, pagetable k_PGD, pagetable u_PGD){
    thread *t = k_calloc(sizeof(thread));
    t->tpid = tpid;
    t->k_stack_pa = k_stack_pa;
    t->u_stack_pa = u_stack_pa;
    t->u_text_pa = u_text_pa;
    t->u_PGD = u_PGD;
    t->k_PGD = k_PGD;
    t->priority = priority;
    return t;
}

/* head is NOT NULL */
void thread_enqueue(thread* head, thread* p){
    thread *cur = head->next;
    thread *prev = head;
    while (cur!=NULL && p->priority>=cur->priority) 
    {
        prev = cur;
        cur = cur->next;
    }

    // prev <=> p <=> cur 
    prev->next = p;
    p->prev = prev;
    p->next = cur;
    if (cur!=NULL)
        cur->prev = p;
    
}

thread* thread_dequeue(thread* head){
    thread *first = head->next;
    if (first!=NULL){
        // prev <=> p <=> next 
        thread* prev = first->prev;
        thread* next = first->next;

        // prev <=> next 
        prev->next = next;
        if (next!=NULL)
            next->prev = prev;
    }
    return first;
}

thread* thread_dequeue_by_id(thread* head, int tpid){
    thread *del = head->next;
    while (del!=NULL && del->tpid!=tpid)
    {
        del = del->next;
    }
    if (del!=NULL){
        // prev <=> p <=> next 
        thread* prev = del->prev;
        thread* next = del->next;

        // prev <=> next 
        prev->next = next;
        if (next!=NULL)
            next->prev = prev;
    }
    return del;
}

thread* current_thread(){
    return exec_thread;
}

thread* get_thread(int tpid){
    return threads[tpid];
}

void tpid_offset_init(){
    asm volatile("msr tpidr_el1, %0"::"r"(0));
}

uint64_t get_current_tpid_offset(){
    uint64_t res;
    asm volatile("mrs %0, tpidr_el1":"=r"(res));
    return res;
}

int get_free_tpid(){
    for(int i=0; i<MAX_THREAD; i++){
        if (tpid_table[i]==PID_FREE){
            tpid_table[i] = PID_TAKEN;
            return i;
        }
    }
    // Assume the case will Not happen
    return MAX_THREAD; // illegal => running out of tpid
}

void set_tpid_free(int tpid){
    tpid_table[tpid] = PID_FREE;
}

void idle(){
    while (1)
    {
        kill_zombies();
        schedule();
    }
    
}

void kill_zombies(){
    thread* zombie;
    while ((zombie=thread_dequeue(&dead_queue))!=NULL)
    {
        // free stack
        free((void*)zombie->k_stack_pa);
        free((void*)zombie->u_stack_pa);
        // free thread data struct
        free(zombie);
        // set tpid available
        set_tpid_free(zombie->tpid);
    }
    
}





void save_sys_regs(){
    sys_reg *reg = &(current_thread()->sys_reg);
    asm volatile("mrs %0, elr_el1":"=r"(reg->elr));
    asm volatile("mrs %0, spsr_el1":"=r"(reg->spsr));
    asm volatile("mrs %0, sp_el0":"=r"(reg->sp_el0));

    uint64_t ttbr0;
    asm volatile("mrs %0, ttbr0_el1":"=r"(ttbr0));
   
    
}

void load_sys_regs(){
    sys_reg *reg = &(current_thread()->sys_reg);

    asm volatile("msr elr_el1, %0"::"r"(reg->elr));
    asm volatile("msr spsr_el1, %0"::"r"(reg->spsr));
    asm volatile("msr sp_el0, %0"::"r"(reg->sp_el0));

    uint64_t ttbr0;
    asm volatile("mrs %0, ttbr0_el1":"=r"(ttbr0));
    
}

void show_sys_regs(){
    sys_reg *reg = &(current_thread()->sys_reg);

    uart_puts("spsr, elr, esr = ");
    uart_hex(reg->spsr);
    uart_puts(" ");
    uart_hex(reg->elr);
    uart_puts("\r\n");

}

