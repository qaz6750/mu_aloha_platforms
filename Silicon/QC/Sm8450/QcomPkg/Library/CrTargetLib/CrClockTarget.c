#include <Library/CrTargetClockLib.h>
#include <oskal/common.h>

#define SM8450_CLOCK_CONTROLLER_COUNT CC_TYPE_MAX
#define SM8450_CLOCK_GCC_CLOCK_COUNT 0xFF

enum Sm8450ClockRcg2Sources {
  SM8450_CLOCK_RCG2_SRC_BI_TCXO   = 0,
  SM8450_CLOCK_RCG2_SRC_SLEEP_CLK = 1,
  /* To be filled */
};

enum {
  /* GCC Clocks */
  GCC_UFS_PHY_ICE_CORE_CLK = 0,
  GCC_PCIE0_PIPE_CLK,
  GCC_PCIE0_AUX_CLK,
  /* Add more clocks here */
} Sm8450ClockNodes;

// Forward declaration to allow ParentController reference
STATIC ClockController
    Sm8450ClockControllers[SM8450_CLOCK_CONTROLLER_COUNT + 1];

STATIC ClockNode GccUfsPhyIceCoreClk = {
    .Name             = "gcc_ufs_phy_ice_core_clk",
    .Type             = CLOCK_NODE_TYPE_BRANCH_2,
    .HaltRegister     = 0x8706c,
    .EnableRegister   = 0x8706c,
    .EnableMsk        = BIT(0),
    .ParentController = &Sm8450ClockControllers[CC_TYPE_GCC],
    .ParentCount      = 0,
    .Parents          = NULL,
};

STATIC ClockNode GccPcie0Gdsc = {
    .Name             = "gcc_pcie0_gdsc",
    .GdscRegister     = 0x7b004,
    .PwrStsFlag       = CLOCK_NODE_GDSC_PWR_STS_RET_ON,
    .ParentController = &Sm8450ClockControllers[CC_TYPE_GCC],
};

STATIC ClockNode GccPcie0AuxClkSrc = {
    .Name        = "gcc_pcie_0_aux_clk_src",
    .Type        = CLOCK_NODE_TYPE_ROOT_CLOCK_GENERATOR_2,
    .CmdRegister = 0x7b064,
    .MNDWidth    = 16,
    .HIDWidth    = 5,
    .HwClockCtrl = TRUE,
    .FreqCount   = 1,
    .FreqTable =
        (ClockRcgFreqTable[]){
            /* Source, Source Cfg, Freq, H, M, N */
            CLOCK_NODE_RCG_FREQ_TABLE_ELEMENT(
                SM8450_CLOCK_RCG2_SRC_BI_TCXO, 0, 19200000, 1, 0, 0),
        },
    .ParentController = &Sm8450ClockControllers[CC_TYPE_GCC],
    .ParentCount      = 0,
    .Parents          = NULL,
};

STATIC ClockNode GccPcie0AuxClk = {
    .Name             = "gcc_pcie_0_aux_clk",
    .Type             = CLOCK_NODE_TYPE_BRANCH_2,
    .HaltRegister     = 0x7b034,
    .HaltCheckFlag    = CLOCK_NODE_BRANCH_HALT | CLOCK_NODE_BRANCH_VOTED,
    .EnableRegister   = 0x62008,
    .EnableMsk        = BIT(3),
    .ParentController = &Sm8450ClockControllers[CC_TYPE_GCC],
    .ParentCount      = 1,
    .Parents          = (ClockNode *[]){&GccPcie0AuxClkSrc},
};

STATIC ClockNode GccPcie0PipeClk = {
    .Name             = "gcc_pcie0_pipe_clk",
    .Type             = CLOCK_NODE_TYPE_BRANCH_2,
    .HaltRegister     = 0x7b03c,
    .EnableRegister   = 0x62008,
    .EnableMsk        = BIT(4),
    .ParentController = &Sm8450ClockControllers[CC_TYPE_GCC],
    .ParentCount      = 0,
    .Parents          = NULL,
};

STATIC ClockController Sm8450ClockControllers[] = {
    [CC_TYPE_GCC] =
        {
            .Address  = 0x00100000,
            .Size     = 0x001f4200,
            .ClkCount = SM8450_CLOCK_GCC_CLOCK_COUNT,
            .Clks =
                (ClockNode *[]){
                    [GCC_UFS_PHY_ICE_CORE_CLK] = &GccUfsPhyIceCoreClk,
                    [GCC_PCIE0_PIPE_CLK]       = &GccPcie0PipeClk,
                    [GCC_PCIE0_AUX_CLK]        = &GccPcie0AuxClk,
                    &GccPcie0AuxClkSrc,
                    /* Add more clocks here */
                },

            .GdscCount = 1,
            .Gdscs =
                (ClockNode *[]){
                    &GccPcie0Gdsc,
                },
        },
    [CC_TYPE_VIDEO_CC] = {.Address = 0xaaf0000, .Size = 0x10000},
    [CC_TYPE_CAM_CC]   = {.Address = 0xade0000, .Size = 0x20000},
    [CC_TYPE_GPU_CC]   = {.Address = 0x3d90000, .Size = 0xa000},
    [CC_TYPE_DISP_CC]  = {.Address = 0xaf00000, .Size = 0x20000},
    [CC_TYPE_APSS_CC]  = {.Address = 0x17a80000, .Size = 0x21000},
    [CC_TYPE_MC_CC]    = {.Address = 0x190ba000, .Size = 0x54},
};

STATIC ClockDriverContext Sm8450ClockContext = {
    .ClockControllerCount = SM8450_CLOCK_CONTROLLER_COUNT,
    .ClockControllers     = Sm8450ClockControllers,
};

ClockDriverContext *CrTargetGetClockContext(VOID)
{
  return &Sm8450ClockContext;
}

CR_STATUS CrTargetClockInit(ClockDriverContext *ClockContext)
{
  // 1. UFS related clocks, set FORCE_MEM_CORE_ON bit
  CrMmioUpdateBits32(
      CLOCK_CLOCK_REGISTER(
          ClockContext, CC_TYPE_GCC, GCC_UFS_PHY_ICE_CORE_CLK, HaltRegister),
      BIT(14), BIT(14));

  return CR_SUCCESS;
}