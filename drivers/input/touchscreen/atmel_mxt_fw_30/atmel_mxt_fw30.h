/* 
 * file_name: atmel_mxt_fw30.h
 *
 * description: atmel max touch driver.
 *
 * Copyright (C) 2008-2010 Atmel & Pantech Co. Ltd.
 * Copyright (C) 2013 Pantech Co. Ltd.
 *
 */

#ifndef _MXT_FW30_H_
#define _MXT_FW30_H_

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

static int pan_debug_state = PAN_TOUCH_DEBUG_OPERATION_ERR_MASK;
#define dbg_cr(fmt, args...)      printk("[+++ TOUCH] " fmt, ##args);
#define dbg_i2c(fmt,args...)      if(pan_debug_state & PAN_TOUCH_DEBUG_I2C_MASK) printk("[+++ TOUCH] " fmt,##args);
#define dbg_op_err(fmt,args...)   if(pan_debug_state & PAN_TOUCH_DEBUG_OPERATION_ERR_MASK) printk("[+++ TOUCH] " fmt,##args); 
#define dbg_op(fmt,args...)       if(pan_debug_state & PAN_TOUCH_DEBUG_OPERATION_MASK) printk("[+++ TOUCH] " fmt,##args);
#define dbg_ioctl(fmt,args...)    if(pan_debug_state & PAN_TOUCH_DEBUG_IOCTL_MASK) printk("[+++ TOUCH] " fmt,##args); 
#define dbg_firmw(fmt,args...)    if(pan_debug_state & PAN_TOUCH_DEBUG_FIRMWARE_MASK) printk("[+++ TOUCH] " fmt,##args); 
#define dbg_func_in()			        if(pan_debug_state & PAN_TOUCH_DEBUG_FUNCTION_NAME_MASK) printk("[+++ TOUCH] %s Func In\n", __func__);
#define dbg_func_out()			      if(pan_debug_state & PAN_TOUCH_DEBUG_FUNCTION_NAME_MASK) printk("[+++ TOUCH] %s Func Out\n", __func__)
#define dbg_hw(fmt,args...)       if(pan_debug_state & PAN_TOUCH_DEBUG_HW_MASK) printk("[+++ TOUCH] " fmt,##args); 
#define dbg_config(fmt,args...)   if(pan_debug_state & PAN_TOUCH_DEBUG_CONFIG_MASK) printk("[+++ TOUCH] " fmt,##args);
#define dbg_touch(fmt,args...)    if(pan_debug_state & (PAN_TOUCH_DEBUG_TOUCH_MASK|PAN_TOUCH_DEBUG_OPERATION_MASK)) printk("[+++ TOUCH] " fmt,##args);



/* -------------------------------------------------------------------- */
/* Default Define */
/* -------------------------------------------------------------------- */
#define TOUCH_IO
#define SKY_PROCESS_CMD_KEY
#define MXT_FIRMUP_ENABLE		// mxt chipset firmware update enable
#define TOUCH_MONITOR
#ifdef TOUCH_IO
#define CHARGER_MODE
#endif

// check feature.
//#define ITO_TYPE_CHECK 
#define OFFLINE_CHARGER_TOUCH_DISABEL



/* -------------------------------------------------------------------- */
/* mxt_fw30_config location defination */ 
/* -------------------------------------------------------------------- */

//++ p11309 - 2013.05.01 for Multi object selection
//	Search for PAN_MULTI_OBJECT_ENABLE.
#define PAN_MULTI_OBJECT_ENABLE
//#define PAN_TOUCH_GOLDREFERENCE_CHECK

#if defined(CONFIG_MACH_MSM8974_EF56S)
	#define PAN_HAVE_TOUCH_KEY
//++ p11309 - 2013.07.10 for Smart Cover Status
	#define PAN_SUPPORT_SMART_COVER
//++ p11309 - 2013.07.19 Support Soft Dead zone
	#define PAN_SUPPORT_SOFT_DEAD_ZONE
//++ p11309 - 2013.07.19 Check Noise Mode shake
	#define PAN_CHECK_NOISE_MODE_SHAKE
//	#define PAN_TOUCH_KEY_REJECTION_ON_DISPLAY
//-- p11309
//++ p11309 - 2013.11.22 for Touch Calibration Revision
	#define PAN_TOUCH_CAL_COMMON
	#define PAN_TOUCH_CAL_PMODE
//-- p11309

#elif defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF65S)
	#define PAN_TOUCH_PEN_DETECT
	#define PAN_T15_KEYARRAY_ENABLE
	#define PAN_SUPPORT_SMART_COVER
	#define PAN_SUPPORT_SOFT_DEAD_ZONE
	#define PAN_CHECK_NOISE_MODE_SHAKE
	#define PAN_TOUCH_KEY_REJECTION_ON_DISPLAY
  //++ p11309 - 2013.11.22 for Touch Calibration Revision
	#define PAN_TOUCH_CAL_COMMON
	#define PAN_TOUCH_CAL_PMODE
  //-- p11309

#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
	#define PAN_T15_KEYARRAY_ENABLE
	#define PAN_SUPPORT_SMART_COVER
	#define PAN_SUPPORT_SOFT_DEAD_ZONE
	#define PAN_CHECK_NOISE_MODE_SHAKE
	#define PAN_TOUCH_KEY_REJECTION_ON_DISPLAY
	
//++ p11309 - 2013.11.22 for Touch Calibration Revision
	#define PAN_TOUCH_CAL_COMMON
	#define PAN_TOUCH_CAL_PMODE
//-- p11309

#else
  #define PAN_T15_KEYARRAY_ENABLE
	#define PAN_SUPPORT_SMART_COVER
	#define PAN_SUPPORT_SOFT_DEAD_ZONE
	#define PAN_CHECK_NOISE_MODE_SHAKE
	#define PAN_TOUCH_KEY_REJECTION_ON_DISPLAY
	#define PAN_TOUCH_CAL_COMMON
	#define PAN_TOUCH_CAL_PMODE
#endif
//-- p11309


/* -------------------------------------------------------------------- */
/* DEFAULT DEFINE                                                       */
/* -------------------------------------------------------------------- */
#define U16     unsigned short int 
#define U8      __u8
#define u8      __u8
#define S16     signed short int
#define U16     unsigned short int
#define S32     signed long int
#define U32     unsigned long int
#define S64     signed long long int
#define U64     unsigned long long int
#define F32     float
#define F64     double

#ifndef min
#define min(a,b) ( a > b ? b : a)
#endif
#ifndef max
#define max(a,b) ( b > a ? b : a )
#endif
#define OBP_INSTANCES(o) ((u16)((o).instances) + 1)
#define OBP_SIZE(o) ((u16)((o).size) + 1)

/* -------------------------------------------------------------------- */
#define IRQ_TOUCH_INT		gpio_to_irq(GPIO_TOUCH_CHG)
/* -------------------------------------------------------------------- */

/* Atmel Firmware 3.0(Beta) Feature */
#define FEATURE_mXT540S_V_3_0
#define MXT_CURRENT_FAMILY_ID	  0x82	// mXT540S

#ifdef FEATURE_mXT540S_V_3_0
	#define MXT_CURRENT_FW_VERSION	0x30
#else
	#define MXT_CURRENT_FW_VERSION	0x21
#endif
#define MXT_CURRENT_FW_BUILD	  0xAA

unsigned char MXT_FW21_E_firmware[] = {
	#include "./firmware/mxt540s_V2.1.AA_.h"
};

#ifdef FEATURE_mXT540S_V_3_0
unsigned char MXT_FW30_E_firmware[] = {
	#include "./firmware/mxt540s_V3.0.AA_.h"
};
#endif

#define MXT_540S_MAX_CHANNEL_NUM	540
#define MXT_540S_DELTA_MODE			0x10
#define MXT_540S_REFERENCE_MODE		0x11
#define NUM_OF_TOUCH_OBJECTS	    0
#define MXT_MAX_BLOCK_WRITE			256
#define MXT_WAKEUP_TIME		        25	/* msec */

#ifdef PAN_TOUCH_PEN_DETECT
#if defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF65S)
#define PAN_TOUCH_PEN_GPIO  79
#else
#define PAN_TOUCH_PEN_GPIO  79
#endif
#endif


typedef enum{
	MXT_T100_TYPE_RESERVED,
	MXT_T100_TYPE_FINGER,
	MXT_T100_TYPE_PASSIVE_STYLUS,
	MXT_T100_TYPE_GLOVE=5,
}mxt_touch_type;

typedef enum{
	MXT_T100_EVENT_NONE,
	MXT_T100_EVENT_MOVE,
	MXT_T100_EVENT_UNSUPPRESS,
	MXT_T100_EVENT_SUPPRESS,
	MXT_T100_EVENT_DOWN,
	MXT_T100_EVENT_UP,
	MXT_T100_EVENT_UNSUPSUP,
	MXT_T100_EVENT_UNSUPUP,
	MXT_T100_EVENT_DOWNSUP,
	MXT_T100_EVENT_DOWNUP
}mxt_event_type;

