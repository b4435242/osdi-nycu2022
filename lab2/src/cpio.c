#include "cpio.h"

unsigned long CPIO_BASE;

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

    unsigned int name_size = hexstr_2_dec(header->c_namesize, 8);
    unsigned int file_size = hexstr_2_dec(header->c_filesize, 8);

    unsigned int header_name_size = sizeof(cpio_newc_header)+name_size;

    char* name = (char*)(header+1);
    char* file = (char*)header+aligned_on_n_bytes(header_name_size, 4);
    cpio_newc_header* next_header = ((cpio_newc_header*)(file+aligned_on_n_bytes(file_size, 4)));


    info->file=file;
    info->name=name;
    info->filesize=file_size;
    info->namesize=name_size;
    info->next_header = next_header;

} 





