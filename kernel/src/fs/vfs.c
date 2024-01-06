#include <kernel/fs/vfs.h>

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    if (node->read != 0)
        return node->read(node, offset, size, buffer, node->lbdev, node->mountfs_private_data);
    else
        return 0;
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    if (node->write != 0)
        return node->write(node, offset, size, buffer, node->lbdev, node->mountfs_private_data);
    else
        return 0;
}

void open_fs(fs_node_t *node, uint8_t mode)
{
    if (node->open != 0)
        return node->open(node, mode, node->lbdev, node->mountfs_private_data);
    else
        return 0;
}

void close_fs(fs_node_t *node)
{
    if (node->close != 0)
        return node->close(node, node->lbdev, node->mountfs_private_data);
    else
        return 0;
}

struct dirent *readdir_fs(fs_node_t *node, uint32_t index)
{
    if ((node->flags & 0x7) == FS_DIRECTORY && node->readdir != 0)
        return node->readdir(node, index, node->lbdev, node->mountfs_private_data);
    else
        return 0;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
    if ((node->flags & 0x7) == FS_DIRECTORY && node->finddir != 0)
        return node->finddir(node, name, node->lbdev, node->mountfs_private_data);
    else
        return 0;
}
