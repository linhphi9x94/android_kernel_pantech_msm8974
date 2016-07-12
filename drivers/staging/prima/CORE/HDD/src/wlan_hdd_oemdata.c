/*
<<<<<<< HEAD
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
=======
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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

#ifdef FEATURE_OEM_DATA_SUPPORT

/*================================================================================ 
    \file wlan_hdd_oemdata.c
  
    \brief Linux Wireless Extensions for oem data req/rsp
  
    $Id: wlan_hdd_oemdata.c,v 1.34 2010/04/15 01:49:23 -- VINAY
  
    Copyright (C) Qualcomm Inc.
    
================================================================================*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <wlan_hdd_includes.h>
#include <net/arp.h>
<<<<<<< HEAD

=======
#include <vos_sched.h>
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
/*---------------------------------------------------------------------------------------------

  \brief hdd_OemDataReqCallback() - 

  This function also reports the results to the user space

<<<<<<< HEAD
  \return - 0 for success, non zero for failure
=======
  \return - eHalStatus enumeration
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

-----------------------------------------------------------------------------------------------*/
static eHalStatus hdd_OemDataReqCallback(tHalHandle hHal, 
        void *pContext,
        tANI_U32 oemDataReqID,
        eOemDataReqStatus oemDataReqStatus)
{
    eHalStatus status = eHAL_STATUS_SUCCESS;
    struct net_device *dev = (struct net_device *) pContext;
    union iwreq_data wrqu;
    char buffer[IW_CUSTOM_MAX+1];

    memset(&wrqu, '\0', sizeof(wrqu));
    memset(buffer, '\0', sizeof(buffer));

    //now if the status is success, then send an event up
    //so that the application can request for the data
    //else no need to send the event up
    if(oemDataReqStatus == eOEM_DATA_REQ_FAILURE)
    {
        snprintf(buffer, IW_CUSTOM_MAX, "QCOM: OEM-DATA-REQ-FAILED");
<<<<<<< HEAD
        hddLog(LOGW, "%s: oem data req %d failed\n", __func__, oemDataReqID);
=======
        hddLog(LOGW, "%s: oem data req %d failed", __func__, oemDataReqID);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    }
    else if(oemDataReqStatus == eOEM_DATA_REQ_INVALID_MODE)
    {
        snprintf(buffer, IW_CUSTOM_MAX, "QCOM: OEM-DATA-REQ-INVALID-MODE");
<<<<<<< HEAD
        hddLog(LOGW, "%s: oem data req %d failed because the driver is in invalid mode (IBSS|BTAMP|AP)\n", __func__, oemDataReqID);
=======
        hddLog(LOGW, "%s: oem data req %d failed because the driver is in invalid mode (IBSS|BTAMP|AP)", __func__, oemDataReqID);
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    }
    else
    {
        snprintf(buffer, IW_CUSTOM_MAX, "QCOM: OEM-DATA-REQ-SUCCESS");
        //everything went alright
    }
    
    wrqu.data.pointer = buffer;
    wrqu.data.length = strlen(buffer);
        
    wireless_send_event(dev, IWEVCUSTOM, &wrqu, buffer);

    return status;
}

/**--------------------------------------------------------------------------------------------

<<<<<<< HEAD
  \brief iw_get_oem_data_rsp() - 
=======
  \brief __iw_get_oem_data_rsp() -
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

  This function gets the oem data response. This invokes
  the respective sme functionality. Function for handling the oem data rsp 
  IOCTL 

  \param - dev  - Pointer to the net device
         - info - Pointer to the iw_oem_data_req
         - wrqu - Pointer to the iwreq data
         - extra - Pointer to the data

  \return - 0 for success, non zero for failure

-----------------------------------------------------------------------------------------------*/
<<<<<<< HEAD
int iw_get_oem_data_rsp(
        struct net_device *dev, 
=======
int __iw_get_oem_data_rsp(
        struct net_device *dev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra)
{
<<<<<<< HEAD
    eHalStatus                            status = eHAL_STATUS_SUCCESS;
    struct iw_oem_data_rsp*               pHddOemDataRsp;
    tOemDataRsp*                          pSmeOemDataRsp;

    hdd_adapter_t *pAdapter = (netdev_priv(dev));

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                                  "%s:LOGP in Progress. Ignore!!!",__func__);
       return -EBUSY;
    }

