#include <kernel/lib/ascii.h>

void int_to_ascii(int n, char str[])
{
  int i, sign;
  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do
  {
    str[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    str[i++] = '-';
  str[i] = '\0';
}

static const char *ErrorString[] = {
    "EOK",
    "EIO",
    "EINVARG",
    "ENOMEM",
    "EBADPATH",
    "EFSNOTUS",
    "ERDONLY",
    "EUNIMP",
    "EISTKN",
    "EINFORMAT"};

const char *string_error(uint32_t error)
{
  return ErrorString[error];
}
