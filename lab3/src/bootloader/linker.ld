ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    __bootloader_start = .;
    .text.boot : { *(.text.boot) }
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
    __bootloader_end = .;

}
__bss_size = (__bss_end - __bss_start)>>3;
__bootloader_size = (__bootloader_end - __bootloader_start)>>3;