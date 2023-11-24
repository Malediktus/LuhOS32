#include <kernel/dev/driver_manager.h>
#include <kernel/dev/tty/ega.h>

static driver_t drivers[MAX_DRIVERS];
static uint8_t num_drivers;

uint32_t driver_manager_init(struct multiboot_tag_framebuffer *framebuffer)
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

uint32_t driver_manager_deinit()
{
  uint32_t result = EOK;
  for (uint8_t i = 0; i < num_drivers; i++)
  {
    switch (drivers->type)
    {
    case DRIVER_TYPE_TTY:
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

uint32_t driver_manager_get_width()
{
  for (uint8_t i = 0; i < num_drivers; i++)
  {
    switch (drivers->type)
    {
    case DRIVER_TYPE_TTY:
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
    case DRIVER_TYPE_TTY:
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
    case DRIVER_TYPE_TTY:
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
