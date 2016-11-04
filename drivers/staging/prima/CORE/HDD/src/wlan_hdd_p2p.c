/*
<<<<<<< HEAD
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
=======
 * Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
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

/**========================================================================

  \file  wlan_hdd_p2p.c

  \brief WLAN Host Device Driver implementation for P2P commands interface

<<<<<<< HEAD
  Copyright 2008 (c) Qualcomm, Incorporated.  All Rights Reserved.

  Qualcomm Confidential and Proprietary.
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

  ========================================================================*/

#include <wlan_hdd_includes.h>
#include <wlan_hdd_hostapd.h>
#include <net/cfg80211.h>
#include "sme_Api.h"
<<<<<<< HEAD
#include "wlan_hdd_p2p.h"
#include "sapApi.h"
#include "wlan_hdd_main.h"

=======
#include "sme_QosApi.h"
#include "wlan_hdd_p2p.h"
#include "sapApi.h"
#include "wlan_hdd_main.h"
#include "vos_trace.h"
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/ieee80211_radiotap.h>
#ifdef FEATURE_WLAN_TDLS
#include "wlan_hdd_tdls.h"
#endif
<<<<<<< HEAD

//Ms to Micro Sec
#define MS_TO_MUS(x)   ((x)*1000);
=======
#include "wlan_hdd_trace.h"
#include "vos_types.h"
#include "vos_trace.h"
#include "vos_sched.h"

//Ms to Micro Sec
#define MS_TO_MUS(x)   ((x)*1000);
tANI_U8* hdd_getActionString( tANI_U16 MsgType )
{
    switch (MsgType)
    {
       CASE_RETURN_STRING(SIR_MAC_ACTION_SPECTRUM_MGMT);
       CASE_RETURN_STRING(SIR_MAC_ACTION_QOS_MGMT);
       CASE_RETURN_STRING(SIR_MAC_ACTION_DLP);
       CASE_RETURN_STRING(SIR_MAC_ACTION_BLKACK);
       CASE_RETURN_STRING(SIR_MAC_ACTION_PUBLIC_USAGE);
       CASE_RETURN_STRING(SIR_MAC_ACTION_RRM);
       CASE_RETURN_STRING(SIR_MAC_ACTION_FAST_BSS_TRNST);
       CASE_RETURN_STRING(SIR_MAC_ACTION_HT);
       CASE_RETURN_STRING(SIR_MAC_ACTION_SA_QUERY);
       CASE_RETURN_STRING(SIR_MAC_ACTION_PROT_DUAL_PUB);
       CASE_RETURN_STRING(SIR_MAC_ACTION_WNM);
       CASE_RETURN_STRING(SIR_MAC_ACTION_UNPROT_WNM);
       CASE_RETURN_STRING(SIR_MAC_ACTION_TDLS);
       CASE_RETURN_STRING(SIR_MAC_ACITON_MESH);
       CASE_RETURN_STRING(SIR_MAC_ACTION_MHF);
       CASE_RETURN_STRING(SIR_MAC_SELF_PROTECTED);
       CASE_RETURN_STRING(SIR_MAC_ACTION_WME);
       CASE_RETURN_STRING(SIR_MAC_ACTION_VHT);
       default:
           return ("UNKNOWN");
    }
}

>>>>>>> 3bbd1bf... staging: add prima WLAN driver

#ifdef WLAN_FEATURE_P2P_DEBUG
#define MAX_P2P_ACTION_FRAME_TYPE 9
const char *p2p_action_frame_type[]={"GO Negotiation Request",
                                     "GO Negotiation Response",
                                     "GO Negotiation Confirmation",
                                     "P2P Invitation Request",
                                     "P2P Invitation Response",
                                     "Device Discoverability Request",
                                     "Device Discoverability Response",
                                     "Provision Discovery Request",
                                     "Provision Discovery Response"};

/* We no need to protect this variable since
 * there is no chance of race to condition
 * and also not make any complicating the code
 * just for debugging log
 */
tP2PConnectionStatus globalP2PConnectionStatus = P2P_NOT_ACTIVE;

#endif
<<<<<<< HEAD
#ifdef WLAN_FEATURE_TDLS_DEBUG
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#define MAX_TDLS_ACTION_FRAME_TYPE 11
const char *tdls_action_frame_type[] = {"TDLS Setup Request",
                                        "TDLS Setup Response",
                                        "TDLS Setup Confirm",
                                        "TDLS Teardown",
                                        "TDLS Peer Traffic Indication",
                                        "TDLS Channel Switch Request",
                                        "TDLS Channel Switch Response",
                                        "TDLS Peer PSM Request",
                                        "TDLS Peer PSM Response",
                                        "TDLS Peer Traffic Response",
                                        "TDLS Discovery Request" };
<<<<<<< HEAD
#endif
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

extern struct net_device_ops net_ops_struct;

static int hdd_wlan_add_rx_radiotap_hdr( struct sk_buff *skb,
                                         int rtap_len, int flag );

static void hdd_wlan_tx_complete( hdd_adapter_t* pAdapter,
                                  hdd_cfg80211_state_t* cfgState,
                                  tANI_BOOLEAN actionSendSuccess );

static void hdd_sendMgmtFrameOverMonitorIface( hdd_adapter_t *pMonAdapter,
<<<<<<< HEAD
                                               tANI_U32 nFrameLength, 
                                               tANI_U8* pbFrames,
                                               tANI_U8 frameType );

=======
                                               tANI_U32 nFrameLength,
                                               tANI_U8* pbFrames,
                                               tANI_U8 frameType );

static v_BOOL_t hdd_p2p_is_action_type_rsp( const u8 *buf )
{
    tActionFrmType actionFrmType;
    const u8 *ouiPtr;

    if ( buf[WLAN_HDD_PUBLIC_ACTION_FRAME_CATEGORY_OFFSET] !=
               WLAN_HDD_PUBLIC_ACTION_FRAME ) {
        return VOS_FALSE;
    }

    if ( buf[WLAN_HDD_PUBLIC_ACTION_FRAME_ACTION_OFFSET] !=
               WLAN_HDD_VENDOR_SPECIFIC_ACTION ) {
        return VOS_FALSE;
    }

    ouiPtr = &buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_OFFSET];

    if ( WPA_GET_BE24(ouiPtr) != WLAN_HDD_WFA_OUI ) {
        return VOS_FALSE;
    }

    if ( buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_TYPE_OFFSET] !=
               WLAN_HDD_WFA_P2P_OUI_TYPE ) {
        return VOS_FALSE;
    }

    actionFrmType = buf[WLAN_HDD_PUBLIC_ACTION_FRAME_SUB_TYPE_OFFSET];
    if ( actionFrmType != WLAN_HDD_INVITATION_REQ &&
        actionFrmType != WLAN_HDD_GO_NEG_REQ &&
        actionFrmType != WLAN_HDD_DEV_DIS_REQ &&
        actionFrmType != WLAN_HDD_PROV_DIS_REQ )
        return VOS_TRUE;
    else
        return VOS_FALSE;
}

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
eHalStatus wlan_hdd_remain_on_channel_callback( tHalHandle hHal, void* pCtx,
                                                eHalStatus status )
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t*) pCtx;
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
<<<<<<< HEAD
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = cfgState->remain_on_chan_ctx;

    if( pRemainChanCtx == NULL )
    {
       hddLog( LOGW,
          "%s: No Rem on channel pending for which Rsp is received", __func__);
       return eHAL_STATUS_SUCCESS;
    }

    hddLog( LOG1, "Received remain on channel rsp");

    cfgState->remain_on_chan_ctx = NULL;

    if( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request )
    {
        if( cfgState->buf )
        {
           hddLog( LOGP, 
=======
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    rem_on_channel_request_type_t req_type;


    if (pHddCtx == NULL)
    {
        hddLog(LOGE, "%s: Hdd Context is NULL", __func__);
        return eHAL_STATUS_FAILURE;
    }

    mutex_lock(&pHddCtx->roc_lock);

    pRemainChanCtx = cfgState->remain_on_chan_ctx;
    if (pRemainChanCtx == NULL)
    {
        hddLog( LOGW,
                "%s: No Rem on channel pending for which Rsp is received", __func__);
        mutex_unlock(&pHddCtx->roc_lock);
        return eHAL_STATUS_SUCCESS;
    }

    hddLog( VOS_TRACE_LEVEL_INFO,
            "Received ROC rsp (request type %d, channel %d, cookie %llu",
            pRemainChanCtx->rem_on_chan_request,
            pRemainChanCtx->chan.center_freq,
            pRemainChanCtx->cookie);
    vos_timer_stop(&pRemainChanCtx->hdd_remain_on_chan_timer);
    vos_timer_destroy(&pRemainChanCtx->hdd_remain_on_chan_timer);
    if ( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request )
    {
        if( cfgState->buf )
        {
           hddLog( LOG1,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                   "%s: We need to receive yet an ack from one of tx packet",
                   __func__);
        }
        cfg80211_remain_on_channel_expired(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                              pRemainChanCtx->dev->ieee80211_ptr,
#else
                              pRemainChanCtx->dev,
#endif
                              pRemainChanCtx->cookie,
                              &pRemainChanCtx->chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                              pRemainChanCtx->chan_type,
#endif
                              GFP_KERNEL);
<<<<<<< HEAD
    }

=======
        pAdapter->lastRocTs = vos_timer_get_system_time();
    }

    req_type = pRemainChanCtx->rem_on_chan_request;
    mutex_unlock(&pHddCtx->roc_lock);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
       )
    {
        tANI_U8 sessionId = pAdapter->sessionId;
<<<<<<< HEAD
        if( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request )
=======
        if( REMAIN_ON_CHANNEL_REQUEST == req_type )
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        {
            sme_DeregisterMgmtFrame(
                       hHal, sessionId,
                      (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                       NULL, 0 );
        }
    }
<<<<<<< HEAD
    else if ( ( WLAN_HDD_SOFTAP== pAdapter->device_mode ) ||
              ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
            )
=======
    else if (WLAN_HDD_P2P_GO == pAdapter->device_mode)
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    {
        WLANSAP_DeRegisterMgmtFrame(
                (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                NULL, 0 );
    }
<<<<<<< HEAD

    vos_mem_free( pRemainChanCtx );
    pRemainChanCtx = NULL;
    complete(&pAdapter->cancel_rem_on_chan_var);
    return eHAL_STATUS_SUCCESS;
}

void wlan_hdd_cancel_existing_remain_on_channel(hdd_adapter_t *pAdapter)
{
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    int status = 0;

    if(cfgState->remain_on_chan_ctx != NULL)
    {
        hddLog( LOG1, "Cancel Existing Remain on Channel");

        /* Wait till remain on channel ready indication before issuing cancel 
         * remain on channel request, otherwise if remain on channel not 
         * received and if the driver issues cancel remain on channel then lim 
         * will be in unknown state.
         */
        status = wait_for_completion_interruptible_timeout(&pAdapter->rem_on_chan_ready_event,
               msecs_to_jiffies(WAIT_REM_CHAN_READY));
        if (!status)
        {
            hddLog( LOGE, 
                    "%s: timeout waiting for remain on channel ready indication",
                    __func__);
        }

        INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);
        
        /* Issue abort remain on chan request to sme.
         * The remain on channel callback will make sure the remain_on_chan
         * expired event is sent.
        */
        if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
             ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
             ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
           )
        {
            sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX( pAdapter ),
                                                     pAdapter->sessionId );
        }
        else if ( (WLAN_HDD_SOFTAP== pAdapter->device_mode) ||
                  (WLAN_HDD_P2P_GO == pAdapter->device_mode)
                )
        {
            WLANSAP_CancelRemainOnChannel(
                                     (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
        }

        status = wait_for_completion_interruptible_timeout(&pAdapter->cancel_rem_on_chan_var,
               msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));

        if (!status)
        {
            hddLog( LOGE, 
                    "%s: timeout waiting for cancel remain on channel ready indication",
                    __func__);
        }
    }
=======
    mutex_lock(&pHddCtx->roc_lock);
    if ( pRemainChanCtx )
    {
        if (pRemainChanCtx->action_pkt_buff.frame_ptr != NULL
                && pRemainChanCtx->action_pkt_buff.frame_length != 0)
        {
            vos_mem_free(pRemainChanCtx->action_pkt_buff.frame_ptr);
        }
    }
    vos_mem_free( pRemainChanCtx );
    pRemainChanCtx = NULL;
    cfgState->remain_on_chan_ctx = NULL;
    mutex_unlock(&pHddCtx->roc_lock);
    if (eHAL_STATUS_SUCCESS != status)
        complete(&pAdapter->rem_on_chan_ready_event);
    complete(&pAdapter->cancel_rem_on_chan_var);
    pAdapter->is_roc_inprogress = FALSE;
    hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
    return eHAL_STATUS_SUCCESS;
}

