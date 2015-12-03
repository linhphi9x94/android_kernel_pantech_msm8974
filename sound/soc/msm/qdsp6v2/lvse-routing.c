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

//FIXME remove junk includes

#include <linux/init.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/atomic.h>
#include <sound/soc.h>
//#include <sound/q6asm.h>
//#include "msm-pcm-routing-v2.h"
#include "lvse-routing.h"
#include <sound/apr_audio-v2.h>
#include <sound/q6asm-v2.h>

#ifdef LVSE_OFFLOAD
//static LVMQDSP_SettingLVMControlInKernel_t gSettingLVMControlInKernel;
//static LVMQDSP_ProcessingInKernel_t gProcessingInKernel; // used for LVMQDSP_OPEN_SESSION
int lvse_get_effect_count(LVMQDSP_SessionControl_t *session_control);
void lvse_session_control_init(void);
int lvse_find_session_control(int session);
int lvse_get_unused_session_control(int session, LVMQDSP_SessionControl_t **out_session_control);
int lvse_release_used_session_control(int session);
int lvse_get_session_control(int session, LVMQDSP_SessionControl_t **out_session_control);

int lvse_set_setting_lvm_control_in_kernel(int cmd, void *param, int session);
int lvse_get_setting_lvm_control_in_kernel(int cmd, void *param, int session);
int lvse_apply_setting_lvm_control_to_dsp(int process_type, int session);

int gSessionControlInitFlag = LVM_FALSE;
int gSettingEffectControlFlag = LVM_FALSE;
int gSessionOffload = -1;


LVMQDSP_SessionControl_t gSessionControls[LVM_MAX_SESSIONS];
#endif

LVMQDSP_SessionContext_t gSessions;


#ifdef ENABLE_PARAM_DUMP
void lvse_dump_control_params(void* ptr);
void lvse_dump_tuning_params(void* ptr);
#endif /* ENABLE_PARAM_DUMP */

int q6asm_set_pp_params(struct audio_client *ac, u32 module_id, u32 param_id,
            void *pp_params, int pp_size);

int q6asm_get_pp_params(struct audio_client *ac, u32 module_id, u32 param_id,
            int pp_size);

int lvse_pcm_close_session(LVMQDSP_Session_t session);
int lvse_pcm_open_session(LVMQDSP_Session_t session);

void setClientId(int id)
{
	LVSE_LOG_1("set Client Id = %d\n", id);

	gSessions.clientId = id;

	/* close session */
	if(id == -1)
	{
		LVSE_LOG_0("\nLVSE: setClientId - close session");
		if(gSessions.ac == NULL)
			pr_err("\nLVSE: setClientId - close session - context should not be null");
              lvse_pcm_close_session(id);
	}
	else /* open_session */
	{
		LVSE_LOG_0("\nLVSE: setClientId - opening session");
		if(gSessions.ac != NULL)
		{
			pr_err("\nLVSE: setClientId - opening session - context should be null");
			lvse_pcm_close_session(0);
		}
		
              lvse_pcm_open_session(id);
	}

}



//----------------------------------------------------------------------------
// lvse_callback()
//----------------------------------------------------------------------------
// Purpose: This function is called from QDSP to inform various event.
//
// Inputs:
//  opcode		Event command
//  token
//  payload		Callback data
//  priv		User data
//
// Outputs:
//
//----------------------------------------------------------------------------
void lvse_callback(uint32_t  opcode,
                   uint32_t  token,
                   uint32_t *payload,
                   void     *priv) {
    struct asm_get_pp_params_cmdrsp* params;
    struct pcm *pcm;

    if(gSessions.ac == NULL)
        return;

    pcm = &gSessions.pcm;

    LVSE_LOG_3("%s:opcde = %d token = 0x%x\n", __func__, opcode, token);

    switch (opcode) {
    case ASM_DATA_EVENT_WRITE_DONE_V2:
        LVSE_LOG_1("LVSE: ASM_DATA_EVENT_WRITE_DONE token=%d", token);
        break;
    case ASM_DATA_EVENT_READ_DONE_V2:
        pcm->read_frame_info[token][0] = payload[7];
        pcm->read_frame_info[token][1] = payload[3];
        atomic_inc(&pcm->read_count);
        LVSE_LOG_2("LVSE: ASM_DATA_EVENT_READ_DONE token=%d, read_count=%d", token, atomic_read(&pcm->read_count));
        wake_up(&pcm->read_wait);
        break;
    case ASM_DATA_EVENT_EOS:
    	LVSE_LOG_0("ASM_DATA_EVENT_EOS");
    	break;
    case 0x000110E8:
    	LVSE_LOG_0("APRV2_IBASIC_RSP_RESULT");
    	break;
    case ASM_STREAM_CMDRSP_GET_PP_PARAMS_V2:
    {

        params = (struct asm_get_pp_params_cmdrsp*)payload;
        LVSE_LOG_1("input param size=%d", params->params.param_size);
        
        LVSE_LOG_0("lvse_callback: count increment"); 
        atomic_inc(&gSessions.count);

	 LVSE_LOG_1("lvse_callback result   = %d", params->cmdrsp.status);
	 LVSE_LOG_1("lvse_callback param_id = %d", params->params.param_id);

        switch (params->params.param_id)
        {
            case PARAM_ID_NXP_LVM_INSTANCE_CTRL:
            {
                LVSE_LOG_0("PARAM_ID_NXP_LVM_INSTANCE_CTRL");
                LVSE_LOG_1("output param size=%d", sizeof(LVMQDSP_InstParams_t));
                memcpy(&gSessions.inst_params, &(params->data), sizeof(LVMQDSP_InstParams_t));
                break;
            }
            case PARAM_ID_NXP_LVM_EFFECT_CTRL:
            {
                LVSE_LOG_0("PARAM_ID_NXP_LVM_EFFECT_CTRL");
                LVSE_LOG_1("output param size=%d", sizeof(LVMQDSP_ControlParams_t));
                memcpy(&gSessions.control_params, &(params->data), sizeof(LVMQDSP_ControlParams_t));
#ifdef ENABLE_PARAM_DUMP
                lvse_dump_control_params(&gSessions.control_params);
                lvse_dump_control_params(&(params->data));
#endif /* ENABLE_PARAM_DUMP */
                break;
            }
            case PARAM_ID_NXP_LVM_HEADROOM_CTRL:
            {
                LVSE_LOG_0("PARAM_ID_NXP_LVM_HEADROOM_CTRL");
                memcpy(&gSessions.headroom_params, &(params->data), sizeof(LVMQDSP_HeadroomParams_t));
                break;
            }
            case PARAM_ID_NXP_LVM_TUNING_CTRL:
            {
                LVSE_LOG_0("PARAM_ID_NXP_LVM_TUNING_CTRL");
                memcpy(&gSessions.tuning_params, &(params->data), sizeof(LVMQDSP_Custom_TuningParams_st));
                break;
            }
            case PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL:
            {
                LVSE_LOG_0("PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL");
                memcpy(&gSessions.spectrum_params, &(params->data), sizeof(LVMQDSP_PSA_SpectrumParams_st));
				break;
            }
			case PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL:
			{
				LVSE_LOG_0("PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL");
				LVSE_LOG_1("output param size=%d", sizeof(LVMQDSP_RevInstParams_t));
				memcpy(&gSessions.reverb_inst_params, &(params->data), sizeof(LVMQDSP_RevInstParams_t));
				break;
			}
			case PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL:
			{
				LVSE_LOG_0("PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL");
				LVSE_LOG_1("output param size=%d", sizeof(LVMQDSP_ControlParams_t));
				memcpy(&gSessions.reverb_control_params, &(params->data), sizeof(LVMQDSP_RevControlParams_t));
				break;
			}
			
            default:
                LVSE_LOG_1("Unknown param_id %d", params->params.param_id);
        }
        
        wake_up(&gSessions.wait);
        LVSE_LOG_0("lvse_callback: wake_up");           
        
        break;
    }
    default:
        LVSE_LOG_2("%s:Unhandled event = 0x%8x", __func__, opcode);
        break;
    }
}

//----------------------------------------------------------------------------
// lvse_pcm_open_session()
//----------------------------------------------------------------------------
// Purpose: Open new session, q6asm client and set up the new topology.
//          Allocate the memory which to read and write between kernel and QDSP.
//
// Inputs:
//  session     audio session id
//
// Outputs:
//
//----------------------------------------------------------------------------
int lvse_pcm_open_session(LVMQDSP_Session_t session)
{
    // if new session
    if (gSessions.ac == NULL) {
    
        LVSE_LOG_1("\nLVSE: init gSessions[%d]", session);
        mutex_init(&gSessions.lock);
        init_waitqueue_head(&gSessions.wait);
        atomic_set(&gSessions.count, 0);
        init_waitqueue_head(&gSessions.pcm.read_wait);
        atomic_set(&gSessions.pcm.read_count, 0);

		LVSE_LOG_1("lvse:lvse_pcm_open_session - client id = %d", gSessions.clientId);

		gSessions.ac = q6asm_get_audio_client(gSessions.clientId);
		if(gSessions.ac == NULL){
			pr_info("can't get the client");
			return LVMQDSP_ERROR;
		}
    }


#ifdef LVSE_OFFLOAD
    if(gSessionOffload != -1){
        LVSE_LOG_0("LVSE: lvse_pcm_open_session : calling lvse_apply_setting_lvm_control_to_dsp()");
        lvse_apply_setting_lvm_control_to_dsp(LVM_OFFLOAD, gSessionOffload);  // apply effect setting information to DSP.
    }
#endif

    return LVMQDSP_SUCCESS;
}

