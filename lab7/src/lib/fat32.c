#include "include/fat32.h"

vnode_operations* fat_v_ops;
file_operations* fat_f_ops;

internal_dir root_dir;
uint32_t cluster_size;
bios_parameter_block bpb;
uint32_t fat32_partition_lba;
uint32_t fat_region_lba;
uint32_t fat_region_lba_range;
uint32_t data_region_lba;

internal_dir* dirty_internals[20];
int num_dirties;

void fat32_init(){
    

    filesystem* fs = k_malloc(sizeof(filesystem));
    fs->name = "fat32";
    fs->setup_mount = fat32_setup_mount;

    fat_v_ops = k_malloc(sizeof(vnode_operations));
    fat_v_ops->create = fat32_cached_create;
    fat_v_ops->mkdir = fat32_mkdir;
    fat_v_ops->lookup = fat32_lookup;

    fat_f_ops = k_malloc(sizeof(file_operations));
    fat_f_ops->open = fat32_open;
    fat_f_ops->close = fat32_close;
    fat_f_ops->read = fat32_cached_read;
    fat_f_ops->write = fat32_cached_write;
    fat_f_ops->lseek64 = fat32_lseek64;

    register_filesystem(fs);
    vfs_mkdir("/boot");
    vfs_mount("/boot", "fat32");

    sd_init();

    int res = parse_fat32_partition(&fat32_partition_lba);
    if (res!=0) 
        return;
    read_bpb(fat32_partition_lba, &bpb);
    root_dir.first_cluster = bpb.cluster_number_root_dir;
    cluster_size = bpb.sectors_per_cluster * bpb.bytes_per_sector;

    fat_region_lba = bpb.reserved_sectors + fat32_partition_lba;
    fat_region_lba_range = bpb.count_sectors_per_FAT32 * bpb.FAT_count;
    data_region_lba = fat_region_lba + fat_region_lba_range;
    
}



void fat32_setup_mount(struct filesystem *fs, struct mount *mount){
    
    vnode* root_vnode = k_calloc(sizeof(vnode));

    mount->fs = fs;
    mount->root = root_vnode;

    root_vnode->v_ops = fat_v_ops;
    root_vnode->f_ops = fat_f_ops;
    root_vnode->internal = &root_dir;
}

int fat32_create(struct vnode *dir_node, struct vnode **target, const char *component_name){
    uint32_t free_cluster = get_free_cluster();
    if (free_cluster==0xffffffff)
        return -1;


    set_cluster_map(free_cluster, EOC);

    internal_dir *internal = k_calloc(sizeof(internal_dir));

    (*target)->f_ops = fat_f_ops;
    (*target)->v_ops = fat_v_ops;
    (*target)->internal = internal;
    (*target)->parent = dir_node;
    strcpy((*target)->name, component_name); 

    internal->first_cluster = free_cluster;
    internal->filesize = 0;
    internal->file_node = (*target);
    dir_entry* dir = internal_to_dir_table_entry(internal);

    uint32_t dir_cluster = ((internal_dir*)dir_node->internal)->first_cluster;
    set_empty_dir(dir_cluster, internal);
    free(dir);


    return 0;
}

int fat32_cached_create(struct vnode *dir_node, struct vnode **target, const char *component_name){
    uint32_t free_cluster = get_free_cluster();
    if (free_cluster==0xffffffff)
        return -1;

    internal_dir *internal = k_calloc(sizeof(internal_dir));
    internal->first_cluster = free_cluster;
    internal->filesize = 0;
    internal->file_node = (*target);
    internal->created = false;
    //set_cluster_map(free_cluster, EOC);
    cached_block** cache = &internal->caches[0];
    *cache = k_calloc(sizeof(cached_block));
    (*cache)->dirty = true;
    (*cache)->cluster_number = free_cluster;
    (*cache)->cluster_map_val = EOC;

    (*target)->f_ops = fat_f_ops;
    (*target)->v_ops = fat_v_ops;
    (*target)->internal = internal;
    (*target)->parent = dir_node;

    return 0;
}

int fat32_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name){

}

