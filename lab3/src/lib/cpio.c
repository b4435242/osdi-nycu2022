#include "include/cpio.h"



extraction cpio_search(char *name){
    cpio_newc_header* header = (cpio_newc_header*)CPIO_BASE;
    extraction info;
    while(1) {
        parse_cpio_header(header, &info);
        //uart_puts(info.name);
        //uart_puts("\r\n");
        // info.namesize counts '\0' 
        if(!_strncmp(info.name, "TRAILER!!!", 10)||!_strncmp(info.name, name, info.namesize-1)) 
            break;

        header = info.next_header;

    } 
    if (!_strncmp(info.name, name, info.namesize-1)){
        uart_puts("1");
        return info;
    }
    return info;
}

void cpio_ls(){
    extraction info;
    cpio_newc_header* header = (cpio_newc_header*)CPIO_BASE;

    /*uart_puts("[cpio_ls] CPIO_BASE ");
    uart_hex(CPIO_BASE);
    uart_puts("\r\n");*/

    while(1) {
        parse_cpio_header(header, &info);
        if(!_strncmp(info.name, "TRAILER!!!", 10)) 
            break;

        uart_puts_withSize(info.name, info.namesize);
        uart_puts("\r\n");

        header = info.next_header;

    } 
    
}

void cpio_cat(){
    
    uart_puts("Filename: ");
    char input[100];
    uart_getline(input);
    
    extraction info;
    cpio_newc_header* header = (cpio_newc_header*)CPIO_BASE;

    while(1) {
        parse_cpio_header(header, &info);
        // info.namesize counts '\0' 
        if(!_strncmp(info.name, "TRAILER!!!", 10)||!_strncmp(info.name, input, info.namesize-1)) 
            break;

        header = info.next_header;

    } 

    if (!_strncmp(info.name, input, info.namesize-1)){
        uart_puts_withSize(info.file, info.filesize);
        uart_puts("\r\n");
    }

}

void parse_cpio_header(cpio_newc_header* header, extraction *info) {
    
    /*uart_puts("[parse_cpio_header] Magic ");
    uart_puts_withSize(header->c_magic, 6);
    uart_puts("\r\n");*/

    if(_strncmp(header->c_magic, "070701", 6))
        return;

    unsigned int name_size = hexstr_2_dec(header->c_namesize, sizeof(header->c_namesize));
    unsigned int file_size = hexstr_2_dec(header->c_filesize, sizeof(header->c_filesize));
    unsigned long CPIO_HEADER_SIZE = sizeof(cpio_newc_header);

    char* name = (char*)header+CPIO_HEADER_SIZE;
    char* file = (char*)cpio_align((char*)header+CPIO_HEADER_SIZE+name_size);
    cpio_newc_header* next_header = (cpio_newc_header*)cpio_align((unsigned long)(file+file_size));
    uart_hex((uint32_t)next_header);
    uart_puts("\r\n");

    info->file=file;
    info->name=name;
    info->filesize=file_size;
    info->namesize=name_size;
    info->next_header = next_header;

} 

unsigned long cpio_align (unsigned long v) {
    unsigned long lower_bits = v & 0x3;

    v = v - lower_bits;

    if (lower_bits > 0)
    {
        v = v + 4;
    }

    return v;
}