VOS_STATUS wlan_hdd_cancel_existing_remain_on_channel(hdd_adapter_t *pAdapter)
{
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    int status = 0;

    if (NULL == pHddCtx)
    {
        hddLog(LOGE, "%s: HddCtx is NULL", __func__);
        return VOS_STATUS_E_FAILURE;
    }

    mutex_lock(&pHddCtx->roc_lock);
    pRemainChanCtx = cfgState->remain_on_chan_ctx;
    if(pRemainChanCtx != NULL)
    {
        if(VOS_TIMER_STATE_RUNNING == vos_timer_getCurrentState(
                    &pRemainChanCtx->hdd_remain_on_chan_timer))
        {
            hddLog(VOS_TRACE_LEVEL_INFO,
                    "Cancel Existing ROC (cookie=%llu)",
                    pRemainChanCtx->cookie);

            vos_timer_stop(&pRemainChanCtx->hdd_remain_on_chan_timer);
        }
        /* Wait till remain on channel ready indication before issuing cancel
         * remain on channel request, otherwise if remain on channel not
         * received and if the driver issues cancel remain on channel then lim
         * will be in unknown state.
         */
        if (pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress != TRUE)
        {
            mutex_unlock(&pHddCtx->roc_lock);
            status = wait_for_completion_interruptible_timeout(
                                        &pAdapter->rem_on_chan_ready_event,
                                        msecs_to_jiffies(WAIT_REM_CHAN_READY));
            if (0 >= status)
            {
                mutex_lock(&pHddCtx->roc_lock);
                pRemainChanCtx = cfgState->remain_on_chan_ctx;
                if (pRemainChanCtx)
                    pRemainChanCtx->is_pending_roc_cancelled = TRUE;
                mutex_unlock(&pHddCtx->roc_lock);
                hddLog( LOGE,
                        "%s: timeout waiting for remain on channel"
                        " ready indication %d",
                        __func__, status);
                return VOS_STATUS_E_FAILURE;
            }

            mutex_lock(&pHddCtx->roc_lock);
            pRemainChanCtx = cfgState->remain_on_chan_ctx;
            if (NULL == pRemainChanCtx)
            {
                mutex_unlock(&pHddCtx->roc_lock);
                hddLog( LOGE,
                        "%s-%d: pRemainChanCtx is NULL",
                        __func__, __LINE__);
                return VOS_STATUS_E_FAILURE;
            }
            /* Check again if cancel remain on channel is started.
             * If its started wait for its completiona and return.
             */
            if (TRUE == pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress)
            {
                mutex_unlock(&pHddCtx->roc_lock);
                hddLog( LOG1,
                      "ROC timer cancellation in progress,"
                      " wait for completion");
                status = wait_for_completion_interruptible_timeout(
                                     &pAdapter->cancel_rem_on_chan_var,
                                     msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
                if (0 >= status)
                {
                    hddLog( LOGE,
                          "%s:wait on cancel_rem_on_chan_var failed %d",
                           __func__, status);
                    return VOS_STATUS_E_FAILURE;
                }
                return VOS_STATUS_SUCCESS;
            }
            else
                pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = TRUE;
            mutex_unlock(&pHddCtx->roc_lock);
            INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);

            /* Issue abort remain on chan request to sme.
             * The remain on channel callback will make sure the remain_on_chan
             * expired event is sent.
             */
            if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
                   ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
                   ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode ))
            {
                if (eHAL_STATUS_SUCCESS !=
                    sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX( pAdapter ),
                                                     pAdapter->sessionId ))
                {
                    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                            FL("Failed to Cancel Remain on Channel"));
                }
            }
            else if (WLAN_HDD_P2P_GO == pAdapter->device_mode)
            {
                 WLANSAP_CancelRemainOnChannel(
                         (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
            }

            status = wait_for_completion_interruptible_timeout(
                                    &pAdapter->cancel_rem_on_chan_var,
                                    msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
            if (0 >= status)
            {
                hddLog( LOGE,
                       "%s: timeout waiting for cancel remain on channel"
                         " ready indication %d",
                          __func__, status);
            }
            hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
         }
         else
         {
              hddLog( LOG1,
                      "ROC timer cancellation in progress,"
                      " wait for completion");
              mutex_unlock(&pHddCtx->roc_lock);
              status = wait_for_completion_interruptible_timeout(
                                     &pAdapter->cancel_rem_on_chan_var,
                                     msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
              if (0 >= status)
              {
                  hddLog( LOGE,
                          "%s:wait on cancel_rem_on_chan_var failed %d",
                           __func__, status);
                  return VOS_STATUS_E_FAILURE;
              }
              return VOS_STATUS_SUCCESS;
         }
    }
    else
    {
       hddLog(LOG1,
               "%s: remain_on_chan_ctx is NULL", __func__);
       mutex_unlock(&pHddCtx->roc_lock);
    }
    return VOS_STATUS_SUCCESS;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
}

int wlan_hdd_check_remain_on_channel(hdd_adapter_t *pAdapter)
{
   int status = 0;
   hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

   if(WLAN_HDD_P2P_GO != pAdapter->device_mode)
   {
     //Cancel Existing Remain On Channel
     //If no action frame is pending
     if( cfgState->remain_on_chan_ctx != NULL)
     {
        //Check whether Action Frame is pending or not
        if( cfgState->buf == NULL)
        {
           wlan_hdd_cancel_existing_remain_on_channel(pAdapter);
        }
        else
        {
<<<<<<< HEAD
           hddLog( LOG1, "Cannot Cancel Existing Remain on Channel");
=======
           hddLog(VOS_TRACE_LEVEL_DEBUG,
                   "Cannot Cancel Existing Remain on Channel");
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
           status = -EBUSY;
        }
     }
   }
   return status;
}
<<<<<<< HEAD
=======
void wlan_hdd_remain_on_chan_timeout(void *data)
{
    hdd_adapter_t *pAdapter = (hdd_adapter_t *)data;
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
    hdd_cfg80211_state_t *cfgState;
    hdd_context_t *pHddCtx;

    if ((NULL == pAdapter) || (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic))
    {
        hddLog( LOGE, FL("pAdapter is invalid %p !!!"), pAdapter);
        return;
    }
    pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    mutex_lock(&pHddCtx->roc_lock);
    pRemainChanCtx = cfgState->remain_on_chan_ctx;

    if (NULL == pRemainChanCtx)
    {
        hddLog( LOGE, FL("No Remain on channel is pending"));
        mutex_unlock(&pHddCtx->roc_lock);
        return;
    }

    if ( TRUE == pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress )
    {
        mutex_unlock(&pHddCtx->roc_lock);
        hddLog( LOGE, FL("Cancellation already in progress"));
        return;
    }

    pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = TRUE;
    mutex_unlock(&pHddCtx->roc_lock);
    INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);
    hddLog( LOG1,"%s: Cancel Remain on Channel on timeout", __func__);
    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
          ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
           ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
       )
    {
        if (eHAL_STATUS_SUCCESS !=
                sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX( pAdapter),
                    pAdapter->sessionId ))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    FL("Failed to Cancel Remain on Channel"));
        }
    }
    else if (WLAN_HDD_P2P_GO == pAdapter->device_mode)
    {
        WLANSAP_CancelRemainOnChannel(
                (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
    }
    hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
}

static int wlan_hdd_p2p_start_remain_on_channel(
        hdd_adapter_t *pAdapter)
{
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
    hdd_adapter_t *pAdapter_temp;
    v_BOOL_t isGoPresent = VOS_FALSE;
    hdd_context_t *pHddCtx;
    hdd_cfg80211_state_t *cfgState;
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
    rem_on_channel_request_type_t request_type;
    unsigned int duration;
    v_U16_t hw_value;

    int ret = 0;

    ENTER();
    if (NULL == pAdapter)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Adapter is NULL",__func__);
        return -EINVAL;
    }
    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    ret = wlan_hdd_validate_context(pHddCtx);
    if (0 != ret)
    {
        return ret;
    }
    cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    if (NULL == cfgState)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: cfgState is not valid ",__func__);
        return -EINVAL;
    }
    mutex_lock(&pHddCtx->roc_lock);
    pRemainChanCtx = cfgState->remain_on_chan_ctx;
    if ( pRemainChanCtx  == NULL)
    {
        mutex_unlock(&pHddCtx->roc_lock);
        hddLog( LOGE,
                "%s-%d: pRemainChanCtx is NULL",
                __func__, __LINE__);
        return ret;
    }
    request_type = pRemainChanCtx->rem_on_chan_request;
    /* Initialize Remain on chan timer */
    status = vos_timer_init(&pRemainChanCtx->hdd_remain_on_chan_timer,
            VOS_TIMER_TYPE_SW,
            wlan_hdd_remain_on_chan_timeout,
            pAdapter);
    if (status != VOS_STATUS_SUCCESS)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,
                FL("Not able to initalize remain_on_chan timer"));
        cfgState->remain_on_chan_ctx = NULL;
        vos_mem_free(pRemainChanCtx);
        mutex_unlock(&pHddCtx->roc_lock);
        return -EINVAL;
    }

    duration = pRemainChanCtx->duration;
    hw_value = pRemainChanCtx->chan.hw_value;
    mutex_unlock(&pHddCtx->roc_lock);

    status =  hdd_get_front_adapter ( pHddCtx, &pAdapterNode );
    while ( NULL != pAdapterNode && VOS_STATUS_SUCCESS == status )
    {
        pAdapter_temp = pAdapterNode->pAdapter;
        if (WLAN_HDD_P2P_GO == pAdapter_temp->device_mode)
        {
            isGoPresent = VOS_TRUE;
        }
        status = hdd_get_next_adapter ( pHddCtx, pAdapterNode, &pNext );
        pAdapterNode = pNext;
    }
    hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
    //call sme API to start remain on channel.
    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
            ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
            ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
       )
    {
        tANI_U8 sessionId = pAdapter->sessionId;
        //call sme API to start remain on channel.
        if (eHAL_STATUS_SUCCESS != sme_RemainOnChannel(
                WLAN_HDD_GET_HAL_CTX(pAdapter), sessionId,
                hw_value, duration,
                wlan_hdd_remain_on_channel_callback, pAdapter,
                (tANI_U8)(request_type == REMAIN_ON_CHANNEL_REQUEST)? TRUE:FALSE))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    FL(" RemainOnChannel returned fail"));

            mutex_lock(&pHddCtx->roc_lock);
            cfgState->remain_on_chan_ctx = NULL;
            vos_timer_destroy(&pRemainChanCtx->hdd_remain_on_chan_timer);
            vos_mem_free (pRemainChanCtx);
            mutex_unlock(&pHddCtx->roc_lock);
            hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
            return -EINVAL;
        }

        if( REMAIN_ON_CHANNEL_REQUEST == request_type)
        {
            if( eHAL_STATUS_SUCCESS != sme_RegisterMgmtFrame(
                        WLAN_HDD_GET_HAL_CTX(pAdapter),
                        sessionId, (SIR_MAC_MGMT_FRAME << 2) |
                        (SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0 ))
            {
                hddLog(VOS_TRACE_LEVEL_ERROR,    "sme_RegisterMgmtFrame returned fail");
            }
        }

    }
    else if (WLAN_HDD_P2P_GO == pAdapter->device_mode)
    {
        //call sme API to start remain on channel.
        if (VOS_STATUS_SUCCESS != WLANSAP_RemainOnChannel(
                    (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                    hw_value, duration,
                    wlan_hdd_remain_on_channel_callback, pAdapter ))

        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: WLANSAP_RemainOnChannel returned fail", __func__);
            mutex_lock(&pHddCtx->roc_lock);
            cfgState->remain_on_chan_ctx = NULL;
            vos_timer_destroy(&pRemainChanCtx->hdd_remain_on_chan_timer);
            vos_mem_free (pRemainChanCtx);
            mutex_unlock(&pHddCtx->roc_lock);
            hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
            return -EINVAL;
        }


        if (VOS_STATUS_SUCCESS != WLANSAP_RegisterMgmtFrame(
                    (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                    (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                    NULL, 0 ))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: WLANSAP_RegisterMgmtFrame returned fail", __func__);
            WLANSAP_CancelRemainOnChannel(
                    (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
            hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
            return -EINVAL;
        }

    }

    pAdapter->is_roc_inprogress = TRUE;
    EXIT();
    return 0;
}

>>>>>>> 3bbd1bf... staging: add prima WLAN driver

static int wlan_hdd_request_remain_on_channel( struct wiphy *wiphy,
                                   struct net_device *dev,
                                   struct ieee80211_channel *chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                                   enum nl80211_channel_type channel_type,
#endif
                                   unsigned int duration, u64 *cookie,
                                   rem_on_channel_request_type_t request_type )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_remain_on_chan_ctx_t *pRemainChanCtx;
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
<<<<<<< HEAD
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
                                 __func__,pAdapter->device_mode);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
    hddLog( LOG1,
        "chan(hw_val)0x%x chan(centerfreq) %d chan type 0x%x, duration %d",
        chan->hw_value, chan->center_freq, channel_type, duration );
#else
    hddLog( LOG1,
        "chan(hw_val)0x%x chan(centerfreq) %d, duration %d",
        chan->hw_value, chan->center_freq, duration );
#endif
    //Cancel existing remain On Channel if any
    wlan_hdd_cancel_existing_remain_on_channel(pAdapter);
=======
    hdd_context_t *pHddCtx = NULL;
    VOS_STATUS checkReadyInd;
    hdd_adapter_t *pStaAdapter;
    int status = 0;

    ENTER();
    if (NULL == pAdapter)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: HDD adapter is Null", __func__);
        return -ENODEV;
    }

    pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    status = wlan_hdd_validate_context(pHddCtx);
    if (0 != status)
    {
        return status;
    }

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
                                 __func__,pAdapter->device_mode);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
    hddLog(VOS_TRACE_LEVEL_INFO,
           "chan(hw_val)0x%x chan(centerfreq) %d chan type 0x%x, dur %d,"
           " request type %d, cookie %llu",
           chan->hw_value, chan->center_freq, channel_type, duration,
           request_type, *cookie);
#else
     hddLog(VOS_TRACE_LEVEL_INFO,
            "chan(hw_val)0x%x chan(centerfreq) %d, duration %d"
            " reuest type %d, cookie %llu", chan->hw_value, chan->center_freq,
            duration, request_type, *cookie );
#endif
    //Cancel existing remain On Channel if any
    checkReadyInd = wlan_hdd_cancel_existing_remain_on_channel(pAdapter);
    if (VOS_STATUS_SUCCESS != checkReadyInd)
    {
       hddLog( LOGE, FL("Cancel Roc in progress"));
       return -EBUSY;
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

    /* When P2P-GO and if we are trying to unload the driver then
     * wlan driver is keep on receiving the remain on channel command
     * and which is resulting in crash. So not allowing any remain on
     * channel requets when Load/Unload is in progress*/
<<<<<<< HEAD
    if (((hdd_context_t*)pAdapter->pHddCtx)->isLoadUnloadInProgress)
    {
        hddLog( LOGE,
                "%s: Wlan Load/Unload is in progress", __func__);
        return -EBUSY;
    }

    if (((hdd_context_t*)pAdapter->pHddCtx)->isLogpInProgress)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
        return -EAGAIN;
    }
