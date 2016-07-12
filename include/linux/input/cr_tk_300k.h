/*******************************************************************************
*
*   This confidential and proprietary software may be used only as
*   authorised by a licensing agreement from CORERIVER Semiconductor Co., Ltd.
*
*   If you change and distribute this Program, you are obligated to report
*   the details of the change to CORERIVER Semiconductor Co., Ltd. byrequest.
*
*   (c) Copyright 2012 CORERIVER Semiconductor Co., Ltd.
*     All Rights Reserved
*
*   The entire notice above must be reproduced on all authorised
*   copies and copies may only be made to the extent permitted
*   by a licensing agreement from CORERIVER Semiconductor Co., Ltd.
*
* ------------------------------------------------------------------------------
*
*   FILE            : cr_tk.h
*   AUTHOR          : CORERIVER
*   DESCRIPTION     : Demo Board Code for Touch Key Device Driver
*******************************************************************************/

#ifndef _CORERIVER_TOUCH_H_
#define _CORERIVER_TOUCH_H_

#ifdef CONFIG_HAS_EARLYSUSPEND_CR
    #include <linux/wakelock.h>
    #include <linux/earlysuspend.h>
    #include <linux/suspend.h>
#endif



//[*]--------------------------------------------------------------------------------------------------[*]
// Touch Key : Debug & Functionality & ISP Define
//[*]--------------------------------------------------------------------------------------------------[*]
#define DEBUG_TKEY
#define ISP_UPGRADE
#define PAN_TM_LED_OPERATION

//[*]--------------------------------------------------------------------------------------------------[*]
// Touch Key : Device Driver Define
//[*]--------------------------------------------------------------------------------------------------[*]
#define I2C_TKEY_NAME          "cr-tk-300k"             // Modify "system/usr/idc/xxx-ts.idc" file w/ ADB tool
#define I2C_SLAVE_ADDR          (0xC0>>1)               // 7bit-I2C ADDR (R:0x41 / W:0x40)

#define TK_GPIO_INT             9       // GPIO Interrupt
#define KEY_3P3_EN             28      // KEY_3P3_EN
#define TM_1P8_EN               16      // TM_1P8_EN
#define TK_GPIO_SDA             55
#define TK_GPIO_SCL             56

#define TK_KEY_MAX_CNT          2                       // 2-key module

#define TK_POS_DATA_SIZE(cnt)   cnt*6                   // CHPer, Diff, RawData size

#define TK_STATUS_FLAG          0x0F
#define TK_UPDOWN_FLAG          0x08                    // 0x08 Up/Down Flag
#define TK_KEYCODE_FLAG         0x07                    // 0x07 Key Code Flag

#define I2C_SEND_MAX_SIZE       32                      // I2C Send data max size
#define I2C_READ_MAX_SIZE       TK_KEY_MAX_CNT*6+6      // I2C Receive data max size

//[*]--------------------------------------------------------------------------------------------------[*]
// Touch Key : Event Type Define
//[*]--------------------------------------------------------------------------------------------------[*]
#define TK_KEY_DOWN             0x00                    // Touch Key Press
#define TK_KEY_UP               0x08                    // Touch Key Release

#define TK_NONE                 0x00
#define TK_KEY1                 0x01                    // Touch Key X1 Press
#define TK_KEY2                 0x02                    // Touch Key X2 Press
#define TK_KEY3                 0x03                    // Touch Key X3 Press
#define TK_KEY4                 0x04                    // Touch Key X4 Press

#define TK_KEY_LED_ON           0x10                    // TOUCH Key LED ON
#define TK_KEY_LED_OFF          0x20                    // TOUCH Key LED OFF
#define TK_KEY_GLOVE_MODE_ON    0x30                    // TOUCH Key GLOBE MODE ON
#define TK_KEY_GLOVE_MODE_OFF   0x40                    // TOUCH Key GLOBE MODE OFF 


#define TK_KEY_COUNT_MAX        TK_KEY_MAX_CNT          // Touch Key Count

#define TK_KEY_THRESHOLD_MIN            10
#define TK_KEY_THRESHOLD_MAX            500
#define TK_KEY_THRESHOLD_DEFAULT_NORMAL 60  
#define TK_KEY_THRESHOLD_DEFAULT_GLOVE  20


//[*]--------------------------------------------------------------------------------------------------[*]
// Touch Key : Touch IRQ Mode Define
//[*]--------------------------------------------------------------------------------------------------[*]
#undef  TK_INT_ACTIVE_HIGH_EDGE
#define TK_INT_ACTIVE_LOW_EDGE

