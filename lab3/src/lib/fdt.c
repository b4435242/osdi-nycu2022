#include "include/fdt.h"

#define SWAP_ENDIAN(x)   (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

void fdt_traverse(fdt_header *header, fdt_callback* callback, size_t callback_size){
    
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

            for(int i=0; i<callback_size; i++)
                (callback[i])(prop, node_name, property_name);

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


void memory_callback(fdt_prop *prop, char* node_name, char *property_name){
    if (!_strncmp(node_name, "memory", 6)&&!_strncmp(property_name, "reg", 3)){
        uint32_t* p = (uint32_t*)(prop+1);
        uint32_t base = SWAP_ENDIAN(*p);
        uint32_t size = SWAP_ENDIAN(*(p+1));
        uart_puts("[memory_callback] base ");
        uart_hex(base);
        uart_puts(" and size ");
        uart_hex(size);
        uart_puts("\n\r");
        MEM_BASE = base;
        MEM_LENGTH = size;
        
    }
}

void fdt_traverse_rsvmap(uint64_t DTB_BASE, void (*memory_reserve)(uint64_t, uint64_t))
{
    fdt_header *header = (fdt_header*)DTB_BASE;
    if (SWAP_ENDIAN(header->magic)!=FDT_MAGIC) 
        return;

    fdt_reserve_entry *p = (fdt_reserve_entry*)((char*)header+SWAP_ENDIAN(header->off_mem_rsvmap));
    while (1)
    {
        uint64_t addr = SWAP_ENDIAN(p->address);
        uint64_t size = SWAP_ENDIAN(p->size);
        
        if (addr==0&&size==0) 
            break;
        memory_reserve(addr, addr+size);
        p++;
        uart_puts("[fdt_traverse_rsvmap] addr, size is ");
        uart_hex(addr);
        uart_puts(" ");
        uart_hex(size);
        uart_puts("\r\n");
    }
    


}
