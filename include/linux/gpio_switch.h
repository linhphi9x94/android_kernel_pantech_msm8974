#ifndef _LINUX_GPIO_SWITCH_H
#define _LINUX_GPIO_SWITCH_H

#define GPIO_SWITCH_NAME "gpio_switch"
#define GPIO_SWITCH_WAKE_LOCK_TIMEOUT 500
#include <linux/switch.h>
#include <linux/wakelock.h>
#include <linux/input.h>

enum{
    DEBOUNCE_UNSTABLE   = BIT(0), // got irq , while debouncing 
    DEBOUNCE_PRESSED    = BIT(1),
    DEBOUNCE_NOTPRESSED = BIT(2), 
    DEBOUNCE_WAIT_IRQ   = BIT(3), // stable irq state

    DEBOUNCE_UNKNOWN    =
        DEBOUNCE_PRESSED | DEBOUNCE_NOTPRESSED,
};
enum{
    EMERGENCY_MODE = 0,
    NORMAL_MODE = 1,
};
enum{
    PAN_INPUTEVENT = 0,
    PAN_UEVENT = 1,
};

struct gpio_switch_data {
    struct switch_dev sdev;
    struct input_dev *input_dev;
    int gpio;
    int irq;
    unsigned char disabled;    // Interrupt status
    ktime_t debounce_time;
    uint8_t debounce;
    int debounce_count;
    struct hrtimer timer;
    struct work_struct work;
    struct wake_lock wakelock;
    struct gpio_switch_data *next;
    int current_state;
    int last_state;
    const char *name;
    int event_mode;
    int flag;
    
    void (*notify)(const int);
    void (*emergency_notify)(const int);
};
void register_notify_func(int mode , const char* name , void (*noti_func)(const int));

#endif
