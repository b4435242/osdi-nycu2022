#ifndef __MBOX__
#define __MBOX__

#include "mmio.h"
#include "uart.h"
#include "stdlib.h"
#include "vm.h"

#define MAILBOX_BASE    (MMIO_BASE + 0xb880)
#define MAILBOX_READ    MAILBOX_BASE
#define MAILBOX_STATUS  (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE   (MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define MBOX_CH_PROP 8

#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define GET_FRAME_BUF_ADDR  0x00040001

#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000


extern unsigned int mailbox[30];

void get_board_revision();
void get_arm_memory();
uint32_t* get_framebuf_mem();
int call_mailbox(unsigned char channel, uint32_t* mbox);

#endif