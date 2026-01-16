/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#include "pdc_internal.h"
#include <Library/CrPdcTargetLib.h>

// Get GIC Irq from PDC Pin Number
UINT32 PdcGetGicIrqFromPdcPin(UINT16 PdcPinNumber)
{
  PdcDeviceContext *Ctx       = GetPdcDevContext();
  PdcRange         *PdcRegion = Ctx->PdcRegion;

  while (PdcRegion->PdcCount != 0) {
    if ((PdcPinNumber >= PdcRegion->PdcPinBase) &&
        (PdcPinNumber < (PdcRegion->PdcPinBase + PdcRegion->PdcCount))) {
      return GIC_SPI(
          PdcRegion->GicSpiBase + (PdcPinNumber - PdcRegion->PdcPinBase));
    }
    PdcRegion++;
  }
  return 0; // Invalid
}

STATIC
CR_STATUS PdcGicSetType(
    IN PdcDeviceContext *Ctx, UINT16 PdcPinNumber,
    CR_INTERRUPT_TRIGGER_TYPE Type, OUT CR_INTERRUPT_TRIGGER_TYPE *GicType)
{
  UINT8 PdcIrqConfig = 0;
  *GicType           = Type;

  // Map CR_INTERRUPT_TRIGGER_TYPE to PdcIrqConfig
  switch ((UINT32)Type) {
  case CR_INTERRUPT_TRIGGER_EDGE_FALLING:
    PdcIrqConfig = PDC_IRQ_EDGE_FALLING;
    // Gic does not support falling edge, pdc will invert the signal
    *GicType = CR_INTERRUPT_TRIGGER_EDGE_RISING;
    break;
  case CR_INTERRUPT_TRIGGER_EDGE_RISING:
    PdcIrqConfig = PDC_IRQ_EDGE_RISING;
    break;
  case CR_INTERRUPT_TRIGGER_EDGE_BOTH:
    PdcIrqConfig = PDC_IRQ_EDGE_DUAL;
    *GicType     = CR_INTERRUPT_TRIGGER_EDGE_RISING;
    break;
  case CR_INTERRUPT_TRIGGER_LEVEL_LOW:
    PdcIrqConfig = PDC_IRQ_LEVEL_LOW;
    *GicType     = CR_INTERRUPT_TRIGGER_LEVEL_HIGH;
    break;
  case CR_INTERRUPT_TRIGGER_LEVEL_HIGH:
    PdcIrqConfig = PDC_IRQ_LEVEL_HIGH;
    break;
  default:
    log_err("Invalid interrupt trigger type");
    return CR_INVALID_PARAMETER;
  }

  // Linux implemented this, it seems not required here.
  // Read and check later, write a different may cause spurious interrupts
  // UINT32 OriPdcIrqCfgRegVal = CrMmioRead32(
  //     Ctx->BaseAddress + PDC_IRQ_I_CFG + PdcPinNumber * sizeof(UINT32));
  CrMmioWrite32(
      Ctx->BaseAddress + PDC_IRQ_I_CFG + PdcPinNumber * sizeof(UINT32),
      PdcIrqConfig);

  // For gpio irq configuration, set spi
  if ((Ctx->SPIConfigBase != 0) && (Type == CR_INTERRUPT_TRIGGER_LEVEL_HIGH ||
                                    Type == CR_INTERRUPT_TRIGGER_LEVEL_LOW)) {
    UINT16 GicSPI  = PdcGetGicIrqFromPdcPin(PdcPinNumber) - GIC_SPI(0);
    UINT32 PinIdx  = GicSPI >> 5;        // pin / 32
    UINT32 PinMask = BIT(GicSPI & 0x1f); // bit(pin % 32)
    if (PinIdx * sizeof(UINT32) > Ctx->SPIConfigSize) {
      log_err("Out of SPI Config Range");
      return CR_INVALID_PARAMETER;
    }

    // Read spi config register
    UINT32 SpiCfgRegVal =
        CrMmioRead32(Ctx->SPIConfigBase + PinIdx * sizeof(UINT32));
    // Clear previous config
    SpiCfgRegVal = CLR_BITS(SpiCfgRegVal, PinMask);
    // Mask if level triggered
    SpiCfgRegVal = SET_BITS(SpiCfgRegVal, PinMask);
    // Write back
    CrMmioWrite32(Ctx->SPIConfigBase + PinIdx * sizeof(UINT32), SpiCfgRegVal);
  }

  // Later, you should configure GIC type and register handler.
  return CR_SUCCESS;
}