=======
    if(hdd_isConnectionInProgress((hdd_context_t *)pAdapter->pHddCtx))
    {
        hddLog( LOGE,
               "%s: Connection is in progress", __func__);
        return -EBUSY;
    }

    mutex_lock(&pHddCtx->roc_lock);

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    pRemainChanCtx = vos_mem_malloc( sizeof(hdd_remain_on_chan_ctx_t) );
    if( NULL == pRemainChanCtx )
    {
        hddLog(VOS_TRACE_LEVEL_FATAL,
             "%s: Not able to allocate memory for Channel context",
                                         __func__);
<<<<<<< HEAD
        return -ENOMEM;
    }

=======
        mutex_unlock(&pHddCtx->roc_lock);
        return -ENOMEM;
    }

    vos_mem_zero(pRemainChanCtx, sizeof (hdd_remain_on_chan_ctx_t));
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    vos_mem_copy( &pRemainChanCtx->chan, chan,
                   sizeof(struct ieee80211_channel) );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
    pRemainChanCtx->chan_type = channel_type;
#endif
    pRemainChanCtx->duration = duration;
    pRemainChanCtx->dev = dev;
<<<<<<< HEAD
    *cookie = (tANI_U32) pRemainChanCtx;
=======
    *cookie = (uintptr_t) pRemainChanCtx;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    pRemainChanCtx->cookie = *cookie;
    pRemainChanCtx->rem_on_chan_request = request_type;
    cfgState->remain_on_chan_ctx = pRemainChanCtx;
    cfgState->current_freq = chan->center_freq;
<<<<<<< HEAD
    
    INIT_COMPLETION(pAdapter->rem_on_chan_ready_event);

    //call sme API to start remain on channel.
    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
       )
    {
        tANI_U8 sessionId = pAdapter->sessionId;
        //call sme API to start remain on channel.
        sme_RemainOnChannel(
                       WLAN_HDD_GET_HAL_CTX(pAdapter), sessionId,
                       chan->hw_value, duration,
                       wlan_hdd_remain_on_channel_callback, pAdapter,
                       (tANI_U8)(request_type == REMAIN_ON_CHANNEL_REQUEST)? TRUE:FALSE);

        if( REMAIN_ON_CHANNEL_REQUEST == request_type)
        {
            sme_RegisterMgmtFrame(WLAN_HDD_GET_HAL_CTX(pAdapter),
                                   sessionId, (SIR_MAC_MGMT_FRAME << 2) |
                                  (SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0 );
        }

    }
    else if ( ( WLAN_HDD_SOFTAP== pAdapter->device_mode ) ||
              ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
            )
    {
        //call sme API to start remain on channel.
        if (VOS_STATUS_SUCCESS != WLANSAP_RemainOnChannel(
                          (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                          chan->hw_value, duration,
                          wlan_hdd_remain_on_channel_callback, pAdapter ))

        {
           VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: WLANSAP_RemainOnChannel returned fail", __func__);
           cfgState->remain_on_chan_ctx = NULL;
           vos_mem_free (pRemainChanCtx);
           return -EINVAL;
        }


        if (VOS_STATUS_SUCCESS != WLANSAP_RegisterMgmtFrame(
                    (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                    (SIR_MAC_MGMT_FRAME << 2) | ( SIR_MAC_MGMT_PROBE_REQ << 4),
                    NULL, 0 ))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: WLANSAP_RegisterMgmtFrame returned fail", __func__);
            WLANSAP_CancelRemainOnChannel(
                    (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
            return -EINVAL;
        }

    }
    return 0;

}

int wlan_hdd_cfg80211_remain_on_channel( struct wiphy *wiphy,
=======
    pRemainChanCtx->action_pkt_buff.freq = 0;
    pRemainChanCtx->action_pkt_buff.frame_ptr = NULL;
    pRemainChanCtx->action_pkt_buff.frame_length = 0;
    pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = FALSE;
    pRemainChanCtx->is_pending_roc_cancelled = FALSE;

    INIT_COMPLETION(pAdapter->rem_on_chan_ready_event);

    if (REMAIN_ON_CHANNEL_REQUEST == request_type)
    {
        pStaAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_INFRA_STATION);
        if((NULL != pStaAdapter)&&
                hdd_connIsConnected( WLAN_HDD_GET_STATION_CTX_PTR(pStaAdapter)))
        {
            if (pAdapter->lastRocTs !=0 &&
                    ((vos_timer_get_system_time() - pAdapter->lastRocTs )
                     < pHddCtx->cfg_ini->gP2PListenDeferInterval))
            {
                if (pRemainChanCtx->duration > HDD_P2P_MAX_ROC_DURATION)
                    pRemainChanCtx->duration = HDD_P2P_MAX_ROC_DURATION;

                mutex_unlock(&pHddCtx->roc_lock);

                schedule_delayed_work(&pAdapter->roc_work,
                        msecs_to_jiffies(pHddCtx->cfg_ini->gP2PListenDeferInterval));
                hddLog(VOS_TRACE_LEVEL_INFO, "Defer interval is %hu, pAdapter %p",
                        pHddCtx->cfg_ini->gP2PListenDeferInterval, pAdapter);
                return 0;
            }
        }
    }

    mutex_unlock(&pHddCtx->roc_lock);
    status = wlan_hdd_p2p_start_remain_on_channel(pAdapter);

    EXIT();
    return status;
}

int __wlan_hdd_cfg80211_remain_on_channel( struct wiphy *wiphy,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                                struct wireless_dev *wdev,
#else
                                struct net_device *dev,
#endif
                                struct ieee80211_channel *chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                                enum nl80211_channel_type channel_type,
#endif
                                unsigned int duration, u64 *cookie )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    struct net_device *dev = wdev->netdev;
#endif
<<<<<<< HEAD
    return wlan_hdd_request_remain_on_channel(wiphy, dev,
=======
    hdd_adapter_t *pAdapter;
    hdd_context_t *pHddCtx;
    int ret = 0;

    ENTER();
    pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    if (NULL == pAdapter)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Adapter is NULL",__func__);
        return -EINVAL;
    }
    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    ret = wlan_hdd_validate_context(pHddCtx);
    if (0 != ret)
    {
        return ret;
    }

    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                     TRACE_CODE_HDD_REMAIN_ON_CHANNEL,
                     pAdapter->sessionId, REMAIN_ON_CHANNEL_REQUEST));
    ret = wlan_hdd_request_remain_on_channel(wiphy, dev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                                        chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                                        channel_type,
#endif
                                        duration, cookie,
                                        REMAIN_ON_CHANNEL_REQUEST);
<<<<<<< HEAD
}

void hdd_remainChanReadyHandler( hdd_adapter_t *pAdapter )
{
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    hdd_remain_on_chan_ctx_t* pRemainChanCtx = cfgState->remain_on_chan_ctx;

    hddLog( LOG1, "Ready on chan ind");

    if( pRemainChanCtx != NULL )
    {
        if( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request )
=======
    EXIT();
    return ret;
}

int wlan_hdd_cfg80211_remain_on_channel( struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                                struct wireless_dev *wdev,
#else
                                struct net_device *dev,
#endif
                                struct ieee80211_channel *chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                                enum nl80211_channel_type channel_type,
#endif
                                unsigned int duration, u64 *cookie )
{
   int ret;

   vos_ssr_protect(__func__);
   ret = __wlan_hdd_cfg80211_remain_on_channel(wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                                               wdev,
#else
                                               dev,
#endif
                                               chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                                               channel_type,
#endif
                                               duration, cookie);
   vos_ssr_unprotect(__func__);

   return ret;
}


void hdd_remainChanReadyHandler( hdd_adapter_t *pAdapter )
{
    hdd_cfg80211_state_t *cfgState = NULL;
    hdd_remain_on_chan_ctx_t* pRemainChanCtx = NULL;
    hdd_context_t *pHddCtx;
    VOS_STATUS status;
    if (NULL == pAdapter)
    {
       hddLog(LOGE, FL("pAdapter is NULL"));
       return;
    }
    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    if (NULL == pHddCtx)
    {
        hddLog(LOGE, FL("pHddCtx is NULL"));
        return;
    }
    cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
    pAdapter->startRocTs = vos_timer_get_system_time();
    mutex_lock(&pHddCtx->roc_lock);
    pRemainChanCtx = cfgState->remain_on_chan_ctx;
    if( pRemainChanCtx != NULL )
    {
        MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                         TRACE_CODE_HDD_REMAINCHANREADYHANDLER,
                         pAdapter->sessionId, pRemainChanCtx->duration));
        //start timer for actual duration
        status = vos_timer_start(&pRemainChanCtx->hdd_remain_on_chan_timer,
                                (pRemainChanCtx->duration));
        if (VOS_STATUS_SUCCESS!=status)
        {
            hddLog( LOGE, FL("Remain on Channel timer start failed"));
        }
        if( REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request)
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        {
            cfg80211_ready_on_channel(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                               pAdapter->dev->ieee80211_ptr,
#else
                               pAdapter->dev,
#endif
<<<<<<< HEAD
                               (tANI_U32)pRemainChanCtx,
=======
                               (uintptr_t)pRemainChanCtx,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                               &pRemainChanCtx->chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                               pRemainChanCtx->chan_type,
#endif
                               pRemainChanCtx->duration, GFP_KERNEL );
        }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
<<<<<<< HEAD
        else if( OFF_CHANNEL_ACTION_TX == pRemainChanCtx->rem_on_chan_request )
=======
        else if( OFF_CHANNEL_ACTION_TX == pRemainChanCtx->rem_on_chan_request)
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        {
            complete(&pAdapter->offchannel_tx_event);
        }
#endif
<<<<<<< HEAD
        complete(&pAdapter->rem_on_chan_ready_event);
    }
    else
    {
=======
        // Check for cached action frame
        if ( pRemainChanCtx->action_pkt_buff.frame_length != 0 )
        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))
            cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr,
                             pRemainChanCtx->action_pkt_buff.freq, 0,
                             pRemainChanCtx->action_pkt_buff.frame_ptr,
                             pRemainChanCtx->action_pkt_buff.frame_length,
                             NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,12,0))
            cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr,
                             pRemainChanCtx->action_pkt_buff.freq, 0,
                             pRemainChanCtx->action_pkt_buff.frame_ptr,
                             pRemainChanCtx->action_pkt_buff.frame_length,
                             NL80211_RXMGMT_FLAG_ANSWERED, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
            cfg80211_rx_mgmt( pAdapter->dev->ieee80211_ptr,
                              pRemainChanCtx->action_pkt_buff.freq, 0,
                              pRemainChanCtx->action_pkt_buff.frame_ptr,
                              pRemainChanCtx->action_pkt_buff.frame_length,
                              GFP_ATOMIC );
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
            cfg80211_rx_mgmt( pAdapter->dev,
                              pRemainChanCtx->action_pkt_buff.freq, 0,
                              pRemainChanCtx->action_pkt_buff.frame_ptr,
                              pRemainChanCtx->action_pkt_buff.frame_length,
                              GFP_ATOMIC );
#else
            cfg80211_rx_mgmt( pAdapter->dev,
                              pRemainChanCtx->action_pkt_buff.freq,
                              pRemainChanCtx->action_pkt_buff.frame_ptr,
                              pRemainChanCtx->action_pkt_buff.frame_length,
                              GFP_ATOMIC );
#endif //LINUX_VERSION_CODE
            hddLog( LOGE, "%s: Sent cached action frame to supplicant", __func__);
            vos_mem_free(pRemainChanCtx->action_pkt_buff.frame_ptr);
            pRemainChanCtx->action_pkt_buff.frame_length = 0;
            pRemainChanCtx->action_pkt_buff.freq = 0;
            pRemainChanCtx->action_pkt_buff.frame_ptr = NULL;
        }
        hddLog( VOS_TRACE_LEVEL_INFO, "Ready on chan ind (cookie=%llu)",
                pRemainChanCtx->cookie);
        complete(&pAdapter->rem_on_chan_ready_event);
        if (TRUE == pRemainChanCtx->is_pending_roc_cancelled)
        {
            mutex_unlock(&pHddCtx->roc_lock);
            /* since pRemainChanCtx->is_pending_roc_cancelled is
             * set, it means Cancel Reamain on channel command is
             * pending because remain on channel event was not
             * ready when cancel ROC  was issued.So issue
             * cancel ROC now.
             */
            wlan_hdd_cancel_existing_remain_on_channel(pAdapter);
        }
        else
        {
            mutex_unlock(&pHddCtx->roc_lock);
        }
    }
    else
    {
        mutex_unlock(&pHddCtx->roc_lock);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        hddLog( LOGW, "%s: No Pending Remain on channel Request", __func__);
    }
    return;
}

<<<<<<< HEAD
int wlan_hdd_cfg80211_cancel_remain_on_channel( struct wiphy *wiphy,
=======
int __wlan_hdd_cfg80211_cancel_remain_on_channel( struct wiphy *wiphy,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                                                struct wireless_dev *wdev,
#else
                                                struct net_device *dev,
#endif
                                                u64 cookie )
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    struct net_device *dev = wdev->netdev;
#endif
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
<<<<<<< HEAD
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    int status;

=======
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    int status;
    u64 cookie_dummy;

    ENTER();
    cookie_dummy = cookie << 32;
    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                     TRACE_CODE_HDD_CFG80211_CANCEL_REMAIN_ON_CHANNEL,
                     pAdapter->sessionId, cookie_dummy));
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    hddLog( LOG1, "Cancel remain on channel req");

    status = wlan_hdd_validate_context(pHddCtx);

    if (0 != status)
    {
<<<<<<< HEAD
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: HDD context is not valid", __func__);
        return status;
    }
    /* FIXME cancel currently running remain on chan.
     * Need to check cookie and cancel accordingly
     */
    if( (cfgState->remain_on_chan_ctx == NULL) ||
        (cfgState->remain_on_chan_ctx->cookie != cookie) )
    {
=======
        return status;
    }
    hddLog( LOG1, "Cancel remain on channel req (cookie = %llu)", cookie);

    /* FIXME cancel currently running remain on chan.
     * Need to check cookie and cancel accordingly
     */
    mutex_lock(&pHddCtx->roc_lock);
    pRemainChanCtx = cfgState->remain_on_chan_ctx;
    if( (cfgState->remain_on_chan_ctx == NULL) ||
        (cfgState->remain_on_chan_ctx->cookie != cookie) )
    {
        mutex_unlock(&pHddCtx->roc_lock);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        hddLog( LOGE,
            "%s: No Remain on channel pending with specified cookie value",
             __func__);
        return -EINVAL;
    }
