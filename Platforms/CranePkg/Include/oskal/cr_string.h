/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#pragma once
#include <Library/BaseMemoryLib.h>
#include <oskal/cr_types.h>

// memcmp wrapper
STATIC inline INTN
cr_memcmp(const VOID *dest, const VOID *src, UINTN len)
{
  return CompareMem(dest, src, len);
}

// memcpy wrapper
STATIC inline VOID *
cr_memcpy(VOID *dest, VOID *src, UINTN len)
{
  return CopyMem(dest, src, len);
}

// strcmp_s wrapper
STATIC inline INTN
cr_strncmp(const CHAR8 *str1, const CHAR8 *str2, UINTN len)
{
  return AsciiStrnCmp(str1, str2, len);
}

// strcpm wrapper
STATIC inline INTN
cr_strcmp(const CHAR8 *str1, const CHAR8 *str2)
{
  return AsciiStrCmp(str1, str2);
}
