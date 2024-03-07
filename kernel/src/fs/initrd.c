#include <kernel/fs/initrd.h>
#include <kernel/lib/string.h>
#include <kernel/heap.h>

struct file_entry
{
    char filename[10];
    uint16_t num_blocks;
    uint32_t real_size;
    uint16_t start_block;
} __attribute__((packed));

typedef struct
{
    uint32_t size;
    uint32_t signature;
    struct file_entry file_table[28];
} __attribute__((packed)) initrd_header_t;

// I can use global variables here because the initrd only exists once (at least for now)
initrd_header_t *initrd_header;
fs_node_t *initrd_root;
fs_node_t *initrd_dev;

struct dirent dirent;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *, void *)
{
    if (!(node->flags & FS_FILE))
    {
        return EINVARG;
    }

    for (uint32_t i = 0; i < 28; i++)
    {
        if (initrd_header->file_table[i].filename[0] == '\0')
        {
            return NULL;
        }

        if (!strncmp(initrd_header->file_table[i].filename, node->path, 10))
        {
            struct file_entry *file_entry = &initrd_header->file_table[i];
            if (offset > file_entry->real_size)
            {
                return EINVARG;
            }

            if (offset + size > file_entry->real_size)
            {
                return EINVARG;
            }

            memcpy(buffer, (uint8_t *)((uint32_t)initrd_header + sizeof(initrd_header_t) + (file_entry->start_block * 128) + offset), size);
        }
    }

    return NULL;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index, logical_block_device_t *, void *)
{
    if (node != initrd_root)
    {
        return NULL;
    }

    if (node == initrd_root && index == 0)
    {
        strcpy(dirent.name, "dev");
        dirent.name[3] = 0;
        dirent.ino = 0;
        return &dirent;
    }

    if (index >= 28)
    {
        return NULL;
    }

    if (initrd_header->file_table[index - 1].filename[0] == '\0')
    {
        return NULL;
    }

    strcpy(dirent.name, initrd_header->file_table[index - 1].filename);
    uint32_t len = strlen(initrd_header->file_table[index - 1].filename);
    dirent.name[len >= 10 ? 9 : len] = 0;
    dirent.ino = 0;

    return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name, logical_block_device_t *, void *)
{
    if (node != initrd_root)
    {
        return NULL;
    }

    if (!strcmp(name, "dev"))
    {
        return initrd_dev;
    }

    for (uint32_t i = 0; i < 28; i++)
    {
        if (initrd_header->file_table[i].filename[0] == '\0')
        {
            return NULL;
        }

        if (!strncmp(initrd_header->file_table[i].filename, name, 10))
        {
            fs_node_t *node = kmalloc(sizeof(fs_node_t));
            strcpy(node->path, initrd_header->file_table[i].filename);
            node->mask = node->uid = node->gid = node->inode = node->length = 0;
            node->flags = FS_FILE;
            node->read = &initrd_read;
            node->write = 0;
            node->open = 0;
            node->close = 0;
            node->readdir = &initrd_readdir;
            node->finddir = &initrd_finddir;
            node->ptr = 0;
            node->impl = 0;
            node->size = initrd_header->file_table[i].real_size;

            return node;
        }
    }

    return NULL;
}

fs_node_t *initialise_initrd(uint32_t location)
{
    initrd_header = (initrd_header_t *)location;
    initrd_root = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    strcpy(initrd_root->path, "initrd");
    initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->inode = initrd_root->length = 0;
    initrd_root->flags = FS_DIRECTORY;
    initrd_root->read = 0;
    initrd_root->write = 0;
    initrd_root->open = 0;
    initrd_root->close = 0;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    initrd_root->ptr = 0;
    initrd_root->impl = 0;

    // Initialise the /dev directory (required!)
    initrd_dev = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    strcpy(initrd_dev->path, "dev");
    initrd_dev->mask = initrd_dev->uid = initrd_dev->gid = initrd_dev->inode = initrd_dev->length = 0;
    initrd_dev->flags = FS_DIRECTORY;
    initrd_dev->read = 0;
    initrd_dev->write = 0;
    initrd_dev->open = 0;
    initrd_dev->close = 0;
    initrd_dev->readdir = &initrd_readdir;
    initrd_dev->finddir = &initrd_finddir;
    initrd_dev->ptr = 0;
    initrd_dev->impl = 0;

    return initrd_root;
}

void free_initrd()
{
    kfree(initrd_root);
    kfree(initrd_dev);
}
