#ifndef CUST_PANTECH_H
#define CUST_PANTECH_H

#define CONFGI_EV10  0
#define CONFIG_PT10  10
#define CONFIG_PT20  12
#define CONFIG_WS10  20
#define CONFIG_WS15  21
#define CONFIG_WS20  22
#define CONFIG_ES10  30
#define CONFIG_ES20  32
#define CONFIG_TP10  40
#define CONFIG_TP20  42
#define CONFIG_TP25  43
#define CONFIG_TP30  44

#include "BOARD_VER.h"

/*******************************************************************************
 * PANTECH LOG SYSTEM - related Debug System (LS1)
 * ****************************************************************************/

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF61K) || defined(T_EF62L)
#define MSM8974_RELEASED_JB
#elif defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L) || defined(T_EF65S)
#define MSM8974_RELEASED_KK
#endif

#if defined(T_EF65S)
#define PANTECH_SKT_SMART_CHARGER
#endif

/******************************************************************************
 * Conditional ON/OFF Feature                                                 *
 * If you use this feature, you must confirm that build completion,           *
 * when feature was enabled or disabeld.                                      *
 * 1. FEATURE_PANTECH_ERR_CRASH_LOGGING                                       *
 * 2. FEATURE_PANTECH_RAWDATA_ACCESS                                          *
 * 3. FEATURE_PANTECH_QMI_SERVER                                              *
 * 4. FEATURE_PANTECH_TESTMENU_SERVER_VER2                                    *
 * ****************************************************************************/

/*******************************************************************************
 * PANTECH LOG SYSTEM - related Debug System (LS1)
 * ****************************************************************************/
#define FEATURE_PANTECH_ERR_CRASH_LOGGING
#include "pantech_sys.h"

/*******************************************************************************
**  SKY_RAW_DATA - related SkyRawData Service (LS1)
*******************************************************************************/
#define FEATURE_PANTECH_RAWDATA_ACCESS

/*******************************************************************************
**  SKY_QMI_SERVICE - related QMI service (LS1)
*******************************************************************************/
#define FEATURE_PANTECH_QMI_SERVER

/*******************************************************************************
**  SKY TEST MENU Socket Service - related pantech_server (LS1)
*******************************************************************************/
#define FEATURE_PANTECH_TESTMENU_SERVER_VER2

/****************************************************
** POWER ON/OFF REASON COUNT
****************************************************/
#define FEATURE_PANTECH_PWR_ONOFF_REASON_CNT
#define FEATURE_PANTECH_DDR_DIVICE_INFO
#define FEATURE_PANTECH_MODEL                       //chjeon20111031@LS1 add CS tool.

/*******************************************************************************
** PVS VALUE
*******************************************************************************/
#define FEATURE_PANTECH_ACPUPVS

/*******************************************************************************
 * SIO(USB&UART&TESTMENU&FACTORY) PART HEADER
 * ****************************************************************************/
#include "CUST_PANTECH_SIO.h"
/*******************************************************************************
**  SENSOR LS2
*******************************************************************************/
#include "CUST_PANTECH_SENSOR.h"  
/*******************************************************************************
**  Display
*******************************************************************************/
#include "CUST_PANTECH_DISPLAY.h"

/*******************************************************************************
**  PMIC
*******************************************************************************/
#include "CUST_PANTECH_PMIC.h"

#define CONFIG_PANTECH /* 20121025 jylee */
/*******************************************************************************
**  USER DATA REBUILDING VERSION
*******************************************************************************/
#define FEATURE_SKY_USER_DATA_VER
#define FEATURE_SKY_FAT16_FOR_SBL3


/* PDL TEMP DEFINE */
#define FEATURE_PANTECH_PDL_DLOADINFO
#define FEATURE_PANTECH_PDL_DLOAD
#define FEATURE_PANTECH_FLASH_ACCESS
#define FEATURE_PANTECH_DLOAD_USB
#define FEATURE_PANTECH_REBOOT_FOR_IDLE_DL
#define FEATURE_PANTECH_GOTA_BACKUP_CODE
#define FEATURE_PANTECH_GPT_RECOVERY     

