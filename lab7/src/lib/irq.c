#include "include/irq.h"

void enable_int(){
    asm("msr daifclr, 0xf");

}

void disable_int(){
    asm("msr daifset, 0xf");
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
    //save_sys_regs();

    unsigned int irq = *((unsigned int*)INT_SOURCE_0);
    if ((irq&CNTPNSIRQ)>0){
        mask_timer_int();
        add_task(core_timer_handler, 0);
        unmask_timer_int();
    } else if ((irq&GPUINTERRUPT)>0){
        unsigned int gpu_irq = *((unsigned int*)IRQpending1);
        if ((gpu_irq&AUX_GPU_SOURCE)>0){
            //mask_aux_int(); NOT WORKING
            uart_disable_recv_int();
            uart_disable_transmit_int();
            add_task(aux_handler, 0);
            //uart_enable_recv_int();
            //unmask_aux_int();
        }
    }

    //load_sys_regs();
}

