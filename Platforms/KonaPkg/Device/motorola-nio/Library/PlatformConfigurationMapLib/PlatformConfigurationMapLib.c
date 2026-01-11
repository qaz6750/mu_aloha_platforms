#include <Library/BaseLib.h>
#include <Library/PlatformConfigurationMapLib.h>

static CONFIGURATION_DESCRIPTOR_EX gDeviceConfigurationDescriptorEx[] = {
    // Configuration Map
    {"NumCpusFuseAddr", 0x5C04C},
    {"EnableShell", 0x1},
    {"EUDEnableAddr", 0x88E2000},
    {"SecPagePoolCount", 0x700},
    {"SharedIMEMBaseAddr", 0x146BF000},
    {"DloadCookieAddr", 0x01FD3000},
    {"DloadCookieValue", 0x10},
    {"MemoryCaptureModeOffset", 0x1C},
    {"NumCpus", 8},
    {"NumActiveCores", 8},
    {"MaxLogFileSize", 0x400000},
    {"UefiMemUseThreshold", 0x77},
    {"USBHS1_Config", 0x0},
    {"UsbFnIoRevNum", 0x00010001},
    {"PwrBtnShutdownFlag", 0x0},
    {"Sdc1GpioConfigOn", 0x1E92},
    {"Sdc2GpioConfigOn", 0x1E92},
    {"Sdc1GpioConfigOff", 0xA00},
    {"Sdc2GpioConfigOff", 0xA00},
    {"EnableSDHCSwitch", 0x1},
    {"EnableUfsIOC", 0},
    {"UfsSmmuConfigForOtherBootDev", 1},
    {"SecurityFlag", 0x81C4},
    {"EnableLogFsSyncInRetail", 0x0},
    {"ShmBridgememSize", 0xA00000},
    {"EnableMultiThreading", 0},
    {"MaxCoreCnt", 8},
    {"EarlyInitCoreCnt", 1},
    {"EnableUefiSecAppDebugLogDump", 0x0},
    {"AllowNonPersistentVarsInRetail", 0x1},
    {"EnableDisplayThread", 0x1},
    {"EnableDisplayImageFv", 0x0},
    /* Terminator */
    {"Terminator", 0xFFFFFFFF}
};

CONFIGURATION_DESCRIPTOR_EX *GetPlatformConfigurationMap()
{
  return gDeviceConfigurationDescriptorEx;
}
