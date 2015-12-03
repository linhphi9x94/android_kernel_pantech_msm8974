#ifndef __CUST_PANTECH_DATA_LINUX_H__
#define __CUST_PANTECH_DATA_LINUX_H__
/* ========================================================================
FILE: cust_pantech_data_linux.h

Copyright (c) 2011 by PANTECH Incorporated.  All Rights Reserved.

USE the format "FEATURE_LGU_DS_XXXX"
=========================================================================== */

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  when        	who     what, where, why
--------   	---      ----------------------------------------------------------
11/09/30    sjm   	  initial
11/12/22    phi   	  move to EF46L from EF65L. 
11/12/30    Alice     delete some of feature not yet adapted.
12/01/13    Alice     change file name. : Cust_pantech_data_linux.h -> CUST_PANTECH_DS.h
12/05/02    kns       move to EF50L from EF46L
12/10/26    Alice     re-arrange features
13/04/08    sjm      move to EF59L from EF52L
14/01/03    sjm      replace LGT to LGU
                           change file name. : CUST_PANTECH_DS.h -> Cust_pantech_data_linux.h 
===========================================================================*/

/*===========================================================================*/
  //4!!ATTENTION!!
/*===========================================================================*/
/*------------------------------------------------------------------------------------

  1. must record history in this file header when you modify this file.

  2. FEEATUR's name start with "FEATURE_LGU_DS_xxx".
  
  3. each FEATURE is need to detailed description. because this file is instad of Feature Specification.
        - Item, Comment(Date, Author), Reason, Modified Files, Added Files, Deleted Files.

  4. In Java Code, Feature' exprssion is comment.
        - Exmaple. // FEATURE_LGU_DS_COMMON
        
  5. this file must be included CUST_PANTECH.h

--------------------------------------------------------------------------------------*/

/*===========================================================================
    Data Service Features
===========================================================================*/

/* 20120105 Alice : Common import, include.. etc. */
#define FEATURE_LGU_DS_COMMON

/* Item : Kernel CONFIG
   Commnet - 20140211 Alice(P15279)
	Reason - added kernel config options

	Modified files - Kconfig (kernel\arch\arm)
*/
#define CONFIG_LGU_DS_K_CONFIG

#ifdef FEATURE_LGU_DS_COMMON

/* -----------------------------------------------------------------------------------*/
    //3 Android & QCT Bug Fix
/*-------------------------------------------------------------------------------------*/

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : TCP buffer
   Commnet - 20130211 Seo Inyong(P14997)
	Reason - To avoid infinite loop on recvmsg by tcp: reduce out_of_order_memory use.
	          
	Modified files - tcp_input.c(kernel\net\ipv4)
*/
#define CONFIG_LGU_DS_TCP_INIFINITE_LOOP_BUG_FIX

/* Item : PING
   Commnet - 20140226 Alice(P15279)
	Reason - ipv6: Introduce ip6_flowinfo() to extract flowinfo (tclass + flowlabel).

	Modified files - ipv6.h (kernel\include\net)
*/
#define CONFIG_LGU_DS_IP6_FLOWINFO

#ifdef CONFIG_LGU_DS_IP6_FLOWINFO
/* Item : PING
   Commnet - 20140226 Alice(P15279)
	Reason - ping: prevent NULL pointer dereference on write to msg_name
	          - A plain read() on a socket does set msg->msg_name to NULL. So check for NULL pointer first.
	          
	Modified files - ping.c (kernel\net\ipv4)
*/
#define CONFIG_LGU_DS_PREVENT_NULL_MSG_NAME
#endif /* CONFIG_LGU_DS_IP6_FLOWINFO */

/* Item : PING
   Comment - 20140602 BKY(P12534)
	Reason 
		- Android Partner Security Bulletin 2014-06-02
	    - ping: Flaw in ping_init_sock() function leading to possible refcounter overflow
	    - Integer overflow in the ping_init_sock function in net/ipv4/ping.c 
	       in the Linux kernel through 3.14.1 allows local users to cause a denial of service
	       (use-after-free and system crash) or possibly gain privileges via a crafted application
	       that leverages an improperly managed reference counter.
	       https://git.kernel.org/cgit/linux/kernel/git/davem/net.git/commit/?id=b04c46190219a4f845e46a459e3102137b7f6cac

	Android Partner Security Bulletin 2014-06
		Flaw in ping_init_sock() function leading to possible refcounter overflow
		ID: CVE-2014-2851
		Versions Affected: Linux 3.14.1 and earlier
		Severity: High
		Publicly known: Yes

		Integer overflow in the ping_init_sock function in net/ipv4/ping.c in the Linux kernel through 3.14.1 allows local users to cause a denial of service (use-after-free and system crash) or possibly gain privileges via a crafted application that leverages an improperly managed reference counter.

		Please apply the following patch to all Android kernels:
		https://git.kernel.org/cgit/linux/kernel/git/davem/net.git/commit/?id=b04c46190219a4f845e46a459e3102137b7f6cac

	Modified files - ping.c (kernel\net\ipv4)
*/
//20140728_yunsik_DATA, modified as patch and renamed
//#define CONFIG_LGU_DS_RETURN_CURRENT_GROUP_ID
#define CONFIG_LGU_DS_SECURITY_PATCH_CVE_2014_2851

#endif /* CONFIG_LGU_DS_K_CONFIG */

/* Item : startusingnetworkfeature()
   Commnet - 20130219 Alice(P15279)
	Reason - do not set IDLE when already try to set up data call.
	          
	Modified files - MobileDataStateTracker.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGU_DS_BUG_FIX_STARTUSINGNETWORKFEATURE

/* Item : Settings >> Data Usage
 	Commnet - 20130522 Alice(P15279)
  	Reason - set to Data Usage >> Set mobile data limit >> Restrict background data,
                UID/SYS_UID's setting is processed one by one as android's source architetecture.
                on the way setting if user request App., occur ANR(Application Not Responsding).
  	           - merge from EF56S
 	Modified files - NetworkPolicyManagerService.java(frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_LGU_DS_BACKGROUND_RESTRICT_BUG_FIX

/* Item : IPTABLEV6
 	Commnet - 20120224 phi
  	Reason - /system/bin/iptables's permission is set to system permission. 
  	             on the other hands ip6tables's permission is set to shell permission.
  	             and change ip6tables's permission to system permission.
  	          - merge from EF45K
 	Modified files - android_filesystem_config.h (system\core\include\private)

 	-unnecessary feature
*/
#undef FEATURE_LGU_DS_IP6TABLE_UID_BUG_FIX

/* Item : Exception
 	Commnet - 20121217 Alice(P15279), 20140422_yunsik_DATA
  	Reason - silent reset
  	                silent reset : MobileDataStateTracker (EF63L PLM#02383)
  	          
 	Modified files 
        - NetworkPolicyManagerService.java (frameworks\base\services\java\com\android\server\net)
        - MobileDataStateTracker.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGU_DS_EXCEPTION_CATCH_BUG_FIX

/* Item : TCP Buffer
 	Commnet - 20121113 Alice(P15279)
  	Reason - It doesn't exist TCP buffer size about EHRPD. 
  	             TCP Buffer size is choosen default size when connected to data in EHRPD.
  	             It's derived to change radio technology from EHRPD to LTE. because of keeping
  	             in Defualt TCP Buffer size.
  	          - modify to change TCP Buffer size

 	Modified files - MobileDataStateTracker.java (frameworks\base\core\java\android\net)
*/
#undef FEATURE_LGU_DS_SET_TCPBUF_IN_RAT_CHANGE

/* Item : Data Connection
   Commnet - 20120726 kns
  	Reason - After anr or kill phone process, data connection isn't established.
  	          - From EF49K.
  Modified files - Qcril_data_netctrl.c(vendor\qcom\proprietary\qcril\common\data)
*/
#define FEATURE_LGU_DS_FOUND_DATA_CALL_AFTER_PHONE_PROCESS_RESTART

/* Item : UI
	Commnet - 20130225 Alice(P15279)
	Reason - remove afterimage of popup window about Mobile data disabled
	Modified files - NetworkOverLimitActivity.java (frameworks\base\packages\SystemUI\src\com\android\systemui\net)
*/
#define FEATURE_LGU_DS_REMOVE_AFTERIMAGE

/* Item : QOS
    Comment : 20130710 SJM
    Reason - Crash RilD when incoming call
               - Disalbe QOS

    Modifiled files - System.prop(device\qcom\msm8974)
*/               
#define FEATURE_LGU_DS_QOS_DISABLE

/* Item : Booting
    Comment : 20140204 SJM
    Reason - Too many notifyDataConnection Intent before boot completed

    Modifiled files - ApnContext.java(frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                        - Dctracker.java(frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                        - SkyDctracker.java(frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_DISCARD_NOTIFYDATACONNECTION

/* Item : Connectivity VDBG
    Comment : 20140212 SJM,  20140212_yunsik_DATA
    Reason : If VDBG is true, DEFAULT_SAMPLING_INTERVAL_IN_SECONDS is 30s. So wake up every 30s.
                   defined VVDBG.
    Modified files - ConnectivityService.java (frameworks\base\core\java\android\server)
*/
#define FEATURE_LGU_DS_ENABLE_VDBG_CONNECTIVITYSERVICE

/* Item : Retry Counter
    Comment : 20140214 SJM
    Reason : Prevent reset Retry count due to startUsingNetworkFeature when DC is retrying state.
    Modified files -QcConnectivityService.java (frameworks\opt\connectivity\services\java),
					    ConnectivityService.java (frameworks\base\core\java\android\server),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection), 
*/                         
#define FEATURE_LGU_DS_IGNORE_STARTUSINGNETWORKFEATURE_DURING_RETRYING

/* Item : VPN security patch
    Comment : 20140305 DGKim
    Reason : To make exempt route for system app. need to investigation more
    Modified files - ContextImpl.java (frameworks\base\core\java\android\app)
    				ConnectivityManager.java (frameworks\base\core\java\android\net)	
    				IConnectivityManager.aidl (frameworks\base\core\java\android\net)	
   				ConnectivityService.java (frameworks\base\services\java\com\android\server)
                          Vpn.java (frameworks\base\services\java\com\android\server\connectivity)
				QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/                         
#define FEATURE_LGU_DS_VPN_PATCH_12937545

/* Item : Memory leaks
	Commnet - 20140613 Alice(P15279)
	Reason
        - merge from FEATURE_SKY_DS_AOVIDING_RESOURCE_LEAKS
        - close cusor when exit method 
        
	Modified files
        - TelephonyProvider.java (packages\providers\telephonyprovider\src\com\android\providers\telephony)
*/
#define FEATURE_LGU_DS_AOVIDING_RESOURCE_LEAKS

