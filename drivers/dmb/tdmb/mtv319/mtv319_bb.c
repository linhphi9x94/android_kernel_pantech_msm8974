//=============================================================================
// File       : mtv319_bb.c
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//
//=============================================================================

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
//#include <mach/gpio.h>

#include "../tdmb_comdef.h"

#include "../../dmb_hw.h"
#include "../../dmb_interface.h"
#include "../tdmb_dev.h"
#include "../tdmb_test.h"
#include "../../dmb_test.h"

#include "mtv319_bb.h"
#include "mtv319.h"
#include "mtv319_internal.h"
#ifdef FEATURE_TDMB_MULTI_CHANNEL_ENABLE
#include "mtv319_cifdec.h"
#endif

/*================================================================== */
/*=================        MTV BB Function        ================== */
/*================================================================== */

#define MTV_CTRL_ID RTV_CHIP_ADDR

#ifdef FEATURE_DMB_SPI_IF
#define MTV_DATA_SIZE  (RTV_SPI_CIF_MODE_INTERRUPT_SIZE)
#elif defined(FEATURE_DMB_TSIF_IF)
#define MTV_DATA_SIZE  (RTV_SPI_CIF_MODE_INTERRUPT_SIZE/2)
#endif

st_subch_info  g_stEnsembleInfo;
boolean tdmb_power_on = FALSE;
int g_packet_read_cnt = 0;
int g_sync_status = 0;
static uint16 prev_subch_id;

extern tdmb_mode_type dmb_mode;
extern ts_data_type ts_data;

static boolean mtv319_function_register(tdmb_bb_function_table_type *);


/*====================================================================
FUNCTION       tdmb_bb_mtv319_init  
DESCRIPTION    matching mtv319 Function at TDMB BB Function
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
boolean tdmb_bb_mtv319_init(tdmb_bb_function_table_type *function_table_ptr)
{
  boolean bb_initialized;

  bb_initialized = mtv319_function_register(function_table_ptr);

  return bb_initialized;
}


static boolean mtv319_function_register(tdmb_bb_function_table_type *ftable_ptr)
{
  ftable_ptr->tdmb_bb_drv_init          = mtv319_init;
  ftable_ptr->tdmb_bb_power_on          = mtv319_power_on;
  ftable_ptr->tdmb_bb_power_off         = mtv319_power_off;
  ftable_ptr->tdmb_bb_ch_scan_start     = mtv319_ch_scan_start;
  ftable_ptr->tdmb_bb_resync            = mtv319_bb_resync;
  ftable_ptr->tdmb_bb_subch_start       = mtv319_subch_start;
  ftable_ptr->tdmb_bb_read_int          = mtv319_read_int;
  ftable_ptr->tdmb_bb_get_sync_status   = mtv319_get_sync_status;
  ftable_ptr->tdmb_bb_read_fib          = mtv319_read_fib;
  ftable_ptr->tdmb_bb_set_subchannel_info = mtv319_set_subchannel_info;
  ftable_ptr->tdmb_bb_read_msc          = mtv319_read_msc;
  ftable_ptr->tdmb_bb_get_ber           = mtv319_get_ber;
  ftable_ptr->tdmb_bb_ch_stop           = mtv319_stop;
  ftable_ptr->tdmb_bb_powersave_mode    = mtv319_set_powersave_mode;
  ftable_ptr->tdmb_bb_ch_test           = mtv350_test;

  return TRUE;
}


/*====================================================================
FUNCTION       mtv319_i2c_write  
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_i2c_write(uint8 reg, uint8 data)
{
  uint8 ret = 0;

#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_write(MTV_CTRL_ID, reg, sizeof(uint8), data, sizeof(uint8));
#endif /* FEATURE_DMB_I2C_CMD */

  return ret;
}


/*====================================================================
FUNCTION       mtv319_i2c_read  
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_i2c_read(uint8 reg)
{
  uint8 ret = 0;
  uint8 data = 0;

#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_read(MTV_CTRL_ID, reg, sizeof(uint8), (uint16*)&data, sizeof(uint8));
#endif /* FEATURE_DMB_I2C_CMD */
  if(ret) 
  {
    return data;
  }
  return ret;
}


