#ifndef __CUST_LGU_CP_H__
#define __CUST_LGU_CP_H__


/*****************************************************
    SKY Android ¸ðµ¨ °øÅë Àû¿ë»çÇ×
    Feature Name : T_SKY_MODEL_TARGET_COMMON
******************************************************/
#ifdef T_SKY_MODEL_TARGET_COMMON


// kkosu, new phantom daemon and interfaces
//#define FEATURE_SKY_CP_PHANTOM
#undef FEATURE_SKY_CP_PHANTOM



/* 
   SkyTelephonyInterfaceManager.java, Service_Manager.c, ISkyTelephony.aidl 
*/
#define FEATURE_LGU_CP_TELEPHONYMANAGER
#ifdef FEATURE_LGU_CP_TELEPHONYMANAGER
/* Java¿¡¼­ Modem API È£ÃâÀ» À§ÇÑ Daemon ( NV access, ÀÚµ¿ÀÀ´ä, VE ±â´É, etc.. ) */
#define FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE
#ifdef FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE
/* Voice Call connection sound event */

#define FEATURE_SKY_CP_CS_CALL_CONNECTION_SND_START

/* SKY ÀÚµ¿ÀÀ´ä±â´É Interface */
#define FEATURE_SKY_CP_AUTOANSWER_INTERFACE

#endif/* FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE */
#endif/* FEATURE_LGU_CP_TELEPHONYMANAGER */


/* QMI ¸¦ ÅëÇØ¼­ CM system selection preference È£Ãâ */
#define FEATURE_SKY_CP_CM_SYS_SELECTION_PREF

/* UMTS SMS MO PS/CS Domain setting menu support */
#define FEATURE_LGU_CP_SMS_CFG_SET_GW_DOMAIN

/* Phone Operation Mode setting (lpm, offline, online .. ) */
#define FEATURE_LGU_CP_PHONE_OPERATION_MODE

/*
    factory command Airplane mode
*/
#define FEATURE_LGU_CP_FACTORY_AIRPLANE_MODE_I

/* QMI ¸¦ ÅëÇØ¼­ CM PH event info. Ã³¸® */
#define FEATURE_LGU_CP_OEM_PH_EVENT_WITH_QMI

/*
   Android Factory reset, NV should be initialized default value.
*/
#define FEATURE_LGU_CP_FACTORY_RESET_NV_INIT


#define FEATURE_LGU_CP_OEM_QMI_SERVICE

#define FEATURE_LGU_CP_COMMON_GLOBAL_SD

#define FEATURE_LGU_CP_SSR_MODEM_CRASH_MONITOR

/*
** 20140305 - LS HiddenCode¸¦ BlockÇÏ±â À§ÇØ Opening Day¸¦ Sky_RawData¿¡ WriteÇÏ´Â ½ÃÁ¡¿¡
** LS¿¡¼­ Á¦°øÇÑ API¸¦ È£ÃâÇÏ¿© Hidden Code Flag¸¦ Disable ÇÔ.
*/
#define FEATURE_SKY_CP_HIDDEN_CODE_DISABLE_FLAG

#endif/* T_SKY_MODEL_TARGET_COMMON */


/* ######################################################################### */

/*************************************************/
/*                     COMMON                    */
/*************************************************/

#define FEATURE_LGU_CP_COMMON_HANDSET_PROPERTIES
#define FEATURE_LGU_CP_COMMON_RSSI
#define FEATURE_LGU_CP_COMMON_CHECK_ROAMING_COUNTRY_CONDITION

#define FEATURE_LGU_CP_COMMON_FACTORY_INIT

/*************************************************/
/*                    LTE                        */
/*************************************************/

#define FEATURE_LGU_CP_LTE_ANDSF_CELLID
#define FEATURE_LGU_CP_LTE_MO_DATA_BARRING_NOTIFICATION
#define FEATURE_LGU_CP_LTE_DBG_SCREEN_REQ_BY_LGU
#define FEATURE_LGU_CP_LTE_EMC_APIS