/* All MSM8974 model support */
#define FEATURE_SKY_MDM_PREVENT_UPGRADE

/*******************************************************************************
** for making and loading boot image
*******************************************************************************/
#define FEATURE_PANTECH_GEN_SKY_ABOOT

/*******************************************************************************
** SOUND
*******************************************************************************/
#include "CUST_PANTECH_SOUND.h"

/*******************************************************************************
**  Camera
*******************************************************************************/
#include "CUST_PANTECH_CAMERA.h"


#if 0 /* 20131028 jylee tmp { */

/*******************************************************************************
**  Display
*******************************************************************************/
#include "CUST_PANTECH_DISPLAY.h"
 

#endif

/*******************************************************************************
** DATA
*******************************************************************************/
#include "cust_pantech_data_linux.h"

/*******************************************************************************
**  EXT4 (repair /data partition)  manual, auto repair
*******************************************************************************/
#define FEATURE_RECOVERY_HIDDEN_MENU
#define FEATURE_CS_USERDATA_BACKUP
#define FEATURE_PANTECH_MANUAL_REPAIR
#define FEATURE_PANTECH_AUTO_REPAIR

/*******************************************************************************
**  MMC(eMMC, MicroSD)
*******************************************************************************/
#define PANTECH_STORAGE_INTERNAL_EMUL
#define FEATURE_PANTECH_SDXC_EXFAT

/*******************************************************************************
**  DEVICE ENCRYPTION ( encrypt /data parition )
*******************************************************************************/
#define FEATURE_SKY_DATA_ENCRYPTION
#define FAST_ENCRYPTION
#if defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L)
#define FEATURE_PANTECH_EFS_UNLINK_PATCH   //p13156@ls1
#endif

/*******************************************************************************
**  Bluetooth
*******************************************************************************/
#define FEATURE_PANTECH_BLUETOOTH_BLUEDROID

#define FEATURE_PANTECH_BLUETOOTH_A2DP_ENABLED	// 20140121 pooyi check A2DP connected and play state.

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L) || defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L)  
#define FEATURE_PANTECH_BLUETOOTH_AVRCP_1_3 /* Use AVRCP 1.3 instead of 1.5  *//*We do not use 'BTRC_FEAT_ABSOLUTE_VOLUME' feature at AVRCP 1.3 *//*remote control Advanced Control command/response*/
#endif

/*******************************************************************************
**  wfd
*******************************************************************************/
#define FEATURE_PANTECH_WFD_ENABLED	// 20140217 wfd enable check.


/*******************************************************************************
**  FACTORY_COMMAND
*******************************************************************************/
#define FEATURE_SKY_MD5_VERIFY	//p14527 add for FACTORY_PRELOAD_CHECK_I
#define PANTECH_DIAG_MSECTOR
#define F_PANTECH_APP_CRC_CHECK
#define FEATURE_PANTECH_FACTORY_COMMAND
#define FEATURE_PANTECH_CS_AUTO_TAKEOVER // p16652 open for reset CNT info.

#if 0 /* 20131028 jylee tmp { */
//#define F_PANTECH_UTS_POWER_UP //p13783 add : for FCMD
#endif

/*******************************************************************************
**  GOTA
*******************************************************************************/
#include "CUST_PANTECH_GOTA.h"



/*******************************************************************************
**  WLAN
*******************************************************************************/
/* WLAN Common Feature */
#define FEATURE_PANTECH_WLAN_PROCESS_CMD
#define FEATURE_PANTECH_WLAN_FTM_TX_OFFSET    // 20131104, Target power control for KCP(FTM) test on regulartory mode
#define FEATURE_PANTECH_WLAN_TESTMENU
#define FEATURE_PANTECH_WLAN_RAWDATA_ACCESS
#define FEATURE_PANTECH_WLAN_FOUR_MAC_ADDRESS // for pantech_wifi_mac_backup.h
#define FEATURE_PANTECH_WLAN_MAC_ADDR_BACKUP // used in skytest_thread.c
#define FEATURE_PANTECH_WLAN_TRP_TIS // 2012-04-09, Pantech only, ymlee_p11019, to config & test TRP TIS
//#define FEATURE_PANTECH_WLAN_DEBUG_LEVEL_FRAMEWORK  // 20121031 thkim_wifi add for wifi logging
//LS3_LeeYoungHo_130402_blk [move to Kbuild file in vendor\qcom\proprietary\wlan\pronto
#if 0
#define FEATURE_PANTECH_WLAN // used in wlan_hdd_main.c
#define FEATURE_PANTECH_WLAN_QCOM_PATCH
#endif//LS3_LeeYoungHo_130402_blk