#ifdef SKY_PROCESS_CMD_KEY
typedef enum {	
	READ_TOUCH_ID = 101,
	APPLY_TOUCH_CONFIG = 501,
	DIAG_DEBUG = 502,
	RESET_TOUCH_CONFIG = 503,
	GET_TOUCH_CONFIG = 504,
	SET_TOUCH_CONFIG = 505,
	//READ_ITO_TYPE = 506,
	ATMEL_GET_REFERENCE_DATA = 508,
	TOUCH_CHARGER_MODE = 701,
	TOUCH_IOCTL_READ_LASTKEY=1001,	
	TOUCH_IOCTL_DO_KEY,	
	TOUCH_IOCTL_RELEASE_KEY, 
	TOUCH_IOCTL_CLEAN,
	TOUCH_IOCTL_DEBUG,
	TOUCH_IOCTL_RESTART,
	TOUCH_IOCTL_PRESS_TOUCH,
	TOUCH_IOCTL_RELEASE_TOUCH,
	TOUCH_IOCTL_CHARGER_MODE,
	TOUCH_IOCTL_EARJACK_MODE,
	POWER_OFF,
	TOUCH_IOCTL_DELETE_ACTAREA = 2001,
	TOUCH_IOCTL_RECOVERY_ACTAREA,
	TOUCH_IOCTL_SENSOR_X = 2005,
	TOUCH_IOCTL_SENSOR_Y,
	TOUCH_IOCTL_CHECK_BASE,
	TOUCH_IOCTL_READ_IC_VERSION,
	TOUCH_IOCTL_READ_FW_VERSION,
	TOUCH_IOCTL_START_UPDATE,
	TOUCH_IOCTL_SELF_TEST,
	TOUCH_IOCTL_DIAGNOSTIC_MIN_DEBUG,
	TOUCH_IOCTL_DIAGNOSTIC_MAX_DEBUG,
	TOUCH_IOCTL_DIAG_DELTA = 2014,
	TOUCH_IOCTL_INIT = 3001,	
	TOUCH_IOCTL_OFF  = 3002,	
	TOUCH_IOCTL_EVENT_AUTOCAL_DISABLE  = 5001,
	TOUCH_IOCTL_MULTI_TSP_OBJECT_SEL  = 6001,
	TOUCH_IOCTL_SUSPEND,
	TOUCH_IOCTL_RESUME,
//++ p11309 - 2013.07.10 for Get Touch Mode 
	TOUCH_IOCTL_MULTI_TSP_OBJECT_GET,
//++ p11309 - 2013.05.14 for gold reference T66
  TOUCH_IOCTL_HALL_IC_GPIO_GET,         //P13106 for reading hall-ic gpio state
	TOUCH_IOCTL_WIFI_DEBUG_APP_ENABLE	= 7004,
	TOUCH_IOCTL_WIFI_DEBUG_APP_DISABLE	= 7005,
//++ p11309 - 2013.07.19 Check Noise Mode shake
#ifdef PAN_CHECK_NOISE_MODE_SHAKE
	TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_ENABLE = 7100,
	TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_DISABLE = 7101,
	TOUCH_IOCTL_NOISE_MODE_SHAKE_CHECK_GET = 7102,
#endif
//-- p11309
//++ p11309 - 2013.07.25 Get Model Color 
	TOUCH_IOCTL_MODEL_COLOR_GET = 7200,
	TOUCH_IOCTL_TOUCH_MODE = 7201,
//++ p11309 - 2013.09.06 for Touch Pen Insertion Check
  TOUCH_IOCTL_CHECK_TOUCH_PEN = 7300,
//-- p11309

//++ p11309 - 2013.10.01 for Touch Knock On		
	TOUCH_IOCTL_KNOCKON_SET_ENABLE = 7400,
	TOUCH_IOCTL_KNOCKON_GET_ENABLE = 7401,
//-- p11309
} TOUCH_IOCTL_CMD;
#endif

#ifdef TOUCH_IO
typedef enum  {
	IOCTL_DEBUG_SUSPEND = 0,	
	IOCTL_DEBUG_RESUME = 1,	
	IOCTL_DEBUG_GET_TOUCH_ANTITOUCH_INFO = 2,
	IOCTL_DEBUG_TCH_CH= 3,	
	IOCTL_DEBUG_ATCH_CH= 4,	
	IOCTL_GET_CALIBRATION_CNT = 5,	
} ioctl_debug_cmd;
#endif

enum mxt_device_state { INIT, BOOTLOADER, APPMODE, SUSMODE };

#define MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID		25
#define MXT_T100_SCREEN_MSG_FIRST_RPT_ID		23

struct mxt_fw30_data_t
{
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct work_struct work;	
	struct mutex    lock;
	enum mxt_device_state state; 
    struct bin_attribute mem_access_attr;
    bool debug_enabled;
    u16 mem_size;
	u16 T5_address;
	u8  T5_msg_size;
	u8  T6_reportid_min;
	u8  T6_reportid_max;
	u16 T6_address;
	u16 T7_address;
	u8  T15_reportid_min;
	u8  T15_reportid_max;
	u8  T42_reportid_min;
	u8  T42_reportid_max;
	u16 T44_address;
	u8  T46_reportid_min;
	u8  T46_reportid_max;
	u8  T66_reportid_min;
	u8  T66_reportid_max;
//++ p11309 - 2013.10.21 for Palm Gesture
	u8  T69_reportid_min;
	u8  T69_reportid_max;
//-- p11309
	u8  T72_reportid_min;
	u8  T72_reportid_max;
	u8  T100_reportid_min;
	u8  T100_reportid_max;
#ifdef ITO_TYPE_CHECK		//P13106
  int ito_color;
#endif
#ifdef PAN_TOUCH_PEN_DETECT
  int pan_touch_pen_state;
#endif
};

#ifdef TOUCH_MONITOR
typedef struct {
	int         size;   /* maximum number of elements           */
	int         start;  /* index of oldest element              */
	int         end;    /* index at which to write new element  */
	char *elems;  /* vector of elements                   */
} CircularBuffer;
#endif


//*****************************************************************************
//
/*! \brief Object table element struct. */
//
//*****************************************************************************

typedef struct
{
	uint8_t object_type;     /*!< Object type ID. */
	uint16_t start_position;    /*!< Start address of the obj config structure. */
	uint8_t size;            /*!< Byte length of the obj config structure -1.*/
	uint8_t instances;       /*!< Number of objects of this obj. type -1. */
	uint8_t num_report_ids;  /*!< The max number of touches in a screen,
							  *   max number of sliders in a slider array, etc.*/
} object_t;


/*! \brief Info ID struct. */
typedef struct
{
	uint8_t family_id;            /* address 0 */
	uint8_t variant_id;           /* address 1 */

	uint8_t version;              /* address 2 */
	uint8_t build;                /* address 3 */

	uint8_t matrix_x_size;        /* address 4 */
	uint8_t matrix_y_size;        /* address 5 */

	/*! Number of entries in the object table. The actual number of objects
	 * can be different if any object has more than one instance. */
	uint8_t num_declared_objects; /* address 6 */
} info_id_t;

typedef struct
{
	info_id_t *info_id;          /*! Info ID struct. */
	object_t *objects;          /*! Pointer to an array of objects. */
	uint16_t CRC;               /*! CRC field, low bytes. */
	uint8_t CRC_hi;             /*! CRC field, high byte. */
} info_block_t;

typedef struct
{
	uint8_t object_type;     /*!< Object type. */
	uint8_t instance;        /*!< Instance number. */
} report_id_map_t;

typedef struct
{
	uint8_t id;                     /*!< id */
	int8_t status;          /*!< dn=1, up=0, none=-1 */
	int16_t x;                      /*!< X */
	int16_t y;                      /*!< Y */
	int8_t mode;
	int area;
//++ p11309 - 2013.07.10 for Smart Cover Status
#ifdef PAN_SUPPORT_SMART_COVER
	int8_t ignored_by_cover;
#endif
//-- p11309

} report_finger_info_t;

typedef struct
{
	uint16_t code;                      /*!< key code */
	uint8_t status;                      /*!< key status (press/release) */
	bool update;     
} report_key_info_t;

typedef struct 
{
	uint16_t tch_ch; /*number of touch channel*/
	uint16_t atch_ch; /*number of anti-touch channel*/	
	uint8_t calibration_cnt; /*number of anti-touch channel*/
	uint8_t autocal_flag; /*calibration flag 0 => normal, over 1 => auto calibration is enabled */
} debug_info_t;

typedef enum 
{
	NO_COMMAND = 0u,
	COMM_MODE1 = 1u,
	COMM_MODE2 = 2u,
	COMM_MODE3 = 3u
}comm_cfg;

typedef enum
{
	TSC_EVENT_NONE,
	TSC_EVENT_WINDOW,
#ifdef PAN_HAVE_TOUCH_KEY
	TSC_EVENT_1ST_KEY,
	TSC_EVENT_2ND_KEY,
	TSC_EVENT_3RD_KEY,
#endif

} tsc_key_mode_type;

typedef enum
{
	TSC_CLEAR_ALL,
	TSC_CLEAR_EVENT,
} tsc_clear_type;

typedef enum
{
	TOUCH_EVENT_RELEASE = 0,
	TOUCH_EVENT_PRESS,
	TOUCH_EVENT_MOVE,
	TOUCH_EVENT_NOTHING,
} TOUCH_EVENT;

#ifdef ITO_TYPE_CHECK	
typedef enum
{
	TOUCH_WINDOW_WHITE = 0,
	TOUCH_WINDOW_BLACK
} TOUCH_WINDOW;

#endif

#ifdef TOUCH_IO

/*
 * vendor_id : 
 * ateml(1) cypress(2)
 * model_id : 
 * EF56S(0560) EF57K(0570) EF58L(0580)
 * 
 * type_id : 
 * model manager would manage ito or color type.

 * return (vendor_id<<16) + (model_id<<4) + type_id;
 */ 
 
static int vendor_id = 1;
#if defined(CONFIG_MACH_MSM8974_EF56S)
static int model_id = 560;
#elif defined(CONFIG_MACH_MSM8974_EF57K)
static int model_id = 570;
#elif defined(CONFIG_MACH_MSM8974_EF58L)
static int model_id = 580;

#elif defined(CONFIG_MACH_MSM8974_NAMI)
static int model_id = 590;

#elif defined(CONFIG_MACH_MSM8974_EF59S)
static int model_id = 600;
#elif defined(CONFIG_MACH_MSM8974_EF59K)
static int model_id = 610;
#elif defined(CONFIG_MACH_MSM8974_EF59L)
static int model_id = 620;