// Jake, Temporary Feature for Domestic LTE Band7 Lab Test 
// kkosu 131210 Block this Feature(Keep QMI Interfaces) then, add B7 to Domestic LTE Bands
//#define FEATURE_LGU_CP_LTE_DOMESTIC_BAND7_TEST

/*************************************************/
/*                    UICC                       */
/*************************************************/

#define FEATURE_LGU_CP_UICC_CARD_REMOVED_EVENT
#define FEATURE_LGU_CP_UICC_CARD_CUSTOM_INFO
#define FEATURE_LGU_CP_UICC_CARD_TYPE
#define FEATURE_LGU_CP_UICC_CARD_MODE
#define FEATURE_LGU_CP_UICC_SUPPORT_FOR_ISIM_APPLICATION

#define FEATURE_LGU_CP_UICC_BIP_STATUS
#define FEATURE_LGU_CP_UICC_GET_ATR_QMI
#define FEATURE_LGU_CP_UICC_SUPPORT_NFC
#define FEATURE_LGU_CP_UICC_SUPPORT_AKA
#define FEATURE_LGU_CP_UICC_CHECK_ROAMING_SETTINGS
#define FEATURE_LGU_CP_UICC_STK_RESEND
#define FEATURE_LGU_CP_UICC_UI
#define FEATURE_LGU_CP_UICC_FIXED_QC_PROBLEM_FOR_CARD_STATUS
#define FEATURE_LGU_CP_UICC_CARD_STATUS_PINPUK_RETRY_CNT

/*************************************************/
/*                   LBS(GPS)                    */
/*************************************************/

//These are for both of modem and linux
#define FEATURE_SKY_CP_GNSS_INTERFACE
#define FEATURE_LGU_CP_GNSS_XTRA_DL
#define FEATURE_LGU_CP_GNSS_XTRA_DL_TIME
#define FEATURE_LGU_CP_GNSS_NMEA_WRITE

//These are for modem
#define FEATURE_SKY_CP_GNSS_COMMON
#define FEATURE_LGU_CP_GNSS_NETWORK_INFO

//These are for linux
#define FEATURE_SKY_CP_GNSS_FIX_FAIL
#define FEATURE_SKY_CP_GNSS_CONFIGURATION
#define FEATURE_SKY_CP_GNSS_MDM_SETTING
#define FEATURE_LGU_CP_GNSS_TEST_SUPPORT

/*****************************************************
    °øÅë Àû¿ë»çÇ×
    Feature Name : T_SKY_MODEL_TARGET_GLOBAL
******************************************************/
#ifdef T_SKY_MODEL_TARGET_GLOBAL

#define FEATURE_LGU_CP_LOCAL_DB
#ifdef FEATURE_LGU_CP_LOCAL_DB
#define FEATURE_LGU_CP_LOCAL_DB_LTE

/* local_db get/put µ¿ÀÛ¿¡ ´ëÇØ¼­ QMI¸¦ ÀÌ¿ëÇÏµµ·Ï ¼öÁ¤ÇÔ. */
#define FEATURE_LGU_CP_LOCAL_DB_WITH_QMI
#endif/* FEATURE_LGU_CP_LOCAL_DB */

/* QMI À» ÀÌ¿ëÇÏ¿© NV access */
#define FEATURE_LGU_CP_NV_ACCESS_WITH_QMI

#define FEATURE_LGU_USIM_CARD_TYPE

#define FEATURE_LGU_CP_ACQ_DB_DELETE

#ifdef FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE
/* cpmgrif.c daemonÀ» ÅëÇÑ NV Access */
#define FEATURE_LGU_CP_NV_ACCESS_CPMGRIF

#ifdef FEATURE_LGU_CP_LOCAL_DB
/* cpmgrif.c daemonÀ» ÅëÇÑ Local DB Access */
#define FEATURE_LGU_CP_LOCAL_DB_ACCESS_CPMGRIF
#endif/* FEATURE_LGU_CP_LOCAL_DB */
#endif/* FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE */

