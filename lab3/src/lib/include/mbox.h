#ifndef __MBOX__
#define __MBOX__

#include "mmio.h"
#include "uart.h"
#include "stdlib.h"

extern unsigned int mailbox[30];

void get_board_revision();
void get_arm_memory();
int call_mailbox(unsigned char channel, uint32_t* mbox);

#endif