<<<<<<< HEAD

    /* wait until remain on channel ready event received
     * for already issued remain on channel request */
    status = wait_for_completion_interruptible_timeout(&pAdapter->rem_on_chan_ready_event,
            msecs_to_jiffies(WAIT_REM_CHAN_READY));
    if (!status)
    {
        hddLog( LOGE, 
                "%s: timeout waiting for remain on channel ready indication",
                __func__);
    }
=======
    if (TRUE != pRemainChanCtx->is_pending_roc_cancelled)
    {
       mutex_unlock(&pHddCtx->roc_lock);
       /* wait until remain on channel ready event received
        * for already issued remain on channel request */
       status = wait_for_completion_interruptible_timeout(&pAdapter->rem_on_chan_ready_event,
            msecs_to_jiffies(WAIT_REM_CHAN_READY));
       if (0 >= status)
       {
           hddLog( LOGE,
                   "%s: timeout waiting for remain on channel ready indication %d",
                   __func__, status);
           mutex_lock(&pHddCtx->roc_lock);
           if (cfgState->remain_on_chan_ctx)
               cfgState->remain_on_chan_ctx->is_pending_roc_cancelled = TRUE;
           mutex_unlock(&pHddCtx->roc_lock);

           return 0;

       }
        mutex_lock(&pHddCtx->roc_lock);
    }
    else
    {
        hddLog( LOG1, FL("Cancel ROC event is already pending, "
                         "waiting for ready on channel indication.") );
        mutex_unlock(&pHddCtx->roc_lock);
        return 0;
    }
    if (NULL != cfgState->remain_on_chan_ctx)
    {
        vos_timer_stop(&cfgState->remain_on_chan_ctx->hdd_remain_on_chan_timer);
        if (TRUE == pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress)
        {
            mutex_unlock(&pHddCtx->roc_lock);
            hddLog( LOG1,
                    FL("ROC timer cancellation in progress,"
                       " wait for completion"));
            status = wait_for_completion_interruptible_timeout(
                                             &pAdapter->cancel_rem_on_chan_var,
                                             msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
            if (0 >= status)
            {
                hddLog( LOGE,
                        "%s:wait on cancel_rem_on_chan_var failed %d",
                        __func__, status);
            }
            return 0;
        }
        else
            pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = TRUE;
    }
    mutex_unlock(&pHddCtx->roc_lock);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);
    /* Issue abort remain on chan request to sme.
     * The remain on channel callback will make sure the remain_on_chan
     * expired event is sent.
     */
    if ( ( WLAN_HDD_INFRA_STATION == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_CLIENT == pAdapter->device_mode ) ||
<<<<<<< HEAD
         ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
       )
    {
        tANI_U8 sessionId = pAdapter->sessionId; 
        sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX( pAdapter ),
                                            sessionId );
    }
    else if ( (WLAN_HDD_SOFTAP== pAdapter->device_mode) ||
              (WLAN_HDD_P2P_GO == pAdapter->device_mode)
            )
    {
        WLANSAP_CancelRemainOnChannel(
                                (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
    }
    else 
=======
         ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode ))
    {
        tANI_U8 sessionId = pAdapter->sessionId;
        if (eHAL_STATUS_SUCCESS !=
                sme_CancelRemainOnChannel( WLAN_HDD_GET_HAL_CTX( pAdapter ),
                                           sessionId ))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    FL("Failed to Cancel Remain on Channel"));
        }
    }
    else if (WLAN_HDD_P2P_GO == pAdapter->device_mode)
    {
        WLANSAP_CancelRemainOnChannel(
                (WLAN_HDD_GET_CTX(pAdapter))->pvosContext);
    }
    else
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    {
       hddLog(VOS_TRACE_LEVEL_ERROR, "%s: Invalid device_mode = %d",
                            __func__, pAdapter->device_mode);
       return -EIO;
    }
<<<<<<< HEAD
    wait_for_completion_interruptible_timeout(&pAdapter->cancel_rem_on_chan_var,
            msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int wlan_hdd_action( struct wiphy *wiphy, struct wireless_dev *wdev,
=======
    status = wait_for_completion_interruptible_timeout(&pAdapter->cancel_rem_on_chan_var,
            msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
    if (0 >= status)
    {
        hddLog( LOGE,
              "%s:wait on cancel_rem_on_chan_var failed %d", __func__, status);
    }
    hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);

    EXIT();
    return 0;
}

int wlan_hdd_cfg80211_cancel_remain_on_channel( struct wiphy *wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                                                struct wireless_dev *wdev,
#else
                                                struct net_device *dev,
#endif
                                                u64 cookie )
{
    int ret;

    vos_ssr_protect(__func__);
    ret = __wlan_hdd_cfg80211_cancel_remain_on_channel(wiphy,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                                                    wdev,
#else
                                                    dev,
#endif
                                                    cookie);
    vos_ssr_unprotect(__func__);

    return ret;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int __wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct wireless_dev *wdev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                     struct ieee80211_channel *chan, bool offchan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid,
#endif
                     unsigned int wait,
                     const u8 *buf, size_t len,  bool no_cck,
                     bool dont_wait_for_ack, u64 *cookie )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
<<<<<<< HEAD
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
=======
int __wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct net_device *dev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                     struct ieee80211_channel *chan, bool offchan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid, unsigned int wait,
                     const u8 *buf, size_t len,  bool no_cck,
                     bool dont_wait_for_ack, u64 *cookie )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
<<<<<<< HEAD
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
=======
int __wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct net_device *dev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                     struct ieee80211_channel *chan, bool offchan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid, unsigned int wait,
                     const u8 *buf, size_t len, u64 *cookie )
#else
<<<<<<< HEAD
int wlan_hdd_action( struct wiphy *wiphy, struct net_device *dev,
=======
int __wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct net_device *dev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                     struct ieee80211_channel *chan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid,
                     const u8 *buf, size_t len, u64 *cookie )
#endif //LINUX_VERSION_CODE
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    struct net_device *dev = wdev->netdev;
#endif
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
<<<<<<< HEAD
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    tANI_U16 extendedWait = 0;
    tANI_U8 type = WLAN_HDD_GET_TYPE_FRM_FC(buf[0]);
    tANI_U8 subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(buf[0]);
    tActionFrmType actionFrmType;
    bool noack = 0;
    int status;
#ifdef WLAN_FEATURE_11W
    tANI_U8 *pTxFrmBuf = (tANI_U8 *) buf; // For SA Query, we have to set protect bit
#endif
=======
    hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
    hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX( pAdapter );
    tANI_U8 type = WLAN_HDD_GET_TYPE_FRM_FC(buf[0]);
    tANI_U8 subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(buf[0]);
    tActionFrmType actionFrmType = WLAN_HDD_ACTION_FRM_TYPE_MAX;
    bool noack = 0;
    int status;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    hdd_adapter_t *goAdapter;
#endif

<<<<<<< HEAD
=======
    ENTER();
    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                      TRACE_CODE_HDD_ACTION, pAdapter->sessionId,
                      pAdapter->device_mode ));
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    status = wlan_hdd_validate_context(pHddCtx);

    if (0 != status)
    {
<<<<<<< HEAD
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: HDD context is not valid", __func__);
        return status;
    }

#ifdef WLAN_FEATURE_P2P_DEBUG
=======
        return status;
    }

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d type: %d",
                            __func__, pAdapter->device_mode, type);


>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    if ((type == SIR_MAC_MGMT_FRAME) &&
            (subType == SIR_MAC_MGMT_ACTION) &&
            (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] == WLAN_HDD_PUBLIC_ACTION_FRAME))
    {
        actionFrmType = buf[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
<<<<<<< HEAD
=======
#ifdef WLAN_FEATURE_P2P_DEBUG
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        if(actionFrmType >= MAX_P2P_ACTION_FRAME_TYPE)
        {
            hddLog(VOS_TRACE_LEVEL_ERROR,"[P2P] unknown[%d] ---> OTA",
                                   actionFrmType);
        }
        else
        {
            hddLog(VOS_TRACE_LEVEL_ERROR,"[P2P] %s ---> OTA",
            p2p_action_frame_type[actionFrmType]);
            if( (actionFrmType == WLAN_HDD_PROV_DIS_REQ) &&
                (globalP2PConnectionStatus == P2P_NOT_ACTIVE) )
            {
                 globalP2PConnectionStatus = P2P_GO_NEG_PROCESS;
                 hddLog(LOGE,"[P2P State]Inactive state to "
                            "GO negotiation progress state");
            }
            else if( (actionFrmType == WLAN_HDD_GO_NEG_CNF) &&
                (globalP2PConnectionStatus == P2P_GO_NEG_PROCESS) )
            {
                 globalP2PConnectionStatus = P2P_GO_NEG_COMPLETED;
                 hddLog(LOGE,"[P2P State]GO nego progress to GO nego"
                             " completed state");
            }
        }
<<<<<<< HEAD
    }
#endif
=======
#endif
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
    noack = dont_wait_for_ack;
#endif

<<<<<<< HEAD
    //If the wait is coming as 0 with off channel set
    //then set the wait to 200 ms
    if (offchan && !wait)
        wait = ACTION_FRAME_DEFAULT_WAIT;

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
                            __func__,pAdapter->device_mode);
