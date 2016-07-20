#ifndef __CUST_SKY_CP_H__
#define __CUST_SKY_CP_H__


/*****************************************************
    SKY Android �� ���� �������
    Feature Name : T_SKY_MODEL_TARGET_COMMON
******************************************************/
#ifdef T_SKY_MODEL_TARGET_COMMON


/* 
   SkyTelephonyInterfaceManager.java, Service_Manager.c, ISkyTelephony.aidl 
*/
#define FEATURE_SKY_CP_TELEPHONYMANAGER
#ifdef FEATURE_SKY_CP_TELEPHONYMANAGER
/* Java���� Modem API ȣ���� ���� Daemon ( NV access, �ڵ�����, VE ���, etc.. ) */
#define FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE
#ifdef FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE
/* Voice Call connection sound event */

#define FEATURE_SKY_CP_CS_CALL_CONNECTION_SND_START

/* SKY �ڵ������� Interface */
#define FEATURE_SKY_CP_AUTOANSWER_INTERFACE

#endif/* FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE */
#endif/* FEATURE_SKY_CP_TELEPHONYMANAGER */


/* QMI �� ���ؼ� CM system selection preference ȣ�� */
#define FEATURE_SKY_CP_CM_SYS_SELECTION_PREF

/* UMTS SMS MO PS/CS Domain setting menu support */
#define FEATURE_SKY_CP_SMS_CFG_SET_GW_DOMAIN

/* Phone Operation Mode setting (lpm, offline, online .. ) */
#define FEATURE_SKY_CP_PHONE_OPERATION_MODE

/*
    factory command Airplane mode
*/
#define FEATURE_SKY_CP_FACTORY_AIRPLANE_MODE_I

/* QMI �� ���ؼ� CM PH event info. ó�� */
#define FEATURE_SKY_CP_OEM_PH_EVENT_WITH_QMI

/*
    QMI�� �̿��Ͽ� Modem�� ���� �ϱ� ���� �ڵ�� 
    ���� Sar Test�� ���ؼ� ������ �ڵ忡 OEM �ڵ带 �߰���.
*/
#define FEATURE_SKY_CP_OEM_QMI_ACCESS


/*
   Android Factory reset, NV should be initialized default value.
*/
#define FEATURE_SKY_CP_FACTORY_RESET_NV_INIT


#define FEATURE_SKY_CP_OEM_QMI_SERVICE

// kkosu, new phantom daemon and interfaces
/*
#define FEATURE_SKY_CP_PHANTOM
*/


#define FEATURE_SKY_CP_SSR_MODEM_CRASH_MONITOR

#endif/* T_SKY_MODEL_TARGET_COMMON */


/* ######################################################################### */



/*****************************************************
    SKT/KT ���� �������
    Feature Name : T_SKY_MODEL_TARGET_WCDMA
******************************************************/
#ifdef T_SKY_MODEL_TARGET_WCDMA


/* OEM RAPI �� �̿��ϱ� ���� Protocol Feature */
//#define FEATURE_SKY_RPC_OEM_RAPI_NEW_INTERFACE

//#define FEATURE_SKY_CP_ADDITIONAL_NV_ITEMS
//#define FEATURE_SKY_CP_RPC_XDR_NV_ACCESS

#define FEATURE_SKY_CP_LOCAL_DB
#ifdef FEATURE_SKY_CP_LOCAL_DB
#define FEATURE_SKY_CP_LOCAL_DB_LTE

/* Additional CA Debug Screen about SCELL */
#define FEATURE_SKY_CP_LOCAL_DB_LTE_SCELL_INFO

/* local_db get/put ���ۿ� ���ؼ� QMI�� �̿��ϵ��� ������. */
#define FEATURE_SKY_CP_LOCAL_DB_WITH_QMI
#endif/* FEATURE_SKY_CP_LOCAL_DB */

/* QMI �� �̿��Ͽ� NV access */
#define FEATURE_SKY_CP_NV_ACCESS_WITH_QMI

#define FEATURE_SKT_CP_SUBSCRIPTION_MODE

#ifdef FEATURE_SKT_CP_SUBSCRIPTION_MODE
/*
  13.07.09 ithong this feature must be confined models which support ota on bip
*/
#define FEATURE_SKT_USIM_SUPPORT_BIP_FOR_SUBSCRIPTION
#endif

#define FEATURE_SKY_CP_OTA_KT_SUBSCRIPTION

#define FEATURE_SKY_USIM_CARD_TYPE

#define FEATURE_SKY_USIM_GET_CARD_MODE_AS_APP_TYPE

#define FEATURE_SKY_CP_ACQ_DB_DELETE