/* Item : Tethering and Hotspot
    Comment : 20140306 sjm
    Reason : Tethering failed due to broken pipe in applyDnsInterfaces().
	Modified Files - network.c(external\dnsmasq\src)
*/
#define FEATURE_LGU_DS_BROKEN_PIPE

/* Item : Modification by Klockwork report
    Comment : 20140402_yunsik_DATA, BKY
    				EF63L_KK Klockwork modification
    				Even modified Qualcomm's original code.

    				PLM -> PDM -> Managing Quality -> SW -> KlockWork search
    
	Modified files
		Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi) ( Issue#71262 )
*/
#define FEATURE_LGU_DS_KLOCKWORK_MODIFICATION_EF63L_KK

/* Item : Android Partner Security Bulletin 2014-07
    Comment : 20140610_yunsik_DATA
                SSL/TLS MITM vulnerability.
                ID: CVE-2014-0224
                Versions Affected: Android version 4.4 and below
                Severity: High
                Publicly known: Yes
    
	Modified files
        - S3_clnt.c (external\openssl\ssl)
        - S3_pkt.c (external\openssl\ssl)
        - Ssl.h (external\openssl\include\openssl)
        - S3_srvr.c (external\openssl\ssl)
        - Ssl.h (external\openssl\ssl)
        - Ssl3.h (external\openssl\include\openssl)
        - Ssl3.h (external\openssl\ssl)
        - Ssl_err.c (external\openssl\ssl)
        - openssl.config (external\openssl)

	Added files
        - early_ccs.patch (external\openssl\patches)
*/
#define FEATURE_LGU_DS_SECURITY_PATCH_CVE_2014_0224

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : net/l2tp: don't fall back on UDP [get|set]sockopt
    Comment - 20140728_yunsik_DATA
	Reason 
        - Android Partner Security Bulletin 2014 #09
        - net/l2tp: don't fall back on UDP [get|set]sockopt
        - Arbitrary kernel code execution via PPPOL2TP
           ID: CVE-2014-4943
           Versions Affected: Linux version 3.15.6 and below
           Severity: High
           Publicly known: Yes
 
           The PPPoL2TP feature in net/l2tp/l2tp_ppp.c in the Linux kernel through 3.15.6 allows local users to gain privileges by leveraging data-structure differences between an l2tp socket and an inet socket.
 
           Please apply the following patch to all Android kernels: 
           https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=3cf521f7dc87c031617fd47e4b7aa2593c2f3daf

	Modified files 
        - L2tp_ppp.c (kernel\net\l2tp)
*/
#define CONFIG_LGU_DS_SECURITY_PATCH_CVE_2014_4943
#endif /* CONFIG_LGU_DS_K_CONFIG */

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement
/*-------------------------------------------------------------------------------------*/

/* Item : CNE
 	Commnet - 20130513 Alice(P15279)
 	Reason - Connectivity Engine = 4
   Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGU_DS_CNE

/* Item : DHCP
	Commnet - 20120227 Alice(P15279)
	Reason - Since default DHCP Lease Time is 1 hour, 
	             VPN, RMNET and android Tethering are disconnected frequently. 
	             So, Increase the DHCP Lease Time to 7 days
	Modified files - TetherController.cpp (system\netd) 
*/
#define FEATURE_LGU_DS_INCREASE_DHCP_LEASETIME

/*Item : RestoreTimer
   Commnet - 20120618 Alice(P15279)
   Reason - not used RestoreTimer : DUN, IMS
              - and HIPRI : Spec Out - but MQS Issue.
              - deleted FEATURE_LGT_DS_DISABLE_INACTIVITY_TIMER
              - concerned FEATURE_LGU_DS_DEFAULT_APN

              - HIPRI : 5 min >> 10min
              
   Modified files - Config.xml (device\qcom\common\overlay\frameworks\base\core\res\res\values)
*/
#define FEATURE_LGU_DS_DISABLE_RESTORE_TIMER

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : TCP sync retries
	Commnet - 20120104 Alice(P15279)
	Reason - change RTO and TCP_SYN_RETRIES when TCP syn transfer
	           - tcp resync nymber is 5.(first transfer, 1s, 2s, 4s, 8s)
	Modified files - tcp.h(kernel\include\net)
*/
#define CONFIG_LGU_DS_TCP_SYN_RETRANSMIT
#endif /* CONFIG_LGU_DS_K_CONFIG */

/* Item : Connectivity
 	Commnet - 20140109 Alice(P15279), sjm
	Reason - Check Data onoff in startUsingNetworkFeature()
              - replaced of FEATURE_LGT_DS_WPS_CHECK_DATA_ONOFF in EF50L ICS.
              - for CTS : android.net.cts.ConnectivityManagerTest
 
   Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),

  Commnet - 20140317 Alice(P15279)
  Reason - do not try to fail over when Bluetooth NAP
             - Mobile Data Icon's representaion is error. when try to connect Wi-Fi during Bluetooth
                it occur repeat connection and disconnection.
             - it doesn't working Wi-Fi connection during Bluetooth Tethering as network attributes's priority.

  Modified files - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_RESTRICT_DATA_CALL

/* Item : APN Changed
 	Commnet - 20120720_phi_DATA , 20140121 Alice(P15279)
  	Reason - To notify ESM Error Cause in Deactivate EPS bearer context request msg to android Data Framework
                    
 	Modified files - DcController.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
       	              Ril.cpp (hardware\ril\libril),
       	              Ril.h (hardware\ril\include\telephony),
                        RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                        RILConstants.java (frameworks\base\telephony\java\com\android\internal\telephony),
                        Ril_unsol_commands.h (hardware\ril\libril),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/    
#define FEATURE_LGU_DS_NOTIFY_DEACT_BEARER_ERR_CAUSE_TO_FRAMEWORK

/*Item : Data Usage - Warning Level
   Commnet - 20130711 Alice(P15279)
   Reason - changed data usage warning level : 2G -> 6G
              
   Modified files - Config.xml(frameworks\base\core\res\res\values)
*/
#define FEATURE_LGU_DS_NW_POLICY_WARNING_6G

/* Item : Handset Property
	Commnet - 20130813 Alice(P15279)
	Reason - changed rmnet interface name
	
	Modified files - HandsetProperty.java (frameworks\base\core\java\android\lgt\handset)
*/
#define FEATURE_LGU_DS_RMNET_INTERFACE_NAME

/* Item : Using Modem Mtu for IPv6 addr
 	Commnet - 201402018 BKY DATA
  	Reason - From KK, the mtu from config.xml corresponding to mcc/mnc is used for setting mtu when each iface is connected.
  					But, we should use mtu from Router Advertisement msg.
  					Therefore we set this mtu of RA to LinkProperties for IPv6 addr using InterfaceController.cpp thru Netd.
                    
 	Modified files - NetworkManagementService.java (frameworks\base\services\java\com\android\server),
 		                    MobileDataStateTracker.java (frameworks\base\core\java\android\net),
 		                    INetworkManagementService.aidl (frameworks\base\core\java\android\os),
*/    
#define FEATURE_LGU_DS_USING_MODEM_MTU_FOR_IPV6_ADDR

/*......................................................................................................................................
  EasySetting, Data On/Off
.........................................................................................................................................*/

/* Item : Data On/off Property
	Commnet - 20121022 Alice(P15279)
	Reason - Manage LGU+'s customized Data on/off property
	          - replaced of FEATURE_LGT_DS_GET_SECUREDB_FOR_LGT

	Modified files - DatabaseHelper.java (frameworks\base\packages\settingsprovider\src\com\android\providers\settings),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        Settings.java (msm8974_kk\frameworks\base\core\java\android\provider),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ADD_SECURE_DB

/* Item : EasySetting
 	Commnet - 20120509 Alice(P15279)
 	Reason - match setting value between EasySetting ans Setting
 	
 	Modified files - ConnectivityManager.java (frameworks\base\core\java\android\net),
                        ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        IConnectivityManager.aidl (frameworks\base\core\java\android\net),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_EASY_SETTING

/* Item : DATA ALLOWED POPUP
 	Commnet - 20140109 Alice(P15279)
 	
 	Reason - display data connection pop-up for user's choice when booting
 	          - to display only once. even silent reset
 	          - added theme(AlertDialog.THEME_DEVICE_DEFAULT_DARK), EF63L(LIGHT to DARK)
 	          - added condition when return to domestic 
 	             concerned to AutoRadReceiver.java(packages\apps\Phone\src\com\android\phone)
             - replaced of FEATURE_LGU_DS_BOOTING_POPUP
 	
 	Modified files - CDMAPhone.java (frameworks\base\telephony\java\com\android\internal\telephony\cdma),
 	                     DatabaseHelper.java (frameworks\base\packages\settingsprovider\src\com\android\providers\settings),
 	                     Intent.java (frameworks\base\core\java\android\content),
 	                     DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        public.xml (frameworks\base\core\res\res\values),
                        Settings.java (frameworks\base\core\java\android\provider),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        System.prop(device\qcom\msm8974)

	Added files - SkyDcDialog.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
	                  strings_ds.xml(frameworks\base\core\res\res\values),
	                  strings_ds.xml(frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_ALLOWED_DATA_POPUP

/* Item : Toast
 	Commnet - 20120517 Alice(P15279)
 	Reason - Show Data Connection and Disconnection.
 	
 	Modified files - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                        Intent.java (frameworks\base\core\java\android\content),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)

	Comment - 20140407 Seo Inyong(P14997)
	Reason - LG U+ Network & Indicator UI Mandatory v1.55
				page    / 	enum type in the codes			/ description
				13p.     /	TOAST_DATA_PROPERTY_FALSE  / data allowed/blocked toast after turn on without booting popup
				13,14p. /	TOAST_DATA_SETTING			/ data allowed/blocked toast after booting popup or setting change
				17p.     /	TOAST_DATA_CONCURRENT		/ mobile data is established during default data connection is not a mobile type
				19p.     /	TOAST_DATA_FAILOVER 		/ mobile data has been enabled and connected by disconnection of other types
			  Modify codes to simplifying.
			  Change the string resource names & rearrange feature in xml
			  Combine and delete FEATURE_LGU_DS_DATA_CONNECTION_TOAST
			  
	Modified files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
						public.xml (frameworks\base\core\res\res\values)
						strings_ds.xml(frameworks\base\core\res\res\values)
						strings_ds.xml(frameworks\base\core\res\res\values-ko)
						Strings_wifi_settings.xml (frameworks\base\core\res\res\values)
						Strings_wifi_settings.xml (frameworks\base\core\res\res\values-ko)

	Comment - 20140612_yunsik_DATA
	Reason - When wifi turned off, toast showed incorrectly.
	                removed airplane condition, added DataRadioTech. added mUserDataEnabledAll sync
	                WifiStateMachine.java modified -> wifi team
			  
	Modified files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_TOAST

/* Item : Toast pantech theme
 	Commnet -  20140409_yunsik_DATA
 	Reason - pantech theme applied to toast (EF63L)

 	Modified files
        - LGURoamingDataDialog.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_TOAST_PANTECH_THEME

/* Item : BIP
 	Commnet - 20130808 SJM
 	Reason - Do not allow data call using EMPTY SIM
 	
 	Modified files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_NOT_ALLOW_DATA_CALL_USING_EMPTY_SIM

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : IPv6 Address Asignment
     Comment - 20130809 SJM 
     Modified files - addrconf.c (kernel\net\ipv6)
*/
#define CONFIG_LGU_DS_OPTIMIZE_IPV6_ASSIGNMENT