STATIC
VOID PdcEnableInterrupt(
    IN PdcDeviceContext *Ctx, IN UINT16 PdcPinNumber, BOOLEAN Enable)
{
  UINT32 RegVal = 0;
  UINT32 Index  = PdcPinNumber >> 5;   // pin / 32
  UINT32 Mask   = PdcPinNumber & 0x1f; // pin % 32

  log_info("%a: PDC Pin idx: %d msk: %d", __FUNCTION__, Index, Mask);
  RegVal = CrMmioRead32(
      Ctx->BaseAddress + PDC_IRQ_ENABLE_BANK + Index * sizeof(UINT32));
  RegVal = Enable ? SET_BITS(RegVal, BIT(Mask)) : CLR_BITS(RegVal, BIT(Mask));
  // Write back
  CrMmioWrite32(
      Ctx->BaseAddress + PDC_IRQ_ENABLE_BANK + Index * sizeof(UINT32), RegVal);

  log_info(
      "%a: PDC Pin %d Interrupt %a", __FUNCTION__, PdcPinNumber,
      Enable ? "Enabled" : "Disabled");
  log_info(
      "%a: RegValue 0x%x", __FUNCTION__,
      CrMmioRead32(
          Ctx->BaseAddress + PDC_IRQ_ENABLE_BANK + Index * sizeof(UINT32)));
}

CR_STATUS PdcRegisterInterrupt(
    UINT16 PdcPinNumber, CR_INTERRUPT_HANDLER InterruptHandler, VOID *Param,
    CR_INTERRUPT_TRIGGER_TYPE Type)
{
  CR_STATUS         Status = CR_SUCCESS;
  UINT32            GicIrq = PdcGetGicIrqFromPdcPin(PdcPinNumber);
  PdcDeviceContext *Ctx    = GetPdcDevContext();

  // Set up Pdc and get gic type
  CR_INTERRUPT_TRIGGER_TYPE GicType;
  Status = PdcGicSetType(Ctx, PdcPinNumber, Type, &GicType);
  if (CR_ERROR(Status)) {
    log_err(
        "%a: Failed to set PDC GIC type for PDC Pin %d, Status=0x%X",
        __FUNCTION__, PdcPinNumber, Status);
    return Status;
  }

  // Enable Pdc Interrupt
  PdcEnableInterrupt(Ctx, PdcPinNumber, TRUE);

  // Register directly to interrupt controller
  log_info(
      "%a: Registering interrupt for PDC Pin %d (GIC IRQ %d) with type %d",
      __FUNCTION__, PdcPinNumber, GicIrq, GicType);
  Status = CrRegisterInterrupt(GicIrq, InterruptHandler, Param, GicType);
  if (CR_ERROR(Status)) {
    log_err(
        "%a: Failed to register interrupt for PDC Pin %d (GIC IRQ %d), "
        "Status=0x%X",
        __FUNCTION__, PdcPinNumber, GicIrq, Status);
    return Status;
  }

  return Status;
}

CR_STATUS
PdcUnregisterInterrupt(UINT16 PdcPinNumber, CR_INTERRUPT_TRIGGER_TYPE Type)
{
  CR_STATUS         Status = CR_SUCCESS;
  UINT32            GicIrq = PdcGetGicIrqFromPdcPin(PdcPinNumber);
  PdcDeviceContext *Ctx    = GetPdcDevContext();

  // Disable Pdc Interrupt
  PdcEnableInterrupt(Ctx, PdcPinNumber, FALSE);

  // Unregister directly from interrupt controller
  Status = CrUnregisterInterrupt(GicIrq);
  if (CR_ERROR(Status)) {
    log_err(
        "%a: Failed to unregister interrupt for PDC Pin %d (GIC IRQ %d), "
        "Status=0x%X",
        __FUNCTION__, PdcPinNumber, GicIrq, Status);
    return Status;
  }
  return Status;
}

STATIC
VOID PdcPinsMapping(PdcDeviceContext *Ctx)
{
  PdcRange *PdcRegion       = Ctx->PdcRegion;
  UINT32    IrqEnBankRegVal = 0;
  UINT16    RegIndex        = 0;
  UINT16    IrqIndex        = 0;

  // Clean all interrupts
  while (PdcRegion->PdcCount != 0) {
    for (UINT16 i = 0; i < PdcRegion->PdcCount; i++) {
      RegIndex        = (i + PdcRegion->PdcPinBase) >> 5;
      IrqIndex        = (i + PdcRegion->PdcPinBase) & 0x1f;
      IrqEnBankRegVal = CrMmioRead32(
          Ctx->BaseAddress + PDC_IRQ_ENABLE_BANK + (RegIndex * sizeof(UINT32)));
      IrqEnBankRegVal = CLR_BITS(IrqEnBankRegVal, IrqIndex);
      // Write back
      CrMmioWrite32(
          Ctx->BaseAddress + PDC_IRQ_ENABLE_BANK + (RegIndex * sizeof(UINT32)),
          IrqEnBankRegVal);
    }
    PdcRegion++;
  }
}

CR_STATUS
PdcLibInit(OUT PdcDeviceContext **CtxOut)
{
  if (CtxOut == NULL) {
    return CR_INVALID_PARAMETER;
  }
  *CtxOut = GetPdcDevContext();

  // Disable All IRQs in PDC
  PdcPinsMapping(*CtxOut);
  return CR_SUCCESS;
}