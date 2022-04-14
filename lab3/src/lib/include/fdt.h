#ifndef __FDT__
#define __FDT__

#include "stdlib.h"
#include "uart.h"
#include "mm.h"

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009
#define FDT_MAGIC 0xd00dfeed


extern uint64_t CPIO_BASE;

extern uint64_t MEM_BASE;
extern uint64_t MEM_LENGTH;

typedef struct fdt_prop fdt_prop;
typedef struct fdt_header fdt_header;
typedef struct fdt_reserve_entry fdt_reserve_entry;
typedef void* (*fdt_callback)(fdt_prop*, char*, char*);

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

struct fdt_prop{
    uint32_t len;
    uint32_t nameoff;
};

struct fdt_reserve_entry {
    uint64_t address;
    uint64_t size;
};

void fdt_traverse(fdt_header *header_addr, fdt_callback *callback, size_t callback_size);
void initramfs_callback(fdt_prop *prop, char* node_name, char *property_name);
void memory_callback(fdt_prop *prop, char* node_name, char *property_name);

void fdt_traverse_rsvmap(uint64_t DTB_BASE, void (*memory_reserve)(uint64_t, uint64_t));

#endif