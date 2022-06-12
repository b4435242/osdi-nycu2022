#ifndef __TMPFS__
#define __TMPFS__

#include "vfs.h"
#include "mm.h"

#define TMPFS_FILE_SIZE PAGE_SIZE

typedef struct tmpfs_vnode tmpfs_vnode;
typedef struct vnode vnode;
struct tmpfs_vnode {
    char content[TMPFS_FILE_SIZE];
    size_t size;
};

void tmpfs_init();
void tmpfs_setup_mount(filesystem* fs, mount* mount);
int tmpfs_mkdir(vnode* dir_node, vnode** target, const char* component_name);
int tmpfs_create(vnode* dir_node, vnode** target, const char* component_name);
int tmpfs_lookup(vnode* dir_node, vnode** target, const char* component_name);

int tmpfs_write(struct file* file, const void* buf, size_t len);
int tmpfs_read(struct file* file, void* buf, size_t len);
int tmpfs_open(struct vnode* file_node, struct file** target);
int tmpfs_close(struct file* file);
long tmpfs_lseek64(struct file* file, long offset, int whence);

#endif