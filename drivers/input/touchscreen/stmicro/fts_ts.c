/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
 *
 * File Name		: fts.c
 * Authors		: AMS(Analog Mems Sensor) Team
 * Description	: FTS Capacitive touch screen controller (FingerTipS)
 *
 ********************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
 * PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
 ********************************************************************************
 * REVISON HISTORY
 * DATE		| DESCRIPTION
 * 03/09/2012| First Release
 * 08/11/2012| Code migration
 * 23/01/2013| SEC Factory Test
 * 29/01/2013| Support Hover Events
 * 08/04/2013| SEC Factory Test Add more - hover_enable, glove_mode, clear_cover_mode, fast_glove_mode
 * 09/04/2013| Support Blob Information
 *******************************************************************************/

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/serio.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/ctype.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/power_supply.h>
#include <linux/firmware.h>
#include <linux/regulator/consumer.h>
#include <linux/miscdevice.h>

#include <linux/input/mt.h>
//++ p11309 - 2014.01.08 for Offline boot process 
#include <mach/msm_smsm.h>
//-- p11309
#include "fts.h"
#include "fts_ts.h"

struct fts_ts_info *shared_info = NULL;

#ifdef CONFIG_POWERSUSPEND
#include <linux/powersuspend.h>
static int fts_suspend(struct fts_ts_info *info);
static int fts_resume(struct fts_ts_info *info);

static void fts_early_suspend(struct power_suspend *h)
{
	fts_suspend(shared_info);
}
static void fts_late_resume(struct power_suspend *h)
{
	fts_resume(shared_info);
}
static struct power_suspend fts_power_suspend = {
	.suspend = fts_early_suspend,
	.resume = fts_late_resume,
};
#endif

//++ p11309 - 2013.12.22 
enum tsp_power_pin_type {
	POWER_NOT_USED=0,
	POWER_GPIO_SETUP,	// gpio setup
	POWER_PM_REGULATOR,	// PMIC regulator setup
};

enum gpio_direction {	
	GPIO_OUTPUT_LOW=0,	// out port, default low
	GPIO_OUTPUT_HIGH,	// out port, default high
	GPIO_INPUT,			// in port
};

enum power_up_down {	
	POWER_DOWN=0,
	POWER_UP,
};

#ifdef PAN_TSP_IO
static struct file_operations ts_fops = {
	.owner = THIS_MODULE,
	.open = ts_fops_open,
	.unlocked_ioctl = ts_fops_ioctl,
};

static struct miscdevice touch_event = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch_fops",
	.fops = &ts_fops,
};

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.unlocked_ioctl =    ioctl,
	.read =     read,
	.write =    write,
	.open =     open,
	.release =  release,
};

static struct miscdevice touch_io = 
{
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "touch_monitor",
	.fops =     &fops
};
#endif

#define GPIO_TOUCH_CHG			61
#define GPIO_TOUCH_AVDD 		94
#define GPIO_TOUCH_DVDD 		96

struct fts_callbacks *fts_charger_callbacks;
static void fts_tsp_register_callback(void *cb);
static int fts_power(bool enable);
static void fts_reinit(struct fts_ts_info *info);
static void fts_delay(unsigned int ms);


#ifdef TSP_FACTORY_TEST
struct class *pan_class;
#endif

#ifdef PAN_IRQ_WORKQUEUE
static struct workqueue_struct *fts_event_wq;
#endif

static struct i2c_driver fts_i2c_driver;
static bool MutualTouchMode = false;

#ifdef CONFIG_GLOVE_TOUCH
enum TOUCH_MODE {
	FTS_TM_NORMAL = 0,
	FTS_TM_GLOVE,
};
#endif

int pan_debug_state = PAN_TOUCH_DEBUG_OPERATION_ERR_MASK;
int touch_error_cnt=0;
char* touch_error_info=NULL;


static int fts_stop_device(struct fts_ts_info *info);
static int fts_start_device(struct fts_ts_info *info);

int fts_wait_for_ready(struct fts_ts_info *info);

int fts_write_reg(struct fts_ts_info *info,
		unsigned char *reg, unsigned short num_com)
{
	struct i2c_msg xfer_msg[2];

	if (info->touch_stopped) {
		dbg_cr("%s: Sensor stopped\n", __func__);
		goto exit;
	}

	xfer_msg[0].addr = info->client->addr;
	xfer_msg[0].len = num_com;
	xfer_msg[0].flags = 0;
	xfer_msg[0].buf = reg;

	return i2c_transfer(info->client->adapter, xfer_msg, 1);

exit:
	return 0;
}

int fts_read_reg(struct fts_ts_info *info, unsigned char *reg, int cnum,unsigned char *buf, int num)
{
	struct i2c_msg xfer_msg[2];

	if (info->touch_stopped) {
		dbg_cr("%s: Sensor stopped\n", __func__);
		goto exit;
	}

	xfer_msg[0].addr = info->client->addr;
	xfer_msg[0].len = cnum;
	xfer_msg[0].flags = 0;
	xfer_msg[0].buf = reg;

	xfer_msg[1].addr = info->client->addr;
	xfer_msg[1].len = num;
	xfer_msg[1].flags = I2C_M_RD;
	xfer_msg[1].buf = buf;

	return i2c_transfer(info->client->adapter, xfer_msg, 2);
exit:
	return 0;
}
#ifdef PAN_TOUCH_SET_PEN_MODE
void fts_stylus_mode_onoff(struct fts_ts_info *info, unsigned char onoff)
{
	unsigned char regAdd[4];

	regAdd[0] = 0x90;                    // sleep in

	fts_write_reg(info, regAdd, 1);
	fts_delay(10);    // Delay 10ms

	dbg_cr("fts_stylus_mode_onoff -> %d\n",onoff);
	///////////////////////////////////////////////////////////////////////////////////////
	// Set Stylus mode off.
	regAdd[0] = 0xB0;
	regAdd[1] = 0x00;
	regAdd[2] = 0xA0;

	if(onoff == 1) 
		regAdd[3] = onoff | 0x02;             // When 1=>Stylus on.
	else
		regAdd[3] = onoff;                    // When 0=>Stylus off.

	fts_write_reg(info, regAdd, 4);

	// Set fgr_thres for stylus detection control.
	regAdd[0] = 0xB0;
	regAdd[1] = 0x00;
	regAdd[2] = 0x62;				 // Set lower address.

	if(onoff == 1) {				 // When 1=>Stylus on.
		regAdd[3] = 0xC8;                    // set lower 1 byte value(0xC8)
		fts_write_reg(info, regAdd, 4);
		regAdd[2] = 0x63;                    // set higher address
		regAdd[3] = 0x00;                    // set upper 1 byte value.(0x00C8 =200)
		fts_write_reg(info, regAdd, 4);
	}
	else {					 // When 0=>Stylus off.
		regAdd[3] = 0x54;                    // set lower 1 byte value(0x54)
		fts_write_reg(info, regAdd, 4);
		regAdd[2] = 0x63;                    // set higher address
		regAdd[3] = 0x01;                    // set upper 1 byte value.(0x0154 =340)
		fts_write_reg(info, regAdd, 4);
	}

	// Change motion_tol X for finger only mode / pen mode.
	regAdd[0] = 0xB0;
	regAdd[1] = 0x00;
	regAdd[2] = 0x86;

	if(onoff == 1) 
		regAdd[3] = 8;             		// When Stylus on mode, set motion_tol X.
	else
		regAdd[3] = 0x0C;                  	// When Stylus off mode, set motion_tol X.

	fts_write_reg(info, regAdd, 4);


	// Change motion_tol Y for finger only mode / pen mode.
	regAdd[0] = 0xB0;
	regAdd[1] = 0x00;
	regAdd[2] = 0x97;

	if(onoff == 1) 
		regAdd[3] = 8;             		// When Stylus on mode, set motion_tol y.
	else
		regAdd[3] = 0x12;                  	// When Stylus off mode, set motion_tol y.

	fts_write_reg(info, regAdd, 4);

	// To apply setting more stable
	regAdd[0] = 0x91;                     // Sleep OUT
	fts_write_reg(info, regAdd, 1);
	fts_delay(10);    // Delay 10ms
	regAdd[0] = 0x93;                     // Sense ON
	fts_write_reg(info, regAdd, 1);
	fts_delay(10);    // Delay 10ms
	regAdd[0] = 0xA2;                     // Force cal.
	fts_write_reg(info, regAdd, 1);
	fts_delay(10);    // Delay 10ms

}

#endif

static void fts_delay(unsigned int ms)
{
	if (ms < 20)
		mdelay(ms);
	else
		msleep(ms);
}

void fts_command(struct fts_ts_info *info, unsigned char cmd)
{
	unsigned char regAdd = 0;
	int ret = 0;

	regAdd = cmd;
	ret = fts_write_reg(info, &regAdd, 1);
	dbg_op("FTS Command (%02X) , ret = %d \n", cmd, ret);
}

void fts_systemreset(struct fts_ts_info *info)
{
	unsigned char regAdd[4] = { 0xB6, 0x00, 0x23, 0x01 };
	dbg_op("FTS SystemReset\n");
	fts_write_reg(info, &regAdd[0], 4);
	fts_delay(10);
}

