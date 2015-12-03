#ifndef _LINUX_FTS_I2C_H_
#define _LINUX_FTS_I2C_H_

/* -------------------------------------------------------------------- */
/* debug option */
/* -------------------------------------------------------------------- */
#define PAN_TOUCH_DEBUG_I2C_MASK                0x00000001
#define PAN_TOUCH_DEBUG_OPERATION_ERR_MASK      0x00000002
#define PAN_TOUCH_DEBUG_OPERATION_MASK          0x00000004
#define PAN_TOUCH_DEBUG_IOCTL_MASK              0x00000008  
#define PAN_TOUCH_DEBUG_FIRMWARE_MASK           0x00000010
#define PAN_TOUCH_DEBUG_FUNCTION_NAME_MASK      0x00000020
#define PAN_TOUCH_DEBUG_HW_MASK                 0x00000040
#define PAN_TOUCH_DEBUG_CONFIG_MASK             0x00000080
#define PAN_TOUCH_DEBUG_TOUCH_MASK              0x00000100
#define PAN_TOUCH_DEBUG_ALL_MASK                0x000001FF

#define FTS_SUPPORT_NOISE_PARAM

struct fts_callbacks {
	void (*inform_charger) (struct fts_callbacks *, int);
};

#ifdef FTS_SUPPORT_NOISE_PARAM
#define MAX_NOISE_PARAM 5
struct fts_noise_param {
	unsigned short pAddr[MAX_NOISE_PARAM];
	unsigned short pData[MAX_NOISE_PARAM];
};
#endif

struct fts_i2c_platform_data {
	bool factory_flatform;
	bool recovery_mode;
	bool support_hover;
	bool support_mshover;
	int max_x;
	int max_y;
	int max_width;
	unsigned char panel_revision;	/* to identify panel info */

	const char *firmware_name;
	const char *project_name;

	int (*power) (bool enable);
	void (*register_cb) (void *);
	void (*enable_sync)(bool on);

	unsigned gpio;
	int irq_type;
};

//++ p11309 - 2013.12.22 for disabled
#define PAN_TSP_IO
//#define PAN_KNOCK_ON
#define TSP_FACTORY_TEST
//#define FTS_SUPPORT_TA_MODE
#define FTS_SUPPORT_TA_MODE_PANTECH // 2014.3.25 P13106. ADD TA MODE

//-- p11309

extern unsigned int lcdtype;

void fts_charger_infom(bool en);
#endif