#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S)
static int model_id = 630;
#elif defined(CONFIG_MACH_MSM8974_EF61K)
static int model_id = 640;
#elif defined(CONFIG_MACH_MSM8974_EF62L)
static int model_id = 650;
#elif defined(CONFIG_MACH_MSM8974_EF63S)
static int model_id = 660;
#elif defined(CONFIG_MACH_MSM8974_EF63K)
static int model_id = 670;
#elif defined(CONFIG_MACH_MSM8974_EF63L)
static int model_id = 680;
#else
static int model_id = 100;
#endif

static int type_id = 0;

typedef enum
{
	BATTERY_PLUGGED_NONE = 0,
	BATTERY_PLUGGED_CHARGER = 1	
}type_charger_mode;

unsigned int tsp_power_status=1;
unsigned long charger_mode;
unsigned long pre_charger_mode;
#endif //TOUCH_IO

/* \brief Defines CHANGE line active mode. */
/* \brief Defines CHANGE line active mode. */
#define CHANGELINE_NEGATED          0u
#define CHANGELINE_ASSERTED         1u


/* mxt chipset firmware update command */
#define MXT_WAITING_BOOTLOAD_COMMAND 	0xC0
#define MXT_WAITING_FRAME_DATA       	0x80
#define MXT_FRAME_CRC_CHECK          	0x02
#define MXT_FRAME_CRC_PASS           	0x04
#define MXT_FRAME_CRC_FAIL           	0x03

/* i2c address for mxt chipset firmware update */
#define I2C_APPL_ADDR_0			0x94
#define I2C_APPL_ADDR_1			0x96
#define I2C_BOOT_ADDR_0			0x48
#define I2C_BOOT_ADDR_1			0x4A

/* -------------------------------------------------------------------- */
/* mxt_fw30 i2c slave address */
/* -------------------------------------------------------------------- */
#define MXT_I2C_ADDR       0x4A
#define MXT_I2C_BOOT_ADDR  0x24
/* -------------------------------------------------------------------- */

/* This sets the I2C frequency to 400kHz (it's a feature in I2C driver that the
   actual value needs to be double that). */
#define I2C_SPEED                   800000u

#define CONNECT_OK                  1u
#define CONNECT_ERROR               2u

#define READ_MEM_OK                 1u
#define READ_MEM_FAILED             2u

#define MESSAGE_READ_OK             1u
#define MESSAGE_READ_FAILED         2u

#define WRITE_MEM_OK                1u
#define WRITE_MEM_FAILED            2u

#define CFG_WRITE_OK                1u
#define CFG_WRITE_FAILED            2u

#define I2C_INIT_OK                 1u
#define I2C_INIT_FAIL               2u

#define CRC_CALCULATION_OK          1u
#define CRC_CALCULATION_FAILED      2u

#define ID_MAPPING_OK               1u
#define ID_MAPPING_FAILED           2u

#define ID_DATA_OK                  1u
#define ID_DATA_NOT_AVAILABLE       2u

enum driver_setup_t {DRIVER_SETUP_OK, DRIVER_SETUP_INCOMPLETE};

/*! \brief Returned by get_object_address() if object is not found. */
#define OBJECT_NOT_FOUND   				      0u
/*! Address where object table starts at touch IC memory map. */
#define OBJECT_TABLE_START_ADDRESS      7U
/*! Size of one object table element in touch IC memory map. */
#define OBJECT_TABLE_ELEMENT_SIZE       6U
/*! Offset to RESET register from the beginning of command processor. */
#define RESET_OFFSET                    0u
/*! Offset to BACKUP register from the beginning of command processor. */
#define BACKUP_OFFSET       			      1u
/*! Offset to CALIBRATE register from the beginning of command processor. */
#define CALIBRATE_OFFSET    			      2u
/*! Offset to REPORTALL register from the beginning of command processor. */
#define REPORTATLL_OFFSET   			      3u
/*! Offset to DEBUG_CTRL register from the beginning of command processor. */
#define DEBUG_CTRL_OFFSET   			      4u
/*! Offset to DIAGNOSTIC_CTRL register from the beginning of command processor. */
#define DIAGNOSTIC_OFFSET   			      5u


//*****************************************************************************
//
//
//		std_objects_driver
//
//
//*****************************************************************************

/*! ===Header File Version Number=== */
#define OBJECT_LIST__VERSION_NUMBER               0x11

#define RESERVED_T0                               0u
#define RESERVED_T1                               1u
#define RESERVED_T2                               2u
#define RESERVED_T3                               3u
#define RESERVED_T4                               4u
#define GEN_MESSAGEPROCESSOR_T5                   5u
#define GEN_COMMANDPROCESSOR_T6                   6u
#define GEN_POWERCONFIG_T7                        7u
#define GEN_ACQUISITIONCONFIG_T8                  8u
#define TOUCH_MULTITOUCHSCREEN_T9                 9u
#define RESERVED_T10                              10u
#define RESERVED_T11                              11u
#define RESERVED_T12                              12u
#define RESERVED_T13                              13u
#define RESERVED_T14                              14u
#define TOUCH_KEYARRAY_T15                        15u
#define PROCI_LINEARIZATIONTABLE_T17              17u
#define SPT_COMCONFIG_T18                         18u
#define SPT_GPIOPWM_T19                           19u
#define PROCI_GRIPFACESUPPRESSION_T20             20u
#define PROCG_NOISESUPPRESSION_T22                22u
#define TOUCH_PROXIMITY_T23                       23u
#define PROCI_ONETOUCHGESTUREPROCESSOR_T24        24u
#define SPT_SELFTEST_T25                          25u
#define DEBUG_CTERANGE_T26                        26u
#define SPT_CTECONFIG_T28                         28u
#define DEBUG_DIAGNOSTIC_T37                      37u
#define SPT_USERDATA_T38						              38u
#define PROCI_GRIPSUPPRESSION_T40                 40u  // added MXT224E
#define PROCI_PALMSUPPRESSION_T41                 41u  // added MXT1386,MXT768E
#define PROCI_TOUCHSUPPRESSION_T42                42u  // added MXT224E
#define SPT_DIGITIZER_T43                         43u  // added MXT1386,MXT768E
#define SPT_MESSAGECOUNT_T44                      44u
#define SPARE_T45                                 45u
#define SPT_CTECONFIG_T46                         46u  // added MXT224E
#define PROCI_STYLUS_T47                          47u  // added MXT224E
#define PROCG_NOISESUPPRESSION_T48                48u  // added MXT224E
#define SPARE_T49                                 49u
#define SPARE_T50                                 50u
#define SPARE_T51                                 51u
#define PROXIMITYKEY_T52						              52u
#define GENDATASOURCE_T53						              53u
#define SPARE_T54                                 54u
#define PROCI_ADAPTIVETHRESHOLD_T55				        55u
#define PROCI_SHIELDLESS_T56					            56u
#define PROCI_EXTRATOUCHSCREENDATA_T57		        57u
#define SPT_TIMER_T61							                61u
#define PROCG_NOISESUPPRESSION_T62                62u 
#define PROCI_LENSBENDING_T65              		    65u 
#define SPT_GOLDENREFERENCES_T66				          66u 
#define PROCI_PALMGESTUREPROCESSOR_T69            69u 
#define SPT_DYNAMICCONFIGURATIONCONTROLLER_T70    70u
#define SPT_DYNAMICCONFIGURATIONCONTAINER_T71     71u
#define PROCG_NOISESUPPRESSION_T72                72u 
#define PROCI_GLOVEDETECTION_T78              	  78u 
#define PROCI_RETRANSMISSIONCOMPENSATION_T80	    80u
#define PROCI_GESTUREPROCESSOR_T84		            84u
#define TOUCH_MULTITOUCHSCREEN_T100               100u
#define SPT_TOUCHSCREENHOVER_T101	                101u
#define SPT_SELFCAPCBCRCONFIG_T102		            102u
#define PROCI_SCHNOISESUPPRESSION_T103		        103u
#define SPT_AUXTOUCHCONFIG_T104			              104u
#define SPT_DRIVENPLATEHOVERCONFIG_T105		        105u

#define GEN_INFOBLOCK16BIT_T254                   254u  //added MXT224E

/*
 * All entries spare up to 255
 */
#define RESERVED_T255    255u

/*----------------------------------------------------------------------------
  Messages definitions
  ----------------------------------------------------------------------------*/
#define TOUCH_DETECT		0x80
#define TOUCH_PRESS			0x40
#define TOUCH_RELEASE		0x20
#define TOUCH_MOVE			0x10
#define TOUCH_VECTOR		0x08
#define TOUCH_AMP			  0x04
#define TOUCH_SUPPRESS	0x02
#define TOUCH_UNGRIP		0x01

/*----------------------------------------------------------------------------
  type definitions
  ----------------------------------------------------------------------------*/

typedef struct
{
	uint8_t reset;       	/*!< Force chip reset             */
	uint8_t backupnv;    	/*!< Force backup to eeprom/flash */
	uint8_t calibrate;   	/*!< Force recalibration          */
	uint8_t reportall;   	/*!< Force all objects to report  */
	uint8_t reserve;     	/*!< Turn on output of debug data */
	uint8_t diagnostic;  	/*!< Controls the diagnostic object */
} gen_commandprocessor_t6_config_t;

typedef struct
{
	uint8_t idleacqint;    /*!< Idle power mode sleep length in ms           */
	uint8_t actvacqint;    /*!< Active power mode sleep length in ms         */
	uint8_t actv2idleto;   /*!< Active to idle power mode delay length in units of 0.2s*/
	uint8_t cfg;
} gen_powerconfig_t7_config_t;

typedef struct
{ 
	uint8_t chrgtime;          /*!< Charge-transfer dwell time             */  
	uint8_t reserved_0;          /*!< reserved                               */
	uint8_t tchdrift;          /*!< Touch drift compensation period        */
	uint8_t driftst;           /*!< Drift suspend time                     */
	uint8_t tchautocal;        /*!< Touch automatic calibration delay in units of 0.2s*/
	uint8_t sync;              /*!< Measurement synchronisation control    */
	uint8_t atchcalst;         /*!< recalibration suspend time after last detection */
	uint8_t atchcalsthr;       /*!< Anti-touch calibration suspend threshold */
	uint8_t atchfrccalthr;     /*!< Anti-touch force calibration threshold */
	int8_t  atchfrccalratio;   /*!< Anti-touch force calibration ratio */  
	uint8_t measallow;         /*!< reserved            */
	uint8_t measidledef;
	uint8_t measactivedef;
	uint8_t reserved_1;

} gen_acquisitionconfig_t8_config_t;