static void fts_interrupt_set(struct fts_ts_info *info, int enable)
{
	unsigned char regAdd[4] = { 0xB6, 0x00, 0x1C, enable };

	dbg_op("%s: FTS INT %s\n", __func__, enable ? "enable" : "disabled");	

	fts_write_reg(info, &regAdd[0], 4);
}

static void fts_set_stylus_mode(struct fts_ts_info *info, bool enable)
{
	if (enable)
		fts_command(info, FTS_CMD_STYLUS_ON);
	else
		fts_command(info, FTS_CMD_STYLUS_OFF);
}

int fts_wait_for_ready(struct fts_ts_info *info)
{
	int rc;
	unsigned char regAdd;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	memset(data, 0x0, FTS_EVENT_SIZE);

	regAdd = READ_ONE_EVENT;
	rc = -1;
	while (fts_read_reg
			(info, &regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if (data[0] == EVENTID_CONTROLLER_READY) {
			rc = 0;
			break;
		}

		if (!(info->mflag_init_test) && data[0] == EVENTID_ERROR) {
			rc = -2;
			break;
		}

		if (retry++ > 30) {
			rc = -1;
			dbg_cr("%s: Time Over\n", __func__);
			break;
		}
		fts_delay(10);
	}

	return rc;
}

int fts_get_version_info(struct fts_ts_info *info)
{
	int rc;
	unsigned char regAdd[3];
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	fts_command(info, FTS_CMD_RELEASEINFO);

	memset(data, 0x0, FTS_EVENT_SIZE);

	regAdd[0] = READ_ONE_EVENT;
	rc = -1;
	while (fts_read_reg(info, &regAdd[0], 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if (data[0] == EVENTID_INTERNAL_RELEASE_INFO) {
			// Internal release Information
			info->fw_version_of_ic = (data[3] << 8) + data[4];
			info->config_version_of_ic = (data[6] << 8) + data[5];
		} else if (data[0] == EVENTID_EXTERNAL_RELEASE_INFO) {
			// External release Information
			info->fw_main_version_of_ic = (data[1] << 8)+data[2];
			rc = 0;
			break;
		}

		if (retry++ > 30) {
			rc = -1;
			dbg_cr("%s: Time Over\n", __func__);
			break;
		}
	}

	dbg_op("IC Firmware Version : 0x%04X "
			"IC Config Version : 0x%04X "
			"IC Main Version : 0x%04X\n",
			info->fw_version_of_ic,
			info->config_version_of_ic,
			info->fw_main_version_of_ic);

	return rc;
}

#ifdef FTS_SUPPORT_NOISE_PARAM
static int fts_get_noise_param_address(struct fts_ts_info *info)
{
	int rc;
	unsigned char regAdd[3];
	struct fts_noise_param *noise_param;
	int i;

	noise_param = (struct fts_noise_param *)&info->noise_param;

	regAdd[0] = 0xd0;
	regAdd[1] = 0x00;
	regAdd[2] = 32 * 2;
	rc = fts_read_reg(info, regAdd, 3, (unsigned char *)noise_param->pAddr,
			2);

	for (i = 1; i < MAX_NOISE_PARAM; i++) {
		noise_param->pAddr[i] = noise_param->pAddr[0] + i * 2;
	}

	for (i = 0; i < MAX_NOISE_PARAM; i++) {
		dbg_op("Get Noise Param%d Address = 0x%4x\n", i, noise_param->pAddr[i]);
	}

	return rc;
}

static int fts_get_noise_param(struct fts_ts_info *info)
{
	int rc;
	unsigned char regAdd[3];
	unsigned char data[MAX_NOISE_PARAM * 2];
	struct fts_noise_param *noise_param;
	int i;
	unsigned char buf[3];

	noise_param = (struct fts_noise_param *)&info->noise_param;
	memset(data, 0x0, MAX_NOISE_PARAM * 2);

	for (i = 0; i < MAX_NOISE_PARAM; i++) {
		regAdd[0] = 0xb3;
		regAdd[1] = 0x00;
		regAdd[2] = 0x10;
		fts_write_reg(info, regAdd, 3);

		regAdd[0] = 0xb1;
		regAdd[1] = (noise_param->pAddr[i] >> 8) & 0xff;
		regAdd[2] = noise_param->pAddr[i] & 0xff;
		rc = fts_read_reg(info, regAdd, 3, &buf[0], 3);

		noise_param->pData[i] = buf[1]+(buf[2]<<8);
		dbg_op("0x%2x%2x%2x 0x%2x 0x%2x\n", 
				regAdd[0],regAdd[1],regAdd[2], buf[1], buf[2]);
	}

	for (i = 0; i < MAX_NOISE_PARAM; i++) {
		dbg_op("Get Noise Param%d Address [ 0x%04x ] = 0x%04x\n", i,
				noise_param->pAddr[i], noise_param->pData[i]);
	}

	return rc;
}

static int fts_set_noise_param(struct fts_ts_info *info)
{
	int i;
	unsigned char regAdd[5];
	struct fts_noise_param *noise_param;

	noise_param = (struct fts_noise_param *)&info->noise_param;

	for (i = 0; i < MAX_NOISE_PARAM; i++) {
		regAdd[0] = 0xb3;
		regAdd[1] = 0x00;
		regAdd[2] = 0x10;
		fts_write_reg(info, regAdd, 3);

		regAdd[0] = 0xb1;
		regAdd[1] = (noise_param->pAddr[i] >> 8) & 0xff;
		regAdd[2] = noise_param->pAddr[i] & 0xff;
		regAdd[3] = noise_param->pData[i] & 0xff;
		regAdd[4] = (noise_param->pData[i] >> 8) & 0xff;
		fts_write_reg(info, regAdd, 5);
	}

	for (i = 0; i < MAX_NOISE_PARAM; i++) {
		dbg_op("Set Noise Param%d Address [ 0x%04x ] = 0x%04x\n", i,
				noise_param->pAddr[i], noise_param->pData[i]);
	}

	return 0;
}
#endif				// FTS_SUPPORT_NOISE_PARAM

#ifdef TSP_BOOSTER
static void fts_change_dvfs_lock(struct work_struct *work)
{
	struct fts_ts_info *info = container_of(work,
			struct fts_ts_info,
			work_dvfs_chg.work);

	mutex_lock(&info->dvfs_lock);

	switch (info->boost_level) {
	case TSP_BOOSTER_LEVEL1:
	case TSP_BOOSTER_LEVEL3:
		if (pm_qos_request_active(&info->tsp_cpu_qos))
			pm_qos_remove_request(&info->tsp_cpu_qos);

		if (pm_qos_request_active(&info->tsp_mif_qos))
			pm_qos_remove_request(&info->tsp_mif_qos);

		if (pm_qos_request_active(&info->tsp_int_qos))
			pm_qos_remove_request(&info->tsp_int_qos);

		dbg_cr("%s: [TSP_DVFS] level[%d] : DVFS OFF\n",
				__func__, info->boost_level);
		break;
	case TSP_BOOSTER_LEVEL2:
		if (pm_qos_request_active(&info->tsp_cpu_qos))
			pm_qos_update_request(&info->tsp_cpu_qos, TOUCH_BOOSTER_CPU_FRQ_2);

		if (pm_qos_request_active(&info->tsp_mif_qos))
			pm_qos_update_request(&info->tsp_mif_qos, TOUCH_BOOSTER_MIF_FRQ_2);

		if (pm_qos_request_active(&info->tsp_int_qos))
			pm_qos_update_request(&info->tsp_int_qos, TOUCH_BOOSTER_INT_FRQ_2);

		dbg_cr("%s: [TSP_DVFS] level[%d] : DVFS CHANGED\n",
				__func__, info->boost_level);
		break;
	default:
		dbg_cr("%s: [TSP_DVFS] Undefined type passed %d\n",
				__func__, info->boost_level);
		break;
	}

	mutex_unlock(&info->dvfs_lock);
}

static void fts_set_dvfs_off(struct work_struct *work)
{
	struct fts_ts_info *info = container_of(work,
			struct fts_ts_info,
			work_dvfs_off.work);

	mutex_lock(&info->dvfs_lock);

	if (pm_qos_request_active(&info->tsp_cpu_qos))
		pm_qos_remove_request(&info->tsp_cpu_qos);

	if (pm_qos_request_active(&info->tsp_mif_qos))
		pm_qos_remove_request(&info->tsp_mif_qos);

	if (pm_qos_request_active(&info->tsp_int_qos))
		pm_qos_remove_request(&info->tsp_int_qos);

	info->dvfs_lock_status = false;
	mutex_unlock(&info->dvfs_lock);

	dbg_cr( "%s: [TSP_DVFS] level[%d] : DVFS OFF\n",
			__func__, info->boost_level);
}

static void fts_init_dvfs_level(struct fts_ts_info *info)
{
	/* DVFS level is depend on booster_level which is writed by sysfs
	 * booster_level : 1	(press)CPU 1300000, MIF 413000, INT 266000 -> after 130msec -> OFF
	 * booster_level : 2	(press)CPU 1300000, MIF 413000, INT 266000 -> after 130msec
	 *					-> CPU 1300000, MIF 206000, INT 160000 -> after 500msec -> OFF
	 * booster_level : 3	(press)CPU 1300000, MIF 206000, INT 160000 -> after 130msec -> OFF
	 */
	unsigned int level = info->boost_level;

	switch (level) {
	case TSP_BOOSTER_LEVEL1:
	case TSP_BOOSTER_LEVEL2:
		if (pm_qos_request_active(&info->tsp_cpu_qos))
			pm_qos_update_request(&info->tsp_cpu_qos, TOUCH_BOOSTER_CPU_FRQ_1);
		else
			pm_qos_add_request(&info->tsp_cpu_qos, PM_QOS_KFC_FREQ_MIN, TOUCH_BOOSTER_CPU_FRQ_1);

		if (pm_qos_request_active(&info->tsp_mif_qos))
			pm_qos_update_request(&info->tsp_mif_qos, TOUCH_BOOSTER_MIF_FRQ_1);
		else
			pm_qos_add_request(&info->tsp_mif_qos, PM_QOS_BUS_THROUGHPUT, TOUCH_BOOSTER_MIF_FRQ_1);

		if (pm_qos_request_active(&info->tsp_int_qos))
			pm_qos_update_request(&info->tsp_int_qos, TOUCH_BOOSTER_INT_FRQ_1);
		else
			pm_qos_add_request(&info->tsp_int_qos, PM_QOS_DEVICE_THROUGHPUT, TOUCH_BOOSTER_INT_FRQ_1);
		break;
	case TSP_BOOSTER_LEVEL3:
		if (pm_qos_request_active(&info->tsp_cpu_qos))
			pm_qos_update_request(&info->tsp_cpu_qos, TOUCH_BOOSTER_CPU_FRQ_3);
		else
			pm_qos_add_request(&info->tsp_cpu_qos, PM_QOS_KFC_FREQ_MIN, TOUCH_BOOSTER_CPU_FRQ_3);

		if (pm_qos_request_active(&info->tsp_mif_qos))
			pm_qos_update_request(&info->tsp_mif_qos, TOUCH_BOOSTER_MIF_FRQ_3);
		else
			pm_qos_add_request(&info->tsp_mif_qos, PM_QOS_BUS_THROUGHPUT, TOUCH_BOOSTER_MIF_FRQ_3);

		if (pm_qos_request_active(&info->tsp_int_qos))
			pm_qos_update_request(&info->tsp_int_qos, TOUCH_BOOSTER_INT_FRQ_3);
		else
			pm_qos_add_request(&info->tsp_int_qos, PM_QOS_DEVICE_THROUGHPUT, TOUCH_BOOSTER_INT_FRQ_3);
		break;
	default:
		dbg_cr("%s: [TSP_DVFS] Undefined type passed %d\n",
				__func__, level);
		break;
	}
}

static void fts_set_dvfs_lock(struct fts_ts_info *info, unsigned int mode, bool restart)
{
	if (info->boost_level == TSP_BOOSTER_DISABLE)
		return;

	mutex_lock(&info->dvfs_lock);

	switch (mode) {
	case TSP_BOOSTER_OFF:
		if (info->dvfs_lock_status) {
			schedule_delayed_work(&info->work_dvfs_off,
					msecs_to_jiffies(TOUCH_BOOSTER_OFF_TIME));
		}
		break;
	case TSP_BOOSTER_ON:
		cancel_delayed_work(&info->work_dvfs_off);

		if (!info->dvfs_lock_status || restart) {
			cancel_delayed_work(&info->work_dvfs_chg);
			fts_init_dvfs_level(info);

			schedule_delayed_work(&info->work_dvfs_chg,
					msecs_to_jiffies(TOUCH_BOOSTER_CHG_TIME));

			dbg_cr( "%s: [TSP_DVFS] level[%d] : DVFS ON\n",
					__func__, info->boost_level);

			info->dvfs_lock_status = true;
		}
		break;
	case TSP_BOOSTER_FORCE_OFF:
		if (info->dvfs_lock_status) {
			cancel_delayed_work(&info->work_dvfs_off);
			cancel_delayed_work(&info->work_dvfs_chg);
			schedule_work(&info->work_dvfs_off.work);
		}
		break;
	default:
		break;
	}
	mutex_unlock(&info->dvfs_lock);
}

static int fts_init_dvfs(struct fts_ts_info *info)
{
	mutex_init(&info->dvfs_lock);

	INIT_DELAYED_WORK(&info->work_dvfs_off, fts_set_dvfs_off);
	INIT_DELAYED_WORK(&info->work_dvfs_chg, fts_change_dvfs_lock);

	info->dvfs_lock_status = false;
	return 0;
}
#endif

#ifdef FTS_PANTECH_PRODUCT_LINE
int fts_check_product_line(struct fts_ts_info *info)
{
	int rc;
	unsigned char regAdd[4], oneAdd;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0, cnt = 0;

	regAdd[0] = 0xB2;
	regAdd[1] = 0x07;
	regAdd[2] = 0xE2;
	regAdd[3] = 0x04;

	fts_write_reg(info, regAdd, 4);

	oneAdd = READ_ONE_EVENT;
	rc = -1;
	while (fts_read_reg(info, &oneAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if ((data[0] == 0x12)&&(data[1]==regAdd[1])&&(data[2]==regAdd[2])) {
			/*
			   if(data[3]==0x08){
			   dbg_cr("%s: Already done auto tune\n", __func__);
			   rc = 0;
			   break;
			   }
			   else{*/
			dbg_cr( "%s: Starting auto tune\n", __func__);

			fts_command(info, SLEEPOUT);
			msleep(100);
			fts_command(info, CX_TUNNING);
			msleep(300);
			cnt = 300;

			//waiting auto tune result 
			while(cnt--){
				fts_read_reg(info, &oneAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE);
				msleep(50);

				if((data[0]==0x16)&&(data[1]==0x0B)&&(data[2]==0x03)){
					dbg_cr( "%s: Auto tune OK\n", __func__);

					regAdd[0] = 0xB0;
					regAdd[1] = 0x07;
					regAdd[2] = 0xE2;
					regAdd[3] = 0x08;

					fts_write_reg(info, regAdd, 4);
					msleep(100);
					fts_command(info, FTS_CMD_SAVE_FWCONFIG);
					msleep(200);
					fts_command(info, FTS_CMD_SAVE_CX_TUNING);
					// Reset FTS
					//fts_systemreset(info);
					//		fts_wait_for_ready(info);
					msleep(200);
					rc = 0;
					break;
				}
			}

			if(cnt < 1){
				dbg_cr("%s: Auto tune Fail\n", __func__);
				rc = -1;
				return rc;
			}
			else
				break;
		}
		//}

		if (retry++ > 30) {
			rc = -1;
			dbg_cr( "%s: Time Over\n", __func__);
			break;
		}

	}


	return rc;
}
#endif /* FTS_PANTECH_PRODUCT_LINE */

/* Added for samsung dependent codes such as Factory test,
 * Touch booster, Related debug sysfs.
 */
#include "fts_pantech.c"

static int fts_init(struct fts_ts_info *info)
{
	unsigned char val[16];
	unsigned char regAdd[8];
	int rc;

	dbg_cr("FTS Initial Start..\n");

	fts_delay(300);

	// TS Chip ID
	regAdd[0] = 0xB6;
	regAdd[1] = 0x00;
	regAdd[2] = 0x07;
	rc = fts_read_reg(info, regAdd, 3, (unsigned char *)val, 5);

	dbg_cr("FTS %02X%02X%02X =  %02X %02X %02X %02X \n",
			regAdd[0], regAdd[1], regAdd[2], val[1], val[2], val[3], val[4]);
	if (val[1] != FTS_ID0 || val[2] != FTS_ID1)
		return 1;

	fts_systemreset(info);

	rc=fts_wait_for_ready(info);
	if (rc==-2) {
		info->fw_version_of_ic =0;
		info->config_version_of_ic=0;
		info->fw_main_version_of_ic=0;
	} else
		fts_get_version_info(info);

	if ((rc = fts_fw_update_on_probe(info)) < 0) {
		if (rc == -2) {
			dbg_cr("%s: Need not Update. Skip to firmware update.\n", __func__);
		}
		else {
			dbg_cr("%s: Failed to firmware update.\n", __func__);
		} 			
	}
#ifdef FTS_PANTECH_PRODUCT_LINE
	if(info->mflag_init_test)
		fts_check_product_line(info);
#endif /* FTS_PANTECH_PRODUCT_LINE */

	info->touch_count = 0;

	fts_command(info, SLEEPOUT);
	fts_delay(300);
	fts_command(info, SENSEON);

#ifdef FTS_SUPPORT_NOISE_PARAM
	fts_get_noise_param_address(info);
#endif

	if (info->board->support_hover) {
		fts_command(info, FTS_CMD_HOVER_ON);
		info->hover_enabled = 1;
	}

	if (info->board->support_mshover) {
		//fts_command(info, FTS_CMD_HOVER_ON);
		info->hover_enabled = 0;
	}

#ifdef TSP_FACTORY_TEST
	rc = getChannelInfo(info);
	if (rc >= 0) {
		dbg_cr("FTS Sense(%02d) Force(%02d)\n",
				info->SenseChannelLength, info->ForceChannelLength);
	} else {
		dbg_cr("FTS read failed rc = %d\n", rc);
		dbg_cr("FTS Initialise Failed\n");
		return 1;
	}
	info->pFrame =
		kzalloc(info->SenseChannelLength * info->ForceChannelLength * 2,
				GFP_KERNEL);
	if (info->pFrame == NULL) {
		dbg_cr("FTS pFrame kzalloc Failed\n");
		return 1;
	}
#endif

	fts_command(info, FORCECALIBRATION);
	fts_command(info, FLUSHBUFFER);

	fts_interrupt_set(info, INT_ENABLE);

	memset(val, 0x0, 4);
	regAdd[0] = READ_STATUS;
	rc = fts_read_reg(info, regAdd, 1, (unsigned char *)val, 4);
	dbg_cr("FTS ReadStatus(0x84) : %02X %02X %02X %02X\n", 
			val[0], val[1], val[2], val[3]);

	MutualTouchMode = false;

	dbg_cr("FTS Initialised..\n");

	return 0;
}

static void fts_unknown_event_handler(struct fts_ts_info *info,
		unsigned char data[])
{
	dbg_cr("FTS Known Event %02X %02X %02X %02X %02X %02X %02X %02X\n",
			data[0], data[1], data[2], data[3], data[4], data[5], data[6],
			data[7]);
}

static unsigned char fts_event_handler_type_b(struct fts_ts_info *info,
		unsigned char data[],
		unsigned char LeftEvent)
{
	unsigned char EventNum = 0;
	unsigned char NumTouches = 0;
	unsigned char TouchID = 0, EventID = 0;
	unsigned char LastLeftEvent = 0;
	int x = 0, y = 0, z = 0;
	int bw = 0, bh = 0, angle = 0, palm = 0;
#if defined (CONFIG_INPUT_BOOSTER) || defined(TSP_BOOSTER)
	bool booster_restart = false;
#endif

#ifdef PAN_KNOCK_ON

	if ( (data[0] == 0x20) && (data[1] == 0x01) ) {
		dbg_cr("Knock On checked..Send Power Key..\n");
		input_report_key(info->input_dev, KEY_POWER, 1);
		input_sync(info->input_dev);
		input_report_key(info->input_dev, KEY_POWER, 0);
		input_sync(info->input_dev);

		return 0;
	}

#endif
#ifdef PAN_TOUCH_EVENT_WITH_HALL_IC
	if(info->touch_mode_state.smart_cover){
		// smart cover closed.
		dbg_touch("fts_event_handler_type_b but smart cover closed\n");
		return 0;
	}  
#endif 


	for (EventNum = 0; EventNum < LeftEvent; EventNum++) {

		dbg_op("Raw Data: %d %2x %2x %2x %2x %2x %2x %2x %2x\n", EventNum,
				data[EventNum * FTS_EVENT_SIZE],
				data[EventNum * FTS_EVENT_SIZE+1],
				data[EventNum * FTS_EVENT_SIZE+2],
				data[EventNum * FTS_EVENT_SIZE+3],
				data[EventNum * FTS_EVENT_SIZE+4],
				data[EventNum * FTS_EVENT_SIZE+5],
				data[EventNum * FTS_EVENT_SIZE+6],
				data[EventNum * FTS_EVENT_SIZE+7]);

		EventID = data[EventNum * FTS_EVENT_SIZE] & 0x0F;

		if ((EventID >= 3) && (EventID <= 5)) {
			LastLeftEvent = 0;
			NumTouches = 1;

			TouchID = (data[EventNum * FTS_EVENT_SIZE] >> 4) & 0x0F;
		} else {
			LastLeftEvent =
				data[7 + EventNum * FTS_EVENT_SIZE] & 0x0F;
			NumTouches =
				(data[1 + EventNum * FTS_EVENT_SIZE] & 0xF0) >> 4;
			TouchID = data[1 + EventNum * FTS_EVENT_SIZE] & 0x0F;
			EventID = data[EventNum * FTS_EVENT_SIZE] & 0xFF;
		}

		switch (EventID) {
		case EVENTID_NO_EVENT:
			break;

		case EVENTID_ENTER_POINTER:
			info->touch_count++;
#if defined (CONFIG_INPUT_BOOSTER) || defined(TSP_BOOSTER)
			booster_restart = true;
#endif
		case EVENTID_MOTION_POINTER:
			info->finger[TouchID].state = EventID;

			if (EventID == EVENTID_MOTION_POINTER)
				info->finger[TouchID].mcount++;

			x = data[1 + EventNum * FTS_EVENT_SIZE] +
				((data[2 + EventNum * FTS_EVENT_SIZE] &
				  0x0f) << 8);
			y = ((data[2 + EventNum * FTS_EVENT_SIZE] &
						0xf0) >> 4) + (data[3 +
						EventNum *
						FTS_EVENT_SIZE] << 4);
			bw = data[4 + EventNum * FTS_EVENT_SIZE];
			bh = data[5 + EventNum * FTS_EVENT_SIZE];

			angle =
				(data[6 + EventNum * FTS_EVENT_SIZE] & 0x7f)
				<< 1;

			if (angle & 0x80)
				angle |= 0xffffff00;

			palm =
				(data[6 + EventNum * FTS_EVENT_SIZE] >> 7) &
				0x01;

			z = data[7 + EventNum * FTS_EVENT_SIZE];

			input_mt_slot(info->input_dev, TouchID);
			//2014.3.18 touch id
			input_report_abs(info->input_dev, ABS_MT_TRACKING_ID, TouchID);			// TOUCH_ID
			input_mt_report_slot_state(info->input_dev,
					MT_TOOL_FINGER,
					1 + (palm << 1));

			input_report_key(info->input_dev, BTN_TOUCH, 1);
			input_report_key(info->input_dev,
					BTN_TOOL_FINGER, 1);
			input_report_abs(info->input_dev,
					ABS_MT_POSITION_X, x);
			input_report_abs(info->input_dev,
					ABS_MT_POSITION_Y, y);

			input_report_abs(info->input_dev,
					ABS_MT_TOUCH_MAJOR, max(bw,
						bh));
			input_report_abs(info->input_dev,
					ABS_MT_TOUCH_MINOR, min(bw,
						bh));

			input_report_abs(info->input_dev,
					ABS_MT_WIDTH_MAJOR, z);

			// 			input_report_abs(info->input_dev, ABS_MT_ANGLE,
			// 					 angle);
			// 			input_report_abs(info->input_dev, ABS_MT_PALM,
			// 					 palm);

#ifdef CONFIG_GLOVE_TOUCH
			dbg_op("[P] tID:%d x:%d y:%d z:%d tc:%d tm:%d\n",
					TouchID, x, y, z, info->touch_count, info->touch_mode);
#else
			dbg_op("[P] tID:%d x:%d y:%d z:%d tc:%d\n",
					TouchID, x, y, z, info->touch_count);
#endif

			break;

		case EVENTID_LEAVE_POINTER:
			info->touch_count--;

			input_mt_slot(info->input_dev, TouchID);
			input_mt_report_slot_state(info->input_dev,
					MT_TOOL_FINGER, 0);

			if (info->touch_count == 0) {
				/* Clear BTN_TOUCH when All touch are released  */
				input_report_key(info->input_dev, BTN_TOUCH, 0);
			}

			dbg_op("[R] tID:%d mc: %d tc:%d Ver[%02X%04X%01X%01X]\n",
					TouchID, info->finger[TouchID].mcount, info->touch_count,
					info->panel_revision, info->fw_main_version_of_ic,
					info->flip_enable, info->mshover_enabled);

			info->finger[TouchID].mcount = 0;
			break;
#ifdef CONFIG_GLOVE_TOUCH
		case EVENTID_STATUS_EVENT:
			if (data[1 + EventNum * FTS_EVENT_SIZE] == 0x0C) {
				if (data[2 + EventNum * FTS_EVENT_SIZE] == 0x01)
					info->touch_mode = FTS_TM_GLOVE;
				else
					info->touch_mode = FTS_TM_NORMAL;

				input_report_switch(info->input_dev,
						SW_GLOVE, info->touch_mode);
			} 
			else
				fts_unknown_event_handler(info,
						&data[EventNum *
						FTS_EVENT_SIZE]);	

			break;
#endif

#ifdef TSP_FACTORY_TEST
		case EVENTID_RESULT_READ_REGISTER:
			procedure_cmd_event(info, &data[EventNum * FTS_EVENT_SIZE]);
			break;
#endif
		case EVENTID_ERROR :
			if(TouchID == EVENT_ERROR_ITO){
				dbg_cr("EVENT_ITO_TEST_RESULT data[2]->%x, data[3]->%x\n",
						data[2 + EventNum * FTS_EVENT_SIZE],data[3 + EventNum * FTS_EVENT_SIZE]);
				info->ito_test[0]=data[2 + EventNum * FTS_EVENT_SIZE];
				info->ito_test[1]=data[3 + EventNum * FTS_EVENT_SIZE];        
			}else{
				dbg_cr("EVENTID_ERROR data[1]->%x, data[5]->%x\n",
						data[ + EventNum * FTS_EVENT_SIZE],data[5 + EventNum * FTS_EVENT_SIZE]);
			}
			break;
		case EVENTID_CONTROLLER_READY :
			dbg_op(" EVENTID_CONTROLLER_READY. FTSD chip-> %d, FTSA0 chip-> %d"
					"FTSA1 chip->%d, fw-> %d, config id-> %d, config ver-> %d\n",
					data[1 + EventNum * FTS_EVENT_SIZE],
					data[2 + EventNum * FTS_EVENT_SIZE]>>4,data[2 + EventNum * FTS_EVENT_SIZE]&0x0F,
					(data[3 + EventNum * FTS_EVENT_SIZE]<<8)|data[4 + EventNum * FTS_EVENT_SIZE],
					data[5 + EventNum * FTS_EVENT_SIZE],data[6 + EventNum * FTS_EVENT_SIZE]);
			break;
		case EVENTID_SLEEPOUT_CONTROLLER_READY :
			dbg_op("EVENTID_SLEEPOUT_CONTROLLER_READY\n");
			break;
		case EVENTID_STATUS_EVENT :
			dbg_op("FTS EVENTID_STATUS_EVENT %02X %02X %02X %02X %02X %02X %02X\n",
					data[1+ EventNum * FTS_EVENT_SIZE], data[2+ EventNum * FTS_EVENT_SIZE],
					data[3+ EventNum * FTS_EVENT_SIZE], data[4+ EventNum * FTS_EVENT_SIZE],
					data[5+ EventNum * FTS_EVENT_SIZE], data[6+ EventNum * FTS_EVENT_SIZE],
					data[7+ EventNum * FTS_EVENT_SIZE]);

		default:
			fts_unknown_event_handler(info,
					&data[EventNum *
					FTS_EVENT_SIZE]);
			continue;
		}

		input_sync(info->input_dev);

#ifdef TSP_BOOSTER
		if ((EventID == EVENTID_ENTER_POINTER)
				|| (EventID == EVENTID_LEAVE_POINTER)) {
			if (info->touch_count)
				fts_set_dvfs_lock(info, TSP_BOOSTER_ON, booster_restart);
			else
				fts_set_dvfs_lock(info, TSP_BOOSTER_OFF, false);
		}
#endif
#if defined (CONFIG_INPUT_BOOSTER)
		if ((EventID == EVENTID_ENTER_POINTER)
				|| (EventID == EVENTID_LEAVE_POINTER)) {
			if (booster_restart) {
				INPUT_BOOSTER_REPORT_KEY_EVENT(info->input_dev, KEY_BOOSTER_TOUCH, 0);
				INPUT_BOOSTER_REPORT_KEY_EVENT(info->input_dev, KEY_BOOSTER_TOUCH, 1);
				INPUT_BOOSTER_SEND_EVENT(KEY_BOOSTER_TOUCH,
						BOOSTER_MODE_ON);
			}
			if (!info->touch_count) {
				INPUT_BOOSTER_REPORT_KEY_EVENT(info->input_dev, KEY_BOOSTER_TOUCH, 0);
				INPUT_BOOSTER_SEND_EVENT(KEY_BOOSTER_TOUCH, BOOSTER_MODE_OFF);
			}
		}
#endif
	}

	return LastLeftEvent;
}

#ifdef FTS_SUPPORT_TA_MODE
static void fts_ta_cb(struct fts_callbacks *cb, int ta_status)
{
	struct fts_ts_info *info =
		container_of(cb, struct fts_ts_info, callbacks);

	if (ta_status == 0x01 || ta_status == 0x03) {
		fts_command(info, FTS_CMD_CHARGER_PLUGGED);
		info->TA_Pluged = true;
		dbg_cr("%s: device_control : CHARGER CONNECTED, ta_status : %x\n",
				__func__, ta_status);
	} else {
		fts_command(info, FTS_CMD_CHARGER_UNPLUGGED);
		info->TA_Pluged = false;
		dbg_cr("%s: device_control : CHARGER DISCONNECTED, ta_status : %x\n",
				__func__, ta_status);
	}
}
#endif

//++ p11309 - 2013.12.21 for first Setup
static void fts_tsp_register_callback(void *cb)
{
	dbg_cr("%s\n", __func__);
	fts_charger_callbacks = cb;
}

static int tsp_power_pin_setuped = 0;

static struct tsp_control_pin atmel_mxt_avdd;
static struct tsp_control_pin atmel_mxt_dvdd;
static struct tsp_control_pin atmel_mxt_int;

void TSP_Power_Pin_Init(void) {
	if ( tsp_power_pin_setuped == 1 ) return;

	atmel_mxt_avdd.type = POWER_GPIO_SETUP;
	atmel_mxt_avdd.name = "touch_power_avdd";
	atmel_mxt_avdd.gpio.num = GPIO_TOUCH_AVDD;	
	atmel_mxt_avdd.gpio.direction = GPIO_OUTPUT_HIGH;

	atmel_mxt_dvdd.type = POWER_GPIO_SETUP;
	atmel_mxt_dvdd.name = "touch_power_dvdd";
	atmel_mxt_dvdd.gpio.num = GPIO_TOUCH_DVDD;	
	atmel_mxt_dvdd.gpio.direction = GPIO_OUTPUT_HIGH;	

	atmel_mxt_int.type = POWER_GPIO_SETUP;
	atmel_mxt_int.name = "touch_int_n";
	atmel_mxt_int.gpio.num = GPIO_TOUCH_CHG;
	atmel_mxt_int.gpio.direction = GPIO_INPUT;	

	tsp_power_pin_setuped = 1;
}

int TSP_PowerSetup(struct tsp_control_pin tsp_ctrl, int up_down) 
{
	int rc;	

	switch ( tsp_ctrl.type ) {
	case POWER_NOT_USED:
		break;
	case POWER_GPIO_SETUP:			
		if ( up_down == POWER_UP ) {
			rc = gpio_request(tsp_ctrl.gpio.num, tsp_ctrl.name);
			if (rc) {
				gpio_free(tsp_ctrl.gpio.num);				
				if (rc) {
					dbg_cr("%s: %d failed, rc=%d\n",
							tsp_ctrl.name, tsp_ctrl.gpio.num, rc);
				}
			}

			if ( tsp_ctrl.gpio.direction == GPIO_INPUT ) {
				rc = gpio_direction_input(tsp_ctrl.gpio.num);				
				if (rc) {
					dbg_cr("%s: %d gpio_direction_input failed, rc=%d\n",
							tsp_ctrl.name, tsp_ctrl.gpio.num, rc);
				}
			}
			else {
				rc = gpio_direction_output(tsp_ctrl.gpio.num, 
						tsp_ctrl.gpio.direction);
				if (rc) {
					dbg_cr("%s: %d gpio_direction_output failed, rc=%d\n",
							tsp_ctrl.name, tsp_ctrl.gpio.num, rc);
				}
			}			
		}
		else {

			if ( tsp_ctrl.gpio.direction == GPIO_OUTPUT_HIGH || 
					tsp_ctrl.gpio.direction == GPIO_OUTPUT_LOW) {				
				gpio_set_value(
						tsp_ctrl.gpio.num, 
						!gpio_get_value(tsp_ctrl.gpio.num)
						);
				msleep(10);
			}

			gpio_free(tsp_ctrl.gpio.num);
		}		

		break;

	case POWER_PM_REGULATOR:

		if ( up_down == POWER_UP ) {

			if ( tsp_ctrl.regulator.reg == NULL ) 
			{
				tsp_ctrl.regulator.reg = regulator_get(NULL, tsp_ctrl.name);
				if( tsp_ctrl.regulator.reg == NULL ) {
					dbg_cr("%s: regulator_get failed \n", tsp_ctrl.name);
					return -EINVAL;
				}

				rc = regulator_set_voltage(
						tsp_ctrl.regulator.reg, 
						tsp_ctrl.regulator.volt, 
						tsp_ctrl.regulator.volt );
				if (rc) { 
					dbg_cr("%s: set_voltage %duV failed, rc=%d\n", 
							tsp_ctrl.name, tsp_ctrl.regulator.volt, rc);
					return rc;
				}
			}

			rc = regulator_enable(tsp_ctrl.regulator.reg);
			if (rc) {
				dbg_cr("%s: regulator enable failed (%d)\n", 
						tsp_ctrl.name, rc);
				return rc;
			}			
		}
		else {			

			if( tsp_ctrl.regulator.reg == NULL ) {
				dbg_cr("%s: No regulator...failed \n", tsp_ctrl.name);

				tsp_ctrl.regulator.reg = regulator_get(NULL, tsp_ctrl.name);
				if( tsp_ctrl.regulator.reg == NULL ) {
					dbg_cr("%s: regulator_get failed \n", tsp_ctrl.name);
					return -EINVAL;
				}

				rc = regulator_set_voltage(
						tsp_ctrl.regulator.reg, 
						tsp_ctrl.regulator.volt, 
						tsp_ctrl.regulator.volt
						);
				if (rc) { 
					dbg_cr("%s: set_voltage %duV failed, rc=%d\n", 
							tsp_ctrl.name, tsp_ctrl.regulator.volt, rc);
					return rc;
				}
			}

			rc = regulator_disable(tsp_ctrl.regulator.reg);
			if (rc) {
				dbg_cr("%s: regulator disable failed (%d)\n", 
						tsp_ctrl.name, rc);
				return rc;
			}

			regulator_put(tsp_ctrl.regulator.reg);
			tsp_ctrl.regulator.reg = NULL;
		}
		break;
	}
	return 0;
}

static int fts_power(bool enable)
{
	TSP_Power_Pin_Init();
	if (enable == true) {
		dbg_op("TSP Power Enable...\n");
		TSP_PowerSetup(atmel_mxt_avdd, POWER_UP);
		TSP_PowerSetup(atmel_mxt_dvdd, POWER_UP);
		TSP_PowerSetup(atmel_mxt_int, POWER_UP);
	}
	else {
		dbg_op("TSP Power Disabled...\n");
		TSP_PowerSetup(atmel_mxt_int, POWER_DOWN);
		TSP_PowerSetup(atmel_mxt_avdd, POWER_DOWN);
		TSP_PowerSetup(atmel_mxt_dvdd, POWER_DOWN);		
	}		

	msleep(100);

	return 0;
}
//-- p11309

static void ts_init_event_handler(struct work_struct *init_test_work)
{
	struct fts_ts_info *info = container_of(init_test_work, struct fts_ts_info, init_test_work);
	fts_reinit(info);
	info->mflag_init_test=false;
	enable_irq(info->irq);
	mutex_unlock(&info->device_mutex);
	return;
}


/**
 * fts_interrupt_handler()
 *
 * Called by the kernel when an interrupt occurs (when the sensor
 * asserts the attention irq).
 *
 * This function is the ISR thread and handles the acquisition
 * and the reporting of finger data when the presence of fingers
 * is detected.
 */

#ifdef PAN_IRQ_WORKQUEUE
static void ts_event_handler(struct work_struct *work)
{
	struct fts_ts_info *info = container_of(work, struct fts_ts_info, work);
	unsigned char data[FTS_EVENT_SIZE * FTS_FIFO_MAX];
	int rc;
	unsigned char regAdd[4] = {0xb6, 0x00, 0x45, READ_ALL_EVENT};
	unsigned short evtcount = 0;

	evtcount = 0;
	rc = fts_read_reg(info, &regAdd[0], 3, (unsigned char *)&evtcount, 2);
	evtcount = evtcount >> 10;

	dbg_op("%s..count=%d\n", __func__, evtcount);

	if (evtcount > 0) {
		memset(data, 0x0, FTS_EVENT_SIZE * evtcount);
		rc = fts_read_reg(info, &regAdd[3], 1, (unsigned char *)data,
				FTS_EVENT_SIZE * evtcount);
		fts_event_handler_type_b(info, data, evtcount);
	}

	enable_irq(info->client->irq);
}

static irqreturn_t fts_interrupt_handler(int irq, void *handle)
{
	struct fts_ts_info *info = handle;
	disable_irq_nosync(info->client->irq);
	queue_work(fts_event_wq, &info->work);	

	return IRQ_HANDLED;
}

#else
static irqreturn_t fts_interrupt_handler(int irq, void *handle)
{
	struct fts_ts_info *info = handle;
	unsigned char data[FTS_EVENT_SIZE * FTS_FIFO_MAX];
	int rc;
	unsigned char regAdd[4] = {0xb6, 0x00, 0x45, READ_ALL_EVENT};
	unsigned short evtcount = 0;	

	evtcount = 0;
	rc = fts_read_reg(info, &regAdd[0], 3, (unsigned char *)&evtcount, 2);
	evtcount = evtcount >> 10;

	dbg_op("%s..count=%d\n", __func__, evtcount);

	if (evtcount > 0) {
		memset(data, 0x0, FTS_EVENT_SIZE * evtcount);
		rc = fts_read_reg(info, &regAdd[3], 1, (unsigned char *)data,
				FTS_EVENT_SIZE * evtcount);
		fts_event_handler_type_b(info, data, evtcount);
	}

	return IRQ_HANDLED;
}
#endif

static int fts_irq_enable(struct fts_ts_info *info,
		bool enable)
{
	int retval = 0;

	if (enable) {
		if (info->irq_enabled)
			return retval;

#ifdef PAN_IRQ_WORKQUEUE
		fts_event_wq = create_singlethread_workqueue("fts_event_wq");
		if (!fts_event_wq) {
			dbg_cr("%s: Failed to create singlethread irq hander%d\n", __func__, retval);
			return -ENOMEM;
		}

		INIT_WORK(&info->work, ts_event_handler);
		INIT_WORK(&info->init_test_work, ts_init_event_handler);
		retval = request_irq(info->irq, fts_interrupt_handler,
				info->board->irq_type, "stmicro_fts_ts", info);
		if (retval) {
			dbg_cr("%s: Failed to create irq hander%d\n", __func__, retval);
			return retval;
		}
#else
		retval = request_threaded_irq(info->irq, NULL,
				fts_interrupt_handler, info->board->irq_type,
				"stmicro_fts_ts", info);
		if (retval) {
			dbg_cr("%s: Failed to create irq thread %d\n", __func__, retval);
			return retval;
		}
#endif
		info->irq_enabled = true;
	} else {
		if (info->irq_enabled) {
			disable_irq(info->irq);
			free_irq(info->irq, info);
			info->irq_enabled = false;
		}
	}

	return retval;
}

static int fts_probe(struct i2c_client *client, const struct i2c_device_id *idp)
{
	int retval;
	struct fts_ts_info *info = NULL;
	static char fts_ts_phys[64] = { 0 };
	int i = 0;
	int ret;

	//++ p11309 - 2014.01.08 for Offline boot process 
	oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
	smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type *)
		smem_alloc(SMEM_ID_VENDOR1,sizeof(oem_pm_smem_vendor1_data_type));

	if(smem_id_vendor1_ptr->power_on_mode == 0) {
		dbg_cr(" OFFLINE_CHARGER is enabled. And Touch Driver IRQ is disabled\n");
		ret = 0;
		touch_error_info="OFFLINE_CHARGER";
		goto err_check_pantech;
	}
	//-- p11309

	printk("+-------------------------------------------+\n");
	printk("|  STMicro FingerTips Probe - Ver: %s    |\n", FTS_TS_DRV_VERSION);
	printk("+-------------------------------------------+\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		touch_error_cnt++;
		dbg_cr("FTS err = EIO!, touch_err_cnt -> %d\n",touch_error_cnt);
		touch_error_info="FTS err = EIO";
		msleep(1000);
		if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
			touch_error_cnt++;
			ret = -EIO;
			goto err_check_pantech;
		}		
	}

	info = kzalloc(sizeof(struct fts_ts_info), GFP_KERNEL);
	if (!info) {
		touch_error_cnt++;
		dbg_cr("FTS err = ENOMEM! touch_error_cnt-> %d\n",touch_error_cnt);
		msleep(1000);
		info = kzalloc(sizeof(struct fts_ts_info), GFP_KERNEL);
		if(!info){
			touch_error_cnt++;
			ret =-ENOMEM;
			touch_error_info="FTS err = ENOMEM!";
			goto err_check_pantech;
		}
	}

	info->client = client;

	info->board = kzalloc(sizeof(struct fts_i2c_platform_data), GFP_KERNEL);
	if (!info->board){
		touch_error_cnt++;
		dbg_cr("info->board is not NULL. touch_error_cnt -> %d \n",touch_error_cnt);		
		msleep(1000);
		touch_error_info="info->board is not NULL!";
		info->board = kzalloc(sizeof(struct fts_i2c_platform_data), GFP_KERNEL);
		if (!info->board){
			touch_error_cnt++;
			ret = -ENOMEM;
			goto err_check_pantech;
		}
	}	

	info->board->factory_flatform = false;
	info->board->max_x = 1079;
	info->board->max_y = 1919;
	info->board->max_width = 28;

	info->board->support_hover = false;
	info->board->support_mshover = false;
	info->board->irq_type = IRQF_TRIGGER_LOW;
	info->board->gpio = GPIO_TOUCH_CHG;

	info->board->power = fts_power;
	info->board->register_cb = fts_tsp_register_callback;
	info->ito_test[0] = PAN_DATA_INIT;
	info->ito_test[1] = PAN_DATA_INIT;
	info->mflag_init_test = false;    /* for PANTECH ##1199 Touch Init Test */
	info->touch_mode_state.touch = 0;
	info->touch_mode_state.smart_cover = 0;
	info->touch_mode_state.smart_cover_from_ui= 0;

	init_timer(&info->touch_mode_state.check_smart_cover);
	info->touch_mode_state.check_smart_cover.function = pan_hallic_ui_sync_timer_func;
	info->touch_mode_state.check_smart_cover.data = 0;
	info->touch_mode_state.check_smart_cover.expires = 500;

	if (info->board->support_hover) {
		dbg_cr("FTS Support Hover Event \n");
	} else {
		dbg_cr("FTS Not support Hover Event \n");
	}

	if (info->board->power)
		info->board->power(true);

	info->dev = &info->client->dev;
	info->input_dev = input_allocate_device();
	if (!info->input_dev) {
		touch_error_cnt++;
		dbg_cr("FTS input_alloc_device err = ENOMEM! touch_error_cnt -> %d\n",touch_error_cnt);
		msleep(1000);
		touch_error_info="FTS input_alloc_device err = ENOMEM!";
		info->input_dev = input_allocate_device();
		if (!info->input_dev) {
			touch_error_cnt++;
			ret = -ENOMEM;
			goto err_input_allocate_device;
		}
	}

	info->input_dev->dev.parent = &client->dev;
	info->input_dev->name = "stmicro_fts_ts";
	snprintf(fts_ts_phys, sizeof(fts_ts_phys), "%s/input0",
			info->input_dev->name);
	info->input_dev->phys = fts_ts_phys;
	info->input_dev->id.bustype = BUS_I2C;

	client->irq = gpio_to_irq(GPIO_TOUCH_CHG);
	info->irq = client->irq;	
	info->irq_type = info->board->irq_type;
	info->irq_enabled = false;

	info->touch_stopped = false;
	info->panel_revision = info->board->panel_revision;
	info->stop_device = fts_stop_device;
	info->start_device = fts_start_device;
	info->fts_command = fts_command;
	info->fts_read_reg = fts_read_reg;
	info->fts_write_reg = fts_write_reg;
	info->fts_systemreset = fts_systemreset;
	info->fts_get_version_info = fts_get_version_info;
	info->fts_wait_for_ready = fts_wait_for_ready;
#ifdef	PAN_TOUCH_SET_PEN_MODE
	info->fts_pen_mode_set = fts_stylus_mode_onoff;
#endif
	init_completion(&info->init_done);

#ifdef TSP_BOOSTER
	fts_init_dvfs(info);
	info->boost_level = TSP_BOOSTER_LEVEL2;
#endif

#ifdef CONFIG_GLOVE_TOUCH
	input_set_capability(info->input_dev, EV_SW, SW_GLOVE);
#endif

	set_bit(EV_SYN, info->input_dev->evbit);
	set_bit(EV_KEY, info->input_dev->evbit);
	set_bit(EV_ABS, info->input_dev->evbit);

#ifdef INPUT_PROP_DIRECT
	set_bit(INPUT_PROP_DIRECT, info->input_dev->propbit);
#endif

	set_bit(BTN_TOUCH, info->input_dev->keybit);
	set_bit(BTN_TOOL_FINGER, info->input_dev->keybit);

#ifdef SKY_PROCESS_CMD_KEY
	set_bit(EV_SYN, info->input_dev->evbit);
	set_bit(EV_KEY, info->input_dev->evbit);
	set_bit(EV_ABS, info->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, info->input_dev->propbit);
	set_bit(BTN_TOUCH, info->input_dev->keybit);
	set_bit(KEY_MENU, info->input_dev->keybit);	
	set_bit(KEY_HOME, info->input_dev->keybit);
	set_bit(KEY_HOMEPAGE, info->input_dev->keybit);
	set_bit(KEY_BACK, info->input_dev->keybit);
	set_bit(KEY_SEARCH, info->input_dev->keybit);
	set_bit(KEY_HOMEPAGE, info->input_dev->keybit);
	set_bit(KEY_0, info->input_dev->keybit);
	set_bit(KEY_1, info->input_dev->keybit);
	set_bit(KEY_2, info->input_dev->keybit);
	set_bit(KEY_3, info->input_dev->keybit);
	set_bit(KEY_4, info->input_dev->keybit);
	set_bit(KEY_5, info->input_dev->keybit);
	set_bit(KEY_6, info->input_dev->keybit);
	set_bit(KEY_7, info->input_dev->keybit);
	set_bit(KEY_8, info->input_dev->keybit);
	set_bit(KEY_9, info->input_dev->keybit);
	set_bit(0xe3, info->input_dev->keybit); /* '*' */
	set_bit(0xe4, info->input_dev->keybit); /* '#' */
	set_bit(0xe5, info->input_dev->keybit); /* 'KEY_END' p13106 120105 */
	set_bit(KEY_POWER, info->input_dev->keybit);
	set_bit(KEY_LEFTSHIFT, info->input_dev->keybit);
	set_bit(KEY_RIGHTSHIFT, info->input_dev->keybit);
	set_bit(KEY_LEFT, info->input_dev->keybit);
	set_bit(KEY_RIGHT, info->input_dev->keybit);
	set_bit(KEY_UP, info->input_dev->keybit);
	set_bit(KEY_DOWN, info->input_dev->keybit);
	set_bit(KEY_ENTER, info->input_dev->keybit);
	set_bit(KEY_SEND, info->input_dev->keybit);
	set_bit(KEY_END, info->input_dev->keybit);
	set_bit(KEY_F1, info->input_dev->keybit);
	set_bit(KEY_F2, info->input_dev->keybit);
	set_bit(KEY_F3, info->input_dev->keybit);				
	set_bit(KEY_F4, info->input_dev->keybit);
	set_bit(KEY_VOLUMEUP, info->input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, info->input_dev->keybit);
	set_bit(KEY_CLEAR, info->input_dev->keybit);
	set_bit(KEY_CAMERA, info->input_dev->keybit);
	set_bit(KEY_VT_CALL, info->input_dev->keybit);       // P13106 VT_CALL
#endif	

#ifdef CONFIG_INPUT_BOOSTER
	set_bit(KEY_BOOSTER_TOUCH, info->input_dev->keybit);
#endif

	input_mt_init_slots(info->input_dev, FINGER_MAX);
	input_set_abs_params(info->input_dev, ABS_MT_POSITION_X,
			0, info->board->max_x, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_POSITION_Y,
			0, info->board->max_y, 0, 0);

	mutex_init(&info->lock);
	mutex_init(&(info->device_mutex));

	info->enabled = false;
	mutex_lock(&info->lock);
	retval = fts_init(info);
	mutex_unlock(&info->lock);
	if (retval) {
		touch_error_cnt++;
		dbg_cr("FTS fts_init fail! touch_error_cnt -> %d\n",touch_error_cnt);
		msleep(1000);
		touch_error_info="FTS fts_init fail!";
		mutex_lock(&info->lock);
		retval = fts_init(info);
		mutex_unlock(&info->lock);
		if(retval){
			touch_error_cnt++;
			goto err_fts_init;
		}
	}
#ifdef PAN_TOUCH_SET_PEN_MODE
	if(info->touch_mode_state.touch == PAN_TOUCH_MODE_FINGER || info->touch_mode_state.touch == PAN_TOUCH_MODE_GLOVE){
		fts_stylus_mode_onoff(info,PAN_TOUCH_MODE_FINGER);
	}
#endif

	input_set_abs_params(info->input_dev, ABS_MT_TOUCH_MAJOR,
			0, 255, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_TOUCH_MINOR,
			0, 255, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_WIDTH_MAJOR,
			0, 255, 0, 0);
	// 	input_set_abs_params(info->input_dev, ABS_MT_ANGLE,
	// 				 -90, 90, 0, 0);
	// 	input_set_abs_params(info->input_dev, ABS_MT_PALM, 0, 1, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_DISTANCE,
			0, 255, 0, 0);

	input_set_drvdata(info->input_dev, info);
	i2c_set_clientdata(client, info);

	retval = input_register_device(info->input_dev);
	if (retval) {
		touch_error_cnt++;
		dbg_cr("FTS input_register_device fail! touch_error_cnt -> %d\n",touch_error_cnt);
		msleep(1000);
		touch_error_info="FTS input_register_device fail!";
		retval = input_register_device(info->input_dev);
		if(retval){
			touch_error_cnt++;
			goto err_register_input;		  
		}
	}

	for (i = 0; i < FINGER_MAX; i++) {
		info->finger[i].state = EVENTID_LEAVE_POINTER;
		info->finger[i].mcount = 0;
	}

	info->enabled = true;

	retval = fts_irq_enable(info, true);
	if (retval < 0) {
		touch_error_cnt++;
		dbg_cr("%s: Failed to enable attention interrupt. touch_error_cnt -> %d\n", __func__,touch_error_cnt);
		msleep(1000);
		touch_error_info="FTS input_register_device fail!";
		retval = fts_irq_enable(info, true);
		if(retval < 0){
			touch_error_cnt++;
			goto err_enable_irq;		
		}
	}

#ifdef FTS_SUPPORT_TA_MODE
	info->register_cb = info->board->register_cb;

	info->callbacks.inform_charger = fts_ta_cb;
	if (info->register_cb)
		info->register_cb(&info->callbacks);
#endif

#ifdef TSP_FACTORY_TEST
	INIT_LIST_HEAD(&info->cmd_list_head);

	for (i = 0; i < ARRAY_SIZE(ft_cmds); i++)
		list_add_tail(&ft_cmds[i].list, &info->cmd_list_head);

	mutex_init(&info->cmd_lock);
	info->cmd_is_running = false;

	pan_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(pan_class)) {
		touch_error_cnt++;
		dbg_cr("FTS Failed to create class(pan_class) for the sysfs\n");
		msleep(1000);
		touch_error_info="FTS Failed to create class(pan_class) for the sysfs";
		pan_class = class_create(THIS_MODULE, "touch");
		if(IS_ERR(pan_class)) {
			touch_error_cnt++;
			goto err_sysfs;
		}
	}

	info->fac_dev_ts = device_create(pan_class, NULL, FTS_ID0, info, "fts");
	if (IS_ERR(info->fac_dev_ts)) {
		touch_error_cnt++;
		dbg_cr("FTS Failed to create device for the sysfs\n");
		msleep(1000);
		touch_error_info="FTS Failed to create device for the sysfs";
		info->fac_dev_ts = device_create(pan_class, NULL, FTS_ID0, info, "fts");
		if (IS_ERR(info->fac_dev_ts)) {
			touch_error_cnt++;
			goto err_sysfs;
		}
	}

	dev_set_drvdata(info->fac_dev_ts, info);

	ret = sysfs_create_group(&info->fac_dev_ts->kobj,
			&sec_touch_factory_attr_group);
	if (ret < 0) {
		touch_error_cnt++;
		dbg_cr("FTS Failed to create sysfs group\n");
		msleep(1000);
		touch_error_info="FTS Failed to create sysfs group";
		ret = sysfs_create_group(&info->fac_dev_ts->kobj,
				&sec_touch_factory_attr_group);
		if (ret < 0) {
			touch_error_cnt++;
			goto err_sysfs;
		}
	}