//----------------------------------------------------------------------------
// lvse_pcm_close_session()
//----------------------------------------------------------------------------
// Purpose: Close the session and free the q6asm client.
//
// Inputs:
//  session     audio session id
//
// Outputs:
//
//----------------------------------------------------------------------------
int lvse_pcm_close_session(LVMQDSP_Session_t session)
{
       LVSE_LOG_0("lvse:lvse_pcm_close_session start");
	if(gSessions.ac != NULL)
	{

		mutex_lock(&gSessions.lock);
		mutex_unlock(&gSessions.lock);
		mutex_destroy(&gSessions.lock);
		
		// close stream
		gSessions.ac = NULL;
		
		// clean memory
		memset(&gSessions, 0, sizeof(LVMQDSP_SessionContext_t)); 
		atomic_set(&gSessions.count, 0);
		
	}
       LVSE_LOG_0("lvse:lvse_pcm_close_session end");

    return LVMQDSP_SUCCESS;
}

//----------------------------------------------------------------------------
// lvse_routing_set_control()
//----------------------------------------------------------------------------
// Purpose: Set parameter to the QDSP. Processing is happened here.
//
// Inputs:
//  kcontrol
//  ucontrol      	User data to set
//
// Outputs:
//
//----------------------------------------------------------------------------
int lvse_routing_set_control(struct snd_kcontrol       *kcontrol,
                             struct snd_ctl_elem_value *ucontrol) {

    int status = 0;
    LVMQDSP_Command_t *lvm;
    LVMQDSP_Session_t session;
    struct audio_client *lvse_ac;
    int session_index = 0;
#ifdef LVSE_OFFLOAD
    LVMQDSP_SessionControl_t *session_control = NULL;
#endif    
    lvm = (LVMQDSP_Command_t*)ucontrol->value.bytes.data;

    LVSE_LOG_1("LVSE: cmd = %d, ###############################################\n", lvm->cmd);

    session = lvm->session;

#ifdef LVSE_OFFLOAD    
    lvse_session_control_init();   // initialize global session control information
#endif

    switch(lvm->cmd)
    {
        case LVMQDSP_OPEN_SESSION:
            LVSE_LOG_0("LVSE: LVMQDSP_OPEN_SESSION");
#ifdef LVSE_OFFLOAD
	     if(gSessions.ac != NULL)
	     {
		     // if gSession.ac is not NULL, this process is OFFLOAD.
                   LVSE_LOG_1("LVSE: KKK LVMQDSP_OPEN_SESSION OFFLOAD session (%d)", session);
                   status =  lvse_get_unused_session_control(session, &session_control);
	            if(status >= 0){
                       LVSE_LOG_1("LVSE: KKK lvse_routing_set_control : LVMQDSP_OPEN_SESSION foundSessionIndex(%d)", status);                
	            }else{
   	                pr_err("\nLVSE: lvse_routing_set_control : Don't get new session control because of fully used session");
	   	         return LVMQDSP_ERROR;
	            }
		     return LVMQDSP_SUCCESS;
	      }else{
	         // if gSession.ac is NULL, this process is NON-OFFLOAD.  
                   LVSE_LOG_1("LVSE: KKK LVMQDSP_OPEN_SESSION NON-OFFLOAD session (%d)", session);
		     status =  lvse_get_unused_session_control(session, &session_control);
	            if(status >= 0){
		          LVSE_LOG_1("LVSE: KKK lvse_routing_set_control : LVMQDSP_OPEN_SESSION foundSessionIndex(%d)", status);                
		     }else{
			  pr_err("\nLVSE: lvse_routing_set_control : Don't get new session control because of fully used session");
		         return LVMQDSP_ERROR;
		     }
		     return LVMQDSP_SUCCESS;
	      }
#else            
		if(gSessions.ac != NULL)
			return LVMQDSP_SUCCESS;
		else
			return LVMQDSP_ERROR;
#endif
        case LVMQDSP_CLOSE_SESSION:
        {
            LVSE_LOG_0("LVSE: LVMQDSP_CLOSE_SESSION");
#ifdef LVSE_OFFLOAD  // Release all Effect, so initialize session control.
	     session_index = lvse_get_session_control(session, &session_control);

         if(NULL == session_control){
 	        pr_err("\nLVSE: lvse_routing_set_control : session control is NULL");
            return LVMQDSP_ERROR;
         }

	     if(session_index != -1){
               int effect_count = lvse_get_effect_count(session_control);
               if(0 == effect_count){
               	session_index = lvse_release_used_session_control(session);
                    LVSE_LOG_1("LVSE: LVMQDSP_CLOSE_SESSION effect_count(%d)", effect_count);
               }

               if(session_index >= 0){
                     LVSE_LOG_1("LVSE: Success to release to used session (%d)", session);
                     return LVMQDSP_SUCCESS; 
               }else{
                     LVSE_LOG_1("LVSE: Fail to release to used session (%d)", session);
                     return LVMQDSP_ERROR;      
               }
	     }else{
	        pr_err("\nLVSE: lvse_routing_set_control : Don't get audio client 01");
	        return LVMQDSP_ERROR;
	     }
#else
            return LVMQDSP_SUCCESS;
#endif
        }    

#ifdef LVSE_OFFLOAD
        case LVMQDSP_SET_OFFLOAD_PARAMS:
        {
            LVMQDSP_OffloadParams_t offloadedParams;
            LVSE_LOG_0("LVSE: KKK LVMQDSP_SET_OFFLOAD_PARAMS");
            if(copy_from_user(&offloadedParams, lvm->cmdParams, sizeof(LVMQDSP_OffloadParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_INSTANCE_PARAMS - read error \n");
                break;
            }

	     session_index = lvse_get_session_control(session, &session_control);
            if(session_index == -1){
                  pr_err("\nLVSE: lvse_routing_set_control : Don't set session control 01");
                  return LVMQDSP_ERROR;
            }else{
               if(NULL == session_control){
	              pr_err("\nLVSE: lvse_routing_set_control : session control is NULL");
       	          return LVMQDSP_ERROR;
               }

	           if(offloadedParams.isOffloaded){
	                LVSE_LOG_1("LVSE: KKK LVMQDSP_SET_OFFLOAD_PARAMS isOffloaded OFFLOAD session(%d)", session);
	                // set offloading audio client
	                session_control->ac = &(gSessions.ac);

	                if(session_control->session != offloadedParams.session)
                       {
       	                session_control->session = offloadedParams.session;
	                       LVSE_LOG_1("LVSE: KKK LVMQDSP_SET_OFFLOAD_PARAMS changed session(%d)", session_control->session);
	                }
	                session_control->offloadFlag = LVM_OFFLOAD;
	                gSettingEffectControlFlag = LVM_TRUE;  //  Set offload mode flag to set effect control to DSP.// Check it !!=> Do set each other session? 
	                gSessionOffload = session;
                  }else{
                       LVSE_LOG_1("LVSE: KKK LVMQDSP_SET_OFFLOAD_PARAMS isOffloaded NON-OFFLOAD session(%d)", session);

	                session_control->session = session;
	                session_control->offloadFlag = LVM_NON_OFFLOAD;
	                gSettingEffectControlFlag = LVM_FALSE;
	                gSessionOffload =  -1;
                  }
                  return LVMQDSP_SUCCESS;
            }
        }
#endif          
    }

    if (gSessions.ac == NULL)
    {
#ifdef ENABLE_PARAM_LOG_OFFLOAD	
	  pr_err("\nLVSE: lvse_routing_set_control : audio client is NULL %d", session);
#endif
#ifdef LVSE_OFFLOAD
	   //CHECKME : Do process mutex_lock?  Calling copy_from_user() do sleep for a while..
	   // I know start/end position on NON-Offload, they are  LVMQDSP_OPEN_SESSION and LVMQDSP_CLOSE_SESSION.
         return lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif	       
    }


    // When gSettingEffectControlFlag is LVM_TRUE, Set effect control to DSP.

    //LVSE_LOG_0("LVSE: lvse_routing_set_control : calling lvse_apply_setting_lvm_control_to_dsp()");
    //lvse_apply_setting_lvm_control_to_dsp(LVM_OFFLOAD, session);  // apply effect setting information to DSP.
#ifdef LVSE_OFFLOAD
/////// CHECK ME!! START  /////////////////////
    session_index = lvse_get_session_control(session, &session_control);
    if(session_index == -1){
          pr_err("\nLVSE: lvse_routing_set_control : Don't set session control 02");
          return LVMQDSP_ERROR;
    }

    if(NULL == session_control){
	      pr_err("\nLVSE: lvse_routing_set_control : session control is NULL");
          return LVMQDSP_ERROR;
    }

    if(LVM_OFFLOAD != session_control->offloadFlag){
          // Don't process DSP (Here is ARM to have the audio client), ARM doesn't send effect control to DSP.
          return lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
    }
/////// CHECK ME!! END  /////////////////////
#endif
    LVSE_LOG_0("LVSE: lock\n");
    mutex_lock(&gSessions.lock);
    lvse_ac = gSessions.ac;    


    switch(lvm->cmd)
    {
        case LVMQDSP_SET_INSTANCE_PARAMS:
        {
            LVMQDSP_InstParams_t instParams;
            LVSE_LOG_1("LVSE: LVMQDSP_SET_INSTANCE_PARAMS - size=%d \n",sizeof(LVMQDSP_InstParams_t));
            if(copy_from_user(&instParams, lvm->cmdParams, sizeof(LVMQDSP_InstParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_INSTANCE_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_INSTANCE_CTRL,
                (void*)&instParams, sizeof(LVMQDSP_InstParams_t));
            LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);

#ifdef LVSE_OFFLOAD
#ifdef ENABLE_PARAM_LOG_OFFLOAD	  
            pr_debug("LVSE: Calling lvse_set_setting_lvm_control_in_kernel on DSP ");
#endif
            // if gSessions.ac is not NULL, this process is offload mode, then save LVM control setting to global variable in kernel. (calling msm-compress-q6-v2.c)
            lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif                      
            break;
        }
        case LVMQDSP_SET_CONTROL_PARAMS:
        {
            LVMQDSP_ControlParams_t params;
            LVSE_LOG_2("LVSE: LVMQDSP_SET_CONTROL_PARAMS - size=%d, %d \n",sizeof(LVMQDSP_ControlParams_t), sizeof(params) );
            if(copy_from_user(& params, lvm->cmdParams, sizeof(LVMQDSP_ControlParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_CONTROL_PARAMS - read error \n");
                break;
            }
#ifdef ENABLE_PARAM_DUMP
            LVSE_LOG_1("LVSE: ####### start CHECK user  session(%d)", session);
            lvse_dump_control_params(lvm->cmdParams);
            LVSE_LOG_1("LVSE: ####### end CHECK user  session(%d)", session);
#endif /* ENABLE_PARAM_DUMP */

            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_EFFECT_CTRL,
                (void*)&params, sizeof(LVMQDSP_ControlParams_t));
            LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);
#ifdef ENABLE_PARAM_DUMP
            LVSE_LOG_1("LVSE: ####### start CHECK q6asm_set_pp_params  session(%d)", session);
            lvse_dump_control_params(&params);
            LVSE_LOG_1("LVSE: ####### end CHECK q6asm_set_pp_params  session(%d)", session);
#endif /* ENABLE_PARAM_DUMP */

 #ifdef LVSE_OFFLOAD
             LVSE_LOG_0("LVSE: Calling lvse_set_setting_lvm_control_in_kernel on DSP ");
            // if gSessions.ac is not NULL, this process is offload mode, then save LVM control setting to global variable in kernel. (calling msm-compress-q6-v2.c)
            lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif                      
            break;
        }
        case LVMQDSP_SET_HEADROOM_PARAMS:
        {
            LVMQDSP_HeadroomParams_t params;

            LVSE_LOG_2("LVSE: LVMQDSP_SET_HEADROOM_PARAMS - size=%d, %d \n",sizeof(LVMQDSP_HeadroomParams_t), sizeof(params) );
            if(copy_from_user(&params, lvm->cmdParams, sizeof(LVMQDSP_HeadroomParams_t)))
            {
                pr_err("LVSE: LVMQDSP_SET_HEADROOM_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_HEADROOM_CTRL,
                (void*)&params, sizeof(LVMQDSP_HeadroomParams_t));
            LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);

 #ifdef LVSE_OFFLOAD
             LVSE_LOG_0("LVSE: Calling lvse_set_setting_lvm_control_in_kernel on DSP ");
            // if gSessions.ac is not NULL, this process is offload mode, then save LVM control setting to global variable in kernel. (calling msm-compress-q6-v2.c)
            lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif                                  
            break;
        }
        case LVMQDSP_SET_TUNING_PARAMS:
        {
            LVMQDSP_Custom_TuningParams_st params;

            LVSE_LOG_1("LVSE: LVMQDSP_SET_TUNING_PARAMS - size=%d \n",sizeof(LVMQDSP_Custom_TuningParams_st));
            if(copy_from_user(&params, lvm->cmdParams, sizeof(LVMQDSP_Custom_TuningParams_st)))
            {
                pr_err("LVSE: LVMQDSP_SET_TUNING_PARAMS - read error \n");
                break;
            }
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_TUNING_CTRL,
                (void*)&params, sizeof(LVMQDSP_Custom_TuningParams_st));
            LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);

 #ifdef LVSE_OFFLOAD
             LVSE_LOG_0("LVSE: Calling lvse_set_setting_lvm_control_in_kernel on DSP ");
            // if gSessions.ac is not NULL, this process is offload mode, then save LVM control setting to global variable in kernel. (calling msm-compress-q6-v2.c)
            lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif                                  
            break;
        }
	case LVMQDSP_CLEAR_AUDIO_BUFFER:
	{
            LVSE_LOG_0("LVSE: LVMQDSP_CLEAR_AUDIO_BUFFER\n");
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_CLEAR_AUDIO_BUFFER_CTRL,
                NULL, 0);
            LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);
		break;
	}
	case LVMQDSP_SET_REVERB_INSTANCE_PARAMS:
	{
		LVMQDSP_RevInstParams_t instParams;

		LVSE_LOG_1("LVSE: LVMQDSP_SET_REVERB_INSTANCE_PARAMS - size=%d \n",sizeof(LVMQDSP_RevInstParams_t));
		if(copy_from_user(&instParams, lvm->cmdParams, sizeof(LVMQDSP_RevInstParams_t)))
		{
			pr_err("LVSE: LVMQDSP_SET_REVERB_INSTANCE_PARAMS - read error \n");
			break;
		}
		status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL,
			(void*)&instParams, sizeof(LVMQDSP_RevInstParams_t));
		LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);

