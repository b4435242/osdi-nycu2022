#include "include/irq.h"

void enable_int(){
    asm("msr daifclr, 2");

}

void disable_int(){
    asm("msr daifset, 2");
}

void irq_handler(){
    unsigned int irq = *((unsigned int*)INT_SOURCE_0);
    if (irq==CNTPNSIRQ){
        core_timer_handler();
    } else if (irq==GPUINTERRUPT){
        unsigned int gpu_irq = *((unsigned int*)IRQpending1);
        if (gpu_irq==AUX_GPU_SOURCE)
            aux_handler();
    }
}

void concurrent_irq_handler(){
    unsigned int irq = *((unsigned int*)INT_SOURCE_0);
    if (irq==CNTPNSIRQ){
        mask_timer_int();
        add_task(core_timer_handler, 1);
        unmask_timer_int();
    } else if (irq==GPUINTERRUPT){
        unsigned int gpu_irq = *((unsigned int*)IRQpending1);
        if (gpu_irq==AUX_GPU_SOURCE){
            //uart_mask_aux(); //NOT WORKING!!! CANNOT UNMASK!!!
            uart_disable_recv_int();
            uart_disable_transmit_int();
            add_task(aux_handler, 0);
            uart_enable_recv_int();
            //uart_unmask_aux();
        }
    }
}