#endif

	complete_all(&info->init_done);

	shared_info = info;

#ifdef CONFIG_POWERSUSPEND
	register_power_suspend(&fts_power_suspend);
#endif

#ifdef PAN_KNOCK_ON
	device_init_wakeup(info->dev, 1);
#endif

#ifdef PAN_TSP_IO
	pan_fts_io_register(info);
	retval = misc_register(&touch_io);
	if (retval){
		dbg_cr("touch_io can''t register misc device\n");
	}
	dbg_cr("touch_io is registered.\n");
	retval = misc_register(&touch_event);
	if (retval) {
		dbg_cr("touch_event can''t register misc device\n");
	}
	dbg_cr("touch_event is registered.\n");
#endif

#ifdef TOUCH_MONITOR
	init_proc();
#endif

	dbg_cr("FTS Probe Done without Error!\n");

	return 0;

#ifdef TSP_FACTORY_TEST
err_sysfs:
	if (info->irq_enabled)
		fts_irq_enable(info, false);
#endif

err_enable_irq:
	input_unregister_device(info->input_dev);
	info->input_dev = NULL;

err_register_input:
	if (info->input_dev)
		input_free_device(info->input_dev);

err_fts_init:
	complete_all(&info->init_done);

err_input_allocate_device:
	info->board->power(false);
	kfree(info);

	dbg_cr("FTS Probe Done with Error!\n");

	//return retval;
