# Continuation of [FV.FvMain]
INF CranePkg/Driver/CmdDBCrDxe/CmdDBCrDxe.inf
INF CranePkg/Driver/RpmhCrDxe/RpmhCrDxe.inf
INF CranePkg/Driver/ClockCrDxe/ClockCrDxe.inf

!if $(USE_UART_GENI_FOR_SERIAL_OUTPUT) == 0
  INF CranePkg/Driver/DebugUartSerialCrDxe/DebugUartSerialCrDxe.inf
!endif
