#ifndef __UART__
#define __UART__

#include "gpio.h"
#include "stdlib.h"
#include "irq.h"
#include "vfs.h"
#include "mm.h"

#define IRQs1 (INT_BASE+0x210)
#define INT_AUX_RECV 0b00000100
#define INT_AUX_TRAN 0b00000010
#define INT_AUX_MASK 0b00000110

void uart_init();
void uart_dev_init();

void uart_send(char c);
char uart_getc();
char* uart_getline(char *buf);
void uart_puts(char *s);
void uart_hex(uint64_t d);
size_t uart_sync_write(char* s, size_t size);
size_t uart_sync_read(char *s, size_t size);
size_t uart_dev_read(struct file* f, char* buf, size_t size);
size_t uart_dev_write(struct file* f, char* buf, size_t size);


void aux_handler();
size_t uart_async_write(char* user_buf, size_t size);
size_t uart_async_read(char* user_buf, size_t size);

void unmask_aux_int();
void mask_aux_int();

void uart_enable_transmit_int();
void uart_disable_transmit_int();
void uart_enable_recv_int();
void uart_disable_recv_int();



#endif