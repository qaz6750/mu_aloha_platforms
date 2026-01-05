/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#include "rpmh_internal.h"

/** Reference
  linux/drivers/soc/qcom/rpmh-rsc.c
*/
STATIC RpmhDrvRegisters drv_registers_v2p7 = {
    .reg_drv_solver_config      = 0x04,
    .reg_drv_print_child_config = 0x0C,

    .reg_drv_irq_enable = 0x00,
    .reg_drv_irq_status = 0x04,
    .reg_drv_irq_clear  = 0x08,
    .reg_drv_control    = 0x14,
    .reg_drv_cmd_enable = 0x1C,
    .reg_drv_cmd_msgid  = 0x30,
    .reg_drv_cmd_addr   = 0x34,
    .reg_drv_cmd_data   = 0x38,

    .reg_rsc_drv_cmd_offset = 0x14,
    .reg_rsc_drv_tcs_offset = 0x2A0,
};

STATIC RpmhDrvRegisters drv_registers_v3p0 = {
    .reg_drv_solver_config      = 0x04,
    .reg_drv_print_child_config = 0x0C,

    .reg_drv_irq_enable = 0x00,
    .reg_drv_irq_status = 0x04,
    .reg_drv_irq_clear  = 0x08,
    .reg_drv_control    = 0x24,
    .reg_drv_cmd_enable = 0x2C,
    .reg_drv_cmd_msgid  = 0x34,
    .reg_drv_cmd_addr   = 0x38,
    .reg_drv_cmd_data   = 0x3C,

    .reg_rsc_drv_cmd_offset = 0x18,
    .reg_rsc_drv_tcs_offset = 0x2A0,
};

STATIC VOID TcsEnableInterrupt(
    RpmhDeviceContext *RpmhContext, UINT32 TcsIndex, BOOLEAN Enable)
{
  UINT32 RegVal;
  UINT32 TcsIrqEnableAddress = RpmhContext->drv_base_address +
                               RpmhContext->tcs_offset +
                               RpmhContext->drv_registers->reg_drv_irq_enable;
  RegVal = CrMmioRead32(TcsIrqEnableAddress);
  if (Enable)
    RegVal = SET_BITS(RegVal, TcsIndex); // enable irq
  else
    RegVal = CLR_BITS(RegVal, TcsIndex); // disable irq

  CrMmioWrite32(TcsIrqEnableAddress, RegVal);
}

STATIC
VOID TcsSetTrigger(
    RpmhDeviceContext *RpmhContext, UINT32 TcsIndex, BOOLEAN Trigger)
{
  UINT32 RegVal;
  RegVal = ReadTcsReg(
      RpmhContext, RpmhContext->drv_registers->reg_drv_control, TcsIndex);

  RegVal = CLR_BITS(TCS_AMC_MODE_TRIGGER, RegVal);
  // Write
  WriteTcsRegSync(
      RpmhContext, RpmhContext->drv_registers->reg_drv_control, TcsIndex,
      RegVal);
  RegVal = CLR_BITS(TCS_AMC_MODE_ENABLE, RegVal);
  WriteTcsRegSync(
      RpmhContext, RpmhContext->drv_registers->reg_drv_control, TcsIndex,
      RegVal);
  if (Trigger) {
    RegVal = TCS_AMC_MODE_ENABLE;
    WriteTcsRegSync(
        RpmhContext, RpmhContext->drv_registers->reg_drv_control, TcsIndex,
        RegVal);
    RegVal |= TCS_AMC_MODE_TRIGGER;
    WriteTcsReg(
        RpmhContext, RpmhContext->drv_registers->reg_drv_control, TcsIndex,
        RegVal);
  }
}

