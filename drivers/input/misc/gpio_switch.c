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
#include <linux/gpio_event.h>
#include <linux/switch.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <asm/mach-types.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <mach/gpio.h>
#include <linux/gpio_switch.h>
#include "linux/spi/fpc1080.h"
#include <linux/miscdevice.h>

#define DBG_ENABLE
#ifdef DBG_ENABLE
#define dbg(fmt, args...)   pr_debug("[gpio_switch] " fmt, ##args)
#else
#define dbg(fmt, args...)
#endif
#define dbg_func_in()       dbg("[+] %s\n", __func__)
#define dbg_func_out()      dbg("[-] %s\n", __func__)
#define dbg_line()          dbg("[LINE] %d(%s)\n", __LINE__, __func__)

#define GPIO_SWITCH_IOCTL_SET_BUMBPER_CASE_DEBOUNCE_TIME 0    // To set debounce time by user space for Hall IC function test.
#define GPIO_SWITCH_IOCTL_SET_COVER_ENABLE 10    // To set cover irq enable
#define GPIO_SWITCH_IOCTL_SET_COVER_DISABLE 11    // To set cover irq disable

struct gpio_switch_data *head = NULL;
struct gpio_switch_data *tail = NULL;

void register_notify_func(int mode , const char* name , void (*noti_func)(const int)){
    struct gpio_switch_data *p = head;
    while(p){
        if(strcmp(name , p->name)==0){
            if(mode == EMERGENCY_MODE){
                p->emergency_notify = noti_func;
                pr_info("%s added emergency_notify func with mode : %d\n" , name , mode);
            }
            else if(mode == NORMAL_MODE){
                p->notify = noti_func;
                pr_info("%s added normal_notify func with mode : %d\n" , name , mode);
            }
            break;
        }
        p = p->next;
    }

}
EXPORT_SYMBOL(register_notify_func);

static void gpio_switch_work(struct work_struct *work){
	struct gpio_switch_data *p = container_of(work, struct gpio_switch_data, work);

    if(p->current_state == p->last_state){
        dbg("current_state is last_state, so framework does not need it\n");
    }
    else{
        if(p->event_mode == PAN_INPUTEVENT){
            input_report_switch(p->input_dev , SW_LID , p->current_state);
            input_sync(p->input_dev);
        }
        else if(p->event_mode == PAN_UEVENT){
            switch_set_state(&p->sdev , p->current_state);
            wake_lock_timeout(&p->wakelock,msecs_to_jiffies(GPIO_SWITCH_WAKE_LOCK_TIMEOUT));
        }
        p->last_state = p->current_state; 
        dbg("data(%d) is sent by %s\n" , p->current_state , p->name);
        if(p->notify){
            p->notify(p->current_state);
            dbg("%s->notify call! sent data(%d)\n" , p->name , p->current_state);
        }
        else{
            dbg("%s->notify did not register!\n" , p->name);
        }
    }
}
static irqreturn_t gpio_switch_irq_handler(int irq, void *dev){
    struct gpio_switch_data *p = head;
   
    while(p){
        if(p->irq == irq) break;
        p = p->next;
    }
    if(p->emergency_notify){
        p->current_state = gpio_get_value(p->gpio) ^ p->flag;
        p->emergency_notify(p->current_state);
        dbg("%s->emergency_notify call! sent data(%d)\n" , p->name , p->current_state);
    }
    else
        dbg("%s->emergency_notify did not register!\n" , p->name);

    if(p->debounce & DEBOUNCE_WAIT_IRQ){
        if(p->debounce_count++ == 0){
            p->debounce = DEBOUNCE_UNSTABLE;
            dbg("hrtimer in ISR\n");
            hrtimer_start(&p->timer , p->debounce_time , HRTIMER_MODE_REL);
        }
    }
    return IRQ_HANDLED;
}
static enum hrtimer_restart gpio_switch_timer_func(struct hrtimer *timer){
    struct gpio_switch_data *p = container_of(timer , struct gpio_switch_data, timer);
    bool pressed;

