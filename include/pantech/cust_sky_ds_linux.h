#ifndef __CUST_SKY_DS_LINUX_H__
#define __CUST_SKY_DS_LINUX_H__

/*****************************************************
    SKY Android model common.
    Feature Name : T_SKY_MODEL_TARGET_COMMON
******************************************************/
#ifdef T_SKY_MODEL_TARGET_COMMON
#endif /* T_SKY_MODEL_TARGET_COMMON */


/* ######################################################################### */
/*****************************************************
    Feature Name : T_SKY_MODEL_TARGET_WCDMA
    Need to include this file in CUST_SKY.h
******************************************************/
#ifdef T_SKY_MODEL_TARGET_WCDMA
/************************************************************************************************************************
**    Related Data-connection
************************************************************************************************************************/

#define FEATURE_SKY_DS_ADD_DATA_AIDL

#define FEATURE_SKY_DS_FAST_DORMANCY

#define FEATURE_SKY_DS_BLOCK_JNI_SW_RESET

#define FEATURE_SKY_DS_IMS_ALWAYS_ENABLE

//MobileDataStateTracker.java
#define FEATURE_SKY_DS_NETWORKINFO_BUG_FIX

//ro.com.android.dataroaming changes the value from true to false
//LINUX\android\build\target\product\full_base_telephony.mk
#define FEATURE_SKY_DS_DEFAULT_DATA_ROMAING_SETTING

#define FEATURE_SKY_DS_CPMGR_PIPE_ENABLED

#define FEATURE_SKT_DS_LOST_PHONE_SRV_SUPPORT

#define FEATURE_SKY_DS_RMNET_INIT_COMPLETION

#define FEATURE_SKY_DS_COMMON_API

#define FEATURE_SKY_DS_QCCONNECTIVITY_SERVICE_ENABLE

#define FEATURE_SKY_DS_QOS_DISABLE

/*
 - INITIAL ATTACH TYPE이 존재 하는 경우에만 IA APN set
*/
#define FEATURE_SKY_DS_IAAPN_SET_IN_CASE_BEING_IA_TYPE

#define FEATURE_SKY_DS_DATA_REGISTRATION_QUERY_FAIL_RECOVERY

#define FEATURE_SKY_DS_NO_SERVICE_BUG_FIX

#define FEATURE_SKY_DS_RADIO_TECH_BUG_FIX

#define FEATURE_SKY_DS_FAST_CONNECT_AFTER_APN_FAIL_OR_CHANGE

#define FEATURE_SKY_DS_SETUP_DATA_RETRY_BUG_FIX

#define FEATURE_SKY_DS_IMMEDIATE_SETUP

#define FEATURE_SKY_DS_DISABLE_CHECK_MOBILE_PROVISIONING

#define FEATURE_SKY_DS_SUPPORT_IPV6_DEFAULT_DNS

#define FEATURE_SKY_DS_FIXED_CLEAR_HOST_EXECMPT

#define FEATURE_SKY_DS_CHECK_DNS_AAAA_TYPE_USING_IFACE

#define FEATURE_SKY_TURN_OFF_DATA_RECOVERY

#define FEATURE_SKY_DS_CLEAR_DEFAULT_IFACE_DNS

#define FEATURE_SKY_DS_BLUETOOTH_TETHERING_BUG_FIX

#define FEATURE_SKY_DS_CONNECTIVITY_CHANGE_DELAY_MODIFY

#define FEATURE_SKY_DS_USE_DUN_TYPE_FOR_TETHERING

#define FEATURE_SKY_DS_DEL_IP_RULE_IN_SND_TABLE

#define FEATURE_SKY_DS_SKIP_UPDATE_NETWORK_SETTING

#define FEATURE_SKY_DS_CLEAR_ROUTE_AND_EXEMPT

/************************************************************************************************************************
**    Network Policy
************************************************************************************************************************/ 

//NetworkPolicyManagerService.java 
#define FEATURE_SKY_DS_BACKGROUND_RESTRICT_BUG_FIX

#define FEATURE_SKY_DS_NET_OVERLIMIT_API // for MMS team

#define FEATURE_SKY_DS_IPV6_STATS_COUNTING_CLAT4

#define FEATURE_SKY_DS_BACKGROUND_RESTRICT_RETRY_FOR_EXCEPTION

/************************************************************************************************************************
**    MENU / UI
************************************************************************************************************************/ 

#define FEATURE_SKY_DS_PDP_REJECT_POPUP

//NetworkController.java because of ims apn type
#define FEATURE_SKY_DS_HIDE_DATA_ICON_WHEN_WIFI_CONNECTED