/*====================================================================
FUNCTION       mtv319_i2c_read_len  
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_i2c_read_len(uint8 reg, uint8 *data, uint16 data_len)
{
  uint8 ret = 0;

#ifdef FEATURE_DMB_I2C_CMD
  dmb_i2c_read_len(MTV_CTRL_ID, reg, data, data_len, sizeof(uint8));
#endif /* FEATURE_DMB_I2C_CMD */

  return ret;
}


/*====================================================================
FUNCTION       mtv319_set_powersave_mode
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_set_powersave_mode(void)
{

}


/*====================================================================
FUNCTION       mtv319_power_on
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_power_on(void)
{
// 1. DMB_EN : LOW
// 2. DMB_EN : HIGH

  TDMB_MSG_RTV_BB("[%s] start!!!\n", __func__);

  dmb_power_on();

  dmb_set_gpio(DMB_PWR_EN, 0);
  msleep(1);

  dmb_power_on_chip();
  mdelay(20);

  tdmb_power_on = TRUE;

  TDMB_MSG_RTV_BB("[%s] end!!!\n", __func__);
}


/*====================================================================
FUNCTION       mtv319_power_off
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_power_off(void)
{
// 1. DMB_EN : LOW

  TDMB_MSG_RTV_BB("[%s] start!!!\n", __func__);

  dmb_power_off();

  tdmb_power_on = FALSE;

  TDMB_MSG_RTV_BB("[%s] end!!!\n", __func__);
}


/*====================================================================
FUNCTION       mtv319_ch_scan_start
DESCRIPTION 
    if for_air is greater than 0, this is for air play.
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_ch_scan_start(int freq, int band, unsigned char for_air)
{
  int res;
  TDMB_MSG_RTV_BB("[%s] Channel Frequency[%d] Band[%d] Mode[%d]\n", __func__, freq, band, for_air);

  rtvTDMB_CloseFIC();

  res = rtvTDMB_ScanFrequency(freq);

  rtvTDMB_OpenFIC();

  // 0501 for autoscan
  if((dmb_mode == TDMB_MODE_AUTOSCAN) 
#ifdef FEATURE_DMB_AUTOSCAN_DISCRETE
     || (dmb_mode == TDMB_MODE_SCAN_METRO)
#endif
  )
  {
    g_sync_status = (res == RTV_SUCCESS ? RTV_TDMB_CHANNEL_LOCK_OK : 0);
  }
  else
  {
    g_sync_status = RTV_TDMB_CHANNEL_LOCK_OK;
  }

  TDMB_MSG_RTV_BB("[%s]  %s  res[%d] sync_status[%d]\n",__func__,res==RTV_SUCCESS?"OK":"FAIL" ,res, g_sync_status);
}


/*====================================================================
FUNCTION       mtv319_init
DESCRIPTION            
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_init(void)
{
  uint8 res;

  RTV_GUARD_INIT;

  res = rtvTDMB_Initialize();
  prev_subch_id = 0xFFFF;

  TDMB_MSG_RTV_BB("[%s] res[%d]  [SDK Ver : 131129] \n", __func__, res);
  return res;
}


/*====================================================================
FUNCTION       mtv319_bb_resync
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_bb_resync(unsigned char imr)
{
  //TDMB_MSG_RTV_BB("[%s] Do nothing\n", __func__);
}


/*====================================================================
FUNCTION       mtv319_stop
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_stop(void)
{
  TDMB_MSG_RTV_BB("[%s] mtv319 STOP\n", __func__);

  rtvTDMB_CloseSubChannel(prev_subch_id); // for single 
  prev_subch_id = 0xFFFF;

  rtvTDMB_CloseFIC();

  // 20101102 cys
  //return RTV_STOP(MTV_CTRL_ID);
  return 1;
}


/*====================================================================
FUNCTION       mtv319_subch_start
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int mtv319_subch_start(uint8 *regs, uint32 data_rate)
{ 
  int res;
  uint16 uiSubChID, uiServiceType;
  uint32 ulRFNum;

  uiSubChID = g_stEnsembleInfo.astSubChInfo[0].uiSubChID;
  uiServiceType = (g_stEnsembleInfo.astSubChInfo[0].uiServiceType) == TDMB_BB_SVC_DMB ? RTV_SERVICE_DMB : RTV_SERVICE_DAB;
  ulRFNum = g_stEnsembleInfo.astSubChInfo[0].ulRFNum;

TDMB_MSG_RTV_BB("[%s] uiSubChID[%d]  uiServiceType[%d]  ulRFNum[%d]\n", __func__, uiSubChID, uiServiceType, (int)ulRFNum);

#ifdef FEATURE_TDMB_MULTI_CHANNEL_ENABLE// PJSIN 20110223 add-- [ 1 
  rtvTDMB_CloseFIC();
#endif// ]-- end 
 
  res = rtvTDMB_OpenSubChannel(ulRFNum, uiSubChID, uiServiceType, RTV_SPI_CIF_MODE_INTERRUPT_SIZE);
  prev_subch_id = uiSubChID;

#ifdef FEATURE_TDMB_MULTI_CHANNEL_ENABLE// PJSIN 20110223 add-- [ 1 
	rtvTDMB_OpenFIC();
#endif// ]-- end 


  if(res != RTV_SUCCESS)
  {
  	if (res != RTV_ALREADY_OPENED_SUBCHANNEL_ID)
    	TDMB_MSG_RTV_BB("[%s] rtvTDMB_OpenSubChannel error [%d]\n", __func__, res);

    return 0;
  }
  
  return 1;
}


/*====================================================================
FUNCTION       mtv319_read_int
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tdmb_bb_int_type mtv319_read_int(void)
{
  // 20101102 cys
  TDMB_MSG_RTV_BB("[%s] Do nothing \n", __func__);

  //mtv319_isr_handler();

  return TDMB_BB_INT_MSC;
}


/*====================================================================
FUNCTION       mtv319_get_sync_status
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tBBStatus mtv319_get_sync_status(void)
{
  // 0501 block for autoscan
  //g_sync_status = rtvTDMB_GetLockStatus();

  TDMB_MSG_RTV_BB("[%s] ret[%d]\n", __func__, g_sync_status);

  //if(g_sync_status  == RTV_TDMB_CHANNEL_LOCK_OK)
  if(g_sync_status >= RTV_TDMB_OFDM_LOCK_MASK)
  { 
    return BB_SUCCESS;
  }
  else
  {
    return BB_SYNCLOST;
  }
}


/*====================================================================
FUNCTION       mtv319_read_fib
DESCRIPTION
DEPENDENCIES
RETURN VALUE number of FIB
SIDE EFFECTS
======================================================================*/
int mtv319_read_fib(uint8 *fibs)
{
  int fib_num;

  fib_num = rtvTDMB_ReadFIC(fibs);

#ifdef FEATURE_DMB_DUMP_FILE
  dmb_data_dump(fibs, fib_num, FILENAME_RAW_FIC);
#endif    

  TDMB_MSG_RTV_BB("[%s] fib num %d  [%x]\n", __func__,fib_num,fibs[0]);

  return fib_num;//rtvTDMB_ReadFIC(fibs);
}


