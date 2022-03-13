#include "fdt.h"

#define SWAP_ENDIAN(x)   (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

void fdt_traverse(fdt_header *header, void (*callback)(fdt_prop*, char*, char*)){
    
    if (SWAP_ENDIAN(header->magic)!=FDT_MAGIC) 
        return;

    uint32_t *struct_addr = (uint32_t*)((char*)header+SWAP_ENDIAN(header->off_dt_struct));
    char *strings_addr = ((char*)header+SWAP_ENDIAN(header->off_dt_strings));

    char *node_name;

    while (1)
    {
        uint32_t token = SWAP_ENDIAN(*struct_addr);
        if(token==FDT_BEGIN_NODE)
        {
            node_name = (char*)(struct_addr+1);

            int size_node_name = len(node_name)+1; // including '\0'
            struct_addr += aligned_on_n_bytes(size_node_name, 4)/4; // padded if not 4 byte aligned

        } else if (token==FDT_PROP)
        {
            fdt_prop* prop = (fdt_prop*)(struct_addr+1);
            struct_addr += (sizeof(fdt_prop)+aligned_on_n_bytes(SWAP_ENDIAN(prop->len), 4))/4;
            char *property_name = strings_addr+SWAP_ENDIAN(prop->nameoff);

            callback(prop, node_name, property_name);

        } else if (token==FDT_END_NODE || token ==FDT_NOP)
        {
            
        } else if (token==FDT_END)
        {
            break;
        }

        struct_addr++;

    }
    
}

void initramfs_callback(fdt_prop *prop, char* node_name, char *property_name){
    
    if (!_strncmp(node_name, "chosen", 6)&&!_strncmp(property_name, "linux,initrd-start", 18)){
        uint32_t load_addr = *((uint32_t*)(prop+1));
        CPIO_BASE = SWAP_ENDIAN(load_addr);
        uart_puts("[initramfs_callback] CPIO_BASE ");
        uart_hex(CPIO_BASE);
        uart_puts("\n\r");
    }
}