#ifndef __IRQ__
#define __IRQ__

#include "uart.h"
#include "gpio.h"
#include "timer.h"
#include "task.h"

#define INT_SOURCE_0 0x40000060

#define CNTPNSIRQ 1<<1
#define GPUINTERRUPT 1<<8

#define INT_BASE (MMIO_BASE+0xB000)

#define IRQpending1 (INT_BASE+0x204)

#define AUX_GPU_SOURCE 1<<29

void enable_int();
void disable_int();
void irq_handler();
void concurrent_irq_handler();

#endif