=======
    int                                   rc = 0;
    eHalStatus                            status;
    struct iw_oem_data_rsp*               pHddOemDataRsp;
    tOemDataRsp*                          pSmeOemDataRsp;
    hdd_adapter_t *pAdapter;
    hdd_context_t *pHddCtx;

    ENTER();
    pAdapter = (netdev_priv(dev));
    if (NULL == pAdapter)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: Adapter is NULL",__func__);
        return -EINVAL;
    }
    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    rc = wlan_hdd_validate_context(pHddCtx);
    if (0 != rc)
    {
        return rc;
    }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    do
    {
        //get the oem data response from sme
        status = sme_getOemDataRsp(WLAN_HDD_GET_HAL_CTX(pAdapter), &pSmeOemDataRsp);
<<<<<<< HEAD
        if(status != eHAL_STATUS_SUCCESS)
        {
            hddLog(LOGE, "%s: failed in sme_getOemDataRsp\n", __func__);
=======
        if (status != eHAL_STATUS_SUCCESS)
        {
            hddLog(LOGE, "%s: failed in sme_getOemDataRsp", __func__);
            rc = -EIO;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            break;
        }
        else
        {
<<<<<<< HEAD
            if(pSmeOemDataRsp != NULL)
=======
            if (pSmeOemDataRsp != NULL)
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            {
                pHddOemDataRsp = (struct iw_oem_data_rsp*)(extra);
                vos_mem_copy(pHddOemDataRsp->oemDataRsp, pSmeOemDataRsp->oemDataRsp, OEM_DATA_RSP_SIZE); 
            }
            else
            {
<<<<<<< HEAD
                hddLog(LOGE, "%s: pSmeOemDataRsp = NULL\n", __func__);
                status = eHAL_STATUS_FAILURE;
=======
                hddLog(LOGE, "%s: pSmeOemDataRsp = NULL", __func__);
                rc = -EIO;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
                break;
            }
        }
    } while(0);

<<<<<<< HEAD
    return eHAL_STATUS_SUCCESS;
=======
    EXIT();
    return rc;
}

int iw_get_oem_data_rsp(
        struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra)
{
    int ret;

    vos_ssr_protect(__func__);
    ret = __iw_get_oem_data_rsp(dev, info, wrqu, extra);
    vos_ssr_unprotect(__func__);

    return ret;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
}

