#include "include/mbox.h"


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
    
    call_mailbox(MBOX_CH_PROP, mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.

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
    
    call_mailbox(MBOX_CH_PROP, mailbox); // message passing procedure call, you should implement it following the 6 steps provided above.

    //return mailbox;
}

uint32_t* get_framebuf_mem(){
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_FRAME_BUF_ADDR;
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0;
    // tags end
    mailbox[7] = END_TAG;
    call_mailbox(MBOX_CH_PROP, mailbox);
    return &mailbox[5];
}

int call_mailbox(unsigned char channel, uint32_t* mbox){
    uint32_t* buf = malloc(PAGE_SIZE);
    uint32_t buf_va = 0xf0000000;
    mappages(current_thread()->u_PGD, buf_va, 1, buf, true);
    memcpy(buf_va, mbox, mbox[0]);
    
    // step 1
    //unsigned int addr = ((unsigned int)((unsigned long)mbox&~0xF))|channel;
    uint32_t addr =  (uint32_t)buf | channel;
    // step 2
    while (mmio_get(MAILBOX_STATUS) & MAILBOX_FULL) {}
    // step 3
    mmio_put(MAILBOX_WRITE, addr);

    while(1){
        // step 4
        while(mmio_get(MAILBOX_STATUS) & MAILBOX_EMPTY) {}
        // step 5
        unsigned int val = mmio_get(MAILBOX_READ);
        // step 6
        if (val==addr){
          memcpy(mbox, buf_va, mbox[0]);
          free(buf_va);
          return (mbox[1]==REQUEST_SUCCEED);
        }
            
    }
    return 0;
}

