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

void uint16_to_hex_string(uint16_t value, char *hex_string)
{
  const char hex_chars[] = "0123456789ABCDEF";

  for (int i = 3; i >= 0; --i)
  {
    uint8_t nibble = (value >> (i * 4)) & 0xF;
    *hex_string++ = hex_chars[nibble];
  }

  *hex_string = '\0';
}

void uint8_to_hex_string(uint8_t value, char *hex_string)
{
  const char hex_chars[] = "0123456789ABCDEF";

  for (int i = 1; i >= 0; --i)
  {
    uint8_t nibble = (value >> (i * 4)) & 0xF;
    *hex_string++ = hex_chars[nibble];
  }

  *hex_string = '\0';
}

void uint32_to_string(uint32_t value, char *str)
{
  const char digits[] = "0123456789";

  // Handle the case of 0 separately
  if (value == 0)
  {
    *str++ = '0';
    *str = '\0';
    return;
  }

  // Determine the number of digits in the value
  int num_digits = 0;
  uint32_t temp = value;
  while (temp > 0)
  {
    temp /= 10;
    num_digits++;
  }

  // Extract digits in reverse order
  for (int i = num_digits - 1; i >= 0; --i)
  {
    uint32_t divisor = 1;
    for (int j = 0; j < i; ++j)
    {
      divisor *= 10;
    }

    uint8_t digit = (value / divisor) % 10;
    *str++ = digits[digit];
  }

  *str = '\0';
}

void uint16_to_string(uint16_t value, char *str)
{
  const char digits[] = "0123456789";

  // Handle the case of 0 separately
  if (value == 0)
  {
    *str++ = '0';
    *str = '\0';
    return;
  }

  // Determine the number of digits in the value
  int num_digits = 0;
  uint16_t temp = value;
  while (temp > 0)
  {
    temp /= 10;
    num_digits++;
  }

  // Extract digits in reverse order
  for (int i = num_digits - 1; i >= 0; --i)
  {
    uint16_t divisor = 1;
    for (int j = 0; j < i; ++j)
    {
      divisor *= 10;
    }

    uint8_t digit = (value / divisor) % 10;
    *str++ = digits[digit];
  }

  *str = '\0';
}

void uint8_to_string(uint8_t value, char *str)
{
  const char digits[] = "0123456789";

  // Handle the case of 0 separately
  if (value == 0)
  {
    *str++ = '0';
    *str = '\0';
    return;
  }

  // Determine the number of digits in the value
  int num_digits = 0;
  uint8_t temp = value;
  while (temp > 0)
  {
    temp /= 10;
    num_digits++;
  }

  // Extract digits in reverse order
  for (int i = num_digits - 1; i >= 0; --i)
  {
    uint8_t divisor = 1;
    for (int j = 0; j < i; ++j)
    {
      divisor *= 10;
    }

    uint8_t digit = (value / divisor) % 10;
    *str++ = digits[digit];
  }

  *str = '\0';
}

void int32_to_string(int32_t value, char *str)
{
  const char digits[] = "0123456789";

  // Handle the case of 0 separately
  if (value == 0)
  {
    *str++ = '0';
    *str = '\0';
    return;
  }

  // Handle negative values
  if (value < 0)
  {
    *str++ = '-';
    value = -value;
  }

  // Determine the number of digits in the value
  int num_digits = 0;
  int32_t temp = value;
  while (temp > 0)
  {
    temp /= 10;
    num_digits++;
  }

  // Extract digits in reverse order
  for (int i = num_digits - 1; i >= 0; --i)
  {
    int32_t divisor = 1;
    for (int j = 0; j < i; ++j)
    {
      divisor *= 10;
    }

    int8_t digit = (value / divisor) % 10;
    *str++ = digits[digit];
  }

  *str = '\0';
}

void int16_to_string(int16_t value, char *str)
{
  const char digits[] = "0123456789";

  // Handle the case of 0 separately
  if (value == 0)
  {
    *str++ = '0';
    *str = '\0';
    return;
  }

  // Handle negative values
  if (value < 0)
  {
    *str++ = '-';
    value = -value;
  }

  // Determine the number of digits in the value
  int num_digits = 0;
  int16_t temp = value;
  while (temp > 0)
  {
    temp /= 10;
    num_digits++;
  }

  // Extract digits in reverse order
  for (int i = num_digits - 1; i >= 0; --i)
  {
    int16_t divisor = 1;
    for (int j = 0; j < i; ++j)
    {
      divisor *= 10;
    }

    int8_t digit = (value / divisor) % 10;
    *str++ = digits[digit];
  }

  *str = '\0';
}

void int8_to_string(int8_t value, char *str)
{
  const char digits[] = "0123456789";

  // Handle the case of 0 separately
  if (value == 0)
  {
    *str++ = '0';
    *str = '\0';
    return;
  }

  // Handle negative values
  if (value < 0)
  {
    *str++ = '-';
    value = -value;
  }

  // Determine the number of digits in the value
  int num_digits = 0;
  int8_t temp = value;
  while (temp > 0)
  {
    temp /= 10;
    num_digits++;
  }

  // Extract digits in reverse order
  for (int i = num_digits - 1; i >= 0; --i)
  {
    int8_t divisor = 1;
    for (int j = 0; j < i; ++j)
    {
      divisor *= 10;
    }

    int8_t digit = (value / divisor) % 10;
    *str++ = digits[digit];
  }

  *str = '\0';
}