err_check_pantech:

	retval = misc_register(&touch_io);
	if (retval){
		dbg_cr("touch_io can''t register misc device\n");
	}
	return ret;

}

static int fts_remove(struct i2c_client *client)
{
	struct fts_ts_info *info = i2c_get_clientdata(client);

	dbg_cr("FTS removed \n");

	fts_interrupt_set(info, INT_DISABLE);
	fts_command(info, FLUSHBUFFER);

	fts_irq_enable(info, false);

	input_mt_destroy_slots(info->input_dev);

#ifdef TSP_FACTORY_TEST
	sysfs_remove_group(&info->fac_dev_ts->kobj,
			&sec_touch_factory_attr_group);

	device_destroy(pan_class, FTS_ID0);
	class_destroy(pan_class);

	list_del(&info->cmd_list_head);

	mutex_destroy(&info->cmd_lock);

	if (info->pFrame)
		kfree(info->pFrame);
#endif

#ifdef TOUCH_MONITOR
	remove_proc();
#endif

#ifdef PAN_TSP_IO	
	misc_deregister(&touch_event);
	misc_deregister(&touch_io);
#endif

#ifdef PAN_KNOCK_ON
	device_init_wakeup(info->dev, 0);
#endif

#ifdef CONFIG_POWERSUSPEND
	unregister_power_suspend(&fts_power_suspend);
#endif

	mutex_destroy(&info->lock);

	input_unregister_device(info->input_dev);
	info->input_dev = NULL;

	info->board->power(false);

	kfree(info);

	return 0;
}

