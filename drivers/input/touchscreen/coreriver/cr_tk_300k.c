/*******************************************************************************
*
*   This confidential and proprietary software may be used only as
*   authorised by a licensing agreement from CORERIVER Semiconductor Co., Ltd.
*
*   If you change and distribute this Program, you are obligated to report
*   the details of the change to CORERIVER Semiconductor Co., Ltd. byrequest.
*
*   (c) Copyright 2013 CORERIVER Semiconductor Co., Ltd.
*     All Rights Reserved
*
*   The entire notice above must be reproduced on all authorised
*   copies and copies may only be made to the extent permitted
*   by a licensing agreement from CORERIVER Semiconductor Co., Ltd.
*
* ------------------------------------------------------------------------------
*
*   FILE            : cr_tk_300k.c
*   AUTHOR          : CORERIVER
*   DESCRIPTION     : Demo Board Code for Touch Key Device Driver
*   VERSION         : 2013 11/22
*
*   History         :
*  2013/11/28   : Modified for 4-key machine.
*  2013/11/26   : Added Power Managerment Func. (Suspend/Resume)
*  2013/11/22   : TC300K 1'st Release Driver Code
*                  : add ISP Upgrade
*******************************************************************************/

#include <linux/device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <asm/unaligned.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <linux/pm.h>
#include <linux/miscdevice.h>
#include <linux/leds.h>
#include <asm/uaccess.h>
#include <linux/input/cr_tk_300k.h>
#include "cr_tk_fw.h"



/* -------------------------------------------------------------------- */
/* debug option */
/* -------------------------------------------------------------------- */
#define PAN_TOUCH_TM_DEBUG_OPERATION_ERR_MASK      0x00000001
#define PAN_TOUCH_TM_DEBUG_OPERATION_MASK          0x00000002  
#define PAN_TOUCH_TM_DEBUG_FIRMWARE_MASK           0x00000004
#define PAN_TOUCH_TM_DEBUG_LED_MASK                0x00000004
#define PAN_TOUCH_TM_DEBUG_ALL_MASK                0x00000007

static int pan_tm_debug_state = PAN_TOUCH_TM_DEBUG_OPERATION_ERR_MASK;
#define dbg_cr(fmt, args...)      printk("[+++ TM] " fmt, ##args);
#define dbg_op_err(fmt,args...)   if(pan_tm_debug_state & PAN_TOUCH_TM_DEBUG_OPERATION_ERR_MASK) printk("[+++ TM] " fmt,##args); 
#define dbg_op(fmt,args...)       if(pan_tm_debug_state & PAN_TOUCH_TM_DEBUG_OPERATION_MASK) printk("[+++ TM] " fmt,##args);
#define dbg_firmw(fmt,args...)    if(pan_tm_debug_state & PAN_TOUCH_TM_DEBUG_FIRMWARE_MASK) printk("[+++ TM] " fmt,##args); 
#define dbg_led(fmt,args...)      if(pan_tm_debug_state & PAN_TOUCH_TM_DEBUG_LED_MASK) printk("[+++ TM] " fmt,##args); 

//[*]------------------------------------------------------------------------[*]
//
// Touch Key : Function Prototype Define
//
//[*]------------------------------------------------------------------------[*]
static  int __devinit   tkey_i2c_probe    (struct i2c_client *client, const struct i2c_device_id *id);
static  int __devexit   tkey_i2c_remove    (struct i2c_client *client);
static  int __init      tkey_i2c_init      (void);
static  void __exit     tkey_i2c_exit      (void);

#ifdef CONFIG_HAS_EARLYSUSPEND_CR
static void tkey_suspend (struct early_suspend *h);
static void tkey_resume (struct early_suspend *h);
#endif

int pan_tm_key_resume (void);
int pan_tm_key_suspend (void);

irqreturn_t tkey_irq (int irq, void *handle);
static void tkey_work_q (struct work_struct *work);

static  void    tkey_input_close (struct input_dev *input);
static  int     tkey_input_open (struct input_dev *input);

static  int     tkey_remove                (struct device *dev);
static  int     tkey_probe                 (struct i2c_client *client);
int cr_tk_key_select(int button);
void set_i2c_from_gpio_function(bool state);
#ifdef PAN_TM_LED_OPERATION
static void pan_tm_set_led_onoff(struct led_classdev *cdev,enum led_brightness brightness);
#endif
void pan_tm_set_mode(int mode);
void pan_tm_set_cover_state(int state);
void init_gpio_cr(void); // p13106
static int tm_fops_open(struct inode *inode, struct file *filp);
static long tm_fops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static ssize_t tm_fops_write(struct file *file, const char *buf, size_t count, loff_t *ppos);
int cr_tk_i2c_write(struct i2c_client *client, unsigned char *data, unsigned int len);
int cr_tk_i2c_read(struct i2c_client *client, unsigned char *data, unsigned int len);
void read_raw_data_tm(void);

static struct tkey *pan_tm;

/* Modify Key Codes for Dependency of AP Machine */
static  unsigned int cr_tk_keycode[] = {
    KEY_BACK,  KEY_MENU,   KEY_HOME,   KEY_SEARCH
};

// misc driver set.
static struct file_operations tm_fops = {
	.owner          = THIS_MODULE,
	.open           = tm_fops_open,
	.unlocked_ioctl = tm_fops_ioctl,
	.write          = tm_fops_write,
};

static struct miscdevice tm_event = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tm_fops",
	.fops = &tm_fops,
};

static int tm_fops_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static long tm_fops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int rc, fw_ver=0;
    u8 data[3];
    
    switch(cmd) 
    {
        case TM_IOCTL_SET_NORMAL_MODE:
            pan_tm_set_mode(PAN_TM_SET_NORMAL_MODE);
            break;
        case TM_IOCTL_SET_GLOVE_MODE:
            pan_tm_set_mode(PAN_TM_SET_GLOVE_MODE);
            break;
        case TM_IOCTL_SET_PEN_MODE:
            pan_tm_set_mode(PAN_TM_SET_PEN_MODE);
            break;
        case TM_IOCTL_SET_THRESHOLD_VALUE:
            if(arg > TK_KEY_THRESHOLD_MAX) {
                arg = TK_KEY_THRESHOLD_MAX;
            } else if(arg < TK_KEY_THRESHOLD_MIN) {
                arg = TK_KEY_THRESHOLD_MIN;
            }
            data[0] = 0x8f;
            data[1] = (arg >> 8) & 0xff;
            data[2] = (arg & 0xff);
            rc = i2c_smbus_write_i2c_block_data(pan_tm->client, REG_CMD_KEY_CODE, sizeof(data), data);
            if(rc < 0) {
                dbg_cr("%s : failed to set TM THRESHOLD_VALUE, error = %d\n", __func__, rc);
                return rc;
            } else {
                dbg_cr("%s : set TM THRESHOLD_VALUE\n", __func__);
            }
            break;
        case TM_IOCTL_READ_RAW_DATA:
            read_raw_data_tm();
            if (copy_to_user(argp, pan_tm->rawdata, sizeof(pan_tm->rawdata)))
                return false;
            break;
        case  TM_IOCTL_SET_LED_ON:
            rc = i2c_smbus_write_byte_data(pan_tm->client, 0, TK_KEY_LED_ON);
            if(rc < 0) {
                dbg_cr("%s : failed to set TM_LED ON, error = %d\n", __func__, rc);
                return rc;
            } else {
                dbg_cr("%s : set TM_LED ON\n", __func__);
            }
            break;
        case  TM_IOCTL_SET_LED_OFF:
            rc = i2c_smbus_write_byte_data(pan_tm->client, 0, TK_KEY_LED_OFF);
            if(rc < 0) {
                dbg_cr("%s : failed to set TM_LED OFF, error = %d\n", __func__, rc);
                return rc;
            } else {
                dbg_cr("%s : set TM_LED OFF\n", __func__);
            }
            break;
        case TM_IOCTL_READ_FW_VERSION:
            rc = i2c_smbus_read_i2c_block_data(pan_tm->client, 0,sizeof(data), data);
            if(rc < 0) {
                dbg_cr("%s : failed to read TM FW version, error = %d\n", __func__, rc);
                fw_ver = -1;
                return rc;
            } else {
                fw_ver=data[1];
            }
            if (copy_to_user(argp,&fw_ver, sizeof(fw_ver)))
                return false;  
            break;
        default :
            dbg_cr("%s : unexpected command : %d\n", __func__, cmd);
            break;
    }

    return 0;
}

