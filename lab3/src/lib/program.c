#include "include/program.h"

void user_program(){
    asm(
        "mov x0, 0;"
    "1:;"
        "add x0, x0, 1;"
        "svc 0;"
        "cmp x0, 5;"
        "blt 1b;"
    "1:;"
        "b 1b;"
    );
}

void exec_user_program(){
    //load
    uart_puts("start\r\n");

    char *p = (char*)0x100000;
    
    char name[17] = "user_program.img\0";
    extraction program = cpio_search(name);
    //if (!res) return;
    for(int i=0; i<program.filesize; i++){
        p[i] = program.file[i];
    }

    uart_puts("prepare program\r\n");
    el1_to_el0((unsigned long long)p, (unsigned long long)p, 0x3c0);
    
}

void el1_to_el0(unsigned long long lr, unsigned long long sp, unsigned long long spsr){
    volatile register unsigned long long x10 asm("x10");
    volatile register unsigned long long x11 asm("x11");
    volatile register unsigned long long x12 asm("x12");

    x10 = lr;
    if (lr!=0)
        asm volatile("msr elr_el1, x10");
    else
        asm volatile("msr elr_el1, lr");

    x11 = sp;
    asm volatile("msr sp_el0, x11");
    x12 = spsr;
    asm volatile("msr spsr_el1, x12");
    uart_puts("el1_to_el0\r\n");

    asm volatile("eret");
}