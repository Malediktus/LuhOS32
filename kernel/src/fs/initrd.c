#include <kernel/fs/initrd.h>
#include <kernel/lib/string.h>
#include <kernel/heap.h>

// Implementation from http://www.jamesmolloy.co.uk/

typedef struct
{
    uint32_t nfiles;
} __attribute__((packed)) initrd_header_t;

typedef struct
{
    uint8_t magic;
    int8_t name[64];
    uint32_t offset;
    uint32_t length;
} __attribute__((packed)) initrd_file_header_t;

// I can use global variables here because the initrd only exists once (at least for now)
initrd_header_t *initrd_header;
initrd_file_header_t *file_headers;
fs_node_t *initrd_root;
fs_node_t *initrd_dev;
fs_node_t *root_nodes;
int nroot_nodes;

struct dirent dirent;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *, void *)
{
    initrd_file_header_t header = file_headers[node->inode];
    if (offset > header.length)
        return 0;
    if (offset + size > header.length)
        size = header.length - offset;
    memcpy(buffer, (uint8_t *)(header.offset + offset), size);
    return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index, logical_block_device_t *, void *)
{
    if (node == initrd_root && index == 0)
    {
        strcpy(dirent.name, "dev");
        dirent.name[3] = 0;
        dirent.ino = 0;
        return &dirent;
    }

    if (index - 1 >= nroot_nodes)
        return 0;

    strcpy(dirent.name, root_nodes[index - 1].path);
    dirent.name[strlen(root_nodes[index - 1].path)] = 0;
    dirent.ino = root_nodes[index - 1].inode;
    return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name, logical_block_device_t *, void *)
{
    if (node == initrd_root &&
        !strcmp(name, "dev"))
        return initrd_dev;

    int i;
    for (i = 0; i < nroot_nodes; i++)
        if (!strcmp(name, root_nodes[i].path))
            return &root_nodes[i];
    return 0;
}

fs_node_t *initialise_initrd(uint32_t location)
{
    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *)(location + sizeof(initrd_header_t));
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

    // TODO: fix. Under some unrelated conditions which I couldn't identify yet this allocates a way to large number of bytes which crashes the kernel
    PANIC_PRINT("this code should not be run because it's faulty at the moment");

    root_nodes = (fs_node_t *)kmalloc(sizeof(fs_node_t) * initrd_header->nfiles);
    nroot_nodes = initrd_header->nfiles;

    int i;
    for (i = 0; i < initrd_header->nfiles; i++)
    {
        // Edit the file's header - currently it holds the file offset
        // relative to the start of the ramdisk. We want it relative to the start
        // of memory.
        file_headers[i].offset += location;
        // Create a new file node.
        strcpy(root_nodes[i].path, &file_headers[i].name);
        root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = FS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = 0;
        root_nodes[i].readdir = 0;
        root_nodes[i].finddir = 0;
        root_nodes[i].open = 0;
        root_nodes[i].close = 0;
        root_nodes[i].impl = 0;
    }

    return initrd_root;
}

void free_initrd()
{
    kfree(initrd_root);
    kfree(initrd_dev);
    kfree(root_nodes);
}
