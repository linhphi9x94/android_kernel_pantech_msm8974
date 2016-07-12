//=============================================================================
// File       : tdmb_bb.c
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2007/08/08       hkyu           drafty
//  1.1.0       2009/04/28       yschoi         Android Porting
//=============================================================================

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <mach/gpio.h>
#include <asm/irq.h>

#include "tdmb_comdef.h"
#include "tdmb_dev.h"
#include "tdmb_chip.h"
#include "tdmb_bb.h"
#include "tdmb_test.h"

/*================================================================== */
/*================        TDMB BB Definition       ================= */
/*================================================================== */

#define FEATURE_TDMB_KERNEL_TIME
#ifdef FEATURE_TDMB_KERNEL_TIME
#include <linux/jiffies.h>
typedef enum
{
  TDMB_TIME_BB_INIT = 0,
  TDMB_TIME_SET_FREQ,
  TDMB_TIME_SET_FREQ_DONE,
  TDMB_TIME_SET_CH,
  TDMB_TIME_SET_CH_DONE,
  TDMB_TIME_AUTOSCAN,
  TDMB_TIME_FIRST_MSC,
  MAX_TDMB_TIME_NUM
} TDMB_KERNEL_TIME_TYPE;

static uint8 first_check=0, tdmb_play_started=0;
unsigned long tdmb_jiffies=0;
static unsigned long tdmb_kernel_time[MAX_TDMB_TIME_NUM];
#endif

#if defined(FEATURE_TDMB_USE_INC_T3900)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_t3900_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [T3900]");
#elif defined(FEATURE_TDMB_USE_INC_T3A00)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_t3a00_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [T3A00]");
#elif defined(FEATURE_TDMB_USE_FCI_FC8050)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_fc8050_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [FC8050]");
#elif defined(FEATURE_TDMB_USE_FCI_FC8080)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_fc8080_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [FC8080]");
#elif defined(FEATURE_TDMB_USE_RTV_MTV350)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_mtv350_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [MTV350]");
#elif defined(FEATURE_TDMB_USE_RTV_MTV319)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_mtv319_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [MTV319]");
#elif defined(FEATURE_TDMB_USE_TCC_TCC3170)
#define TDMB_BB_DRIVE_INIT(x)  \
      tdmb_bb_tcc3170_init(x); \
      TDMB_MSG_BB("TDMB BB ---> [TCC3170]");
#else
#error code "no tdmb baseband"
#endif

/*================================================================== */
/*====================== jaksal add BB Function =================== */
/*================================================================== */
tdmb_bb_function_table_type tdmb_bb_function_table;
static boolean tdmb_bb_initialized = FALSE;
tBBSYNCStage bb_sync_stage;
tSignalQuality g_tSigQual;

tdmb_mode_type dmb_mode = TDMB_MODE_AIR_PLAY;
tdmb_autoscan_state autoscan_state = AUTOSCAN_EXIT_STATE;

const uint32 KOREA_BAND_TABLE[] = {
  175280,177008,178736,
  181280,183008,184736,
  187280,189008,190736,
  193280,195008,196736,
  199280,201008,202736,
  205280,207008,208736,
  211280,213008,214736
};

const uint16 BAND_TABLE_MAX[]= {21, 38, 23, 23, 31};//{KOREA, BAND-3, L-BAND, CANADA, CHINA_BAND_III}

#ifdef FEATURE_DMB_AUTOSCAN_DISCRETE
const uint32 KOREA_BAND_TABLE_METRO[] = {
  181280,183008,184736,
  205280,207008,208736,
};
#endif

typedef struct {
  unsigned int snr;
  unsigned int pcber;
  unsigned int rsber;
  unsigned char rssi;
} temp_debug_type;

uint8 gFrequencyBand;
uint8 gFreqTableNum;

tBBStatus ch_verify_result = BB_SUCCESS;

#ifdef FEATURE_DMB_EBI_IF
void *ebi2_tdmb_base;
#endif

#ifdef FEATURE_TDMB_USE_FCI
tdmb_bb_int_type fci_int_type;
#endif /* FEATURE_TDMB_USE_FCI */



