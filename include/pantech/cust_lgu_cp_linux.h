#ifndef __CUST_LGU_CP_H__
#define __CUST_LGU_CP_H__


/*****************************************************
    SKY Android �� ���� �������
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
/* Java���� Modem API ȣ���� ���� Daemon ( NV access, �ڵ�����, VE ���, etc.. ) */
#define FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE
#ifdef FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE
/* Voice Call connection sound event */

#define FEATURE_SKY_CP_CS_CALL_CONNECTION_SND_START

/* SKY �ڵ������� Interface */
#define FEATURE_SKY_CP_AUTOANSWER_INTERFACE

#endif/* FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE */
#endif/* FEATURE_LGU_CP_TELEPHONYMANAGER */


/* QMI �� ���ؼ� CM system selection preference ȣ�� */
#define FEATURE_SKY_CP_CM_SYS_SELECTION_PREF

/* UMTS SMS MO PS/CS Domain setting menu support */
#define FEATURE_LGU_CP_SMS_CFG_SET_GW_DOMAIN

/* Phone Operation Mode setting (lpm, offline, online .. ) */
#define FEATURE_LGU_CP_PHONE_OPERATION_MODE

/*
    factory command Airplane mode
*/
#define FEATURE_LGU_CP_FACTORY_AIRPLANE_MODE_I

/* QMI �� ���ؼ� CM PH event info. ó�� */
#define FEATURE_LGU_CP_OEM_PH_EVENT_WITH_QMI

/*
   Android Factory reset, NV should be initialized default value.
*/
#define FEATURE_LGU_CP_FACTORY_RESET_NV_INIT


#define FEATURE_LGU_CP_OEM_QMI_SERVICE

#define FEATURE_LGU_CP_COMMON_GLOBAL_SD

#define FEATURE_LGU_CP_SSR_MODEM_CRASH_MONITOR

/*
** 20140305 - LS HiddenCode�� Block�ϱ� ���� Opening Day�� Sky_RawData�� Write�ϴ� ������
** LS���� ������ API�� ȣ���Ͽ� Hidden Code Flag�� Disable ��.
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
    ���� �������
    Feature Name : T_SKY_MODEL_TARGET_GLOBAL
******************************************************/
#ifdef T_SKY_MODEL_TARGET_GLOBAL

#define FEATURE_LGU_CP_LOCAL_DB
#ifdef FEATURE_LGU_CP_LOCAL_DB
#define FEATURE_LGU_CP_LOCAL_DB_LTE

/* local_db get/put ���ۿ� ���ؼ� QMI�� �̿��ϵ��� ������. */
#define FEATURE_LGU_CP_LOCAL_DB_WITH_QMI
#endif/* FEATURE_LGU_CP_LOCAL_DB */

/* QMI �� �̿��Ͽ� NV access */
#define FEATURE_LGU_CP_NV_ACCESS_WITH_QMI

#define FEATURE_LGU_USIM_CARD_TYPE

#define FEATURE_LGU_CP_ACQ_DB_DELETE

#ifdef FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE
/* cpmgrif.c daemon�� ���� NV Access */
#define FEATURE_LGU_CP_NV_ACCESS_CPMGRIF

#ifdef FEATURE_LGU_CP_LOCAL_DB
/* cpmgrif.c daemon�� ���� Local DB Access */
#define FEATURE_LGU_CP_LOCAL_DB_ACCESS_CPMGRIF
#endif/* FEATURE_LGU_CP_LOCAL_DB */
#endif/* FEATURE_LGU_CP_MANAGER_DAEMON_INTERFACE */

/*
   ���� ���� ������ MCC/MNC �� update �Ǵ� �κ��� Limited service���¿����� update�� �����ϵ��� ������.
*/
#define FEATURE_LGU_CP_GET_MCCMNC_UPDATE_IN_LIMITED_SRV

/* 
  �ܸ� PS ONLY ���� ������ android�ܿ��� Reg state�� No Service�� ó���ϴ� ������ ���� 
  CS reject, PS accept �Ǵ� reject cause���迡���� ���� ���� �߻�.
*/
#define FEATURE_LGU_CP_SUPPORT_PS_ONLY_MODE

/* qcril ���� ui �� �ö󰡴� reg_status�� ������ �־� �̸� ������ */
#define FEATURE_LGU_CP_REG_STATUS_UPDATE_CORRECTION