#ifdef CONFIG_LGU_DS_OPTIMIZE_IPV6_ASSIGNMENT
/* Item : Prevent device from crashing
    Comment : 20131205 BKY
    Reason - Device crashed when optimizing IPv6 address assignment which means reducing assignment duration.
    				It's crashed because 'ifa->idev' is null at 'if6_get_first' function.
    				
	Related FEATURE - FEATURE_LGU_DS_OPTIMIZE_IPV6_ASSIGNMENT
				How to reduce?
			    	- set NO DAD for link local address
    				- shrink DAD timer to 50 ms for global address.
    				
    Solution - spin_lock and spin_unlock points are adjusted and adding.
    Related issues - EF59L_JB PLM 01708 / EF62L_JB PLM 00836

    Modifiled files - addrconf.c (kernel\net\ipv6)
*/               
#define CONFIG_LGU_DS_OPTIMIZE_IPV6_ASSIGNMENT_CRASH_FIX
#endif /* FEATURE_LGU_DS_OPTIMIZE_IPV6_ASSIGNMENT */
#endif /* CONFIG_LGU_DS_K_CONFIG */

/*......................................................................................................................................
  QMI
.........................................................................................................................................*/

/* Item : QMI
	Commnet - 20120426 Alice(P15279)
	Reason - Added for communication between Modem and Linux
	          - Just only use for Data Service.
	          - must be adpated to pair with Linux.
	
	Modified files - Android.mk (android\frameworks\opt\telephony),
	                    Core.mk (android\build\target\product)
	          
	Added files - LINUX\android\pantech\frameworks\qmi_data

   Commnet - 20120503 Alice(P15279)
   Reason - not only IDL QMI but also Legacy QMI
             -  use Legacy QMI, must block the compile option flags.
                (LOCAL_CFLAGS += -DQCCI_OVER_QMUX )
             - added permission for failure socket
             
   Modified files - Android.mk (vendor\qcom\proprietary\qmi-framework\qcci\src),
                        //ipc_socket.c(kernel\arch\arm\mach-msm)
*/
#define FEATURE_LGU_DS_QMI

/*......................................................................................................................................
  Hidden Menu : Enginerring, Debug screen, etc.
.........................................................................................................................................*/

/* Item : IP Addr, DNS Addr, PCSCF Addr
	Commnet - 20120527 kns, 20101026 Alice(P15279)
	Reason - display IP Addr in Debug Screen.
	Modified files - LteScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
	                        Lte_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
	                        Strings_cp.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\values)

	Comment - 20131017_yunsik_DATA, added for wcdma
    Modified files -    WcdmaScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
                           Wcdma_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
                           Strings_cp.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\values)

	Comment - 20131017_yunsik_DATA, added DNS address (LTE/WCDMA)
    Added files -    Strings_ds.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\values)
    Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
                            SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                            LteScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
                            Lte_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)
                            WcdmaScreen.java (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\src\com\android\hiddenmenu)
                            Wcdma_screen.xml (vendor\qcom\proprietary\pantech_ps_ril\gwlhiddenmenu\res\layout)

	Comment - 20140415 BKY, added Emergency IP address, Emergency DNS address, and IMS, Emergency PCSCF address.(LTE)
*/
#define FEATURE_LGU_DS_MULTIPDN_DBGSCN

/* Item : Data Service Hidden Menu
	Commnet - 20130724 Alice(P15279)
	Reason - added Hidden menu for Data Service
	
	Modified files - AndroidManifest.xml (packages\apps\settings),
                        SpecialCharSequenceMgr.java (packages\apps\dialer\src\com\android\dialer)

   Added files -    packages\apps\settings\src\com\android\settings\data\*
                        Sky_data_nsrm.xml (packages\apps\settings\res\xml),
                        Sky_data_service.xml (packages\apps\settings\res\xml),
                        Sky_data_settings.xml (packages\apps\settings\res\xml)
                        Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGU_DS_HIDDEN_MENU

#ifdef FEATURE_LGU_DS_HIDDEN_MENU
/* Item : Alwayson Setting
 	Commnet - 20120128 Alice(P15279)
  	Reason - added AlwasyOn/Off API
 	Modified files -  SkyDcService.java (packages\apps\settings\src\com\android\settings\data),
                         Sky_data_service.xml (packages\apps\settings\res\xml),
 	                      Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGU_DS_ON_OFF_TEST

#ifdef FEATURE_LGU_DS_QMI
/* Item : Equipment Test
	Commnet - 20130816 Alice(P15279)
	Reason - used LGU+ USIM in equipment network
	           - send QMI command for change Modem's configuration
              - disable dns check in equipment network
   Modified files -  QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                        QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                        Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                        Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                        SkyDcSettings.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_settings.xml (packages\apps\settings\res\xml),
                        Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGU_DS_EQUIPMENT_TEST

/* Item : ROHC
	Commnet - 20130820 Alice(P15279)
	Reason - ROHC ON/OFF

   Modified files - QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                        QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata)
                        Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                        Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                        SkyDcSettings.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_settings.xml (packages\apps\settings\res\xml),
                        Strings_ds.xml (packages\apps\settings\res\values)
*/
#define FEATURE_LGU_DS_ROHC_ONOFF

/* Item : APN
	Commnet - 20130820 Alice(P15279), 20130906 hongss
	Reason - change IP type at modem's profile

   Modified files - Qdp.c (vendor\qcom\proprietary\data\qdp\src), //hongss
                        SkyDcService.java (packages\apps\settings\src\com\android\settings\data),
                        Sky_data_service.xml (packages\apps\settings\res\xml),
                        Strings_ds.xml (packages\apps\settings\res\values) 
*/
#define FEATURE_LGU_DS_CHANGE_IP_TYPE

/* Item : IMS_TEST
	Commnet - 20140415 BKY(P12534)
	Reason - For ims test blocking pcscf and dns address(es)

   Modified files - QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
						QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
						Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
						Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
						SkyDcService.java (packages\apps\settings\src\com\android\settings\data),
						Sky_data_service.xml (packages\apps\settings\res\xml),
						Strings_ds.xml (packages\apps\settings\res\values),
*/
// Temporary holding now
//#define FEATURE_LGU_DS_IMS_TEST_BLOCK_PCSCF_DNS

#undef FEATURE_LGU_DS_DUN_MENU
#endif /* FEATURE_LGU_DS_QMI */

/* Item : DSA
    Comment -  20140411_yunsik_DATA
    Reason - Turn off DSA for equipment test (property : persist.pantech.ds.stall.off)
                    this property will be set if device entered to equipment test mode
    Modified Files 
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - SkyDcSettings.java (packages\apps\settings\src\com\android\settings\data)
        - property_service.c (system\core\init)
*/
#define FEATURE_LGU_DS_DSA_TURN_OFF_FOR_EQUIPMENT

#endif /* FEATURE_LGU_DS_HIDDEN_MENU */

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: Multiple APN
/*-------------------------------------------------------------------------------------*/

/* Item : APN List
	Commnet - 2012
	Reason - added multiple apn
	Modified files - Apns.xml (frameworks\base\core\res\res\xml),
                        Apns-conf.xml (vendor\qcom\proprietary\common\etc)
*/
#define FEATURE_LGU_DS_DEFAULT_APN

/* Item : tethering
	Commnet - 20140129 Alice(P15279)
	Reason - set TETHER_DUN_REQUIRED to 1
	           - 2 = not set, 0 = DUN not required, 1 = DUN required
	           - added NetworkStateTracker for BT tethering

	Modified files - Config.xml (device\qcom\common\overlay\frameworks\base\core\res\res\values),
  	                     Config.xml (frameworks\base\core\res\res\values),
                        ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        Tethering.java (frameworks\base\services\java\com\android\server\connectivity)

	!!!!!!!!!!!!!!!!!!!!!!!!!!!!-------- ATTENTION --------!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	For upgrading models like JB -> KK, please look at the below FEATURE_LGU_DS_TETHERING_APN_ONLY_UPGRADE

	!!!!!!!!!!!!!!!!!!!!!!!!!!!!-------- ATTENTION --------!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#define FEATURE_LGU_DS_TETHERING_APN

#ifdef FEATURE_LGU_DS_TETHERING_APN
/* Item : Tethering APN reset flag setting for upgrading models
	Commnet - 20140402 BKY (P12534)

   !ONLY JB -> KK ! So, this feature is blocked in this EF63L model.
   
   Reason - update telephony.db
             - if found out new Version, initalized telephony.db
	Modified files - TelephonyProvider.java (packages\providers\telephonyprovider\src\com\android\providers\telephony)
*/
//#define FEATURE_LGU_DS_TETHERING_APN_ONLY_UPGRADE
#endif /* FEATURE_LGU_DS_TETHERING_APN */

