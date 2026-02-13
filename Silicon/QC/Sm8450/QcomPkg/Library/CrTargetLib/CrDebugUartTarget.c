#include <Library/CrTargetDebugUartLib.h>
#include <oskal/common.h>

STATIC CrDebugUartContext mCrDebugUartContext = {
    .BaseAddress = 0x99c000,
    .InterruptConfig =
        {
            .InterruptNumber = GIC_SPI(608),
            .TriggerType     = CR_INTERRUPT_TRIGGER_LEVEL_HIGH,
        },
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
