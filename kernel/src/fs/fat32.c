#include <kernel/fs/fat32.h>
#include <kernel/lib/string.h>
#include <kernel/heap.h>

// TODO:
// - multiple clusters in directories
// - long filename support
// - fix memory leaks
// - use const char * instead of char * and ensure that their are not modified
// - after that remove unnecessary strdups

struct fat32_boot_info
{
    uint32_t sectors_per_fat;
    uint16_t fat_flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_sector;
    uint8_t reserved1[12];
    uint8_t driver_number;
    uint8_t ext_sig;
    uint32_t serial;
    char label[11];
    char type[8];
} __attribute__((packed));

struct fat32_boot_sectors
{
    uint8_t jmp_boot[3];
    char oemname[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_max_entries;
    uint16_t total_sectors_small;
    uint8_t media_info;
    uint16_t sectors_per_fat_small;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t fs_offset;
    uint32_t total_sectors;
    struct fat32_boot_info fat32;
} __attribute__((packed));

struct fat32_direntry
{
    union
    {
        struct
        {
            char name[8];
            char ext[3];
        };
        char nameext[11];
    };

    uint8_t attr;
    uint8_t reserved;
    uint8_t c_time_thenth;
    uint16_t c_time;
    uint16_t c_date;
    uint16_t a_time;
    uint16_t first_cluster_hi;
    uint16_t w_time;
    uint16_t w_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed));

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define VFAT_ATTR_LFN 0xf
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define VFAT_ATTR_INVAL (0x80 | 0x40 | 0x08)

struct fat32_direntry_long
{
    uint8_t seq;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t csum;
    uint16_t name2[6];
    uint16_t reserved2;
    uint16_t name3[2];
} __attribute__((__packed__));

#define VFAT_LFN_SEQ_START 0x40
#define VFAT_LFN_SEQ_DELETED 0x80
#define VFAT_LFN_SEQ_MASK 0x3f

struct fat32_private_data
{
    struct fat32_boot_sectors *fat_boot;
    uint32_t data_start;
    uint32_t root_cluster;
};

uint32_t fat32_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data);
uint32_t fat32_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data);
fs_node_t *fat32_open(const char *path, logical_block_device_t *lbdev, void *private_data);
void fat32_close(fs_node_t *node, logical_block_device_t *lbdev, void *private_data);
struct dirent *fat32_readdir(fs_node_t *node, uint32_t index, logical_block_device_t *lbdev, void *private_data);
fs_node_t *fat32_finddir(fs_node_t *node, char *name, logical_block_device_t *lbdev, void *private_data);

static char *fat32_nameext_to_name(const char *nameext, char *filename)
{
    if (nameext[0] == 0x20)
    {
        PANIC_PRINT("invalid nameext in fat32 filesystem");
    }

    uint32_t file_name_cnt = 0;
    bool before_extension = true;
    bool in_spaces = false;
    bool in_extension = false;

    for (int i = 0; i < 11; i++)
    {
        if (before_extension)
        {
            if (nameext[i] == 0x20)
            {
                before_extension = false;
                in_spaces = true;
                filename[file_name_cnt++] = '.';
            }
            else if (i == 8)
            {
                before_extension = false;
                in_spaces = true;
                filename[file_name_cnt++] = '.';
                filename[file_name_cnt++] = nameext[i];
                in_extension = true;
            }
            else
            {
                filename[file_name_cnt++] = nameext[i];
            }
        }
        else if (in_spaces)
        {
            if (nameext[i] != 0x20)
            {
                in_spaces = false;
                in_extension = true;
                filename[file_name_cnt++] = nameext[i];
            }
        }
        else if (in_extension)
        {
            if (nameext[i] == 0x20)
            {
                break;
            }
            else
            {
                filename[file_name_cnt++] = nameext[i];
            }
        }
    }

    if (strncmp(filename, ".          ", 11) == 0)
    {
        filename[0] = '.';
        filename[1] = '\0';
    }
    else if (strncmp(filename, "..         ", 11) == 0)
    {
        filename[0] = '.';
        filename[1] = '.';
        filename[2] = '\0';
    }

    if (filename[file_name_cnt - 1] == '.')
    {
        filename--;
    }
    filename[file_name_cnt] = '\0';
    return filename;
}

static uint32_t cluster_to_lba(uint32_t cluster, logical_block_device_t *lbdev, struct fat32_private_data *private_data)
{
    (void)lbdev;
    return private_data->data_start + private_data->fat_boot->sectors_per_cluster * (cluster - 2);
}

