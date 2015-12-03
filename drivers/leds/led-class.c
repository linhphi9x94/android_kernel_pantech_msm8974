/*
 * LED Class Core
 *
 * Copyright (C) 2005 John Lenz <lenz@cs.wisc.edu>
 * Copyright (C) 2005-2007 Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include "leds.h"

#define LED_BUFF_SIZE 50

static struct class *leds_class;

//++ p11309 - 2013.08.05 for ON Sync LCD & LED
#if defined(CONFIG_MACH_MSM8974_EF56S)
static int lcd_backlight_state = 0;
static int gpled_0_level = 0;
static int gpled_1_level = 0;
static int gpled_2_level = 0;
static int gpled_3_level = 0;
static struct led_classdev *gpled_0;
static struct led_classdev *gpled_1;
static struct led_classdev *gpled_2;
static struct led_classdev *gpled_3;
#elif defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
static int lcd_backlight_state = 0;
static int lp5523_value = 0;
static int lp5523_menu_back_state = 0;
static struct led_classdev *lp5523_control;
#elif defined(CONFIG_MACH_MSM8974_EF65S)
static int lcd_backlight_state = 0;
static struct led_classdev *kpdbl_menu;
static struct led_classdev *kpdbl_back;
static int kpdbl_menu_level = 0;
static int kpdbl_back_level = 0;
#endif
//-- p11309

static void led_update_brightness(struct led_classdev *led_cdev)
{
    if (led_cdev->brightness_get)
        led_cdev->brightness = led_cdev->brightness_get(led_cdev);
}

static ssize_t led_brightness_show(struct device *dev, 
                                   struct device_attribute *attr, char *buf)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);

    /* no lock needed for this */
    led_update_brightness(led_cdev);

    return snprintf(buf, LED_BUFF_SIZE, "%u\n", led_cdev->brightness);
}

static ssize_t led_brightness_store(struct device *dev,
                                    struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    ssize_t ret = -EINVAL;
    char *after;
    unsigned long state = simple_strtoul(buf, &after, 10);
    size_t count = after - buf;

    if (isspace(*after))
        count++;

    if (count == size) {
        ret = count;

        if (state == LED_OFF)
            led_trigger_remove(led_cdev);

        //++ p11309 - 2013.08.05 for ON Sync LCD & LED
#if defined(CONFIG_MACH_MSM8974_EF56S)

        //		led_set_brightness(led_cdev, state);

        if ( strcmp(led_cdev->name, "lcd-backlight") == 0 ) {

            led_set_brightness(led_cdev, state);

            if ( state > 0 ) {
                lcd_backlight_state = 1;				

                led_set_brightness(gpled_0, gpled_0_level);
                led_set_brightness(gpled_1, gpled_1_level);
                led_set_brightness(gpled_2, gpled_2_level);
                led_set_brightness(gpled_3, gpled_3_level);
            }
            else {
                lcd_backlight_state = 0;
                gpled_0_level = 0; led_set_brightness(gpled_0, gpled_0_level);
                gpled_1_level = 0; led_set_brightness(gpled_1, gpled_1_level);
                gpled_2_level = 0; led_set_brightness(gpled_2, gpled_2_level);
                gpled_3_level = 0; led_set_brightness(gpled_3, gpled_3_level);
            }			
        }
        else if ( strcmp(led_cdev->name, "gpled_0") == 0 ) {
            gpled_0_level = state;
            if ( lcd_backlight_state == 0 ) {
                if ( state == 0 ) led_set_brightness(led_cdev, state);			
            }
            else {			
                led_set_brightness(led_cdev, state);
            }
        }
        else if ( strcmp(led_cdev->name, "gpled_1") == 0 ) {
            gpled_1_level = state;
            if ( lcd_backlight_state == 0 ) {
                if ( state == 0 ) led_set_brightness(led_cdev, state);			
            }
            else {			
                led_set_brightness(led_cdev, state);
            }
        }
        else if ( strcmp(led_cdev->name, "gpled_2") == 0 ) {
            gpled_2_level = state;
            if ( lcd_backlight_state == 0 ) {
                if ( state == 0 ) led_set_brightness(led_cdev, state);			
            }
            else {			
                led_set_brightness(led_cdev, state);
            }
        }
        else if ( strcmp(led_cdev->name, "gpled_3") == 0 ) {
            gpled_3_level = state;
            if ( lcd_backlight_state == 0 ) {
                if ( state == 0 ) led_set_brightness(led_cdev, state);			
            }
            else {			
                led_set_brightness(led_cdev, state);
            }
        }	
        else {
            led_set_brightness(led_cdev, state);
        }
#elif defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) 
        if ( strcmp(led_cdev->name, "lcd-backlight") == 0 ) {			
            led_set_brightness(led_cdev, state);

            if ( state > 0 ) {
                lcd_backlight_state = 1;
                if(lp5523_value == 1){
                    led_set_brightness(lp5523_control, lp5523_value);
                    //printk("[LED_TEST] lcd-backlight state -> %d\n",(int)state);
                    lp5523_value = 0;
                }
            }
            else {
                //printk("[LED_TEST] lcd-backlight state is 0\n");
                lcd_backlight_state = 0;
                lp5523_value = 0;
            }			
        }else if(strcmp(led_cdev->name, lp5523_control->name) == 0){
            //printk("[LED_TEST] led_cdev->name : %s, cnt ->%d, lp5523_control->name : %s, cnt ->%d\n",led_cdev->name,strlen(led_cdev->name),lp5523_control->name,strlen(lp5523_control->name));
            if(!lcd_backlight_state && state == 1 && lp5523_menu_back_state){
                //printk("[LED_TEST] lp5523_value is 1\n");
                lp5523_value = state;
            }else{
                led_set_brightness(led_cdev, state);
                //printk("[LED_TEST] lcd_backlight_state is %d, state -> %d\n",lcd_backlight_state,(int)state);
            }
        }else if ( strcmp(led_cdev->name, "lp5523:channel4") == 0 ){
            lp5523_menu_back_state = state;
            //printk("[LED_TEST] lp5523_menu_back_state -> %x\n", (int)lp5523_menu_back_state);
            led_set_brightness(led_cdev, state);
        }else{
            //printk("[LED_TEST] led_cdev->name -> %s\n", led_cdev->name);
            led_set_brightness(led_cdev, state);
        }
#elif defined(CONFIG_MACH_MSM8974_EF65S)
        if ( strcmp(led_cdev->name, "lcd-backlight") == 0 ) {
            led_set_brightness(led_cdev, state);
            if ( state > 0 ) {
                lcd_backlight_state = 1;				
                led_set_brightness(kpdbl_menu, kpdbl_menu_level);
                led_set_brightness(kpdbl_back, kpdbl_back_level);
            }
            else {
                lcd_backlight_state = 0;
                kpdbl_menu_level = 0; led_set_brightness(kpdbl_menu, kpdbl_menu_level);
                kpdbl_back_level = 0; led_set_brightness(kpdbl_back, kpdbl_back_level);
            }			
        }
        else if ( strcmp(led_cdev->name, "kpdbl_menu") == 0 ) {
            kpdbl_menu_level = state;
            if ( lcd_backlight_state == 0 ) {
                if ( state == 0 ) led_set_brightness(led_cdev, state);			
            }
            else {			
                led_set_brightness(led_cdev, state);
            }
        }
        else if ( strcmp(led_cdev->name, "kpdbl_back") == 0 ) {
            kpdbl_back_level = state;
            if ( lcd_backlight_state == 0 ) {
                if ( state == 0 ) led_set_brightness(led_cdev, state);			
            }
            else {			
                led_set_brightness(led_cdev, state);
            }
        }
        else {
            led_set_brightness(led_cdev, state);
        }
#else
        led_set_brightness(led_cdev, state);
#endif
        //-- p11309

    }

    return ret;
}