/*================================================================== */
/*====================== jaksal add BB Function =================== */
/*================================================================== */
int tdmb_bb_set_freq_band(int, int);
#ifdef FEATURE_TDMB_KERNEL_TIME
static void  reset_tdmb_kernel_time(void)
{
  tdmb_jiffies =0;
  first_check = 0;
  memset(tdmb_kernel_time, 0, sizeof(tdmb_kernel_time));
}

unsigned long get_tdmb_jiffies(void)
{
  unsigned long val;
  val = tdmb_play_started ? tdmb_jiffies : 0;
  return val;
}

void set_tdmb_data_jiffies(void)
{
  if(!first_check) return;
  
  tdmb_kernel_time[TDMB_TIME_FIRST_MSC] = get_jiffies_64() - tdmb_jiffies;  
  TDMB_MSG_BB("TDMB 1st Data Read[%d]  After Set_CH[%d]\n",jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_FIRST_MSC]-tdmb_kernel_time[TDMB_TIME_SET_CH]),
              jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_FIRST_MSC]-tdmb_kernel_time[TDMB_TIME_SET_CH_DONE]));
  reset_tdmb_kernel_time();
}
#endif

/*====================================================================
FUNCTION       tdmb_bb_func_tbl_init  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
boolean tdmb_bb_func_tbl_init(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  if(tdmb_bb_initialized)
    return TRUE;    

  memset((void*)&tdmb_bb_function_table, 0, sizeof(tdmb_bb_function_table_type));

  tdmb_bb_initialized = TDMB_BB_DRIVE_INIT(&tdmb_bb_function_table);

  return tdmb_bb_initialized;
}

/*====================================================================
FUNCTION       tdmb_bb_set_freq_band
DESCRIPTION         
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int tdmb_bb_set_freq_band(int which_Band, int scan_mode)
{
  int ret = 1;

  //TDMB_MSG_BB("[%s]~!!!\n", __func__);
  
  switch(which_Band)
  {
    case KOREA_BAND:
      gFrequencyBand = KOREA_BAND;
#ifdef FEATURE_DMB_AUTOSCAN_DISCRETE
      if(scan_mode == TDMB_MODE_SCAN_METRO)
      {
        gFreqTableNum = (sizeof(KOREA_BAND_TABLE_METRO) / sizeof(KOREA_BAND_TABLE_METRO[0]));
      }
      else if(scan_mode == TDMB_MODE_AUTOSCAN)
#endif
      {
        gFreqTableNum = BAND_TABLE_MAX[(KOREA_BAND-1)];
      }
    break;
    
    case BAND_III:  
    case L_BAND:   
    case CANADA_BAND:
    default:
      ret = 0;
    break;
  }

  //TDMB_MSG_BB("[%s] band [%d]  freq num [%d]\n", __func__, gFrequencyBand, gFreqTableNum);

  return ret;
}

/*====================================================================
FUNCTION       tdmb_bb_drv_init
DESCRIPTION  
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int tdmb_bb_drv_init(void)
{
  int ret = 0;

  TDMB_MSG_BB("[%s]~!!!\n", __func__);

#ifdef FEATURE_TDMB_KERNEL_TIME
  tdmb_play_started = 0;
  reset_tdmb_kernel_time();
  tdmb_jiffies = get_jiffies_64();
#endif

#ifndef FEATURE_DMB_AUTOSCAN_DISCRETE
  ret = tdmb_bb_set_freq_band(KOREA_BAND,TDMB_MODE_AUTOSCAN);
#endif
 
  tdmb_bb_power_on(); 
  tdmb_bb_init();

#ifdef FEATURE_TDMB_KERNEL_TIME
  tdmb_kernel_time[TDMB_TIME_BB_INIT] = get_jiffies_64() - tdmb_jiffies;
#endif

  return ret;
}

/*====================================================================
FUNCTION       tdmb_bb_init  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_init(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  tdmb_bb_function_table.tdmb_bb_drv_init();
}

/*====================================================================
FUNCTION       tdmb_bb_powersave_mode  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_powersave_mode(void)
{
  TDMB_MSG_BB("[%s]!!!\n", __func__);

  tdmb_bb_function_table.tdmb_bb_powersave_mode();
}

EXPORT_SYMBOL(tdmb_bb_powersave_mode);

/*====================================================================
FUNCTION       tdmb_bb_power_on  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_power_on(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  if(!tdmb_bb_initialized)
  {
    if(tdmb_bb_func_tbl_init() == FALSE)
      return;
  }

  tdmb_bb_function_table.tdmb_bb_power_on();

#ifdef CONFIG_EF10_BOARD // hdmi powersave_mode
  // EF10은 hdmi와 전원 공유하므로 hdmi on시 TDMB powersave_mode 필요.
#endif /* CONFIG_EF10_BOARD */
}