//#define FEATURE_PANTECH_5G_DPD_ENABLE_AUTO_TEST // 2012-04-02, Pantech only, ymlee_p11019, On Auto test mode 5G DPD enabled // 20121217 thkim_wifi block for wifi 80mhz test
#define FEATURE_PANTECH_WLAN_FOUR_MAC_ADDRESS_WCN3660
#define FEATURE_PANTECH_WLAN_MAC_ADDR_BACKUP_WCN3660


/*******************************************************************************
 **  PANTECH CERTIFICATION FOR Image_verify
*******************************************************************************/
#define FEATURE_PANTECH_KEY_CERTIFICATION


/*******************************************************************************
 **  PANTECH ROOTING CHECK		//lsi@ls1
*******************************************************************************/
#define F_PANTECH_OEM_ROOTING
#define F_PANTECH_ADB_ROOT
/*******************************************************************************
 **  PANTECH SECURE BOOT		//lsi@ls1
*******************************************************************************/
// 130905 LS1-JHM modified : secure boot (F_PANTECH_SECBOOT)
// $$ CAUTION : DO NOT CHANGE BELOW CODES ! THESE ARE CONTROLED BY BUILD SCRIPT
//#define F_PANTECH_SECBOOT
//#define F_PANTECH_BLD_QC_DL_BOOTLOADER
// 130905 LS1-JHM modified - END


/*******************************************************************************
 **  WIDEVINE DRM
*******************************************************************************/
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L) || defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L)  
#define FEATURE_PANTECH_WIDEVINE_DRM
#endif

#if 0

/*************************************************************************/
/****************************  PANTECH UIM ********************************/
/*************************************************************************/
#define F_PANTECH_UIM_TESTMENU

/* Emergency Dload USB */
/* define after merging dload module #define FEATURE_PANTECH_DLOAD_USB*/
/*******************************************************************************
  **  PDL (LK(emergency), bootimage(phoneinfo), KERNEL(idle download))
  *******************************************************************************/
#define FEATURE_PANTECH_PDL_DLOADINFO
#define FEATURE_PANTECH_PDL_DLOAD
#define FEATURE_PANTECH_FLASH_ACCESS
#define FEATURE_PANTECH_DLOAD_USB
#define FEATURE_PANTECH_REBOOT_FOR_IDLE_DL

/* All MSM8974 model support */
#define FEATURE_SKY_MDM_PREVENT_UPGRADE
// #define FEATURE_SKY_MDM_PREVENT_TEST // just testing.FEATURE_SKY_MDM_PREVENT_UPGRADE

//booting error #define FEATURE_PANTECH_GOTA_BACKUP_CODE
#define FEATURE_PANTECH_GPT_RECOVERY     //chjeon20120412@LS1 add

/*******************************************************************************
**  PMIC
*******************************************************************************/
#include "CUST_PANTECH_PMIC.h"

#endif

/*******************************************************************************
**  P12554 Codec
*******************************************************************************/
#include "CUST_PANTECH_MMP.h"

/******************************************************************************
**  Media Framework
******************************************************************************/
#include "CUST_PANTECH_MF.h"

/*******************************************************************************
** Android Patternlock Reset
*******************************************************************************/
#define FEATURE_PANTECH_PATTERN_UNLOCK

/* 20140204 imei erase factory command */
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L) || defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L)  
#define FEATURE_FACTORY_IMEI_ERASE
#endif

/*******************************************************************************
** Certus Service
*******************************************************************************/
#define FEATURE_PANTECH_CERTUS