static void fts_reinit(struct fts_ts_info *info)
{
	int rc;
	fts_wait_for_ready(info);

	fts_systemreset(info);

	rc=fts_wait_for_ready(info);
	if (rc==-2) {
		info->fw_version_of_ic =0;
		info->config_version_of_ic=0;
		info->fw_main_version_of_ic=0;
	} else
		fts_get_version_info(info);

#ifdef FTS_PANTECH_PRODUCT_LINE
	if(info->mflag_init_test)
		fts_check_product_line(info);
#endif /* FTS_PANTECH_PRODUCT_LINE */	

#ifdef FTS_SUPPORT_NOISE_PARAM
	fts_set_noise_param(info);
#endif

#ifdef PAN_TOUCH_SET_PEN_MODE
	if(info->touch_mode_state.touch == PAN_TOUCH_MODE_FINGER || info->touch_mode_state.touch == PAN_TOUCH_MODE_GLOVE){
		fts_stylus_mode_onoff(info,PAN_TOUCH_MODE_FINGER);
	}else{
		fts_command(info, SLEEPOUT);
		fts_command(info, SENSEON);
	}
#else
	fts_command(info, SLEEPOUT);
	fts_command(info, SENSEON);
#endif


	if (info->flip_enable) {
		fts_set_stylus_mode(info, false);
	} else {
		if (info->mshover_enabled)
			fts_command(info, FTS_CMD_MSHOVER_ON);
	}
#if defined(FTS_SUPPORT_TA_MODE) || defined (FTS_SUPPORT_TA_MODE_PANTECH)
	if (info->TA_Pluged)
		fts_command(info, FTS_CMD_CHARGER_PLUGGED);
#endif


	info->touch_count = 0;

	fts_command(info, FLUSHBUFFER);
	fts_interrupt_set(info, INT_ENABLE);
}