/*
   -. card power down Ŀ�ǵ� ó������ �ʵ��� ��.
      < �� ������ �߻��ϴ� �ֿ� ������ >
      1. ������ ��� ���Խ� USIM card access���� �ʴ� ����
      2. ������ ��� ���Խ� SIM card�� �νĵ��� �ʴ´ٴ� ���� ǥ�õ�.
      3. on chip sim mode�� ������ ������ ��� ���� card�� ���� ���¿��� UIM���� card power downĿ�ǵ尡 ���޵Ǿ�
*/
#define FEATURE_LGU_CP_CARD_POWER_DOWN_NOT_SUPPORT


/* Limited service���¸� �����ϱ� ���ؼ� ServiceState ����. */
#define FEATURE_LGU_CP_SERVICE_STATUS_DETAIL_SUBSTATE


/* 
    Network Name�� ������������ �ұ��ϰ�, "nas_cached_info.current_plmn" ���� Invalid�� ó���Ǿ� 
   �ش� Network Name�� �ƴ� Table�� name�� ǥ���ϴ� ������ ���� Qcril���� �����ϴ� system info�� 
   state ��ȭ�ø��� update�ϵ��� ��.
   QCT�� �� system info�� LCD enable�ÿ��� update �ϵ��� �ϰ� ����
*/
#define FEATURE_LGU_CP_QMI_SYS_INFO_ALWAYS_UPDATE

/* OEM QMI commands set */
#define FEATURE_LGU_CP_OEM_COMMANDS_WITH_QMI

/*
   �ؿ� �ιֽ� (52501: Sing Tel) NITZ ������ �������� ������ OP NAME ������ NULL�� �ƴ����� length�� (0)�� ��쿡 �ش� ��.
   MCCMNC Table���� �ش� OP NAME�� �Ѹ��ٰ� MM INFO ���� ���� NAME�� �Ⱥ��̴� ��� ������.
   MM INFO�� Long Name�� NULL�̰�, Short Name�� �������� ��� ���� qcril_qmi_nas_persist_entry_update �Լ���
   ���� ������ E_FAILURE���� ���޵Ǵ��� Short Name�� ���� ó���ϵ��� ��.   
*/
#define FEATURE_LGU_CP_FIX_OPERATOR_NAME_DISPLAY

/*
     sky_rawdata ������ opening day backup 
*/    
#define FEATURE_LGU_CP_NEW_OPENING_DAY

/* 20120228 ithong.
   sim inserted �� ���� sim status indication �������� ���� �Ҵ�� slot �� ��������. gw provision index�� �����Ǿ� linux �� ���ŵ�
   �ش� ������ �״�� �ȵ���̵� ������ ���޵Ǿ� �ʱ� card ��ü�� app ��ü ������ ������ ��ĥ ����� �Ǿ� ���� �ڵ� ������.
   �� �Ҵ�� slot�� ���� ���¿����� sim status indication�� �ȵ���̵� ������ �������� �ʵ��� ��.
*/
#define FEATURE_LGU_CP_SKIP_SIM_STAT_INT_FOR_NON_SLOT_STATE

/* 
   20120423 Lee Jonghwan
   30145 patch�� qcril.c�� android_request_id�� QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID (127)�� ���ѵ�.
   Ril.h�� �߰��� event id���� �߰��Ͽ� 132 ������ ū ������ ������.
*/
#define FEATURE_LGU_CP_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID_FIX

/*
   20120613 ithong
   UISM GCF 7.2.5 �׸� Failure : nfc ���� send apdu�� �⵿�ϴ� ������ failure �߻���.
   gcf mode�� property �� �����ϰ� uca-ril_qmi.c �� ���� send apdu �� �õ��� ����
   gcf mode�� Ȯ���Ͽ� no sim ���� ����ó���ϵ��� ��.   
*/
#define FEATURE_LGU_CP_SET_GCF_MODE_PROPERTY

/*
   20120424 Lee Jonghwan
   30145 patch�� qcril.c �߰��� RIL���� ITEM ó�� ������ ��ȭ�� ���������� �̹�Ʈ ó���� ������ ��.
   Voice & Data REG STATE, Auto & Malual Network Selection ���� RIL event�� 30145 patch ������ ���� ó���ǵ��� 
   qmi_ril_fw_dedicated_thrd_exec_android_requests_set ���̺����� ������.
*/
#define FEATURE_LGU_CP_FW_DEDICATED_ANDROID_REQ_BUG_FIX


