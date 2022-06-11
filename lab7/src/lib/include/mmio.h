#ifndef __MMIO__
#define __MMIO__

#define MMIO_BASE       0xFFFF00003F000000

#include "stdlib.h"

void mmio_put (uint64_t addr, unsigned int value);
unsigned int mmio_get (uint64_t addr);

#endif