/*====================================================================
FUNCTION       mtv319_set_subchannel_info
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_set_subchannel_info(void * sub_ch_info)
{
  static chan_info *ch_info;

  TDMB_MSG_RTV_BB("[%s]\n", __func__);

  ch_info = (chan_info *)sub_ch_info;

  g_stEnsembleInfo.astSubChInfo[0].ucOption         = ch_info->ucOption;
  g_stEnsembleInfo.astSubChInfo[0].ucTableIndex     = ch_info->ucTableIndex;
  g_stEnsembleInfo.astSubChInfo[0].uiBitRate        = ch_info->uiBitRate;
  g_stEnsembleInfo.astSubChInfo[0].uiEnsumbleID     = ch_info->uiEnsumbleID;
  g_stEnsembleInfo.astSubChInfo[0].uiProtectionLevel= ch_info->uiProtectionLevel;
  g_stEnsembleInfo.astSubChInfo[0].uiStarAddr       = ch_info->uiStarAddr;
  g_stEnsembleInfo.astSubChInfo[0].uiSchSize        = ch_info->uiSchSize;
  g_stEnsembleInfo.astSubChInfo[0].uiServiceType    = ch_info->uiServiceType;
  g_stEnsembleInfo.astSubChInfo[0].uiSlFlag         = ch_info->uiSlFlag;
  g_stEnsembleInfo.astSubChInfo[0].uiSubChID        = ch_info->uiSubChID;
  g_stEnsembleInfo.astSubChInfo[0].uiTmID           = ch_info->uiTmID;
  g_stEnsembleInfo.astSubChInfo[0].ulRFNum          = ch_info->ulRFNum;
  g_stEnsembleInfo.astSubChInfo[0].uiDifferentRate  = ch_info->uiDifferentRate;
}


/*====================================================================
FUNCTION       mtv319_read_msc
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int mtv319_read_msc(uint8 *msc_buffer)
{
  int read_len;
#ifdef  FEATURE_TDMB_MULTI_CHANNEL_ENABLE
  int ret;
  struct RTV_CIF_DEC_INFO cif_dec_info;
  static U8 cif_ts_read_buf[RTV_SPI_CIF_MODE_INTERRUPT_SIZE];

  cif_dec_info.fic_buf_ptr = ts_data.fic_buf;
  cif_dec_info.subch_buf_ptr[0] = ts_data.msc_buf;
  cif_dec_info.subch_buf_size[0] = BB_MAX_DATA_SIZE;
#endif

 // TDMB_MSG_RTV_BB("[%s] Start \n", __func__);

#ifdef  FEATURE_TDMB_MULTI_CHANNEL_ENABLE
  read_len = rtvTDMB_ReadTS(cif_ts_read_buf);
  if (read_len == 0) {
     TDMB_MSG_RTV_BB("mtv319 ReadTS is zero \n");
	  ts_data.type = TYPE_NONE;
	  ts_data.fic_size = 0;
	  ts_data.msc_size = 0;
	  return read_len;
  }
#else
  read_len = rtvTDMB_ReadTS(msc_buffer);
  if (read_len == 0) {
	ts_data.msc_size = 0;
	return read_len;
  }
#endif

#ifdef  FEATURE_TDMB_MULTI_CHANNEL_ENABLE
#ifdef FEATURE_DMB_DUMP_FILE
  dmb_data_dump(cif_ts_read_buf, read_len, FILENAME_BEFORE_PARSING);
#endif

  ret = rtvCIFDEC_Decode(&cif_dec_info, cif_ts_read_buf, read_len);

  if(ret == 1)
  {
    ts_data.type = TYPE_NONE;
    ts_data.fic_size = ts_data.msc_size = 0;
    return ts_data.msc_size;
  }

  ts_data.fic_size = cif_dec_info.fic_size;
  ts_data.msc_size = cif_dec_info.subch_size[0];
  
#ifdef FEATURE_DMB_DUMP_FILE
  dmb_data_dump(cif_dec_info.subch_buf_ptr[0], ts_data.msc_size, FILENAME_AFTER_PARSING);
#endif  
  
  if((ts_data.msc_size==0) && (ts_data.fic_size == 0))
  {
    TDMB_MSG_RTV_BB("mtv319 CIFDEC parsing error\n");
    ts_data.type = TYPE_NONE;
    ts_data.fic_size = 0;
    ts_data.msc_size = 0;
  }
  else
  {
    if(ts_data.msc_size)
    {
      ts_data.type = MSC_DATA;
      if(ts_data.fic_size)
        ts_data.type = FIC_MSC_DATA;
    }
    else if(ts_data.fic_size)
    {
      ts_data.type = FIC_DATA;
    }
    //TDMB_MSG_RTV_BB("mtv319 CIFDEC parsing end type[%d], msc[%d] fic[%d]\n",ts_data.type , ts_data.msc_size,ts_data.fic_size);
  }
#else
  ts_data.type = MSC_DATA;
  ts_data.msc_size = read_len;
  ts_data.fic_size = 0;

#ifdef FEATURE_DMB_DUMP_FILE
  dmb_data_dump(msc_buffer, ts_data.msc_size, FILENAME_RAW_MSC);
#endif

  //TDMB_MSG_RTV_BB("mtv319 Read MSC end msc[%d]\n",ts_data.msc_size);
#endif

#if defined(FEATURE_TEST_ON_BOOT) && !defined(FEATURE_TDMB_MULTI_CHANNEL_ENABLE)
  {
    int i;
    for(i=0;i<ts_data.msc_size;i+=188)
    {
      TDMB_MSG_RTV_BB("%s  MSC data[%x][%x][%x][%x]",__func__,msc_buffer[i],msc_buffer[i+1],msc_buffer[i+2],msc_buffer[i+3]);
    }
  }
#endif

  return ts_data.msc_size;
}


/*====================================================================
FUNCTION       mtv319_Ant_Level
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/

static uint16 mtv319_Ant_Level(uint16 pcber)
{
  uint8 level = 0;
  static uint8 save_level = 0;
  uint16 ant_tbl[] = {980,//900, // 0 <-> 1
                               600, // 1 <-> 2
                               400, // 2 <-> 3
                               250, // 3 <-> 4
                               100};  // 4 <-> 5
//900, 600, 300, 100, 30, 10, 0
  uint16 hystery_value[]= {0,50,50,20,20};

  //TDMB_MSG_RTV_BB("[%s]\n", __func__);

  if(pcber < ant_tbl[4]) level = 5;//(ber >= 1 && ber <= 100) printf("ANT: 5\n");
  else if(pcber >= ant_tbl[4] && pcber < ant_tbl[3]) level = 4;//(ber >= 101 && ber <= 150) printf("ANT: 4\n");
  else if(pcber >= ant_tbl[3] && pcber < ant_tbl[2]) level = 3;//(ber >= 151 && ber <= 300) printf("ANT: 3\n");
  else if(pcber >= ant_tbl[2] && pcber < ant_tbl[1]) level = 2;//(ber >= 301 && ber <= 500) printf("ANT: 2\n");
  else if(pcber >= ant_tbl[1] && pcber < ant_tbl[0]) level = 1;//(ber >= 501 && ber <= 800) printf("ANT: 1\n");
  else if(pcber >= ant_tbl[0]) level = 0;//(ber >= 801) printf("ANT: 0\n");

  if(level == save_level + 1) // Level ÀÌ 1Ä­ ¿Ã¶ó°£ °æ¿ì¿¡¸¸.
  {
    if(pcber < (ant_tbl[level - 1] - hystery_value[level - 1]))
    {
      save_level = level;
    }
  }
  else 
  {
    save_level = level;
  }

  return save_level;
}


/*====================================================================
FUNCTION       mtv319_reconfig_n_ber
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_get_ber(tdmb_bb_sig_type *psigs)
{
  int rssi;

  //TDMB_MSG_RTV_BB("[%s]\n", __func__);

  rssi = (rtvTDMB_GetRSSI() / (int)RTV_TDMB_RSSI_DIVIDER);

  // 04/20 block
  //if(g_sync_status == RTV_TDMB_CHANNEL_LOCK_OK)// ì±„ë„ lock ?¼ë•Œë§?ê°’ì„ ?½ìŒ
      psigs->PCBER = rtvTDMB_GetCER();//(int)RTV_TDMB_CER_DIVIDER;
  //else
  //    psigs->PCBER = 20000;

  psigs->RSBER = rtvTDMB_GetPER();
  psigs->SNR = (rtvTDMB_GetCNR()/100);
  psigs->RSSI = mtv319_Ant_Level(psigs->PCBER);//rtvTDMB_GetCNR() / (int)RTV_TDMB_CNR_DIVIDER;
  psigs->RxPowerLevel = rssi;
  

  //TDMB_MSG_BB("[%s] pcber[%d], rssi[%d] snr[%d] rsber[%d]\n", __func__, psigs->PCBER, psigs->RSSI, psigs->SNR, psigs->RSBER);

#if (defined(FEATURE_TDMB_KERNEL_MSG_ON) && !defined(FEATURE_TS_PKT_MSG))
  TDMB_MSG_RTV_BB("[%s] Packet read Cnt[%d]\n", __func__, g_packet_read_cnt);

  g_packet_read_cnt = 0;
#endif /* FEATURE_TS_PKT_MSG */
}

