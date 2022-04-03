#ifndef __UART__
#define __UART__

#include "gpio.h"
#include "stdlib.h"
#include "irq.h"

#define IRQs1 (INT_BASE+0x210)
#define INT_AUX_RECV 0b00000100
#define INT_AUX_TRAN 0b00000010
#define INT_AUX_MASK 0b00000110

void uart_init();
void uart_send(char c);
char uart_getc();
char* uart_getline(char *buf);
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_puts_withSize(char* s, unsigned int size);

void aux_handler();
void uart_async_write(char* s);
int uart_async_read(char* p);

void uart_unmask_aux();
void uart_mask_aux();

void uart_enable_transmit_int();
void uart_disable_transmit_int();
void uart_enable_recv_int();
void uart_disable_recv_int();



#endif