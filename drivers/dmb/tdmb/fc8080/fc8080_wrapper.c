//=============================================================================

// File       : fc8080_wrapper.c
// Description:
//
// Revision History:
// Version   Date         Author       Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2009             jaksal
//  1.1.0       2010/12/06       yschoi         add tsif, i2c drivers
//=============================================================================

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <linux/mutex.h>
#include <linux/spinlock.h>

#include "../../dmb_hw.h"
#include "../../dmb_interface.h"
#include "../../dmb_test.h"

#include "../tdmb_comdef.h"
#include "../tdmb_dev.h"
#include "../tdmb_test.h"

#ifdef FEATURE_DMB_PMIC_POWER
#include <mach/vreg.h>
#include <mach/mpp.h>
#endif /* FEATURE_DMB_PMIC_POWER */

#ifdef FEATURE_DMB_SET_ANT_PATH
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/pmic8058-regulator.h>
#endif /* FEATURE_DMB_SET_ANT_PATH */

#include "fc8080_wrapper.h"
#include "fc8080_demux.h"
#include "fc8080_isr.h"
#include "fci_types.h"
#include "fci_oal.h"

/*==================================================================*/
/*                      DEFINITION                                  */
/*==================================================================*/
//#define FC8080_IF_RW_TEST
//#define FC8080_READ_ALL_FIC // play시 tsif로 들어온 FIC DATA를 전부 read할 경우
#define FC8080_MSC_BUFFERS        16
#define FC8080_MSC_BUFFER_SIZE    (CH0_BUF_THR + 1)

#define FC8080_SPI_WORKAROUND

/*==================================================================*/
/*                      TYPE DEFINITION                             */
/*==================================================================*/
typedef struct {
  uint32 freq;
  uint8  subChId;
  uint8  servicetype;
  uint16 bitrate;
} REGS;

tdmb_bb_int_type g_Int_type;

typedef struct _INT_TYPE
{
  uint8   valid;
  uint32  address;
  uint32  length;
  uint8   subch_id;
  uint8   svc_type;
}INT_TYPE;

INT_TYPE fic_type;
INT_TYPE msc_type[FC8080_MSC_BUFFERS];
int gInputBuffer = 0;
int gOutputBuffer = 0;

REGS fci_subch_info;
s8 rssi_value;

/*==================================================================*/
/*                      VARIABLE                                    */
/*==================================================================*/
static int subChId;
u8 msc_data[FC8080_MSC_BUFFER_SIZE * FC8080_MSC_BUFFERS];
u8 fc8080_fic_data[188*8*8];

tdmb_bb_service_type serviceType;
boolean tdmb_power_on = FALSE;

uint8 tune_res;
u16 g_mfIntStatus;
extern tdmb_mode_type dmb_mode;
extern ts_data_type ts_data;

#ifdef FC8080_USE_TSIF
static int g_fci_ch_start = 0;
#ifdef FEATURE_FIT_FRAME_SIZE
extern u32 audio_frame_size;
extern u32 remain_audio_frame;
#endif
#endif

#ifdef FC8080_SPI_WORKAROUND
#define TOTAL_TRY_NUM 10
static int total_init_cnt = 0;
static int total_first_try_fail_cnt = 0;
static int total_retry_info[TOTAL_TRY_NUM];
#endif

/*==================================================================*/
/*                      FUNCTION PROTOTYPE                          */
/*==================================================================*/
static boolean fc8080_function_register(tdmb_bb_function_table_type *ftable_ptr)
{
  ftable_ptr->tdmb_bb_drv_init          = fc8080_drv_init;
  ftable_ptr->tdmb_bb_power_on          = fc8080_power_on;
  ftable_ptr->tdmb_bb_power_off         = fc8080_power_off;
  ftable_ptr->tdmb_bb_ch_scan_start     = fc8080_ch_scan_start;
  ftable_ptr->tdmb_bb_resync            = fc8080_bb_resync;
  ftable_ptr->tdmb_bb_subch_start       = fc8080_subch_start;
  ftable_ptr->tdmb_bb_read_int          = fc8080_read_int;
  ftable_ptr->tdmb_bb_get_sync_status   = fc8080_get_sync_status;
  ftable_ptr->tdmb_bb_read_fib          = fc8080_read_fib;
  ftable_ptr->tdmb_bb_set_subchannel_info = fc8080_set_subchannel_info;
  ftable_ptr->tdmb_bb_read_msc          = fc8080_read_msc;
  ftable_ptr->tdmb_bb_get_ber           = fc8080_get_ber;
  ftable_ptr->tdmb_bb_ch_stop           = fc8080_stop;
  ftable_ptr->tdmb_bb_ch_test           = fc8050_test;

  return TRUE;
}