// drv tx done/ drv isr
VOID RpmhDrvTcsTxDoneIsr(VOID *Params)
{
  UINT32             IrqStatus;
  RpmhDeviceContext *RpmhContext = (RpmhDeviceContext *)Params;

  log_info("RpmhDrvTcsTxDoneIsr called");
  if (!RpmhContext->IrqFromRpmhCr) {
    log_info("RpmhDrvTcsTxDoneIsr: Not from RpmhCr, ignore");
    log_info("Entering BSP Driver... Disable interrupt");
    // Unregister interrupt
    CrUnregisterInterrupt(RpmhContext->interrupt_no);
    return;
  }
  // Unset flag
  RpmhContext->IrqFromRpmhCr = FALSE;

  // Read irq status
  IrqStatus = CrMmioRead32(
      RpmhContext->drv_base_address + RpmhContext->tcs_offset +
      RpmhContext->drv_registers->reg_drv_irq_status);
  log_info("RpmhDrvTcsTxDoneIsr IrqStatus=0x%X", IrqStatus);

  // Check each bit and clear
  for (UINTN i = 0; i < RpmhContext->NumCmdsPerTcs; i++) {
    if (IrqStatus & BIT(i)) {

      // Set trigger
      TcsSetTrigger(RpmhContext, i, FALSE);

      // Enable TCS again
      WriteTcsReg(
          RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_enable, i, 0);
      // Clear irq
      CrMmioWrite32(
          RpmhContext->drv_base_address + RpmhContext->tcs_offset +
              RpmhContext->drv_registers->reg_drv_irq_clear,
          BIT(i));
      log_info("3");
      if (!RpmhContext->tcs_config.active_tcs.tcs_count) {
        // Disable irq for wake tcs
        TcsEnableInterrupt(RpmhContext, i, FALSE);
        log_info("1");
      }
      log_info("2");
      // Clear busy flag
      RpmhContext->TcsBusy = CLR_BITS(BIT(i), RpmhContext->TcsBusy);
    }
  }
  log_info("RpmhDrvTcsTxDoneIsr exit");
}

// Check if this TCS is busy
BOOLEAN RpmhIsTcsBusy(RpmhDeviceContext *RpmhContext, UINT32 TcsIndex)
{
  return (RpmhContext->TcsBusy & BIT(TcsIndex)) != 0;
}

