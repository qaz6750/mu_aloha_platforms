/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#pragma once
// OS Independent debug print lib
#include <Library/DebugLib.h>

#define LOG_COLOR_RESET "\x1b[0m"
#define LOG_COLOR_INFO "\x1b[97m"       /* bright white */
#define LOG_COLOR_WARN "\x1b[38;5;208m" /* orange (256-color) */
#define LOG_COLOR_ERROR "\x1b[31m"      /* red */

#define log_raw(fmt, ...)                                                      \
  DebugPrint(DEBUG_WARN, fmt LOG_COLOR_RESET, ##__VA_ARGS__)
#define log_info(fmt, ...)                                                     \
  DebugPrint(                                                                  \
      DEBUG_WARN, LOG_COLOR_INFO "[INFO] " fmt LOG_COLOR_RESET "\n",           \
      ##__VA_ARGS__)
#define log_warn(fmt, ...)                                                     \
  DebugPrint(                                                                  \
      DEBUG_WARN, LOG_COLOR_WARN "[WARN] " fmt LOG_COLOR_RESET "\n",           \
      ##__VA_ARGS__)
#define log_err(fmt, ...)                                                      \
  DebugPrint(                                                                  \
      DEBUG_ERROR,                                                             \
      LOG_COLOR_ERROR "[ERROR] " fmt " (in %a:%d)" LOG_COLOR_RESET "\n",       \
      ##__VA_ARGS__, __FILE__, __LINE__)