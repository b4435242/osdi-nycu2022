.section ".text.boot"

.global _start

__relocate_addr = 0x60000

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0

    // DTB base 
    mov x20, x0

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =__relocate_addr
    mov     sp, x1

    // relocate
    ldr     x1, =__relocate_addr
    ldr     x2, =__bootloader_size
    ldr     x3, =__bootloader_start
relocate:
    ldr     x4, [x3], #8
    str     x4, [x1], #8
    sub     x2, x2, #1
    cbnz    x2, relocate


    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  bl      load_kernel-0x20000
    // for failsafe, halt this core too
    b       1b