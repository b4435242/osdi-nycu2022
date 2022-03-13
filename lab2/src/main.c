#include "uart.h"
#include "lib.h"
#include "mbox.h"
#include "reset.h"
#include "cpio.h"
#include "fdt.h"                        


void main()
{
    register unsigned long x0 asm("x0");
    unsigned long DTB_BASE = x0;
    // set up serial console
    uart_init();    
    

    uart_puts("[kernel] DTB_BASE ");
    uart_hex(DTB_BASE);
    uart_puts("\n \r");

    fdt_traverse((fdt_header*)(DTB_BASE), initramfs_callback);

    

    while(1) {
        char input[100];
        uart_puts("#");
        uart_getline(input);
        uart_puts("\r");
        if (!_strncmp(input, "help", 4)){
            uart_puts("help      : print this help menu\r\nhello     : print Hello World!\r\nreboot    : reboot the device\r\n");
        } else if (!_strncmp(input, "hello", 5)){
            uart_puts("Hello World!\r\n");
        } else if (!_strncmp(input, "reboot", 6)){
            uart_puts("\r");
            reset(1);
        } else if (!_strncmp(input, "ls", 2)) {
            cpio_ls();
        } else if (!_strncmp(input, "cat", 3)) {
            cpio_cat();
        } else if (!_strncmp(input, "malloc", 6)){
            char* str1=simple_malloc(30);
            char* str2=simple_malloc(20);
            str1 = "Hi! It is the first malloc\n";
            str2 = "The second malloc\n";
            uart_puts(str1);
            uart_puts(str2);
        } 
        

    }
}