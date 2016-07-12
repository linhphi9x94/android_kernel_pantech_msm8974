/*
<<<<<<< HEAD
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
=======
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
<<<<<<< HEAD
/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
=======

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 */

#if !defined( WLAN_HDD_HOSTAPD_H )
#define WLAN_HDD_HOSTAPD_H

/**===========================================================================

  \file  WLAN_HDD_HOSTAPD_H.h

  \brief Linux HDD HOSTAPD include file
<<<<<<< HEAD
         Copyright 2008-2013 (c) Qualcomm, Incorporated.
         All Rights Reserved.
<<<<<<< HEAD:CORE/HDD/inc/wlan_hdd_hostapd.h
         Qualcomm Confidential and Proprietary.
  
=======
         Qualcomm Technologies Confidential and Proprietary.

>>>>>>> 009551c... wlan: hdd: remove obsolete "WLAN_SOFTAP_FEATURE" featurization:prima/CORE/HDD/inc/wlan_hdd_hostapd.h
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
  ==========================================================================*/

/*---------------------------------------------------------------------------
  Include files
  -------------------------------------------------------------------------*/

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <vos_list.h>
#include <vos_types.h>

#include <wlan_qct_tl.h>
#include <wlan_hdd_main.h>

/*---------------------------------------------------------------------------
  Preprocessor definitions and constants
  -------------------------------------------------------------------------*/

<<<<<<< HEAD
=======
/* max length of command string in hostapd ioctl */
#define HOSTAPD_IOCTL_COMMAND_STRLEN_MAX   8192

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
hdd_adapter_t* hdd_wlan_create_ap_dev( hdd_context_t *pHddCtx, tSirMacAddr macAddr, tANI_U8 *name);

VOS_STATUS hdd_register_hostapd(hdd_adapter_t *pAdapter, tANI_U8 rtnl_held);

<<<<<<< HEAD
VOS_STATUS hdd_unregister_hostapd(hdd_adapter_t *pAdapter);
=======
VOS_STATUS hdd_unregister_hostapd(hdd_adapter_t *pAdapter, tANI_U8 rtnl_held);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

eCsrAuthType 
hdd_TranslateRSNToCsrAuthType( u_int8_t auth_suite[4]);

eCsrEncryptionType 
hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4]);

eCsrEncryptionType 
hdd_TranslateRSNToCsrEncryptionType(u_int8_t cipher_suite[4]);

eCsrAuthType 
hdd_TranslateWPAToCsrAuthType(u_int8_t auth_suite[4]);

eCsrEncryptionType 
hdd_TranslateWPAToCsrEncryptionType(u_int8_t cipher_suite[4]);

<<<<<<< HEAD
VOS_STATUS hdd_softap_sta_deauth(hdd_adapter_t*,v_U8_t*);
void hdd_softap_sta_disassoc(hdd_adapter_t*,v_U8_t*);
void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t*,v_BOOL_t);
int hdd_softap_unpackIE( tHalHandle halHandle,
                eCsrEncryptionType *pEncryptType, 
                eCsrEncryptionType *mcEncryptType, 
                eCsrAuthType *pAuthType, 
                u_int16_t gen_ie_len, 
=======
VOS_STATUS hdd_softap_sta_deauth(hdd_adapter_t*, struct tagCsrDelStaParams*);
void hdd_softap_sta_disassoc(hdd_adapter_t*,v_U8_t*);
void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t*,v_BOOL_t);
int hdd_softap_unpackIE( tHalHandle halHandle,
                eCsrEncryptionType *pEncryptType,
                eCsrEncryptionType *mcEncryptType,
                eCsrAuthType *pAuthType,
                v_BOOL_t *pMFPCapable,
                v_BOOL_t *pMFPRequired,
                u_int16_t gen_ie_len,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                u_int8_t *gen_ie );

VOS_STATUS hdd_hostapd_SAPEventCB( tpSap_Event pSapEvent, v_PVOID_t usrDataForCallback);
VOS_STATUS hdd_init_ap_mode( hdd_adapter_t *pAdapter );
void hdd_set_ap_ops( struct net_device *pWlanHostapdDev );
<<<<<<< HEAD
int hdd_hostapd_set_mc_rate(hdd_adapter_t *pHostapdAdapter,
                            int targetRateHkbps);
#ifdef FEATURE_WLAN_CH_AVOID
void hdd_hostapd_ch_avoid_cb(void *pAdapter, void *indParam);
#endif /* FEATURE_WLAN_CH_AVOID */
=======
int hdd_hostapd_stop (struct net_device *dev);
void hdd_restart_softap (hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter);
#ifdef FEATURE_WLAN_CH_AVOID
void hdd_hostapd_ch_avoid_cb(void *pAdapter, void *indParam);
#endif /* FEATURE_WLAN_CH_AVOID */
int hdd_del_all_sta(hdd_adapter_t *pAdapter);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#endif    // end #if !defined( WLAN_HDD_HOSTAPD_H )
