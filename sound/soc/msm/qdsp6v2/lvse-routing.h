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
     $Revision: 37504 $
     $Date: 2013-03-04 15:55:50 +0900 (¿ù, 04 3 2013) $

*****************************************************************************************/

#define ALGORITHM_CI
#define ALGORITHM_PB
#define ALGORITHM_TE
#define ALGORITHM_LM
#define ALGORITHM_AVL
#define ALGORITHM_PSA
#define ALGORITHM_EQNB
#define __CI_OUT_SMOOTHING

#include "LVMQDSP_Types.h"

#include <sound/apr_audio-v2.h>

#define ASM_STREAM_POSTPROC_TOPO_ID_NXP_LVM 0x00010FFC

// The numbers corresponding to these macros are just placeholders
#define MODULE_ID_NXP_LVM                                  0x1000B100
#define PARAM_ID_NXP_LVM_EFFECT_CTRL                       0x1000B101
#define PARAM_ID_NXP_LVM_TUNING_CTRL                       0x1000B102
#define PARAM_ID_NXP_LVM_HEADROOM_CTRL                     0x1000B103
#define PARAM_ID_NXP_LVM_INSTANCE_CTRL                     0x1000B104
#define PARAM_ID_NXP_LVM_CLEAR_AUDIO_BUFFER_CTRL           0x1000B105
#define PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL                 0x1000B106
#define PARAM_ID_NXP_LVM_SET_VOLNOSMOOTHING_CTRL           0x1000B107
#define PARAM_ID_NXP_LVM_SET_PADDING                       0x1000B108

#define MODULE_ID_NXP_LVM_REVERB                           0x1000C100
#define PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL              0x1000C101
#define PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL                0x1000C102
#define PARAM_ID_NXP_LVM_REVERB_CLEAR_AUDIO_BUFFER_CTRL    0x1000C103


//#define ENABLE_PARAM_LOG
//#define ENABLE_PARAM_LOG_OFFLOAD

#ifdef ENABLE_PARAM_LOG
#define LVSE_LOG_0(param) pr_debug(param)
#define LVSE_LOG_1(param, log1) pr_debug(param, log1)
#define LVSE_LOG_2(param, log1, log2) pr_debug(param, log1, log2)
#define LVSE_LOG_3(param, log1, log2, log3) pr_debug(param, log1, log2, log3)
#else
#define LVSE_LOG_0(param)
#define LVSE_LOG_1(param, log1) 
#define LVSE_LOG_2(param, log1, log2) 
#define LVSE_LOG_3(param, log1, log2, log3) 
#endif

#ifndef LVSE_OFFLOAD
#define LVSE_OFFLOAD
#endif
#ifdef LVSE_OFFLOAD 

#define LVM_UNUSED_SESSION         INT_MAX
#define LVM_USED_SESSION             0x1111

#define LVM_MAX_SESSIONS           32

#define LVM_NOT_FOUND_SESSION	 	-1
#define LVM_NOT_USE_SPACE	 		-2


typedef enum{
    LVM_NOT_SETTING = 0x1000,
	LVM_NON_OFFLOAD,
	LVM_OFFLOAD,
}LVM_ProcessMode;

typedef struct{
    int isSetInstParams;
    int isSetControlParams;
    int isSetHeadRoomParams;
    int isSetCustomTuningParams;
    int isSetRevInstParams;
    int isSetRevControlParams;

    LVMQDSP_InstParams_t instParams;
    LVMQDSP_ControlParams_t controlParams;
    LVMQDSP_HeadroomParams_t headRoomParams;
    LVMQDSP_Custom_TuningParams_st customTuningParams;
    LVMQDSP_RevInstParams_t revInstParams;
    LVMQDSP_RevControlParams_t revControlParams;
}LVMQDSP_SettingLVMControlInKernel_t;

typedef struct{
    LVM_ProcessMode processMode;
    int isGetAudioClientFromOffload;
}LVMQDSP_ProcessingInKernel_t;

typedef struct {
    int session;
    int offloadFlag;
    int useFlag;
    struct audio_client         **ac;
    LVMQDSP_SettingLVMControlInKernel_t settingLVMControlInKernel;
  //  LVMQDSP_SessionContext_t gSessions;  // the code name change gSessions to gSessionContext.
} LVMQDSP_SessionControl_t;
#endif

//FIXME:HENRY
struct asm_get_pp_params_cmdrsp {
    struct  asm_stream_cmdrsp_get_pp_params_v2 cmdrsp;
    struct  asm_stream_param_data_v2           params;
	void   *data;
} __attribute__ ((packed));

#define MAX_BUF 4

struct pcm {
    wait_queue_head_t read_wait;
    atomic_t          read_count;
    uint32_t          read_frame_info[MAX_BUF][2];
};

struct sync {
    wait_queue_head_t wait;
    atomic_t          count;
};

typedef struct
{
    struct audio_client         *ac;
    struct mutex                lock;
    wait_queue_head_t           wait;
    atomic_t                    count;
    struct pcm                  pcm;
    LVMQDSP_ControlParams_t     control_params;
    LVMQDSP_HeadroomParams_t    headroom_params;
    LVMQDSP_InstParams_t        inst_params;
    LVMQDSP_Custom_TuningParams_st tuning_params;
    LVMQDSP_PSA_SpectrumParams_st spectrum_params;
    //Reverb
    LVMQDSP_RevInstParams_t        reverb_inst_params;
    LVMQDSP_RevControlParams_t     reverb_control_params;
    int                         isFirstWrite;
    int                         clientId;
    int                         refCount;
} LVMQDSP_SessionContext_t;

int lvse_routing_set_control(struct snd_kcontrol *kcontrol,
                struct snd_ctl_elem_value *ucontrol);

int lvse_routing_get_control(struct snd_kcontrol *kcontrol,
                struct snd_ctl_elem_value *ucontrol);

void lvse_callback(uint32_t  opcode,
                   uint32_t  token,
                   uint32_t *payload,
                   void     *priv);

void setClientId(int id);

