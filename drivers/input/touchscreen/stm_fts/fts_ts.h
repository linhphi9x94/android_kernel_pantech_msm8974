#ifndef _LINUX_FTS_TS_H_
#define _LINUX_FTS_TS_H_

#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/i2c/fts.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#ifdef CONFIG_POWERSUSPEND
#include <linux/powersuspend.h>
#endif
#ifdef CONFIG_SEC_DEBUG_TSP_LOG
#include <mach/sec_debug.h>
#endif

#include <linux/i2c/fts.h>

#define tsp_debug_dbg(mode, dev, fmt, args...)	printk(fmt, ##args)
#define tsp_debug_info(mode, dev, fmt, args...)	printk(fmt, ##args)
#define tsp_debug_err(mode, dev, fmt, args...)	printk(fmt, ##args)

#define dbg_cr(fmt, args...)      printk("[+++ TOUCH] " fmt, ##args)
#define dbg_i2c(fmt,args...)      printk("[+++ TOUCH] " fmt,##args)
#define dbg_op_err(fmt,args...)   printk("[+++ TOUCH] " fmt,##args)
#define dbg_op(fmt,args...)       printk("[+++ TOUCH] " fmt,##args)
#define dbg_ioctl(fmt,args...)    printk("[+++ TOUCH] " fmt,##args) 
#define dbg_firmw(fmt,args...)    printk("[+++ TOUCH] " fmt,##args)
#define dbg_func_in()			    printk("[+++ TOUCH] %s Func In\n", __func__)
#define dbg_func_out()			    printk("[+++ TOUCH] %s Func Out\n", __func__)
#define dbg_hw(fmt,args...)       printk("[+++ TOUCH] " fmt,##args)
#define dbg_config(fmt,args...)   printk("[+++ TOUCH] " fmt,##args)
#define dbg_touch(fmt,args...)    printk("[+++ TOUCH] " fmt,##args)

//++ p11309 - 2013.12.22 for disabled by wcjeong
#define PAN_IRQ_WORKQUEUE

#define TSP_INIT_COMPLETE

//#define USE_OPEN_CLOSE

//#ifdef USE_OPEN_DWORK
//#define TOUCH_OPEN_DWORK_TIME 10

#define FIRMWARE_IC					"fts_ic"

#define FTS_MAX_FW_PATH	64

#define FTS_TS_DRV_NAME			"stmicro_fts_ts"
#define FTS_TS_DRV_VERSION		"0132"

#define STM_DEVICE_NAME	"STM"

#define FTS_ID0							0x39
#define FTS_ID1							0x80

#define FTS_FIFO_MAX					32
#define FTS_EVENT_SIZE				8

#define PRESSURE_MIN					0
#define PRESSURE_MAX				127
#define P70_PATCH_ADDR_START	0x00420000
#define FINGER_MAX						5
#define AREA_MIN							PRESSURE_MIN
#define AREA_MAX						PRESSURE_MAX

#define EVENTID_NO_EVENT					0x00
#define EVENTID_ENTER_POINTER				0x03
#define EVENTID_LEAVE_POINTER				0x04
#define EVENTID_MOTION_POINTER				0x05
#define EVENTID_HOVER_ENTER_POINTER			0x07
#define EVENTID_HOVER_LEAVE_POINTER			0x08
#define EVENTID_HOVER_MOTION_POINTER		0x09
#define EVENTID_PROXIMITY_IN				0x0B
#define EVENTID_PROXIMITY_OUT				0x0C
#define EVENTID_MSKEY						0x0E

#define EVENTID_ERROR						0x0F
#define EVENTID_CONTROLLER_READY			0x10
#define EVENTID_SLEEPOUT_CONTROLLER_READY	0x11
#define EVENTID_RESULT_READ_REGISTER        0x12
#define EVENTID_STATUS_EVENT            	0x16
#define EVENTID_INTERNAL_RELEASE_INFO       0x19
#define EVENTID_EXTERNAL_RELEASE_INFO       0x1A

#define EVENTID_GESTURE                     0x20

#define INT_ENABLE						0x41
#define INT_DISABLE						0x00

