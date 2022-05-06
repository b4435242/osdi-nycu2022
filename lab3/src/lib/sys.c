#include "include/sys.h"

#define CALL_EXP asm volatile("svc 0")
#define SET_EXP(n) asm volatile("mov x8, %0"::"r"(n))

/* system call API */

// Trap frame setup before call_exp
// later preserved by save_all when transiting to exception

int getpid(){
    SET_EXP(SYS_GETPID);
    CALL_EXP;
    int pid;
    asm volatile("mov %0, x0":"=r"(pid));
    return pid;

}

size_t uart_read(char buf[], size_t size){
    SET_EXP(SYS_UARTREAD);
    asm volatile("mov x1, %0"::"r"(size));
    asm volatile("mov x0, %0"::"r"(buf));
    CALL_EXP;
    size_t _size;
    asm volatile("mov %0, x0":"=r"(_size));
    return _size;
}

size_t uart_write(const char buf[], size_t size){
    SET_EXP(SYS_UARTWRITE);
    asm volatile("mov x1, %0"::"r"(size));
    asm volatile("mov x0, %0"::"r"(buf));
    CALL_EXP;
    size_t _size;
    asm volatile("mov %0, x0":"=r"(_size));
    return _size;
}

int exec(char* name, char** argv){
    SET_EXP(SYS_EXEC);
    asm volatile("mov x1, %0"::"r"(argv));
    asm volatile("mov x0, %0"::"r"(name));
    CALL_EXP;
    int res;
    asm volatile("mov %0, x0":"=r"(res));
    return res;
}

int fork(){
    int pid;
    SET_EXP(SYS_FORK);
    CALL_EXP;
    asm volatile("mov %0, x0":"=r"(pid));
    return pid;
}

void exit(){
    SET_EXP(SYS_EXIT);
    CALL_EXP;
}

int mbox_call(unsigned char ch, unsigned int *mbox){
    SET_EXP(SYS_MBOXCALL);
    asm volatile("mov x1, %0"::"r"(mbox));
    asm volatile("mov x0, %0"::"r"(ch));
    CALL_EXP;
    int status;
    asm volatile("mov %0, x0":"=r"(status));
    return status;
}

void _kill(int pid){
    SET_EXP(SYS_KILL);
    asm volatile("mov x0, %0"::"r"(pid));
    CALL_EXP;
}

void signal(int SIGNAL, void (*handler)()){
    SET_EXP(SYS_SIG);
    asm volatile("mov x1, %0"::"r"(handler));
    asm volatile("mov x0, %0"::"r"(SIGNAL));
    CALL_EXP;
}

void kill(int pid, int SIGNAL){
    SET_EXP(SYS_KILL);
    asm volatile("mov x1, %0"::"r"(SIGNAL));
    asm volatile("mov x0, %0"::"r"(pid));
    CALL_EXP;
}

void sigreturn(){
    SET_EXP(SYS_RET);
    CALL_EXP;
}