#include "include/partition.h"

int parse_fat32_partition(uint32_t *offset){
    char buf[512];
    readblock(0, buf);
    partition_table* table = k_calloc(sizeof(partition_table));
    int ret = read_partition_table(buf, table);
    if (ret!=0)
        return ret;
    for(int i=0; i<4; i++){
        partition_table_entry *entry = &table->partitions[i];
        if (entry->bootable==0x80) { // bootable
            *offset = entry->offset;
            return 0;
        }
    }
}

int read_partition_table(char* buf, partition_table* table){
    memcpy(&table->partitions, &buf[446], 64);
    memcpy(&table->signature, &buf[510], 2);
    if (table->signature!=0xaa55)
        return -1;
    return 0;
}