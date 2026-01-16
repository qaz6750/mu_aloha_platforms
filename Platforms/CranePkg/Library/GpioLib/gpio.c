/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */
#include "gpio_internal.h"
#include <oskal/cr_memory.h>

VOID GpioInitConfigParams(
    OUT GpioConfigParams *ConfigParams, IN UINT16 PinNumber)
{
  if (ConfigParams == NULL) {
    log_err("%a: ConfigParams is NULL", __FUNCTION__);
  }
  ConfigParams->PinNumber     = PinNumber;
  ConfigParams->OutputEnable  = FALSE; // Default to Input
  ConfigParams->FunctionSel   = GPIO_FUNC_UNCHANGE;
  ConfigParams->Pull          = GPIO_PULL_UNCHANGE;
  ConfigParams->DriveStrength = GPIO_DRIVE_STRENGTH_UNCHANGE;
  ConfigParams->OutputValue   = GPIO_VALUE_UNCHANGE;
}

VOID GpioReadConfig(
    IN GpioDeviceContext *GpioContext, IN OUT GpioConfigParams *ConfigParams)
{
  if (GpioContext == NULL || ConfigParams == NULL) {
    log_err("%a: Invalid parameters", __FUNCTION__);
    return;
  }

  // Read current control register
  UINT32 GpioCtlRegVal = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_CTL_REG, ConfigParams->PinNumber));
  UINT32 GpioIoRegVal  = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_IO_REG, ConfigParams->PinNumber));

  // Map register value to structure
  GpioControlRegInfo *GpioCtlRegInfo = (VOID *)&GpioCtlRegVal;
  GpioIoRegInfo      *GpioIoRegInfo  = (VOID *)&GpioIoRegVal;

  // Fill ConfigParams
  ConfigParams->FunctionSel   = GpioCtlRegInfo->MuxSel;
  ConfigParams->OutputEnable  = GpioCtlRegInfo->OutputEnable;
  ConfigParams->DriveStrength = GpioCtlRegInfo->DriveStrength;
  ConfigParams->Pull          = GpioCtlRegInfo->Pull;

  if (!GpioCtlRegInfo->OutputEnable)
    ConfigParams->OutputValue = GPIO_VALUE_UNCHANGE;
  else
    ConfigParams->OutputValue = GpioIoRegInfo->Output & 1;
}

VOID GpioSetConfig(
    IN GpioDeviceContext *GpioContext, IN GpioConfigParams *ConfigParams)
{
  // Read current control register
  UINT32 GpioCtlRegVal = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_CTL_REG, ConfigParams->PinNumber));
  UINT32 GpioIoRegVal  = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_IO_REG, ConfigParams->PinNumber));

  // Map register value to structure
  GpioControlRegInfo *GpioCtlRegInfo = (VOID *)&GpioCtlRegVal;
  GpioIoRegInfo      *GpioIoRegInfo  = (VOID *)&GpioIoRegVal;

  // Set mux sel if changed
  if (ConfigParams->FunctionSel != GPIO_FUNC_UNCHANGE) {
    GpioCtlRegInfo->MuxSel = ConfigParams->FunctionSel;
  }

  // Set in/out ctl reg
  GpioCtlRegInfo->OutputEnable = ConfigParams->OutputEnable;

  // Set output value if output
  if (GpioCtlRegInfo->OutputEnable) {
    // Set value if output(io reg)
    if (ConfigParams->OutputValue != GPIO_VALUE_UNCHANGE)
      GpioIoRegInfo->Output = ConfigParams->OutputValue & 1;
    // Clean up pull type for output
    GpioCtlRegInfo->Pull = GPIO_PULL_NONE;
    // Set drive strength for output
    if (ConfigParams->DriveStrength != GPIO_DRIVE_STRENGTH_UNCHANGE) {
      GpioCtlRegInfo->DriveStrength = ConfigParams->DriveStrength;
    }
  }
  else {
    // Set pull type if input(ctl reg)
    if (ConfigParams->Pull != GPIO_PULL_UNCHANGE) {
      if (GpioContext->PullUpNoKeeper && ConfigParams->Pull == GPIO_PULL_UP) {
        GpioCtlRegInfo->Pull = GPIO_PULL_UP_NO_KEEPER;
      }
      else {
        GpioCtlRegInfo->Pull = ConfigParams->Pull;
      }
    }
  }

  // Write back to registers
  CrMmioWrite32(
      GpioGetCfgRegByIndex(
          GpioContext, GPIO_CFG_REG_TYPE_CTL_REG, ConfigParams->PinNumber),
      *(UINT32 *)GpioCtlRegInfo);
  CrMmioWrite32(
      GpioGetCfgRegByIndex(
          GpioContext, GPIO_CFG_REG_TYPE_IO_REG, ConfigParams->PinNumber),
      *(UINT32 *)GpioIoRegInfo);
}

