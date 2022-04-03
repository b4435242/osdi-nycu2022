#ifndef __MMIO__
#define __MMIO__

void mmio_put (long addr, unsigned int value);
unsigned int mmio_get (long addr);

#endif