#ifndef __TIMER__
#define __TIMER__

#include "uart.h"
#include "stdlib.h"
#include "mmio.h"

#define CORE0_TIMER_IRQ_CTRL 0x40000040


typedef struct _Timer {
    unsigned long long ticks;
    callback_ptr callback;
    char buf[100];
} Timer;

void core_timer_enable();
void core_timer_disable();
void mask_timer_int();
void unmask_timer_int();
void set_expire_time(int seconds);
void core_timer_handler();
void add_timer(callback_ptr callback, void* arg, unsigned long long seconds);
void alert_seconds();
void exec_timer_callback();
#endif 