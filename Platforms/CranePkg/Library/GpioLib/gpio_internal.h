/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */
#pragma once
#include <Library/CrTargetGpioLib.h>
#include <Library/gpio.h>
#include <oskal/common.h>
#include <oskal/cr_debug.h>
#include <oskal/cr_status.h>
#include <oskal/cr_time.h>
#include <oskal/cr_types.h>

#include <oskal/cr_interrupt.h>

CR_STATUS
GpioInitIrq(GpioDeviceContext *Context);

VOID GpioCleanIrqStatus(IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex);

CR_STATUS GpioSetInterruptCfg(
    IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex,
    IN CR_INTERRUPT_TRIGGER_TYPE TriggerType);

CR_STATUS GpioEnableInterrupt(
    IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex, BOOLEAN Enable,
    CR_INTERRUPT_TRIGGER_TYPE TriggerType);