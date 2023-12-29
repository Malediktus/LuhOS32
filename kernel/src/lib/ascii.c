#include <kernel/lib/ascii.h>
#include <kernel/dev/driver_manager.h>

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
    "EINFORMAT",
    "EHRDWRE"};

const char *string_error(uint32_t error)
{
    return ErrorString[error];
}

static char *_qwertzuiop = "qwertzuiop"; // 0x10-0x1c
static char *_asdfghjkl = "asdfghjkl";
static char *_yxcvbnm = "yxcvbnm";
static char *_num = "123456789";
uint8_t key_code_to_ascii(uint32_t key)
{
    if (key == BACKSPACE_PRESSED)
        return '\b';
    if (key == 0x1C)
        return '\n';
    if (key == 0x39)
        return ' ';
    if (key == 0xE)
        return '\r';
    if (key == POINT_RELEASED)
        return '.';
    if (key == SLASH_RELEASED)
        return '/';
    if (key == ZERO_PRESSED)
        return '0';
    if (key >= ONE_PRESSED && key <= NINE_PRESSED)
        return _num[key - ONE_PRESSED];
    if (key >= 0x10 && key <= 0x1C)
    {
        return _qwertzuiop[key - 0x10];
    }
    else if (key >= 0x1E && key <= 0x26)
    {
        return _asdfghjkl[key - 0x1E];
    }
    else if (key >= 0x2C && key <= 0x32)
    {
        return _yxcvbnm[key - 0x2C];
    }
    return 0;
}
