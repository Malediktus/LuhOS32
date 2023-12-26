#include <kernel/dev/tty/ega.h>
#include <kernel/lib/string.h>

uint32_t ega_driver_init(driver_t *driver, struct multiboot_tag_framebuffer *framebuffer)
{
  driver->type = DRIVER_TYPE_EGA;
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

uint32_t ega_driver_scroll_line(driver_t *driver)
{
  // Calculate the address of the start and end of the screen buffer
  uint16_t *screen_buffer = (uint16_t *)0xB8000; // Address of text mode video buffer

  // Move each line up by copying the content of the line below it
  for (int i = 0; i < driver_manager_get_height() - 1; ++i)
  {
    // Calculate the start of the current and next lines
    uint16_t *current_line_start = screen_buffer + (i * 80);
    uint16_t *next_line_start = screen_buffer + ((i + 1) * 80);

    // Copy the content of the next line to the current line
    memcpy(current_line_start, next_line_start, 80 * sizeof(uint16_t));
  }

  // Clear the last line by filling it with empty spaces
  uint16_t empty_space = 0x0700; // Assuming 0x0700 represents a space in your color scheme
  for (int i = 0; i < 80; ++i)
  {
    screen_buffer[((driver_manager_get_height() - 1) * 80) + i] = empty_space;
  }

  return EOK;
}
