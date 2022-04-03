
#include "include/uart.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))
char read_buf[1000];
char write_buf[1000];
int r=0, w=0;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    /* map UART1 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r=150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx


}

/**
 * Send a character
 */
void uart_send(char c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    *AUX_MU_IO=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

char* uart_getline(char *buf) {
    char c;
    int i=0;
    _memset(buf, 0, sizeof(buf));
    do {
        c = uart_getc();
        if(c<=127){
            uart_send(c);
            buf[i++] = c;
        }
    } while (c!='\n'&&c!='\r');
    return buf;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
     for (int i = 0; s[i] != '\0'; i ++) {
        uart_send((char)s[i]);
    }
}


void uart_puts_withSize(char* s, unsigned int size){
    for(int i=0; i<size; i++){
        uart_send(*(s+i));
    }
}


/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

void aux_handler(){
    unsigned int iir = *AUX_MU_IIR;
    unsigned int cause = iir & INT_AUX_MASK;
    switch (cause){
        case INT_AUX_RECV:
            read_buf[r++] = *AUX_MU_IO;
            break;
        case INT_AUX_TRAN:
            uart_puts_withSize(write_buf, w);
            w = 0;
            
            break;
        default:
            break;
    }
    uart_disable_transmit_int();
    //uart_enable_recv_int();
}

void uart_async_write(char* s){
    while(s[w]!='\0'){
        write_buf[w] = s[w];
        w++;
    }
    uart_disable_recv_int();
    uart_enable_transmit_int();
}

int uart_async_read(char* p){
    for (int i=0; i<r; i++){
        p[i] = read_buf[i];
    }
    int old = r;
    r = 0;
    return old;
}

void uart_unmask_aux(){
    //*((unsigned int*)IRQs1) = *((unsigned int*)IRQs1) | AUX_GPU_SOURCE; // second level interrupt controller enable bit29 AUX
    unsigned int val = mmio_get(IRQs1);
    mmio_put(IRQs1, val | AUX_GPU_SOURCE);
}

void uart_mask_aux(){
    //*((unsigned int*)IRQs1) = *((unsigned int*)IRQs1) & ~AUX_GPU_SOURCE; // second level interrupt controller disable bit29 AUX
    unsigned int val = mmio_get(IRQs1);
    mmio_put(IRQs1, val & ~AUX_GPU_SOURCE);
}

void uart_enable_transmit_int(){
    *AUX_MU_IER = *AUX_MU_IER | 1<<1;
}
void uart_disable_transmit_int(){
    *AUX_MU_IER = *AUX_MU_IER & ~(1<<1);
}
void uart_enable_recv_int(){
    *AUX_MU_IER = *AUX_MU_IER | 1;
}
void uart_disable_recv_int(){
    *AUX_MU_IER = *AUX_MU_IER & ~1;
}