/* pantech_sys.h
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

/*****************
 *** INCLUDE ***
 *****************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <mach/pantech_sys.h>
#include <linux/proc_fs.h>
#include <mach/msm_smem.h>
#include <asm/uaccess.h>

#if defined(CONFIG_PANTECH_DEBUG)
#include <mach/pantech_debug.h> 
#endif

/*******************
 *** VARIABLES ***
 *******************/

unsigned int prev_reset_reason = 0x0;
unsigned int prev_backlight_off_flag = 0x0;

static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
static uint32_t oem_prev_reset=0;
unsigned int silent_boot_mode;
unsigned int silent_boot_mode_backlight;

#if defined(CONFIG_MACH_MSM8974_EF65S)
unsigned int skt_charging_mode;
#endif

/***************************************
 *** STATIC FUNCTION DEFINATION ***
 ***************************************/

static int oem_pm_read_proc_reset_info(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
			sizeof(oem_pm_smem_vendor1_data_type));

	len  = sprintf(page, "Factory Cable Adc : 0x%x\n", smem_id_vendor1_ptr->factory_cable_adc);
	len += sprintf(page + len, "Battery Id Adc: %d\n", smem_id_vendor1_ptr->battery_id_adc);
	len += sprintf(page + len, "HW Revision Adc: %d\n", smem_id_vendor1_ptr->hw_rev_adc);
	len += sprintf(page + len, "Power On Mode : %d\n", smem_id_vendor1_ptr->power_on_mode);
	len += sprintf(page + len, "SilentBoot: %d\n", (smem_id_vendor1_ptr->silent_boot_mode ? 1 : 0 ) );
	len += sprintf(page + len, "HW Revision: %d\n", smem_id_vendor1_ptr->hw_rev);
	len += sprintf(page + len, "Battery Id: %d\n", smem_id_vendor1_ptr->battery_id);
	len += sprintf(page + len, "Backlight Off: %d\n", (smem_id_vendor1_ptr->backlight_off ? 1 : 0 ) );
	len += sprintf(page + len, "Reset: %d\n", oem_prev_reset);
#if defined(CONFIG_MACH_MSM8974_EF65S)
	len += sprintf(page + len, "SKT Charging Mode: %d\n", (smem_id_vendor1_ptr->skt_charging_mode ? 1 : 0 ) );
#endif
	
	printk(KERN_INFO "Factory Cable Adc : 0x%x\n", smem_id_vendor1_ptr->factory_cable_adc);
	printk(KERN_INFO "Battery Id Adc: %d\n", smem_id_vendor1_ptr->battery_id_adc);
	printk(KERN_INFO "HW Revision Adc: %d\n", smem_id_vendor1_ptr->hw_rev_adc);
	printk(KERN_INFO "Power On Mode : %d\n", smem_id_vendor1_ptr->power_on_mode);
	printk(KERN_INFO "SilentBoot: %d\n", (smem_id_vendor1_ptr->silent_boot_mode ? 1 : 0 ) );
	printk(KERN_INFO "HW Revision: %d\n", smem_id_vendor1_ptr->hw_rev);
	printk(KERN_INFO "Battery Id: %d\n", smem_id_vendor1_ptr->battery_id);
	printk(KERN_INFO "Backlight Off: %d\n", (smem_id_vendor1_ptr->backlight_off ? 1 : 0 ) );
	printk(KERN_INFO "Reset: %d\n", oem_prev_reset);
#if defined(CONFIG_MACH_MSM8974_EF65S)
	printk(KERN_INFO "SKT Charging Mode: %d\n", (smem_id_vendor1_ptr->skt_charging_mode ? 1 : 0 ) );
#endif
	return len;
}

/********************************************
 *** NON-STATIC FUNCTION DEFINATION ***
 ********************************************/

int oem_pm_write_proc_reset_info(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len;
	char tbuffer[2];

	if(count > 1 )
		len = 1;

	memset(tbuffer, 0x00, 2);

	if(copy_from_user(tbuffer, buffer, len))
		return -EFAULT;

	tbuffer[len] = '\0';

	if(tbuffer[0] >= '0' && tbuffer[0] <= '9')
		oem_prev_reset = tbuffer[0] - '0';

	return len;
}

