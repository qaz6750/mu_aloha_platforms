[LibraryClasses.common]
  # OS/Kernel Abstract Layer Libs
  OskInterruptLib|CranePkg/Library/OskInterruptLib/oskinterrupt.inf

  # Platform specific data
  CrTargetLib|QcomPkg/Library/CrTargetLib/CrTargetLib.inf

  # Generic libraries that interact with oskal
  CmdDBLib|CranePkg/Library/CmdDBLib/cmddb.inf
  RpmhLib|CranePkg/Library/RpmhLib/rpmh.inf

[Components.common]
  CranePkg/Driver/RpmhCrDxe/RpmhCrDxe.inf
  CranePkg/Driver/CmdDBCrDxe/CmdDBCrDxe.inf