/*
   ¼­ºñ½º »óÅÂ ¿¡¼­¸¸ MCC/MNC °¡ update µÇ´ø ºÎºÐÀ» Limited service»óÅÂ¿¡¼­µµ update°¡ °¡´ÉÇÏµµ·Ï ¼öÁ¤ÇÔ.
*/
#define FEATURE_LGU_CP_GET_MCCMNC_UPDATE_IN_LIMITED_SRV

/* 
  ´Ü¸» PS ONLY ¸ðµå·Î ¼³Á¤½Ã android´Ü¿¡¼­ Reg state¸¦ No Service·Î Ã³¸®ÇÏ´Â ¿À·ù¸¦ ¼öÁ¤ 
  CS reject, PS accept µÇ´Â reject cause½ÃÇè¿¡¼­µµ µ¿ÀÏ Áõ»ó ¹ß»ý.
*/
#define FEATURE_LGU_CP_SUPPORT_PS_ONLY_MODE

/* qcril ¿¡¼­ ui ·Î ¿Ã¶ó°¡´Â reg_status¿¡ ¿À·ù°¡ ÀÖ¾î ÀÌ¸¦ ¼öÁ¤ÇÔ */
#define FEATURE_LGU_CP_REG_STATUS_UPDATE_CORRECTION

/*
   -. card power down Ä¿¸Çµå Ã³¸®ÇÏÁö ¾Êµµ·Ï ÇÔ.
      < ¹Ì ¼öÁ¤½Ã ¹ß»ýÇÏ´Â ÁÖ¿ä ¹®Á¦Á¡ >
      1. ºñÇàÁß ¸ðµå ÁøÀÔ½Ã USIM card accessµÇÁö ¾Ê´Â ¹®Á¦
      2. ºñÇàÁß ¸ðµå ÁøÀÔ½Ã SIM card°¡ ÀÎ½ÄµÇÁö ¾Ê´Â´Ù´Â ¹®±¸ Ç¥½ÃµÊ.
      3. on chip sim mode·Î µ¿ÀÛÁß ºñÇàÁß ¸ðµå µé¾î°¡¸é card°¡ ¾ø´Â »óÅÂ¿¡¼­ UIMÀ¸·Î card power downÄ¿¸Çµå°¡ Àü´ÞµÇ¾î
*/
#define FEATURE_LGU_CP_CARD_POWER_DOWN_NOT_SUPPORT


/* Limited service»óÅÂ¸¦ ±¸ºÐÇÏ±â À§ÇØ¼­ ServiceState ¼³Á¤. */
#define FEATURE_LGU_CP_SERVICE_STATUS_DETAIL_SUBSTATE


/* 
    Network NameÀ» ¼ö½ÅÇßÀ½¿¡µµ ºÒ±¸ÇÏ°í, "nas_cached_info.current_plmn" °ªÀÌ Invalid·Î Ã³¸®µÇ¾î 
   ÇØ´ç Network NameÀÌ ¾Æ´Ñ TableÀÇ nameÀ» Ç¥½ÃÇÏ´Â ¹®Á¦¸¦ À§ÇØ Qcril¿¡¼­ °ü¸®ÇÏ´Â system info¸¦ 
   state º¯È­½Ã¸¶´Ù updateÇÏµµ·Ï ÇÔ.
   QCT´Â ÀÌ system info¸¦ LCD enable½Ã¿¡¸¸ update ÇÏµµ·Ï ÇÏ°í ÀÖÀ½
*/
#define FEATURE_LGU_CP_QMI_SYS_INFO_ALWAYS_UPDATE

/* OEM QMI commands set */
#define FEATURE_LGU_CP_OEM_COMMANDS_WITH_QMI

