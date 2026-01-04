#pragma once
#include <Library/TimerLib.h>

#define cr_sleep(us)                                                           \
  do {                                                                         \
    MicroSecondDelay(us);                                                      \
  } while (0)
