/* 
 * file_name: atmel_mxt_fw30.c
 *
 * description: atmel max touch driver.
 *
 * Copyright (C) 2008-2010 Atmel & Pantech Co. Ltd.
 * Copyright (C) 2013 Pantech Co. Ltd.
 *
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/input/mt.h>  //PROTOCOL_B 
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/earlysuspend.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <mach/vreg.h>
#include <linux/regulator/consumer.h>
#include <mach/gpio.h>
#include <linux/miscdevice.h>
#include <linux/hrtimer.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>
#include <linux/wakelock.h>
#include <linux/gpio_switch.h>

//#include "../../../../../vendor/pantech/build/CUST_PANTECH.h" 

#include "atmel_mxt_fw30.h"

#if defined(CONFIG_MACH_MSM8974_EF56S)
#include "ef56/atmel_540s_v30_cfg_multi.h"
#elif defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L)
#include "ef59/atmel_540s_v30_cfg_multi.h"
#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) || defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L)
#include "ef60_61_62/atmel_540s_v30_cfg_multi.h"
#else
#include "ef59/atmel_540s_v30_cfg_multi.h"
#endif

#ifdef ITO_TYPE_CHECK		//P13106 
#include <linux/qpnp/qpnp-adc.h>
#endif
#ifdef TOUCH_MONITOR
#include <linux/proc_fs.h>
#endif
#include <linux/qpnp/vibrator.h>            // for vib_debug
#ifdef OFFLINE_CHARGER_TOUCH_DISABEL
#include <mach/msm_smsm.h>
#endif


/* -------------------------------------------------------------------- */
/* function proto type & variable for driver							*/
/* -------------------------------------------------------------------- */
static int __devinit mxt_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devexit mxt_remove(struct i2c_client *client);
static int mxt_resume(struct i2c_client *client);
static int mxt_suspend(struct i2c_client *client, pm_message_t mesg);
#ifdef MXT_FIRMUP_ENABLE
void	MXT_reprogram(void);
uint8_t	MXT_Boot(bool withReset);
#endif // MXT_FIRMUP_ENABLE

/*------------------------------ functions prototype -----------------------------------*/
uint8_t init_touch_driver(void);
uint8_t reset_chip(void);
uint8_t calibrate_chip(void);
uint8_t diagnostic_chip(uint8_t mode);
uint8_t backup_config(void);

/*------------------------------ General Object Config Write----------------------------*/
uint8_t write_power_T7_config(gen_powerconfig_t7_config_t power_config);
uint8_t write_acquisition_T8_config(gen_acquisitionconfig_t8_config_t acq_config);
void mxt_Acquisition_Config_T8_Init(void);
/*------------------------------ Touch Object Config Write------------------------------*/
uint8_t write_keyarray_T15_config(uint8_t key_array_number, touch_keyarray_t15_config_t cfg);
uint8_t write_proximity_T23_config(uint8_t instance, touch_proximity_t23_config_t cfg);
uint8_t write_multitouchscreen_T100_config(uint8_t instance, touch_multitouchscreen_t100_config_t cfg);
/*------------------------------ Signal Processing Object Config Write------------------*/
uint8_t write_grip_suppression_T40_config(proci_gripsuppression_t40_config_t cfg);
uint8_t write_touch_suppression_T42_config(proci_touchsuppression_t42_config_t cfg);
uint8_t write_stylus_T47_config(proci_stylus_t47_config_t cfg);
uint8_t write_adaptivethreshold_T55_config(proci_adaptivethreshold_t55_config_t cfg);
uint8_t write_shieldless_T56_config(proci_shieldless_t56_config_t cfg);
uint8_t write_lensbending_T65_config(uint8_t instance, proci_lensbending_t65_config_t cfg);
uint8_t write_palmgestureprocessor_T69_config(proci_palmgestureprocessor_t69_config_t cfg);
uint8_t write_noisesuppression_T72_config(procg_noisesuppression_t72_config_t cfg);
uint8_t write_glovedetection_T78_config(proci_glovedetection_t78_config_t cfg);
uint8_t write_retransmissioncompensation_T80_config(proci_retransmissioncompensation_t80_config_t cfg);
uint8_t write_gesturepocessor_T84_config(proci_gesturepocessor_t84_config_t cfg);
uint8_t write_schnoisesuppression_T103_config(uint8_t instance, proci_schnoisesuppression_t103_config_t cfg);

/*------------------------------ Support Processing Object Config Write------------------*/
uint8_t write_comms_T18_config(uint8_t instance, spt_commsconfig_t18_config_t cfg);
uint8_t write_gpiopwm_T19_config(uint8_t instance, spt_gpiopwm_t19_config_t cfg);
uint8_t write_selftest_T25_config(uint8_t instance, spt_selftest_t25_config_t cfg);
uint8_t write_CTE_T46_config(uint8_t instance, spt_cteconfig_t46_config_t cfg);
uint8_t write_timer_T61_config(spt_timer_t61_config_t cfg);
uint8_t write_goldenreferences_T66_config(spt_goldenreferences_t66_config_t cfg);
uint8_t write_dynamicconfigurationcontroller_t70_config(uint8_t instance, spt_dynamicconfigurationcontroller_t70_config_t cfg);
uint8_t write_dynamicconfigurationcontainer_t71_config(spt_dynamicconfigurationcontainer_t71_config_t cfg);
uint8_t write_touchscreenhover_T101_config(uint8_t instance, spt_touchscreenhover_t101_config_t cfg);
uint8_t write_selfcapcbcrconfig_T102_config(uint8_t instance, spt_selfcapcbcrconfig_t102_config_t cfg);
uint8_t write_auxtouch_T104_config(spt_auxtouchconfig_t104_config_t cfg);
uint8_t write_drivenplatehover_T105_config(spt_drivenplatehoverconfig_t105_config_t cfg);

void mxt_Multitouchscreen_T100_Init(void);
void mxt_Stylus_T47_Config_Init(void);

uint8_t write_simple_config(uint8_t object_type, uint8_t instance, void *cfg);
uint8_t get_object_size(uint8_t object_type);
uint8_t read_id_block(info_id_t *id);
uint16_t get_object_address(uint8_t object_type, uint8_t instance);
uint32_t get_stored_infoblock_crc(void);
uint8_t calculate_infoblock_crc(uint32_t *crc_pointer);
uint32_t CRC_24(uint32_t crc, uint8_t byte1, uint8_t byte2);
int TSP_PowerOn(void);							
void TSP_reset_pin_shake(void);								
int8_t check_chip_calibration(void);
int8_t get_touch_antitouch_info(void);
void quantum_touch_probe(void);
U8 read_mem(U16 start, U8 size, U8 *mem);
U8 read_U16(U16 start, U16 *mem);
U8 write_mem(U16 start, U8 size, U8 *mem);
void  clear_event(uint8_t clear);

#ifdef SKY_PROCESS_CMD_KEY
static int* diag_debug(int command); 
static long ts_fops_ioctl(struct file *filp,unsigned int cmd, unsigned long arg);
static int ts_fops_open(struct inode *inode, struct file *filp);
#endif //SKY_PROCESS_CMD_KEY
#ifdef TOUCH_IO
static int open(struct inode *inode, struct file *file);
static int release(struct inode *inode, struct file *file);
static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos);
static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos);
static long ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int ioctl_debug(unsigned long arg);
static void apply_touch_config(void); 
static void reset_touch_config(void);
#endif // TOUCH_IO
#ifdef ITO_TYPE_CHECK
static int read_touch_id(void);
//static int tsp_ito_type = -1;
#endif //ITO_TYPE_CHECK
void  mxt_front_test_init(void);
void TSP_PowerOff(void);
#ifdef TOUCH_MONITOR
void cbInit(CircularBuffer *cb, int size);
void cbFree(CircularBuffer *cb); 
int cbIsFull(CircularBuffer *cb);
int cbIsEmpty(CircularBuffer *cb);
void cbWrite(CircularBuffer *cb, char *elem); 
void cbRead(CircularBuffer *cb, char *elem);
int read_log(char *page, char **start, off_t off, int count, int *eof, void *data_unused); 
int read_touch_info(char *page, char **start, off_t off, int count, int *eof, void *data_unused);
void printp(const char *fmt, ...);
void init_proc(void);
void remove_proc(void);
/*------------------------------ Touch Get Message Operation Funtion------------------*/
static uint8_t get_object_type(uint8_t *quantum_msg);
static void get_message_T6(uint8_t *quantum_msg);
static void get_message_T15(uint8_t *quantum_msg); 
static void get_message_T66(uint8_t *quantum_msg); 

//++ p11309 - 2013.10.21 for Palm Gesture
static void get_message_T69(uint8_t *quantum_msg); 
//-- p11309
static void get_message_T72(uint8_t *quantum_msg); 
static int get_message_T100(uint8_t *quantum_msg, unsigned int *touch_status);
static void report_input (int touch_status);
char printproc_buf[1024];
char touch_info_vendor[] = "atmel";
char touch_info_chipset[] = "mxt540s_fw30";
CircularBuffer cb;
spinlock_t cb_spinlock;
#endif

static ssize_t mxt_debug_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t mxt_debug_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
static ssize_t mxt_mem_access_read(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count);
static ssize_t mxt_mem_access_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off,size_t count);
static int mxt_check_mem_access_params(struct mxt_fw30_data_t *data, loff_t off, size_t *count);
static int __mxt_read_reg(struct i2c_client *client,u16 reg, u16 len, void *val);
static int __mxt_write_reg(struct i2c_client *client, u16 reg, u16 len,const void *val);

static DEVICE_ATTR(debug_enable, S_IWUSR | S_IRUSR, mxt_debug_enable_show,mxt_debug_enable_store);

#ifdef VIBRATOR_PANTECH_PATCH
// pantech vib driver debug enable/disable function 
extern void pantech_vib_debug_enable(void);
extern void pantech_vib_debug_disable(void);
#endif


//++ p11309 - 2013.05.14 for gold reference T66, add 2013.05.26 for stabilization & dual x
#define PAN_GLD_REF_STATUS_IDLE				0x00
#define PAN_GLD_REF_STATUS_BAD_DATA			0x01
#define PAN_GLD_REF_STATUS_PRIMED			0x02
#define PAN_GLD_REF_STATUS_GENERATED		0x04
#define PAN_GLD_REF_STATUS_SEQ_ERROR		0x08
#define PAN_GLD_REF_STATUS_SEQ_TIMEOUT		0x10
#define PAN_GLD_REF_STATUS_SEQ_DONE			0x20
#define PAN_GLD_REF_STATUS_PASS				0x40
#define PAN_GLD_REF_STATUS_FAIL				0x80
#define PAN_GLD_REF_CMD_NONE            0x00
#define PAN_GLD_REF_CMD_ENABLE          0x01
#define PAN_GLD_REF_CMD_REPORT          0x02
#define PAN_GLD_REF_CMD_PRIME           0x04
#define PAN_GLD_REF_CMD_GENERATE        0x08
#define PAN_GLD_REF_CMD_CONFIRM         0x0C
#define PAN_GLD_REF_PROGRESS_IDLE			0x00
#define PAN_GLD_REF_PROGRESS_CAL			0x01
#define PAN_GLD_REF_PROGRESS_PRIME			0x02
#define PAN_GLD_REF_PROGRESS_GENERATE		0x04
#define PAN_GLD_REF_PROGRESS_COMPLETE_FAIL	0x08
#define PAN_GLD_REF_PROGRESS_COMPLETE_PASS	0x10
#define PAN_GLD_REF_PROGRESS_DELTA_PASS	    0x20
#define PAN_GLD_REF_PROGRESS_DELTA_FAIL	    0x40

static int pan_gld_ref_ic_status = PAN_GLD_REF_STATUS_IDLE;
static int pan_gld_ref_cal_status = PAN_GLD_REF_STATUS_IDLE;

static u8 pan_gld_ref_max_diff = 0;
static u8 pan_gld_ref_max_diff_x = 0;
static u8 pan_gld_ref_max_diff_y = 0;

//p16619 keep this code when deleting switch device code
struct wake_lock touch_pen_wakelock;
#define TOUCH_PEN_WAKELOCK_TIMEOUT 500
#define TOUCH_MODE_CHANGE_WAKELOCK_TIMEOUT 200

//++ p11309 - 2013.07.19 Check Noise Mode shake
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
static int pan_noise_mode_shake_enable = 0;
static int pan_noise_mode_shake_count = 0;
static int pan_noise_mode_shake_state = 0;
#endif
//++ p11309 - 2013.07.19 Support Soft Dead zone
#ifdef PAN_SUPPORT_SOFT_DEAD_ZONE
static int pan_support_soft_dead_zone = 1;
#endif
//-- p11309

#ifdef PAN_TOUCH_CAL_COMMON
static debug_info_t debugInfo;
static uint8_t cal_check_flag = 0u;
unsigned char not_yet_count = 0;
#endif

#ifdef PAN_TOUCH_CAL_PMODE

#define PAN_PMODE_TCHAUTOCAL 			10  /* 10*(200ms) */
#define PAN_PMODE_ATCHCALST				0
#define PAN_PMODE_ATCHCALSTHR			0
#define PAN_PMODE_ATCHFRCCALTHR			70        
#define PAN_PMODE_ATCHFRCCALRATIO		-120     
#define PAN_PMODE_AUTOCAL_ENABLE_TIME	6000
#define PAN_PMODE_ANTICAL_ENABLE_TIME	500

static uint8_t cal_correction_limit = 0;
static bool pan_pmode_resume_cal=false;

struct workqueue_struct *pan_pmode_work_queue;
struct work_struct pan_pmode_antical_wq;
struct work_struct pan_pmode_autocal_wq;
//2014.3.27 p13106 
//static struct timer_list pan_pmode_antical_timer;
static struct timer_list pan_pmode_autocal_timer;
//2014.3.27 p13106
//static void pan_pmode_antical_timer_func(unsigned long data);
static void pan_pmode_autocal_timer_func(unsigned long data);
void pan_pmode_autocal_wq_func(struct work_struct * p);
void pan_pmode_antical_wq_func(struct work_struct * p);
void pan_touch_protection_mode(void);
#endif
//-- p11309

static const struct i2c_device_id mxt_fw30_id[] = {
	{ "atmel_mxt_fw30", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mxt_fw30_id);

static struct of_device_id mxt_match_table[] = {
	{ .compatible = "atmel,atmel_mxt_540s",},
	{ },
};

static struct i2c_driver mxt_fw30_driver = {
	.driver = {
		.name	= "Atmel MXT540S",
		.owner	= THIS_MODULE,
		.of_match_table = mxt_match_table,
	},
	.probe		= mxt_probe,
	.remove		= __devexit_p(mxt_remove),
	.id_table	= mxt_fw30_id,
};

#ifdef SKY_PROCESS_CMD_KEY
static struct file_operations ts_fops = {
	.owner = THIS_MODULE,
	.open = ts_fops_open,
	.unlocked_ioctl = ts_fops_ioctl, // mirinae
};

static struct miscdevice touch_event = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch_fops",
	.fops = &ts_fops,
};
#endif



/* -------------------------------------------------------------------- */
/* ----------- Global Variable----------- */
/* -------------------------------------------------------------------- */
static struct class *touch_atmel_class;
struct workqueue_struct *mxt_fw30_wq;
struct mxt_fw30_data_t *mxt_fw30_data = NULL;

static report_finger_info_t fingerInfo[MAX_NUM_FINGER];
static report_key_info_t keyInfo[MAX_NUM_FINGER];
static info_block_t *info_block = NULL;
volatile uint8_t read_pending;
static int max_report_id = 0;
static uint8_t max_message_length;
static uint16_t message_processor_address;
static uint16_t command_processor_address;                           /*! Command processor address. */ 
static enum driver_setup_t driver_setup = DRIVER_SETUP_INCOMPLETE;   /*! Flag indicating if driver setup is OK. */
static uint8_t *quantum_msg_total = NULL; 
static U16 address_pointer;                                           /*! \brief The current address pointer. */
static unsigned int mxt_time_point;
static unsigned int good_calibration_cnt;
static bool active_event = true;
static bool is_cal_success = false;
#ifdef TOUCH_IO
static bool touch_diagnostic_ret = true;
#endif //TOUCH_IO
#ifdef SKY_PROCESS_CMD_KEY
static int diagnostic_min =0;
static int diagnostic_max =0;
#endif //SKY_PROCESS_CMD_KEY
static int reference_data[540] = {0};
static struct attribute *mxt_attrs[] = {
	&dev_attr_debug_enable.attr,
	NULL
};


//++ p11309 - 2013.07.30 for direct set smart cover - add 2013.08.26 check hallic vs ui state.
#ifdef PAN_SUPPORT_SMART_COVER
#define TOUCH_COVER_OPENED		  0
#define TOUCH_COVER_CLOSED		  1
static int mTouch_cover_status_hallic = TOUCH_COVER_OPENED;
static int mTouch_cover_status_ui = TOUCH_COVER_OPENED;
static struct timer_list pan_hallic_ui_sync_timer;
static int touch_probe_state = 0;

static void pan_hallic_ui_sync_timer_func(unsigned long data)
{
	mTouch_cover_status_hallic = mTouch_cover_status_ui;
	dbg_cr("[hallic ui sync timer] Touch Cover Status is %d\n", mTouch_cover_status_hallic);
}
#endif

void set_smart_cover(int cover) 
{

#ifdef PAN_SUPPORT_SMART_COVER

	mTouch_cover_status_hallic = cover;

	if ( touch_probe_state == 0 ) return;	
	dbg_cr("[EXPORT_SYMBOL] Touch Cover Status is %d\n", mTouch_cover_status_hallic);	

	mod_timer(&pan_hallic_ui_sync_timer, jiffies + msecs_to_jiffies(500));
	//p13106 EF59 Clear Touch Event.
	clear_event(TSC_CLEAR_ALL);
#endif

}
EXPORT_SYMBOL(set_smart_cover);
//-- p11309

#ifdef PAN_TOUCH_PEN_DETECT
void set_pan_touch_state(int pen){
  dbg_touch("set_pan_touch_state -> %d\n",pen);
  mxt_fw30_data->pan_touch_pen_state=pen;
  return;
}
#endif

#ifdef SKY_PROCESS_CMD_KEY
static int ts_fops_open(struct inode *inode, struct file *filp)
{
	filp->private_data = mxt_fw30_data;
	return 0;
}

static long ts_fops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;	
  pm_message_t pm_null={0};
//++ p11309 - 2013.07.10 for Get Touch Mode
	int touch_mode_flag = 0;
//++ p11309 - 2013.07.19 Check Noise Mode shake
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
	int pan_noise_mode_shake_count_temp = 0;
#endif
//-- p11309 
//-- p11309 

	dbg_ioctl("touch_fops IOCTL function. cmd => %d, arg -> %lu\n",cmd, arg);	

	switch (cmd) 
	{
	case TOUCH_IOCTL_READ_LASTKEY:
		break;
	case TOUCH_IOCTL_DO_KEY:
		dbg_ioctl("TOUCH_IOCTL_DO_KEY  = %d\n",(int)argp);			
		if ( (int)argp == KEY_NUMERIC_STAR )
			input_report_key(mxt_fw30_data->input_dev, 0xe3, 1);
		else if ( (int)argp == KEY_NUMERIC_POUND )
			input_report_key(mxt_fw30_data->input_dev, 0xe4, 1);
		else
			input_report_key(mxt_fw30_data->input_dev, (int)argp, 1);
		input_sync(mxt_fw30_data->input_dev); 
		break;
	case TOUCH_IOCTL_RELEASE_KEY:		
		dbg_ioctl("TOUCH_IOCTL_RELEASE_KEY  = %d\n",(int)argp);
		if ( (int)argp == 0x20a )
			input_report_key(mxt_fw30_data->input_dev, 0xe3, 0);
		else if ( (int)argp == 0x20b )
			input_report_key(mxt_fw30_data->input_dev, 0xe4, 0);
		else
			input_report_key(mxt_fw30_data->input_dev, (int)argp, 0);
		input_sync(mxt_fw30_data->input_dev); 
		break;		
	case TOUCH_IOCTL_DEBUG:
		dbg_ioctl("Touch Screen Read Queue ~!!\n");	
		queue_work(mxt_fw30_wq, &mxt_fw30_data->work);
		break;
	case TOUCH_IOCTL_CLEAN:
		dbg_ioctl("Touch Screen Previous Data Clean ~!!\n");
		clear_event(TSC_CLEAR_ALL);
		break;
	case TOUCH_IOCTL_RESTART:
		dbg_ioctl("Touch Screen Calibration Restart ~!!\n");			
		calibrate_chip();
		break;
	case TOUCH_IOCTL_PRESS_TOUCH:
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TOUCH_MAJOR, 255);
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_WIDTH_MAJOR, 1);			
		input_sync(mxt_fw30_data->input_dev);
		break;
	case TOUCH_IOCTL_RELEASE_TOUCH:		
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_X, (int)(arg&0x0000FFFF));
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_Y, (int)((arg >> 16) & 0x0000FFFF));
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_WIDTH_MAJOR, 1);			
		input_sync(mxt_fw30_data->input_dev); 
		break;			
	case TOUCH_IOCTL_CHARGER_MODE:
		break;
	case POWER_OFF:
		pm_power_off();
		break;
	case TOUCH_IOCTL_DELETE_ACTAREA:
		touchscreen_config.yloclip = 0;		// Change Active area
		touchscreen_config.yhiclip = 0;
		if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK){
			dbg_ioctl("mxt_Multitouchscreen_config Error!!!\n");
		}
		break;
	case TOUCH_IOCTL_RECOVERY_ACTAREA:
		touchscreen_config.yloclip = 15;	// Change Active area
		touchscreen_config.yhiclip = 15;
		if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK)
			dbg_ioctl("mxt_Multitouchscreen_config Error!!!\n");
		break;
	case TOUCH_IOCTL_INIT:
		dbg_ioctl("Touch init \n");
		mxt_front_test_init();
		break;

	case TOUCH_IOCTL_OFF:
		dbg_ioctl("Touch off \n");
		TSP_PowerOff();
		break;		

	case TOUCH_IOCTL_SENSOR_X:
		{
			int send_data;
			send_data = touchscreen_config.xsize;

			if (copy_to_user(argp, &send_data, sizeof(send_data)))
				return false;
		}
		break;
	case TOUCH_IOCTL_SENSOR_Y:
		{
			int send_data;
			send_data = touchscreen_config.ysize;
			if (copy_to_user(argp, &send_data, sizeof(send_data)))
				return false;
		}
		break;

	case TOUCH_IOCTL_CHECK_BASE:
	case TOUCH_IOCTL_START_UPDATE:
		break;

	case TOUCH_IOCTL_SELF_TEST:
		{
			int* send_byte;
			disable_irq(mxt_fw30_data->client->irq);
			mutex_lock(&mxt_fw30_data->lock);
			send_byte = diag_debug(MXT_540S_REFERENCE_MODE);
			mutex_unlock(&mxt_fw30_data->lock);
			enable_irq(mxt_fw30_data->client->irq);
			diagnostic_min = MXT_REFERENCE_MIN;
			diagnostic_max = MXT_REFERENCE_MAX;

			if (copy_to_user(argp, send_byte, sizeof(int) * MXT_540S_MAX_CHANNEL_NUM))
				return false;

			return touch_diagnostic_ret;
			break;
		}
	case TOUCH_IOCTL_DIAGNOSTIC_MIN_DEBUG:
		return MXT_REFERENCE_MIN;
		break;
	case TOUCH_IOCTL_DIAGNOSTIC_MAX_DEBUG:
		return MXT_REFERENCE_MAX;
		break;
	case TOUCH_IOCTL_DIAG_DELTA:
		{
			int* send_byte;
			disable_irq(mxt_fw30_data->client->irq);
			mutex_lock(&mxt_fw30_data->lock);
			send_byte = diag_debug(MXT_540S_DELTA_MODE);
			mutex_unlock(&mxt_fw30_data->lock);
			enable_irq(mxt_fw30_data->client->irq);
			if (copy_to_user(argp, send_byte, sizeof(int) * MXT_540S_MAX_CHANNEL_NUM))
				return false;

			return touch_diagnostic_ret;
			break;
		}

	case TOUCH_IOCTL_MULTI_TSP_OBJECT_SEL:
		// format: obj_sel_# (0~9)
		if ( arg < TOUCH_MODE_MAX_NUM ) 
		{
			if(mTouch_mode == (int)arg) {
				dbg_cr("[touch_monitor] Touch Mode selection is equal, mTouch_mode -> %d\n",mTouch_mode);
			}
			else {
  				if(mxt_fw30_data->state == SUSMODE)	{
                    //p16619 keep this code when deleting switch device code
  					wake_lock_timeout(&touch_pen_wakelock,msecs_to_jiffies(TOUCH_MODE_CHANGE_WAKELOCK_TIMEOUT));

					mTouch_mode = arg;

        			reset_touch_config();
        	if(pan_pmode_resume_cal){
        	  pan_touch_protection_mode();
        	}
					dbg_cr("[touch_monitor] Suspend Touch Mode selection is %d\n", mTouch_mode);
				}
				else
				{		    
					mTouch_mode = arg;
        			disable_irq(mxt_fw30_data->client->irq);
					reset_touch_config();
					if(pan_pmode_resume_cal){
        	  pan_touch_protection_mode();
        	}
					enable_irq(mxt_fw30_data->client->irq);

					dbg_cr("[touch_monitor] Touch Mode selection is %d\n", mTouch_mode);
				}
			}
		}
		//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
		else if ( arg == 10 || arg == 11 ) {
			mTouch_cover_status_ui = arg - 10;
		//++ add 2013.08.26 check hallic vs ui state.
			mTouch_cover_status_hallic = mTouch_cover_status_ui;
    // ++ p13106 for calibrate_chip when smart cover open.
      if(mTouch_cover_status_hallic == TOUCH_COVER_OPENED){
        calibrate_chip();
      }
    // -- p13106 for calibrate_chip when smart cover open.
      
      if ( touch_probe_state == 1 )
				del_timer_sync(&pan_hallic_ui_sync_timer);
		//--
			dbg_cr("[touch_fops] Touch Cover Status is %d\n", mTouch_cover_status_ui);
		}
