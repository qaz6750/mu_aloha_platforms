/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#pragma once
#include <oskal/cr_status.h>
#include <oskal/cr_types.h>
#include <Protocol/HardwareInterrupt2.h>

#define CR_INTERRUPT_TRIGGER_LEVEL_LOW EFI_HARDWARE_INTERRUPT2_TRIGGER_LEVEL_LOW
#define CR_INTERRUPT_TRIGGER_LEVEL_HIGH                                        \
  EFI_HARDWARE_INTERRUPT2_TRIGGER_LEVEL_HIGH
#define CR_INTERRUPT_TRIGGER_EDGE_FALLING                                      \
  EFI_HARDWARE_INTERRUPT2_TRIGGER_EDGE_FALLING
#define CR_INTERRUPT_TRIGGER_EDGE_RISING                                       \
  EFI_HARDWARE_INTERRUPT2_TRIGGER_EDGE_RISING
#define CR_INTERRUPT_TRIGGER_TYPE EFI_HARDWARE_INTERRUPT2_TRIGGER_TYPE

typedef VOID (*CR_INTERRUPT_HANDLER)(VOID *Parameters);

typedef struct {
  CR_INTERRUPT_HANDLER Handler;
  VOID                *Param;
  UINT64               Source;
} CR_INTERRUPT_ENTRY;
// 16 is enough for a single driver linked to this library.
#define MAX_INTERRUPT_ENTRIES 16

/**
 * Register an interrupt handler for a specific interrupt vector
 *
 * @param InterruptNumber  The interrupt vector number to register
 * @param InterruptHandler The handler function to call when interrupt fires, disable irq if null.
 * @param Param            Parameter to pass to the interrupt handler
 * @param TriggerType      Interrupt trigger type (edge or level)
 *
 * @return CR_SUCCESS if successful, error code otherwise
 */
CR_STATUS
CrRegisterInterrupt(
    IN UINT32 InterruptNumber, IN CR_INTERRUPT_HANDLER InterruptHandler,
    IN VOID *Param, IN CR_INTERRUPT_TRIGGER_TYPE TriggerType);

/**
 * Unregister an interrupt handler
 *
 * @param InterruptNumber  The interrupt vector number to unregister
 *
 * @return CR_SUCCESS if successful, error code otherwise
 */
CR_STATUS
CrUnregisterInterrupt(IN UINT32 InterruptNumber);
