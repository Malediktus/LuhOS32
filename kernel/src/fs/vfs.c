#include <kernel/fs/vfs.h>

fs_node_t *fs_root = 0;

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

fs_node_t *open_fs(const char *path)
{
    if (fs_root->open != 0) // TODO: use mountpoints
        return fs_root->open(path, fs_root->lbdev, fs_root->mountfs_private_data);
    else
        return NULL;
}

void close_fs(fs_node_t *node)
{
    if (node->close != 0)
        return node->close(node, node->lbdev, node->mountfs_private_data);

    PANIC_PRINT("close_fs failed because node->close is NULL");
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
