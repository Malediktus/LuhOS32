#include <kernel/tty.h>

static uint32_t row;
static uint32_t col;

void clear_tty()
{
  for (uint32_t y = 0; y < driver_manager_get_height(); y++)
  {
    for (uint32_t x = 0; x < driver_manager_get_width(); x++)
    {
      uint32_t result = driver_manager_write_tty(x, y, TTY_WHITE_ON_BLACK, ' ');
      if (result != EOK)
      {
        PANIC_CODE(kprint_color("failed to write to tty. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
      }
    }
  }

  row = 0;
  col = 0;
}

void kprint_at_color(const char *message, uint32_t _col, uint32_t _row, uint8_t color)
{
  row = _row;
  col = _col;

  for (uint32_t i = 0; message[i] != 0; i++)
  {
    if (message[i] == '\n')
    {
      col = 0;
      row++;
      // TODO: Scroll
      continue;
    }

    uint32_t result = driver_manager_write_tty(col, row, color, message[i]);
    if (result != EOK)
    {
      PANIC_CODE(kprint_color("failed to write to tty. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
    }

    col++;
    if (col >= driver_manager_get_width())
    {
      col = 0;
      row++;
      // TODO: Scroll
    }
  }
}

void kprint_color(const char *message, uint8_t color)
{
  kprint_at_color(message, col, row, color);
}

void kprint_at(const char *message, uint32_t _col, uint32_t _row)
{
  kprint_at_color(message, _col, _row, TTY_WHITE_ON_BLACK);
}

void kprint(const char *message)
{
  kprint_at_color(message, col, row, TTY_WHITE_ON_BLACK);
}