void fts_release_all_finger(struct fts_ts_info *info)
{
	int i;

	for (i = 0; i < FINGER_MAX; i++) {
		input_mt_slot(info->input_dev, i);
		input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, 0);

		if (info->finger[i].state != EVENTID_LEAVE_POINTER) {
			info->touch_count--;
			if (info->touch_count < 0)
				info->touch_count = 0;

			dbg_op("[R] tID:%d mc: %d tc:%d Ver[%02X]\n",
					i, info->finger[i].mcount,
					info->touch_count, info->panel_revision);
		}

		info->finger[i].state = EVENTID_LEAVE_POINTER;
		info->finger[i].mcount = 0;
	}

	input_report_key(info->input_dev, BTN_TOUCH, 0);
#ifdef CONFIG_GLOVE_TOUCH
	input_report_switch(info->input_dev, SW_GLOVE, false);
	info->touch_mode = FTS_TM_NORMAL;
#endif

#ifdef CONFIG_INPUT_BOOSTER
	INPUT_BOOSTER_REPORT_KEY_EVENT(info->input_dev, KEY_BOOSTER_TOUCH, 0);
	INPUT_BOOSTER_SEND_EVENT(KEY_BOOSTER_TOUCH, BOOSTER_MODE_FORCE_OFF);
#endif

	input_sync(info->input_dev);