int fat32_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name){
    for(int i=0; i<dir_node->n_edges; i++){
        vnode* child = dir_node->children[i];
        if (!strcmp(child->name, component_name)){
            *target = child;
            return 0;
        }
    }
    internal_dir *entry = (internal_dir*) dir_node->internal;
    internal_dir* internal = search_dir(entry->first_cluster, component_name);

    if (internal==NULL)
        return -1;
    
    *target = k_calloc(sizeof(vnode));
    (*target)->internal = internal;
    (*target)->parent = dir_node;
    (*target)->f_ops = fat_f_ops;
    (*target)->v_ops = fat_v_ops;

    dir_node->children[dir_node->n_edges++] = *target;
    strcpy((*target)->name, component_name);


    return 0;
    
}

int fat32_open(struct vnode *file_node, struct file **target){}

int fat32_close(struct file *file){}

int fat32_read(struct file *file, void *buf, size_t len){
    internal_dir* internal = file->vnode->internal;
    int r_size = min(min(len, internal->filesize-file->f_pos), internal->filesize);
    char tmp_buf[512];

    uint32_t cluster_offset = file->f_pos / cluster_size;
    uint32_t n_cluster = (r_size/cluster_size) + (r_size%cluster_size==0?0:1); 
    uint32_t cluster_number = internal->first_cluster;
    uint32_t left = r_size;

    for(int i=0; i<cluster_offset; i++)
        cluster_number = get_cluster_map(cluster_number);

    for(int i=0; i<n_cluster; i++){
        read_fat_data(cluster_number, tmp_buf);
        uint32_t tmp_offset = file->f_pos % cluster_size;
        char *p = &tmp_buf[tmp_offset];
        size_t tmp_size = min(left, 512-tmp_offset); // left to read, can read in the cluster
        memcpy(buf, p, tmp_size);
        file->f_pos += tmp_size;
        left -= tmp_size;
        cluster_number = get_cluster_map(cluster_number);
    }
    return r_size;
}

int fat32_cached_read(struct file *file, void *buf, size_t len){
    

    internal_dir* internal = file->vnode->internal;
    int r_size = min(min(len, internal->filesize-file->f_pos), internal->filesize);
    
    
    uint32_t cluster_offset = file->f_pos / cluster_size;
    uint32_t n_cluster = (r_size/cluster_size) + (r_size%cluster_size==0?0:1); 
    uint32_t cluster_number = internal->first_cluster;
    uint32_t left = r_size;



    for(int i=0; i<cluster_offset; i++)
        cluster_number = get_cluster_map(cluster_number);

    for(int i=0; i<n_cluster; i++){

        cached_block *cache = get_cache_from_internal(internal, cluster_number);
        if (cache==NULL) {
            fat32_read_data_to_cache(internal, cluster_number);
            cache = internal->caches[cluster_number];
        }
        uint32_t tmp_offset = file->f_pos % cluster_size;
        char *p = &(cache->buf[tmp_offset]);
        size_t tmp_size = min(left, 512-tmp_offset); // left to read, can read in the cluster
        memcpy(buf, p, tmp_size);
        file->f_pos += tmp_size;
        left -= tmp_size;
        cluster_number = get_cluster_map(cluster_number);
    }
    return r_size;
}

int fat32_write(struct file *file, void *buf, size_t len){
    internal_dir* internal = file->vnode->internal;
    char tmp_buf[512];

    uint32_t cluster_offset = file->f_pos / cluster_size;
    uint32_t n_cluster = (len/cluster_size) + (len%cluster_size==0?0:1); 
    uint32_t left = len;

    uint32_t cluster_number = internal->first_cluster;

    for(int i=0; i<cluster_offset; i++)
        cluster_number = get_cluster_map(cluster_number);
    
    for(int i=0; i<n_cluster; i++){
        read_fat_data(cluster_number, tmp_buf);
        uint32_t tmp_offset = file->f_pos % cluster_size;
        char *p = &tmp_buf[tmp_offset];
        size_t tmp_size = min(left, 512-tmp_offset); // left to write, can write in the cluster
        memcpy(p, buf, tmp_size);
        write_fat_data(cluster_number, tmp_buf);
        file->f_pos += tmp_size;
        left -= tmp_size;

        if (left>0){
            uint32_t cluster_val = get_cluster_map(cluster_number);
            if (cluster_val==EOC){
                cluster_val = get_free_cluster();
                set_cluster_map(cluster_number, cluster_val);
            }
        }
    }
    internal->filesize += len;

    vnode *dir_node = file->vnode->parent;
    internal_dir *dir_internal = dir_node->internal;
    int ret = set_dir_entry(dir_internal->first_cluster, file->vnode->name, internal);
    if (ret!=0)
        return ret;

    return len;
}

