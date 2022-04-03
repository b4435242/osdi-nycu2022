#include "include/mbox.h"


#define MAILBOX_BASE    (MMIO_BASE + 0xb880)

#define MAILBOX_READ    ((volatile unsigned int*)MAILBOX_BASE)
#define MAILBOX_STATUS  ((volatile unsigned int*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE   ((volatile unsigned int*)(MAILBOX_BASE + 0x20))
#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000
unsigned int __attribute__((aligned(16))) mailbox[30]; // without manually setting alinged still work

void get_board_revision(){
  mailbox[0] = 7 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_BOARD_REVISION; // tag identifier
  mailbox[3] = 4; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  // tags end
  mailbox[6] = END_TAG;
  
  mailbox_call(8); // message passing procedure call, you should implement it following the 6 steps provided above.

  //printf("0x%x\n", mailbox[5]); // it should be 0xa020d3 for rpi3 b+
  //return mailbox[5];
}

void get_arm_memory(){
  mailbox[0] = 8 * 4; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = GET_ARM_MEMORY; // tag identifier
  mailbox[3] = 8; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0; // value buffer
  mailbox[6] = 0;
  // tags end
  mailbox[7] = END_TAG;
  
  mailbox_call(8); // message passing procedure call, you should implement it following the 6 steps provided above.

  //return mailbox;
}

int mailbox_call(unsigned char channel){
    // step 1
    unsigned int addr = ((unsigned int)((unsigned long)mailbox&~0xF))|channel;
    // step 2
    while(*MAILBOX_STATUS & MAILBOX_FULL) {}
    // step 3
    *MAILBOX_WRITE = addr;
    while(1){
        // step 4
        while(*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        // step 5
        unsigned int val = *MAILBOX_READ;
        // step 6
        if (val==addr)
            return mailbox[1]==REQUEST_SUCCEED;
    }
    return 0;
}