#ifndef __KERNEL_VFS_H
#define __KERNEL_VFS_H

#include <kernel/types.h>
#include <kernel/dev/block_device.h>

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_CHARDEVICE 0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE 0x05
#define FS_SYMLINK 0x06
#define FS_MOUNTPOINT 0x08

#define OPEN_MODE_READ 1 << 0
#define OPEN_MODE_WRITE 1 << 1
#define OPEN_MODE_APPEND 1 << 2
#define OPEN_MODE_BINARY 1 << 3

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *, logical_block_device_t *, void *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *, logical_block_device_t *, void *);
typedef struct fs_node *(*open_type_t)(const char *, logical_block_device_t *, void *);
typedef void (*close_type_t)(struct fs_node *, logical_block_device_t *, void *);
typedef struct dirent *(*readdir_type_t)(struct fs_node *, uint32_t, logical_block_device_t *, void *);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, char *name, logical_block_device_t *, void *);

typedef struct fs_node
{
    char path[128];
    uint32_t size;
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    struct fs_node *ptr;
    logical_block_device_t *lbdev;
    void *mountfs_private_data;
} fs_node_t;

struct dirent
{
    char name[128];
    uint32_t ino;
};

extern fs_node_t *fs_root;

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
fs_node_t *open_fs(const char *path);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

#endif