int fat32_cached_write(struct file *file, void *buf, size_t len){

    internal_dir* internal = file->vnode->internal;
    char tmp_buf[512];

    uint32_t cluster_offset = file->f_pos / cluster_size;
    uint32_t num_cluster = (len/cluster_size) + (len%cluster_size==0?0:1); 
    uint32_t left = len;

    uint32_t cluster_number = internal->first_cluster;

    //fat32_cached_read(file, tmp_buf, len);

    for(int i=0; i<cluster_offset; i++)
        cluster_number = get_cluster_map(cluster_number);
    
    for(int i=0; i<num_cluster; i++){
        cached_block *cache = get_cache_from_internal(internal, cluster_number);
        if (cache==NULL) {
            fat32_read_data_to_cache(internal, cluster_number);
            cache = internal->caches[cluster_number];
        }
        uint32_t tmp_offset = file->f_pos % cluster_size;
        char *p = &cache->buf[tmp_offset];
        size_t tmp_size = min(left, 512-tmp_offset); // left to write, can write in the cluster
        memcpy(p, buf, tmp_size);
        cache->dirty = true;
        file->f_pos += tmp_size;
        left -= tmp_size;

        if (left>0){
            uint32_t cluster_val = get_cluster_map(cluster_number);
            if (cluster_val==EOC){
                cluster_val = get_free_cluster();
                //set_cluster_map(cluster_number, cluster_val);
                cache->cluster_map_val = cluster_val;
            }
            cluster_number = cluster_val;
        }
    }
    internal->filesize += len;
    internal->dir_table_cluster = ((internal_dir*) file->vnode->parent->internal)->first_cluster;
    internal->file_node = file->vnode;

    dirty_internals[num_dirties++] = internal;

    // fileSize 
    /*vnode *dir_node = file->vnode->parent;
    internal_dir *dir_internal = dir_node->internal;
    int ret = set_dir_entry(dir_internal->first_cluster, file->vnode->name, internal);
    if (ret!=0)
        return ret;
    */
    return len;
}

int fat32_lseek64(struct file *file, long offset, int whence){

}

internal_dir* search_dir(uint32_t first_cluster, char* component_name){
    char buf[512];
    uint32_t cluster_number = first_cluster;
    dir_entry dir;

    char* tmp = k_calloc(len(component_name)+1);
    strcpy(tmp, component_name);
    char* filename = strtok_r(tmp, ".");
    char* ext = strtok_r(NULL, ".");


    do{
        read_fat_data(cluster_number, buf);
        
        for(int i=0; i<cluster_size; i+=32){
            //parse_dir_entry(&buf[i], &dir);
            dir_entry* dir = &buf[i];
            uint8_t first_byte = *((uint8_t*)&dir);
            if (first_byte==0x00)
                continue;
            char* dir_filename = strtok_r(dir->filename, " ");
            char* dir_ext = strtok_r(dir->extension, " ");
            if (!strcmp(filename, dir_filename) && (ext==NULL || !strcmp(ext, dir_ext))){
                internal_dir* internal = k_malloc(sizeof(internal_dir));
                internal->first_cluster = dir->eaIndex<<16 | dir->firstCluster;
                internal->filesize = dir->fileSize;
                return internal;
            }
        
        }
    } while((cluster_number=get_cluster_map(cluster_number))<=DATA_CLUSTER_U_BOUND && cluster_number>=DATA_CLUSTER_L_BOUND);

    free(tmp);

    return NULL;
}

int set_dir_entry(uint32_t first_cluster, char* component_name, internal_dir* internal){
    char buf[512];
    dir_entry dir;

    uint32_t cluster_number = first_cluster;
    uint32_t cluster_val = first_cluster;

    char* tmp = k_calloc(len(component_name)+1);
    strcpy(tmp, component_name);
    char* filename = strtok_r(tmp, ".");
    char* ext = strtok_r(NULL, ".");

    do{
        cluster_number = cluster_val;
        read_fat_data(cluster_number, buf);
        
        for(int i=0; i<cluster_size; i+=32){
            uint8_t first_byte = *((uint8_t*)&buf[i]);
            if (first_byte==0x00)
                continue;
            //parse_dir_entry(&buf[i], &dir);
            dir_entry* dir = &buf[i];
            if (!_strncmp(filename, dir->filename, len(filename)) && (ext==NULL || !_strncmp(ext, dir->extension, len(ext)))){
                // internal -> dir table entry
                dir->fileSize = internal->filesize;
                dir->eaIndex = (uint16_t)(internal->first_cluster)>>16;
                dir->firstCluster = (uint16_t)internal->first_cluster;
                
               // memcpy(&buf[i], &dir, sizeof(dir_entry));
                write_fat_data(cluster_number, buf);
                return 0;
            }
        }

    } while((cluster_val=get_cluster_map(cluster_number))<=DATA_CLUSTER_U_BOUND && cluster_val>=DATA_CLUSTER_L_BOUND);
    return -1;
}