typedef struct
{
	/* Key Array Configuration */
	uint8_t ctrl;               /*!< ACENABLE LCENABLE Main configuration field           */
	uint8_t xorigin;           /*!< ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;           /*!< ACMASK LCMASK Object y start position on matrix  */
	uint8_t xsize;             /*!< ACMASK LCMASK Object x size (i.e. width)         */
	uint8_t ysize;             /*!< ACMASK LCMASK Object y size (i.e. height)        */
	uint8_t akscfg;             /*!< Adjacent key suppression config     */
	uint8_t blen;               /*!< ACMASK Burst length for all object channels*/
	uint8_t tchthr;             /*!< ACMASK LCMASK Threshold for all object channels   */
	uint8_t tchdi;              /*!< Detect integration config           */
	uint8_t reserved[2];        /*!< Spare x2 */

} touch_keyarray_t15_config_t;

typedef struct
{
	uint8_t  ctrl;
	uint8_t  cmd;

} spt_commsconfig_t18_config_t;

typedef struct
{
	uint8_t  ctrl;
	uint8_t  reportmask;
	uint8_t  dir;
	uint8_t  intpullup;
	uint8_t  out;
	uint8_t  wake;
} spt_gpiopwm_t19_config_t;

typedef struct {

	/* Prox Configuration */
	uint8_t ctrl;               /*!< ACENABLE LCENABLE Main configuration field           */
	uint8_t xorigin;           /*!< ACMASK LCMASK Object x start position on matrix  */
	uint8_t yorigin;           /*!< ACMASK LCMASK Object y start position on matrix  */
	uint8_t xsize;             /*!< ACMASK LCMASK Object x size (i.e. width)         */
	uint8_t ysize;             /*!< ACMASK LCMASK Object y size (i.e. height)        */
	uint8_t reserved_0;
	uint8_t blen;               /*!< ACMASK Burst length for all object channels*/
	uint16_t fxddthr;             /*!< Fixed detection threshold   */
	uint8_t fxddi;              /*!< Fixed detection integration  */
	uint8_t average;            /*!< Acquisition cycles to be averaged */
	uint16_t mvnullrate;               /*!< Movement nulling rate */
	uint16_t mvdthr;               /*!< Movement detection threshold */
	uint8_t  cfg;
} touch_proximity_t23_config_t;

typedef struct
{
	uint16_t upsiglim;              /* LCMASK */
	uint16_t losiglim;              /* LCMASK */

} siglim_t;

/*! = Config Structure = */

typedef struct
{
	uint8_t  ctrl;                 /* LCENABLE */
	uint8_t  cmd;
#if(NUM_OF_TOUCH_OBJECTS)
	siglim_t siglim[NUM_OF_TOUCH_OBJECTS];   /* LCMASK */
#endif
	uint8_t  pindwellus;
#if(NUM_OF_TOUCH_OBJECTS)  
	uint16_t sigrangelim[NUM_OF_TOUCH_OBJECTS];   /* LCMASK */
#endif
} spt_selftest_t25_config_t;

typedef struct
{
	uint8_t mode;
	uint8_t page;
	uint8_t data[128];

} debug_diagnositc_t37_t;

typedef struct
{
	uint8_t mode;
	uint8_t page;
	int8_t data[128];

} debug_diagnositc_t37_delta_t;

typedef struct
{
	uint8_t mode;
	uint8_t page;
	uint16_t data[64];

} debug_diagnositc_t37_reference_t;

typedef struct
{
	uint8_t ctrl;          /*!< Reserved/ GRIPMODE/ Reserved/ ENABLE */
	uint8_t xlogrip;       /*!< Grip suppression X low boundary   */
	uint8_t xhigrip;       /*!< Grip suppression X high boundary  */
	uint8_t ylogrip;       /*!< Grip suppression Y low boundary   */
	uint8_t yhigrip;       /*!< Grip suppression Y high boundary  */
} proci_gripsuppression_t40_config_t;

typedef struct
{
	uint8_t ctrl;            /*!< ctrl field reserved for future expansion */
	uint8_t reserved;         /*!< Approach threshold */
	uint8_t maxapprarea;     /*!< Maximum approach area threshold */
	uint8_t maxtcharea;      /*!< Maximum touch area threshold */
	uint8_t supstrength;     /*!< Suppression aggressiveness */
	uint8_t supextto;        /*!< Suppression extension timeout */ 
	uint8_t maxnumtchs;      /*!< Maximum touches */
	uint8_t shapestrength;   /*!< Shaped-based aggressiveness */
	uint8_t supdist;
	uint8_t disthyst;
	uint8_t reserved_0;
	uint8_t cfg;
	uint8_t reserved_1;
} proci_touchsuppression_t42_config_t;

typedef struct
{
	uint8_t ctrl;          /*!< ctrl field reserved for future expansion */
	uint8_t reserved_0;          /*!< X line start position   */                           
	uint8_t idlesyncsperx; /*!< Number of sets of ADC conversions per X when idle  */
	uint8_t actvsyncsperx; /*!< Number of sets of ADC conversions per X when active*/
	uint8_t adcspersync;   /*!< Number of ADC conversions per sync edge            */
	uint8_t pulsesperadc;  /*!< Number of pulses for each ADC conversion           */
	uint8_t xslew;         /*!< X pulse slew rate                                  */
	uint16_t syncdelay; /* < Delay from SYNC edge	*/
	uint8_t xvoltage;	/* < XVdd voltage mode	*/
}spt_cteconfig_t46_config_t;


typedef struct
{
	uint8_t  ctrl;          /*!< Reserved ENABLE            */   
	uint8_t  contmin;       /*!< Minimum contact diameter   */
	uint8_t  contmax;       /*!< Maximum contact diameter   */
	uint8_t  stability;     /*!< Stability                  */
	uint8_t  maxtcharea;    /*!< Maximum touch are          */
	uint8_t  amplthr;       /*!< Maximum touch amplitude    */
	uint8_t  styshape;      /*!< Stylus shape adjustment    */
	uint8_t  hoversup;      /*!< Hovering finger suppression*/
	uint8_t  confthr;       /*!< Confidence threshold       */
	uint8_t  syncsperx;     /*!< ADC sets per X             */
	int8_t   xposadj;
	int8_t   yposadj;	
	uint8_t  cfg;
	uint8_t  reserved0;
	uint8_t  reserved1;
	uint8_t  reserved2;
	uint8_t  reserved3;
	uint8_t  reserved4;
	uint8_t  reserved5;
	uint8_t  reserved6;
	uint8_t  supstyto;	/*!< Suppression timeout        */
	uint8_t  maxnumsty;	/*!< Maximum stylus touches             */
	uint8_t  xedgectrl;
	uint8_t  xedgedist;
	uint8_t  yedgectrl;
	uint8_t  yedgedist;
}proci_stylus_t47_config_t;


typedef struct
{
	uint8_t ctrl;				
	uint8_t targetthr;	
	uint8_t thradjlim;
	uint8_t resetsteptime;
	uint8_t forcechgdist;
	uint8_t forcechgtime;
	int8_t lowestthr;
} proci_adaptivethreshold_t55_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t reserved_0;
	uint8_t optint; 
	uint8_t inttime; 
	uint8_t intdelay0;
	uint8_t intdelay1;	
	uint8_t intdelay2;	
	uint8_t intdelay3;	
	uint8_t intdelay4;	
	uint8_t intdelay5;	
	uint8_t intdelay6;	
	uint8_t intdelay7;	
	uint8_t intdelay8;		
	uint8_t intdelay9;	
	uint8_t intdelay10;
	uint8_t intdelay11;
	uint8_t intdelay12;
	uint8_t intdelay13;
	uint8_t intdelay14;
	uint8_t intdelay15;	
	uint8_t intdelay16;
	uint8_t intdelay17;
	uint8_t intdelay18;	
	uint8_t intdelay19;
	uint8_t intdelay20;
	uint8_t intdelay21;	
	uint8_t intdelay22;
	uint8_t intdelay23;
	uint8_t intdelay24;
	uint8_t intdelay25;
	uint8_t intdelay26;
	uint8_t intdelay27;
	uint8_t intdelay28;
	uint8_t intdelay29;
	uint8_t multicutgc; 
	uint8_t gclimit;	
} proci_shieldless_t56_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t cmd;
	uint8_t mode;
	uint16_t period;
} spt_timer_t61_config_t;

typedef struct {
	uint8_t ctrl;          /*!< ctrl field reserved for future expansion */
	uint8_t gradthr;	/* < Gradient Threshold	*/
	uint16_t ylonoisemul; /*!< Y low noise multiplier  */
	uint16_t ylonoisediv; /*!< Y low noise divisor*/
	uint16_t yhinoisemul; /*!< Y high noise multiplier  */
	uint16_t yhinoisediv; /*!< Y high noise divisor*/
	uint8_t lpfiltcoef;         /*!< Filter Coefficient                                  */
	uint16_t forcescale; /*! < Force Scale	*/
	uint8_t forcethr;	/*! < Force Threshold	*/
	uint8_t forcethrhyst;	/*! < Force Threshold Hysteresis	*/
	uint8_t forcedi;	/*! < Force DI	*/
	uint8_t forcehyst;	/*! < Force Update Hysteresis	*/
	uint8_t atchratio;	
	uint8_t reserved_0;
	uint8_t reserved_1;

}proci_lensbending_t65_config_t;

typedef struct
{
	uint8_t ctrl;				
	uint8_t fcalfailthr;	
	uint8_t fcaldriftcnt;
} spt_goldenreferences_t66_config_t;

