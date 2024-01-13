#include <kernel/lib/string.h>
#include <kernel/heap.h>

void *memcpy(void *dest, const void *src, uint32_t n)
{
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  for (uint32_t i = 0; i < n; i++)
  {
    pdest[i] = psrc[i];
  }

  return dest;
}

void *memset(void *s, int c, uint32_t n)
{
  uint8_t *p = (uint8_t *)s;

  for (uint32_t i = 0; i < n; i++)
  {
    p[i] = (uint8_t)c;
  }

  return s;
}

void *memmove(void *dest, const void *src, uint32_t n)
{
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest)
  {
    for (uint32_t i = 0; i < n; i++)
    {
      pdest[i] = psrc[i];
    }
  }
  else if (src < dest)
  {
    for (uint32_t i = n; i > 0; i--)
    {
      pdest[i - 1] = psrc[i - 1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, uint32_t n)
{
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (uint32_t i = 0; i < n; i++)
  {
    if (p1[i] != p2[i])
    {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}

void *memchr(const void *s, unsigned char c, uint32_t n)
{
  if (n != 0)
  {
    const uint8_t *p = s;

    do
    {
      if (*p++ == c)
        return ((void *)(p - 1));
    } while (--n != 0);
  }
  return (NULL);
}

void *rawmemchr(const void *s, int c)
{
  if (c != '\0')
    return memchr(s, c, (size_t)-1);

  return (char *)s + strlen(s);
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1 && (*s1 == *s2))
  {
    s1++;
    s2++;
  }
  return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}

int strncmp(const char *s1, const char *s2, uint32_t n)
{
  if (n == 0)
    return (0);
  do
  {
    if (*s1 != *s2++)
      return (*(unsigned char *)s1 - *(unsigned char *)--s2);
    if (*s1++ == 0)
      break;
  } while (--n != 0);
  return (0);
}

char *strchr(const char *s, int c)
{
  do
  {
    if (*s == c)
    {
      return (char *)s;
    }
  } while (*s++);
  return (0);
}

uint32_t strspn(const char *s1, const char *s2)
{
  uint32_t i, j;
  i = 0;

  while (*(s1 + i))
  {
    j = 0;
    while (*(s2 + j))
    {
      if (*(s1 + i) == *(s2 + j))
      {
        break; // Found a match.
      }
      j++;
    }
    if (!*(s2 + j))
    {
      return i; // No match found.
    }
    i++;
  }
  return i;
}

uint32_t strcspn(const char *s1, const char *s2)
{
  uint32_t len = 0;

  if ((s1 == NULL) || (s2 == NULL))
    return len;

  while (*s1)
  {
    if (strchr(s2, *s1))
    {
      return len;
    }
    else
    {
      s1++;
      len++;
    }
  }
  return len;
}

uint32_t strlen(const char *str)
{
  uint32_t len = 0;
  while (*str != '\0')
  {
    len++;
    str++;
  }
  return len;
}

char *strdup(const char *src)
{
  int src_size;
  static char *dup;
  char *dup_offset;

  src_size = strlen(src);
  dup = kmalloc(src_size + 1);
  if (dup == NULL)
  {
    return NULL;
  }

  dup_offset = dup;
  while (*src)
  {
    *dup_offset = *src;
    dup_offset++;
    src++;
  }
  *dup_offset = '\0';

  return dup;
}

char *strndup(char *src, uint32_t size)
{
  uint32_t src_size;
  static char *dup;

  src_size = strlen(src);
  if (src_size > size)
  {
    src_size = size;
  }

  dup = kmalloc(src_size + 1);
  if (dup == NULL)
  {
    return NULL;
  }

  for (uint32_t i = 0; i < src_size; i++)
  {
    dup[i] = src[i];
  }
  dup[src_size] = '\0';

  return dup;
}

char *strcat(char *dest, const char *src)
{
  char *ptr = dest + strlen(dest);

  while (*src != '\0')
  {
    *ptr++ = *src++;
  }

  *ptr = '\0';

  return dest;
}

char *strpbrk(char *s1, const char *s2)
{
  if ((s1 == NULL) || (s2 == NULL))
    return NULL;
  while (*s1)
  {
    if (strchr(s2, *s1))
    {
      return s1;
    }
    else
    {
      s1++;
    }
  }
  return NULL;
}

static char *olds;

char *strtok(char *s, const char *delim)
{
  char *token;

  if (s == NULL)
    s = olds;

  s += strspn(s, delim);
  if (*s == '\0')
  {
    olds = s;
    return NULL;
  }

  token = s;
  s = strpbrk(token, delim);
  if (s == NULL)
    olds = rawmemchr(token, '\0');
  else
  {
    *s = '\0';
    olds = s + 1;
  }
  return token;
}

char *strcpy(char *dest, const char *src)
{
  char *temp = dest;
  while (*dest++ = *src++)
    ;
  return temp;
}

char *strtoupper(char *str)
{
  for (char *p = str; *p != '\0'; p++)
  {
    if (*p >= 'a' && *p <= 'z')
      *p -= 32;
  }
  return str;
}