/* Item : EMERGENCY PDN
   Commnet - 20130515 sjm, 20140320 Alice(P15279)
   Reason - for Emergency PDN
              - used TYPE_MOBIE_CBS to connect Emergency PDN.
              - added EXEMPT type for VPN
              - do not added 30000 at  "ip rule"'s priority

   Modified files - Config.xml (frameworks\base\core\res\res\values),
                       RouteController.cpp (msm8974_kk\system\netd),
                       ConnectivityManager.java (frameworks\base\core\java\android\net),
                       SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_EMERGENCY_APN

#ifdef FEATURE_LGU_DS_EMERGENCY_APN
/* Item : IMS
	Commnet - 20120620 sjm, 20121029 Alice(P15279)
	Reason - Always on for IMS/MMS/EMERGENCY/PHOTORING
	          - replaced of FEATURE_LGT_DS_IMS_MMS_ALWAYS_ON
	          - replaced of FEATURE_LGT_DS_IMS_MMS_EMERGENCY_ALWAYS_ON 20131128 by sjm
	          
   Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ALWAYS_ON

/* Item : Data Usage
    Commnet - 20120816 sjm, 20140212 Alice(P15279)

    Reason - decide network types to count data usage
               - Except for IMS/MMS/EMERGENCY
               - include other mobile type
               - replaced of FEATURE_LGU_DS_IMS_DATA_USAGE

    Modified files -Config.xml (frameworks\base\core\res\res\values)
*/
#define FEATURE_LGU_DS_DATA_USAGE

/* Item : IMS/EMERGENCY
   Commnet - 20120912 sjm, 20121025 Alice(P15279), 20130912 sjm
   Reason - Do not retry data connection for ims

             - added EMERGENCY
             - replaced fo FEATURE_LGU_DS_IMS_RETRY

   Modified files - DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_IMS_MMS_EMERGENCY_RETRY

#ifdef FEATURE_LGU_DS_QMI
/* Item : Emergency Call Noti to Modem
    Comment - 20130801 SJM
    Reason - Attch with Emergnecy APN when dialing emergency call during airplane mode or not attached state
    
    Modified Files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                         ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                         QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                         QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                         QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                         Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                         Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_BIP_OTA_EMERGENCY_CALL

#ifdef FEATURE_LGU_DS_ALLOWED_DATA_POPUP
/* Item : Process ODB in Attach Reject
    Comment - 20130822 SJM
    Reason - Reject Cause set to 257
    
    Comment - 20140217 Alice(P15279)
    Reason - move alertDialog to SkyDcDialog 
    
    Modified Files - Intent.java(frameworks\base\core\java\android\content),
                         Public.xml (frameworks\base\core\res\res\values),
                         QmiDataClnt.cpp (pantech\frameworks\qmi_data\jni),
                         QmiDataClnt.java (pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                         Qmi_data_clnt.c (pantech\frameworks\qmi_data\qmi),
                         Qmi_data_def.h (pantech\frameworks\qmi_data\qmi),
                         RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                         SkyDcDialog.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         Strings_ds.xml (frameworks\base\core\res\res\values),
                         Strings_ds.xml (frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_ATTACH_REJECT_BY_ODB
#endif /* FEATURE_LGU_DS_ALLOWED_DATA_POPUP */
#endif /* FEATURE_LGU_DS_QMI */

/* Item : Emergency Call Reject Noti to Modem and App.
    Comment - 20130813 SJM
    Reason - mode change to WCDMA due to emergnecy pdn reject
               - Reject Cause set to 256
    
    Modified Files -  DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                          Intent.java(frameworks\base\core\java\android\content),
                          ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                          RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                          SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_EMERGENCY_CALL_REJECT
#endif /* FEATURE_LGU_DS_EMERGENCY_APN */

#ifdef FEATURE_LGU_DS_ALWAYS_ON
/* Item : IMS
  Commnet - 20120704 sjm
  Reason - In case IMS registration started before GsmDataConnectionTracker is not ready 
               (e.g. mAllApnList is not created yet) 
               so IMS apn request failed, GsmDataConnectionTracker should retry IMS apn request. 
               
   Modified files - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_IMS_APN_REQ_RETRY

/* Item : Data Icon
    Commnet - 20120816 sjm, 20130521 Alice(P15279),  20140217_yunsik_DATA, 20140303_yunsik_DATA, 20140404_yunsik_DATA
    Reason - 1. IMS/MMS/EMERGENCY/PHOTORING type doesn't display data activity icon.
                    2. KK version, method made and changed from getTxPackets,getRxPackets to getTcpTxPackets,getTcpRxPackets
                    3. add null point exception in TrafficStats.java
                    4. changed from getTcpTxPackets to getTxPackets (issue : Data arrow stopped even though there are UDP RX/TX packets)

    Modified files - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                        TrafficStats.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGU_DS_DATA_ACTIVITY_HIDE

/* Item : DATA
    Commnet - 20120313 sjm, 20121029 Alice(P15279)
    Reason - For hide data icon when alwayson apn only activated.
              - replaced of FEATURE_LGT_LTE_EHRPD_MULTIPLE_APN_IMS_DATA_ICON_VISIBLE
              - 
    Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                         NetworkController.java (frameworks\base\packages\systemui\src\com\android\systemui\statusbar\policy),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_DATA_ICON_HIDE
#endif /* FEATURE_LGU_DS_ALWAYS_ON */

/* Item : Retry algorithm
    Commnet - 20120712 sjm, 20121024 Alice(P15279)
    Reason - Req. for mpdn retry algoritm.
               
    Modified files - DcFailCause.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                         DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         Public.xml (frameworks\base\core\res\res\values),
                         Ril.h (hardware\ril\include\telephony),
                         SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                         Strings_ds.xml (frameworks\base\core\res\res\values),
                         Strings_ds.xml (frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_MPDN_RETRY_ALGORITHM

/* Item : HotSpot
    Commnet - 20120808 sjm, 20131113 Alice(P15279)
    Reason - Hotspot disabled when data off
              - added condition about data on/off in roaming
              - added condition about MPDN retry algorithm

    Modified files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_TETHERING_HOTSPOT_OFF

/* Item : Tethering
    Commnet - 20120719 sjm, 20130107 Alice(P15279)
    Reason - Req. for mdpn tethering
              - show Notification when enable tethering in data off.
              - modified BigTextStyle : MQS No.6

    Modified files - Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
//unnecessariness feature becaure LGU+ request is changed. #define FEATURE_LGT_DS_TETHERING_ERROR_POPUP

/* Item : Tethering
	Comment - 20140527 Seo Inyong(P14997)
	Reason - When The device connected to network via Wi-Fi, Tethering should not controlled by mobile data

	Modified file - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_TETHERING_VIA_WIFI_IGNORE_DATA_ONOFF_SETTING

#ifdef FEATURE_LGU_DS_RESTRICT_DATA_CALL
/* Item : IMS
    Commnet - 20120828 sjm, 20121025 Alice(P15279)
    Reason - Error return for ims when imei is null or empty

    Modified files - PhoneConstants.java (msm8974_kk\frameworks\base\telephony\java\com\android\internal\telephony),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_IMS_ERROR_RETURN
#endif /* FEATURE_LGU_DS_RESTRICT_DATA_CALL */

/* Item : VoLTE PCSCF 
    Comment - 20130510 BKY
                 - removed code for initializing P-CSCF address because of side effect
                    If P-CSCF address is initialized, you have to initialize P-CSCF address after detach
                 - replaced of FEATURE_LGT_DS_VOLTE_PCSCF_ADDR
                 - adding PCSCF3, PCSCF4 in case of connecting to emergency PDN. (7/29)
                    
    Comment - 20130510 BKY
    Reason - To save P-CSCF address for IMS registration by netmgr

    Modified files - HandsetProperty.java (frameworks\base\core\java\android\lgt\handset), 
                        netmgr.h (vendor\qcom\proprietary\data\netmgr\inc),
                        netmgr_defs.h (vendor\qcom\proprietary\data\netmgr\src),
                        netmgr_kif.c (vendor\qcom\proprietary\data\netmgr\src),
                        netmgr_qmi.c (vendor\qcom\proprietary\data\netmgr\src)		
*/    
#define FEATURE_LGU_DS_VOLTE_PCSCF_ADDR_BY_NETMGR

/* Item : IMS
  Commnet - 20121119 Alice(P15279)
  Reason - prevent to retry IMS in framework layer. becauce VT App.'s 

   Modified files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_PREVENT_TO_RETRY_IMS

/* Item : Add mutex protection to serialize DHCP operations.
     Commnet - 20130218 bky
	 If two DHCP threads are going on in parallel, they call into
	 ifc_utils library which is not thread safe. As soon as the first
	 DHCP completes, the ifc_utils control socket is also closed,
	 thus causing issues during the second link setup. This patch
	 serializes the DHCP operations for different threads.
   
  	 Reason - CRs-Fixed 413480
  	 
     Modified files - netmgr_kif.c (vendor\qcom\proprietary\data\netmgr\src)
     						 netmgr_main.c (vendor\qcom\proprietary\data\netmgr\src)
*/
#undef FEATURE_LGU_DS_NETMGR_ADD_MUTEX

#ifdef FEATURE_LGU_DS_MPDN_RETRY_ALGORITHM
/* Item : Reset ODB by Detach
    Comment - 20130822 SJM
    Reason - Reject Cause set to 255
    
    Modified Files - RIL.java (rameworks\base\telephony\java\com\android\internal\telephony),
                         Intent.java(frameworks\base\core\java\android\content),
                         SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_RESET_BARRING_DETACH 
#endif /* FEATURE_LGU_DS_MPDN_RETRY_ALGORITHM */

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: NSWO
/*-------------------------------------------------------------------------------------*/
/* Item : NSWO
    Commnet - 2012.11.01 sjm,  20130904 Alice(P15279)
    Reason - LGU+ HO Client porting
              - added package name for deactivating "Disable" button

    Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
                         QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                         Core.mk (android\build\target\product),
                         SKYSystem.java (pantech\frameworks\skysettings\java\com\pantech\providers\skysettings)

    Added files - pantech\apps\LGUHOclient\*
                      HOClient.java (frameworks\base\services\java\com\lguplus\ho_client),
                      Native.java (frameworks\base\services\java\com\lguplus\ho_client),
                      PolicyProxy.java (frameworks\base\services\java\com\lguplus\ho_client)                     
*/
#define FEATURE_LGU_DS_NSWO

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: NSRM Agent
/*-------------------------------------------------------------------------------------*/

