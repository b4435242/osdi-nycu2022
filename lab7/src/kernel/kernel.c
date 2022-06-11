#include "../lib/include/uart.h"
#include "../lib/include/fdt.h"
#include "../lib/include/shell.h"
#include "../lib/include/irq.h"
#include "../lib/include/mm.h"
#include "../lib/include/stdlib.h"
#include "../lib/include/thread.h"
#include "../lib/include/process.h"
#include "../lib/include/vm.h"
#include "../lib/include/initramfs.h"
#include "../lib/include/tmpfs.h"
#include "../lib/include/sd_driver.h"


#define FDT_CALLBACK_SIZE 2

void main()
{
    uint64_t DTB_BASE;
    asm volatile("mov %0, x20":"=r"(DTB_BASE));
    DTB_BASE = KERNEL_SPACE(DTB_BASE);

    // set up serial console
    uart_init();    

    enable_int();

    uart_puts("[kernel] DTB_BASE ");
    uart_hex(DTB_BASE);
    uart_puts("\n \r");

    fdt_callback callbacks[FDT_CALLBACK_SIZE] = {initramfs_callback, memory_callback};
    fdt_traverse((fdt_header*)(DTB_BASE), callbacks, FDT_CALLBACK_SIZE);

    startup_alloc(DTB_BASE);
    
    thread_init();

    // fs
    tmpfs_init();
    initramfs_init();
    uart_dev_init();
    fat32_init();
 

    tpid_offset_init();
    sig_init();
    periodical_schedule();

    shell();
}