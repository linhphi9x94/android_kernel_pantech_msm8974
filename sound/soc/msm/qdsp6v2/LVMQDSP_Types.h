/************************************************************************/
/*  Copyright (c) 2012-2013 NXP Software. All rights are reserved.      */
/*  Reproduction in whole or in part is prohibited without the prior    */
/*  written consent of the copyright owner.                             */
/*                                                                      */
/*  This software and any compilation or derivative thereof is and      */
/*  shall remain the proprietary information of NXP Software and is     */
/*  highly confidential in nature. Any and all use hereof is restricted */
/*  and is subject to the terms and conditions set forth in the         */
/*  software license agreement concluded with NXP Software.             */
/*                                                                      */
/*  Under no circumstances is this software or any derivative thereof   */
/*  to be combined with any Open Source Software in any way or          */
/*  licensed under any Open License Terms without the express prior     */
/*  written  permission of NXP Software.                                */
/*                                                                      */
/*  For the purpose of this clause, the term Open Source Software means */
/*  any software that is licensed under Open License Terms. Open        */
/*  License Terms means terms in any license that require as a          */
/*  condition of use, modification and/or distribution of a work        */
/*                                                                      */
/*           1. the making available of source code or other materials  */
/*              preferred for modification, or                          */
/*                                                                      */
/*           2. the granting of permission for creating derivative      */
/*              works, or                                               */
/*                                                                      */
/*           3. the reproduction of certain notices or license terms    */
/*              in derivative works or accompanying documentation, or   */
/*                                                                      */
/*           4. the granting of a royalty-free license to any party     */
/*              under Intellectual Property Rights                      */
/*                                                                      */
/*  regarding the work and/or any work that contains, is combined with, */
/*  requires or otherwise is based on the work.                         */
/*                                                                      */
/*  This software is provided for ease of recompilation only.           */
/*  Modification and reverse engineering of this software are strictly  */
/*  prohibited.                                                         */
/*                                                                      */
/************************************************************************/

/*****************************************************************************************

     $Author: nxp47058 $
     $Revision: 37197 $
     $Date: 2013-02-27 16:09:51 +0900 (¼ö, 27 2 2013) $

*****************************************************************************************/
#ifndef __LVMQDSP_TYPES_H__
#define __LVMQDSP_TYPES_H__

#include "LVM.h"
#include "LVREV.h"

#define LVMQDSP_MAX_SESSIONS            8
#define LVMQDSP_MAX_EQ_BANDS            10
#define LVMQDSP_MAX_HEADROOM_BANDS      LVM_HEADROOM_MAX_NBANDS
#define LVMQDSP_MAX_BLOCK_SIZE          1023
#define LVMQDSP_MAX_PSA_BANDS           64
#define LVMQDSP_SUCCESS                 0
#define LVMQDSP_ERROR                   (-1)

#define LVMQDSP_ERROR_CHECK(status){\
            if (status != LVMQDSP_SUCCESS) {\
                LOGE("LVMQDSP: Error returned by %s in %s, %d", __func__, __FILE__, __LINE__); \
            }\
        }

//#define ENABLE_QDSP_SYNC
//#define DISABLE_QDSP_INTERLEAVING

#define LVSE_OFFLOAD

typedef unsigned int LVMQDSP_Session_t;

typedef struct
{
    LVM_INT16  *pIndata;
    LVM_INT16  *pOutData;
    LVM_UINT16  numSamples;
    LVM_UINT32  audioTime;
} LVMQDSP_Process_t;

typedef struct
{
    unsigned int        cmd;
    LVMQDSP_Session_t   session;
    void                *cmdParams;
} LVMQDSP_Command_t;

typedef enum
{
    LVMQDSP_OPEN_SESSION = 0x1000,
    LVMQDSP_CLOSE_SESSION,
    LVMQDSP_SET_INSTANCE_PARAMS,
    LVMQDSP_GET_INSTANCE_PARAMS,
    LVMQDSP_SET_CONTROL_PARAMS,
    LVMQDSP_GET_CONTROL_PARAMS,
    LVMQDSP_SET_HEADROOM_PARAMS,
    LVMQDSP_GET_HEADROOM_PARAMS,
    LVMQDSP_SET_TUNING_PARAMS,
    LVMQDSP_GET_TUNING_PARAMS,
    LVMQDSP_CLEAR_AUDIO_BUFFER,
    LVMQDSP_GET_SPECTRUM_PARAMS,
    LVMQDSP_SET_VOLNOSMOOTHING_PARAMS,
	LVMQDSP_SET_REVERB_INSTANCE_PARAMS,
	LVMQDSP_GET_REVERB_INSTANCE_PARAMS,
	LVMQDSP_SET_REVERB_CONTROL_PARAMS,
	LVMQDSP_GET_REVERB_CONTROL_PARAMS,
    LVMQDSP_REVERB_CLEAR_AUDIO_BUFFER,
#ifdef LVSE_OFFLOAD
    LVMQDSP_SET_OFFLOAD_PARAMS,
#endif
    LVMQDSP_PROCESS
} LVMQDSP_Command_enum;

#ifdef LVSE_OFFLOAD
typedef enum
{
	LVMQDSP_NON_OFFLOAD = 0,
	LVMQDSP_OFFLOAD
}LVMQDSP_ProcessType_t;
#endif

typedef LVM_InstParams_t LVMQDSP_InstParams_t;

typedef struct
{
    LVM_ControlParams_t     LvmParams;
    LVM_EQNB_BandDef_t      LvmBands[LVMQDSP_MAX_EQ_BANDS];
} LVMQDSP_ControlParams_t;

typedef struct
{
    LVM_Custom_TuningParams_st  LvmParams;
    LVM_EQNB_BandDef_t          LvmBands[LVMQDSP_MAX_EQ_BANDS];
} LVMQDSP_Custom_TuningParams_st;

typedef struct
{
    LVM_HeadroomParams_t    LvmParams;
    LVM_HeadroomBandDef_t   LvmBands[LVMQDSP_MAX_HEADROOM_BANDS];
} LVMQDSP_HeadroomParams_t;

typedef struct
{
	LVM_UINT32     totalSamples;
	LVM_UINT32     processLeft;
} LVMQDSP_ProcessParams_t;

typedef struct
{
    LVM_INT8                  CurrentPeaks[LVMQDSP_MAX_PSA_BANDS];            /* PSA Current Spectral Peaks array*/
} LVMQDSP_PSA_SpectrumParams_st;

typedef struct
{
    LVREV_ControlParams_st     LvmParams;
} LVMQDSP_RevControlParams_t;

#ifdef LVSE_OFFLOAD
typedef struct
{
     LVM_INT32 session;
     LVM_INT16 isOffloaded;
}LVMQDSP_OffloadParams_t;
#endif

typedef LVREV_InstanceParams_st LVMQDSP_RevInstParams_t;

#endif /* __LVMQDSP_TYPES_H__ */