/* Item : NSRM
    Commnet - 20140105 Alice(P15279)
    Reason - prepared LGU+ NSRM
              - according to persist.preload.nsrm, android trigger init.pantech.nsrm.sh when booting.
              - NSRM Binary 2.0 : persist.preload.nsrm=OFF in buildinfo.sh
              - Except for NSRM Binary 2.0 : persist.preload.nsrm=NONE in buildinfo.sh

    Modified files - Android.mk (device\qcom\common\rootdir),
                         buildinfo.sh (msm8974_kk\build\tools),
                         common.mk (msm8974_kk\device\qcom\common),
                         init.qcom.rc (device\qcom\common\rootdir\etc),
						 property_service.c (system\core\init),
						 system.prop (device\qcom\msm8974)

    Added files - NsrmConfiguration.xml(pantech\apps\LGUNsrm),
                         Android.mk(pantech\apps\LGUNsrm)
                      init.pantech.nsrm.sh(\device\qcom\common\rootdir\etc)    

    Commnet - 20140411 Alice(P15279)
    Reason - modify LGU+ NSRM's Policy according to LGU+ Requirement
               - <EAQSRDT>60</EAQSRDT> -> <EAQSRDT>0</EAQSRDT>
               - disabled EAQSRDT
               - persist.preload.nsrm : complete -> complete_eaqsrdt

    Modified files - init.qcom.rc (device\qcom\common\rootdir\etc),
                        init.pantech.nsrm.sh(\device\qcom\common\rootdir\etc)
*/
#define FEATURE_LGU_DS_NSRM

#ifdef FEATURE_LGU_DS_NSRM
/* Item : NSRM Agent
    Commnet - 2013.8.15 CHANGRYUL
    Reason - LGU+ NSRM AGENT porting

    Modified files - Android.mk (pantech\apps\lgunsrm),
                         system.prop (device\qcom\msm8974)

    Added files - nsrm_client.apk (pantech\apps\LGUNsrm\pre-built)

    Commnet - 20131106 Alice(P15279)
    Reason - added package name for deactivating "Disable" button
               - merged from EF59L
    
    Modified files - SKYSystem.java (pantech\frameworks\skysettings\java\com\pantech\providers\skysettings)
*/

#define FEATURE_LGU_DS_NSRM_AGENT
#endif /* FEATURE_LGU_DS_NSRM */

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: Roaming
/*-------------------------------------------------------------------------------------*/

// TODO: NOT USED ??
/* Item : Roaming Data Connection
 	Comment - 20121207 Eomhyunwoo
  	Reason - When data roaming is disabled in overseas roaming area, popup message is displayed; "Unable to set Tethering_hotspot"
 	Modified files - Tethering.java
*/
//#define FEATURE_LGU_DS_ROAMING_Tethering_hotspot_ERROR_POPUP

// TODO: NOT USED ??
/* Item : Roaming Data Connection
 	Comment - 20120315 Yoonjunho
  	Reason - Check whether PS domain attachment is rejected or not when we are in roaming area
 	Modified files
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_PS_REJECT


/* Item : Roaming Data Connection
 	Comment -  20140108_yunsik_DATA
  	Reason - original feature is FEATURE_SKT_DS_ROAMING
  	                merged feature FEATURE_LGU_DS_DIFFERENT_DATA_ROAMING (20130320 ParkMinOh, MOBILE_DATA and DATA_ROAMING have different usage with Original, So make seperate usage)
 	Modified files 
        - ConnectivityService.java (frameworks\base\services\java\com\android\server)
        - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
        - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ROAMING

/* Item : Roaming Data Connection
 	Comment -  20140301_yunsik_DATA
  	Reason - roaming to domestic  or  domestic to roaming intent
  	                changed from getCurrentDataConnectionState() to mSS.getVoiceRegState() in doProcessDataRoaming()
 	Modified files 
        - GsmServiceStateTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
        - Intent.java (frameworks\base\core\java\android\content)
        - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_TO_ROAMING_OR_TO_DOMESTIC_INDICATOR

/* Item : Roaming Data Connection Pop Up
 	Commnet - 20120315 Yoonjunho, 20140228_yunsik_DATA
  	Reason - 1. Show roaming data connection dialog box when we are in roaming area
  	                2. add compare mCurrDataRoaming and mOldDataRoaming in onRoamingOff()
 	Modified files
        - DctConstants.java (frameworks\base\telephony\java\com\android\internal\telephony)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
        - LGURoamingDataDialog.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - Public.xml (frameworks\base\core\res\res\values)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - Strings_ds.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_ROAMING_DATA_MENU_POPUP

/* Item : Roaming Data Connection Pop Up
 	Commnet - 20140128_yunsik_DATA, 20140310_yunsik_DATA
  	Reason - Roaming Pop up Color String (from Call UI : FEATURE_SET_COLOR)
  	                changed from Color.RED to Color.rgb(0, 176, 223)
 	Modified files
        - LGURoamingDataDialog.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - Public.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_ROAMING_DATA_MENU_POPUP_COLOR_STRING


/* Item : Roaming Data Connection Pop Up
 	Comment -  20140108_yunsik_DATA, 20140228_yunsik_DATA
  	Reason - 1. original features are FEATURE_LGU_DS_ROAMING_ALARM_WINDOW & FEATURE_LGU_DS_STRING_CHANGE
  	                2. add compare mCurrDataRoaming and mOldDataRoaming in onRoamingOn()
 	Modified files 
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - GsmServiceStateTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
        - Intent.java (frameworks\base\core\java\android\content)
        - NotificationMgr.java (packages\services\telephony\src\com\android\phone)
        - PhoneGlobals.java (packages\services\telephony\src\com\android\phone)
 	Added files 
        - Strings_ds.xml (packages\services\telephony\res\values)
        - Strings_ds.xml (packages\services\telephony\res\values-ko)
*/
#define FEATURE_LGU_DS_ROAMING_POPUP_AND_NOTI_CAUSED_DATA_DISCONNECTED_DUE_TO_ROAMING

/* Item : Roaming Data Connection Pop Up
 	Comment - 20120510 Yoonjunho
  	Reason - Display Background selection popup in alarm window
 	Modified files
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - NetworkPolicyManagerService.java (frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_LGU_DS_ROAMING_ALARM_RESTRICT_BACKGROUND_DATA

/* Item : Roaming Data Connection
 	Comment - 20120315 Yoonjunho, 20140228_yunsik_DATA, 20140303_yunsik_DATA
  	Reason - 1. Disable data connection menu in easy setting when we are in roaming area
 	                2. changed from roaming to mCurrDataRoaming
 	                3. when re-booted (zone changed) in roaming area, easy setting(Data roaming) did not sync with menu setting
 	Modified files
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - GsmServiceStateTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ALWAYSON_MENU_DISABLED_IN_ROAMING


/* Item : Data Connection
 	Comment - 20120406 Yoonjunho
  	Reason - for 3rd parth app, provide API
 	Modified files
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/ 
#define FEATURE_LGU_DS_NET_OVERLIMIT_API

/* Item : Roaming Data Connection
 	Comment - 20120510 Yoonjunho
  	Reason - Change default APN with "wroaming.lguplus.co.kr" when we are in roaming area
 	Modified files
        - ApnSettings.java (packages\apps\settings\src\com\android\settings)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
        - LGURoamingStatus.java (packages\services\telephony\src\com\android\phone)
        - LteRoamingCountryDialog.java (frameworks\opt\telephony\skytelephony\common\java\com\android\internal\telephony\gsm)
        - Qcril_data_netctrl.c (vendor\qcom\proprietary\qcril\common\data)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - TelephonyProvider.java (packages\providers\telephonyprovider\src\com\android\providers\telephony)
*/
#define FEATURE_LGU_DS_ROAMING_APN_CHANGE

/* Item : Roaming Data Connection
 	Comment - 20121213 Eomhyunwoo
  	Reason - default setting of data roaming is "false"
 	Modified files
        - Full_base_telephony.mk (build\target\product)
*/
#define FEATURE_LGU_DS_ROAMING_DEFAULT_SETTING 

/* Item : Roaming Data Connection PDP reject
 	Comment - 20120510 Yoonjunho
  	Reason - send PDP reject popup
 	Modified files - GsmDataConnectionTracker.java, strings_ds.xml,public.xml

 	Comment -  20131031_yunsik_DATA, 20140211_yunsik_DATA, 20140218_yunsik_DATA
  	Reason - 1. changed text and DO NOT remove notification. If the data connection succeeds, it will be disappear (new UI req. 77page)
                    2. added FLAG_ACTIVITY_SINGLE_TOP flag
                        display one more if fail cause is different
 	                    remove notification if LTE mode changed
 	                3. moved to DataConnection
 	Modified files
        - DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DctConstants.java (frameworks\base\telephony\java\com\android\internal\telephony)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - Public.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_PDP_REJECT_POPUP

/* Item : Roaming Data Connection PDP reject
 	Comment - 20140103_yunsik_DATA
  	Reason - requirement updated. (20131224_v1_6_9 : 4.1.6 - 3.3)
   	                if ue received PDP reject(#27) on LTE mode, Should notify "please LTE mode off".
 	Modified files
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - Public.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values)
        - Strings_ds.xml (frameworks\base\core\res\res\values-ko)
*/
#define FEATURE_LGU_DS_PDP_REJECT_POPUP_ERROR_REASON_27

/* Item : Roaming Data Connection PDP reject
 	Comment - 20140526_yunsik_DATA
  	Reason - Keep expanded notification.
  	                Notification should show error code.

 	Added files
        - Sky_data_custom_pdp_reject_notification_base.xml (frameworks\base\core\res\res\layout)
        - Sky_data_custom_pdp_reject_notification_big_text.xml (frameworks\base\core\res\res\layout)

 	Modified files
        - BaseStatusBar.java (frameworks\base\packages\systemui\src\com\android\systemui\statusbar)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - Public.xml (frameworks\base\core\res\res\values)
*/
#define FEATURE_LGU_DS_PDP_REJECT_POPUP_KEEP_EXPANDED_NOTIFICATION


/* Item : Roaming Data Connection PDP reject Test Code
 	Comment - 20140211_yunsik_DATA, 20140218_yunsik_DATA
  	Reason - you can change the fail cause for Test in eng version (telephony.test.fail.cause)
   	                if you set property, DCT will retry even though permanent fail.
 	Modified files
        - DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_PDP_REJECT_POPUP_TEST

/* Item : Roaming Data Connection
 	Comment -  20140103_yunsik_DATA, 20140214_yunsik_DATA
  	Reason - 1. requirement updated. (20131224_v1_6_9 : 4.7.1 - 5)
  	                    when moved from oversea to domestic, must clear restrict background
  	                2. in case battery removed and then boot. can not recognized roaming to domestic. so, made a persist property (persist.radio.prev.roaming)
  	                3. add getDataRegState condition (STATE_IN_SERVICE) in GsmServiceStateTracker
  	                    and made a property (gsm.radio.curr.roaming)
 	Modified files
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - GsmServiceStateTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
*/
#define FEATURE_LGU_DS_ROAMING_RESTRICT_BACKGROUND_CLEAR_WHEN_COME_BACK_DOMESTIC