// Check if cmd is processing (inflight)
BOOLEAN
RpmhIsCmdInflight(RpmhDeviceContext *RpmhContext, UINT32 CmdRequestAddress)
{
  // Only check busy tcs
  for (UINT32 tcs_inuse = RpmhContext->tcs_config.active_tcs.offset;
       tcs_inuse < RpmhContext->tcs_config.active_tcs.offset +
                       RpmhContext->tcs_config.active_tcs.tcs_count;
       tcs_inuse++) {
    if (RpmhIsTcsBusy(RpmhContext, tcs_inuse)) {
      // Check cmd addr running
      UINT32 CurrentEnable = ReadTcsReg(
          RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_enable,
          tcs_inuse);
      for (UINTN cmd_idx = 0; cmd_idx < RPMH_MAX_CMDS_EACH_TCS; cmd_idx++) {
        if (!(CurrentEnable & BIT(cmd_idx))) {
          continue;
        }
        UINT32 Address = ReadTcsCmdReg(
            RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_addr,
            tcs_inuse, cmd_idx);
        log_info(
            "%a: TCS %u, CMD %u Address=0x%X", __FUNCTION__, tcs_inuse, cmd_idx,
            Address);
        if (CmdDBIsAddrEqual(CmdRequestAddress, Address)) {
          log_warn(
              "%a: Cmd address 0x%X is inflight in TCS %u, CMD %u BUSY!",
              __FUNCTION__, CmdRequestAddress, tcs_inuse, cmd_idx);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

// Find free tcs id
STATIC INT32
GetNextFreeTcs(IN RpmhDeviceContext *RpmhContext, IN RpmhDrvTcsData *TcsData)
{
  for (UINT32 i = TcsData->offset; i < TcsData->offset + TcsData->tcs_count;
       i++) {
    if (!RpmhIsTcsBusy(RpmhContext, i)) {
      return i;
    }
  }
  // No free tcs
  log_warn("%a: No free TCS found!", __FUNCTION__);
  return -CR_NOT_FOUND;
}

STATIC
INT32
ClaimFreeTcs(
    IN RpmhDeviceContext *RpmhContext, IN RpmhDrvTcsData *TcsData,
    IN UINT32 CmdRequestAddress)
{
  INT32 FreeTcsIndex = 0;

  // If cmd is inflight, return busy
  if (RpmhIsCmdInflight(RpmhContext, CmdRequestAddress)) {
    log_warn(
        "%a: Cmd address 0x%X is already inflight!", __FUNCTION__,
        CmdRequestAddress);
    return -CR_BUSY;
  }

  FreeTcsIndex = GetNextFreeTcs(RpmhContext, TcsData);
  if (FreeTcsIndex < 0) {
    log_warn("%a: No free TCS available!", __FUNCTION__);
    return -CR_BUSY;
  }

  // Set busy flag
  RpmhContext->TcsBusy = SET_BITS(RpmhContext->TcsBusy, BIT(FreeTcsIndex));
  // Also set a mark to indicate a interrupt may happen later
  RpmhContext->IrqFromRpmhCr = TRUE;
  log_info("%a: Claimed TCS %d", __FUNCTION__, FreeTcsIndex);
  return FreeTcsIndex;
}

STATIC
VOID TcsWriteBuffer(
    RpmhDeviceContext *RpmhContext, UINT32 TcsIndex, UINT32 CmdId,
    RpmhTcsCmd *TcsCmd, UINT32 NumCmds)
{
  UINT32 CmdIdEnabled = 0;
  UINT32 CmdMsgId =
      RPMH_TCS_CMD_MSG_ID_LEN | RPMH_TCS_CMD_MSG_ID_WRITE_FLAG_BIT;

  // Write each commands
  for (UINT32 cmd_idx = CmdId; cmd_idx < CmdId + NumCmds; cmd_idx++, TcsCmd++) {
    CmdIdEnabled |= BIT(cmd_idx);
    if (TcsCmd->response_required)
      CmdMsgId |= RPMH_TCS_CMD_MSG_ID_RESPONSE_REQUEST_BIT;
    // Write tcs cmd to hardware
    WriteTcsCmdReg(
        RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_msgid, TcsIndex,
        cmd_idx, CmdMsgId);
    WriteTcsCmdReg(
        RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_addr, TcsIndex,
        cmd_idx, TcsCmd->addr);
    WriteTcsCmdReg(
        RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_data, TcsIndex,
        cmd_idx, TcsCmd->data);
  }

  // Write to enabled cmd id
  WriteTcsReg(
      RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_enable, TcsIndex,
      CmdIdEnabled |
          ReadTcsReg(
              RpmhContext, RpmhContext->drv_registers->reg_drv_cmd_enable,
              TcsIndex));
}

// Rpmh write
CR_STATUS
RpmhWrite(RpmhDeviceContext *RpmhContext, RpmhTcsCmd *TcsCmd, UINT32 NumCmds)
{
  INT32 TcsIndex;
  // Claim free TCS
  TcsIndex = ClaimFreeTcs(
      RpmhContext, &RpmhContext->tcs_config.active_tcs, TcsCmd->addr);
  if (TcsIndex < 0) {
    return TcsIndex;
  }

  // Write buffer
  TcsWriteBuffer(
      RpmhContext, TcsIndex, 0, TcsCmd,
      NumCmds); // Currently only support 1 cmd

  // Set trigger
  TcsSetTrigger(RpmhContext, TcsIndex, TRUE);
  return CR_SUCCESS;
}

CR_STATUS
RpmhLibInit(OUT RpmhDeviceContext **RpmhContextOut)
{
  UINT32             DrvId       = 0;
  UINT32             DrvMajorVer = 0;
  UINT32             DrvMinorVer = 0;
  UINT32             DrvConfig   = 0;
  UINT32             MaxTcs      = 0;
  CR_STATUS          Status      = CR_SUCCESS;
  RpmhDeviceContext *RpmhContext = NULL;

  if (RpmhContextOut == NULL) {
    log_err("RpmhLibInit: NULL output parameter");
    return CR_INVALID_PARAMETER;
  }

  // Init driver structure
  RpmhContext = CrTargetGetRpmhContext();
  if (RpmhContext == NULL) {
    log_err("RpmhLibInit: Failed to get Rpmh Context");
    return CR_NOT_FOUND;
  }
  *RpmhContextOut = RpmhContext;

  // Get Rpmh DRV(Direct Resource Voter) version ID
  DrvId       = CrMmioRead32(RpmhContext->drv_base_address);
  DrvMajorVer = GET_FIELD(DrvId, RPMH_DRV_ID_MAJOR_MSK);
  DrvMinorVer = GET_FIELD(DrvId, RPMH_DRV_ID_MINOR_MSK);
  log_info("RPMH DRV Version: %d.%d", DrvMajorVer, DrvMinorVer);

  // Check if we are running on a supported version
  if (DrvMajorVer >= 3) {
    RpmhContext->drv_registers = &drv_registers_v3p0;
  }
  else {
    RpmhContext->drv_registers = &drv_registers_v2p7;
  }

  // probe tcs config
  DrvConfig = CrMmioRead32(
      RpmhContext->drv_base_address +
      RpmhContext->drv_registers->reg_drv_print_child_config);

  MaxTcs =
      ((DrvConfig &
        ((RSC_DRV_TCS_NUM_MSK >> (__ffs(RSC_DRV_TCS_NUM_MSK) - 1))
         << (((__ffs(RSC_DRV_TCS_NUM_MSK) - 1)) * RpmhContext->drv_id))) >>
       ((__ffs(RSC_DRV_TCS_NUM_MSK) - 1) * RpmhContext->drv_id));
  log_info("RPMH DRV Max TCS: %d", MaxTcs);

  RpmhContext->NumCmdsPerTcs = GET_FIELD(DrvConfig, RSC_DRV_NCPT_MSK);
  log_info("RPMH DRV Num Cmds per TCS: %d", RpmhContext->NumCmdsPerTcs);
  CR_ASSERT(
      RpmhContext->tcs_config.active_tcs.tcs_count +
          RpmhContext->tcs_config.sleep_tcs.tcs_count +
          RpmhContext->tcs_config.wake_tcs.tcs_count +
          RpmhContext->tcs_config.control_tcs.tcs_count <=
      MaxTcs);

  // Calculate tcs offsets, masks, etc.
  RpmhContext->tcs_config.active_tcs.offset = 0;
  RpmhContext->tcs_config.active_tcs.mask =
      (BIT(RpmhContext->tcs_config.active_tcs.tcs_count) - 1) << 0;

  RpmhContext->tcs_config.sleep_tcs.offset =
      RpmhContext->tcs_config.active_tcs.tcs_count;
  RpmhContext->tcs_config.sleep_tcs.mask =
      (BIT(RpmhContext->tcs_config.sleep_tcs.tcs_count) - 1)
      << RpmhContext->tcs_config.sleep_tcs.offset;

  RpmhContext->tcs_config.wake_tcs.offset =
      RpmhContext->tcs_config.sleep_tcs.offset +
      RpmhContext->tcs_config.sleep_tcs.tcs_count;
  RpmhContext->tcs_config.wake_tcs.mask =
      (BIT(RpmhContext->tcs_config.wake_tcs.tcs_count) - 1)
      << RpmhContext->tcs_config.wake_tcs.offset;

  // Ignore control tcs
  log_info(
      "RPMH DRV TCS Config: Active TCS - offset: %d, mask: 0x%X",
      RpmhContext->tcs_config.active_tcs.offset,
      RpmhContext->tcs_config.active_tcs.mask);
  log_info(
      "RPMH DRV TCS Config: Sleep  TCS - offset: %d, mask: 0x%X",
      RpmhContext->tcs_config.sleep_tcs.offset,
      RpmhContext->tcs_config.sleep_tcs.mask);
  log_info(
      "RPMH DRV TCS Config: Wake   TCS - offset: %d, mask: 0x%X",
      RpmhContext->tcs_config.wake_tcs.offset,
      RpmhContext->tcs_config.wake_tcs.mask);

  // Enable Interrupt
  Status = CrRegisterInterrupt(
      RpmhContext->interrupt_no,        // Irq Number
      RpmhDrvTcsTxDoneIsr,              // Handler
      RpmhContext,                      // Param
      CR_INTERRUPT_TRIGGER_LEVEL_HIGH); // Level trigger
  if (CR_ERROR(Status)) {
    log_err("Failed to register RPMH DRV interrupt, Status=0x%X", Status);
    return Status;
  }

  // Ignore solver config

  // Calculate Active tcs mask and write it into drv_irq_enable
  CrMmioWrite32(
      RpmhContext->drv_base_address + RpmhContext->tcs_offset +
          RpmhContext->drv_registers->reg_drv_irq_clear,
      RpmhContext->tcs_config.active_tcs.mask);

  return Status;
}

// Retrieve Address from Cmd DB protocol and call this lib function
CR_STATUS
RpmhEnableVreg(
    RpmhDeviceContext *RpmhContext, CONST UINT32 Address, BOOLEAN Enable)
{
  CR_STATUS Status = 0;
  if ((Address == 0) || (RpmhContext == NULL) || Enable > 1) {
    log_err("Invalid parameters");
    Status = CR_INVALID_PARAMETER;
    return Status;
  }

  RpmhTcsCmd VregEnableCmd = {
      .addr              = (Address + RPMH_REGULATOR_ENABLE_REG),
      .data              = Enable,
      .wait              = 0,
      .response_required = 1,
  };

  // Write to RPMH
  Status = RpmhWrite(RpmhContext, &VregEnableCmd, 1);
  if (CR_ERROR(Status)) {
    log_err("Failed to write vreg data to RPMH, Status=0x%X", Status);
    return Status;
  }

  log_info(
      "Vreg at 0x%X %a via RPMH", Address, Enable ? "enabled" : "disabled");
  return CR_SUCCESS;
}

// Set vreg voltage
CR_STATUS
RpmhSetVregVoltage(
    RpmhDeviceContext *RpmhContext, CONST UINT32 Address,
    CONST UINT32 VoltageMv)
{
  CR_STATUS Status = 0;
  if ((Address == 0) || (RpmhContext == NULL)) {
    log_err("Invalid parameters");
    Status = CR_INVALID_PARAMETER;
    return Status;
  }

  // Data is in mV (0.001V)
  RpmhTcsCmd VregVoltageCmd = {
      .addr              = (Address + RPMH_REGULATOR_VOLTAGE_REG),
      .data              = VoltageMv,
      .wait              = 0,
      .response_required = 1,
  };

  // Write to RPMH
  Status = RpmhWrite(RpmhContext, &VregVoltageCmd, 1);
  if (CR_ERROR(Status)) {
    log_err("Failed to write vreg voltage to RPMH, Status=0x%X", Status);
    return Status;
  }
  log_info("Vreg at 0x%X set to %u uV via RPMH", Address, VoltageMv * 1000);
  return CR_SUCCESS;
}

// Set Vreg mode
CR_STATUS
RpmhSetVregMode(
    RpmhDeviceContext *RpmhContext, CONST UINT32 Address, CONST UINT8 Mode)
{
  CR_STATUS Status = 0;
  if ((Address == 0) || (RpmhContext == NULL) || Mode > 8) {
    log_err("Invalid parameters");
    Status = CR_INVALID_PARAMETER;
    return Status;
  }

  RpmhTcsCmd VregModeCmd = {
      .addr              = (Address + RPMH_REGULATOR_MODE_REG),
      .data              = Mode,
      .wait              = 0,
      .response_required = 1,
  };

  // Write to RPMH
  Status = RpmhWrite(RpmhContext, &VregModeCmd, 1);
  if (CR_ERROR(Status)) {
    log_err("Failed to write vreg mode to RPMH, Status=0x%X", Status);
    return Status;
  }

  log_info("Vreg at 0x%X set to mode %u via RPMH", Address, Mode);
  return CR_SUCCESS;
}