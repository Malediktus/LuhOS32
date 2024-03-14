#ifndef __KERNEL_VFS_H
#define __KERNEL_VFS_H

#include <kernel/types.h>
#include <kernel/dev/block_device.h>

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_SYMLINK 0x03

#define MASK_EXECUTE 1 << 1
#define MASK_READ 1 << 2
#define MASK_WRITE 1 << 3
#define MASK_HIDDEN 1 << 4
#define MASK_SYSTEM 1 << 5

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*write_type_t)(struct fs_node *, uint32_t, uint32_t, uint8_t *);
typedef struct fs_node *(*open_type_t)(const char *);
typedef void (*close_type_t)(struct fs_node *);
typedef struct dirent *(*readdir_type_t)(struct fs_node *, uint32_t);
typedef struct fs_node *(*finddir_type_t)(struct fs_node *, const char *name);

typedef struct fs_node
{
    char path[128];
    uint32_t filesize;
    uint32_t mask;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t last_access_date;
    uint32_t flags;

    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;

    logical_block_device_t *lbdev;
    void *fs_private_data;
} fs_node_t;

struct dirent
{
    char name[128];
};

extern fs_node_t *fs_root;

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
fs_node_t *open_fs(const char *path);
void close_fs(fs_node_t *node);
struct dirent *readdir_fs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fs(fs_node_t *node, const char *name);

#endif