#include <kernel/fs/fat32.h>
#include <kernel/lib/string.h>
#include <kernel/lib/ascii.h>
#include <kernel/heap.h>
#include <kernel/dev/block_device.h>

#define BACKUP_SECTOR_NUM 6
#define ROUND_UP_INT_DIV(x, y) (x + y - 1) / y

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_INVALID (0x80 | 0x40 | 0x08)

struct FAT32_bpb
{
    uint32_t FAT_size_32;
    uint16_t ext_flags;
    uint16_t filesystem_version;
    uint32_t root_cluster;
    uint16_t filesystem_info;
    uint16_t backup_boot_sector;
    uint8_t zero1[12];
    uint8_t drive_num;
    uint8_t reserved;
    uint8_t boot_signature;
    uint32_t volume_ID;
    char volume_label[11];
    char filesystem_type[8];
    uint8_t zero2[420];
    uint16_t signature;
} __attribute__((packed));

struct bios_parameter_block
{
    uint16_t byter_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t num_FATs;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media;
    uint16_t FAT_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;

    struct FAT32_bpb FAT32;
} __attribute__((packed));

struct boot_sector
{
    uint8_t jmp_boot[3];
    char OEM_name[8];
    struct bios_parameter_block bpb;
} __attribute__((packed));

struct directory_entry
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
    uint8_t creation_time_tenth;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_hi;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed));

struct lfn_directory_entry
{
    uint8_t ord;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t first_cluster_low;
    uint16_t name3[2];
} __attribute__((packed));

static void read_fat_device(logical_block_device_t *lbdev, uint32_t lba, uint32_t num_blocks, uint8_t *buf)
{
    block_request_t request = {};
    request.bdev = lbdev->parent;
    request.buffer = buf;
    request.is_write = false;
    request.lba = lba + lbdev->lba_offset;
    request.num_blocks = num_blocks;

    submit_read_request(lbdev->parent, &request);
}

static void write_fat_device(logical_block_device_t *lbdev, uint32_t lba, uint32_t num_blocks, uint8_t *buf)
{
    block_request_t request = {};
    request.bdev = lbdev->parent;
    request.buffer = buf;
    request.is_write = true;
    request.lba = lba + lbdev->lba_offset;
    request.num_blocks = num_blocks;

    submit_write_request(lbdev->parent, &request);
}