static ssize_t tm_fops_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    int i, nBufSize = 0;
    int rc, fw_ver, mode = 0;
    u8 data[2];
	
    if((size_t)(*ppos) > 0)
        return 0;
    
    if(buf != NULL) {
        nBufSize=strlen(buf);
        if(strncmp(buf, "debug_",6) == 0) {
            if(buf[6] > '0') {
                i = buf[6] - '1';
                if(pan_tm_debug_state & 0x00000001 <<i) {
                    pan_tm_debug_state &= ~(0x00000001 <<i);
                } else {
                    pan_tm_debug_state |= (0x00000001 <<i);
                }
            }
            dbg_cr("%s : pan_tm_debug_state = %x, i = %d\n", __func__, pan_tm_debug_state, i);
        } else if(strncmp(buf, "set_normal_mode", 15) == 0) {
            pan_tm_set_mode(PAN_TM_SET_NORMAL_MODE);
        } else if(strncmp(buf, "set_glove_mode", 14) == 0) {
            pan_tm_set_mode(PAN_TM_SET_GLOVE_MODE);
        } else if (strncmp(buf, "read_fw_ver", 11) == 0) {
            rc = i2c_smbus_read_i2c_block_data(pan_tm->client, REG_CMD_FW_VERSION, sizeof(data), data);
            if(rc < 0) {
                dbg_cr("%s : failed to read TM FW version, error : %d\n", __func__, rc);
                return rc;
            } else {
                fw_ver = data[0];
            }
            dbg_cr("%s : TM FW version : %d\n", __func__, fw_ver);
        } else if (strncmp(buf, "read_mode", 9) == 0) {
            rc = i2c_smbus_read_i2c_block_data(pan_tm->client, REG_CMD_MODE, sizeof(data), data);
            if(rc < 0) {
                dbg_cr("%s : failed to read TM_KEY mode, error : %d\n", __func__, rc);
                return rc;
            } else {
                mode = data[0];
            }
            dbg_cr("%s : TM_KEY mode : %d\n", __func__, mode);
        } else {
            dbg_cr("%s : unexpected command\n", __func__);
        }
    }
	
    return nBufSize;
}

//[*]------------------------------------------------------------------------[*]
//
// Touch Key : Function
//
//[*]------------------------------------------------------------------------[*]
void read_raw_data_tm(void)
{
  int rc,i;  
  u8 data[16];

  memset(data, 0, sizeof(data));
  rc=i2c_smbus_read_i2c_block_data(pan_tm->client, 0x10,sizeof(data),data);
  if(rc<0)
    dbg_cr("read_raw_data_tm i2c_smbus_read_i2c_block_data rc-> %d\n",rc);
  for(i=0;i<PAN_TM_LED_RAWDATA_MAX;i++){
    pan_tm->rawdata[i]=((data[i*2]<<8)|data[i*2+1]);
  }  
}


//[*]------------------------------------------------------------------------[*]
// Touch Key : I2C Control Function
//[*]------------------------------------------------------------------------[*]
int cr_tk_i2c_read(struct i2c_client *client, unsigned char *data, unsigned int len)
{
    /* Standard I2C Read Function (Combined Mode) */
    struct i2c_msg msg[2];
    int            ret;

    if((len == 0) || (data == NULL)) {
        dev_err(&client->dev, "I2C read error: Null pointer or length == 0\n");
    }

    memset(msg, 0x00, sizeof(msg));

    msg[0].addr  = client->addr;
    msg[0].flags = 0;
    msg[0].len   = 1;
    msg[0].buf   = data;

    msg[1].addr  = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len   = len;
    msg[1].buf   = data;

    if((ret = i2c_transfer(client->adapter, msg, 2)) != 2) {
        dev_err(&client->dev, "I2C read error: (%d) reg: 0x%X len: %d\n", ret, *data, len);
        return -EIO;
    }

    return len;
}

int cr_tk_i2c_write(struct i2c_client *client, unsigned char *data, unsigned int len)
{
    /* Standard I2C Write Function */
    struct i2c_msg msg;
    int            ret;

    if((len == 0) || (data == NULL)) {
        dev_err(&client->dev, "I2C read error: Null pointer or length == 0\n");
        return  -1;
    }

    memset(&msg, 0x00, sizeof(msg));

    msg.addr    = client->addr;
    msg.flags   = 0;
    msg.len     = len;
    msg.buf     = data;

    if ((ret = i2c_transfer(client->adapter, &msg, 1)) != 1) {
        dev_err(&client->dev, "I2C command send error: (%d) reg: 0x%x\n", ret, *data);
        return -EIO;
    }

    return  len;
}


#ifdef PAN_TM_LED_OPERATION
static void pan_tm_set_led_onoff(struct led_classdev *tm_led_cdev,enum led_brightness brightness)
{
    int rc = 0;
    struct tkey *tk = container_of(tm_led_cdev, struct tkey, tm_led_cdev);  
    dbg_op("%s : brightness = %d\n", __func__, brightness);
    
    tk->tm_led_brightness = brightness;
    
    if(tk->state != APPMODE) {
        dbg_cr(" %s : TM state is not APPMODE\n", __func__);
	 return;
    }
    
    if(tk->tm_led_brightness) {
        rc = i2c_smbus_write_byte_data(tk->client, 0, TK_KEY_LED_ON);
	 if(rc < 0) {
	     dbg_cr("%s : failed to set TM_LED ON, error = %d\n", __func__, rc);
	     return;
	 } else {
	     dbg_op("%s : set TM_LED ON\n", __func__);
	 }
    } else {
	rc=i2c_smbus_write_byte_data(tk->client, 0, TK_KEY_LED_OFF);
	if(rc < 0) {
	    dbg_cr("%s : failed to set TM_LED OFF, error = %d\n", __func__, rc);
	    return;
	} else {
	    dbg_op("%s : set TM_LED OFF\n", __func__);
	}
    }
}
#endif