    if(p->debounce & DEBOUNCE_UNSTABLE){
        dbg("STATE DEBOUNCE_UNSTABLE\n");
        p->debounce = DEBOUNCE_UNKNOWN;
    }

    p->current_state = gpio_get_value(p->gpio) ^ p->flag;
    pressed = (p->current_state == 1) ? false : true;
    if(pressed && (p->debounce & DEBOUNCE_NOTPRESSED)){
        dbg("STATE DEBOUNCE_PRESSED\n");
        p->debounce = DEBOUNCE_PRESSED;
        goto check_again;
    }
    if(!pressed && (p->debounce & DEBOUNCE_PRESSED)){
        dbg("STATE DEBOUNCE_NOTPRESSED\n");
        p->debounce = DEBOUNCE_NOTPRESSED;
        goto check_again;
    }
    p->debounce_count--;
    p->debounce |= DEBOUNCE_WAIT_IRQ;
	schedule_work(&p->work);

check_again:
    if(p->debounce_count){
        hrtimer_start(&p->timer , p->debounce_time , HRTIMER_MODE_REL);
    }
    return HRTIMER_NORESTART;
}
static ssize_t gpio_switch_print_state(struct switch_dev *sdev, char *buf){
    const char *state;

    if (switch_get_state(sdev))
        state = "OFF";
	else
		state ="ON";

	if (state)
		return sprintf(buf, "%s\n", state);

	return -1;
}
static int init_gpios(void){
    int ret;
    struct gpio_switch_data *p = head;
    while(p){
	    ret = gpio_tlmm_config(GPIO_CFG(p->gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	    if (ret){
	    	pr_err("Could not configure gpio %d\n", p->gpio);
            return -EINVAL;
        }
        gpio_request(p->gpio , p->name);

        if(p->event_mode == PAN_INPUTEVENT){
            p->input_dev = input_allocate_device();
            if(p->input_dev == NULL){
                pr_err("switch_gpio failed to allocate input device\n");
                return -ENOMEM;
            }
            p->input_dev->name = p->name;
	        set_bit(EV_SW, p->input_dev->evbit);
	        set_bit(SW_LID, p->input_dev->swbit);
            ret = input_register_device(p->input_dev);
            if(ret){
                pr_err("switch_gpio unable to register %s input device\n", p->input_dev->name);
                return -EINVAL;
            }
        }
        else if(p->event_mode == PAN_UEVENT){
            p->sdev.print_state = gpio_switch_print_state;
            p->sdev.name = p->name;
            ret = switch_dev_register(&p->sdev);
            if(ret < 0){
                pr_err("%s Switch dev register is failed\n" , p->sdev.name);
                return -EINVAL;
            }
        }
        else{
            pr_err("event_mode : %d does not exist\n" , p->event_mode); 
            return -EINVAL;
        }
        p->current_state = gpio_get_value(p->gpio) ^ p->flag;

        if(p->event_mode == PAN_INPUTEVENT){
            dbg("INPUTEVENT %s sent event : %d\n" , p->name , p->current_state);
            input_report_switch(p->input_dev , SW_LID , p->current_state);
            input_sync(p->input_dev);
        }
        else if(p->event_mode == PAN_UEVENT){
            dbg("UEVENT %s sent event : %d\n" , p->name , p->current_state);
            switch_set_state(&p->sdev , p->current_state);
        }
        p->last_state = p->current_state;

        ret = request_threaded_irq(p->irq , NULL , gpio_switch_irq_handler , IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING , p->name , p);
        if(ret){
            pr_err("%s request_irq is failed reason : %d\n" , p->name , ret);
            return -EINVAL;
        }
        
        hrtimer_init(&p->timer , CLOCK_MONOTONIC , HRTIMER_MODE_REL);
        p->timer.function = gpio_switch_timer_func;
        p->debounce = DEBOUNCE_UNKNOWN | DEBOUNCE_WAIT_IRQ;
        p->debounce_count = 0;

        p->notify = NULL;
        p->emergency_notify = NULL;

        INIT_WORK(&p->work , gpio_switch_work);

        wake_lock_init(&p->wakelock, WAKE_LOCK_SUSPEND, p->name);
        enable_irq_wake(p->irq);

        p->disabled = false;
        
        p = p->next;
    }
    return 0;
}

static void gpio_switch_enable_irq (struct gpio_switch_data *p)
{
    dbg("%s : enter\n", __func__);
    
    if(p->disabled) {
        if(p->irq) {
            pr_info("%s : enable %s irq\n", __func__, p->name);
            enable_irq_wake(p->irq);
            enable_irq(p->irq);
            p->disabled = false;
        }
    }
}

static void gpio_switch_disable_irq(struct gpio_switch_data *p)
{
    dbg("%s : enter\n", __func__);

    if (!p->disabled) {
        if(p->irq) {
            pr_info("%s : disable %s irq\n", __func__, p->name);
            disable_irq_wake(p->irq);
            disable_irq(p->irq);
            p->disabled = true;
       }
    }
}

static ssize_t gpio_switch_write(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    int nBufSize = 0;
    struct gpio_switch_data *p = head;

    if((size_t)(*ppos) > 0)
        return 0;

    if(buf != NULL) {
        nBufSize = strlen(buf);
        
        if(strncmp(buf, "test",4) == 0) {
            dbg("%s : test\n", __func__);
        }
        else if (strncmp(buf, "cover_enable",12) == 0) {
            dbg("%s : cover_enable\n", __func__);
            while(p) {
                if(strcmp(p->name , "smart_cover") == 0) {
                    gpio_switch_enable_irq(p);
                }
                p = p->next;
            }
        }
        else if (strncmp(buf, "cover_disable",13) == 0) {
            dbg("%s : cover_disable\n", __func__);
            while(p) {
                if(strcmp(p->name , "smart_cover") == 0) {
                    gpio_switch_disable_irq(p);
                }
                p = p->next;
            }
        }
        else {
            pr_err("%s : unexpected argument\n", __func__);
        }
    }

    return nBufSize;
}

static long gpio_switch_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    struct gpio_switch_data *p = head;
    
    switch (cmd) {
        case GPIO_SWITCH_IOCTL_SET_BUMBPER_CASE_DEBOUNCE_TIME:
            dbg("%s : called ioctl GPIO_SWITCH_IOCTL_SET_BUMBPER_CASE_DEBOUNCE_TIME(%d)\n", __func__, GPIO_SWITCH_IOCTL_SET_BUMBPER_CASE_DEBOUNCE_TIME);
            while(p) {
                if(strcmp(p->name , "sub_smart_cover1_detection") == 0) {
                    pr_info("%s : set %s debounce time -> %dms\n", __func__, p->name, (int)arg);
                    p->debounce_time = ktime_set(0, arg * 1000000);
                }
                p = p->next;
            }
            break;
        case GPIO_SWITCH_IOCTL_SET_COVER_ENABLE:
            dbg("%s : called ioctl GPIO_SWITCH_IOCTL_SET_COVER_ENABLE(%d)\n", __func__, GPIO_SWITCH_IOCTL_SET_COVER_ENABLE);
            while(p) {
                if(strcmp(p->name , "smart_cover") == 0) {
                    gpio_switch_enable_irq(p);
                }
                p = p->next;
            }
            break;
        case GPIO_SWITCH_IOCTL_SET_COVER_DISABLE:
            dbg("%s : called ioctl GPIO_SWITCH_IOCTL_SET_COVER_DISABLE(%d)\n", __func__, GPIO_SWITCH_IOCTL_SET_COVER_DISABLE);
            while(p) {
                if(strcmp(p->name , "smart_cover") == 0) {
                    gpio_switch_disable_irq(p);
                    // Force report cover open event to prevent cover state is closed after disable cover irq
                    msleep(20);
                    input_report_switch(p->input_dev , SW_LID , 0);
                    input_sync(p->input_dev);
                    p->last_state = 0;
                }
                p = p->next;
            }
            break;
        default:
            printk("%s : unexpected command : %d\n", __func__, cmd);
            break;
	}
	
	return 0;
}

static struct file_operations gpio_switch_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = gpio_switch_ioctl,
	.write          = gpio_switch_write,
};

