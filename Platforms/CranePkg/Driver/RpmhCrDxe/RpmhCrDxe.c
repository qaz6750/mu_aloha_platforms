#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
// #include <Library/UefiDriverEntryPoint.h>
#include <Uefi.h>

#include <oskal/cr_debug.h>

#include <Library/rpmh.h>

#include <Protocol/EFICmdDBCrProtocol.h>
#include <Protocol/EFIRpmhCrProtocol.h>

STATIC RpmhDeviceContext   *mRpmhContext   = NULL;
STATIC EFI_CMD_DB_PROTOCOL *mCmdDbProtocol = NULL;

EFI_STATUS
EFIAPI
ProtocolRpmhWrite(
    IN EFI_RPMH_CR_PROTOCOL *This, IN RpmhTcsCmd *TcsCmd, IN UINT32 NumCmds)
{
  EFI_STATUS Status;
  if (mRpmhContext == NULL) {
    log_err("ProtocolRpmhWrite: Failed to get Rpmh Context");
    return EFI_NOT_FOUND;
  }
  Status = RpmhWrite(mRpmhContext, TcsCmd, NumCmds);
  if (CR_ERROR(Status)) {
    log_err("ProtocolRpmhWrite: RpmhWrite failed with status 0x%X", Status);
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ProtocolRpmhEnableVreg(
    IN EFI_RPMH_CR_PROTOCOL *This, IN CONST UINT32 Address, IN BOOLEAN Enable)
{
  EFI_STATUS Status;
  if (mRpmhContext == NULL) {
    log_err("ProtocolRpmhEnableVreg: Failed to get Rpmh Context");
    return EFI_NOT_FOUND;
  }
  Status = RpmhEnableVreg(mRpmhContext, Address, Enable);
  if (CR_ERROR(Status)) {
    log_err(
        "ProtocolRpmhEnableVreg: RpmhEnableVreg failed with status 0x%X",
        Status);
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ProtocolRpmhSetVregVoltage(
    IN EFI_RPMH_CR_PROTOCOL *This, IN CONST UINT32 Address,
    IN CONST UINT32 VoltageMv)
{
  EFI_STATUS Status;
  if (mRpmhContext == NULL) {
    log_err("ProtocolRpmhSetVregVoltage: Failed to get Rpmh Context");
    return EFI_NOT_FOUND;
  }
  Status = RpmhSetVregVoltage(mRpmhContext, Address, VoltageMv);
  if (CR_ERROR(Status)) {
    log_err(
        "ProtocolRpmhSetVregVoltage: RpmhSetVregVoltage failed with status "
        "0x%X",
        Status);
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ProtocolRpmhSetVregMode(
    IN EFI_RPMH_CR_PROTOCOL *This, IN CONST UINT32 Address, IN CONST UINT8 Mode)
{
  EFI_STATUS Status;
  if (mRpmhContext == NULL) {
    log_err("ProtocolRpmhSetVregMode: Failed to get Rpmh Context");
    return EFI_NOT_FOUND;
  }
  Status = RpmhSetVregMode(mRpmhContext, Address, Mode);
  if (CR_ERROR(Status)) {
    log_err(
        "ProtocolRpmhSetVregMode: RpmhSetVregMode failed with status 0x%X",
        Status);
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

EFI_RPMH_CR_PROTOCOL gRpmhCrProtocol = {
    .Revision           = EFI_RPMH_CR_PROTOCOL_REVISION,
    .RpmhWrite          = ProtocolRpmhWrite,
    .RpmhEnableVreg     = ProtocolRpmhEnableVreg,
    .RpmhSetVregVoltage = ProtocolRpmhSetVregVoltage,
    .RpmhSetVregMode    = ProtocolRpmhSetVregMode,
};

EFI_STATUS
RpmhEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status = EFI_SUCCESS;

  // Locate Cmd DB protocol
  Status = gBS->LocateProtocol(
      &gEfiCmdDBCrProtocolGuid, NULL, (VOID **)&mCmdDbProtocol);
  if (EFI_ERROR(Status)) {
    log_err("Failed to locate Cmd DB Protocol, Status=0x%X", Status);
    return Status;
  }

  if (CR_ERROR(RpmhLibInit(&mRpmhContext)) || (mRpmhContext == NULL)) {
    log_err("RpmhLibInit failed");
    return EFI_DEVICE_ERROR;
  }

  // Install protocol
  Status = gBS->InstallMultipleProtocolInterfaces(
      &ImageHandle, &gEfiRpmhCrProtocolGuid, &gRpmhCrProtocol, NULL);
  if (EFI_ERROR(Status)) {
    log_err("Failed to install RPMH CR Protocol, Status=0x%X", Status);
    return Status;
  }

// test cases (for hdk sm8450)
#if 0
  {
    // Get vreg address from cmd db
    UINT32       Address   = 0;
    CONST CHAR8 *VregName  = "ldoc3"; // for hdk sm8450
    UINT32       VoltageMv = 3300;

    // Locate our protocol
    EFI_RPMH_CR_PROTOCOL *RpmhCrProtocol = NULL;
    Status                               = gBS->LocateProtocol(
        &gEfiRpmhCrProtocolGuid, NULL, (VOID **)&RpmhCrProtocol);
    if (EFI_ERROR(Status)) {
      log_err("Failed to locate Rpmh Cr Protocol, Status=0x%X", Status);
      return Status;
    }

    Status = mCmdDbProtocol->GetEntryAddressByName(
        mCmdDbProtocol, VregName, &Address);
    if (EFI_ERROR(Status)) {
      log_err(
          "GetCmdDBEntryAddressByName failed for %a, Status=0x%X", VregName,
          Status);
      return Status;
    }

    // Print retrieved address
    log_info("Vreg %a address: 0x%X", VregName, Address);

    // Enable vreg
    Status = RpmhCrProtocol->RpmhEnableVreg(RpmhCrProtocol, Address, TRUE);
    if (EFI_ERROR(Status)) {
      log_err("RpmhEnableVreg failed, Status=0x%X", Status);
      return Status;
    }
    log_info("Vreg %a enabled", VregName);
    // Wait 5s for observation
    cr_sleep(5000);
    // Disable vreg
    Status = RpmhCrProtocol->RpmhEnableVreg(RpmhCrProtocol, Address, FALSE);
    if (EFI_ERROR(Status)) {
      log_err("RpmhEnableVreg failed, Status=0x%X", Status);
      return Status;
    }
    log_info("Vreg %a disabled", VregName);
    // Wait 5s for observation
    cr_sleep(5000);

    // Set vreg voltage to 3.3v
    Status = RpmhCrProtocol->RpmhSetVregVoltage(
        RpmhCrProtocol, Address, VoltageMv); // in mV
    if (EFI_ERROR(Status)) {
      log_err("RpmhSetVregVoltage failed, Status=0x%X", Status);
      return Status;
    }
    log_info("Vreg %a voltage set to %u mV", VregName, VoltageMv);
    // Wait 5s for observation
    cr_sleep(5000);
  }
#endif
  return EFI_SUCCESS;
}