void pantech_sys_reset_reason_init(void)
{
	unsigned int temp_usbdump_mode = 0x0;
	unsigned int temp_ramdump_mode = 0x0;

	// related with silent reboot - p16652
	struct proc_dir_entry *oem_pm_power_on_info;

	prev_reset_reason = GET_SYS_RESET_REASON_ALL;
	printk(KERN_INFO "[%s] prev_reset_reason : 0x%08X\n",__func__, prev_reset_reason);

	temp_usbdump_mode = IS_SYS_USBDUMP_MODE;
	temp_ramdump_mode = IS_SYS_RAMDUMP_MODE;

#if defined(CONFIG_PANTECH_DEBUG)
	if(IS_SYS_RAMDUMP_MODE)
		pantech_debug_enable = 1;
	else
		pantech_debug_enable = 0;
#endif
	silent_boot_mode = IS_SYS_RESET;

#if defined(CONFIG_MACH_MSM8974_EF65S)
	skt_charging_mode = IS_SYS_SKT_CHARGING_MODE;
#endif

	silent_boot_mode_backlight = GET_SYS_RESET_REASON_FLAG(SYS_RESET_BACKLIGHT_OFF_FLAG);

	SET_SYS_RESET_REASON_CLEAR(0x0);
	SET_SYS_RESET_REASON_ERROR(SYS_RESET_REASON_ABNORMAL);

	if(temp_usbdump_mode)
		SET_SYS_RESET_REASON_MODE(SYS_RESET_USBDUMP_MASK, 1);

	if(temp_ramdump_mode)
		SET_SYS_RESET_REASON_MODE(SYS_RESET_RAMDUMP_MASK, 1);

	printk(KERN_INFO "[%s] changed_reset_reason :0x%08X\n",__func__, GET_SYS_RESET_REASON_ALL);

	/* section - related with silent reboot info */
	oem_pm_power_on_info = create_proc_entry("pantech_resetinfo", S_IRUGO | S_IWUSR | S_IWGRP, NULL);

	if (oem_pm_power_on_info) {
		oem_pm_power_on_info->read_proc  = oem_pm_read_proc_reset_info;
		oem_pm_power_on_info->write_proc = oem_pm_write_proc_reset_info;
		oem_pm_power_on_info->data = NULL;
	}
}
EXPORT_SYMBOL(pantech_sys_reset_reason_init);

void pantech_sys_reset_reason_set(unsigned int reason)
{
	unsigned int reset_reason;

	reset_reason = GET_SYS_RESET_REASON_ERROR;
	printk(KERN_INFO "[%s] current_reset_reason : 0x%08X\n",__func__, reset_reason);

	if(reset_reason == SYS_RESET_REASON_ABNORMAL)
	{
		SET_SYS_RESET_REASON_ERROR(reason);
	}
	printk(KERN_INFO "[%s] changed_reset_reason :0x%08X\n",__func__, GET_SYS_RESET_REASON_ALL);
}
EXPORT_SYMBOL(pantech_sys_reset_reason_set);

unsigned int pantech_sys_reset_backlight_flag_get(void)
{
	return silent_boot_mode_backlight;
}
EXPORT_SYMBOL(pantech_sys_reset_backlight_flag_get);

void pantech_sys_reset_backlight_flag_set(unsigned int flag)
{
	SET_SYS_RESET_REASON_FLAG(SYS_RESET_BACKLIGHT_OFF_FLAG, flag);
	printk(KERN_INFO "[%s] changed_reset_reason :0x%08X\n",__func__, GET_SYS_RESET_REASON_ALL);
}
EXPORT_SYMBOL(pantech_sys_reset_backlight_flag_set);
unsigned int pantech_sys_rst_is_silent_boot_mode(void)
{
	return silent_boot_mode;
}
#if defined(CONFIG_MACH_MSM8974_EF65S)
unsigned int pantech_sys_rst_is_skt_charging_mode(void)
{
	return skt_charging_mode;
}
#endif