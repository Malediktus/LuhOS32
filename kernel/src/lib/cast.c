#include <kernel/lib/cast.h>

void uint32_to_hex_string(uint32_t value, char *hex_string)
{
  const char hex_chars[] = "0123456789ABCDEF";

  for (int i = 7; i >= 0; --i)
  {
    uint8_t nibble = (value >> (i * 4)) & 0xF;
    *hex_string++ = hex_chars[nibble];
  }

  *hex_string = '\0';
}
