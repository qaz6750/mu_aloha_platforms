[PcdsFixedAtBuild.common]
gAndromedaPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1272
gAndromedaPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|2772

# Display Caller
#gAndromedaPkgTokenSpaceGuid.PcdDisplayCallerExitDisableDisplay|FALSE
#gAndromedaPkgTokenSpaceGuid.PcdDisplayCallerStallBeforeEnable|1000

# Smbios Info
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemBrand|"OnePlus"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"PLK110"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"OnePlus 15"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemModel|"15"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosBoardModel|"OP60FFL1"

gAndromedaPkgTokenSpaceGuid.PcdABLProduct|"canoe"

[PcdsDynamicDefault.common]
gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1272
gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|2772
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1272
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|2772
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|159  # = 1272 / EFI_GLYPH_WIDTH(8)
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|145     # = 2772 / EFI_GLYPH_HEIGHT(19)
gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|159       # = 1272 / EFI_GLYPH_WIDTH(8)
gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|145          # = 2772 / EFI_GLYPH_HEIGHT(19)
