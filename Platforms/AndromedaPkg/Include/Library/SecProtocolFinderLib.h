#ifndef _SEC_PROTOCOL_FINDER_LIB_H_
#define _SEC_PROTOCOL_FINDER_LIB_H_

#include <Library/BaseLib.h>

VOID InitProtocolFinder(
    EFI_PHYSICAL_ADDRESS *ScheAddr,
    EFI_PHYSICAL_ADDRESS *XBLDTOpsAddr
);

VOID StartUpScheduler(
  VOID *PeiContinueBoot,
  VOID *AuxCoresEntryPoint
);

VOID BootSecondaryCpu(
  UINTN CpuIdx
);

#endif // _SEC_PROTOCOL_FINDER_LIB_H_