#ifdef FEATURE_SKY_CP_QCRILHOOK_INTERFACE
/* OEM Hook ������� NV access */
#define FEATURE_SKY_CP_NV_ACCESS_OEMHOOK

#ifdef FEATURE_SKY_CP_LOCAL_DB
/* OEM Hook ������� Local DB access */
#define FEATURE_SKY_CP_LOCAL_DB_ACCESS_OEMHOOK
#endif/* FEATURE_SKY_CP_LOCAL_DB */
#endif/* FEATURE_SKY_CP_QCRILHOOK_INTERFACE */

#ifdef FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE
/* cpmgrif.c daemon�� ���� NV Access */
#define FEATURE_SKY_CP_NV_ACCESS_CPMGRIF

#ifdef FEATURE_SKY_CP_LOCAL_DB
/* cpmgrif.c daemon�� ���� Local DB Access */
#define FEATURE_SKY_CP_LOCAL_DB_ACCESS_CPMGRIF
#endif/* FEATURE_SKY_CP_LOCAL_DB */
#endif/* FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE */

/*
    SKT/KT ���� Me Personalization ( With QMI interface )
*/
#define FEATURE_SKY_USIM_ME_PERSONALIZATION

#ifdef FEATURE_SKY_USIM_ME_PERSONALIZATION
//hrkim 20140311  do not ask pck number on SSR .
//#define FEATURE_SKY_CP_SUPPORT_USIM_SSR
#endif
/* 
   1. qcril ���� default voice tech.�� 3GPP2�� �̴� 3GPP�� ����.
     --> ���� �ʱ⿡ 3GPP2�� radio technology�� õ�̵Ǿ��ٰ� 3GPP�� �ٲ� 3GPP2�� CDMA SIM ���� SIM ready�� 
            �߻��Ͽ� PUK�� PIN ������ ����� �ȵǰų� SIM Record�Լ����� CDMA������ �����ϴ� ��찡 �߻���.
   2. java class���� Phone type�� GSMPhone���� create �ǵ��� ������.
*/
#define FEATURE_SKY_CP_DEFAULT_NETWORK_UMTS_IN_RIL


/*
    -. WMS Link control�� ����ϸ� MMS app.�� ���� �߽� �� ������ �Ѱܹ޾� WMS���� ó���ϵ��� RIL I/F ����
*/
#define FEATURE_SKY_CP_MULTI_SMS_REQUIREMENT

/* CNAP ������ ���� RIL �� framework ����. */
#define FEATURE_SKY_CP_CNAP_SUPPORT

/* ���� ����� (SKT, KT) ������ KSC5601 format�� USSD msg.�� CNAP ������ ���� ����. */
#define FEATURE_SKY_CP_USSD_CNAP_8BIT_CHAR_SUPPORT

/* WCDMA, GSM �� RSSI level ���� ����,  H/W ��û�� ���� ���� */
#define FEATURE_SKY_CP_GW_RSSI_LEVEL

/* 
    RSSI report �� �������� �̷�� �� �� �ֵ��� delta threadhold ���� ����
    modem�ڵ忡�� cmph.h �� ���� ����.
 */
#define FEATURE_SKY_CP_RSSI_REPORT_VARIATION_FIX

/*
    ����ȣ �߽��� ���ؼ� CSFB���� �� �Ǵ� ����ȣ ���� �� LTE�� �̵��Ҷ� 
    �ް��� Ant. bar�� ��ȭ�� ���� �ϱ� ���� ó��.
    ���� ����
*/
/* #define FEATURE_SKY_CP_RSSI_REPORT_TIMING_GAP_FIX */

/*
- Limited Service���¿����� RSSI Bar�� ������ 0���� ǥ���ϵ��� ����.
- SIM���� ���õǰų� �����ʱ� reject���� limited service�� �����Ǹ� no service�� �νĵǱ� ������ 
   qcril_cm.c ���� qcril_cm_util_srv_sys_info_to_reg_state() �Լ��� limited service �� �������ִ� ���� ������ �ʿ���.
*/
#define FEATURE_SKY_CP_RSSI_BAR_ZERO_DISPLAY

/*
   ���� ���� ������ MCC/MNC �� update �Ǵ� �κ��� Limited service���¿����� update�� �����ϵ��� ������.
*/
#define FEATURE_SKY_CP_GET_MCCMNC_UPDATE_IN_LIMITED_SRV

/* 
  �ܸ� PS ONLY ���� ������ android�ܿ��� Reg state�� No Service�� ó���ϴ� ������ ���� 
  CS reject, PS accept �Ǵ� reject cause���迡���� ���� ���� �߻�.
*/
#define FEATURE_SKY_CP_SUPPORT_PS_ONLY_MODE

