#ifndef __SYS_HANDLER__
#define __SYS_HANDLER__

#include "sys.h"
#include "thread.h"
#include "uart.h"
#include "process.h"
#include "mbox.h"
#include "sig.h"





typedef void(*handler)(uint64_t*);

void sys_handler(uint64_t tf_base);
void getpid_handler(uint64_t* tf);
void uart_read_handler(uint64_t* tf);
void uart_write_handler(uint64_t* tf);
void exec_handler(uint64_t* tf);
void fork_handler(uint64_t* tf);
void exit_handler(uint64_t* tf);
void mbox_call_handler(uint64_t* tf);
void _kill_handler(uint64_t* tf);

void signal_regis_handler(uint64_t* tf);
void kill_handler(uint64_t* tf);
void sigreturn_handler(uint64_t* tf);
#endif