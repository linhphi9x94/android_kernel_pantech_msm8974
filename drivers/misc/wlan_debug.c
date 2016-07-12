/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#include <linux/ratelimit.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/uaccess.h>

#ifdef CONFIG_PANTECH_WLAN_DEBUG
#include <linux/kernel.h> // sscanf, sprintf
#endif

#define DEVICE "wlan_debug"
#define VERSION "1.00"

/* module params */

// 20130725 lcj@ls3
#ifdef CONFIG_PANTECH_WLAN_DEBUG
struct trace_entry {
	const char* module;
	int 	module_id;
	int 	level;
	int 	enable;
};
static struct trace_entry TRACE_CONFIGS[] = {
	{"BAP", 0, 0, 0},
	{"TL", 1, 0, 0},
	{"WDI", 2,  0, 0},
	{"", 3, 0, 0},
	{"", 4, 0, 0},
	{"HDD", 5,  0, 0},
	{"SME", 6, 0, 0},
	{"PE", 7, 0, 0},
	{"WDA", 8, 0, 0},
	{"SYS", 9, 0, 0},
	{"VOSS", 10, 0, 0},
	{"SAP", 11, 0, 0},
	{"HDD_SOFTAP", 12, 0, 0},
};

static int debug_trace=0;
static int param_set_trace(const char *val, const struct kernel_param *kp);
static int param_get_trace(char *val, const struct kernel_param *kp);

static const struct kernel_param_ops params_ops_trace = {
	.set = &param_set_trace,
	.get = &param_get_trace,
};
// 0666 -->  error: negative width in bit-field '<anonymous>'
// build error, other can't be 6 but 4
module_param_cb(debug_trace, &params_ops_trace, &debug_trace, 0664);
MODULE_PARM_DESC(debug_trace, "debug trace for wlan");

static int param_set_trace(const char *val, const struct kernel_param *kp) 
{
	int module_id=0;
	int level=0;
	int enable=0;
	int a =0;

	pr_info("%s(%d):[WIFI] val=%s\n", __func__, __LINE__, val) ;
	sscanf(val, "%d %d %d", &module_id, &level, &enable) ;
	
	for(a=0; a<sizeof(TRACE_CONFIGS)/sizeof(TRACE_CONFIGS[0]); ++a) {
		if( TRACE_CONFIGS[a].module_id == module_id ) {
			TRACE_CONFIGS[a].level = level ;
			TRACE_CONFIGS[a].enable= enable;
			pr_info("%s(%d):[WIFI] %s(%d),level=%d,enable=%d\n", __func__, __LINE__, 
				TRACE_CONFIGS[a].module, 
				module_id, level, enable);
			break ;
		}
	}
	// no error
	return 0;	
}

static int param_get_trace(char *val, const struct kernel_param *kp)
{
	char line[64];
	int a=0;
	int result=0;
	for(a=0; a<sizeof(TRACE_CONFIGS)/sizeof(TRACE_CONFIGS[0]); ++a) {
		if( a != 0 ) {
			strcat(val, "\n") ;	
			result++ ;
		}
		result+= sprintf(line, "%s %d %d %d", 
			TRACE_CONFIGS[a].module, 
			TRACE_CONFIGS[a].module_id, 
			TRACE_CONFIGS[a].level,
			TRACE_CONFIGS[a].enable) ;
		strcat(val, line) ;	
	}
	return result ;
}


/*
 * \brief  call cb on all entries of TRACE_CONFIGS 
 * \param - cb - a callback function, cb( module_name, module_id, level, enable)
 * \param - user - user data
 * \return none
 */
void wlan_debug_enum_trace( void (*cb)(const char*,int, int,int,void*), void* user) 
{ 
	int a=0;
	if(cb) {
		for( a=0; a < sizeof(TRACE_CONFIGS)/sizeof(TRACE_CONFIGS[0]); ++a) {
			cb( TRACE_CONFIGS[a].module, 
				TRACE_CONFIGS[a].module_id,
				TRACE_CONFIGS[a].level,
				TRACE_CONFIGS[a].enable, 
				user) ;
		}
	}
}
EXPORT_SYMBOL(wlan_debug_enum_trace);
#endif // CONFIG_PANTECH_WLAN_DEBUG


static int __devinit
wlan_debug_probe(struct platform_device *pdev)
{
	int ret = 0;
	pr_info("%s(%d):[WIFI]\n", __func__, __LINE__);
	return ret ;
}

static int __devexit
wlan_debug_remove(struct platform_device *pdev)
{
	pr_info("%s(%d):[WIFI]\n", __func__, __LINE__);
	return 0;
}


static struct platform_driver wlan_debug_driver = {
	.driver = {
		.name   = DEVICE,
		.owner  = THIS_MODULE,
	},
	.probe  = wlan_debug_probe,
	.remove = __devexit_p(wlan_debug_remove),
};

static int __init wlan_debug_init(void)
{
	int ret = 0;
	pr_info("%s(%d):[WIFI]\n", __func__, __LINE__);
	platform_driver_register(&wlan_debug_driver);

	return ret;
}

static void __exit wlan_debug_exit(void)
{
	pr_info("%s(%d):[WIFI]\n", __func__, __LINE__);
	platform_driver_unregister(&wlan_debug_driver);
}

module_init(wlan_debug_init);
module_exit(wlan_debug_exit);


MODULE_LICENSE("GPL v2");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION(DEVICE "Driver");
