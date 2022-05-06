#include "include/thread.h"

thread dead_queue;
thread *exec_thread;
thread run_queue;
thread wait_queue;
char tpid_table[MAX_THREAD]; // 0 ~ MAX_THREAD-1


sys_reg sys_regs[MAX_THREAD];


void thread_init(){ // Call by kernel
    _memset(tpid_table, PID_FREE, sizeof(tpid_table));
    tpid_table[0] = PID_TAKEN; // tpid 0 is taken by current
    uint64_t k_stack;
    asm volatile("ldr %0, =_start":"=r"(k_stack));
    exec_thread = Thread(0, 0, k_stack, 0);
}




thread* thread_create(thread_callback callback, char **argv, uint64_t p_k_sp, uint32_t priority){
    int tpid = get_free_tpid();
    uint64_t c_u_stack = (uint64_t)malloc(THREAD_SP_SIZE) + THREAD_SP_SIZE;
    uint64_t c_k_stack = (uint64_t)malloc(THREAD_SP_SIZE) + THREAD_SP_SIZE;

    thread* p_t = current_thread();
    uint64_t c_k_sp;
    if (p_k_sp>0){ // copy kernel stack 
        c_k_sp = cp_stack(c_k_stack, p_t->k_stack, p_k_sp);
    } else { // empty kernel
        c_k_sp = c_k_stack;
    }


    uint64_t lr = (uint64_t)callback;
    uint64_t tpid_offset = tpid * THREAD_BLOCK_SIZE;
    asm volatile("mov x10, %0"::"r"(tpid_offset));
    //asm volatile("str %0, [x10, 16 * 5]"::"r"(c_k_sp));
    asm volatile("str %0, [x10, 16*5 + 8]"::"r"(lr)); // x0 -> foo+88, why?
    asm volatile("str %0, [x10, 16 * 6]"::"r"(c_k_sp-0x10)); // switch_to sp+0x10
    

    thread* t = Thread(tpid, priority, c_k_stack, c_u_stack);
    
    thread_enqueue(&run_queue, t);
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
    uint64_t prev_offset = get_current_tpid_offset();
    thread* first = thread_dequeue(&run_queue);
    if (first!=NULL) {
        if (exec_thread!=NULL)
            thread_enqueue(&run_queue, exec_thread);
        exec_thread = first;
        uint64_t next_offset = first->tpid*THREAD_BLOCK_SIZE;
        switch_to(prev_offset, next_offset);
    } else { // When there are no other runnable threads, pick idle thread
        thread_create(idle, NULL, 0, 99); // lowest priority
        schedule();
    }
}

inline void switch_to(uint64_t prev, uint64_t next){ // tpid_offset
    asm volatile(
        "stp x19, x20, [x0, 16 * 0];"
        "stp x21, x22, [x0, 16 * 1];"
        "stp x23, x24, [x0, 16 * 2];"
        "stp x25, x26, [x0, 16 * 3];"
        "stp x27, x28, [x0, 16 * 4];"
        "stp fp, lr, [x0, 16 * 5];"
        "mov x9, sp;"
        "str x9, [x0, 16 * 6];"

        "ldp x19, x20, [x1, 16 * 0];"
        "ldp x21, x22, [x1, 16 * 1];"
        "ldp x23, x24, [x1, 16 * 2];"
        "ldp x25, x26, [x1, 16 * 3];"
        "ldp x27, x28, [x1, 16 * 4];"
        "ldp fp, lr, [x1, 16 * 5];"
        "ldr x9, [x1, 16 * 6];"
        "mov sp,  x9;"
        "msr tpidr_el1, x1;"
    );
}



thread* Thread(int tpid, uint32_t priority, uint64_t k_stack, uint64_t u_stack){
    thread *t = malloc(sizeof(thread));
    t->next = NULL;
    t->prev = NULL;
    t->tpid = tpid;
    t->k_stack = k_stack;
    t->u_stack = u_stack;
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
        next->prev = prev;
    }
    return del;
}

thread* current_thread(){
    return exec_thread;
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
        //uint64_t fp;
        //asm volatile("mov x0, %0"::"r"(zombie->tpid));
        //asm volatile("ldr %0, [x0, 16 * 5]":"=r"(fp));
        uint64_t k_stack_head = zombie->k_stack-THREAD_SP_SIZE;
        uint64_t u_stack_head = zombie->u_stack-THREAD_SP_SIZE;

        free((void*)k_stack_head);
        free((void*)u_stack_head);
        // free thread data struct
        free(zombie);
        // set tpid available
        set_tpid_free(zombie->tpid);
    }
    
}

uint64_t cp_stack(uint64_t c_stack, uint64_t p_stack, uint64_t p_sp){
    uint64_t stack_size = p_stack - p_sp; // bottom - sp
    uint64_t c_sp = c_stack - stack_size;
    memcpy((void*)c_sp, (void*)p_sp, (size_t)stack_size);
    return c_sp;
}




void save_sys_regs(){
    int curr = current_thread()->tpid;
    sys_reg *reg = &sys_regs[curr];
    asm volatile("mrs %0, elr_el1":"=r"(reg->elr));
    asm volatile("mrs %0, spsr_el1":"=r"(reg->spsr));
    asm volatile("mrs %0, sp_el0":"=r"(reg->sp_el0));

    
}

void load_sys_regs(){
    int curr = current_thread()->tpid;
    sys_reg *reg = &sys_regs[curr];
    asm volatile("msr elr_el1, %0"::"r"(reg->elr));
    asm volatile("msr spsr_el1, %0"::"r"(reg->spsr));
    asm volatile("msr sp_el0, %0"::"r"(reg->sp_el0));

}

void show_sys_regs(){
    int curr = current_thread()->tpid;
    sys_reg *reg = &sys_regs[curr];
    uart_puts("spsr, elr, esr = ");
    uart_hex(reg->spsr);
    uart_puts(" ");
    uart_hex(reg->elr);
    uart_puts("\r\n");

}

sys_reg* get_current_sysreg(){
    int curr = current_thread()->tpid;
    return &sys_regs[curr];
}

sys_reg* get_sysreg(int tpid){
    return &sys_regs[tpid];
}