// centralized_eons_supported  ��� ���ۿ� ���� network name �� manual search list ǥ�� ������ �߻���.
#define FEATURE_LGU_CP_CENTRALIZED_EONS_NOT_SUPPORTED

/* 	
	Qualcomm Test SBA
	�ܸ� ���� ���� �� MT call ���� ���¿��� ��ȭ�߰� �� ������ Hold reject �� ������ ��� 
	���� call ȭ������ ��ȯ���� �ʰ� Managed call�� ����Ǵ� ����.
	qualcomm SR �����Ͽ� Test SBA�� �����Ͽ� fail_cause �� valid���� �ʱ�ȭ �����ֵ��� ������. 
*/
#define FEATURE_LGU_CP_GW_LAST_CALL_FAILURE_FORCED_INIT

/* 	
	USSD Send �� ������ ������ ���� ��� ó��
*/
#define FEATURE_LGU_CP_GW_USSD_NEWORK_NO_RESPONSE

/* WCDMA, GSM �� RSSI level ���� ����,  H/W ��û�� ���� ���� */
#define FEATURE_LGU_CP_GW_RSSI_LEVEL

/*
    USSD decoding error fix.
    �̱� AT&T ����SIM �ܾ�ȯ�ν� ( *777# send ) ���� ����.
*/
#define FEATURE_LGU_CP_GW_USSD_RESPONSE_MSG_DECODING_BUG_FIX

/*
    WCDMA/GSM voice call auto answer ( qcril_qmi_voice.c )
*/
#define FEATURE_LGU_CP_GW_VOICE_AUTO_ANSWER

// android/build/config.mk �� define�Ǿ�����...
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
   SD ī�带 ���� DM Logging
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
   ������������ ȣ ����� �����κ��� RRC release�� ���� ���� ���ϰ�
   ���� �ܸ����� �߽� �õ� �� �ٷ� ȣ����� 30�ʰ� MO Pending�Ǿ� Dial ȭ���� ���������� ���̴� ���� ����
*/
#define FEATURE_SKY_CP_HANGUP_BUG_FIX

#endif/* T_SKY_MODEL_TARGET_GLOBAL */


/* ######################################################################### */


/*****************************************************
    LG U+ �� �������
    Feature Name : T_SKY_MODEL_TARGET_LGU
******************************************************/
#ifdef T_SKY_MODEL_TARGET_LGU

#if defined(T_SKY_MODEL_TARGET_SKT) || defined(T_SKY_MODEL_TARGET_KT)
#error Occured !!  This section is LGU only !!
#endif

#define FEATURE_LGU_CP_GWL_DEFAULTS
#define FEATURE_LGU_CP_LTE_SETTINGS_CONTROL

/* Network �ڵ�����,�������� �׸� �Ʒ��� ���� ������ ( RAT, PLMN, name ) �� ǥ���� */
#define FEATURE_LGU_CP_NETWOTK_INFO_DISPLAY

/* LGU ����ڿ� PLMN Table */
#define FEATURE_LGU_CP_PLMN_TABLE_LIST_SEARCH

#define FEATURE_LGU_CP_INIT_NITZ_INFO_PROPERTY

/*
  reg reject ���� ǥ�� �����ϵ��� system property �� ������.
  reg reject ������ keyguard ȭ�鿡 ǥ�õǵ��� ��.
  AMSS���� feature : FEATURE_SKY_CP_REJECT_CAUSE_DISPLAY
  reject cause������ ServiceState ���� �ϵ��� ����.
*/
#define FEATURE_LGU_CP_REJECT_CAUSE_DISPLAY

/*
   LGU �� PLMN Selection(�ڵ�/����) ���� ��������.
*/
#define FEATURE_LGU_CP_NETWORK_PLMN_MANUAL_SELECTION

/* ��Ʈ������ ǥ�� ��� - Short Name ǥ�� �켱. */
#define FEATURE_LGU_CP_DISPLAY_NETWORK_NAME

#define FEATURE_LGU_CP_NETWORK_REG_INFO_IND

#define FEATURE_LGU_CP_MANUAL_SELECTION_WITH_RAT

#define FEATURE_LGU_CP_1X_REG_STATE_UPDATE
#endif/* T_SKY_MODEL_TARGET_LGU */

/* ######################################################################### */

#endif/* __CUST_LGU_CP_H__ */