void pan_tm_set_mode(int mode)
{
    int rc = 0;

    dbg_op("%s enter, mode = %d\n",__func__, mode);

    if(pan_tm == NULL)
        return;
    
    if(pan_tm->state != APPMODE) {
        dbg_cr(" %s : TM state is not APPMODE\n", __func__);
        return;
    }

    switch (mode) {
        case PAN_TM_SET_NORMAL_MODE:
            rc = i2c_smbus_write_byte_data(pan_tm->client, REG_CMD_KEY_CODE, TK_KEY_GLOVE_MODE_OFF);
            if(rc < 0) {
                dbg_cr("%s : failed to set TM_KEY normal mode, error = %d\n", __func__, rc);
                return;
            } else {
                dbg_cr("%s : set TM_KEY normal mode\n", __func__);
                pan_tm->mode = 0;
            }
            break;
        case PAN_TM_SET_GLOVE_MODE:
            rc=i2c_smbus_write_byte_data(pan_tm->client, REG_CMD_KEY_CODE, TK_KEY_GLOVE_MODE_ON);
            if(rc  < 0) {
                dbg_cr("%s : failed to set TM_KEY glove mode, error = %d\n", __func__, rc);
                return;
            } else {
                dbg_cr("%s : set TM_KEY glove mode\n", __func__);
                pan_tm->mode = 1;
            }
            break;
        case PAN_TM_SET_PEN_MODE:
            rc = i2c_smbus_write_byte_data(pan_tm->client, REG_CMD_KEY_CODE, TK_KEY_GLOVE_MODE_ON);
            if(rc < 0) {
                dbg_cr("%s : failed to set TM_KEY pen mode, error = %d\n", __func__, rc);
                return;
            } else {
                dbg_cr("%s : set TM_KEY pen mode\n", __func__);
                pan_tm->mode = 1;
            }
            break;
        default:
            dbg_cr("%s : unexpected argument = %d\n", __func__, mode);
            break;
    }
}
EXPORT_SYMBOL(pan_tm_set_mode);

void pan_tm_set_cover_state(int state) {
    dbg_op("%s enter, state = %d\n",__func__, state);
    
    if(pan_tm == NULL)
        return;

    if(state == 0) { // opened
        pan_tm->cover_state = 0;
        pan_tm_key_resume();
    }
    else if(state == 1) { // closed
        pan_tm->cover_state = 1;
        pan_tm_key_suspend();
    }
    else {
        dbg_cr("%s : unexpected argument = %d\n", __func__, state);
		return;
    }
}
EXPORT_SYMBOL(pan_tm_set_cover_state);

//[*]------------------------------------------------------------------------[*]
// Touch Key : Data Processing Function
//[*]------------------------------------------------------------------------[*]
void cr_tk_work(struct tkey *tk)
{
    unsigned char cnt, buf[I2C_READ_MAX_SIZE] = {0};
    int keyCode = 0;
    dbg_op("%s\n",__func__);
    
    /* I2C Data Read Buffer */
    buf[0] = REG_CMD_KEY_CODE;
    //if(cr_tk_i2c_read(tk->client, buf, I2C_READ_MAX_SIZE)<0)
    if(cr_tk_i2c_read(tk->client, buf, 1)<0)
        return;

    tk->touch_key = buf[REG_CMD_KEY_CODE]&0x07;
    tk->touch_event = buf[REG_CMD_KEY_CODE]&0x08;

    /* Received Key Code Check */
    for(cnt=0; cnt<TK_KEY_COUNT_MAX; cnt++) {
        if(tk->touch_key == (TK_KEY1+cnt)) break;
    }
    if(cnt == TK_KEY_COUNT_MAX) return;        

    /* Key Touch Event */
    if(tk->touch_event == TK_KEY_UP)
    {
        dbg_op("Key Code (%d) <= (%d) \n", tk->touch_key, tk->touch_event);
        keyCode = cr_tk_key_select(tk->touch_key);
        if (keyCode < 0) {
            return;
        }
        input_report_key(tk->input, cr_tk_keycode[keyCode], false);
    }
    else
    {
        dbg_op("Key Code (%d) => (%d) \n", tk->touch_key, tk->touch_event);
        keyCode = cr_tk_key_select(tk->touch_key);
        if (keyCode < 0) {
            return;
        }
        input_report_key(tk->input, cr_tk_keycode[keyCode], true);
    }

    input_sync(tk->input); 
}

//[*]------------------------------------------------------------------------[*]
// Touch Key : Data Report Function
//[*]------------------------------------------------------------------------[*]
int cr_tk_key_select(int button)
{
    /* Temporary Function */
    switch(button){
    case TK_KEY1:
        return 0;
    case TK_KEY2:
        return 1;
    case TK_KEY3:
        return 2;
    case TK_KEY4:
        return 3;
    default:
        dbg_cr(" ERROR : key select fail, button_X : %d \n", button);
        return -1;
        break;
    }
//    return (button-1);
}

static void cr_tk_event_clear(struct tkey *tk)
{
    unsigned char cnt = 0;

    dbg_op("%s\n",__func__);
    
    // All key Release 
    for (cnt = 0; cnt<TK_KEY_MAX_CNT ; cnt++) {
         input_report_key(tk->input, cr_tk_keycode[cnt], false);     
    }    
    input_sync(tk->input);
}

//[*]------------------------------------------------------------------------[*]
// Touch Key : Enable/Disable Function
//[*]------------------------------------------------------------------------[*]
void cr_tk_enable(struct tkey *tk)
{
    unsigned char buf[I2C_READ_MAX_SIZE]={0};

    if(tk->disabled) {
        dbg_op("%s\n",__func__);
        
        // Check Interrupt Clear
        while(!gpio_get_value(TK_GPIO_INT)) {
            /* INT Clear after I2C Read of TSP buffer - Dummy Data */
            /* I2C Data Read Buffer */
            buf[0] = REG_CMD_KEY_CODE;
            if(cr_tk_i2c_read(tk->client, buf, I2C_READ_MAX_SIZE)<0)
                return;
        }

        dbg_op("%s : INT = %x, Disable = %x\n",__func__, gpio_get_value(TK_GPIO_INT), tk->disabled);

        enable_irq(tk->irq);
        tk->disabled = false;
    }
}

