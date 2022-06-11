#ifndef __PARTITION__
#define __PARTITION__

#include "sd_driver.h"
#include "stdlib.h"
#include "mm.h"

typedef struct partition_table partition_table;
typedef struct partition_table_entry partition_table_entry;

struct partition_table_entry
{
    uint8_t bootable;         /* 0x00=not bootable, 0x80=bootable. */
    uint8_t start_chs[3];     /* Encoded starting cylinder, head, sector. */
    uint8_t type;             /* Partition type (see partition_type_name). */
    uint8_t end_chs[3];       /* Encoded ending cylinder, head, sector. */
    uint32_t offset;          /* Start sector offset from partition table. */
    uint32_t size;            /* Number of sectors. */
};

struct partition_table
{
    uint8_t loader[446];      /* Loader, in top-level partition table. */
    partition_table_entry partitions[4];       /* Table entries. */
    uint16_t signature;       /* Should be 0xaa55. */
};

int parse_fat32_partition(uint32_t *offset);
int read_partition_table(char* buf, partition_table* table);

#endif