=======

    //If the wait is coming as 0 with off channel set
    //then set the wait to 200 ms
    if (offchan && !wait)
    {
        wait = ACTION_FRAME_DEFAULT_WAIT;
        mutex_lock(&pHddCtx->roc_lock);
        pRemainChanCtx = cfgState->remain_on_chan_ctx;
        if (pRemainChanCtx)
        {
            tANI_U32 current_time = vos_timer_get_system_time();
            int remaining_roc_time = ((int) pRemainChanCtx->duration -
                    (current_time - pAdapter->startRocTs));
            if ( remaining_roc_time > ACTION_FRAME_DEFAULT_WAIT)
                wait = remaining_roc_time;
        }
        mutex_unlock(&pHddCtx->roc_lock);
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

    //Call sme API to send out a action frame.
    // OR can we send it directly through data path??
    // After tx completion send tx status back.
    if ( ( WLAN_HDD_SOFTAP == pAdapter->device_mode ) ||
         ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
       )
    {
        if (type == SIR_MAC_MGMT_FRAME)
        {
            if (subType == SIR_MAC_MGMT_PROBE_RSP)
            {
<<<<<<< HEAD
                /* Drop Probe response recieved from supplicant, as for GO and 
                   SAP PE itself sends probe response
                   */ 
=======
                /* Drop Probe response recieved from supplicant, as for GO and
                   SAP PE itself sends probe response
                   */
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                goto err_rem_channel;
            }
            else if ((subType == SIR_MAC_MGMT_DISASSOC) ||
                    (subType == SIR_MAC_MGMT_DEAUTH))
            {
                /* During EAP failure or P2P Group Remove supplicant
                 * is sending del_station command to driver. From
                 * del_station function, Driver will send deauth frame to
                 * p2p client. No need to send disassoc frame from here.
                 * so Drop the frame here and send tx indication back to
                 * supplicant.
                 */
                tANI_U8 dstMac[ETH_ALEN] = {0};
                memcpy(&dstMac, &buf[WLAN_HDD_80211_FRM_DA_OFFSET], ETH_ALEN);
                hddLog(VOS_TRACE_LEVEL_INFO,
                        "%s: Deauth/Disassoc received for STA:"
                        MAC_ADDRESS_STR,
                        __func__,
                        MAC_ADDR_ARRAY(dstMac));
                goto err_rem_channel;
            }
        }
    }

    if( NULL != cfgState->buf )
    {
        if ( !noack )
        {
            hddLog( LOGE, "(%s):Previous P2P Action frame packet pending",
                          __func__);
            hdd_cleanup_actionframe(pAdapter->pHddCtx, pAdapter);
        }
        else
        {
            hddLog( LOGE, "(%s):Pending Action frame packet return EBUSY",
                          __func__);
            return -EBUSY;
        }
    }

<<<<<<< HEAD
    hddLog( LOG1, "Action frame tx request");
=======
    if( subType == SIR_MAC_MGMT_ACTION)
    {
        hddLog( LOG1, "Action frame tx request : %s",
            hdd_getActionString(buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET]));
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    goAdapter = hdd_get_adapter( pAdapter->pHddCtx, WLAN_HDD_P2P_GO );

    //If GO adapter exists and operating on same frequency
    //then we will not request remain on channel
    if( goAdapter && ( ieee80211_frequency_to_channel(chan->center_freq)
                         == goAdapter->sessionCtx.ap.operatingChannel ) )
    {
<<<<<<< HEAD
=======
        /*  if GO exist and is not off channel
         *  wait time should be zero.
         */
        wait = 0;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        goto send_frame;
    }
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    if( offchan && wait)
    {
        int status;
<<<<<<< HEAD

        // In case of P2P Client mode if we are already
        // on the same channel then send the frame directly

        if((cfgState->remain_on_chan_ctx != NULL) &&
           (cfgState->current_freq == chan->center_freq)
          )
        {
            hddLog(LOG1,"action frame: extending the wait time\n");
            extendedWait = (tANI_U16)wait;
            goto send_frame;
        }

        INIT_COMPLETION(pAdapter->offchannel_tx_event);

=======
        rem_on_channel_request_type_t req_type = OFF_CHANNEL_ACTION_TX;
        // In case of P2P Client mode if we are already
        // on the same channel then send the frame directly
        mutex_lock(&pHddCtx->roc_lock);
        pRemainChanCtx = cfgState->remain_on_chan_ctx;
        if( (pRemainChanCtx != NULL) &&
            (cfgState->current_freq == chan->center_freq)
          )
        {
            if ( VOS_TIMER_STATE_RUNNING == vos_timer_getCurrentState(
                      &pRemainChanCtx->hdd_remain_on_chan_timer) )
            {
               /* Some times FW is taking almost 500 msec for
                * full 15 retries, which leads to ROC expiration
                * by the time peer gets response from other peer.
                * Therefore as part of temporary fix , in host
                * ROC time is extended. For frames where we are
                * expecting response from peer , its extended by
                * 500 msec to make ROC wait time as 1 sec and
                * in other cases its extended by 300 msec to make
                * total ROC wait as 500 msec.
                * TODO: FW needs to fix as why 15 retry is taking
                *       such long time.
                */
                if ( actionFrmType == WLAN_HDD_INVITATION_REQ ||
                     actionFrmType == WLAN_HDD_GO_NEG_REQ ||
                     actionFrmType == WLAN_HDD_GO_NEG_RESP )
                   wait = wait + ACTION_FRAME_RSP_WAIT;
                else if ( actionFrmType == WLAN_HDD_GO_NEG_CNF ||
                          actionFrmType == WLAN_HDD_INVITATION_RESP )
                   wait = wait + ACTION_FRAME_ACK_WAIT;
                vos_timer_stop(
                      &pRemainChanCtx->hdd_remain_on_chan_timer);
                status = vos_timer_start(
                       &pRemainChanCtx->hdd_remain_on_chan_timer,
                       wait);

                mutex_unlock(&pHddCtx->roc_lock);

                hddLog(VOS_TRACE_LEVEL_INFO,
                   "action frame: extending the wait time %u",
                   wait);

                if ( status != VOS_STATUS_SUCCESS )
                {
                    hddLog( LOGE, "Remain on Channel timer start failed");
                }
                goto send_frame;
            }
            else
            {
                if ( TRUE ==
                     pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress )
                {
                    mutex_unlock(&pHddCtx->roc_lock);
                    hddLog(VOS_TRACE_LEVEL_INFO,
                           "action frame tx: waiting for completion of ROC ");

                    status = wait_for_completion_interruptible_timeout(
                                       &pAdapter->cancel_rem_on_chan_var,
                                       msecs_to_jiffies(WAIT_CANCEL_REM_CHAN));
                    if (0 >= status)
                    {
                        hddLog( LOGE,
                               "%s:wait on cancel_rem_on_chan_var failed %d",
                                __func__, status);
                    }
                    goto bypass_lock;
                }
            }
        }
        mutex_unlock(&pHddCtx->roc_lock);
bypass_lock:
        hddLog(VOS_TRACE_LEVEL_INFO,
               "action frame: Request ROC for wait time %u", wait);
        INIT_COMPLETION(pAdapter->offchannel_tx_event);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        status = wlan_hdd_request_remain_on_channel(wiphy, dev,
                                        chan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                                        channel_type,
#endif
                                        wait, cookie,
<<<<<<< HEAD
                                        OFF_CHANNEL_ACTION_TX);
=======
                                        req_type);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

        if(0 != status)
        {
            if( (-EBUSY == status) &&
                (cfgState->current_freq == chan->center_freq) )
            {
                goto send_frame;
            }
            goto err_rem_channel;
        }
<<<<<<< HEAD
        /* This will extend timer in LIM when sending Any action frame
         * It will cover remain on channel timer till next action frame
         * in rx direction.
         */
        extendedWait = (tANI_U16)wait;
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        /* Wait for driver to be ready on the requested channel */
        status = wait_for_completion_interruptible_timeout(
                     &pAdapter->offchannel_tx_event,
                     msecs_to_jiffies(WAIT_CHANGE_CHANNEL_FOR_OFFCHANNEL_TX));
<<<<<<< HEAD
        if(!status)
        {
            hddLog( LOGE, "Not able to complete remain on channel request"
                          " within timeout period");
=======
        if(0 >= status)
        {
            hddLog( LOGE, "wait on offchannel_tx_event failed %d", status);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            goto err_rem_channel;
        }
    }
    else if ( offchan )
    {
        /* Check before sending action frame
           whether we already remain on channel */
        if(NULL == cfgState->remain_on_chan_ctx)
        {
            goto err_rem_channel;
        }
    }
    send_frame:
#endif

    if(!noack)
    {
        cfgState->buf = vos_mem_malloc( len ); //buf;
        if( cfgState->buf == NULL )
            return -ENOMEM;

        cfgState->len = len;

        vos_mem_copy( cfgState->buf, buf, len);

<<<<<<< HEAD
=======
        mutex_lock(&pHddCtx->roc_lock);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
        if( cfgState->remain_on_chan_ctx )
        {
            cfgState->action_cookie = cfgState->remain_on_chan_ctx->cookie;
            *cookie = cfgState->action_cookie;
        }
        else
        {
#endif
<<<<<<< HEAD
            *cookie = (tANI_U32) cfgState->buf;
=======
            *cookie = (uintptr_t) cfgState->buf;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            cfgState->action_cookie = *cookie;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
        }
#endif
<<<<<<< HEAD
    } 
=======
        mutex_unlock(&pHddCtx->roc_lock);
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

    if ( (WLAN_HDD_INFRA_STATION == pAdapter->device_mode) ||
         (WLAN_HDD_P2P_CLIENT == pAdapter->device_mode) ||
         ( WLAN_HDD_P2P_DEVICE == pAdapter->device_mode )
       )
    {
        tANI_U8 sessionId = pAdapter->sessionId;

        if ((type == SIR_MAC_MGMT_FRAME) &&
                (subType == SIR_MAC_MGMT_ACTION) &&
                (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] == WLAN_HDD_PUBLIC_ACTION_FRAME))
        {
            actionFrmType = buf[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
<<<<<<< HEAD
            hddLog(LOG1, "Tx Action Frame %u \n", actionFrmType);
            if (actionFrmType == WLAN_HDD_PROV_DIS_REQ)
            {
                cfgState->actionFrmState = HDD_PD_REQ_ACK_PENDING;
                hddLog(LOG1, "%s: HDD_PD_REQ_ACK_PENDING \n", __func__);
=======
            hddLog(LOG1, "Tx Action Frame %u.", actionFrmType);
            if (actionFrmType == WLAN_HDD_PROV_DIS_REQ)
            {
                cfgState->actionFrmState = HDD_PD_REQ_ACK_PENDING;
                hddLog(LOG1, "%s: HDD_PD_REQ_ACK_PENDING.", __func__);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            }
            else if (actionFrmType == WLAN_HDD_GO_NEG_REQ)
            {
                cfgState->actionFrmState = HDD_GO_NEG_REQ_ACK_PENDING;
<<<<<<< HEAD
                hddLog(LOG1, "%s: HDD_GO_NEG_REQ_ACK_PENDING \n", __func__);
            }
        }
#ifdef WLAN_FEATURE_11W
        if ((type == SIR_MAC_MGMT_FRAME) &&
                (subType == SIR_MAC_MGMT_ACTION) &&
                (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] == WLAN_HDD_SA_QUERY_ACTION_FRAME))
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                     "%s: Calling sme_sendAction. For Category %s", __func__, "SA Query");
            // Since this is an SA Query Action Frame, we have to protect it
            WLAN_HDD_SET_WEP_FRM_FC(pTxFrmBuf[1]);
        }
#endif
        if (eHAL_STATUS_SUCCESS !=
               sme_sendAction( WLAN_HDD_GET_HAL_CTX(pAdapter),
                               sessionId, buf, len, extendedWait, noack))
=======
                hddLog(LOG1, "%s: HDD_GO_NEG_REQ_ACK_PENDING.", __func__);
            }
        }

        if (eHAL_STATUS_SUCCESS !=
               sme_sendAction( WLAN_HDD_GET_HAL_CTX(pAdapter),
                               sessionId, buf, len, wait, noack))
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                     "%s: sme_sendAction returned fail", __func__);
            goto err;
        }
    }
<<<<<<< HEAD
    else if( ( WLAN_HDD_SOFTAP== pAdapter->device_mode ) ||
              ( WLAN_HDD_P2P_GO == pAdapter->device_mode )
            )
=======
    else if (WLAN_HDD_P2P_GO == pAdapter->device_mode)
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
     {
        if( VOS_STATUS_SUCCESS !=
             WLANSAP_SendAction( (WLAN_HDD_GET_CTX(pAdapter))->pvosContext,
                                  buf, len, 0 ) )
        {
            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                    "%s: WLANSAP_SendAction returned fail", __func__);
            goto err;
        }
    }

    return 0;
err:
    if(!noack)
    {
       hdd_sendActionCnf( pAdapter, FALSE );
    }
    return 0;
err_rem_channel:
<<<<<<< HEAD
    *cookie = (tANI_U32)cfgState;
=======
    *cookie = (uintptr_t)cfgState;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    cfg80211_mgmt_tx_status(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                            pAdapter->dev->ieee80211_ptr,
#else
                            pAdapter->dev,
#endif
                            *cookie, buf, len, FALSE, GFP_KERNEL );
<<<<<<< HEAD
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                          struct wireless_dev *wdev,
                                          u64 cookie)
{
    return wlan_hdd_cfg80211_cancel_remain_on_channel( wiphy, wdev, cookie );
}
#else
int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                          struct net_device *dev,
                                          u64 cookie)
{
=======
    EXIT();
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
                     struct cfg80211_mgmt_tx_params *params, u64 *cookie)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct wireless_dev *wdev,
                     struct ieee80211_channel *chan, bool offchan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid,
#endif
                     unsigned int wait,
                     const u8 *buf, size_t len,  bool no_cck,
                     bool dont_wait_for_ack, u64 *cookie )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
int wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan, bool offchan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid, unsigned int wait,
                     const u8 *buf, size_t len,  bool no_cck,
                     bool dont_wait_for_ack, u64 *cookie )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
int wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan, bool offchan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid, unsigned int wait,
                     const u8 *buf, size_t len, u64 *cookie )
#else
int wlan_hdd_mgmt_tx( struct wiphy *wiphy, struct net_device *dev,
                     struct ieee80211_channel *chan,
                     enum nl80211_channel_type channel_type,
                     bool channel_type_valid,
                     const u8 *buf, size_t len, u64 *cookie )
#endif //LINUX_VERSION_CODE
{
    int ret;

    vos_ssr_protect(__func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
    ret =  __wlan_hdd_mgmt_tx(wiphy, wdev, params->chan, params->offchan,
                              params->wait, params->buf, params->len,
                              params->no_cck, params->dont_wait_for_ack,
                              cookie);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    ret =  __wlan_hdd_mgmt_tx(wiphy, wdev,
                              chan, offchan,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
                              channel_type,
                              channel_type_valid,
#endif
                              wait,
                              buf, len, no_cck,
                              dont_wait_for_ack, cookie);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0))
    ret = __wlan_hdd_mgmt_tx(wiphy, dev, chan, offchan,
                             channel_type, channel_type_valid, wait,
                             buf, len,  no_cck,
                             dont_wait_for_ack, cookie);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    ret = __wlan_hdd_mgmt_tx(wiphy, dev, chan, offchan,
                     channel_type, channel_type_valid, wait,
                     buf, len, cookie);
#else
    ret = __wlan_hdd_mgmt_tx(wiphy, dev, chan, channel_type,
                             channel_type_valid, buf, len, cookie);
#endif //LINUX_VERSION_CODE
    vos_ssr_unprotect(__func__);

    return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                          struct wireless_dev *wdev,
                                          u64 cookie)
{
    u64 cookie_dummy;
    cookie_dummy = cookie << 32;
    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                     TRACE_CODE_HDD_MGMT_TX_CANCEL_WAIT, NO_SESSION, cookie_dummy));
    return wlan_hdd_cfg80211_cancel_remain_on_channel( wiphy, wdev, cookie );
}
#else
int __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                          struct net_device *dev,
                                          u64 cookie)
{
    u64 cookie_dummy;
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    cookie_dummy = cookie << 32;
    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                     TRACE_CODE_HDD_MGMT_TX_CANCEL_WAIT,
                     pAdapter->sessionId, cookie_dummy));
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    return wlan_hdd_cfg80211_cancel_remain_on_channel( wiphy, dev, cookie );
}
#endif
#endif

<<<<<<< HEAD
=======
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                          struct wireless_dev *wdev,
                                          u64 cookie)
{
    int ret;

    vos_ssr_protect(__func__);
    ret = __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(wiphy, wdev, cookie);
    vos_ssr_unprotect(__func__);

    return ret;
}
#else
int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
                                          struct net_device *dev,
                                          u64 cookie)
{
    int ret;

    vos_ssr_protect(__func__);
    ret = __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(wiphy, dev, cookie);
    vos_ssr_unprotect(__func__);

    return ret;
}
#endif
#endif

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
void hdd_sendActionCnf( hdd_adapter_t *pAdapter, tANI_BOOLEAN actionSendSuccess )
{
    hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );

    cfgState->actionFrmState = HDD_IDLE;

    hddLog( LOG1, "Send Action cnf, actionSendSuccess %d", actionSendSuccess);
    if( NULL == cfgState->buf )
    {
        return;
    }

    /* If skb is NULL it means this packet was received on CFG80211 interface
     * else it was received on Monitor interface */
    if( cfgState->skb == NULL )
    {
        /*
         * buf is the same pointer it passed us to send. Since we are sending
         * it through control path, we use different buffers.
         * In case of mac80211, they just push it to the skb and pass the same
         * data while sending tx ack status.
         * */
         cfg80211_mgmt_tx_status(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
                pAdapter->dev->ieee80211_ptr,
#else
                pAdapter->dev,
#endif
                cfgState->action_cookie,
                cfgState->buf, cfgState->len, actionSendSuccess, GFP_KERNEL );
         vos_mem_free( cfgState->buf );
         cfgState->buf = NULL;
    }
    else
    {
        hdd_adapter_t* pMonAdapter =
                    hdd_get_adapter( pAdapter->pHddCtx, WLAN_HDD_MONITOR );
        if( pMonAdapter == NULL )
        {
            hddLog( LOGE, "Not able to get Monitor Adapter");
            cfgState->skb = NULL;
            vos_mem_free( cfgState->buf );
            cfgState->buf = NULL;
            complete(&pAdapter->tx_action_cnf_event);
            return;
        }
        /* Send TX completion feedback over monitor interface. */
        hdd_wlan_tx_complete( pMonAdapter, cfgState, actionSendSuccess );
        cfgState->skb = NULL;
        vos_mem_free( cfgState->buf );
        cfgState->buf = NULL;
        /* Look for the next Mgmt packet to TX */
        hdd_mon_tx_mgmt_pkt(pAdapter);
    }
    complete(&pAdapter->tx_action_cnf_event);
}

