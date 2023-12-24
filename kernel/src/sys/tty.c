#include <kernel/tty.h>
#include <kernel/lib/cast.h>

static uint32_t row;
static uint32_t col;
static uint8_t current_color;
static uint8_t current_fg_color;
static uint8_t current_bg_color;

static uint8_t ansi_color_to_tty_color(uint8_t ansi_state, bool bright)
{
  switch (ansi_state)
  {
  case 30:
    if (bright)
    {
      return TTY_LIGHT_GRAY;
    }
    return TTY_BLACK;
  case 31:
    if (bright)
    {
      return TTY_LIGHT_RED;
    }
    return TTY_RED;
  case 32:
    if (bright)
    {
      return TTY_LIGHT_GREEN;
    }
    return TTY_GREEN;
  case 33:
    return TTY_YELLOW;
  case 34:
    if (bright)
    {
      return TTY_LIGHT_BLUE;
    }
    return TTY_BLUE;
  case 35:
    if (bright)
    {
      return TTY_LIGHT_MAGENTA;
    }
    return TTY_MAGENTA;
  case 36:
    if (bright)
    {
      return TTY_LIGHT_CYAN;
    }
    return TTY_CYAN;
  case 37:
    if (bright)
    {
      return TTY_WHITE;
    }
    return TTY_WHITE;
  case 40:
    if (bright)
    {
      return TTY_BG_LIGHT_GRAY;
    }
    return TTY_BG_BLACK;
  case 41:
    if (bright)
    {
      return TTY_BG_LIGHT_RED;
    }
    return TTY_BG_RED;
  case 42:
    if (bright)
    {
      return TTY_BG_LIGHT_GREEN;
    }
    return TTY_BG_GREEN;
  case 43:
    return TTY_BG_YELLOW;
  case 44:
    if (bright)
    {
      return TTY_BG_LIGHT_BLUE;
    }
    return TTY_BG_BLUE;
  case 45:
    if (bright)
    {
      return TTY_BG_LIGHT_MAGENTA;
    }
    return TTY_BG_MAGENTA;
  case 46:
    if (bright)
    {
      return TTY_BG_LIGHT_CYAN;
    }
    return TTY_BG_CYAN;
  case 47:
    if (bright)
    {
      return TTY_BG_WHITE;
    }
    return TTY_BG_WHITE;
  default:
    return TTY_WHITE;
  }
}

void clear_tty()
{
  for (uint32_t y = 0; y < driver_manager_get_height(); y++)
  {
    for (uint32_t x = 0; x < driver_manager_get_width(); x++)
    {
      uint32_t result = driver_manager_write_tty(x, y, TTY_COLOR(TTY_WHITE, TTY_BG_BLACK), ' ');
      if (result != EOK)
      {
        PANIC_CODE(kprint_color("failed to write to tty. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
      }
    }
  }

  row = 0;
  col = 0;
  current_fg_color = TTY_WHITE;
  current_bg_color = TTY_BG_BLACK;
  current_color = TTY_COLOR(TTY_WHITE, TTY_BG_BLACK);
}

void kprint_at_color(const char *message, uint32_t _col, uint32_t _row, uint8_t color)
{
  row = _row;
  col = _col;

  for (uint32_t i = 0; message[i] != 0; i++)
  {
    if (message[i] == '\t')
    {
      for (uint8_t j = 0; j < 4; j++)
      {
        if (col >= driver_manager_get_width())
        {
          col = 0;
          row++;
          // TODO: Scroll
          break;
        }
        uint32_t result = driver_manager_write_tty(col, row, color, ' ');
        if (result != EOK)
        {
          PANIC_CODE(kprint_color("failed to write to tty. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
        }
        col++;
      }
      continue;
    }
    else if (message[i] == '\n')
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
  row = _row;
  col = _col;

  for (uint32_t i = 0; message[i] != 0; i++)
  {
    if (message[i] == '\t')
    {
      for (uint8_t j = 0; j < 4; j++)
      {
        if (col >= driver_manager_get_width())
        {
          col = 0;
          row++;
          // TODO: Scroll
          break;
        }
        uint32_t result = driver_manager_write_tty(col, row, current_color, ' ');
        if (result != EOK)
        {
          PANIC_CODE(kprint_color("failed to write to tty. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
        }
        col++;
      }
      continue;
    }
    else if (message[i] == '\033')
    {
      i++;
      if (message[i] != '[')
      {
        continue;
      }
      i++;

      uint8_t code = 0;
      bool bright = false;
      for (uint8_t j = 0; j < 50; j++)
      {
        if (message[i] == 'm')
        {
          i++;
          break;
        }
        if (message[i] == ';')
        {
          bright = true;
          i++;
          if (message[i] != '1')
          {
            goto out;
          }
          i++;
          if (message[i] != 'm')
          {
            goto out;
          }
          i++;
          break;
        }

        if (!(message[i] >= '0' && message[i] <= '9'))
        {
          goto out;
        }
        code *= 10;
        code += message[i] - '0';
        i++;
      }

      if (code == 0)
      {
        current_fg_color = TTY_WHITE;
        current_bg_color = TTY_BG_BLACK;
        current_color = TTY_COLOR(TTY_WHITE, TTY_BG_BLACK);
        goto out;
      }

      if (code < 40)
      {
        current_fg_color = ansi_color_to_tty_color(code, bright);
        current_color = TTY_COLOR(current_fg_color, current_bg_color);
      }
      else
      {
        current_bg_color = ansi_color_to_tty_color(code, bright);
        current_color = TTY_COLOR(current_fg_color, current_bg_color);
      }
    out:
    }
    else if (message[i] == '\n')
    {
      col = 0;
      row++;
      // TODO: Scroll
      continue;
    }

    uint32_t result = driver_manager_write_tty(col, row, current_color, message[i]);
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

void kprint(const char *message)
{
  kprint_at(message, col, row);
}
