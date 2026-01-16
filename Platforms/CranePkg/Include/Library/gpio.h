/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */
#pragma once

#include <oskal/common.h>
#include <oskal/cr_status.h>
#include <oskal/cr_types.h>
#include <oskal/cr_interrupt.h>

#define GPIO_PINS_MAX 300
#define GPIO_FUNCS_NUM_MAX 10
#define GPIO_PDC_PIN_NUMBER_INVALID 0xFFFF

enum GpioTlmmTile {
  GPIO_TLMM_TILE_DEFAULT,
  GPIO_TLMM_TILE_EAST =
      GPIO_TLMM_TILE_DEFAULT, // Some platforms only have one tile
  GPIO_TLMM_TILE_WEST,
  GPIO_TLMM_TILE_SOUTH,
  GPIO_TLMM_TILE_NORTH,
  GPIO_TLMM_TILE_MAX = GPIO_TLMM_TILE_NORTH,
};

typedef enum {
  GPIO_CFG_REG_TYPE_CTL_REG,
  GPIO_CFG_REG_TYPE_IO_REG,
  GPIO_CFG_REG_TYPE_INT_CFG_REG,
  GPIO_CFG_REG_TYPE_INT_STATUS_REG,
  GPIO_CFG_REG_TYPE_INT_TARGET_REG,
  GPIO_CFG_REG_TYPE_WAKE_REG,
  GPIO_CFG_REG_TYPE_MAX
} GpioConfigRegType;

/* Gpio config params */
typedef enum {
  GPIO_PULL_NONE         = 0,
  GPIO_PULL_DOWN         = 1,
  GPIO_PULL_KEEPER       = 2,
  GPIO_PULL_UP_NO_KEEPER = 2,
  GPIO_PULL_UP           = 3,
  GPIO_PULL_MAX          = GPIO_PULL_UP,
  GPIO_PULL_UNCHANGE     = 0xFF,
} GpioPullType;

typedef enum {
  GPIO_FUNC_NORMAL = 0,
  GPIO_FUNC_1,
  GPIO_FUNC_2,
  GPIO_FUNC_3,
  GPIO_FUNC_4,
  GPIO_FUNC_5,
  GPIO_FUNC_6,
  GPIO_FUNC_7,
  GPIO_FUNC_8,
  GPIO_FUNC_9,
  GPIO_FUNC_10,
  GPIO_FUNC_11,
  GPIO_FUNC_12,
  GPIO_FUNC_13,
  GPIO_FUNC_14,
  GPIO_FUNC_15,
  GPIO_FUNC_MAX      = GPIO_FUNC_15,
  GPIO_FUNC_UNCHANGE = 0xFF,
} GpioFunctionType;

// way to calc: ma / 2 - 1
typedef enum {
  GPIO_DRIVE_STRENGTH_2MA      = 0,
  GPIO_DRIVE_STRENGTH_4MA      = 1,
  GPIO_DRIVE_STRENGTH_6MA      = 2,
  GPIO_DRIVE_STRENGTH_8MA      = 3,
  GPIO_DRIVE_STRENGTH_10MA     = 4,
  GPIO_DRIVE_STRENGTH_12MA     = 5,
  GPIO_DRIVE_STRENGTH_14MA     = 6,
  GPIO_DRIVE_STRENGTH_16MA     = 7,
  GPIO_DRIVE_STRENGTH_MAX      = GPIO_DRIVE_STRENGTH_16MA,
  GPIO_DRIVE_STRENGTH_UNCHANGE = 0xFF,
} GpioDriveStrength;

typedef enum {
  GPIO_EGPIO_OWNER_REMOTE_OWNER = 0,
  GPIO_EGPIO_OWNER_APPS_OWNER   = 1,
  GPIO_EGPIO_OWNER_MAX          = GPIO_EGPIO_OWNER_APPS_OWNER,
  GPIO_EGPIO_OWNER_UNCHANGE     = 0xFF,
} GpioEGpioOwnerType;

// IO Reg
typedef enum {
  GPIO_VALUE_LOW      = 0,
  GPIO_VALUE_HIGH     = 1,
  GPIO_VALUE_MAX      = GPIO_VALUE_HIGH,
  GPIO_VALUE_UNCHANGE = 0xFF,
} GpioValueType;

typedef struct {
  UINT16            PinNumber;
  BOOLEAN           OutputEnable; // 0 = Input, 1 = Output
  GpioFunctionType  FunctionSel;
  GpioPullType      Pull;
  GpioDriveStrength DriveStrength; // in mA
  GpioValueType     OutputValue;   // Valid if OutputEnable is 1
} GpioConfigParams;

// Gpio Pin Info
typedef struct {
  // Gpio Pin Name
  // CHAR8 *PinName;
  // Gpio Pin Number
  UINT16 PinNumber;
  UINT16 FunctionMux[GPIO_FUNCS_NUM_MAX];
  UINT8  WakeBit;
  UINT32 WakeRegOffset;
  // Tile
  UINT8 Tile;
  // Pdc Pin Number
  UINT16 PdcPinNumber;
} GpioPinInfo;

// Context
typedef struct {
  UINT64  TlmmBaseAddress;
  UINT64  TileOffset[GPIO_TLMM_TILE_MAX];
  UINT16  PinCount;
  BOOLEAN PullUpNoKeeper;
  UINT32  InterruptNo;
  /* Gpio pins */
  GpioPinInfo *GpioPins;
  /* AUX Data */
  UINT8 InterruptDetectionWidth;
} GpioDeviceContext;

GpioValueType
GpioReadIoVal(IN GpioDeviceContext *GpioContext, IN UINT16 GpioIndex);

VOID GpioSetConfig(
    IN GpioDeviceContext *GpioContext, IN GpioConfigParams *ConfigParams);

VOID GpioReadConfig(
    IN GpioDeviceContext *GpioContext, IN OUT GpioConfigParams *ConfigParams);

VOID GpioInitConfigParams(
    OUT GpioConfigParams *ConfigParams, IN UINT16 PinNumber);

CR_STATUS GpioRegisterGpioIrq(
    IN GpioDeviceContext *Context, IN UINT16 GpioIndex,
    IN CR_INTERRUPT_HANDLER InterruptHandler, IN VOID *Param,
    IN CR_INTERRUPT_TRIGGER_TYPE TriggerType);

CR_STATUS GpioLibInit(OUT GpioDeviceContext **GpioContext);