static struct fat32_direntry *find_entry_by_name(const char *name, uint32_t dir_cluster, logical_block_device_t *lbdev, struct fat32_private_data *private_data)
{
    char *name_cpy = strdup(name);
    name_cpy = strtoupper(name_cpy);
    uint8_t *cluster = kmalloc(private_data->fat_boot->bytes_per_sector * private_data->fat_boot->sectors_per_cluster);

    block_request_t request;
    request.bdev = lbdev->parent;
    request.buffer = cluster;
    request.is_write = false;
    request.lba = cluster_to_lba(dir_cluster, lbdev, private_data);
    request.num_blocks = private_data->fat_boot->sectors_per_cluster;

    submit_read_request(lbdev->parent, &request);

    struct fat32_direntry *dirents = (struct fat32_direntry *)cluster;

    for (uint8_t i = 0; i < 16; i++)
    {
        if (dirents[i].name[0] == 0x00)
        {
            break;
        }

        if ((uint8_t)dirents[i].name[0] == 0xE5)
        {
            continue;
        }

        if ((dirents[i].attr & 0x0F) == 0x0F)
        {
            continue;
        }

        char filename[20];
        fat32_nameext_to_name(dirents[i].nameext, filename);

        if (strncmp(filename, name_cpy, 11) == 0)
        {
            kfree(cluster);
            kfree(name_cpy);
            return &dirents[i];
        }
    }

    kfree(name_cpy);
    return NULL;
}

static struct fat32_direntry *find_entry_by_index(uint32_t index, uint32_t dir_cluster, logical_block_device_t *lbdev, struct fat32_private_data *private_data)
{
    uint8_t *cluster = kmalloc(private_data->fat_boot->bytes_per_sector * private_data->fat_boot->sectors_per_cluster);

    block_request_t request;
    request.bdev = lbdev->parent;
    request.buffer = cluster;
    request.is_write = false;
    request.lba = cluster_to_lba(dir_cluster, lbdev, private_data);
    request.num_blocks = private_data->fat_boot->sectors_per_cluster;

    submit_read_request(lbdev->parent, &request);

    struct fat32_direntry *dirents = (struct fat32_direntry *)cluster;

    for (uint8_t i = 0; i < 16; i++)
    {
        if (dirents[i].name[0] == 0x00)
        {
            break;
        }

        if ((uint8_t)dirents[i].name[0] == 0xE5)
        {
            index++;
            continue;
        }

        if ((dirents[i].attr & 0x0F) == 0x0F)
        {
            index++;
            continue;
        }

        char filename[20];
        fat32_nameext_to_name(dirents[i].nameext, filename);

        if (i == index)
        {
            return &dirents[i];
        }
    }

    kfree(cluster);
    return NULL;
}

static uint32_t first_cluster_from_path(const char *path, logical_block_device_t *lbdev, struct fat32_private_data *private_data)
{
    char *path_cpy = strdup(path);
    char *pch = strtok(path_cpy, "/");

    uint32_t current_cluster = private_data->root_cluster;
    while (pch != NULL)
    {
        struct fat32_direntry *direntry = find_entry_by_name(pch, current_cluster, lbdev, private_data);
        if (!direntry)
        {
            kfree(path_cpy);
            return -1;
        }
        current_cluster = (((uint32_t)direntry->first_cluster_hi) << 16) | ((uint32_t)direntry->first_cluster_low);

        char filename[20];
        fat32_nameext_to_name(direntry->nameext, filename);
        pch = strtok(NULL, "/");
    }

    kfree(path_cpy);
    return current_cluster;
}

static struct fat32_direntry *fat32_direntry_from_path(const char *path, logical_block_device_t *lbdev, struct fat32_private_data *private_data)
{
    char *path_cpy = strdup(path);
    char *pch = strtok(path_cpy, "/");

    uint32_t current_cluster = private_data->root_cluster;
    struct fat32_direntry *current_direntry = 0;
    while (pch != NULL)
    {
        current_direntry = find_entry_by_name(pch, current_cluster, lbdev, private_data);
        if (!current_direntry)
        {
            kfree(path_cpy);
            return NULL;
        }
        current_cluster = (((uint32_t)current_direntry->first_cluster_hi) << 16) | ((uint32_t)current_direntry->first_cluster_low);

        char filename[20];
        fat32_nameext_to_name(current_direntry->nameext, filename);
        pch = strtok(NULL, "/");
    }

    kfree(path_cpy);
    return current_direntry;
}