/* Item : setRestrictBackground
 	Comment - 20140120_yunsik_DATA
  	Reason - If other package calls "setRestrictBackground" directly. it will be caused exception.
  	                eg) AppOps                Bad call: specified package android under uid 1001 but it is really 1000 
  	                    LGURoamingDataDialog  setRestrictBackground exception : java.lang.SecurityException: Package android does not belong to 1001 
  	                so, it is changed intent method.
 	Modified files
        - DataEnabler.java (frameworks\base\packages\systemui\src\com\android\systemui\statusbar\easysetting\enabler)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - GsmServiceStateTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
        - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
        - LGURoamingDataDialog.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - LGURoamingDataDialog.java (packages\services\telephony\src\com\android\phone)
        - LGURoamingResetDialog.java (packages\services\telephony\src\com\android\phone)
        - LGURoamingSettings.java (packages\services\telephony\src\com\android\phone)
        - LteRoamingCountryDialog.java (frameworks\opt\telephony\skytelephony\common\java\com\android\internal\telephony\gsm)
        - NetworkPolicyManagerService.java (frameworks\base\services\java\com\android\server\net)
        - RoamingDataPopupLGU.java (frameworks\base\packages\systemui\src\com\android\systemui\statusbar\easysetting)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_SET_RESTRICT_BACKGROUND_USING_INTENT

/* Item : setRestrictBackground
 	Comment - 20140127_yunsik_DATA
  	Reason - when setRestrictBackground(true) called, notification is blinking because other event.
  	                so, seperated from mActiveNotifs.
 	Modified files
        - NetworkPolicyManagerService.java (frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_LGU_DS_RESTRICT_BACKGROUND_NOTIFICATION_BLINKING_FIX

/* Item : LTE Roaming APN
    Comment - 20130521 SJM
    Reason - added LTE Romaing APN Controll
    Modified files
        - Apns.xml (frameworks\base\core\res\res\xml)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_LTE_ROAMING_APN 

/* Item : Oversea UICC
    Comment - 20130910 SJM
    Reason - Support permant fail like roaming
    Modified files
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_OVERSEACOUNTRY_COMMON_DATA 

/* Item : Roaming Test
    Comment - 20140114_yunsik_DATA
    Reason - You can force the telephony stack to always assume that it's roaming to verify higher-level framework functionality:device# setprop telephony.test.forceRoaming true  (PROP_FORCE_ROAMING)
                    this property provides from google default (KK)
    Modified files
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - GsmServiceStateTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\gsm)
*/
#define FEATURE_LGU_DS_FORCE_ROAMING_FOR_TEST

/* Item : OverSea SIM test
    Comment - 20140205_yunsik_DATA
    Reason - Other SIM test simulating
                    set telephony.test.apn.numeric property and kill phone process
                    ex) - adb shell setprop telephony.test.apn.numeric 46601
                          - adb shell ps | grep phone
                          - adb shell kill xxxx
    Modified files
        - ApnSettings.java (packages\apps\settings\src\com\android\settings)
        - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
        - MccTable.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
        - TelephonyProvider.java (packages\providers\telephonyprovider\src\com\android\providers\telephony)
*/
#define FEATURE_LGU_DS_FORCE_APN_NUMERIC


/* Item : GwHiddenMenu
    Comment - 20140205_yunsik_DATA
    Reason - original feature names are FEATURE_SKT_DS_HSPA, FEATURE_SKT_DS_HSUPA, FEATURE_LGT_DS_ENG_MENU
    Modified files
        <FEATURE_SKT_DS_HSPA, FEATURE_SKT_DS_HSUPA>
        - cpmgrnative.cpp (vendor\qcom\proprietary\pantech_ps_ril\cpManager\cpmgrjni)
        - GwHiddenMenu.java (vendor\qcom\proprietary\pantech_ps_ril\GwlHiddenmenu\src\com\android\Hiddenmenu)
           or HiddenMenu.java (vendor\qcom\proprietary\pantech_ps_ril\GwlHiddenmenu\src\com\android\Hiddenmenu)
        <FEATURE_LGT_DS_ENG_MENU>
           removed
*/
#define FEATURE_LGU_DS_HSPA
#define FEATURE_LGU_DS_HSUPA

/* Item : Roaming MTU
    Comment - 20140206_yunsik_DATA
    Reason - Roaming MTU setting to 1428 (original value is 1358)
    Modified files
        - Config.xml (frameworks\base\core\res\res\values-mcc204-mnc04)
*/
#define FEATURE_LGU_DS_ROAMING_20404_MTU

/* -----------------------------------------------------------------------------------*/
    //3    LGU+ Requirement :: UICC Mobility
/*-------------------------------------------------------------------------------------*/

/* Item : USIM Mobility (SKT/KT/MVNO USIM)
    Comment -  20131105_yunsik_DATA
    Reason - added SKT/KT/MVNO APN, CHECK apns-conf.xml checksum (for telephony.db rebuilding)
                    block MMS MO in LTE
    Modified files
        - Apns.xml (frameworks\base\core\res\res\xml)
        - ApnSettings.java (packages\apps\settings\src\com\android\settings)
        - SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_USIM_MOBILITY 

/* Item : Telephony DB rebuilding (only for RC model)
    Comment - 20140703_yunsik_DATA
    Reason
        - KCT apn changed from web.sktelecom.com to lte.sktelecom.com
        - KCT's ims type removed
    Related feature
        - FEATURE_LGU_DS_USIM_MOBILITY
    Modified files
        - Apns.xml (frameworks\base\core\res\res\xml)
        - TelephonyProvider.java (packages\providers\telephonyprovider\src\com\android\providers\telephony)
*/
#ifdef FEATURE_LGU_DS_USIM_MOBILITY 
#define FEATURE_LGU_DS_TELEPHONY_DB_REBUILDING_BY_APN_CONF_CHECKSUM
#endif

/* -----------------------------------------------------------------------------------*/
    //3 Issue Follow up
/*-------------------------------------------------------------------------------------*/

/* Item : DCT
	Commnet - 20120831 Alice(P15279)
	Reason - Added additional DCT for Pantech features.

	Modified files - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
				           
	Added files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ADDITIONAL_DCT

/* Item : Data Connection Interface, AIDL
	Commnet - 20120831 Alice(P15279)
	Reason - Added DCT interface for other service.

	Modified files - Android.mk(frameworks\base), 
			              Service_manager.c (frameworks\native\cmds\servicemanager),
				           SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
				           
	Added files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
			            SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ADD_DATA_AIDL

/* Item : Max windowsize/memsize for LTE
 	Commnet - 20140105 Alice(P15279)
 	Reason - change max windowsize/memsize for LTE
 	          -  FEATURE_LGU_DS_SET_TCPBUF_IN_RAT_CHANGE

 	Commnet - 20130903 BKY 
 	Reason - catching up with CA speed. 

 	Modified files - Init.qcom.rc (device\qcom\common\rootdir\etc)
*/
#define FEATURE_LGU_DS_TCP_BUFFER_FOR_LTE

// TODO: ?????????

/* Item : Data Connection
 	Commnet - 20120111 Alice(P15279)
 	Reason - do query failure cause when occur failure in DC.
 	Modified files - DataConnection.java(frameworks\base\telephony\java\com\android\internal\telephony)
*/
//#define FEATURE_LGU_DS_PS_FAIL_CAUSE

/* Item : Data Connection
 	Commnet - 20120111 Alice(P15279)
 	Modified files - DcFailCause.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_PS_FAIL_CAUSE_FATAL_EXCEPTION

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : VPN
 	Commnet - 20120117 Alice(P15279), 20120217 Alice(P15279), 20120509 kns
 	Reason - VPN Editing -> Null check
 	                
 	Modified files  - VpnDialog.java (packages\apps\settings\src\com\android\settings\vpn2)

 	Commnet - 20140304 Alice(P15279)
 	Reason - to possible VoLTE in connecting VPN
 	          -  IMS/EMERGENCY type should be exempt from VPN routing rules
 	                
    Modified files  - ConnectivityManager.java (frameworks\base\core\java\android\net)

    Commnet - 20140304 Alice(P15279)
    Reason - added "ip rule" about all type at priority 30000 + tableNum except for IMS/EMERGENCY
              - priority 100 is for VPN.
 	                
    Modified files  - RouteController.cpp (system\netd)
*/
#define CONFIG_LGU_DS_VPN

/* Item : VPN
  Commnet - 20120117 Alice(P15279)
  Reason - enable INET CONFIG for IP SEC
  Modified files  -  Kconfig (kernel\arch\arm)
*/
#define CONFIG_LGU_DS_IPSEC
#endif /* CONFIG_LGU_DS_K_CONFIG */

/* Item : NetworkInfo
 	Commnet - 20120417 Alice(P15279)
  	Reason - NetworkInfo isAvailable is false when APN State is failed
  	          - merge From STARQ, FEATURE_P_VZW_DS_APN_FAILED_STATE_BUG
 	Modified files -  DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_APN_FAILED_STATE_BUG

/* Item : Route
 	Commnet - 20121109 Alice(P15279)
  	Reason - Reconnect when routing add failed.
  	          - If occured failure to add Route, do tear down and try to reconnect after 3 sec.
  	          - merge From STARQ, FEATURE_P_VZW_DS_ROUTE_ADD_FAIL
  	          
 	Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
 	                     MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        NetworkStateTracker.java (frameworks\base\core\java\android\net),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_ROUTE_ADD_FAILURE

/* Item : APN Changed
 	Commnet - 20120727 sjm, 20121023 Alice(P15279)
  	Reason - Ignore APN Changed event due to setRoaminAPN()

 	Modified files - DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define  FEATURE_LGU_DS_IGNORE_APN_CHANGED

// TODO: NEED?

/*
 - 20120816 sjm
 - immediatly send Connection_action intent 
 */
// #define FEATURE_LGU_DS_CONNECTIVTY_INTENT_IMMEDIATE