int TDMB_FIC_CALLBACK(u32 userdata, u8 *data, int length)
{
  g_Int_type = TDMB_BB_INT_FIC;

  if((fic_type.address == 0) && (fic_type.length == 0))
    fic_type.address = (uint32)&fc8080_fic_data[0];

  memcpy((void*)&fc8080_fic_data[fic_type.length], data, length);
  fic_type.length = length;

  return TRUE;
}

int TDMB_MSC_CALLBACK(u32 userdata, u8 subChId, u8 *data, int length)
{
  g_Int_type = TDMB_BB_INT_MSC;

  if(((gInputBuffer + 1) % FC8080_MSC_BUFFERS) == gOutputBuffer)
  {
    TDMB_MSG_FCI_BB("[%s] BUFFER ALLOCATION ERRROR\n", __func__);
    return 0;
  }

  memcpy((void*)&msc_data[gInputBuffer * FC8080_MSC_BUFFER_SIZE], data, length);

  msc_type[gInputBuffer].length = length;
  msc_type[gInputBuffer].subch_id = subChId;
  msc_type[gInputBuffer].address = (uint32)&msc_data[gInputBuffer * FC8080_MSC_BUFFER_SIZE];

  gInputBuffer = (gInputBuffer + 1) % FC8080_MSC_BUFFERS;

  //TDMB_MSG_FCI_BB("[%s]  total_len[%d] data0[%x] data1[%x]\n", __func__, length, *data, *(data+1));

#if defined(FEATURE_TEST_ON_BOOT) && !defined(FEATURE_TDMB_MULTI_CHANNEL_ENABLE)
  if(serviceType == TDMB_BB_SVC_DMB)
  {
    int i;

    for(i = 0; i < length; i += 188)
    {
      if((*(data+i) !=0x47) && !((*(data + i + 1)) & 0x80))
      {
        TDMB_MSG_FCI_BB("[%s]   [%x] [%x] [%x]\n", __func__,*(data+i), *(data+i+1),*(data+i+2));
        break;
      }
    }
  }
#endif

  return TRUE;
}