/* qcril ���� ui �� �ö󰡴� reg_status�� ������ �־� �̸� ������ */
#define FEATURE_SKY_CP_REG_STATUS_UPDATE_CORRECTION

/*
   -. �ܸ��� service status�� SYS_SRV_STATUS_PWR_SAVE �϶� QCRIL_CM_SCREEN_STATE_ON ���°� �ɶ� ��
      Key interrup �߻����� LCD�� on�ɶ� �ܸ��� power save���¶�� modem�� wakeup��Ű���� �ǳ�, 
      qualcomm bug�� ���� �ȵǴ� ���� ���� 
*/
#define FEATURE_SKY_CP_MODEM_PWR_SAVE_WAKEUP_FIX

/*
   -. card power down Ŀ�ǵ� ó������ �ʵ��� ��.
      < �� ������ �߻��ϴ� �ֿ� ������ >
      1. ������ ��� ���Խ� USIM card access���� �ʴ� ����
      2. ������ ��� ���Խ� SIM card�� �νĵ��� �ʴ´ٴ� ���� ǥ�õ�.
      3. on chip sim mode�� ������ ������ ��� ���� card�� ���� ���¿��� UIM���� card power downĿ�ǵ尡 ���޵Ǿ�
*/
#define FEATURE_SKY_CP_CARD_POWER_DOWN_NOT_SUPPORT

/*
   ��ȭ�� ����� ������ ���� Setup ind. ���� signal value�� ������.
   AMSS���� FEATURE_SKY_CP_SETUP_IND_SIGNAL_VALUE �� �ݵ�� �����Ǿ�� ��.
*/   
//#define FEATURE_SKY_CP_SETUP_IND_SIGNAL_VALUE

/*
   ETC(���濬��) ���� ����� call end�� ���ؼ� CM_EVENT_CALL_MNG_CALLS_CONF �� ó������ ���ϰ� 
   pending�Ǿ� �ִ°�� call endȭ�� ó���� ���� ���ϰ� ���� ���� �˾��� �߻��ϴ� ���� ����.
*/
#define FEATURE_SKY_CP_ETC_BUG_FIX

/*
   ����ȣ ���� �� Disconnecting ���� Disconnected �� ���� õ�� �� ���� �ʴ� ���
   RIL �� ���ؼ� call end�� ���������� ȣ�� �Ǿ����� ���� ���� property�� ������ ����.
*/
#define FEATURE_SKY_CP_CALL_END_REQ_DEBUGGING

/* 
  Network preferred mode ���� Menu
  Automatic, WCDMA only, GSM only
  ���������� �޴� ���� ���ϵ��� ����.
*/
#define FEATURE_SKY_CP_PREFERRED_NETWORK_MODE_SELECTION

/** GNSSS Feature :  common **/
#define FEATURE_SKY_CP_GPS_TEST_SUPPORT

/** GNSS Feature :  Configuration **/
#define FEATURE_SKY_CP_GNSS_CONFIGURATION

/**GNSS Feature :  report fix fail  **/
#define FEATURE_SKY_CP_GNSS_FIX_FAIL

/**GNSS Feature :  Qop,  operation mode interface **/
#define FEATURE_SKY_CP_GNSS_INTERFACE

/* SIM �� ���� �ν� ���ο� ���¸� �˾ƺ������� �׽�Ʈ �ڵ� */
#define FEATURE_SKY_USIM_CARD_STATUS_PROPERTY

/*
   RIL_REQUEST_GET_NEIGHBORING_CELL_IDS Ŀ�ǵ尡 ȣ�� �Ǿ����� 
   Neighbor Cell������ ������ �ִ� ��� system halt�� ������.
   ���ʿ��� Ŀ�ǵ�� ó������ �ʵ��� ����.
*/
#define FEATURE_SKY_CP_GET_NEIGHBORING_CELL_IDS_NOT_SUPPORT

/* Limited service���¸� �����ϱ� ���ؼ� ServiceState ����. */
#define FEATURE_SKY_CP_SERVICE_STATUS_DETAIL_SUBSTATE


/* 
    Network Name�� ������������ �ұ��ϰ�, "nas_cached_info.current_plmn" ���� Invalid�� ó���Ǿ� 
   �ش� Network Name�� �ƴ� Table�� name�� ǥ���ϴ� ������ ���� Qcril���� �����ϴ� system info�� 
   state ��ȭ�ø��� update�ϵ��� ��.
   QCT�� �� system info�� LCD enable�ÿ��� update �ϵ��� �ϰ� ����
*/
#define FEATURE_SKY_CP_QMI_SYS_INFO_ALWAYS_UPDATE

/* OEM QMI commands set */
#define FEATURE_SKY_CP_OEM_COMMANDS_WITH_QMI

