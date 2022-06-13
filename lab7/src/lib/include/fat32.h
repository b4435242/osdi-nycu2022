#ifndef __FAT32__
#define __FAT32__

#include "sd_driver.h"
#include "stdlib.h"
#include "partition.h"
#include "vfs.h"


#define TF_ATTR_OFFSET 0x0b
#define TF_ATTR_READ_ONLY 0x01
#define TF_ATTR_HIDDEN 0x02
#define TF_ATTR_SYSTEM 0x04
#define TF_ATTR_VOLUME_LABEL 0x08
#define TF_ATTR_DIRECTORY 0x10
#define TF_ATTR_ARCHIVE 0x20
#define TF_ATTR_DEVICE 0x40 // Should never happen!
#define TF_ATTR_UNUSED 0x80

#define DATA_CLUSTER_U_BOUND 0xFFFFFEF
#define DATA_CLUSTER_L_BOUND 0x2
#define EOC 0xffffff8

#define MAX_CACAHED_BLOCKS 10

typedef struct bios_parameter_block bios_parameter_block;
typedef struct dir_entry dir_entry;
typedef struct internal_dir internal_dir;
typedef struct cached_block cached_block;
typedef struct vnode vnode;

struct bios_parameter_block {
    uint16_t bytes_per_sector;          // IMPORTANT
    uint8_t sectors_per_cluster;        // IMPORTANT
    uint16_t reserved_sectors;          // IMPORTANT
    uint8_t FAT_count;                  // IMPORTANT
    uint16_t dir_entries;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t count_sectors_per_FAT12_16; // FAT12/FAT16 only.
    uint16_t count_sectors_per_track;
    uint16_t count_heads_or_sizes_on_media;
    uint32_t count_hidden_sectors;
    uint32_t large_sectors_on_media;  // This is set instead of total_sectors if it's > 65535

    // Extended Boot Record
    uint32_t count_sectors_per_FAT32;   // IMPORTANT
    uint16_t flags;
    uint16_t FAT_version;
    uint32_t cluster_number_root_dir;   // IMPORTANT
    uint16_t sector_number_FSInfo;
    uint16_t sector_number_backup_boot_sector;

    uint8_t drive_number;
    uint8_t windows_flags;
    uint8_t signature;                  // IMPORTANT
    uint32_t volume_id;
    char volume_label[12];
    char system_id[9];
};

struct dir_entry {
	char filename[8];
	char extension[3];
	uint8_t attributes;
	uint8_t reserved;
	uint8_t creationTimeMs;
	uint16_t creationTime;
	uint16_t creationDate;
	uint16_t lastAccessTime;
	uint16_t eaIndex;
	uint16_t modifiedTime;
	uint16_t modifiedDate;
	uint16_t firstCluster;
	uint32_t fileSize;
};

struct cached_block {
    char buf[512];
    bool dirty;
    uint32_t cluster_map_val;
    uint32_t cluster_number; // map index
};

struct internal_dir {
    bool created;
    vnode* file_node;
    uint32_t dir_table_cluster;
    uint32_t first_cluster;
    uint32_t filesize;
    cached_block* caches[MAX_CACAHED_BLOCKS]; 
};



void fat32_init();

void fat32_setup_mount(struct filesystem *fs, struct mount *mount);
int fat32_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fat32_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fat32_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fat32_open(struct vnode *file_node, struct file **target);
int fat32_close(struct file *file);
int fat32_read(struct file *file, void *buf, size_t len);
int fat32_write(struct file *file, void *buf, size_t len);
int fat32_lseek64(struct file *file, long offset, int whence);

int fat32_cached_write(struct file *file, void *buf, size_t len);
int fat32_cached_read(struct file *file, void *buf, size_t len);
int fat32_cached_create(struct vnode *dir_node, struct vnode **target, const char *component_name);

internal_dir* search_dir(uint32_t first_cluster, char* component_name);
int set_empty_dir(uint32_t first_cluster, internal_dir* internal);
int set_dir_entry(uint32_t first_cluster, char* component_name, internal_dir* internal);

static uint32_t readi32(uint8_t *buff, size_t offset);
static uint16_t readi16(uint8_t *buff, size_t offset);
static void read_bpb(uint32_t fat_lba , bios_parameter_block *bpb);
uint32_t get_free_cluster();

void set_cluster_map(uint32_t index, uint32_t val);
uint32_t get_cluster_map(uint32_t index);

void read_fat_data(uint32_t index, void* buf);
void write_fat_data(uint32_t index, void* buf);

void parse_dir_entry(char* buf, dir_entry* entry);

dir_entry* internal_to_dir_table_entry(internal_dir* internal);
void fat32_read_data_to_cache(internal_dir* internal, uint32_t cluster_number);
cached_block* get_cache_from_internal(internal_dir* internal, uint32_t cluster_number);

static uint32_t readi32(uint8_t *buff, size_t offset);
static uint16_t readi16(uint8_t *buff, size_t offset);

#endif