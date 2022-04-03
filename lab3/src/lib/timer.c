#include "include/timer.h"

Timer timer_queue[100];
int timer_queue_size = 0;

void core_timer_enable(int seconds){
    
    //asm("msr spsr_el1, xzr"); // EL0

    set_expire_time(seconds);
    asm volatile(
        "mov x0, 1;"
        "msr cntp_ctl_el0, x0;" // enable
    );
    unmask_timer_int();
        
}

void core_timer_disable(){
    asm volatile(
        "mov x0, 0;"
        "msr cntp_ctl_el0, x0;" // disable
    );
}

void mask_timer_int(){
    //*((volatile unsigned int*)CORE0_TIMER_IRQ_CTRL) = 0; // mask timer interrupt
    mmio_put(CORE0_TIMER_IRQ_CTRL, 0);

}
void unmask_timer_int(){
    //*((volatile unsigned int*)CORE0_TIMER_IRQ_CTRL) = 2; // unmask timer interrupt
    mmio_put(CORE0_TIMER_IRQ_CTRL, 2);
}

void set_expire_time(int seconds){
    asm volatile("mrs x0, cntfrq_el0");
    register unsigned long long x0 asm ("x0");
    x0 *= seconds;
    asm volatile("msr cntp_tval_el0, x0");  // set expired time
    
}

void core_timer_handler(){
    exec_timer_callback();
}

void alert_seconds(){

    asm("mrs x1, cntpct_el0");
    asm("mrs x2, cntfrq_el0");
    register unsigned long long count asm("x1");
    register unsigned long long frq asm("x2");
    unsigned long long seconds = count/frq;
    uart_hex(seconds);
    uart_puts("\n\r");
    //frq *= 2; // set x2 to 2 seconds
    //asm("msr cntp_tval_el0, x2");
    asm("msr spsr_el1, xzr");
    add_timer(alert_seconds, NULL, 2);

}

void add_timer(callback_ptr callback, void* arg, unsigned long long seconds){ 
    // Init timer
    asm("mrs x0, cntfrq_el0");
    register unsigned long long x0 asm("x0");
    unsigned long long frq = x0;
    unsigned long long ticks = seconds * frq;


    Timer timer = {
        .ticks=ticks, 
        .callback=callback,
    };

    _memset(timer.buf, 0, sizeof(timer.buf));
    _strcpy(timer.buf, (char*)arg);


    if (timer_queue_size==0){
        timer_queue[0] = timer;
        timer_queue_size++;
        core_timer_enable(seconds);
    } else {

        asm("mrs x1, cntp_tval_el0");
        register unsigned long long x1 asm("x1");
        unsigned long long elapsed = timer_queue[0].ticks - x1;

        // insert to sorted queue, asending
        int idx = 0;
        while (idx<timer_queue_size && (timer_queue[idx].ticks-elapsed)<=timer.ticks) idx++;
        timer_queue_size++;
        for(int i=idx+1; i<timer_queue_size; i++){
            timer_queue[i] = timer_queue[i-1];
        }
        timer_queue[idx] = timer;
        // end of insertion

    
        // reprogram physical timer if first element is swaped after sorting
        if (idx == 0){
            // reprogram physical timer
            register unsigned long long x2 asm("x2");
            x2 = timer.ticks;
            asm("msr cntp_tval_el0, x2");
            // update the ticks of the rest
            for(int i=1; i<timer_queue_size; i++){
                timer_queue[i].ticks -= elapsed;
            }

        }
    }

    
}

void exec_timer_callback(){
    if (timer_queue_size>0){
        
        // update the rest of existing timer
        Timer timer = timer_queue[0];
        unsigned long long elapsed = timer.ticks;
        for(int i=0; i+1<timer_queue_size; i++){
            timer_queue[i] = timer_queue[i+1]; 
            timer_queue[i].ticks = timer_queue[i+1].ticks - elapsed;
        }
        timer_queue_size--;
        // exec callback after update for the reason that if callback includes add_timer,
        // it causes newly inserted timer is deducted elasped, which should not happen
        timer.callback(timer.buf);
    
    }
    if (timer_queue_size>0){
        // reprogram physical timer
        register unsigned long long x0 asm("x0");
        x0 = timer_queue[0].ticks;
        asm("msr cntp_tval_el0, x0");
    } else {
        core_timer_disable();
    }
}