/* Item : Exception
 	Commnet - 20140411 Alice(P15279)
  	Reason - systemServer FATAL EXCEPTION 
  	          - merge from EF63S : FEATURE_SKY_DS_PREVENT_FOR_CONCURRENT_MODIFICATION_EXCEPTION
  	          - apply to DualConnectivityState.handleDnsConfigurationChange if WQE enabled

 	Modified files - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
 */
#define FATURE_LGU_DS_CONCURRENTMODIFICATIONEXCEPTION

 /*
 - 20120912 sjm
 - APN State Bug WorkAround Code
 DcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
 */
 #define FEATURE_LGU_DS_APN_STATE_FIX

// TODO: WAIT

/* Item : Retry count
	Commnet - 20121204 Alice(P15279)
	Reason - prevent to reset retry counter 
	             according to occur EVENT_TETHERED_MODE_STATE_CHANGED
	             when fail data connection
	          - LGU+ MultiMode TEST : 16.16. PDN connectivity reject #27 (internet PDN)
	          - retry interval : 5s, 10s, 20s. and repeat after 30 min.
	             
	Modified files - DataConnectionTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony)
*/
//#define FEATURE_LGU_DS_DO_NOT_RESET_RETRY_COUNT

/* Item : OMH
	Commnet - 20130219 Alice(P15279)
	Reason - do not read unnecessary profile.
	
	Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGU_DS_OMH_DISABLED

/* Item : PARTIAL RETRY
	Commnet - 20131028 hongss
	Reason - disable partial retry
	
	Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGU_DS_DISABLE_PARTIAL_RETRY

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : DL Throughput
    Comment - 20131119 SJM
    Reason : DL Throughput form SKT

    Modified Files - bam_dmux.c (kernel\arch\arm\mach-msm)
*/
#define CONFIG_LGU_DS_BAM_ADAPTIVE_TIMER_OFF
#endif /* CONFIG_LGU_DS_K_CONFIG */

/* Item : Android Initial Attach APN
	Commnet -  20140108_yunsik_DATA
	Reason - disable Android Initial Attach APN setting (except defined APN as ia type)
                   20140206_yunsik_DATA : disable Android Initial Attach APN setting (all block)
	Modified files - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_IA_DISABLED

/* Item : Reconnect DATA
    Commnet - 20140127 sjm, 20140115 Alice(P15279)
    Reason - try to set up data right now. ignored Reconnect Alarm.

    Modified files - SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_RESET_RETRY_COUNTER

/* Item : CONNECTIVITY_CHANGE
    Comment - 20140212 SJM
    Reason - Appservice(IMS) receives CONNECTIVITY_CHANGE Intent after 3 sec.
                    changed from 1000 to 300 (for CCD) //20140701_yunsik_DATA FEATURE_LGU_DS_CCD_CONNECTIVITY

    Modified files - ConnectivityManager.java (frameworks\base\core\java\android\net)
*/
#define FEATURE_LGT_DS_REDUCE_CONNECTIVTY_CHANGE_INTENT_DELAY

/* Item : DSA
    Comment - 20131217 sjm, 20131224 hongss,  20140214_yunsik_DATA
    Reason - Clean Up all connection due to Mirror Call
                 - merge from EF52L
                 - KK version, changed from getMobileTcpTxPackets to getMobileTxPackets for Data stall
    Modified Files - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_MODIFY_DSA

/* Item : ATFWD service
    Comment - 20140219 DGKim 
    Reason - For Factory test
                 - from EF61S_KK code
    Modified Files - AndroidManifest.xml (vendor\qcom\proprietary\telephony-apps\atfwd)
                           AtFwdAutoboot.java (vendor\qcom\proprietary\telephony-apps\atfwd\src\com\qualcomm\atfwd)
                           DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
                           Msm8974.mk (device\qcom\msm8974)	
*/
#define FEATURE_LGU_DS_ATFWD_PROCESS


/* Item : Check Provisioning
    Comment - 20140221 Alice(P15279)
    Reason - do not execute to connect HIPRI for provisioning
    
    Modified Files -ConnectivityService.java (frameworks\base\services\java\com\android\server),
                        QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_PREVENT_TO_CHECK_PROVISIONING

/*
  comment - 20140224_phi_DATA.
  problem : if "exception" happen in modifyRoute() to remove route, host exemption cannot be cleared.  
*/
#define FEATURE_LGU_DS_FIXED_CLEAR_HOST_EXECMPT

/* Item : IMS startUsingNetworkFeature
    Comment - 20140327_yunsik_DATA
    Reason - when booted, if IMS calls startUsingNetworkFeature before receiving attached event, IMS state remained unavailable sometimes.
                    so, StartUsingNetworkFeature method do not return APN_TYPE_NOT_AVAILABLE only for IMS type.
    Modified Files 
        - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_SET_ALWAYS_IMS_AVAILABLE

/* Item : Retry Timer, DSA
    Comment - 20140422_yunsik_DATA
    Reason - reduced delay timer after DSA recovery action.
                    and after APN changed, after retry algorithm
    Modified Files 
        - DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_APN_DELAY_TIMER_REDUCED

/* Item : TCP Buffer
    Comment - 20140514 Alice(P15279)
    Reason - keep bigger tcp buffer when default network type is Wi-Fi or Bluetooth(only concurrent connected)
              - for exmaple at the same time Wi-Fi connected and Mobile connected, they has a different tcp buffer size.
                and  do not set tcp buffer size to last connected's network type

    Modified Files - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_KEEP_BIGGER_TCPBUF

/*......................................................................................................................................
  DNS
.........................................................................................................................................*/

/* Item : DNS
 	Commnet - 20120105 Alice(P15279)
 	Reason - check null DNS.
              - not only NULL_IP(0.0.0.0) but also length 0
 	          
 	Modified files - DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_PS_NULL_DNS_CHECK

/* Item : DNS
 	Commnet - 20120113 Alice(P15279)
 	Reason - do not add route KT's(168.126.63.1/168.126.63.2) for VT(startusingnetworkfeature(), TYPE_DUN).
 	
 	Modified files - ConnectivityService.java (frameworks\base\services\java\com\android\server),
 	                     QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_3G_DNS_REMOVE_IN_ROUTETABLE

/* Item : Tethering
 	Commnet - 20120723 sjm
  	Reason - Tethering DNS Forward Error Fix from SKT
 	Modified files - Tethering.java (frameworks\base\services\java\com\android\server\connectivity)
*/
#define FEATURE_LGU_DS_TETHERING_DNS_FORWARD

/* Item : DNS for IMS PDN
	Commnet - 20130816 SJM
	Reason - do not query DNS IPv4 for using IPv6.
	Modified files - getaddrinfo.c(bionic\libc\netbsd\net)

	Commnet - 20140408 Alice(P15279)
	Reason - added property for Rad Vision like JB.     
	          - net.dns1.pid/net.dns2.pid : pid = VT App.'s
	          - EF62L KK MQS No.16, EF63L MQS No.30
	          - adapted only previous EF67L
	Modified files - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_IMS_PDN_DNS_QUERY

#ifdef FEATURE_LGU_DS_EMERGENCY_APN
/* Item : Null DNS for IMS PDN
	Commnet - 20131128 SJM
	Reason - Allow connection with null DNS for IMS PDN
	          - replaced of FEATURE_LGU_DS_ALLOW_NULL_DNS_IMS
	          
	Modified files - DataConnection.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_ALLOW_NULL_DNS_IMS_EMERGENCY
#endif /* FEATURE_LGU_DS_EMERGENCY_APN */

/* Item : DNS
	Commnet - 20140310 Alice(P15279)
	Reason - bind to the specific link interface 
	          - IMS/INTERNET/TETHERING PDN when  same DNS allocated.
	          - merge from EF60S.
	
	Modified files - Res_send.c (bionic\libc\netbsd\resolv)
*/
#define FEATURE_LGU_DS_BIND_TO_LINK_IFACE

/* Item : DNS for photoring
	Comment - 20140320_yunsik_DATA
	Reason - During mobile data off, DNS query type can not be assigned when photoring try to establish internet APN. (query_ipv6 & query_ipv4 are zero)
	                So, when mobile data off, set setDefaultInterfaceForDns to null.
	Modified files
        - QcConnectivityService.java (frameworks\opt\connectivity\services\java)
*/
#define FEATURE_LGU_DS_RESET_DEFAULT_INTERFACE_FOR_DNS

/*......................................................................................................................................
  For NAT. - TETHER
.........................................................................................................................................*/

#ifdef CONFIG_LGU_DS_K_CONFIG
  /* Item : MTU
    Commnet - 20140211 Alice(P15279)
    Reason - added iptables policy in mangle's forward chain
              -  must enable CONFIG_NETFILTER about TCP MSS.
          
    Modified files -  SecondaryTableController.cpp (system\netd)
  */
#define CONFIG_LGU_DS_TETHER_MSS
#endif /* CONFIG_LGU_DS_K_CONFIG */

/* Item : IP RULE
	Commnet - 20140207 Alice(P15279)
	Reason - skip "ip rule flush" when disable NAT.
	           - skip excute duplication command about iptables

	Modified files - NatController.cpp (system\netd)
*/
#define FEATURE_LGU_DS_NAT_SETDEFAULTS_BUG_FIX

/* Item : IP RULE
	Commnet - 20140210 Alice(P15279)
	Reason - delete "ip rule" in secondary table when disable NAT.
	          - internal interface doesn't have IP Address when disable NAT, SocketException
	          - SOLUTION : keeping NetworkInterface when enable NAT

	Modified files - NetworkManagementService.java (frameworks\base\services\java\com\android\server)
*/
#define FEATURE_LGU_DS_DEL_IP_RULE_IN_SND_TABLE

/* Item : VPN
	Commnet - 20140417 Alice(P15279)
	Reason - tehter packet avoid VPN when connected VPN
	           - added iptables's policy about DNSMASQ.
	           - added iptables's policy about NAT

	Modified files - Android_filesystem_config.h (system\core\include\private),
                        Config.h (external\dnsmasq\src),
                        NatController.cpp (system\netd),
                        SecondaryTableController.cpp (system\netd),
                        SecondaryTableController.h (system\netd)
*/
#define FEATURE_LGU_DS_TETHER_AVOID_VPN

/*......................................................................................................................................
  For App.
.........................................................................................................................................*/