/*====================================================================
FUNCTION       tdmb_bb_fc8080_init
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
boolean tdmb_bb_fc8080_init(tdmb_bb_function_table_type *function_table_ptr)
{
    boolean bb_initialized;

    bb_initialized = fc8080_function_register(function_table_ptr);

    return bb_initialized;
}

/*====================================================================
FUNCTION       msWait
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void msWait(unsigned long ms)
{
  msleep(ms);
}


/*====================================================================
FUNCTION       FC8080_OVERRUN_CHECK
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int FC8080_OVERRUN_CHECK(u8 mask)
{
  u16 mfoverStatus;

  bbm_com_word_read(NULL, BBM_BUF_OVERRUN, &mfoverStatus);

  if(mfoverStatus & mask)
  {
    bbm_com_word_write(NULL, BBM_BUF_OVERRUN, mfoverStatus);
    return TRUE;
  }

  return FALSE;	//is not overrun
}

/*====================================================================
FUNCTION       fc8080_i2c_write_byte
DESCRIPTION  8bit reg. / 8bit data
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_i2c_write_byte(uint8 chipid, uint8 reg, uint8 *data, uint16 length)
{
  uint8 ret = 0;

  //TDMB_MSG_FCI_BB("[%s]\n", __func__);

#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_write(chipid, reg, sizeof(uint8), *data, length);
  //ret = dmb_i2c_write_len(chipid, reg, 1, data, length);
#endif

  return ret;
}

/*====================================================================
FUNCTION       fc8080_i2c_write_word
DESCRIPTION  16bit reg. / 16bit data
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_i2c_write_word(uint8 chipid, uint16 reg, uint16 *data, uint16 length)
{
  uint8 ret = 0;

  //TDMB_MSG_FCI_BB("[%s]\n", __func__);

#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_write(chipid, reg, sizeof(uint16), *data, length);
  //ret = dmb_i2c_write_word(chipid, reg, 2, data, length);
#endif

  return ret;
}

/*====================================================================
FUNCTION       fc8080_i2c_write_len
DESCRIPTION  8bit reg. / N-bit data
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_i2c_write_len(uint8 chipid, uint8 reg, uint8 *data, uint16 length)
{
  uint8 ret = 0;
  uint16 w_data;

  w_data = ((*(data+1)<<8) |*data);
#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_write(chipid, reg, sizeof(uint8), w_data, length);
  //ret = dmb_i2c_write_len(chipid, reg, 1, data, length);
#endif

  return ret;
}

/*====================================================================
FUNCTION       fc8080_i2c_read_byte
DESCRIPTION  8bit reg. / 8bit data
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_i2c_read_byte(uint8 chipid, uint8 reg, uint8 *data, uint16 length)
{
  uint8 ret = 0;

  //TDMB_MSG_FCI_BB("[%s]\n", __func__);

#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_read(chipid, reg, sizeof(uint8), (uint16*)data, length);
  //ret = dmb_i2c_read_len(chipid, reg, 1, data, length);
#endif

  return ret;
}

/*====================================================================
FUNCTION       fc8080_i2c_read_word
DESCRIPTION  16bit reg. / 16bit data
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_i2c_read_word(uint8 chipid, uint16 reg, uint16 *data, uint16 length)
{
  uint8 ret = 0;

  //TDMB_MSG_FCI_BB("[%s]\n", __func__);

#ifdef FEATURE_DMB_I2C_CMD
  ret = dmb_i2c_read(chipid, reg, sizeof(uint16), data, length);
  //ret = dmb_i2c_read_word(chipid, reg, 2, data, length);
#endif

  return ret;
}

/*====================================================================
FUNCTION       fc8080_i2c_read_len
DESCRIPTION  8bit reg. / N-bit data
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_i2c_read_len(uint8 chipid, uint8 reg, uint8 *buf_ptr, uint16 length)
{
  uint8 ret = 0;

  //TDMB_MSG_FCI_BB("[%s]\n", __func__);

#ifdef FEATURE_DMB_I2C_CMD
  dmb_i2c_read_len(chipid, reg, buf_ptr, length, sizeof(uint8));
  //ret = dmb_i2c_read_len(chipid, reg, 1, buf_ptr, length);
#endif

  return ret;
}

#if defined(FC8080_IF_RW_TEST) || defined(FEATURE_TEST_ON_BOOT)
/*====================================================================
FUNCTION       fc8080_if_test
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_if_test(uint8 test_id)
{
  uint8 data, temp;
  uint16 wdata,i = 0;
  unsigned int ldata;

  TDMB_MSG_FCI_BB("[%s] start!!!\n", __func__);
#if 0
#ifdef FEATURE_DMB_I2C_CMD
  if(bbm_com_hostif_select(NULL, BBM_I2C))
#elif defined(FEATURE_DMB_SPI_IF)
  if(bbm_com_hostif_select(NULL, BBM_SPI))
#endif
  {
    return;
  }
#endif

  data = bbm_com_probe(NULL);
  if(data == BBM_NOK)
  {
     TDMB_MSG_FCI_BB("[%s] chipID read fail !!\n", __func__);
     return;
  }

  switch(test_id)
  {
    case 1:
      TDMB_MSG_FCI_BB("[%s] byte R/W test\n", __func__);

      for(i = 0; i < 100; i++)
      {
        bbm_com_write(NULL, 0xa4, i & 0xff);
        bbm_com_read(NULL, 0xa4, &data);
        if((i & 0xff) != data)
          TDMB_MSG_FCI_BB("[%s] byte R/W test err !![0x%x,0x%x]\n", __func__, i, data);
      }
    break;

    case 2:
      TDMB_MSG_FCI_BB("[%s] word R/W test\n", __func__);

      for(i = 0; i < 100; i++)
      {
        bbm_com_word_write(NULL, 0xa4, i & 0xffff);
        bbm_com_word_read(NULL, 0xa4, &wdata);
        if((i & 0xffff) != wdata)
          TDMB_MSG_FCI_BB("[%s] word R/W test err !![0x%x,0x%x]\n", __func__, i, wdata);
      }
    break;

    case 3:
      TDMB_MSG_FCI_BB("[%s] long R/W test\n", __func__);

      for(i = 0; i < 100; i++)
      {
        bbm_com_long_write(NULL, 0xa4, i & 0xffffffff);
        bbm_com_long_read(NULL, 0xa4, &ldata);

        if((i & 0xffffffff) != ldata)
          TDMB_MSG_FCI_BB("[%s] long R/W test err !![0x%x,0x%x]\n", __func__, i, ldata);
      }
    break;

    case 4:
      TDMB_MSG_FCI_BB("[%s] tuner R/W test\n", __func__);

      for(i = 0; i < 100; i++)
      {
        temp = i&0xff;
        bbm_com_tuner_write(NULL, 0x58, 0x01, &temp, 0x01);
        bbm_com_tuner_read(NULL, 0x58, 0x01, &data, 0x01);
        if((i & 0xff) != data)
          TDMB_MSG_FCI_BB("[%s] tuner R/W test err !![0x%x,0x%x]\n", __func__, i, data);
      }
    break;

    default:
    break;
  }
}
#endif /* FC8080_I2C_RW_TEST */


