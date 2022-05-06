#ifndef __TIMER__
#define __TIMER__

#include "uart.h"
#include "stdlib.h"
#include "mmio.h"
#include "mm.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040

typedef struct Timer Timer;
struct Timer {
    int ticks;
    callback_ptr callback;
    void *arg;
    Timer *next;
    Timer *prev;
};

void core_timer_enable();
void core_timer_disable();
void mask_timer_int();
void unmask_timer_int();
void set_expire_time(int ticks);
void core_timer_handler();
void add_timer(callback_ptr callback, void* arg, int ticks);
void add_timer_with_second(callback_ptr callback, void* arg, int seconds);
void alert_seconds();
void exec_timer_callback();

uint32_t timer_enqueue(Timer* head, Timer* p, int elapsed);
Timer* timer_dequeue(Timer* head);
Timer* timer_queue_top(Timer* head);
void update_ticks(Timer* head, uint32_t elapsed);

void delay(int ticks);
void delay_callback(int *flag);



#endif 