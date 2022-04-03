#ifndef __PROGRAM__
#define __PROGRAM__

#include "cpio.h"
#include "stdlib.h"

void user_program();
void exec_user_program();
void el1_to_el0(unsigned long long lr, unsigned long long sp, unsigned long long spsr);
#endif