/*
   ÇØ¿Ü ·Î¹Ö½Ã (52501: Sing Tel) NITZ Á¤º¸°¡ ³»·Á¿À´Â ¸Á¿¡¼­ OP NAME Á¤º¸°¡ NULLÀº ¾Æ´ÏÁö¸¸ length°¡ (0)ÀÎ °æ¿ì¿¡ ÇØ´ç µÊ.
   MCCMNC Table¿¡¼­ ÇØ´ç OP NAMEÀ» »Ñ¸®´Ù°¡ MM INFO ¼ö½Å ÀÌÈÄ NAMEÀÌ ¾Èº¸ÀÌ´Â °æ¿ì ¼öÁ¤ÇÔ.
   MM INFO·Î Long NameÀº NULLÀÌ°í, Short Name¸¸ ³»·Á¿À´Â °æ¿ì À§ÀÇ qcril_qmi_nas_persist_entry_update ÇÔ¼öÀÇ
   ¸®ÅÏ °ªÀ¸·Î E_FAILUREÀ¸·Î Àü´ÞµÇ´õ¶óµµ Short Name¿¡ ´ëÇØ Ã³¸®ÇÏµµ·Ï ÇÔ.   
*/
#define FEATURE_LGU_CP_FIX_OPERATOR_NAME_DISPLAY

/*
     sky_rawdata ¿µ¿ª¿¡ opening day backup 
*/    
#define FEATURE_LGU_CP_NEW_OPENING_DAY

/* 20120228 ithong.
   sim inserted ¿¡ ÀÇÇÑ sim status indication Á¤º¸³»¿¡ ½ÇÁ¦ ÇÒ´çµÈ slot ÀÌ ¾øÀ½¿¡µµ. gw provision index°¡ ¼³Á¤µÇ¾î linux ¿¡ ¼ö½ÅµÊ
   ÇØ´ç Á¤º¸°¡ ±×´ë·Î ¾Èµå·ÎÀÌµå ´ÜÀ¸·Î Àü´ÞµÇ¾î ÃÊ±â card °´Ã¼¿Í app °´Ã¼ »ý¼º¿¡ ¿µÇâÀ» ¹ÌÄ¥ ¿ì·Á°¡ µÇ¾î ¹æÁö ÄÚµå Àû¿ëÇÔ.
   Áï ÇÒ´çµÈ slotÀÌ ¾ø´Â »óÅÂ¿¡¼­ÀÇ sim status indicationÀº ¾Èµå·ÎÀÌµå ´ÜÀ¸·Î Àü´ÞÇÏÁö ¾Êµµ·Ï ÇÔ.
*/
#define FEATURE_LGU_CP_SKIP_SIM_STAT_INT_FOR_NON_SLOT_STATE

/* 
   20120423 Lee Jonghwan
   30145 patchï¿½ï¿½ qcril.cï¿½ï¿½ android_request_idï¿½ï¿½ QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID (127)ï¿½ï¿½ ï¿½ï¿½ï¿½Ñµï¿½.
   Ril.hï¿½ï¿½ ï¿½ß°ï¿½ï¿½ï¿½ event idï¿½ï¿½ï¿½ï¿½ ï¿½ß°ï¿½ï¿½Ï¿ï¿½ 132 ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ Å« ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
*/
#define FEATURE_LGU_CP_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID_FIX

/*
   20120613 ithong
   UISM GCF 7.2.5 ï¿½×¸ï¿½ Failure : nfc ï¿½ï¿½ï¿½ï¿½ send apduï¿½ï¿½ ï¿½âµ¿ï¿½Ï´ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ failure ï¿½ß»ï¿½ï¿½ï¿½.
   gcf modeï¿½ï¿½ property ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½Ï°ï¿½ uca-ril_qmi.c ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ send apdu ï¿½ï¿½ ï¿½Ãµï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
   gcf modeï¿½ï¿½ È®ï¿½ï¿½ï¿½Ï¿ï¿½ no sim ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½Ã³ï¿½ï¿½ï¿½Ïµï¿½ï¿½ï¿½ ï¿½ï¿½.   
*/
#define FEATURE_LGU_CP_SET_GCF_MODE_PROPERTY

