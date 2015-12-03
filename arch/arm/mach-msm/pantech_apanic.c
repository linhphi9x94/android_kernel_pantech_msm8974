/* drivers/misc/pantech_apanic.c
 *
 * Copyright (C) 2011 PANTECH, Co. Ltd.
 * based on drivers/misc/apanic.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      See the
 * GNU General Public License for more details.
 *
 */

/*---------------------------------------------------------------------------------------------------
 ** Common Header Include
 **-------------------------------------------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <linux/fb.h>
#include <linux/time.h>
#include <mach/pantech_apanic.h>
#include <linux/input.h>
#include <linux/io.h>
#include "smd_private.h"
#include "modem_notifier.h"
#include <linux/nmi.h>
#include <mach/msm_iomap.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <mach/pantech_sys_info.h>
#include <mach/pantech_sys.h>
/*---------------------------------------------------------------------------------------------------
 ** Define
 **-------------------------------------------------------------------------------------------------*/
#define PANIC_MAGIC    0xDAEDDAED
#define PHDR_VERSION   0x01


struct fuel_gauge_info{
	int percent_soc;
	unsigned int cable_type;
};

typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */

typedef struct {
  unsigned int partition_size_;
  char   partition_name_[ 8 ];
}partition_info_type;

typedef struct {
  uint32 version_;
  char   model_name_    [ 16 ];
  char   binary_version_[ 16 ];
  uint32 fs_version_;
  uint32 nv_version_;
  char   build_date_    [ 16 ];
  char   build_time_    [ 16 ];

  uint32 boot_loader_version_;
  uint32 boot_section_id_[4];

  uint32              efs_size_;
  uint32              partition_num_;
  partition_info_type partition_info_[ 6 ];

  uint32 FusionID;
  uint8  Imei[15];
  /* F_PANTECH_SECBOOT - DO NOT erase this comment! */
  char   secure_magic_[ 7 ];
  char   sub_binary_version_[8];

  uint32 preload_checksum; 
  uint8  reserved_2[ 42 ];
} __attribute__((packed)) phoneinfo_type;
/*
  PANTECH Shared memory region
*/
struct pantech_panic_info {
	unsigned int magic;
	/* log info - each log pointer*/
	struct msg_log msg;

	/* Dump mode setting info */
	int in_panic;
	int ramdump; /* 0:Log, 1:Log & RAM dump */
	int usbdump; /* 0:Dump via emmc 1: Dump via USB*/

	/* Force crash */
	int errortest;

	/* Fuel gauge info - percent_soc , cable_type*/
	struct fuel_gauge_info fuel_gauge;

	/* Phone info - struct phoneinfo_type */
	phoneinfo_type phoneinfo_buff_ptr;

	/*Schedule Info - index, time, task name, pid*/
	atomic_t sched_idx[CONFIG_NR_CPUS];
	struct sched_log sched[CONFIG_NR_CPUS][SCHED_LOG_MAX];

	/*IRQ Info - irq number, start time, elapsed time, irqs flag, */
	atomic_t irq_idx[CONFIG_NR_CPUS];
	struct irq_log irq[CONFIG_NR_CPUS][SCHED_LOG_MAX];
};
/*---------------------------------------------------------------------------------------------------
 ** extern function declare
 **-------------------------------------------------------------------------------------------------*/
extern pantech_log_header *get_pantech_klog_dump_address(void);
extern pantech_log_header *get_pantech_logcat_dump_address(void);

/*---------------------------------------------------------------------------------------------------
 ** global / static variable define
 **-------------------------------------------------------------------------------------------------*/
static pantech_log_header *crash_buf_header = NULL;

/*---------------------------------------------------------------------------------------------------
 ** static function define
 **-------------------------------------------------------------------------------------------------*/
#define POS(x) (x > 0 ? x : 0)
#define WRITE_LOG(...) \
	do{\
		if(bufsize != 0) { \
			n = snprintf(s, POS(bufsize), __VA_ARGS__); \
			s+=n; \
			total +=n; \
			bufsize-=n;\
  		}\
	}while(0)

static void apanic_get_mmu_info(void)
{
	int  bufsize = MMU_SCRIPT_BUF_SIZE, n = 0,total=0;
	char *s;

	unsigned int mmu_transbase0,mmu_transbase1;
	unsigned int mmu_dac,mmu_control;
	unsigned int mmu_prrr,mmu_nmrr;

	asm("mrc p15,0,%0,c1,c0,0" : "=r" (mmu_control));
	asm("mrc p15,0,%0,c2,c0,0" : "=r" (mmu_transbase0));
	asm("mrc p15,0,%0,c3,c0,0" : "=r" (mmu_dac));
	asm("mrc p15,0,%0,c2,c0,1" : "=r" (mmu_transbase1));
	asm("mrc p15,0,%0,c10,c2,0" : "=r" (mmu_prrr));
	asm("mrc p15,0,%0,c10,c2,1" : "=r" (mmu_nmrr));

	s =(char *)crash_buf_header->mmu_cmm_script;	

	WRITE_LOG("PER.S C15:0x1 %%LONG 0x%X\n",mmu_control);
	WRITE_LOG("PER.S C15:0x2 %%LONG 0x%X\n",mmu_transbase0);
	WRITE_LOG("PER.S C15:0x3 %%LONG 0x%X\n",mmu_dac);
	WRITE_LOG("PER.S C15:0x102 %%LONG 0x%X\n",mmu_transbase1);
	WRITE_LOG("PER.S C15:0x2A %%LONG 0x%X\n",mmu_prrr);
	WRITE_LOG("PER.S C15:0x12A %%LONG 0x%X\n",mmu_nmrr);
	WRITE_LOG("MMU.SCAN\n");
	WRITE_LOG("MMU.ON\n");
	WRITE_LOG("\n\n\n");  /* 32bit boundary */
	
	crash_buf_header->mmu_cmm_size = total;
}