/**
 * hdd_setP2pNoa
 *
 *FUNCTION:
<<<<<<< HEAD
 * This function is called from hdd_hostapd_ioctl function when Driver 
 * get P2P_SET_NOA comand from wpa_supplicant using private ioctl
 *
 *LOGIC:
 * Fill NoA Struct According to P2P Power save Option and Pass it to SME layer 
=======
 * This function is called from hdd_hostapd_ioctl function when Driver
 * get P2P_SET_NOA comand from wpa_supplicant using private ioctl
 *
 *LOGIC:
 * Fill NoA Struct According to P2P Power save Option and Pass it to SME layer
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 *
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  dev         Pointer to net device structure
<<<<<<< HEAD
 * @param  command     Pointer to command 
=======
 * @param  command     Pointer to command
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
 *
 * @return Status
 */

int hdd_setP2pNoa( struct net_device *dev, tANI_U8 *command )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tP2pPsConfig NoA;
    int count, duration, start_time;
    char *param;
    tANI_U8 ret = 0;

    param = strnchr(command, strlen(command), ' ');
    if (param == NULL)
<<<<<<< HEAD
      return -EINVAL;
=======
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: strnchr failed to find delimeter",__func__);
          return -EINVAL;
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    param++;
    ret = sscanf(param, "%d %d %d", &count, &start_time, &duration);
    if (ret < 3)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: P2P_SET GO NoA: fail to read param "
<<<<<<< HEAD
               "count=%d duration=%d interval=%d \n",
=======
               "count=%d duration=%d interval=%d ",
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                __func__, count, start_time, duration);
        return -EINVAL;
    }
    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
<<<<<<< HEAD
               "%s: P2P_SET GO NoA: count=%d duration=%d interval=%d \n",
=======
               "%s: P2P_SET GO NoA: count=%d duration=%d interval=%d",
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                __func__, count, start_time, duration);
    duration = MS_TO_MUS(duration);
    /* PS Selection
     * Periodic NoA (2)
     * Single NOA   (4)
     */
    NoA.opp_ps = 0;
    NoA.ctWindow = 0;
    if (count == 1)
    {
        NoA.duration = 0;
        NoA.single_noa_duration = duration;
        NoA.psSelection = P2P_POWER_SAVE_TYPE_SINGLE_NOA;
    }
    else
    {
        NoA.duration = duration;
        NoA.single_noa_duration = 0;
        NoA.psSelection = P2P_POWER_SAVE_TYPE_PERIODIC_NOA;
    }
    NoA.interval = MS_TO_MUS(100);
    NoA.count = count;
    NoA.sessionid = pAdapter->sessionId;

    VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                "%s: P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
                "interval %d count %d single noa duration %d "
<<<<<<< HEAD
                "PsSelection %x \n", __func__, NoA.opp_ps, 
=======
                "PsSelection %x", __func__, NoA.opp_ps,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                NoA.ctWindow, NoA.duration, NoA.interval, 
                NoA.count, NoA.single_noa_duration,
                NoA.psSelection);

    sme_p2pSetPs(hHal, &NoA);
    return status;
}

/**
 * hdd_setP2pOpps
 *
 *FUNCTION:
 * This function is called from hdd_hostapd_ioctl function when Driver
 * get P2P_SET_PS comand from wpa_supplicant using private ioctl
 *
 *LOGIC:
 * Fill NoA Struct According to P2P Power save Option and Pass it to SME layer
 *
 *ASSUMPTIONS:
 *
 *
 *NOTE:
 *
 * @param  dev         Pointer to net device structure
 * @param  command     Pointer to command
 *
 * @return Status
 */

int hdd_setP2pOpps( struct net_device *dev, tANI_U8 *command )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tP2pPsConfig NoA;
    char *param;
    int legacy_ps, opp_ps, ctwindow;
    tANI_U8 ret = 0;

    param = strnchr(command, strlen(command), ' ');
    if (param == NULL)
<<<<<<< HEAD
        return -EINVAL;
=======
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
               "%s: strnchr failed to find delimeter",__func__);
        return -EINVAL;
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    param++;
    ret = sscanf(param, "%d %d %d", &legacy_ps, &opp_ps, &ctwindow);
    if (ret < 3)
    {
        VOS_TRACE (VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                 "%s: P2P_SET GO PS: fail to read param "
<<<<<<< HEAD
                 " legacy_ps=%d opp_ps=%d ctwindow=%d \n",
=======
                 " legacy_ps=%d opp_ps=%d ctwindow=%d ",
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                 __func__, legacy_ps, opp_ps, ctwindow);
        return -EINVAL;
    }
    VOS_TRACE (VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
<<<<<<< HEAD
                 "%s: P2P_SET GO PS: legacy_ps=%d opp_ps=%d ctwindow=%d \n",
=======
                 "%s: P2P_SET GO PS: legacy_ps=%d opp_ps=%d ctwindow=%d",
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                 __func__, legacy_ps, opp_ps, ctwindow);

    /* PS Selection
     * Opportunistic Power Save (1)
     */

    /* From wpa_cli user need to use separate command to set ctWindow and Opps
     * When user want to set ctWindow during that time other parameters
<<<<<<< HEAD
     * values are coming from wpa_supplicant as -1. 
     * Example : User want to set ctWindow with 30 then wpa_cli command : 
     * P2P_SET ctwindow 30 
     * Command Received at hdd_hostapd_ioctl is as below: 
     * P2P_SET_PS -1 -1 30 (legacy_ps = -1, opp_ps = -1, ctwindow = 30)
     */ 
=======
     * values are coming from wpa_supplicant as -1.
     * Example : User want to set ctWindow with 30 then wpa_cli command :
     * P2P_SET ctwindow 30
     * Command Received at hdd_hostapd_ioctl is as below:
     * P2P_SET_PS -1 -1 30 (legacy_ps = -1, opp_ps = -1, ctwindow = 30)
     */
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    if (ctwindow != -1)
    {

        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
<<<<<<< HEAD
                    "Opportunistic Power Save is %s \n", 
=======
                    "Opportunistic Power Save is %s",
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                    (TRUE == pAdapter->ops) ? "Enable" : "Disable" );

        if (ctwindow != pAdapter->ctw)
        {
            pAdapter->ctw = ctwindow;
<<<<<<< HEAD
        
=======

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            if(pAdapter->ops)
            {
                NoA.opp_ps = pAdapter->ops;
                NoA.ctWindow = pAdapter->ctw;
                NoA.duration = 0;
                NoA.single_noa_duration = 0;
                NoA.interval = 0;
                NoA.count = 0;
                NoA.psSelection = P2P_POWER_SAVE_TYPE_OPPORTUNISTIC;
                NoA.sessionid = pAdapter->sessionId;
<<<<<<< HEAD
 
                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                            "%s: P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
                            "interval %d count %d single noa duration %d "
                            "PsSelection %x \n", __func__, NoA.opp_ps, 
=======

                VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                            "%s: P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
                            "interval %d count %d single noa duration %d "
                            "PsSelection %x", __func__, NoA.opp_ps,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                            NoA.ctWindow, NoA.duration, NoA.interval, 
                            NoA.count, NoA.single_noa_duration,
                            NoA.psSelection);

               sme_p2pSetPs(hHal, &NoA);
           }
           return 0;
        }
    }

    if (opp_ps != -1)
    {
        pAdapter->ops = opp_ps;

<<<<<<< HEAD
        if ((opp_ps != -1) && (pAdapter->ctw)) 
=======
        if ((opp_ps != -1) && (pAdapter->ctw))
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        {
            NoA.opp_ps = opp_ps;
            NoA.ctWindow = pAdapter->ctw;
            NoA.duration = 0;
            NoA.single_noa_duration = 0;
            NoA.interval = 0;
            NoA.count = 0;
            NoA.psSelection = P2P_POWER_SAVE_TYPE_OPPORTUNISTIC;
            NoA.sessionid = pAdapter->sessionId;

            VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                        "%s: P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
                        "interval %d count %d single noa duration %d "
<<<<<<< HEAD
                        "PsSelection %x \n", __func__, NoA.opp_ps, 
=======
                        "PsSelection %x", __func__, NoA.opp_ps,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                        NoA.ctWindow, NoA.duration, NoA.interval, 
                        NoA.count, NoA.single_noa_duration,
                        NoA.psSelection);

           sme_p2pSetPs(hHal, &NoA);
        }
    }
    return status;
}

int hdd_setP2pPs( struct net_device *dev, void *msgData )
{
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
    VOS_STATUS status = VOS_STATUS_SUCCESS;
    tP2pPsConfig NoA;
    p2p_app_setP2pPs_t *pappNoA = (p2p_app_setP2pPs_t *) msgData;

    NoA.opp_ps = pappNoA->opp_ps;
    NoA.ctWindow = pappNoA->ctWindow;
    NoA.duration = pappNoA->duration;
    NoA.interval = pappNoA->interval;
    NoA.count = pappNoA->count;
    NoA.single_noa_duration = pappNoA->single_noa_duration;
    NoA.psSelection = pappNoA->psSelection;
    NoA.sessionid = pAdapter->sessionId;

    sme_p2pSetPs(hHal, &NoA);
    return status;
}

static tANI_U8 wlan_hdd_get_session_type( enum nl80211_iftype type )
{
    tANI_U8 sessionType;

    switch( type )
    {
        case NL80211_IFTYPE_AP:
            sessionType = WLAN_HDD_SOFTAP;
            break;
        case NL80211_IFTYPE_P2P_GO:
            sessionType = WLAN_HDD_P2P_GO;
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
            sessionType = WLAN_HDD_P2P_CLIENT;
            break;
        case NL80211_IFTYPE_STATION:
            sessionType = WLAN_HDD_INFRA_STATION;
            break;
        case NL80211_IFTYPE_MONITOR:
            sessionType = WLAN_HDD_MONITOR;
            break;
        default:
            sessionType = WLAN_HDD_INFRA_STATION;
            break;
    }

    return sessionType;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
<<<<<<< HEAD
struct wireless_dev* wlan_hdd_add_virtual_intf(
=======
struct wireless_dev* __wlan_hdd_add_virtual_intf(
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                  struct wiphy *wiphy, const char *name,
                  enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
<<<<<<< HEAD
struct wireless_dev* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
struct net_device* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#else
int wlan_hdd_add_virtual_intf( struct wiphy *wiphy, char *name,
=======
struct wireless_dev* __wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
struct net_device* __wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#else
int __wlan_hdd_add_virtual_intf( struct wiphy *wiphy, char *name,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                               enum nl80211_iftype type,
                               u32 *flags, struct vif_params *params )
#endif
{
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
<<<<<<< HEAD
    hdd_adapter_t* pAdapter = NULL;

    ENTER();

    if(hdd_get_adapter(pHddCtx, wlan_hdd_get_session_type(type)) != NULL)
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Interface type %d already exists. Two"
                     "interfaces of same type are not supported currently.",__func__, type);
       return NULL;
    }

    if (pHddCtx->isLogpInProgress)
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                "%s:LOGP in Progress. Ignore!!!", __func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
       return NULL;
=======
    hdd_adapter_t *pAdapter = NULL;
    hdd_scaninfo_t *pScanInfo = NULL;
    int ret = 0;

    ENTER();

    ret = wlan_hdd_validate_context(pHddCtx);
    if (0 != ret)
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
       return ERR_PTR(-EINVAL);
#else
       return -EAGAIN;
#endif
    }

    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                     TRACE_CODE_HDD_ADD_VIRTUAL_INTF, NO_SESSION, type));
    if (WLAN_HDD_P2P_CLIENT != wlan_hdd_get_session_type(type) &&
            WLAN_HDD_INFRA_STATION != wlan_hdd_get_session_type(type) &&
            hdd_get_adapter(pHddCtx, wlan_hdd_get_session_type(type)) != NULL)
    {
       hddLog(VOS_TRACE_LEVEL_ERROR,"%s: Interface type %d already exists. Two"
                     "interfaces of same type are not supported currently.",__func__, type);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
       return ERR_PTR(-EINVAL);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#else
       return -EAGAIN;
#endif
    }
<<<<<<< HEAD
    if ( pHddCtx->cfg_ini->isP2pDeviceAddrAdministrated )
    {
        if( (NL80211_IFTYPE_P2P_GO == type) || 
            (NL80211_IFTYPE_P2P_CLIENT == type) )
        {
=======

    pAdapter = hdd_get_adapter(pHddCtx, WLAN_HDD_INFRA_STATION);
    pScanInfo =  &pHddCtx->scan_info;
    if ((pScanInfo != NULL) && (pAdapter != NULL) &&
        (pHddCtx->scan_info.mScanPending))
    {
        hdd_abort_mac_scan(pHddCtx, pAdapter->sessionId,
                           eCSR_SCAN_ABORT_DEFAULT);
        hddLog(VOS_TRACE_LEVEL_INFO,
               "%s: Abort Scan while adding virtual interface",__func__);
    }

    pAdapter = NULL;
    if (pHddCtx->cfg_ini->isP2pDeviceAddrAdministrated &&
        ((NL80211_IFTYPE_P2P_GO == type) ||
         (NL80211_IFTYPE_P2P_CLIENT == type)))
    {
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            /* Generate the P2P Interface Address. this address must be
             * different from the P2P Device Address.
             */
            v_MACADDR_t p2pDeviceAddress = pHddCtx->p2pDeviceAddress;
            p2pDeviceAddress.bytes[4] ^= 0x80;
<<<<<<< HEAD
            pAdapter = hdd_open_adapter( pHddCtx, 
                                         wlan_hdd_get_session_type(type),
                                         name, p2pDeviceAddress.bytes,
                                         VOS_TRUE );
        }
=======
            pAdapter = hdd_open_adapter( pHddCtx,
                                         wlan_hdd_get_session_type(type),
                                         name, p2pDeviceAddress.bytes,
                                         VOS_TRUE );
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    }
    else
    {
       pAdapter = hdd_open_adapter( pHddCtx, wlan_hdd_get_session_type(type),
                          name, wlan_hdd_get_intf_addr(pHddCtx), VOS_TRUE );
    }

    if( NULL == pAdapter)
    {
        hddLog(VOS_TRACE_LEVEL_ERROR,"%s: hdd_open_adapter failed",__func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
<<<<<<< HEAD
        return NULL;
=======
        return ERR_PTR(-EINVAL);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#else
        return -EINVAL;
#endif
    }
<<<<<<< HEAD
=======

    if ((type == NL80211_IFTYPE_P2P_CLIENT) ||
          (type == NL80211_IFTYPE_P2P_GO))
    {
        /* Below function Notifies Mode change and
         * If p2p session is detected then invokes functionality to
         * Teardown TDLS links and disable offchannel if any. Since
         * TDLS is not supported in case of concurrency.
         */
        hddLog(LOG1, FL("Interface type = %d"), type);
        hdd_tdls_notify_mode_change(pAdapter, pHddCtx);
    }

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    EXIT();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    return pAdapter->dev->ieee80211_ptr;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    return pAdapter->dev;
#else
    return 0;
#endif
}

<<<<<<< HEAD
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct wireless_dev *wdev )
#else
int wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct net_device *dev )
=======
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0))
struct wireless_dev* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, const char *name,
                  enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
