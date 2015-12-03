/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <mach/gpio.h>

#include "../staging/android/timed_output.h"

#define DRV2603_NAME "drv2603"
#define DRV2603_MAX_LEVEL 50
#define DRV2603_DUTY 100 //100us 10kHz
#define DBG_ENABLE
#ifdef DBG_ENABLE
#define dbg(fmt, args...)   pr_debug("[VIB] " fmt, ##args)
#else
#define dbg(fmt, args...)
#endif
#define dbg_func_in()       dbg("[+] %s\n", __func__)
#define dbg_func_out()      dbg("[-] %s\n", __func__)
#define dbg_line()          dbg("[LINE] %d(%s)\n", __LINE__, __func__)

struct drv2603_gpio{
    int pwm;
    int en;
};
struct drv2603_pwm{
    int high;
    int low;
};
struct drv2603_data{
    struct platform_device *pdev;
    struct hrtimer vib_timer;
    struct timed_output_dev timed_dev;
    struct work_struct work;

    int level;
    int timeout;
    struct drv2603_pwm pwm;
    struct drv2603_gpio gpios;
    struct mutex lock;
    struct task_struct *thread_id;
};
static struct drv2603_data *drv2603_data_static = NULL;

static int vib_thread_func(void *arg);
static void drv2603_set_pwm(struct drv2603_data *vib , int level);
static int vibrator_init(struct drv2603_data *vib);
static void vib_disable(struct drv2603_data *vib);
static void vib_enable(struct drv2603_data *vib);
static enum hrtimer_restart drv2603_vib_timer_func(struct hrtimer *timer);
static int drv2603_vib_get_time(struct timed_output_dev *dev);
static void drv2603_vib_enable(struct timed_output_dev *dev, int value);
static void drv2603_vib_update(struct work_struct *work);
static void drv2603_vib_set(struct drv2603_data *vib, int level);


static int vibrator_init(struct drv2603_data *vib){
    int rc;
    unsigned gpioConfig;

    dbg_func_in();
    gpio_request(vib->gpios.en , "vib_en");
    gpioConfig = GPIO_CFG(vib->gpios.en , 0 , GPIO_CFG_OUTPUT , GPIO_CFG_PULL_DOWN , GPIO_CFG_2MA);
    rc = gpio_tlmm_config(gpioConfig , GPIO_CFG_ENABLE);
    if(rc){
        pr_err("setting enable gpio failed\n");
        return -EINVAL;
    }
    gpio_request(vib->gpios.pwm , "vib_pwm");
    gpioConfig = GPIO_CFG(vib->gpios.pwm , 0 , GPIO_CFG_OUTPUT , GPIO_CFG_PULL_DOWN , GPIO_CFG_2MA);
    rc = gpio_tlmm_config(gpioConfig , GPIO_CFG_ENABLE);
    if(rc){
        pr_err("setting pwm gpio failed\n");
        return -EINVAL;
    }
    vib->pwm.high = DRV2603_DUTY;
    vib->pwm.low  = 0;
    dbg_func_out();
    return 0;
}
static void drv2603_set_pwm(struct drv2603_data *vib , int level){
    vib->pwm.high = (DRV2603_DUTY/2) + level ;
    vib->pwm.low  = DRV2603_DUTY - vib->pwm.high ;
    dbg("set_pwm, pwm.high : %d , pwm.low : %d\n" , vib->pwm.high , vib->pwm.low);
}
static int vib_thread_func(void *arg){
    while(!kthread_should_stop()){
        gpio_set_value(drv2603_data_static->gpios.pwm , 1);
        udelay(drv2603_data_static->pwm.high);
        gpio_set_value(drv2603_data_static->gpios.pwm , 0);
        udelay(drv2603_data_static->pwm.low);
    }
    return 0;
}
static void vib_enable(struct drv2603_data *vib){
    mutex_lock(&vib->lock);
    if(vib->thread_id == NULL){
        dbg("call vib_enable\n");
        vib->thread_id = (struct task_struct *)kthread_run(vib_thread_func , NULL , "vibPWM");
        gpio_set_value(vib->gpios.en , 1);
    }
	mutex_unlock(&vib->lock);
}

static void vib_disable(struct drv2603_data *vib){
    mutex_lock(&vib->lock);
    if(vib->thread_id != NULL){
        dbg("call vib_disable\n");
        kthread_stop(vib->thread_id);
        vib->thread_id = NULL;
        gpio_set_value(vib->gpios.en , 0);
        gpio_set_value(vib->gpios.pwm , 0);
    }
	mutex_unlock(&vib->lock);
}
static void drv2603_vib_set(struct drv2603_data *vib, int level)
{
    drv2603_set_pwm(vib ,level);
    if (level)
        vib_enable(vib);
    else
        vib_disable(vib);
}