/*
   �ؿ� �ιֽ� (52501: Sing Tel) NITZ ������ �������� ������ OP NAME ������ NULL�� �ƴ����� length�� (0)�� ��쿡 �ش� ��.
   MCCMNC Table���� �ش� OP NAME�� �Ѹ��ٰ� MM INFO ���� ���� NAME�� �Ⱥ��̴� ��� ������.
   MM INFO�� Long Name�� NULL�̰�, Short Name�� �������� ��� ���� qcril_qmi_nas_persist_entry_update �Լ���
   ���� ������ E_FAILURE���� ���޵Ǵ��� Short Name�� ���� ó���ϵ��� ��.   
*/
#define FEATURE_SKY_CP_FIX_OPERATOR_NAME_DISPLAY

/*
   ������ ��� ���� �� ��� ���� �� Lock Screen, status bar�� spn�� ������� �ʴ� ���� ������.
*/
#define FEATURE_SKY_CP_SPN_UPDATE_IN_AIRPLANE_MODE

/*
    NFC Ȯ�� API �����Ͽ� Get ATR �Լ��� QMI ����
*/
#define FEATURE_SKY_USIM_GET_ATR_QMI

/*
  Data ���񽺽ÿ� RSSI BAR�� 0���� �Ǵ� ���� �߻� Background�� TAU�ö� ���鼭 �̿� ���� 
  Srv State change �� ���� ���� �ʴ� Linux���� Limited�� �ν��� ���� ���� ..Data ���� ���¿�����
  Sig strength �� �ö���� ��Ȳ��..�̶� Sig info �� ���Ҿ� Srv State change �� ���� �ö� ������ ��
*/
#define FEATURE_SKY_CP_REPORT_STATE_CHANGE_WITH_SIG_INFO_DURING_DATA_SVC

/*
     sky_rawdata ������ opening day backup 
*/    
#define FEATURE_SKY_CP_NEW_OPENING_DAY

/*
  illegal SIM ���°� �Ǹ� SIM state�� not ready ���°� �Ǿ� SIM menu ������ ���� �ʴµ��� 
  ����� ������ �߻���. ( ������  illegal sim ���°� �Ǵ��� ME perso. ���� ������ �����ؾ� ��. )
*/
#define FEATURE_SKY_CP_MAINTAIN_SIM_READY_ILLEGAL_SIM

#define FEATURE_SKY_USIM_PIN_RETRY_INIT

/* 20120228 ithong.
   sim inserted �� ���� sim status indication �������� ���� �Ҵ�� slot �� ��������. gw provision index�� �����Ǿ� linux �� ���ŵ�
   �ش� ������ �״�� �ȵ���̵� ������ ���޵Ǿ� �ʱ� card ��ü�� app ��ü ������ ������ ��ĥ ����� �Ǿ� ���� �ڵ� ������.
   �� �Ҵ�� slot�� ���� ���¿����� sim status indication�� �ȵ���̵� ������ �������� �ʵ��� ��.
*/
#define FEATURE_SKY_CP_SKIP_SIM_STAT_INT_FOR_NON_SLOT_STATE

/* 20120314 ithong
   isim application ������ ���� feature.
   �𵩴ܿ����� isim app init �� �������� �ʱ⿡ isim app detected ���� usim ready�Ǹ�,
   isim app ���� ready �� ó���Ͽ� �ȵ���̵������ �����ϵ��� ��.
*/
#define FEATURE_SKY_CP_OEM_SUPPORT_FOR_ISIM_APP_IN_QCRIL

/* 	
	Qualcomm Test SBA
	�ܸ� ���� ���� �� MT call ���� ���¿��� ��ȭ�߰� �� ������ Hold reject �� ������ ��� 
	���� call ȭ������ ��ȯ���� �ʰ� Managed call�� ����Ǵ� ����.
	qualcomm SR �����Ͽ� Test SBA�� �����Ͽ� fail_cause �� valid���� �ʱ�ȭ �����ֵ��� ������. 
*/
#define FEATURE_SKY_CP_LAST_CALL_FAILURE_FORCED_INIT

/* 	
	USSD Send �� ������ ������ ���� ��� ó��
*/
#define FEATURE_SKY_CP_USSD_NEWORK_NO_RESPONSE

/* 
   20120423 Lee Jonghwan
   30145 patch�� qcril.c�� android_request_id�� QMI_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID (127)�� ���ѵ�.
   Ril.h�� �߰��� event id���� �߰��Ͽ� 132 ������ ū ������ ������.
*/
#define FEATURE_SKY_CP_RIL_FW_ANDROID_REQUEST_HNDL_MAX_EVT_ID_FIX

