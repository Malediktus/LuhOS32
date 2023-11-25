#include <kernel/dev/driver_manager.h>
#include <kernel/dev/tty/ega.h>
#include <kernel/dev/disk/ide.h>

static driver_t drivers[MAX_DRIVERS];
static uint8_t num_drivers;

static disk_t *disks[MAX_DISKS];
static uint8_t num_disks;

uint32_t driver_manager_init_early(struct multiboot_tag_framebuffer *framebuffer)
{
  uint32_t result = EOK;
  num_drivers = 0;

  switch (framebuffer->common.framebuffer_type)
  {
  case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
    result = ega_driver_init(&drivers[num_drivers], framebuffer);
    if (result != EOK)
    {
      goto out;
    }

    num_drivers++;
    break;
  default:
    break;
  }

out:
  return result;
}

uint32_t driver_manager_init()
{
  uint32_t result = EOK;

  uint8_t num_ide_disks;
  result = ide_driver_init(&drivers[num_drivers], disks + num_disks, &num_ide_disks);
  if (result != EOK)
  {
    goto out;
  }

  num_disks += num_ide_disks;
  num_drivers++;

out:
  return result;
}

uint32_t driver_manager_deinit()
{
  uint32_t result = EOK;
  for (uint8_t i = 0; i < num_drivers; i++)
  {
    switch (drivers->type)
    {
    case DRIVER_TYPE_EGA:
      result = ega_driver_deinit(&drivers[i]);
      if (result != EOK)
      {
        goto out;
      }
      break;
    default:
      break;
    }
  }

out:
  return result;
}

uint32_t driver_manager_disk_write_sector(uint32_t disk_id, uint32_t lba, uint8_t *buf)
{
  disk_t *disk = NULL;
  for (uint8_t i = 0; i < num_disks; i++)
  {
    if (disks[i]->id == disk_id)
    {
      disk = disks[i];
    }
  }

  if (disk == NULL)
  {
    return -EINVARG;
  }

  for (uint8_t i = 0; i < num_drivers; i++)
  {
    if (drivers[i].type == disk->driver_type)
    {
      switch (disk->driver_type)
      {
      case DRIVER_TYPE_IDE:
        return ide_driver_write_sector(disk, lba, buf);
      default:
        break;
      }
    }
  }

  return -EINVARG;
}

uint32_t driver_manager_disk_read_sector(uint32_t disk_id, uint32_t lba, uint8_t *buf)
{
  disk_t *disk = NULL;
  for (uint8_t i = 0; i < num_disks; i++)
  {
    if (disks[i]->id == disk_id)
    {
      disk = disks[i];
    }
  }

  if (disk == NULL)
  {
    return -EINVARG;
  }

  for (uint8_t i = 0; i < num_drivers; i++)
  {
    if (drivers[i].type == disk->driver_type)
    {
      switch (disk->driver_type)
      {
      case DRIVER_TYPE_IDE:
        return ide_driver_read_sector(disk, lba, buf);
      default:
        break;
      }
    }
  }

  return -EINVARG;
}

uint32_t get_num_disks()
{
  return num_disks;
}

disk_t **get_disks()
{
  return disks;
}

uint32_t driver_manager_get_width()
{
  for (uint8_t i = 0; i < num_drivers; i++)
  {
    switch (drivers->type)
    {
    case DRIVER_TYPE_EGA:
      return ega_driver_get_width(&drivers[i]);
    default:
      break;
    }
  }

  PANIC_PRINT("no ega_driver found");
  return 0;
}

uint32_t driver_manager_get_height()
{
  for (uint8_t i = 0; i < num_drivers; i++)
  {
    switch (drivers->type)
    {
    case DRIVER_TYPE_EGA:
      return ega_driver_get_height(&drivers[i]);
    default:
      break;
    }
  }

  PANIC_PRINT("no ega_driver found");
  return 0;
}

uint32_t driver_manager_write_tty(uint32_t x, uint32_t y, uint8_t color, const char c)
{
  uint32_t result = EOK;

  for (uint8_t i = 0; i < num_drivers; i++)
  {
    switch (drivers->type)
    {
    case DRIVER_TYPE_EGA:
      result = ega_driver_write_tty(&drivers[i], x, y, color, c);
      if (result != EOK)
      {
        goto out;
      }
      goto out;
    default:
      break;
    }
  }

out:
  return result;
}
