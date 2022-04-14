.section ".text"


.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
  b exception_handler // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b el1_irq
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b el0_irq
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

.global set_exception_vector_table
set_exception_vector_table:
  adr x0, exception_vector_table
  msr vbar_el1, x0
  ret

// save system registers to stack
.macro save_sys_reg
    sub sp, sp, 2 * 8
    mrs x0, spsr_el1
    mrs x1, elr_el1
    stp x0, x1, [sp, 0]
.endm

// load system registers from stack
.macro load_sys_reg
    ldp x0, x1, [sp, 0]
    msr spsr_el1, x0
    msr elr_el1, x1
    add sp, sp, 2 * 8
.endm

// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

el1_irq:
  save_all
  save_sys_reg // nested in save_all, otherwise it will corrupt general registers
  //bl print_register
  bl concurrent_irq_handler
  load_sys_reg
  load_all
  eret

el0_irq:
  save_all
  //bl print_register
  bl concurrent_irq_handler
  load_all
  eret

exception_handler:
  save_all
  bl print_register
  load_all
loo:
  b loo  
  eret

.data
    spsr_str: .asciz "spsr   "
    elr_str: .asciz "elr    "
    esr_str: .asciz "esr    "
    next_line: .asciz " \n\r"
print_register:
  mov x20, lr
  //spsr
  ldr x0, =spsr_str
  bl uart_puts
  mrs x0, spsr_el1
  bl uart_hex
  ldr x0, =next_line
  bl uart_puts
  //elr
  ldr x0, =elr_str
  bl uart_puts
  mrs x0, elr_el1
  bl uart_hex
  ldr x0, =next_line
  bl uart_puts
  //esr
  ldr x0, =esr_str
  bl uart_puts
  mrs x0, esr_el1
  bl uart_hex
  ldr x0, =next_line
  bl uart_puts
  mov lr, x20
  ret