/**--------------------------------------------------------------------------------------------

<<<<<<< HEAD
  \brief iw_set_oem_data_req() - 
=======
  \brief __iw_set_oem_data_req() -
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

  This function sets the oem data req configuration. This invokes
  the respective sme oem data req functionality. Function for 
  handling the set IOCTL for the oem data req configuration

  \param - dev  - Pointer to the net device
     - info - Pointer to the iw_oem_data_req
     - wrqu - Pointer to the iwreq data
     - extra - Pointer to the data

  \return - 0 for success, non zero for failure

-----------------------------------------------------------------------------------------------*/
<<<<<<< HEAD
int iw_set_oem_data_req(
        struct net_device *dev, 
=======
int __iw_set_oem_data_req(
        struct net_device *dev,
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra)
{
<<<<<<< HEAD
    eHalStatus status = eHAL_STATUS_SUCCESS;
    struct iw_oem_data_req *pOemDataReq = NULL;
    tOemDataReqConfig oemDataReqConfig;

    tANI_U32 oemDataReqID = 0;

    hdd_adapter_t *pAdapter = (netdev_priv(dev));
    hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

    if ((WLAN_HDD_GET_CTX(pAdapter))->isLogpInProgress)
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                                  "%s:LOGP in Progress. Ignore!!!",__func__);
       return -EBUSY;
=======
    int rc = 0;
    eHalStatus status = eHAL_STATUS_SUCCESS;
    struct iw_oem_data_req *pOemDataReq = NULL;
    tOemDataReqConfig oemDataReqConfig;
    tANI_U32 oemDataReqID = 0;
    hdd_adapter_t *pAdapter;
    hdd_context_t *pHddCtx;
    hdd_wext_state_t *pwextBuf;

    ENTER();

    if (!capable(CAP_NET_ADMIN))
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  FL("permission check failed"));
        return -EPERM;
    }

    pAdapter = (netdev_priv(dev));
    if (NULL == pAdapter)
    {
       VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                 "%s: Adapter is NULL",__func__);
       return -EINVAL;
    }
    pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
    rc = wlan_hdd_validate_context(pHddCtx);
    if (0 != rc)
    {
        return rc;
    }

    pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
    if (NULL == pwextBuf)
    {
        VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                  "%s: pwextBuf is NULL",__func__);
        return -EINVAL;
    }

    if (pHddCtx->isPnoEnable)
    {
        VOS_TRACE( VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_INFO,
                   FL("pno scan in progress"));
        return -EBUSY;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    }

    do
    {
<<<<<<< HEAD
        if(NULL != wrqu->data.pointer)
=======
        if (NULL != wrqu->data.pointer)
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        {
            pOemDataReq = (struct iw_oem_data_req *)wrqu->data.pointer;
        }

<<<<<<< HEAD
        if(pOemDataReq == NULL)
        {
            hddLog(LOGE, "in %s oemDataReq == NULL\n", __func__);
            status = eHAL_STATUS_FAILURE;
=======
        if (pOemDataReq == NULL)
        {
            hddLog(LOGE, "in %s oemDataReq == NULL", __func__);
            rc = -EIO;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
            break;
        }

        vos_mem_zero(&oemDataReqConfig, sizeof(tOemDataReqConfig));

        if (copy_from_user((&oemDataReqConfig)->oemDataReq,
                           pOemDataReq->oemDataReq, OEM_DATA_REQ_SIZE))
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_FATAL,
                      "%s: copy_from_user() failed!", __func__);
<<<<<<< HEAD
            return -EFAULT;
=======
            rc = -EFAULT;
            break;
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
        }

        status = sme_OemDataReq(WLAN_HDD_GET_HAL_CTX(pAdapter), 
                                                pAdapter->sessionId,
                                                &oemDataReqConfig, 
                                                &oemDataReqID, 
                                                &hdd_OemDataReqCallback, 
                                                dev);
<<<<<<< HEAD
=======
        if (status != eHAL_STATUS_SUCCESS)
        {
            VOS_TRACE(VOS_MODULE_ID_HDD, VOS_TRACE_LEVEL_ERROR,
                      "%s: sme_OemDataReq status %d", __func__, status);
            rc = -EFAULT;
            break;
        }
>>>>>>> 3bbd1bf... staging: add prima WLAN driver
    
        pwextBuf->oemDataReqID = oemDataReqID;
        pwextBuf->oemDataReqInProgress = TRUE;

    } while(0);
<<<<<<< HEAD
    
    return status;
}

=======

    EXIT();
    return rc;
}

int iw_set_oem_data_req(
        struct net_device *dev,
        struct iw_request_info *info,
        union iwreq_data *wrqu,
        char *extra)
{
    int ret;

    vos_ssr_protect(__func__);
    ret = __iw_set_oem_data_req(dev, info, wrqu, extra);
    vos_ssr_unprotect(__func__);

    return ret;
}
>>>>>>> 3bbd1bf... staging: add prima WLAN driver

#endif
