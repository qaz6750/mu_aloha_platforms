/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>

#include <Library/IoLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <Library/PlatformPrePiLib.h>
#include <Library/QcomMmuDetachHelperLib.h>

#include "PlatformUtils.h"

VOID SetWatchdogState(BOOLEAN Enable)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX WDTMemoryRegion;
  LocateMemoryMapAreaByName("APSS_WDT_TMR1", &WDTMemoryRegion);

  MmioWrite32(WDTMemoryRegion.Address + APSS_WDT_ENABLE_OFFSET, Enable);
}

VOID PlatformInitialize(VOID)
{
  // Disable WatchDog Timer
  SetWatchdogState(FALSE);

  MmuDetach();
}