/*
   20120613 ithong
   UISM GCF 7.2.5 �׸� Failure : nfc ���� send apdu�� �⵿�ϴ� ������ failure �߻���.
   gcf mode�� property �� �����ϰ� uca-ril_qmi.c �� ���� send apdu �� �õ��� ����
   gcf mode�� Ȯ���Ͽ� no sim ���� ����ó���ϵ��� ��.   
*/
#define FEATURE_SKY_CP_SET_GCF_MODE_PROPERTY

/*
   20120424 Lee Jonghwan
   30145 patch�� qcril.c �߰��� RIL���� ITEM ó�� ������ ��ȭ�� ���������� �̹�Ʈ ó���� ������ ��.
   Voice & Data REG STATE, Auto & Malual Network Selection ���� RIL event�� 30145 patch ������ ���� ó���ǵ��� 
   qmi_ril_fw_dedicated_thrd_exec_android_requests_set ���̺����� ������.
*/
#define FEATURE_SKY_CP_FW_DEDICATED_ANDROID_REQ_BUG_FIX

// centralized_eons_supported  ��� ���ۿ� ���� network name �� manual search list ǥ�� ������ �߻���.
#define FEATURE_SKY_CP_CENTRALIZED_EONS_NOT_SUPPORTED

/*
   google hiddenmenu ���� ���� mode pref ���� ���� ���� 
   packages\apps\Settings\res\xml\testing_settings.xml
*/
#define FEATURE_SKY_CP_GOOGLE_HIDDEN_MENU_BLOCK


// android/build/config.mk �� define�Ǿ�����...
#ifdef T_SKY_CP_DEBUG_LOG_FUNC

/* 
   ADB Log for OEM
*/
/* 
#define FEATURE_SKY_CP_ADB_LOG_FOR_VITAMIN
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
  DDMS�α׿� RADIO �α� �����ϰ���
*/
#define FEATURE_SKY_CP_LOGCAT_RADIO

#endif/* T_SKY_CP_DEBUG_LOG_FUNC */

/* 20120314 ithong
   isim application ������ ���� feature.
   �𵩴ܿ����� isim app init �� �������� �ʱ⿡ isim app detected ���� usim ready�Ǹ�,
   isim app ���� ready �� ó���Ͽ� �ȵ���̵������ �����ϵ��� ��.
*/
#define FEATURE_SKY_CP_OEM_SUPPORT_FOR_ISIM_APP_IN_QCRIL

/*
    USSD decoding error fix.
    �̱� AT&T ����SIM �ܾ�ȯ�ν� ( *777# send ) ���� ����.
*/
#define FEATURE_SKY_CP_USSD_RESPONSE_MSG_DECODING_BUG_FIX

/*
   Target for MDM model.
   This feature is for gps and air mode setting.
   
*/
#define FEATURE_SKY_CP_MDM_SETTING

/* 
    LTE 3GFB call��  system time�� �����Ǵ� ���� ���� 900ms ������ system time setting�� skip�ϵ��� �Ͽ� ���������� ����
*/
#define FEATURE_SKY_CP_DECREASE_TIME_ROLLBACK

/*
  Auto Answer property ���� �߰� 
*/
#define FEATURE_SKY_CP_GW_VOICE_AUTO_ANSWER

/* [EF56�迭] 1025 ��ġ ������ LTE������ ���� ��/�߽� �õ��� CSFB ������ �ȵǴ� ���� ����.
*  QCT���� CR �ݿ��Ǹ� �����ʿ���.
*/
#define FEATURE_SKY_CP_VOICE_CALL_ORIG_QCT_BUG_FIX

/*
  3'rd party IMS srvcc ����
*/
#define FEATURE_SKY_CP_SRVCC_SUPPORT

/*
  srvcc debugging
*/
#define FEATURE_SKY_CP_SRVCC_DEBUG

/*
   Display FAKE security mode 
*/   
#define FEATURE_SKY_CP_NOTIFY_FAKE_SECURITY_MODE


#define FEATURE_SKY_CP_MANAGER_TEST_MODE

#define FEATURE_SKY_CP_MEM_FREE_QUALCOMM_BUG_FIX

/*
   LTE only ���� �� CDMAPhone ���� create �Ǵ� ������ �߻�.
   radio tech�� unknown���� ���� �Ǿ�������� qcril�� �ʱ� ������ IS95A �� �Ǿ�����.
*/
#define FEATURE_SKY_CP_RADIO_TECH_BUG_FIX

/*
   ������������ ȣ ����� �����κ��� RRC release�� ���� ���� ���ϰ�
   ���� �ܸ����� �߽� �õ� �� �ٷ� ȣ����� 30�ʰ� MO Pending�Ǿ� Dial ȭ���� ���������� ���̴� ���� ����
*/
#define FEATURE_SKY_CP_HANGUP_BUG_FIX

