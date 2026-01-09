#include <Library/CrTargetDebugUartLib.h>
#include <oskal/common.h>

STATIC CrDebugUartContext mCrDebugUartContext = {
    .BaseAddress = 0xa90000,
    .InterruptNo = GIC_SPI(357),
    .IsDebugUart = TRUE,
    .Type        = DEBUG_UART_TYPE_GENI,
    .BaudRate    = 115200,
    .DataBits    = 0,
    .Parity      = 0,
    .StopBits    = 0,
    
};

CrDebugUartContext *CrTargetGetDebugUartContext(VOID)
{
  return &mCrDebugUartContext;
}