typedef struct
{
	uint8_t ctrl;				
	uint8_t longdimthr;	
	uint8_t shortdimthr;
	uint8_t longdimhyst;
	uint8_t shortdimhyst;
	uint8_t movthr;
	uint8_t movthrto;
	uint8_t areathr;
	uint8_t areathrto;
} proci_palmgestureprocessor_t69_config_t;

typedef struct
{
	uint8_t ctrl;
	uint16_t event;	
	uint8_t objtype;
	uint8_t reserved_0;
	uint8_t objinst;
	uint8_t dstoffset;
	uint16_t srcoffset;
	uint8_t length;
}spt_dynamicconfigurationcontroller_t70_config_t;

typedef struct
{
	uint8_t data[112];
}spt_dynamicconfigurationcontainer_t71_config_t;

typedef struct {
	uint8_t ctrl;
	uint8_t calcfg1;
	uint8_t cfg1;
	uint8_t cfg2;
	uint8_t reserved_0;
	uint8_t hopcnt;
	uint8_t hopcntper;
	uint8_t hopevalto;
	uint8_t hopst;	
	uint8_t nlgaindualx; 
	uint8_t minnlthr; 
	uint8_t incnlthr; 
	uint8_t fallnlthr; 
	uint8_t nlthrmargin; 
	uint8_t minthradj;
	uint8_t nlthrlimit;
	uint8_t reserved_1;
	uint8_t nlgainsingx;
	uint8_t blknlthr;
	uint8_t reserved_2;
	uint8_t stabctrl;
	uint8_t stabfreq_0;
	uint8_t stabfreq_1;
	uint8_t stabfreq_2;
	uint8_t stabfreq_3;
	uint8_t stabfreq_4;
	uint8_t stabtchapx_0;
	uint8_t stabtchapx_1;		
	uint8_t stabtchapx_2;
	uint8_t stabtchapx_3;
	uint8_t stabtchapx_4;
	uint8_t stabnotchapx_0;
	uint8_t stabnotchapx_1;
	uint8_t stabnotchapx_2;
	uint8_t stabnotchapx_3;
	uint8_t stabnotchapx_4;
	uint8_t reserved_3;
	uint8_t reserved_4;
	uint8_t stabhighnlthr;
	uint8_t reserved_5;
	uint8_t noisctrl;
	uint8_t noisfreq_0;
	uint8_t noisfreq_1;
	uint8_t noisfreq_2;
	uint8_t noisfreq_3;
	uint8_t noisfreq_4;
	uint8_t noistchapx_0;
	uint8_t noistchapx_1;
	uint8_t noistchapx_2;
	uint8_t noistchapx_3;
	uint8_t noistchapx_4;
	uint8_t noisnotchapx_0;
	uint8_t noisnotchapx_1;
	uint8_t noisnotchapx_2;
	uint8_t noisnotchapx_3;
	uint8_t noisnotchapx_4;
	uint8_t reserved_6;
	uint8_t noislownlthr;
	uint8_t noishighnlthr;
	uint8_t noiscnt;
	uint8_t vnoictrl;
	uint8_t vnoifreq_0;
	uint8_t vnoifreq_1;
	uint8_t vnoifreq_2;
	uint8_t vnoifreq_3;
	uint8_t vnoifreq_4;
	uint8_t vnoitchapx_0;
	uint8_t vnoitchapx_1;
	uint8_t vnoitchapx_2;
	uint8_t vnoitchapx_3;
	uint8_t vnoitchapx_4;
	uint8_t vnoinotchapx_0;
	uint8_t vnoinotchapx_1;
	uint8_t vnoinotchapx_2;
	uint8_t vnoinotchapx_3;
	uint8_t vnoinotchapx_4;	
	uint8_t reserved_7;
	uint8_t vnoilownlthr;
	uint8_t reserved_8;
	uint8_t vnoicnt;
} procg_noisesuppression_t72_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t minarea;	
	uint8_t confthr;
	uint8_t mindist;
	uint8_t glovemodeto;
	uint8_t supto;
	uint8_t syncsperx;
	uint8_t hithrmargin;

}proci_glovedetection_t78_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t compgain;	
	uint8_t targetdelta;
	uint8_t compthr;
	uint8_t atchthr;
	uint8_t moistcfg;
	uint8_t moistdto;
}proci_retransmissioncompensation_t80_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t zonethr;	
	uint8_t direl;
	uint8_t dto;
}proci_gesturepocessor_t84_config_t;

typedef struct {
	/* Screen Configuration */
	uint8_t ctrl;            /*!< ACENABLE LCENABLE Main configuration field  */
	uint8_t cfg1;
	uint8_t scraux;
	uint8_t tchaux;
	uint8_t tcheventcfg;
	uint8_t akscfg;
	uint8_t numtch;
	uint8_t xycfg;
	uint8_t xorigin;         /*!< LCMASK ACMASK Object x start position on matrix  */
	uint8_t xsize;           /*!< LCMASK ACMASK Object x size (i.e. width)         */
	uint8_t xpitch;
	int8_t xloclip;       /*!< LCMASK */
	int8_t xhiclip;       /*!< LCMASK */
	uint16_t xrange;       /*!< LCMASK */
	uint8_t xedgecfg;     /*!< LCMASK */
	uint8_t xedgedist;     /*!< LCMASK */
	uint8_t dxxedgecfg;     /*!< LCMASK */
	uint8_t dxxedgedist;     /*!< LCMASK */
	uint8_t yorigin;         /*!< LCMASK ACMASK Object y start position on matrix  */
	uint8_t ysize;           /*!< LCMASK ACMASK Object y size (i.e. height)        */
	uint8_t ypitch;
	int8_t yloclip;       /*!< LCMASK */
	int8_t yhiclip;       /*!< LCMASK */
	uint16_t yrange;       /*!< LCMASK */
	uint8_t yedgecfg;     /*!< LCMASK */
	uint8_t yedgedist;     /*!< LCMASK */
	uint8_t gain;
	uint8_t dxgain;
	uint8_t tchthr;          /*!< ACMASK Threshold for all object channels   */
	uint8_t tchhyst;       /* Touch threshold hysteresis */
	uint8_t intthr;
	uint8_t noisesf;
	uint8_t cutoffthr;
	uint8_t mrgthr;     /*!< The threshold for the point when two peaks are
						 *   considered one touch */
	uint8_t mrgthradjstr;
	uint8_t mrghyst;    /*!< The hysteresis applied on top of the merge threshold
						 *   to stop oscillation */
	uint8_t dxthrsf;
	uint8_t tchdidown;
	uint8_t tchdiup;
	uint8_t nexttchdi;
	uint8_t reserved;
	uint8_t jumplimit;
	uint8_t movfilter;
	uint8_t movsmooth;
	uint8_t movpred;
	uint16_t movhysti;
	uint16_t movhystn;
	uint8_t amplhyst;
	uint8_t scrareahyst;
	uint8_t intthrhyst;
} touch_multitouchscreen_t100_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t xloclip;	
	uint8_t xhiclip;
	uint8_t xedgecfg;
	uint8_t xedgedist;
	uint8_t xgain;
	uint8_t xhvrthr;
	uint8_t xhvrhyst;
	uint8_t yloclip;
	uint8_t yhiclip;
	uint8_t yedgecfg;
	uint8_t yedgedist;
	uint8_t ygain;
	uint8_t yhvthr;	
	uint8_t yhvrhyst;
	uint8_t hvrdi;
	uint8_t confthr;
	uint8_t movfilter;
	uint8_t movsmooth;
	uint8_t movpred;
	uint16_t movhysti;
	uint16_t movhystn;
	uint8_t hvraux;
}spt_touchscreenhover_t101_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t cmd;	
	uint8_t mode;
	uint16_t tunthr;
	uint16_t tunhyst;
	uint8_t reserved_0;
	uint8_t tuncfg;
	uint8_t tunsyncsperl;
	uint8_t tungain;
	uint8_t reserved_1;
	uint8_t prechrgtime;
	uint8_t chrgtime;
	uint8_t inttime;	
	uint8_t reserved_2;
	uint8_t reserved_3;
	uint8_t reserved_4;
	uint8_t idlesyncsperl;
	uint8_t actvsyncsperl;
	uint8_t drift;
	uint8_t driftst;
	uint8_t reserved_5;
	uint8_t filter;
	uint8_t filtcfg;	
	uint8_t dyniirthru;
	uint8_t dyniirthrl;
	uint8_t dyniirclmp;	
	uint8_t recalcfg;
}spt_selfcapcbcrconfig_t102_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t cfg;	
	uint8_t reserved_0;
	uint8_t procst;
	uint16_t schsiglothr;
	uint16_t schsighithr;
	uint8_t nlxgain;
	uint8_t nlygain;	
	uint16_t nllothr;
	uint16_t nlhicthr;
//	uint8_t reserved[6];
}proci_schnoisesuppression_t103_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t xgain;	
	uint8_t xtchthr;
	uint8_t xtchhyst;
	uint8_t xintthr;
	uint8_t xinthyst;
	uint8_t ygain;	
	uint8_t ytchthr;
	uint8_t ytchhyst;
	uint8_t yintthr;
	uint8_t yinthyst;
}spt_auxtouchconfig_t104_config_t;

typedef struct
{
	uint8_t ctrl;
	uint8_t mode;	
	uint8_t prechrgtime;
	uint8_t chrgtime;
	uint8_t inttime;
	uint8_t reserved_0;
	uint8_t reserved_1;	
	uint8_t reserved_2;
	uint8_t idlesyncsperl;
	uint8_t actvsyncsperl;
	uint8_t drift;
	uint8_t driftst;
	uint8_t driftsthrsf;
	uint8_t filter;
	uint8_t filtcfg;
	uint8_t dyniirthru;
	uint8_t dyniirthrl;
	uint8_t dyniirclmp;
}spt_drivenplatehoverconfig_t105_config_t;