#define READ_STATUS					0x84
#define READ_ONE_EVENT				0x85
#define READ_ALL_EVENT				0x86

#define SLEEPIN							0x90
#define SLEEPOUT						0x91
#define SENSEOFF						0x92
#define SENSEON							0x93
#define SENSEON_SLOW					0x9C


#define FTS_CMD_HOVER_OFF           0x94
#define FTS_CMD_HOVER_ON            0x95

#if defined(CONFIG_SEC_S_PROJECT)
#define FTS_CMD_FAST_SCAN           0x98
#define FTS_CMD_SLOW_SCAN           0x99

#define FTS_CMD_FLIPCOVER_OFF		0x9C
#define FTS_CMD_FLIPCOVER_ON		0x9D
#define FTS_RETRY_COUNT		10

#else
#define FTS_CMD_FLIPCOVER_OFF		0x96
#define FTS_CMD_FLIPCOVER_ON		0x97
#define FTS_RETRY_COUNT		30

#endif

#define FTS_CMD_KEY_SENSE_ON		0x9B


#define FTS_CMD_SET_FAST_GLOVE_MODE	0x9D

#define FTS_CMD_MSHOVER_OFF         0x9E
#define FTS_CMD_MSHOVER_ON          0x9F
#define FTS_CMD_SET_NOR_GLOVE_MODE	0x9F

#define FLUSHBUFFER					0xA1
#define FORCECALIBRATION			0xA2
#define CX_TUNNING					0xA3
#define SELF_AUTO_TUNE				0xA4
#define KEY_CX_TUNNING				0x96

#define FTS_CMD_CHARGER_PLUGGED     0xA8
#define FTS_CMD_CHARGER_UNPLUGGED	0xAB

#define FTS_CMD_RELEASEINFO     0xAA
#define FTS_CMD_STYLUS_OFF          0xAB
#define FTS_CMD_STYLUS_ON           0xAC

#define FTS_CMD_WRITE_PRAM          0xF0
#define FTS_CMD_BURN_PROG_FLASH     0xF2
#define FTS_CMD_ERASE_PROG_FLASH    0xF3
#define FTS_CMD_READ_FLASH_STAT     0xF4
#define FTS_CMD_UNLOCK_FLASH        0xF7
#define FTS_CMD_SAVE_FWCONFIG       0xFB
#define FTS_CMD_SAVE_CX_TUNING      0xFC

#define TSP_BUF_SIZE 1024
#define CMD_STR_LEN 32
#define CMD_PARAM_NUM 8

#define PAN_DATA_INIT -1
#define RAW_MAX	3750

#define FTS_TX_LENGTH		17
#define FTS_RX_LENGTH		29
#define READ_CHUNK_SIZE	2 //			(2 * 1024) - 16
#define FTS_MIN_RAWDATA 2250
#define FTS_MAX_RAWDATA 3750
#define FTS_PANTECH_PRODUCT_LINE
#define PAN_TOUCH_MODE_FINGER 0
#define PAN_TOUCH_MODE_GLOVE  1
#define PAN_TOUCH_MODE_PEN    2
#define PAN_TOUCH_MODE_MAX    3

#define PAN_SMART_COVER_STATE_OPENED  10
#define PAN_SMART_COVER_STATE_CLOSED  11

#define PAN_SMART_COVER_GPIO  65
#define PAN_SMART_COVER_CHECK_TIMER 500

struct tsp_control_pin {
	int	type;
	const char *name;

	struct {
		int	num;		
		int	direction;
	} gpio;

	struct {		
		int	volt;
		struct regulator *reg;
	} regulator;
};

struct pan_touch_state{
  u8 touch;
  u8 smart_cover;
  u8 smart_cover_from_ui;
  struct timer_list check_smart_cover;	
};  

/**
 * struct fts_finger - Represents fingers.
 * @ state: finger status (Event ID).
 * @ mcount: moving counter for debug.
 */
struct fts_finger {
	unsigned char state;
	unsigned short mcount;
};

