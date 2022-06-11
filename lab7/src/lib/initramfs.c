#include "include/initramfs.h"

vnode_operations* initramfs_v_ops;
file_operations* initramfs_f_ops;

void initramfs_init(){
    filesystem* fs = k_malloc(sizeof(filesystem));
    fs->name = "initramfs";
    fs->setup_mount = initramfs_setup_mount;

    initramfs_v_ops = k_malloc(sizeof(vnode_operations));
    initramfs_v_ops->create = initramfs_create;
    initramfs_v_ops->mkdir = initramfs_mkdir;
    initramfs_v_ops->lookup = initramfs_lookup;

    initramfs_f_ops = k_malloc(sizeof(file_operations));
    initramfs_f_ops->open = initramfs_open;
    initramfs_f_ops->close = initramfs_close;
    initramfs_f_ops->read = initramfs_read;
    initramfs_f_ops->write = initramfs_write;
    initramfs_f_ops->lseek64 = initramfs_lseek64;

    register_filesystem(fs);
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "initramfs");

}

void initramfs_setup_mount(struct filesystem *fs, struct mount *mount){
    vnode* root_vnode = k_malloc(sizeof(vnode));

    mount->fs = fs;
    mount->root = root_vnode;

    root_vnode->v_ops = initramfs_v_ops;
    root_vnode->f_ops = initramfs_f_ops;
    root_vnode->internal = NULL;
}


int initramfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name){
    return -1;
}

int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name){
    return -1;
}

int initramfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name){
    for(int i=0; i<dir_node->n_edges; i++){
        vnode* child = dir_node->children[i];
        if (!strcmp(child->name, component_name)){
            *target = child;
            return 0;
        }
    }

    cpio_ext* ext = cpio_search(component_name);
    if (ext==NULL)
        return -1;
    *target = k_calloc(sizeof(vnode));
    (*target)->internal = ext;
    (*target)->parent = dir_node;
    (*target)->f_ops = initramfs_f_ops;
    (*target)->v_ops = initramfs_v_ops;

    dir_node->children[dir_node->n_edges++] = *target;
    strcpy((*target)->name, component_name);

    return 0;
}

int initramfs_open(struct vnode *file_node, struct file **target){
    *target = k_calloc(sizeof(file));
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    (*target)->flags = 0;
    return 0;
}

int initramfs_close(struct file *file){
    free(file);
    return 0;
}

int initramfs_read(struct file *file, void *buf, size_t len){
    cpio_ext* ext = file->vnode->internal;
    int r_size = min(min(len, ext->filesize-file->f_pos), ext->filesize);
    void* p = &ext->file[file->f_pos];
    memcpy(buf, p, r_size);
    file->f_pos += len;
    return r_size;
}

int initramfs_write(struct file *file, void *buf, size_t len){
    return -1;
}

int initramfs_lseek64(struct file *file, long offset, int whence){
    return 0;
}