static void verify_boot_sector(struct boot_sector *boot_sector, uint32_t bytes_per_sector)
{
    if (!(boot_sector->jmp_boot[0] == 0xEB && boot_sector->jmp_boot[2] == 0x90) && boot_sector->jmp_boot[0] != 0xE9)
    {
        PANIC_PRINT("corrupted boot sector: invalid jmp_boot field");
    }

    if (boot_sector->bpb.byter_per_sector != bytes_per_sector)
    {
        if (boot_sector->bpb.byter_per_sector == 512 || boot_sector->bpb.byter_per_sector == 1024 || boot_sector->bpb.byter_per_sector == 2048 || boot_sector->bpb.byter_per_sector == 4096)
        {
            PANIC_PRINT("corrupted bios parameter block: invalid bytes_per_sector field");
        }
        else
        {
            PANIC_PRINT("wrong bytes per sector argument specified");
        }
    }

    if (boot_sector->bpb.sectors_per_cluster == 0 || (boot_sector->bpb.sectors_per_cluster & (boot_sector->bpb.sectors_per_cluster - 1)) != 0)
    {
        PANIC_PRINT("corrupted bios parameter block: sectors_per_cluster must be power of two and non zero");
    }

    if (boot_sector->bpb.reserved_sector_count == 0)
    {
        PANIC_PRINT("corrupted bios parameter block: reserved_sector_count should be a non zero value");
    }

    if (boot_sector->bpb.root_entry_count != 0)
    {
        PANIC_PRINT("corrupted bios parameter block: not a FAT32 variant");
    }

    if (boot_sector->bpb.total_sectors_16 != 0)
    {
        PANIC_PRINT("corrupted bios parameter block: total_sectors_16 should be zero on a FAT32 variant");
    }

    if (boot_sector->bpb.media != 0xF0 && boot_sector->bpb.media < 0xF8)
    {
        PANIC_PRINT("corrupted bios parameter block: invalid media field");
    }

    if (boot_sector->bpb.FAT_size_16 != 0)
    {
        PANIC_PRINT("corrupted bios parameter block: FAT_size_16 should be zero on a FAT32 variant");
    }

    if (boot_sector->bpb.total_sectors_32 == 0)
    {
        PANIC_PRINT("corrupted bios parameter block: total_sectors_32 should be non zero on a FAT32 variant");
    }

    if (boot_sector->bpb.total_sectors_32 == 0 && boot_sector->bpb.total_sectors_16 == 0)
    {
        PANIC_PRINT("corrupted bios parameter block: total_sectors_16 and total_sectors_32 or both zero");
    }

    if (boot_sector->bpb.FAT32.filesystem_version != 0)
    {
        PANIC_PRINT("corrupted bios parameter block: filesystem_version must be zero");
    }

    if (boot_sector->bpb.FAT32.root_cluster < 2)
    {
        PANIC_PRINT("corrupted bios parameter block: root_cluster is less then two");
    }

    if (boot_sector->bpb.FAT32.backup_boot_sector != 0 && boot_sector->bpb.FAT32.backup_boot_sector != 6)
    {
        PANIC_PRINT("corrupted bios parameter block: backup_boot_sector field must be either zero or six");
    }

    if (boot_sector->bpb.FAT32.reserved != 0)
    {
        PANIC_PRINT("corrupted bios parameter block: reserved value is not zero");
    }

    if (strncmp(boot_sector->bpb.FAT32.filesystem_type, "FAT32", 5))
    {
        PANIC_PRINT("corrupted bios parameter block: invalid filesystem_type field");
    }

    if (boot_sector->bpb.FAT32.signature != 0xAA55)
    {
        PANIC_PRINT("corrupted bios parameter block: wrong boot signature");
    }

    char OEM_name[9];
    strncpy(OEM_name, boot_sector->OEM_name, 8);
    OEM_name[8] = 0;

    char volume_label[12];
    strncpy(volume_label, boot_sector->bpb.FAT32.volume_label, 11);
    volume_label[11] = 0;

    kprintf("boot sector and bios parameter block verified\nOEM name: %s\nvolume label: %s\n", OEM_name, volume_label);
}

struct boot_sector *scan_fat(uint32_t bytes_per_sector, logical_block_device_t *lbdev)
{
    struct boot_sector *boot_sector = kmalloc(bytes_per_sector);
    read_fat_device(lbdev, 0, 1, (uint8_t *)boot_sector);
    verify_boot_sector(boot_sector, bytes_per_sector);

    return boot_sector;
}

void free_fat()
{
    kfree(fs_root->fs_private_data);
    kfree(fs_root);
}

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

static void read_cluster(uint32_t cluster_num, uint8_t *buf, struct boot_sector *boot_sector, logical_block_device_t *lbdev)
{
    uint32_t fat_size = boot_sector->bpb.FAT_size_16;
    if (fat_size == 0)
    {
        fat_size = boot_sector->bpb.FAT32.FAT_size_32;
    }

    uint32_t data_start = boot_sector->bpb.reserved_sector_count + boot_sector->bpb.num_FATs * fat_size;
    uint32_t lba = data_start + cluster_num - 2;

    read_fat_device(lbdev, lba, boot_sector->bpb.sectors_per_cluster, buf);
}

static uint32_t read_fat_entry(uint32_t cluster_num, struct boot_sector *boot_sector, logical_block_device_t *lbdev)
{
    uint32_t fat_index = cluster_num * sizeof(uint32_t);
    uint32_t lba_offset = fat_index % boot_sector->bpb.byter_per_sector;
    uint32_t lba = boot_sector->bpb.reserved_sector_count + fat_index - lba_offset;

    uint8_t *fat_section = kmalloc(boot_sector->bpb.byter_per_sector);
    read_fat_device(lbdev, lba, 1, fat_section);

    uint32_t fat_entry = fat_section[lba_offset];

    kfree(fat_section);
    return fat_entry;
}