/******************************************************************************
 *       MXT_FW30 Object table init
 * *****************************************************************************/
//General Object
gen_powerconfig_t7_config_t 			power_config = {0};  
gen_acquisitionconfig_t8_config_t 		acquisition_config = {0};

//Touch Object
touch_keyarray_t15_config_t 			keyarray_config = {0};  
touch_proximity_t23_config_t 			proximity_config = {0};      
touch_multitouchscreen_t100_config_t 	touchscreen_config = {0};

//Support Objects
spt_commsconfig_t18_config_t 			    comms_config = {0};
spt_gpiopwm_t19_config_t              gpiopwm_config = {0};
spt_selftest_t25_config_t 				    selftest_config = {0};  
spt_cteconfig_t46_config_t              cte_t46_config = {0};
spt_timer_t61_config_t					spt_timer_t61_config = {0};
spt_goldenreferences_t66_config_t		goldenreferences_t66_config = {0};
spt_dynamicconfigurationcontroller_t70_config_t dynamicconfigurationcontroller_t70_config= {0};
spt_dynamicconfigurationcontainer_t71_config_t dynamicconfigurationcontainer_t71_config={{0},};
spt_selfcapcbcrconfig_t102_config_t			selfcapcbcrconfig_t102_config = {0};
spt_auxtouchconfig_t104_config_t			auxtouchconfig_t104_config = {0};

//Signal Processing Objects
proci_gripsuppression_t40_config_t      gripsuppression_t40_config = {0};       
proci_touchsuppression_t42_config_t     touchsuppression_t42_config = {0};
proci_stylus_t47_config_t               stylus_t47_config = {0};
proci_adaptivethreshold_t55_config_t 	proci_adaptivethreshold_t55_config = {0}; 
proci_shieldless_t56_config_t			proci_shieldless_t56_config = {0}; 
proci_lensbending_t65_config_t			lensbending_t65_config = {0};
proci_palmgestureprocessor_t69_config_t	palmgestureprocessor_t69_config = {0};
procg_noisesuppression_t72_config_t     noisesuppression_t72_config = {0};
proci_glovedetection_t78_config_t       glovedetection_t78_config = {0};
proci_retransmissioncompensation_t80_config_t	retransmissioncompensation_t80_config = {0};
proci_schnoisesuppression_t103_config_t			schnoisesuppression_t103_config = {0};

// none object firmware 3.0
proci_gesturepocessor_t84_config_t gesturepocessor_t84_config = {0};
spt_touchscreenhover_t101_config_t touchscreenhover_t101_config = {0};
spt_drivenplatehoverconfig_t105_config_t drivenplatehoverconfig_t105_config = {0};

//*****************************************************************************
//		Config Table for Touch Monitor Interface by SWKim
//*****************************************************************************
typedef enum {
	UINT8 = 0,
	UINT16 = 1,
	INT8 = 2,
	INT16 = 3,
}enum_size;

typedef struct {
	int16_t* value;
	enum_size size;
}config_table_element;

config_table_element t7_power_config_table[] = { 
	{(int16_t*)&power_config.idleacqint, UINT8},
	{(int16_t*)&power_config.actvacqint, UINT8},
	{(int16_t*)&power_config.actv2idleto, UINT8},
	{(int16_t*)&power_config.cfg, UINT8},
};

config_table_element t8_acquisition_config_table[] = { 
	{(int16_t*)&acquisition_config.chrgtime, UINT8},
	{(int16_t*)&acquisition_config.reserved_0, UINT8},
	{(int16_t*)&acquisition_config.tchdrift, UINT8},
	{(int16_t*)&acquisition_config.driftst, UINT8},
	{(int16_t*)&acquisition_config.tchautocal, UINT8},
	{(int16_t*)&acquisition_config.sync, UINT8},
	{(int16_t*)&acquisition_config.atchcalst, UINT8},
	{(int16_t*)&acquisition_config.atchcalsthr, UINT8},
	{(int16_t*)&acquisition_config.atchfrccalthr, UINT8},
	{(int16_t*)&acquisition_config.atchfrccalratio, INT8},
	{(int16_t*)&acquisition_config.measallow, UINT8},
	{(int16_t*)&acquisition_config.measidledef, UINT8},
	{(int16_t*)&acquisition_config.measactivedef, UINT8},
	{(int16_t*)&acquisition_config.reserved_1, UINT8},

};

config_table_element t15_keyarray_config_table[] = { 
	{(int16_t*)&keyarray_config.ctrl, UINT8},
	{(int16_t*)&keyarray_config.xorigin, UINT8},
	{(int16_t*)&keyarray_config.yorigin, UINT8},
	{(int16_t*)&keyarray_config.xsize, UINT8},
	{(int16_t*)&keyarray_config.ysize, UINT8},
	{(int16_t*)&keyarray_config.akscfg, UINT8},
	{(int16_t*)&keyarray_config.blen, UINT8},
	{(int16_t*)&keyarray_config.tchthr, UINT8},
	{(int16_t*)&keyarray_config.tchdi, UINT8},
	{(int16_t*)&keyarray_config.reserved[0], UINT8},
	{(int16_t*)&keyarray_config.reserved[1], UINT8},
};

config_table_element t18_commsconfig_config_table[] = { 
	{(int16_t*)&comms_config.ctrl, UINT8},
	{(int16_t*)&comms_config.cmd, UINT8},
};

config_table_element t19_gpiopwm_config_table[] = { 
	{(int16_t*)&gpiopwm_config.ctrl, UINT8},
	{(int16_t*)&gpiopwm_config.reportmask, UINT8},
	{(int16_t*)&gpiopwm_config.dir, UINT8},	
	{(int16_t*)&gpiopwm_config.intpullup, UINT8},
	{(int16_t*)&gpiopwm_config.out, UINT8},
	{(int16_t*)&gpiopwm_config.wake, UINT8},
};

config_table_element t23_proximity_config_table[] = { 
	{(int16_t*)&proximity_config.ctrl, UINT8},
	{(int16_t*)&proximity_config.xorigin, UINT8},
	{(int16_t*)&proximity_config.yorigin, UINT8},	
	{(int16_t*)&proximity_config.xsize, UINT8},
	{(int16_t*)&proximity_config.ysize, UINT8},
	{(int16_t*)&proximity_config.reserved_0, UINT8},
	{(int16_t*)&proximity_config.blen, UINT8},	
	{(int16_t*)&proximity_config.fxddthr, UINT16},
	{0},
	{(int16_t*)&proximity_config.fxddi, UINT8},
	{(int16_t*)&proximity_config.average, UINT8},
	{(int16_t*)&proximity_config.mvnullrate, UINT16},
	{0},
	{(int16_t*)&proximity_config.mvdthr, UINT16},
	{0},
	{(int16_t*)&proximity_config.cfg, UINT8},
};



config_table_element t25_selftest_config_table[] = {
	{(int16_t*)&selftest_config.ctrl, UINT8},
	{(int16_t*)&selftest_config.cmd, UINT8},
#if(NUM_OF_TOUCH_OBJECTS)
	{(int16_t*)&selftest_config.siglim[0].upsiglim, UINT16},
	{0},
	{(int16_t*)&selftest_config.siglim[0].losiglim, UINT16},
	{0},
	{(int16_t*)&selftest_config.siglim[1].upsiglim, UINT16},
	{0},
	{(int16_t*)&selftest_config.siglim[1].losiglim, UINT16},
	{0},
	{(int16_t*)&selftest_config.siglim[2].upsiglim, UINT16},
	{0},
	{(int16_t*)&selftest_config.siglim[2].losiglim, UINT16},
	{0},
#endif
	{(int16_t*)&selftest_config.pindwellus, UINT8},
#if(NUM_OF_TOUCH_OBJECTS)
	{(int16_t*)&selftest_config.sigrangelim[0], UINT16},
	{0},
	{(int16_t*)&selftest_config.sigrangelim[1], UINT16},
	{0},
	{(int16_t*)&selftest_config.sigrangelim[2], UINT16},
	{0},
#endif
};

config_table_element t40_gripsuppression_config_table[] = {
	{(int16_t*)&gripsuppression_t40_config.ctrl, UINT8},
	{(int16_t*)&gripsuppression_t40_config.xlogrip, UINT8},
	{(int16_t*)&gripsuppression_t40_config.xhigrip, UINT8},
	{(int16_t*)&gripsuppression_t40_config.ylogrip, UINT8},
	{(int16_t*)&gripsuppression_t40_config.yhigrip, UINT8},
};
config_table_element t42_touchsuppression_config_table[] = {
	{(int16_t*)&touchsuppression_t42_config.ctrl, UINT8},
	{(int16_t*)&touchsuppression_t42_config.reserved, UINT8},
	{(int16_t*)&touchsuppression_t42_config.maxapprarea, UINT8},
	{(int16_t*)&touchsuppression_t42_config.maxtcharea, UINT8},
	{(int16_t*)&touchsuppression_t42_config.supstrength, UINT8},
	{(int16_t*)&touchsuppression_t42_config.supextto, UINT8},
	{(int16_t*)&touchsuppression_t42_config.maxnumtchs, UINT8},
	{(int16_t*)&touchsuppression_t42_config.shapestrength, UINT8},
	{(int16_t*)&touchsuppression_t42_config.supdist, UINT8},
	{(int16_t*)&touchsuppression_t42_config.disthyst, UINT8},
	{(int16_t*)&touchsuppression_t42_config.reserved_0, UINT8},
	{(int16_t*)&touchsuppression_t42_config.cfg, UINT8},
	{(int16_t*)&touchsuppression_t42_config.reserved_1, UINT8},

};
config_table_element t46_cteconfig_config_table[] = {
	{(int16_t*)&cte_t46_config.ctrl, UINT8},
	{(int16_t*)&cte_t46_config.reserved_0, UINT8},
	{(int16_t*)&cte_t46_config.idlesyncsperx, UINT8},
	{(int16_t*)&cte_t46_config.actvsyncsperx, UINT8},
	{(int16_t*)&cte_t46_config.adcspersync, UINT8},
	{(int16_t*)&cte_t46_config.pulsesperadc, UINT8}, 
	{(int16_t*)&cte_t46_config.xslew, UINT8},  
	{(int16_t*)&cte_t46_config.syncdelay, UINT16},
	{0},
	{(int16_t*)&cte_t46_config.xvoltage, UINT8}
};