#ifdef TSP_BOOSTER
	fts_set_dvfs_lock(info, TSP_BOOSTER_FORCE_OFF, false);
#endif
}

static int fts_stop_device(struct fts_ts_info *info)
{
	mutex_lock(&info->device_mutex);

	if (info->touch_stopped) {
		dbg_cr("%s already power off\n", __func__);
		goto out;
	}

	fts_interrupt_set(info, INT_DISABLE);
	disable_irq(info->irq);

	fts_command(info, FLUSHBUFFER);
	fts_command(info, SLEEPIN);

	fts_release_all_finger(info);

#ifdef FTS_SUPPORT_NOISE_PARAM
	fts_get_noise_param(info);
#endif

	info->touch_stopped = true;

	if (info->board->power)
		info->board->power(false);

out:
	mutex_unlock(&info->device_mutex);
	return 0;
}

static int fts_start_device(struct fts_ts_info *info)
{
	mutex_lock(&info->device_mutex);

	if (!info->touch_stopped) {
		dbg_cr("%s already power on\n", __func__);
		goto out;
	}

	if (info->board->power)
		info->board->power(true);

	info->touch_stopped = false;

	if(info->mflag_init_test){
		queue_work(fts_event_wq, &info->init_test_work);	  
		return 0;
	}else{  
		fts_reinit(info);
		enable_irq(info->irq);
	}
out:
	mutex_unlock(&info->device_mutex);
	return 0;

}