/*====================================================================
FUNCTION       fc8080_power_on
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_power_on(void)
{
  TDMB_MSG_FCI_BB("[%s] start!!!\n", __func__);

  if(tdmb_power_on == TRUE) 
  {
    TDMB_MSG_FCI_BB("[%s] return %d \n", __func__,tdmb_power_on);
    return;
  }

  //TDMB_EN first
  dmb_power_on_chip(); 
  msleep(5);

  dmb_power_on();
  msleep(30);

  tdmb_power_on = TRUE;
}

/*====================================================================
FUNCTION       fc8080_power_off
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_power_off(void)
{
  TDMB_MSG_FCI_BB("[%s] start!!!\n", __func__);

  if(tdmb_power_on == FALSE)
  {
    TDMB_MSG_FCI_BB("[%s] return %d \n", __func__,tdmb_power_on);
    return;
  }

  dmb_power_off();

  tdmb_power_on = FALSE;
}


/*====================================================================
FUNCTION       fc8080_control_fic
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int8 fc8080_control_fic(uint8 enable, uint8 mode) //mode : 0(ts_mode), 1(i2c_mode)
{
  unsigned short mask;
#ifdef FC8080_USE_TSIF
#ifndef FC8080_USE_QUP_I2C
  unsigned char lmode;
#endif
#endif /* FC8080_USE_TSIF */

  bbm_com_word_read(NULL, BBM_BUF_INT, &mask);

  if(enable == 1)
  {
#ifdef FC8080_USE_TSIF
    if(mode == 1)
    {
      bbm_com_write(NULL, BBM_MD_INT_EN, 1);
      bbm_com_write(NULL, BBM_MD_INT_STATUS, 1);
    }
#endif
    mask |= 0x100;
  }
  else
  {
#ifdef FC8080_USE_TSIF
    bbm_com_write(NULL, BBM_MD_INT_EN, 0);
    bbm_com_write(NULL, BBM_MD_INT_STATUS, 0);
#endif
    mask &= ~0x100;
  }

  bbm_com_word_write(NULL, BBM_BUF_INT, mask);
  bbm_com_word_write(NULL, BBM_BUF_ENABLE, mask);

#ifdef FC8080_USE_TSIF
#ifndef FC8080_USE_QUP_I2C
  bbm_com_read(NULL, BBM_TSO_SELREG, &lmode);

  if(mode == 1)
    lmode &= ~0x40;
  else
    lmode |= 0x40;

  bbm_com_write(NULL, BBM_TSO_SELREG, lmode);
#endif
#endif /* FC8080_USE_TSIF */

  return TRUE;
}

/*====================================================================
FUNCTION       fc8080_set_int
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_set_int(uint8 enable)
{
  TDMB_MSG_FCI_BB("[%s] !!!\n", __func__);

  if(enable == 1)
  {
    bbm_com_write(NULL, BBM_MD_INT_EN, 1);
    bbm_com_write(NULL, BBM_MD_INT_STATUS, 1);
  }
  else
  {
    bbm_com_write(NULL, BBM_MD_INT_EN, 0);
    bbm_com_write(NULL, BBM_MD_INT_STATUS, 0);
  }
}

/*====================================================================
FUNCTION       fc8080_get_fic
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int8 fc8080_get_fic(uint8* buffer, uint32* buffer_size)
{
  HANDLE hDevice = NULL;
  u16      mfIntStatus = 0;
  u16      size;
  int i;

  if(buffer == NULL)
    return FALSE;

  for(i = 0; i < 20; i++)
  {
    bbm_com_word_read(hDevice, BBM_BUF_STATUS, &mfIntStatus);

    if(mfIntStatus)
	    break;

    msWait(5);
  }

  if(mfIntStatus == 0)
    return FALSE;

  bbm_com_word_write(hDevice, BBM_BUF_STATUS, mfIntStatus);

  if(mfIntStatus & 0x0100)
  {
    bbm_com_word_read(hDevice, BBM_BUF_FIC_THR, &size);
    size += 1;
    if(size-1)
    {
      bbm_com_data(hDevice, BBM_FIC_I2C_RD, buffer, size);
      *buffer_size = size;
    }
    return TRUE;
  }

  return FALSE;
}

/*====================================================================
FUNCTION       fc8080_ch_scan_start
DESCRIPTION
    if for_air is greater than 0, this is for air play.
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_ch_scan_start(int freq, int band, unsigned char for_air)
{
  TDMB_MSG_FCI_BB("[%s] Channel Frequency[%d] \n", __func__, freq);

  fc8080_control_fic(0, 1);

  if(bbm_com_tuner_set_freq(NULL, freq)){
    tune_res = BBM_NOK;
    return;
  }

  tune_res = bbm_com_scan_status(NULL);

  fc8080_control_fic(1, 1);

  TDMB_MSG_FCI_BB("[%s] tune_res[%d]\n", __func__, tune_res);
}

/*====================================================================
FUNCTION       fc8080_drv_init
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_drv_init(void)
{
  int res = BBM_NOK;
#ifdef FC8080_SPI_WORKAROUND
  int i,j;
  int try_cnt = 0;
#endif

  TDMB_MSG_FCI_BB("[%s] [SDK VER : 140410](%s)\n", __func__, DRIVER_VER);

#if defined(FC8080_I2C)
  if(bbm_com_hostif_select(NULL, BBM_I2C))
#elif defined(FC8080_EBI2)
  if(bbm_com_hostif_select(NULL, BBM_PPI))
#elif defined(FC8080_SPI)
  if(bbm_com_hostif_select(NULL, BBM_SPI))
#endif
  {
    TDMB_MSG_FCI_BB("[%s] hostif select fail!!! \n", __func__);
    return FALSE;
  }

  bbm_com_fic_callback_register(0, TDMB_FIC_CALLBACK);
  bbm_com_msc_callback_register(0, TDMB_MSC_CALLBACK);

#ifdef FC8080_SPI_WORKAROUND
  total_init_cnt++;
  if(bbm_com_probe(NULL) == BBM_NOK)
  {
    total_first_try_fail_cnt++;
    for(i=0; i<10; i++)
    {
      dmb_power_off();
      msleep(250);
      dmb_set_gpio(DMB_PWR_EN, 1);
      dmb_power_on();
      msleep(30);
      try_cnt++;
      if(bbm_com_probe(NULL) == BBM_OK)
      {
        TDMB_MSG_FCI_BB("[%s] fc8080 probe ReTry %d OK\n",__func__, (i+1));
        break;
      }
    }
  }
#endif

  res = bbm_com_init(NULL);
  res |= bbm_com_tuner_select(NULL, FC8080_TUNER, BAND3_TYPE);

#ifdef FC8080_IF_RW_TEST
  fc8080_if_test(1); // byte
  fc8080_if_test(2); // word
  fc8080_if_test(3); // long
  fc8080_if_test(4); // tuner
#endif /*  FC8080_I2C_RW_TEST */

  if(res)
    return FALSE;

