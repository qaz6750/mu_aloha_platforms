[LibraryClasses.common]
  # OS/Kernel Abstract Layer Libs
  OskInterruptLib|CranePkg/Library/OskInterruptLib/oskinterrupt.inf
  OskMemmapLib|CranePkg/Library/OskMemmapLib/oskmemmap.inf

  # Platform specific data
  CrTargetLib|QcomPkg/Library/CrTargetLib/CrTargetLib.inf

  # Generic libraries that interact with oskal
  CmdDBLib|CranePkg/Library/CmdDBLib/cmddb.inf
  RpmhLib|CranePkg/Library/RpmhLib/rpmh.inf
  ClockLib|CranePkg/Library/ClockLib/clock.inf

[Components.common]
  CranePkg/Driver/CmdDBCrDxe/CmdDBCrDxe.inf
  CranePkg/Driver/RpmhCrDxe/RpmhCrDxe.inf
  CranePkg/Driver/ClockCrDxe/ClockCrDxe.inf