#define FEATURE_SKY_DS_ADD_APN_SETTING_HIDDEN_CODE
 
#define FEATURE_SKY_DS_PREVENT_EDIT_DEFAULT_APN
 
#define FEATURE_SKY_DS_RESOURCE
 
#define FEATURE_SKY_DS_EASY_SETTING

#define FEATURE_SKY_DS_AIRPLANEMODE_ICON_TIMING

#define FEATURE_SKY_DS_DISABLE_BEARER_IN_APN

#define FEATURE_SKY_DS_MODIFY_UPDATE_TX_RX_SUM

/************************************************************************************************************************
**    Tether / NAT
************************************************************************************************************************/ 

#define FEATURE_SKY_DS_TETHERING_SETDNSFORWARD_BUG_FIX

#define FEATURE_SKY_DS_WIFI_HOTSPOT_TETHER_TOGGLE_BUG_FIX

#define FEATURE_SKY_DS_TETHER_MSS

#define FEATURE_SKY_DS_ADD_IP_RULE_IN_SND_TABLE_FOR_WIFI_BT

/************************************************************************************************************************
**    CTS / PortBridge / DUN / Log / VPN / CNE
************************************************************************************************************************/
 
#define FEATURE_SKY_DS_ENABLE_NSRM

#ifndef FEATURE_SKY_DS_ENABLE_NSRM
#define FEATURE_SKY_DS_CNE_DISABLE
#endif

#define FEATURE_SKY_DS_NSRM_UPDATE

// vendor\qcom\proprietary\prebuilt_HY11\target\product\msm8974\system\etc\cne\NsrmConfiguration.xml
#define FEATURE_SKY_DS_NSRM_EXCLUSION_LIST

#define FEATURE_SKY_DS_CTS_LISTENING_PORT_TEST_FAIL_FIX

#define FEATURE_SKY_DS_DATAMANAGER

#define FEATURE_SKT_DS_SYNC_MANAGER
 
#define FEATURE_SKY_DS_ATFWD_PROCESS
 
#define FEATURE_SKY_DS_CTS_ROOT_PROCESS

#define FEATURE_SKY_DS_VPN_FIX
 
#define FEATURE_SKY_DS_CTS_LOCALHOST

#define FEATURE_SKY_DS_MODIFY_ATFWD_DELAY

#define FEATURE_SKY_DS_ATFWD_USER_BUILD

#define FEATURE_SKY_DS_ANDROID_SECURITY_PATCH_12937545

#define FEATURE_SKY_DS_SET_IP_RULE_FOR_VPN

#define FEATURE_SKY_DS_SECURITY_PATCH_CVE_2014_0224

#define FEATURE_SKY_DS_FIX_CTS_FAIL_FOR_SSLSOCKET

/************************************************************************************************************************
**    Workaround
************************************************************************************************************************/ 

#define FEATURE_SKY_DS_BLOCK_JNI_SW_RESET

#define FEATURE_SKY_DS_ADD_DATA_LOG

#define FEATURE_SKY_DS_MDM_REJ_IP_PS3

#define FEATURE_SKY_DS_MDM_DATA_ON_OFF_PS3

#define FEATURE_SKY_DS_MDM_TETHER_ON_OFF_PS3

#define FEATURE_SKY_DS_464XLAT_SUPPORT

#define FEATURE_SKY_DS_EXCEPTION_BUG_FIX

#define FEATURE_SKY_DS_DNS_IFACE_BIND

#define FEATURE_SKY_DS_ADD_TCPDUMP_IN_USER_MODE

#define FEATURE_SKY_DS_INESSENTIAL_ALARM_MANAGER

#define FEATURE_SKY_DS_WDS_INIT_FOR_SSR_CR634504

#define FEATURE_SKY_DS_DROP_SSDP_FOR_CLAT4

#define FEATURE_SKY_DS_PREVENT_FOR_CONCURRENT_MODIFICATION_EXCEPTION

#define FEATURE_SKY_DS_AOVIDING_RESOURCE_LEAKS

#define FEATURE_SKY_DS_CCD_CONNECTIVITY

//KK removes checksum parts in TelephonyProvider.java and restore JB codes to update apns-conf.xml.
#define FEATURE_SKY_DS_TELEPHONY_DB_UPDATE_APNS_CONF_XML

#endif/* T_SKY_MODEL_TARGET_WCDMA */

/************************************************************************************************************************
**    Kernel configs / System properties (in defconfig files, system.prop)
************************************************************************************************************************/ 

#define FEATURE_SKY_DS_KERNEL_CONFIG