GpioValueType
GpioReadIoVal(IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex)
{
  // Ignore io mode and only read current io register
  UINT32 GpioIoRegVal = CrMmioRead32(
      GpioGetCfgRegByIndex(GpioContext, GPIO_CFG_REG_TYPE_IO_REG, GpioIndex));
  GpioIoRegInfo *GpioIoRegInfo = (VOID *)&GpioIoRegVal;
  return (GpioIoRegInfo->Input) ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW;
};

VOID GpioCleanIrqStatus(IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex)
{
  // Clear interrupt status
  UINT32 IntStatusRegVal = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_INT_STATUS_REG, GpioIndex));
  GpioInterruptStatusRegInfo *IntStatusRegInfo = (VOID *)&IntStatusRegVal;
  IntStatusRegInfo->InterruptStatus            = 0;
  // Write Back
  CrMmioWrite32(
      GpioGetCfgRegByIndex(
          GpioContext, GPIO_CFG_REG_TYPE_INT_STATUS_REG, GpioIndex),
      *(UINT32 *)IntStatusRegInfo);
}

CR_STATUS GpioEnableInterrupt(
    IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex, BOOLEAN Enable,
    CR_INTERRUPT_TRIGGER_TYPE TriggerType)
{

  UINT32 RegVal = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_INT_CFG_REG, GpioIndex));

  GpioInterruptCfgRegInfo *IntCfgRegInfo = (VOID *)&RegVal;

  if (Enable) {
    IntCfgRegInfo->InterruptEn        = 1;
    IntCfgRegInfo->InterruptRawStatus = 1;
  }
  else {
    IntCfgRegInfo->InterruptEn = 0;
    // Only change raw status for level trigger
    if (TriggerType == CR_INTERRUPT_TRIGGER_LEVEL_LOW ||
        TriggerType == CR_INTERRUPT_TRIGGER_LEVEL_HIGH)
      IntCfgRegInfo->InterruptRawStatus = 0;
  }

  // Write back to register
  CrMmioWrite32(
      GpioGetCfgRegByIndex(
          GpioContext, GPIO_CFG_REG_TYPE_INT_CFG_REG, GpioIndex),
      *(UINT32 *)IntCfgRegInfo);

  return CR_SUCCESS;
}

