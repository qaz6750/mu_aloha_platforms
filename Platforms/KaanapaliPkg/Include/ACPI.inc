FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
    SECTION RAW = SurfaceDuoACPI/8850/builtin/APIC.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/CSRT.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/DBG2.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/FACS.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/FACP.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/GTDT.aml
#    SECTION RAW = SurfaceDuoACPI/8850/builtin/IORT.aml #TODO
    SECTION RAW = SurfaceDuoACPI/8850/builtin/MCFG.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/PPTT.aml
    SECTION RAW = SurfaceDuoACPI/8850/builtin/SPCR.aml
    SECTION RAW = KaanapaliPkg/Device/$(TARGET_DEVICE)/ACPI/DSDT.aml
  #  SECTION RAW = SurfaceDuoACPI/QcomACPI/common/builtin/SSDT.aml
  #  SECTION RAW = SurfaceDuoACPI/QcomACPI/common/builtin/TPMDev.dat
  #  SECTION RAW = SurfaceDuoACPI/QcomACPI/common/builtin/SoftwareTpm2Table.aml
	SECTION UI = "AcpiTables"
}