#ifdef FEATURE_SKY_DS_KERNEL_CONFIG

//kernel/drivers/usb/gadget/rndis.c
#define CONFIG_SKY_DS_BAM_ADAPTIVE_TIMER_OFF

//kernel\arch\arm\mach-msm\bam_dmux.c
#define CONFIG_SKY_DS_CHANGE_RNDIS_MTU

// nf_conntrack_sip.c
#define CONFIG_SKY_DS_ACCEPT_SIP_LAGRE_PACKET

// tcp_input.c
#define CONFIG_SKY_DS_TCP_INIFINITE_LOOP_BUG_FIX

// cts ping test fail for ipv6
#define CONFIG_SKY_DS_CTS_ICMPV6_ECHO_REQUEST

//\kernel\net\ipv4\ping.c
#define CONFIG_SKY_DS_KERNEL_CRASH_DUT_TO_NULL_POINTER

//\kernel\net\ipv6\addrconf.c
#define CONFIG_SKY_DS_OPTIMIZE_IPV6_ASSIGNMENT

//\kernel\net\ipv6\addrconf.c
#define SKY_DS_OPTIMIZE_IPV6_ASSIGNMENT_CRASH_FIX

//\kernel\net\ipv4\ping.c
#define CONFIG_SKY_DS_ANDROID_SECURITY_PATCH_CVE_2014_2851

//\kernel\net\l2tp\l2tp_ppp.c
#define CONFIG_SKY_DS_ANDROID_SECURITY_PATCH_CVE_2014_4943

#endif /* FEATURE_SKY_DS_KERNEL_CONFIG */

/* ######################################################################### */


/*****************************************************
    SKT Model common.
    Feature Name : T_SKY_MODEL_TARGET_SKT
******************************************************/
#ifdef T_SKY_MODEL_TARGET_SKT
#ifdef T_SKY_MODEL_TARGET_KT
#error Occured !!  This section is SKT only !!
#endif

/************************************************************************************************************************
**    Data-connection
************************************************************************************************************************/
 
#define FEATURE_SKT_DS_DATA_CONNECTION_API 

#define FEATURE_SKT_DS_ADD_DATA_SUSPEND_FUNC
 
#define FEATURE_SKT_DS_ROAMING_APN_CHANGE
 
#define FEATURE_SKT_DS_VOICE_CALL_PROTECTION_IN_CS_CALL
 
#define FEATURE_SKT_DS_LTE_TCP_BUFFER_CHANGE
 
#define FEATURE_SKT_DS_CSFB_REJECT_IN_VT

#define FEATURE_SKT_DS_CHECK_SKT_SIM

#define FEATURE_SKT_DS_SUPPORT_VOLTE

// moved init.EF60S.rc to init.qcom.rc 
#define FEATURE_SKT_DS_RESTORE_DEFAULT_APN
// use config_dontPreferApn as a true in config.xml

#define FEATURE_SKT_DS_DONT_USE_PREFERAPN

#define FEATURE_SKT_DS_COMMON_CM_PH_EVENT

#ifdef FEATURE_SKT_DS_COMMON_CM_PH_EVENT
#define FEATURE_SKT_DS_LTE_MO_DATA_BARRING_NOTIFICATION
#define FEATURE_SKT_DS_ROAMING_STATUS_CHANGED_FIRST_TIME_NOTIFICATION
#endif

// added frameworks\base\core\res\res\values-mcc450-mnc11\config.xml
#define FEATURE_SKT_DS_ADD_KCT_MTU

/************************************************************************************************************************
**    2. Related MENU / UI
************************************************************************************************************************/
/*====================== (1) Related Data-connection UI ======================*/
 
#define FEATURE_SKT_DS_ADD_ALWAYSON_MENU

#define FEATURE_SKT_DS_ADD_3G_DATA_POPUP
 
#define FEATURE_SKT_DS_NO_CONNECTION_POPUP_IN_NOIMEI
 
#define FEATURE_SKT_DS_SW_RESET_RELEASE_MODE_NO_DATA_POPUP

#define FEATURE_SKT_DS_MAKE_PDP_CONNETION_TOAST
 
#define FEATURE_SKT_DS_PS_REJECT

#define FEATURE_SKT_DS_SUPPORT_LTE_B2B

//config.xml
#define FEATURE_SKT_DS_IMS_DATA_USAGE

/*====================== (2) Related roaming ======================*/
 
#define FEATURE_SKT_DS_ROAMING

#define FEATURE_SKT_DS_LTE_ROAMING_MODE

/*====================== (3) Related application ====================== */
 
