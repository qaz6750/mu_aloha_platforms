#pragma once
#include <Library/IoLib.h>
#include <oskal/cr_types.h>

STATIC inline UINT32 CrMmioRead32(UINTN Address) { return MmioRead32(Address); }
STATIC inline VOID   CrMmioWrite32(UINTN Address, UINT32 Value)
{
  MmioWrite32(Address, Value);
}