static struct directory_entry *find_entry_by_name(const char *name, uint32_t directory_cluster_num, struct boot_sector *boot_sector, logical_block_device_t *lbdev)
{
    uint32_t current_cluster = directory_cluster_num;
    uint8_t *cluster_buf = kmalloc(boot_sector->bpb.sectors_per_cluster * boot_sector->bpb.byter_per_sector);

    while (current_cluster < 0x0FFFFFF8)
    {
        read_cluster(current_cluster, cluster_buf, boot_sector, lbdev);
        struct directory_entry *direntries = (struct directory_entry *)cluster_buf;
        for (uint8_t i = 0; i < 16; i++)
        {
            if (direntries[i].name[0] == 0x00)
            {
                kfree(cluster_buf);
                return NULL;
            }

            if ((uint8_t)direntries[i].name[0] == 0xE5)
            {
                continue;
            }

            if ((direntries[i].attr & ATTR_LONG_NAME) == ATTR_LONG_NAME)
            {
                continue;
            }

            char filename[256];
            if (i >= 1 && ((direntries[i - 1].attr & ATTR_LONG_NAME) == ATTR_LONG_NAME))
            {
                uint32_t long_filename_size = 0;

                uint32_t j = i;
                while (j >= 1 && ((direntries[j - 1].attr & ATTR_LONG_NAME) == ATTR_LONG_NAME))
                {
                    j--;
                    for (uint8_t k = 0; k < 5; k++)
                    {
                        filename[long_filename_size++] = utf16_to_ascii(((struct lfn_directory_entry *)&direntries[j])->name1[k]);
                    }
                    for (uint8_t k = 0; k < 6; k++)
                    {
                        filename[long_filename_size++] = utf16_to_ascii(((struct lfn_directory_entry *)&direntries[j])->name2[k]);
                    }
                    for (uint8_t k = 0; k < 2; k++)
                    {
                        filename[long_filename_size++] = utf16_to_ascii(((struct lfn_directory_entry *)&direntries[j])->name3[k]);
                    }
                }

                filename[long_filename_size] = '\0';
            }
            else
            {
                fat32_nameext_to_name(direntries[i].nameext, filename);
            }

            if (strncmp(filename, name, 11) == 0)
            {
                struct directory_entry *res = kmalloc(sizeof(struct directory_entry));
                memcpy(res, &direntries[i], sizeof(struct directory_entry));
                kfree(cluster_buf);
                return res;
            }
        }

        current_cluster = read_fat_entry(current_cluster, boot_sector, lbdev);
    }

    kfree(cluster_buf);
    return NULL;
}

static struct directory_entry *find_entry_by_index(uint32_t index, uint32_t directory_cluster_num, char *filename /*256 bytes*/, struct boot_sector *boot_sector, logical_block_device_t *lbdev)
{
    uint32_t current_cluster = directory_cluster_num;
    uint8_t *cluster_buf = kmalloc(boot_sector->bpb.sectors_per_cluster * boot_sector->bpb.byter_per_sector);

