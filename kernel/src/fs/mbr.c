#include <kernel/fs/mbr.h>
#include <kernel/heap.h>
#include <kernel/lib/string.h>

struct partition_table_entry
{
    uint8_t bootable;

    uint8_t start_head;
    uint8_t start_sector : 6;
    uint16_t start_cylinder : 10;

    uint8_t parititon_type;

    uint8_t end_head;
    uint8_t end_sector : 6;
    uint16_t end_cylinder : 10;

    uint32_t start_lba;
    uint32_t length;
} __attribute__((packed));

struct master_boot_record
{
    uint8_t bootloader[440];
    uint32_t signature;
    uint16_t unused;
    struct partition_table_entry primary_partitions[4];
    uint16_t magic_number;
} __attribute__((packed));

static void parse_block_device(logical_block_device_t lbdevs[4], block_device_t *bdev)
{
    struct master_boot_record mbr;

    block_request_t request = {};
    request.bdev = bdev;
    request.buffer = (uint8_t *)&mbr;
    request.is_write = false;
    request.lba = 0;
    request.num_blocks = 1;

    submit_read_request(bdev, &request);

    for (uint8_t i = 0; i < 4; i++)
    {
        lbdevs[i].parent = NULL;

        if (mbr.primary_partitions[i].parititon_type == 0x00)
        {
            continue;
        }

        lbdevs[i].parent = bdev;

        if (mbr.primary_partitions[i].bootable != 0x00)
        {
            lbdevs[i].flags = LOGICAL_BLOCK_DEVICE_FLAGS_BOOTABLE;
        }

        lbdevs[i].type = mbr.primary_partitions[i].parititon_type;
        lbdevs[i].lba_offset = mbr.primary_partitions[i].start_lba;
        lbdevs[i].num_blocks = mbr.primary_partitions[i].length;
        lbdevs[i].local_id = i;
    }
}

uint32_t scan_logical_block_devices_mbr()
{
    uint32_t num_bdevs = get_num_block_devices();
    block_device_t **bdevs = get_block_devices();

    for (uint32_t i = 0; i < num_bdevs; i++)
    {
        logical_block_device_t lbdevs[4];
        parse_block_device(lbdevs, bdevs[i]);

        for (uint8_t i = 0; i < 4; i++)
        {
            if (lbdevs[i].parent == NULL)
            {
                continue;
            }

            logical_block_device_t *lbdev = kmalloc(sizeof(logical_block_device_t));
            memcpy(lbdev, &lbdevs[i], sizeof(logical_block_device_t));
            register_logical_block_device(lbdev);
        }
    }

    return EOK;
}