#if 0 /* 20131028 jylee tmp { */
/*******************************************************************************
** for making and loading boot image
*******************************************************************************/
#define FEATURE_PANTECH_GEN_SKY_ABOOT
/*******************************************************************************
** for Touch Gold Reference
*******************************************************************************/
#if defined(T_EF56S) || defined(T_EF57K) || defined(T_EF58L) || defined(T_EF59S) ||  defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L)  
#define FEATURE_PANTECH_TOUCH_GOLDREFERENCE
#endif

/*******************************************************************************
** UICC NO Power Down when going to LPM 
*******************************************************************************/
#if defined(T_NAMI)
#define FEATURE_PANTECH_MMGSDI_CARD_NOT_POWER_DOWN
#endif

/*******************************************************************************
** for DRV2603 IN EF63 Series
*******************************************************************************/
#if defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L)
#define FEATURE_VIBRATOR_DRV2603
#endif

/*******************************************************************************
** for checking formatting flag
*******************************************************************************/
#define FEATURE_CHECK_FORMATTING_FLAG

/******************************************************************************
**  ECO CPU Mode I/F
******************************************************************************/
#if defined(T_EF56S)
#define FEATURE_PANTECH_ECO_CPU_MODE
#endif

#endif /* 20131028 jylee tmp } */

/******************************************************************************
** [Thermal Mitigation]  CPU Min Freq.limit for Thermal Mitigation
******************************************************************************/
#define FEATURE_PANTECH_CPU_MIN_FREQ_LIMIT

/******************************************************************************
** [Thermal Mitigation]  For using  External Thermistor.  3C3
******************************************************************************/
#define FEATURE_PANTECH_EXTERNAL_THERMISTOR

/******************************************************************************
**  US4/LS3 camcorder requirement
******************************************************************************/
/* 2014/04/08, workaround error handling for timestamp error on switching main<->sub during recording dualcam. cf) CASE#01452901 */
#define FEATURE_PANTECH_CAMCORDER_DUALCAM_SWITCH_WORKAROUND

/* 2013/12/23, workaround for app crash during camcoring GoofyFace */
#define FEATURE_PANTECH_CAMCORDER_SURFACE_MEDIA_WORKAROUND

/* 2013/12/17, workaround implementation of Android camcorder(media recorder) pause(/resume) function */
#define FEATURE_PANTECH_CAMCORDER_PAUSE_RESUME_WORKAROUND

/******************************************************************************
** Preload:Sample Content
******************************************************************************/
//p13156@lks 
#define FEATURE_PANTECH_PRELOAD_SAMPLE_CONTENT
/*******************************************************************************
**  WIFI
*******************************************************************************/
#include "CUST_PANTECH_WIFI.h"

/******************************************************************************
**  SELinux
******************************************************************************/
#if defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L) || defined(T_EF65S) || defined(T_EF65K) || defined(T_EF65L)
#define FEATURE_PANTECH_SELINUX_DENIAL_LOG //P11536-SHPARK-SELinux 
#endif

/******************************************************************************
**  TBD(to be defined)
******************************************************************************/

/******************************************************************************
**  Power On/Off Test (auto booting on offline charging mode)
******************************************************************************/
//#define FEATURE_PANTECH_POWER_ONOFF

/*******************************************************************************
**  Call PROTOCOL
*******************************************************************************/
/* 2013-04-10 octopusy added  [PS1/2 Team Feature] */
#ifdef T_SKY_MODEL_TARGET_COMMON
#include "cust_lgu_cp_linux.h"
#endif/* T_SKY_MODEL_TARGET_COMMON */

//+US1-CF1
//Feature : FW_VENDOR_FOR_AOT_VIDEO_APP
//API support for AOT 
#define FEATURE_PANTECH_MMP_SUPPORT_AOT
//-US1-CF1

/******************************************************************************
** FINGERPRINT BUMPER
** P10646 Leejungwoon
******************************************************************************/
#define FEATURE_PANTECH_FINGERPRINT_BUMPER_CASE

// p13040 ++ [DS4] for DRM
#include "CUST_PANTECH_DRM.h"
// p13040 -- [DS4] for DRM

/*******************************************************************************
**  IMS PROTOCOL - UI VOB Only
*******************************************************************************/
#include "CUST_PANTECH_IMS.h"

#endif/* CUST_PANTECH_H */

