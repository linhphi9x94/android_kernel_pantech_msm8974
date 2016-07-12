//=============================================================================
// File       : dmb_test.c
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2012/03/29       yschoi         Create
//=============================================================================

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/syscalls.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "dmb_comdef.h"
#include "dmb_type.h"
#include "dmb_test.h"

#ifdef FEATURE_TEST_ON_BOOT
#include <linux/timer.h>
#include <linux/jiffies.h>

#ifdef FEATURE_DMB_TSIF_IF
#include "dmb_tsif.h"
#endif
#include "dmb_hw.h"

#ifdef CONFIG_SKY_TDMB
#include "tdmb/tdmb_comdef.h"
#include "tdmb/tdmb_chip.h"
#include "tdmb/tdmb_bb.h"
#endif

#ifdef CONFIG_SKY_ISDBT
#include "isdbt/isdbt_comdef.h"
#include "isdbt/isdbt_chip.h"
#include "isdbt/isdbt_bb.h"
#endif
#endif

/*================================================================== */
/*==============        DMB TEST Definition     =============== */
/*================================================================== */
#ifdef FEATURE_TEST_ON_BOOT
#ifdef CONFIG_SKY_TDMB
#define DMB_TEST_CH 11 //TDMB_MFLO_IU
#endif

#ifdef CONFIG_SKY_ISDBT
#define DMB_TEST_CH 27 //557143
tIsdbtTunerInfo test_sig_info;
#endif

static struct timer_list dmb_test_tmer;
static struct workqueue_struct *dmb_boot_wq;
static struct work_struct dmb_test_startwq;

#define TDMB_BOOT_TEST_START_TIME 10

//#define FEATURE_BOOTTEST_READ_BER
#ifdef FEATURE_BOOTTEST_READ_BER
static struct timer_list dmb_ber_timer;
static struct work_struct dmb_test_berwq;
#endif
static int dmb_test_cnt = 0;
#endif /* FEATURE_TEST_ON_BOOT */

/*================================================================== */
/*==============        DMB TEST Function     =============== */
/*================================================================== */

#ifdef DMB_MSG_LOG_SAVE
/*===========================================================================
FUNCTION       dmb_msg_log
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_msg_log(const char *fmt, ...)
{
  struct file *file;
  loff_t pos = 0;
  int fd, s_len;
  char log_buf[512];
  va_list args;

  char tbuf[50];
  int cpu = raw_smp_processor_id();
  unsigned tlen;
  unsigned long long t;
  unsigned long nanosec_rem;

  mm_segment_t old_fs = get_fs();
  set_fs(KERNEL_DS);
  
  t = cpu_clock(cpu);
  nanosec_rem = do_div(t, 1000000000);
#if 0
  tlen = sprintf(tbuf, "[%5lu.%06lu] ", (unsigned long) t, nanosec_rem / 1000);
#else
{
  struct tm test_date;
  struct timeval test_time;

  do_gettimeofday(&test_time);
  time_to_tm(test_time.tv_sec, 0, &test_date); //check "0"..

  tlen = sprintf(tbuf, "[%5lu.%06lu  %02d/%02d %02d:%02d:%02d:%03lu] ", (unsigned long) t, nanosec_rem / 1000,  
                     test_date.tm_mon+1, test_date.tm_mday, test_date.tm_hour-sys_tz.tz_minuteswest/60, test_date.tm_min, test_date.tm_sec, (unsigned long)test_time.tv_usec/1000);
}
#endif
  strcpy(&log_buf[0], tbuf);

  va_start(args, fmt);
#if 0
  vsprintf(&log_buf[15], fmt, args);
#else
  vsprintf(&log_buf[35], fmt, args);
#endif
  va_end(args);

  s_len = strlen(log_buf);
  
  fd = sys_open("/data/misc/dmb/dmb_msg.log", O_CREAT | O_RDWR | O_APPEND | O_LARGEFILE, 0644);

  if(fd >= 0) 
  {
    file = fget(fd);
    if(file) 
    {
      vfs_write(file, log_buf, s_len, &pos);
      fput(file);
    }
    sys_close(fd);
  }
  else
  {
    DMB_MSG_TEST("%s open failed  fd [%d]\n", __func__, fd);
  }
  set_fs(old_fs);
}
#endif

/*===========================================================================
FUNCTION       dmb_data_dump
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_data_dump(u8 *p, u32 size, char *filename)
{
  struct file *file;
  loff_t pos = 0;
  int fd;
  mm_segment_t old_fs = get_fs();
  set_fs(KERNEL_DS);
  
  fd = sys_open(filename, O_CREAT | O_RDWR | O_APPEND | O_LARGEFILE, 0644);
  if(fd >= 0) 
  {
    file = fget(fd);
    if(file) 
    {
      vfs_write(file, p, size, &pos);
      fput(file);
    }
    sys_close(fd);
  }
  else
  {
    DMB_MSG_TEST("%s open failed  fd [%d]\n", __func__, fd);
  }
  set_fs(old_fs);
}


#ifdef FEATURE_TEST_ON_BOOT
#ifdef FEATURE_BOOTTEST_READ_BER
/*===========================================================================
FUNCTION       dmb_ber_work
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_ber_work(struct work_struct *work)
{
  //DMB_MSG_TEST("%s",__func__);

#ifdef  CONFIG_SKY_TDMB
  tdmb_bb_get_ber();
#endif

#ifdef CONFIG_SKY_ISDBT
  isdbt_bb_get_tuner_info(&test_sig_info);
#endif
}

/*===========================================================================
FUNCTION       dmb_ber_callback
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_ber_callback(unsigned long data)
{
  //DMB_MSG_TEST("%s",__func__);

  mod_timer(&dmb_ber_timer, jiffies + msecs_to_jiffies(1000));
  if(dmb_boot_wq != NULL)
  {
    queue_work(dmb_boot_wq, &dmb_test_berwq);
  }
}
#endif

/*===========================================================================
FUNCTION       dmb_test_delete_timer
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_test_delete_timer(void)
{
  //DMB_MSG_TEST("%s..\n",__func__);

  dmb_test_cnt = 0;  
  del_timer(&dmb_test_tmer); 
}

/*===========================================================================
FUNCTION       dmb_test_work
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_test_work(struct work_struct *work)
{
  //DMB_MSG_TEST("%s",__func__);

  tdmb_bb_power_on();

  dmb_set_ant_path(DMB_ANT_EARJACK);

#if (defined(FEATURE_DMB_EBI_IF) || defined(FEATURE_DMB_SPI_IF))
  tdmb_bb_set_fic_isr(1);
#endif

#ifdef FEATURE_DMB_TSIF_IF
  dmb_tsif_test();
#endif /* FEATURE_DMB_TSIF_IF */

