#ifndef __INITRAMFS__
#define __INITRAMFS__

#include "vfs.h"
#include "cpio.h"

void initramfs_init();
void initramfs_setup_mount(struct filesystem *fs, struct mount *mount);
int initramfs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int initramfs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int initramfs_open(struct vnode *file_node, struct file **target);
int initramfs_close(struct file *file);
int initramfs_read(struct file *file, void *buf, size_t len);
int initramfs_write(struct file *file, void *buf, size_t len);
int initramfs_lseek64(struct file *file, long offset, int whence);

#endif