#include "include/vfs.h"

struct mount* rootfs;
struct filesystem* filesystems[MAX_FILESYSTEMS];
file_operations* dev_ops[MAX_DEVS];


int register_filesystem(struct filesystem* fs) {
    // register the file system to the kernel.
    // you can also initialize memory pool of the file system here.
    set_filesystem(fs);
}

int register_dev(file_operations* f_ops){
    return set_dev(f_ops);
}

int vfs_open(const char* pathname, int flags, struct file** target) {
    // 1. Lookup pathname
    // 2. Create a new file handle for this vnode if found.
    // 3. Create a new file if O_CREAT is specified in flags and vnode not found
    // lookup error code shows if file exist or not or other error occurs
    // 4. Return error code if fails
    vnode* file_node;
    int ret = vfs_lookup(pathname, &file_node);
    if (flags|O_CREAT==1){
        if (ret==0){
            return -2; // O_CREAT but file exists
        } else{
            vfs_create(pathname);
            vfs_lookup(pathname, &file_node);
        }
            
    } else {
        if (ret!=0)
            return -1; // file Not found
    }

    //file_node->f_ops->open(file_node, target);
    *target = k_calloc(sizeof(file));
    (*target)->vnode = file_node;
    (*target)->f_ops = file_node->f_ops;
    (*target)->f_pos = 0;
    (*target)->flags = 0;

    int fd = set_fd(*target);
    return fd;
}

int vfs_close(file* file) {
    // 1. release the file handle
    // 2. Return error code if fails
    
    //int ret = file->f_ops->close(file);
    free(file);
    return 0;
}

int vfs_write(struct file* file, const void* buf, size_t len) {
    // 1. write len byte from buf to the opened file.
    // 2. return written size or error code if an error occurs.
    return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file* file, void* buf, size_t len) {
    // 1. read min(len, readable size) byte to buf from the opened file.
    // 2. block if nothing to read for FIFO type
    // 2. return read size or error code if an error occurs.
    /*size_t r_size;
    while((r_size=file->f_ops->read(file, buf, len))==0);
    return r_size;
    */
   return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char* pathname){
    vnode* dir_node;
    vnode* target = k_calloc(sizeof(vnode));
    if (vfs_lookup(pathname, &dir_node)==0) 
        return -1; // Dir exists!
    char* component_name = strrchr(pathname, '/') + 1; // exclude "/"
    dir_node->v_ops->mkdir(dir_node, &target, component_name);
    return 0;
}

int vfs_create(const char* pathname){
    char* filename = strrchr(pathname, '/') + 1; // exclude "/"
    vnode* dir_node;
    vnode* target = k_calloc(sizeof(vnode));
    if (vfs_lookup(pathname, &dir_node)==0)
        return -1; // File exists!

    int ret = dir_node->v_ops->create(dir_node, &target, filename);
    if (ret!=0)
        return ret;
    // tree edges update
    strcpy(target->name, filename); 
    dir_node->children[dir_node->n_edges++] = target;
    return 0;
}

int vfs_mount(const char* target_name, const char* fs_name){
    filesystem *fs = get_filesystem(fs_name);
    mount *mnt = k_calloc(sizeof(mount));
    fs->setup_mount(fs, mnt);

    if (rootfs==NULL){
        rootfs = mnt;
    }
    else {
        vnode* target_node;
        int ret = vfs_lookup(target_name, &target_node);
        if (ret != 0)
            return ret;
 

        char* dir_name = strrchr(target_name, '/') + 1; // exclude "/"

        target_node->mount = mnt;
    }
    
    return 0;
}

int vfs_lookup(const char* pathname, struct vnode** target){

    char *tmp = k_calloc(len(pathname)+1);
    strcpy(tmp, pathname);
    char* component_name = strtok_r(tmp, "/");
    
    vnode *vnode_itr = rootfs->root;
    if (!_strncmp(component_name, ".", 1)||!_strncmp(component_name, "..", 2))
        vnode_itr = get_current_dir();

    do 
    {
        vnode* next_vnode;

        while (vnode_itr->mount!=NULL)
            vnode_itr = vnode_itr->mount->root;
        
        int ret = vnode_itr->v_ops->lookup(vnode_itr, &next_vnode, component_name);
        if (ret != 0) {
            *target = vnode_itr;
            return ret;
        }
        vnode_itr = next_vnode;
    } while((component_name=strtok_r(NULL, "/"))!=NULL);
    free(tmp);
    *target = vnode_itr;
    return 0;
}

int vfs_lseek64(struct file* file, long offset, int whence){
    if (file->f_ops->lseek64==NULL){
        if (whence==SEEK_SET)
            file->f_pos = SEEK_SET + offset;
        return file->f_pos;
    }
    
    return file->f_ops->lseek64(file, offset, whence);
}

int vfs_chdir(const char* path){
    vnode* target;
    int ret = vfs_lookup(path, &target);
    if (ret!=0)
        return ret;
    set_current_dir(target);
    return 0;
}

int vfs_mknod(const char* path, int dev_id){
    vnode* target;
    
    int ret = vfs_create(path);
    if (ret!=0)
        return ret;
    ret = vfs_lookup(path, &target);
    if (ret!=0)
        return ret;
    target->f_ops = get_dev_by_id(dev_id);
    return 0;
}

int vfs_ioctl(file* file, uint32_t req, void* buf){
    if (req==0){

    }
}

void set_filesystem(struct filesystem* fs){
    for(int i=0; i<MAX_FILESYSTEMS; i++){
        if (filesystems[i]==NULL){
            filesystems[i] = fs;
            break;
        }
    }
}

filesystem* get_filesystem(const char* fs_name){
    for(int i=0; i<MAX_FILESYSTEMS; i++){
        if (filesystems[i]!=NULL && !strcmp(filesystems[i]->name, fs_name)){
            return filesystems[i];
        }
    }
}


// file handles operation

int set_fd(file* f){
    file** fd_table = &(current_thread()->fd_table);
    for(int i=0; i<MAX_FD; i++){
        if (fd_table[i]==NULL){
            fd_table[i] = f;
            return i;
        }
    }
    return -1;
}

file* get_file_by_fd(int fd){
    if (fd<0 || fd>=MAX_FD) 
        return NULL;
    file** fd_table = &(current_thread()->fd_table);
    return fd_table[fd];
}

int clear_fd(int fd){
    if (fd<0 || fd>=MAX_FD) 
        return NULL;
    file** fd_table = &(current_thread()->fd_table);
    fd_table[fd] = NULL;
    return 0;
}

vnode* get_current_dir(){
    vnode* dir = current_thread()->current_dir_node;
    if (dir==NULL)
        dir = rootfs->root;
    return dir;
}

int set_current_dir(vnode* dir_node){
    current_thread()->current_dir_node = dir_node;
    return 0;
}

int set_dev(file_operations* f_ops){
    for(int i=0; i<MAX_DEVS; i++){
        if (dev_ops[i]==NULL){
            dev_ops[i] = f_ops;
            return i;
        }
    }
    return -1;
}

file_operations* get_dev_by_id(int id){
    if (id>=0&&id<MAX_DEVS)
        return dev_ops[id];
    return NULL;
}