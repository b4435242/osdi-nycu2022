#ifndef __FRAMEBUF__
#define __FRAMEBUF__

#include "vfs.h"
#include "mm.h"
#include "mbox.h"


void framebuf_init();
void framebuf_dev_init();
long framebuf_dev_lseek64(struct file* file, long offset, int whence);
int framebuf_dev_write(struct file* file, const void* buf, size_t len);


#endif