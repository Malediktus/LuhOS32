#include <kernel/dev/disk/ide.h>
#include <kernel/heap.h>
#include <kernel/ports.h>
#include <kernel/tty.h>
#include <kernel/lib/cast.h>

#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF 0x20
#define ATA_SR_DSC 0x10
#define ATA_SR_DRQ 0x08
#define ATA_SR_CORR 0x04
#define ATA_SR_IDX 0x02
#define ATA_SR_ERR 0x01

#define ATA_ER_BBK 0x80
#define ATA_ER_UNC 0x40
#define ATA_ER_MC 0x20
#define ATA_ER_IDNF 0x10
#define ATA_ER_MCR 0x08
#define ATA_ER_ABRT 0x04
#define ATA_ER_TK0NF 0x02
#define ATA_ER_AMNF 0x01

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

#define ATAPI_CMD_READ 0xA8
#define ATAPI_CMD_EJECT 0x1B

#define ATA_IDENT_DEVICETYPE 0
#define ATA_IDENT_CYLINDERS 2
#define ATA_IDENT_HEADS 6
#define ATA_IDENT_SECTORS 12
#define ATA_IDENT_SERIAL 20
#define ATA_IDENT_MODEL 54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID 106
#define ATA_IDENT_MAX_LBA 120
#define ATA_IDENT_COMMANDSETS 164
#define ATA_IDENT_MAX_LBA_EXT 200

#define IDE_ATA 0x00
#define IDE_ATAPI 0x01

#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07
#define ATA_REG_SECCOUNT1 0x08
#define ATA_REG_LBA3 0x09
#define ATA_REG_LBA4 0x0A
#define ATA_REG_LBA5 0x0B
#define ATA_REG_CONTROL 0x0C
#define ATA_REG_ALTSTATUS 0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

// Directions:
#define ATA_READ 0x00
#define ATA_WRITE 0x01

typedef struct
{
  uint16_t flags;
  uint16_t unused1[9];
  char serial[20];
  uint16_t unused2[3];
  char firmware[8];
  char model[40];
  uint16_t sectors_per_int;
  uint16_t unused3;
  uint16_t capabilities[2];
  uint16_t unused4[2];
  uint16_t valid_ext_data;
  uint16_t unused5[5];
  uint16_t size_of_rw_mult;
  uint32_t sectors_28;
  uint16_t unused6[38];
  uint64_t sectors_48;
  uint16_t unused7[152];
} __attribute__((packed)) ata_identify_t;

#define IDE_DEVICE_FLAG_MASTER 1 << 0
#define IDE_DEVICE_FLAG_PRESENT 1 << 1

typedef struct
{
  uint16_t bus;
  uint8_t flags;
} ide_device_t;

typedef struct
{
  ide_device_t devices[8];
} ide_private_data_t;

static uint16_t ide_buses[] = {0x1F0,
                               0x1F0,
                               0x170,
                               0x170};

static void ata_io_wait(uint16_t bus)
{
  port_byte_in(bus + ATA_REG_ALTSTATUS);
  port_byte_in(bus + ATA_REG_ALTSTATUS);
  port_byte_in(bus + ATA_REG_ALTSTATUS);
  port_byte_in(bus + ATA_REG_ALTSTATUS);
}

static void ata_select(uint16_t bus, bool master)
{
  port_byte_out(bus + ATA_REG_HDDEVSEL, master ? 0xA0 : 0xB0);
}

static void ata_wait_ready(uint16_t bus)
{
  while (port_byte_in(bus + ATA_REG_STATUS) & ATA_SR_BSY)
    ;
}

static int ata_wait(uint16_t bus, int advanced)
{
  uint8_t status = 0;

  ata_io_wait(bus);

  while ((status = port_byte_in(bus + ATA_REG_STATUS)) & ATA_SR_BSY)
    ;

  if (advanced)
  {
    status = port_byte_in(bus + ATA_REG_STATUS);
    if (status & ATA_SR_ERR)
      return 1;
    if (status & ATA_SR_DF)
      return 1;
    if (!(status & ATA_SR_DRQ))
      return 1;
  }

  return 0;
}

