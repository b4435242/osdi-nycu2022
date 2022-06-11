#include "include/sig.h"

sig signal_queues[MAX_THREAD];
context contexts[MAX_THREAD];

sig_callback default_handler[MAX_SIG];

void sig_init(){
    
    default_handler[SIGKILL] = thread_exit;
    

}

void sig_handler(){
    thread* exec_thread = current_thread();
    int curr = exec_thread->tpid;
    sig* q = &signal_queues[curr];
    sig* s;
    while ((s=sig_dequeue(q))!=NULL)
    {
        int SIG = s->SIG;
        sig_callback reg_callback = exec_thread->registered_handler[SIG];
        if (reg_callback>0){
        
            uint64_t handler_stack = malloc(STACK_SIZE);
            uint64_t ret_sp, ret_fp;
            asm volatile("mov %0, sp":"=r"(ret_sp));
            asm volatile("mov %0, fp":"=r"(ret_fp));

            mappages(exec_thread->u_PGD, VM_HANDLER_STACK_HEAD, 4, handler_stack, true);

            call_regis_handler(reg_callback, VM_HANDLER_STACK_BOTTOM, ret_sp, ret_fp);

            free(VM_HANDLER_STACK_HEAD);
            
        } else {
            default_handler[SIG]();
        }
    }
    
}

void set_sig_handler(int SIG, uint64_t handler){
    int curr = current_thread()->registered_handler[SIG] = handler;
}



void call_regis_handler(uint64_t handler, uint64_t sp_el0, uint64_t ret_sp, uint64_t ret_fp){
    save_context(ret_sp, ret_fp);

    // -> el0
    asm volatile("mov lr, %0"::"r"(sigreturn));
    asm volatile("msr elr_el1, %0"::"r"(handler));
    asm volatile("msr sp_el0, %0"::"r"(sp_el0));
    asm volatile("msr spsr_el1, %0"::"r"(0x340)); // clear IRQ mask, el0
    asm volatile("eret");
}

void add_sig_to_proc(int pid, int SIG){
    sig* q_head = &signal_queues[pid];
    sig* _sig = k_malloc(sizeof(sig));
    
    _sig->SIG = SIG;
    _sig->next = NULL;
    _sig->prev = NULL;
    sig_enqueue(q_head, _sig);
}

void resume_sig_handler(){
    load_context();
    asm volatile("ret");
}


inline void save_context(uint64_t ret_sp, uint64_t ret_fp){
    uint64_t lr;
    asm volatile("mov %0, lr":"=r"(lr));

    int curr = current_thread()->tpid;
    context *c = &contexts[curr];

    asm volatile("mov %0, x19":"=r"(c->gen_regs[0]));
    asm volatile("mov %0, x20":"=r"(c->gen_regs[1]));
    asm volatile("mov %0, x21":"=r"(c->gen_regs[2]));
    asm volatile("mov %0, x22":"=r"(c->gen_regs[3]));
    asm volatile("mov %0, x23":"=r"(c->gen_regs[4]));
    asm volatile("mov %0, x24":"=r"(c->gen_regs[5]));
    asm volatile("mov %0, x25":"=r"(c->gen_regs[6]));
    asm volatile("mov %0, x26":"=r"(c->gen_regs[7]));
    asm volatile("mov %0, x27":"=r"(c->gen_regs[8]));
    asm volatile("mov %0, x28":"=r"(c->gen_regs[9]));
    //asm volatile("mov %0, fp":"=r"(c->gen_regs[10]));
    //asm volatile("mov %0, lr":"=r"(c->gen_regs[11]));
    //asm volatile("mov %0, sp":"=r"(c->gen_regs[12]));
    c->gen_regs[10] = ret_fp;
    c->gen_regs[11] = lr;
    c->gen_regs[12] = ret_sp;


    //asm volatile("mrs %0, spsr_el1":"=r"(c->sys_regs->spsr));
    //asm volatile("mrs %0, sp_el0":"=r"(c->sys_regs->sp_el0));
    //asm volatile("mrs %0, elr_el1":"=r"(c->sys_regs->elr));
}


inline void load_context(){
    int curr = current_thread()->tpid;
    context *c = &contexts[curr];

    asm volatile("mov x19, %0"::"r"(c->gen_regs[0]));
    asm volatile("mov x20, %0"::"r"(c->gen_regs[1]));
    asm volatile("mov x21, %0"::"r"(c->gen_regs[2]));
    asm volatile("mov x22, %0"::"r"(c->gen_regs[3]));
    asm volatile("mov x23, %0"::"r"(c->gen_regs[4]));
    asm volatile("mov x24, %0"::"r"(c->gen_regs[5]));
    asm volatile("mov x25, %0"::"r"(c->gen_regs[6]));
    asm volatile("mov x26, %0"::"r"(c->gen_regs[7]));
    asm volatile("mov x27, %0"::"r"(c->gen_regs[8]));
    asm volatile("mov x28, %0"::"r"(c->gen_regs[9]));
    asm volatile("mov fp, %0"::"r"(c->gen_regs[10]));
    asm volatile("mov lr, %0"::"r"(c->gen_regs[11]));
    asm volatile("mov sp, %0"::"r"(c->gen_regs[12]));
    
    //asm volatile("msr spsr_el1, %0"::"r"(c->sys_regs->spsr));
    //asm volatile("msr sp_el0, %0"::"r"(c->sys_regs->sp_el0));
    //asm volatile("msr elr_el1, %0"::"r"(c->sys_regs->elr));
}


/* head is NOT NULL */
void sig_enqueue(sig* head, sig* p){
    sig *cur = head->next;
    sig *prev = head;

    // prev <=> p <=> cur 
    prev->next = p;
    p->prev = prev;
    p->next = cur;
    if (cur!=NULL)
        cur->prev = p;
    
}

sig* sig_dequeue(sig* head){
    sig *first = head->next;
    if (first!=NULL){
        // prev <=> p <=> next 
        sig* prev = first->prev;
        sig* next = first->next;

        // prev <=> next 
        prev->next = next;
        if (next!=NULL)
            next->prev = prev;
    }
    return first;
}

sig* sig_queue_top(sig* head){
    sig *first = head->next;
    return first;
}