int set_empty_dir(uint32_t first_cluster, internal_dir* internal){
    char buf[512];
    uint32_t cluster_number = first_cluster;
    uint32_t cluster_val = first_cluster;

    do{
        cluster_number = cluster_val;
        read_fat_data(cluster_number, buf);
        
        for(int i=0; i<cluster_size; i+=32){
            uint8_t first_byte = *((uint8_t*)&buf[i]);
            if (first_byte==0x00 || first_byte==0xe5){
                dir_entry* dir = internal_to_dir_table_entry(internal);
                memcpy(&buf[i], dir, sizeof(dir_entry));
                write_fat_data(cluster_number, buf);
                free(dir);
                return 0;
            }
        }

    } while((cluster_val=get_cluster_map(cluster_number))<=DATA_CLUSTER_U_BOUND && cluster_val>=DATA_CLUSTER_L_BOUND);
    uint32_t free_cluster = get_free_cluster();
    set_cluster_map(cluster_number, free_cluster);
    set_cluster_map(free_cluster, EOC);
    return set_empty_dir(free_cluster, internal);
}

uint32_t get_cluster_map(uint32_t index){
    uint32_t pos = index * 4;
    char buf[512];
    uint32_t lba = fat_region_lba + (pos / bpb.bytes_per_sector);
    readblock(lba, buf);
    uint32_t val = *((uint32_t*)&buf[pos%bpb.bytes_per_sector]);
    return val;
}

void set_cluster_map(uint32_t index, uint32_t val){
    uint32_t pos = index * 4;
    char buf[512];
    uint32_t lba = fat_region_lba + (pos / bpb.bytes_per_sector);
    readblock(lba, buf);
    uint32_t *val_pos = (uint32_t*) &buf[pos%bpb.bytes_per_sector];
    *val_pos = val;
    writeblock(lba, buf);
}

void read_fat_data(uint32_t index, void* buf){
    uint32_t lba = data_region_lba + (index-2) * bpb.sectors_per_cluster;
    readblock(lba, buf);
}

void write_fat_data(uint32_t index, void* buf){
    uint32_t lba = data_region_lba + (index-2) * bpb.sectors_per_cluster;
    writeblock(lba, buf);
}

uint32_t get_free_cluster(){
    char buf[512];
    for(int i=0; i<bpb.count_sectors_per_FAT32; i++){
        readblock(fat_region_lba+i, buf);
        for(int j=0; j<512; j+=4){
            uint32_t *val = (uint32_t*)&buf[j];
            if (*val==0){ // free
                uint32_t pos = i*bpb.bytes_per_sector + j;
                uint32_t index = pos/4;
                return index;
            }
        }
    }
    return 0xffffffff;
}

static void read_bpb(uint32_t fat_lba , bios_parameter_block *bpb) {
    char buf[512];
    readblock(fat_lba, buf);

    bpb->bytes_per_sector = readi16(buf, 11);;
    bpb->sectors_per_cluster = buf[13];
    bpb->reserved_sectors = readi16(buf, 14);
    bpb->FAT_count = buf[16];
    bpb->dir_entries = readi16(buf, 17);
    bpb->total_sectors = readi16(buf, 19);
    bpb->media_descriptor_type = buf[21];
    bpb->count_sectors_per_FAT12_16 = readi16(buf, 22);
    bpb->count_sectors_per_track = readi16(buf, 24);
    bpb->count_heads_or_sizes_on_media = readi16(buf, 26);
    bpb->count_hidden_sectors = readi32(buf, 28);
    bpb->large_sectors_on_media = readi32(buf, 32);
    // EBR
    bpb->count_sectors_per_FAT32 = readi32(buf, 36);
    bpb->flags = readi16(buf, 40);
    bpb->FAT_version = readi16(buf, 42);
    bpb->cluster_number_root_dir = readi32(buf, 44);
    bpb->sector_number_FSInfo = readi16(buf, 48);
    bpb->sector_number_backup_boot_sector = readi16(buf, 50);
    // Skip 12 bytes
    bpb->drive_number = buf[64];
    bpb->windows_flags = buf[65];
    bpb->signature = buf[66];
    bpb->volume_id = readi32(buf, 67);
    memcpy(&bpb->volume_label, buf + 71, 11); bpb->volume_label[11] = 0;
    memcpy(&bpb->system_id, buf + 82, 8); bpb->system_id[8] = 0;
}

