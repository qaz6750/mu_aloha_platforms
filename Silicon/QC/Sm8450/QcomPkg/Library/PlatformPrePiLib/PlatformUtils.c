#include <PiPei.h>

#include <Library/IoLib.h>
#include <Library/PlatformPrePiLib.h>
#include <Library/QcomMmuDetachHelperLib.h>

VOID PlatformInitialize(VOID) {
  MmuDetach();
}