void cr_tk_disable(struct tkey *tk)
{
    if(!tk->disabled) {
        disable_irq(tk->irq);
        tk->disabled = true;
        cr_tk_event_clear(tk);
        dbg_op("%s\n",__func__);
        
    }
}
/*
//[*]------------------------------------------------------------------------[*]
// Touch Key : ISP Define(FW Image Upgrade)
//[*]------------------------------------------------------------------------[*]
void cr_tk_fw_gpio(int pin, unsigned int dir, unsigned int value, unsigned int pull)
{
    if(gpio_request(pin, "GPX3") < 0) {
        printk("%s : GPIO request port error!\n", __func__);
    } else {
        if(dir) {
            s3c_gpio_cfgpin(pin, S3C_GPIO_OUTPUT);
            gpio_direction_output(pin, 0);  // GPIO OUTPUT
        } else {
            s3c_gpio_cfgpin(pin, S3C_GPIO_INPUT);
            gpio_direction_input(pin);      // GPIO INPUT
        }

        if(value)
            gpio_set_value(pin, 1);         // High
        else
            gpio_set_value(pin, 0);         // Low

        if(pull == 2)
            s3c_gpio_setpull(pin, S3C_GPIO_PULL_UP);
        else if(pull == 1)
            s3c_gpio_setpull(pin, S3C_GPIO_PULL_DOWN);
        else
            s3c_gpio_setpull(pin, S3C_GPIO_PULL_NONE);

        printk("%s : GPIO %d = InOut %x, LowHigh %x, Pull UpDown %x\n", __func__, pin, dir, value, pull);
        gpio_free(pin);
    }
}
*/
#if defined(ISP_UPGRADE)
#define CSYNC1                      0xA3
#define CSYNC2                      0xAC
#define CSYNC3                      0xA5
#define CCFG                        0x92
#define PCRST                       0xB4
#define PECHIP                      0x8A
#define PWDATA                      0x83
#define LDDATA                      0xB2
#define LDMODE                      0xB8
#define PEDISC                      0xB0
#define PRDATA                      0x81
#define RDDATA                      0xB9

#define TSYNC1                      300     // us
#define TSYNC2                      50      // 1ms~50ms
#define TSYNC3                      100     // us
#define TDLY1                       1       // us
#define TDLY2                       2       // us
#define TFERASE                     20      // ms
#define TPROG                       20      // us

#define POWERON_DELAY               100
#define FW_FLASH_RETRY              5
#define BD_VERSION                  4
#define ISP_FW_HEADER_LEN           CR_TK_FW_TABLE[1]

static inline void setsda(struct tkey *tk, int state)
{
  if(state){
    //gpio_direction_output(TK_GPIO_SDA, 1);
    gpio_set_value(TK_GPIO_SDA, 1);
  }else{
    //gpio_direction_output(TK_GPIO_SDA, 0);
    gpio_set_value(TK_GPIO_SDA, 0);
  }
  return;
    /* Place your code to control SDA pin (GPIO) */
	if(gpio_request(TK_GPIO_SDA, "tkey sda")) {
    	dbg_cr("%s : request port error\n", __func__);
    } else {
      if (state) { // set sda
        	gpio_set_value(TK_GPIO_SDA, 1);
        } else { // clear sda
        	gpio_direction_output(TK_GPIO_SDA, 0);
        	gpio_set_value(TK_GPIO_SDA, 0);
        }
    	gpio_free(TK_GPIO_SDA);
    }
}

static inline void setscl(struct tkey *tk, int state)
{

  if(state){
    //gpio_direction_output(TK_GPIO_SCL, 1);
    gpio_set_value(TK_GPIO_SCL, 1);
  }else{
    //gpio_direction_output(TK_GPIO_SCL, 0);
    gpio_set_value(TK_GPIO_SCL, 0);
  }
  return;
  

    /* Place your code to control SCL pin (GPIO) */
	if(gpio_request(TK_GPIO_SCL, "tkey scl")) {
    	dbg_cr("%s : request port error\n", __func__);
    } else {
      //printk("setscl state -> %d\n",state);
    	if (state) { // set scl
        	//gpio_direction_input(TK_GPIO_SCL);
        	gpio_set_value(TK_GPIO_SCL, 1);
        } 
        else { // clear scl
         //   GPIO_SCL_PULL_UPDOWN &=  (unsigned long)(~(GPIO_PULLUP << (GPIO_SCL_PIN * 2)));
         //   GPIO_SCL_PULL_UPDOWN |=  (unsigned long)( (GPIO_PULLUP << (GPIO_SCL_PIN * 2)));
         
        	gpio_direction_output(TK_GPIO_SCL, 0);
        	gpio_set_value(TK_GPIO_SCL, 0);
        }        
    	gpio_free(TK_GPIO_SCL);
    }
}

static inline int getsda(struct tkey *tk)
{
    /* Place your code to control SDA pin (GPIO) */
    int data;
    //data = gpio_get_value(TK_GPIO_SDA);
    //return data;
    
	
    //gpio_direction_input(TK_GPIO_SDA);
    data = gpio_get_value(TK_GPIO_SDA);
    //gpio_direction_output(TK_GPIO_SDA,0);
    return data;
}

static inline int getscl(struct tkey *tk)
{
    /* Place your code to control SCL pin (GPIO) */
    int data;
    //data = gpio_get_value(TK_GPIO_SCL);
    //return data;
    //gpio_direction_input(TK_GPIO_SCL);
    data = gpio_get_value(TK_GPIO_SCL);
    //gpio_direction_output(TK_GPIO_SCL,0);
    return data;
}
#if 0
static void gpio_to_i2c_conf(unsigned char pin)
{
    /* Place your code to change pin Conf. here (SDA/SCL : GPIO to I2C) */
    if(pin == GPIO_SDA_PIN)
    {
        GPIO_I2C_SDA_CON_PORT &=  (unsigned long)(~(0xF << (pin * 4)));    // clear all pins
        GPIO_I2C_SDA_CON_PORT |=  (unsigned long)( (0x2 << (pin * 4)));    // set i2c pin
    }
    else if(pin == GPIO_SCL_PIN)
    {
        GPIO_I2C_SCL_CON_PORT &=  (unsigned long)(~(0xF << (pin * 4)));    
        GPIO_I2C_SCL_CON_PORT |=  (unsigned long)( (0x2 << (pin * 4)));
    }
}
#endif

static void cr_tk_fw_power_onoff(struct tkey *tk, bool onoff)
{
    /* Place Power On/Off Sequence Here */
    if(onoff){
        gpio_set_value(TM_1P8_EN, 1);
        msleep(64);
        gpio_set_value(KEY_3P3_EN, 1);
        dbg_cr("Power On\n");
    }else{
        gpio_set_value(TM_1P8_EN, 0);
        gpio_set_value(KEY_3P3_EN, 0);
        dbg_cr("Power Off\n");
    }
}

static void send_9bit(struct tkey *tk, unsigned char data)
{
    int i;

    setscl(tk, 1);
    setsda(tk, 0);
    setscl(tk, 0);

    for(i = 0; i < 8; i++) {
        setscl(tk, 1);
        setsda(tk, (data>>i)&0x01);
        setscl(tk, 0);
    }
    
    setsda(tk, 0);
}

static unsigned char wait_9bit(struct tkey *tk)
{
    //struct i2c_client *client = tk->client;
    int i;
    int buf;
    unsigned char data = 0;
    gpio_request(TK_GPIO_SDA,"TK_GPIO_SDA");
    gpio_direction_input(TK_GPIO_SDA);
    

    getsda(tk); 
    setscl(tk, 1);
    setscl(tk, 0);

    for(i = 0; i < 8; i++) {
        setscl(tk, 1);
        buf = getsda(tk);
        setscl(tk, 0);
        data |= (buf&0x01)<<i;
    }

    gpio_free(TK_GPIO_SDA);
    set_i2c_from_gpio_function(false);
    return data;
}