struct fts_ts_info {
	struct device *dev;
	struct i2c_client *client;
	struct fts_ts_platform_data	*pdata;
	struct input_dev *input_dev;
	struct hrtimer timer;
	struct timer_list timer_charger;
	struct timer_list timer_firmware;
	struct work_struct work;
	int irq;
	int irq_type;
	bool irq_enabled;
	//const struct fts_i2c_platform_data *board;
	struct fts_i2c_platform_data *board;
	void (*register_cb) (void *);
#ifdef FTS_SUPPORT_TA_MODE
	struct fts_callbacks callbacks;
#endif
	struct mutex lock;
	bool enabled;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
#ifdef CONFIG_POWERSUSPEND
	struct power_suspend power_suspend;
#endif
#ifdef TSP_FACTORY_TEST
	struct device *fac_dev_ts;
	struct list_head cmd_list_head;
	u8 cmd_state;
	char cmd[CMD_STR_LEN];
	int cmd_param[CMD_PARAM_NUM];
	char *cmd_result;
	int cmd_buffer_size;
	struct mutex cmd_lock;
	bool cmd_is_running;
	int SenseChannelLength;
	int ForceChannelLength;
	short *pFrame;
#endif

	struct completion init_done;

	bool slow_report_rate;
	bool hover_ready;
	bool hover_enabled;
	bool mshover_enabled;
	bool fast_mshover_enabled;
	bool flip_enable;

#ifdef FTS_SUPPORT_TA_MODE
	bool TA_Pluged;
#endif

	int touch_count;
	struct fts_finger finger[FINGER_MAX];

	int touch_mode;

	int ic_product_id;			/* product id of ic */
	int ic_revision_of_ic;		/* revision of reading from IC */
	int fw_version_of_ic;		/* firmware version of IC */
	int ic_revision_of_bin;		/* revision of reading from binary */
	int fw_version_of_bin;		/* firmware version of binary */
	int config_version_of_ic;		/* Config release data from IC */
	int config_version_of_bin;	/* Config release data from IC */
	unsigned short fw_main_version_of_ic;	/* firmware main version of IC */
	unsigned short fw_main_version_of_bin;	/* firmware main version of binary */
	int panel_revision;			/* Octa panel revision */

#ifdef USE_OPEN_DWORK
	struct delayed_work open_work;
#endif

#ifdef FTS_SUPPORT_NOISE_PARAM
	struct fts_noise_param noise_param;
	int (*fts_get_noise_param_address) (struct fts_ts_info *info);
#endif

	struct mutex i2c_mutex;
	struct mutex device_mutex;
	bool touch_stopped;
	bool reinit_done;
	int ito_test[2];

#ifdef FTS_SUPPORT_TOUCH_KEY
	unsigned char tsp_keystatus;
	bool report_dummy_key;
	bool ignore_menu_key;
	bool ignore_back_key;
	bool ignore_menu_key_by_back;
	bool ignore_back_key_by_menu;
	int touchkey_threshold;
#endif // FTS_SUPPORT_TOUCH_KEY

	unsigned char data[FTS_EVENT_SIZE * FTS_FIFO_MAX];

	int (*stop_device) (struct fts_ts_info * info);
	int (*start_device) (struct fts_ts_info * info);

	int (*fts_write_reg)(struct fts_ts_info *info, unsigned char *reg, unsigned short num_com);
	int (*fts_read_reg)(struct fts_ts_info *info, unsigned char *reg, int cnum, unsigned char *buf, int num);
	void (*fts_systemreset)(struct fts_ts_info *info);
	int (*fts_wait_for_ready)(struct fts_ts_info *info);
	void (*fts_command)(struct fts_ts_info *info, unsigned char cmd);
	int (*fts_get_version_info)(struct fts_ts_info *info);
#ifdef READ_LCD_ID
	int lcd_id;
#endif
};

int fts_fw_update_on_probe(struct fts_ts_info *info);
int fts_fw_update_on_hidden_menu(struct fts_ts_info *info, int update_type);
void fts_fw_init(struct fts_ts_info *info);
int GetSystemStatus(struct fts_ts_info *info, unsigned char *val1, unsigned char *val2);

#endif				//_LINUX_FTS_TS_H_