#ifdef FC8080_USE_TSIF
  g_fci_ch_start = 0;
#endif

  gInputBuffer = 0;
  gOutputBuffer = 0;

  memset((void*)&msc_type[0], 0x00, sizeof(msc_type));
  memset((void*)&fic_type, 0x00, sizeof(INT_TYPE));

#ifdef FEATURE_TDMB_MULTI_CHANNEL_ENABLE
  TDMB_MSG_FCI_BB("[%s] MULTI_CHANNEL \n", __func__);
#else
  TDMB_MSG_FCI_BB("[%s] SINGLE_CHANNEL \n", __func__);
#endif

#ifdef FC8080_SPI_WORKAROUND
  TDMB_MSG_FCI_BB("[%s]======= total probe ReTry count[%d]/ total drv Init count[%d]=======\n",__func__, total_first_try_fail_cnt, total_init_cnt);
  if(try_cnt != 0)
  {
    total_retry_info[try_cnt-1]++;
    for(j=0;j<TOTAL_TRY_NUM;j++)
    {
      if(total_retry_info[j] !=0)
        TDMB_MSG_FCI_BB("[%s] try[%d]-----[%d]\n",__func__, j+1, total_retry_info[j]); 
    }
  }
#endif
  return TRUE;
}

/*====================================================================
FUNCTION       fc8080_bb_resync
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_bb_resync(unsigned char imr)
{
}

/*====================================================================
FUNCTION       fc8080_stop
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
uint8 fc8080_stop(void)
{
  int res = BBM_NOK;

  TDMB_MSG_FCI_BB("[%s] \n", __func__);

  if(serviceType == TDMB_BB_SVC_DAB)
  {
    res=bbm_com_audio_deselect(NULL, 0, 2);
#ifdef FC8080_USE_TSIF
    fc8080_demux_deselect_channel( subChId, 2);
#endif
  }
  else if(serviceType == TDMB_BB_SVC_DMB)
  {
    res=bbm_com_video_deselect(NULL, 0, 0, 0);
#ifdef FC8080_USE_TSIF
    fc8080_demux_deselect_video( subChId, 0);
#endif
  }
  else if(serviceType == TDMB_BB_SVC_DATA)
  {
    res=bbm_com_data_deselect(NULL, 0, 1);
#ifdef FC8080_USE_TSIF
    fc8080_demux_deselect_channel( subChId, 1);
#endif
  }
  else
  {
    return FALSE;
  }

  bbm_com_word_write(NULL, BBM_BUF_ENABLE, 0x0000); //FIC BUF clear

  ms_wait(60);

  if(res)
    return FALSE;

#ifdef FC8080_USE_TSIF
  g_fci_ch_start = 0;
#endif

  gInputBuffer = 0;
  gOutputBuffer = 0;

  memset((void*)&msc_type[0], 0x00, sizeof(msc_type));
  memset((void*)&fic_type, 0x00, sizeof(INT_TYPE));

  //TDMB_MSG_FCI_BB("[%s] end!\n", __func__);

  return TRUE;
}

/*====================================================================
FUNCTION       fc8080_subch_start
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8080_subch_start(uint8 *regs, uint32 data_rate)
{
  uint32 freq = 0;
  REGS* p_regs;
  int res = BBM_NOK;
  int fic_mode = 0;
  p_regs = &fci_subch_info;

#ifdef FC8080_USE_TSIF
  g_fci_ch_start = 1;
#endif

  fc8080_stop();

  if(dmb_mode == TDMB_MODE_NETBER)
  {
    fc8080_control_fic(0, fic_mode); // FIC OFF
  }
  else
  {
#ifdef FEATURE_TDMB_MULTI_CHANNEL_ENABLE
    fc8080_control_fic(1, fic_mode); // FIC ON
    //fc8080_control_fic(0, fic_mode); // FIC OFF
#else
    fc8080_control_fic(0, fic_mode); // FIC OFF
#endif
  }

  freq = p_regs->freq;
  subChId = p_regs->subChId;
  serviceType = p_regs->servicetype;
  TDMB_MSG_FCI_BB("[%s] subchid[%x] type[%x] freq[%d]\n", __func__, subChId, serviceType, (int)freq);

  res = bbm_com_tuner_set_freq(NULL, freq);

  if(serviceType == TDMB_BB_SVC_DAB)
  {
    //TDMB_MSG_FCI_BB("[%s] bitrate[%d]", __func__, fci_subch_info.bitrate);
#ifdef FC8080_USE_TSIF
#ifdef FEATURE_FIT_FRAME_SIZE
    bbm_com_word_write(NULL, BBM_BUF_CH2_END, CH2_BUF_START + (184*TSIF_CHUNK_SIZE*2)-1);
    bbm_com_word_write(NULL, BBM_BUF_CH2_THR, (184*TSIF_CHUNK_SIZE)-1);
    remain_audio_frame=0;
    audio_frame_size=fci_subch_info.bitrate*3;
#else
    bbm_com_word_write(NULL, BBM_BUF_CH2_END, CH2_BUF_START + (fci_subch_info.bitrate*12*2)-1);
    bbm_com_word_write(NULL, BBM_BUF_CH2_THR, (fci_subch_info.bitrate*6*2)-1);
#endif
#else
    bbm_com_word_write(NULL, BBM_BUF_CH2_END, CH2_BUF_START + (fci_subch_info.bitrate*12*2)-1);
    bbm_com_word_write(NULL, BBM_BUF_CH2_THR, (fci_subch_info.bitrate*6*2)-1);
#endif
    res = bbm_com_audio_select(NULL, subChId, 2);
#ifdef FC8080_USE_TSIF
    fc8080_demux_select_channel(subChId, 2);
#endif
  }
  else if(serviceType == TDMB_BB_SVC_DMB)
  {
    res = bbm_com_video_select(NULL, subChId, 0, 0);
#ifdef FC8080_USE_TSIF
    fc8080_demux_select_video(subChId, 0);
#endif
  }
  else if(serviceType == TDMB_BB_SVC_DATA)
  {
    res = bbm_com_data_select(NULL, subChId, 1);
#ifdef FC8080_USE_TSIF
    fc8080_demux_select_channel(subChId, 1);
#endif
  }

  if(res)
    return FALSE;

  return TRUE;
}

/*====================================================================
FUNCTION       fc8080_read_int
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tdmb_bb_int_type fc8080_read_int(void)
{
  //TDMB_MSG_FCI_BB("[%s]\n", __func__);

#ifdef FC8080_USE_TSIF
  if(g_fci_ch_start)
  {
    TDMB_MSG_FCI_BB("Return ISR [%d]\n",g_fci_ch_start);
    return g_Int_type;
  }
#endif

  fc8080_isr(NULL);
  return g_Int_type;
}

/*====================================================================
FUNCTION       fc8080_get_sync_status
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
tBBStatus fc8080_get_sync_status(void)
{
  uint8  sync_status = 0;

  bbm_com_read(NULL, BBM_SYNC_STAT, &sync_status);

  if(sync_status & 0x20)
  {
    TDMB_MSG_FCI_BB("[%s] SYNC SUCESS\n", __func__);
    return BB_SUCCESS;
  }
  else
  {
    TDMB_MSG_FCI_BB("[%s] SYNC FAIL\n", __func__);
    return BB_FAIL;
  }
}

/*====================================================================
FUNCTION       fc8080_read_fib
DESCRIPTION   For FIC_CALLBACK using
DEPENDENCIES
RETURN VALUE number of FIB
SIDE EFFECTS
======================================================================*/
int fc8080_read_fib(uint8 *fibs)
{
  uint32 size=0;
  size = fic_type.length;

  if(fic_type.address!=0)
  {
    memcpy(&fibs[0], (void*)fic_type.address, size);

#ifdef FEATURE_DMB_DUMP_FILE
    dmb_data_dump(fibs, size, FILENAME_RAW_FIC);
#endif

  TDMB_MSG_FCI_BB("[%s] size[%d] data[%x]\n", __func__, (int)size,fibs[0]);
  }

  fic_type.length=0;
  fic_type.address=0;

  return size/32;
}