CR_STATUS GpioSetInterruptCfg(
    IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex,
    IN CR_INTERRUPT_TRIGGER_TYPE TriggerType)
{
  CR_STATUS                   Status           = CR_SUCCESS;
  BOOLEAN                     IrqEnabled       = FALSE;
  GpioInterruptTargetRegInfo *IntTargetRegInfo = NULL;
  GpioInterruptCfgRegInfo    *IntCfgRegInfo    = NULL;
  UINT32                      RegVal;

  // If width is not 1 or 2, return directly
  if (GpioContext->InterruptDetectionWidth != 1 &&
      GpioContext->InterruptDetectionWidth != 2) {
    log_err(
        "%a: Unsupported InterruptDetectionWidth %d", __FUNCTION__,
        GpioContext->InterruptDetectionWidth);
    return CR_UNSUPPORTED;
  }

  // Route interrupt to apps processor
  RegVal           = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_INT_TARGET_REG, GpioIndex));
  IntTargetRegInfo = (VOID *)&RegVal;

  IntTargetRegInfo->Target = GPIO_INTERRUPT_TARGET_KPSS_VAL;
  CrMmioWrite32(
      GpioGetCfgRegByIndex(
          GpioContext, GPIO_CFG_REG_TYPE_INT_TARGET_REG, GpioIndex),
      *(UINT32 *)IntTargetRegInfo);

  // Configure interrupt trigger type
  RegVal        = CrMmioRead32(GpioGetCfgRegByIndex(
      GpioContext, GPIO_CFG_REG_TYPE_INT_CFG_REG, GpioIndex));
  IntCfgRegInfo = (VOID *)&RegVal;
  IrqEnabled    = IntCfgRegInfo->InterruptRawStatus;

  // Always set raw status(note: setting raw status will cause an interrupt)
  IntCfgRegInfo->InterruptRawStatus = 1;

  // Clear previous settings
  IntCfgRegInfo->InterruptDetection = 0;
  IntCfgRegInfo->InterruptPolarity  = 0;

  // Check if platform supports width(2 => support dualedge)
  if (GpioContext->InterruptDetectionWidth == 2) {
    switch ((UINT32)TriggerType) {
    case CR_INTERRUPT_TRIGGER_EDGE_RISING:
      IntCfgRegInfo->InterruptDetection = 1; // Edge Rising
      IntCfgRegInfo->InterruptPolarity  = 1;
      break;
    case CR_INTERRUPT_TRIGGER_EDGE_FALLING:
      IntCfgRegInfo->InterruptDetection = 2; // Edge Falling
      IntCfgRegInfo->InterruptPolarity  = 1;
      break;
    case CR_INTERRUPT_TRIGGER_EDGE_BOTH:
      IntCfgRegInfo->InterruptDetection = 3; // Edge Both
      IntCfgRegInfo->InterruptPolarity  = 1;
      break;
    case CR_INTERRUPT_TRIGGER_LEVEL_HIGH:
      IntCfgRegInfo->InterruptPolarity = 1; // Level High
      break;
    case CR_INTERRUPT_TRIGGER_LEVEL_LOW:
      // Ignore already cleaned bits
      break;
    default:
      log_err(
          "%a: Unsupported TriggerType %d, default to Level High", __FUNCTION__,
          TriggerType);
      Status = CR_UNSUPPORTED;
      break;
    }
  }
  else if (GpioContext->InterruptDetectionWidth == 1) {
    switch ((UINT32)TriggerType) {
    case CR_INTERRUPT_TRIGGER_EDGE_RISING:
      IntCfgRegInfo->InterruptDetection = 1; // Edge Rising
      IntCfgRegInfo->InterruptPolarity  = 1;
      break;
    case CR_INTERRUPT_TRIGGER_EDGE_FALLING:
      IntCfgRegInfo->InterruptDetection = 1; // Edge Falling
      break;
    case CR_INTERRUPT_TRIGGER_EDGE_BOTH:
      // Need workaround for both edge, ignore currently
      IntCfgRegInfo->InterruptDetection = 1;
      IntCfgRegInfo->InterruptPolarity  = 1;
      log_warn(
          "Unsupported TriggerType EDGE_BOTH for width 1, default to RISING");
      Status = CR_UNSUPPORTED;
      break;
    case CR_INTERRUPT_TRIGGER_LEVEL_HIGH:
      IntCfgRegInfo->InterruptPolarity = 1; // Level High
      break;
    case CR_INTERRUPT_TRIGGER_LEVEL_LOW:
      // Ignore already cleaned bits
      break;
    default:
      log_err(
          "%a: Unsupported TriggerType %d, default to Level High", __FUNCTION__,
          TriggerType);
      Status = CR_UNSUPPORTED;
      break;
    }
  }

  // Setting raw status will cause an interrupt, clean it
  if (!IrqEnabled)
    GpioCleanIrqStatus(GpioContext, GpioIndex);

  // Write back to register
  CrMmioWrite32(
      GpioGetCfgRegByIndex(
          GpioContext, GPIO_CFG_REG_TYPE_INT_CFG_REG, GpioIndex),
      *(UINT32 *)IntCfgRegInfo);

  return Status;
}

CR_STATUS GpioLibInit(OUT GpioDeviceContext **GpioContext)
{
  CR_STATUS Status;

  if (GpioContext == NULL)
    return CR_INVALID_PARAMETER;

  // Get Target Info
  *GpioContext = CrTargetGetGpioContext();
  if (*GpioContext == NULL) {
    log_err("CrTargetGetGpioContext failed.");
    return CR_NOT_FOUND;
  }

  // Init Gpio Interrupt
  Status = GpioInitIrq(*GpioContext);
  if (CR_ERROR(Status)) {
    log_err("GpioInitIrq failed: %r", Status);
    return Status;
  }
  return Status;
}