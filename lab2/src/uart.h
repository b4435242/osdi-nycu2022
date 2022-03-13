#ifndef __UART__
#define __UART__

#include "gpio.h"
#include "lib.h"

void uart_init();
void uart_send(char c);
char uart_getc();
char* uart_getline(char *buf);
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_puts_withSize(char* s, unsigned int size);

#endif