/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */
#pragma once

#include <oskal/common.h>
#include <oskal/cr_status.h>
#include <oskal/cr_types.h>
#include <oskal/cr_interrupt.h>

typedef struct {
  UINT16 PdcPinBase;
  UINT16 GicSpiBase;
  UINT16 PdcCount;
} PdcRange;

typedef struct {
  UINTN     BaseAddress;
  UINTN     Size;
  UINTN     SPIConfigBase;
  UINTN     SPIConfigSize;
  PdcRange *PdcRegion;
} PdcDeviceContext;

CR_STATUS
PdcLibInit(OUT PdcDeviceContext **CtxOut);

UINT32 PdcGetGicIrqFromPdcPin(UINT16 PdcPinNumber);

CR_STATUS
PdcRegisterInterrupt(
    UINT16 PdcPinNumber, CR_INTERRUPT_HANDLER InterruptHandler, VOID *Param,
    CR_INTERRUPT_TRIGGER_TYPE Type);

CR_STATUS
PdcUnregisterInterrupt(UINT16 PdcPinNumber, CR_INTERRUPT_TRIGGER_TYPE Type);
