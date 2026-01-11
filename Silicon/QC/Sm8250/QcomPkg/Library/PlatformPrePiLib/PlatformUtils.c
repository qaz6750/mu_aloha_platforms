/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>

#include <Library/IoLib.h>
#include <Library/PlatformPrePiLib.h>

#include "PlatformUtils.h"

VOID SetWatchdogState(BOOLEAN Enable)
{
  MmioWrite32(APSS_WDT_BASE + APSS_WDT_ENABLE_OFFSET, Enable);
}

VOID PlatformInitialize(VOID)
{
  // Disable WatchDog Timer
  SetWatchdogState(FALSE);
}