/*
** 20140305 - LS HiddenCode�� Block�ϱ� ���� Opening Day�� Sky_RawData�� Write�ϴ� ������
** LS���� ������ API�� ȣ���Ͽ� Hidden Code Flag�� Disable ��.
*/
#define FEATURE_SKY_CP_HIDDEN_CODE_DISABLE_FLAG

#endif/* T_SKY_MODEL_TARGET_WCDMA */


/* ######################################################################### */


/*****************************************************
    SKT �� �������
    Feature Name : T_SKY_MODEL_TARGET_SKT
******************************************************/
#ifdef T_SKY_MODEL_TARGET_SKT

#ifdef T_SKY_MODEL_TARGET_KT
#error Occured !!  This section is SKT only !!
#endif

#ifdef FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE

/* USIM card removed event */
#define FEATURE_SKY_CP_USIM_CARD_REMOVED_EVENT

#endif/* FEATURE_SKY_CP_MANAGER_DAEMON_INTERFACE */

/*
   GNSS requirement ( Debug Screen)
*/
#define FEATURE_SKT_CP_GNSS_DEBUG_SCREEN

/*
   GNSS requirement (SUPL)
*/
#define FEATURE_SKT_CP_GNSS_SUPL_CONFIGURATION


/* SKT ����ڿ� PLMN Table */
#define FEATURE_SKT_CP_PLMN_TABLE_LIST_SEARCH

/* SKT IOT Problem fix.
    PLMN�� network name �� property�� ����Ǿ� RTS ������ 
    MM info.�� network name ���� / ������ ���迡�� fail�Ǵ� ����.
*/
#define FEATURE_SKT_CP_INIT_NITZ_INFO_PROPERTY

/* SKT �ѹ��÷��� ���� ���� */
#define FEATURE_SKT_CP_NUMBER_PLUS_SUPPORT
#ifdef FEATURE_SKT_CP_NUMBER_PLUS_SUPPORT
#define FEATURE_SKT_CP_NUM_PLUS_WITH_QMI
#endif/* FEATURE_SKT_CP_NUMBER_PLUS_SUPPORT */

/*
  reg reject ���� ǥ�� �����ϵ��� system property �� ������.
  reg reject ������ keyguard ȭ�鿡 ǥ�õǵ��� ��.
  AMSS���� feature : FEATURE_SKY_CP_REJECT_CAUSE_DISPLAY
  reject cause������ ServiceState ���� �ϵ��� ����.
*/
#define FEATURE_SKT_CP_REJECT_CAUSE_DISPLAY

/* SKT ���� �÷��� ���� Interface */
#define FEATURE_SKT_CP_VE_SUPPORT

/*
   SKT�� PLMN Selection(�ڵ�/����) ���� ��������.
*/
#define FEATURE_SKT_CP_NETWORK_PLMN_MANUAL_SELECTION

/* 
   IMEI mismatch Lock applet �����δ� ���� Lock �� �ɸ��� �ʱ� ������
   USIM Lock�� ���ؼ� �߰� 
 */
/* #define FEATURE_SKT_USIM_IMEI_LOCK */

/*
    SKT �䱸����.   
    LTE reject cause #10, #40 �� ���ؼ� no service�� ǥ������ �ʵ��� ��.
*/
#define FEATURE_SKT_CP_LTE_RSSI_DISPLAY_REQUIREMENT

/*
    SKT USIM �䱸����. (WS-USIM-1.75)   
    ME should not cache EF CFIS/ EF MBI/ EF MWIS/ EF MBDN in case of SKT USIM inserted.
    Except for SKT USIM, handset shall work with CPHS or 3GPP for interoperability issue.
*/
#define FEATURE_SKT_USIM_NOT_CACHE_EF_FIELD

/*
  GUAM������ �ð����� ǥ�� ���� ���� ���� ����..
*/
#define FEATURE_SKY_CP_TIMEZONE_FIX

/*
   after missed 7 calls, incoming call is restricted... ( but mo call is possible .. ) 
   qcril_qmi_voice.c 
*/
#define FEATURE_SKY_CP_MISSED_CALL_BUG_FIX

/*
    SKT requirement 
    Area being received SIB19 ( LTE and WCDMA overlay area ), Data icon should be displayed with LTE ( not 3G ) icon.
    regardless of voice call state.....
*/
#define FEATURE_SKT_CP_SIB19_SCHEDULING_NOTIFICATION

/*
    SKT QA support
*/
#define FEATURE_SKT_CP_FA_CHANGE

/*
    SKT LTE/WCDMA RSSI Bar Req.
*/
#define FEATURE_SKT_CP_WL_RSSI_LEVEL