#ifdef CONFIG_SKY_TDMB
#ifdef FEATURE_TDMB_USE_INC  
   t3700_test(DMB_TEST_CH);
#elif defined(FEATURE_TDMB_USE_FCI)
   fc8050_test(DMB_TEST_CH);
#elif defined(FEATURE_TDMB_USE_RTV)
   mtv350_test(DMB_TEST_CH);
#elif defined(FEATURE_TDMB_USE_TCC)
   tcc3170_test(DMB_TEST_CH);
#else
  #error
#endif
#endif /* CONFIG_SKY_TDMB */

#ifdef CONFIG_SKY_ISDBT
#if defined (FEATURE_ISDBT_USE_FC8150)
    fc8150_test(DMB_TEST_CH);
#elif defined (FEATURE_ISDBT_USE_SHARP)
  sharp_test(DMB_TEST_CH);
#else
  #error
#endif
#endif /* CONFIG_SKY_ISDBT */

#ifdef FEATURE_BOOTTEST_READ_BER
  setup_timer(&dmb_ber_timer, dmb_ber_callback, 0);
  mod_timer(&dmb_ber_timer, jiffies+msecs_to_jiffies(1000));
  INIT_WORK(&dmb_test_berwq, dmb_ber_work);
#endif
}

/*===========================================================================
FUNCTION       dmb_test_callback
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_test_callback(unsigned long data)
{  
  DMB_MSG_TEST("%s  cnt [%d]",__func__, dmb_test_cnt);

  dmb_test_cnt++;
  if(dmb_test_cnt == TDMB_BOOT_TEST_START_TIME)
  {
    dmb_test_delete_timer();
    if(dmb_boot_wq != NULL)
    {
      queue_work(dmb_boot_wq, &dmb_test_startwq);
    }
  }
  else
  {
    mod_timer(&dmb_test_tmer, jiffies + msecs_to_jiffies(1000));
  }
}

/*===========================================================================
FUNCTION       dmb_ch_test
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_ch_test(void)
{
  DMB_MSG_TEST("[%s] !!\n", __func__);

  dmb_boot_wq = create_singlethread_workqueue("dmb_boot_wq");
  if(dmb_boot_wq == NULL)
  {
    DMB_MSG_TEST("[%s] dmb work queue is NULL !!\n",__func__);
  }

  setup_timer(&dmb_test_tmer, dmb_test_callback, 0);
  mod_timer(&dmb_test_tmer, jiffies+msecs_to_jiffies(1000));
  INIT_WORK(&dmb_test_startwq, dmb_test_work);
}

/*===========================================================================
FUNCTION       dmb_test_on_boot
DESCRIPTION
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
===========================================================================*/
void dmb_test_on_boot(void)
{
  DMB_MSG_TEST("[%s] Test start!!!\n", __func__);

#ifdef CONFIG_SKY_TDMB
  //tdmb_bb_power_on();

#ifdef FEATURE_NETBER_TEST_ON_BOOT
  netber_init();
  dmb_mode = TDMB_MODE_NETBER;
#endif

  dmb_ch_test();
#endif /* CONFIG_SKY_TDMB */

#ifdef CONFIG_SKY_ISDBT
  isdbt_bb_power_on();

#ifdef FEATURE_COMMAND_TEST_ON_BOOT  
  #if defined(FEATURE_DMB_SPI_CMD)
      // TO DO
  #else
    #ifdef FEATURE_ISDBT_USE_SHARP
      sharp_i2c_test();
    #endif
  #endif
#else
  dmb_ch_test();
#if defined (FEATURE_DMB_SPI_IF)
  isdbt_bb_set_isr(1);
#endif
#endif
#endif /* CONFIG_SKY_ISDBT */

  DMB_MSG_TEST("[%s] Test end!!!\n", __func__);
}
#endif /* FEATURE_TEST_ON_BOO T*/