    uint32_t j = 0;
    while (current_cluster < 0x0FFFFFF8)
    {
        read_cluster(current_cluster, cluster_buf, boot_sector, lbdev);
        struct directory_entry *direntries = (struct directory_entry *)cluster_buf;
        for (uint8_t i = 0; i < 16; i++)
        {
            if (direntries[i].name[0] == 0x00)
            {
                kfree(cluster_buf);
                return NULL;
            }

            if ((uint8_t)direntries[i].name[0] == 0xE5)
            {
                index++;
                continue;
            }

            if ((direntries[i].attr & 0x0F) == 0x0F)
            {
                index++;
                continue;
            }

            if (index == j + i)
            {
                if (i >= 1 && ((direntries[i - 1].attr & ATTR_LONG_NAME) == ATTR_LONG_NAME))
                {
                    uint32_t long_filename_size = 0;

                    uint32_t j = i;
                    while (j >= 1 && ((direntries[j - 1].attr & ATTR_LONG_NAME) == ATTR_LONG_NAME))
                    {
                        j--;
                        for (uint8_t k = 0; k < 5; k++)
                        {
                            filename[long_filename_size++] = utf16_to_ascii(((struct lfn_directory_entry *)&direntries[j])->name1[k]);
                        }
                        for (uint8_t k = 0; k < 6; k++)
                        {
                            filename[long_filename_size++] = utf16_to_ascii(((struct lfn_directory_entry *)&direntries[j])->name2[k]);
                        }
                        for (uint8_t k = 0; k < 2; k++)
                        {
                            filename[long_filename_size++] = utf16_to_ascii(((struct lfn_directory_entry *)&direntries[j])->name3[k]);
                        }
                    }

                    filename[long_filename_size] = '\0';
                }
                else
                {
                    fat32_nameext_to_name(direntries[i].nameext, filename);
                }

                struct directory_entry *res = kmalloc(sizeof(struct directory_entry));
                memcpy(res, &direntries[i], sizeof(struct directory_entry));
                kfree(cluster_buf);
                return res;
            }
        }

        current_cluster = read_fat_entry(current_cluster, boot_sector, lbdev);
        j += 16;
    }

    kfree(cluster_buf);
    return NULL;
}

static struct directory_entry *direntry_from_path(const char *path, struct boot_sector *boot_sector, logical_block_device_t *lbdev)
{
    struct directory_entry *direntry = (struct directory_entry *)1; // TODO: this is a bad way of signaling the root directory

    char *path_cpy = strdup(path);
    char *pch = strtok(path_cpy, "/");

    uint32_t current_cluster = boot_sector->bpb.FAT32.root_cluster;
    while (pch != NULL)
    {
        direntry = find_entry_by_name(pch, current_cluster, boot_sector, lbdev);
        if (!direntry)
        {
            kfree(path_cpy);
            return NULL;
        }
        current_cluster = (((uint32_t)direntry->first_cluster_hi) << 16) | ((uint32_t)direntry->first_cluster_low);
        pch = strtok(NULL, "/");
        if (pch != NULL)
        {
            kfree(direntry);
        }
    }
    kfree(path_cpy);

    return direntry;
}

static uint32_t first_cluster_from_direntry(struct directory_entry *direntry)
{
    return (((uint32_t)direntry->first_cluster_hi) << 16) | ((uint32_t)direntry->first_cluster_low);
}

static uint32_t first_cluster_from_path(const char *path, struct boot_sector *boot_sector, logical_block_device_t *lbdev)
{
    if (strcmp(path, "/") == 0) // TODO: find a more 'bulletproof' way to check (ignore whitespace, ...)
    {
        return boot_sector->bpb.FAT32.root_cluster;
    }

    struct directory_entry *direntry = direntry_from_path(path, boot_sector, lbdev);
    if (!direntry)
    {
        return -1;
    }

    return first_cluster_from_direntry(direntry);
}