/*
   20120424 Lee Jonghwan
   30145 patchï¿½ï¿½ qcril.c ï¿½ß°ï¿½ï¿½ï¿½ RILï¿½ï¿½ï¿½ï¿½ ITEM Ã³ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½È­ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½Ì¹ï¿½Æ® Ã³ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½.
   Voice & Data REG STATE, Auto & Malual Network Selection ï¿½ï¿½ï¿½ï¿½ RIL eventï¿½ï¿½ 30145 patch ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ Ã³ï¿½ï¿½ï¿½Çµï¿½ï¿½ï¿½ 
   qmi_ril_fw_dedicated_thrd_exec_android_requests_set ï¿½ï¿½ï¿½Ìºï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
*/
#define FEATURE_LGU_CP_FW_DEDICATED_ANDROID_REQ_BUG_FIX


// centralized_eons_supported  ±â´É µ¿ÀÛ¿¡ µû¶ó network name ¹× manual search list Ç¥½Ã ¿À·ù°¡ ¹ß»ýÇÔ.
#define FEATURE_LGU_CP_CENTRALIZED_EONS_NOT_SUPPORTED

/* 	
	Qualcomm Test SBA
	´Ü¸» ÃÖÃÊ ºÎÆÃ ÈÄ MT call ¼ö½Å »óÅÂ¿¡¼­ ÅëÈ­Ãß°¡ ½Ã ¸Á¿¡¼­ Hold reject ÀÌ ³»·Á¿Ã °æ¿ì 
	ÀÌÀü call È­¸éÀ¸·Î ÀüÈ¯µÇÁö ¾Ê°í Managed call·Î º¯°æµÇ´Â Çö»ó.
	qualcomm SR ÁøÇàÇÏ¿© Test SBA¸¦ Àû¿ëÇÏ¿© fail_cause ÀÇ valid°ªÀ» ÃÊ±âÈ­ ½ÃÄÑÁÖµµ·Ï ¼öÁ¤ÇÔ. 
*/
#define FEATURE_LGU_CP_GW_LAST_CALL_FAILURE_FORCED_INIT

/* 	
	USSD Send ÈÄ ¸Á¿¡¼­ ÀÀ´äÀÌ ¾øÀ» °æ¿ì Ã³¸®
*/
#define FEATURE_LGU_CP_GW_USSD_NEWORK_NO_RESPONSE

/* WCDMA, GSM ÀÇ RSSI level ±âÁØ º¯°æ,  H/W ¿äÃ»¿¡ ÀÇÇÑ º¯°æ */
#define FEATURE_LGU_CP_GW_RSSI_LEVEL

/*
    USSD decoding error fix.
    ¹Ì±¹ AT&T ¼±ºÒSIM ÀÜ¾×È¯ÀÎ½Ã ( *777# send ) ¹®ÀÚ ±úÁü.
*/
#define FEATURE_LGU_CP_GW_USSD_RESPONSE_MSG_DECODING_BUG_FIX

/*
    WCDMA/GSM voice call auto answer ( qcril_qmi_voice.c )
*/
#define FEATURE_LGU_CP_GW_VOICE_AUTO_ANSWER

// android/build/config.mk ï¿½ï¿½ defineï¿½Ç¾ï¿½ï¿½ï¿½ï¿½ï¿½...
#ifdef T_SKY_CP_DEBUG_LOG_FUNC

/* 
   ADB Log for OEM
*/
/*
#ifdef FEATURE_TODO
#define FEATURE_SKY_CP_ADB_LOG_FOR_VITAMIN
#endif
*/

/* 
   ADB Log for RIL
*/
/*
#define FEATURE_SKY_CP_DATA_LOG_QMI_LOG_ADB_ON
*/

/* 
   SD Ä«ï¿½å¸¦ ï¿½ï¿½ï¿½ï¿½ DM Logging
*/
#define FEATURE_SKY_CP_DM_LOG_STORE_TEMP_MEMORY

