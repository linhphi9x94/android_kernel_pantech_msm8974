/* drivers/misc/pantech_restart.c
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <mach/msm_iomap.h>
#include <linux/io.h>
#include <linux/console.h>
#include "smd_private.h"
#include <mach/pantech_sys.h>
#include <mach/pantech_restart.h>

static int ramdump = 0;
static int usbdump = 0;
static int errortest = 0;
static int mdmdump = 0;
static int ssrmode = 0;

// p15060
#include "pantech_ssrsystem.h"
static char backup_buffer[SECTOR_SIZE];
sky_ssr_info_type *ssrinfo; // setting option
extern int modem_ssr_dump_enable;
int userdump = 0;

typedef enum {
	MODEM_ERR_FATAL=10,
	MODEM_WATCHDOG=11,
	MODEM_EXCP_SWI=12,
	MODEM_EXCP_UNDEF=13,
	MODEM_EXCP_MIS_ALIGN=14,  
	MODEM_EXCP_PAGE_FAULT=15,
	MODEM_EXCP_EXE_FAULT=16,
	MODEM_EXCP_DIV_BY_Z=17,
	LINUX_ERR_FATAL=20,
	LINUX_WATCHDOG=21,
	LINUX_EXCP_SWI=22,
	LINUX_EXCP_UNDEF=23,
	LINUX_EXCP_MIS_ALIGN=24,  
	LINUX_EXCP_PAGE_FAULT=25,
	LINUX_EXCP_EXE_FAULT=26,
	LINUX_EXCP_DIV_BY_Z=27,
	LINUX_FS_CRASH=28,
	SUBSYSTEM_ERR_MAX_NUM
}subsystem_err_type;

// p15060
int pantech_is_usbdump_enabled(void)
{
    return usbdump;
}
EXPORT_SYMBOL(pantech_is_usbdump_enabled);

static int pantech_prep_restart(struct notifier_block *this, unsigned long event, void *ptr) {
	return NOTIFY_DONE;
}

static struct notifier_block pantech_restart_blk = {
	.notifier_call    = pantech_prep_restart,
};

typedef void (*func_ptr)(void);
static const int _undef_inst = 0xFF0000FF;
static int div0_y=0;
static void diag_error_data_abort(void)
{
	*(int *)0 = 0;
}

static void diag_error_prefetch_abort(void)
{
	func_ptr func;
	func = (func_ptr)0xDEADDEAD;
	(*func)();
}

static void diag_error_undefined_instruction(void)
{
	func_ptr func;
	func = (func_ptr)(&_undef_inst);
	(*func)();
}

static DEFINE_SPINLOCK(state_lock);

static void diag_error_sw_watchdog(void)
{
	unsigned long irqflags;
	unsigned long  value = 0;  

	spin_lock_irqsave(&state_lock, irqflags);
	while(1){
		value = value ? 0 : 1;
	}
	spin_unlock_irqrestore(&state_lock, irqflags);
}

static void diag_error_div0(void)
{
	int x = 1234567;
	int *y;
	y = &div0_y;
	x = x / *y;
}

static void linux_crash_test(int sel)
{
	switch(sel)
	{
		case LINUX_ERR_FATAL:
			BUG_ON(1);
			break;
		case LINUX_WATCHDOG:
			diag_error_sw_watchdog();
			break;
		case LINUX_EXCP_SWI:
			break;
		case LINUX_EXCP_UNDEF:
			diag_error_undefined_instruction();
			break;
		case LINUX_EXCP_MIS_ALIGN:
			break;            
		case LINUX_EXCP_PAGE_FAULT:
			diag_error_data_abort();
			break;
		case LINUX_EXCP_EXE_FAULT:
			diag_error_prefetch_abort();
			break;
		case LINUX_EXCP_DIV_BY_Z:
			diag_error_div0();
			break;
		case LINUX_FS_CRASH:
			pantech_sys_reset_reason_set(SYS_RESET_REASON_USERDATA_FS);
			panic("EXT4-fs (device) ERRORS_RO : panic forced after FS crash TEST\n");
			break;
		default:
			break;
	}
}

static ssize_t ramdump_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	ramdump = (int)IS_SYS_RAMDUMP_MODE;
	printk(KERN_INFO "%s : , ramdump : %d\n", __func__, ramdump);
	return sprintf(buf, "%d\n", ramdump);
}

static ssize_t ramdump_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &ramdump);
	printk(KERN_INFO "%s : , ramdump : %d\n", __func__, ramdump);
	if(ramdump == 1)
		SET_SYS_RESET_REASON_MODE(SYS_RESET_RAMDUMP_MASK, 1);
	else
		SET_SYS_RESET_REASON_MODE(SYS_RESET_RAMDUMP_MASK, 0);
	return count;
}

static struct kobj_attribute ramdump_attribute =
	__ATTR(ramdump, 0664, ramdump_show, ramdump_store);

static ssize_t usbdump_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	usbdump = (int)IS_SYS_USBDUMP_MODE;
	printk(KERN_INFO "%s : , usbdump : %d\n", __func__, usbdump);
	return sprintf(buf, "%d\n", usbdump);
}

static ssize_t usbdump_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &usbdump);
	printk(KERN_INFO "%s : , usbdump : %d\n", __func__, usbdump);
	if(usbdump == 1)
		SET_SYS_RESET_REASON_MODE(SYS_RESET_USBDUMP_MASK, 1);
	else
		SET_SYS_RESET_REASON_MODE(SYS_RESET_USBDUMP_MASK, 0);
	return count;
}

static struct kobj_attribute usbdump_attribute =
	__ATTR(usbdump, 0664, usbdump_show, usbdump_store);

// p15060
// LOGSET_USERDUMP_DISABLE          -> 1
// LOGSET_USERDUMP_COREDUMP         -> 2
// LOGSET_USERDUMP_FRAME_COREDUMP   -> 3
// LOGSET_USERDUMP_USER_RAMDUMP     -> 4
// LOGSET_USERDUMP_FRAME_RAMDUMP    -> 5
static ssize_t userdump_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO "%s : , userdump : %d\n", __func__, userdump);
	return sprintf(buf, "%d\n", userdump);
}
static ssize_t userdump_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &userdump);
	printk(KERN_INFO "%s : , userdump : %d\n", __func__, userdump);
	return count;
}
static struct kobj_attribute userdump_attribute =
	__ATTR(userdump, 0664, userdump_show, userdump_store);

static ssize_t errortest_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", errortest);
}

static ssize_t errortest_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &errortest);
	printk(KERN_INFO "%s : , errortest : %d\n", __func__, errortest);
	linux_crash_test(errortest);
	return count;
}

static struct kobj_attribute errortest_attribute =
	__ATTR(errortest, 0664, errortest_show, errortest_store);

static ssize_t mdmdump_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	mdmdump = (int)GET_SYS_RESET_REASON_FLAG(SYS_RESET_MDM_DUMP_FLAG);
	printk(KERN_INFO "%s : , mdmdump : %d\n", __func__, mdmdump);
	return sprintf(buf, "%d\n", mdmdump);
}

static ssize_t mdmdump_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &mdmdump);
	printk(KERN_INFO "%s : , mdmdump : %d\n", __func__, mdmdump);
	if(mdmdump == 1)
		SET_SYS_RESET_REASON_FLAG(SYS_RESET_MDM_DUMP_FLAG, 1);
	else
		SET_SYS_RESET_REASON_FLAG(SYS_RESET_MDM_DUMP_FLAG, 0);
	return count;
}

static struct kobj_attribute mdmdump_attribute =
	__ATTR(mdmdump, 0664, mdmdump_show, mdmdump_store);

static ssize_t ssrmode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	ssrmode = (int)GET_SYS_RESET_REASON_FLAG(SYS_RESET_SSR_MODE_FLAG);
	printk(KERN_INFO "%s : , ssrmode : %d\n", __func__, ssrmode);
	return sprintf(buf, "%d\n", ssrmode);
}

static ssize_t ssrmode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int temp;
	sscanf(buf, "%du", &temp);

	printk(KERN_INFO "%s : , ssrmode : %d\n", __func__, ssrmode);
	if(ssrmode == 1)
		SET_SYS_RESET_REASON_FLAG(SYS_RESET_SSR_MODE_FLAG, 1);
	else
		SET_SYS_RESET_REASON_FLAG(SYS_RESET_SSR_MODE_FLAG, 0);
	return count;
}

static struct kobj_attribute ssrmode_attribute =
	__ATTR(ssrmode, 0664, ssrmode_show, ssrmode_store);

// (+) p15060
static ssize_t ssrdump_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    char temp[64];

    memset( temp, 0, sizeof(temp) );

    if( SSR_SET_MODEM_CHECK(ssrinfo) )
    {
        strcpy( temp, "modem_disable\n" );
    }
    else
    {
        strcpy( temp, "modem_enable\n" );
    }

    if( SSR_SET_MODEM_DUMP_CHECK(ssrinfo) )
    {
        strcat( temp, "modem_dump_enable" );
    }
    else
    {
        strcat( temp, "modem_dump_disable" );
    }

	printk(KERN_INFO "%s : %s\n", __func__, temp);
	return sprintf(buf, "%s\n", temp);
}

static ssize_t ssrdump_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk(KERN_INFO "%s : ssrdump : %s, count : %i\n", __func__, buf, count);

    if( !strncmp("modem_enable", buf, count-1) )
    {
        SSR_SET_MODEM_ENABLE( ssrinfo );
    }
    else if( !strncmp("modem_disable", buf, count-1) )
    {
        SSR_SET_MODEM_DISABLE( ssrinfo );
    }
    else if( !strncmp("modem_dump_enable", buf, count-1) )
    {
        SSR_SET_MODEM_DUMP_ENABLE( ssrinfo );
        modem_ssr_dump_enable = 1;
    }
    else if( !strncmp("modem_dump_disable", buf, count-1) )
    {
        SSR_SET_MODEM_DUMP_DISABLE( ssrinfo );
        modem_ssr_dump_enable = 0;
    }
    else
    {
        printk( KERN_ERR "%s : unknown ssr set: %s\n", __func__, buf);
    }

	return count;
}

static struct kobj_attribute ssrdump_attribute =
	__ATTR(ssrdump, 0664, ssrdump_show, ssrdump_store);
// (-) p15060

static struct attribute *attrs[] = {
	&ramdump_attribute.attr,
	&usbdump_attribute.attr,
	&userdump_attribute.attr, // p15060
	&errortest_attribute.attr,
	&mdmdump_attribute.attr,
	&ssrmode_attribute.attr,
    &ssrdump_attribute.attr, // p15060
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *pantech_restart_kobj;

int __init pantech_restart_init(void)
{
	int retval;

	pantech_restart_kobj = kobject_create_and_add("pantech_restart", kernel_kobj);
	if (!pantech_restart_kobj)
		return -ENOMEM;
	retval = sysfs_create_group(pantech_restart_kobj, &attr_group);
	if (retval)
		kobject_put(pantech_restart_kobj);

	atomic_notifier_chain_register(&panic_notifier_list, &pantech_restart_blk);

    //set sys file by mode
	ramdump = (int)IS_SYS_RAMDUMP_MODE;
	usbdump = (int)IS_SYS_USBDUMP_MODE;
	mdmdump = (int)GET_SYS_RESET_REASON_FLAG(SYS_RESET_MDM_DUMP_FLAG);
	ssrmode = (int)GET_SYS_RESET_REASON_FLAG(SYS_RESET_MDM_DUMP_FLAG);

// p15060
    memset( backup_buffer, 0, sizeof(backup_buffer) );
    ssrinfo = (sky_ssr_info_type *)backup_buffer;

	return retval;
}

arch_initcall(pantech_restart_init);

MODULE_AUTHOR("Pantech LS1");
MODULE_DESCRIPTION("Pantech Restart driver");
MODULE_LICENSE("GPL v2");
