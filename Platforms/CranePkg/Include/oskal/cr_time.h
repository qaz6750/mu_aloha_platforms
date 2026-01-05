/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#pragma once
#include <Library/TimerLib.h>

#define cr_sleep(us)                                                           \
  do {                                                                         \
    MicroSecondDelay(us);                                                      \
  } while (0)
