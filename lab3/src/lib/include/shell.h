#ifndef __SHELL__
#define __SHELL__

#include "uart.h"
#include "stdlib.h"
#include "mbox.h"
#include "reset.h"
#include "cpio.h"
#include "fdt.h"
#include "process.h"    
#include "timer.h"
#include "mm.h"
#include "thread.h"
#include "sys.h"


void shell();
void foo();
void thread_test();
void fork_test();
void exec_test();
void read_test();

#endif