static void ide_device_identify(ide_device_t *ide_device)
{
  port_byte_out(ide_device->bus + 1, 1);
  port_byte_out(ide_device->bus + 0x306, 0);

  ata_select(ide_device->bus + ATA_REG_HDDEVSEL, ide_device->flags & IDE_DEVICE_FLAG_MASTER);
  ata_io_wait(ide_device->bus);

  port_byte_out(ide_device->bus + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

  ata_io_wait(ide_device->bus);

  int status = port_byte_in(ide_device->bus + ATA_REG_COMMAND);
  if (status == 0x00) // Device not present
  {
    return;
  }

  ata_wait_ready(ide_device->bus);

  ata_identify_t device;
  uint16_t *buf = (uint16_t *)&device;

  for (int i = 0; i < 256; ++i)
  {
    buf[i] = port_word_in(ide_device->bus);
  }

  uint8_t *ptr = (uint8_t *)&device.model;
  for (int i = 0; i < 39; i += 2)
  {
    uint8_t tmp = ptr[i + 1];
    ptr[i + 1] = ptr[i];
    ptr[i] = tmp;
  }

  ide_device->flags |= IDE_DEVICE_FLAG_PRESENT;

  port_byte_out(ide_device->bus + ATA_REG_CONTROL, 0x02);
}

uint32_t ide_driver_init(driver_t *driver, disk_t **disks, uint8_t *num_ide_disks)
{
  *num_ide_disks = 0;

  driver->type = DRIVER_TYPE_IDE;
  driver->private_data = kmalloc(sizeof(ide_private_data_t));

  ide_private_data_t *private_data = (ide_private_data_t *)driver->private_data;

  for (uint8_t i = 0; i < 4; i++)
  {
    private_data->devices[i].bus = ide_buses[i];
    private_data->devices[i].flags = 0x00;
    if (i % 2)
    {
      private_data->devices[i].flags = IDE_DEVICE_FLAG_MASTER;
      // TODO: Fix master devices
      continue;
    }

    ide_device_identify(&private_data->devices[i]);

    if (private_data->devices[i].flags & IDE_DEVICE_FLAG_PRESENT)
    {
      disk_t *disk = kmalloc(sizeof(disk_t));
      disk->driver_type = DRIVER_TYPE_IDE;
      disk->id = i + IDE_DRIVER_DISK_ID_OFFSET;

      disks[*num_ide_disks] = disk;

      (*num_ide_disks)++;
    }
  }

  return EOK;
}

uint32_t ide_driver_write_sector(disk_t *disk, uint32_t lba, uint8_t *buf)
{
  bool master = !((disk->id - IDE_DRIVER_DISK_ID_OFFSET) % 2);
  uint16_t bus = ide_buses[disk->id - IDE_DRIVER_DISK_ID_OFFSET];

  port_byte_out(bus + ATA_REG_CONTROL, 0x02);

  ata_wait_ready(bus);

  port_byte_out(bus + ATA_REG_HDDEVSEL, (master ? 0xE0 : 0xF0) | ((lba & 0x0f000000) >> 24));
  ata_wait(bus, 0);
  port_byte_out(bus + ATA_REG_FEATURES, 0x00);
  port_byte_out(bus + ATA_REG_SECCOUNT0, 0x01);
  port_byte_out(bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
  port_byte_out(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 8);
  port_byte_out(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
  port_byte_out(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
  ata_wait(bus, 0);

  uint16_t *write_buf = (uint16_t *)buf;
  for (uint32_t i = 0; i < 256; i++)
  {
    uint16_t data = write_buf[i];
    port_word_out(bus, data);
  }

  port_byte_out(bus + 0x07, ATA_CMD_CACHE_FLUSH);
  ata_wait(bus, 0);

  return EOK;
}

uint32_t ide_driver_read_sector(disk_t *disk, uint32_t lba, uint8_t *buf)
{
  bool master = !((disk->id - IDE_DRIVER_DISK_ID_OFFSET) % 2);
  uint16_t bus = ide_buses[disk->id - IDE_DRIVER_DISK_ID_OFFSET];

  port_byte_out(bus + ATA_REG_CONTROL, 0x02);

  ata_wait_ready(bus);

  port_byte_out(bus + ATA_REG_HDDEVSEL, (master ? 0xE0 : 0xF0) | ((lba & 0x0f000000) >> 24));
  port_byte_out(bus + ATA_REG_FEATURES, 0x00);
  port_byte_out(bus + ATA_REG_SECCOUNT0, 1);
  port_byte_out(bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
  port_byte_out(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 8);
  port_byte_out(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
  port_byte_out(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

  if (ata_wait(bus, 1))
  {
    PANIC_PRINT("error during ATA read");
    return EHRDWRE;
  }

  uint16_t *read_buf = (uint16_t *)buf;
  for (uint32_t i = 0; i < 256; i++)
  {
    read_buf[i] = port_word_in(bus);
  }

  ata_wait(bus, 0);

  return EOK;
}
