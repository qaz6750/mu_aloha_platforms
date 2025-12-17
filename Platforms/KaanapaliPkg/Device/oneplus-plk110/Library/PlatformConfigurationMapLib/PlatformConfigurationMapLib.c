#include <Library/BaseLib.h>
#include <Library/PlatformConfigurationMapLib.h>

static CONFIGURATION_DESCRIPTOR_EX gDeviceConfigurationDescriptorEx[] = {
    {"AllowNonPersistentVarsInRetail", 0x1},
    {"DDRInfoNotifyFlag", 0x0},
    {"DRAM_CLK_PERIOD_ADDR", 0x240BA050},
    {"DetectRetailUserAttentionHotkey", 0x00},
    {"DetectRetailUserAttentionHotkeyCode", 0x17},
    {"DloadCookieAddr", 0x01FD9000},
    {"DloadCookieValue", 0x10},
    {"EarlyInitCoreCnt", 2},
    {"EnableACPIFallback", 0x0},
    {"EnableDisplayImageFv", 0x0},
    {"EnableLogFsSyncInRetail", 0x0},
    {"EnableDisplayThread", 0},
    {"EnableMultiThreading", 0},
    {"EnableMultiCoreFvDecompression", 1},
    {"EnableSDHCSwitch", 0x1},
    {"EnableShell", 0x1},
    {"EnableUfsIOC", 1},
    {"EnableUefiSecAppDebugLogDump", 0x0},
    {"EnableVariablePolicyEngine", 0},
    {"IsFunctionalCoreCountToOverrideFlag", 0},
    {"MaxCount", 0x29},
    {"MaxCoreCount", 8},
    {"MaxLogFileSize", 0x400000},
    {"NumActiveCores", 8},
    {"NumCpus", 8},
    {"PilSubsysDbgCookieAddr", 0x146806DC},
    {"PilSubsysDbgCookieVal", 0x53444247},
    {"PwrBtnShutdownFlag", 0x0},
    {"Sdc1GpioConfigOff", 0xA00},
    {"Sdc1GpioConfigOn", 0x1E92},
    {"Sdc2GpioConfigOff", 0xA00},
    {"Sdc2GpioConfigOn", 0x1E92},
    {"SecurityFlag", 0xC4},
    {"SecPagePoolCount", 0x1800},
    {"SharedIMEMBaseAddr", 0x14680000},
    {"ShmBridgememSize", 0xA00000},
    {"StrMaxCount", 0x0A},
    {"SkipDBISetup", 0x1},
    {"UfsSmmuConfigForOtherBootDev", 1},
    {"UsbFnIoRevNum", 0x00010001},
    {"UefiMemUseThreshold", 0xE1},
    {"USBHS1_Config", 0x0},
    /* Terminator */
    {"Terminator", 0xFFFFFFFF}};

CONFIGURATION_DESCRIPTOR_EX *GetPlatformConfigurationMap()
{
  return gDeviceConfigurationDescriptorEx;
}