/*
    SKT AMR-WB Control API
*/
#define FEATURE_SKT_CP_WCDMA_AMR_WB_CONTROL_API

/*
   SKT IRR-2.4 requirement.
   SKT name table --> Nitz info. --> qualcomm name table --> mccmnc number
*/
#define FEATURE_SKT_CP_NETWORK_NAME_DISP_ORDER

#define FEATURE_SKT_CP_NETWORK_REG_INFO_IND


/*
    SSA�� QMI service
*/
#define FEATURE_SKT_CP_SSA_QMI_SERVICE

/*
   SSA log report
*/
#define FEATURE_SKT_CP_SSA_LOG

#endif/* T_SKY_MODEL_TARGET_SKT */


/* ######################################################################### */


/*****************************************************
    KT �� �������
    Feature Name : T_SKY_MODEL_TARGET_KT
******************************************************/
#ifdef T_SKY_MODEL_TARGET_KT

#ifdef T_SKY_MODEL_TARGET_SKT
#error Occured !!  This section is KT only !!
#endif


#endif/* T_SKY_MODEL_TARGET_KT */





/***********************************************************
   JAVA code���� ����� Feature �� code Ȯ�ο�...
   ������ C/C++ �ڵ忡�� ������� �ʴ� �������� 
   �ڵ� �˻��� Ȯ�ο뵵�� ����ϱ� ����.
************************************************************/

/* import ���̳� ���� ��ɿ� �������� ���� �ڵ� ǥ�� */
#define FEATURE_SKY_CP_COMMON_JAVA_DECLARATION

/* EF File ������ Path �� �̿��ϱ� ���ؼ� ���� �ڵ� */
#define FEATURE_SKY_USIM_EF_FILE_ACCESS_BY_PATH

/* USIM Card ���Ž� ����� noti. */
#define FEATURE_SKY_CP_CARD_REMOVED

/* �ιֻ��¿��� UI���� ������ ���� fake roaming setting */
#define FEATURE_SKY_CP_ANDROID_FAKE_ROAMING_SETTING

/* Hidden Code ����. �� ���ʿ��� code���� ����. */
#define FEATURE_SKY_CP_HIDDEN_CODE_CONTROL

/* SKT USIM���� UA Field������ �ʿ��� MIN/IRM ����. */
#define FEATURE_SKT_CP_CDMA_MIN_EF_ACCESS

/* GCF mode ���� ���� */
#define FEATURE_SKT_CP_CHECK_GCF_MODE

/* 
   android Settings.DEVICE_PROVISIONED �� �׻� 1�� �����Ǿ��ֵ��� ��. 
   ���� �ʱ⿡ GSM ���� ���Ž� ���� �źεǾ� �׽�Ʈ ���࿡ ������ �߱� ��Ŵ..
*/
#define FEATURE_SKY_CP_DISABLE_CHECK_DEVICE_PROVISIONED

/* Qualcomm Auto Answer ���� ���� ����. ( SKY �ڵ������� �ƴ� !!) */
#define FEATURE_SKY_CP_AUTO_ANSWER_BUG_FIX

/* SKT Emergency Call requirement. */
#define FEATURE_SKT_CP_CHECK_EMERGENCY_CALL

/* ��Ʈ������ ǥ�� ��� - Short Name ǥ�� �켱. */
#define FEATURE_SKT_CP_DISPLAY_NETWORK_NAME

/* Roming indicatorǥ�� ���� ����. */
#define FEATURE_SKY_CP_UPDATING_ROAMING_INDICATOR


/* Lock Screen�� ǥ�õǴ� network name on-off ���  */
/* not used feature */
/* #define FEATURE_SKT_CP_NETWORK_NAME_DISPLAY_ONOFF */

/* SIM state check ���� ����. patch�� �ش� �ڵ� Ȯ�ο��. */
/* #define FEATURE_SKY_USIM_GET_SIM_STATE */

/* 
   Power off ������ PS�� release�Ǳ� ���� android power off sequence�� �Ϸ�Ǿ� 
   ������ Detach�� �ø��� ���ϴ� ��찡 �־� ������ 1�� ���� delay��Ŵ.
*/
#define FEATURE_SKY_CP_DELAYED_POWER_OFF

/* KT SIM MSISDNǥ�� �� SKT SIM MSISDN ǥ������ ����. */
//#define FEATURE_SKY_CP_DISPLAY_PHONE_NUMBER_WITH_TOA

/* Android Setting �޴����� ��ȭ���� �޴� ����.  */
//#define FEATURE_SKY_CP_REMOVE_ANDROID_CALL_FEATURE_SETTING_MENU