static ssize_t led_max_brightness_store(struct device *dev,
                                        struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    ssize_t ret = -EINVAL;
    unsigned long state = 0;

    ret = strict_strtoul(buf, 10, &state);
    if (!ret) {
        ret = size;
        if (state > LED_FULL)
            state = LED_FULL;
        led_cdev->max_brightness = state;
        led_set_brightness(led_cdev, led_cdev->brightness);
    }

    return ret;
}

static ssize_t led_max_brightness_show(struct device *dev,
                                       struct device_attribute *attr, char *buf)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);

    return snprintf(buf, LED_BUFF_SIZE, "%u\n", led_cdev->max_brightness);
}

static struct device_attribute led_class_attrs[] = {
    __ATTR(brightness, 0644, led_brightness_show, led_brightness_store),
    __ATTR(max_brightness, 0644, led_max_brightness_show,
           led_max_brightness_store),
#ifdef CONFIG_LEDS_TRIGGERS
    __ATTR(trigger, 0644, led_trigger_show, led_trigger_store),
#endif
    __ATTR_NULL,
};

static void led_timer_function(unsigned long data)
{
    struct led_classdev *led_cdev = (void *)data;
    unsigned long brightness;
    unsigned long delay;

    if (!led_cdev->blink_delay_on || !led_cdev->blink_delay_off) {
        led_set_brightness(led_cdev, LED_OFF);
        return;
    }

    brightness = led_get_brightness(led_cdev);
    if (!brightness) {
        /* Time to switch the LED on. */
        brightness = led_cdev->blink_brightness;
        delay = led_cdev->blink_delay_on;
    } else {
        /* Store the current brightness value to be able
         * to restore it when the delay_off period is over.
         */
        led_cdev->blink_brightness = brightness;
        brightness = LED_OFF;
        delay = led_cdev->blink_delay_off;
    }

    led_set_brightness(led_cdev, brightness);

    mod_timer(&led_cdev->blink_timer, jiffies + msecs_to_jiffies(delay));
}

/**
 * led_classdev_suspend - suspend an led_classdev.
 * @led_cdev: the led_classdev to suspend.
 */
