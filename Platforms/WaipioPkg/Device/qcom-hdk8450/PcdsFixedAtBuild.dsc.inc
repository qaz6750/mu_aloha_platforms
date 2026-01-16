[PcdsFixedAtBuild.common]
gAndromedaPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1920
gAndromedaPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|1080

# Display Caller
gAndromedaPkgTokenSpaceGuid.PcdDisplayCallerExitDisableDisplay|FALSE
gAndromedaPkgTokenSpaceGuid.PcdDisplayCallerStallBeforeEnable|800

# Smbios Info
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemBrand|"Qualcomm"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"6"

gAndromedaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Snapdragon (TM) 8 Gen 1 @ 2.995 GHz"
gAndromedaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"SM8450"

gAndromedaPkgTokenSpaceGuid.PcdABLProduct|"waipio"

# Lt9611 Config
gCranePkgTokenSpaceGuid.PcdLt9611GpioInt|44
# Vdd 1p2: Gpio109
# Vdd 3v3: Gpio9
# Reset:   Gpio107
gCranePkgTokenSpaceGuid.PcdLt9611GpioHigh|{0x6d, 0x00, 0x09, 0x00, 0x6b, 0x00}

[PcdsDynamicDefault.common]
gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1920
gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1080
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1920
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1080
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|240     # = 1920 / EFI_GLYPH_WIDTH(8)
gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|56         # = 1080 / EFI_GLYPH_HEIGHT(19)
gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|240          # = 1920 / EFI_GLYPH_WIDTH(8)
gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|56              # = 1080 / EFI_GLYPH_HEIGHT(19)