uint32_t fat32_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data)
{
    (void)node;
    (void)offset;
    (void)size;
    (void)buffer;
    (void)lbdev;
    (void)private_data;
    return 0;
}

uint32_t fat32_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data)
{
    (void)node;
    (void)offset;
    (void)size;
    (void)buffer;
    (void)lbdev;
    (void)private_data;
    return 0;
}

fs_node_t *fat32_open(const char *path, logical_block_device_t *lbdev, void *private_data)
{
    if (strcmp(path, "/") == 0)
    {
        return fs_root;
    }
    struct fat32_direntry *fat32_direntry = fat32_direntry_from_path(path, lbdev, private_data);

    if (!fat32_direntry)
    {
        return NULL;
    }

    bool is_dir = (fat32_direntry->attr & ATTR_DIRECTORY) == ATTR_DIRECTORY;

    fs_node_t *fs_node = kmalloc(sizeof(fs_node_t)); // TODO: free fs_node_t
    memset(fs_node, 0x00, sizeof(fs_node_t));
    strcpy(fs_node->path, path);

    fs_node->readdir = &fat32_readdir;
    fs_node->finddir = &fat32_finddir;
    fs_node->open = &fat32_open;
    fs_node->close = &fat32_close;
    fs_node->read = &fat32_read;
    fs_node->write = &fat32_write;
    fs_node->lbdev = lbdev;
    fs_node->mountfs_private_data = private_data;

    if (is_dir)
    {
        fs_node->flags = FS_DIRECTORY;
    }

    return fs_node;
}

void fat32_close(fs_node_t *node, logical_block_device_t *lbdev, void *private_data)
{
    (void)lbdev;
    (void)private_data;
    if (node == fs_root)
    {
        return;
    }
    kfree(node);
}

struct dirent *fat32_readdir(fs_node_t *node, uint32_t index, logical_block_device_t *lbdev, void *private_data)
{
    uint32_t cluster = first_cluster_from_path(node->path, lbdev, private_data);
    if (cluster == UINT32_MAX)
    {
        return NULL;
    }

    struct fat32_direntry *fat32_direntry = find_entry_by_index(index, cluster, lbdev, private_data);
    if (!fat32_direntry)
    {
        return NULL;
    }

    char filename[20];
    fat32_nameext_to_name(fat32_direntry->nameext, filename);

    struct dirent *dirent = kmalloc(sizeof(struct dirent)); // TODO: free dirent
    strcpy(dirent->name, filename);
    dirent->ino = 0;
    return dirent;
}

fs_node_t *fat32_finddir(fs_node_t *node, char *name, logical_block_device_t *lbdev, void *private_data)
{
    char full_path[128];
    strcpy(full_path, node->path);
    if (full_path[strlen(full_path) - 1] != '/')
    {
        strcat(full_path, "/");
    }
    strcat(full_path, name);

    struct fat32_direntry *fat32_direntry = fat32_direntry_from_path(full_path, lbdev, private_data);
    if (!fat32_direntry)
    {
        return NULL;
    }

    bool is_dir = (fat32_direntry->attr & ATTR_DIRECTORY) == ATTR_DIRECTORY;

    fs_node_t *fs_node = kmalloc(sizeof(fs_node_t)); // TODO: free fs_node_t
    memset(fs_node, 0x00, sizeof(fs_node_t));
    strcpy(fs_node->path, full_path);
    fs_node->readdir = &fat32_readdir;
    fs_node->finddir = &fat32_finddir;
    fs_node->open = &fat32_open;
    fs_node->close = &fat32_close;
    fs_node->read = &fat32_read;
    fs_node->write = &fat32_write;
    fs_node->lbdev = lbdev;
    fs_node->mountfs_private_data = private_data;

    if (is_dir)
    {
        fs_node->flags = FS_DIRECTORY;
    }

    return fs_node;
}