static void load(struct tkey *tk, unsigned char data)
{
    send_9bit(tk, LDDATA);
    udelay(TDLY1);
    send_9bit(tk, data);
    udelay(TDLY1);
}

static void step(struct tkey *tk, unsigned char data)
{
    send_9bit(tk, CCFG);
    udelay(TDLY1);
    send_9bit(tk, data);
    udelay(TDLY2);
}

static void setpc(struct tkey *tk, unsigned int addr)
{
    unsigned char buf[4];
    int i;

    buf[0] = 0x02;
    buf[1] = addr>>8;
    buf[2] = addr&0xff;
    buf[3] = 0x00;

    for(i = 0; i < 4; i++)
        step(tk, buf[i]);
}

static void configure_isp(struct tkey *tk)
{
    unsigned char buf[7];
    int i;

    buf[0] = 0x75;    buf[1] = 0xFC;    buf[2] = 0xAC;
    buf[3] = 0x75;    buf[4] = 0xFC;    buf[5] = 0x35;
    buf[6] = 0x00;

    /* Step(cmd) */
    for(i = 0; i < 7; i++)
        step(tk, buf[i]);

    dbg_cr("  configure_isp end\n");  
}

static unsigned char isp_enable_condition(struct tkey *tk)
{
    unsigned char state;

    udelay(TSYNC1);
    send_9bit(tk, CSYNC1);
    udelay(TDLY1);
    send_9bit(tk, CSYNC2);
    udelay(TDLY1);
    send_9bit(tk, CSYNC3);
    udelay(TSYNC3);

    state = wait_9bit(tk);
    dbg_cr("%s : state = %d\n", __func__, state);

    return state;
}

static void reset_for_isp(struct tkey *tk)
{
    /* Place Reset Sequence Here */
    cr_tk_fw_power_onoff(tk, 0);

    
    setscl(tk, 0);
    setsda(tk, 0);
    
    mdelay(POWERON_DELAY);
        
    cr_tk_fw_power_onoff(tk, 1);
    
    usleep_range(5000, 6000);

    #if defined(DEBUG_TKEY)
    dev_info(&tk->client->dev, "%s\n", __func__);
    #endif
}

static int tc300k_erase_fw(struct tkey *tk)
{
    //struct i2c_client *client = tk->client;
    unsigned char data_buf;
    int test=5;

    dbg_cr("%s Start\n", __func__);
    
    /* Enter ISP Mode */
    reset_for_isp(tk);
    udelay(TSYNC1);
   
    if(isp_enable_condition(tk) != 0x01) {
        dbg_cr("ISP enable error\n");
        return -1;
    }

    configure_isp(tk);

    

    /* Full Chip Erase */
    send_9bit(tk, PCRST);
    udelay(TDLY1);
    send_9bit(tk, PECHIP);
    mdelay(TFERASE);

    do {
        if(test<0) break;
        test--;
        udelay(TDLY2);
        send_9bit(tk, CSYNC1);
        udelay(TDLY1);
        send_9bit(tk, CSYNC2);
        udelay(TDLY1);
        send_9bit(tk, CSYNC3);
        udelay(TSYNC3);

        data_buf = wait_9bit(tk);
        dbg_cr(" data_buf -> 0x%x\n",data_buf);
        
    } while((data_buf&0x04) != 0x00);
    dbg_cr("%s -> STATUS : %d\n", __func__, data_buf);
    
    return 0;
}

static int tc300k_write_fw(struct tkey *tk, unsigned int Start_Addr)
{
    struct i2c_client *client = tk->client;
    unsigned int addr, code_size;
    
    dbg_cr("%s Start\n", __func__);

    addr = Start_Addr;
    Start_Addr += ISP_FW_HEADER_LEN;

    setpc(tk, addr);
    load(tk, PWDATA);
    send_9bit(tk, LDMODE);
    udelay(TDLY1);

    code_size = sizeof(CR_TK_FW_TABLE) - Start_Addr; // code_size = CR_TK_FW_TABLE[9]*256 + CR_TK_FW_TABLE[8];
    dbg_cr("\nCODE Size : %d\n", code_size);
    dev_info(&client->dev, "fw code size = 0x%04X", code_size);
    while(addr < code_size) {
        //if((addr % 0x40) == 0)
        //    dbg_cr("\n0x%04X ", addr);

        load(tk, CR_TK_FW_TABLE[Start_Addr + (addr++)]);
        //dbg_cr(".");
        udelay(TPROG);
    }
    dbg_cr("\n");

    send_9bit(tk, PEDISC);
    udelay(TDLY1);

    return 0;
}

static int tc300k_verify_fw(struct tkey *tk, unsigned int Start_Addr)
{
//    struct i2c_client *client = tk->client;
    unsigned int addr, code_size;
    unsigned char code_data;

    dbg_cr("%s Start\n", __func__);

    addr = Start_Addr;
    Start_Addr += ISP_FW_HEADER_LEN;

    setpc(tk, addr);

    code_size = sizeof(CR_TK_FW_TABLE) - Start_Addr;
    while(addr < code_size) {
        //if((addr % 0x40) == 0)
        //    dbg_cr("\n0x%04X ", addr);
            
        send_9bit(tk, PRDATA);
        udelay(TDLY2);
        code_data = wait_9bit(tk);
        udelay(TDLY1);
        //dbg_cr(".");
        if(code_data != CR_TK_FW_TABLE[Start_Addr + (addr++)]) {
            dbg_cr("\nAddr : 0x%04X, Data is not correct (0x%02X)\n", addr-1, code_data);
            return -1;
        }
    }
    dbg_cr("\n");

    return 0;
}

