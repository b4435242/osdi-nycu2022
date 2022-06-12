#ifndef __VFS__
#define __VFS__

#include "stdlib.h"
#include "mm.h"
#include "mbox.h"

#define MAX_FILESYSTEMS 10
#define MAX_DEVS 10
#define O_CREAT 0100
#define SEEK_SET 0

typedef struct vnode vnode;
typedef struct vedge vedge;
typedef struct file file;
typedef struct mount mount;
typedef struct filesystem filesystem;
typedef struct file_operations file_operations;
typedef struct vnode_operations vnode_operations;

struct vnode {
  struct mount* mount;
  struct vnode_operations* v_ops;
  struct file_operations* f_ops;
  void* internal;
  
  vnode* parent;
  vnode* children[16]; 
  size_t n_edges;
  char name[16];

};

// file handle
struct file {
  struct vnode* vnode;
  size_t f_pos;  // RW position of this file handle
  struct file_operations* f_ops;
  int flags;
};

struct mount {
  struct vnode* root;
  struct filesystem* fs;
};

struct filesystem {
  const char* name;
  int (*setup_mount)(struct filesystem* fs, struct mount* mount);
};

struct file_operations {
  int (*write)(struct file* file, const void* buf, size_t len);
  int (*read)(struct file* file, void* buf, size_t len);
  int (*open)(struct vnode* file_node, struct file** target);
  int (*close)(struct file* file);
  long (*lseek64)(struct file* file, long offset, int whence);
};

struct vnode_operations {
  int (*lookup)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*create)(struct vnode* dir_node, struct vnode** target,
                const char* component_name);
  int (*mkdir)(struct vnode* dir_node, struct vnode** target,
              const char* component_name);
};


int register_dev(file_operations* f_ops);
int register_filesystem(struct filesystem* fs);
int vfs_open(const char* pathname, int flags, struct file** target);
int vfs_close(file* file);
int vfs_write(struct file* file, const void* buf, size_t len);
int vfs_read(struct file* file, void* buf, size_t len);
int vfs_lseek64(struct file* file, long offset, int whence) ;


int vfs_create(const char* pathname);
int vfs_mkdir(const char* pathname);
int vfs_mount(const char* target, const char* filesystem);
int vfs_lookup(const char* pathname, struct vnode** target);
int vfs_chdir(const char* path);
int vfs_mknod(const char* path, int dev_id);
int vfs_ioctl(file* file, uint32_t req, void* buf);

void set_filesystem(struct filesystem* fs);
filesystem* get_filesystem(const char* fs_name);


int set_fd(file* f);
file* get_file_by_fd(int fd);
int clear_fd(int fd);

int set_current_dir(vnode* dir_node);
vnode* get_current_dir();

int set_dev(file_operations* f_ops);
file_operations* get_dev_by_id(int id);

#endif