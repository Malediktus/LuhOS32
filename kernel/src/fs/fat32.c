#include <kernel/fs/fat32.h>
#include <kernel/lib/string.h>
#include <kernel/heap.h>

struct fat32_boot_info
{
    uint32_t sectors_per_fat;
    uint16_t fat_flags;
    uint16_t version;
    uint32_t root_cluster_lba;
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
    char name[8];
    char ext[3];

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

#define VFAT_LFN_SEQ_START 0x40
#define VFAT_LFN_SEQ_DELETED 0x80
#define VFAT_LFN_SEQ_MASK 0x3f

struct fat32_private_data
{
    struct fat32_boot_sectors *fat_boot;
    uint32_t root_cluster_lba;
};

uint32_t fat32_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data)
{
}

uint32_t fat32_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data)
{
}

void fat32_open(fs_node_t *node, uint8_t mode, logical_block_device_t *lbdev, void *private_data)
{
}

void fat32_close(fs_node_t *node, logical_block_device_t *lbdev, void *private_data)
{
}

struct dirent *fat32_readdir(fs_node_t *node, uint32_t index, logical_block_device_t *lbdev, void *private_data)
{
}

fs_node_t *fat32_finddir(fs_node_t *node, char *name, logical_block_device_t *lbdev, void *private_data)
{
}

void iterate_dir(struct fat32_boot_sectors *boot_sectors, logical_block_device_t *lbdev, uint32_t start_lba)
{
    uint8_t *cluster = kmalloc(boot_sectors->bytes_per_sector * boot_sectors->sectors_per_cluster);

    block_request_t request;
    request.bdev = lbdev->parent;
    request.buffer = cluster;
    request.is_write = false;
    request.lba = start_lba;
    request.num_blocks = boot_sectors->sectors_per_cluster;

    submit_read_request(lbdev->parent, &request);

    struct fat32_direntry *entries = (struct fat32_direntry *)cluster;
    for (uint8_t i = 0; i < 16; i++)
    {
        if (entries[i].name[0] == 0x00)
        {
            break;
        }

        if ((entries[i].attr & 0x0F) == 0x0F)
        {
            continue;
        }

        char filename[9];
        strcpy(filename, entries[i].name);
        filename[8] = 0;
        kprintf("file found: %s\n", filename);

        if (entries[i].attr & ATTR_DIRECTORY)
        {
            kprintf("\tdirectory\nstart\n");
            uint32_t fat_start = lbdev->lba_offset + boot_sectors->reserved_sectors;
            uint32_t data_start = fat_start + boot_sectors->fat32.sectors_per_fat * boot_sectors->fat_count;
            uint32_t new_cluster = ((uint32_t)entries[i].first_cluster_hi) << 16 | ((uint32_t)entries[i].first_cluster_low);
            uint32_t new_start_lba = data_start + boot_sectors->sectors_per_cluster * (new_cluster - 2);
            kprintf("new_cluster: 0x%x, new lba: 0x%x\n", new_cluster, new_start_lba);
            iterate_dir(boot_sectors, lbdev, new_start_lba);
        }
    }

    kfree(cluster);
    kprintf("end\n");
}

fs_node_t *initialise_fat32(logical_block_device_t *lbdev)
{
    fs_node_t *root_node = kmalloc(sizeof(fs_node_t));
    memset(root_node, 0x00, sizeof(fs_node_t));
    strcpy(root_node->name, "/");
    root_node->flags = FS_DIRECTORY;
    root_node->readdir = &fat32_readdir;
    root_node->finddir = &fat32_finddir;
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

    submit_read_request(lbdev->parent, &request);
    root_node->mountfs_private_data = private_data;

    uint32_t fat_start = lbdev->lba_offset + boot_sectors->reserved_sectors;
    uint32_t data_start = fat_start + boot_sectors->fat32.sectors_per_fat * boot_sectors->fat_count;
    uint32_t root_start_lba = data_start + boot_sectors->sectors_per_cluster * (boot_sectors->fat32.root_cluster_lba - 2);

    private_data->fat_boot = boot_sectors;
    private_data->root_cluster_lba = root_start_lba;

    kprintf("start\n");
    iterate_dir(boot_sectors, lbdev, root_start_lba);

    return root_node;
}
