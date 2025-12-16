//
//  Copyright (c) 2011 - 2020, Arm Limited. All rights reserved.<BR>
//
//  SPDX-License-Identifier: BSD-2-Clause-Patent
//
//

#include <AsmMacroLib.h>

.text
.align 3

.set CTRL_SPAN_BIT, (1 << 23)

ASM_FUNC(_ModuleEntryPoint)
  // Do early platform specific actions
  bl    ASM_PFX(ArmPlatformPeiBootAction)

  // Get ID of this CPU in multi-core system
  bl    ASM_PFX(ArmReadMpidr)
  // Keep a copy of the MpId register value
  mov   x10, x0

_SetSVCMode:
  mov x0, #0
  mov x1, #0

  /* First ensure all interrupts are disabled */
  bl ASM_PFX(ArmDisableInterrupts)

  /* Ensure that the MMU and caches are off */
  bl ASM_PFX(ArmDisableCachesAndMmu)

  /* Invalide I-Cache */
  bl ASM_PFX(ArmInvalidateInstructionCache)

  /* Invalidate TLB */
  bl ASM_PFX(ArmInvalidateTlb)

  mrs x0, sctlr_el1
  orr x0, x0, #CTRL_SPAN_BIT
  msr sctlr_el1, x0

_SetupExceptionVector:
  ldr x0, =FixedPcdGet64(PcdCpuVectorBaseAddress)
  bl ArmWriteVBar

  ldr x1, _NeverReturn
  add x2, x0, #0x800
  mov x3, x0

_FillVectors:
  stp     x1, x1, [x3], #16 /* Fill every 16 byte */
  cmp     x3, x2
  b.lt    _FillVectors

_DonNotTrap_VFP_SIMD:
  mrs x0, CPACR_EL1
  /* Bit 20 and 21 */
  orr x0, x0, #0x300000
  msr CPACR_EL1, x0
  isb sy

////////////////////////////////////////////////
// Patch: call lk scheduler related function. //
_CheckCpuId:                                  //
  /* Check if we are on primary core */       //
  // Core 0 MpIDR:                            //
  //   0xXX000000                             //
  and x0, x10, #0xFFFFFF                      //
  cbnz x0, _SetupSecondaryCoreStack           //
////////////////////////////////////////////////

_SetupPrimaryCoreStack:
  ldr x0, =FixedPcdGet32(PcdPrePiStackBase)     /* Stack base arg0 */
  ldr x1, =FixedPcdGet32(PcdPrePiStackSize)     /* Stack size arg1 */

  /* Zero Init stack */
  add x2, x0, x1         /* End of Stack */
  mov x3, x0             /* Stack Base */

  mov v4.d[0], xzr
  mov v4.d[1], xzr
  mov v5.16b, v4.16b
  mov v6.16b, v4.16b
  mov v7.16b, v4.16b

_ClearStack:
  /* Assumes StackBase is 128-bit aligned, StackSize is a multiple of 64B */
  st4   {v4.16b, v5.16b, v6.16b, v7.16b}, [x3], #64  /* Fill every 64 bytes */
  cmp   x3, x2                                   /* Compare Size */
  b.lt  _ClearStack

  add sp, x2, xzr                                /* Initalize SP */

_EnableCache:
  bl ASM_PFX(ArmDisableDataCache)
  bl ASM_PFX(ArmEnableInstructionCache)
  bl ASM_PFX(ArmEnableDataCache)

_PrepareArguments:
  mov   x0, x10
  mov   x1, x11
  mov   x2, x12

  // Move sec startup address into a data register
  // Ensure we're jumping to FV version of the code (not boot remapped alias)
  ldr   x4, =ASM_PFX(CEntryPoint)

  // Set the frame pointer to NULL so any backtraces terminate here
  mov   x29, xzr

  // Jump to PrePiCore C code
  //    x0 = MpId
  //    x1 = UefiMemoryBase
  //    x2 = StacksBase
  blr   x4

//////////////////////////////////////////////////////////////////////////
// Patch: call lk scheduler related function.
  b _NeverReturn

// X0 = MpId
_SetupSecondaryCoreStack:
  // Calculate cpu index from MpId
  // WARN: Only consider aff1 currently
  ubfm x0, x0, #8, #11
  // Load stack base and size
  ldr x1, =FixedPcdGet32(PcdPrePiStackBase)
  ldr x2, =FixedPcdGet32(PcdPrePiStackSize)
  ldr x3, =FixedPcdGet32(PcdCoreCount)
  add x1, x1, x2                  // x1 = stack_top address

  cbnz x3, _SecondaryCalStackSize // Check for zero core count
  mov x3, #8 // Fallback to default core count

_SecondaryCalStackSize:
  // Get stack size per cpu
  udiv x3, x2, x3   // x3 = stack size per cpu
  // Calculate stack pointer address of this cpu
  mul x2, x0, x3    // offset = cpu_index * stack_size_per_cpu
  sub x2, x1, x2    // x2 = secondary_stack_top = stack_top - offset
  sub x3, x2, x3    // x3 = secondary_stack_base = secondary_stack_top - stack_size_per_cpu

  sub     x1, x0, #1
  lsl     x1, x1, #4              // x1 = (x0-1)*16
  ldr     x0, =0xDFC00000
  add     x0, x0, x1
  str     x2, [x0]
  add     x0, x0, #8
  str     x3, [x0]

  add sp, x2, xzr                                /* Initalize SP */

  /* Zero Init stack */
  mov v4.d[0], xzr
  mov v4.d[1], xzr
  mov v5.2d, v4.2d
  mov v6.2d, v4.2d
  mov v7.2d, v4.2d

_SecondaryClearStack:
  /* Assumes StackBase is 128-bit aligned, StackSize is a multiple of 64B */
  st4   {v4.2d, v5.2d, v6.2d, v7.2d}, [x3], #64  /* Fill every 64 bytes */
  cmp   x3, x2                                   /* Compare Size */
  b.lt  _SecondaryClearStack

_SecondaryEnableCache:
  bl ASM_PFX(ArmDisableDataCache)
  bl ASM_PFX(ArmEnableInstructionCache)
  bl ASM_PFX(ArmEnableDataCache)

  // Goto Secondary CEntry
  ldr   x4, =ASM_PFX(SecondaryCoreCEntry)

  // Set the frame pointer to NULL so any backtraces terminate here
  mov   x29, xzr

  // Ensure cache coherence
  dsb   sy
  dmb   ld
  isb

  // Jump to PrePiCore C code
  //    x0 = CpuIndex
  ubfm  x0, x10, #8, #11
  blr   x4
//
//////////////////////////////////////////////////////////////////////////

.align 3
_NeverReturn:
  b _NeverReturn
