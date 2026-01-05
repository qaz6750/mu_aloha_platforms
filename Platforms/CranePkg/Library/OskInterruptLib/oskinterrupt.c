/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <oskal/cr_assert.h>
#include <oskal/cr_debug.h>
#include <oskal/cr_interrupt.h>
#include <oskal/cr_status.h>
#include <oskal/cr_types.h>

STATIC CR_INTERRUPT_ENTRY                mCrIntTable[MAX_INTERRUPT_ENTRIES];
STATIC EFI_HARDWARE_INTERRUPT2_PROTOCOL *mHwInterrupt2;

VOID EFIAPI CrInternalInterruptEntry(
    IN HARDWARE_INTERRUPT_SOURCE Source, IN EFI_SYSTEM_CONTEXT SystemContext)
{
  CR_INTERRUPT_ENTRY *Entry = NULL;
  for (UINTN i = 0; i < MAX_INTERRUPT_ENTRIES; i++) {
    if ((mCrIntTable[i].Source == Source) && Source &&
        (mCrIntTable[i].Handler != NULL)) {
      Entry = &mCrIntTable[i];
      Entry->Handler(Entry->Param);
      break;
    }
  }
  // End of interrupt handling
  mHwInterrupt2->EndOfInterrupt(mHwInterrupt2, Source);
}

CR_STATUS
CrInterruptInit(VOID)
{
  CR_STATUS Status;
  // Locate Hardware Interrupt2 Protocol
  Status = gBS->LocateProtocol(
      &gHardwareInterrupt2ProtocolGuid, NULL, (VOID **)&mHwInterrupt2);
  if (CR_ERROR(Status)) {
    log_err("HardwareInterrupt2 Protocol not found: %r", Status);
    CR_ASSERT(FALSE);
    return CR_NOT_FOUND;
  }
  return CR_SUCCESS;
}

CR_STATUS
CrRegisterInterrupt(
    IN UINT32 InterruptNumber, IN CR_INTERRUPT_HANDLER InterruptHandler,
    IN VOID *Param, IN CR_INTERRUPT_TRIGGER_TYPE TriggerType)
{
  EFI_STATUS Status;

  // Assuming disabling interrupt
  if (InterruptHandler == NULL) {
    return CrUnregisterInterrupt(InterruptNumber);
  }

  // Initialize protocol if not done already
  if (mHwInterrupt2 == NULL)
    if (CrInterruptInit() != CR_SUCCESS)
      return CR_NOT_FOUND;

  // Find a free entry
  {
    UINTN i;
    for (i = 0; i < MAX_INTERRUPT_ENTRIES; i++) {
      if (mCrIntTable[i].Handler == NULL) {
        mCrIntTable[i].Handler = InterruptHandler;
        mCrIntTable[i].Param   = Param;
        mCrIntTable[i].Source  = InterruptNumber;
        break;
      }
    }
    if (i == MAX_INTERRUPT_ENTRIES) {
      log_err("%a: No free interrupt entries available", __FUNCTION__);
      return CR_OUT_OF_RESOURCES;
    }
  }

  // Register irq handler
  Status = mHwInterrupt2->RegisterInterruptSource(
      mHwInterrupt2, InterruptNumber, CrInternalInterruptEntry);
  if (CR_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR,
         "%a: Failed to register interrupt handler for interrupt %u: %r\n",
         __FUNCTION__, InterruptNumber, Status));
    return Status;
  }

  // Configure interrupt trigger type
  Status = mHwInterrupt2->SetTriggerType(
      mHwInterrupt2, InterruptNumber, TriggerType);
  if (CR_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR, "%a: Failed to set trigger type for interrupt %u: %r\n",
         __FUNCTION__, InterruptNumber, Status));
    return Status;
  }

  // Enable the interrupt source
  Status = mHwInterrupt2->EnableInterruptSource(mHwInterrupt2, InterruptNumber);
  if (CR_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR, "%a: Failed to enable interrupt %u: %r\n", __FUNCTION__,
         InterruptNumber, Status));
    return Status;
  }

  return Status;
}

CR_STATUS
CrUnregisterInterrupt(IN UINT32 InterruptNumber)
{
  EFI_STATUS Status;

  if (mHwInterrupt2 == NULL)
    if (CrInterruptInit() != CR_SUCCESS)
      return CR_NOT_FOUND;

  // Disable the interrupt source
  Status =
      mHwInterrupt2->DisableInterruptSource(mHwInterrupt2, InterruptNumber);
  if (CR_ERROR(Status)) {
    DEBUG(
        (DEBUG_WARN, "%a: Failed to disable interrupt %u: %r\n", __FUNCTION__,
         InterruptNumber, Status));
  }

  return Status;
}