void pantech_force_dump_key(unsigned int code, int value)
{
	static unsigned int step = 0;
       static int usbdump = 0;
       //printk(KERN_DEBUG "Crash key : %d , value : %d\n", code, value);
	
      	usbdump = (int)IS_SYS_USBDUMP_MODE;
    	//p16101 printk(KERN_DEBUG "USBDUMP : %d \n", usbdump);

	if( (usbdump == 0) || (value !=1) )
       {
		step = 0;
		return;
	}

	switch(step)
	{
#if defined (CONFIG_MACH_MSM8974_EF60S) || defined (CONFIG_MACH_MSM8974_EF65S) || defined (CONFIG_MACH_MSM8974_EF61K) || defined (CONFIG_MACH_MSM8974_EF62L)
		case 0:
			if(code == KEY_VOLUMEUP)
				step = 1;
			else 
				step = 0;
			break;
		case 1:
			if(code == KEY_POWER)
				step = 2;
			else 
				step = 0;
			break;
		case 2:
			if(code == KEY_HOME)
				panic("linux- Force dump key");
			else
				step = 0;
			break;

#else
		case 0:
			if(code == KEY_VOLUMEDOWN)
				step = 1;
			else 
				step = 0;
			break;
		case 1:
			if(code == KEY_POWER)
				step = 2;
			else 
				step = 0;
			break;
#if defined (CONFIG_MACH_MSM8974_EF56S)			
		case 2:
			if(code == KEY_VOLUMEUP)
				panic("linux- Force dump key");
			else
				step = 0;
			break;
#else
		case 2:
			if(code == KEY_VOLUMEUP)
				step = 3;
			else 
				step = 0;
			break;
		case 3:
			if(code == KEY_HOME)
				panic("linux- Force dump key");
			else
				step = 0;
			break;
#endif

#endif
	}
}
EXPORT_SYMBOL(pantech_force_dump_key);

static int apanic_logging(struct notifier_block *this, unsigned long event, void *ptr)
{
      printk(KERN_EMERG "pantech_apanic: apanic_logging start\n");

#ifdef CONFIG_PREEMPT
      /* Ensure that cond_resched() won't try to preempt anybody */
      add_preempt_count(PREEMPT_ACTIVE);
#endif

      touch_softlockup_watchdog();

      if(crash_buf_header != NULL)
      {
          crash_buf_header->magic = PANIC_MAGIC;
          crash_buf_header->version = PHDR_VERSION;
          apanic_get_mmu_info();
      }
      else
          printk(KERN_ERR "apanic_logging : crash_buf_header is not initialized!\n");

      flush_cache_all();

#ifdef CONFIG_PREEMPT
      sub_preempt_count(PREEMPT_ACTIVE);
#endif

      return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
      .notifier_call    = apanic_logging,
};


/*---------------------------------------------------------------------------------------------------
 ** non - static function define
 **-------------------------------------------------------------------------------------------------*/
int crash_buf_header_init(void)
{
	unsigned size;

	crash_buf_header = (pantech_log_header *)smem_get_entry(SMEM_ID_VENDOR2, &size);

	if(!crash_buf_header){
		printk(KERN_ERR "pantech_apanic: no available crash buffer , initial failed\n");
		return 0;
	}
	else
	{
		atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
	}

	return 1;
}

void set_pantech_dbg_buf_info(unsigned int* addr, unsigned int size)
{
	int ret;

	if(!crash_buf_header)
	{
		ret = crash_buf_header_init();
		if(!ret)
			return ;
	}

	crash_buf_header->pantech_dbg_addr = addr;
	crash_buf_header->pantech_dbg_size = size;
}

/*****************************************************
 * PNANTEH APANIC MODULE INIT
 * **************************************************/
int __init pantech_apanic_init(void)
{
	int ret;

      pantech_log_header *klog_header, *logcat_log_header;

      klog_header = get_pantech_klog_dump_address();
      logcat_log_header = get_pantech_logcat_dump_address();

	  if(!crash_buf_header)
	  {
		  ret = crash_buf_header_init();
		  if(!ret)
			  return 0;
	  }

	  crash_buf_header->magic=0;
	  crash_buf_header->version=0;
	  crash_buf_header->klog_buf_address = klog_header->klog_buf_address;
	  crash_buf_header->klog_end_idx = klog_header->klog_end_idx;
	  crash_buf_header->klog_size = klog_header->klog_size;
	  
	  crash_buf_header->mlogcat_buf_address = logcat_log_header->mlogcat_buf_address;
	  crash_buf_header->mlogcat_w_off = logcat_log_header->mlogcat_w_off;
	  crash_buf_header->mlogcat_size = logcat_log_header->mlogcat_size;

	  crash_buf_header->slogcat_buf_address = logcat_log_header->slogcat_buf_address;
	  crash_buf_header->slogcat_w_off = logcat_log_header->slogcat_w_off;
	  crash_buf_header->slogcat_size = logcat_log_header->slogcat_size;

	  crash_buf_header->rlogcat_buf_address = logcat_log_header->rlogcat_buf_address;
	  crash_buf_header->rlogcat_w_off = logcat_log_header->rlogcat_w_off;
	  crash_buf_header->rlogcat_size = logcat_log_header->rlogcat_size;

	  printk("pantech_apanic : pantech_log_header initialized success for write to SMEM\n");

	  printk(KERN_INFO "apanic_pantech_init\n");
	  return 0;
}

module_init(pantech_apanic_init);

MODULE_AUTHOR("Pantech ls4 part1>");
MODULE_DESCRIPTION("Pantech errlogging driver");
MODULE_LICENSE("GPL v2");
