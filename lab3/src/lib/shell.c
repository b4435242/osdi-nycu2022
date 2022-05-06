#include "include/shell.h"

void shell(){
    
    //write_test();
    //thread_test();
    //fork_test();
    exec_test();
    //asm volatile("svc 0");

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
        } else if (!_strncmp(input, "timer", 5)){
            add_timer_with_second(alert_seconds, NULL, 2);
        } else if (!_strncmp(input, "async", 5)){
            char p[1000];
            
            unmask_aux_int();
            uart_enable_recv_int();
            
            delay(10);
            uart_async_write("ASYNC WRITE", 11);
            //uart_async_read(p, 10);
            
            //add_timer_with_second(uart_async_write, "async_write preempt timer", 0);

            uart_async_write("\r\n", 2);
            uart_disable_recv_int();
        } else if (!_strncmp(input, "setTimeout", 10)){
            char *cmd = strtok_r(input, " ");
            if (cmd==NULL) continue;
            char *msg = strtok_r(NULL, " ");
            if (msg==NULL) continue;
            char *seconds = strtok_r(NULL, " \n");

            uint32_t len_msg = len(msg);
            char *arg = malloc(len_msg);
            memcpy(arg, msg, len_msg);
            
            add_timer_with_second(uart_puts, arg, atoi(seconds));
            add_timer_with_second(free, arg, atoi(seconds)+1);
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

/*void foo(){
    for(int i = 0; i < 10; ++i) {
        uart_puts("Thread id: ");
        uart_hex(current_thread()->tpid);
        uart_puts(" ");
        uart_hex(i);
        uart_puts("\r\n");
        delay(1000000);
        //schedule();
    }
    exit();
}

void thread_test() {
    // ...
    // boot setup
    // ...
    for(int i = 0; i < 3; ++i) { // N should > 2
        thread_create(foo, NULL, 0);
    }
    schedule();
    //idle();
}

void fork_test(){
    //printf("\nFork Test, pid %d\n", getpid());
    uart_puts("Fork Test, pid "); uart_hex(getpid()); uart_puts("\r\n");
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        //printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        uart_puts("first child pid: "); uart_hex(getpid()); uart_puts(", cnt: "); uart_hex(cnt); uart_puts(", ptr: "); uart_hex(&cnt);
        uart_puts(" sp: "); uart_hex(cur_sp); uart_puts("\r\n");

        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            //printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
            uart_puts("first child pid: "); uart_hex(getpid()); uart_puts(", cnt: "); uart_hex(cnt); uart_puts(", ptr: "); uart_hex(&cnt);
            uart_puts(", sp: "); uart_hex(cur_sp); uart_puts("\r\n");
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                //printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                uart_puts("second child pid: "); uart_hex(getpid()); uart_puts(", cnt: "); uart_hex(cnt); uart_puts(", ptr: "); uart_hex(&cnt);
                uart_puts(", sp: "); uart_hex(cur_sp); uart_puts("\r\n");
                delay(10000000); 
                ++cnt;
            }
        }
        exit();
    }
    else {
        //printf("parent here, pid %d, child %d\n", get_pid(), ret);
        uart_puts("parent here, pid: "); uart_hex(getpid());
        uart_puts(", child: "); uart_hex(ret); uart_puts("\r\n");
        //schedule(); // 0->1
        //schedule(); // 0->2
    }
}*/

void exec_test(){
    
    exec("syscall.img", NULL);
}

void write_test(){
    uint64_t sp = (uint64_t)malloc(THREAD_SP_SIZE)+THREAD_SP_SIZE;
    from_el1_to_el0(0, sp); // exec

    unmask_aux_int();
    uart_enable_recv_int();

    uart_write("hi", 2);

    
}