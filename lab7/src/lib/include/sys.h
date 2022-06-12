#ifndef __SYS__
#define __SYS__

#include "thread.h"
#include "stdlib.h"
#include "process.h"

#define SYS_N 22

#define SYS_GETPID 0
#define SYS_UARTREAD 1
#define SYS_UARTWRITE 2
#define SYS_EXEC 3
#define SYS_FORK 4
#define SYS_EXIT 5
#define SYS_MBOXCALL 6
#define SYS_KILL 7
#define SYS_SIG 8
#define SYS_KILL_SIG 9
#define SYS_MMAP 10
#define SYS_OPEN 11
#define SYS_CLOSE 12
#define SYS_WRITE 13
#define SYS_READ 14
#define SYS_MKDIR 15
#define SYS_MOUNT 16
#define SYS_CHDIR 17
#define SYS_LSEEK 18
#define SYS_IOCTL 19
#define SYS_SYNC 20
#define SYS_RET 21


/* system call API */

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(char* name, char** argv);
int fork();
void exit();
int mbox_call(unsigned char ch, unsigned int *mbox);
void _kill(int pid);
void signal(int SIGNAL, void (*handler)());
void kill(int pid, int SIGNAL);
void sigreturn();

#endif