#define FEATURE_SKT_DS_SKAF_SUPPORT
 
#define FEATURE_SKT_DS_ALLOW_MMS_IN_DATA_DISABLE

#define FEATURE_SKT_DS_SUPPORT_BACKGROUND_TRFFIC_CONTROL_AGENT 

/*====================== (4) etc ======================*/
 
#define FEATURE_SKT_DS_HSUPA

#define FEATURE_SKT_DS_GET_IP_ADDRESS

#define FEATURE_SKY_DS_RESTART_TETHER_AFTER_MIRACAST_OFF

#define FEATURE_SKY_DS_DEFAULT_DATA_USAGE_WARNING_SIZE_MODIFY

/************************************************************************************************************************
**     3. CTS / PortBridge / DUN / Log /vpn
************************************************************************************************************************/


/************************************************************************************************************************
**     4. VT
************************************************************************************************************************/


/************************************************************************************************************************
**     5. VoIP(SKT VoIP/TAPS)
************************************************************************************************************************/
 
//#define FEATURE_SKT_DS_TAPS
 
//#define FEATURE_SKY_DS_SKT_VOIP_BLOCK_WIFI_CUT_DOWN
 
//#define FEATURE_SKY_DS_SKT_VOIP_USER_POWER_OFF_HANDLE
 
//#define FEATURE_SKY_DS_SKT_VOIP_USER_AIRPLANE_MODE_HANDLE
 
//#define FEATURE_SKT_DS_VOIP_DEBUG_SCREEN
 
//#define FEATURE_SKT_DS_VOIP_HIDDEN_MENU
 
//#define FEATURE_SKT_DS_VOIP_MANUAL_CFG
 
//#define FEATURE_SKY_DS_BLOCK_GB_BUILTIN_SIP
 
//#define FEATURE_SKY_DS_APPLY_VOIP_PROVIDER
 
//#define FEATURE_SKY_DS_BLOCK_VOIP

/************************************************************************************************************************
**     6. GingerBread SIP
************************************************************************************************************************/
 
//#define FEATURE_SKT_DS_SIP_PORT_DEL
 
//#define FEATURE_SKT_DS_SIP_REGI_REFRESH

/************************************************************************************************************************
**     7. Deleted Feature
************************************************************************************************************************/

//FC version resolved this feature.
// replaced (FEATRUE_SKY_SET_PREFERAPN_BUG_FIX_ICS_4_4)
//#define FEATRUE_SKY_SET_PREFERAPN_BUG_FIX
//#define FEATRUE_SKY_SET_PREFERAPN_BUG_FIX_ICS_4_4

//FC version resolved this feature.
//#define FEATURE_SKY_DS_SET_TETHERED_DNS

//#define FEATURE_SKT_DS_ADD_ROUTEREMOVE_FUNC

//#define FEATURE_SKT_DS_ALLOW_MMSONLY_APN_IN_DATA_DISABED

//Post FC version resolved this feature
//#define FEATURE_SKY_DS_SBA_1045_CR348377

//MobileNetworkSettings.java(packages\apps\phone\src\com\android\phone) is added in Settings.java(packages\apps\phone\src\com\android\phone) PS5
//#define FEATURE_SKY_DS_DATA_USAGE_MENU_BLOCKING

//ICS 4.0.4 resolved it
//#define FEATURE_SKY_DS_FOR_CTS_TEST

//pre-CS patch resolved it
//Tethering.java
//#define FEATURE_SKY_DS_EXCEPTION_CATCH_BUG_FIX

//Confirm this feature in the CS patch. 20120625
//#define FEATURE_SKY_DS_SBA_1045_CR350813

//CS patch resolved it. TelephonyProvider.java
//#define FEATURE_SKY_DS_CHECK_NULL_POINTER_EXCEPTION	//20120602

//CS patch resolved it.
//LINUX\android\bionic\libc\netbsd\resolv\res_init.c
//#define FEATURE_SKY_DS_GOOGLE_PATCH_CVE_2012_2808

//1033 patch resolved it
//#define FEATURE_SKY_DS_SYNC_CS_SERVICE_STATE //20120609

//bionic\libc\netbsd\resolv\res_cache.c
//JB resolved it, CONFIG_MAX_ENTRIES is 64->64*2*5
//#define FEATURE_SKT_DS_DNSMASQ_DNS_FORWARD

//frameworks\base\core\jni\android_net_TrafficStats.cpp
//JB resolved it.
//#define FEATURE_SKY_DS_CHANGE_MOBILE_IFACE_LIST

//android_filesystem_config.h
//#define FEATURE_SKY_DS_IP6TABLE_UID_BUG_FIX