config_table_element t47_stylus_config_table[] = {
	{(int16_t*)&stylus_t47_config.ctrl, UINT8},
	{(int16_t*)&stylus_t47_config.contmin, UINT8},
	{(int16_t*)&stylus_t47_config.contmax, UINT8},
	{(int16_t*)&stylus_t47_config.stability, UINT8},
	{(int16_t*)&stylus_t47_config.maxtcharea, UINT8},
	{(int16_t*)&stylus_t47_config.amplthr, UINT8},
	{(int16_t*)&stylus_t47_config.styshape, UINT8},
	{(int16_t*)&stylus_t47_config.hoversup, UINT8},
	{(int16_t*)&stylus_t47_config.confthr, UINT8},
	{(int16_t*)&stylus_t47_config.syncsperx, UINT8},
	{(int16_t*)&stylus_t47_config.xposadj, INT8},
	{(int16_t*)&stylus_t47_config.yposadj, INT8},
	{(int16_t*)&stylus_t47_config.cfg, UINT8},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{(int16_t*)&stylus_t47_config.supstyto, UINT8},
	{(int16_t*)&stylus_t47_config.maxnumsty, UINT8},
	{(int16_t*)&stylus_t47_config.xedgectrl, UINT8},
	{(int16_t*)&stylus_t47_config.xedgedist, UINT8},
	{(int16_t*)&stylus_t47_config.yedgectrl, UINT8},
	{(int16_t*)&stylus_t47_config.yedgedist, UINT8},
};

config_table_element t55_adaptivethreshold_config_table[] = {
	{(int16_t*)&proci_adaptivethreshold_t55_config.ctrl, UINT8},
	{(int16_t*)&proci_adaptivethreshold_t55_config.targetthr, UINT8},
	{(int16_t*)&proci_adaptivethreshold_t55_config.thradjlim, UINT8},
	{(int16_t*)&proci_adaptivethreshold_t55_config.resetsteptime, UINT8},
	{(int16_t*)&proci_adaptivethreshold_t55_config.forcechgdist, UINT8},
	{(int16_t*)&proci_adaptivethreshold_t55_config.forcechgtime, UINT8},
	{(int16_t*)&proci_adaptivethreshold_t55_config.lowestthr, INT8},
};

config_table_element t56_shieldless_config_table[] = {
	{(int16_t*)&proci_shieldless_t56_config.ctrl, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.reserved_0, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.optint, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.inttime, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay0, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay1, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay2, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay3, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay4, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay5, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay6, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay7, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay8, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay9, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay10, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay11, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay12, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay13, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay14, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay15, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay16, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay17, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay18, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay19, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay20, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay21, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay22, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay23, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay24, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay25, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay26, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay27, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay28, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.intdelay29, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.multicutgc, UINT8},
	{(int16_t*)&proci_shieldless_t56_config.gclimit, UINT8},
};

config_table_element t61_timer_config[] = {
	{(int16_t*)&spt_timer_t61_config.ctrl, UINT8},
	{(int16_t*)&spt_timer_t61_config.cmd, UINT8},
	{(int16_t*)&spt_timer_t61_config.mode, UINT8},
	{(int16_t*)&spt_timer_t61_config.period, UINT16},
	{0},
};

config_table_element t65_lensbending_config_table[] = {
	{(int16_t*)&lensbending_t65_config.ctrl, UINT8},
	{(int16_t*)&lensbending_t65_config.gradthr, UINT8},
	{(int16_t*)&lensbending_t65_config.ylonoisemul, UINT16},
	{0},
	{(int16_t*)&lensbending_t65_config.ylonoisediv, UINT16},
	{0},
	{(int16_t*)&lensbending_t65_config.yhinoisemul, UINT16},
	{0},
	{(int16_t*)&lensbending_t65_config.yhinoisediv, UINT16},
	{0},
	{(int16_t*)&lensbending_t65_config.lpfiltcoef, UINT8},
	{(int16_t*)&lensbending_t65_config.forcescale, UINT16},
	{0},
	{(int16_t*)&lensbending_t65_config.forcethr, UINT8},
	{(int16_t*)&lensbending_t65_config.forcethrhyst, UINT8},
	{(int16_t*)&lensbending_t65_config.forcedi, UINT8},
	{(int16_t*)&lensbending_t65_config.forcehyst, UINT8},
	{(int16_t*)&lensbending_t65_config.atchratio, UINT8},
	{(int16_t*)&lensbending_t65_config.reserved_0, UINT8},
	{(int16_t*)&lensbending_t65_config.reserved_1, UINT8},
};

config_table_element t66_goldenreferences_config_table[] = {
	{(int16_t*)&goldenreferences_t66_config.ctrl, UINT8},
	{(int16_t*)&goldenreferences_t66_config.fcalfailthr, UINT8},
	{(int16_t*)&goldenreferences_t66_config.fcaldriftcnt, UINT8},
};

config_table_element t69_palmgestureprocessor_config_table[] = {
	{(int16_t*)&palmgestureprocessor_t69_config.ctrl, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.longdimthr, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.shortdimthr, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.longdimhyst, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.shortdimhyst, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.movthr, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.movthrto, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.areathr, UINT8},
	{(int16_t*)&palmgestureprocessor_t69_config.areathrto, UINT8},
};

config_table_element t70_dynamicconfigurationcontroller_config_table[] = {
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.ctrl, UINT8},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.event, UINT16},
	{0},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.objtype, UINT8},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.reserved_0, UINT8},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.objinst, UINT8},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.dstoffset, UINT8},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.srcoffset, UINT16},
	{0},
	{(int16_t*)&dynamicconfigurationcontroller_t70_config.length, UINT8},
};

config_table_element t72_noisesuppression_config_table[] = {
	{(int16_t*)&noisesuppression_t72_config.ctrl, UINT8},
	{(int16_t*)&noisesuppression_t72_config.calcfg1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.cfg1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.cfg2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.hopcnt, UINT8},
	{(int16_t*)&noisesuppression_t72_config.hopcntper, UINT8},
	{(int16_t*)&noisesuppression_t72_config.hopevalto, UINT8},
	{(int16_t*)&noisesuppression_t72_config.hopst, UINT8},
	{(int16_t*)&noisesuppression_t72_config.nlgaindualx, UINT8},
	{(int16_t*)&noisesuppression_t72_config.minnlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.incnlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.fallnlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.nlthrmargin, UINT8},
	{(int16_t*)&noisesuppression_t72_config.minthradj, UINT8},
	{(int16_t*)&noisesuppression_t72_config.nlthrlimit, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.nlgainsingx, UINT8},
	{(int16_t*)&noisesuppression_t72_config.blknlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabctrl, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabfreq_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabfreq_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabfreq_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabfreq_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabfreq_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabtchapx_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabtchapx_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabtchapx_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabtchapx_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabtchapx_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabnotchapx_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabnotchapx_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabnotchapx_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabnotchapx_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabnotchapx_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.stabhighnlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_5, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisctrl, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisfreq_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisfreq_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisfreq_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisfreq_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisfreq_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noistchapx_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noistchapx_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noistchapx_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noistchapx_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noistchapx_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisnotchapx_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisnotchapx_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisnotchapx_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisnotchapx_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noisnotchapx_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_6, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noislownlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noishighnlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.noiscnt, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoictrl, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoifreq_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoifreq_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoifreq_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoifreq_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoifreq_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoitchapx_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoitchapx_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoitchapx_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoitchapx_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoitchapx_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoinotchapx_0, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoinotchapx_1, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoinotchapx_2, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoinotchapx_3, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoinotchapx_4, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_7, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoilownlthr, UINT8},
	{(int16_t*)&noisesuppression_t72_config.reserved_8, UINT8},
	{(int16_t*)&noisesuppression_t72_config.vnoicnt, UINT8},
};

config_table_element t78_glovedetection_config_table[] = {
	{(int16_t*)&glovedetection_t78_config.ctrl, UINT8},
	{(int16_t*)&glovedetection_t78_config.minarea, UINT8},
	{(int16_t*)&glovedetection_t78_config.confthr, UINT8},
	{(int16_t*)&glovedetection_t78_config.mindist, UINT8},
	{(int16_t*)&glovedetection_t78_config.glovemodeto, UINT8},
	{(int16_t*)&glovedetection_t78_config.supto, UINT8},
	{(int16_t*)&glovedetection_t78_config.syncsperx, UINT8},
	{(int16_t*)&glovedetection_t78_config.hithrmargin, UINT8},
};


config_table_element t80_retransmissioncompensation_config_table[] = {
	{(int16_t*)&retransmissioncompensation_t80_config.ctrl, UINT8},
	{(int16_t*)&retransmissioncompensation_t80_config.compgain, UINT8},
	{(int16_t*)&retransmissioncompensation_t80_config.targetdelta, UINT8},
	{(int16_t*)&retransmissioncompensation_t80_config.compthr, UINT8},
	{(int16_t*)&retransmissioncompensation_t80_config.atchthr, UINT8},
	{(int16_t*)&retransmissioncompensation_t80_config.moistcfg, UINT8},
	{(int16_t*)&retransmissioncompensation_t80_config.moistdto, UINT8},
};

config_table_element t84_gesturepocessor_config_table[] = {
  {(int16_t*)&gesturepocessor_t84_config.ctrl, UINT8},
  {(int16_t*)&gesturepocessor_t84_config.zonethr, UINT8}, 
  {(int16_t*)&gesturepocessor_t84_config.direl, UINT8},
  {(int16_t*)&gesturepocessor_t84_config.dto, UINT8},
};