/* Item : Route
 	Comment - 20121113 Alice(P15279), 20140228_yunsik_DATA
  	Reason - added method to suspend data call. 
  	                removed return routine when called resumeDataCall() multiple.
  	                only set setInternalDataEnabled()

 	Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/ 
#define FEATURE_LGU_DS_ADD_DATA_SUSPEND_FUNC

/* Item : RADIO
	Commnet - 20120113 Alice(P15279)
	Reason - App. is not working When enter Out Of Servcie(OOS)
	             Although App. already had receivced patcket.
	          - the reason why NetworkInfo.available is false.

	Modified files - MobileDataStateTracker.java (frameworks\base\core\java\android\net),
*/
#define FEATURE_LGU_DS_AVOID_OOS_FOR_APP

/* Item : RADIO
 	Commnet - 20140121 sjm, 20120320 Alice(P15279)
 	
 	Reason - set to false "TelephonyProperties.PROPERTY_OOS_IS_DISCONNECT".
 	           - if mOosIsDisconnect == true, changed data connection to disconnection as soon as entering no-service.
 	           - cause confusion in App. layer.
 	Modified files - System.prop(device\qcom\msm8974)
*/
#define FEATURE_LGU_DS_OOS_PROPERTY_INITIAL_VAL

/* Item : MMS
 	Commnet - 20120217 Alice(P15279)
  	Reason - MMS App. Request.
            - added "isAvailableForMms" return to connection state of mobile_mms before "startUsingNetworkFeature" 
            - return true :: AlwaysOnSetting true, AuthFail false, DC.FailCause not permanantfail, In service.

            - added getServiceState:: for No service.
            - as 1x Data Disable return false when RadioTech is 1x
            
    Modified files - ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                        MobileDataStateTracker.java (frameworks\base\core\java\android\net),
                        SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_CHECK_NETWORK_AVAILABLE_FOR_MMS

/* Item : MDM
    Commnet - 20140115, Alice(P15279)
    Reason : MDM App. Requirement
    
    Modified files - BandwidthController.cpp (system\netd),
                        BandwidthController.h (system\netd),
                        CommandListener.cpp (system\netd),
                        DcTrackerBase.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        DevicePolicyManager.java (frameworks\base\core\java\android\app\admin),
                        DevicePolicyManagerService.java (frameworks\base\services\java\com\android\server),
                        INetworkManagementService.aidl (frameworks\base\core\java\android\os),
                        ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),                        
                        NetworkManagementService.java (frameworks\base\services\java\com\android\server),
                        SkyDcTracker.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection),
                        Tethering.java (frameworks\base\services\java\com\android\server\connectivity)                        
*/
#define FEATURE_LGU_DS_MDM_DATA_REQUIREMENT

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : SIP packet with over MTU using TCP dropped 
     Comment : 20130821 SJM
     Modified files : nf_conntrack_sip.c    (Kernel\net\netfilter)
*/
#define CONFIG_LGU_DS_TCP_DROP_OVER_MTU_PORT_5060
#endif /* CONFIG_LGU_DS_K_CONFIG */

#if defined(FEATURE_LGU_DS_QMI) && defined(FEATURE_LGU_DS_EMERGENCY_APN)
/* Item : Setting to disconnect Default PDN
    Comment - 20130827 KYJ
    Reason - set Modem flag to disconnect Default PDN(IMS).

    Modified Files - ConnectivityManager.java (frameworks\base\core\java\android\net),
                         IConnectivityManager.aidl (frameworks\base\core\java\android\net),
                         ISkyDataConnection.aidl (frameworks\base\telephony\java\com\android\internal\telephony\dataconnection),
                         QcConnectivityService.java (frameworks\opt\connectivity\services\java),
                         QmiDataClnt.java (msm8974_jb\linux\android\pantech\frameworks\qmi_data\java\com\pantech\qmidata),
                         QmiDataClnt.cpp (msm8974_jb\linux\android\pantech\frameworks\qmi_data\jni),
                         Qmi_data_clnt.c (msm8974_jb\linux\android\pantech\frameworks\qmi_data\qmi),
                         Qmi_data_def.h (msm8974_jb\linux\android\pantech\frameworks\qmi_data\qmi),
                         SkyDataConInterfaceManager.java (frameworks\opt\telephony\src\java\com\android\internal\telephony\dataconnection)
*/
#define FEATURE_LGU_DS_DISCONNECT_DEFAULT_PDN_FOR_IMS
#endif /* FEATURE_LGU_DS_QMI && FEATURE_LGU_DS_EMERGENCY_APN */

/* Item : Power Save
     Comment : 20140124 Alice(P15279)
     Reason - request PowerSave App.
               - it's possible to control Wi-Fi data by PowerSave.
               - using iptables
               - merge from EF60S KK : FEATURE_SKY_DS_RESTRICT_BACKGROUD_WIFI
     
     Modified files : INetworkPolicyManager.aidl (frameworks\base\core\java\android\net),
                          NetworkPolicyManager.java (frameworks\base\core\java\android\net),
                          NetworkPolicyManagerService.java (frameworks\base\services\java\com\android\server\net)
*/
#define FEATURE_LGU_DS_ADD_METHOD_FOR_POWERSAVE

/* Item : VPN popup modify 
     Comment : 20140306 DGKim(P13157)
     Reason - PLM 00061 to use device default alarm window

     Modified files : AndroidManifest.xml (frameworks\base\packages\VpnDialogs)
                          ManageDialog.java (frameworks\base\packages\vpndialogs\src\com\android\vpndialogs)
*/
#define FEATURE_LGU_DS_VPN_POPUP

/*......................................................................................................................................
  CTS
.........................................................................................................................................*/
// TODO: WAIT

/* Item : CTS IPv6 port listening fail
    Commnet - 20131030 hongss
    Reason - not use QC ims stack and cts listening port test fail fix(CTS TEST Fail)
           - merge from EF61K
    Modified files - init.target.rc (android\device\qcom\msm8974),
                     device-vendor.mk (android\vendor\qcom\proprietary\common\config)
*/
//#define FEATURE_LGU_DS_CTS_LISTENING_PORT_TEST_FAIL_FIX

#ifdef CONFIG_LGU_DS_K_CONFIG
/* Item : CTS IPV6 testLoopbackPing test fail
    Comment - 20140214 SJM
    Reason - libcore.io.ErrnoException: socket failed: EPROTONOSUPPORT (Protocol not supported) at libcore.io.Posix.socket(Native Method)
    Modified fiels - Af_inet.c (kernel\net\ipv4),
                        Af_inet6.c (kernel\net\ipv6),
                        Icmp.c (msm8974_kk\kernel\net\ipv6),
                        Icmp.c (kernel\net\ipv6),
                        Ipv6.h (kernel\include\net),
                        Makefile (kernel\net\ipv6),
                        Ping.c (kernel\net\ipv4),
                        Ping.h (kernel\include\net),
                        Transp_v6.h (kernel\include\net)
                        
    Added files - ping.c((kernel/net/ipv6)
*/
//APSS(Linux) LNX.LA.3.5.1-01110-8x74.0-1 patch  20140311 Alice(P15279)
//#define CONFIG_LGU_DS_CTS_IPV6_PING_TEST_FAIL_FIX
#endif

/* Item : CTS test_SSLSocket_reusedNpnSocket fail
    Commnet - 20140702_yunsik_DATA
    Reason
         - Compatibility Test Package: android.core.tests.libcore.package.libcore
            libcore.javax.net.ssl.SSLSocketTest
            -- test_SSLSocket_reusedNpnSocket
                 javax.net.ssl.SSLException: Connection closed by peer at com.android.org.conscrypt.NativeCrypto.SSL_do_handshake(Native Method) 
         - Fail Condition : 
             1. CTS version 4.4r3
             2. Android Version : 4.4.2 r2
    Modified files
        - OpenSSLECPrivateKey.java (libcore\crypto\src\main\java\org\conscrypt)
        - org_conscrypt_NativeCrypto.cpp (libcore\crypto\src\main\native)
*/
#define FEATURE_LGU_DS_CTS_SSLSocketTest_FAIL_FIX

/*......................................................................................................................................
  Network Tools
.........................................................................................................................................*/
/* Item : BUSYBOX 
	Commnet - 20120509 Alice(P15279)
	Reason - busybox install for root(eng) version 1.20.0

   Modified fiels -Android.mk (vendor\pantech\development\network)
   Added files - busybox (vendor\pantech\development\network)
*/
#define FEATURE_LGU_DS_BUSYBOX_INSTALL 

/* Item : IPERF 
	Commnet - 20120509 Alice(P15279)
	Reason - iperf install for root, version : 2.0.5 (08 Jul 2010)

   Modified fiels -Android.mk (vendor\pantech\development\network)
   Added files - iperf (vendor\pantech\development\network)
*/
#define FEATURE_LGU_DS_IPERF_INSTALL

/* Item : MPDP
	Comment - 20140116_yunsik_DATA
	Reason - Pantech MPDP created
	                Qualcomm MPDP : modified text size and added tethering/photoring apn types
	                FEATURE is not labeling
	                You can grant permission and meta data
					added stopUsingNetworkFeaturePdnDisconnect button

    Added files
        - PantechMultiplePdnTest folder all (vendor\qcom\proprietary\telephony-apps\PantechMultiplePdnTest)
    Modified files
        - ServiceTypeListActivity.java (vendor\qcom\proprietary\telephony-apps\MultiplePdnTest\src\com\android\MutiplePdpTest)
        - list_item.xml (vendor\qcom\proprietary\telephony-apps\MultiplePdnTest\res\layout)
        - settings_detail.xml (vendor\qcom\proprietary\telephony-apps\MultiplePdnTest\res\layout)
*/
#define FEATURE_LGU_DS_PANTECH_MPDP_APK

/*......................................................................................................................................
  CCD
.........................................................................................................................................*/

/* Item : CCD (dual download/fast download)
	Comment - 20140613 Alice(P15279)
	Reason
        - merge from FEATURE_SKY_DS_CCD_CONNECTIVITY
        - for CCD        

    Modified files
        - QcConnectivityService.java (frameworks\opt\connectivity\services\java)

*/
#define FEATURE_LGU_DS_CCD_CONNECTIVITY

#endif /* FEATURE_LGU_DS_COMMON */

/*===========================================================================
    Others
===========================================================================*/

/* Item : Log change (Radio -> Main)
    Commnet - 20140122 Alice(P15279)
    Reason - change buffer to print Data Framework log, RADIO BUFFER -> MAIN BUFFER.
                 for Data Call State. 

    Modified files - Logd_write.c (system\core\liblog)
*/
#define FEATURE_LGU_DS_CHANGE_ADB_BUFFER

#endif/* __CUST_PANTECH_DATA_LINUX_H__ */