fs_node_t *initialise_fat32(logical_block_device_t *lbdev)
{
    fs_node_t *root_node = kmalloc(sizeof(fs_node_t));
    memset(root_node, 0x00, sizeof(fs_node_t));
    strcpy(root_node->path, "/");
    root_node->flags = FS_DIRECTORY;
    root_node->readdir = &fat32_readdir;
    root_node->finddir = &fat32_finddir;
    root_node->open = &fat32_open;
    root_node->close = &fat32_close;
    root_node->lbdev = lbdev;

    struct fat32_private_data *private_data = kmalloc(sizeof(struct fat32_private_data));

    uint32_t num_blocks = (lbdev->parent->block_size + (sizeof(struct fat32_boot_sectors) - 1)) / sizeof(struct fat32_boot_sectors);
    struct fat32_boot_sectors *boot_sectors = kmalloc(num_blocks);

    block_request_t request;
    request.bdev = lbdev->parent;
    request.buffer = (uint8_t *)boot_sectors;
    request.is_write = false;
    request.lba = lbdev->lba_offset;
    request.num_blocks = num_blocks;

    private_data->fat_boot = boot_sectors;

    submit_read_request(lbdev->parent, &request);
    root_node->mountfs_private_data = private_data;

    // configuring filesystem

    if (private_data->fat_boot->root_max_entries != 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. root_max_entries should be 0");
    }

    uint16_t root_dir_sectors = ((private_data->fat_boot->root_max_entries * 32) + (private_data->fat_boot->bytes_per_sector - 1)) / private_data->fat_boot->bytes_per_sector;
    uint32_t fat_size, total_sectors, data_sector, num_clusters;

    if (private_data->fat_boot->sectors_per_fat_small != 0)
    {
        fat_size = private_data->fat_boot->sectors_per_fat_small;
    }
    else
    {
        fat_size = private_data->fat_boot->fat32.sectors_per_fat;
    }

    if (private_data->fat_boot->total_sectors_small != 0)
    {
        total_sectors = private_data->fat_boot->total_sectors_small;
    }
    else
    {
        total_sectors = private_data->fat_boot->total_sectors;
    }

    data_sector = total_sectors - (private_data->fat_boot->reserved_sectors + (private_data->fat_boot->fat_count * fat_size) + root_dir_sectors);
    num_clusters = data_sector / private_data->fat_boot->sectors_per_cluster;

    if (num_clusters < 4085)
    {
        PANIC_PRINT("fat32 driver invoked on fat12 filesystem");
    }
    else if (num_clusters < 65525)
    {
        PANIC_PRINT("fat32 driver invoked on fat16 filesystem");
    }

    // verifying filesystem

    if (((uint8_t *)private_data->fat_boot)[510] != 0x55 &&
        ((uint8_t *)private_data->fat_boot)[511] != (uint8_t)0xAA)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. Magic value not present");
    }

    if (private_data->fat_boot->jmp_boot[0] == 0xEB)
    {
        if (private_data->fat_boot->jmp_boot[2] != 0x90)
        {
            PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. jmp_boot[2] value is wrong");
        }
    }
    else if (private_data->fat_boot->jmp_boot[0] != 0xE9)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. jmp_boot[0] value is wrong");
    }

    if (private_data->fat_boot->bytes_per_sector != 512 &&
        private_data->fat_boot->bytes_per_sector != 1024 &&
        private_data->fat_boot->bytes_per_sector != 2048 &&
        private_data->fat_boot->bytes_per_sector != 5096)
    {

        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. bytes_per_sector is wrong");
    }

    if (private_data->fat_boot->sectors_per_cluster != 1 &&
        private_data->fat_boot->sectors_per_cluster % 2 != 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. sectors_per_cluster is wrong");
    }

    if (private_data->fat_boot->sectors_per_cluster *
            private_data->fat_boot->bytes_per_sector >
        32 * 1024)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. bytes_per_sector * sectors_per_cluster is too large");
    }

    if (private_data->fat_boot->reserved_sectors == 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. reserved_sectors is zero");
    }

    if (private_data->fat_boot->fat_count < 2)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. fat_count is less than two");
    }

    if (private_data->fat_boot->root_max_entries != 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. root_max_entries must be zero");
    }

    if (private_data->fat_boot->total_sectors_small != 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. total_sectors_small must be zero");
    }

    if (private_data->fat_boot->media_info != 0xF0 &&
        private_data->fat_boot->media_info < 0xF8)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. Wrong media info");
    }

    if (private_data->fat_boot->sectors_per_fat_small != 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. Sectors per fat small must be zero");
    }

    if (private_data->fat_boot->total_sectors == 0)
    {
        PANIC_PRINT("fat32 driver invoked on illegal fat32 filesystem. total_sectors must be non-zero");
    }

    private_data->root_cluster = 0xFFFFFFF & private_data->fat_boot->fat32.root_cluster;

    uint32_t fat_start = lbdev->lba_offset + private_data->fat_boot->reserved_sectors;
    private_data->data_start = fat_start + private_data->fat_boot->fat32.sectors_per_fat * private_data->fat_boot->fat_count;

    return root_node;
}