void led_classdev_suspend(struct led_classdev *led_cdev)
{
    led_cdev->flags |= LED_SUSPENDED;
    led_cdev->brightness_set(led_cdev, 0);
}
EXPORT_SYMBOL_GPL(led_classdev_suspend);

/**
 * led_classdev_resume - resume an led_classdev.
 * @led_cdev: the led_classdev to resume.
 */
void led_classdev_resume(struct led_classdev *led_cdev)
{
    led_cdev->brightness_set(led_cdev, led_cdev->brightness);
    led_cdev->flags &= ~LED_SUSPENDED;
}
EXPORT_SYMBOL_GPL(led_classdev_resume);

static int led_suspend(struct device *dev, pm_message_t state)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);

    if (led_cdev->flags & LED_CORE_SUSPENDRESUME)
        led_classdev_suspend(led_cdev);

    return 0;
}

static int led_resume(struct device *dev)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);

    if (led_cdev->flags & LED_CORE_SUSPENDRESUME)
        led_classdev_resume(led_cdev);

    return 0;
}

/**
 * led_classdev_register - register a new object of led_classdev class.
 * @parent: The device to register.
 * @led_cdev: the led_classdev structure for this device.
 */
int led_classdev_register(struct device *parent, struct led_classdev *led_cdev)
{
    led_cdev->dev = device_create(leds_class, parent, 0, led_cdev,
                                  "%s", led_cdev->name);
    if (IS_ERR(led_cdev->dev))
        return PTR_ERR(led_cdev->dev);

#ifdef CONFIG_LEDS_TRIGGERS
    init_rwsem(&led_cdev->trigger_lock);
#endif
    /* add to the list of leds */
    down_write(&leds_list_lock);
    list_add_tail(&led_cdev->node, &leds_list);
    up_write(&leds_list_lock);

    if (!led_cdev->max_brightness)
        led_cdev->max_brightness = LED_FULL;

    led_update_brightness(led_cdev);

    init_timer(&led_cdev->blink_timer);
    led_cdev->blink_timer.function = led_timer_function;
    led_cdev->blink_timer.data = (unsigned long)led_cdev;

#ifdef CONFIG_LEDS_TRIGGERS
    led_trigger_set_default(led_cdev);
#endif

    //++ p11309 - 2013.08.05 for ON Sync LCD & LED
#if defined(CONFIG_MACH_MSM8974_EF56S)
    if ( strcmp(led_cdev->name, "gpled_0") == 0 ) gpled_0 = led_cdev;
    if ( strcmp(led_cdev->name, "gpled_1") == 0 ) gpled_1 = led_cdev;
    if ( strcmp(led_cdev->name, "gpled_2") == 0 ) gpled_2 = led_cdev;
    if ( strcmp(led_cdev->name, "gpled_3") == 0 ) gpled_3 = led_cdev;	
#elif defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
    if ( strcmp(led_cdev->name, "lp5523:channel8") == 0 ){
        //printk("[LED_TEST] led_cdev->name -> lp5523:channel8 is sellected\n");
        lp5523_control= led_cdev;
    }
#elif defined(CONFIG_MACH_MSM8974_EF65S)
    if ( strcmp(led_cdev->name, "kpdbl_menu") == 0 ) kpdbl_menu = led_cdev;
    if ( strcmp(led_cdev->name, "kpdbl_back") == 0 ) kpdbl_back = led_cdev;
#endif
    //-- p11309

    printk(KERN_DEBUG "Registered led device: %s\n",
           led_cdev->name);

    return 0;
}
EXPORT_SYMBOL_GPL(led_classdev_register);

/**
 * led_classdev_unregister - unregisters a object of led_properties class.
 * @led_cdev: the led device to unregister
 *
 * Unregisters a previously registered via led_classdev_register object.
 */
void led_classdev_unregister(struct led_classdev *led_cdev)
{
#ifdef CONFIG_LEDS_TRIGGERS
    down_write(&led_cdev->trigger_lock);
    if (led_cdev->trigger)
        led_trigger_set(led_cdev, NULL);
    up_write(&led_cdev->trigger_lock);
#endif

    /* Stop blinking */
    led_brightness_set(led_cdev, LED_OFF);

    device_unregister(led_cdev->dev);

    down_write(&leds_list_lock);
    list_del(&led_cdev->node);
    up_write(&leds_list_lock);
}
EXPORT_SYMBOL_GPL(led_classdev_unregister);

static int __init leds_init(void)
{
    leds_class = class_create(THIS_MODULE, "leds");
    if (IS_ERR(leds_class))
        return PTR_ERR(leds_class);
    leds_class->suspend = led_suspend;
    leds_class->resume = led_resume;
    leds_class->dev_attrs = led_class_attrs;
    return 0;
}

static void __exit leds_exit(void)
{
    class_destroy(leds_class);
}

subsys_initcall(leds_init);
module_exit(leds_exit);

MODULE_AUTHOR("John Lenz, Richard Purdie");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LED Class Interface");