//LINUX\android\kernel\net\ipv6\route.c
//JB resolved it.
//#define FEATURE_SKY_DS_GOOGLE_PATCH_CVE_2012_2811

//qcril_data_netctrl.c
//#define FEATURE_SKY_DS_EF44S_1530_PATCH_DATA_DISABLE_BUG_FIX_TEMP

//qcril_data_netctrl.c
//#define FEATURE_SKY_DS_FOUND_DATA_CALL_AFTER_PHONE_PROCESS_RESTART

//netmgr_main.c
//#define FEATURE_SKY_DS_CTS_SUID_CHECK

//NatController.cpp
//#define FEATURE_SKY_DS_TETHERING_DO_NOT_SEND_PRIVATE_IP

// ef56s, no need(no dhcp) #define FEATURE_SKY_DS_DHCP_DISCOVER_TIMER

// replaced (FEATURE_SKY_DS_ICON_NO_SRV_CR347576)
//#define FEATURE_SKY_DS_AIRPLANEMODE_BUG_FIX   // about to remove after 1046 patch
//temp_block #define FEATURE_SKY_DS_ICON_NO_SRV_CR347576

// ef56s, no need (msm, need check) #define FEATURE_SKY_DS_SKIP_3GPP2_PROFILE_PROCESS

// ef56s, no need #define FEATURE_SKY_DS_FIX_RADIO_TECH_UNKOWN_IN_CS

// ef56s, no need(need check) #define FEATURE_SKY_DS_SET_TCPBUF_IN_RAT_CHANGE

// ef56s, no need #define FEATURE_SKY_DS_ICS_IPTYPE_QC_ERROR_FIX

// ef52s, no code (need check history) #define FEATURE_SKY_DS_RETRY_TIMER_RESET_BUG_FIX

//NetworkPolicyManagerService.java
// e56s, temp block, porting later #define FEATURE_SKY_DS_RESTRICT_EXCEPT_FOR_IMS

//ConnectivityService.java for stability test from EF49K
// kk, no need #define FEATURE_SKY_DS_CONCURRENTMODIFICATIONEXCEPTION_BUG

// e56s, temp block, after test #define FEATURE_SKY_DS_TETHERED_MODE_BUG_FIX

// ef56s, no need(no dhcp) #define FEATURE_SKY_DS_IFC_TIMING_FIX

//#define FEATURE_SKY_DS_LTE_ROAMING_MODE

//no need #define FEATURE_SKY_DS_FIX_ROAM_CHECK_UI_BUG

//#define FEATURE_SKY_DS_SHOW_DATA_ICON_DURING_WIFI_CONNECTED  // US3 role

//ef56s, no need (alreay defined TUN) #define FEATURE_SKY_DS_SUPPOR_TUN

//ef56s, no need (after Test) #define FEATURE_SKY_DS_REMOVE_DBG_RMNET

//temp_block #define FEATURE_SKT_DS_RESUME_DOWNLOAD_FOR_WIFI_TO_DATA_CHANGE

//ef56s, change name to FEATURE_SKT_DS_DATA_CONNECTION_API #define FEATURE_SKT_DS_CHANGE_DATA_ONOFF

//ef56s, no code #define FEATURE_SKT_DS_ALWAYSON_MENU_DISABLED_IN_GPRS

//#define FEATURE_SKT_DS_LTE_B3_ROAMING (feature name changed) -> #define FEATURE_SKT_DS_LTE_ROAMING_MODE

//#define FEATURE_SKT_DS_CALL_TYPE_FOR_HD_SERVICE

//kk, no need #define FEATURE_SKY_DS_BUG_FIX_STARTUSINGNETWORKFEATURE

// no need #define FEATURE_SKT_DS_TETHERING_HOTSPOT_TOGGLE_TEMP

// no need #define FEATURE_SKY_DS_MOBILE_REJECT_SSDP_PACKET

// no need #define FEATURE_SKY_DS_BUILD_CNEAPI_ON_USER

// no need #define FEATURE_SKY_DS_BUSYBOX_INSTALL // (vendor_pantech_debugApps)

// no need #define FEATURE_SKY_DS_UNAVAILABLE_DEFERRED_MSG

// aready fixed..
//#define FEATURE_SKY_DS_GOOGLE_PATCH

// aready fixed..
//#define FEATURE_SKY_DS_KERNEL_CRASH_CR01155420_FROM_OSCAR_JB //ef56s, google fixed, for code history

#endif/* T_SKY_MODEL_TARGET_SKT */

 
#endif /* __CUST_SKY_DS_LINUX_H__ */
