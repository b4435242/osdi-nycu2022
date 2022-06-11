#include "include/tmpfs.h"

vnode_operations* tmpfs_v_ops;
file_operations* tmpfs_f_ops;


void tmpfs_init(){
    filesystem* tmpfs = k_calloc(sizeof(filesystem));
    tmpfs->name = "tmpfs";
    tmpfs->setup_mount = tmpfs_setup_mount;


    tmpfs_v_ops = k_calloc(sizeof(vnode_operations));
    tmpfs_v_ops->create = tmpfs_create;
    tmpfs_v_ops->lookup = tmpfs_lookup;
    tmpfs_v_ops->mkdir = tmpfs_mkdir;

    tmpfs_f_ops = k_calloc(sizeof(file_operations));
    tmpfs_f_ops->write = tmpfs_write;
    tmpfs_f_ops->read = tmpfs_read;
    tmpfs_f_ops->open = tmpfs_open;
    tmpfs_f_ops->close = tmpfs_close;
    tmpfs_f_ops->lseek64 = tmpfs_lseek64;

    register_filesystem(tmpfs);
    vfs_mount("/","tmpfs");

}

void tmpfs_setup_mount(filesystem* fs, mount* mount){
    vnode* root_node = k_calloc(sizeof(vnode));

    mount->fs = fs;
    mount->root = root_node;

    root_node->v_ops = tmpfs_v_ops;
    root_node->f_ops = tmpfs_f_ops;
}

int tmpfs_mkdir(vnode* dir_node, vnode** target, const char* component_name){
    
    
    (*target)->f_ops = tmpfs_f_ops;
    (*target)->v_ops = tmpfs_v_ops;

    memset((*target)->children, NULL, sizeof((*target)->children));
    (*target)->n_edges = 0;

    // tree edges update
    dir_node->children[dir_node->n_edges++] = *target;
    strcpy((*target)->name, component_name);
    
    (*target)->parent = dir_node;
}

int tmpfs_create(vnode* dir_node, vnode** target, const char* component_name){
    (*target)->f_ops = tmpfs_f_ops;
    (*target)->v_ops = tmpfs_v_ops;
    (*target)->internal = k_calloc(sizeof(tmpfs_vnode));

    
    return 0;
}

int tmpfs_lookup(vnode* dir_node, vnode** target, const char* component_name){
    
    if (component_name==NULL){
        *target = dir_node;
        return 0;
    }
    if (!_strncmp(component_name, "..", 2)){
        *target = dir_node->parent;
        if (*target==NULL)
            *target = dir_node;
        return 0;
    } 
    if (!_strncmp(component_name, ".", 1)){
        *target = dir_node;
        return 0;
    }  

    for(int i=0; i<dir_node->n_edges; i++){
        vnode* child = dir_node->children[i];
        if (!strcmp(child->name, component_name)){
            *target = child;
            return 0;
        }
    }
    return -1;
}

int tmpfs_write(struct file* file, const void* buf, size_t len){
    tmpfs_vnode *tmpfs_node = (tmpfs_vnode*)file->vnode->internal;
    len = min(len, TMPFS_FILE_SIZE-(file->f_pos));
    void* internal_buf = &tmpfs_node->content[file->f_pos];
    memcpy(internal_buf, buf, len);
    file->f_pos += len;
    tmpfs_node->size = file->f_pos;
    return len;
}

int tmpfs_read(struct file* file, void* buf, size_t len){
    tmpfs_vnode *tmpfs_node = file->vnode->internal;
    int r_size = min(min(len, TMPFS_FILE_SIZE-file->f_pos), tmpfs_node->size);
    void* internal_buf = &tmpfs_node->content[file->f_pos];
    memcpy(buf, internal_buf, r_size);
    file->f_pos += len;
    return r_size;
}

int tmpfs_open(struct vnode* file_node, struct file** target){
    *target = k_calloc(sizeof(file));
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    (*target)->flags = 0;

    return 0;
}

int tmpfs_close(struct file* file){
    free(file);
}

long tmpfs_lseek64(struct file* file, long offset, int whence){

}

