#include "include/framebuf.h"

#define MBOX_REQUEST 0
#define MBOX_CH_PROP 8
#define MBOX_TAG_LAST 0

unsigned char *lfb;   /* raw frame buffer address */


void framebuf_dev_init(){
    file_operations *framebuf_f_ops = k_calloc(sizeof(file_operations));

    framebuf_f_ops->write = framebuf_dev_write;
    framebuf_f_ops->lseek64 = framebuf_dev_lseek64;


    int dev_id = register_dev(framebuf_f_ops);
    vfs_mknod("/dev/framebuffer", dev_id);

    framebuf_init();
}

long framebuf_dev_lseek64(struct file* file, long offset, int whence){
    if (whence==SEEK_SET)
        file->f_pos = SEEK_SET+offset;
    return file->f_pos;
}

int framebuf_dev_write(struct file* file, const void* buf, size_t len){
    char *p = lfb + file->f_pos;
    memcpy(p, buf, len);
    file->f_pos += len;
    return len;
}

int framebuf_ioctl(file* file, uint32_t* mem){
    uint32_t w = mem[0];
    uint32_t h = mem[1];
    uint32_t pitch = mem[2];
    uint32_t isrgb = mem[3];

}

void framebuf_init(){
    unsigned int __attribute__((aligned(16))) mbox[36];
    unsigned int width, height, pitch, isrgb; /* dimensions and channel order */

    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // FrameBufferInfo.width
    mbox[6] = 768;  // FrameBufferInfo.height

    mbox[7] = 0x48004; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox[11] = 768;  // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (call_mailbox(MBOX_CH_PROP, mbox) && mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        width = mbox[5];        // get actual physical width
        height = mbox[6];       // get actual physical height
        pitch = mbox[33];       // get number of bytes per line
        isrgb = mbox[24];       // get the actual channel order
        lfb = (void *)((unsigned long)mbox[28]);
    } else {
        uart_puts("Unable to set screen resolution to 1024x768x32\n");
    }
}