#endif
		//-- p11309
		else if ( arg == 20 || arg == 21 ) {
		  if(mTouch_mode ==2 || mTouch_mode ==3){
		    if(arg == 20){
		      dbg_cr("Touch Pen Idle mode\n");
		      mxt_Multitouchscreen_T100_Init();
		      mxt_Stylus_T47_Config_Init();
		    }else if(arg == 21){
		      dbg_cr("Touch Pen Text mode\n");
		      touchscreen_config = obj_multi_touch_t100[mTouch_mode];
		      touchscreen_config.movhysti=2;
		      touchscreen_config.movhystn=1;
		      if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK)
		        dbg_cr("T100 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);

		      stylus_t47_config = obj_stylus_t47[mTouch_mode];
          stylus_t47_config.cfg=3;
		      if (write_stylus_T47_config(stylus_t47_config) != CFG_WRITE_OK)
			      dbg_cr("T47 Configuration Fail!!! , Line %d \n\r", __LINE__);		      
		    }
		  }
		}
		break;
	case TOUCH_IOCTL_SUSPEND :
	   mxt_suspend(NULL,pm_null);
	   break;

	case TOUCH_IOCTL_RESUME :
		mxt_resume(NULL);
		break;

//++ p11309 - 2013.07.10 for Get Touch Mode
	case TOUCH_IOCTL_MULTI_TSP_OBJECT_GET:
#ifdef PAN_SUPPORT_SMART_COVER
		touch_mode_flag = mTouch_mode + mTouch_cover_status_ui*10;
#else
		touch_mode_flag = mTouch_mode;
#endif
		if (copy_to_user(argp, &touch_mode_flag, sizeof(touch_mode_flag))) {
			dbg_ioctl(" TOUCH_IOCTL_MULTI_TSP_OBJECT_GET\n");
		}		
		break;
//-- p11309


	
//++ p11309 - 2013.07.19 Check Noise Mode shake
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
	case TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_ENABLE:
		pan_noise_mode_shake_enable = 1;
		pan_noise_mode_shake_count = 0;
		break;
	case TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_DISABLE:
		pan_noise_mode_shake_enable = 0;
		break;
	case TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_GET:
		pan_noise_mode_shake_count_temp = pan_noise_mode_shake_count + pan_noise_mode_shake_state*1000;
		if (copy_to_user(argp, &pan_noise_mode_shake_count_temp, sizeof(pan_noise_mode_shake_count_temp))) {
			dbg_ioctl("TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_GET\n");
		}		
		break;
#endif
//++ p11309 - 2013.07.25 Get Model Color 
#ifdef ITO_TYPE_CHECK
	case TOUCH_IOCTL_MODEL_COLOR_GET:		
		if (copy_to_user(argp, &mxt_fw30_data->ito_color, sizeof(mxt_fw30_data->ito_color))) {
			dbg_ioctl("TOUCH_IOCTL_MODEL_COLOR_GET\n");
		}		
		break;
#endif		
  case TOUCH_IOCTL_HALL_IC_GPIO_GET:

    touch_mode_flag = gpio_get_value(9);
    if (copy_to_user(argp, &touch_mode_flag, sizeof(touch_mode_flag))) {
			dbg_ioctl("TOUCH_IOCTL_HALL_IC_GPIO_GET\n");
		}
		dbg_op("[%s] Hall ic Status: %d\n", __func__, touch_mode_flag);
    break;
#ifdef PAN_TOUCH_PEN_DETECT
	case TOUCH_IOCTL_CHECK_TOUCH_PEN:
		dbg_op("[%s] Touch Pen Status: %d\n", __func__, mxt_fw30_data->pan_touch_pen_state);
		if (copy_to_user(argp, &mxt_fw30_data->pan_touch_pen_state, sizeof(mxt_fw30_data->pan_touch_pen_state))) {
			dbg_ioctl("TOUCH_IOCTL_CHECK_TOUCH_PEN\n");
		}		
		break;		
#endif    
	default:
		break;
	}
	return true;
}
#endif

#ifdef TOUCH_IO
static int* diag_debug(int command) 
{
	/*command 0x10: Delta, 0x11: Reference*/
	uint8_t data_buffer[130] = { 0 };
	uint8_t data_byte = 0; /* dianostic command to get touch refs*/
	uint16_t diag_address;
	uint8_t page;
	int i;
	int j=0;
	uint16_t value;
	uint16_t max_page = 9; // max_page = ceil(540 / (128/2) );
	int16_t signed_value;
	int rc=0;

	dbg_func_in();


	if (driver_setup != DRIVER_SETUP_OK){
		mutex_unlock(&mxt_fw30_data->lock);
		enable_irq(mxt_fw30_data->client->irq);
		return NULL;
	}

	touch_diagnostic_ret = true;
	diagnostic_min = 0;
	diagnostic_max = 0;
	/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
	diagnostic_chip(command);
	msleep(20); 

	/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
	//memset( data_buffer , 0xFF, sizeof( data_buffer ) );
	diag_address = get_object_address(DEBUG_DIAGNOSTIC_T37,0);
	dbg_i2c("[TOUCH] diag_Address -> %d\n",diag_address);
 	/* data array is 20 x 16 bits for each set of flags, 2 byte header, 40 bytes for touch flags 40 bytes for antitouch flags*/
	/* count up the channels/bits if we recived the data properly */
	for (page = 0; page < max_page; page++) {
	
    rc=read_mem(diag_address, 130,data_buffer);
    msleep(20);
    dbg_i2c("[TOUCH] Current mode => %d, Current Page => %d, j-> %d\n",data_buffer[0],data_buffer[1],j);
    if(rc != READ_MEM_OK)
      dbg_i2c("[TOUCH] read_mem is failed.\n");
    
		for(i = 2; i < 130; i+=2) /* check X lines - data is in words so increment 2 at a time */
		{
			value =  (data_buffer[1+i]<<8) + data_buffer[i];
			if(j>=MXT_540S_MAX_CHANNEL_NUM)continue;
			if (command == MXT_540S_REFERENCE_MODE){
				reference_data[j] = value;			
			}
			else if (command == MXT_540S_DELTA_MODE){
				signed_value = value;
				reference_data[j] = (int16_t)value;
				diagnostic_min = min(diagnostic_min, reference_data[j] );
				diagnostic_max = max(diagnostic_max, reference_data[j] );
			}
			else{
				reference_data[j] = value;
				//diagnostic_min = min(diagnostic_min, reference_data[j] );
				//diagnostic_max = max(diagnostic_max, reference_data[j] );
			}
			j++;
		}
   		
		data_byte = 0x01;
		write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte);
		msleep(20);
	}
	return (int *)reference_data;;
}

static struct file_operations fops = 
{
	.owner =    THIS_MODULE,
	.unlocked_ioctl =    ioctl,  // mirinae
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

static int open(struct inode *inode, struct file *file) 
{
	return 0; 
}

static int release(struct inode *inode, struct file *file) 
{
	return 0; 
}
static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	int i=0,j,nBufSize=0;
	if((size_t)(*ppos) > 0) return 0;	
  dbg_ioctl(" Touch IO Write function\n");

	if(buf!=NULL)
	{
		nBufSize=strlen(buf);
		if(strncmp(buf, "queue", 5)==0)
		{
			queue_work(mxt_fw30_wq, &mxt_fw30_data->work);
		}

		if(strncmp(buf, "debug_",6)==0)
		{ 
			if(buf[6] > '0'){
				i = buf[6] - '1';
				if(pan_debug_state & 0x00000001 <<i) {
					pan_debug_state &= ~(0x00000001 <<i);	
				}else{
					pan_debug_state |= (0x00000001 <<i);
				}
			}
			dbg_cr(" pan_debug_state -> %x, i-> %d\n",pan_debug_state,i);
		}

		if(strncmp(buf, "wifion", 6)==0)
		{		 
			if(sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&mxt_fw30_data->mem_access_attr.attr,S_IRWXUGO))
				printk("[TOUCH] sysfs_chmod_file is failed\n");
			i=sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&(*mxt_attrs[0]), S_IRWXUGO);
		}
		if(strncmp(buf, "wifioff", 7)==0)
		{
			if(sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&mxt_fw30_data->mem_access_attr.attr,S_IRUGO | S_IWUSR))
				printk("[TOUCH] sysfs_chmod_file is failed\n");		
			i=sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&(*mxt_attrs[0]), S_IWUSR | S_IRUSR);
		}

#ifdef VIBRATOR_PANTECH_PATCH

		if(strncmp(buf, "vibon", 5)==0)
		{
		  pantech_vib_debug_enable();
		}
		if(strncmp(buf, "viboff", 6)==0)
		{
		  pantech_vib_debug_disable();
		}
#endif
		if(strncmp(buf, "touchid", 7)==0)
		{	
			j=0;
			for (i = 0 ; i < MAX_NUM_FINGER; ++i) { 
				if (fingerInfo[i].status == -1)
					continue;
				j++;
				dbg_ioctl("[TOUCH] TOUCH ID => %d, fingerInfo[i].status=> %d, fingerInfo[i].mode=> %d\n",i,fingerInfo[i].status,fingerInfo[i].mode); 
			}
		//	dbg_ioctl("[TOUCH] TOUCH ID CNT => %d, cal_correction_limit=> %d\n", j, cal_correction_limit); 
		}
		if(strncmp(buf, "checkcal", 8)==0)
		{			
			check_chip_calibration();
		}
		if(strncmp(buf, "cal", 3)==0)
		{			
			calibrate_chip();
		}
		if(strncmp(buf, "reset", 5)==0)
		{			
			reset_chip();
		}
		if(strncmp(buf, "save", 4)==0)
		{			
			backup_config();	    
		}
		if(strncmp(buf, "reference", 9)==0)
		{			
			disable_irq(mxt_fw30_data->client->irq);
			mutex_lock(&mxt_fw30_data->lock);
			diag_debug(MXT_540S_REFERENCE_MODE);
			mutex_unlock(&mxt_fw30_data->lock);
			enable_irq(mxt_fw30_data->client->irq);
		}
		if(strncmp(buf, "delta", 5)==0)
		{			
			disable_irq(mxt_fw30_data->client->irq);
			mutex_lock(&mxt_fw30_data->lock);
			diag_debug(MXT_540S_DELTA_MODE);
			mutex_unlock(&mxt_fw30_data->lock);
			enable_irq(mxt_fw30_data->client->irq);
		}
		if(strncmp(buf, "init",4)==0)
		{			
			mxt_front_test_init();   
		}
		if(strncmp(buf, "off",3)==0)
		{			
			TSP_PowerOff();   
		}		
		if(strncmp(buf, "on",2)==0)
		{			
			TSP_PowerOn();
		}		
		if(strncmp(buf, "obj_sel", 7)==0)
		{
			// format: obj_sel_# (0~9)
			i = buf[7] - '0';
			if ( i < TOUCH_MODE_MAX_NUM ) {
				mTouch_mode = i;
                disable_irq(mxt_fw30_data->client->irq);
                reset_touch_config();
	            enable_irq(mxt_fw30_data->client->irq);
	      
				dbg_cr(" TOUCH MODE selection is %d\n", mTouch_mode);
			}			
//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
			else if ( i == 10 || i == 11 ) {
				mTouch_cover_status_ui = i - 10;
			//++ add 2013.08.26 check hallic vs ui state.
				mTouch_cover_status_hallic = mTouch_cover_status_ui;
      // ++ p13106 for calibrate_chip when smart cover open.
        if(mTouch_cover_status_hallic == TOUCH_COVER_OPENED){
          calibrate_chip();
        }
      // -- p13106 for calibrate_chip when smart cover open.
        if ( touch_probe_state == 1 )
					del_timer_sync(&pan_hallic_ui_sync_timer);
			//--
				dbg_cr("[touch_monitor] Touch Cover Status is %d\n", mTouch_cover_status_ui);
			}		
#endif
//-- p11309
		}			

//++ p11309 - 2013.07.19 Check Noise Mode shake
#ifdef PAN_SUPPORT_SOFT_DEAD_ZONE
		if(strncmp(buf, "soft_dz_on", 10)==0)
		{ 
			pan_support_soft_dead_zone = 1;
		}		
		if(strncmp(buf, "soft_dz_off", 11)==0)
		{ 
			pan_support_soft_dead_zone = 0;
		}	
#endif


#ifdef ITO_TYPE_CHECK
		if(strncmp(buf, "id",2)==0)
		{			
			read_touch_id();   
		}
#endif
		if(strncmp(buf, "read_hallic",11)==0){
			dbg_cr("hall-ic gpio -> %d, mTouch_cover_status_hallic -> %d\n",gpio_get_value(9),mTouch_cover_status_hallic);
		}

	}

	*ppos +=nBufSize;
	return nBufSize;
}

static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	return 0; 
}

static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	config_table_element config;
	void __user *argp = (void __user *)arg;
	int return_value = -1;
	int data, object_type, field_index;
	pm_message_t pm_null={0};
//++ p11309 - 2013.07.10 for Get Touch Mode
	int touch_mode_flag = 0;
//-- p11309 

	dbg_ioctl("Touch Monitor IOCTL function. cmd => %d, arg -> %lu\n",cmd, arg);

	switch (cmd)
	{
	  case READ_TOUCH_ID:
	    return ((vendor_id<<16) + (model_id<<4) + type_id);
	    break;
	  
	case APPLY_TOUCH_CONFIG:
		apply_touch_config();
		break;
		
  case DIAG_DEBUG:
		dbg_ioctl(" DIAG_DEBUG arg => %d\n",(int)arg);
		if (arg == 5010) 
		{
			disable_irq(mxt_fw30_data->client->irq);
			mutex_lock(&mxt_fw30_data->lock);
			diag_debug(MXT_540S_DELTA_MODE);
			mutex_unlock(&mxt_fw30_data->lock);
			enable_irq(mxt_fw30_data->client->irq);
			return 0;
		}
		if (arg == 5011) 
		{
			disable_irq(mxt_fw30_data->client->irq);
			mutex_lock(&mxt_fw30_data->lock);
			diag_debug(MXT_540S_REFERENCE_MODE);
			mutex_unlock(&mxt_fw30_data->lock);
			enable_irq(mxt_fw30_data->client->irq);
			return 0;
		}
		else if (arg > 224-1)
		{
			dbg_ioctl(" ERROR! arg -> %d",(int)arg);
			return 0;
		}
		break;
		
	case RESET_TOUCH_CONFIG:
		reset_touch_config();
		break;
		/*
		   case READ_ITO_TYPE:
		   return tsp_ito_type;
		   break;
		 */
	case GET_TOUCH_CONFIG:
		object_type 	= (int)((arg & 0x0000FF00) >> 8);
		field_index 	= (int)((arg & 0x000000FF) >> 0);
		if (config_table[object_type] == 0) {
			dbg_cr("[TOUCH] Get Touch Config is Error! undefined object type! %d\n", object_type);
			break;
		}
		config = config_table[object_type][field_index];
		if (config.size == UINT8 || config.size == INT8) {
			return_value = ((int16_t)*(config.value) & 0xFF) + (config.size << 16);
		}
		else if (config.size == UINT16 || config.size == INT16) {
			return_value = ((int16_t)*(config.value) & 0xFFFF) + (config.size << 16);
		}
		else {
			// Error
		}
		dbg_ioctl("Touch IO IOCTL GET config: %d-%d: %d (%d)\n", object_type, field_index, (return_value & 0xFFFF), (return_value & 0xF0000)>>16);
		return return_value;
		break;
		
  case SET_TOUCH_CONFIG:
    data    = (int)((arg & 0xFFFF0000) >> 16);
    object_type   = (int)((arg & 0x0000FF00) >> 8);
    field_index   = (int)((arg & 0x000000FF) >> 0);
    if (config_table[object_type] == 0) {
      dbg_cr("[TOUCH] Error! undefined object type! %d\n", object_type);
    break;
    }
    config = config_table[object_type][field_index];
    if (config.size == UINT8) {
      *((uint8_t*)config_table[object_type][field_index].value) = (data & 0xFF);
    }
    else if (config.size == UINT16) {
      *((uint16_t*)config_table[object_type][field_index].value) = (data & 0xFFFF);
    }
    else if (config.size == INT8) {
      *((int8_t*)config_table[object_type][field_index].value) = (data & 0xFF);
    }
    else if (config.size == INT16) {
      *((int16_t*)config_table[object_type][field_index].value) = (data & 0xFFFF);
    }
    else {
      // Error
    }
    dbg_ioctl("Touch IC IOCTL set %d-%d with %d\n", object_type, field_index, data);
    break;

  case ATMEL_GET_REFERENCE_DATA:
  	{
  	  if (copy_to_user(argp, &reference_data, sizeof(reference_data))){
  	    dbg_cr("[TOUCH IO] ATMEL_GET_REFERENCE_DATA is failed\n");
  	  }
			break;
  	} 
  	
  case TOUCH_IOCTL_DEBUG:
		return ioctl_debug(arg);
		break;
	case TOUCH_CHARGER_MODE:
		break;

  case TOUCH_IOCTL_DO_KEY:
		input_report_key(mxt_fw30_data->input_dev, (int)arg, 1);
		break;
		
	case TOUCH_IOCTL_RELEASE_KEY:		
		input_report_key(mxt_fw30_data->input_dev, (int)arg, 0);
		break;
		
	case TOUCH_IOCTL_INIT:
		dbg_cr("[TOUCH] Touch IC init \n");
		mxt_front_test_init();
		break;
		
	case TOUCH_IOCTL_OFF:
		dbg_cr("[TOUCH] Touch IC off \n");
		TSP_PowerOff();
		break;   
	 
	case TOUCH_IOCTL_MULTI_TSP_OBJECT_SEL:
		// format: obj_sel_# (0~9)
		if ( arg < TOUCH_MODE_MAX_NUM ) {
		  if(mTouch_mode == (int)arg){
		    dbg_cr("[touch_monitor] Touch Mode selection is equal, mTouch_mode -> %d\n",mTouch_mode);
		  }else{
		    if(mxt_fw30_data->state == SUSMODE){
                    //p16619 keep this code when deleting switch device code
		      wake_lock_timeout(&touch_pen_wakelock,msecs_to_jiffies(TOUCH_MODE_CHANGE_WAKELOCK_TIMEOUT));
		      mTouch_mode = arg;
        	reset_touch_config();
        	if(pan_pmode_resume_cal){
        	  pan_touch_protection_mode();
        	}
          dbg_cr("[touch_monitor] Suspend Touch Mode selection is %d\n", mTouch_mode);
		    }else{		    
			    mTouch_mode = arg;
        	disable_irq(mxt_fw30_data->client->irq);
          reset_touch_config();
          if(pan_pmode_resume_cal){
        	  pan_touch_protection_mode();
        	}
          enable_irq(mxt_fw30_data->client->irq);
			    dbg_cr("[touch_monitor] Touch Mode selection is %d\n", mTouch_mode);
			  }
			}
		}		
//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
		else if ( arg == 10 || arg == 11 ) {
			mTouch_cover_status_ui = arg - 10;
			//++ add 2013.08.26 check hallic vs ui state.
			mTouch_cover_status_hallic = mTouch_cover_status_ui;
    // ++ p13106 for calibrate_chip when smart cover open.
      if(mTouch_cover_status_hallic == TOUCH_COVER_OPENED){
        calibrate_chip();
      }
    // -- p13106 for calibrate_chip when smart cover open.
      if ( touch_probe_state == 1 )
				del_timer_sync(&pan_hallic_ui_sync_timer);
			//--
			dbg_cr("[touch_monitor] Touch Cover Status is %d\n", mTouch_cover_status_ui);
		}
#endif
//-- p11309
    else if ( arg == 20 || arg == 21 ) {
      if(mTouch_mode ==2 || mTouch_mode ==3){
        
        if(arg == 20){
          dbg_cr("Touch Pen Idle mode\n");
          mxt_Multitouchscreen_T100_Init();
          mxt_Stylus_T47_Config_Init();
        }else if(arg == 21){
          dbg_cr("Touch Pen Text mode\n");
          touchscreen_config = obj_multi_touch_t100[mTouch_mode];
          touchscreen_config.movhysti=2;
          touchscreen_config.movhystn=1;
          if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK)
            dbg_cr("T100 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);

          stylus_t47_config = obj_stylus_t47[mTouch_mode];
          stylus_t47_config.cfg=3;
          if (write_stylus_T47_config(stylus_t47_config) != CFG_WRITE_OK)
            dbg_cr("T47 Configuration Fail!!! , Line %d \n\r", __LINE__);         
        }
      }
    }

		break;

	case TOUCH_IOCTL_SUSPEND :
	   mxt_suspend(NULL,pm_null);
	   break;

	case TOUCH_IOCTL_RESUME :
	  mxt_resume(NULL);
	  break;
	  
//++ p11309 - 2013.07.10 for Get Touch Mode
	case TOUCH_IOCTL_MULTI_TSP_OBJECT_GET:

#ifdef PAN_SUPPORT_SMART_COVER
		touch_mode_flag = mTouch_mode + mTouch_cover_status_ui*10;
#else
		touch_mode_flag = mTouch_mode;
#endif		
		if (copy_to_user(argp, &touch_mode_flag, sizeof(touch_mode_flag))) {
			dbg_ioctl(" TOUCH_IOCTL_MULTI_TSP_OBJECT_GET\n");
		}		
		break;
//-- p11309

	case TOUCH_IOCTL_WIFI_DEBUG_APP_ENABLE :
		if(sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&mxt_fw30_data->mem_access_attr.attr,S_IRWXUGO))
			printk("[TOUCH] sysfs_chmod_file is failed\n");
		return_value=sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&(*mxt_attrs[0]), S_IRWXUGO);
		break;

	case TOUCH_IOCTL_WIFI_DEBUG_APP_DISABLE :
		if(sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&mxt_fw30_data->mem_access_attr.attr,S_IRUGO | S_IWUSR))
			printk("[TOUCH] sysfs_chmod_file is failed\n");		
		return_value=sysfs_chmod_file(&mxt_fw30_data->client->dev.kobj,&(*mxt_attrs[0]), S_IWUSR | S_IRUSR);
		break;
//++ P13106 TOUCH MODE READ
  case TOUCH_IOCTL_TOUCH_MODE:   
    if (copy_to_user(argp, &mTouch_mode, sizeof(mTouch_mode))) {
      dbg_ioctl("TOUCH_IOCTL_TOUCH_MODE\n");
    }   
    break;
//-- P13106 TOUCH MODE READ
	default:
		break;
	}
	return 0;
}

static int ioctl_debug(unsigned long arg) 
{
	pm_message_t pm_null={0};
	switch (arg)
	{
	case IOCTL_DEBUG_SUSPEND:
		mxt_suspend(NULL,pm_null);
		break;
	case IOCTL_DEBUG_RESUME:
		mxt_resume(NULL);
		break;
// 	case IOCTL_DEBUG_GET_TOUCH_ANTITOUCH_INFO:
// 		check_chip_calibration();
// 		return get_touch_antitouch_info();
// 		break;
// 	case IOCTL_DEBUG_TCH_CH:
// 		return debugInfo.tch_ch;
// 		break;
// 	case IOCTL_DEBUG_ATCH_CH:
// 		return debugInfo.atch_ch;
// 		break;
// 	case IOCTL_GET_CALIBRATION_CNT:
// 		return debugInfo.calibration_cnt;
// 		break;
	default:
		break;
	}
	return 0;
}

static const struct attribute_group mxt_attr_group = {
	.attrs = mxt_attrs,
};

static ssize_t mxt_debug_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int count;
	char c;

	c = mxt_fw30_data->debug_enabled ? '1' : '0';
	count = sprintf(buf, "%c\n", c);

	return count;
}

static ssize_t mxt_debug_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int i;

	if (sscanf(buf, "%u", &i) == 1 && i < 2) {
		mxt_fw30_data->debug_enabled = (i == 1);

		dbg_cr("[TOUCH] %s %s\n",__FUNCTION__ , i ? "debug enabled" : "debug disabled");
		return count;
	} else {
		dbg_cr("[TOUCH] %s debug_enabled write error\n",__FUNCTION__);
		return -EINVAL;
	}
}


static ssize_t mxt_mem_access_read(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off, size_t count)
{
	int ret = 0;
  //printk("%s , buf -> %s, cnt -> %d\n",__FUNCTION__,buf,count);
	ret = mxt_check_mem_access_params(mxt_fw30_data, off, &count);
	if (ret < 0)
		return ret;

	if (count > 0)
		ret = __mxt_read_reg(mxt_fw30_data->client, off, count, buf);
	if(ret)
		printk("[TOUCH] __mxt_read_reg is failed\n");

	return ret == 0 ? count : ret;
}
static void mxt_dump_message(u8 *message)
{
	print_hex_dump(KERN_DEBUG, "MXT MSG:", DUMP_PREFIX_NONE, 16, 1,
			message,max_message_length, false);
}

static ssize_t mxt_mem_access_write(struct file *filp, struct kobject *kobj,
	struct bin_attribute *bin_attr, char *buf, loff_t off,size_t count)
{
	int ret = 0;
	ret = mxt_check_mem_access_params(mxt_fw30_data, off, &count);
	if (ret < 0)
		return ret;

	if (count > 0)
		ret = __mxt_write_reg(mxt_fw30_data->client, off, count, buf);
	if(ret)
		printk("[TOUCH] __mxt_write_reg is failed\n");
	return ret == 0 ? count : 0;
}