int cr_tk_fw_upgrade(struct tkey *tk)
{
    struct i2c_client *client = tk->client;
    int retries;
    int ret;
    unsigned long flags;

    retries = FW_FLASH_RETRY;

    /* set gpio of scl and sda to gpio function. */   
    set_i2c_from_gpio_function(false);    
    gpio_set_value(TK_GPIO_SCL, 0);
    gpio_set_value(TK_GPIO_SDA, 0);
    mdelay(100);
   
    dbg_cr("--------------------------------------------------------\n");
    dbg_cr("        TSP ISP Upgrade (%s) \n",__func__);
    dbg_cr("--------------------------------------------------------\n");

erase_fw:
    // Full Erase
    local_irq_save(flags);
    
    ret = tc300k_erase_fw(tk);
    if(ret < 0) {
        dev_err(&client->dev, "fail to erase fw (%d)\n", ret);
        if(retries-- > 0) {
            dev_info(&client->dev, "retry erasing fw (%d)\n", retries);
            goto erase_fw;
        } else {
            goto err;
        }
    }
    dev_info(&client->dev, "succeed in erasing fw\n");

    retries = FW_FLASH_RETRY;
//write_fw:
    // FW "cr_tk_fw.i"
    ret = tc300k_write_fw(tk, 0);
    if(ret < 0) {
        dev_err(&client->dev, "fail to write fw (%d)\n", ret);        
        if (retries-- > 0) {
            dev_info(&client->dev, "retry writing fw (%d)\n", retries);
            goto erase_fw;    //write_fw;
        } else {
            goto err;
        }
    }
	dev_info(&client->dev, "succeed in writing fw\n");

    retries = FW_FLASH_RETRY;
//verify_fw:
    ret = tc300k_verify_fw(tk, 0);
    if(ret < 0) {
        dev_err(&client->dev, "fail to verify fw (%d)\n", ret);
        if (retries-- > 0) {
            dev_info(&client->dev, "retry verify fw (%d)\n", retries);
            goto erase_fw;    //write_fw;
        } else {
            goto err;
        }
    }
    dev_info(&client->dev, "succeed in verify fw\n");

    
    /* Power Reset */
    cr_tk_fw_power_onoff(tk, 0);
    mdelay(100);
    cr_tk_fw_power_onoff(tk, 1);
    mdelay(100);

    /* set gpio of scl and sda to qup-i2c function. */       
    set_i2c_from_gpio_function(true);

    local_irq_restore(flags);
	return ret;

err:
/* data is not deallocated for debugging */
    dev_err(&client->dev, "fail to fw flash. driver is removed\n");

    /* GPIO to I2C */
    //gpio_to_i2c_conf(GPIO_SDA_PIN);
    //gpio_to_i2c_conf(GPIO_SCL_PIN);    
    
    local_irq_restore(flags);
    return  ret;
}

#endif

int cr_tk_fw_check(struct tkey *tk) // read my fw ver
{
    int ret;

    ret = CR_TK_FW_TABLE[2];
    //ret = BD_VERSION;
    tk->bd_version = ret;

    return  ret;
}

int cr_tk_fw_probe(struct tkey *tk)
{
    unsigned char buf[I2C_READ_MAX_SIZE]={0};
    int ret;

    dbg_cr("%s\n",__func__);
    
    /* TKey FW Version */
    buf[0] = REG_CMD_FW_VERSION;
    // Ignore when I2C Fail (-6 = No such device or address), Not Exist FW Image
    if(cr_tk_i2c_read(tk->client, buf, 2)<0) {
        tk->fw_version = 0xEE;
        dbg_cr("    TKey has Problem or Blank FW\n");
    } else {
        tk->fw_version = buf[0];
        // TSP FW Problem and bd_version is '0' when I2C Fail (-6)
        dbg_cr("    TKey Version(%02X), INT = %X\n",tk->fw_version, gpio_get_value(TK_GPIO_INT));
    }

    /* Version Compare TSP Board vs FW Image */
    ret = cr_tk_fw_check(tk);
    
#if defined(ISP_UPGRADE)
    if(((ret != tk->fw_version)&&(ret >= 0)) || (tk->fw_version == 0xEE)) {
        /* ISP FW Image Upgrade */
        if(cr_tk_fw_upgrade(tk)<0) 
            return   -1;
        dbg_cr("    FW Upgrade Completed...\n");

        tk->fw_version = CR_TK_FW_TABLE[2];
    }
#endif

    return  0;
}



//[*]------------------------------------------------------------------------[*]
// Touch Key : Interrupt Function
//[*]------------------------------------------------------------------------[*]
irqreturn_t tkey_irq(int irq, void *handle)
{
    struct tkey *tk = handle;
    queue_work(tk->work_queue, &tk->work);  // normal mode (work q used)
    return IRQ_HANDLED;
}

static void tkey_work_q(struct work_struct *work)
{
    struct tkey *tk = container_of(work, struct tkey, work);

    cr_tk_work(tk);
}

//[*]------------------------------------------------------------------------[*]
// Touch Key : First Called Function from Device Driver
//[*]------------------------------------------------------------------------[*]
static void tkey_enable (struct tkey *tk)
{
    dbg_op("%s\n", __func__);
    
    if(tk->disabled) {
        if(tk->irq)
            enable_irq(tk->irq);
        tk->disabled = false;
    }
}
EXPORT_SYMBOL(tkey_enable);


static void tkey_disable (struct tkey *tk)
{
    dbg_op("%s\n", __func__);
    
    if(!tk->disabled) {
        if(tk->irq)
            disable_irq(tk->irq);
        tk->disabled = true;
    }
}
EXPORT_SYMBOL(tkey_disable);


static int tkey_input_open(struct input_dev *input)
{
    struct tkey *tk = input_get_drvdata(input);

    dbg_op("%s\n", __func__);
    
    /* FW Upgrade Check (ISP) */
    //cr_tk_fw_probe(tk);

    cr_tk_enable(tk);

    return 0;
}

static void tkey_input_close(struct input_dev *input)
{
    struct tkey    *tk = input_get_drvdata(input);

    cr_tk_disable(tk);

    dbg_op("%s\n", __func__);
    
}
int tkey_info_display(struct tkey *tk)
{
    dbg_cr("--------------------------------------------------------\n");
    dbg_cr("        TOUCH KEY INFORMATION\n");
    dbg_cr("--------------------------------------------------------\n");
    dbg_cr("TOUCH INPUT Name = %s\n", I2C_TKEY_NAME);
    dbg_cr("TOUCH IRQ Mode   = %s\n", "IRQ_MODE_NORMAL");
    dbg_cr("TOUCH Key/Binary Version = %02X / %02X (%x)\n", tk->fw_version, tk->bd_version, gpio_get_value(TK_GPIO_INT));
    dbg_cr("TOUCH KEY MAX = %d\n", TK_KEY_MAX_CNT);
    dbg_cr("--------------------------------------------------------\n");
    return  0;
}
void init_gpio_cr(void){
  int rc=0;
  dbg_cr("init_gpio_cr\n");
  
  rc=gpio_request_one(KEY_3P3_EN,GPIOF_OUT_INIT_HIGH,"cr-key-3p3");
  if(rc<0)
    dbg_cr("KEY_3P3_EN init is failed\n");
  rc=gpio_request_one(TM_1P8_EN,GPIOF_OUT_INIT_HIGH,"cr-key-1p8");
  if(rc<0)
    dbg_cr("TM_1P8_EN init is failed\n");
  rc=gpio_request_one(TK_GPIO_INT,GPIOF_IN,"cr-key-irq");
   if(rc<0)
    dbg_cr("TK_GPIO_INT init is failed\n");
  
}