/*====================================================================
FUNCTION       fc8080_set_subchannel_info
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_set_subchannel_info(void * sub_ch_info)
{
  static chan_info *fci_ch_info;

  fci_ch_info = (chan_info *)sub_ch_info;

  fci_subch_info.freq = fci_ch_info->ulRFNum;
  fci_subch_info.servicetype = fci_ch_info->uiServiceType;
  fci_subch_info.subChId = (uint8)fci_ch_info->uiSubChID;
  fci_subch_info.bitrate = fci_ch_info->uiBitRate;
}

/*====================================================================
FUNCTION       fc8080_read_msc
DESCRIPTION  For MSC_CALLBACK using
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8080_read_msc(uint8 *msc_buffer)
{
  int msc_size, fic_size=0;

  //TDMB_MSG_FCI_BB("[%s] \n",__func__);

  fic_size = fic_type.length;
  fic_type.length=0;

#if defined(FEATURE_TDMB_MULTI_CHANNEL_ENABLE)
  if(fic_size != 0)
  {
    if(fic_type.address!=0)
    {
      memcpy(&ts_data.fic_buf[0], (void*)fic_type.address, fic_size);
      ts_data.type = FIC_DATA;
      ts_data.fic_size = fic_size;
      //TDMB_MSG_FCI_BB("[%s] fic data [%x] size[%d]\n", __func__, (unsigned int)fic_type.address, fic_size);
    }
  }
  fic_type.address=0;
#endif

  if(gOutputBuffer == gInputBuffer)
  {
    //TDMB_MSG_FCI_BB("[%s] msc buffer error !\n", __func__);
    return 0;
  }

  msc_size = msc_type[gOutputBuffer].length;

  if(!msc_type[gOutputBuffer].address || !msc_size)
  {
    TDMB_MSG_FCI_BB("[%s] msc size error ![%d]\n", __func__,msc_size);
    return 0;
  }

  memcpy(&msc_buffer[0], (void*) &msc_data[gOutputBuffer * FC8080_MSC_BUFFER_SIZE], msc_size);

#ifdef FEATURE_DMB_DUMP_FILE
  dmb_data_dump(msc_buffer, msc_size, FILENAME_AFTER_PARSING);
#endif  

  if(ts_data.type == FIC_DATA)
    ts_data.type = FIC_MSC_DATA;
  else
    ts_data.type = MSC_DATA;

  msc_type[gOutputBuffer].address = 0;
  msc_type[gOutputBuffer].length = 0;
  msc_type[gOutputBuffer].subch_id = 0;

  gOutputBuffer = (gOutputBuffer + 1) % FC8080_MSC_BUFFERS;

  //TDMB_MSG_FCI_BB("[%s]  type[%d] msc[%d] fic[%d]",__func__, ts_data.type, msc_size, fic_size);

#if defined(FEATURE_TEST_ON_BOOT) && !defined(FEATURE_TDMB_MULTI_CHANNEL_ENABLE)
  {
    uint16 i = 0;
    for(i=0; i<msc_size;i+=188)
    {
      TDMB_MSG_FCI_BB("[%s] size[%d]  [%x] [%x] [%x] [%x]\n", __func__, msc_size, msc_buffer[i+0], msc_buffer[i+1], msc_buffer[i+2], msc_buffer[i+3]);
    }
  }
#endif

  return msc_size;
}

/*====================================================================
FUNCTION       fc8080_RFInputPower
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8080_RFInputPower(void)
{
  return abs(rssi_value);
}

/*====================================================================
FUNCTION       fc8080_reconfig_n_ber
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_get_ber(tdmb_bb_sig_type *psigs)
{
  struct dm_st {
  	u32 start;

  	u16 reserved0;
  	u16 reserved1;
  	u32 reserved2;

  	u32 dmp_ber_rxd_bits;
  	u32 dmp_ber_err_bits;

  	u16 ber_rxd_rsps;
  	u16 ber_err_rsps;
  	u32 ber_err_bits;
  };

  struct dm_st dm;

  bbm_com_bulk_read(NULL, BBM_DM, (u8*) &dm, sizeof(dm));

  rssi_value = (s8) ((dm.ber_err_bits & 0xff000000) >> 24);

  dm.ber_err_bits &= 0x00ffffff;
  dm.dmp_ber_err_bits &= 0x00ffffff;


  if(dm.dmp_ber_rxd_bits == 0)
  	psigs->PCBER = 20000;
  else if(dm.dmp_ber_err_bits == 0)
  	psigs->PCBER = 0;
  else
  {
  	if(dm.dmp_ber_err_bits > 429490)
  		psigs->PCBER = ((dm.dmp_ber_err_bits * 1000)/dm.dmp_ber_rxd_bits)*10;
  	else
  		psigs->PCBER = (dm.dmp_ber_err_bits*10000)/dm.dmp_ber_rxd_bits;
  }

  if(dm.ber_rxd_rsps == 0)
  	psigs->RSBER = 20000;

  else if((dm.ber_err_bits == 0) && (dm.ber_err_rsps == 0))
  {
  	psigs->RSBER = 0;
  }
  else
  {
  	if(dm.ber_err_bits > 429490)
  		psigs->RSBER = ((dm.ber_err_bits * 1000)/(dm.ber_rxd_rsps * 204 * 8))*10;
  	else
  		psigs->RSBER = (dm.ber_err_bits*10000)/(dm.ber_rxd_rsps * 204 * 8);
  }

  psigs->SNR = fc8080_Ant_Level(rssi_value, psigs->PCBER);
  psigs->RSSI = rssi_value;
  psigs->RxPowerLevel = rssi_value;
  //TDMB_MSG_FCI_BB("pcber[%d] snr[%d] rsber[%d] rssi[%d]  [%d]\n", psigs->PCBER, psigs->SNR, psigs->RSBER,psigs->RSSI,psigs->RxPowerLevel);
}

/*====================================================================
FUNCTION       fc8080_Ant_Level
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int fc8080_Ant_Level(int RSSI, uint32 pcber)
{
  uint8 level = 0;
  static uint16 save_level=0;
#if 1 //EF63 ANT level change (0~4)
  uint16 ant_tbl[] = {1000, // 0 <-> 1
                       750,  // 1 <-> 2
                       500,  // 2 <-> 3
                       200};  // 3 <-> 4
  uint16 hystery_value[]= {0, 30, 30, 30};
#else
  uint16 ant_tbl[] = {1100, // 0 <-> 1
                       800,  // 1 <-> 2
                       550,  // 2 <-> 3
                       300,  // 3 <-> 4
                       100}; // 4 <-> 5
  uint16 hystery_value[]= {0, 20, 20, 20, 20};
#endif

  if(pcber >= ant_tbl[0])
  {
    save_level = level = 0;
    return save_level;
  }

  if((pcber >= ant_tbl[1] && pcber < ant_tbl[0])) level = 1;
  else if((pcber >= ant_tbl[2] && pcber < ant_tbl[1])) level = 2;
  else if((pcber >= ant_tbl[3] && pcber < ant_tbl[2])) level = 3;
  else if(pcber < ant_tbl[3]) level = 4;

  if(level == save_level + 1) // Level 이 1칸 올라간 경우에만.
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
FUNCTION       fc8080_get_demux_buffer
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8080_get_demux_buffer(u8 **res_ptr, u32 *res_size)
{
  TDMB_MSG_FCI_BB("[%s]\n", __func__);

  if(gOutputBuffer == gInputBuffer)
    return;

  *res_size = msc_type[gOutputBuffer].length;

  if(!msc_type[gOutputBuffer].address || !*res_size)
    return;

  *res_ptr = &msc_data[gOutputBuffer * FC8080_MSC_BUFFER_SIZE];

  if(ts_data.type == FIC_DATA)
    ts_data.type = FIC_MSC_DATA;
  else
    ts_data.type = MSC_DATA;

  msc_type[gOutputBuffer].address = 0;
  msc_type[gOutputBuffer].length = 0;
  msc_type[gOutputBuffer].subch_id = 0;

  gOutputBuffer = (gOutputBuffer + 1) % FC8080_MSC_BUFFERS;
}

/*====================================================================
FUNCTION       fc8050_test
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void fc8050_test(int ch)
{
  uint32 bitrate;
  int result;
  REGS fci_regs;
  chan_info fci_ch_info;

  TDMB_MSG_FCI_BB("[%s] ch[%d]\n", __func__, ch);

  if(tdmb_power_on == FALSE)
  {
    fc8080_power_on();
  }
  
  fc8080_drv_init();

#if defined(FC8080_IF_RW_TEST) || defined(FEATURE_TEST_ON_BOOT)
  fc8080_if_test(1); // byte
  fc8080_if_test(2); // word
  fc8080_if_test(3); // long
  fc8080_if_test(4); // tuner
#endif /*  FC8080_I2C_RW_TEST */

  tdmb_get_fixed_chan_info((service_t)ch, &fci_ch_info);

  fci_regs.freq = fci_ch_info.ulRFNum;
  fci_regs.servicetype = (fci_ch_info.uiServiceType == TDMB_BB_SVC_DAB) ? 1 : 2;
  fci_ch_info.uiServiceType = fci_regs.servicetype;
  fci_regs.subChId = (uint8)fci_ch_info.uiSubChID;

  bitrate = (uint32)fci_ch_info.uiBitRate;

  TDMB_MSG_FCI_BB("[%s] freq[%d]  type[%d] subchid[0x%x] bitrate[%d]\n", __func__, (int)fci_regs.freq, fci_regs.servicetype, fci_regs.subChId, (int)bitrate);

  fc8080_set_subchannel_info(&fci_ch_info);

  // cys
  fc8080_ch_scan_start((int)fci_regs.freq, 0, 0);
//return;
  result = fc8080_subch_start((uint8*)&fci_regs, bitrate);
  //fc8080_power_off();

  if(result == FALSE)
  {
    TDMB_MSG_FCI_BB("[%s] fc8080_subch_start fail [%d]\n", __func__, result);
  }
}

