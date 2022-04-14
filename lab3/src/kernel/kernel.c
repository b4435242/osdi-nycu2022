#include "../lib/include/uart.h"
#include "../lib/include/fdt.h"
#include "../lib/include/shell.h"
#include "../lib/include/irq.h"
#include "../lib/include/mm.h"
#include "../lib/include/stdlib.h"

#define FDT_CALLBACK_SIZE 2

void main()
{
    register uint64_t x20 asm("x20");
    uint64_t DTB_BASE = x20;
    // set up serial console
    uart_init();    

    enable_int();

    uart_puts("[kernel] DTB_BASE ");
    uart_hex(DTB_BASE);
    uart_puts("\n \r");

    fdt_callback callbacks[FDT_CALLBACK_SIZE] = {initramfs_callback, memory_callback};
    fdt_traverse((fdt_header*)(DTB_BASE), callbacks, FDT_CALLBACK_SIZE);

    startup_alloc(DTB_BASE);

    shell();
}