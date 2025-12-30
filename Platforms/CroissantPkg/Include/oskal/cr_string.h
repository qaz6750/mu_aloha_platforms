#pragma once
#include <Library/BaseMemoryLib.h>
#include <oskal/cr_types.h>

// memcmp wrapper
INTN
cr_memcmp(const VOID *dest, const VOID *src, UINTN len)
{
  return CompareMem(dest, src, len);
}

// memcpy wrapper
VOID *
cr_memcpy(VOID *dest, const VOID *src, UINTN len)
{
  return CopyMem(dest, src, len);
}

// strcmp_s wrapper
INTN
cr_strncmp(const CHAR8 *str1, const CHAR8 *str2, UINTN len)
{
  return AsciiStrnCmp(str1, str2, len);
}
