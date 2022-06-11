
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
size_t r_buf_size=0;
size_t w_buf_size=0, w_byte;
int w_fin;

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

void uart_dev_init(){
    file_operations *uart_f_ops = k_calloc(sizeof(file_operations));

    uart_f_ops->read = uart_dev_read;
    uart_f_ops->write = uart_dev_write;

    int dev_id = register_dev(uart_f_ops);
    vfs_mkdir("/dev");
    vfs_mknod("/dev/uart", dev_id);
    vnode *target;
    vfs_open("/dev/uart", 0, &target); //0
    vfs_open("/dev/uart", 0, &target); //1
    vfs_open("/dev/uart", 0, &target); //2
    
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
    memset(buf, 0, sizeof(buf));
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


size_t uart_sync_write(char* s, size_t size){
    int i;
    for(i=0; i<size; i++){
        uart_send(*(s+i));
    }
    return i;
}

size_t uart_sync_read(char* buf, size_t size){
    size_t r_size = 0;
    while (r_size<size){
        buf[r_size++] = uart_getc();
    }
    return r_size;
}

size_t uart_dev_write(file* f, char* s, size_t size){
    return uart_sync_write(s, size);
}

size_t uart_dev_read(file* f, char* buf, size_t size){
    return uart_sync_read(buf, size);
}



/**
 * Display a binary value in hexadecimal
 */

void uart_hex(uint64_t d) {
    unsigned int n;
    int c;
    int flag = false;
    for(c=60;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        if (n>'0')
            flag = true;
        if (flag)
            uart_send(n);
    }
}

void aux_handler(){
    unsigned int iir = *AUX_MU_IIR;
    unsigned int aux = iir & INT_AUX_MASK;

    if (aux==INT_AUX_RECV){
        uart_r_int_handler();
    } else if (aux==INT_AUX_TRAN){
        uart_w_int_handler();
    }
}

void uart_r_int_handler(){
    uart_disable_recv_int();
    while (*AUX_MU_LSR&0x01)
    {
       read_buf[r_buf_size++] = *AUX_MU_IO;
    }
}

void uart_w_int_handler(){
    uart_disable_transmit_int();
    while(w_byte<w_buf_size && (*AUX_MU_LSR&0x20)){ // write if available
        *AUX_MU_IO = write_buf[w_byte++];
    }
    w_buf_size = 0;
}

size_t uart_async_write(char* user_buf, size_t size){
    memcpy(write_buf, user_buf, size);
    w_buf_size = size;
    w_byte = 0;

    // preempt to irq_handler->aux_handler
    unmask_aux_int();
    uart_enable_transmit_int();
    // end of preemption
    

    //return w_byte; // w_byte is configured in aux_handler
    return size;
}

size_t uart_async_read(char* user_buf, size_t size){
    unmask_aux_int();
    uart_enable_recv_int();

    size_t r_size = min(size, r_buf_size);

    memcpy(user_buf, read_buf, r_size);
    r_buf_size = 0;

    return r_size;
}

void unmask_aux_int(){
    //*((unsigned int*)IRQs1) = *((unsigned int*)IRQs1) | AUX_GPU_SOURCE; // second level interrupt controller enable bit29 AUX
    unsigned int val = mmio_get(IRQs1);
    mmio_put(IRQs1, val | AUX_GPU_SOURCE);
}

void mask_aux_int(){
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