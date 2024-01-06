#include <kernel/fs/fat32.h>

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
    uint8_t res;
    uint8_t ctime_ms;
    uint16_t ctime_time;
    uint16_t ctime_date;
    uint16_t atime_date;
    uint16_t cluster_hi;
    uint16_t mtime_time;
    uint16_t mtime_date;
    uint16_t cluster_lo;
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
    struct fat32_boot_sectors fat_boot;
    uint32_t root_cluster;
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
