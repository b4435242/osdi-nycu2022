#include "program.h"


void exec_user_program(){
    //load
    char *p = (char*)0x100000;
    extraction program;
    char name[16] = "user_program.img";
    int res = cpio_search(name, &program);
    if (!res) return;
    for(int i=0; i<program.filesize; i++){
        p[i] = program.file[i];
    }

    el1_to_el0((unsigned long)p, (unsigned long)p, 0x3c0);
    
}

void el1_to_el0(unsigned long lr, unsigned long sp, unsigned long spsr){
    register unsigned long x0 asm("x0");
    register unsigned long x1 asm("x1");
    register unsigned long x2 asm("x2");

    x0 = lr;
    x1 = sp;
    x2 = spsr;

    if (lr!=0)
        asm("msr elr_el1, x0");
    else
        asm("msr elr_el1, lr");

    asm("msr sp_el0, x1");
    asm("msr spsr_el1, x2");
    asm("eret");
}