struct wireless_dev* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
struct net_device* wlan_hdd_add_virtual_intf(
                  struct wiphy *wiphy, char *name, enum nl80211_iftype type,
                  u32 *flags, struct vif_params *params )
#else
int wlan_hdd_add_virtual_intf( struct wiphy *wiphy, char *name,
                               enum nl80211_iftype type,
                               u32 *flags, struct vif_params *params )
#endif
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)))
    struct wireless_dev* wdev;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    struct net_device* ndev;
#else
    int ret;
#endif
    vos_ssr_protect(__func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    wdev = __wlan_hdd_add_virtual_intf(wiphy, name, type, flags, params);
    vos_ssr_unprotect(__func__);
    return wdev;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
    ndev = __wlan_hdd_add_virtual_intf(wiphy, name, type, flags, params);
    vos_ssr_unprotect(__func__);
    return ndev;
#else
    ret  = __wlan_hdd_add_virtual_intf(wiphy, name, type, flags, params);
    vos_ssr_unprotect(__func__);
    return ret;
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int __wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct wireless_dev *wdev )
#else
int __wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct net_device *dev )
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    struct net_device *dev = wdev->netdev;
#endif
    hdd_context_t *pHddCtx = (hdd_context_t*) wiphy_priv(wiphy);
<<<<<<< HEAD
    hdd_adapter_t *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    int status;
    ENTER();

=======
    hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR( dev );
    hdd_adapter_t *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
    int status;

    ENTER();

    MTRACE(vos_trace(VOS_MODULE_ID_HDD,
                     TRACE_CODE_HDD_DEL_VIRTUAL_INTF,
                     pAdapter->sessionId, pAdapter->device_mode));
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    hddLog(VOS_TRACE_LEVEL_INFO, "%s: device_mode = %d",
           __func__,pVirtAdapter->device_mode);

    status = wlan_hdd_validate_context(pHddCtx);

    if (0 != status)
    {
<<<<<<< HEAD
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                   "%s: HDD context is not valid", __func__);
=======
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        return status;
    }

    wlan_hdd_release_intf_addr( pHddCtx,
                                 pVirtAdapter->macAddressCurrent.bytes );

<<<<<<< HEAD
    hdd_stop_adapter( pHddCtx, pVirtAdapter );
=======
    hdd_stop_adapter( pHddCtx, pVirtAdapter, VOS_TRUE);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    hdd_close_adapter( pHddCtx, pVirtAdapter, TRUE );
    EXIT();
    return 0;
}

<<<<<<< HEAD
void hdd_sendMgmtFrameOverMonitorIface( hdd_adapter_t *pMonAdapter,
                                        tANI_U32 nFrameLength,
                                        tANI_U8* pbFrames,
                                        tANI_U8 frameType )  
=======
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
int wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct wireless_dev *wdev )
#else
int wlan_hdd_del_virtual_intf( struct wiphy *wiphy, struct net_device *dev )
#endif
{
    int ret;

    vos_ssr_protect(__func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
    ret = __wlan_hdd_del_virtual_intf(wiphy, wdev);
#else
    ret = __wlan_hdd_del_virtual_intf(wiphy, dev);
#endif
    vos_ssr_unprotect(__func__);

    return ret;
}

void hdd_sendMgmtFrameOverMonitorIface( hdd_adapter_t *pMonAdapter,
                                        tANI_U32 nFrameLength,
                                        tANI_U8* pbFrames,
                                        tANI_U8 frameType )
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
{
    //Indicate a Frame over Monitor Intf.
    int rxstat;
    struct sk_buff *skb = NULL;
    int needed_headroom = 0;
    int flag = HDD_RX_FLAG_IV_STRIPPED | HDD_RX_FLAG_DECRYPTED |
               HDD_RX_FLAG_MMIC_STRIPPED;
#ifdef WLAN_OPEN_SOURCE
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
    hdd_context_t* pHddCtx = (hdd_context_t*)(pMonAdapter->pHddCtx);
#endif
#endif
    hddLog( LOG1, FL("Indicate Frame over Monitor Intf"));

    if (NULL == pbFrames)
    {
        hddLog(LOGE, FL("NULL frame pointer"));
        return;
    }

    /* room for the radiotap header based on driver features
     * 1 Byte for RADIO TAP Flag, 1 Byte padding and 2 Byte for
     * RX flags.
     * */
     needed_headroom = sizeof(struct ieee80211_radiotap_header) + 4;

     //alloc skb  here
     skb = alloc_skb(VPKT_SIZE_BUFFER, GFP_ATOMIC);
     if (unlikely(NULL == skb))
     {
         hddLog( LOGW, FL("Unable to allocate skb"));
         return;
     }
     skb_reserve(skb, VPKT_SIZE_BUFFER);
     if (unlikely(skb_headroom(skb) < nFrameLength))
     {
         VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                   "HDD [%d]: Insufficient headroom, "
                   "head[%p], data[%p], req[%d]",
                   __LINE__, skb->head, skb->data, nFrameLength);
         kfree_skb(skb);
         return ;
     }
     // actually push the data
     memcpy(skb_push(skb, nFrameLength), pbFrames, nFrameLength);
     /* prepend radiotap information */
     if( 0 != hdd_wlan_add_rx_radiotap_hdr( skb, needed_headroom, flag ) )
     {
         hddLog( LOGE, FL("Not Able Add Radio Tap"));
         //free skb
         kfree_skb(skb);
         return ;
     }

     skb_reset_mac_header( skb );
     skb->dev = pMonAdapter->dev;
     skb->protocol = eth_type_trans( skb, skb->dev );
     skb->ip_summed = CHECKSUM_NONE;
#ifdef WLAN_OPEN_SOURCE
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
<<<<<<< HEAD
     wake_lock_timeout(&pHddCtx->rx_wake_lock, msecs_to_jiffies(HDD_WAKE_LOCK_DURATION));
=======
     vos_wake_lock_timeout_release(&pHddCtx->rx_wake_lock,
                                   HDD_WAKE_LOCK_DURATION,
                                   WIFI_POWER_EVENT_WAKELOCK_HOLD_RX);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#endif
#endif
     rxstat = netif_rx_ni(skb);
     if( NET_RX_SUCCESS == rxstat )
     {
         hddLog( LOG1, FL("Success"));
     }
     else
<<<<<<< HEAD
         hddLog( LOGE, FL("Failed %d"), rxstat);                   
=======
         hddLog( LOGE, FL("Failed %d"), rxstat);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

     return ;
}

<<<<<<< HEAD
void hdd_indicateMgmtFrame( hdd_adapter_t *pAdapter,
                            tANI_U32 nFrameLength, 
                            tANI_U8* pbFrames,
                            tANI_U8 frameType,
                            tANI_U32 rxChan,
                            tANI_S8 rxRssi )
{
    tANI_U16 freq;
=======
void __hdd_indicate_mgmt_frame(hdd_adapter_t *pAdapter,
                            tANI_U32 nFrameLength,
                            tANI_U8* pbFrames,
                            tANI_U8 frameType,
                            tANI_U32 rxChan,
                            tANI_S8 rxRssi)
{
    tANI_U16 freq;
    tANI_U16 extend_time;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    tANI_U8 type = 0;
    tANI_U8 subType = 0;
    tActionFrmType actionFrmType;
    hdd_cfg80211_state_t *cfgState = NULL;
<<<<<<< HEAD

    hddLog(VOS_TRACE_LEVEL_INFO, "%s: Frame Type = %d Frame Length = %d\n",
            __func__, frameType, nFrameLength);
=======
    hdd_scaninfo_t *pScanInfo = NULL;
    hdd_context_t *pHddCtx = NULL;
    VOS_STATUS status;
    hdd_remain_on_chan_ctx_t* pRemainChanCtx = NULL;

    hddLog(VOS_TRACE_LEVEL_INFO, FL("Frame Type = %d Frame Length = %d"),
                     frameType, nFrameLength);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

    if (NULL == pAdapter)
    {
        hddLog(LOGE, FL("pAdapter is NULL"));
        return;
    }

    if (0 == nFrameLength)
    {
        hddLog(LOGE, FL("Frame Length is Invalid ZERO"));
        return;
    }

    if (NULL == pbFrames)
    {
        hddLog(LOGE, FL("pbFrames is NULL"));
        return;
    }

    type = WLAN_HDD_GET_TYPE_FRM_FC(pbFrames[0]);
    subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(pbFrames[0]);

    /* Get pAdapter from Destination mac address of the frame */
    if ((type == SIR_MAC_MGMT_FRAME) &&
            (subType != SIR_MAC_MGMT_PROBE_REQ))
    {
         pAdapter = hdd_get_adapter_by_macaddr( WLAN_HDD_GET_CTX(pAdapter),
                            &pbFrames[WLAN_HDD_80211_FRM_DA_OFFSET]);
         if (NULL == pAdapter)
         {
             /* Under assumtion that we don't receive any action frame
              * with BCST as destination we dropping action frame
              */
             hddLog(VOS_TRACE_LEVEL_FATAL,"pAdapter for action frame is NULL Macaddr = "
                               MAC_ADDRESS_STR ,
                               MAC_ADDR_ARRAY(&pbFrames[WLAN_HDD_80211_FRM_DA_OFFSET]));
             hddLog(VOS_TRACE_LEVEL_FATAL, "%s: Frame Type = %d Frame Length = %d"
<<<<<<< HEAD
                              " subType = %d \n",__func__,frameType,nFrameLength,subType);
=======
                              " subType = %d",__func__,frameType,nFrameLength,subType);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
             return;
         }
    }


    if (NULL == pAdapter->dev)
    {
        hddLog( LOGE, FL("pAdapter->dev is NULL"));
        return;
    }

    if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic)
    {
        hddLog( LOGE, FL("pAdapter has invalid magic"));
        return;
    }

<<<<<<< HEAD
=======
    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    if (NULL == pHddCtx)
    {
         hddLog(VOS_TRACE_LEVEL_FATAL,"%s: HDD Context Null Pointer", __func__);
         return;
    }

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    if ((WLAN_HDD_SOFTAP == pAdapter->device_mode) ||
        (WLAN_HDD_P2P_GO == pAdapter->device_mode ))
    {
        hdd_adapter_t *pMonAdapter =
            hdd_get_mon_adapter( WLAN_HDD_GET_CTX(pAdapter) );

        if (NULL != pMonAdapter)
        {
            hddLog( LOG1, FL("Indicate Frame over Monitor Interface"));
            hdd_sendMgmtFrameOverMonitorIface( pMonAdapter, nFrameLength,
                    pbFrames, frameType);
            return;
        }
    }

    //Channel indicated may be wrong. TODO
    //Indicate an action frame.
    if( rxChan <= MAX_NO_OF_2_4_CHANNELS )
    {
        freq = ieee80211_channel_to_frequency( rxChan,
                IEEE80211_BAND_2GHZ);
    }
    else
    {
        freq = ieee80211_channel_to_frequency( rxChan,
                IEEE80211_BAND_5GHZ);
    }

    cfgState = WLAN_HDD_GET_CFG_STATE_PTR( pAdapter );
<<<<<<< HEAD
    
    if ((type == SIR_MAC_MGMT_FRAME) && 
=======

    if ((type == SIR_MAC_MGMT_FRAME) &&
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        (subType == SIR_MAC_MGMT_ACTION))
    {
        if(pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] == WLAN_HDD_PUBLIC_ACTION_FRAME)
        {
            // public action frame
            if((pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET+1] == SIR_MAC_ACTION_VENDOR_SPECIFIC) &&
                vos_mem_compare(&pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET+2], SIR_MAC_P2P_OUI, SIR_MAC_P2P_OUI_SIZE))
            // P2P action frames
            {
                actionFrmType = pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
<<<<<<< HEAD
                hddLog(LOG1, "Rx Action Frame %u \n", actionFrmType);
=======
                hddLog(LOG1, "Rx Action Frame %u", actionFrmType);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#ifdef WLAN_FEATURE_P2P_DEBUG
                if(actionFrmType >= MAX_P2P_ACTION_FRAME_TYPE)
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR,"[P2P] unknown[%d] <--- OTA",
                                                                actionFrmType);
                }
                else
                {
                    hddLog(VOS_TRACE_LEVEL_ERROR,"[P2P] %s <--- OTA",
                    p2p_action_frame_type[actionFrmType]);
                    if( (actionFrmType == WLAN_HDD_PROV_DIS_REQ) &&
                        (globalP2PConnectionStatus == P2P_NOT_ACTIVE) )
                    {
                         globalP2PConnectionStatus = P2P_GO_NEG_PROCESS;
                         hddLog(LOGE,"[P2P State]Inactive state to "
                           "GO negotiation progress state");
                    }
                    else if( (actionFrmType == WLAN_HDD_GO_NEG_CNF) &&
                        (globalP2PConnectionStatus == P2P_GO_NEG_PROCESS) )
                    {
                         globalP2PConnectionStatus = P2P_GO_NEG_COMPLETED;
                 hddLog(LOGE,"[P2P State]GO negotiation progress to "
                             "GO negotiation completed state");
                    }
                    else if( (actionFrmType == WLAN_HDD_INVITATION_REQ) &&
                        (globalP2PConnectionStatus == P2P_NOT_ACTIVE) )
                    {
                         globalP2PConnectionStatus = P2P_GO_NEG_COMPLETED;
                         hddLog(LOGE,"[P2P State]Inactive state to GO negotiation"
                                     " completed state Autonomous GO formation");
                    }
                }
