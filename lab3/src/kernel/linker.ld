ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    _start = .;
    .text.boot : { *(.text.boot) }
    .text :  {
        *(.text)
    }
    .rodata : { *(.rodata) }
    .data : {
        . = ALIGN(8);
        *(.data)
    }
    . = ALIGN(0x8);
    __bss_start = .;
    .bss : {
     . = ALIGN(16);
     *(.bss*) 
    } 
    __bss_end = .;
    
}
__bss_size = (__bss_end - __bss_start)>>3;
_image_size = (__bss_end - _start);