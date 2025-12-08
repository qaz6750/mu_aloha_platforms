[PcdsFixedAtBuild.common]
gAndromedaPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1440
gAndromedaPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|2560

# Smbios Info
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemBrand|"Qualcomm"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"2.01"

gAndromedaPkgTokenSpaceGuid.PcdABLProduct|"napali"

# Display Caller
# gAndromedaPkgTokenSpaceGuid.PcdDisplayCallerExitDisableDisplay|FALSE
# gAndromedaPkgTokenSpaceGuid.PcdDisplayCallerStallBeforeEnable|2000000

[PcdsDynamicDefault.common]
gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1440
gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|2560
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1440
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|2560
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|180 # = 1440 / EFI_GLYPH_WIDTH(8)
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|134    # = 2560 / EFI_GLYPH_HEIGHT(19)
gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|180      # = 1440 / EFI_GLYPH_WIDTH(8)
gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|134         # = 2560 / EFI_GLYPH_HEIGHT(19)