#ifdef LVSE_OFFLOAD
            LVSE_LOG_0("LVSE: Calling lvse_set_setting_lvm_control_in_kernel on DSP ");
               // if gSessions.ac is not NULL, this process is offload mode, then save LVM control setting to global variable in kernel. (calling msm-compress-q6-v2.c)
               lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif
		break;
	}
	case LVMQDSP_SET_REVERB_CONTROL_PARAMS:
	{
		LVMQDSP_RevControlParams_t params;

		LVSE_LOG_2("LVSE: LVMQDSP_SET_REVERB_CONTROL_PARAMS - size=%d, %d \n",sizeof(LVMQDSP_RevControlParams_t), sizeof(params) );
		if(copy_from_user(& params, lvm->cmdParams, sizeof(LVMQDSP_RevControlParams_t)))
		{
			pr_err("LVSE: LVMQDSP_SET_REVERB_CONTROL_PARAMS - read error \n");
			break;
		}
		status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL,
			(void*)&params, sizeof(LVMQDSP_RevControlParams_t));
		LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);
#ifdef ENABLE_PARAM_DUMP
		lvse_dump_control_params(&params);
#endif /* ENABLE_PARAM_DUMP */

#ifdef LVSE_OFFLOAD
            LVSE_LOG_0("LVSE: Calling lvse_set_setting_lvm_control_in_kernel on DSP ");
               // if gSessions.ac is not NULL, this process is offload mode, then save LVM control setting to global variable in kernel. (calling msm-compress-q6-v2.c)
               lvse_set_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
