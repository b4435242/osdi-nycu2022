#ifndef __MMIO__
#define __MMIO__

#define MMIO_BASE       0x3F000000


void mmio_put (long addr, unsigned int value);
unsigned int mmio_get (long addr);

#endif