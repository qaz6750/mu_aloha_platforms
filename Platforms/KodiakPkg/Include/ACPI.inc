FILE FREEFORM = 7E374E25-8E01-4FEE-87F2-390C23C606CD {
	SECTION RAW = SurfaceDuoACPI/7325/builtin/MADT.aml
#	SECTION RAW = SurfaceDuoACPI/7325/builtin/BERT.aml
#	SECTION RAW = SurfaceDuoACPI/7325/builtin/BGRT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/CSRT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/DBG2.aml
#	SECTION RAW = SurfaceDuoACPI/7325/builtin/DSDT.aml
    SECTION RAW = KodiakPkg/Device/$(TARGET_DEVICE)/ACPI/DSDT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/FACP.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/FACS.aml
# 	SECTION RAW = SurfaceDuoACPI/7325/builtin/FPDT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/GTDT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/IORT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/MCFG.aml
# 	SECTION RAW = SurfaceDuoACPI/7325/builtin/MSDM.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/PPTT.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/SPCR.aml
	SECTION RAW = SurfaceDuoACPI/7325/builtin/TPM2.aml
# 	SECTION RAW = SurfaceDuoACPI/7325/builtin/XSDT.aml
# 	SECTION RAW = SurfaceDuoACPI/QcomACPI/common/builtin/SSDT.aml
#	SECTION RAW = SurfaceDuoACPI/QcomACPI/common/builtin/TPMDev.dat
#	SECTION RAW = SurfaceDuoACPI/QcomACPI/common/builtin/SoftwareTpm2Table.aml
	SECTION UI = "AcpiTables"
}