config_table_element t100_touchscreen_config_table[] = {
	{(int16_t*)&touchscreen_config.ctrl, UINT8},
	{(int16_t*)&touchscreen_config.cfg1, UINT8},
	{(int16_t*)&touchscreen_config.scraux, UINT8},
	{(int16_t*)&touchscreen_config.tchaux, UINT8},
	{(int16_t*)&touchscreen_config.tcheventcfg, UINT8},
	{(int16_t*)&touchscreen_config.akscfg, UINT8},
	{(int16_t*)&touchscreen_config.numtch, UINT8},
	{(int16_t*)&touchscreen_config.xycfg, UINT8},
	{(int16_t*)&touchscreen_config.xorigin, UINT8},
	{(int16_t*)&touchscreen_config.xsize, UINT8},
	{(int16_t*)&touchscreen_config.xpitch, UINT8},
	{(int16_t*)&touchscreen_config.xloclip, INT8},
	{(int16_t*)&touchscreen_config.xhiclip, INT8},
	{(int16_t*)&touchscreen_config.xrange, UINT16},
	{0},
	{(int16_t*)&touchscreen_config.xedgecfg, UINT8},
	{(int16_t*)&touchscreen_config.xedgedist, UINT8},
	{(int16_t*)&touchscreen_config.dxxedgecfg, UINT8},
	{(int16_t*)&touchscreen_config.dxxedgedist, UINT8},
	{(int16_t*)&touchscreen_config.yorigin, UINT8},
	{(int16_t*)&touchscreen_config.ysize, UINT8},
	{(int16_t*)&touchscreen_config.ypitch, UINT8},
	{(int16_t*)&touchscreen_config.yloclip, INT8},
	{(int16_t*)&touchscreen_config.yhiclip, INT8},
	{(int16_t*)&touchscreen_config.yrange, UINT16},
	{0},
	{(int16_t*)&touchscreen_config.yedgecfg, UINT8},
	{(int16_t*)&touchscreen_config.yedgedist, UINT8},
	{(int16_t*)&touchscreen_config.gain, UINT8},
	{(int16_t*)&touchscreen_config.dxgain, UINT8},
	{(int16_t*)&touchscreen_config.tchthr, UINT8},
	{(int16_t*)&touchscreen_config.tchhyst, UINT8},
	{(int16_t*)&touchscreen_config.intthr, UINT8},
	{(int16_t*)&touchscreen_config.noisesf, UINT8},
	{(int16_t*)&touchscreen_config.cutoffthr, UINT8},
	{(int16_t*)&touchscreen_config.mrgthr, UINT8},
	{(int16_t*)&touchscreen_config.mrgthradjstr, UINT8},
	{(int16_t*)&touchscreen_config.mrghyst, UINT8},
	{(int16_t*)&touchscreen_config.dxthrsf, UINT8},
	{(int16_t*)&touchscreen_config.tchdidown, UINT8},
	{(int16_t*)&touchscreen_config.tchdiup, UINT8},
	{(int16_t*)&touchscreen_config.nexttchdi, UINT8},
	{(int16_t*)&touchscreen_config.reserved, UINT8},
	{(int16_t*)&touchscreen_config.jumplimit, UINT8},
	{(int16_t*)&touchscreen_config.movfilter, UINT8},
	{(int16_t*)&touchscreen_config.movsmooth, UINT8},
	{(int16_t*)&touchscreen_config.movpred, UINT8},
	{(int16_t*)&touchscreen_config.movhysti, UINT16},
	{0},
	{(int16_t*)&touchscreen_config.movhystn, UINT16},
	{0},
	{(int16_t*)&touchscreen_config.amplhyst, UINT8},
	{(int16_t*)&touchscreen_config.scrareahyst, UINT8},
	{(int16_t*)&touchscreen_config.intthrhyst, UINT8},
};

config_table_element t101_touchscreenhover_config_table[] = {
  {(int16_t*)&touchscreenhover_t101_config.ctrl, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xloclip, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xhiclip, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xedgecfg, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xedgedist, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xgain, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xhvrthr, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.xhvrhyst, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.yloclip, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.yhiclip, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.yedgecfg, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.yedgedist, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.ygain, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.yhvthr, UINT8},  
  {(int16_t*)&touchscreenhover_t101_config.yhvrhyst, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.hvrdi, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.confthr, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.movfilter,  UINT8},
  {(int16_t*)&touchscreenhover_t101_config.movsmooth,  UINT8},
  {(int16_t*)&touchscreenhover_t101_config.movpred, UINT8},
  {(int16_t*)&touchscreenhover_t101_config.movhysti, UINT16},
  {0},
  {(int16_t*)&touchscreenhover_t101_config.movhystn, UINT16},
  {0},
  {(int16_t*)&touchscreenhover_t101_config.hvraux,  UINT8},
};



config_table_element t102_selfcapcbcrconfig_config_table[] = {
	{(int16_t*)&selfcapcbcrconfig_t102_config.ctrl, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.cmd, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.mode, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.tunthr, UINT16},
	{0},
	{(int16_t*)&selfcapcbcrconfig_t102_config.tunhyst, UINT16},
	{0},
	{(int16_t*)&selfcapcbcrconfig_t102_config.reserved_0, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.tuncfg, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.tunsyncsperl, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.tungain, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.reserved_1, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.prechrgtime, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.chrgtime, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.inttime, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.reserved_2, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.reserved_3, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.reserved_4, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.idlesyncsperl, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.actvsyncsperl, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.drift, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.driftst, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.reserved_5, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.filter, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.filtcfg, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.dyniirthru, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.dyniirthrl, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.dyniirclmp, UINT8},
	{(int16_t*)&selfcapcbcrconfig_t102_config.recalcfg, UINT8},
};

config_table_element t103_schnoisesuppression_config_table[] = {
	{(int16_t*)&schnoisesuppression_t103_config.ctrl, UINT8},
	{(int16_t*)&schnoisesuppression_t103_config.cfg, UINT8},
	{(int16_t*)&schnoisesuppression_t103_config.reserved_0, UINT8},
	{(int16_t*)&schnoisesuppression_t103_config.procst, UINT8},
	{(int16_t*)&schnoisesuppression_t103_config.schsiglothr, UINT16},
	{0},
	{(int16_t*)&schnoisesuppression_t103_config.schsighithr, UINT16},
	{0},
	{(int16_t*)&schnoisesuppression_t103_config.nlxgain, UINT8},
	{(int16_t*)&schnoisesuppression_t103_config.nlygain, UINT8},
	{(int16_t*)&schnoisesuppression_t103_config.nllothr, UINT16},
	{0},
	{(int16_t*)&schnoisesuppression_t103_config.nlhicthr, UINT16},
	{0},
};

config_table_element t104_auxtouchconfig_config_table[] = {
	{(int16_t*)&auxtouchconfig_t104_config.ctrl, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.xgain, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.xtchthr, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.xtchhyst, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.xintthr, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.xinthyst, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.ygain, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.ytchthr, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.ytchhyst, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.yintthr, UINT8},
	{(int16_t*)&auxtouchconfig_t104_config.yinthyst, UINT8},
};

config_table_element t105_drivenplatehoverconfig_config_table[] = {
  {(int16_t*)&drivenplatehoverconfig_t105_config.ctrl, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.mode, UINT8},	
  {(int16_t*)&drivenplatehoverconfig_t105_config.prechrgtime, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.chrgtime, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.inttime, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.reserved_0, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.reserved_1, UINT8},	
  {(int16_t*)&drivenplatehoverconfig_t105_config.reserved_2, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.idlesyncsperl, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.actvsyncsperl, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.drift, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.driftst, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.driftsthrsf, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.filter, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.filtcfg, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.dyniirthru, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.dyniirthrl, UINT8},
  {(int16_t*)&drivenplatehoverconfig_t105_config.dyniirclmp, UINT8},
};


config_table_element* config_table[150] = {
	[7] = (config_table_element*) &t7_power_config_table,
	[8] = (config_table_element*) &t8_acquisition_config_table,
	[15] = (config_table_element*) &t15_keyarray_config_table,
	[18] = (config_table_element*) &t18_commsconfig_config_table,
	[19] = (config_table_element*) &t19_gpiopwm_config_table,
	[23] = (config_table_element*) &t23_proximity_config_table,
	[25] = (config_table_element*) &t25_selftest_config_table,
	[40] = (config_table_element*) &t40_gripsuppression_config_table,
	[42] = (config_table_element*) &t42_touchsuppression_config_table,
	[46] = (config_table_element*) &t46_cteconfig_config_table,
	[47] = (config_table_element*) &t47_stylus_config_table,
	[55] = (config_table_element*) &t55_adaptivethreshold_config_table,
	[56] = (config_table_element*) &t56_shieldless_config_table,
	[61] = (config_table_element*) &t61_timer_config,
	[65] = (config_table_element*) &t65_lensbending_config_table,
	[66] = (config_table_element*) &t66_goldenreferences_config_table,
	[69] = (config_table_element*) &t69_palmgestureprocessor_config_table,
	[70] = (config_table_element*) &t70_dynamicconfigurationcontroller_config_table,	
	[72] = (config_table_element*) &t72_noisesuppression_config_table,
	[78] = (config_table_element*) &t78_glovedetection_config_table,
	[100] = (config_table_element*) &t100_touchscreen_config_table,
	[80] = (config_table_element*) &t80_retransmissioncompensation_config_table,
  [84] = (config_table_element*) &t84_gesturepocessor_config_table,
  [101] = (config_table_element*) &t101_touchscreenhover_config_table,  
	[102] = (config_table_element*) &t102_selfcapcbcrconfig_config_table,
	[103] = (config_table_element*) &t103_schnoisesuppression_config_table,
	[104] = (config_table_element*) &t104_auxtouchconfig_config_table,
	[105] = (config_table_element*) &t105_drivenplatehoverconfig_config_table,
};

#endif //_MXT_FW30_H_