void set_i2c_from_gpio_function(bool state)
{
  unsigned gpioConfig;
  int rc;
  if(state){
    //printk("%s : state true\n",__FUNCTION__);
    gpioConfig = GPIO_CFG(TK_GPIO_SDA, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA); // GPIO_CFG_INPUT / GPIO_CFG_NO_PULL
    rc = gpio_tlmm_config(gpioConfig, GPIO_CFG_ENABLE);
    if (rc) {
    	dbg_cr("%s: TK_GPIO_SDA failed (%d)\n",__func__, rc);
    	return;
    }
    gpioConfig = GPIO_CFG(TK_GPIO_SCL, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA); // GPIO_CFG_INPUT / GPIO_CFG_NO_PULL
    rc = gpio_tlmm_config(gpioConfig, GPIO_CFG_ENABLE);
    if (rc) {
    	dbg_cr("%s: TK_GPIO_SCL failed (%d)\n",__func__, rc);
    	return;
    }
    
  }else{
    //printk("%s : state false\n",__FUNCTION__);
    gpioConfig = GPIO_CFG(TK_GPIO_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA); // GPIO_CFG_INPUT / GPIO_CFG_NO_PULL
    rc = gpio_tlmm_config(gpioConfig, GPIO_CFG_ENABLE);
    if (rc) {
    	dbg_cr("%s: TK_GPIO_SDA failed (%d)\n",__func__, rc);
    	return;
    }
    gpioConfig = GPIO_CFG(TK_GPIO_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA); // GPIO_CFG_INPUT / GPIO_CFG_NO_PULL
    rc = gpio_tlmm_config(gpioConfig, GPIO_CFG_ENABLE);
    if (rc) {
    	dbg_cr("%s: TK_GPIO_SCL failed (%d)\n",__func__, rc);
    	return;
    }    
  }
   
}



int tkey_probe (struct i2c_client *client)
{
    int rc = -1;
    int key;
    struct device *dev = &client->dev;
    struct tkey *tk;

    dbg_cr("%s\n", __func__);
    
    /* Memory Allocation for tk data */
    if(!(tk = kzalloc(sizeof(struct tkey), GFP_KERNEL))) {
        kfree(tk);
        dbg_cr("tkey struct malloc error!\n");
        return  -ENOMEM;
    }
    pan_tm = tk;

    /* Linux - I2C Client Device Set */
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dbg_cr("i2c byte data not supported\n");
        return -EIO;
    }
    tk->client  = client;
    i2c_set_clientdata(client, tk);

    /* Linux - Set Device Driver */
    dev_set_drvdata(dev, tk);

    /* Linux - Alloc Device */
    if(!(tk->input  = input_allocate_device()))
        goto err_free_mem;

    snprintf(tk->phys, sizeof(tk->phys), "%s/input0", I2C_TKEY_NAME);

    /* Linux - Set Event */
    tk->input->open         = tkey_input_open;
    tk->input->close        = tkey_input_close;

    tk->input->name         = I2C_TKEY_NAME;
    tk->input->phys         = tk->phys;
    tk->input->dev.parent   = dev;
    tk->input->id.bustype   = BUS_I2C;

    tk->input->id.vendor    = 0x16B4;
    tk->input->id.product   = 0x0702;
    tk->input->id.version   = 0x0001;

    input_set_drvdata(tk->input, tk);

    /* Linux - Key Touch Coordinate Event */
    set_bit(EV_KEY, tk->input->evbit);

    for(key = 0; key < TK_KEY_COUNT_MAX; key++) {
        if(cr_tk_keycode[key] <= 0) continue;
        set_bit(cr_tk_keycode[key] & KEY_MAX, tk->input->keybit);
    }

    /* Linux - Register Device */
    if ((rc = input_register_device(tk->input))) {
        dev_err(dev, "(%s) input register fail!\n", I2C_TKEY_NAME);
        goto err_free_input_mem;
    }

    /* init gpio config */
    init_gpio_cr();
    
    /* Linux - FW Upgrade Check (ISP) */
    
    if((rc = cr_tk_fw_probe(tk)) < 0)
        goto    err_free_all;
    //cr_tk_fw_gpio(TK_GPIO_INT, 0, 1, 2); p13106 temp

    /* Linux - Display TKey Information */
    tkey_info_display(tk);

    /* Linux - Set Power Save Function */
#if defined(CONFIG_HAS_EARLYSUSPEND_CR)
    tk->power.resume    = tkey_resume;
    tk->power.suspend   = tkey_suspend;
    tk->power.level     = EARLY_SUSPEND_LEVEL_DISABLE_FB-1;

    //if, is in USER_SLEEP status and no active auto expiring wake lock
    register_early_suspend(&tk->power);
#endif

    /* Linux - Register ISR for Interupt */
    tk->irq = gpio_to_irq(TK_GPIO_INT);

    if(tk->irq) {
        //INIT_WORK(&tk->work, tkey_work_q);
        if((tk->work_queue = create_singlethread_workqueue("tm_work_queue")) == NULL)
            goto err_free_irq;
        INIT_WORK(&tk->work, tkey_work_q);

        #if defined(TK_INT_ACTIVE_HIGH_EDGE)
        if((rc = request_irq(tk->irq, tkey_irq, IRQF_TRIGGER_RISING|IRQF_DISABLED, I2C_TKEY_NAME, tk)))
        #else
        if((rc = request_irq(tk->irq, tkey_irq, IRQF_TRIGGER_FALLING|IRQF_DISABLED, I2C_TKEY_NAME, tk)))
        #endif
        {
            dbg_cr("irq %d request fail!\n", tk->irq);
            goto err_free_irq;
        }

        //disable_irq_nosync(tk->irq);
    }
    tk->disabled = false;

#ifdef PAN_TM_LED_OPERATION
    // init ledclass device 
    tk->tm_led_cdev.name = "pan_tm_led";
    tk->tm_led_cdev.brightness=0;
    tk->tm_led_cdev.brightness_set=pan_tm_set_led_onoff;
    rc = led_classdev_register(dev, &tk->tm_led_cdev);
    if (rc < 0) {
        dbg_cr("%s: couldn't register TM LED device\n", __func__);
        return rc;
    }
#endif    
    tk->threshold_normal = TK_KEY_THRESHOLD_DEFAULT_NORMAL;
    tk->threshold_globe  = TK_KEY_THRESHOLD_DEFAULT_GLOVE;
    
    // register set 
    rc = misc_register(&tm_event);
	  if (rc) {
		  dbg_cr("tm_event can''t register misc device\n");
	  }

    tk->state = APPMODE;
    tk->mode = 0;
    tk->cover_state = 0;
    
    return 0;
    
	err_free_all:
	
	err_free_irq:
    free_irq(tk->irq, tk);
    /* Linux - Un-Register Device */
    input_unregister_device(tk->input);
    dbg_cr("%s	err_free_irq\n", __func__);
    err_free_input_mem:
    /* Linux - Free Device */
    input_free_device(tk->input);
    tk->input = NULL;
    dbg_cr("%s  err_free_input_mem\n", __func__);
    err_free_mem:
    kfree(tk);              
    tk = NULL;
    dbg_cr("%s  err_free_mem\n", __func__);
    pan_tm=NULL;
    return rc;
    
}



