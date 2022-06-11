
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

.global init_tcr
init_tcr:
    ldr x0, = TCR_CONFIG_DEFAULT
    msr tcr_el1, x0
    ldp fp, lr, [sp]


#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

init_mair:
    ldr x0, =( \
    (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
    (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
    )
    msr mair_el1, x0


#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE


#define BOOT_PUD_DEV_ATTR (PD_ACCESS | PD_BLOCK | (MAIR_IDX_DEVICE_nGnRnE << 2))
#define BOOT_PUD_RAM_ATTR (PD_ACCESS | PD_BLOCK | (MAIR_IDX_NORMAL_NOCACHE << 2))
#define BOOT_PMD_DEV_ATTR BOOT_PUD_DEV_ATTR
#define BOOT_PMD_RAM_ATTR BOOT_PUD_RAM_ATTR

#define KERNEL_PGD_PA 0x1000
#define KERNEL_PUD_PA 0x2000
#define KERNEL_PMD_PA 0x3000

init_iden_paging:
    
    ldr x0, = KERNEL_PGD_PA // PGD's page frame at 0x1000
    ldr x1, = KERNEL_PUD_PA // PUD's page frame at 0x2000
    ldr x4, = KERNEL_PMD_PA // PMD's page frame at 0x3000

    ldr x2, = BOOT_PGD_ATTR
    orr x2, x1, x2 // combine the physical address of next level page with attribute.
    str x2, [x0]

    ldr x2, = PD_TABLE
    mov x3, x4
    orr x3, x2, x3
    str x3, [x1] // 1st 1GB mapped by the 1st entry of PUD

    ldr x2, = BOOT_PUD_DEV_ATTR
    mov x3, 0x40000000
    orr x3, x2, x3
    str x3, [x1, 8] // 2nd 1GB mapped by the 2nd entry of PUD


    ldr x2, = BOOT_PMD_RAM_ATTR
    mov x3, 0x00000000 // start
    orr x3, x2, x3
    mov x5, 0x3f000000 // end

1:  str x3, [x4], 8
    add x3, x3, 0x200000 // 2MB
    cmp x3, x5
    blt 1b


    ldr x2, = BOOT_PMD_DEV_ATTR
    mov x3, 0x3f000000 // start
    orr x3, x2, x3
    mov x5, 0x40000000 // end

2:  str x3, [x4], 8
    add x3, x3, 0x200000 // 2MB
    cmp x3, x5
    blt 2b

    msr ttbr0_el1, x0 // load PGD to the bottom translation-based register.
    msr ttbr1_el1, x0 // also load PGD to the upper translation based register.

    mrs x2, sctlr_el1
    orr x2, x2, 1
    msr sctlr_el1, x2

    ldr x2, = _boot_rest // indirect branch to the virtual address
    br x2
