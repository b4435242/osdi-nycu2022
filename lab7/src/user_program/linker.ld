ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    __bss_start = .;
    .bss : {
     . = ALIGN(16);
     *(.bss*) 
    } 
    __bss_end = .;
    
}
__bss_size = (__bss_end - __bss_start)>>3;