/*
  F3 Trace Logging UI
*/  
#define FEATURE_SKY_CP_F3_TRACE

#define FEATURE_SKY_CP_DMLOGGING_DPL

/*
  FIX the BUG for the ORIGIN CALL TYPE to Set the automatic always.
*/
#define FEATURE_LGU_CP_VOICE_CALL_ORIG_QCT_BUG_FIX

#endif/* T_SKY_CP_DEBUG_LOG_FUNC */

/*
   À½¿µÁö¿ª¿¡¼­ È£ Á¾·á½Ã ¸ÁÀ¸·ÎºÎÅÍ RRC release¸¦ ¼ö½Å ¹ÞÁö ¸øÇÏ°í
   ÀçÂ÷ ´Ü¸»¿¡¼­ ¹ß½Å ½Ãµµ ÈÄ ¹Ù·Î È£Á¾·á½Ã 30ÃÊ°£ MO PendingµÇ¾î Dial È­¸éÀÌ Áö¼ÓÀûÀ¸·Î º¸ÀÌ´Â Çö»ó ¼öÁ¤
*/
#define FEATURE_SKY_CP_HANGUP_BUG_FIX

#endif/* T_SKY_MODEL_TARGET_GLOBAL */


/* ######################################################################### */


/*****************************************************
    LG U+ ¸ðµ¨ Àû¿ë»çÇ×
    Feature Name : T_SKY_MODEL_TARGET_LGU
******************************************************/
#ifdef T_SKY_MODEL_TARGET_LGU

#if defined(T_SKY_MODEL_TARGET_SKT) || defined(T_SKY_MODEL_TARGET_KT)
#error Occured !!  This section is LGU only !!
#endif

#define FEATURE_LGU_CP_GWL_DEFAULTS
#define FEATURE_LGU_CP_LTE_SETTINGS_CONTROL

/* Network ÀÚµ¿¼±ÅÃ,¼öµ¿¼±ÅÃ Ç×¸ñ ¾Æ·¡¿¡ ÇöÀç ¸ÁÁ¤º¸ ( RAT, PLMN, name ) À» Ç¥½ÃÇÔ */
#define FEATURE_LGU_CP_NETWOTK_INFO_DISPLAY

/* LGU »ç¾÷ÀÚ¿ë PLMN Table */
#define FEATURE_LGU_CP_PLMN_TABLE_LIST_SEARCH

#define FEATURE_LGU_CP_INIT_NITZ_INFO_PROPERTY

/*
  reg reject ³»¿ë Ç¥½Ã °¡´ÉÇÏµµ·Ï system property °ª Àû¿ëÇÔ.
  reg reject ³»¿ëÀÌ keyguard È­¸é¿¡ Ç¥½ÃµÇµµ·Ï ÇÔ.
  AMSS°ü·Ã feature : FEATURE_SKY_CP_REJECT_CAUSE_DISPLAY
  reject cause°ü¸®¸¦ ServiceState ¿¡¼­ ÇÏµµ·Ï ¼öÁ¤.
*/
#define FEATURE_LGU_CP_REJECT_CAUSE_DISPLAY

/*
   LGU Çâ PLMN Selection(ÀÚµ¿/¼öµ¿) °ü·Ã ±¸Çö»çÇ×.
*/
#define FEATURE_LGU_CP_NETWORK_PLMN_MANUAL_SELECTION

/* ³×Æ®¿÷³×ÀÓ Ç¥½Ã ¹æ¹ý - Short Name Ç¥½Ã ¿ì¼±. */
#define FEATURE_LGU_CP_DISPLAY_NETWORK_NAME

#define FEATURE_LGU_CP_NETWORK_REG_INFO_IND

#define FEATURE_LGU_CP_MANUAL_SELECTION_WITH_RAT

#define FEATURE_LGU_CP_1X_REG_STATE_UPDATE
#endif/* T_SKY_MODEL_TARGET_LGU */

/* ######################################################################### */

#endif/* __CUST_LGU_CP_H__ */
