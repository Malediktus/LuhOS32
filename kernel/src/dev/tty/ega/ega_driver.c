#include <kernel/dev/tty/ega.h>

uint32_t ega_driver_init(driver_t *driver, struct multiboot_tag_framebuffer *framebuffer)
{
  driver->type = DRIVER_TYPE_TTY;
  driver->private_data = framebuffer;

  return EOK;
}

uint32_t ega_driver_deinit(driver_t *)
{
  return EOK;
}

uint32_t ega_driver_get_width(driver_t *driver)
{
  struct multiboot_tag_framebuffer *framebuffer = driver->private_data;
  return framebuffer->common.framebuffer_width;
}

uint32_t ega_driver_get_height(driver_t *driver)
{
  struct multiboot_tag_framebuffer *framebuffer = driver->private_data;
  return framebuffer->common.framebuffer_height;
}

uint32_t ega_driver_write_tty(driver_t *driver, uint32_t x, uint32_t y, uint8_t color, const char c)
{
  // TODO: Check if x or y are out of range
  uint8_t *vidmem = (uint8_t *)0xB8000;
  uint32_t offset = 2 * (y * ega_driver_get_width(driver) + x);

  vidmem[offset] = c;
  vidmem[offset + 1] = color;

  return EOK;
}