/*====================================================================
FUNCTION       mtv350_test
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv350_test(int servicetype)
{
  static boolean powered = FALSE;
  st_subch_info *stInfo;
  int res;

  if(!powered)
  {
    if(!tdmb_power_on)
      mtv319_power_on();
    else
      TDMB_MSG_RTV_BB("[%s] skip mtv319_power_on [%d]\n", __func__, tdmb_power_on);

    powered = TRUE;
  }

  TDMB_MSG_RTV_BB("[%s] mtv319_init\n", __func__);

  mtv319_init();

#ifdef  FEATURE_DMB_I2C_CMD
  mtv319_i2c_write(0x03, 0x07); //24.576Mhz ê²½ìš°
  res = mtv319_i2c_read(0x00); //0x8A ?˜ì????
  TDMB_MSG_RTV_BB("[%s] RW test  Reg. 0x00 val[0x%x]==0x8a\n", __func__, res);
#endif

  stInfo = kmalloc(sizeof(st_subch_info), GFP_KERNEL);
  memset(stInfo, 0, sizeof(st_subch_info));

  tdmb_get_fixed_chan_info((service_t)servicetype, &stInfo->astSubChInfo[stInfo->nSetCnt]);

  stInfo->nSetCnt++;

  // 2012/04/26: RAONTECH
  stInfo->astSubChInfo[0].uiServiceType = (stInfo->astSubChInfo[0].uiServiceType==0x18) ? RTV_SERVICE_DMB : RTV_SERVICE_DAB;
  TDMB_MSG_RTV_BB("[%s] TEST start freq [%d]  dmb_mode [%d]\n", __func__, (int)stInfo->astSubChInfo[0].ulRFNum, (int)dmb_mode);
  TDMB_MSG_RTV_BB("[%s] TEST start service type [0x%x]  schsize[0x%x]\n", __func__, stInfo->astSubChInfo[0].uiServiceType, stInfo->astSubChInfo[0].uiSchSize);
  TDMB_MSG_RTV_BB("[%s] TEST start subch id [0x%x]\n", __func__, stInfo->astSubChInfo[0].uiSubChID);

  res = rtvTDMB_ScanFrequency(stInfo->astSubChInfo[0].ulRFNum);
  TDMB_MSG_RTV_BB("rtvTDMB_ScanFrequency %d\n", res);  
  if(res == RTV_SUCCESS)
  {
    TDMB_MSG_RTV_BB("rtvTDMB_ScanFrequency OK %d\n", res);
  }

  g_sync_status = rtvTDMB_GetLockStatus();

  TDMB_MSG_RTV_BB("RTV GetLockStatus  g_sync_status[%d]  [%d]\n", g_sync_status,RTV_TDMB_CHANNEL_LOCK_OK);

  rtvTDMB_CloseFIC(); //wgon test add

  rtvTDMB_CloseSubChannel(prev_subch_id); // for single 
  res = rtvTDMB_OpenSubChannel(stInfo->astSubChInfo[0].ulRFNum, stInfo->astSubChInfo[0].uiSubChID, stInfo->astSubChInfo[0].uiServiceType, RTV_SPI_CIF_MODE_INTERRUPT_SIZE);
  prev_subch_id = stInfo->astSubChInfo[0].uiSubChID;

  if(res != RTV_SUCCESS)
  {
    TDMB_MSG_RTV_BB("[%s] rtvTDMB_OpenSubChannel error [%d]\n", __func__, res);
  }

  kfree(stInfo);
}

/*====================================================================
FUNCTION       mtv319_i2c_test
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_i2c_test(void) 
{
  uint16  wData;

  if(!tdmb_power_on)
    mtv319_power_on();
  else
    TDMB_MSG_RTV_BB("[%s] skip mtv319_power_on [%d]\n", __func__, tdmb_power_on);

  msleep(100); //for test
  
  mtv319_i2c_write(0x03, 0x07); //24.576Mhz
  //mtv319_i2c_write(0x03, 0x87); // 32Mhzë³´ë‹¤ ??ê²½ìš° 
  wData = mtv319_i2c_read(0x00); //0x8A ?˜ì????
  TDMB_MSG_RTV_BB("[%s] %d\n", __func__, wData);

  return RTV_SUCCESS;
}

/*====================================================================
FUNCTION       mtv319_spi_test
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 mtv319_spi_test(void)
{
  return RTV_SUCCESS;
}

/*====================================================================
FUNCTION       mtv319_if_test
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void mtv319_if_test(void)
{
#if defined(FEATURE_DMB_SPI_CMD)
    mtv319_spi_test();
#elif defined(FEATURE_DMB_I2C_CMD)
    mtv319_i2c_test();
#endif
}