static void drv2603_vib_update(struct work_struct *work)
{
	struct drv2603_data *vib = container_of(work, struct drv2603_data, work);
    dbg("work is scheduled\n");
	drv2603_vib_set(vib, vib->level);
}
static void drv2603_vib_enable(struct timed_output_dev *dev, int value)
{
	struct drv2603_data *vib = container_of(dev, struct drv2603_data, timed_dev);
    
    mutex_lock(&vib->lock);
    vib->level = ((value & 0xFFFF0000) >> 16);
    vib->timeout = (value & 0x0000FFFF);
    dbg("vib_level : %d , vib_timeout : %d\n" , vib->level , vib->timeout);

	hrtimer_cancel(&vib->vib_timer);
    
    if(vib->level <= 0)
        vib->level = 0;
    else{
        if(vib->level > DRV2603_MAX_LEVEL)
            vib->level = DRV2603_MAX_LEVEL;
        hrtimer_start(&vib->vib_timer, ktime_set(vib->timeout / 1000 , (vib->timeout % 1000) * 1000000), HRTIMER_MODE_REL);
    }
	mutex_unlock(&vib->lock);
	schedule_work(&vib->work);
}
static int drv2603_vib_get_time(struct timed_output_dev *dev){
	struct drv2603_data *vib = container_of(dev, struct drv2603_data,
							 timed_dev);

	if (hrtimer_active(&vib->vib_timer)) {
		ktime_t r = hrtimer_get_remaining(&vib->vib_timer);
		return (int)ktime_to_us(r);
	} else
		return 0;
}
static enum hrtimer_restart drv2603_vib_timer_func(struct hrtimer *timer){
	struct drv2603_data *vib = container_of(timer, struct drv2603_data,
							 vib_timer);
    dbg("hrtimer expired!\n");
	vib->level = 0;
	schedule_work(&vib->work);

	return HRTIMER_NORESTART;
}
static int __devinit drv2603_probe(struct platform_device *pdev){
    struct drv2603_data *drv2603_data_t = NULL;
    struct device *dev = &pdev->dev;
    struct device_node *node , *pp = NULL;
    const char *label;
    int rc;
    u32 reg;

    dbg_func_in();
    drv2603_data_t = kzalloc(sizeof(struct drv2603_data) , GFP_KERNEL);
    if(drv2603_data_t == NULL){
        pr_err("Error , Fail to allocate mem\n");
        return -ENOMEM;
    }
    drv2603_data_t->pdev = pdev;
    node = dev->of_node;

    drv2603_data_t->thread_id = NULL;
    
    if(node == NULL)
        return -ENODEV;

    drv2603_data_t->gpios.pwm = -1;  
    drv2603_data_t->gpios.en = -1;
   
    while( (pp=of_get_next_child(node,pp))){
        if(!of_find_property(pp , "gpios" , NULL)){
            continue;
        }
        label = of_get_property(pp,"label",NULL);
        if(strncmp(label , "drv2603_pwm" , 11) == 0){
            if(of_property_read_u32(pp , "gpios" , &reg)){
                pr_err("Can not find pwm gpio description\n");
                return -EINVAL;
            }
            drv2603_data_t->gpios.pwm = reg;
            dbg("label : %s , gpios : %d\n" , label , drv2603_data_t->gpios.pwm);
        }
        else if(strncmp(label , "drv2603_en" , 10) == 0){
            if(of_property_read_u32(pp , "gpios" , &reg)){
                pr_err("Can not find enable gpio description\n");
                return -EINVAL;
            }
            drv2603_data_t->gpios.en = reg;
            dbg("label : %s , gpios : %d\n" , label , drv2603_data_t->gpios.en);
        }
    }
    rc = vibrator_init(drv2603_data_t);
    if(rc){
        pr_err("Error, vibrator initailization\n");
        return -EINVAL;
    }

    mutex_init(&drv2603_data_t->lock);
    INIT_WORK(&drv2603_data_t->work , drv2603_vib_update);
   
    hrtimer_init(&drv2603_data_t->vib_timer , CLOCK_MONOTONIC , HRTIMER_MODE_REL);
    drv2603_data_t->vib_timer.function = drv2603_vib_timer_func;

    drv2603_data_t->timed_dev.name = "vibrator";
    drv2603_data_t->timed_dev.get_time = drv2603_vib_get_time;
    drv2603_data_t->timed_dev.enable = drv2603_vib_enable;

    dev_set_drvdata(&pdev->dev , drv2603_data_t);

    rc = timed_output_dev_register(&drv2603_data_t->timed_dev);
    if(rc){
        pr_err("Error, timed_output_dev_register fail\n");
        return -ENODEV;
    }
    drv2603_data_static = drv2603_data_t;
    dbg_func_out();
    return 0;
}
static int __devexit drv2603_remove(struct platform_device *pdev){
    struct drv2603_data *ddata = dev_get_drvdata(&pdev->dev);

	cancel_work_sync(&ddata->work);
	hrtimer_cancel(&ddata->vib_timer);
	timed_output_dev_unregister(&ddata->timed_dev);
	mutex_destroy(&ddata->lock);

	return 0;
}
static struct of_device_id drv2603_of_match[] = {
    { .compatible = DRV2603_NAME ,},
    { },
};
static struct platform_driver drv2603_driver = {
    .probe = drv2603_probe,
    .driver = {
        .name = DRV2603_NAME,
        .owner = THIS_MODULE,
        .of_match_table = drv2603_of_match,
    },
    .remove = __devexit_p(drv2603_remove),
};
static int __init drv2603_init(void){
    dbg("drv2603_init\n");
    return platform_driver_register(&drv2603_driver);
}
static void __exit drv2603_exit(void){
    dbg("drv2603_exit\n");
    platform_driver_unregister(&drv2603_driver);
}

module_init(drv2603_init);
module_exit(drv2603_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("TI DRV2603 chip driver");