uint32_t read_fat32(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    struct boot_sector *boot_sector = (struct boot_sector *)node->fs_private_data;
    logical_block_device_t *lbdev = node->lbdev;

    uint32_t current_cluster = first_cluster_from_path(node->path, boot_sector, lbdev);
    uint32_t bytes_per_cluster = boot_sector->bpb.sectors_per_cluster * boot_sector->bpb.byter_per_sector;
    uint8_t *cluster_buf = kmalloc(bytes_per_cluster);

    uint32_t current_cluster_num = 0;
    uint32_t buffer_index = 0;

    while (current_cluster < 0x0FFFFFF8)
    {
        uint32_t byte_offset = current_cluster_num * bytes_per_cluster;
        if (offset >= byte_offset && offset < (byte_offset + bytes_per_cluster))
        {
            read_cluster(current_cluster, cluster_buf, boot_sector, lbdev);
            uint32_t cluster_offset = offset - byte_offset;
            uint32_t cpy_size = (size > (bytes_per_cluster - cluster_offset)) ? (bytes_per_cluster - cluster_offset) : size;

            memcpy((void *)((uintptr_t)buffer + buffer_index), (void *)((uintptr_t)cluster_buf + cluster_offset), cpy_size);
            buffer_index += cpy_size;

            if (buffer_index == size)
            {
                kfree(cluster_buf);
                return 0;
            }
        }
        else if (offset < byte_offset && (offset + size) >= (byte_offset + bytes_per_cluster))
        {
            read_cluster(current_cluster, cluster_buf, boot_sector, lbdev);
            memcpy((void *)((uintptr_t)buffer + buffer_index), (void *)((uintptr_t)cluster_buf), bytes_per_cluster);
            buffer_index += bytes_per_cluster;
        }
        else if ((offset + size) >= byte_offset && (offset + size) < (byte_offset + bytes_per_cluster))
        {
            read_cluster(current_cluster, cluster_buf, boot_sector, lbdev);
            memcpy((void *)((uintptr_t)buffer + buffer_index), cluster_buf, size - buffer_index);
            kfree(cluster_buf);
            return 0;
        }

        current_cluster = read_fat_entry(current_cluster, boot_sector, lbdev);
        current_cluster_num++;
    }

    kfree(cluster_buf);
    return 0;
}

uint32_t write_fat32(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
    (void)node;
    (void)offset;
    (void)size;
    (void)buffer;
    return 0;
}