/*====================================================================
FUNCTION       tdmb_bb_power_off  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_power_off(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  tdmb_bb_function_table.tdmb_bb_power_off();

#ifdef FEATURE_TDMB_KERNEL_TIME
#ifdef FEATURE_DMB_AUTOSCAN_DISCRETE
  if((dmb_mode == TDMB_MODE_AUTOSCAN) || (dmb_mode == TDMB_MODE_SCAN_METRO))
  {
    tdmb_kernel_time[TDMB_TIME_AUTOSCAN] = get_jiffies_64() - tdmb_jiffies;    
    TDMB_MSG_BB("TDMB %s Scan Time [%d]\n", dmb_mode==TDMB_MODE_AUTOSCAN?"FULL":"METRO", jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_AUTOSCAN]));
  }
#else
  if(dmb_mode == TDMB_MODE_AUTOSCAN)
  {
    tdmb_kernel_time[TDMB_TIME_AUTOSCAN] = get_jiffies_64() - tdmb_jiffies;    
    TDMB_MSG_BB("TDMB Scan Time [%d]\n",jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_AUTOSCAN]));
  }
#endif
  tdmb_play_started = 0;
#endif
}

/*====================================================================
FUNCTION       tdmb_bb_ch_scan_start
DESCRIPTION   single channel scan
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_ch_scan_start(int freq, int band, unsigned char freq_offset)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

#ifdef FEATURE_TDMB_KERNEL_TIME
  if(dmb_mode == TDMB_MODE_AIR_PLAY)
  {
    tdmb_kernel_time[TDMB_TIME_SET_FREQ] = get_jiffies_64() - tdmb_jiffies;
  }
#endif

  tdmb_bb_function_table.tdmb_bb_ch_scan_start(freq, band, freq_offset);

#ifdef FEATURE_TDMB_KERNEL_TIME
  if(dmb_mode == TDMB_MODE_AIR_PLAY)
  {
    tdmb_kernel_time[TDMB_TIME_SET_FREQ_DONE] = get_jiffies_64() - tdmb_jiffies;
  }
#endif

}

/*====================================================================
FUNCTION       tdmb_bb_get_frequency
DESCRIPTION  해당 Band에 맞는 주파수와 주파수를 얻어온다
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_get_frequency(unsigned long *freq, unsigned char band,unsigned int index)
{
  //TDMB_MSG_BB("[%s]\n", __func__);

  switch(band)
  {
    case KOREA_BAND :
#ifdef FEATURE_DMB_AUTOSCAN_DISCRETE
      if(dmb_mode == TDMB_MODE_SCAN_METRO)
      {
        *freq = KOREA_BAND_TABLE_METRO[index];
      }
      else if(dmb_mode == TDMB_MODE_AUTOSCAN)
#endif      
      {
        *freq = KOREA_BAND_TABLE[index];
      }

    break;
    
    case BAND_III :
    break;
    
    case L_BAND :
    break;
    
    case CANADA_BAND :
    break;
    
    default:
    break;
  }
  
  //TDMB_MSG_BB("[%s] freq[%d]  band[%d] index[%d]\n", __func__, (int)*freq, (int)band,index);
  
}

/*====================================================================
FUNCTION       tdmb_bb_fic_process
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_fic_process(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);
}

/*====================================================================
FUNCTION       tdmb_bb_set_int
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_set_int(boolean on_off)
{
  TDMB_MSG_BB("[%s] on_off[%d]\n", __func__, on_off);

#if(defined(FEATURE_TDMB_USE_FCI) && defined(FEATURE_DMB_TSIF_IF))
  fc8050_set_int(on_off);
#endif

  return;
}

/*====================================================================
FUNCTION       tdmb_bb_set_fic_isr
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
boolean tdmb_bb_set_fic_isr(boolean on_off)
{
  bool ret;

  TDMB_MSG_BB("[%s] on_off[%d]\n", __func__, on_off);

  ret = tdmb_set_isr(on_off);

  return ret;
}

/*====================================================================
FUNCTION       tdmb_bb_chw_IntHandler2
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_chw_IntHandler2(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);
}

/*====================================================================
FUNCTION       tdmb_bb_ebi2_chw_IntHandler
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint16 tdmb_bb_ebi2_chw_IntHandler(uint8 *ts_stream_buffer)
{
  uint16 chw_size;

  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  chw_size = tdmb_bb_read_msc(ts_stream_buffer);

  //TDMB_MSG_BB("[%s] chw INTR  TS size[%d]= [0x%x] [0x%x] [0x%x] [0x%x] 0x%x\n", __func__, chw_size, ts_stream_buffer[0], ts_stream_buffer[1],ts_stream_buffer[2],size[0],size[1]);

  return chw_size;
}

/*====================================================================
FUNCTION       tdmb_bb_resync
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_resync(unsigned char ucIMR)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  tdmb_bb_function_table.tdmb_bb_resync(ucIMR);
}

/*====================================================================
FUNCTION       tdmb_bb_subch_start
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int tdmb_bb_subch_start (uint8 *Regs, uint32 data_rate)
{
  int ret;

#ifdef FEATURE_TDMB_KERNEL_TIME
  unsigned int freq_set_time, ch_set_time;
  tdmb_kernel_time[TDMB_TIME_SET_CH] = get_jiffies_64() - tdmb_jiffies;
#endif

  ret = tdmb_bb_function_table.tdmb_bb_subch_start(Regs, data_rate);

#ifdef FEATURE_TDMB_KERNEL_TIME
  tdmb_kernel_time[TDMB_TIME_SET_CH_DONE] = get_jiffies_64() - tdmb_jiffies;
  freq_set_time = jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_SET_FREQ_DONE]-tdmb_kernel_time[TDMB_TIME_SET_FREQ]);
  ch_set_time = jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_SET_CH_DONE]-tdmb_kernel_time[TDMB_TIME_SET_CH]);
  if(tdmb_play_started)
  {
    TDMB_MSG_BB("TDMB change Set_freq [%d]  Set_CH [%d]\n",freq_set_time,ch_set_time);
  }
  else
  {
    TDMB_MSG_BB("TDMB enter BB_init[%d]  Set_freq[%d]  Set_CH[%d] = [%d]  T[%d]\n",jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_BB_INIT]),freq_set_time, ch_set_time,
                (jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_BB_INIT])+freq_set_time+ch_set_time), jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_SET_CH_DONE]));

    tdmb_play_started = 1;
  }
  first_check = 1;
#endif

  return ret;
}

/*====================================================================
FUNCTION       tdmb_bb_drv_start
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_drv_start(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);
}

/*====================================================================
FUNCTION       tdmb_bb_drv_stop
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_drv_stop(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);
}

/*====================================================================
FUNCTION       tdmb_bb_report_debug_info  
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_report_debug_info(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);
}

/*====================================================================
FUNCTION       tdmb_bb_get_tuning_status
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tBBSYNCStage tdmb_bb_get_tuning_status(void)
{
  //TDMB_MSG_BB("[%s]~!!!\n", __func__);

  if(tdmb_bb_get_sync_status() == BB_SUCCESS)
  {
      bb_sync_stage = BB_SYNC_STAGE_3;      
  }
  else
  {
      bb_sync_stage = BB_SYNC_STAGE_0;      
    TDMB_MSG_BB("[%s] SYNC_LOCK fail %d\n", __func__, bb_sync_stage);
  }  

  return bb_sync_stage;
}

/*====================================================================
FUNCTION       tdmb_bb_set_fic_ch_result
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_set_fic_ch_result(tBBStatus result)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  ch_verify_result = result;
}

/*====================================================================
FUNCTION       tdmb_bb_get_fic_ch_result
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tBBStatus tdmb_bb_get_fic_ch_result(void)
{
  TDMB_MSG_BB("[%s]~!!!\n", __func__);

  return ch_verify_result;
}

/*====================================================================
FUNCTION       tdmb_bb_read_int
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tdmb_bb_int_type tdmb_bb_read_int(void)
{
  return tdmb_bb_function_table.tdmb_bb_read_int();
}

/*====================================================================
FUNCTION       tdmb_bb_get_sync_status
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tBBStatus tdmb_bb_get_sync_status(void)
{
  return tdmb_bb_function_table.tdmb_bb_get_sync_status();
}

/*====================================================================
FUNCTION       FIC_read_fib_data
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
dword tdmb_bb_read_fib(byte *fib_buf)
{
  return tdmb_bb_function_table.tdmb_bb_read_fib(fib_buf);
}

/*====================================================================
FUNCTION       tdmb_bb_set_subchannel_info
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_set_subchannel_info(void *sub_ch_info)
{
  tdmb_bb_function_table.tdmb_bb_set_subchannel_info(sub_ch_info);
}

/*====================================================================
FUNCTION       tdmb_bb_read_msc
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int tdmb_bb_read_msc(uint8 *msc_buf)
{
  int ret;

#if 0//def FEATURE_DMB_SPI_IF
#ifdef FEATURE_TDMB_KERNEL_TIME
  if(first_check)
  {
    tdmb_kernel_time[TDMB_TIME_FIRST_MSC] = get_jiffies_64() - tdmb_jiffies;
    TDMB_MSG_BB("TDMB 1st Data Read[%d]  After Set_CH[%d]\n",jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_FIRST_MSC]-tdmb_kernel_time[TDMB_TIME_SET_CH]),
                jiffies_to_msecs(tdmb_kernel_time[TDMB_TIME_FIRST_MSC]-tdmb_kernel_time[TDMB_TIME_SET_CH_DONE]));
    reset_tdmb_kernel_time();
  }
#endif
#endif

  ret = tdmb_bb_function_table.tdmb_bb_read_msc(msc_buf);

#ifdef FEATURE_NETBER_TEST_ON_BOOT
    netber_GetError(ret, msc_buf);
#endif

  return ret;
}

/*====================================================================
FUNCTION       tdmb_bb_get_ber
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_get_ber(void)
{
  tdmb_bb_sig_type signals;

  tdmb_bb_function_table.tdmb_bb_get_ber(&signals);

  g_tSigQual.PCBER = signals.PCBER;
#ifdef FEATURE_TDMB_USE_FCI
  //antenna bar = RSSI = SNR  change..
  g_tSigQual.RSSI = (unsigned char)signals.SNR;  
  g_tSigQual.SNR = (unsigned int)signals.RSSI;
#else
  g_tSigQual.RSSI = signals.RSSI;
  g_tSigQual.SNR = signals.SNR;
#endif /* FEATURE_TDMB_USE_INC */
  g_tSigQual.RSBER = signals.RSBER;
  g_tSigQual.RxPowerLevel = signals.RxPowerLevel;

  //TDMB_MSG_BB("[%s] pcber[%d], rssi[%d] snr[%d] rsber[%d]\n", __func__, g_tSigQual.PCBER, g_tSigQual.RSSI, g_tSigQual.SNR, g_tSigQual.RSBER);
}

/*====================================================================
FUNCTION       tdmb_bb_ch_stop
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_ch_stop(void)
{
#ifdef FEATURE_TDMB_KERNEL_TIME
  if(tdmb_play_started)
  {
    reset_tdmb_kernel_time();
    tdmb_jiffies = get_jiffies_64();
  }
#endif
  tdmb_bb_function_table.tdmb_bb_ch_stop();
}

/*====================================================================
FUNCTION       tdmb_bb_ch_test
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void tdmb_bb_ch_test(int ch)
{
  tdmb_bb_function_table.tdmb_bb_ch_test(ch);
}