#endif
<<<<<<< HEAD

                if (((actionFrmType == WLAN_HDD_PROV_DIS_RESP) &&
                            (cfgState->actionFrmState == HDD_PD_REQ_ACK_PENDING)) ||
                        ((actionFrmType == WLAN_HDD_GO_NEG_RESP) &&
                         (cfgState->actionFrmState == HDD_GO_NEG_REQ_ACK_PENDING)))
                {
                    hddLog(LOG1, "%s: ACK_PENDING and But received RESP for Action frame ",
                            __func__);
                    hdd_sendActionCnf(pAdapter, TRUE);
                }
=======
             mutex_lock(&pHddCtx->roc_lock);
             pRemainChanCtx = cfgState->remain_on_chan_ctx;

             if (pRemainChanCtx != NULL && VOS_TIMER_STATE_RUNNING
                                 == vos_timer_getCurrentState(&pRemainChanCtx->hdd_remain_on_chan_timer))
             {
                 if ( actionFrmType == WLAN_HDD_GO_NEG_REQ ||
                      actionFrmType == WLAN_HDD_GO_NEG_RESP ||
                      actionFrmType == WLAN_HDD_INVITATION_REQ ||
                      actionFrmType == WLAN_HDD_DEV_DIS_REQ ||
                      actionFrmType == WLAN_HDD_PROV_DIS_REQ )
                 {
                      hddLog( LOG1, "Extend RoC timer on reception of"
                              " Action Frame");
                      if ((actionFrmType == WLAN_HDD_GO_NEG_REQ)
                                  || (actionFrmType == WLAN_HDD_GO_NEG_RESP))
                              extend_time = 2 * ACTION_FRAME_DEFAULT_WAIT;
                      else
                              extend_time = ACTION_FRAME_DEFAULT_WAIT;
                      if (completion_done(&pAdapter->rem_on_chan_ready_event))
                      {
                          vos_timer_stop(
                                    &pRemainChanCtx->hdd_remain_on_chan_timer);
                          status = vos_timer_start(
                                     &pRemainChanCtx->hdd_remain_on_chan_timer,
                                     extend_time);
                          if (status != VOS_STATUS_SUCCESS)
                          {
                              hddLog( LOGE, "ROC timer start failed");
                          }
                      }
                      else
                      {
                        // Buffer Packet
                          if (pRemainChanCtx->action_pkt_buff.frame_length == 0)
                          {
                              pRemainChanCtx->action_pkt_buff.frame_length =
                                                                  nFrameLength;
                              pRemainChanCtx->action_pkt_buff.freq = freq;
                              pRemainChanCtx->action_pkt_buff.frame_ptr
                                                = vos_mem_malloc(nFrameLength);
                              vos_mem_copy(
                                     pRemainChanCtx->action_pkt_buff.frame_ptr,
                                     pbFrames, nFrameLength);
                              hddLog( LOGE,"%s:"
                                "Action Pkt Cached successfully !!!", __func__);
                          }
                          else
                          {
                              hddLog( LOGE,"%s:"
                                      "Frames are pending. dropping frame !!!",
                                      __func__);
                          }
                          mutex_unlock(&pHddCtx->roc_lock);
                          return;
                      }
                 }
             }
             if (pRemainChanCtx != NULL &&
                  VOS_TIMER_STATE_RUNNING != vos_timer_getCurrentState(
                      &cfgState->remain_on_chan_ctx->hdd_remain_on_chan_timer))
                 hddLog( LOG1, "%s:"
                         "Rcvd action frame after timer expired ", __func__);

             mutex_unlock(&pHddCtx->roc_lock);

             if (((actionFrmType == WLAN_HDD_PROV_DIS_RESP) &&
                   (cfgState->actionFrmState == HDD_PD_REQ_ACK_PENDING)) ||
                  ((actionFrmType == WLAN_HDD_GO_NEG_RESP) &&
                   (cfgState->actionFrmState == HDD_GO_NEG_REQ_ACK_PENDING)))
             {
                 hddLog(LOG1, "%s: ACK_PENDING and But received RESP for Action frame ",
                         __func__);
                 hdd_sendActionCnf(pAdapter, TRUE);
             }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            }
#ifdef FEATURE_WLAN_TDLS
            else if(pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET+1] == WLAN_HDD_PUBLIC_ACTION_TDLS_DISC_RESP)
            {
                u8 *mac = &pbFrames[WLAN_HDD_80211_FRM_DA_OFFSET+6];
<<<<<<< HEAD
#ifdef WLAN_FEATURE_TDLS_DEBUG
                hddLog(VOS_TRACE_LEVEL_ERROR,"[TDLS] TDLS Discovery Response," MAC_ADDRESS_STR " RSSI[%d] <--- OTA",
                 MAC_ADDR_ARRAY(mac),rxRssi);
#endif
=======
                hddLog(VOS_TRACE_LEVEL_INFO,"[TDLS] TDLS Discovery Response," MAC_ADDRESS_STR " RSSI[%d] <--- OTA",
                 MAC_ADDR_ARRAY(mac),rxRssi);

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                wlan_hdd_tdls_set_rssi(pAdapter, mac, rxRssi);
                wlan_hdd_tdls_recv_discovery_resp(pAdapter, mac);
            }
#endif
        }
<<<<<<< HEAD
#ifdef WLAN_FEATURE_TDLS_DEBUG
=======

         pScanInfo =  &pHddCtx->scan_info;
         if ((pScanInfo != NULL) && (pHddCtx->scan_info.mScanPending))
         {
             hddLog(LOGE,"Action frame received when Scanning is in"
                          " progress. Abort Scan.");
             hdd_abort_mac_scan(pAdapter->pHddCtx,
                                pScanInfo->sessionId,
                                eCSR_SCAN_ABORT_DEFAULT);
         }

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        if(pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] == WLAN_HDD_TDLS_ACTION_FRAME)
        {
            actionFrmType = pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET+1];
            if(actionFrmType >= MAX_TDLS_ACTION_FRAME_TYPE)
            {
<<<<<<< HEAD
                hddLog(VOS_TRACE_LEVEL_ERROR,"[TDLS] unknown[%d] <--- OTA",
=======
                hddLog(VOS_TRACE_LEVEL_INFO,"[TDLS] Action type[%d] <--- OTA",
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                                                            actionFrmType);
            }
            else
            {
<<<<<<< HEAD
                hddLog(VOS_TRACE_LEVEL_ERROR,"[TDLS] %s <--- OTA",
                    tdls_action_frame_type[actionFrmType]);
            }
        }
#endif
=======
                hddLog(VOS_TRACE_LEVEL_INFO,"[TDLS] %s <--- OTA",
                    tdls_action_frame_type[actionFrmType]);
            }
        }

        if((pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] == WLAN_HDD_QOS_ACTION_FRAME)&&
             (pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET+1] == WLAN_HDD_QOS_MAP_CONFIGURE) )
        {
            sme_UpdateDSCPtoUPMapping(pHddCtx->hHal,
                pAdapter->hddWmmDscpToUpMap, pAdapter->sessionId);
        }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    }

    //Indicate Frame Over Normal Interface
    hddLog( LOG1, FL("Indicate Frame over NL80211 Interface"));
<<<<<<< HEAD

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
=======
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0))
    cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr, freq, 0, pbFrames,
                     nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,12,0))
    cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr, freq, 0, pbFrames,
                     nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0))
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    cfg80211_rx_mgmt( pAdapter->dev->ieee80211_ptr, freq, 0,
                      pbFrames, nFrameLength,
                      GFP_ATOMIC );
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
    cfg80211_rx_mgmt( pAdapter->dev, freq, 0,
                      pbFrames, nFrameLength,
                      GFP_ATOMIC );
#else
    cfg80211_rx_mgmt( pAdapter->dev, freq,
                      pbFrames, nFrameLength,
                      GFP_ATOMIC );
#endif //LINUX_VERSION_CODE
}

/*
 * ieee80211_add_rx_radiotap_header - add radiotap header
 */
static int hdd_wlan_add_rx_radiotap_hdr (
             struct sk_buff *skb, int rtap_len, int flag )
{
    u8 rtap_temp[20] = {0};
    struct ieee80211_radiotap_header *rthdr;
    unsigned char *pos;
    u16 rx_flags = 0;

    rthdr = (struct ieee80211_radiotap_header *)(&rtap_temp[0]);

    /* radiotap header, set always present flags */
    rthdr->it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_FLAGS)   |
                                    (1 << IEEE80211_RADIOTAP_RX_FLAGS));
    rthdr->it_len = cpu_to_le16(rtap_len);

    pos = (unsigned char *) (rthdr + 1);

    /* the order of the following fields is important */

    /* IEEE80211_RADIOTAP_FLAGS */
    *pos = 0;
    pos++;

    /* IEEE80211_RADIOTAP_RX_FLAGS: Length 2 Bytes */
    /* ensure 2 byte alignment for the 2 byte field as required */
    if ((pos - (u8 *)rthdr) & 1)
        pos++;
    put_unaligned_le16(rx_flags, pos);
    pos += 2;
<<<<<<< HEAD
    
=======

>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    // actually push the data
    memcpy(skb_push(skb, rtap_len), &rtap_temp[0], rtap_len);

    return 0;
}

static void hdd_wlan_tx_complete( hdd_adapter_t* pAdapter,
                                  hdd_cfg80211_state_t* cfgState,
                                  tANI_BOOLEAN actionSendSuccess )
{
    struct ieee80211_radiotap_header *rthdr;
    unsigned char *pos;
    struct sk_buff *skb = cfgState->skb;
#ifdef WLAN_OPEN_SOURCE
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
    hdd_context_t *pHddCtx = (hdd_context_t*)(pAdapter->pHddCtx);
#endif
#endif

    /* 2 Byte for TX flags and 1 Byte for Retry count */
    u32 rtHdrLen = sizeof(*rthdr) + 3;

    u8 *data;

    /* We have to return skb with Data starting with MAC header. We have
     * copied SKB data starting with MAC header to cfgState->buf. We will pull
     * entire skb->len from skb and then we will push cfgState->buf to skb
     * */
    if( NULL == skb_pull(skb, skb->len) )
    {
        hddLog( LOGE, FL("Not Able to Pull %d byte from skb"), skb->len);
        kfree_skb(cfgState->skb);
        return;
    }

    data = skb_push( skb, cfgState->len );

    if (data == NULL)
    {
<<<<<<< HEAD
        hddLog( LOGE, FL("Not Able to Push %d byte to skb"), cfgState->len);
=======
        hddLog( LOGE, FL("Not Able to Push %zu byte to skb"), cfgState->len);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        kfree_skb( cfgState->skb );
        return;
    }

    memcpy( data, cfgState->buf, cfgState->len );

    /* send frame to monitor interfaces now */
    if( skb_headroom(skb) < rtHdrLen )
    {
        hddLog( LOGE, FL("No headroom for rtap header"));
        kfree_skb(cfgState->skb);
        return;
    }

    rthdr = (struct ieee80211_radiotap_header*) skb_push( skb, rtHdrLen );

    memset( rthdr, 0, rtHdrLen );
    rthdr->it_len = cpu_to_le16( rtHdrLen );
    rthdr->it_present = cpu_to_le32((1 << IEEE80211_RADIOTAP_TX_FLAGS) |
                                    (1 << IEEE80211_RADIOTAP_DATA_RETRIES)
                                   );

    pos = (unsigned char *)( rthdr+1 );

    // Fill TX flags
    *pos = actionSendSuccess;
    pos += 2;

    // Fill retry count
    *pos = 0;
    pos++;

    skb_set_mac_header( skb, 0 );
    skb->ip_summed = CHECKSUM_NONE;
    skb->pkt_type  = PACKET_OTHERHOST;
    skb->protocol  = htons(ETH_P_802_2);
    memset( skb->cb, 0, sizeof( skb->cb ) );
#ifdef WLAN_OPEN_SOURCE
#ifdef WLAN_FEATURE_HOLD_RX_WAKELOCK
<<<<<<< HEAD
    wake_lock_timeout(&pHddCtx->rx_wake_lock, msecs_to_jiffies(HDD_WAKE_LOCK_DURATION));
=======
    vos_wake_lock_timeout_release(&pHddCtx->rx_wake_lock,
                                  HDD_WAKE_LOCK_DURATION,
                                  WIFI_POWER_EVENT_WAKELOCK_HOLD_RX);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
#endif
#endif
    if (in_interrupt())
        netif_rx( skb );
    else
        netif_rx_ni( skb );

    /* Enable Queues which we have disabled earlier */
<<<<<<< HEAD
=======
    hddLog(VOS_TRACE_LEVEL_INFO, FL("Enabling queues"));
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    netif_tx_start_all_queues( pAdapter->dev );

}

<<<<<<< HEAD
=======
void __hdd_p2p_roc_work_queue(struct work_struct *work)
{
    hdd_adapter_t *pAdapter = container_of(to_delayed_work(work), hdd_adapter_t, roc_work);
    hddLog( VOS_TRACE_LEVEL_INFO, FL("%s: "), __func__);
    wlan_hdd_p2p_start_remain_on_channel(pAdapter);
    return;
}

void hdd_p2p_roc_work_queue(struct work_struct *work)
{
    vos_ssr_protect(__func__);
    __hdd_p2p_roc_work_queue(work);
    vos_ssr_unprotect(__func__);
    return;
}
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