static int fts_suspend(struct fts_ts_info *info)
{
	dbg_cr("%s\n", __func__);
	fts_stop_device(info);
	return 0;
}

static int fts_resume(struct fts_ts_info *info)
{
	dbg_cr("%s\n", __func__);
	fts_start_device(info);
	return 0;
}

static const struct i2c_device_id fts_device_id[] = {
	{"stmicro_fts_ts", 0},
	{}
};

static struct of_device_id fts_match_table[] = {
	{ .compatible = "stmicro,stmicro_fts_ts",},
	{ },
};

static struct i2c_driver fts_i2c_driver = {
	.driver = {
		.name = "stmicro_fts_ts",
		.owner = THIS_MODULE,
		.of_match_table = fts_match_table,
	},
	.probe = fts_probe,
	.remove = fts_remove,

	.id_table = fts_device_id,
};

static int __init fts_driver_init(void)
{
	return i2c_add_driver(&fts_i2c_driver);
}

static void __exit fts_driver_exit(void)
{
	i2c_del_driver(&fts_i2c_driver);
}

MODULE_DESCRIPTION("STMicroelectronics MultiTouch IC Driver");
MODULE_AUTHOR("STMicroelectronics, Inc.");
MODULE_LICENSE("GPL v2");

module_init(fts_driver_init);
module_exit(fts_driver_exit);
