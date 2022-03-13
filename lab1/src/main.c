#include "uart.h"
#include "lib.h"
#include "mbox.h"
#include "reset.h"

void main()
{
    // set up serial console
    uart_init();    

    // Mailbox
    get_board_revision();
    uart_puts("\r\nboard revision is ");
    uart_hex(mailbox[5]);
    uart_puts("\r\n");

    get_arm_memory();
    uart_puts("ARM memory base address is ");
    uart_hex(mailbox[5]);
    uart_puts("\r\n");
    uart_puts("ARM memory size is ");
    uart_hex(mailbox[6]);
    uart_puts("\n\r");

   

    char buf[100];
    char c;
    // echo everything back
    while(1) {
        int i=0;
        _memset(buf, 0, sizeof(buf));
        do {
            c = uart_getc();
            if(c<=127){
                uart_send(c);
                //uart_hex(c);
                buf[i++] = c;
            }
        } while (c!='\n');
        uart_puts("\r");
        if (_strncmp(buf, "help", 4)==0){
            uart_puts("help      : print this help menu\r\nhello     : print Hello World!\r\nreboot    : reboot the device\r\n");
        } else if (_strncmp(buf, "hello", 5)==0){
            uart_puts("Hello World!\r\n");
        } else if (_strncmp(buf, "reboot", 6)==0){
            uart_puts("\r");
            reset(1);
        }

    }
}