#include <kernel/tty.h>
#include <kernel/lib/cast.h>
#include <stdarg.h>

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

static void scroll_screen()
{
  if (row < driver_manager_get_height())
  {
    return;
  }

  driver_manager_scroll_line();
  col = 0;
  row--;
}

static void kput_char(char c)
{
  uint32_t result = driver_manager_write_tty(col, row, current_color, c);
  if (result != EOK)
  {
    while (true)
    {
      // Can't print panic message, because printing faults
      asm("cli");
      asm("hlt");
    }
  }
  col++;
  if (col >= driver_manager_get_width())
  {
    col = 0;
    row++;
    scroll_screen();
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
        while (true)
        {
          // Can't print panic message, because printing faults
          asm("cli");
          asm("hlt");
        }
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
        kput_char(' ');
      }
      continue;
    }
    else if (message[i] == '\n')
    {
      col = 0;
      row++;
      scroll_screen();
      continue;
    }
    else if (message[i] == '\b')
    {
      if (col > 0)
      {
        col--;
      }
      else
      {
        row--;
        col = driver_manager_get_width() - 1;
      }

      uint32_t result = driver_manager_write_tty(col, row, current_color, ' ');
      if (result != EOK)
      {
        while (true)
        {
          // Can't print panic message, because printing faults
          asm("cli");
          asm("hlt");
        }
      }
    }

    kput_char(message[i]);
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
        kput_char(' ');
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
          break;
        }
        else if (message[i] == ';')
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
      continue;
    }
    else if (message[i] == '\n')
    {
      col = 0;
      row++;
      scroll_screen();
      continue;
    }
    else if (message[i] == '\b')
    {
      if (col > 0)
      {
        col--;
      }
      else
      {
        row--;
        col = driver_manager_get_width() - 1;
      }

      uint32_t result = driver_manager_write_tty(col, row, current_color, ' ');
      if (result != EOK)
      {
        while (true)
        {
          // Can't print panic message, because printing faults
          asm("cli");
          asm("hlt");
        }
      }
    }

    kput_char(message[i]);
  }
}

void kprint(const char *message)
{
  kprint_at(message, col, row);
}

void kprintf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  while (*fmt != '\0')
  {
    if (*fmt != '%')
    {
      if (*fmt == '\t')
      {
        for (uint8_t j = 0; j < 4; j++)
        {
          kput_char(' ');
        }
        fmt++;
      }
      else if (*fmt == '\033')
      {
        fmt++;
        if (*fmt != '[')
        {
          continue;
        }
        fmt++;

        uint8_t code = 0;
        bool bright = false;
        for (uint8_t j = 0; j < 50; j++)
        {
          if (*fmt == 'm')
          {
            fmt++;
            break;
          }
          if (*fmt == ';')
          {
            bright = true;
            fmt++;
            if (*fmt != '1')
            {
              goto out;
            }
            fmt++;
            if (*fmt != 'm')
            {
              goto out;
            }
            fmt++;
            break;
          }

          if (!(*fmt >= '0' && *fmt <= '9'))
          {
            goto out;
          }
          code *= 10;
          code += *fmt - '0';
          fmt++;
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
      else if (*fmt == '\n')
      {
        col = 0;
        row++;
        fmt++;
        scroll_screen();
      }
      else if (*fmt == '\b')
      {
        fmt++;
        if (col > 0)
        {
          col--;
        }
        else
        {
          row--;
          col = driver_manager_get_width() - 1;
        }

        uint32_t result = driver_manager_write_tty(col, row, current_color, ' ');
        if (result != EOK)
        {
          while (true)
          {
            // Can't print panic message, because printing faults
            asm("cli");
            asm("hlt");
          }
        }
      }
      else
      {
        kput_char(*fmt++);
      }
    }
    else
    {
      switch (*++fmt)
      {
      case 'd':
      case 'i':
      {
        char buf[20];
        int32_t value = va_arg(args, int32_t);
        int32_to_string(value, buf);
        kprint(buf);
        break;
      }
      case 's':
      {
        kprint(va_arg(args, char *));
        break;
      }
      case 'c':
      {
        char c = (char)va_arg(args, uint32_t);
        if (c == '\t')
        {
          for (uint8_t j = 0; j < 4; j++)
          {
            kput_char(' ');
          }
        }
        else if (c == '\n')
        {
          col = 0;
          row++;
          scroll_screen();
        }
        else if (c == '\b')
        {
          if (col > 0)
          {
            col--;
          }
          else
          {
            row--;
            col = driver_manager_get_width() - 1;
          }

          uint32_t result = driver_manager_write_tty(col, row, current_color, ' ');
          if (result != EOK)
          {
            while (true)
            {
              // Can't print panic message, because printing faults
              asm("cli");
              asm("hlt");
            }
          }
        }
        else
        {
          kput_char(c);
        }
        break;
      }
      case 'x':
      case 'X':
      {
        char buf[20];
        uint32_t value = va_arg(args, uint32_t);
        uint32_to_hex_string(value, buf);
        kprint(buf);
        break;
      }
      case 'p':
      {
        char buf[20];
        uint32_t value = va_arg(args, uintptr_t);
        uint32_to_hex_string(value, buf);
        kprint(buf);
        break;
      }
      case '%':
      {
        kput_char('%');
        break;
      }
      default:
      {
        break;
      }
      }
      fmt++;
    }
  }

  va_end(args);
}
