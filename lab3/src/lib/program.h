#ifndef __PROGRAM__
#define __PROGRAM__

#include "cpio.h"
#include "stdlib.h"

void exec_user_program();
void el1_to_el0(unsigned long lr, unsigned long sp, unsigned long spsr);
#endif