#pragma once

#include <Library/clock.h>
#include <oskal/common.h>

ClockDriverContext   *CrTargetGetClockContext(VOID);
DebugccDriverContext *CrTargetGetDebugccContext(VOID);
CR_STATUS             CrTargetClockInit(ClockDriverContext *ClockContext);