fs_node_t *open_fat32(const char *path);
void close_fat32(fs_node_t *node);
struct dirent *readdir_fat32(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fat32(fs_node_t *node, const char *name);

fs_node_t *open_fat32(const char *path)
{
    struct directory_entry *direntry = direntry_from_path(path, (struct boot_sector *)fs_root->fs_private_data, fs_root->lbdev);
    if (direntry == 0)
    {
        return NULL;
    }
    else if ((uintptr_t)direntry == 1)
    {
        return fs_root;
    }

    bool is_dir = (direntry->attr & ATTR_DIRECTORY) == ATTR_DIRECTORY;

    fs_node_t *fs_node = kmalloc(sizeof(fs_node_t));
    memset(fs_node, 0x00, sizeof(fs_node_t));
    strcpy(fs_node->path, path);

    if (is_dir)
    {
        fs_node->readdir = &readdir_fat32;
        fs_node->finddir = &finddir_fat32;
    }
    else
    {
        fs_node->read = &read_fat32;
        fs_node->write = &write_fat32;
    }
    fs_node->open = &open_fat32;
    fs_node->close = &close_fat32;

    fs_node->lbdev = fs_root->lbdev;
    fs_node->fs_private_data = fs_root->fs_private_data;

    fs_node->creation_time = direntry->creation_time;
    fs_node->creation_date = direntry->creation_date;
    fs_node->write_time = direntry->write_time;
    fs_node->write_date = direntry->write_date;
    fs_node->last_access_date = direntry->last_access_date;

    fs_node->mask = MASK_READ;

    if (!(direntry->attr & ATTR_READ_ONLY))
    {
        fs_node->mask |= MASK_WRITE;
    }

    if (direntry->attr & ATTR_HIDDEN)
    {
        fs_node->mask |= MASK_HIDDEN;
    }

    if (direntry->attr & ATTR_SYSTEM)
    {
        fs_node->mask |= MASK_SYSTEM;
    }

    if (is_dir)
    {
        fs_node->flags |= FS_DIRECTORY;
    }
    else
    {
        fs_node->flags |= FS_FILE;
        fs_node->filesize = direntry->file_size;

        fs_node->mask |= MASK_EXECUTE;
    }

    return fs_node;
}

void close_fat32(fs_node_t *node)
{
    if (node != fs_root)
    {
        kfree(node);
    }
}

struct dirent dirent_fat32;

struct dirent *readdir_fat32(fs_node_t *node, uint32_t index)
{
    struct boot_sector *boot_sector = (struct boot_sector *)node->fs_private_data;
    logical_block_device_t *lbdev = node->lbdev;

    uint32_t cluster = first_cluster_from_path(node->path, boot_sector, lbdev);
    if (cluster == (uint32_t)-1)
    {
        return NULL;
    }

    char filename[256];
    struct directory_entry *direntry = find_entry_by_index(index, cluster, filename, boot_sector, lbdev);
    if (!direntry)
    {
        return NULL;
    }

    kfree(direntry);

    memset(&dirent_fat32, 0, sizeof(struct dirent));
    strcpy(dirent_fat32.name, filename);
    return &dirent_fat32;
}

fs_node_t *finddir_fat32(fs_node_t *node, const char *name)
{
    struct boot_sector *boot_sector = (struct boot_sector *)node->fs_private_data;
    logical_block_device_t *lbdev = node->lbdev;

    char full_path[128];
    strcpy(full_path, node->path);
    if (full_path[strlen(full_path) - 1] != '/')
    {
        strcat(full_path, "/");
    }
    strcat(full_path, name);

    struct directory_entry *direntry = direntry_from_path(full_path, boot_sector, lbdev);
    if (!direntry)
    {
        return NULL;
    }

    bool is_dir = false;
    if ((uintptr_t)direntry == 1)
    {
        is_dir = true;
    }
    else if (direntry->attr & ATTR_DIRECTORY)
    {
        is_dir = true;
    }

    fs_node_t *fs_node = kmalloc(sizeof(fs_node_t)); // remember to call close_fs on the fs_node_t *
    memset(fs_node, 0x00, sizeof(fs_node_t));
    strcpy(fs_node->path, full_path);

    if (is_dir)
    {
        fs_node->readdir = &readdir_fat32;
        fs_node->finddir = &finddir_fat32;
    }
    else
    {
        fs_node->read = &read_fat32;
        fs_node->write = &write_fat32;
    }
    fs_node->open = &open_fat32;
    fs_node->close = &close_fat32;

    fs_node->lbdev = lbdev;
    fs_node->fs_private_data = (void *)boot_sector;

    fs_node->creation_time = direntry->creation_time;
    fs_node->creation_date = direntry->creation_date;
    fs_node->write_time = direntry->write_time;
    fs_node->write_date = direntry->write_date;
    fs_node->last_access_date = direntry->last_access_date;

    fs_node->mask = MASK_READ;

    if (!(direntry->attr & ATTR_READ_ONLY))
    {
        fs_node->mask |= MASK_WRITE;
    }

    if (direntry->attr & ATTR_HIDDEN)
    {
        fs_node->mask |= MASK_HIDDEN;
    }

    if (direntry->attr & ATTR_SYSTEM)
    {
        fs_node->mask |= MASK_SYSTEM;
    }

    if (is_dir)
    {
        fs_node->flags |= FS_DIRECTORY;
    }
    else
    {
        fs_node->flags |= FS_FILE;
        fs_node->filesize = direntry->file_size;

        fs_node->mask |= MASK_EXECUTE;
    }

    return fs_node;
}

fs_node_t *initialise_fat32(logical_block_device_t *lbdev)
{
    fs_node_t *root_node = kmalloc(sizeof(fs_node_t));
    memset(root_node, 0x00, sizeof(fs_node_t));
    strcpy(root_node->path, "/");
    root_node->flags = FS_DIRECTORY;
    root_node->readdir = &readdir_fat32;
    root_node->finddir = &finddir_fat32;
    root_node->open = &open_fat32;
    root_node->close = &close_fat32;
    root_node->lbdev = lbdev;
    root_node->fs_private_data = (void *)scan_fat(lbdev->parent->block_size, lbdev);

    return root_node;
}
