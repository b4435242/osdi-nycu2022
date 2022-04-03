#include "../lib/include/uart.h"
#include "../lib/include/fdt.h"
#include "../lib/include/shell.h"
#include "../lib/include/irq.h"
void main()
{
    register unsigned long x20 asm("x20");
    unsigned long DTB_BASE = x20;
    // set up serial console
    uart_init();    

    enable_int();

    uart_puts("[kernel] DTB_BASE ");
    uart_hex(DTB_BASE);
    uart_puts("\n \r");

    fdt_traverse((fdt_header*)(DTB_BASE), initramfs_callback);

    shell();
}