//[*]--------------------------------------------------------------------------------------------------[*]
// Touch Key : System Control Register Address(CMD) Define
//[*]--------------------------------------------------------------------------------------------------[*]
enum cr_tk_register_addr {
    REG_CMD_KEY_CODE        = 0x00,
    REG_CMD_FW_VERSION      = 0x01,
    REG_CMD_MODULE_VERSION  = 0x02,
    REG_CMD_MODE        = 0x03,
    REG_CMD_CHECKSUMH       = 0x04,
    REG_CMD_CHECKSUML       = 0x05,
    REG_CMD_THRESHOLDH      = 0x06,
    REG_CMD_THRESHOLDL      = 0x07,
    // Touch Key 1 Data    
    REG_CMD_CHPERH1         = 0x08,
    REG_CMD_CHPERL1         = 0x09,
    REG_CMD_DIFFH1          = 0x0A,
    REG_CMD_DIFFL1          = 0x0B,
    REG_CMD_RAWH1           = 0x0C,
    REG_CMD_RAWL1           = 0x0D,
    // Touch Key 2 Data
    REG_CMD_CHPERH2         = 0x0E,
    REG_CMD_CHPERL2         = 0x0F,
    REG_CMD_DIFFH2          = 0x10,
    REG_CMD_DIFFL2          = 0x11,
    REG_CMD_RAWH2           = 0x12,
    REG_CMD_RAWL2           = 0x13,
    // Touch Key 3 Data
    REG_CMD_CHPERH3         = 0x14,
    REG_CMD_CHPERL3         = 0x15,
    REG_CMD_DIFFH3          = 0x16,
    REG_CMD_DIFFL3          = 0x17,
    REG_CMD_RAWH3           = 0x18,
    REG_CMD_RAWL3           = 0x19,
    // Touch Key 4 Data
    REG_CMD_CHPERH4         = 0x1A,
    REG_CMD_CHPERL4         = 0x1B,
    REG_CMD_DIFFH4          = 0x1C,
    REG_CMD_DIFFL4          = 0x1D,
    REG_CMD_RAWH4           = 0x1E,
    REG_CMD_RAWL4           = 0x1F,
    // Touch Key 5 Data
    REG_CMD_CHPERH5         = 0x20,
    REG_CMD_CHPERL5         = 0x21,
    REG_CMD_DIFFH5          = 0x22,
    REG_CMD_DIFFL5          = 0x23,
    REG_CMD_RAWH5           = 0x24,
    REG_CMD_RAWL5           = 0x25,
};

enum {
	PAN_TM_SET_NORMAL_MODE = 0,
	PAN_TM_SET_GLOVE_MODE,
	PAN_TM_SET_PEN_MODE,	// same with glove mode
};

enum {
	TM_IOCTL_SET_NORMAL_MODE = 0,
	TM_IOCTL_SET_GLOVE_MODE,
	TM_IOCTL_SET_PEN_MODE,
	TM_IOCTL_SET_THRESHOLD_VALUE,
	TM_IOCTL_READ_RAW_DATA,
	TM_IOCTL_SET_LED_ON,
	TM_IOCTL_SET_LED_OFF,
	TM_IOCTL_READ_FW_VERSION,
};

enum {
	PAN_TM_LED_MENU_THRESHOLD,
	PAN_TM_LED_MENU_PERCENT,
	PAN_TM_LED_MENU_DIFFDATA,
	PAN_TM_LED_MENU_RAWDATA,
	PAN_TM_LED_BACK_THRESHOLD,
	PAN_TM_LED_BACK_PERCENT,
	PAN_TM_LED_BACK_DIFFDATA,
	PAN_TM_LED_BACK_RAWDATA,
	PAN_TM_LED_RAWDATA_MAX,
};

enum pan_tm_key_state { INIT, APPMODE, SUSMODE };


//[*]--------------------------------------------------------------------------------------------------[*]
// Touch Key : TSP & I2C Structure Define
//[*]--------------------------------------------------------------------------------------------------[*]
struct  i2c_client;
struct  input_dev;
struct  device;


//[*]--------------------------------------------------------------------------------------------------[*]


//[*]--------------------------------------------------------------------------------------------------[*]
struct tkey {
    int                 irq;
    struct i2c_client   *client;
    struct input_dev    *input;
    char                phys[32];

    // data
    unsigned char       touch_key;
    unsigned char       touch_event;

    // control flags
    unsigned char       disabled;       // Interrupt status
    int                 bd_version;
    int                 fw_version;
    int                 fw_revision;
    unsigned char       *builddate;     // WORK_DATE    : 2013/11/04
    unsigned char       *vender_id;     // VENDOR_NAME  :
    unsigned char       *prouct_id;     // PRODUCT_NAME :
    unsigned char       *chipset;       // CHIPSET      : CORERIVER TC370

    char                *fw_name;       // "/data/cr_ts_isp.bin"
    unsigned char       fw_chksum;

    // irq func used
    struct workqueue_struct     *work_queue;
    struct work_struct          work;   // Interrupt isr function
#ifdef PAN_TM_LED_OPERATION
    struct led_classdev         tm_led_cdev;
    struct workqueue_struct     *tm_led_work_queue;
    struct work_struct          tm_led_work;   // Interrupt isr function
    unsigned char			          tm_led_brightness;
#endif 
    unsigned int threshold_normal;
    unsigned int threshold_globe;
    unsigned int rawdata[PAN_TM_LED_RAWDATA_MAX];
	enum pan_tm_key_state       state;
#if	defined(CONFIG_HAS_EARLYSUSPEND_CR)
    struct early_suspend        power;
#endif
    int mode; // 0 : normal, 1 : glove
    int cover_state; // 0 : opened, 1 : closed
};



//[*]--------------------------------------------------------------------------------------------------[*]
#endif /* _CORERIVER_TOUCH_H_ */
//[*]--------------------------------------------------------------------------------------------------[*]
//[*]--------------------------------------------------------------------------------------------------[*]