static uint16_t readi16(uint8_t *buff, size_t offset)
{
    uint8_t *ubuff = buff + offset;
    return ubuff[1] << 8 | ubuff[0];
}
static uint32_t readi32(uint8_t *buff, size_t offset) {
    uint8_t *ubuff = buff + offset;
    return
        ((ubuff[3] << 24) & 0xFF000000) |
        ((ubuff[2] << 16) & 0x00FF0000) |
        ((ubuff[1] << 8) & 0x0000FF00) |
        (ubuff[0] & 0x000000FF);
}

void parse_dir_entry(char* buf, dir_entry* entry){
    memcpy(entry->filename, buf, 8);
    memcpy(entry->extension, &buf[8], 3);
    memcpy(&entry->attributes, &buf[11], 1);
    memcpy(&entry->reserved, &buf[12], 1);
    memcpy(&entry->creationTimeMs, &buf[13], 1);
    memcpy(&entry->creationTime, &buf[14], 2);
    memcpy(&entry->creationDate, &buf[16], 2);
    memcpy(&entry->lastAccessTime, &buf[18], 2);
    memcpy(&entry->modifiedTime, &buf[22], 2);
    memcpy(&entry->modifiedDate, &buf[24], 2);

    entry->eaIndex = readi16(entry, 20);
    entry->firstCluster = readi16(entry, 26);
    entry->fileSize = readi32(&buf[28], 4);
}

void fat32_sync(){
    for(int i=0; i<num_dirties; i++){
        internal_dir* internal = dirty_internals[i];
        if (internal==NULL) 
            continue;
        // directory table
        if (internal->created==false){
            set_empty_dir(internal->dir_table_cluster, internal);
        } else {
            set_dir_entry(internal->dir_table_cluster, internal->file_node->name, internal);
        }
        for(int j=0; j<MAX_CACAHED_BLOCKS; j++){
            cached_block* cache = internal->caches[j];
            if (cache==NULL)
                break;
            if (cache->dirty){
                write_fat_data(cache->cluster_number, cache->buf);
                if (cache->cluster_map_val!=0){
                    set_cluster_map(cache->cluster_number, cache->cluster_map_val);
                    if (cache->cluster_map_val!=EOC)
                        set_cluster_map(cache->cluster_map_val, EOC);
                }
            }
        }
        dirty_internals[i] = NULL;
    }
}

dir_entry* internal_to_dir_table_entry(internal_dir* internal){
    dir_entry* dir = k_calloc(sizeof(dir_entry));
    dir->fileSize = internal->filesize;
    dir->eaIndex = (uint16_t)(internal->first_cluster)>>16;
    dir->firstCluster = (uint16_t)internal->first_cluster;

    char* name;
    if (internal->file_node!=NULL && (name=internal->file_node->name)!=NULL){
        char* tmp = k_calloc(len(name)+1);
        strcpy(tmp, name);
        char* filename = strtok_r(tmp, ".");
        char* ext = strtok_r(NULL, ".");

        memset(dir->filename, 0, 8);
        memset(dir->extension, 0, 8);
        if (filename!=NULL)
            memcpy(dir->filename, filename, len(filename));
        if (ext!=NULL)
            memcpy(dir->extension, ext, len(ext));
        
        free(tmp);
    }

    return dir;
}

void fat32_read_data_to_cache(internal_dir* internal, uint32_t cluster_number){
    cached_block **cache = &internal->caches[cluster_number];
    if (*cache==NULL){
        *cache = k_calloc(sizeof(cached_block));
        (*cache)->cluster_number = cluster_number;
        read_fat_data(cluster_number, (*cache)->buf);
    }
}

cached_block* get_cache_from_internal(internal_dir* internal, uint32_t cluster_number){
    for(int i=0; i<MAX_CACAHED_BLOCKS; i++){
        if (internal->caches[i]->cluster_number==cluster_number){
            return internal->caches[i];
        }
    }
    return NULL;
}