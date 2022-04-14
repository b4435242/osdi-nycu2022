#include "include/shell.h"

void shell(){

    uart_unmask_aux();


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
            char* str1=heap_malloc(30);
            char* str2=heap_malloc(20);
            str1 = "Hi! It is the first malloc\n";
            str2 = "The second malloc\n";
            uart_puts(str1);
            uart_puts(str2);
        } else if (!_strncmp(input, "exec", 4)){
            exec_user_program();
        } else if (!_strncmp(input, "timer", 5)){
            add_timer(alert_seconds, NULL, 2);
        } else if (!_strncmp(input, "async", 5)){
            char p[1000];
            
            uart_unmask_aux();
            uart_enable_recv_int();
            
            add_timer(uart_async_write, "async_write preempt timer", 0);

            int size = uart_async_read(p);

            uart_async_write(p);
            uart_async_write("\r\nsize: ");
            uart_hex(size);
            uart_async_write("\r\n");
            uart_disable_recv_int();
        } else if (!_strncmp(input, "setTimeout", 10)){
            char *cmd = strtok_r(input, " ");
            if (cmd==NULL) continue;
            char *msg = strtok_r(NULL, " ");
            if (msg==NULL) continue;
            char *seconds = strtok_r(NULL, " \n");

            
            add_timer(uart_puts, msg, atoi(seconds));
        } else if (!_strncmp(input, "mm", 2)){
            /* mm Test */
            void* p = malloc(2*PAGE_SIZE);
            free(p);
            void *arr = malloc(32);
            free(arr);
            /* mm Test END */      
        }
        

    }
}