//=============================================================================
// File       : Tdmb_modeldef.h
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2010/12/06       yschoi         Create
//                                              (tdmb_dev.h, tdmb_comdef.h ���� �и�)
//=============================================================================

#ifndef _ISDBT_MODELDEF_INCLUDES_H_
#define _ISDBT_MODELDEF_INCLUDES_H_

#ifdef CONFIG_ARCH_TEGRA
#include <../gpio-names.h>
#endif

/*================================================================== */
/*================     MODEL FEATURE               ================= */
/*================================================================== */

#if 0
/////////////////////////////////////////////////////////////////////////
// DMB GPIO (depend on model H/W)
// �ߺ��Ǿ� define �� ������ �����Ƿ� �����ϴ� ������ �Ǿ����� �����´�.
/////////////////////////////////////////////////////////////////////////

#elif defined (CONFIG_MACH_MSM8974_NAMI)
    #define FEATURE_DMB_USE_DTS //CONFIG_ARCH_MSM8974
#if (BOARD_VER >= WS10)
    #define FEATURE_DMB_PMIC_GPIO       900
    #define DMB_PMIC_GPIO_NUM  1
    #define DMB_RESET                   (18 + FEATURE_DMB_PMIC_GPIO)
    #define DMB_INT                     74
#else
    #define DMB_RESET                   58
    #define DMB_INT                     92
#endif /*(BOARD_VER >= WS10)*/
    #define DMB_1SEG_EN                 91
    #define DMB_PWR_EN                  DMB_1SEG_EN
    #define DMB_LNA_EN                  127 /* 1SEG_LNA_EN */
    #define DMB_ANT_SEL                 118 /* 1SEG_CTL1 */
    #define DMB_ANT_EAR_ACT             1
    #define FEATURE_DMB_CLK_26000
    #define FEATURE_DMB_SET_ANT_PATH
    #define FEATURE_DMB_GPIO_INIT
    #define FEATURE_ISDBT_THREAD

#elif defined(CONFIG_MACH_MSM8960_SIRIUSLTE)
  #define DMB_RESET      38
  #define DMB_INT        39
  #define DMB_1SEG_EN    43
  #define DMB_PWR_EN     DMB_1SEG_EN
  #define DMB_LNA_EN     6
  #define DMB_LNA        63
  #define FEATURE_DMB_GPIO_INIT
#if (BOARD_VER >= PT20)
  #define FEATURE_DMB_PMIC_POWER
  #define FEATURE_DMB_PMIC8921
  #define FEATURE_DMB_LNA_CTRL
#endif
  #define FEATURE_DMB_SHARP_I2C_READ
  //#define FEATURE_DMB_USE_TASKLET
  #define FEATURE_ISDBT_THREAD


#elif defined(CONFIG_MACH_MSM8960_VEGAPKDDI)
  #define DMB_RESET      38
  #define DMB_INT        39
  #define DMB_1SEG_EN    43
  #define DMB_PWR_EN     DMB_1SEG_EN
  #define DMB_I2C_SCL    41
  #define DMB_I2C_SDA    40
  #define DMB_ANT_SEL   26
  #define DMB_ANT_EAR_ACT   1 
  #define FEATURE_DMB_GPIO_INIT
  //#define FEATURE_DMB_PMIC_POWER

/////////////////////////////////////////////////////////////////////////
#elif defined(CONFIG_MACH_MSM8960_RACERJ)
  #define DMB_RESET      38
  #define DMB_INT        39
  #define DMB_1SEG_EN    43
  #define DMB_PWR_EN     DMB_1SEG_EN
  #define DMB_I2C_SCL    41
  #define DMB_I2C_SDA    40
  #define DMB_ANT_SEL   26
  #define DMB_ANT_EAR_ACT   1 
  #define FEATURE_DMB_GPIO_INIT
  //#define FEATURE_DMB_PMIC_POWER
  //#define FEATURE_DMB_THREAD

#else
  #error
#endif


/*================================================================== */
/*================     TEST FEATURE                ================= */
/*================================================================== */

//#define FEATURE_TS_PKT_MSG // Single CH : ���� ��Ŷ�� ������, Mulch CH : ���� �������� ù ��Ŷ�� ������.
//#define FEATURE_TEST_ON_BOOT
//#define FEATURE_TEST_READ_DATA_ON_BOOT
//#define FEATURE_NETBER_TEST_ON_BOOT
//#define FEATURE_NETBER_TEST_ON_AIR
//#define FEATURE_DMB_DUMP_FILE
//#define FEATURE_APP_CALL_TEST_FUNC
//#define FEAUTRE_USE_FIXED_FIC_DATA
//#define FEATURE_DMB_GPIO_DEBUG => dmb_hw.c �� �̵� (���� ����)
//#define FEATURE_COMMAND_TEST_ON_BOOT
//#define FEATURE_DMB_I2C_WRITE_CHECK => dmb_i2c.c �� �̵� (���� ����)
//#define FEATURE_DMB_I2C_DBG_MSG => dmb_i2c.c �� �̵� (���� ����)
//#define FEATURE_EBI_WRITE_CHECK
//#define FEATURE_HW_INPUT_MATCHING_TEST

#ifndef CONFIG_SKY_DMB_TSIF_IF
#if (defined(FEATURE_TEST_ON_BOOT) && !defined(FEATURE_DMB_THREAD)) // �����׽�Ʈ�� �������� ���� ������ ���� ����
#define FEATURE_DMB_THREAD
#endif
#endif


#endif /* _ISDBT_MODELDEF_INCLUDES_H_ */
