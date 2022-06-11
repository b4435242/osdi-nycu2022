#include "../lib/include/uart.h"



void load_kernel(){

    register uint64_t x20 asm("x20");
    uint64_t DTB_BASE = x20;

    unsigned int kernel_size = 0;
    char* p = (char*)0x80000;

    uart_init();    
    //uart_puts("[Uartboot] Waiting for kernel image.\n\r");

    // There are some weird bytes in uart register on Rpi3
    // Use tricks below to filter them
    char start_code[5]="hello";
    for(int i=0; i<5; i++){
        char b = uart_getc();
        if (b!=start_code[i]) 
            i--;
    }

    //uart_puts("[Uartboot] Receive start code\n\r"); 


    for (int i = 0; i < 4; i++) {
        char b = uart_getc();
        kernel_size = kernel_size << 8;
        kernel_size = kernel_size | b;
    }

    //uart_puts("[Uartboot] Receive kernel size "); 
    //uart_hex(kernel_size);
    //uart_puts(" bytes\n\r");


    for (int i = 0; i < kernel_size; i++) {
        p[i] = uart_getc();
    }
    uart_puts("[Uartboot] Received all kernel image.\n\r");

    void (*kernel_boot)(uint64_t) = (void *)p;
    kernel_boot(DTB_BASE);

}