static int mxt_check_mem_access_params(struct mxt_fw30_data_t *data, loff_t off, size_t *count)
{
	if (off >= data->mem_size)
		return -EIO;

	if (off + *count > data->mem_size)
		*count = data->mem_size - off;

	if (*count > MXT_MAX_BLOCK_WRITE)
		*count = MXT_MAX_BLOCK_WRITE;

	return 0;
}

static int __mxt_read_reg(struct i2c_client *client,
			       u16 reg, u16 len, void *val)
{
	struct i2c_msg xfer[2];
	u8 buf[2];
	int ret;
	bool retry = false;

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

retry_read:
	ret = i2c_transfer(client->adapter, xfer, ARRAY_SIZE(xfer));
	if (ret != ARRAY_SIZE(xfer)) {
		if (!retry) {
			dbg_i2c(" %s: i2c retry\n",__FUNCTION__);
			msleep(MXT_WAKEUP_TIME);
			retry = true;
			goto retry_read;
		} else {
			dbg_i2c(" %s: i2c transfer failed (%d)\n",__FUNCTION__, ret);
			return -EIO;
		}
	}

	return 0;
}

static int __mxt_write_reg(struct i2c_client *client, u16 reg, u16 len,
			   const void *val)
{
	u8 *buf;
	size_t count;
	int ret;
	bool retry = false;
  
	count = len + 2;
	buf = kmalloc(count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;
	memcpy(&buf[2], val, len);

retry_write:
	ret = i2c_master_send(client, buf, count);
	if (ret == count) {
		ret = 0;
	} else {
		if (!retry) {
      dbg_i2c(" %s: i2c retry\n",__FUNCTION__);
			msleep(MXT_WAKEUP_TIME);
			retry = true;
			goto retry_write;
		} else {
		    dbg_i2c(" %s: i2c send failed (%d)\n",__FUNCTION__, ret);
			ret = -EIO;
		}
	}

	kfree(buf);
	return ret;
}

static void apply_touch_config(void)
{
	if (driver_setup != DRIVER_SETUP_OK){
	  dbg_config("%s driver setup is failed\n",__FUNCTION__);
		return;
  }
  
	if (write_power_T7_config(power_config) != CFG_WRITE_OK)	
		dbg_cr(" T7 Configuration Fail!!! , Line %d \n\r", __LINE__);
	if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK)	
		dbg_cr("T8 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_keyarray_T15_config(0, keyarray_config) != CFG_WRITE_OK)	
		dbg_cr("T15 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_comms_T18_config(0, comms_config) != CFG_WRITE_OK)
		dbg_cr("T18 Configuration Fail!!! , Line %d \n\r", __LINE__);	

	if (write_proximity_T23_config(0, proximity_config) != CFG_WRITE_OK)		
		dbg_cr("T23 Configuration Fail!!! , Line %d \n\r", __LINE__);	
	/*
	if (write_selftest_T25_config(0,selftest_config) != CFG_WRITE_OK)		
		dbg_cr("T25 Configuration Fail!!! , Line %d \n\r", __LINE__);	
	*/	
	if (write_grip_suppression_T40_config(gripsuppression_t40_config) !=	CFG_WRITE_OK)		
		dbg_cr("T40 Configuration Fail!!! , Line %d \n\r", __LINE__);	
	
	if (write_touch_suppression_T42_config(touchsuppression_t42_config) != CFG_WRITE_OK)	
		dbg_cr("T42 Configuration Fail!!! , Line %d \n\r", __LINE__);	
	
	if (write_CTE_T46_config(0,cte_t46_config) != CFG_WRITE_OK)		
		dbg_cr("T46 Configuration Fail!!! , Line %d \n\r", __LINE__);	

	if (write_stylus_T47_config(stylus_t47_config) != CFG_WRITE_OK)	
		dbg_cr("T47 Configuration Fail!!! , Line %d \n\r", __LINE__);	
	
	if (write_adaptivethreshold_T55_config(proci_adaptivethreshold_t55_config) != CFG_WRITE_OK) 
		dbg_cr("T55 Configuration Fail!!! , Line %d \n\r", __LINE__);
			
	if (write_shieldless_T56_config(proci_shieldless_t56_config) != CFG_WRITE_OK) 
		dbg_cr("T56 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_lensbending_T65_config(0, lensbending_t65_config) != CFG_WRITE_OK) 
		dbg_cr("T65 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_goldenreferences_T66_config(goldenreferences_t66_config) != CFG_WRITE_OK) 
		dbg_cr("T66 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_palmgestureprocessor_T69_config(palmgestureprocessor_t69_config) != CFG_WRITE_OK) 
		dbg_cr("T69 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_noisesuppression_T72_config(noisesuppression_t72_config) != CFG_WRITE_OK) 
		dbg_cr("T72 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_glovedetection_T78_config(glovedetection_t78_config) != CFG_WRITE_OK) 
		dbg_cr("T78 Configuration Fail!!! , Line %d \n\r", __LINE__);

	if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK) 
		dbg_cr("T100 Configuration Fail!!! , Line %d \n\r", __LINE__);
		
#ifdef FEATURE_mXT540S_V_3_0
	if (write_retransmissioncompensation_T80_config(retransmissioncompensation_t80_config) != CFG_WRITE_OK)
		dbg_cr("T80 Configuration Fail!!! , Line %d \n\r", __LINE__);

  if (write_gesturepocessor_T84_config(gesturepocessor_t84_config) != CFG_WRITE_OK)
		dbg_cr("T80 Configuration Fail!!! , Line %d \n\r", __LINE__);

  if (write_touchscreenhover_T101_config(0, touchscreenhover_t101_config) != CFG_WRITE_OK)
		dbg_cr("T101 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);

	if (write_selfcapcbcrconfig_T102_config(0, selfcapcbcrconfig_t102_config) != CFG_WRITE_OK)
		dbg_cr("T102 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);

	if (write_schnoisesuppression_T103_config(0, schnoisesuppression_t103_config) != CFG_WRITE_OK)
		dbg_cr("T103 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);

	if (write_auxtouch_T104_config(auxtouchconfig_t104_config) != CFG_WRITE_OK)
		dbg_cr("T104 Configuration Fail!!! , Line %d \n\r", __LINE__);

#endif
}
#endif //TOUCH_IO

#ifdef ITO_TYPE_CHECK
static int read_touch_id(void)
{
  
  struct qpnp_vadc_result result;
  
  int err = 0,adc,i;  
  
  result.physical = -EINVAL; 
  
  err = qpnp_vadc_read(P_MUX6_1_1, &result); // 18 is channel number. 12
  if (err < 0) { 
    dbg_cr("read touch id is failed\n");  
    return err;  
  } 
  
  adc = (int) result.physical; 
  
  for(i=0;i<number_of_elements(ito_table);i++) {
    if( adc >= ito_table[i].min && adc < ito_table[i].max){
//++ p11309 - 2013.07.25 Get Model Color 
      if ( mxt_fw30_data )
        mxt_fw30_data->ito_color = i+1;	// 0 is not read.
	  else {
        dbg_cr("not allocated memory in mxt_fw30_data.\n");  
        return -1;
	  }
//-- p11309
      break;
    }
  }

  dbg_cr(" chan=%d, adc_code=%d, measurement=%lld, physical=%lld translate voltage %d, color -> %d\n", 
	  result.chan, result.adc_code, result.measurement, result.physical, adc,mxt_fw30_data->ito_color);
  return 1; 
}
#endif

void mxt_Power_Sleep(void)
{
	dbg_func_in();

	if (driver_setup != DRIVER_SETUP_OK)
		return;

	/* Set Idle Acquisition Interval to 32 ms. */
	power_config.idleacqint = 0;

	/* Set Active Acquisition Interval to 16 ms. */
	power_config.actvacqint = 0;

	/* Set Active to Idle Timeout to 4 s (one unit = 200ms). */
	power_config.actv2idleto = 0;

	/* Write power config to chip. */
	if (write_power_T7_config(power_config) != CFG_WRITE_OK)	{
		dbg_cr("%s: T7_POWERCONFIG Configuration Fail!!!\n", __func__);
	}

	dbg_func_out();
}

void mxt_T7_Power_Config_Init(void)
{
	power_config = obj_power_config_t7[mTouch_mode];

	/* Write power config to chip. */
	if (write_power_T7_config(power_config) != CFG_WRITE_OK)
		dbg_cr(" T7 Configuration Fail!!! , Line %d \n", __LINE__);
	}

void mxt_Acquisition_Config_T8_Init(void)
{
	dbg_func_in();
	acquisition_config = obj_acquisition_config_t8[mTouch_mode];


	if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK)	
		dbg_cr(" T8 Configuration Fail!!! , Line %d \n", __LINE__);

	dbg_func_out();
}

void mxt_KeyArray_T15_Init(void)
{
	keyarray_config = obj_key_array_t15[mTouch_mode];

	if (write_keyarray_T15_config(0, keyarray_config) != CFG_WRITE_OK)
		dbg_cr(" T15 Configuration Fail!!! , Line %d \n", __LINE__);
}

void mxt_CommsConfig_T18_Init(void)
{
	comms_config = obj_comm_config_t18[mTouch_mode];

	if (get_object_address(SPT_COMCONFIG_T18, 0) != OBJECT_NOT_FOUND){
		if (write_comms_T18_config(0, comms_config) != CFG_WRITE_OK)
			dbg_cr("T18 Configuration Fail!!! , Line %d \n", __LINE__);
	}

	dbg_func_out();
}

void mxt_Gpio_Pwm_T19_Init(void)
{
  gpiopwm_config = obj_gpiopwm_config_t19[mTouch_mode];
  /*
	gpiopwm_config.ctrl       = obj_gpiopwm_config_t19[mTouch_mode].ctrl;
	gpiopwm_config.reportmask = obj_gpiopwm_config_t19[mTouch_mode].reportmask;
	gpiopwm_config.dir        = obj_gpiopwm_config_t19[mTouch_mode].dir;
	gpiopwm_config.intpullup  = obj_gpiopwm_config_t19[mTouch_mode].intpullup;
	gpiopwm_config.out        = obj_gpiopwm_config_t19[mTouch_mode].out;
	gpiopwm_config.wake       = obj_gpiopwm_config_t19[mTouch_mode].wake;
*/
	if (get_object_address(SPT_GPIOPWM_T19, 0) != OBJECT_NOT_FOUND){
		if (write_gpiopwm_T19_config(0, gpiopwm_config) != CFG_WRITE_OK)
			dbg_cr("T19 Configuration Fail!!! , Line %d \n", __LINE__);
	}
}

void mxt_Proximity_Config_T23_Init(void)
{
	proximity_config = obj_proximity_sensor_t23[mTouch_mode];

	if (get_object_address(TOUCH_PROXIMITY_T23, 0) != OBJECT_NOT_FOUND)	{
		if (write_proximity_T23_config(0, proximity_config) != CFG_WRITE_OK)
			dbg_cr("T23 Configuration Fail!!! , Line %d \n", __LINE__);
	}
}

void mxt_T25_Selftest_Init(void)
{
	selftest_config.ctrl = obj_self_test_t25[mTouch_mode].ctrl;
	selftest_config.cmd  = obj_self_test_t25[mTouch_mode].cmd ;

#if(NUM_OF_TOUCH_OBJECTS)
	selftest_config.siglim[0].upsiglim = obj_self_test_t25[mTouch_mode].siglim[0].upsiglim;
	selftest_config.siglim[0].losiglim = obj_self_test_t25[mTouch_mode].siglim[0].losiglim;
	selftest_config.siglim[1].upsiglim = obj_self_test_t25[mTouch_mode].siglim[1].upsiglim;
	selftest_config.siglim[1].losiglim = obj_self_test_t25[mTouch_mode].siglim[1].losiglim;
	selftest_config.siglim[2].upsiglim = obj_self_test_t25[mTouch_mode].siglim[2].upsiglim;
	selftest_config.siglim[2].losiglim = obj_self_test_t25[mTouch_mode].siglim[2].losiglim;
#endif 

	selftest_config.pindwellus = obj_self_test_t25[mTouch_mode].pindwellus;

#if(NUM_OF_TOUCH_OBJECTS)
	selftest_config.sigrangelim[0] = obj_self_test_t25[mTouch_mode].sigrangelim[0];
	selftest_config.sigrangelim[1] = obj_self_test_t25[mTouch_mode].sigrangelim[1];
	selftest_config.sigrangelim[2] = obj_self_test_t25[mTouch_mode].sigrangelim[2];
#endif

	if (get_object_address(SPT_SELFTEST_T25, 0) != OBJECT_NOT_FOUND) {
		if (write_selftest_T25_config(0,selftest_config) != CFG_WRITE_OK)
			dbg_cr("T25 Configuration Fail!!! , Line %d \n", __LINE__);
	}

	dbg_func_out();
}

void mxt_Grip_Suppression_T40_Config_Init(void)
{
	gripsuppression_t40_config = obj_grip_suppression_t40[mTouch_mode];

	if (get_object_address(PROCI_GRIPSUPPRESSION_T40, 0) != OBJECT_NOT_FOUND) {
		if (write_grip_suppression_T40_config(gripsuppression_t40_config) != CFG_WRITE_OK)
			dbg_cr("T40 Configuration Fail!!! , Line %d \n", __LINE__);

	}
}

void mxt_Touch_Suppression_T42_Config_Init(void)
{
	touchsuppression_t42_config = obj_touch_suppression_t42[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_TOUCHSUPPRESSION_T42, 0) != OBJECT_NOT_FOUND){
		if (write_touch_suppression_T42_config(touchsuppression_t42_config) != CFG_WRITE_OK)
			dbg_cr("T42 Configuration Fail!!! , Line %d \n", __LINE__);		
	}
}

void mxt_CTE_T46_Config_Init(void)
{
	cte_t46_config = obj_cte_config_t46[mTouch_mode];

	if (get_object_address(SPT_CTECONFIG_T46, 0) != OBJECT_NOT_FOUND){
		if (write_CTE_T46_config(0,cte_t46_config) != CFG_WRITE_OK)
			dbg_cr("T46 Configuration Fail!!! , Line %d \n", __LINE__);
	}
}

void mxt_Stylus_T47_Config_Init(void)
{
	stylus_t47_config = obj_stylus_t47[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_STYLUS_T47, 0) != OBJECT_NOT_FOUND) {
		if (write_stylus_T47_config(stylus_t47_config) != CFG_WRITE_OK)
			dbg_cr("T47 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Adaptive_Threshold_T55_Config_Init(void)
{
	proci_adaptivethreshold_t55_config = obj_adaptive_threshold_t55[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_ADAPTIVETHRESHOLD_T55, 0) != OBJECT_NOT_FOUND) {
		if (write_adaptivethreshold_T55_config(proci_adaptivethreshold_t55_config) != CFG_WRITE_OK)
			dbg_cr("T55 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}
void mxt_Shieldless_T56_Config_Init(void)
{
	proci_shieldless_t56_config = obj_slim_sensor_t56[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_SHIELDLESS_T56, 0) != OBJECT_NOT_FOUND) {
		if (write_shieldless_T56_config(proci_shieldless_t56_config) != CFG_WRITE_OK)
			dbg_cr("T56 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Timer_T61_Config_Init(void)
{
	spt_timer_t61_config = obj_timer_t61[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(SPT_TIMER_T61, 0) != OBJECT_NOT_FOUND) {
		if (write_timer_T61_config(spt_timer_t61_config) != CFG_WRITE_OK)
			dbg_cr("T61 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Lensbending_T65_Config_Init(void)
{
	lensbending_t65_config = obj_lens_bending_t65[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_LENSBENDING_T65, 0) != OBJECT_NOT_FOUND) {
		if (write_lensbending_T65_config(0,lensbending_t65_config) != CFG_WRITE_OK)
			dbg_cr("T65 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Goldenreferences_T66_Config_Init(void)
{
	goldenreferences_t66_config = obj_mxt_startup_t66[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(SPT_GOLDENREFERENCES_T66, 0) != OBJECT_NOT_FOUND) {
		if (write_goldenreferences_T66_config(goldenreferences_t66_config) != CFG_WRITE_OK)
			dbg_cr("T66 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Palmgestureprocessor_T69_Config_Init(void)
{
	palmgestureprocessor_t69_config = obj_palm_gesture_processor_t69[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_PALMGESTUREPROCESSOR_T69, 0) != OBJECT_NOT_FOUND) {
		if (write_palmgestureprocessor_T69_config(palmgestureprocessor_t69_config) != CFG_WRITE_OK)
			dbg_cr("T69 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

//++ p11309 - 2013.07.19 for T70 dynamic config
void mxt_Dynamicconfigurationcontroller_T70_Config_Init(int instance)
{
	uint16_t object_address = 0;

	dynamicconfigurationcontroller_t70_config = obj_dynamic_config_controller_t70[mTouch_mode][instance];

//	dbg_cr(" --> dynamicconfigurationcontroller_t70_config debug...%d\n", instance);
// 	dbg_cr("ctrl = %d\n", dynamicconfigurationcontroller_t70_config.ctrl);
// 	dbg_cr("event = %d\n", dynamicconfigurationcontroller_t70_config.event);
// 	dbg_cr("objtype = %d\n", dynamicconfigurationcontroller_t70_config.objtype);
// 	dbg_cr("reserved = %d\n", dynamicconfigurationcontroller_t70_config.reserved);
// 	dbg_cr("objinst = %d\n", dynamicconfigurationcontroller_t70_config.objinst);
// 	dbg_cr("dstoffset = %d\n", dynamicconfigurationcontroller_t70_config.dstoffset);
// 	dbg_cr("srcoffset = %d\n", dynamicconfigurationcontroller_t70_config.srcoffset);
// 	dbg_cr("length = %d\n", dynamicconfigurationcontroller_t70_config.length);

	/* Write grip suppression config to chip. */
	object_address = get_object_address(SPT_DYNAMICCONFIGURATIONCONTROLLER_T70, instance);
	if ( object_address != OBJECT_NOT_FOUND) {
		if (write_dynamicconfigurationcontroller_t70_config(instance, dynamicconfigurationcontroller_t70_config) != CFG_WRITE_OK)
			dbg_cr("T70 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
	else {
		dbg_cr("T70 Configuration not find object address...Fail!!! , Line %d\n\r", __LINE__);
}
}

void mxt_Dynamicconfigurationcontainer_T71_Config_Init(void)
{
	memset((void *)&dynamicconfigurationcontainer_t71_config,0,sizeof(spt_dynamicconfigurationcontainer_t71_config_t));

	dynamicconfigurationcontainer_t71_config.data[0] = obj_dynamic_config_container_t71[mTouch_mode].data[0];
	dynamicconfigurationcontainer_t71_config.data[1] = obj_dynamic_config_container_t71[mTouch_mode].data[1];
	dynamicconfigurationcontainer_t71_config.data[2] = obj_dynamic_config_container_t71[mTouch_mode].data[2];
	dynamicconfigurationcontainer_t71_config.data[3] = obj_dynamic_config_container_t71[mTouch_mode].data[3];
	dynamicconfigurationcontainer_t71_config.data[4] = obj_dynamic_config_container_t71[mTouch_mode].data[4];
	dynamicconfigurationcontainer_t71_config.data[5] = obj_dynamic_config_container_t71[mTouch_mode].data[5];
	dynamicconfigurationcontainer_t71_config.data[6] = obj_dynamic_config_container_t71[mTouch_mode].data[6];
	dynamicconfigurationcontainer_t71_config.data[7] = obj_dynamic_config_container_t71[mTouch_mode].data[7];
	dynamicconfigurationcontainer_t71_config.data[8] = obj_dynamic_config_container_t71[mTouch_mode].data[8];
	dynamicconfigurationcontainer_t71_config.data[9] = obj_dynamic_config_container_t71[mTouch_mode].data[9];
	dynamicconfigurationcontainer_t71_config.data[10] = obj_dynamic_config_container_t71[mTouch_mode].data[10];
	dynamicconfigurationcontainer_t71_config.data[11] = obj_dynamic_config_container_t71[mTouch_mode].data[11];
	
	if (get_object_address(SPT_DYNAMICCONFIGURATIONCONTAINER_T71, 0) != OBJECT_NOT_FOUND) {
		if (write_dynamicconfigurationcontainer_t71_config(dynamicconfigurationcontainer_t71_config) != CFG_WRITE_OK)
			dbg_cr("T71 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

//-- p11309

void mxt_Noisesuppression_T72_Config_Init(void)
{
	noisesuppression_t72_config = obj_mxt_charger_t72[mTouch_mode];

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCG_NOISESUPPRESSION_T72, 0) != OBJECT_NOT_FOUND) {
		if (write_noisesuppression_T72_config(noisesuppression_t72_config) != CFG_WRITE_OK)
			dbg_cr("T72 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Glovedetection_T78_Config_Init(void)
{
	glovedetection_t78_config = obj_glove_detect_t78[mTouch_mode];

	if (get_object_address(PROCI_GLOVEDETECTION_T78, 0) != OBJECT_NOT_FOUND) {
		if (write_glovedetection_T78_config(glovedetection_t78_config) != CFG_WRITE_OK)
			dbg_cr("T78 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

#ifdef FEATURE_mXT540S_V_3_0
void mxt_Retransmissioncompensation_T80_Config_Init(void)
{
	retransmissioncompensation_t80_config = obj_retransmissioncompensation_t80[mTouch_mode];

	if (get_object_address(PROCI_RETRANSMISSIONCOMPENSATION_T80, 0) != OBJECT_NOT_FOUND) {
		if (write_retransmissioncompensation_T80_config(retransmissioncompensation_t80_config) != CFG_WRITE_OK)
			dbg_cr("T80 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Gestureprocess_T84_Config_Init(void)
{
	gesturepocessor_t84_config = obj_gesturepocessor_t84[mTouch_mode];

	if (get_object_address(PROCI_GESTUREPROCESSOR_T84, 0) != OBJECT_NOT_FOUND) {
		if (write_gesturepocessor_T84_config(gesturepocessor_t84_config) != CFG_WRITE_OK)
			dbg_cr("T84 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}


#endif

void mxt_Multitouchscreen_T100_Init(void)
{	
	touchscreen_config = obj_multi_touch_t100[mTouch_mode];

	if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK)
		dbg_cr("T100 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);
}

#ifdef FEATURE_mXT540S_V_3_0

void mxt_Touchscreenhover_T101_config_Init(void) {
	touchscreenhover_t101_config = obj_touchscreenhover_t101[mTouch_mode];

	if (get_object_address(SPT_TOUCHSCREENHOVER_T101, 0) != OBJECT_NOT_FOUND) {
		if (write_touchscreenhover_T101_config(0, touchscreenhover_t101_config) != CFG_WRITE_OK)
			dbg_cr("T101 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);
	}
}

void mxt_Selfcapcbcrconfig_T102_config_Init(void) {
	selfcapcbcrconfig_t102_config = obj_selfcapcbcrconfig_t102[mTouch_mode];

	if (get_object_address(SPT_SELFCAPCBCRCONFIG_T102, 0) != OBJECT_NOT_FOUND) {
		if (write_selfcapcbcrconfig_T102_config(0, selfcapcbcrconfig_t102_config) != CFG_WRITE_OK)
			dbg_cr("T102 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);
	}
}

void mxt_Schnoisesuppression_T103_config_Init(void) {
	schnoisesuppression_t103_config = obj_schnoisesuppression_t103[mTouch_mode];

	if (get_object_address(PROCI_SCHNOISESUPPRESSION_T103, 0) != OBJECT_NOT_FOUND) {
		if (write_schnoisesuppression_T103_config(0, schnoisesuppression_t103_config) != CFG_WRITE_OK)
			dbg_cr("T103 Configuration Fail!!!, %s, Line %d \n", __func__, __LINE__);
	}
}

void mxt_Auxtouch_T104_config_Init(void) {
	auxtouchconfig_t104_config = obj_auxtouchconfig_t104[mTouch_mode];

	if (get_object_address(SPT_AUXTOUCHCONFIG_T104, 0) != OBJECT_NOT_FOUND) {
		if (write_auxtouch_T104_config(auxtouchconfig_t104_config) != CFG_WRITE_OK)
			dbg_cr("T104 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}

void mxt_Drivenplatehoverconfig_T105_config_Init(void) {
	drivenplatehoverconfig_t105_config = obj_drivenplatehoverconfig_t105[mTouch_mode];

	if (get_object_address(SPT_DRIVENPLATEHOVERCONFIG_T105, 0) != OBJECT_NOT_FOUND) {
		if (write_drivenplatehover_T105_config(drivenplatehoverconfig_t105_config) != CFG_WRITE_OK)
			dbg_cr("T105 Configuration Fail!!! , Line %d \n\r", __LINE__);
	}
}


#endif

static void reset_touch_config(void)
{
	mxt_T7_Power_Config_Init();
	mxt_Acquisition_Config_T8_Init();
	mxt_Multitouchscreen_T100_Init();
	mxt_KeyArray_T15_Init();
	mxt_CommsConfig_T18_Init();
	mxt_Gpio_Pwm_T19_Init();
	mxt_Proximity_Config_T23_Init();
	//mxt_Selftest_T25_Init();
	mxt_Grip_Suppression_T40_Config_Init();
	mxt_Touch_Suppression_T42_Config_Init();
	mxt_CTE_T46_Config_Init();
	mxt_Stylus_T47_Config_Init();	
	mxt_Adaptive_Threshold_T55_Config_Init();
	mxt_Shieldless_T56_Config_Init();
	mxt_Lensbending_T65_Config_Init();
	mxt_Goldenreferences_T66_Config_Init();
	mxt_Palmgestureprocessor_T69_Config_Init();

//++ p11309 - 2013.07.19 for T70 dynamic config
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(0);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(1);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(2);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(3);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(4);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(5);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(6);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(7);

	mxt_Dynamicconfigurationcontainer_T71_Config_Init();
//-- p11309	
	mxt_Noisesuppression_T72_Config_Init();
	mxt_Glovedetection_T78_Config_Init();

	mxt_Retransmissioncompensation_T80_Config_Init();
	mxt_Gestureprocess_T84_Config_Init();
	mxt_Touchscreenhover_T101_config_Init();
	mxt_Selfcapcbcrconfig_T102_config_Init();
	mxt_Schnoisesuppression_T103_config_Init();
	mxt_Auxtouch_T104_config_Init();
	mxt_Drivenplatehoverconfig_T105_config_Init();
	if(mxt_fw30_data->state == SUSMODE){
    mxt_Power_Sleep();
  }
}

uint8_t reset_chip(void)
{
	uint8_t data = 1u;
	uint8_t rc;

	dbg_func_in();

	if (driver_setup != DRIVER_SETUP_OK)
		return WRITE_MEM_FAILED;

	rc = write_mem(command_processor_address + RESET_OFFSET, 1, &data);

	dbg_func_out();

	return rc;
}

/*****************************************************************************
 *
 *  FUNCTION
 *  PURPOSE
 * \brief Calibrates the chip.
 * 
 * This function will send a calibrate command to touch chip.
 * Whilst calibration has not been confirmed as good, this function will set
 * the ATCHCALST and ATCHCALSTHR to zero to allow a bad cal to always recover
 * 
 * @return WRITE_MEM_OK if writing the command to touch chip was successful.
 * 
 *  INPUT
 *  OUTPUT
 *
 * ***************************************************************************/

uint8_t calibrate_chip(void)
{
	uint8_t data = 1u;
	int ret = WRITE_MEM_OK;

	dbg_func_in();
	dbg_cr(" %s\n",__FUNCTION__);
	if (driver_setup != DRIVER_SETUP_OK)
		return WRITE_MEM_FAILED;

	not_yet_count = 0;
	mxt_time_point = 0;
	good_calibration_cnt = 0;

	/* resume calibration must be performed with zero settings */
	acquisition_config.atchcalst       = obj_acquisition_config_t8[mTouch_mode].atchcalst      ;
	acquisition_config.atchcalsthr     = obj_acquisition_config_t8[mTouch_mode].atchcalsthr    ;
	acquisition_config.atchfrccalthr   = obj_acquisition_config_t8[mTouch_mode].atchfrccalthr  ;
	acquisition_config.atchfrccalratio = obj_acquisition_config_t8[mTouch_mode].atchfrccalratio;

	dbg_op("[TSP] reset acq atchcalst=%d, atchcalsthr=%d\n", acquisition_config.atchcalst, acquisition_config.atchcalsthr );

	/* Write temporary acquisition config to chip. */
	if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK)	{
		/* "Acquisition config write failed!\n" */
		dbg_cr("T8 config write is failed line : %d\n",__LINE__);
		ret = WRITE_MEM_FAILED; /* calling function should retry calibration call */
	}

	/* send calibration command to the chip */
	if(ret == WRITE_MEM_OK)	{
		/* change calibration suspend settings to zero until calibration confirmed good */
		ret = write_mem(command_processor_address + CALIBRATE_OFFSET, 1, &data);

		/* set flag for calibration lockup recovery if cal command was successful */
		if(ret == WRITE_MEM_OK){ 
			/* set flag to show we must still confirm if calibration was good or bad */
			cal_check_flag = 1u;
		}
		else{
			dbg_cr(" Touch Calibration is failed line : %d\n",__LINE__);
		}
	}

//	msleep(120);  // p11309 - 2013.07.26

	dbg_func_out();
	return ret;
}

uint8_t diagnostic_chip(uint8_t mode)
{
	uint8_t status;
	dbg_func_in();

	if (driver_setup != DRIVER_SETUP_OK)
		return WRITE_MEM_FAILED;

	status = write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &mode);

	dbg_func_out();
	return(status);
}

uint8_t backup_config(void)
{
	/* Write 0x55 to BACKUPNV register to initiate the backup. */
	uint8_t data = 0x55u;
	uint8_t rc;

	dbg_func_in();

	if (driver_setup != DRIVER_SETUP_OK)
		return WRITE_MEM_FAILED;

	rc = write_mem(command_processor_address + BACKUP_OFFSET, 1, &data);

	dbg_func_out();

	return rc;
}

uint8_t write_power_T7_config(gen_powerconfig_t7_config_t cfg)
{
	return write_simple_config(GEN_POWERCONFIG_T7, 0, (void *) &cfg);
}

uint8_t write_acquisition_T8_config(gen_acquisitionconfig_t8_config_t cfg)
{
	return write_simple_config(GEN_ACQUISITIONCONFIG_T8, 0, (void *) &cfg);	
}

uint8_t write_keyarray_T15_config(uint8_t instance, touch_keyarray_t15_config_t cfg)
{
	return write_simple_config(TOUCH_KEYARRAY_T15, instance, (void *) &cfg);
}

uint8_t write_comms_T18_config(uint8_t instance, spt_commsconfig_t18_config_t cfg)
{
	return write_simple_config(SPT_COMCONFIG_T18, instance, (void *) &cfg);
}

uint8_t write_gpiopwm_T19_config(uint8_t instance, spt_gpiopwm_t19_config_t cfg)
{
	return write_simple_config(SPT_GPIOPWM_T19, instance, (void *) &cfg);
}

uint8_t write_proximity_T23_config(uint8_t instance, touch_proximity_t23_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(TOUCH_PROXIMITY_T23);
	if (object_size == 0)
		return(CFG_WRITE_FAILED);

	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
	if (tmp == NULL)
		return(CFG_WRITE_FAILED);

	memset(tmp,0,object_size);

	*(tmp + 0) = cfg.ctrl;
	*(tmp + 1) = cfg.xorigin;
	*(tmp + 2) = cfg.yorigin;
	*(tmp + 3) = cfg.xsize;
	*(tmp + 4) = cfg.ysize;
	*(tmp + 5) = cfg.reserved_0;
	*(tmp + 6) = cfg.blen;
	*(tmp + 7) = (uint8_t) (cfg.fxddthr & 0x00FF);
	*(tmp + 8) = (uint8_t) (cfg.fxddthr >> 8);
	*(tmp + 9) = cfg.fxddi;
	*(tmp + 10) = cfg.average;
	*(tmp + 11) = (uint8_t) (cfg.mvnullrate & 0x00FF);
	*(tmp + 12) = (uint8_t) (cfg.mvnullrate >> 8);
	*(tmp + 13) = (uint8_t) (cfg.mvdthr & 0x00FF);
	*(tmp + 14) = (uint8_t) (cfg.mvdthr >> 8);
	*(tmp + 15) = cfg.cfg;

	object_address = get_object_address(TOUCH_PROXIMITY_T23,instance);

	if (object_address == 0)
		return(CFG_WRITE_FAILED);

	status = write_mem(object_address, object_size, tmp);
  dbg_op(" write_proximity_T23_config object_type -> %d , object_Address -> %d, object_size -> %d\n",TOUCH_PROXIMITY_T23,object_address,object_size);

	kfree(tmp);
	return(status);
}

uint8_t write_selftest_T25_config(uint8_t instance, spt_selftest_t25_config_t cfg)
{

	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(SPT_SELFTEST_T25);
	if (object_size == 0)
		return(CFG_WRITE_FAILED);

	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

	if (tmp == NULL)
		return(CFG_WRITE_FAILED);

	memset(tmp,0,object_size);

	*(tmp + 0) = cfg.ctrl;
	*(tmp + 1) = cfg.cmd;
	object_address = get_object_address(SPT_SELFTEST_T25,instance);

	if (object_address == 0)
		return(CFG_WRITE_FAILED);

	status = write_mem(object_address, object_size, tmp);
  dbg_op(" write_selftest_T25_config object_type -> %d , object_Address -> %d, object_size -> %d\n",SPT_SELFTEST_T25,object_address,object_size);

	kfree(tmp);
	return(status);
}

uint8_t write_grip_suppression_T40_config(proci_gripsuppression_t40_config_t cfg)
{
	return(write_simple_config(PROCI_GRIPSUPPRESSION_T40, 0, (void *) &cfg));
}

uint8_t write_touch_suppression_T42_config(proci_touchsuppression_t42_config_t cfg)
{
	return(write_simple_config(PROCI_TOUCHSUPPRESSION_T42, 0, (void *) &cfg));
}

uint8_t write_CTE_T46_config(uint8_t instance, spt_cteconfig_t46_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(SPT_CTECONFIG_T46);
	if (object_size == 0)
		return(CFG_WRITE_FAILED);

	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

	if (tmp == NULL)
		return(CFG_WRITE_FAILED);

	memset(tmp,0,object_size);

	memcpy(tmp, &cfg, 7);
	*(tmp + 7) = (uint8_t) (cfg.syncdelay& 0x00FF);
	*(tmp + 8) = (uint8_t) (cfg.syncdelay >> 8);
	*(tmp + 9) = cfg.xvoltage;

	object_address = get_object_address(SPT_CTECONFIG_T46,instance);

	if (object_address == 0)
		return(CFG_WRITE_FAILED);

	status = write_mem(object_address, object_size, tmp);
	dbg_op(" write_CTE_T46_config object_type -> %d , object_Address -> %d, object_size -> %d\n",SPT_CTECONFIG_T46,object_address,object_size);
  
	kfree(tmp);
	return(status);

}

uint8_t  write_stylus_T47_config(proci_stylus_t47_config_t cfg){
	return(write_simple_config(PROCI_STYLUS_T47, 0, (void *) &cfg));
}

uint8_t  write_adaptivethreshold_T55_config(proci_adaptivethreshold_t55_config_t cfg){
	return(write_simple_config(PROCI_ADAPTIVETHRESHOLD_T55, 0, (void *) &cfg));
}

uint8_t  write_shieldless_T56_config(proci_shieldless_t56_config_t cfg){
	return(write_simple_config(PROCI_SHIELDLESS_T56, 0, (void *) &cfg));
}

uint8_t write_timer_T61_config(spt_timer_t61_config_t cfg){
	return(write_simple_config(SPT_TIMER_T61, 0, (void *) &cfg));
}

uint8_t write_lensbending_T65_config(uint8_t instance, proci_lensbending_t65_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(PROCI_LENSBENDING_T65);
	if (object_size == 0)
		return(CFG_WRITE_FAILED);
	
	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
	
	if (tmp == NULL)
		return(CFG_WRITE_FAILED);
	

	memset(tmp,0,object_size);

	*(tmp + 0) = cfg.ctrl;
	*(tmp + 1) = cfg.gradthr;
	*(tmp + 2) = (uint8_t) (cfg.ylonoisemul & 0x00FF);
	*(tmp + 3) = (uint8_t) (cfg.ylonoisemul >> 8);
	*(tmp + 4) = (uint8_t) (cfg.ylonoisediv & 0x00FF);
	*(tmp + 5) = (uint8_t) (cfg.ylonoisediv >> 8);
	*(tmp + 6) = (uint8_t) (cfg.yhinoisemul & 0x00FF);
	*(tmp + 7) = (uint8_t) (cfg.yhinoisemul >> 8);
	*(tmp + 8) = (uint8_t) (cfg.yhinoisediv & 0x00FF);
	*(tmp + 9) = (uint8_t) (cfg.yhinoisediv >> 8);
	*(tmp + 10) = cfg.lpfiltcoef;
	*(tmp + 11) = (uint8_t) (cfg.forcescale & 0x00FF);
	*(tmp + 12) = (uint8_t) (cfg.forcescale >> 8);
	*(tmp + 13) = cfg.forcethr;
	*(tmp + 14) = cfg.forcethrhyst;
	*(tmp + 15) = cfg.forcedi;
	*(tmp + 16) = cfg.forcehyst;
	*(tmp + 17) = cfg.atchratio;
	*(tmp + 18) = cfg.reserved_0;
	*(tmp + 19) = cfg.reserved_1;


	object_address = get_object_address(PROCI_LENSBENDING_T65, instance);

	if (object_address == 0)
		return(CFG_WRITE_FAILED);
	
	status = write_mem(object_address, object_size, tmp);
	dbg_op(" write_lensbending_T65_config object_type -> %d , object_Address -> %d, object_size -> %d\n",PROCI_LENSBENDING_T65,object_address,object_size);
  
	kfree(tmp);
	return(status);
}

uint8_t  write_goldenreferences_T66_config(spt_goldenreferences_t66_config_t cfg)
{
	return(write_simple_config(SPT_GOLDENREFERENCES_T66, 0, (void *) &cfg));
}

uint8_t  write_palmgestureprocessor_T69_config(proci_palmgestureprocessor_t69_config_t cfg)
{
	return(write_simple_config(PROCI_PALMGESTUREPROCESSOR_T69, 0, (void *) &cfg));
}

uint8_t  write_dynamicconfigurationcontroller_t70_config(uint8_t instance, spt_dynamicconfigurationcontroller_t70_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(SPT_DYNAMICCONFIGURATIONCONTROLLER_T70);
	if (object_size == 0){
		dbg_cr("Write T70 Error: Not assigned object size.\n");
		return(CFG_WRITE_FAILED);
	}
	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
	if (tmp == NULL){
		dbg_cr("Write T70 Error: Not assigned object mem. size = %d\n", object_size);
		return(CFG_WRITE_FAILED);
	}

	memset(tmp,0,object_size);

	*(tmp + 0) = cfg.ctrl;
	*(tmp + 1) = (uint8_t) (cfg.event & 0x00FF);
	*(tmp + 2) = (uint8_t) (cfg.event >> 8);
	*(tmp + 3) = cfg.objtype;
	*(tmp + 4) = cfg.reserved_0;
	*(tmp + 5) = cfg.objinst;
	*(tmp + 6) = cfg.dstoffset;
	*(tmp + 7) = (uint8_t) (cfg.srcoffset & 0xFF);
	*(tmp + 8) = (uint8_t) (cfg.srcoffset >> 8);
	*(tmp + 9) = cfg.length;

	object_address = get_object_address(SPT_DYNAMICCONFIGURATIONCONTROLLER_T70, instance);

	if (object_address == 0)
	{
		dbg_cr("Write T70 Error: Not assigned object address.\n");
		return(CFG_WRITE_FAILED);
	}
  
	status = write_mem(object_address, object_size, tmp);
	dbg_op(" write_dynamicconfigurationcontroller_t70_config object_type -> %d , object_Address -> %d, object_size -> %d\n",SPT_DYNAMICCONFIGURATIONCONTROLLER_T70,object_address,object_size);
  
	kfree(tmp);	
	return(status);
}

uint8_t  write_dynamicconfigurationcontainer_t71_config(spt_dynamicconfigurationcontainer_t71_config_t cfg)
{
	return(write_simple_config(SPT_DYNAMICCONFIGURATIONCONTAINER_T71, 0, (void *) &cfg));
}

uint8_t  write_noisesuppression_T72_config(procg_noisesuppression_t72_config_t cfg)
{
	return(write_simple_config(PROCG_NOISESUPPRESSION_T72, 0, (void *) &cfg));
}

uint8_t  write_glovedetection_T78_config(proci_glovedetection_t78_config_t cfg)
{
	return(write_simple_config(PROCI_GLOVEDETECTION_T78, 0, (void *) &cfg));
}

uint8_t  write_retransmissioncompensation_T80_config(proci_retransmissioncompensation_t80_config_t cfg)
{
	return(write_simple_config(PROCI_RETRANSMISSIONCOMPENSATION_T80, 0, (void *) &cfg));
}

uint8_t  write_gesturepocessor_T84_config(proci_gesturepocessor_t84_config_t cfg){
   return(write_simple_config(PROCI_GESTUREPROCESSOR_T84, 0, (void *) &cfg));
}


uint8_t write_multitouchscreen_T100_config(uint8_t instance, touch_multitouchscreen_t100_config_t cfg) {
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(TOUCH_MULTITOUCHSCREEN_T100);
	if (object_size == 0)
	{
		return(CFG_WRITE_FAILED);
	}
	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
	if (tmp == NULL)
	{
		return(CFG_WRITE_FAILED);
	}

	memset(tmp,0,object_size);

	/* 18 elements at beginning are 1 byte. */
	memcpy(tmp, &cfg, 13);
	/* Next two are 2 bytes. */
	*(tmp + 13) = (uint8_t) (cfg.xrange &  0xFF);
	*(tmp + 14) = (uint8_t) (cfg.xrange >> 8);
	*(tmp + 15) = cfg.xedgecfg;
	*(tmp + 16) = cfg.xedgedist;
	*(tmp + 17) = cfg.dxxedgecfg;
	*(tmp + 18) = cfg.dxxedgedist;
	*(tmp + 19) = cfg.yorigin;
	*(tmp + 20) = cfg.ysize;
	*(tmp + 21) = cfg.ypitch;
	*(tmp + 22) = cfg.yloclip;
	*(tmp + 23) = cfg.yhiclip;	
	*(tmp + 24) = (uint8_t) (cfg.yrange &  0xFF);
	*(tmp + 25) = (uint8_t) (cfg.yrange >> 8);

	memcpy((tmp+26), &(cfg.yedgecfg), 21);
	*(tmp + 47) = (uint8_t) (cfg.movhysti&  0xFF);
	*(tmp + 48) = (uint8_t) (cfg.movhysti >> 8);
	*(tmp + 49) = (uint8_t) (cfg.movhystn &  0xFF);
	*(tmp + 50) = (uint8_t) (cfg.movhystn >> 8);

	*(tmp + 51) = cfg.amplhyst;
	*(tmp + 52) = cfg.scrareahyst;
	*(tmp + 53) = cfg.intthrhyst;	

	object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T100,instance);

	if (object_address == 0)
		return(CFG_WRITE_FAILED);

	status = write_mem(object_address, object_size, tmp);
  dbg_op(" write_multitouchscreen_T100_config object_type -> %d , object_Address -> %d, object_size -> %d\n",TOUCH_MULTITOUCHSCREEN_T100,object_address,object_size);
    
	kfree(tmp);
	return(status);

}

uint8_t write_touchscreenhover_T101_config(uint8_t instance, spt_touchscreenhover_t101_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(SPT_TOUCHSCREENHOVER_T101);

	if (object_size == 0)
		return(CFG_WRITE_FAILED);
	
	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

	if (tmp == NULL)
		return(CFG_WRITE_FAILED);	

	memset(tmp,0,object_size);
	memcpy(tmp, &cfg, 20);

	*(tmp + 20) = (uint8_t) (cfg.movhysti &  0xFF);
	*(tmp + 21) = (uint8_t) (cfg.movhysti >> 8);
	*(tmp + 22) = (uint8_t) (cfg.movhystn &  0xFF);
	*(tmp + 23) = (uint8_t) (cfg.movhystn >> 8);
  *(tmp + 24) = cfg.hvraux;

	object_address = get_object_address(SPT_TOUCHSCREENHOVER_T101, instance);

	//TODO
	if (object_address == 0)
		return(CFG_WRITE_FAILED);	

	status = write_mem(object_address, object_size, tmp);

  dbg_op(" write_touchscreenhover_T101_config object_type -> %d , object_Address -> %d, object_size -> %d\n",SPT_TOUCHSCREENHOVER_T101,object_address,object_size);
	kfree(tmp);
	return(status);

}

uint8_t  write_selfcapcbcrconfig_T102_config(uint8_t instance, spt_selfcapcbcrconfig_t102_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(SPT_SELFCAPCBCRCONFIG_T102);

	if (object_size == 0)
		return(CFG_WRITE_FAILED);

	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

	if (tmp == NULL)
		return(CFG_WRITE_FAILED);
	
	memset(tmp,0,object_size);
	memcpy(tmp, &cfg, 3);

	*(tmp + 3) = (uint8_t) (cfg.tunthr &  0xFF);
	*(tmp + 4) = (uint8_t) (cfg.tunthr >> 8);
	*(tmp + 5) = (uint8_t) (cfg.tunhyst &  0xFF);
	*(tmp + 6) = (uint8_t) (cfg.tunhyst >> 8);

	memcpy((tmp+7), &cfg.reserved_0, 22);

	object_address = get_object_address(SPT_SELFCAPCBCRCONFIG_T102, instance);

	//TODO
	if (object_address == 0)
		return(CFG_WRITE_FAILED);
	
	status = write_mem(object_address, object_size, tmp);
	
  dbg_op(" write_selfcapcbcrconfig_T102_config object_type -> %d , object_Address -> %d, object_size -> %d\n",SPT_SELFCAPCBCRCONFIG_T102,object_address,object_size);
 
	kfree(tmp);
	return(status);

}

uint8_t  write_schnoisesuppression_T103_config(uint8_t instance, proci_schnoisesuppression_t103_config_t cfg)
{
	uint16_t object_address;
	uint8_t *tmp;
	uint8_t status;
	uint8_t object_size;

	object_size = get_object_size(PROCI_SCHNOISESUPPRESSION_T103);

	if (object_size == 0)
		return(CFG_WRITE_FAILED);

	tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

	if (tmp == NULL)
		return(CFG_WRITE_FAILED);
	
	memset(tmp,0,object_size);
	memcpy(tmp, &cfg, 2);

	*(tmp + 2) = cfg.reserved_0;
	*(tmp + 3) = cfg.procst;
	*(tmp + 4) = (uint8_t) (cfg.schsiglothr &  0xFF);
	*(tmp + 5) = (uint8_t) (cfg.schsiglothr >> 8);
	*(tmp + 6) = (uint8_t) (cfg.schsighithr &  0xFF);
	*(tmp + 7) = (uint8_t) (cfg.schsighithr >> 8);

	*(tmp + 8) = cfg.nlxgain;
	*(tmp + 9) = cfg.nlygain;
	*(tmp + 10) = (uint8_t) (cfg.nllothr &  0xFF);
	*(tmp + 11) = (uint8_t) (cfg.nllothr >> 8);
	*(tmp + 12) = (uint8_t) (cfg.nlhicthr &  0xFF);
	*(tmp + 13) = (uint8_t) (cfg.nlhicthr >> 8);

	object_address = get_object_address(PROCI_SCHNOISESUPPRESSION_T103, instance);

	//TODO
	if (object_address == 0)
		return(CFG_WRITE_FAILED);

	status = write_mem(object_address, object_size, tmp);

  dbg_op(" write_schnoisesuppression_T103_config object_type -> %d , object_Address -> %d, object_size -> %d\n",PROCI_SCHNOISESUPPRESSION_T103,object_address,object_size);
 
	kfree(tmp);
	return(status);

}

uint8_t  write_auxtouch_T104_config(spt_auxtouchconfig_t104_config_t cfg){
	return(write_simple_config(SPT_AUXTOUCHCONFIG_T104, 0, (void *) &cfg));
}

uint8_t  write_drivenplatehover_T105_config(spt_drivenplatehoverconfig_t105_config_t cfg){
	return(write_simple_config(SPT_DRIVENPLATEHOVERCONFIG_T105, 0, (void *) &cfg));
}



uint8_t write_simple_config(uint8_t object_type, uint8_t instance, void *cfg)
{
	uint16_t object_address;
	uint8_t object_size;
	uint8_t rc;

	object_address = get_object_address(object_type, instance);
	object_size = get_object_size(object_type);
  dbg_op(" write_simple_config object_type -> %d , object_Address -> %d, object_size -> %d\n",object_type,object_address,object_size);

	if ((object_size == 0) || (object_address == 0)){
		rc = CFG_WRITE_FAILED;
	}
	else{
		rc = write_mem(object_address, object_size, cfg);
	}
	return rc; 
}

uint8_t get_object_size(uint8_t object_type)
{
	uint8_t object_table_index = 0;
	uint8_t object_found = 0;
	uint16_t size = OBJECT_NOT_FOUND;
	object_t *object_table;
	object_t obj;

	dbg_func_in();

	if(info_block == NULL)		
		return 0;

	object_table = info_block->objects;
	while ((object_table_index < info_block->info_id->num_declared_objects) &&
			!object_found)
	{
		obj = object_table[object_table_index];
		/* Does object type match? */
		if (obj.object_type == object_type){
			object_found = 1;
			size = obj.size + 1;
		}
		object_table_index++;
	}

	dbg_func_out();
	return(size);
}

uint8_t read_id_block(info_id_t *id)
{
	uint8_t status;	

	status = read_mem(0, 1, (void *) &id->family_id);
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("family_id = 0x%X\n",id->family_id);

	status = read_mem(1, 1, (void *) &id->variant_id);
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("variant_id = 0x%X\n",id->variant_id);

	status = read_mem(2, 1, (void *) &id->version);
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("version = 0x%X\n",id->version);

	status = read_mem(3, 1, (void *) &id->build);
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("familybuild_id = %d\n",id->build);

	status = read_mem(4, 1, (void *) &id->matrix_x_size);
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("matrix_x_size = %d\n",id->matrix_x_size);

	status = read_mem(5, 1, (void *) &id->matrix_y_size);
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("matrix_y_size = %d\n",id->matrix_y_size);

	status = read_mem(6, 1, (void *) &id->num_declared_objects);	
	if (status != READ_MEM_OK) goto read_id_block_exit;
	dbg_i2c("num_declared_objects = %d\n",id->num_declared_objects);

	return status;

read_id_block_exit:

	dbg_cr("error : read_id_block_exit\n");	
	return status;
}

uint16_t get_object_address(uint8_t object_type, uint8_t instance)
{
	uint8_t object_table_index = 0;
	uint8_t address_found = 0;
	uint16_t address = OBJECT_NOT_FOUND;
	object_t *object_table;
	object_t obj;

	if(info_block == NULL)		
		return 0;

	object_table = info_block->objects;
	while ((object_table_index < info_block->info_id->num_declared_objects) &&
			!address_found)
	{
		obj = object_table[object_table_index];
		/* Does object type match? */
		if (obj.object_type == object_type){

			address_found = 1;

			/* Are there enough instances defined in the FW? */
			if (obj.instances >= instance){
				address = obj.start_position + (obj.size + 1) * instance;
			}
		}
		object_table_index++;
	}

	return(address);
}

uint32_t get_stored_infoblock_crc()
{
	uint32_t crc;	

	crc = (uint32_t) (((uint32_t) info_block->CRC_hi) << 16);
	crc = crc | info_block->CRC;

	return(crc);
}

uint8_t calculate_infoblock_crc(uint32_t *crc_pointer)
{

	uint32_t crc = 0;
	uint16_t crc_area_size;
	uint8_t *mem;
	uint8_t i;
	uint8_t rc;

	uint8_t status;

	rc = CRC_CALCULATION_OK;

	/* 7 bytes of version data, 6 * NUM_OF_OBJECTS bytes of object table. */
	crc_area_size = 7 + info_block->info_id->num_declared_objects * 6;

	mem = (uint8_t *) kmalloc(crc_area_size, GFP_KERNEL | GFP_ATOMIC);
	if (mem == NULL){
		rc = CRC_CALCULATION_FAILED;
		dbg_cr("crc_area_size memory is not allocated.\n");	
		goto calculate_infoblock_crc_exit;
	}

	status = read_mem(0, crc_area_size, mem);
	if (status != READ_MEM_OK){
		kfree(mem);
		rc = CRC_CALCULATION_FAILED;
		dbg_cr("[ERROR] crc_area_size read mem.\n");	
		goto calculate_infoblock_crc_exit;
	}

	i = 0;
	while (i < (crc_area_size - 1))
	{
		crc = CRC_24(crc, *(mem + i), *(mem + i + 1));
		i += 2;
	}

	crc = CRC_24(crc, *(mem + i), 0);

	kfree(mem);

	/* Return only 24 bit CRC. */
	*crc_pointer = (crc & 0x00FFFFFF);	

calculate_infoblock_crc_exit:	
	return rc;
}

uint32_t CRC_24(uint32_t crc, uint8_t byte1, uint8_t byte2)
{
	static const uint32_t crcpoly = 0x80001B;
	uint32_t result;
	uint16_t data_word;	

	data_word = (uint16_t) ((uint16_t) (byte2 << 8u) | byte1);
	result = ((crc << 1u) ^ (uint32_t) data_word);

	if (result & 0x1000000)
	{
		result ^= crcpoly;
	}	

	return(result);
}

void touch_data_init(void)
{
	int i = 0;

	for (i = 0; i<MAX_NUM_FINGER; i++ )
	{
		fingerInfo[i].mode = TSC_EVENT_NONE;
		fingerInfo[i].status = -1;
		fingerInfo[i].area = 0;
		keyInfo[i].update = false;
		input_mt_slot(mxt_fw30_data->input_dev, i);						// TOUCH_ID_SLOT
		input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TRACKING_ID, -1);		// RELEASE TOUCH_ID					
	}
	input_report_key(mxt_fw30_data->input_dev, BTN_TOUCH, 0);  // mirinae_ICS
	input_sync(mxt_fw30_data->input_dev);
	active_event = true;
	is_cal_success = false;
}

/*------------------------------ main block -----------------------------------*/
void quantum_touch_probe(void)
{
	uint32_t crc, stored_crc;

	if (init_touch_driver() == DRIVER_SETUP_OK) {
		dbg_config("Touch device found\n");
	}
	else {
		dbg_cr("[ERROR] Touch device NOT found\n");
		return ;
	}

	/* Get and show the version information. */
	if(calculate_infoblock_crc(&crc) != CRC_CALCULATION_OK) {
		dbg_config("Calculating CRC failed, skipping check!\n\r");
	}	

	stored_crc = get_stored_infoblock_crc();
	if (stored_crc != crc) {
		dbg_config("Warning: info block CRC value doesn't match the calculated!\n\r");
	}	

  /* General Object Config Write */
	mxt_T7_Power_Config_Init();		               
	mxt_Acquisition_Config_T8_Init();	           
	/* Touch Object Config Write */
  mxt_KeyArray_T15_Init();      
  mxt_Proximity_Config_T23_Init();	
	mxt_Multitouchscreen_T100_Init();		
  /* Signal Processing Object Config Write */		
	mxt_Grip_Suppression_T40_Config_Init();	
	mxt_Touch_Suppression_T42_Config_Init();	
	mxt_Stylus_T47_Config_Init();
	mxt_Adaptive_Threshold_T55_Config_Init(); 
	mxt_Shieldless_T56_Config_Init();
	mxt_Lensbending_T65_Config_Init();
  mxt_Palmgestureprocessor_T69_Config_Init();
  mxt_Noisesuppression_T72_Config_Init();
  mxt_Glovedetection_T78_Config_Init(); 
  /* Support Processing Object Config Write */
	mxt_CommsConfig_T18_Init();			
	mxt_Gpio_Pwm_T19_Init();			
  mxt_CTE_T46_Config_Init();  
	//mxt_Selftest_T25_Init();
	mxt_CTE_T46_Config_Init();
	mxt_Timer_T61_Config_Init();
	mxt_Goldenreferences_T66_Config_Init();
	
//++ p11309 - 2013.07.19 for T70 dynamic config
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(0);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(1);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(2);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(3);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(4);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(5);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(6);
 	mxt_Dynamicconfigurationcontroller_T70_Config_Init(7);

	mxt_Dynamicconfigurationcontainer_T71_Config_Init();
//-- p11309

#ifdef FEATURE_mXT540S_V_3_0
	mxt_Retransmissioncompensation_T80_Config_Init();
	mxt_Gestureprocess_T84_Config_Init();
	mxt_Touchscreenhover_T101_config_Init();
	mxt_Selfcapcbcrconfig_T102_config_Init();
	mxt_Schnoisesuppression_T103_config_Init();
	mxt_Auxtouch_T104_config_Init();
	mxt_Drivenplatehoverconfig_T105_config_Init();
#endif
  if(mxt_fw30_data->state == SUSMODE){
    mxt_Power_Sleep();
  }
	
	/* Backup settings to NVM. */
	if (backup_config() != WRITE_MEM_OK){
		dbg_config("Failed to backup, exiting...\n");
		return;
	}
	else{
		dbg_config("Backed up the config to non-volatile memory!\n");
	}

	/* Calibrate the touch IC. */
	if (calibrate_chip() != WRITE_MEM_OK){
		dbg_cr(" Failed to calibrate, exiting...\n");
		return;
	}
	else{
		dbg_config("Chip calibrated!\n");
	}

	touch_data_init();

#ifdef TOUCH_IO
	charger_mode = -1;
	pre_charger_mode = -1;
#endif

	dbg_func_out();
	dbg_config("Waiting for touch chip messages...\n");
}

uint8_t init_touch_driver(void)
{
	int i;
	uint8_t tmp;
	uint16_t current_address;
	uint16_t crc_address;
	object_t *object_table;
	info_id_t *id;
	uint8_t status;	
  u16 end_address;
	u8 min_id, max_id,reportid=1;


	// P13106 max_report_id values is changed because memory leaks 120130
	max_report_id = 0;

	// To avoid memory leaks	P13106 120130
	if(info_block){
		dbg_i2c("info_block is not NULL\n");
		if(info_block->info_id){
			dbg_i2c("info_block->info_id is not NULL\n");
			kfree(info_block->info_id);
		}
		if(info_block->objects){
			dbg_i2c("info_block->objects is not NULL\n");
			kfree(info_block->objects);
		}		
		kfree(info_block);
	}
	// To avoid memory leaks	P13106 120130

	/* Read the info block data. */
	id = (info_id_t *) kmalloc(sizeof(info_id_t), GFP_KERNEL | GFP_ATOMIC);
	if (id == NULL)
		return(DRIVER_SETUP_INCOMPLETE);

	if (read_id_block(id) != 1){
		dbg_i2c(" can't read info block data.\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}  

	/* Read object table. */
	object_table = (object_t *) kmalloc(id->num_declared_objects * sizeof(object_t), GFP_KERNEL | GFP_ATOMIC);
	if (object_table == NULL){
		dbg_i2c(" object table memory is not allocated.\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Reading the whole object table block to memory directly doesn't work cause sizeof object_t isn't necessarily the same on every compiler/platform due to alignment issues. Endianness can also cause trouble. */
	current_address = OBJECT_TABLE_START_ADDRESS;

	for (i = 0; i < id->num_declared_objects; i++)
	{
		status = read_mem(current_address, 1, &(object_table[i]).object_type);
		if (status != READ_MEM_OK){
			dbg_i2c(" read object table - type\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_U16(current_address, &object_table[i].start_position);
		if (status != READ_MEM_OK){
			dbg_i2c(" read object table - start position\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address += 2;

		status = read_mem(current_address, 1, (U8*)&object_table[i].size);
		if (status != READ_MEM_OK){
			dbg_i2c(" read object table - size\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_mem(current_address, 1, &object_table[i].instances);
		if (status != READ_MEM_OK){
			dbg_i2c(" read object table - num of instance\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_mem(current_address, 1, &object_table[i].num_report_ids);
		if (status != READ_MEM_OK){
			dbg_i2c(" read object table - num_report_ids\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		max_report_id += object_table[i].num_report_ids * (object_table[i].instances+1);

		/* Find out the maximum message length. */
		if (object_table[i].object_type == GEN_MESSAGEPROCESSOR_T5){
			max_message_length = object_table[i].size + 1;
		}

		dbg_i2c("Object Table: T%u, pos=%u, sz=%u, instance=%u, report_ids=%u(%d)\n", 			
				object_table[i].object_type, 
				object_table[i].start_position, 
				object_table[i].size, 
				object_table[i].instances, 
				object_table[i].num_report_ids,
				max_report_id);

		if(object_table[i].num_report_ids){
			min_id = reportid;
			reportid += object_table[i].num_report_ids *
				OBP_INSTANCES(object_table[i]);
			max_id = reportid - 1;
		} else {
			min_id = 0;
			max_id = 0;
		}

		switch(object_table[i].object_type){
		case GEN_MESSAGEPROCESSOR_T5:
			/* CRC not enabled, therefore don't read last byte */
			mxt_fw30_data->T5_msg_size = OBP_SIZE(object_table[i]) - 1;
			mxt_fw30_data->T5_address = object_table[i].start_position;
			break;
		case GEN_COMMANDPROCESSOR_T6:
			mxt_fw30_data->T6_reportid_min = min_id;
			mxt_fw30_data->T6_reportid_max = max_id;
			mxt_fw30_data->T6_address = object_table[i].start_position;
			break;
		case GEN_POWERCONFIG_T7:
			mxt_fw30_data->T7_address = object_table[i].start_position;
			break;
		case TOUCH_KEYARRAY_T15:
			mxt_fw30_data->T15_reportid_min = min_id;
			mxt_fw30_data->T15_reportid_max = max_id;
			break;
		case PROCI_TOUCHSUPPRESSION_T42:
			mxt_fw30_data->T42_reportid_min = min_id;
			mxt_fw30_data->T42_reportid_max = max_id;
			break;
		case SPT_MESSAGECOUNT_T44:
			mxt_fw30_data->T44_address = object_table[i].start_position;
			break;
		case SPT_CTECONFIG_T46:
			mxt_fw30_data->T46_reportid_min = min_id;
			mxt_fw30_data->T46_reportid_max = max_id;
			break;
		case SPT_GOLDENREFERENCES_T66:
			mxt_fw30_data->T66_reportid_min= min_id;
			mxt_fw30_data->T66_reportid_max= max_id;
			break;
//++ p11309 - 2013.10.21 for Palm Gesture
		case PROCI_PALMGESTUREPROCESSOR_T69:
			mxt_fw30_data->T69_reportid_min= min_id;
			mxt_fw30_data->T69_reportid_max= max_id;
			break;
//-- p11309
		case PROCG_NOISESUPPRESSION_T72:
			mxt_fw30_data->T72_reportid_min = min_id;
			mxt_fw30_data->T72_reportid_max = max_id;
			break;
		case TOUCH_MULTITOUCHSCREEN_T100:
			mxt_fw30_data->T100_reportid_min = min_id;
			mxt_fw30_data->T100_reportid_max = max_id;
			break;
		}

		end_address = object_table[i].start_position + (OBP_SIZE(object_table[i])* OBP_INSTANCES(object_table[i])) - 1;
		//printk("[TOUCH] Object Table -> T%d (min->%d,max->%d)\n",object_table[i].object_type,min_id,max_id);
		if (end_address >= mxt_fw30_data->mem_size)
			mxt_fw30_data->mem_size = end_address + 1;    

	}

	/* Check that message processor was found. */
	if (max_message_length == 0){
		dbg_i2c(" read object table - max_message length\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}	

	info_block = kmalloc(sizeof(info_block_t), GFP_KERNEL | GFP_ATOMIC);
	if (info_block == NULL){
		dbg_i2c(" info block memory is not allocated.\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	info_block->info_id = id;
	info_block->objects = object_table;
	crc_address = OBJECT_TABLE_START_ADDRESS + id->num_declared_objects * OBJECT_TABLE_ELEMENT_SIZE;

	status = read_mem(crc_address, 1u, &tmp);
	if (status != READ_MEM_OK){
		dbg_i2c(" read object table - CRC LSB byte\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	info_block->CRC = tmp;

	status = read_mem(crc_address + 1u, 1u, &tmp);
	if (status != READ_MEM_OK){
		dbg_i2c(" read object table - CRC MSB byte\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	info_block->CRC |= (tmp << 8u);

	status = read_mem(crc_address + 2u, 1u, &info_block->CRC_hi);
	if (status != READ_MEM_OK){
		dbg_i2c(" read object table - CRC high byte\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Store message processor address, it is needed often on message reads. */
	message_processor_address = get_object_address(GEN_MESSAGEPROCESSOR_T5, 0);
	if (message_processor_address == 0){
		dbg_cr(" message processor address is invalid.\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	dbg_i2c("message processor address = %u\n", message_processor_address);

	/* Store command processor address. */
	command_processor_address = get_object_address(GEN_COMMANDPROCESSOR_T6, 0);
	if (command_processor_address == 0){
		dbg_cr(" command processor address is invalid.\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	dbg_i2c("command processor address = %u\n", command_processor_address);

	//p13106
	quantum_msg_total = kcalloc(id->num_declared_objects,max_message_length, GFP_KERNEL);
	if (quantum_msg_total == NULL){
		dbg_cr(" quantum_msg_total msg memory is not allocated.\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Allocate memory for report id map now that the number of report id's is known. */

	max_report_id++; // p11309 - 2013.04.07 0 is reserved, +1 size
	driver_setup = DRIVER_SETUP_OK;

	/* Initialize the pin connected to touch ic pin CHANGELINE to catch the
	 * falling edge of signal on that line. */	
	dbg_i2c("%s: complete.\n", __func__);
	return(DRIVER_SETUP_OK);
}

void  clear_event(uint8_t clear)
{
	uint8_t valid_input_count=0;
	int i;   

	dbg_func_in();
	for ( i= 0; i<MAX_NUM_FINGER; i++ )
	{
		if(fingerInfo[i].mode == TSC_EVENT_WINDOW)
		{
			dbg_op("[MXT_FW30] clear_event U:(%d, %d) (id:%d)\n", fingerInfo[i].x, fingerInfo[i].y, fingerInfo[i].id);
			input_mt_slot(mxt_fw30_data->input_dev, fingerInfo[i].id);						// TOUCH_ID_SLOT
			input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TRACKING_ID, -1);		// RELEASE TOUCH_ID
			fingerInfo[i].mode = TSC_EVENT_NONE;
			fingerInfo[i].status= -1;
		}
		else{
			valid_input_count++;
		}
	}
	input_report_key(mxt_fw30_data->input_dev, BTN_TOUCH, 0);  // mirinae_ICS
	dbg_op(" touch event num => %d\n",valid_input_count);
	input_sync(mxt_fw30_data->input_dev);

	if(clear == TSC_CLEAR_ALL)
	{
		for ( i= 0; i<MAX_NUM_FINGER; i++ )
		{
			fingerInfo[i].mode = TSC_EVENT_NONE;
			fingerInfo[i].status = -1;
			fingerInfo[i].area = 0;
			keyInfo[i].update = false; 
		}     
	}
	dbg_func_out();
}

#ifdef PAN_TOUCH_CAL_PMODE
void pan_touch_protection_mode(void){
  dbg_cr("pan_touch_protection_mode set\n");
  
  //2014.3.27 P13106 T8 TOUCH Auto Cal & Recal 
  acquisition_config.tchautocal= PAN_PMODE_TCHAUTOCAL;
	acquisition_config.atchcalst = PAN_PMODE_ATCHCALST;
	acquisition_config.atchcalsthr = PAN_PMODE_ATCHCALSTHR;
	acquisition_config.atchfrccalthr = PAN_PMODE_ATCHFRCCALTHR;
	acquisition_config.atchfrccalratio = PAN_PMODE_ATCHFRCCALRATIO;

	if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK) {
		dbg_cr("write_acquisition_T8_config error\n");
	}
/*  
 //2014.3.27 P13106 T100 TOUCH THRESHOLD SET INTTHRESHOLD VALUE.
  if(mTouch_mode==0){
    touchscreen_config.tchthr = obj_multi_touch_t100[mTouch_mode].intthr;
  	touchscreen_config.tchhyst= obj_multi_touch_t100[mTouch_mode].intthrhyst;
  	if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK){
  		dbg_cr("mxt_Multitouchscreen_config Error!!!\n");
  	}
	}
*/
  //2014.3.27 p13106 disable T42 touch suppression.
  if(touchsuppression_t42_config.ctrl != 0){
    touchsuppression_t42_config.ctrl = 0;
    if (get_object_address(PROCI_TOUCHSUPPRESSION_T42, 0) != OBJECT_NOT_FOUND){
		  if (write_touch_suppression_T42_config(touchsuppression_t42_config) != CFG_WRITE_OK)
			  dbg_cr("T42 Configuration Fail!!! , Line %d \n", __LINE__);		
	  }
	}
//#if defined(CONFIG_MACH_MSM8974_EF56S) ||	defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) 
	//2014.3.27 p13106 disable T80 touch suppression.
  if(retransmissioncompensation_t80_config.ctrl != 0){
  	retransmissioncompensation_t80_config.ctrl = 0;
  	if (get_object_address(PROCI_RETRANSMISSIONCOMPENSATION_T80, 0) != OBJECT_NOT_FOUND) {
  		if (write_retransmissioncompensation_T80_config(retransmissioncompensation_t80_config) != CFG_WRITE_OK)
  			dbg_cr("T80 Configuration Fail!!! , Line %d \n\r", __LINE__);
  	}
	}
//#endif
}

#endif

void cal_maybe_good(void)
{  	
	/* Check if the timer is enabled */
	if (good_calibration_cnt >= 1) {
		dbg_touch("[PMODE] good_calibration_cnt = %d \n", good_calibration_cnt);
		/* Check if the timer timedout of 0.3seconds */
		if ((jiffies_to_msecs(jiffies) - mxt_time_point) >= 300) {
			dbg_op(" time from touch press after calibration started = %d\n", (jiffies_to_msecs(jiffies) - mxt_time_point));
			/* cal was good - don't need to check any more */
			mxt_time_point = 0;
			cal_check_flag = 0;
			
#ifdef PAN_TOUCH_CAL_PMODE
			cal_correction_limit = 5;
#endif
/*      
//++ p11309 - 2014.01.02 for Preventing from Reverse Acceleration 
      touchscreen_config.ctrl= obj_multi_touch_t100[mTouch_mode].ctrl;
      touchscreen_config.scraux = obj_multi_touch_t100[mTouch_mode].scraux;
      if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK){
        dbg_cr("mxt_Multitouchscreen_config Error!!!\n");
      }      
//-- p11309
      */
			/* Write back the normal acquisition config to chip. */
			acquisition_config.atchcalst = obj_acquisition_config_t8[mTouch_mode].atchcalst;
			acquisition_config.atchcalsthr = obj_acquisition_config_t8[mTouch_mode].atchcalsthr;
			acquisition_config.atchfrccalthr = obj_acquisition_config_t8[mTouch_mode].atchfrccalthr;
			acquisition_config.atchfrccalratio = obj_acquisition_config_t8[mTouch_mode].atchfrccalratio;
			
			if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK)
			{
				/* "Acquisition config write failed!\n" */
				dbg_cr("%s: write_acquisition_T8_config error\n", __func__);
				//ret = WRITE_MEM_FAILED; /* calling function should retry calibration call */
			}
			dbg_touch("[PMODE] Calibration success!!\n");
		}
		else { 
			cal_check_flag = 1u;
			good_calibration_cnt++;
		}
	}
	else { 
		/* Timer not enabled, so enable it */
		cal_check_flag = 1u;
		good_calibration_cnt++;
	}   
}

int8_t check_chip_calibration(void)
{
	uint8_t tch_ch = 0, atch_ch = 0;
	uint8_t i;
	uint8_t CAL_THR = 10;
	uint8_t num_of_antitouch = 1;
	uint8_t finger_cnt = 0;


	if(get_touch_antitouch_info()>0) {
		dbg_op(" tch_ch:%d, atch_ch:%d\n",debugInfo.tch_ch, debugInfo.atch_ch);
		tch_ch = debugInfo.tch_ch;
		atch_ch = debugInfo.atch_ch;
		/* process counters and decide if we must re-calibrate or if cal was good */      

		// Count fingers
		for (i = 0 ; i < MAX_NUM_FINGER; ++i) { 
			if (fingerInfo[i].status == -1) continue;
			finger_cnt++;
		}

		if (cal_check_flag != 1) {
			dbg_op(" check_chip_calibration just return!! finger_cnt = %d\n", finger_cnt);
			return 0;
		}

		/* process counters and decide if we must re-calibrate or if cal was good */ 

		// Palm 
		if ((tch_ch > 18) || (atch_ch > 15) || ((tch_ch + atch_ch) > 20)) {  
			dbg_op(" maybe palm, re-calibrate!! \n");
			calibrate_chip();
		} 
		// Good 
		else if((tch_ch > 0) && (tch_ch <= 18) && (atch_ch == 0)) {
			dbg_op(" calibration maybe good\n");
			if ((finger_cnt >= 2) && (tch_ch <= 3)) {
				dbg_op(" finger_cnt = %d, re-calibrate!! \n", finger_cnt);	
				calibrate_chip();
			} else {
				cal_maybe_good();
				not_yet_count = 0;
			}
		}
		// When # of antitouch is large related to finger cnt	
		else if (atch_ch > ((finger_cnt * num_of_antitouch) + 5)) { //2 
			dbg_op(" calibration was bad (finger : %d, max_antitouch_num : %d)\n", finger_cnt, finger_cnt*num_of_antitouch);
			calibrate_chip();
		} 
		// When atch > tch 
		else if((tch_ch + CAL_THR) <= atch_ch) { 
			dbg_op(" calibration was bad (CAL_THR : %d)\n",CAL_THR);
			/* cal was bad - must recalibrate and check afterwards */
			calibrate_chip();
		} 
		// No touch, only antitouch
		else if((tch_ch == 0 ) && (atch_ch >= 2)) { 
			dbg_op(" calibration was bad, tch_ch = %d, atch_ch = %d)\n", tch_ch, atch_ch);
			/* cal was bad - must recalibrate and check afterwards */
			calibrate_chip();
		} 
		else if((tch_ch == 0 ) && (atch_ch ==0)) { 
			dbg_op(" calibration was good, tch_ch = %d, atch_ch = %d)\n", tch_ch, atch_ch);
			cal_maybe_good();
			not_yet_count = 0;
		}
		// Else. 
		else { 
			not_yet_count++;
			dbg_op(" calibration was not decided yet, not_yet_count = %d\n", not_yet_count);
			if(not_yet_count >= 2) {		// retry count 30 -> 2
				dbg_op(" not_yet_count over 2, re-calibrate!! \n");
				not_yet_count =0;
				calibrate_chip();
			}
		}
	}
	
	return 0;
}

int8_t get_touch_antitouch_info(void) {
//++ p11309 - 2014.01.02 for Preventing from Reverse Acceleration   
#if (1)
    return 1;
#else
	uint8_t *data_buffer;
	uint8_t *touch_flag;	
	uint8_t try_ctr = 0;
	uint8_t data_byte = 0xF3; /* dianostic command to get touch flags */
	uint16_t diag_address;
	uint8_t tch_ch = 0, atch_ch = 0;
	uint8_t check_mask;
	uint8_t i;
	uint8_t j;
		
	if (driver_setup != DRIVER_SETUP_OK)
		return -1;

	data_buffer = kzalloc(128, GFP_KERNEL);
	if (!data_buffer) 
		return -ENOMEM;

	touch_flag = kzalloc(30*4*2, GFP_KERNEL);
	if (!touch_flag)
		return -ENOMEM;		

	memset(touch_flag , 0x00, sizeof(touch_flag));

	write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte);
	diag_address = get_object_address(DEBUG_DIAGNOSTIC_T37,0);

	msleep(10); 
	
	memset( data_buffer , 0xFF, sizeof( data_buffer ));	
	while(!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00)))
	{
		if(try_ctr > 10) {
			dbg_cr(" Diagnostic Data did not update!!\n");
			return -EBUSY;
		}

		msleep(5); 
		try_ctr++;
		read_mem(diag_address, 2,data_buffer);
	}

	read_mem(diag_address, 128, data_buffer);
	memcpy(touch_flag, data_buffer+2, 126);	

	data_byte = 0x01;
	write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte);
	diag_address = get_object_address(DEBUG_DIAGNOSTIC_T37,0);
	
	memset( data_buffer , 0xFF, sizeof( data_buffer ));	
	while(!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x01)))
	{
		if(try_ctr > 10) {
			dbg_cr(" Diagnostic Data did not update!!\n");
			return -EBUSY;
		}

		msleep(5); 
		try_ctr++;
		read_mem(diag_address, 2, data_buffer);
	}

	read_mem(diag_address, 128, data_buffer);
	memcpy(touch_flag+126, data_buffer+2, 114);	

	if(data_buffer[0] == 0xF3)
	{		
		for(i = 0; i < 120; i += 4)
		{			
			dbg_op("Detect Flags X%d, [TCH] %x %x %x %x, [ATCH] %x %x %x %x\n", 
				i/4,
				touch_flag[3+i],touch_flag[2+i],
				touch_flag[1+i],touch_flag[0+i],
				touch_flag[123+i],touch_flag[122+i],
				touch_flag[121+i],touch_flag[120+i]
			);			

			/* count how many bits set for this row */
			for(j = 0; j < 8; j++)
			{
				/* create a bit mask to check against */
				check_mask = 1 << j;

				/* check detect flags */
				if(touch_flag[0+i] & check_mask) tch_ch++;
				if(touch_flag[1+i] & check_mask) tch_ch++;
				if(touch_flag[2+i] & check_mask) tch_ch++;
				if(touch_flag[3+i] & check_mask) tch_ch++;

				if(touch_flag[120+i] & check_mask) atch_ch++;
				if(touch_flag[121+i] & check_mask) atch_ch++;
				if(touch_flag[122+i] & check_mask) atch_ch++;
				if(touch_flag[123+i] & check_mask) atch_ch++;
			}
		}

		debugInfo.tch_ch = tch_ch;
		debugInfo.atch_ch = atch_ch;		
		
		dbg_op("Flags Counted channels: t:%d a:%d \n", tch_ch, atch_ch);

		data_byte = 0x01;
		write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte);

		kfree(data_buffer);
		kfree(touch_flag);		

		return 1;
	}

	dbg_func_out();

	return 0;	
#endif
//-- p11309 - 2014.01.02 for Preventing from Reverse Acceleration   

}
#ifdef PAN_HAVE_TOUCH_KEY
int get_touch_key_pos(int x, int y) 
{	
	dbg_op("(%d, %d), (%d~%d, %d~%d), (%d~%d, %d~%d), (%d~%d, %d~%d)\n",
			x, y, 
			(PAN_1ST_TOUCH_KEY_X - PAN_1ST_TOUCH_KEY_MARGIN_X),
			(PAN_1ST_TOUCH_KEY_X + PAN_1ST_TOUCH_KEY_MARGIN_X),
			(PAN_TOUCH_KEY_Y - PAN_TOUCH_KEY_MARGIN_Y),
			(PAN_TOUCH_KEY_Y + PAN_TOUCH_KEY_MARGIN_Y),
			(PAN_2ND_TOUCH_KEY_X - PAN_2ND_TOUCH_KEY_MARGIN_X),
			(PAN_2ND_TOUCH_KEY_X + PAN_2ND_TOUCH_KEY_MARGIN_X),
			(PAN_TOUCH_KEY_Y - PAN_TOUCH_KEY_MARGIN_Y),
			(PAN_TOUCH_KEY_Y + PAN_TOUCH_KEY_MARGIN_Y),
			(PAN_3RD_TOUCH_KEY_X - PAN_3RD_TOUCH_KEY_MARGIN_X),
			(PAN_3RD_TOUCH_KEY_X + PAN_3RD_TOUCH_KEY_MARGIN_X),
			(PAN_TOUCH_KEY_Y - PAN_TOUCH_KEY_MARGIN_Y),
			(PAN_TOUCH_KEY_Y + PAN_TOUCH_KEY_MARGIN_Y));

	if ( x >= (PAN_1ST_TOUCH_KEY_X - PAN_1ST_TOUCH_KEY_MARGIN_X) && 
			x <= (PAN_1ST_TOUCH_KEY_X + PAN_1ST_TOUCH_KEY_MARGIN_X) &&
			y >= (PAN_TOUCH_KEY_Y - PAN_TOUCH_KEY_MARGIN_Y) && 
			y <= (PAN_TOUCH_KEY_Y + PAN_TOUCH_KEY_MARGIN_Y) ) {
		return 0;
	}

	if ( x >= (PAN_2ND_TOUCH_KEY_X - PAN_2ND_TOUCH_KEY_MARGIN_X) && 
			x <= (PAN_2ND_TOUCH_KEY_X + PAN_2ND_TOUCH_KEY_MARGIN_X) &&
			y >= (PAN_TOUCH_KEY_Y - PAN_TOUCH_KEY_MARGIN_Y) && 
			y <= (PAN_TOUCH_KEY_Y + PAN_TOUCH_KEY_MARGIN_Y) ) {
		return 1;
	}

	if ( x >= (PAN_3RD_TOUCH_KEY_X - PAN_3RD_TOUCH_KEY_MARGIN_X) && 
			x <= (PAN_3RD_TOUCH_KEY_X + PAN_3RD_TOUCH_KEY_MARGIN_X) &&
			y >= (PAN_TOUCH_KEY_Y - PAN_TOUCH_KEY_MARGIN_Y) && 
			y <= (PAN_TOUCH_KEY_Y + PAN_TOUCH_KEY_MARGIN_Y) ) {
		return 2;	
	}	

	return -1;
}
#endif
int get_message_T100(uint8_t *quantum_msg, unsigned int *touch_status)
{
	uint8_t touch_type = 0, touch_event = 0, touch_detect = 0, check_press=0, check_release=0, id =0;
	unsigned long x=0, y=0;
	int size=0;

	/* Treat screen messages */
	if (quantum_msg[0] < MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		if (quantum_msg[0] == MXT_T100_SCREEN_MSG_FIRST_RPT_ID)
			dbg_op("SCRSTATUS:[%02X] %02X %04X %04X %04X\n",
				quantum_msg[0], quantum_msg[1], (quantum_msg[3] << 8) | quantum_msg[2],
				(quantum_msg[5] << 8) | quantum_msg[4],
				(quantum_msg[7] << 8) | quantum_msg[6]);
//++ p11309 - 2014.01.02 for Preventing from Reverse Acceleration 
			debugInfo.tch_ch = (quantum_msg[3] << 8) | quantum_msg[2];
			debugInfo.atch_ch = (quantum_msg[5] << 8) | quantum_msg[4];
			dbg_cr("Touch cnt -> %d, Anti Touch cnt-> %d\n",debugInfo.tch_ch,debugInfo.atch_ch);
//-- p11309	
		return MESSAGE_READ_FAILED;
	}

	/* Treate touch status messages */
	id = quantum_msg[0] - MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID;
	touch_detect = (quantum_msg[1] & 0x80) >> 7;
	touch_type = (quantum_msg[1] & 0x70) >> 4;
	touch_event = quantum_msg[1] & 0x0F;

	dbg_op("TCHSTATUS [%d] : DETECT[%d] TYPE[%d] EVENT[%d] %d,%d,%d,%d,%d\n",
		id, touch_detect, touch_type, touch_event,
		quantum_msg[1] | (quantum_msg[2] << 8), quantum_msg[3] | (quantum_msg[4] << 8),
		quantum_msg[5], quantum_msg[6], quantum_msg[7]);

	switch (touch_type) {
	case MXT_T100_TYPE_FINGER:
	case MXT_T100_TYPE_PASSIVE_STYLUS:
	case MXT_T100_TYPE_GLOVE:

		if(touch_detect) {
			check_press = 1;

#ifdef PAN_TOUCH_CAL_PMODE
			if(debugInfo.autocal_flag==1){	
			  if(touch_event==MXT_T100_EVENT_DOWN){ // After touch calibration, first touch press event.
  				dbg_touch("[PMODE] First touch event after touch calibration\n");
  				debugInfo.autocal_flag++;
  				
  					dbg_touch("[PMODE] Autocal Timer Enabled.. %d msec\n", 
  						PAN_PMODE_AUTOCAL_ENABLE_TIME);
  					mod_timer(&pan_pmode_autocal_timer, jiffies + 
  						msecs_to_jiffies(PAN_PMODE_AUTOCAL_ENABLE_TIME));
  				/*
  				acquisition_config.atchcalst = obj_acquisition_config_t8[mTouch_mode].atchcalst;
  				acquisition_config.atchcalsthr = obj_acquisition_config_t8[mTouch_mode].atchcalsthr;
  				acquisition_config.atchfrccalthr = obj_acquisition_config_t8[mTouch_mode].atchfrccalthr;
  				acquisition_config.atchfrccalratio = obj_acquisition_config_t8[mTouch_mode].atchfrccalratio;

  				if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK) {
  					dbg_cr("%s: write_acquisition_T8_config error\n", __func__);
  				}
  				*/
				}else{
				  dbg_touch("Move Event after touch calibration\n");
				}
			}
#endif 
		}
		else {
			if (touch_event == MXT_T100_EVENT_UP || touch_event == MXT_T100_EVENT_SUPPRESS) {
			  if(touch_event == MXT_T100_EVENT_SUPPRESS && pan_pmode_resume_cal){
				  dbg_cr(" Touch Suppress! calibrate_chip!!\n\n\n");
				  //calibrate_chip();
				}
        
				fingerInfo[id].status= 0;
				dbg_op(" TOUCH_RELEASE || TOUCH_SUPPRESS !!, 0x%x\n", touch_event);
				check_release = 1;
	
			} else {
				dbg_cr("Untreated Undetectd touch : type[%d], event[%d]\n",
				touch_type, touch_event);
				return MESSAGE_READ_FAILED;
			}
			break;
		}

		if (touch_event == MXT_T100_EVENT_DOWN || 
			touch_event == MXT_T100_EVENT_UNSUPPRESS || 
			touch_event == MXT_T100_EVENT_MOVE || 
			touch_event == MXT_T100_EVENT_NONE) {

			x = quantum_msg[2] | (quantum_msg[3] << 8);
			y = quantum_msg[4] | (quantum_msg[5] << 8);

			x = (u16)((x * SCREEN_RESOLUTION_X) / obj_multi_touch_t100[mTouch_mode].yrange);  
#ifdef PAN_HAVE_TOUCH_KEY
			y = (u16)((y * SCREEN_RESOLUTION_WHOLE_Y) / obj_multi_touch_t100[mTouch_mode].xrange);
#else
			y = (u16)((y * SCREEN_RESOLUTION_SCREEN_Y) / obj_multi_touch_t100[mTouch_mode].xrange);
#endif
			size =  quantum_msg[6];       

			fingerInfo[id].id = id;
			fingerInfo[id].status = TOUCH_EVENT_PRESS;
			fingerInfo[id].area = size;
			fingerInfo[id].x = (int16_t)x;
			fingerInfo[id].y = (int16_t)y;

//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
			fingerInfo[id].ignored_by_cover = false;

//++ add 2013.08.26 check hallic vs ui state.
			if ( mTouch_cover_status_hallic || mTouch_cover_status_ui ) {
//-- p11309
				if ( ( fingerInfo[id].x < SMART_COVER_AREA_LEFT )  ||
					 ( fingerInfo[id].x > SMART_COVER_AREA_RIGHT ) ||
					 ( fingerInfo[id].y < SMART_COVER_AREA_TOP )   ||
					 ( fingerInfo[id].y > SMART_COVER_AREA_BOTTOM ) )

			   fingerInfo[id].ignored_by_cover = true;		
			}
#endif
//-- p11309

		} else {
			dbg_cr("Untreated Detectd touch : type[%d], event[%d]\n",
					touch_type, touch_event);
			return MESSAGE_READ_FAILED;
		}
		break;
	}
  
	if ( touch_event == MXT_T100_EVENT_UP || touch_event == MXT_T100_EVENT_SUPPRESS )    
	{
		fingerInfo[id].status= TOUCH_EVENT_RELEASE;
		*touch_status=TOUCH_EVENT_RELEASE;
		fingerInfo[id].area= 0;
		dbg_touch("#####[%d] Touch[%d] Up (%d,%d) size (%d)\n",touch_type, id, fingerInfo[id].x, fingerInfo[id].y , fingerInfo[id].area);
	}
	else if ( touch_event == MXT_T100_EVENT_MOVE )  
	{
		fingerInfo[id].id= id;
		fingerInfo[id].status= TOUCH_EVENT_MOVE;
		*touch_status=TOUCH_EVENT_MOVE;
		fingerInfo[id].x= (int16_t)x;
		fingerInfo[id].y= (int16_t)y;
		fingerInfo[id].area= size;
		dbg_touch("#####[%d] Touch[%d] Move (%d,%d) size (%d)\n ",touch_type, id, fingerInfo[id].x, fingerInfo[id].y, fingerInfo[id].area );
	}
	else if ( touch_event == MXT_T100_EVENT_DOWN || touch_event == MXT_T100_EVENT_UNSUPPRESS )
	{                               
		fingerInfo[id].id= id;
		fingerInfo[id].status= TOUCH_EVENT_PRESS;
		*touch_status=TOUCH_EVENT_PRESS;
		fingerInfo[id].x= (int16_t)x;
		fingerInfo[id].y= (int16_t)y;
		fingerInfo[id].area= size;
		dbg_touch("#####[%d] Touch[%d] Down (%d,%d) size (%d)\n",touch_type, id, fingerInfo[id].x, fingerInfo[id].y, fingerInfo[id].area );

		if(touch_event == MXT_T100_EVENT_UNSUPPRESS){
		  dbg_touch("Touch MXT_T100_EVENT_UNSUPPRESS\n");
		}
	}                 
	else {
		*touch_status = TOUCH_EVENT_NOTHING;
		dbg_cr("[TOUCH] Unknown state ! status = %4d, %4d, %4d, %4d, %4d, %4d, %4d\n", quantum_msg[1],quantum_msg[2],quantum_msg[3],quantum_msg[4],quantum_msg[5],quantum_msg[6],quantum_msg[7]);
		return MESSAGE_READ_FAILED;
	}

//++ p11309 - 2013.07.19 Support Soft Dead zone
#ifdef PAN_SUPPORT_SOFT_DEAD_ZONE
	if ( pan_support_soft_dead_zone == 1 ) {

	#ifdef PAN_HAVE_TOUCH_KEY
		if ( get_touch_key_pos(x, y) < 0 ) 
	#endif
		{	// on screen case.
			if ( x <= PAN_SOFT_DEAD_ZONE_SIZE && x >= 0 ) {
				fingerInfo[id].x = PAN_SOFT_DEAD_ZONE_SIZE;
			}
			if ( x >= PAN_SOFT_DZ_X_R_LO && x <= PAN_SOFT_DZ_X_R_HI ) {
				fingerInfo[id].x = PAN_SOFT_DZ_X_R_LO;
			}
			if ( y <= PAN_SOFT_DEAD_ZONE_SIZE && y >= 0 ) {
				fingerInfo[id].y = PAN_SOFT_DEAD_ZONE_SIZE;
			}
			if ( y >= PAN_SOFT_DZ_Y_BTM_LO && y <= PAN_SOFT_DZ_Y_BTM_HI) {
				fingerInfo[id].y = PAN_SOFT_DZ_Y_BTM_LO;
			}
		}		
	}
#endif
//-- p11309

#ifdef PAN_TOUCH_CAL_PMODE
	/* touch_message_flag - set when tehre is a touch message
	   facesup_message_flag - set when there is a face suppression message */
	// check chip's calibration status when touch down or up
	/*
	if(cal_check_flag == 1 && (fingerInfo[id].status == TOUCH_EVENT_PRESS)) {
		if (mxt_time_point == 0) 
			mxt_time_point = jiffies_to_msecs(jiffies);
		cal_correction_limit = 0;
		check_chip_calibration();
	}
	*/
#endif

 return MESSAGE_READ_OK;
}

void get_message_T6(uint8_t *quantum_msg){
	if(quantum_msg[1] & 0x80 ) {
		dbg_op_err("in reset.\n");
		//clear_event(TSC_CLEAR_ALL);
	}
	if(quantum_msg[1] & 0x40 ) {
		dbg_op_err("T6 in Overflow in acquisition and processing cycle length.\n");
	}
	if(quantum_msg[1] & 0x20 ) {
		dbg_op_err("T6 in Acquisition Error.\n");
	}
	if(quantum_msg[1] & 0x10) {

#ifdef PAN_TOUCH_CAL_PMODE 			
		
		dbg_touch("[PMODE] Received Calibration Message\n");

		cal_check_flag=1u;
		mxt_time_point = 0;

		debugInfo.calibration_cnt++;
		
    //2014.3.27 p13106
		//cancel_work_sync(&pan_pmode_antical_wq);
		//del_timer(&pan_pmode_antical_timer);

		if(debugInfo.autocal_flag) {
			dbg_touch("[PMODE] Autocal enabled...Auto cal timer refresh.\n");
			cancel_work_sync(&pan_pmode_autocal_wq);
			del_timer(&pan_pmode_autocal_timer);
		}
		/*
//++ p11309 - 2014.01.02 for Preventing from Reverse Acceleration 
    touchscreen_config.ctrl = 131;
		touchscreen_config.scraux = 6;
		if (write_multitouchscreen_T100_config(0, touchscreen_config) != CFG_WRITE_OK){
			dbg_cr("mxt_Multitouchscreen_config Error!!!\n");
		}
//-- p11309
    */
    pan_touch_protection_mode();
    cal_correction_limit = 5;
  	pan_pmode_resume_cal = true;
  	debugInfo.autocal_flag=1;
  	//2014.3.27 P13106
  	/*
		if(!pan_pmode_resume_cal){			
			dbg_touch("[PMODE] PROTECTION MODE Timer - T6 Cmd %d msec\n", 
				PAN_PMODE_AUTOCAL_ENABLE_TIME);
			mod_timer(&pan_pmode_autocal_timer, jiffies + 
				msecs_to_jiffies(PAN_PMODE_AUTOCAL_ENABLE_TIME));
		}
		*/

		
#endif
	}   
	if(quantum_msg[1] & 0x08 ) {
		dbg_op_err("T6 in Object Configuration Error.\n");
	}
	if(quantum_msg[1] & 0x04) {
		dbg_op_err("T6 in Communication checksum Error.\n");
	}   
	if(quantum_msg[1] & 0x00) {
		dbg_op("T6 in idle.\n");
	}
	return;
}


//++ p11309 - 2013.05.14 for gold reference T66, add 2013.05.26 for stabilization & dual x
void get_message_T66(uint8_t *quantum_msg){
  
	pan_gld_ref_ic_status = quantum_msg[1] & 0xFF;
	pan_gld_ref_cal_status = (pan_gld_ref_ic_status & 0x06);
	pan_gld_ref_max_diff = quantum_msg[2];
	pan_gld_ref_max_diff_x = quantum_msg[3];
	pan_gld_ref_max_diff_y = quantum_msg[4];    
  
	if(pan_gld_ref_ic_status & PAN_GLD_REF_STATUS_BAD_DATA) {
		dbg_cr(" Gold Referecne status: Bad Stored Data.\n");
		return;
	}
  
	if( pan_gld_ref_ic_status & PAN_GLD_REF_STATUS_FAIL ) {
		dbg_cr(" Gold Referecne status: FAIL\n");
		dbg_cr(" Gold Referecne max diff: %d (FCALFAILTHR: %d)\n",pan_gld_ref_max_diff, goldenreferences_t66_config.fcalfailthr);
		dbg_cr(" Gold Referecne max diff x: %d\n", pan_gld_ref_max_diff_x);
		dbg_cr(" Gold Referecne max diff y: %d\n", pan_gld_ref_max_diff_y);
 		return;
	}

	if( pan_gld_ref_ic_status & PAN_GLD_REF_STATUS_SEQ_TIMEOUT ) {
		dbg_cr(" Gold Referecne status: Seq Timeout\n");
	}   
	
	if( pan_gld_ref_ic_status & PAN_GLD_REF_STATUS_SEQ_ERROR ) {
		dbg_cr(" Gold Referecne status: Seq Error\n");		
	}  
	
	if( pan_gld_ref_ic_status & PAN_GLD_REF_STATUS_PASS ) {
		dbg_cr(" Gold Referecne status: PASS\n");      
	}
  
	if ( pan_gld_ref_cal_status == PAN_GLD_REF_STATUS_GENERATED ) {
		dbg_cr(" Gold Referecne cal status: Generated\n");     
	}

	if( pan_gld_ref_ic_status & PAN_GLD_REF_STATUS_SEQ_DONE ) {
		dbg_cr(" Gold Referecne status: Seq Done. \n");
	}
}

//++ p11309 - 2013.10.21 for Palm Gesture
static void get_message_T69(uint8_t *quantum_msg)
{
	dbg_cr(" T69 STATUS1 0x%x STATUS2 0x%x\n",quantum_msg[1],quantum_msg[2]);
	if( quantum_msg[1] == 0 ) {
		dbg_cr("T69 Palm Gesture: [Type] None.\n");
	}
	if( quantum_msg[1] == 1 ) {
		dbg_cr("T69 Palm Gesture: [Type] Flat Palm.\n");
	}
	if( quantum_msg[1] == 2 ) {
		dbg_cr("T69 Palm Gesture: [Type] Palm Edge Swipe.\n");
	}
	if( quantum_msg[1] == 3 ) {
		dbg_cr("T69 Palm Gesture: [Type] Palm Edge Swipe Up.\n");
	}
	if( quantum_msg[1] == 4 ) {
		dbg_cr("T69 Palm Gesture: [Type] Palm Edge Swipe Down.\n");
	}
	if( quantum_msg[1] == 5 ) {
		dbg_cr("T69 Palm Gesture: [Type] Palm Edge Swipe Right.\n");
	}
	if( quantum_msg[1] == 6 ) {
		dbg_cr("T69 Palm Gesture: [Type] Palm Edge Swipe Left.\n");
	}

	if( quantum_msg[2] == 0 ) {
		dbg_cr("T69 Palm Gesture: [Event] None.\n");
	}
	if( quantum_msg[2] == 0x04 ) {
		dbg_cr("T69 Palm Gesture: [Event] Press.\n");
	}
	if( quantum_msg[2] == 0x08 ) {
		dbg_cr("T69 Palm Gesture: [Event] Release.\n");
	}
	if( quantum_msg[2] == 0x10 ) {
		dbg_cr("T69 Palm Gesture: [Event] Complete Event.\n");
	}
}
//-- p11309

//++ p11309 - 2013.07.19 Check Noise Mode shake
static void get_message_T72(uint8_t *quantum_msg){

	dbg_cr("T72 STATUS1 0x%x STATUS2 0x%x\n",quantum_msg[1],quantum_msg[2]);

	if( quantum_msg[1] & 0x01) {
		dbg_op_err("T72 Blcok Touch Processing.\n");
	}
	if( quantum_msg[1] & 0x02) {
		dbg_op_err("T72 Touch is Present.\n");
	}
	if( quantum_msg[1] & 0x04 ) {
		dbg_op_err("T72 The numver of ADCs per X is changed.\n");
	}
	if( quantum_msg[1] & 0x08 ) {
		dbg_op_err("T72 Noise state is changed.\n");
	}
	if( quantum_msg[1] & 0x10 ) {
		dbg_op_err("T72 Noise Level is changed.\n");
	}
	if( quantum_msg[1] & 0x20 ) {
		dbg_op_err("T72 Noise Level Threshold is changed.\n");
	}
	if( quantum_msg[1] & 0x40 ) {
		dbg_op_err("T72 Dual x Operation state is changed...%d\n", (quantum_msg[2] & 0x08)>>3);
	}
	if( quantum_msg[1] & 0x80 ) {
		dbg_op_err("T72 NOISE_IN pin state is changed\n");
	}

	if( (quantum_msg[2] & 0x07) == 1 ) {
		dbg_op_err("T72 Noise Suppression States in OFF.\n");
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
		pan_noise_mode_shake_state = 0;
#endif
	}
	if( (quantum_msg[2] & 0x07) == 2 ) {
		dbg_op_err("T72 Noise Suppression States in STABLE.\n");
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
		pan_noise_mode_shake_state = 0;
#endif
	}
	if( (quantum_msg[2] & 0x07) == 3 ) {
		dbg_op_err("T72 Noise Suppression States in NOISY.\n");
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
		pan_noise_mode_shake_state = 1;
#endif
	}
	if( (quantum_msg[2] & 0x07) == 4 ) {
		dbg_op_err("T72 Noise Suppression States in VERY_NOISY.\n");
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
		pan_noise_mode_shake_state = 2;
#endif
	} 
 
	dbg_op("T72 Current Minimum Touch Threshold: %d\n", quantum_msg[3]);
	dbg_op("T72 Peak Measured Noise Level: %d\n", quantum_msg[4]);
	dbg_op("T72 Current Measured Noise Level: %d\n", quantum_msg[5]);
	dbg_op("T72 Current Noise lines threshold: %d\n", quantum_msg[6]);
}
//-- p11309

static void get_message_T15(uint8_t *quantum_msg)
{
	//++ p11309 - 2013.09.01 for Optional
#ifdef PAN_TOUCH_KEY_REJECTION_ON_DISPLAY
	int i;
#endif
	dbg_op("T15 STATUS 0x%x KEYSTATE 0x%x\n", quantum_msg[1], quantum_msg[2]);

#ifdef PAN_T15_KEYARRAY_ENABLE
	if(quantum_msg[1] == 0x80)
	{
//++ p11309 - 2013.09.01 for Optional
#ifdef PAN_TOUCH_KEY_REJECTION_ON_DISPLAY
		for(i=0;i<MAX_NUM_FINGER;i++){
			if(fingerInfo[i].status > 0){
				dbg_op("[KeyArray] Even if Menu/back key is pressed, Menu/back key is ignored because there is touch event on window,\n");
				return;
			}
		}
#endif

#ifdef PAN_SUPPORT_SMART_COVER
		if(mTouch_cover_status_hallic || mTouch_cover_status_ui) {
			dbg_cr("[KeyArray] Menu/back key is pressed but smart-cover is closed.\n");
			return;
		}
#endif

		if(quantum_msg[2] == 1){
			dbg_op("[KeyArray] MENU DOWN\n");
			mPan_KeyArray[0].key_state=true;
			input_report_key(mxt_fw30_data->input_dev, mPan_KeyArray[0].key_num, 1);
			input_sync(mxt_fw30_data->input_dev);
		}
		else if(quantum_msg[2] == 2)   {
			dbg_op("[KeyArray] BACK DOWN\n");
			mPan_KeyArray[1].key_state=true;
			input_report_key(mxt_fw30_data->input_dev,  mPan_KeyArray[1].key_num, 1);
			input_sync(mxt_fw30_data->input_dev); 
		}
		else{
			dbg_op("[KeyArray] The undefined Key..%d. T15 down is wrong key detected\n", quantum_msg[2]);
		}    
	}
	else if(quantum_msg[1] == 0x0){
		if(mPan_KeyArray[0].key_state==true){
			dbg_op("[KeyArray] MENU UP\n");
			mPan_KeyArray[0].key_state=false;
			input_report_key(mxt_fw30_data->input_dev, mPan_KeyArray[0].key_num, 0);
			input_sync(mxt_fw30_data->input_dev);
		}else if(mPan_KeyArray[1].key_state==true){
			dbg_op("[KeyArray] BACK UP\n");
			mPan_KeyArray[1].key_state=false;
			input_report_key(mxt_fw30_data->input_dev, mPan_KeyArray[1].key_num, 0);
			input_sync(mxt_fw30_data->input_dev);
		}else{
			dbg_op("[KeyArray] The Pressed Key is none...%d\n", quantum_msg[2]);
		}
	}
#endif
}   

uint8_t get_object_type(uint8_t *quantum_msg)
{
	uint8_t object_type=0;
	u8 report_id = quantum_msg[0];
  
	// search object type
	if (report_id >= mxt_fw30_data->T100_reportid_min && report_id <= mxt_fw30_data->T100_reportid_max) {
		object_type = TOUCH_MULTITOUCHSCREEN_T100;	
	}else if (report_id >= mxt_fw30_data->T6_reportid_min && report_id <= mxt_fw30_data->T6_reportid_max) {
		object_type = GEN_COMMANDPROCESSOR_T6;
	} else if (report_id >= mxt_fw30_data->T42_reportid_min	&& report_id <= mxt_fw30_data->T42_reportid_max) {
		object_type = PROCI_TOUCHSUPPRESSION_T42;	
	} else if (report_id >= mxt_fw30_data->T15_reportid_min	&& report_id <= mxt_fw30_data->T15_reportid_max) {
		object_type = TOUCH_KEYARRAY_T15;
	} else if (report_id >= mxt_fw30_data->T46_reportid_min	&& report_id <= mxt_fw30_data->T46_reportid_max) {
		object_type = SPT_CTECONFIG_T46;
	} else if (report_id >= mxt_fw30_data->T66_reportid_min && report_id <= mxt_fw30_data->T66_reportid_max) {
		object_type = SPT_GOLDENREFERENCES_T66;
//++ p11309 - 2013.10.21 for Palm Gesture
	} else if (report_id >= mxt_fw30_data->T69_reportid_min && report_id <= mxt_fw30_data->T69_reportid_max) {
		object_type = PROCI_PALMGESTUREPROCESSOR_T69;
//-- p11309
	} else if (report_id >= mxt_fw30_data->T72_reportid_min && report_id <= mxt_fw30_data->T72_reportid_max) {
		object_type = PROCG_NOISESUPPRESSION_T72;
	}else{
		object_type = report_id;		
    }
    
	return object_type;  
}

int get_message_process(uint8_t *quantum_msg)
{
	uint8_t ret_val = MESSAGE_READ_FAILED;
	uint8_t object_type=0;
	unsigned int touch_status = 0;
  
	/* Call the main application to handle the message. */
	object_type = get_object_type(quantum_msg);

	// parcing & processing Touch Message
	switch(object_type){

	case GEN_COMMANDPROCESSOR_T6 :
		get_message_T6(quantum_msg);
		break;

	case TOUCH_KEYARRAY_T15 :
		get_message_T15(quantum_msg);
		break;

	case SPT_CTECONFIG_T46 : 
		break;

	case SPT_GOLDENREFERENCES_T66 :
		get_message_T66(quantum_msg);
		break;

//++ p11309 - 2013.10.21 for Palm Gesture
	case PROCI_PALMGESTUREPROCESSOR_T69 :
		get_message_T69(quantum_msg);
		break;
//-- p11309

	case PROCG_NOISESUPPRESSION_T72 :
		get_message_T72(quantum_msg);
		break;

	case TOUCH_MULTITOUCHSCREEN_T100:
		ret_val = get_message_T100(quantum_msg, &touch_status);
		if(ret_val == MESSAGE_READ_FAILED) {
			dbg_op("get_message_T100 ret_val is failed\n");
			break;			
		}

		report_input(touch_status);
		break;

	default :
		dbg_cr(" get_object_type is not matched(Unused Message). object_type -> %d\n",object_type);

	}

	dbg_op("Obj= T%u raw data = %x, %x, %x, %x, %x, %x, %x, %x, %x, %x (%u, %u)\n", object_type,
			quantum_msg[0], quantum_msg[1], quantum_msg[2], quantum_msg[3], quantum_msg[4], 
			quantum_msg[5], quantum_msg[6], quantum_msg[7], quantum_msg[8], quantum_msg[9], 
			quantum_msg[2] | (quantum_msg[3] << 8), quantum_msg[4] | (quantum_msg[5] << 8));

	if(mxt_fw30_data->debug_enabled)
		mxt_dump_message(quantum_msg);

	return 0;
	
}

void  get_message(struct work_struct * p)
{
	u8 count=0;
	int i=0;
	uint8_t read_fail_cnt = 0;

	/* Get the lock */
	mutex_lock(&mxt_fw30_data->lock);

	if (driver_setup != DRIVER_SETUP_OK)
		goto fail_to_read_reg;

	// read T44_MESSAGE_CNT and First Message
	if(__mxt_read_reg(mxt_fw30_data->client, mxt_fw30_data->T44_address,mxt_fw30_data->T5_msg_size + 1, quantum_msg_total)){
		do{
			read_fail_cnt++;
			dbg_op_err(" read_mem failed cnt=%d.\n", read_fail_cnt);	

			if(read_fail_cnt>3)
			{
				dbg_cr("Hard Reset \n");
				read_fail_cnt=0;		  
				clear_event(TSC_CLEAR_ALL);	
				TSP_PowerOff();
				msleep(20);	  
				TSP_PowerOn();
				msleep(100); 
				goto fail_to_read_reg;
			}
			msleep(20);
		}while(__mxt_read_reg(mxt_fw30_data->client, mxt_fw30_data->T44_address,mxt_fw30_data->T5_msg_size + 1, quantum_msg_total));
	}

	// count is Message CNT
	count=quantum_msg_total[0];
	dbg_op("[TOUCH] count -> %d\n",count);

	if (!count) {
		dbg_op("Interrupt triggered but zero messages\n");
		goto fail_to_read_reg;
	} else if (count > info_block->info_id->num_declared_objects) {
		dbg_cr("T44 count exceeded max report id\n");
		count = info_block->info_id->num_declared_objects;
	}
	/* Process first message */
	get_message_process(quantum_msg_total+1);
	count--;
	/* Process remaining message */
	if(count){
		if(__mxt_read_reg(mxt_fw30_data->client, message_processor_address,mxt_fw30_data->T5_msg_size * count, quantum_msg_total)){
			do{
				read_fail_cnt++;
				dbg_op_err(" read_mem failed cnt=%d.\n", read_fail_cnt);  

				if(read_fail_cnt>3)
				{
					dbg_cr("Hard Reset \n");
					read_fail_cnt=0;      
					clear_event(TSC_CLEAR_ALL); 
					TSP_PowerOff();
					msleep(20);   
					TSP_PowerOn();
					msleep(100); 
					goto fail_to_read_reg;
	}                     
				msleep(20);
			}while(__mxt_read_reg(mxt_fw30_data->client, message_processor_address,max_message_length * count, quantum_msg_total));
	}

		for(i=0;i<count;i++){
			dbg_op("[TOUCH] i-> %d, count -> %d\n",i,count);
			get_message_process(quantum_msg_total+mxt_fw30_data->T5_msg_size*i);
	}
	}

fail_to_read_reg:
	enable_irq(mxt_fw30_data->client->irq);
	mutex_unlock(&mxt_fw30_data->lock);	
	return;

}

void report_input (int touch_status) {
	int i;
	int valid_input_count=0;

	for ( i= 0; i<MAX_NUM_FINGER; i++ )	{
		//printk("X : %d, Y: %d\n", fingerInfo[i].x, fingerInfo[i].y);

		if ( fingerInfo[i].status == -1 || (fingerInfo[i].mode == TSC_EVENT_NONE && fingerInfo[i].status == 0)) 
			continue;

		if(fingerInfo[i].mode == TSC_EVENT_NONE ){			// TOUCH_EVENT_PRESS (DOWN)
			//printk("[TSP] Finger[%d] Down(TSC_EVENT_WINDOW) XY(%d, %d)\n", i, fingerInfo[i].x, fingerInfo[i].y);  //p11223			

//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
			if ( fingerInfo[i].ignored_by_cover ) continue;
#endif
//-- p11309

			if(fingerInfo[i].y < SCREEN_RESOLUTION_SCREEN_Y){
				input_mt_slot(mxt_fw30_data->input_dev, fingerInfo[i].id);									// TOUCH_ID_SLOT
				input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TRACKING_ID, fingerInfo[i].id);			// TOUCH_ID
				input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);				// TOUCH_X
				input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);				// TOUCH_Y
				input_report_abs(mxt_fw30_data->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].area);			// TOUCH_SIZE
				input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].area);			// TOUCH_SIZE
				fingerInfo[i].mode = TSC_EVENT_WINDOW;
			}
#ifdef PAN_HAVE_TOUCH_KEY
			else {

				fingerInfo[i].mode = TSC_EVENT_NONE;
				if ( get_touch_key_pos(fingerInfo[i].x, fingerInfo[i].y) == 0 ){	
					input_report_key(mxt_fw30_data->input_dev, PAN_1ST_TOUCH_KEY_TYPE, 1);
					fingerInfo[i].mode = TSC_EVENT_1ST_KEY;
				}
				if ( get_touch_key_pos(fingerInfo[i].x, fingerInfo[i].y) == 1 ){
					input_report_key(mxt_fw30_data->input_dev, PAN_2ND_TOUCH_KEY_TYPE, 1);
					fingerInfo[i].mode = TSC_EVENT_2ND_KEY;
				}
				if ( get_touch_key_pos(fingerInfo[i].x, fingerInfo[i].y) == 2 ){
					input_report_key(mxt_fw30_data->input_dev, PAN_3RD_TOUCH_KEY_TYPE, 1);
					fingerInfo[i].mode = TSC_EVENT_3RD_KEY;
				}				
			}
#endif

			valid_input_count++;
		}
		else
		{
			// TOUCH_EVENT_RELEASE (UP)
			if (fingerInfo[i].status == TOUCH_EVENT_RELEASE && fingerInfo[i].mode == TSC_EVENT_WINDOW) { 		 
				dbg_op(" U:(%d, %d) (id:%d)\n", fingerInfo[i].x, fingerInfo[i].y, fingerInfo[i].id);
				input_mt_slot(mxt_fw30_data->input_dev, fingerInfo[i].id);						// TOUCH_ID_SLOT
				input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TRACKING_ID, -1);		// RELEASE TOUCH_ID
				fingerInfo[i].mode = TSC_EVENT_NONE;
				fingerInfo[i].status= -1;				
			}
#ifdef PAN_HAVE_TOUCH_KEY
			else if (fingerInfo[i].status == TOUCH_EVENT_RELEASE && fingerInfo[i].mode == TSC_EVENT_1ST_KEY) {
				input_report_key(mxt_fw30_data->input_dev, PAN_1ST_TOUCH_KEY_TYPE, 0);
				fingerInfo[i].mode = TSC_EVENT_NONE; fingerInfo[i].status= -1;				
			}
			else if (fingerInfo[i].status == TOUCH_EVENT_RELEASE && fingerInfo[i].mode == TSC_EVENT_2ND_KEY) {
				input_report_key(mxt_fw30_data->input_dev, PAN_2ND_TOUCH_KEY_TYPE, 0);
				fingerInfo[i].mode = TSC_EVENT_NONE; fingerInfo[i].status= -1;				
			}
			else if (fingerInfo[i].status == TOUCH_EVENT_RELEASE && fingerInfo[i].mode == TSC_EVENT_3RD_KEY) {
				input_report_key(mxt_fw30_data->input_dev, PAN_3RD_TOUCH_KEY_TYPE, 0);
				fingerInfo[i].mode = TSC_EVENT_NONE; fingerInfo[i].status= -1;				
			}
#endif
			// TOUCH_EVENT_MOVE
			else if(fingerInfo[i].status == TOUCH_EVENT_MOVE && fingerInfo[i].mode == TSC_EVENT_WINDOW)
			{

//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
				if ( fingerInfo[i].ignored_by_cover ) continue;
#endif
//-- p11309

				if(fingerInfo[i].y>(SCREEN_RESOLUTION_SCREEN_Y)) {
					fingerInfo[i].y=SCREEN_RESOLUTION_SCREEN_Y;
					dbg_op("Finger[%d] Move(TSC_EVENT_WINDOW) XY(%d, %d) - disabled\n", i, fingerInfo[i].x, fingerInfo[i].y);
				}
				else {
				dbg_op("Finger[%d] Move(TSC_EVENT_WINDOW) XY(%d, %d)\n", i, fingerInfo[i].x, fingerInfo[i].y);
					input_mt_slot(mxt_fw30_data->input_dev, fingerInfo[i].id);							// TOUCH_ID_SLOT
					input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);		// TOUCH_X
					input_report_abs(mxt_fw30_data->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);		// TOUCH_Y
					input_report_abs(mxt_fw30_data->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].area); // TOUCH_SIZE
					input_report_abs(mxt_fw30_data->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].area);			// TOUCH_SIZE
				}				

				fingerInfo[i].status= TOUCH_EVENT_PRESS;
				valid_input_count++;
			}
			// TOUCH_EVENT_PRESS 
			else if (fingerInfo[i].status == TOUCH_EVENT_PRESS)	{
				valid_input_count++;
			}
			else {
			}
		}
	}

	input_report_key(mxt_fw30_data->input_dev, BTN_TOUCH, !!valid_input_count);  // mirinae_ICS
	dbg_op(" touch event num => %d\n",valid_input_count);
	input_sync(mxt_fw30_data->input_dev);

#ifdef PAN_TOUCH_CAL_PMODE
	if( touch_status == TOUCH_EVENT_RELEASE ) {
		if(pan_pmode_resume_cal && !cal_correction_limit && !debugInfo.autocal_flag){
			dbg_cr("[PMODE] Protection Mode complete..\n");
			pan_pmode_resume_cal = false;

			//2014.3.27 P13106 Enable Touch Suppression
			if(touchsuppression_t42_config.ctrl != obj_touch_suppression_t42[mTouch_mode].ctrl){
  			touchsuppression_t42_config.ctrl = obj_touch_suppression_t42[mTouch_mode].ctrl;
        if (get_object_address(PROCI_TOUCHSUPPRESSION_T42, 0) != OBJECT_NOT_FOUND){
  		    if (write_touch_suppression_T42_config(touchsuppression_t42_config) != CFG_WRITE_OK)
  			    dbg_cr("T42 Configuration Fail!!! , Line %d \n", __LINE__);		
        }
      }  
//#if defined(CONFIG_MACH_MSM8974_EF56S) ||	defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) 
      // 2014.3.27 P13106 Enable T80 Retransmission.
      if(retransmissioncompensation_t80_config.ctrl != obj_retransmissioncompensation_t80[mTouch_mode].ctrl){
        retransmissioncompensation_t80_config.ctrl = obj_retransmissioncompensation_t80[mTouch_mode].ctrl;     
        if (get_object_address(PROCI_RETRANSMISSIONCOMPENSATION_T80, 0) != OBJECT_NOT_FOUND) {
      		if (write_retransmissioncompensation_T80_config(retransmissioncompensation_t80_config) != CFG_WRITE_OK)
      			dbg_cr("T80 Configuration Fail!!! , Line %d \n\r", __LINE__);
      	}
      }	
//#endif    	
    	// 2014.3.27 P13106 Disable Anti Touch Recalibration
    	acquisition_config.atchcalst = obj_acquisition_config_t8[mTouch_mode].atchcalst;
  		acquisition_config.atchcalsthr = obj_acquisition_config_t8[mTouch_mode].atchcalsthr;
  		acquisition_config.atchfrccalthr = obj_acquisition_config_t8[mTouch_mode].atchfrccalthr;
  		acquisition_config.atchfrccalratio = obj_acquisition_config_t8[mTouch_mode].atchfrccalratio;

  		if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK) {
  			dbg_cr("%s: write_acquisition_T8_config error\n", __func__);
  		}  	
		}

		valid_input_count=0;		
		for (i = 0 ; i < MAX_NUM_FINGER; ++i) { 
			if ( fingerInfo[i].status == -1 || (fingerInfo[i].mode == TSC_EVENT_NONE && fingerInfo[i].status == 0))
				continue;
			valid_input_count++;
		}
		// When all finger released (finger_cnt == 0)
		if (valid_input_count == 0 && cal_correction_limit > 0 && debugInfo.autocal_flag!=1) {
			cal_correction_limit--;
			/*
			2014.3.27 p13106
			dbg_touch("[PMODE] Antical is on till %d msec\n",
				PAN_PMODE_ANTICAL_ENABLE_TIME);
			acquisition_config.atchcalst = PAN_PMODE_ATCHCALST;
			acquisition_config.atchcalsthr = PAN_PMODE_ATCHCALSTHR;
			acquisition_config.atchfrccalthr = PAN_PMODE_ATCHFRCCALTHR;
			acquisition_config.atchfrccalratio = PAN_PMODE_ATCHFRCCALRATIO;
			if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK) {
				dbg_cr("\n[TSP][ERROR] line : %d\n", __LINE__);
			}
			mod_timer(&pan_pmode_antical_timer, jiffies + 
				msecs_to_jiffies(PAN_PMODE_ANTICAL_ENABLE_TIME));
			*/
		}
	}
#endif

}


/*------------------------------ I2C Driver block -----------------------------------*/
#define I2C_M_WR 0 /* for i2c */
#define I2C_MAX_SEND_LENGTH     300
int mxt_i2c_write(u16 reg, u8 *read_val, unsigned int len)
{
	struct i2c_msg wmsg;
	unsigned char data[I2C_MAX_SEND_LENGTH];
	int ret,i;

	address_pointer = reg;

	if(len+2 > I2C_MAX_SEND_LENGTH)	{
		dbg_cr(" %s() data length error\n", __FUNCTION__);
		return -ENODEV;
	}

	wmsg.addr = mxt_fw30_data->client->addr;
	wmsg.flags = I2C_M_WR;
	wmsg.len = len + 2;
	wmsg.buf = data;
	data[0] = reg & 0x00ff;
	data[1] = reg >> 8;

	for (i = 0; i < len; i++){
		data[i+2] = *(read_val+i);
	}

	ret = i2c_transfer(mxt_fw30_data->client->adapter, &wmsg, 1);

	return ret;
}

int boot_mxt_i2c_write(u16 reg, u8 *read_val, unsigned int len)
{
	struct i2c_msg wmsg;
	unsigned char data[I2C_MAX_SEND_LENGTH];
	int ret,i;

	if(len+2 > I2C_MAX_SEND_LENGTH) {
		dbg_cr(" %s() data length error\n", __FUNCTION__);
		return -ENODEV;
	}

	wmsg.addr = MXT_I2C_BOOT_ADDR;
	wmsg.flags = I2C_M_WR;
	wmsg.len = len;
	wmsg.buf = data;


	for (i = 0; i < len; i++) {
		data[i] = *(read_val+i);
	}

	ret = i2c_transfer(mxt_fw30_data->client->adapter, &wmsg, 1);

	return ret;
}


int mxt_i2c_read(u16 reg,unsigned char *rbuf, int buf_size)
{
	static unsigned char first_read=1;
	struct i2c_msg rmsg;
	int ret;
	unsigned char data[2];

	rmsg.addr = mxt_fw30_data->client->addr;

	if(first_read == 1)	{
		first_read = 0;
		address_pointer = reg+1;
	}

	if((address_pointer != reg) || (reg != message_processor_address))
	{
		address_pointer = reg;
		rmsg.flags = I2C_M_WR;
		rmsg.len = 2;
		rmsg.buf = data;
		data[0] = reg & 0x00ff;
		data[1] = reg >> 8;
		ret = i2c_transfer(mxt_fw30_data->client->adapter, &rmsg, 1);
	}

	rmsg.flags = I2C_M_RD;
	rmsg.len = buf_size;
	rmsg.buf = rbuf;
	ret = i2c_transfer(mxt_fw30_data->client->adapter, &rmsg, 1);

	return ret;
}

/*! \brief Maxtouch Memory read by I2C bus */
U8 read_mem(U16 start, U8 size, U8 *mem)
{
	int ret;
	U8 rc,read_mem_cnt=0;

	memset(mem,0xFF,size);
	ret = mxt_i2c_read(start,mem,size);
	if(ret < 0) {
	  do{
			read_mem_cnt++;
			dbg_cr("mxt_i2c_read fail. ret=%d, read_mem_cnt -> %d\n", ret,read_mem_cnt);
			if(read_mem_cnt>3)
			{
				dbg_cr("Hard Reset \n");
				read_mem_cnt=0;		  
				clear_event(TSC_CLEAR_ALL);	
				TSP_PowerOff();
				msleep(20);	  
				TSP_PowerOn();
				msleep(100); 
				ret=mxt_i2c_read(start,mem,size);
				break;
			}
			msleep(20);
		}while(mxt_i2c_read(start,mem,size)<0);
		if(ret<0){
		  dbg_cr("mxt_i2c_read fail after reset. ret=%d.\n", ret);
		rc = READ_MEM_FAILED;
		}else{
		  dbg_cr("mxt_i2c_read success after reset. ret=%d.\n", ret);
		  rc = READ_MEM_OK;
	}
	}
	else {
		rc = READ_MEM_OK;
	}

	return rc;
}

U8 boot_read_mem(U16 start, U8 size, U8 *mem)
{
	struct i2c_msg rmsg;
	int ret;
	rmsg.addr = MXT_I2C_BOOT_ADDR;
	rmsg.flags = I2C_M_RD;
	rmsg.len = size;
	rmsg.buf = mem;
	ret = i2c_transfer(mxt_fw30_data->client->adapter, &rmsg, 1);
	return ret;
}

U8 read_U16(U16 start, U16 *mem)
{
	U8 status;
	status = read_mem(start, 2, (U8 *) mem);
	return status;
}

U8 write_mem(U16 start, U8 size, U8 *mem)
{
	int ret;
	U8 rc,write_mem_cnt=0;

	ret = mxt_i2c_write(start,mem,size);
	if(ret < 0) {
		do{
			write_mem_cnt++;
			dbg_cr("mxt_i2c_write fail. ret=%d,write_mem_cnt -> %d\n", ret,write_mem_cnt);
			if(write_mem_cnt>3)
			{
				dbg_cr("Hard Reset \n");
				write_mem_cnt=0;		  
				clear_event(TSC_CLEAR_ALL);	
				TSP_PowerOff();
				msleep(20);	  
				TSP_PowerOn();
				msleep(100); 
				ret=mxt_i2c_write(start,mem,size);
				break;
			}
			msleep(20);
		}while(mxt_i2c_write(start,mem,size)<0);
		if(ret<0){
		  dbg_cr("mxt_i2c_write fail after reset. ret=%d.\n", ret);
		  rc = WRITE_MEM_FAILED;
		}else{
		  dbg_cr("mxt_i2c_write success after reset. ret=%d.\n", ret);
		  rc = WRITE_MEM_OK;
		}
	}
	else
		rc = WRITE_MEM_OK;

	return rc;
}

U8 boot_write_mem(U16 start, U16 size, U8 *mem)
{
	int ret;
	U8 rc;

	ret = boot_mxt_i2c_write(start,mem,size);
	if(ret < 0){
		dbg_cr("boot write mem fail: %d \n",ret);
		rc = WRITE_MEM_FAILED;
	}
	else {
		rc = WRITE_MEM_OK;
	}

	return rc;
}

irqreturn_t mxt_irq_handler(int irq, void *dev_id)
{	
	disable_irq_nosync(mxt_fw30_data->client->irq);
	if(!queue_work(mxt_fw30_wq, &mxt_fw30_data->work)){			// p13106 
		enable_irq(mxt_fw30_data->client->irq);
		dbg_cr(" mxt_irq_handler queue_work failed\n");
	}
	return IRQ_HANDLED;
}

static int mxt_remove(struct i2c_client *client)
{
	dbg_func_in();

	input_mt_destroy_slots(mxt_fw30_data->input_dev); // PROTOCOL TYPE B
	if(mxt_fw30_data->client->irq)
	{
		free_irq(mxt_fw30_data->client->irq, mxt_fw30_data);
	}
	mutex_destroy(&mxt_fw30_data->lock);	
#ifdef SKY_PROCESS_CMD_KEY
	misc_deregister(&touch_event);
#endif	//SKY_PROCESS_CMD_KEY
#ifdef TOUCH_IO
	misc_deregister(&touch_io);
#endif //TOUCH_IO

	input_unregister_device(mxt_fw30_data->input_dev);
	input_free_device(mxt_fw30_data->input_dev);
	device_destroy(touch_atmel_class, 0);
	class_destroy(touch_atmel_class);
	if (mxt_fw30_wq)
		destroy_workqueue(mxt_fw30_wq);
	kfree(mxt_fw30_data);        

	TSP_PowerOff();
	dbg_func_out();
	return 0;
}

static int mxt_suspend(struct i2c_client *client, pm_message_t mesg)
{	
	dbg_op("suspend start\n");

#ifdef PAN_TOUCH_CAL_PMODE 
	dbg_cr("[PMODE] %s: Suspend, remove pmode timer\n", __func__);
	cancel_work_sync(&pan_pmode_antical_wq);
	cancel_work_sync(&pan_pmode_autocal_wq);
//2014.3.27 p13106
	//del_timer(&pan_pmode_antical_timer);
	//del_timer(&pan_pmode_autocal_timer);	
#endif

	if(mxt_fw30_data->state == SUSMODE){
		return 0;
	}

	disable_irq(mxt_fw30_data->client->irq);
	mxt_fw30_data->state = SUSMODE;
	mxt_Power_Sleep();
	clear_event(TSC_CLEAR_ALL);

//++ p11309 2012.11.02 for remove mt slots for ghost touch
	input_mt_destroy_slots(mxt_fw30_data->input_dev); // PROTOCOL_B
//-- p11309

#ifdef PAN_T15_KEYARRAY_ENABLE
	if(mPan_KeyArray[0].key_state) mPan_KeyArray[0].key_state=false;
	if(mPan_KeyArray[1].key_state) mPan_KeyArray[1].key_state=false;
#endif

	dbg_op(" suspend complete\n");
	return 0;
}

static int mxt_resume(struct i2c_client *client)
{	
	dbg_op("resume start\n");

	if(mxt_fw30_data->state == APPMODE){
		return 0;
	}

//++ p11309 2012.11.02 for remove mt slots for ghost touch
	input_mt_init_slots(mxt_fw30_data->input_dev, MAX_NUM_FINGER); // PROTOCOL_B
//-- p11309

	touch_data_init();

	power_config.idleacqint  = obj_power_config_t7[mTouch_mode].idleacqint;
	power_config.actvacqint  = obj_power_config_t7[mTouch_mode].actvacqint;
	power_config.actv2idleto = obj_power_config_t7[mTouch_mode].actv2idleto;

	if (write_power_T7_config(power_config) != CFG_WRITE_OK)
		dbg_cr("%s: T7_POWERCONFIG Configuration Fail!!!\n", __func__);

	// touch ic calibration.
	calibrate_chip();

//++ p11309 - 2013.11.22 for calibration Protection mode
#ifdef PAN_TOUCH_CAL_PMODE  
  pan_touch_protection_mode();
	
	pan_pmode_resume_cal=true;
	cal_correction_limit = 5;
	debugInfo.calibration_cnt=0;
	debugInfo.autocal_flag=1;

#endif
//-- p11309

	enable_irq(mxt_fw30_data->client->irq);
	mxt_fw30_data->state = APPMODE;


	dbg_op("resume complete\n");

	//2014.3.24 check hall ic state
  dbg_cr("mTouch_cover_status_hallic(open:0, close :1) -> %d, hall_ic gpio(open:1, close :0) -> %d\n",mTouch_cover_status_hallic,gpio_get_value(9));
	
	return 0;
}

/* I2C driver probe function */
static int __devinit mxt_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc;

#if defined (OFFLINE_CHARGER_TOUCH_DISABEL) 
  oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;

  smem_id_vendor1_ptr =  (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,sizeof(oem_pm_smem_vendor1_data_type));
#if defined (OFFLINE_CHARGER_TOUCH_DISABEL)  
  if(smem_id_vendor1_ptr->power_on_mode == 0){
    dbg_cr(" OFFLINE_CHARGER is enabled. And Touch Driver IRQ is disabled\n");
    TSP_PowerOff();
    return 0;
  }
#endif
#endif
	mxt_fw30_data = kzalloc(sizeof(struct mxt_fw30_data_t), GFP_KERNEL);
	if (!mxt_fw30_data){
		dbg_cr("mxt_fw30_data is not NULL.\n");
		return -ENOMEM;
	}
#ifdef ITO_TYPE_CHECK		//P13106
//++ p11309 - 2013.07.25 Get Model Color 
	mxt_fw30_data->ito_color = 0;	// 0 is not read.
//-- p11309 
#endif
	mxt_fw30_data->client = client;
	mxt_fw30_data->state = INIT;
	i2c_set_clientdata(client, mxt_fw30_data);

	mxt_fw30_wq = create_singlethread_workqueue("mxt_fw30_wq");
	if (!mxt_fw30_wq){
		dbg_cr("create_singlethread_workqueue(mxt_fw30_wq) is error.\n");
		return -ENOMEM;
	}

	if(!touch_atmel_class)
		touch_atmel_class=class_create(THIS_MODULE, "touch_atmel");

	dbg_cr("+-----------------------------------------+\n");
	dbg_cr("|  Atmel Max Touch FW30 Probe!            |\n");
	dbg_cr("+-----------------------------------------+\n");

	INIT_WORK(&mxt_fw30_data->work, get_message );
	
	mxt_fw30_data->input_dev = input_allocate_device();
	if (mxt_fw30_data->input_dev == NULL)
	{
		rc = -ENOMEM;
		dbg_cr("mxt_probe: Failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
  mxt_fw30_data->input_dev->name="atmel_mxt_540s";
	set_bit(EV_SYN, mxt_fw30_data->input_dev->evbit);
	set_bit(EV_KEY, mxt_fw30_data->input_dev->evbit);
	set_bit(EV_ABS, mxt_fw30_data->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, mxt_fw30_data->input_dev->propbit);
	set_bit(BTN_TOUCH, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_MENU, mxt_fw30_data->input_dev->keybit);	
	set_bit(KEY_HOME, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_HOMEPAGE, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_BACK, mxt_fw30_data->input_dev->keybit);	
#ifdef SKY_PROCESS_CMD_KEY
	set_bit(KEY_SEARCH, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_HOMEPAGE, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_0, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_1, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_2, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_3, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_4, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_5, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_6, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_7, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_8, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_9, mxt_fw30_data->input_dev->keybit);
	set_bit(0xe3, mxt_fw30_data->input_dev->keybit); /* '*' */
	set_bit(0xe4, mxt_fw30_data->input_dev->keybit); /* '#' */
	set_bit(0xe5, mxt_fw30_data->input_dev->keybit); /* 'KEY_END' p13106 120105 */
	set_bit(KEY_POWER, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_LEFTSHIFT, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_RIGHTSHIFT, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_LEFT, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_RIGHT, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_UP, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_DOWN, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_ENTER, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_SEND, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_END, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_F1, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_F2, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_F3, mxt_fw30_data->input_dev->keybit);				
	set_bit(KEY_F4, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_VOLUMEUP, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_CLEAR, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_CAMERA, mxt_fw30_data->input_dev->keybit);
	set_bit(KEY_VT_CALL, mxt_fw30_data->input_dev->keybit);       // P13106 VT_CALL
	
#endif // SKY_PROCESS_CMD_KEY

	input_mt_init_slots(mxt_fw30_data->input_dev, MAX_NUM_FINGER); // PROTOCOL_B
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_X, 0, SCREEN_RESOLUTION_X, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_Y, 0, SCREEN_RESOLUTION_SCREEN_Y, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_TOOL_WIDTH, 0, 15, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_MT_POSITION_X, 0, SCREEN_RESOLUTION_X, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_MT_POSITION_Y, 0, SCREEN_RESOLUTION_SCREEN_Y, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(mxt_fw30_data->input_dev, ABS_MT_WIDTH_MAJOR, 0, MXT_540S_MAX_CHANNEL_NUM, 0, 0);

	rc = input_register_device(mxt_fw30_data->input_dev);
	if (rc){
		dbg_cr("mxt_probe: Unable to register %s input device\n", mxt_fw30_data->input_dev->name);
		goto err_input_register_device_failed;
	}
	
#ifdef TOUCH_IO  
	rc = misc_register(&touch_io);
	if (rc){
		dbg_cr("touch_io can''t register misc device\n");
	}
#endif //TOUCH_IO

	rc = sysfs_create_group(&client->dev.kobj, &mxt_attr_group);
	if (rc) {
		dbg_cr(" mxt_attr_group creating sysfs group is failed. rc-> %d \n",rc);		
	}

	sysfs_bin_attr_init(&mxt_fw30_data->mem_access_attr);
	mxt_fw30_data->mem_access_attr.attr.name = "mem_access";
	mxt_fw30_data->mem_access_attr.attr.mode = S_IRUGO | S_IWUSR;
	mxt_fw30_data->mem_access_attr.read = mxt_mem_access_read;
	mxt_fw30_data->mem_access_attr.write = mxt_mem_access_write;
	mxt_fw30_data->mem_access_attr.size = mxt_fw30_data->mem_size;
	if (sysfs_create_bin_file(&client->dev.kobj,&mxt_fw30_data->mem_access_attr) < 0) {
		dbg_cr(" Failed to create %s\n",mxt_fw30_data->mem_access_attr.attr.name);
	}

#ifdef ITO_TYPE_CHECK		//p13106 TOUCH_ID
	read_touch_id();
#endif //TOUCH_ID

	mutex_init(&mxt_fw30_data->lock);	
#ifdef MXT_FIRMUP_ENABLE
	MXT_reprogram();
#else
	quantum_touch_probe();
#endif	

#ifdef PAN_T15_KEYARRAY_ENABLE
  mPan_KeyArray[0].key_state = false;
  mPan_KeyArray[0].key_num = KEY_MENU;
  mPan_KeyArray[1].key_state = false;
  mPan_KeyArray[1].key_num = KEY_BACK;
#endif

    //p16619 keep this code when deleting switch device code
  wake_lock_init(&touch_pen_wakelock, WAKE_LOCK_SUSPEND, "touch_pen");
#ifdef SKY_PROCESS_CMD_KEY
	rc = misc_register(&touch_event);
	if (rc) {
		dbg_cr("touch_event can''t register misc device\n");
	}
#endif //SKY_PROCESS_CMD_KEY

//++ p11309 - 2013.07.30 for direct set smart cover - add 2013.08.26 check hallic vs ui state.
#ifdef PAN_SUPPORT_SMART_COVER
	init_timer(&pan_hallic_ui_sync_timer);
	pan_hallic_ui_sync_timer.function = pan_hallic_ui_sync_timer_func;
	pan_hallic_ui_sync_timer.data = 0;
	pan_hallic_ui_sync_timer.expires = 500;
#endif
  register_notify_func(EMERGENCY_MODE,"smart_cover",set_smart_cover);   // smart cover callback function
#ifdef PAN_TOUCH_PEN_DETECT
#if defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) 
  mxt_fw30_data->pan_touch_pen_state=!gpio_get_value(PAN_TOUCH_PEN_GPIO);
#elif defined(CONFIG_MACH_MSM8974_EF65S)
  mxt_fw30_data->pan_touch_pen_state=gpio_get_value(PAN_TOUCH_PEN_GPIO);
#endif  
  register_notify_func(NORMAL_MODE,"touch_pen_detection",set_pan_touch_state);
#endif


//++ p11309 - 2013.11.22 for calibration Protection mode
#ifdef PAN_TOUCH_CAL_PMODE
	
	debugInfo.calibration_cnt = 0;
	debugInfo.autocal_flag = 0; 
	pan_pmode_resume_cal = false;

	pan_pmode_work_queue = create_singlethread_workqueue("pan_protection_timer_work_queue");
	if (!pan_pmode_work_queue){
		dbg_cr("create_singlethread_workqueue(pan_protection_timer_work_queue) error.\n");
		return -ENOMEM;
	}
  //2014.3.27
	//INIT_WORK(&pan_pmode_antical_wq, pan_pmode_antical_wq_func);
	INIT_WORK(&pan_pmode_autocal_wq, pan_pmode_autocal_wq_func);
/*
	init_timer(&pan_pmode_antical_timer);
	pan_pmode_antical_timer.function = pan_pmode_antical_timer_func;
	pan_pmode_antical_timer.data = 0;
*/
	init_timer(&pan_pmode_autocal_timer);
	pan_pmode_autocal_timer.function = pan_pmode_autocal_timer_func;
	pan_pmode_autocal_timer.data = 0;
	
#endif
//-- p11309
  mxt_fw30_data->client->irq = IRQ_TOUCH_INT;
	rc = request_irq(mxt_fw30_data->client->irq, mxt_irq_handler, IRQF_TRIGGER_LOW, "mxt_fw30-irq", mxt_fw30_data);
	if (rc){		
		dbg_cr("%s request_irq failed: %d\n", __func__, rc);
	}	

	mxt_fw30_data->state = APPMODE;
	dbg_hw("Atmel Max Touch Probe Complete!\n");
	touch_probe_state=1;
	return 0;

err_input_register_device_failed:
	input_free_device(mxt_fw30_data->input_dev);

err_input_dev_alloc_failed:
	kfree(mxt_fw30_data);
	dbg_cr("mxt_probe failed: rc=%d\n", rc);
	return rc;
}

void  mxt_front_test_init(void)
{
	disable_irq(mxt_fw30_data->client->irq);

#ifdef PAN_TOUCH_CAL_PMODE 
//2014.3.27 p13106
//	cancel_work_sync(&pan_pmode_antical_wq);
	cancel_work_sync(&pan_pmode_autocal_wq);
//	del_timer(&pan_pmode_antical_timer);
	del_timer(&pan_pmode_autocal_timer);	
#endif

	TSP_PowerOff();
	msleep(20);	  
	TSP_PowerOn();
	msleep(100); 	
	quantum_touch_probe();

	enable_irq(mxt_fw30_data->client->irq);
	return ;
}

#ifdef MXT_FIRMUP_ENABLE

uint8_t boot_unlock(void)
{
	int ret;
	unsigned char data[2];
	uint8_t rc;
	//   read_buf = (char *)kmalloc(size, GFP_KERNEL | GFP_ATOMIC);
	data[0] = 0xDC;
	data[1] = 0xAA;

	ret = boot_mxt_i2c_write(0,data,2);
	if(ret < 0) {
		dbg_cr("%s : i2c write failed\n",__func__);
		rc = WRITE_MEM_FAILED;
	}
	else{
		rc = WRITE_MEM_OK;
	}
	return rc;
}

uint8_t MXT_Boot(bool withReset)
{
	unsigned char	boot_status;
	unsigned char	retry_cnt, retry_cnt_max;
	unsigned long int	character_position = 0;
	unsigned int	frame_size = 0;
	unsigned int	next_frame = 0;
	unsigned int	crc_error_count = 0;
	unsigned int	size1,size2;
	uint8_t			data = 0xA5;
	uint8_t			reset_result = 0;
	unsigned char	*firmware_data;
#ifdef FEATURE_mXT540S_V_3_0
	firmware_data = MXT_FW30_E_firmware;
#else
	firmware_data = MXT_FW21_E_firmware;
#endif
	if(withReset){
		retry_cnt_max = 10;
		reset_result = write_mem(command_processor_address + RESET_OFFSET, 1, &data);

		if(reset_result != WRITE_MEM_OK){
			for(retry_cnt =0; retry_cnt < 3; retry_cnt++){
				msleep(100);
				reset_result = write_mem(command_processor_address + RESET_OFFSET, 1, &data);
				if(reset_result == WRITE_MEM_OK){
					dbg_firmw("write_mem(RESET_OFFSET) : success.\n");
					break;
				}
			}			
		}
		else{
			dbg_firmw("write_mem(RESET_OFFSET) : fail.\n");
		}
		msleep(100);
	}
	else{
		retry_cnt_max = 30;
	}	

	for(retry_cnt = 0; retry_cnt < retry_cnt_max; retry_cnt++){
		if(boot_read_mem(0,1,&boot_status) == READ_MEM_OK){
			retry_cnt = 0;

			if((boot_status & MXT_WAITING_BOOTLOAD_COMMAND) == MXT_WAITING_BOOTLOAD_COMMAND){
				if(boot_unlock() == WRITE_MEM_OK){
					msleep(10);				
				}				
			}
			else if((boot_status & 0xC0) == MXT_WAITING_FRAME_DATA){
				/* Add 2 to frame size, as the CRC bytes are not included */
				size1 =  *(firmware_data+character_position);
				size2 =  *(firmware_data+character_position+1)+2;
				frame_size = (size1<<8) + size2;

				dbg_firmw("Frame size:%d\n", frame_size);
				dbg_firmw("Firmware pos:%d\n", (int)character_position);
				/* Exit if frame data size is zero */
				if( 0 == (int)frame_size ){
					dbg_firmw(" 0 == frame_size\n");
					return 1;
				}
				next_frame = 1;
				mxt_fw30_data->state = BOOTLOADER;
				boot_write_mem(0,frame_size, (firmware_data +character_position));
				msleep(10);
				mxt_fw30_data->state = APPMODE;
			}
			else if(boot_status == MXT_FRAME_CRC_CHECK){				
			}
			else if(boot_status == MXT_FRAME_CRC_PASS){
				if( next_frame == 1){					
					character_position += frame_size;
					next_frame = 0;
				}				
			}
			else if(boot_status  == MXT_FRAME_CRC_FAIL){
				dbg_firmw("CRC Fail\n");
				crc_error_count++;
			}
			if(crc_error_count > 10){
				return MXT_FRAME_CRC_FAIL;
			}
		}
	}

	return (0);
}

/* mxt chipset version check */
void MXT_reprogram(void)
{
	uint8_t family_id=0, version=0, build=0;
	uint8_t status = 0;
	unsigned char rxdata=0;		

	//	if TSP is in bootloader, force to download - p11309
	//  boot_read_mem have 0x24 i2c address.
	if(boot_read_mem(0,1,&rxdata) == READ_MEM_OK)
	{
		dbg_firmw(" Bootloader - Download Start\n");
		if(MXT_Boot(0)) {
			TSP_reset_pin_shake(); 
			quantum_touch_probe();
			TSP_reset_pin_shake();
		}
		quantum_touch_probe();	
		TSP_reset_pin_shake();	
		dbg_firmw(" Download End\n");
	}	

	status = read_mem(0, 1, (void *) &family_id);
	if (status != READ_MEM_OK) dbg_cr("[+++ Atmel Max Touch] family id read ERROR!");
	dbg_cr("family_id = 0x%X (mXT540S is 0x%X)\n",family_id, MXT_CURRENT_FAMILY_ID);

	status = read_mem(2, 1, (void *) &version);
	if (status != READ_MEM_OK) dbg_cr("[+++ Atmel Max Touch] version read ERROR!");
	dbg_cr("version = 0x%X (current is 0x%X)\n",version, MXT_CURRENT_FW_VERSION);

	status = read_mem(3, 1, (void *) &build);
	if (status != READ_MEM_OK) dbg_cr("[+++ Atmel Max Touch] family build id read ERROR!");
	dbg_cr("familybuild_id = 0x%X (current is 0x%X)\n", build, MXT_CURRENT_FW_BUILD);	

	if ( family_id != MXT_CURRENT_FAMILY_ID ) {
		dbg_cr(" Not Supported Touch IC!!\n");
		return;
	}	

	if(version != MXT_CURRENT_FW_VERSION){ 
		quantum_touch_probe();
		dbg_cr(" Firmware Download mode - Download Start\n");

		if(MXT_Boot(1)) {
			TSP_reset_pin_shake();
			quantum_touch_probe();
//++ p11309 - 2013.09.06 for Fix after Download.
		//	TSP_reset_pin_shake();
		}		
		TSP_reset_pin_shake();
		msleep(100);
//-- p11309
		dbg_cr(" Firmware Download mode Download End\n");
//++ p13106 -2014.3.17 for FW Download and reset
		quantum_touch_probe();
    reset_chip();
    return;
//-- p13106  
	}
	quantum_touch_probe();
}
#endif

//++ p11309 - 2013.11.22 for calibration Protection mode
#ifdef PAN_TOUCH_CAL_PMODE
//2014.3.27 p13106
/*
static void pan_pmode_antical_timer_func(unsigned long data)
{
	queue_work(pan_pmode_work_queue, &pan_pmode_antical_wq);
}
*/
static void pan_pmode_autocal_timer_func(unsigned long data)
{
	queue_work(pan_pmode_work_queue, &pan_pmode_autocal_wq);
}

void pan_pmode_autocal_wq_func(struct work_struct * p) 
{
	
	dbg_cr("[PMODE] Autocal is disabled\n");
	acquisition_config.tchautocal = obj_acquisition_config_t8[mTouch_mode].tchautocal;
	debugInfo.calibration_cnt=0;
	debugInfo.autocal_flag=0;

	if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK){
		dbg_cr("[TOUCH] Configuration Fail!!! , Line %d \n", __LINE__);
	}	
}
/*
void pan_pmode_antical_wq_func(struct work_struct * p) 
{
	
	dbg_cr("[PMODE] Antical is disabled\n");
	
	acquisition_config.atchcalst = obj_acquisition_config_t8[mTouch_mode].atchcalst;
	acquisition_config.atchcalsthr = obj_acquisition_config_t8[mTouch_mode].atchcalsthr;
	acquisition_config.atchfrccalthr = obj_acquisition_config_t8[mTouch_mode].atchfrccalthr;
	acquisition_config.atchfrccalratio = obj_acquisition_config_t8[mTouch_mode].atchfrccalratio;

	if (write_acquisition_T8_config(acquisition_config) != CFG_WRITE_OK)
	{
		dbg_cr("[TSP] Configuration Fail!!! , Line %d \n", __LINE__);
	}
}
*/
#endif
//-- p11309

#ifdef TOUCH_MONITOR
void cbInit(CircularBuffer *cb, int size) {
	cb->size  = size + 1; /* include empty elem */
	cb->start = 0;
	cb->end   = 0;
	cb->elems = (char*)kmalloc(cb->size * sizeof(char), GFP_KERNEL | GFP_ATOMIC);
}

void cbFree(CircularBuffer *cb) {
	kfree(cb->elems); /* OK if null */ 
}

int cbIsFull(CircularBuffer *cb) {
	return (cb->end + 1) % cb->size == cb->start; 
}

int cbIsEmpty(CircularBuffer *cb) {
	return cb->end == cb->start; 
}

/* Write an element, overwriting oldest element if buffer is full. App can
 *    choose to avoid the overwrite by checking cbIsFull(). */
void cbWrite(CircularBuffer *cb, char *elem) {
	cb->elems[cb->end] = *elem;
	cb->end = (cb->end + 1) % cb->size;
	if (cb->end == cb->start)
		cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
}

/* Read oldest element. App must ensure !cbIsEmpty() first. */
void cbRead(CircularBuffer *cb, char *elem) {
	*elem = cb->elems[cb->start];
	cb->start = (cb->start + 1) % cb->size;
}

int read_log(char *page, char **start, off_t off, int count, int *eof, void *data_unused) {
	char *buf;
	char elem = {0};
	buf = page;

	spin_lock(&cb_spinlock);
	while (!cbIsEmpty(&cb)) {
		cbRead(&cb, &elem);
		buf += sprintf(buf, &elem);
	}
	spin_unlock(&cb_spinlock);
	*eof = 1;
	return buf - page;
}

int read_touch_info(char *page, char **start, off_t off, int count, int *eof, void *data_unused) {
	char *buf;
	buf = page;
	buf += sprintf(buf, "Vendor: \t%s\n", touch_info_vendor);
	buf += sprintf(buf, "Chipset: \t%s\n", touch_info_chipset);

	*eof = 1;
	return buf - page;
}


void printp(const char *fmt, ...) {
	int count = 0;
	int i;
	va_list args;
	spin_lock(&cb_spinlock);
	va_start(args, fmt);
	count += vsnprintf(printproc_buf, 1024, fmt, args);
	for (i = 0; i<count; i++) {
		cbWrite(&cb, &printproc_buf[i]);
	}
	va_end(args);
	spin_unlock(&cb_spinlock);
}

void init_proc(void) { 
	int testBufferSize = 1024;

	struct proc_dir_entry *touch_log_ent;
	struct proc_dir_entry *touch_info_ent;
	
	touch_log_ent = create_proc_entry("touchlog", S_IFREG|S_IRUGO, 0); 
	touch_log_ent->read_proc = read_log;
	
	touch_info_ent = create_proc_entry("touchinfo", S_IFREG|S_IRUGO, 0); 
	touch_info_ent->read_proc = read_touch_info;

	spin_lock_init(&cb_spinlock);
	cbInit(&cb, testBufferSize);
}

void remove_proc(void) {
	remove_proc_entry("touchlog", 0);
	remove_proc_entry("touchinfo", 0);
	cbFree(&cb);
}
#endif

int __init mxt_init(void)
{
	int rc;
	rc = TSP_PowerOn();
	if(rc<0){
		dbg_cr(" init_hw_setting failed. (rc=%d)\n", rc);
		return rc;
	}
	dbg_hw("i2c_add_driver\n");
	rc = i2c_add_driver(&mxt_fw30_driver);
	if(rc){
	  dbg_cr(" i2c_add_driver is failed. rc -> %d\n",rc);
	}	
#ifdef TOUCH_MONITOR
  init_proc();
#endif
	dbg_func_out();
	return rc;
}

void __exit mxt_exit(void)
{
  
	dbg_func_in();
#ifdef TOUCH_MONITOR
  remove_proc();
#endif
	i2c_del_driver(&mxt_fw30_driver);
	dbg_func_out();
    //p16619 keep this code when deleting switch device code
	wake_lock_destroy(&touch_pen_wakelock);
	return;
}                                 

late_initcall(mxt_init);
module_exit(mxt_exit);

MODULE_DESCRIPTION("ATMEL mxt_fw30 Touchscreen Driver");
MODULE_LICENSE("GPL");

