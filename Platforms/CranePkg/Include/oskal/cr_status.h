/** @file
 *   Copyright (c) 2025-2026. Project Aloha Authors. All rights reserved.
 *   Copyright (c) 2025-2026. Kancy Joe. All rights reserved.
 *   SPDX-License-Identifier: MIT
 */

#pragma once
#include <Uefi.h>
#include <Library/BaseLib.h>

typedef EFI_STATUS CR_STATUS;
#define CR_SUCCESS EFI_SUCCESS
#define CR_ABORT EFI_ABORTED
#define CR_INVALID_PARAMETER EFI_INVALID_PARAMETER
#define CR_NOT_FOUND EFI_NOT_FOUND
#define CR_OUT_OF_RESOURCES EFI_OUT_OF_RESOURCES
#define CR_UNSUPPORTED EFI_UNSUPPORTED
#define CR_DEVICE_ERROR EFI_DEVICE_ERROR
#define CR_TIMEOUT EFI_TIMEOUT
#define CR_BUSY EFI_ACCESS_DENIED
#define CR_ERROR(x) EFI_ERROR(x)