#endif                      
		break;
	}
	case LVMQDSP_REVERB_CLEAR_AUDIO_BUFFER:
	{
            LVSE_LOG_0("LVSE: LVMQDSP_REVERB_CLEAR_AUDIO_BUFFER\n");
            status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_CLEAR_AUDIO_BUFFER_CTRL,
                NULL, 0);
            LVSE_LOG_1("LVSE: q6asm_set_pp_params returned %d", status);
			break;
	}

        default:
            LVSE_LOG_1("LVSE: Unknown lvse_routing_set_control command %d \n", lvm->cmd);
    }

    LVSE_LOG_0("LVSE: unlock\n");
    mutex_unlock(&gSessions.lock);
    return status;
}
//----------------------------------------------------------------------------
// lvse_routing_get_control()
//----------------------------------------------------------------------------
// Purpose: Get parameter from the QDSP. After the q6asm_get_pp_params() is called,
//          the data is passed to the lvse_callback() with opcode. And the data is
//          copied to the user space here.
//
// Inputs:
//  kcontrol
//  ucontrol      	User data to get
//
// Outputs:
//  ucontrol        updated params
//----------------------------------------------------------------------------
int lvse_routing_get_control(struct snd_kcontrol       *kcontrol,
                             struct snd_ctl_elem_value *ucontrol) {

    int status = LVMQDSP_SUCCESS;
    int session_index = 0;
    int rc;
    int qdsp_param_type;
    void* output_params;
    int param_size;
    LVMQDSP_Command_t *lvm;
    LVMQDSP_Session_t session;
    struct audio_client *lvse_ac;
#ifdef LVSE_OFFLOAD
    LVMQDSP_SessionControl_t *session_control = NULL;
 #endif    

    lvm = (LVMQDSP_Command_t*)ucontrol->value.bytes.data;

    LVSE_LOG_1("LVSE: cmd = %d ###############################################\n", lvm->cmd);
    session = lvm->session;

#ifdef LVSE_OFFLOAD    
    lvse_session_control_init();   // initialize global session control information
#endif
    
    if (gSessions.ac == NULL)
    {
#ifdef ENABLE_PARAM_LOG_OFFLOAD  
        pr_err("\nLVSE: lvse_routing_get_control : audio client is NULL (%d), cmd (%d)", session,  lvm->cmd);
#endif
#ifdef LVSE_OFFLOAD
	 {
#ifdef ENABLE_PARAM_LOG_OFFLOAD	  	 
            pr_err("\nLVSE: calling LVM_NON_OFFLOAD : lvse_get_setting_lvm_control_in_kernel()");
#endif
            return lvse_get_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
	 }
#endif	 
        return LVMQDSP_ERROR;
    }

    // When gSettingEffectControlFlag is LVM_TRUE, Set effect control to DSP.
    //LVSE_LOG_0("LVSE: lvse_routing_get_control : calling lvse_apply_setting_lvm_control_to_dsp()");
    //lvse_apply_setting_lvm_control_to_dsp(LVM_OFFLOAD, session);  // apply effect setting information to DSP.
#ifdef LVSE_OFFLOAD
/////// CHECK ME!! START  /////////////////////
    session_index = lvse_get_session_control(session, &session_control);
    if(session_index == -1){
          pr_err("\nLVSE: lvse_routing_set_control : Don't set session control 02");
          return LVMQDSP_ERROR;
    }

    if(NULL == session_control){
	      pr_err("\nLVSE: lvse_routing_set_control : session control is NULL");
          return LVMQDSP_ERROR;
    }

    if(LVM_OFFLOAD != session_control->offloadFlag){
          // Don't process DSP (Here is ARM to have the audio client), ARM doesn't send effect control to DSP.
          return lvse_get_setting_lvm_control_in_kernel(lvm->cmd, lvm->cmdParams, session);
    }
/////// CHECK ME!! END  /////////////////////
#endif

    
    lvse_ac = gSessions.ac; 
    
    switch (lvm->cmd) {
    case LVMQDSP_GET_INSTANCE_PARAMS:   LVSE_LOG_0("LVSE: LVMQDSP_GET_INSTANCE_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_INSTANCE_CTRL;
                                        param_size = sizeof(LVMQDSP_InstParams_t);
                                        output_params = (void*)&gSessions.inst_params;
                                        memset(&gSessions.inst_params, 1, sizeof(LVMQDSP_InstParams_t));
                                        break;
    case LVMQDSP_GET_CONTROL_PARAMS:    LVSE_LOG_0("LVSE: LVMQDSP_GET_CONTROL_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_EFFECT_CTRL;
                                        param_size = sizeof(LVMQDSP_ControlParams_t);
                                        output_params = (void*)&gSessions.control_params;
                                        memset(&gSessions.control_params, 1, sizeof(LVMQDSP_ControlParams_t));
                                        break;
    case LVMQDSP_GET_HEADROOM_PARAMS:   LVSE_LOG_0("LVSE: LVMQDSP_GET_HEADROOM_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_HEADROOM_CTRL;
                                        param_size = sizeof(LVMQDSP_HeadroomParams_t);
                                        output_params = (void*)&gSessions.headroom_params;
                                        memset(&gSessions.headroom_params, 1, sizeof(LVMQDSP_HeadroomParams_t));
                                        break;
    case LVMQDSP_GET_TUNING_PARAMS:     LVSE_LOG_0("LVSE: LVMQDSP_GET_TUNING_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_TUNING_CTRL;
                                        param_size = sizeof(LVMQDSP_Custom_TuningParams_st);
                                        output_params = (void*)&gSessions.tuning_params;
                                        memset(&gSessions.tuning_params, 1, sizeof(LVMQDSP_Custom_TuningParams_st));
                                        break;
    case LVMQDSP_GET_SPECTRUM_PARAMS:   LVSE_LOG_0("LVSE: LVMQDSP_GET_SPECTRUM_PARAMS\n");
                                        qdsp_param_type = PARAM_ID_NXP_LVM_GET_SPECTRUM_CTRL;
                                        param_size = sizeof(LVMQDSP_PSA_SpectrumParams_st);
                                        output_params = (void*)&gSessions.spectrum_params;
                                        memset(&gSessions.spectrum_params, 1, sizeof(LVMQDSP_PSA_SpectrumParams_st));
                                        break;
	case LVMQDSP_GET_REVERB_INSTANCE_PARAMS:	LVSE_LOG_0("LVSE: LVMQDSP_GET_INSTANCE_PARAMS\n");
										qdsp_param_type = LVMQDSP_GET_REVERB_INSTANCE_PARAMS;
										param_size = sizeof(LVMQDSP_RevInstParams_t);
										output_params = (void*)&gSessions.reverb_inst_params;
										memset(&gSessions.reverb_inst_params, 1, sizeof(LVMQDSP_RevInstParams_t));
										break;
	case LVMQDSP_GET_REVERB_CONTROL_PARAMS:	LVSE_LOG_0("LVSE: LVMQDSP_GET_REVERB_CONTROL_PARAMS\n");
										qdsp_param_type = PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL;
										param_size = sizeof(LVMQDSP_RevControlParams_t);
										output_params = (void*)&gSessions.reverb_control_params;
										memset(&gSessions.reverb_control_params, 1, sizeof(LVMQDSP_RevControlParams_t));
										break;

    default:    pr_err("LVSE: lvse_routing_get_control, unknown param %d", lvm->cmd);
                return LVMQDSP_ERROR;
    }
    
    LVSE_LOG_0("LVSE: mutex lock\n");
    mutex_lock(&gSessions.lock);

    LVSE_LOG_1("LVSE: lvse_routing_get_control : q6asm_get_pp_params session (%d)", session);

    if((lvm->cmd == LVMQDSP_GET_REVERB_INSTANCE_PARAMS) || (lvm->cmd == LVMQDSP_GET_REVERB_CONTROL_PARAMS))
    {
	 rc = q6asm_get_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, qdsp_param_type, param_size);
    }
    else
    {
        rc = q6asm_get_pp_params(lvse_ac, MODULE_ID_NXP_LVM, qdsp_param_type, param_size);
    }

    LVSE_LOG_1("LVSE: q6asm_get_pp_params returned %d", status);
    if(rc != 0){
		pr_err("LVSE: %s: q6asm_get_pp_params failed\n", __func__);
        goto exit;
	}
    LVSE_LOG_1("LVSE: lvse_routing_get_control: params_count = %d", atomic_read(&gSessions.count));
    rc = wait_event_timeout(gSessions.wait, atomic_read(&gSessions.count), 5 * HZ);
    atomic_set(&gSessions.count, 0);
    if(!rc){
        pr_err("LVSE: %s: wait_event_timeout failed\n", __func__);
        goto exit;
    }

#if 0//def LVSE_OFFLOAD
    //Update control by DSP content
    if(LVM_OFFLOAD == gProcessingInKernel.processMode)
    {
        switch(lvm->cmd)
        {
		case LVMQDSP_GET_INSTANCE_PARAMS:
		{
                     memcpy((void*)&(gProcessingInKernel.instParams),output_params , sizeof(LVMQDSP_InstParams_t));  
		}
		break;
        }
    }
#endif

    if(copy_to_user(lvm->cmdParams, output_params, param_size))
    {
        pr_err("LVSE: LVMQDSP_GET_CONTROL_PARAMS write error \n");
        status = LVMQDSP_ERROR;
        goto exit;
    }

exit:
    LVSE_LOG_0("LVSE: mutex unlock\n");
    mutex_unlock(&gSessions.lock);
    return status;
}

#ifdef ENABLE_PARAM_DUMP
void lvse_dump_control_params(void* ptr) {

    LVMQDSP_ControlParams_t *qdsp_params = (LVMQDSP_ControlParams_t*)ptr;
    LVM_ControlParams_t *params = &qdsp_params->LvmParams;
    LVM_EQNB_BandDef_t  *bands = &qdsp_params->LvmBands[0];
    int i = 0;
    int eqnb = 0;

    pr_debug("\nLVSE: ControlParams.OperatingMode = 0x%lx",   (long unsigned int)params->OperatingMode);
    pr_debug("LVSE: ControlParams.SampleRate = 0x%lx",      (long unsigned int)params->SampleRate);
    pr_debug("LVSE: ControlParams.SourceFormat = 0x%lx",    (long unsigned int)params->SourceFormat);
    pr_debug("LVSE: ControlParams.SpeakerType = 0x%lx",     (long unsigned int)params->SpeakerType);

#ifdef ALGORITHM_CS
    /* Concert Sound Virtualizer parameters*/
    pr_debug("\nLVSE: ControlParams.VirtualizerOperatingMode = 0x%lx",    (long unsigned int)params->VirtualizerOperatingMode);
    pr_debug("LVSE: ControlParams.VirtualizerType = 0x%lx",             (long unsigned int)params->VirtualizerType);
    pr_debug("LVSE: ControlParams.VirtualizerReverbLevel = 0x%lx",      (long unsigned int)params->VirtualizerReverbLevel);
    pr_debug("LVSE: ControlParams.CS_EffectLevel = 0x%lx",             (long unsigned int) params->CS_EffectLevel);
#endif /* ALGORITHM_CS */

#ifdef ALGORITHM_CI
    /* Concert Sound Virtualizer parameters*/
    pr_debug("\nLVSE: ControlParams.VirtualizerOperatingMode = 0x%lx",   (long unsigned int) params->VirtualizerOperatingMode);
    pr_debug("LVSE: ControlParams.VirtualizerType = 0x%lx",            (long unsigned int) params->VirtualizerType);
    pr_debug("LVSE: ControlParams.VirtualizerReverbLevel = 0x%lx",     (long unsigned int) params->VirtualizerReverbLevel);
    pr_debug("LVSE: ControlParams.CS_EffectLevel = 0x%lx",             (long unsigned int) params->CS_EffectLevel);
#endif /* ALGORITHM_CI */

    /* Bass Enhancement parameters */
    pr_debug("\nLVSE: ControlParams.BE_OperatingMode = 0x%lx",   (long unsigned int)params->BE_OperatingMode);
    pr_debug("LVSE: ControlParams.BE_EffectLevel = 0x%lx",     (long unsigned int) params->BE_EffectLevel);
    pr_debug("LVSE: ControlParams.BE_CentreFreq = 0x%lx",    (long unsigned int)params->BE_CentreFreq);
    pr_debug("LVSE: ControlParams.BE_HPF = 0x%lx",     (long unsigned int)params->BE_HPF);

    /* volume control */
    pr_debug("LVSE: ControlParams.VC_EffectLevel = 0x%lx",   (long unsigned int)params->VC_EffectLevel);
    pr_debug("LVSE: ControlParams.VC_Balance = 0x%lx",    (long unsigned int) params->VC_Balance);

     /* Treble Enhancement parameters */
    pr_debug("LVSE: ControlParams.TE_OperatingMode = 0x%lx",   (long unsigned int) params->TE_OperatingMode);
    pr_debug("LVSE: ControlParams.TE_EffectLevel = 0x%lx",    (long unsigned int) params->TE_EffectLevel);

   /* LM */
    pr_debug("LVSE: ControlParams.LM_OperatingMode = 0x%lx",   (long unsigned int) params->LM_OperatingMode);
    pr_debug("LVSE: ControlParams.LM_EffectLevel = 0x%lx",    (long unsigned int) params->LM_EffectLevel);
    pr_debug("LVSE: ControlParams.LM_Attenuation = 0x%lx",   (long unsigned int) params->LM_Attenuation);
    pr_debug("LVSE: ControlParams.LM_CompressorGain = 0x%lx",    (long unsigned int) params->LM_CompressorGain);
    pr_debug("LVSE: ControlParams.LM_SpeakerCutOff = 0x%lx",   (long unsigned int) params->LM_SpeakerCutOff);
	   
    /* AVL/ PSA */
    pr_debug("LVSE: ControlParams.AVL_OperatingMode = 0x%lx",    (long unsigned int) params->AVL_OperatingMode);
    pr_debug("LVSE: ControlParams.PSA_PeakDecayRate = 0x%lx",   (long unsigned int) params->PSA_PeakDecayRate);
    pr_debug("LVSE: ControlParams.PSA_NumBands = 0x%lx",    (long unsigned int) params->PSA_NumBands);
    pr_debug("LVSE: ControlParams.PSA_Enable = 0x%lx",   (long unsigned int) params->PSA_Enable);

    ///* N-Band Equaliser parameters */
    pr_debug("\nLVSE: ControlParams.EQNB_OperatingMode = 0x%lx",   (long unsigned int) params->EQNB_OperatingMode);    
    pr_debug("LVSE: ControlParams.EQNB_LPF_Mode = 0x%lx",     (long unsigned int)   params->EQNB_LPF_Mode);    
    pr_debug("LVSE: ControlParams.EQNB_LPF_CornerFreq = 0x%lx",  (long unsigned int) params->EQNB_LPF_CornerFreq);    
    pr_debug("LVSE: ControlParams.EQNB_HPF_Mode = 0x%lx",        (long unsigned int) params->EQNB_HPF_Mode);  
    pr_debug("LVSE: ControlParams.EQNB_HPF_CornerFreq = 0x%lx",   (long unsigned int) params->EQNB_HPF_CornerFreq);        
		    
    pr_debug("\nLVSE: ControlParams.EQNB_NBands = 0x%lx",          (long unsigned int) params->EQNB_NBands);

    eqnb = (int)params->EQNB_NBands;

    for(i=0 ; i < eqnb ; i++)
    {
       pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[%d].Gain = 0x%lx",  i,  (long unsigned int) bands[i].Gain);    
       pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[%d].Frequency = 0x%lx", i, (long unsigned int) bands[i].Frequency);    
       pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[%d].QFactor = 0x%lx", i, (long unsigned int) bands[i].QFactor);    
    }

#if 0//def ALGORITHM_EQNB
    ///* N-Band Equaliser parameters */
    pr_debug("\nLVSE: ControlParams.EQNB_OperatingMode = %d",    params->EQNB_OperatingMode);    
    pr_debug("LVSE: ControlParams.EQNB_LPF_Mode = %d",         params->EQNB_LPF_Mode);    
    pr_debug("LVSE: ControlParams.EQNB_LPF_CornerFreq = %d",   params->EQNB_LPF_CornerFreq);    
    pr_debug("LVSE: ControlParams.EQNB_HPF_Mode = %d",         params->EQNB_HPF_Mode);  
    pr_debug("LVSE: ControlParams.EQNB_HPF_CornerFreq = %d",    params->EQNB_HPF_CornerFreq);        
    
    pr_debug("\nLVSE: ControlParams.EQNB_NBands = %d",           params->EQNB_NBands);
    pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[9].Gain = %d",      bands[9].Gain);    
    pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[9].Frequency = %d", bands[9].Frequency);    
    pr_debug("LVSE: ControlParams.pEQNB_BandDefinition[9].QFactor = %d",   bands[9].QFactor);    
    
    //LVM_EQNB_FilterMode_en      EQNB_LPF_Mode;          /* Low pass filter */
    //LVM_INT16                   EQNB_LPF_CornerFreq;
    //LVM_EQNB_FilterMode_en      EQNB_HPF_Mode;          /* High pass filter */
    //LVM_INT16                   EQNB_HPF_CornerFreq;
    //LVM_UINT16                  EQNB_NBands;            /* Number of bands */
    //LVM_EQNB_BandDef_t          *pEQNB_BandDefinition;  /* Pointer to equaliser definitions */

#endif /* ALGORITHM_EQNB */

//#ifdef ALGORITHM_DBE
    ///* Bass Enhancement parameters */
    //LVM_BE_Mode_en              BE_OperatingMode;       /* Bass Enhancement operating mode */
    //LVM_INT16                   BE_EffectLevel;         /* Bass Enhancement effect level */
    //LVM_BE_CentreFreq_en        BE_CentreFreq;          /* Bass Enhancement centre frequency */
    //LVM_BE_FilterSelect_en      BE_HPF;                 /* Bass Enhancement high pass filter selector */

//#endif /* ALGORITHM_DBE */
//#ifdef ALGORITHM_PB
    ///* Bass Enhancement parameters */
    //LVM_BE_Mode_en              BE_OperatingMode;       /* Bass Enhancement operating mode */
    //LVM_INT16                   BE_EffectLevel;         /* Bass Enhancement effect level */
    //LVM_BE_CentreFreq_en        BE_CentreFreq;          /* Bass Enhancement centre frequency */
    //LVM_BE_FilterSelect_en      BE_HPF;                 /* Bass Enhancement high pass filter selector */

//#endif /* ALGORITHM_PB */
    ///* Volume Control parameters */
    //LVM_INT16                   VC_EffectLevel;         /* Volume Control setting in dBs */
    //LVM_INT16                   VC_Balance;             /* Left Right Balance control in dB (-96 to 96 dB), -ve values reduce */

//#ifdef ALGORITHM_TE
    ///* Treble Enhancement parameters */
    //LVM_TE_Mode_en              TE_OperatingMode;       /* Treble Enhancement On/Off */
    //LVM_INT16                   TE_EffectLevel;         /* Treble Enhancement gain dBs */

//#endif /* ALGORITHM_TE */
//#ifdef ALGORITHM_LM
    ///* Loudness Maximiser parameters */
    //LVM_LM_Mode_en              LM_OperatingMode;       /* Loudness Maximiser operating mode */
    //LVM_LM_Effect_en            LM_EffectLevel;         /* Loudness Maximiser effect level */
    //LVM_UINT16                  LM_Attenuation;         /* Loudness Maximiser output attenuation */
    //LVM_UINT16                  LM_CompressorGain;      /* Loudness Maximiser output compressor gain */
    //LVM_UINT16                  LM_SpeakerCutOff;       /* Loudness Maximiser speaker cut off frequency */

//#endif /* ALGORITHM_LM */
//#ifdef ALGORITHM_GM
    ///* Gentle Mix parameters */
    //LVM_GM_Mode_en              GM_OperatingMode;       /* Gentle mix operating mode */
    //LVM_GM_Effect_en            GM_EffectLevel;         /* Gentle mix effect level */

//#endif /* ALGORITHM_GM */
//#ifdef ALGORITHM_RS
    ///* Richsound parameters */
    //LVM_RS_Mode_en              RS_OperatingMode;       /* Richsound operating mode */
    //LVM_RS_Config_en            RS_Config;              /* Richsound configuration */
    //LVM_RS_AVLEffect_en         RS_AVLEffect;           /* Richsound auto volume effect */
    //LVM_INT16                   RS_EQBandLow;           /* Richsound low frequency band setting */
    //LVM_INT16                   RS_EQBandMid;           /* Richsound middle frequency band setting */
    //LVM_INT16                   RS_EQBandHigh;          /* Richsound high frequency band setting */

//#endif /* ALGORITHM_RS */
//#ifdef ALGORITHM_AVL
    ///* AVL parameters */
    //LVM_AVL_Mode_en             AVL_OperatingMode;      /* AVL operating mode */

//#endif /* ALGORITHM_AVL */
//#ifdef ALGORITHM_TG
    ///* Tone Generator parameters */
    //LVM_TG_Mode_en              TG_OperatingMode;       /* Tone generator mode */
    //LVM_TG_SweepMode_en         TG_SweepMode;           /* Log or linear sweep */
    //LVM_UINT16                  TG_StartFrequency;      /* Sweep start frequency in Hz */
    //LVM_INT16                   TG_StartAmplitude;      /* Sweep start amplitude in dBr */
    //LVM_UINT16                  TG_StopFrequency;       /* Sweep stop frequency in Hz */
    //LVM_INT16                   TG_StopAmplitude;       /* Sweep stop amplitude in dBr */
    //LVM_UINT16                  TG_SweepDuration;       /* Sweep duration in seconds, 0 for infinite duration tone */
    //LVM_Callback                pTG_CallBack;           /* End of sweep callback */
    //LVM_INT16                   TG_CallBackID;          /* Callback ID*/
    //void                        *pTGAppMemSpace;        /* Application instance handle or memory area */
//#endif /* ALGORITHM_TG */

//#ifdef ALGORITHM_PSA
    ///* General Control */
    //LVM_PSA_Mode_en             PSA_Enable;

    ///* Spectrum Analyzer parameters */
    //LVM_PSA_DecaySpeed_en       PSA_PeakDecayRate;      /* Peak value decay rate*/
    //LVM_UINT16                  PSA_NumBands;           /* Number of Bands*/l

//#endif /* ALGORITHM_PSA */

    pr_debug("\n");
}


void lvse_dump_tuning_params(void* ptr) 
{

    LVMQDSP_Custom_TuningParams_st *qdsp_params = (LVMQDSP_Custom_TuningParams_st*)ptr;
    LVM_Custom_TuningParams_st *params = &qdsp_params->LvmParams;
    LVM_EQNB_BandDef_t         *bands = &qdsp_params->LvmBands[0];

    pr_debug("\nLVSE: TuningParams.MidGain = %d",   params->MidGain);
    pr_debug("LVSE: TuningParams.MidCornerFreq = %d",   params->MidCornerFreq);
    pr_debug("LVSE: TuningParams.SideHighPassCutoff = %d",   params->SideHighPassCutoff);
    pr_debug("LVSE: TuningParams.SideLowPassCutoff = %d",   params->SideLowPassCutoff);
    pr_debug("LVSE: TuningParams.SideGain = %d",   params->SideGain);
    
    // LVM_INT16                   Elevation;          /* Virtual speaker elevation in degrees */
    // LVM_INT16                   FrontAngle;         /* Front speaker angle */
    // LVM_INT16                   SurroundAngle;      /* Surround speaker angle */

    // LVM_EQNB_FilterMode_en      Tuning_HPF_Mode;    /* High pass filter */
    // LVM_INT16                   Tuning_HPF_CornerFreq;
    
    pr_debug("\nLVSE: TuningParams.NumTuningBands = %d",   params->NumTuningBands);
    pr_debug("LVSE: TuningParams.pTuningBandDefs[0].Gain = %d",      bands[0].Gain);    
    pr_debug("LVSE: TuningParams.pTuningBandDefs[0].Frequency = %d", bands[0].Frequency);    
    pr_debug("LVSE: TuningParams.pTuningBandDefs[0].QFactor = %d",   bands[0].QFactor);  
 
    pr_debug("\n"); 
}
#endif /* ENABLE_PARAM_DUMP */

#ifdef LVSE_OFFLOAD

int lvse_get_effect_count(LVMQDSP_SessionControl_t *session_control){
    int effect_count = 0;

    if(1 == session_control->settingLVMControlInKernel.controlParams.LvmParams.BE_OperatingMode){
        effect_count++;
    }else if(1 == session_control->settingLVMControlInKernel.controlParams.LvmParams.VirtualizerOperatingMode){
        effect_count++;
    }else if(1 == session_control->settingLVMControlInKernel.controlParams.LvmParams.EQNB_OperatingMode){
        effect_count++;
    }else if(1 == session_control->settingLVMControlInKernel.controlParams.LvmParams.TE_OperatingMode){
        effect_count++;
    }else if(1 == session_control->settingLVMControlInKernel.controlParams.LvmParams.LM_OperatingMode){
        effect_count++;
    }else if(1 == session_control->settingLVMControlInKernel.revControlParams.LvmParams.OperatingMode){
        effect_count++;
    }
    return effect_count;
}

void lvse_session_control_init(){
    int idx = 0;
    if(gSessionControlInitFlag == LVM_FALSE){
        LVSE_LOG_0("LVSE: KKK lvse_session_control_init");
        gSessionControlInitFlag = LVM_TRUE;
        memset(gSessionControls, 0, sizeof(LVMQDSP_SessionControl_t) * LVM_MAX_SESSIONS);
        
        for(idx=0; idx<LVM_MAX_SESSIONS; idx++){
            gSessionControls[idx].useFlag = LVM_UNUSED_SESSION;
            gSessionControls[idx].session = LVM_UNUSED_SESSION;
            gSessionControls[idx].offloadFlag = LVM_NOT_SETTING;
        }
    }
}

int lvse_find_session_control(int session){
      int idx;
      for(idx=0; idx<LVM_MAX_SESSIONS;idx++){
           if(gSessionControls[idx].session == session){
           	return idx;
           }
      }
      return -1; // not found session
}

int lvse_get_unused_session_control(int session, LVMQDSP_SessionControl_t **out_session_control){
    int foundSessionIndex = lvse_find_session_control(session);
    int idx = 0;

    if(-1 == foundSessionIndex){
           // Searching unused session from gSessionControls..
	    for(idx = 0; idx < LVM_MAX_SESSIONS; idx++){
		if(LVM_UNUSED_SESSION == gSessionControls[idx].useFlag){
                  *out_session_control = &(gSessionControls[idx]);
                  foundSessionIndex = idx;

                  // set to use session control information
                  (*out_session_control)->session = session;
                  (*out_session_control)->useFlag = LVM_USED_SESSION;
                  break;
		}
	    }
           if(LVM_MAX_SESSIONS == idx){
		 return -1; // Not support using space for session
           }
    } else {
           // Being used session control about session
           *out_session_control = &(gSessionControls[foundSessionIndex]);
    }
    return foundSessionIndex;
}


int lvse_release_used_session_control(int session){
    int foundSessionIndex = lvse_find_session_control(session);

    // release session control about session
    if(foundSessionIndex >= 0){
	 memset(&(gSessionControls[foundSessionIndex]), 0, sizeof(LVMQDSP_SessionControl_t));
	 gSessionControls[foundSessionIndex].useFlag = LVM_UNUSED_SESSION;
	 gSessionControls[foundSessionIndex].session = LVM_UNUSED_SESSION;
	 gSessionControls[foundSessionIndex].offloadFlag = LVM_NOT_SETTING;
    }else{
        return -1;
    }
    return foundSessionIndex;
}

int lvse_get_session_control(int session, LVMQDSP_SessionControl_t **out_session_control){
    int foundSessionIndex = lvse_find_session_control(session);

    if(foundSessionIndex >= 0){
        *out_session_control = &(gSessionControls[foundSessionIndex]);
    }else{
	 return -1;  // Not found session control
    }
    return foundSessionIndex; // Return to find session index
}


int lvse_set_setting_lvm_control_in_kernel(int cmd, void *param, int session)
{
    int status;
    LVMQDSP_SessionControl_t *session_control = NULL;

    status = lvse_get_session_control(session, &session_control);
    if(-1 == status){
         pr_err("\nLVSE: lvse_set_setting_lvm_control_in_kernel : Don't get session client");
         return LVMQDSP_ERROR;
    }

    if(NULL == session_control){
	     pr_err("\nLVSE: lvse_set_setting_lvm_control_in_kernel : session control is NULL");
         return LVMQDSP_ERROR;
    }

    switch(cmd)
    {
        case LVMQDSP_SET_INSTANCE_PARAMS:
        {
#ifdef ENABLE_PARAM_LOG_OFFLOAD        
            pr_debug("LVSE: (%s) LVMQDSP_SET_INSTANCE_PARAMS - size=%d \n",__func__, sizeof(LVMQDSP_InstParams_t));
#endif
//            memcpy(&(settingLVMControlInKernel.instParams), param, sizeof(LVMQDSP_InstParams_t));  
            if(copy_from_user(&(session_control->settingLVMControlInKernel.instParams), param, sizeof(LVMQDSP_InstParams_t))) // Copy user memory to kernel memory
            {
                pr_err("LVSE:  (%s) LVMQDSP_SET_INSTANCE_PARAMS - read error \n", __func__);
                return LVMQDSP_ERROR;
            }
            
            session_control->settingLVMControlInKernel.isSetInstParams = 1;
            break;
        }
        case LVMQDSP_SET_CONTROL_PARAMS:
        case LVMQDSP_SET_VOLNOSMOOTHING_PARAMS:

        {
#ifdef ENABLE_PARAM_LOG_OFFLOAD        
	     if(cmd == LVMQDSP_SET_CONTROL_PARAMS)     
                pr_debug("LVSE:  (%s) LVMQDSP_SET_CONTROL_PARAMS - size=%d \n",__func__, sizeof(LVMQDSP_ControlParams_t));
            else
                pr_debug("LVSE:  (%s) LVMQDSP_SET_VOLNOSMOOTHING_PARAMS - size=%d \n",__func__, sizeof(LVMQDSP_ControlParams_t));
#endif
#ifdef ENABLE_PARAM_DUMP        
            pr_debug("LVSE:  (%s) START DUMP ========================================================= \n",__func__);
            lvse_dump_control_params(param);
            pr_debug("LVSE:  (%s) END DUMP ========================================================== \n",__func__);
#endif

            if(copy_from_user(&(session_control->settingLVMControlInKernel.controlParams), param, sizeof(LVMQDSP_ControlParams_t)))
            {
                pr_err("LVSE:  (%s) LVMQDSP_SET_CONTROL_PARAMS or LVMQDSP_SET_VOLNOSMOOTHING_PARAMS - read error \n", __func__);
                return LVMQDSP_ERROR;
            }
            session_control->settingLVMControlInKernel.isSetControlParams = 1;
            break;
        }
        case LVMQDSP_SET_HEADROOM_PARAMS:
        {
#ifdef ENABLE_PARAM_LOG_OFFLOAD        
            pr_debug("LVSE: (%s) LVMQDSP_SET_HEADROOM_PARAMS - size=%d \n", __func__, sizeof(LVMQDSP_HeadroomParams_t));
#endif
            if(copy_from_user(&(session_control->settingLVMControlInKernel.headRoomParams), param, sizeof(LVMQDSP_HeadroomParams_t)))
            {
                pr_err("LVSE: (%s) LVMQDSP_SET_HEADROOM_PARAMS - read error \n", __func__);
                return LVMQDSP_ERROR;
            }
            session_control->settingLVMControlInKernel.isSetHeadRoomParams = 1;
            break;
        }
        case LVMQDSP_SET_TUNING_PARAMS:
        {
#ifdef ENABLE_PARAM_LOG_OFFLOAD        
            pr_debug("LVSE: (%s) LVMQDSP_SET_TUNING_PARAMS - size=%d \n", __func__, sizeof(LVMQDSP_Custom_TuningParams_st));
#endif
            if(copy_from_user(&(session_control->settingLVMControlInKernel.customTuningParams), param, sizeof(LVMQDSP_Custom_TuningParams_st)))
            {
                pr_err("LVSE: (%s) LVMQDSP_SET_TUNING_PARAMS - read error \n", __func__);
                return LVMQDSP_ERROR;
            }
            session_control->settingLVMControlInKernel.isSetCustomTuningParams = 1;
            break;
        }
	 case LVMQDSP_SET_REVERB_INSTANCE_PARAMS:
	 {
#ifdef ENABLE_PARAM_LOG_OFFLOAD	 
	     pr_debug("LVSE: (%s) LVMQDSP_SET_REVERB_INSTANCE_PARAMS - size=%d \n", __func__, sizeof(LVMQDSP_RevInstParams_t));
#endif
            if(copy_from_user(&(session_control->settingLVMControlInKernel.revInstParams), param, sizeof(LVMQDSP_RevInstParams_t)))
            {
                pr_err("LVSE: (%s) LVMQDSP_SET_REVERB_INSTANCE_PARAMS - read error \n", __func__);
                return LVMQDSP_ERROR;
            }
            session_control->settingLVMControlInKernel.isSetRevInstParams = 1;
	     break;
	  }
	  case LVMQDSP_SET_REVERB_CONTROL_PARAMS:
	  {
#ifdef ENABLE_PARAM_LOG_OFFLOAD	  
	     pr_debug("LVSE: (%s) LVMQDSP_SET_REVERB_CONTROL_PARAMS - size=%d  \n", __func__ ,sizeof(LVMQDSP_RevControlParams_t));
#endif
            if(copy_from_user(&(session_control->settingLVMControlInKernel.revControlParams), param, sizeof(LVMQDSP_RevControlParams_t)))
            {
                pr_err("LVSE: (%s) LVMQDSP_SET_REVERB_CONTROL_PARAMS - read error \n", __func__ );
                return LVMQDSP_ERROR;
            }
            session_control->settingLVMControlInKernel.isSetRevControlParams = 1;
	     break;
	  }

        default:
            pr_debug("LVSE: Unknown lvse_set_setting_lvm_control_in_kernel command %d \n", cmd);
            return LVMQDSP_ERROR;
    }

#ifdef ENABLE_PARAM_DUMP
    pr_debug("LVSE: LVM_NON_OFFLOAD start ###############################################\n");
    pr_debug("LVSE: gProcessingInKernel.isSetInstParams = 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isSetInstParams);
    pr_debug("LVSE: gProcessingInKernel.isSetControlParams = 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isSetControlParams);
    pr_debug("LVSE: gProcessingInKernel.isSetHeadRoomParams = 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isSetHeadRoomParams);
    pr_debug("LVSE: gProcessingInKernel.isSetCustomTuningParams 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isSetCustomTuningParams);
    pr_debug("LVSE: gProcessingInKernel.isSetRevInstParams = 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isSetRevInstParams);
    pr_debug("LVSE: gProcessingInKernel.isSetRevControlParams = 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isSetRevControlParams);
    pr_debug("LVSE: gProcessingInKernel.isGetAudioClientFromOffload = 0x%lx\n", (long unsigned int)session_control->settingLVMControlInKernel.isGetAudioClientFromOffload);

    if(1 == session_control->settingLVMControlInKernel.isSetControlParams)
        lvse_dump_control_params(&(session_control->settingLVMControlInKernel.controlParams));
                      
    if(1 == session_control->settingLVMControlInKernel.isSetCustomTuningParams)
        lvse_dump_tuning_params(&(session_control->settingLVMControlInKernel.customTuningParams));

    pr_debug("LVSE: LVM_NON_OFFLOAD end###############################################\n");
#endif	           

    return LVMQDSP_SUCCESS;
}

int lvse_get_setting_lvm_control_in_kernel(int cmd, void *param, int session)
{
   int status;
   LVMQDSP_SessionControl_t *session_control = NULL;

   status = lvse_get_session_control(session, &session_control);
   if(status == -1){
        pr_err("\nLVSE: lvse_get_setting_lvm_control_in_kernel : Don't get session control");
        return LVMQDSP_ERROR;
   }

    if(NULL == session_control){
	    pr_err("\nLVSE: lvse_get_setting_lvm_control_in_kernel : session control is NULL");
        return LVMQDSP_ERROR;
    }

    switch(cmd){
        case LVMQDSP_GET_INSTANCE_PARAMS:
        {
            if(1 == session_control->settingLVMControlInKernel.isSetInstParams){
#ifdef ENABLE_PARAM_LOG_OFFLOAD            
                pr_debug("LVSE: LVMQDSP_GET_INSTANCE_PARAMS - size=%d \n",sizeof(LVMQDSP_InstParams_t));
#endif
                if(copy_to_user(param, &(session_control->settingLVMControlInKernel.instParams), sizeof(LVMQDSP_InstParams_t))) // copy kernel memory to user memory.
                {
                      pr_err("LVSE: LVMQDSP_GET_INSTANCE_PARAMS write error \n");
                      return LVMQDSP_ERROR;
                }
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_INSTANCE_PARAMS SUCCESS");
                return LVMQDSP_SUCCESS;
            }else{
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_INSTANCE_PARAMS Not Setting");
                return LVMQDSP_ERROR; 
            }
        }
        case LVMQDSP_GET_CONTROL_PARAMS:
        {
            if(1 == session_control->settingLVMControlInKernel.isSetControlParams)
            {
#ifdef ENABLE_PARAM_LOG_OFFLOAD            
                pr_debug("LVSE: LVMQDSP_GET_CONTROL_PARAMS - size=%d \n",sizeof(LVMQDSP_ControlParams_t) );
#endif
                if(copy_to_user(param, &(session_control->settingLVMControlInKernel.controlParams), sizeof(LVMQDSP_ControlParams_t)))
                {
                      pr_err("LVSE: LVMQDSP_GET_CONTROL_PARAMS write error \n");
                      return LVMQDSP_ERROR;
                }
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_CONTROL_PARAMS SUCCESS");
		  return LVMQDSP_SUCCESS;
            }else{
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_CONTROL_PARAMS Not Setting");
                return LVMQDSP_ERROR; 
            }
        }
        case LVMQDSP_GET_HEADROOM_PARAMS:
        {
            if(1 == session_control->settingLVMControlInKernel.isSetHeadRoomParams)
            {
#ifdef ENABLE_PARAM_LOG_OFFLOAD            
                pr_debug("LVSE: LVMQDSP_GET_HEADROOM_PARAMS - size=%d \n",sizeof(LVMQDSP_HeadroomParams_t));
#endif
                if(copy_to_user(param, &(session_control->settingLVMControlInKernel.headRoomParams), sizeof(LVMQDSP_HeadroomParams_t)))
                {
                      pr_err("LVSE: LVMQDSP_GET_HEADROOM_PARAMS write error \n");
                      return LVMQDSP_ERROR;
                }  
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_HEADROOM_PARAMS SUCCESS");
		  return LVMQDSP_SUCCESS;
            }else{
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_HEADROOM_PARAMS Not Setting");
                return LVMQDSP_ERROR; 
            }
        }
        //FIXED : Not tuning on start  (LVMQDSP_OPEN_SESSION->LVMQDSP_SET_INSTANCE_PARAMS->LVMQDSP_SET_CONTROL_PARAMS->##LVMQDSP_GET_TUNING_PARAMS##(NON-LVM)->LVMQDSP_SET_TUNING_PARAMS...
        //            so LVMQDSP_GET_TUNING_PARAMS don't process not to exist LVM..  or set default setting 
        case LVMQDSP_GET_TUNING_PARAMS:
        {
            if(1 == session_control->settingLVMControlInKernel.isSetCustomTuningParams)
            {
#ifdef ENABLE_PARAM_LOG_OFFLOAD            
                pr_debug("LVSE: LVMQDSP_GET_TUNING_PARAMS - size=%d \n",sizeof(LVMQDSP_Custom_TuningParams_st));
#endif
                if(copy_to_user(param, &(session_control->settingLVMControlInKernel.customTuningParams), sizeof(LVMQDSP_Custom_TuningParams_st)))
                {
                      pr_err("LVSE: LVMQDSP_GET_TUNING_PARAMS write error \n");
                      return LVMQDSP_ERROR;
                }  
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_TUNING_PARAMS SUCCESS");
		  return LVMQDSP_SUCCESS;
            }else{
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_TUNING_PARAMS Not Setting");
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_TUNING_PARAMS ignore ERROR");
                //return LVMQDSP_ERROR; 
                return LVMQDSP_SUCCESS; // ignore error..
            }
        }
	 case LVMQDSP_GET_REVERB_INSTANCE_PARAMS:
	 {
	     if(1 == session_control->settingLVMControlInKernel.isSetRevInstParams)
	     {
#ifdef ENABLE_PARAM_LOG_OFFLOAD	     
	         pr_debug("LVSE: LVMQDSP_GET_REVERB_INSTANCE_PARAMS - size=%d \n",sizeof(LVMQDSP_RevInstParams_t));
#endif
                if(copy_to_user(param, &(session_control->settingLVMControlInKernel.revInstParams), sizeof(LVMQDSP_RevInstParams_t)))
                {
                      pr_err("LVSE: LVMQDSP_GET_REVERB_INSTANCE_PARAMS write error \n");
                      return LVMQDSP_ERROR;
                }  
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_REVERB_INSTANCE_PARAMS SUCCESS");
		  return LVMQDSP_SUCCESS;
            }else{
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_REVERB_INSTANCE_PARAMS Not Setting");
                return LVMQDSP_ERROR; 
            }
	  }
	  case LVMQDSP_GET_REVERB_CONTROL_PARAMS:
	  {
	     if(1 == session_control->settingLVMControlInKernel.isSetRevControlParams)
	     {
#ifdef ENABLE_PARAM_LOG_OFFLOAD	     
	         pr_debug("LVSE: LVMQDSP_GET_REVERB_CONTROL_PARAMS - size=%d  \n",sizeof(LVMQDSP_RevControlParams_t));
#endif
                if(copy_to_user(param, &(session_control->settingLVMControlInKernel.revControlParams), sizeof(LVMQDSP_RevControlParams_t)))
                {
                      pr_err("LVSE: LVMQDSP_GET_REVERB_CONTROL_PARAMS write error \n");
                      return LVMQDSP_ERROR;
                }  
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_REVERB_CONTROL_PARAMS SUCCESS");
		  return LVMQDSP_SUCCESS;
            }else{
                LVSE_LOG_0("LVSE: lvse_get_setting_lvm_control_in_kernel - LVMQDSP_GET_REVERB_CONTROL_PARAMS Not Setting");
                return LVMQDSP_ERROR; 
            }
	  }
        default:
            pr_debug("LVSE: Unknown lvse_get_setting_lvm_control_in_kernel command %d \n", cmd);
    }
    pr_debug("\nLVSE: lvse_get_setting_lvm_control_in_kernel : default Not setting");
    return LVMQDSP_ERROR;  // Not setting
}

int lvse_apply_setting_lvm_control_to_dsp(int process_type, int session)
{
	int status = 0;
	struct audio_client *lvse_ac = NULL;
	LVMQDSP_SessionControl_t *session_control = NULL;

	if (LVM_TRUE  == gSettingEffectControlFlag && -1 < gSessionOffload) {
		LVSE_LOG_1("LVSE: START lvse_apply_setting_lvm_control_to_dsp() session(%d)", session);

		status = lvse_get_session_control(session, &session_control);
		if (status == -1) {
			pr_err("\nLVSE: lvse_apply_setting_lvm_control_to_dsp : Don't get the session control");
			goto LVM_ERR;
			//return LVMQDSP_ERROR;
		}

		if (NULL == session_control) {
			pr_err("\nLVSE: lvse_apply_setting_lvm_control_to_dsp : session control is NULL");
			goto LVM_ERR;
			//return LVMQDSP_ERROR;
		}

		if (NULL == session_control->ac) {
			pr_err("\nLVSE: lvse_apply_setting_lvm_control_to_dsp : Don't get the audio client");
			goto LVM_ERR;
			//return LVMQDSP_ERROR;
		}

		lvse_ac = *(session_control->ac);

		if (NULL == lvse_ac) {
			pr_err("\nLVSE: lvse_apply_setting_lvm_control_to_dsp : audio client is NULL");
			goto LVM_ERR;
			//return LVMQDSP_ERROR;
		}

		if (LVM_OFFLOAD  == process_type)
		{
			if (1 == session_control->settingLVMControlInKernel.isSetInstParams) {
				status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_INSTANCE_CTRL,
						(void*)&(session_control->settingLVMControlInKernel.instParams), sizeof(LVMQDSP_InstParams_t));

#ifdef ENABLE_PARAM_LOG_OFFLOAD
				pr_debug("LVSE: q6asm_set_pp_params (PARAM_ID_NXP_LVM_INSTANCE_CTRL) returned %d", status);
#endif
			}

			if (1 == session_control->settingLVMControlInKernel.isSetControlParams) {
				status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_EFFECT_CTRL,
						(void*)&(session_control->settingLVMControlInKernel.controlParams), sizeof(LVMQDSP_ControlParams_t));

#ifdef ENABLE_PARAM_LOG_OFFLOAD
				pr_debug("LVSE: q6asm_set_pp_params (PARAM_ID_NXP_LVM_EFFECT_CTRL) returned %d", status);
#endif

#ifdef ENABLE_PARAM_DUMP
				LVSE_LOG_0("LVSE: start CHECK user returned");
				lvse_dump_control_params(&(session_control->settingLVMControlInKernel.controlParams));
				LVSE_LOG_0("LVSE: end CHECK user returned");
#endif /* ENABLE_PARAM_DUMP */
			}

			if (1 == session_control->settingLVMControlInKernel.isSetHeadRoomParams) {
				status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_HEADROOM_CTRL,
						(void*)&(session_control->settingLVMControlInKernel.headRoomParams), sizeof(LVMQDSP_HeadroomParams_t));

#ifdef ENABLE_PARAM_LOG_OFFLOAD
				pr_debug("LVSE: q6asm_set_pp_params (PARAM_ID_NXP_LVM_HEADROOM_CTRL) returned %d", status);
#endif
			}

			if (1 == session_control->settingLVMControlInKernel.isSetCustomTuningParams) {
				status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM, PARAM_ID_NXP_LVM_TUNING_CTRL,
						(void*)&(session_control->settingLVMControlInKernel.customTuningParams), sizeof(LVMQDSP_Custom_TuningParams_st));

#ifdef ENABLE_PARAM_LOG_OFFLOAD
				pr_debug("LVSE: q6asm_set_pp_params (PARAM_ID_NXP_LVM_TUNING_CTRL) returned %d", status);
#endif
			}

			if (1 == session_control->settingLVMControlInKernel.isSetRevInstParams) {
				status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL,
						(void*)&(session_control->settingLVMControlInKernel.revInstParams), sizeof(LVMQDSP_RevInstParams_t));

#ifdef ENABLE_PARAM_LOG_OFFLOAD
				pr_debug("LVSE: q6asm_set_pp_params (PARAM_ID_NXP_LVM_REVERB_INSTANCE_CTRL) returned %d", status);
#endif
			}

			if (1 == session_control->settingLVMControlInKernel.isSetRevControlParams) {
				status = q6asm_set_pp_params(lvse_ac, MODULE_ID_NXP_LVM_REVERB, PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL,
						(void*)&(session_control->settingLVMControlInKernel.revControlParams), sizeof(LVMQDSP_RevControlParams_t));

#ifdef ENABLE_PARAM_LOG_OFFLOAD
				pr_debug("LVSE: q6asm_set_pp_params (PARAM_ID_NXP_LVM_REVERB_EFFECT_CTRL) returned %d", status);
#endif
			}
			LVSE_LOG_1("LVSE: END lvse_apply_setting_lvm_control_to_dsp() session(%d)", session);
		}

		gSettingEffectControlFlag = LVM_FALSE;
		gSessionOffload =  -1;
	}

	return LVMQDSP_SUCCESS;

LVM_ERR:
	gSettingEffectControlFlag = LVM_FALSE;
	gSessionOffload = -1;
	return LVMQDSP_ERROR;
}

#endif