static struct miscdevice gpio_switch_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gpio_switch_dev",
	.fops = &gpio_switch_fops,
};
static int gpio_switch_miscdev_registered = 0;

static int __devinit gpio_switch_probe(struct platform_device *pdev){
    struct device *dev = &pdev->dev;
    struct device_node *node , *pp = NULL;
    struct gpio_switch_data *sdata;
    u32 reg;
    int ret;

    dbg("switch_gpio probe!\n");

    node = dev->of_node;
    if(node == NULL){
        pr_err("node is null\n");
        return -ENODEV;
    }
    while( (pp=of_get_next_child(node,pp))){
        sdata = kzalloc(sizeof(struct gpio_switch_data) , GFP_KERNEL);
        if(!sdata){
            pr_err("sdata memory alloc is failed\n");
            return -ENOMEM;
        }
        if(!of_find_property(pp , "gpios" , NULL)){
            continue;
        }
        sdata->name = of_get_property(pp,"label",NULL);

        ret = of_property_read_u32(pp , "event_mode" , &reg);
        if(ret < 0 )
            goto err_free_mem;
        
        sdata->event_mode = reg;

        ret = of_property_read_u32(pp , "flag" , &reg);
        if(ret < 0 )
            goto err_free_mem;
        
        sdata->flag = reg;
        
        ret = of_property_read_u32(pp , "gpios" , &reg);
        if(ret < 0 )
            goto err_free_mem;
        
        sdata->gpio = reg;
        sdata->irq = gpio_to_irq(reg);

        ret = of_property_read_u32(pp , "debounce_time" , &reg);
        if(ret < 0)
            goto err_free_mem;

        sdata->debounce_time = ktime_set(0, reg * 1000000);

        pr_info("label : %s , gpios : %d , irq : %d , event_mode : %d , flag = %d\n" , sdata->name , sdata->gpio , sdata->irq , sdata->event_mode , sdata->flag);

        sdata->next = NULL;
        if(head == NULL){
            head = sdata;
            tail = sdata;
        }else{
            tail->next = sdata;
            tail = sdata;
        }
    }
    ret = init_gpios();
    if(ret < 0){
        pr_err("init gpios is failed with error : %d\n" , ret);
        return ret;
    }

    ret = misc_register(&gpio_switch_miscdev);
    if (ret)
        printk("could not register gpio switch miscdevice, error: %d\n", ret);
    else
        gpio_switch_miscdev_registered = 1;
    
    dbg("switch_gpio probe end!\n");
    return 0;
err_free_mem:
    kfree(sdata);
    return ret;
}
static struct of_device_id gpio_switch_of_match[] = {
    { .compatible = GPIO_SWITCH_NAME ,},
    { },
};
static struct platform_driver gpio_switch_driver = {
    .probe = gpio_switch_probe,
    .driver = {
        .name = GPIO_SWITCH_NAME,
        .owner = THIS_MODULE,
        .of_match_table = gpio_switch_of_match,
    },
};
static int __init gpio_switch_init(void){
    pr_info("gpio_switch_init\n");
    return platform_driver_register(&gpio_switch_driver);
}
static void __exit gpio_switch_exit(void){
    struct gpio_switch_data *p = head;
    pr_info("gpio_switch_exit\n");
    
    if (gpio_switch_miscdev_registered)
        misc_deregister(&gpio_switch_miscdev);

    platform_driver_unregister(&gpio_switch_driver);
    while(p){
        switch_dev_unregister(&p->sdev);
        hrtimer_cancel(&p->timer);
        cancel_work_sync(&p->work);
        wake_lock_destroy(&p->wakelock);
        p = p->next;
    }
}
rootfs_initcall(gpio_switch_init);
module_exit(gpio_switch_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HALL IC drivers");