int tkey_remove	(struct device *dev)
{
	struct tkey *tk = dev_get_drvdata(dev);
  int rc;
  dbg_cr("%s\n", __func__);
	if(tk->irq)                 
    free_irq(tk->irq, tk);

  input_unregister_device(tk->input);
  dev_set_drvdata(dev, NULL);

  rc = misc_deregister(&tm_event);
  if (rc) {
	  dbg_cr("tm_event can''t deregister misc device\n");
  }
#ifdef PAN_TM_LED_OPERATION
  led_classdev_unregister(&tk->tm_led_cdev);
#endif
  kfree(tk);

	return 0;
}
#ifdef CONFIG_KEYBOARD_TC370_SLEEP
#ifdef CONFIG_HAS_EARLYSUSPEND_CR
static void tkey_resume (struct early_suspend *h)
{
    struct tkey *tk = container_of(h, struct tkey, power);
    tkey_enable(tk);
}
static void tkey_suspend (struct early_suspend *h)
{
    struct tkey *tk = container_of(h, struct tkey, power);
    tkey_disable(tk);
}
#endif


#ifdef CONFIG_KEYBOARD_TC370
static int tkey_i2c_resume (struct i2c_client *client)
{
    struct tkey *tk = i2c_get_clientdata(client);
    int rc;

    dbg_cr("%s\n",__FUNCTION__);

    // check resume.
    return 0;
    
    // enable tm key irq.
    tkey_enable(tk);
#ifdef PAN_TM_LED_OPERATION
    // set pan_tm_led on state.
    rc=i2c_smbus_write_byte_data(tk->client, 0, 0x10);
    if(rc)
      dbg_cr("TM i2c_smbus_write_byte_data rc -> %d\n",rc);
#endif      
    
    
    return 0;
}

static int tkey_i2c_suspend (struct i2c_client *client, pm_message_t message)
{
    struct tkey *tk = i2c_get_clientdata(client);
    int rc;

    dbg_cr("%s\n",__FUNCTION__);

    
    // check suspend.
    return 0;

    // disable tm key irq.
    tkey_disable(tk);
#ifdef PAN_TM_LED_OPERATION

     // set pan_tm_led off state.
    rc=i2c_smbus_write_byte_data(tk->client, 0, 0x20);
    if(rc)
      dbg_cr("TM i2c_smbus_write_byte_data rc -> %d\n",rc);

#endif    
    return 0;
}



#elif CONFIG_PM
static int tkey_i2c_resume (struct i2c_client *client)
{
    #ifndef CONFIG_HAS_EARLYSUSPEND_CR
        struct tkey *tk = i2c_get_clientdata(client);

        tkey_enable(tk);
    #endif

    return 0;
}

static int tkey_i2c_suspend (struct i2c_client *client, pm_message_t message)
{
    #ifndef CONFIG_HAS_EARLYSUSPEND_CR
        struct tkey *tk = i2c_get_clientdata(client);

        tkey_disable(tk);
    #endif

    return 0;
}
#else
    #define tkey_i2c_resume    NULL
    #define tkey_i2c_suspend   NULL
#endif

#else
// suspend & resume func of pan_tm_key driver is called for stmicro touch ic sus & resum func().
int pan_tm_key_resume (void)
{
    int rc = 0;

    dbg_cr("%s\n",__FUNCTION__);
    
    if(pan_tm == NULL)
        return 0;

    if(pan_tm->state == APPMODE) {
        dbg_cr("%s : TM state is already APPMODE\n", __func__);
        return 0;
    }

    if(pan_tm->cover_state == 1) {
        dbg_cr("%s : cover state is closed\n", __func__);
        return 0;
    }

    // set gpio mode to qup i2c
    set_i2c_from_gpio_function(true);
    msleep(10);
    cr_tk_fw_power_onoff(pan_tm, 1);
    msleep(100);
        
#ifdef PAN_TM_LED_OPERATION
    if(pan_tm->tm_led_brightness) {
        rc = i2c_smbus_write_byte_data(pan_tm->client, 0, TK_KEY_LED_ON);
        if(rc < 0) {
            dbg_cr("%s : failed to set TM_LED ON, error = %d\n", __func__, rc);
            return rc;
        } else {
            dbg_cr("%s : set TM_LED ON\n", __func__);
        }
        msleep(20);
    }
#endif
    
    // set APPMODE
    pan_tm->state = APPMODE;

    // set mode
    pan_tm_set_mode(pan_tm->mode);
    
    // enable tm key irq.
    tkey_enable(pan_tm);
    return 0;
}
EXPORT_SYMBOL(pan_tm_key_resume);

int pan_tm_key_suspend (void)
{
    int rc = 0;

    dbg_cr("%s\n",__FUNCTION__);

    if(pan_tm == NULL)
        return 0;

    if(pan_tm->state == SUSMODE) {
        dbg_cr("%s : TM state is already SUSMODE\n", __func__);
        return 0;
    }

    // disable tm key irq.
    tkey_disable(pan_tm);

#ifdef PAN_TM_LED_OPERATION
    if(pan_tm->tm_led_brightness) {
        rc = i2c_smbus_write_byte_data(pan_tm->client, 0, TK_KEY_LED_OFF);
        if(rc < 0) {
            dbg_cr("%s : failed to set TM_LED OFF, error = %d\n", __func__, rc);
            return rc;
        } else {
            dbg_cr("%s : set TM_LED OFF\n", __func__);
        }
         pan_tm->tm_led_brightness = 0; 
    }
#endif
    // set qup i2c to gpio mode
    set_i2c_from_gpio_function(false);
    msleep(10);
    gpio_set_value(TK_GPIO_SCL, 0);
    gpio_set_value(TK_GPIO_SCL, 0);
    
    // power off
    cr_tk_fw_power_onoff(pan_tm, 0);
    pan_tm->state = SUSMODE;
    return 0;
}
EXPORT_SYMBOL(pan_tm_key_suspend);
#endif

static int __devinit tkey_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    dbg_cr("%s\n", __func__);
    return  tkey_probe(client);
}

static int __devexit tkey_i2c_remove(struct i2c_client *client)
{
    printk("%s\n", __func__);
    return  tkey_remove(&client->dev);
}

static const struct i2c_device_id touch_id[] = {
    { I2C_TKEY_NAME, 0 },
    { }
};

static struct of_device_id cr_match_table[] = {
	{ .compatible = "coreriver,tk-300k",},
	{ },
};


MODULE_DEVICE_TABLE(i2c, touch_id);

static struct i2c_driver tkey_i2c_driver = {
    .driver = {
        .name   = I2C_TKEY_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = cr_match_table,
    },
    .probe      = tkey_i2c_probe,
    .remove     = __devexit_p(tkey_i2c_remove),
#ifdef CONFIG_KEYBOARD_TC370_SLLEP
// suspend & resume func is called from stmicro touch driver.    
    .suspend    = tkey_i2c_suspend,
    .resume     = tkey_i2c_resume,
#endif
    .id_table   = touch_id,
};

static int __init tkey_i2c_init(void)
{
    dbg_cr("%s\n", __func__);    
    return i2c_add_driver(&tkey_i2c_driver);
}

static void __exit tkey_i2c_exit(void)
{
    i2c_del_driver(&tkey_i2c_driver);
    dbg_cr("%s\n", __func__);    
}

module_init(tkey_i2c_init);
module_exit(tkey_i2c_exit);

//[*]------------------------------------------------------------------------[*]
MODULE_AUTHOR("HardKernel Co., Ltd.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TouchKey Driver");
MODULE_ALIAS("i2c:tkey");
//[*]------------------------------------------------------------------------[*]

