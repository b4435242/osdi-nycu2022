#include "include/mmio.h"

void mmio_put (uint64_t addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

unsigned int mmio_get (uint64_t addr) {
    volatile unsigned int* point = (unsigned int*)addr;
    return *point;
}