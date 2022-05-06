#include "include/timer.h"

Timer timer_queue;

void core_timer_enable(int ticks){
    
    //asm("msr spsr_el1, xzr"); // EL0

    set_expire_time(ticks);
    asm volatile("msr cntp_ctl_el0, %0"::"r"(1)); // enable
    unmask_timer_int();
    // cpu timer 
    uint64_t tmp;
    asm volatile("mrs %0, cntkctl_el1":"=r"(tmp));
    tmp |= 1;
    asm volatile("msr cntkctl_el1, %0"::"r"(tmp));
        
}

void core_timer_disable(){
    asm volatile("msr cntp_ctl_el0, %0"::"r"(0)); // disable
    mask_timer_int();
}

void mask_timer_int(){
    //*((volatile unsigned int*)CORE0_TIMER_IRQ_CTRL) = 0; // mask timer interrupt
    mmio_put(CORE0_TIMER_IRQ_CTRL, 0);

}
void unmask_timer_int(){
    //*((volatile unsigned int*)CORE0_TIMER_IRQ_CTRL) = 2; // unmask timer interrupt
    mmio_put(CORE0_TIMER_IRQ_CTRL, 2);
}

void set_expire_time(int ticks){
    asm volatile("msr cntp_tval_el0, %0"::"r"(ticks));  // set expired time
    
}

void core_timer_handler(){
    exec_timer_callback();
}

void alert_seconds(){
    int count, frq;
    asm volatile("mrs %0, cntpct_el0":"=r"(count));
    asm volatile("mrs %0, cntfrq_el0":"=r"(frq));
    int seconds = count/frq;
    uart_hex(seconds);
    uart_puts("\n\r");

    asm("msr spsr_el1, xzr");
    add_timer_with_second(alert_seconds, NULL, 2);

}

void add_timer_with_second(callback_ptr callback, void* arg, int seconds){
    int frq;
    asm volatile("mrs %0, cntfrq_el0":"=r"(frq));
    int ticks = seconds * frq;
    add_timer(callback, arg, ticks);
}

void add_timer(callback_ptr callback, void* arg, int ticks){ 
    // Init timer
    Timer *timer = malloc(sizeof(Timer));
    timer->arg = arg;
    timer->callback = callback;
    timer->ticks = ticks;
    timer->next = NULL;
    timer->prev = NULL;


    mask_timer_int();

    Timer* first;
    if ((first=timer_queue_top(&timer_queue))==NULL){
        timer_enqueue(&timer_queue, timer, 0);
        core_timer_enable(ticks);
    } else {
        int count;
        asm("mrs %0, cntp_tval_el0":"=r"(count));
        int elapsed = first->ticks - count;
        uint32_t idx = timer_enqueue(&timer_queue, timer, elapsed);

        // reprogram physical timer if there comes a shorter timer
        if (idx == 0){
            reprogram_physical_timer(&timer_queue);
            update_ticks(timer_queue.next, elapsed);  // update the ticks of the rest, excluding the first
        }
    }

    unmask_timer_int();
}

void exec_timer_callback(){
    

    Timer* timer = timer_dequeue(&timer_queue);
    if (timer==NULL) return;

    // update the rest of existing timer
    int elapsed = timer->ticks;
    update_ticks(&timer_queue, elapsed);
    reprogram_physical_timer(&timer_queue);


    timer->callback(timer->arg);     
    free(timer);

    

}

void reprogram_physical_timer(Timer *timer_queue){
    Timer *next = timer_queue_top(timer_queue);
    if (next!=NULL){
        asm volatile("msr cntp_tval_el0, %0"::"r"(next->ticks));
    } else {
        core_timer_disable();
    }
}

void delay(int ticks){
    int flag = 1;
    add_timer(delay_callback, &flag, ticks);
    while (flag);
}

void delay_callback(int *flag){
    *flag = 0;
}


uint32_t timer_enqueue(Timer* head, Timer* p, int elapsed){
    uint32_t index = 0;
    Timer *cur = head->next;
    Timer *prev = head;
    while (cur!=NULL && p->ticks>=cur->ticks-elapsed) 
    {
        prev = cur;
        cur = cur->next;
        index++;
    }

    // prev <=> p <=> cur 
    prev->next = p;
    p->prev = prev;
    p->next = cur;
    if (cur!=NULL)
        cur->prev = p;


    return index;
}

Timer* timer_dequeue(Timer* head){
    Timer *first = head->next;
    if (first!=NULL){
        // prev <=> p <=> next 
        Timer* prev = first->prev;
        Timer* next = first->next;

        // prev <=> next 
        prev->next = next;
        next->prev = prev;

    }
    return first;
}

Timer* timer_queue_top(Timer* head){
    Timer *first = head->next;
    return first;
}

void update_ticks(Timer* head, uint32_t elapsed){ // head will not be updated
    Timer *p = head->next;
    while (p!=NULL)
    {
        p->ticks -= elapsed;
        if (p->ticks<0) p->ticks = 0;
        p = p->next;
    }
    
}