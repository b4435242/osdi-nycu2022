#ifndef __MBOX__
#define __MBOX__

#include "gpio.h"
#include "uart.h"

extern unsigned int mailbox[30];

void get_board_revision();
void get_arm_memory();
int mailbox_call(unsigned char channel);

#endif