/*
    framework/base/core/res/res/config.xml
    network location provider �������� 
    �ʱⰪ�� null �� �Ǿ��־� wifi, cell ��� ��ġ������ �ȵ�.
*/
//#define FEATURE_SKY_CP_NETWORK_LOCATION_PROVIDER_SETTING

/*
   SKT requirement. ( ���� �߽ſ�. )
   *23# �� ���� feature code�� ����� ȣ�߽��� sups. service�� �νĵ��� �ʵ��� ��.
*/
#define FEATURE_SKT_CP_SUPS_SERVICE_REQUEST

/* Roaming Dual Clock �� RAD ������ ���� �̺�Ʈ �߻�. */
#define FEATURE_SKT_CP_EVENT_TIME_AND_RAD

/* lockscreen���� ���ȣ ��ư, ���� ���� ǥ�ù� ��ġ ������ ���� ����. */
#define FEATURE_SKT_CP_EMERGENCY_BUTTON_LOCKSCREEN

/* 
   ������� �������� ��������.
   A�� B�� ��ȭ�� C���� ��ȭ�� ������ A�� B�� ��ȭ�� �ߴ��ϸ� C�� �ڵ����� ����Ǿ� 
   ������ ��.  Alert���¸� �����ϵ��� ����.
*/
//#define FEATURE_SKY_CP_BLUEBOOTH_CERTIFICATION

/* USIM PUK permanent block ó���� ���� �߰�����  */
//#define FEATURE_SKY_CP_ADD_PERM_BLOCKED_CARD_STATUS

/*
    Absend, Ready, Error ���� card state�� return�ޱ� ���� interface
    �����κ��� ilegal MS: Reject (3)�� �޴� USIM�� ��� �����ҽ��� SIM_STATE UNKNOWN������ STATE�� READY�� ����.
*/
//#define FEATURE_SKY_CP_GET_ICC_CARD_STATE

/* ��ġ�� ���� �޴� �������� AGPS( GPS ����� ��� ) ������ ����˾� */
//#define FEATURE_SKT_CP_ALERT_FOR_USING_AGPS

/*  
   Airplane mode ���Խ� network mode menu ������ ���������� ������ ���������� ���� �߻�  
   ���� EF30S�� ���� ������ ��� service state�� ���� airplane mode check box�� 
   disable ��Ű���� �ϴ� �κ���   ����..������ ���� �������� �ش� �κ� ����...
   EF30S �ҽ�������� �ش� �Լ� ���� ���� 6140�� �ش� �κ���  ���� ���� ����..
*/
//#define FEATURE_SKY_CP_AIRPLANE_MODE_CHECK_BOX_CONTROL

/*
  PLMN Selection ���� ���°� Manual ���¿��� FPLMN�� �ִ� �� ( KT�� ����)
  �� �����ؼ� reject�� �ް� �ܸ��� ������ϸ� RPLMN(SKT) ���� ����� ����
  ��ٷ� ������ reject �޾Ҵ� ������ ������ �õ��ϴ°��� ����.
*/
//#define FEATURE_SKY_CP_BLOCK_RESTORING_NETWORK_SELECTION

/*
   ���� �ʱ� socket open fail�ǰ� 4�ʰ������� ��õ��ϴ� ���� 1�� �������� 8ȸ �ݺ��ϵ��� ����.
   PIN lock ȭ�鿡 ���ݴ� ���� ���Եǵ��� �ϱ� ����. 
   ��Ÿ ����....
*/
/*
#define FEATURE_SKY_CP_RIL_SOCKET_OPEN_RETRY_MILLIS
*/

/*
    Illegal ME reject cause �� Ư�� reject cause���� �� gsm.sim.operator.numeric property�� 
    null �� �ʱ�ȭ �Ǵ� ���� ����.
*/
//#define FEATURE_SKY_USIM_ICC_OPERATOR_PROPERTY_NOT_RESET

/*
    USIM EF_CFIS ���� ������� Call Forwarding ���� indicator ǥ������ �ʵ��� ������.
*/
#define FEATURE_SKY_USIM_CALL_FORWARDING_EF_CFIS_NOT_SUPPORT

/*
    Config.xml �� csp_enable �� true�� �����Ǿ� �ִٸ� operator selection menu�� �߻����� �ʴ´�.
*/
//#define FEATURE_SKY_CP_DISABLE_CSP_FLAG

#define FEATURE_SKY_CP_DEBUGGING_LOG_FOR_TIME_SETTING

/*
     ���ȣ�� second call�� reject�� �� first call time �ʱ�ȭ�Ǵ� ���� ����
*/
#define FEATURE_SKY_CP_CALL_CONNECTION_TIME_BUG_FIX

#define FEATURE_SKT_CP_REJECT_USIM

#endif/* __CUST_SKY_CP_H__ */
