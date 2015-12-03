
/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/spmi.h>
#include <linux/qpnp/pwm.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>

//++ p11309 - 2014.01.03 for Offline Charger
#ifdef PAN_LED_CONTROL
#include <mach/msm_smsm.h>
#endif
//-- p11309

#define WLED_MOD_EN_REG(base, n)	(base + 0x60 + n*0x10)
#define WLED_IDAC_DLY_REG(base, n)	(WLED_MOD_EN_REG(base, n) + 0x01)
#define WLED_FULL_SCALE_REG(base, n)	(WLED_IDAC_DLY_REG(base, n) + 0x01)
#define WLED_MOD_SRC_SEL_REG(base, n)	(WLED_FULL_SCALE_REG(base, n) + 0x01)

/* wled control registers */
#define WLED_BRIGHTNESS_CNTL_LSB(base, n)	(base + 0x40 + 2*n)
#define WLED_BRIGHTNESS_CNTL_MSB(base, n)	(base + 0x41 + 2*n)
#define WLED_MOD_CTRL_REG(base)			(base + 0x46)
#define WLED_SYNC_REG(base)			(base + 0x47)
#define WLED_FDBCK_CTRL_REG(base)		(base + 0x48)
#define WLED_SWITCHING_FREQ_REG(base)		(base + 0x4C)
#define WLED_OVP_CFG_REG(base)			(base + 0x4D)
#define WLED_BOOST_LIMIT_REG(base)		(base + 0x4E)
#define WLED_CURR_SINK_REG(base)		(base + 0x4F)
#define WLED_HIGH_POLE_CAP_REG(base)		(base + 0x58)
#define WLED_CURR_SINK_MASK		0xE0
#define WLED_CURR_SINK_SHFT		0x05
#define WLED_DISABLE_ALL_SINKS		0x00
#define WLED_SWITCH_FREQ_MASK		0x0F
#define WLED_OVP_VAL_MASK		0x03
#define WLED_OVP_VAL_BIT_SHFT		0x00
#define WLED_BOOST_LIMIT_MASK		0x07
#define WLED_BOOST_LIMIT_BIT_SHFT	0x00
#define WLED_BOOST_ON			0x80
#define WLED_BOOST_OFF			0x00
#define WLED_EN_MASK			0x80
#define WLED_NO_MASK			0x00
#define WLED_CP_SELECT_MAX		0x03
#define WLED_CP_SELECT_MASK		0x02
#define WLED_USE_EXT_GEN_MOD_SRC	0x01
#define WLED_CTL_DLY_STEP		200
#define WLED_CTL_DLY_MAX		1400
#define WLED_MAX_CURR			25
#define WLED_NO_CURRENT			0x00
#define WLED_OVP_DELAY			1000
#define WLED_MSB_MASK			0x0F
#define WLED_MAX_CURR_MASK		0x1F
#define WLED_OP_FDBCK_MASK		0x07
#define WLED_OP_FDBCK_BIT_SHFT		0x00
#define WLED_OP_FDBCK_DEFAULT		0x00

#define WLED_MAX_LEVEL			4095
#define WLED_8_BIT_MASK			0xFF
#define WLED_4_BIT_MASK			0x0F
#define WLED_8_BIT_SHFT			0x08
#define WLED_MAX_DUTY_CYCLE		0xFFF

#define WLED_SYNC_VAL			0x07
#define WLED_SYNC_RESET_VAL		0x00

#define PMIC_VER_8026			0x04
#define PMIC_VERSION_REG		0x0105

#define WLED_DEFAULT_STRINGS		0x01
#define WLED_DEFAULT_OVP_VAL		0x02
#define WLED_BOOST_LIM_DEFAULT		0x03
#define WLED_CP_SEL_DEFAULT		0x00
#define WLED_CTRL_DLY_DEFAULT		0x00
#define WLED_SWITCH_FREQ_DEFAULT	0x0B

#define FLASH_SAFETY_TIMER(base)	(base + 0x40)
#define FLASH_MAX_CURR(base)		(base + 0x41)
#define FLASH_LED_0_CURR(base)		(base + 0x42)
#define FLASH_LED_1_CURR(base)		(base + 0x43)
#define FLASH_CLAMP_CURR(base)		(base + 0x44)
#define FLASH_LED_TMR_CTRL(base)	(base + 0x48)
#define FLASH_HEADROOM(base)		(base + 0x4A)
#define FLASH_STARTUP_DELAY(base)	(base + 0x4B)
#define FLASH_MASK_ENABLE(base)		(base + 0x4C)
#define FLASH_VREG_OK_FORCE(base)	(base + 0x4F)
#define FLASH_ENABLE_CONTROL(base)	(base + 0x46)
#define FLASH_LED_STROBE_CTRL(base)	(base + 0x47)
#define FLASH_LED_UNLOCK_SECURE(base)	(base + 0xD0)
#define FLASH_LED_TORCH(base)		(base + 0xE4)
#define FLASH_FAULT_DETECT(base)	(base + 0x51)
#define FLASH_PERIPHERAL_SUBTYPE(base)	(base + 0x05)
#define FLASH_CURRENT_RAMP(base)	(base + 0x54)

#define FLASH_MAX_LEVEL			0x4F
#define TORCH_MAX_LEVEL			0x0F
#define	FLASH_NO_MASK			0x00

#define FLASH_MASK_1			0x20
#define FLASH_MASK_REG_MASK		0xE0
#define FLASH_HEADROOM_MASK		0x03
#define FLASH_SAFETY_TIMER_MASK		0x7F
#define FLASH_CURRENT_MASK		0xFF
#define FLASH_MAX_CURRENT_MASK		0x7F
#define FLASH_TMR_MASK			0x03
#define FLASH_TMR_WATCHDOG		0x03
#define FLASH_TMR_SAFETY		0x00
#define FLASH_FAULT_DETECT_MASK		0X80
#define FLASH_HW_VREG_OK		0x40
#define FLASH_VREG_MASK			0xC0
#define FLASH_STARTUP_DLY_MASK		0x02
#define FLASH_CURRENT_RAMP_MASK		0xBF

#define FLASH_ENABLE_ALL		0xE0
#define FLASH_ENABLE_MODULE		0x80
#define FLASH_ENABLE_MODULE_MASK	0x80
#define FLASH_DISABLE_ALL		0x00
#define FLASH_ENABLE_MASK		0xE0
#define FLASH_ENABLE_LED_0		0xC0
#define FLASH_ENABLE_LED_1		0xA0
#define FLASH_INIT_MASK			0xE0
#define	FLASH_SELFCHECK_ENABLE		0x80
#define FLASH_RAMP_STEP_27US		0xBF

#define FLASH_HW_SW_STROBE_SEL_MASK	0x04
#define FLASH_STROBE_MASK		0xC7
#define FLASH_LED_0_OUTPUT		0x80
#define FLASH_LED_1_OUTPUT		0x40
#define FLASH_TORCH_OUTPUT		0xC0

#define FLASH_CURRENT_PRGM_MIN		1
#define FLASH_CURRENT_PRGM_SHIFT	1
#define FLASH_CURRENT_MAX		0x4F
#define FLASH_CURRENT_TORCH		0x07

#define FLASH_DURATION_200ms		0x13
#define FLASH_CLAMP_200mA		0x0F

#define FLASH_TORCH_MASK		0x03
#define FLASH_LED_TORCH_ENABLE		0x00
#define FLASH_LED_TORCH_DISABLE		0x03
#define FLASH_UNLOCK_SECURE		0xA5
#define FLASH_SECURE_MASK		0xFF

#define FLASH_SUBTYPE_DUAL		0x01
#define FLASH_SUBTYPE_SINGLE		0x02

#define FLASH_RAMP_UP_DELAY_US		1000
#define FLASH_RAMP_DN_DELAY_US		2160

#define LED_TRIGGER_DEFAULT		"none"

#define RGB_LED_SRC_SEL(base)		(base + 0x45)
#define RGB_LED_EN_CTL(base)		(base + 0x46)
#define RGB_LED_ATC_CTL(base)		(base + 0x47)

#define RGB_MAX_LEVEL			LED_FULL
#define RGB_LED_ENABLE_RED		0x80
#define RGB_LED_ENABLE_GREEN		0x40
#define RGB_LED_ENABLE_BLUE		0x20
#define RGB_LED_SOURCE_VPH_PWR		0x01
#define RGB_LED_ENABLE_MASK		0xE0
#define RGB_LED_SRC_MASK		0x03
#define QPNP_LED_PWM_FLAGS	(PM_PWM_LUT_LOOP | PM_PWM_LUT_RAMP_UP)
#define QPNP_LUT_RAMP_STEP_DEFAULT	255
#define	PWM_LUT_MAX_SIZE		63
#define	PWM_GPLED_LUT_MAX_SIZE		31
#define RGB_LED_DISABLE			0x00

#ifdef CONFIG_PANTECH_CAMERA //flash
bool global_torch_enable;
#endif
#define MPP_MAX_LEVEL			LED_FULL
#define LED_MPP_MODE_CTRL(base)		(base + 0x40)
#define LED_MPP_VIN_CTRL(base)		(base + 0x41)
#define LED_MPP_EN_CTRL(base)		(base + 0x46)
#define LED_MPP_SINK_CTRL(base)		(base + 0x4C)

#define LED_MPP_CURRENT_MIN		5
#define LED_MPP_CURRENT_MAX		40
#define LED_MPP_VIN_CTRL_DEFAULT	0
#define LED_MPP_CURRENT_PER_SETTING	5
#define LED_MPP_SOURCE_SEL_DEFAULT	LED_MPP_MODE_ENABLE

#define LED_MPP_SINK_MASK		0x07
#define LED_MPP_MODE_MASK		0x7F
#define LED_MPP_VIN_MASK		0x03
#define LED_MPP_EN_MASK			0x80
#define LED_MPP_SRC_MASK		0x0F
#define LED_MPP_MODE_CTRL_MASK		0x70

#define LED_MPP_MODE_SINK		(0x06 << 4)
#define LED_MPP_MODE_ENABLE		0x01
#define LED_MPP_MODE_OUTPUT		0x10
#define LED_MPP_MODE_DISABLE		0x00
#define LED_MPP_EN_ENABLE		0x80
#define LED_MPP_EN_DISABLE		0x00

#define MPP_SOURCE_DTEST1		0x08

#define KPDBL_MAX_LEVEL			LED_FULL
#define KPDBL_ROW_SRC_SEL(base)		(base + 0x40)
#define KPDBL_ENABLE(base)		(base + 0x46)
#define KPDBL_ROW_SRC(base)		(base + 0xE5)

#define KPDBL_ROW_SRC_SEL_VAL_MASK	0x0F
#define KPDBL_ROW_SCAN_EN_MASK		0x80
#define KPDBL_ROW_SCAN_VAL_MASK		0x0F
#define KPDBL_ROW_SCAN_EN_SHIFT		7
#define KPDBL_MODULE_EN			0x80
#define KPDBL_MODULE_DIS		0x00
#define KPDBL_MODULE_EN_MASK		0x80

/**
 * enum qpnp_leds - QPNP supported led ids
 * @QPNP_ID_WLED - White led backlight
 */
enum qpnp_leds {
	QPNP_ID_WLED = 0,
	QPNP_ID_FLASH1_LED0,
	QPNP_ID_FLASH1_LED1,
	QPNP_ID_RGB_RED,
	QPNP_ID_RGB_GREEN,
	QPNP_ID_RGB_BLUE,
	QPNP_ID_LED_MPP,
	QPNP_ID_KPDBL,
	QPNP_ID_MAX,
};

/* current boost limit */
enum wled_current_boost_limit {
	WLED_CURR_LIMIT_105mA,
	WLED_CURR_LIMIT_385mA,
	WLED_CURR_LIMIT_525mA,
	WLED_CURR_LIMIT_805mA,
	WLED_CURR_LIMIT_980mA,
	WLED_CURR_LIMIT_1260mA,
	WLED_CURR_LIMIT_1400mA,
	WLED_CURR_LIMIT_1680mA,
};

/* over voltage protection threshold */
enum wled_ovp_threshold {
	WLED_OVP_35V,
	WLED_OVP_32V,
	WLED_OVP_29V,
	WLED_OVP_27V,
};

enum flash_headroom {
	HEADROOM_250mV = 0,
	HEADROOM_300mV,
	HEADROOM_400mV,
	HEADROOM_500mV,
};

enum flash_startup_dly {
	DELAY_10us = 0,
	DELAY_32us,
	DELAY_64us,
	DELAY_128us,
};

enum led_mode {
	PWM_MODE = 0,
	LPG_MODE,
	MANUAL_MODE,
};

static u8 wled_debug_regs[] = {
	/* common registers */
	0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	/* LED1 */
	0x60, 0x61, 0x62, 0x63, 0x66,
	/* LED2 */
	0x70, 0x71, 0x72, 0x73, 0x76,
	/* LED3 */
	0x80, 0x81, 0x82, 0x83, 0x86,
};

static u8 flash_debug_regs[] = {
	0x40, 0x41, 0x42, 0x43, 0x44, 0x48, 0x49, 0x4b, 0x4c,
	0x4f, 0x46, 0x47,
};

static u8 rgb_pwm_debug_regs[] = {
	0x45, 0x46, 0x47,
};

static u8 mpp_debug_regs[] = {
	0x40, 0x41, 0x42, 0x45, 0x46, 0x4c,
};

static u8 kpdbl_debug_regs[] = {
	0x40, 0x46, 0xb1, 0xb3, 0xb4, 0xe5,
};

/**
 *  pwm_config_data - pwm configuration data
 *  @lut_params - lut parameters to be used by pwm driver
 *  @pwm_device - pwm device
 *  @pwm_channel - pwm channel to be configured for led
 *  @pwm_period_us - period for pwm, in us
 *  @mode - mode the led operates in
 *  @old_duty_pcts - storage for duty pcts that may need to be reused
 *  @default_mode - default mode of LED as set in device tree
 *  @use_blink - use blink sysfs entry
 *  @blinking - device is currently blinking w/LPG mode
 */
struct pwm_config_data {
	struct lut_params	lut_params;
	struct pwm_device	*pwm_dev;
	int			pwm_channel;
	u32			pwm_period_us;
	struct pwm_duty_cycles	*duty_cycles;
	int	*old_duty_pcts;
	u8	mode;
	u8	default_mode;
	bool use_blink;
	bool blinking;
};

/**
 *  wled_config_data - wled configuration data
 *  @num_strings - number of wled strings supported
 *  @ovp_val - over voltage protection threshold
 *  @boost_curr_lim - boot current limit
 *  @cp_select - high pole capacitance
 *  @ctrl_delay_us - delay in activation of led
 *  @dig_mod_gen_en - digital module generator
 *  @cs_out_en - current sink output enable
 *  @op_fdbck - selection of output as feedback for the boost
 */
struct wled_config_data {
	u8	num_strings;
	u8	ovp_val;
	u8	boost_curr_lim;
	u8	cp_select;
	u8	ctrl_delay_us;
	u8	switch_freq;
	u8	op_fdbck;
	u8	pmic_version;
	bool	dig_mod_gen_en;
	bool	cs_out_en;
};

/**
 *  mpp_config_data - mpp configuration data
 *  @pwm_cfg - device pwm configuration
 *  @current_setting - current setting, 5ma-40ma in 5ma increments
 *  @source_sel - source selection
 *  @mode_ctrl - mode control
 *  @vin_ctrl - input control
 *  @min_brightness - minimum brightness supported
 *  @pwm_mode - pwm mode in use
 */
struct mpp_config_data {
	struct pwm_config_data	*pwm_cfg;
	u8	current_setting;
	u8	source_sel;
	u8	mode_ctrl;
	u8	vin_ctrl;
	u8	min_brightness;
	u8 pwm_mode;
};

/**
 *  flash_config_data - flash configuration data
 *  @current_prgm - current to be programmed, scaled by max level
 *  @clamp_curr - clamp current to use
 *  @headroom - headroom value to use
 *  @duration - duration of the flash
 *  @enable_module - enable address for particular flash
 *  @trigger_flash - trigger flash
 *  @startup_dly - startup delay for flash
 *  @strobe_type - select between sw and hw strobe
 *  @peripheral_subtype - module peripheral subtype
 *  @current_addr - address to write for current
 *  @second_addr - address of secondary flash to be written
 *  @safety_timer - enable safety timer or watchdog timer
 *  @torch_enable - enable flash LED torch mode
 *  @flash_reg_get - flash regulator attached or not
 *  @flash_on - flash status, on or off
 *  @torch_on - torch status, on or off
 *  @flash_boost_reg - boost regulator for flash
 *  @torch_boost_reg - boost regulator for torch
 */
struct flash_config_data {
	u8	current_prgm;
	u8	clamp_curr;
	u8	headroom;
	u8	duration;
	u8	enable_module;
	u8	trigger_flash;
	u8	startup_dly;
	u8	strobe_type;
	u8	peripheral_subtype;
	u16	current_addr;
	u16	second_addr;
	bool	safety_timer;
	bool	torch_enable;
	bool	flash_reg_get;
	bool	flash_on;
	bool	torch_on;
	struct regulator *flash_boost_reg;
	struct regulator *torch_boost_reg;
};

/**
 *  kpdbl_config_data - kpdbl configuration data
 *  @pwm_cfg - device pwm configuration
 *  @mode - running mode: pwm or lut
 *  @row_id - row id of the led
 *  @row_src_vbst - 0 for vph_pwr and 1 for vbst
 *  @row_src_en - enable row source
 *  @always_on - always on row
 *  @lut_params - lut parameters to be used by pwm driver
 *  @duty_cycles - duty cycles for lut
 */
struct kpdbl_config_data {
	struct pwm_config_data	*pwm_cfg;
	u32	row_id;
	bool	row_src_vbst;
	bool	row_src_en;
	bool	always_on;
	struct pwm_duty_cycles  *duty_cycles;
	struct lut_params	lut_params;
};

/**
 *  rgb_config_data - rgb configuration data
 *  @pwm_cfg - device pwm configuration
 *  @enable - bits to enable led
 */
struct rgb_config_data {
	struct pwm_config_data	*pwm_cfg;
	u8	enable;
};

/**
 * struct qpnp_led_data - internal led data structure
 * @led_classdev - led class device
 * @delayed_work - delayed work for turning off the LED
 * @work - workqueue for led
 * @id - led index
 * @base_reg - base register given in device tree
 * @lock - to protect the transactions
 * @reg - cached value of led register
 * @num_leds - number of leds in the module
 * @max_current - maximum current supported by LED
 * @default_on - true: default state max, false, default state 0
 * @turn_off_delay_ms - number of msec before turning off the LED
 */
struct qpnp_led_data {
	struct led_classdev	cdev;
	struct spmi_device	*spmi_dev;
	struct delayed_work	dwork;
	struct work_struct	work;
	int			id;
	u16			base;
	u8			reg;
	u8			num_leds;
	struct mutex		lock;
	struct wled_config_data *wled_cfg;
	struct flash_config_data	*flash_cfg;
	struct kpdbl_config_data	*kpdbl_cfg;
	struct rgb_config_data	*rgb_cfg;
	struct mpp_config_data	*mpp_cfg;
	int			max_current;
	bool			default_on;
	int			turn_off_delay_ms;
};

static int num_kpbl_leds_on;


//=================================================================
//++ p11309 - 2013.12.03 for LED Pattern Customization
#ifdef PAN_LED_CONTROL

#define PAN_RAINBOW_SETUP_TIME		60
#define PAN_RAINBOW_KEEP_TIME		60
#define PAN_LUT_MAX_SIZE			61

#define PAN_PATTERN_MAX_SIZE_10		11
#define PAN_PATTERN_MAX_SIZE_30		31
#define PAN_PATTERN_CHARGING_CNT	3

#define PAN_MAX_RAMP_STEP_MS		500
#define PAN_WHITE_BRIGHTNESS		60
#define PAN_RGB_BRIGHTNESS_WEIGHT	50/100

#define PAN_OFFLINE_CHECK_INTERVAL	3000

//++ p11309 - 2014.01.03 for Offline Charger
enum pan_offline_charging_type {
	PAN_BATT_NOT_DEFINED = 0,
	PAN_BATT_CHARGING = 1,
	PAN_BATT_DISCHARGING = 2,
	PAN_BATT_NOT_CHARGING = 3,
	PAN_BATT_FULL = 4,
};

//-- p11309
enum pan_rgb_id_type {
	RGB_RED = 0,
	RGB_GREEN = 1,
	RGB_BLUE = 2,
	RGB_MAX = 3,
};

enum pan_rgb_status {
	PAN_LED_OFF = 0,
	PAN_LED_ONESHOT,
	PAN_LED_LOOP,
	PAN_LED_BLINK,
	PAN_LED_RAINBOW,
	PAN_LED_BRIGHTNESS,
	PAN_LED_STATUS_MAX
};

enum pan_lut_rise_type {
	PAN_LUT_RISE_MODE_LOG	= 0,
	PAN_LUT_RISE_MODE_LINEAR = 1,
	PAN_LUT_RISE_MODE_MAX	= 2,
};

enum pan_dimming_lut_size_type {
	PAN_LUT_SIZE_10	= 0,
	PAN_LUT_SIZE_30 = 1,
	PAN_LUT_SIZE_MAX = 2,
};

enum pan_dimming_mode_type {
	PAN_DIM_ALL_ZERO = 0,
	PAN_DIM_ALL_FULL = 1,
	PAN_DIM_ALL_MID	= 2,
	PAN_DIM_ZERO_TO_FULL = 3,
	PAN_DIM_FULL_TO_ZERO = 4,	
	PAN_DIM_ZERO_TO_HALF = 5,
	PAN_DIM_HALF_TO_FULL = 6,
	PAN_DIM_FULL_TO_HALF = 7,
	PAN_DIM_HALF_TO_ZERO = 8,
	PAN_DIM_ZERO_TO_FULL_TO_ZERO = 9,

	PAN_DIM_CHARGING_1 = 10,
	PAN_DIM_CHARGING_2 = 11,
	PAN_DIM_CHARGING_3 = 12,
};

static int PAN_LUT_SIZE[PAN_LUT_SIZE_MAX] = {
 	PAN_PATTERN_MAX_SIZE_10,
 	PAN_PATTERN_MAX_SIZE_30,
};

static int PAN_LUT_OFFSET[RGB_MAX] = {
	1,		/* red, unit is percent */
	21,		/* green */
	42,		/* blue */
};

#if defined(CONFIG_MACH_MSM8974_EF65S)
static int BRIGHTNESS_WEIGHT[RGB_MAX] = {
	40, //25,		//WS1=60,		/* red, unit is percent */
	60, //50,		//WS1=35,		/* green */
	60, //100,	/* blue */
};

static int WHITE_WEIGHT[RGB_MAX] = {
	40,//20,	/* red, unit is percent */
	60,//60,	/* green */
	60,//40,	/* blue */
};
#else //EF63S
static int BRIGHTNESS_WEIGHT[RGB_MAX] = {
	60, //25,		//WS1=60,		/* red, unit is percent */
	140, //50,		//WS1=35,		/* green */
	130, //100,	/* blue */
};

static int WHITE_WEIGHT[RGB_MAX] = {
	50,//20,	/* red, unit is percent */
	90,//60,	/* green */
	60,//40,	/* blue */
};
#endif


static int LUT_DATA_UP_10[PAN_LUT_RISE_MODE_MAX][PAN_PATTERN_MAX_SIZE_10] = {
	{	0,
		0,	1,	2,	4,	7,	12,	21,	35,	59,	100,
	}, 
	{	0,
		0,	11,	22,	33,	44,	56,	67,	78,	89,	100,
	}
};
static int LUT_DATA_DOWN_10[PAN_LUT_RISE_MODE_MAX][PAN_PATTERN_MAX_SIZE_10] ={
	{	100,
		100,59,	35,	21,	12,	7,	4,	2,	1,	0,	
	},
	{	100,
		100,89,	78,	67,	56,	44,	33,	22,	11,	0,
	}
};
static int LUT_DATA_UP_30[PAN_LUT_RISE_MODE_MAX][PAN_PATTERN_MAX_SIZE_30] = {
	{	0,
		0,	0,	0,	1,	1,	1,	2,	2,	3,	3,
		4,	5,	6,	7,	8,	10,	12,	14,	17,	20,
		23,	27,	32,	38,	45,	53,	62,	73,	85,	100
	},
	{	0,
		0,	3,	7,	10,	14,	17,	21,	24,	28,	31,
		34,	38,	41,	45,	48,	52,	55,	59,	62,	66,
		69,	72,	76,	79,	83,	86,	90,	93,	97,	100
	}
};
static int LUT_DATA_DOWN_30[PAN_LUT_RISE_MODE_MAX][PAN_PATTERN_MAX_SIZE_30] ={
	{	100,
		100,85,	73,	62,	53,	45,	38,	32,	27,	23,	
		20,	17,	14,	12,	10,	8,	7,	6,	5,	4,
		3,	3,	2,	2,	1,	1,	1,	0,	0,	0
	},
	{	100,
		100,97,	93,	90,	86,	83,	79,	76,	72,	69,
		66,	62,	59,	55,	52,	48,	45,	41,	38,	34,
		31,	28,	24,	21,	17,	14,	10,	7,	3,	0
	}
};

static int LUT_DATA_CHARGING[PAN_PATTERN_CHARGING_CNT][PAN_LUT_MAX_SIZE] = {
	{   0,
		0,   0,  12,  21,  35,  59, 100, 100, 100, 100,
	  100, 100, 100, 100,  80,  68,  61,  56,  54, 100,
	  100, 100, 100, 100, 100, 100, 100, 100,  80,  68,
	   61,  56,  54, 100, 100, 100, 100, 100, 100, 100,
	  100, 100,  80,  68,  61,  56,  54, 100, 100, 100,
	  100, 100, 100, 100, 100,  59,  35,  21,  12,   0,
	},
	{ 100,
	  100, 100, 100, 100, 100, 100, 100, 100, 100,  80,
	   68,  61,  56,  54,  52, 100, 100, 100, 100, 100,
	  100, 100, 100, 100,  80,  68,  61,  56,  54,  52,
	  100, 100, 100, 100, 100, 100, 100, 100, 100,  80,
	   68,  61,  56,  54,  52, 100, 100, 100, 100, 100,
	  100, 100, 100, 100,  80,  68,  61,  56,  54,  52,
	},
	{	0,
	    0,   1,   2,   4,   7,  12,  21,  35,  59,  79,
	   88,  93,  96,  98,  99, 100, 100, 100, 100, 100,
	  100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	  100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	  100, 100, 100, 100, 100,  99,  98,  96,  93,  88,
	   79,  59,  35,  21,  12   ,7,   4,   2,   1,   0,
	},
};

struct pan_rgb_pattern {
	int is_rainbow;
	int is_white;
	int oper_pnt;
	int rise_mode;
	int pattern;
	int setup_t;
	int hold_t;
    //p16619
    int control_rainbow_timer;
};

struct pan_led_data {
	struct timer_list timer;
	struct work_struct wq;	
	struct qpnp_led_data *led[3];	
	int led_state[3];	

//++ p11309 - 2014.01.03 for Offline Charger
	int offline_boot;
	int batt_status;
	struct timer_list offline_timer;
	struct work_struct offline_wq;
//-- p11309
};


int is_rgb_registered = 0;
static struct pan_led_data pan_led_data;
static struct pan_rgb_pattern pan_rgb_pattern;

static void pan_rgb_led_off(int id);
static void led_blink(struct qpnp_led_data *led, 
					  struct pwm_config_data *pwm_cfg);
static void qpnp_led_set(struct led_classdev *led_cdev,
						 enum led_brightness value);
//p16619
static int qpnp_led_set_block_pending(struct led_classdev *led_cdev,
						 enum led_brightness value);
static int qpnp_pwm_init(struct pwm_config_data *pwm_cfg,
						 struct spmi_device *spmi_dev,
						 const char *name);

static struct work_struct pan_rainbow_off_timer_work;
static struct timer_list pan_rainbow_off_timer;

static void pan_rainbow_off_timer_func(unsigned long data)
{
	pan_debug("%s\n", __func__);
    if(pan_rgb_pattern.control_rainbow_timer)
	    schedule_work(&pan_rainbow_off_timer_work);
}

static void pan_rainbow_off_timer_work_func(struct work_struct * p)
{	
	int i;

	pan_rgb_pattern.is_rainbow = 0;
    pan_rgb_pattern.control_rainbow_timer = 0;
	del_timer(&pan_led_data.timer);

	for (i=0; i<RGB_MAX; i++)
		pan_rgb_led_off(i);
}

static int pan_rgb_brightness(int id, int brightness)
{	
	int recal_brightness;
	if ( id > RGB_BLUE || id < RGB_RED ) {
		printk("%s, Wrong color id error...color=%d\n", 
			__func__, id);
		return 0;
	}	

	if ( pan_rgb_pattern.is_white == 1 ) 
		recal_brightness = brightness * WHITE_WEIGHT[id] / 100;
	else 
		recal_brightness = brightness * BRIGHTNESS_WEIGHT[id] / 100;

	return (recal_brightness * PAN_RGB_BRIGHTNESS_WEIGHT);
}

static int pan_get_pattern_duty_pcts(int id, int dim_mode, int lut, int *pcts) 
{
	int i;
	int ret_cnt=0;
	int type = pan_rgb_pattern.rise_mode;
	int lut_size = PAN_LUT_SIZE[lut];	
	int *lut_up, *lut_down;	

	ret_cnt = lut_size;	
	if (lut == PAN_LUT_SIZE_30 ) {		
		lut_up = LUT_DATA_UP_30[type];
		lut_down = LUT_DATA_DOWN_30[type];
	}
	else {		
		lut_up = LUT_DATA_UP_10[type];
		lut_down = LUT_DATA_DOWN_10[type];
	}	

	switch (dim_mode) {
	case PAN_DIM_ALL_ZERO:		
		ret_cnt = 2;
		for (i = 0; i < lut_size; i++)
			pcts[i] = 0;
		break;

	case PAN_DIM_ALL_FULL:		
		ret_cnt = 2;
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, 100);
		break;		

	case PAN_DIM_ALL_MID:
		ret_cnt = 2;
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, 50);
		break;

	case PAN_DIM_ZERO_TO_FULL:		
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_up[i]);			
		break;

	case PAN_DIM_FULL_TO_ZERO:
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_down[i]);
		break;	

	case PAN_DIM_ZERO_TO_HALF:
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_up[i]/2);
		break;

	case PAN_DIM_HALF_TO_FULL:
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_up[i]/2+50);
		break;

	case PAN_DIM_FULL_TO_HALF:
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_down[i]/2+50);
		break;

	case PAN_DIM_HALF_TO_ZERO:
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_down[i]/2);		
		break;

	case PAN_DIM_ZERO_TO_FULL_TO_ZERO:
		ret_cnt = lut_size * 2 - 1;
		for (i = 0; i < lut_size; i++)
			pcts[i] = pan_rgb_brightness(id, lut_up[i]);		
		for (i = lut_size; i < ret_cnt; i++)
			pcts[i] = pan_rgb_brightness(id, lut_down[i - lut_size + 1]);
		break;

	case PAN_DIM_CHARGING_1:
		ret_cnt = PAN_LUT_MAX_SIZE;
		for (i = 0; i < PAN_LUT_MAX_SIZE; i++)
			pcts[i] = pan_rgb_brightness(id, LUT_DATA_CHARGING[0][i]);
		break;
	case PAN_DIM_CHARGING_2:
		ret_cnt = PAN_LUT_MAX_SIZE;
		for (i = 0; i < PAN_LUT_MAX_SIZE; i++)
			pcts[i] = pan_rgb_brightness(id, LUT_DATA_CHARGING[1][i]);		
		break;
	case PAN_DIM_CHARGING_3:
		ret_cnt = PAN_LUT_MAX_SIZE;
		for (i = 0; i < PAN_LUT_MAX_SIZE; i++)
			pcts[i] = pan_rgb_brightness(id, LUT_DATA_CHARGING[2][i]);		
		break;
	}

	pan_debug("%s: id:%d, dim:%d, cnt:%d...%d %d %d %d %d %d\n", 
		__func__, id, dim_mode, ret_cnt,
		pcts[0],pcts[1],pcts[2],pcts[3],pcts[4],pcts[5]);
	return ret_cnt;
}

static int pan_get_blink_duty_pcts(
	int id, 
	int cnt, 
	int lut_max_size, 
	int *on, 
	int *off, 
	int *pcts) 
{
	int i, j;
	int ret_cnt=0;	
	int slot_cnt;
	int on_slot = 1;
	int need_to_recal = 0;
	int on_time = *on;

	if ( *on > *off ) 
		return ret_cnt;	

	if ( *on > PAN_MAX_RAMP_STEP_MS) {
		
		need_to_recal = 1;
		do {
			on_slot++;
			if ( on_slot > (lut_max_size / 4) ) {
				on_slot = (on_time + PAN_MAX_RAMP_STEP_MS - 1) 
								/ PAN_MAX_RAMP_STEP_MS;
				*on = on_time / on_slot;
				need_to_recal = 0;
				break;
			}

			if ( !(on_time % on_slot) )
				*on = on_time / on_slot;

			if ( *on <= PAN_MAX_RAMP_STEP_MS ) {
				if ( !(*off % *on) ) {
					if ( ( (*off + *on * on_slot) / *on ) < lut_max_size )
						need_to_recal = 0;
				}
			}
		} 
		while (need_to_recal);
	}

	slot_cnt = (*off + (*on * on_slot)) / *on;
	if ( slot_cnt > lut_max_size ) {

		if ( (*off / *on + on_slot) > lut_max_size) {
			*off = *on * (lut_max_size - on_slot);
		}
		else {
			*on = (*off + (*on * on_slot)) / lut_max_size;
			if ( *on > PAN_MAX_RAMP_STEP_MS ) {
				*on = PAN_MAX_RAMP_STEP_MS;
				on_slot = 1;
			}
			*off = *on * (lut_max_size - on_slot);
		}
		
		slot_cnt = (*off + (*on * on_slot)) / *on;
	}
	
	for (i = 0; i < lut_max_size; i++)
		pcts[i] = 0;
		
	for (i = 0; i < cnt; i++ )
		for (j=0; j<on_slot; j++)
			pcts[i * on_slot * 2 + j + 1] = pan_rgb_brightness(id, 100);

	ret_cnt = slot_cnt;

	pan_debug("%s: on=%d, off=%d, ret_cnt=%d\n", __func__, *on, *off, ret_cnt);	

	return ret_cnt;
}

static void pan_pattern_brightness(int id, int brightness)
{
	struct qpnp_led_data *led = pan_led_data.led[id];
	struct pwm_config_data *pwm_cfg = led->rgb_cfg->pwm_cfg;
    int ret;
	mutex_lock(&led->lock);	

	pwm_cfg->lut_params.idx_len = 3;
	pwm_cfg->duty_cycles->duty_pcts[0] = brightness;
	pwm_cfg->duty_cycles->duty_pcts[1] = brightness;
	pwm_cfg->duty_cycles->duty_pcts[2] = brightness;

	pwm_cfg->lut_params.ramp_step_ms = 10;
	pwm_cfg->lut_params.start_idx = PAN_LUT_OFFSET[id];
	pwm_cfg->lut_params.flags = PM_PWM_LUT_RAMP_UP;
	pwm_cfg->duty_cycles->num_duty_pcts = pwm_cfg->lut_params.idx_len;

	led->cdev.brightness = (brightness > 0) ? 
								led->cdev.max_brightness: LED_OFF;		

	pwm_free(pwm_cfg->pwm_dev);
	if (qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name))
	{	
		printk("[+++ LED] %s: qpnp_pwm_init failed..\n", __func__);
		return;
	}
	mutex_unlock(&led->lock);
	ret = qpnp_led_set_block_pending(&led->cdev, led->cdev.brightness);
	ret = qpnp_led_set_block_pending(&led->cdev, led->cdev.brightness);

	pan_debug("%s: id=%d, LED=%d, brightness=%d, pwm=0x%X, pwm_chan=%d\n", 
		__func__, id, pan_led_data.led[id]->id, brightness,
		(int)pwm_cfg, pwm_cfg->pwm_channel);
}

static void pan_pattern_process(int id, int dim_mode, int flag, int lut_size)
{
	struct qpnp_led_data *led = pan_led_data.led[id];
	struct pwm_config_data *pwm_cfg = led->rgb_cfg->pwm_cfg;
    int ret;
	mutex_lock(&led->lock);	
	
	pwm_cfg->lut_params.idx_len = 
		pan_get_pattern_duty_pcts(id, dim_mode, lut_size, 
			pwm_cfg->duty_cycles->duty_pcts);
	
	if ( dim_mode >= PAN_DIM_CHARGING_1 ) 
		pwm_cfg->lut_params.ramp_step_ms = 
			(pan_rgb_pattern.setup_t*10) / (PAN_LUT_MAX_SIZE - 1);
	else 
		pwm_cfg->lut_params.ramp_step_ms = 
			pan_rgb_pattern.setup_t / (PAN_LUT_SIZE[lut_size] - 1);

	if ( pwm_cfg->lut_params.ramp_step_ms < 1 ) {
		pwm_cfg->lut_params.ramp_step_ms = 1;
		pan_debug("%s: ramp step ms is zero\n", __func__);
	}
	
	if ( pan_rgb_pattern.is_rainbow == 1 || dim_mode >= PAN_DIM_CHARGING_1)
		pwm_cfg->lut_params.start_idx = 1;
	else 
		pwm_cfg->lut_params.start_idx = PAN_LUT_OFFSET[id];

	pwm_cfg->lut_params.flags = flag;
	pwm_cfg->duty_cycles->num_duty_pcts = pwm_cfg->lut_params.idx_len;

	led->cdev.brightness = (dim_mode == PAN_DIM_ALL_ZERO) ? 
								LED_OFF: led->cdev.max_brightness;

	pwm_free(pwm_cfg->pwm_dev);
	if (qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name))
	{	
		printk("[+++ LED] %s: qpnp_pwm_init failed..\n", __func__);
		return;
	}
	mutex_unlock(&led->lock);
    pan_debug("%s id:%d , dim_mode:%d , flag:%d , lut_size:%d\n" ,__func__, id , dim_mode , flag , lut_size);
	ret = qpnp_led_set_block_pending(&led->cdev, led->cdev.brightness);
	ret = qpnp_led_set_block_pending(&led->cdev, led->cdev.brightness);
}  

static void pan_blink_process(int id, int cnt, int on, int off, 
							  int lut_offset, int lut_max_size)
{
	struct qpnp_led_data *led = pan_led_data.led[id];
	struct pwm_config_data *pwm_cfg = led->rgb_cfg->pwm_cfg;	
    int ret;
	mutex_lock(&led->lock);	

	pwm_cfg->lut_params.idx_len = 
		pan_get_blink_duty_pcts(id, cnt, lut_max_size, &on, &off,
			pwm_cfg->duty_cycles->duty_pcts);

	pwm_cfg->lut_params.ramp_step_ms = on;
	if ( pwm_cfg->lut_params.ramp_step_ms < 1 ) {
		pwm_cfg->lut_params.ramp_step_ms = 1;
		pan_debug("%s: ramp step ms is zero\n", __func__);
	}
	pwm_cfg->lut_params.flags = QPNP_LED_PWM_FLAGS;
	
	pwm_cfg->lut_params.start_idx = lut_offset;
	pwm_cfg->duty_cycles->num_duty_pcts = pwm_cfg->lut_params.idx_len;
	led->cdev.brightness = led->cdev.max_brightness;

	pwm_free(pwm_cfg->pwm_dev);
	if (qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name))
	{	
		printk("[+++ LED] %s: qpnp_pwm_init failed..\n", __func__);
		return;
	}
	mutex_unlock(&led->lock);
	ret = qpnp_led_set_block_pending(&led->cdev, led->cdev.brightness);
	ret = qpnp_led_set_block_pending(&led->cdev, led->cdev.brightness);

	pan_debug("%s: id=%d, LED=%d , cnt=%d , on=%d , off=%d , lut_offset=%d , lut_max_size=%d , \n",
		__func__, id, pan_led_data.led[id]->id,	cnt , on , off , lut_offset , lut_max_size);
}

static void pan_pattern_process_loop(int id, int dim_mode) {
    pan_debug("%s id : %d , dim_mode : %d\n" ,__func__, id , dim_mode);
	pan_pattern_process(id, dim_mode, QPNP_LED_PWM_FLAGS, PAN_LUT_SIZE_10);
}

static void pan_pattern_process_oneshot(int id, int dim_mode) {
    pan_debug("%s id : %d , dim_mode : %d\n" ,__func__, id , dim_mode);
	pan_pattern_process(id, dim_mode, PM_PWM_LUT_RAMP_UP, PAN_LUT_SIZE_10);
}

static void pan_rainbow_generate(void)
{
	int id;
	int dim_mode;	

	if ( pan_rgb_pattern.pattern == 0 || pan_rgb_pattern.pattern == 2) {
		switch (pan_rgb_pattern.oper_pnt) {
			case 0: id = RGB_RED; dim_mode = PAN_DIM_ZERO_TO_FULL; break;
			case 1: id = RGB_GREEN; dim_mode = PAN_DIM_ZERO_TO_FULL; break;
			case 2: id = RGB_RED; dim_mode = PAN_DIM_FULL_TO_ZERO; break;
			case 3: id = RGB_BLUE; dim_mode = PAN_DIM_ZERO_TO_FULL; break;
			case 4: id = RGB_GREEN; dim_mode = PAN_DIM_FULL_TO_ZERO; break;
			case 5: id = RGB_RED; dim_mode = PAN_DIM_ZERO_TO_FULL; break;
			case 6: id = RGB_BLUE; dim_mode = PAN_DIM_FULL_TO_ZERO; break;		
		}

		pan_rgb_pattern.oper_pnt++;
		if ( pan_rgb_pattern.oper_pnt > 6) {
			if ( pan_rgb_pattern.pattern == 0 )
				pan_rgb_pattern.oper_pnt = 1;
			else
				pan_rgb_pattern.is_rainbow = 0;
		}
	}
	else if ( pan_rgb_pattern.pattern == 1 || pan_rgb_pattern.pattern == 3) {
		switch (pan_rgb_pattern.oper_pnt) {
			case 0: id = RGB_RED; dim_mode = PAN_DIM_ZERO_TO_FULL; break;
			case 1: id = RGB_GREEN; dim_mode = PAN_DIM_ZERO_TO_HALF; break;
			case 2: id = RGB_GREEN; dim_mode = PAN_DIM_HALF_TO_FULL; break;
			case 3: id = RGB_RED; dim_mode = PAN_DIM_FULL_TO_HALF; break;
			case 4: id = RGB_RED; dim_mode = PAN_DIM_HALF_TO_ZERO; break;
			case 5: id = RGB_BLUE; dim_mode = PAN_DIM_ZERO_TO_HALF; break;
			case 6: id = RGB_BLUE; dim_mode = PAN_DIM_HALF_TO_FULL; break;
			case 7: id = RGB_GREEN; dim_mode = PAN_DIM_FULL_TO_HALF; break;
			case 8: id = RGB_GREEN; dim_mode = PAN_DIM_HALF_TO_ZERO; break;
			case 9: id = RGB_RED; dim_mode = PAN_DIM_ZERO_TO_HALF; break;
			case 10: id = RGB_RED; dim_mode = PAN_DIM_HALF_TO_FULL; break;
			case 11: id = RGB_BLUE; dim_mode = PAN_DIM_FULL_TO_HALF; break;
			case 12: id = RGB_BLUE; dim_mode = PAN_DIM_HALF_TO_ZERO; break;	
		}

		pan_rgb_pattern.oper_pnt++;
		if ( pan_rgb_pattern.oper_pnt > 12) {
			if ( pan_rgb_pattern.pattern == 1 )
				pan_rgb_pattern.oper_pnt = 1;
			else
				pan_rgb_pattern.is_rainbow = 0;
		}
	}	
	else 
		return;

	pan_debug("%s: id=%d, dim_mode=%d, is_rainbow=%d\n", __func__, id, dim_mode, pan_rgb_pattern.is_rainbow);
	if ( pan_rgb_pattern.is_rainbow )
		pan_pattern_process(id, dim_mode, PM_PWM_LUT_RAMP_UP, PAN_LUT_SIZE_30);	
}

static void pan_do_rainbow(int pattern, int lut, int t_s, int h_k)
{
	pan_rgb_pattern.is_rainbow = 1;
	pan_rgb_pattern.is_white = 0;
	pan_rgb_pattern.oper_pnt = 0;
	pan_rgb_pattern.rise_mode = lut;
	pan_rgb_pattern.pattern = pattern;
	pan_rgb_pattern.setup_t = t_s;
	pan_rgb_pattern.hold_t = h_k;
    pan_rgb_pattern.control_rainbow_timer = 1;
	pan_debug("%s: pattern=%d, lut=%d, t_s=%d, h_k=%d , control_rainbow_timer=%d\n", 
		__func__, pattern, lut, t_s, h_k , pan_rgb_pattern.control_rainbow_timer);

	pan_rainbow_generate();
 	mod_timer(&pan_led_data.timer, jiffies + msecs_to_jiffies(
 		pan_rgb_pattern.setup_t+pan_rgb_pattern.hold_t));
}

static void pan_led_process_timer_func(unsigned long data)
{	
    if(pan_rgb_pattern.control_rainbow_timer)
	    schedule_work(&pan_led_data.wq);
}

static void pan_led_process_timer_wq_func(struct work_struct * p)
{	
	int i=0;
	pan_rainbow_generate();
	if (pan_rgb_pattern.is_rainbow == 1 )
		mod_timer(&pan_led_data.timer, jiffies + msecs_to_jiffies(
			pan_rgb_pattern.setup_t+pan_rgb_pattern.hold_t));
	else {
		for (i=0; i<RGB_MAX; i++) 
			pan_rgb_led_off(i);
	}
}

//++ p11309 - 2014.01.03 for Offline Charger
extern int get_batt_status(void);

static void pan_offline_timer_func(unsigned long data)
{
	pan_debug("%s\n", __func__);
	schedule_work(&pan_led_data.offline_wq);
}

static void pan_offline_timer_work_func(struct work_struct * p)
{	
	int status=0;

	status = get_batt_status();
	
	pan_debug("%s: batt status=%d, old=%d\n", __func__, status, pan_led_data.batt_status);
	pan_rgb_pattern.is_white = 0;
	pan_rgb_pattern.rise_mode = 0;
	pan_rgb_pattern.setup_t = 400;	

	if (status != pan_led_data.batt_status) {
		pan_led_data.batt_status = status;
		switch (pan_led_data.batt_status) {				
		case PAN_BATT_CHARGING:
			pan_pattern_process_loop(RGB_GREEN, PAN_DIM_ALL_ZERO);
			pan_pattern_process_loop(RGB_RED, PAN_DIM_CHARGING_1);
			break;
		case PAN_BATT_NOT_DEFINED:
		case PAN_BATT_DISCHARGING:			
		case PAN_BATT_NOT_CHARGING:
			pan_pattern_process_loop(RGB_GREEN, PAN_DIM_ALL_ZERO);
			pan_pattern_process_loop(RGB_RED, PAN_DIM_ALL_ZERO);
			break;
		case PAN_BATT_FULL:
			pan_pattern_process_loop(RGB_RED, PAN_DIM_ALL_ZERO);
			pan_pattern_process_loop(RGB_GREEN, PAN_DIM_ALL_FULL);
			break;
		}
	}

	mod_timer(&pan_led_data.offline_timer,
		jiffies + msecs_to_jiffies(PAN_OFFLINE_CHECK_INTERVAL));	
}
//-- p11309

#endif // PAN_LED_CONTROL
//-- p11309 
//=================================================================

static int
qpnp_led_masked_write(struct qpnp_led_data *led, u16 addr, u8 mask, u8 val)
{
	int rc;
	u8 reg;

	rc = spmi_ext_register_readl(led->spmi_dev->ctrl, led->spmi_dev->sid,
		addr, &reg, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to read from addr=%x, rc(%d)\n", addr, rc);
	}

	reg &= ~mask;
	reg |= val;

	rc = spmi_ext_register_writel(led->spmi_dev->ctrl, led->spmi_dev->sid,
		addr, &reg, 1);
	if (rc)
		dev_err(&led->spmi_dev->dev,
			"Unable to write to addr=%x, rc(%d)\n", addr, rc);
	return rc;
}

static void qpnp_dump_regs(struct qpnp_led_data *led, u8 regs[], u8 array_size)
{
	int i;
	u8 val;

	pr_debug("===== %s LED register dump start =====\n", led->cdev.name);
	for (i = 0; i < array_size; i++) {
		spmi_ext_register_readl(led->spmi_dev->ctrl,
					led->spmi_dev->sid,
					led->base + regs[i],
					&val, sizeof(val));
		pr_debug("%s: 0x%x = 0x%x\n", led->cdev.name,
					led->base + regs[i], val);
	}
	pr_debug("===== %s LED register dump end =====\n", led->cdev.name);
}

static int qpnp_wled_sync(struct qpnp_led_data *led)
{
	int rc;
	u8 val;

	/* sync */
	val = WLED_SYNC_VAL;
	rc = spmi_ext_register_writel(led->spmi_dev->ctrl, led->spmi_dev->sid,
		WLED_SYNC_REG(led->base), &val, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
				"WLED set sync reg failed(%d)\n", rc);
		return rc;
	}

	val = WLED_SYNC_RESET_VAL;
	rc = spmi_ext_register_writel(led->spmi_dev->ctrl, led->spmi_dev->sid,
		WLED_SYNC_REG(led->base), &val, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
				"WLED reset sync reg failed(%d)\n", rc);
		return rc;
	}
	return 0;
}

static int qpnp_wled_set(struct qpnp_led_data *led)
{
	int rc, duty, level;
	u8 val, i, num_wled_strings, sink_val;

	num_wled_strings = led->wled_cfg->num_strings;

	level = led->cdev.brightness;

	if (level > WLED_MAX_LEVEL)
		level = WLED_MAX_LEVEL;
	if (level == 0) {
		for (i = 0; i < num_wled_strings; i++) {
			rc = qpnp_led_masked_write(led,
				WLED_FULL_SCALE_REG(led->base, i),
				WLED_MAX_CURR_MASK, WLED_NO_CURRENT);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Write max current failure (%d)\n",
					rc);
				return rc;
			}
		}

		rc = qpnp_wled_sync(led);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED sync failed(%d)\n", rc);
			return rc;
		}

		rc = spmi_ext_register_readl(led->spmi_dev->ctrl,
			led->spmi_dev->sid, WLED_CURR_SINK_REG(led->base),
			&sink_val, 1);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED read sink reg failed(%d)\n", rc);
			return rc;
		}

		if (led->wled_cfg->pmic_version == PMIC_VER_8026) {
			val = WLED_DISABLE_ALL_SINKS;
			rc = spmi_ext_register_writel(led->spmi_dev->ctrl,
				led->spmi_dev->sid,
				WLED_CURR_SINK_REG(led->base), &val, 1);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"WLED write sink reg failed(%d)\n", rc);
				return rc;
			}

			usleep(WLED_OVP_DELAY);
		}

		val = WLED_BOOST_OFF;
		rc = spmi_ext_register_writel(led->spmi_dev->ctrl,
			led->spmi_dev->sid, WLED_MOD_CTRL_REG(led->base),
			&val, 1);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED write ctrl reg failed(%d)\n", rc);
			return rc;
		}

		for (i = 0; i < num_wled_strings; i++) {
			rc = qpnp_led_masked_write(led,
				WLED_FULL_SCALE_REG(led->base, i),
				WLED_MAX_CURR_MASK, led->max_current);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Write max current failure (%d)\n",
					rc);
				return rc;
			}
		}

		rc = qpnp_wled_sync(led);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED sync failed(%d)\n", rc);
			return rc;
		}

		rc = spmi_ext_register_writel(led->spmi_dev->ctrl,
			led->spmi_dev->sid, WLED_CURR_SINK_REG(led->base),
			&sink_val, 1);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED write sink reg failed(%d)\n", rc);
			return rc;
		}

	} else {
		val = WLED_BOOST_ON;
		rc = spmi_ext_register_writel(led->spmi_dev->ctrl,
			led->spmi_dev->sid, WLED_MOD_CTRL_REG(led->base),
			&val, 1);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED write ctrl reg failed(%d)\n", rc);
			return rc;
		}
	}

	duty = (WLED_MAX_DUTY_CYCLE * level) / WLED_MAX_LEVEL;

	/* program brightness control registers */
	for (i = 0; i < num_wled_strings; i++) {
		rc = qpnp_led_masked_write(led,
			WLED_BRIGHTNESS_CNTL_MSB(led->base, i), WLED_MSB_MASK,
			(duty >> WLED_8_BIT_SHFT) & WLED_4_BIT_MASK);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED set brightness MSB failed(%d)\n", rc);
			return rc;
		}
		val = duty & WLED_8_BIT_MASK;
		rc = spmi_ext_register_writel(led->spmi_dev->ctrl,
			led->spmi_dev->sid,
			WLED_BRIGHTNESS_CNTL_LSB(led->base, i), &val, 1);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED set brightness LSB failed(%d)\n", rc);
			return rc;
		}
	}

	rc = qpnp_wled_sync(led);
	if (rc) {
		dev_err(&led->spmi_dev->dev, "WLED sync failed(%d)\n", rc);
		return rc;
	}
	return 0;
}

static int qpnp_mpp_set(struct qpnp_led_data *led)
{
	int rc, val;
	int duty_us;

	if (led->cdev.brightness) {
		if (led->cdev.brightness < led->mpp_cfg->min_brightness) {
			dev_warn(&led->spmi_dev->dev,
				"brightness is less than supported..." \
				"set to minimum supported\n");
			led->cdev.brightness = led->mpp_cfg->min_brightness;
		}

		if (led->mpp_cfg->pwm_mode != MANUAL_MODE) {
			if (!led->mpp_cfg->pwm_cfg->blinking) {
				led->mpp_cfg->pwm_cfg->mode =
					led->mpp_cfg->pwm_cfg->default_mode;
				led->mpp_cfg->pwm_mode =
					led->mpp_cfg->pwm_cfg->default_mode;
			}
		}
		if (led->mpp_cfg->pwm_mode == PWM_MODE) {
			pwm_disable(led->mpp_cfg->pwm_cfg->pwm_dev);
			duty_us = (led->mpp_cfg->pwm_cfg->pwm_period_us *
					led->cdev.brightness) / LED_FULL;
			/*config pwm for brightness scaling*/
			rc = pwm_config_us(led->mpp_cfg->pwm_cfg->pwm_dev,
					duty_us,
					led->mpp_cfg->pwm_cfg->pwm_period_us);
			if (rc < 0) {
				dev_err(&led->spmi_dev->dev, "Failed to " \
					"configure pwm for new values\n");
				return rc;
			}
		}

		if (led->mpp_cfg->pwm_mode != MANUAL_MODE)
			pwm_enable(led->mpp_cfg->pwm_cfg->pwm_dev);
		else {
			if (led->cdev.brightness < LED_MPP_CURRENT_MIN)
				led->cdev.brightness = LED_MPP_CURRENT_MIN;

			val = (led->cdev.brightness / LED_MPP_CURRENT_MIN) - 1;

			rc = qpnp_led_masked_write(led,
					LED_MPP_SINK_CTRL(led->base),
					LED_MPP_SINK_MASK, val);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Failed to write sink control reg\n");
				return rc;
			}
		}

		val = (led->mpp_cfg->source_sel & LED_MPP_SRC_MASK) |
			(led->mpp_cfg->mode_ctrl & LED_MPP_MODE_CTRL_MASK);

		rc = qpnp_led_masked_write(led,
			LED_MPP_MODE_CTRL(led->base), LED_MPP_MODE_MASK,
			val);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
					"Failed to write led mode reg\n");
			return rc;
		}

		rc = qpnp_led_masked_write(led,
				LED_MPP_EN_CTRL(led->base), LED_MPP_EN_MASK,
				LED_MPP_EN_ENABLE);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
					"Failed to write led enable " \
					"reg\n");
			return rc;
		}
	} else {
		if (led->mpp_cfg->pwm_mode != MANUAL_MODE) {
			led->mpp_cfg->pwm_cfg->mode =
				led->mpp_cfg->pwm_cfg->default_mode;
			led->mpp_cfg->pwm_mode =
				led->mpp_cfg->pwm_cfg->default_mode;
			pwm_disable(led->mpp_cfg->pwm_cfg->pwm_dev);
		}
		rc = qpnp_led_masked_write(led,
					LED_MPP_MODE_CTRL(led->base),
					LED_MPP_MODE_MASK,
					LED_MPP_MODE_DISABLE);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
					"Failed to write led mode reg\n");
			return rc;
		}

		rc = qpnp_led_masked_write(led,
					LED_MPP_EN_CTRL(led->base),
					LED_MPP_EN_MASK,
					LED_MPP_EN_DISABLE);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
					"Failed to write led enable reg\n");
			return rc;
		}
	}

	if (led->mpp_cfg->pwm_mode != MANUAL_MODE)
		led->mpp_cfg->pwm_cfg->blinking = false;
	qpnp_dump_regs(led, mpp_debug_regs, ARRAY_SIZE(mpp_debug_regs));

	return 0;
}

#ifdef CONFIG_PANTECH_CAMERA //flash
void qpnp_set_flash_mode(bool on) {
 	global_torch_enable = on;
}
EXPORT_SYMBOL_GPL(qpnp_set_flash_mode);
#endif

static int qpnp_flash_regulator_operate(struct qpnp_led_data *led, bool on)
{
	int rc, i;
	struct qpnp_led_data *led_array;
	bool regulator_on = false;

	led_array = dev_get_drvdata(&led->spmi_dev->dev);
	if (!led_array) {
		dev_err(&led->spmi_dev->dev,
				"Unable to get LED array\n");
		return -EINVAL;
	}

	for (i = 0; i < led->num_leds; i++)
		regulator_on |= led_array[i].flash_cfg->flash_on;

	if (!on)
		goto regulator_turn_off;

	if (!regulator_on && !led->flash_cfg->flash_on) {
		for (i = 0; i < led->num_leds; i++) {
			if (led_array[i].flash_cfg->flash_reg_get) {
#if (defined(CONFIG_PANTECH_CAMERA))//flash
			if (!global_torch_enable) {
#endif
				rc = regulator_enable(
					led_array[i].flash_cfg->\
					flash_boost_reg);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
						"Regulator enable failed(%d)\n",
									rc);
					return rc;
				}
#if (defined(CONFIG_PANTECH_CAMERA))//flash
				}
#endif
				led->flash_cfg->flash_on = true;
			}
			break;
		}
	}

	return 0;

regulator_turn_off:
	if (regulator_on && led->flash_cfg->flash_on) {
		for (i = 0; i < led->num_leds; i++) {
			if (led_array[i].flash_cfg->flash_reg_get) {
				rc = qpnp_led_masked_write(led,
					FLASH_ENABLE_CONTROL(led->base),
					FLASH_ENABLE_MASK,
					FLASH_DISABLE_ALL);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
						"Enable reg write failed(%d)\n",
						rc);
				}
#if (defined(CONFIG_PANTECH_CAMERA))//flash
			if (!global_torch_enable) {
#endif
				rc = regulator_disable(led_array[i].flash_cfg->\
							flash_boost_reg);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
						"Regulator disable failed(%d)\n",
									rc);
					return rc;
				}
#if (defined(CONFIG_PANTECH_CAMERA))//flash
			}
#endif
				led->flash_cfg->flash_on = false;
			}
			break;
		}
	}

	return 0;
}

static int qpnp_torch_regulator_operate(struct qpnp_led_data *led, bool on)
{
	int rc;

	if (!on)
		goto regulator_turn_off;

	if (!led->flash_cfg->torch_on) {
#if (defined(CONFIG_PANTECH_CAMERA))//flash
		if (!global_torch_enable) {
#endif
		rc = regulator_enable(led->flash_cfg->torch_boost_reg);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"Regulator enable failed(%d)\n", rc);
				return rc;
		}
#if (defined(CONFIG_PANTECH_CAMERA))//flash
		}
#endif
		led->flash_cfg->torch_on = true;
	}
	return 0;

regulator_turn_off:
	if (led->flash_cfg->torch_on) {
		rc = qpnp_led_masked_write(led,	FLASH_ENABLE_CONTROL(led->base),
				FLASH_ENABLE_MODULE_MASK, FLASH_DISABLE_ALL);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"Enable reg write failed(%d)\n", rc);
		}
#if (defined(CONFIG_PANTECH_CAMERA))//flash
		if (!global_torch_enable) {
#endif
		rc = regulator_disable(led->flash_cfg->torch_boost_reg);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"Regulator disable failed(%d)\n", rc);
			return rc;
		}
#if (defined(CONFIG_PANTECH_CAMERA))//flash
		}
#endif
		led->flash_cfg->torch_on = false;
	}
	return 0;
}

static int qpnp_flash_set(struct qpnp_led_data *led)
{
	int rc, error;
	int val = led->cdev.brightness;

	if (led->flash_cfg->torch_enable)
		led->flash_cfg->current_prgm =
			(val * TORCH_MAX_LEVEL / led->max_current);
	else
		led->flash_cfg->current_prgm =
			(val * FLASH_MAX_LEVEL / led->max_current);

	/* Set led current */
	if (val > 0) {
#ifdef CONFIG_PANTECH_CAMERA //flash
		if (global_torch_enable) {
#else
		if (led->flash_cfg->torch_enable) {
#endif
			if (led->flash_cfg->peripheral_subtype ==
							FLASH_SUBTYPE_DUAL) {
				rc = qpnp_torch_regulator_operate(led, true);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
					"Torch regulator operate failed(%d)\n",
					rc);
					return rc;
				}
			} else if (led->flash_cfg->peripheral_subtype ==
							FLASH_SUBTYPE_SINGLE) {
				rc = qpnp_flash_regulator_operate(led, true);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
					"Flash regulator operate failed(%d)\n",
					rc);
					goto error_flash_set;
				}
				}

			rc = qpnp_led_masked_write(led,
				FLASH_LED_UNLOCK_SECURE(led->base),
				FLASH_SECURE_MASK, FLASH_UNLOCK_SECURE);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Secure reg write failed(%d)\n", rc);
				goto error_reg_write;
			}

			rc = qpnp_led_masked_write(led,
				FLASH_LED_TORCH(led->base),
				FLASH_TORCH_MASK, FLASH_LED_TORCH_ENABLE);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Torch reg write failed(%d)\n", rc);
				goto error_reg_write;
			}

			rc = qpnp_led_masked_write(led,
				led->flash_cfg->current_addr,
				FLASH_CURRENT_MASK,
				led->flash_cfg->current_prgm);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Current reg write failed(%d)\n", rc);
				goto error_reg_write;
			}

			rc = qpnp_led_masked_write(led,
				led->flash_cfg->second_addr,
				FLASH_CURRENT_MASK,
				led->flash_cfg->current_prgm);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"2nd Current reg write failed(%d)\n",
					rc);
				goto error_reg_write;
			}

			qpnp_led_masked_write(led, FLASH_MAX_CURR(led->base),
				FLASH_CURRENT_MASK,
				TORCH_MAX_LEVEL);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Max current reg write failed(%d)\n",
					rc);
				goto error_reg_write;
			}

			rc = qpnp_led_masked_write(led,
				FLASH_ENABLE_CONTROL(led->base),
				FLASH_ENABLE_MASK,
				led->flash_cfg->enable_module);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Enable reg write failed(%d)\n",
					rc);
				goto error_reg_write;
			}

			if (!led->flash_cfg->strobe_type)
				led->flash_cfg->trigger_flash &=
						~FLASH_HW_SW_STROBE_SEL_MASK;
			else
				led->flash_cfg->trigger_flash |=
						FLASH_HW_SW_STROBE_SEL_MASK;

			rc = qpnp_led_masked_write(led,
				FLASH_LED_STROBE_CTRL(led->base),
				led->flash_cfg->trigger_flash,
				led->flash_cfg->trigger_flash);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"LED %d strobe reg write failed(%d)\n",
					led->id, rc);
				goto error_reg_write;
			}
		} else {
			rc = qpnp_flash_regulator_operate(led, true);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Flash regulator operate failed(%d)\n",
					rc);
				goto error_flash_set;
			}

			/* Set flash safety timer */
			rc = qpnp_led_masked_write(led,
				FLASH_SAFETY_TIMER(led->base),
				FLASH_SAFETY_TIMER_MASK,
				led->flash_cfg->duration);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Safety timer reg write failed(%d)\n",
					rc);
				goto error_flash_set;
			}

			/* Set max current */
			rc = qpnp_led_masked_write(led,
				FLASH_MAX_CURR(led->base), FLASH_CURRENT_MASK,
				FLASH_MAX_LEVEL);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Max current reg write failed(%d)\n",
					rc);
				goto error_flash_set;
			}

			/* Set clamp current */
			rc = qpnp_led_masked_write(led,
				FLASH_CLAMP_CURR(led->base),
				FLASH_CURRENT_MASK,
				led->flash_cfg->clamp_curr);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Clamp current reg write failed(%d)\n",
					rc);
				goto error_flash_set;
			}

			rc = qpnp_led_masked_write(led,
				led->flash_cfg->current_addr,
				FLASH_CURRENT_MASK,
				led->flash_cfg->current_prgm);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Current reg write failed(%d)\n", rc);
				goto error_flash_set;
			}

#ifdef CONFIG_PANTECH_CAMERA //flash
			rc = qpnp_led_masked_write(led,
				FLASH_CLAMP_CURR(led->base),
				FLASH_CURRENT_MASK, FLASH_CURRENT_MAX);//FLASH_CURRENT_TORCH);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Clamp Current reg write failed(%d)\n",
					rc);
				return rc;
			}

			msleep(3);
			rc = qpnp_led_masked_write(led,
				FLASH_ENABLE_CONTROL(led->base),
				FLASH_ENABLE_MODULE_MASK, FLASH_ENABLE_MODULE);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Enable reg write failed(%d)\n", rc);
				return rc;
			}
#endif		
			rc = qpnp_led_masked_write(led,
				FLASH_ENABLE_CONTROL(led->base),
				led->flash_cfg->enable_module,
				led->flash_cfg->enable_module);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Enable reg write failed(%d)\n", rc);
				goto error_flash_set;
			}

			/*
			 * Add 1ms delay for bharger enter stable state
			 */
			usleep(FLASH_RAMP_UP_DELAY_US);

			if (!led->flash_cfg->strobe_type)
				led->flash_cfg->trigger_flash &=
						~FLASH_HW_SW_STROBE_SEL_MASK;
			else
				led->flash_cfg->trigger_flash |=
						FLASH_HW_SW_STROBE_SEL_MASK;

				rc = qpnp_led_masked_write(led,
					FLASH_LED_STROBE_CTRL(led->base),
					led->flash_cfg->trigger_flash,
					led->flash_cfg->trigger_flash);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
					"LED %d strobe reg write failed(%d)\n",
					led->id, rc);
					goto error_flash_set;
				}
				}
	} else {
		rc = qpnp_led_masked_write(led,
			FLASH_LED_STROBE_CTRL(led->base),
			led->flash_cfg->trigger_flash,
			FLASH_DISABLE_ALL);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"LED %d flash write failed(%d)\n", led->id, rc);
			if (led->flash_cfg->torch_enable)
				goto error_torch_set;
			else
				goto error_flash_set;
		}

#ifdef CONFIG_PANTECH_CAMERA //flash
		if (global_torch_enable) {
#else
		if (led->flash_cfg->torch_enable) {
#endif
			rc = qpnp_led_masked_write(led,
				FLASH_LED_UNLOCK_SECURE(led->base),
				FLASH_SECURE_MASK, FLASH_UNLOCK_SECURE);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Secure reg write failed(%d)\n", rc);
				goto error_torch_set;
			}

			rc = qpnp_led_masked_write(led,
					FLASH_LED_TORCH(led->base),
					FLASH_TORCH_MASK,
					FLASH_LED_TORCH_DISABLE);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Torch reg write failed(%d)\n", rc);
				goto error_torch_set;
			}

			if (led->flash_cfg->peripheral_subtype ==
							FLASH_SUBTYPE_DUAL) {
				rc = qpnp_torch_regulator_operate(led, false);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
						"Torch regulator operate failed(%d)\n",
						rc);
					return rc;
				}
			} else if (led->flash_cfg->peripheral_subtype ==
							FLASH_SUBTYPE_SINGLE) {
				rc = qpnp_flash_regulator_operate(led, false);
				if (rc) {
					dev_err(&led->spmi_dev->dev,
						"Flash regulator operate failed(%d)\n",
						rc);
					return rc;
				}
			}
		} else {
			/*
			 * Disable module after ramp down complete for stable
			 * behavior
			 */
			usleep(FLASH_RAMP_DN_DELAY_US);

			rc = qpnp_led_masked_write(led,
				FLASH_ENABLE_CONTROL(led->base),
				led->flash_cfg->enable_module &
				~FLASH_ENABLE_MODULE_MASK,
				FLASH_DISABLE_ALL);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Enable reg write failed(%d)\n", rc);
				if (led->flash_cfg->torch_enable)
					goto error_torch_set;
				else
					goto error_flash_set;
			}

			rc = qpnp_flash_regulator_operate(led, false);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Flash regulator operate failed(%d)\n",
					rc);
				return rc;
			}
		}
	}

	qpnp_dump_regs(led, flash_debug_regs, ARRAY_SIZE(flash_debug_regs));

	return 0;

error_reg_write:
	if (led->flash_cfg->peripheral_subtype == FLASH_SUBTYPE_SINGLE)
		goto error_flash_set;

error_torch_set:
	error = qpnp_torch_regulator_operate(led, false);
	if (error) {
		dev_err(&led->spmi_dev->dev,
			"Torch regulator operate failed(%d)\n", rc);
		return error;
	}
	return rc;

error_flash_set:
	error = qpnp_flash_regulator_operate(led, false);
	if (error) {
		dev_err(&led->spmi_dev->dev,
			"Flash regulator operate failed(%d)\n", rc);
		return error;
	}
	return rc;
}

static int qpnp_kpdbl_set(struct qpnp_led_data *led)
{
	int duty_us;
	int rc;

	if (led->cdev.brightness) {
		if (!led->kpdbl_cfg->pwm_cfg->blinking)
			led->kpdbl_cfg->pwm_cfg->mode =
				led->kpdbl_cfg->pwm_cfg->default_mode;
		if (!num_kpbl_leds_on) {
			rc = qpnp_led_masked_write(led, KPDBL_ENABLE(led->base),
					KPDBL_MODULE_EN_MASK, KPDBL_MODULE_EN);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Enable reg write failed(%d)\n", rc);
				return rc;
			}
		}

		if (led->kpdbl_cfg->pwm_cfg->mode == PWM_MODE) {
			duty_us = (led->kpdbl_cfg->pwm_cfg->pwm_period_us *
				led->cdev.brightness) / KPDBL_MAX_LEVEL;
			rc = pwm_config_us(led->kpdbl_cfg->pwm_cfg->pwm_dev,
					duty_us,
					led->kpdbl_cfg->pwm_cfg->pwm_period_us);
			if (rc < 0) {
				dev_err(&led->spmi_dev->dev, "pwm config failed\n");
				return rc;
			}
		}

		rc = pwm_enable(led->kpdbl_cfg->pwm_cfg->pwm_dev);
		if (rc < 0) {
			dev_err(&led->spmi_dev->dev, "pwm enable failed\n");
			return rc;
		}

		num_kpbl_leds_on++;

	} else {
		led->kpdbl_cfg->pwm_cfg->mode =
			led->kpdbl_cfg->pwm_cfg->default_mode;

		if (led->kpdbl_cfg->always_on) {
			rc = pwm_config_us(led->kpdbl_cfg->pwm_cfg->pwm_dev, 0,
					led->kpdbl_cfg->pwm_cfg->pwm_period_us);
			if (rc < 0) {
				dev_err(&led->spmi_dev->dev,
						"pwm config failed\n");
				return rc;
			}

			rc = pwm_enable(led->kpdbl_cfg->pwm_cfg->pwm_dev);
			if (rc < 0) {
				dev_err(&led->spmi_dev->dev, "pwm enable failed\n");
				return rc;
			}
		} else
			pwm_disable(led->kpdbl_cfg->pwm_cfg->pwm_dev);

		if (num_kpbl_leds_on > 0)
			num_kpbl_leds_on--;

		if (!num_kpbl_leds_on) {
			rc = qpnp_led_masked_write(led, KPDBL_ENABLE(led->base),
					KPDBL_MODULE_EN_MASK, KPDBL_MODULE_DIS);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
					"Failed to write led enable reg\n");
				return rc;
			}
		}
	}

	led->kpdbl_cfg->pwm_cfg->blinking = false;

	qpnp_dump_regs(led, kpdbl_debug_regs, ARRAY_SIZE(kpdbl_debug_regs));

	return 0;
}

static int qpnp_rgb_set(struct qpnp_led_data *led)
{
	int duty_us;
	int rc;

	pan_debug("%s: id=%d, brightness=%d, len=%d, pwm=0x%X, pwm_chan=%d, %d %d %d %d %d %d\n", 
		__func__,
		led->id, 
		led->cdev.brightness,
		led->rgb_cfg->pwm_cfg->lut_params.idx_len,
		(unsigned int)led->rgb_cfg->pwm_cfg->pwm_dev, 
		led->rgb_cfg->pwm_cfg->pwm_channel,
		led->rgb_cfg->pwm_cfg->duty_cycles->duty_pcts[0],
		led->rgb_cfg->pwm_cfg->duty_cycles->duty_pcts[1],
		led->rgb_cfg->pwm_cfg->duty_cycles->duty_pcts[2],
		led->rgb_cfg->pwm_cfg->duty_cycles->duty_pcts[3],
		led->rgb_cfg->pwm_cfg->duty_cycles->duty_pcts[4],
		led->rgb_cfg->pwm_cfg->duty_cycles->duty_pcts[5]);

	if (led->cdev.brightness) {
		if (!led->rgb_cfg->pwm_cfg->blinking)
			led->rgb_cfg->pwm_cfg->mode =
				led->rgb_cfg->pwm_cfg->default_mode;
		if (led->rgb_cfg->pwm_cfg->mode == PWM_MODE) {

			duty_us = (led->rgb_cfg->pwm_cfg->pwm_period_us *
				led->cdev.brightness) / LED_FULL;
			rc = pwm_config_us(led->rgb_cfg->pwm_cfg->pwm_dev,
					duty_us,
					led->rgb_cfg->pwm_cfg->pwm_period_us);
			if (rc < 0) {
				printk("[+++ LED] pwm config failed\n");
				return rc;
			}
		}
		rc = qpnp_led_masked_write(led,
			RGB_LED_EN_CTL(led->base),
			led->rgb_cfg->enable, led->rgb_cfg->enable);
		if (rc) {
			printk("[+++ LED] Failed to write led enable reg\n");
			return rc;
		}

		rc = pwm_enable(led->rgb_cfg->pwm_cfg->pwm_dev);
		if (rc < 0) {
			printk("[+++ LED] pwm enable failed\n");
			return rc;
		}
	} else {
		led->rgb_cfg->pwm_cfg->mode =
			led->rgb_cfg->pwm_cfg->default_mode;
		pwm_disable(led->rgb_cfg->pwm_cfg->pwm_dev);
		rc = qpnp_led_masked_write(led,
			RGB_LED_EN_CTL(led->base),
			led->rgb_cfg->enable, RGB_LED_DISABLE);
		if (rc) {
			printk("[+++ LED] Failed to write led enable reg\n");
			return rc;
		}
	}

	led->rgb_cfg->pwm_cfg->blinking = false;
	qpnp_dump_regs(led, rgb_pwm_debug_regs, ARRAY_SIZE(rgb_pwm_debug_regs));

	return 0;
}

static void qpnp_led_set(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	struct qpnp_led_data *led;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);
	if (value < LED_OFF) {
		dev_err(&led->spmi_dev->dev, "Invalid brightness value\n");
		return;
	}

	if (value > led->cdev.max_brightness)
		value = led->cdev.max_brightness;

	led->cdev.brightness = value;
	schedule_work(&led->work);
}
#ifdef PAN_LED_CONTROL	
static int qpnp_led_set_block_pending(struct led_classdev *led_cdev,
				enum led_brightness value)
{
	struct qpnp_led_data *led;
    int ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);
	if (value < LED_OFF) {
		dev_err(&led->spmi_dev->dev, "Invalid brightness value\n");
		return -1;
	}

	if (value > led->cdev.max_brightness)
		value = led->cdev.max_brightness;

	led->cdev.brightness = value;
	ret = schedule_work(&led->work);
    pan_debug("%s ret = %d\n" , __func__ , ret);
    if(!ret){
        pan_debug("%s work pendding\n",__func__);
        flush_work(&led->work);
	    schedule_work(&led->work);
    }
    return ret;
}
#endif

static void __qpnp_led_work(struct qpnp_led_data *led,
				enum led_brightness value)
{
	int rc;

	pan_debug("%s: id=%d, brightness=%d\n", __func__, led->id, value);
	mutex_lock(&led->lock);

	switch (led->id) {
	case QPNP_ID_WLED:
		rc = qpnp_wled_set(led);
		if (rc < 0)
			dev_err(&led->spmi_dev->dev,
				"WLED set brightness failed (%d)\n", rc);
		break;
	case QPNP_ID_FLASH1_LED0:
	case QPNP_ID_FLASH1_LED1:
		rc = qpnp_flash_set(led);
		if (rc < 0)
			dev_err(&led->spmi_dev->dev,
				"FLASH set brightness failed (%d)\n", rc);
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		rc = qpnp_rgb_set(led);
		if (rc < 0)
			dev_err(&led->spmi_dev->dev,
				"RGB set brightness failed (%d)\n", rc);		
		break;
	case QPNP_ID_LED_MPP:
		rc = qpnp_mpp_set(led);
		if (rc < 0)
			dev_err(&led->spmi_dev->dev,
					"MPP set brightness failed (%d)\n", rc);
		break;
	case QPNP_ID_KPDBL:
		rc = qpnp_kpdbl_set(led);
		if (rc < 0)
			dev_err(&led->spmi_dev->dev,
				"KPDBL set brightness failed (%d)\n", rc);
		break;
	default:
		dev_err(&led->spmi_dev->dev, "Invalid LED(%d)\n", led->id);
		break;
	}
	mutex_unlock(&led->lock);

}

static void qpnp_led_work(struct work_struct *work)
{
	struct qpnp_led_data *led = container_of(work,
					struct qpnp_led_data, work);

	__qpnp_led_work(led, led->cdev.brightness);

	return;
}

static int __devinit qpnp_led_set_max_brightness(struct qpnp_led_data *led)
{
	switch (led->id) {
	case QPNP_ID_WLED:
		led->cdev.max_brightness = WLED_MAX_LEVEL;
		break;
	case QPNP_ID_FLASH1_LED0:
	case QPNP_ID_FLASH1_LED1:
		led->cdev.max_brightness = led->max_current;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		led->cdev.max_brightness = RGB_MAX_LEVEL;
		break;
	case QPNP_ID_LED_MPP:
		if (led->mpp_cfg->pwm_mode == MANUAL_MODE)
			led->cdev.max_brightness = led->max_current;
		else
			led->cdev.max_brightness = MPP_MAX_LEVEL;
		break;
	case QPNP_ID_KPDBL:
		led->cdev.max_brightness = KPDBL_MAX_LEVEL;
		break;
	default:
		dev_err(&led->spmi_dev->dev, "Invalid LED(%d)\n", led->id);
		return -EINVAL;
	}

	return 0;
}

static enum led_brightness qpnp_led_get(struct led_classdev *led_cdev)
{
	struct qpnp_led_data *led;

	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	return led->cdev.brightness;
}

static void qpnp_led_turn_off_delayed(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct qpnp_led_data *led
		= container_of(dwork, struct qpnp_led_data, dwork);

	led->cdev.brightness = LED_OFF;
	qpnp_led_set(&led->cdev, led->cdev.brightness);
}

static void qpnp_led_turn_off(struct qpnp_led_data *led)
{
	INIT_DELAYED_WORK(&led->dwork, qpnp_led_turn_off_delayed);
	schedule_delayed_work(&led->dwork,
		msecs_to_jiffies(led->turn_off_delay_ms));
}

static int __devinit qpnp_wled_init(struct qpnp_led_data *led)
{
	int rc, i;
	u8 num_wled_strings;

	num_wled_strings = led->wled_cfg->num_strings;

	/* verify ranges */
	if (led->wled_cfg->ovp_val > WLED_OVP_27V) {
		dev_err(&led->spmi_dev->dev, "Invalid ovp value\n");
		return -EINVAL;
	}

	if (led->wled_cfg->boost_curr_lim > WLED_CURR_LIMIT_1680mA) {
		dev_err(&led->spmi_dev->dev, "Invalid boost current limit\n");
		return -EINVAL;
	}

	if (led->wled_cfg->cp_select > WLED_CP_SELECT_MAX) {
		dev_err(&led->spmi_dev->dev, "Invalid pole capacitance\n");
		return -EINVAL;
	}

	if ((led->max_current > WLED_MAX_CURR)) {
		dev_err(&led->spmi_dev->dev, "Invalid max current\n");
		return -EINVAL;
	}

	if ((led->wled_cfg->ctrl_delay_us % WLED_CTL_DLY_STEP) ||
		(led->wled_cfg->ctrl_delay_us > WLED_CTL_DLY_MAX)) {
		dev_err(&led->spmi_dev->dev, "Invalid control delay\n");
		return -EINVAL;
	}

	/* program over voltage protection threshold */
	rc = qpnp_led_masked_write(led, WLED_OVP_CFG_REG(led->base),
		WLED_OVP_VAL_MASK,
		(led->wled_cfg->ovp_val << WLED_OVP_VAL_BIT_SHFT));
	if (rc) {
		dev_err(&led->spmi_dev->dev,
				"WLED OVP reg write failed(%d)\n", rc);
		return rc;
	}

	/* program current boost limit */
	rc = qpnp_led_masked_write(led, WLED_BOOST_LIMIT_REG(led->base),
		WLED_BOOST_LIMIT_MASK, led->wled_cfg->boost_curr_lim);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
				"WLED boost limit reg write failed(%d)\n", rc);
		return rc;
	}

	/* program output feedback */
	rc = qpnp_led_masked_write(led, WLED_FDBCK_CTRL_REG(led->base),
		WLED_OP_FDBCK_MASK,
		(led->wled_cfg->op_fdbck << WLED_OP_FDBCK_BIT_SHFT));
	if (rc) {
		dev_err(&led->spmi_dev->dev,
				"WLED fdbck ctrl reg write failed(%d)\n", rc);
		return rc;
	}

	/* program switch frequency */
	rc = qpnp_led_masked_write(led,
		WLED_SWITCHING_FREQ_REG(led->base),
		WLED_SWITCH_FREQ_MASK, led->wled_cfg->switch_freq);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"WLED switch freq reg write failed(%d)\n", rc);
		return rc;
	}

	/* program current sink */
	if (led->wled_cfg->cs_out_en) {
		rc = qpnp_led_masked_write(led, WLED_CURR_SINK_REG(led->base),
			WLED_CURR_SINK_MASK,
			(((1 << led->wled_cfg->num_strings) - 1)
			<< WLED_CURR_SINK_SHFT));
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED curr sink reg write failed(%d)\n", rc);
			return rc;
		}
	}

	/* program high pole capacitance */
	rc = qpnp_led_masked_write(led, WLED_HIGH_POLE_CAP_REG(led->base),
		WLED_CP_SELECT_MASK, led->wled_cfg->cp_select);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
				"WLED pole cap reg write failed(%d)\n", rc);
		return rc;
	}

	/* program modulator, current mod src and cabc */
	for (i = 0; i < num_wled_strings; i++) {
		rc = qpnp_led_masked_write(led, WLED_MOD_EN_REG(led->base, i),
			WLED_NO_MASK, WLED_EN_MASK);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED mod enable reg write failed(%d)\n", rc);
			return rc;
		}

		if (led->wled_cfg->dig_mod_gen_en) {
			rc = qpnp_led_masked_write(led,
				WLED_MOD_SRC_SEL_REG(led->base, i),
				WLED_NO_MASK, WLED_USE_EXT_GEN_MOD_SRC);
			if (rc) {
				dev_err(&led->spmi_dev->dev,
				"WLED dig mod en reg write failed(%d)\n", rc);
			}
		}

		rc = qpnp_led_masked_write(led,
			WLED_FULL_SCALE_REG(led->base, i), WLED_MAX_CURR_MASK,
			led->max_current);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"WLED max current reg write failed(%d)\n", rc);
			return rc;
		}

	}

	/* dump wled registers */
	qpnp_dump_regs(led, wled_debug_regs, ARRAY_SIZE(wled_debug_regs));

	return 0;
}

static ssize_t led_mode_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	unsigned long state;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &state);
	if (ret)
		return ret;

	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	/* '1' to enable torch mode; '0' to switch to flash mode */
	if (state == 1)
		led->flash_cfg->torch_enable = true;
	else
		led->flash_cfg->torch_enable = false;

	return count;
}

static ssize_t led_strobe_type_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	unsigned long state;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &state);
	if (ret)
		return ret;

	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	/* '0' for sw strobe; '1' for hw strobe */
	if (state == 1)
		led->flash_cfg->strobe_type = 1;
	else
		led->flash_cfg->strobe_type = 0;

	return count;
}

static int qpnp_pwm_init(struct pwm_config_data *pwm_cfg,
					struct spmi_device *spmi_dev,
					const char *name)
{
	int rc, start_idx, idx_len;

	if (pwm_cfg->pwm_channel != -1) {
		pwm_cfg->pwm_dev =
			pwm_request(pwm_cfg->pwm_channel, name);

		if (IS_ERR_OR_NULL(pwm_cfg->pwm_dev)) {
			dev_err(&spmi_dev->dev,
				"could not acquire PWM Channel %d, " \
				"error %ld\n",
				pwm_cfg->pwm_channel,
				PTR_ERR(pwm_cfg->pwm_dev));
			pwm_cfg->pwm_dev = NULL;
			return -ENODEV;
		}

		if (pwm_cfg->mode == LPG_MODE) {
			start_idx =	pwm_cfg->duty_cycles->start_idx;
			idx_len = pwm_cfg->duty_cycles->num_duty_pcts;

			if (idx_len >= PWM_LUT_MAX_SIZE &&
					start_idx) {
				dev_err(&spmi_dev->dev,
					"Wrong LUT size or index\n");
				return -EINVAL;
			}
			if ((start_idx + idx_len) >
					PWM_LUT_MAX_SIZE) {
				dev_err(&spmi_dev->dev,
					"Exceed LUT limit\n");
				return -EINVAL;
			}
			rc = pwm_lut_config(pwm_cfg->pwm_dev,
				PM_PWM_PERIOD_MIN, /* ignored by hardware */
				pwm_cfg->duty_cycles->duty_pcts,
				pwm_cfg->lut_params);
			if (rc < 0) {
				dev_err(&spmi_dev->dev, "Failed to " \
					"configure pwm LUT\n");
				return rc;
			}
		}
	} else {
		dev_err(&spmi_dev->dev,
			"Invalid PWM channel\n");
		return -EINVAL;
	}

	return 0;
}

static ssize_t pwm_us_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	u32 pwm_us;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret;
	u32 previous_pwm_us;
	struct pwm_config_data *pwm_cfg;

	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	ret = kstrtou32(buf, 10, &pwm_us);
	if (ret)
		return ret;

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for pwm_us\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	previous_pwm_us = pwm_cfg->pwm_period_us;

	pwm_cfg->pwm_period_us = pwm_us;
	pwm_free(pwm_cfg->pwm_dev);
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret) {
		pwm_cfg->pwm_period_us = previous_pwm_us;
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm with new pwm_us value\n");
		return ret;
	}
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;
}

static ssize_t pause_lo_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	u32 pause_lo;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret;
	u32 previous_pause_lo;
	struct pwm_config_data *pwm_cfg;

	ret = kstrtou32(buf, 10, &pause_lo);
	if (ret)
		return ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for pause lo\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	previous_pause_lo = pwm_cfg->lut_params.lut_pause_lo;

	pwm_free(pwm_cfg->pwm_dev);
	pwm_cfg->lut_params.lut_pause_lo = pause_lo;
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret) {
		pwm_cfg->lut_params.lut_pause_lo = previous_pause_lo;
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm with new pause lo value\n");
		return ret;
	}
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;
}

static ssize_t pause_hi_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	u32 pause_hi;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret;
	u32 previous_pause_hi;
	struct pwm_config_data *pwm_cfg;

	ret = kstrtou32(buf, 10, &pause_hi);
	if (ret)
		return ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for pause hi\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	previous_pause_hi = pwm_cfg->lut_params.lut_pause_hi;

	pwm_free(pwm_cfg->pwm_dev);
	pwm_cfg->lut_params.lut_pause_hi = pause_hi;
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret) {
		pwm_cfg->lut_params.lut_pause_hi = previous_pause_hi;
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm with new pause hi value\n");
		return ret;
	}
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;
}

static ssize_t start_idx_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	u32 start_idx;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret;
	u32 previous_start_idx;
	struct pwm_config_data *pwm_cfg;

	ret = kstrtou32(buf, 10, &start_idx);
	if (ret)
		return ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for start idx\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	previous_start_idx = pwm_cfg->duty_cycles->start_idx;
	pwm_cfg->duty_cycles->start_idx = start_idx;
	pwm_cfg->lut_params.start_idx = pwm_cfg->duty_cycles->start_idx;
	pwm_free(pwm_cfg->pwm_dev);
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret) {
		pwm_cfg->duty_cycles->start_idx = previous_start_idx;
		pwm_cfg->lut_params.start_idx = pwm_cfg->duty_cycles->start_idx;
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm with new start idx value\n");
		return ret;
	}
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;
}

static ssize_t ramp_step_ms_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	u32 ramp_step_ms;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret;
	u32 previous_ramp_step_ms;
	struct pwm_config_data *pwm_cfg;

	ret = kstrtou32(buf, 10, &ramp_step_ms);
	if (ret)
		return ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for ramp step\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	previous_ramp_step_ms = pwm_cfg->lut_params.ramp_step_ms;

	pwm_free(pwm_cfg->pwm_dev);
	pwm_cfg->lut_params.ramp_step_ms = ramp_step_ms;
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret) {
		pwm_cfg->lut_params.ramp_step_ms = previous_ramp_step_ms;
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm with new ramp step value\n");
		return ret;
	}
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;
}

static ssize_t lut_flags_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	u32 lut_flags;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret;
	u32 previous_lut_flags;
	struct pwm_config_data *pwm_cfg;

	ret = kstrtou32(buf, 10, &lut_flags);
	if (ret)
		return ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for lut flags\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	previous_lut_flags = pwm_cfg->lut_params.flags;

	pwm_free(pwm_cfg->pwm_dev);
	pwm_cfg->lut_params.flags = lut_flags;
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret) {
		pwm_cfg->lut_params.flags = previous_lut_flags;
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm with new lut flags value\n");
		return ret;
	}
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;
}

static ssize_t duty_pcts_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	int num_duty_pcts = 0;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	char *buffer;
	ssize_t ret;
	int i = 0;
	int max_duty_pcts;
	struct pwm_config_data *pwm_cfg;
	u32 previous_num_duty_pcts;
	int value;
	int *previous_duty_pcts;

	led = container_of(led_cdev, struct qpnp_led_data, cdev);

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		pwm_cfg = led->mpp_cfg->pwm_cfg;
		max_duty_pcts = PWM_LUT_MAX_SIZE;
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		pwm_cfg = led->rgb_cfg->pwm_cfg;
		max_duty_pcts = PWM_LUT_MAX_SIZE;
		break;
	default:
		dev_err(&led->spmi_dev->dev,
			"Invalid LED id type for duty pcts\n");
		return -EINVAL;
	}

	if (pwm_cfg->mode == LPG_MODE)
		pwm_cfg->blinking = true;

	buffer = (char *)buf;

	for (i = 0; i < max_duty_pcts; i++) {
		if (buffer == NULL)
			break;
		ret = sscanf((const char *)buffer, "%u,%s", &value, buffer);
		pwm_cfg->old_duty_pcts[i] = value;
		num_duty_pcts++;
		if (ret <= 1)
			break;
	}

	if (num_duty_pcts >= max_duty_pcts) {
		dev_err(&led->spmi_dev->dev,
			"Number of duty pcts given exceeds max (%d)\n",
			max_duty_pcts);
		return -EINVAL;
	}

	previous_num_duty_pcts = pwm_cfg->duty_cycles->num_duty_pcts;
	previous_duty_pcts = pwm_cfg->duty_cycles->duty_pcts;

	pwm_cfg->duty_cycles->num_duty_pcts = num_duty_pcts;
	pwm_cfg->duty_cycles->duty_pcts = pwm_cfg->old_duty_pcts;
	pwm_cfg->old_duty_pcts = previous_duty_pcts;
	pwm_cfg->lut_params.idx_len = pwm_cfg->duty_cycles->num_duty_pcts;

	pwm_free(pwm_cfg->pwm_dev);
	ret = qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	if (ret)
		goto restore;

	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return count;

restore:
	dev_err(&led->spmi_dev->dev,
		"Failed to initialize pwm with new duty pcts value\n");
	pwm_cfg->duty_cycles->num_duty_pcts = previous_num_duty_pcts;
	pwm_cfg->old_duty_pcts = pwm_cfg->duty_cycles->duty_pcts;
	pwm_cfg->duty_cycles->duty_pcts = previous_duty_pcts;
	pwm_cfg->lut_params.idx_len = pwm_cfg->duty_cycles->num_duty_pcts;
	pwm_free(pwm_cfg->pwm_dev);
	qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
	qpnp_led_set(&led->cdev, led->cdev.brightness);
	return ret;
}

static void led_blink(struct qpnp_led_data *led,
			struct pwm_config_data *pwm_cfg)
{
	if (pwm_cfg->use_blink) {
		if (led->cdev.brightness) {
			pwm_cfg->blinking = true;
			if (led->id == QPNP_ID_LED_MPP)
				led->mpp_cfg->pwm_mode = LPG_MODE;
			pwm_cfg->mode = LPG_MODE;
		} else {
			pwm_cfg->blinking = false;
			pwm_cfg->mode = pwm_cfg->default_mode;
			if (led->id == QPNP_ID_LED_MPP)
				led->mpp_cfg->pwm_mode = pwm_cfg->default_mode;
		}
		pwm_free(pwm_cfg->pwm_dev);
		qpnp_pwm_init(pwm_cfg, led->spmi_dev, led->cdev.name);
		qpnp_led_set(&led->cdev, led->cdev.brightness);
	}
}

static ssize_t blink_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct qpnp_led_data *led;
	unsigned long blinking;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &blinking);
	if (ret)
		return ret;
	led = container_of(led_cdev, struct qpnp_led_data, cdev);
	led->cdev.brightness = blinking ? led->cdev.max_brightness : 0;

	switch (led->id) {
	case QPNP_ID_LED_MPP:
		led_blink(led, led->mpp_cfg->pwm_cfg);
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		led_blink(led, led->rgb_cfg->pwm_cfg);
		break;
	default:
		dev_err(&led->spmi_dev->dev, "Invalid LED id type for blink\n");
		return -EINVAL;
	}
	return count;
}

//++ p11309 - 2013.12.03 for LED Pattern Customization
#ifdef PAN_LED_CONTROL
static void pan_rgb_led_off(int id) 
{
	if ( pan_led_data.led_state[id] == PAN_LED_OFF ) return;

	pan_rgb_pattern.is_white = 0;
	pan_rgb_pattern.is_rainbow = 0;
	pan_led_data.led[id]->cdev.brightness = 0;
	//__qpnp_led_work(pan_led_data.led[id], 0);	
	pan_pattern_process_loop(id, PAN_DIM_ALL_ZERO);
	pan_led_data.led_state[id] = PAN_LED_OFF;
    pan_debug("%s id : %d\n" , __func__ , id);
}

static ssize_t pan_rainbow_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{	
	unsigned long rainbow;	
	ssize_t ret = -EINVAL;
	
	int i=0;
	int pattern;
	int lut;
	int setup_t;
	int hold_t;

	ret = kstrtoul(buf, 10, &rainbow);
	if (ret){
        pan_debug("%s data is overflow!\n",__func__);
		return ret;
    }

	pattern = rainbow & 0xFF;
	lut = (rainbow >> 8) & 0xFF;
	setup_t = ((rainbow >> 16) & 0xFF)*10;
	hold_t = ((rainbow >> 24) & 0xFF)*10;

	pan_debug("%s: pattern=%d, lut=%d, setup_time=%d, keep_time=%d\n", 
		__func__, pattern, lut, setup_t, hold_t);
	
	pan_rgb_pattern.is_white = 0;
	pan_rgb_pattern.is_rainbow = 0;
	del_timer(&pan_led_data.timer);
//	cancel_work_sync(&pan_led_data.wq);
	
	if (pattern) {		
		pan_do_rainbow(pattern-1, lut, setup_t, hold_t);

		pan_led_data.led_state[RGB_RED] = PAN_LED_RAINBOW;
		pan_led_data.led_state[RGB_GREEN] = PAN_LED_RAINBOW;
		pan_led_data.led_state[RGB_BLUE] = PAN_LED_RAINBOW;
	}
	else {
		for (i=0; i<RGB_MAX; i++) 
			pan_rgb_led_off(i);
	}

	return count;
}

static ssize_t pan_oneshot_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{	
	unsigned long data;	
	ssize_t ret = -EINVAL;
	
	int id;
	int rgb;
	int lut;
	int dim_mode;
	int setup_t;
	
	ret = kstrtoul(buf, 10, &data);
	if (ret){
        pan_debug("%s data is overflow!\n",__func__);
		return ret;
    }
	
	rgb = data & 0xFF;
	lut = (data >> 8) & 0xFF;
	dim_mode = ((data >> 16) & 0xFF);
	setup_t = ((data >> 24) & 0xFF)*10;

	pan_debug("%s: rgb=%d, lut=%d, dim_mode=%d, setup_time=%d\n", 
		__func__, rgb, lut, dim_mode, setup_t);	

	if ( rgb == 7 ) 
		pan_rgb_pattern.is_white = 1;

	pan_rgb_pattern.rise_mode = lut;
	pan_rgb_pattern.setup_t = setup_t;	
	for (id=0; id<RGB_MAX; id++) {
		if ( rgb & (1<<id) ) {
			if ( dim_mode == PAN_DIM_ALL_ZERO ) 
				pan_rgb_led_off(id);
			else {
				pan_led_data.led_state[id] = PAN_LED_ONESHOT;
				pan_pattern_process_oneshot(id, dim_mode);
			}
		}
		else 
			pan_rgb_led_off(id);
	}	

	return count;
}

static ssize_t pan_loop_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{	
	unsigned long data;	
	ssize_t ret = -EINVAL;
	
	int id;
	int rgb;
	int lut;
	int dim_mode;
	int setup_t;
	
	ret = kstrtoul(buf, 10, &data);
	if (ret){
        pan_debug("%s data is overflow!\n",__func__);
		return ret;
    }
	
	rgb = data & 0xFF;
	lut = (data >> 8) & 0xFF;
	dim_mode = ((data >> 16) & 0xFF);
	setup_t = ((data >> 24) & 0xFF)*10;

	pan_debug("%s: rgb=%d, lut=%d, dim_mode=%d, setup_time=%d\n", 
		__func__, rgb, lut, dim_mode, setup_t);

	if ( rgb == 7 ) 
		pan_rgb_pattern.is_white = 1;

	pan_rgb_pattern.rise_mode = lut;
	pan_rgb_pattern.setup_t = setup_t;	
	for (id=0; id<RGB_MAX; id++) {
		if ( rgb & (1<<id) ) {
			if ( dim_mode == PAN_DIM_ALL_ZERO ) 
				pan_rgb_led_off(id);
			else {
				pan_led_data.led_state[id] = PAN_LED_LOOP;
				pan_pattern_process_loop(id, dim_mode);
			}
		}
		else 
			pan_rgb_led_off(id);
	}	

	return count;
}

static ssize_t pan_blink_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{	
	unsigned long data;	
	ssize_t ret = -EINVAL;	
	
	int id;
	int rgb;
	int on_cnt;
	int on;
	int off;	
	int led_cnt=0;
	int lut_max_size;
	int lut_offset[3] = {1, 21, 42};

	ret = kstrtoul(buf, 10, &data);
	if (ret){
        pan_debug("%s data is overflow!\n",__func__);
		return ret;	
    }

	rgb = data & 0xF;
	on_cnt = ((data >> 4) & 0xF);
	on = ((data >> 8) & 0xFF)*10;
	off = ((data >> 16) & 0xFFFF)*10;	

	for (id=0; id<RGB_MAX; id++) {
		if ( rgb & (1<<id) )
			led_cnt++;
	}
	
	switch (led_cnt) {
	case 0: return count;
	case 1:
		lut_max_size = 61;
		lut_offset[1] = 1;
		lut_offset[2] = 1;
		break;
	case 2:
		lut_max_size = 31;
		lut_offset[1] = 32;
		lut_offset[2] = 32;
		break;
	case 3:
		lut_max_size = 21;
		break;
	default:
		return count;
	}

	if ( rgb == 7 ) 
		pan_rgb_pattern.is_white = 1;

	if ( on_cnt > 5 ) on_cnt = 5;
	if ( on_cnt <= 0 ) on_cnt = 1;
	
	pan_debug("%s: rgb=%d, count=%d, on=%d, off=%d, led_cnt=%d\n", 
		__func__, rgb, on_cnt, on, off, led_cnt);

	led_cnt=0;
	for (id=0; id<RGB_MAX; id++) {
		if ( rgb & (1<<id) ) {
			pan_led_data.led_state[id] = PAN_LED_BLINK;
			pan_blink_process(id, on_cnt, on, off, 
				lut_offset[led_cnt], lut_max_size);
			led_cnt++;
		}
		else 
			pan_rgb_led_off(id);
	}

	return count;
}

static ssize_t pan_rgb_off_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{	
	unsigned long data;	
	ssize_t ret = -EINVAL;	
	
	int id;
	int rgb;	

	ret = kstrtoul(buf, 10, &data);
	if (ret){
        pan_debug("%s data is overflow!\n",__func__);
		return ret;	
    }

	rgb = data & 0xFF;	
    pan_rgb_pattern.control_rainbow_timer = 0;

	pan_debug("%s: rgb=%d , control_rainbow_timer=%d\n", __func__, rgb , pan_rgb_pattern.control_rainbow_timer);
	for (id=0; id<RGB_MAX; id++) {
		if ( rgb & (1<<id) )
			pan_rgb_led_off(id);
	}

	return count;
}

static ssize_t pan_brightness_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{	
	unsigned long data;	
	ssize_t ret = -EINVAL;	
	
	int id;
	int rgb;	
	int brightness;	

	ret = kstrtoul(buf, 10, &data);
	if (ret){
        pan_debug("%s data is overflow!\n",__func__);
		return ret;	
    }

	pan_rgb_pattern.is_white = 0;

	rgb = data & 0xFF;		
	brightness = (data >> 8) & 0xFFFFFF;	

	pan_debug("%s: rgb=%d, brightness=%d\n", __func__, rgb, brightness);

	for (id=0; id<RGB_MAX; id++) {
		if ( rgb & (1<<id) ) {
			pan_led_data.led_state[id] = PAN_LED_BRIGHTNESS;
			pan_pattern_brightness(id, brightness);
		}
	}

	return count;
}

static DEVICE_ATTR(pan_rainbow, 0664, NULL, pan_rainbow_store);
static DEVICE_ATTR(pan_oneshot, 0664, NULL, pan_oneshot_store);
static DEVICE_ATTR(pan_loop, 0664, NULL, pan_loop_store);
static DEVICE_ATTR(pan_blink, 0664, NULL, pan_blink_store);
static DEVICE_ATTR(pan_rgb_off, 0664, NULL, pan_rgb_off_store);
static DEVICE_ATTR(pan_brightness, 0664, NULL, pan_brightness_store);
#endif // PAN_LED_CONTROL
//--p11309

static DEVICE_ATTR(led_mode, 0664, NULL, led_mode_store);
static DEVICE_ATTR(strobe, 0664, NULL, led_strobe_type_store);
static DEVICE_ATTR(pwm_us, 0664, NULL, pwm_us_store);
static DEVICE_ATTR(pause_lo, 0664, NULL, pause_lo_store);
static DEVICE_ATTR(pause_hi, 0664, NULL, pause_hi_store);
static DEVICE_ATTR(start_idx, 0664, NULL, start_idx_store);
static DEVICE_ATTR(ramp_step_ms, 0664, NULL, ramp_step_ms_store);
static DEVICE_ATTR(lut_flags, 0664, NULL, lut_flags_store);
static DEVICE_ATTR(duty_pcts, 0664, NULL, duty_pcts_store);
static DEVICE_ATTR(blink, 0664, NULL, blink_store);

static struct attribute *led_attrs[] = {
	&dev_attr_led_mode.attr,
	&dev_attr_strobe.attr,
	NULL
};

static const struct attribute_group led_attr_group = {
	.attrs = led_attrs,
};

static struct attribute *pwm_attrs[] = {
	&dev_attr_pwm_us.attr,
	NULL
};

static struct attribute *lpg_attrs[] = {
	&dev_attr_pause_lo.attr,
	&dev_attr_pause_hi.attr,
	&dev_attr_start_idx.attr,
	&dev_attr_ramp_step_ms.attr,
	&dev_attr_lut_flags.attr,
	&dev_attr_duty_pcts.attr,
//++ p11309 - 2013.12.03 for LED Pattern Customization
#ifdef PAN_LED_CONTROL
	&dev_attr_pan_rainbow.attr,
	&dev_attr_pan_oneshot.attr,
	&dev_attr_pan_loop.attr,
	&dev_attr_pan_blink.attr,
	&dev_attr_pan_rgb_off.attr,
	&dev_attr_pan_brightness.attr,
#endif
//-- p11309
	NULL
};

static struct attribute *blink_attrs[] = {
	&dev_attr_blink.attr,
	NULL
};

static const struct attribute_group pwm_attr_group = {
	.attrs = pwm_attrs,
};

static const struct attribute_group lpg_attr_group = {
	.attrs = lpg_attrs,
};

static const struct attribute_group blink_attr_group = {
	.attrs = blink_attrs,
};

static int __devinit qpnp_flash_init(struct qpnp_led_data *led)
{
	int rc;

	led->flash_cfg->flash_on = false;

	rc = qpnp_led_masked_write(led,
		FLASH_LED_STROBE_CTRL(led->base),
		FLASH_STROBE_MASK, FLASH_DISABLE_ALL);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"LED %d flash write failed(%d)\n", led->id, rc);
		return rc;
	}

	/* Disable flash LED module */
	rc = qpnp_led_masked_write(led, FLASH_ENABLE_CONTROL(led->base),
		FLASH_ENABLE_MASK, FLASH_DISABLE_ALL);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Enable reg write failed(%d)\n", rc);
		return rc;
	}

	if (led->flash_cfg->torch_enable)
		return 0;

	/* Set headroom */
	rc = qpnp_led_masked_write(led, FLASH_HEADROOM(led->base),
		FLASH_HEADROOM_MASK, led->flash_cfg->headroom);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Headroom reg write failed(%d)\n", rc);
		return rc;
	}

	/* Set startup delay */
	rc = qpnp_led_masked_write(led,
		FLASH_STARTUP_DELAY(led->base), FLASH_STARTUP_DLY_MASK,
		led->flash_cfg->startup_dly);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Startup delay reg write failed(%d)\n", rc);
		return rc;
	}

	/* Set timer control - safety or watchdog */
	if (led->flash_cfg->safety_timer) {
		rc = qpnp_led_masked_write(led,
			FLASH_LED_TMR_CTRL(led->base),
			FLASH_TMR_MASK, FLASH_TMR_SAFETY);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"LED timer ctrl reg write failed(%d)\n",
				rc);
			return rc;
		}
	}

	/* Set Vreg force */
	rc = qpnp_led_masked_write(led,	FLASH_VREG_OK_FORCE(led->base),
		FLASH_VREG_MASK, FLASH_HW_VREG_OK);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Vreg OK reg write failed(%d)\n", rc);
		return rc;
	}

	/* Set self fault check */
	rc = qpnp_led_masked_write(led, FLASH_FAULT_DETECT(led->base),
		FLASH_FAULT_DETECT_MASK, FLASH_SELFCHECK_ENABLE);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Fault detect reg write failed(%d)\n", rc);
		return rc;
	}

	/* Set mask enable */
	rc = qpnp_led_masked_write(led, FLASH_MASK_ENABLE(led->base),
		FLASH_MASK_REG_MASK, FLASH_MASK_1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Mask enable reg write failed(%d)\n", rc);
		return rc;
	}

	/* Set current ramp */
	rc = qpnp_led_masked_write(led, FLASH_CURRENT_RAMP(led->base),
		FLASH_CURRENT_RAMP_MASK, FLASH_RAMP_STEP_27US);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Current ramp reg write failed(%d)\n", rc);
		return rc;
	}

	led->flash_cfg->strobe_type = 0;

#ifdef CONFIG_PANTECH_CAMERA //flash
	global_torch_enable = false;
#endif

	/* dump flash registers */
	qpnp_dump_regs(led, flash_debug_regs, ARRAY_SIZE(flash_debug_regs));

	return 0;
}

static int __devinit qpnp_kpdbl_init(struct qpnp_led_data *led)
{
	int rc;
	u8 val;

	/* select row source - vbst or vph */
	rc = spmi_ext_register_readl(led->spmi_dev->ctrl, led->spmi_dev->sid,
				KPDBL_ROW_SRC_SEL(led->base), &val, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to read from addr=%x, rc(%d)\n",
			KPDBL_ROW_SRC_SEL(led->base), rc);
		return rc;
	}

	if (led->kpdbl_cfg->row_src_vbst)
		val |= 1 << led->kpdbl_cfg->row_id;
	else
		val &= ~(1 << led->kpdbl_cfg->row_id);

	rc = spmi_ext_register_writel(led->spmi_dev->ctrl, led->spmi_dev->sid,
				KPDBL_ROW_SRC_SEL(led->base), &val, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to read from addr=%x, rc(%d)\n",
			KPDBL_ROW_SRC_SEL(led->base), rc);
		return rc;
	}

	/* row source enable */
	rc = spmi_ext_register_readl(led->spmi_dev->ctrl, led->spmi_dev->sid,
				KPDBL_ROW_SRC(led->base), &val, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to read from addr=%x, rc(%d)\n",
			KPDBL_ROW_SRC(led->base), rc);
		return rc;
	}

	if (led->kpdbl_cfg->row_src_en)
		val |= KPDBL_ROW_SCAN_EN_MASK | (1 << led->kpdbl_cfg->row_id);
	else
		val &= ~(1 << led->kpdbl_cfg->row_id);

	rc = spmi_ext_register_writel(led->spmi_dev->ctrl, led->spmi_dev->sid,
		KPDBL_ROW_SRC(led->base), &val, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to write to addr=%x, rc(%d)\n",
			KPDBL_ROW_SRC(led->base), rc);
		return rc;
	}

	/* enable module */
	rc = qpnp_led_masked_write(led, KPDBL_ENABLE(led->base),
		KPDBL_MODULE_EN_MASK, KPDBL_MODULE_EN);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Enable module write failed(%d)\n", rc);
		return rc;
	}

	rc = qpnp_pwm_init(led->kpdbl_cfg->pwm_cfg, led->spmi_dev,
				led->cdev.name);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm\n");
		return rc;
	}

	/* dump kpdbl registers */
	qpnp_dump_regs(led, kpdbl_debug_regs, ARRAY_SIZE(kpdbl_debug_regs));

	return 0;
}

static int __devinit qpnp_rgb_init(struct qpnp_led_data *led)
{
	int rc;

	rc = qpnp_led_masked_write(led, RGB_LED_SRC_SEL(led->base),
		RGB_LED_SRC_MASK, RGB_LED_SOURCE_VPH_PWR);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Failed to write led source select register\n");
		return rc;
	}

	rc = qpnp_pwm_init(led->rgb_cfg->pwm_cfg, led->spmi_dev,
				led->cdev.name);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Failed to initialize pwm\n");
		return rc;
	}
	/* Initialize led for use in auto trickle charging mode */
	rc = qpnp_led_masked_write(led, RGB_LED_ATC_CTL(led->base),
		led->rgb_cfg->enable, led->rgb_cfg->enable);

	return 0;
}

static int __devinit qpnp_mpp_init(struct qpnp_led_data *led)
{
	int rc, val;


	if (led->max_current < LED_MPP_CURRENT_MIN ||
		led->max_current > LED_MPP_CURRENT_MAX) {
		dev_err(&led->spmi_dev->dev,
			"max current for mpp is not valid\n");
		return -EINVAL;
	}

	val = (led->mpp_cfg->current_setting / LED_MPP_CURRENT_PER_SETTING) - 1;

	if (val < 0)
		val = 0;

	rc = qpnp_led_masked_write(led, LED_MPP_VIN_CTRL(led->base),
		LED_MPP_VIN_MASK, led->mpp_cfg->vin_ctrl);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Failed to write led vin control reg\n");
		return rc;
	}

	rc = qpnp_led_masked_write(led, LED_MPP_SINK_CTRL(led->base),
		LED_MPP_SINK_MASK, val);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Failed to write sink control reg\n");
		return rc;
	}

	if (led->mpp_cfg->pwm_mode != MANUAL_MODE) {
		rc = qpnp_pwm_init(led->mpp_cfg->pwm_cfg, led->spmi_dev,
					led->cdev.name);
		if (rc) {
			dev_err(&led->spmi_dev->dev,
				"Failed to initialize pwm\n");
			return rc;
		}
	}

	return 0;
}

static int __devinit qpnp_led_initialize(struct qpnp_led_data *led)
{
	int rc = 0;

	switch (led->id) {
	case QPNP_ID_WLED:
		rc = qpnp_wled_init(led);
		if (rc)
			dev_err(&led->spmi_dev->dev,
				"WLED initialize failed(%d)\n", rc);
		break;
	case QPNP_ID_FLASH1_LED0:
	case QPNP_ID_FLASH1_LED1:
		rc = qpnp_flash_init(led);
		if (rc)
			dev_err(&led->spmi_dev->dev,
				"FLASH initialize failed(%d)\n", rc);
		break;
	case QPNP_ID_RGB_RED:
	case QPNP_ID_RGB_GREEN:
	case QPNP_ID_RGB_BLUE:
		rc = qpnp_rgb_init(led);
		if (rc)
			dev_err(&led->spmi_dev->dev,
				"RGB initialize failed(%d)\n", rc);
		break;
	case QPNP_ID_LED_MPP:
		rc = qpnp_mpp_init(led);
		if (rc)
			dev_err(&led->spmi_dev->dev,
				"MPP initialize failed(%d)\n", rc);
		break;
	case QPNP_ID_KPDBL:
		rc = qpnp_kpdbl_init(led);
		if (rc)
			dev_err(&led->spmi_dev->dev,
				"KPDBL initialize failed(%d)\n", rc);
		break;
	default:
		dev_err(&led->spmi_dev->dev, "Invalid LED(%d)\n", led->id);
		return -EINVAL;
	}

	return rc;
}

static int __devinit qpnp_get_common_configs(struct qpnp_led_data *led,
				struct device_node *node)
{
	int rc;
	u32 val;
	const char *temp_string;

	led->cdev.default_trigger = LED_TRIGGER_DEFAULT;
	rc = of_property_read_string(node, "linux,default-trigger",
		&temp_string);
	if (!rc)
		led->cdev.default_trigger = temp_string;
	else if (rc != -EINVAL)
		return rc;

	led->default_on = false;
	rc = of_property_read_string(node, "qcom,default-state",
		&temp_string);
	if (!rc) {
		if (strncmp(temp_string, "on", sizeof("on")) == 0)
			led->default_on = true;
	} else if (rc != -EINVAL)
		return rc;

	led->turn_off_delay_ms = 0;
	rc = of_property_read_u32(node, "qcom,turn-off-delay-ms", &val);
	if (!rc)
		led->turn_off_delay_ms = val;
	else if (rc != -EINVAL)
		return rc;

	return 0;
}

/*
 * Handlers for alternative sources of platform_data
 */
static int __devinit qpnp_get_config_wled(struct qpnp_led_data *led,
				struct device_node *node)
{
	u32 val;
	int rc;

	led->wled_cfg = devm_kzalloc(&led->spmi_dev->dev,
				sizeof(struct wled_config_data), GFP_KERNEL);
	if (!led->wled_cfg) {
		dev_err(&led->spmi_dev->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

	rc = spmi_ext_register_readl(led->spmi_dev->ctrl, led->spmi_dev->sid,
		PMIC_VERSION_REG, &led->wled_cfg->pmic_version, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to read pmic ver, rc(%d)\n", rc);
	}

	led->wled_cfg->num_strings = WLED_DEFAULT_STRINGS;
	rc = of_property_read_u32(node, "qcom,num-strings", &val);
	if (!rc)
		led->wled_cfg->num_strings = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->ovp_val = WLED_DEFAULT_OVP_VAL;
	rc = of_property_read_u32(node, "qcom,ovp-val", &val);
	if (!rc)
		led->wled_cfg->ovp_val = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->boost_curr_lim = WLED_BOOST_LIM_DEFAULT;
	rc = of_property_read_u32(node, "qcom,boost-curr-lim", &val);
	if (!rc)
		led->wled_cfg->boost_curr_lim = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->cp_select = WLED_CP_SEL_DEFAULT;
	rc = of_property_read_u32(node, "qcom,cp-sel", &val);
	if (!rc)
		led->wled_cfg->cp_select = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->ctrl_delay_us = WLED_CTRL_DLY_DEFAULT;
	rc = of_property_read_u32(node, "qcom,ctrl-delay-us", &val);
	if (!rc)
		led->wled_cfg->ctrl_delay_us = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->op_fdbck = WLED_OP_FDBCK_DEFAULT;
	rc = of_property_read_u32(node, "qcom,op-fdbck", &val);
	if (!rc)
		led->wled_cfg->op_fdbck = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->switch_freq = WLED_SWITCH_FREQ_DEFAULT;
	rc = of_property_read_u32(node, "qcom,switch-freq", &val);
	if (!rc)
		led->wled_cfg->switch_freq = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->wled_cfg->dig_mod_gen_en =
		of_property_read_bool(node, "qcom,dig-mod-gen-en");

	led->wled_cfg->cs_out_en =
		of_property_read_bool(node, "qcom,cs-out-en");

	return 0;
}

static int __devinit qpnp_get_config_flash(struct qpnp_led_data *led,
				struct device_node *node, bool *reg_set)
{
	int rc;
	u32 val;

	led->flash_cfg = devm_kzalloc(&led->spmi_dev->dev,
				sizeof(struct flash_config_data), GFP_KERNEL);
	if (!led->flash_cfg) {
		dev_err(&led->spmi_dev->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

	rc = spmi_ext_register_readl(led->spmi_dev->ctrl, led->spmi_dev->sid,
			FLASH_PERIPHERAL_SUBTYPE(led->base),
			&led->flash_cfg->peripheral_subtype, 1);
	if (rc) {
		dev_err(&led->spmi_dev->dev,
			"Unable to read from addr=%x, rc(%d)\n",
			FLASH_PERIPHERAL_SUBTYPE(led->base), rc);
	}

	led->flash_cfg->torch_enable =
		of_property_read_bool(node, "qcom,torch-enable");

	if (led->id == QPNP_ID_FLASH1_LED0) {
		led->flash_cfg->enable_module = FLASH_ENABLE_LED_0;
		led->flash_cfg->current_addr = FLASH_LED_0_CURR(led->base);
		led->flash_cfg->trigger_flash = FLASH_LED_0_OUTPUT;
		if (!*reg_set) {
			led->flash_cfg->flash_boost_reg =
				regulator_get(&led->spmi_dev->dev,
							"flash-boost");
			if (IS_ERR(led->flash_cfg->flash_boost_reg)) {
				rc = PTR_ERR(led->flash_cfg->flash_boost_reg);
				dev_err(&led->spmi_dev->dev,
					"Regulator get failed(%d)\n", rc);
				goto error_get_flash_reg;
			}
			led->flash_cfg->flash_reg_get = true;
			*reg_set = true;
		} else
			led->flash_cfg->flash_reg_get = false;

		if (led->flash_cfg->torch_enable) {
			led->flash_cfg->second_addr =
						FLASH_LED_1_CURR(led->base);
		}
	} else if (led->id == QPNP_ID_FLASH1_LED1) {
		led->flash_cfg->enable_module = FLASH_ENABLE_LED_1;
		led->flash_cfg->current_addr = FLASH_LED_1_CURR(led->base);
		led->flash_cfg->trigger_flash = FLASH_LED_1_OUTPUT;
		if (!*reg_set) {
			led->flash_cfg->flash_boost_reg =
					regulator_get(&led->spmi_dev->dev,
								"flash-boost");
			if (IS_ERR(led->flash_cfg->flash_boost_reg)) {
				rc = PTR_ERR(led->flash_cfg->flash_boost_reg);
				dev_err(&led->spmi_dev->dev,
					"Regulator get failed(%d)\n", rc);
				goto error_get_flash_reg;
			}
			led->flash_cfg->flash_reg_get = true;
			*reg_set = true;
		} else
			led->flash_cfg->flash_reg_get = false;

		if (led->flash_cfg->torch_enable) {
			led->flash_cfg->second_addr =
						FLASH_LED_0_CURR(led->base);
		}
	} else {
		dev_err(&led->spmi_dev->dev, "Unknown flash LED name given\n");
		return -EINVAL;
	}

	if (led->flash_cfg->torch_enable) {
		if (of_find_property(of_get_parent(node), "torch-boost-supply",
									NULL)) {
			led->flash_cfg->torch_boost_reg =
				regulator_get(&led->spmi_dev->dev,
								"torch-boost");
			if (IS_ERR(led->flash_cfg->torch_boost_reg)) {
				rc = PTR_ERR(led->flash_cfg->torch_boost_reg);
				dev_err(&led->spmi_dev->dev,
					"Torch regulator get failed(%d)\n", rc);
				goto error_get_torch_reg;
			}
			led->flash_cfg->enable_module = FLASH_ENABLE_MODULE;
		} else
			led->flash_cfg->enable_module = FLASH_ENABLE_ALL;
		led->flash_cfg->trigger_flash = FLASH_TORCH_OUTPUT;
	}

	rc = of_property_read_u32(node, "qcom,current", &val);
	if (!rc) {
		if (led->flash_cfg->torch_enable) {
			led->flash_cfg->current_prgm = (val *
				TORCH_MAX_LEVEL / led->max_current);
			return 0;
		}
		else
			led->flash_cfg->current_prgm = (val *
				FLASH_MAX_LEVEL / led->max_current);
	} else
		if (led->flash_cfg->torch_enable)
			goto error_get_torch_reg;
		else
			goto error_get_flash_reg;

	rc = of_property_read_u32(node, "qcom,headroom", &val);
	if (!rc)
		led->flash_cfg->headroom = (u8) val;
	else if (rc == -EINVAL)
		led->flash_cfg->headroom = HEADROOM_500mV;
	else
		goto error_get_flash_reg;

	rc = of_property_read_u32(node, "qcom,duration", &val);
	if (!rc)
		led->flash_cfg->duration = (u8)((val - 10) / 10);
	else if (rc == -EINVAL)
		led->flash_cfg->duration = FLASH_DURATION_200ms;
	else
		goto error_get_flash_reg;

	rc = of_property_read_u32(node, "qcom,clamp-curr", &val);
	if (!rc)
		led->flash_cfg->clamp_curr = (val *
				FLASH_MAX_LEVEL / led->max_current);
	else if (rc == -EINVAL)
		led->flash_cfg->clamp_curr = FLASH_CLAMP_200mA;
	else
		goto error_get_flash_reg;

	rc = of_property_read_u32(node, "qcom,startup-dly", &val);
	if (!rc)
		led->flash_cfg->startup_dly = (u8) val;
	else if (rc == -EINVAL)
		led->flash_cfg->startup_dly = DELAY_128us;
	else
		goto error_get_flash_reg;

	led->flash_cfg->safety_timer =
		of_property_read_bool(node, "qcom,safety-timer");

	return 0;

error_get_torch_reg:
	regulator_put(led->flash_cfg->torch_boost_reg);

error_get_flash_reg:
	regulator_put(led->flash_cfg->flash_boost_reg);
	return rc;

}

static int __devinit qpnp_get_config_pwm(struct pwm_config_data *pwm_cfg,
				struct spmi_device *spmi_dev,
				struct device_node *node)
{
	struct property *prop;
	int rc, i;
	u32 val;
	u8 *temp_cfg;

	rc = of_property_read_u32(node, "qcom,pwm-channel", &val);
	if (!rc)
		pwm_cfg->pwm_channel = val;
	else
		return rc;

	if (pwm_cfg->mode == PWM_MODE) {
		rc = of_property_read_u32(node, "qcom,pwm-us", &val);
		if (!rc)
			pwm_cfg->pwm_period_us = val;
		else
			return rc;
	}

	pwm_cfg->use_blink =
		of_property_read_bool(node, "qcom,use-blink");

	if (pwm_cfg->mode == LPG_MODE || pwm_cfg->use_blink) {
		pwm_cfg->duty_cycles =
			devm_kzalloc(&spmi_dev->dev,
			sizeof(struct pwm_duty_cycles), GFP_KERNEL);
		if (!pwm_cfg->duty_cycles) {
			dev_err(&spmi_dev->dev,
				"Unable to allocate memory\n");
			rc = -ENOMEM;
			goto bad_lpg_params;
		}

		prop = of_find_property(node, "qcom,duty-pcts",
			&pwm_cfg->duty_cycles->num_duty_pcts);
		if (!prop) {
			dev_err(&spmi_dev->dev, "Looking up property " \
				"node qcom,duty-pcts failed\n");
			rc =  -ENODEV;
			goto bad_lpg_params;
		} else if (!pwm_cfg->duty_cycles->num_duty_pcts) {
			dev_err(&spmi_dev->dev, "Invalid length of " \
				"duty pcts\n");
			rc =  -EINVAL;
			goto bad_lpg_params;
		}

		pwm_cfg->duty_cycles->duty_pcts =
			devm_kzalloc(&spmi_dev->dev,
			sizeof(int) * PWM_LUT_MAX_SIZE,
			GFP_KERNEL);
		if (!pwm_cfg->duty_cycles->duty_pcts) {
			dev_err(&spmi_dev->dev,
				"Unable to allocate memory\n");
			rc = -ENOMEM;
			goto bad_lpg_params;
		}

		pwm_cfg->old_duty_pcts =
			devm_kzalloc(&spmi_dev->dev,
			sizeof(int) * PWM_LUT_MAX_SIZE,
			GFP_KERNEL);
		if (!pwm_cfg->old_duty_pcts) {
			dev_err(&spmi_dev->dev,
				"Unable to allocate memory\n");
			rc = -ENOMEM;
			goto bad_lpg_params;
		}

		temp_cfg = devm_kzalloc(&spmi_dev->dev,
				pwm_cfg->duty_cycles->num_duty_pcts *
				sizeof(u8), GFP_KERNEL);
		if (!temp_cfg) {
			dev_err(&spmi_dev->dev, "Failed to allocate " \
				"memory for duty pcts\n");
			rc = -ENOMEM;
			goto bad_lpg_params;
		}

		memcpy(temp_cfg, prop->value,
			pwm_cfg->duty_cycles->num_duty_pcts);

		for (i = 0; i < pwm_cfg->duty_cycles->num_duty_pcts; i++)
			pwm_cfg->duty_cycles->duty_pcts[i] =
				(int) temp_cfg[i];

		rc = of_property_read_u32(node, "qcom,start-idx", &val);
		if (!rc) {
			pwm_cfg->lut_params.start_idx = val;
			pwm_cfg->duty_cycles->start_idx = val;
		} else
			goto bad_lpg_params;

		pwm_cfg->lut_params.lut_pause_hi = 0;
		rc = of_property_read_u32(node, "qcom,pause-hi", &val);
		if (!rc)
			pwm_cfg->lut_params.lut_pause_hi = val;
		else if (rc != -EINVAL)
			goto bad_lpg_params;

		pwm_cfg->lut_params.lut_pause_lo = 0;
		rc = of_property_read_u32(node, "qcom,pause-lo", &val);
		if (!rc)
			pwm_cfg->lut_params.lut_pause_lo = val;
		else if (rc != -EINVAL)
			goto bad_lpg_params;

		pwm_cfg->lut_params.ramp_step_ms =
				QPNP_LUT_RAMP_STEP_DEFAULT;
		rc = of_property_read_u32(node, "qcom,ramp-step-ms", &val);
		if (!rc)
			pwm_cfg->lut_params.ramp_step_ms = val;
		else if (rc != -EINVAL)
			goto bad_lpg_params;

		pwm_cfg->lut_params.flags = QPNP_LED_PWM_FLAGS;
		rc = of_property_read_u32(node, "qcom,lut-flags", &val);
		if (!rc)
			pwm_cfg->lut_params.flags = (u8) val;
		else if (rc != -EINVAL)
			goto bad_lpg_params;

		pwm_cfg->lut_params.idx_len =
			pwm_cfg->duty_cycles->num_duty_pcts;
	}
	return 0;

bad_lpg_params:
	pwm_cfg->use_blink = false;
	if (pwm_cfg->mode == PWM_MODE) {
		dev_err(&spmi_dev->dev, "LPG parameters not set for" \
			" blink mode, defaulting to PWM mode\n");
		return 0;
	}
	return rc;
};

static int qpnp_led_get_mode(const char *mode)
{
	if (strncmp(mode, "manual", strlen(mode)) == 0)
		return MANUAL_MODE;
	else if (strncmp(mode, "pwm", strlen(mode)) == 0)
		return PWM_MODE;
	else if (strncmp(mode, "lpg", strlen(mode)) == 0)
		return LPG_MODE;
	else
		return -EINVAL;
};

static int __devinit qpnp_get_config_kpdbl(struct qpnp_led_data *led,
				struct device_node *node)
{
	int rc;
	u32 val;
	u8 led_mode;
	const char *mode;

	led->kpdbl_cfg = devm_kzalloc(&led->spmi_dev->dev,
				sizeof(struct kpdbl_config_data), GFP_KERNEL);
	if (!led->kpdbl_cfg) {
		dev_err(&led->spmi_dev->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

	rc = of_property_read_string(node, "qcom,mode", &mode);
	if (!rc) {
		led_mode = qpnp_led_get_mode(mode);
		if ((led_mode == MANUAL_MODE) || (led_mode == -EINVAL)) {
			dev_err(&led->spmi_dev->dev, "Selected mode not " \
				"supported for kpdbl.\n");
			return -EINVAL;
		}
		led->kpdbl_cfg->pwm_cfg = devm_kzalloc(&led->spmi_dev->dev,
					sizeof(struct pwm_config_data),
					GFP_KERNEL);
		if (!led->kpdbl_cfg->pwm_cfg) {
			dev_err(&led->spmi_dev->dev,
				"Unable to allocate memory\n");
			return -ENOMEM;
		}
		led->kpdbl_cfg->pwm_cfg->mode = led_mode;
		led->kpdbl_cfg->pwm_cfg->default_mode = led_mode;
	} else
		return rc;

	rc = qpnp_get_config_pwm(led->kpdbl_cfg->pwm_cfg, led->spmi_dev,  node);
	if (rc < 0)
		return rc;

	rc = of_property_read_u32(node, "qcom,row-id", &val);
	if (!rc)
		led->kpdbl_cfg->row_id = val;
	else
		return rc;

	led->kpdbl_cfg->row_src_vbst =
			of_property_read_bool(node, "qcom,row-src-vbst");

	led->kpdbl_cfg->row_src_en =
			of_property_read_bool(node, "qcom,row-src-en");

	led->kpdbl_cfg->always_on =
			of_property_read_bool(node, "qcom,always-on");

	return 0;
}

static int __devinit qpnp_get_config_rgb(struct qpnp_led_data *led,
				struct device_node *node)
{
	int rc;
	u8 led_mode;
	const char *mode;

	led->rgb_cfg = devm_kzalloc(&led->spmi_dev->dev,
				sizeof(struct rgb_config_data), GFP_KERNEL);
	if (!led->rgb_cfg) {
		dev_err(&led->spmi_dev->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

//++ p11309 - 2013.12.03 for LED Pattern Customization
#ifdef PAN_LED_CONTROL	
	if (led->id == QPNP_ID_RGB_RED) {
		led->rgb_cfg->enable = RGB_LED_ENABLE_RED;
		pan_led_data.led[RGB_RED] = led;		
	}
	else if (led->id == QPNP_ID_RGB_GREEN) {
		led->rgb_cfg->enable = RGB_LED_ENABLE_GREEN;
		pan_led_data.led[RGB_GREEN] = led;
	}
	else if (led->id == QPNP_ID_RGB_BLUE) {
		led->rgb_cfg->enable = RGB_LED_ENABLE_BLUE;
		pan_led_data.led[RGB_BLUE] = led;
	}
	else
		return -EINVAL;
#else
	if (led->id == QPNP_ID_RGB_RED)
		led->rgb_cfg->enable = RGB_LED_ENABLE_RED;
	else if (led->id == QPNP_ID_RGB_GREEN)
		led->rgb_cfg->enable = RGB_LED_ENABLE_GREEN;
	else if (led->id == QPNP_ID_RGB_BLUE)
		led->rgb_cfg->enable = RGB_LED_ENABLE_BLUE;
	else
		return -EINVAL;
#endif
//-- p11309

	rc = of_property_read_string(node, "qcom,mode", &mode);
	if (!rc) {
		led_mode = qpnp_led_get_mode(mode);
		if ((led_mode == MANUAL_MODE) || (led_mode == -EINVAL)) {
			dev_err(&led->spmi_dev->dev, "Selected mode not " \
				"supported for rgb.\n");
			return -EINVAL;
		}
		led->rgb_cfg->pwm_cfg = devm_kzalloc(&led->spmi_dev->dev,
					sizeof(struct pwm_config_data),
					GFP_KERNEL);
		if (!led->rgb_cfg->pwm_cfg) {
			dev_err(&led->spmi_dev->dev,
				"Unable to allocate memory\n");
			return -ENOMEM;
		}
		led->rgb_cfg->pwm_cfg->mode = led_mode;
		led->rgb_cfg->pwm_cfg->default_mode = led_mode;
	} else
		return rc;

	rc = qpnp_get_config_pwm(led->rgb_cfg->pwm_cfg, led->spmi_dev, node);
	if (rc < 0)
		return rc;

	return 0;
}

static int __devinit qpnp_get_config_mpp(struct qpnp_led_data *led,
		struct device_node *node)
{
	int rc;
	u32 val;
	u8 led_mode;
	const char *mode;

	led->mpp_cfg = devm_kzalloc(&led->spmi_dev->dev,
			sizeof(struct mpp_config_data), GFP_KERNEL);
	if (!led->mpp_cfg) {
		dev_err(&led->spmi_dev->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}

	led->mpp_cfg->current_setting = LED_MPP_CURRENT_MIN;
	rc = of_property_read_u32(node, "qcom,current-setting", &val);
	if (!rc) {
		if (led->mpp_cfg->current_setting < LED_MPP_CURRENT_MIN)
			led->mpp_cfg->current_setting = LED_MPP_CURRENT_MIN;
		else if (led->mpp_cfg->current_setting > LED_MPP_CURRENT_MAX)
			led->mpp_cfg->current_setting = LED_MPP_CURRENT_MAX;
		else
			led->mpp_cfg->current_setting = (u8) val;
	} else if (rc != -EINVAL)
		return rc;

	led->mpp_cfg->source_sel = LED_MPP_SOURCE_SEL_DEFAULT;
	rc = of_property_read_u32(node, "qcom,source-sel", &val);
	if (!rc)
		led->mpp_cfg->source_sel = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->mpp_cfg->mode_ctrl = LED_MPP_MODE_SINK;
	rc = of_property_read_u32(node, "qcom,mode-ctrl", &val);
	if (!rc)
		led->mpp_cfg->mode_ctrl = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->mpp_cfg->vin_ctrl = LED_MPP_VIN_CTRL_DEFAULT;
	rc = of_property_read_u32(node, "qcom,vin-ctrl", &val);
	if (!rc)
		led->mpp_cfg->vin_ctrl = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	led->mpp_cfg->min_brightness = 0;
	rc = of_property_read_u32(node, "qcom,min-brightness", &val);
	if (!rc)
		led->mpp_cfg->min_brightness = (u8) val;
	else if (rc != -EINVAL)
		return rc;

	rc = of_property_read_string(node, "qcom,mode", &mode);
	if (!rc) {
		led_mode = qpnp_led_get_mode(mode);
		led->mpp_cfg->pwm_mode = led_mode;
		if (led_mode == MANUAL_MODE)
			return MANUAL_MODE;
		else if (led_mode == -EINVAL) {
			dev_err(&led->spmi_dev->dev, "Selected mode not " \
				"supported for mpp.\n");
			return -EINVAL;
		}
		led->mpp_cfg->pwm_cfg = devm_kzalloc(&led->spmi_dev->dev,
					sizeof(struct pwm_config_data),
					GFP_KERNEL);
		if (!led->mpp_cfg->pwm_cfg) {
			dev_err(&led->spmi_dev->dev,
				"Unable to allocate memory\n");
			return -ENOMEM;
		}
		led->mpp_cfg->pwm_cfg->mode = led_mode;
		led->mpp_cfg->pwm_cfg->default_mode = led_mode;
	} else
		return rc;

	rc = qpnp_get_config_pwm(led->mpp_cfg->pwm_cfg, led->spmi_dev, node);
	if (rc < 0)
		return rc;

	return 0;
}

static int __devinit qpnp_leds_probe(struct spmi_device *spmi)
{
	struct qpnp_led_data *led, *led_array;
	struct resource *led_resource;
	struct device_node *node, *temp;
	int rc, i, num_leds = 0, parsed_leds = 0;
	const char *led_label;
	bool regulator_probe = false;

//++ p11309 - 2014.01.03 for Offline Charger
#ifdef PAN_LED_CONTROL
	int offline_charger = 0;
	oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;

	smem_id_vendor1_ptr =  (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,sizeof(oem_pm_smem_vendor1_data_type));
	if(smem_id_vendor1_ptr->power_on_mode == 0){
		offline_charger=1;
	}

//++ p11309 2013.12.01 for LED Load Information
	printk("[+++ LED] ============================================\n");
	printk("[+++ LED]    PM8941 LED Driver - Boot Mode: %s\n", offline_charger ? "Offline": "Online");
	printk("[+++ LED] ============================================\n");
//-- p11309
#endif

	node = spmi->dev.of_node;
	if (node == NULL) {
//++ p11309 2013.12.01 for LED Load Information
		printk("[+++ LED] no dt data(No Device Tree Data).\n");
//-- p11309
		return -ENODEV;
	}

	temp = NULL;
	while ((temp = of_get_next_child(node, temp)))
		num_leds++;

	if (!num_leds) {
//++ p11309 2013.12.01 for LED Load Information
		printk("[+++ LED] no LED data in device tree..\n");
//-- p11309
		return -ECHILD;
	}

	led_array = devm_kzalloc(&spmi->dev,
		(sizeof(struct qpnp_led_data) * num_leds), GFP_KERNEL);
	if (!led_array) {
		printk("[+++ LED] Unable to allocate memory\n");
		return -ENOMEM;
	}

	for_each_child_of_node(node, temp) {
		led = &led_array[parsed_leds];
		led->num_leds = num_leds;
		led->spmi_dev = spmi;

		led_resource = spmi_get_resource(spmi, NULL, IORESOURCE_MEM, 0);
		if (!led_resource) {
			printk("[+++ LED] Unable to get LED base address\n");
			rc = -ENXIO;
			goto fail_id_check;
		}
		led->base = led_resource->start;

		rc = of_property_read_string(temp, "label", &led_label);
		if (rc < 0) {
			printk("[+++ LED] Failure reading label, rc = %d\n", rc);
			goto fail_id_check;
		}

		rc = of_property_read_string(temp, "linux,name",
			&led->cdev.name);
		if (rc < 0) {
			printk("[+++ LED] Failure reading led name, rc = %d\n", rc);
			goto fail_id_check;
		}

		rc = of_property_read_u32(temp, "qcom,max-current",
			&led->max_current);
		if (rc < 0) {
			printk("[+++ LED] Failure reading max_current, rc =  %d\n", rc);
			goto fail_id_check;
		}

		rc = of_property_read_u32(temp, "qcom,id", &led->id);
		if (rc < 0) {
			printk("[+++ LED] Failure reading led id, rc =  %d\n", rc);
			goto fail_id_check;
		}

		rc = qpnp_get_common_configs(led, temp);
		if (rc) {
			printk("[+++ LED] Failure reading common led configuration," \
				" rc = %d\n", rc);
			goto fail_id_check;
		}

		led->cdev.brightness_set    = qpnp_led_set;
		led->cdev.brightness_get    = qpnp_led_get;

		if (strncmp(led_label, "wled", sizeof("wled")) == 0) {
			rc = qpnp_get_config_wled(led, temp);
			if (rc < 0) {
				printk("[+++ LED] Unable to read wled config data\n");
				goto fail_id_check;
			}
		} else if (strncmp(led_label, "flash", sizeof("flash"))
				== 0) {
			if (!of_find_property(node, "flash-boost-supply", NULL))
				regulator_probe = true;
			rc = qpnp_get_config_flash(led, temp, &regulator_probe);
			if (rc < 0) {
				printk("[+++ LED] Unable to read flash config data\n");
				goto fail_id_check;
			}
		} else if (strncmp(led_label, "rgb", sizeof("rgb")) == 0) {
			rc = qpnp_get_config_rgb(led, temp);
			if (rc < 0) {
				printk("[+++ LED] Unable to read rgb config data\n");
				goto fail_id_check;
			}
		} else if (strncmp(led_label, "mpp", sizeof("mpp")) == 0) {
			rc = qpnp_get_config_mpp(led, temp);
			if (rc < 0) {
				printk("[+++ LED] Unable to read mpp config data\n");
				goto fail_id_check;
			}
		} else if (strncmp(led_label, "kpdbl", sizeof("kpdbl")) == 0) {
			num_kpbl_leds_on = 0;
			rc = qpnp_get_config_kpdbl(led, temp);
			if (rc < 0) {
				printk("[+++ LED] Unable to read kpdbl config data\n");
				goto fail_id_check;
			}
		} else {
			printk("[+++ LED] No LED matching label\n");
			rc = -EINVAL;
			goto fail_id_check;
		}

		mutex_init(&led->lock);
		INIT_WORK(&led->work, qpnp_led_work);

		rc =  qpnp_led_initialize(led);
		if (rc < 0) {
			printk("[+++ LED] qpnp_led_initialize failed\n");
			goto fail_id_check;
		}

		rc = qpnp_led_set_max_brightness(led);
		if (rc < 0) {
			printk("[+++ LED] qpnp_led_set_max_brightness failed\n");
			goto fail_id_check;
		}

		rc = led_classdev_register(&spmi->dev, &led->cdev);
		if (rc) {
			printk("[+++ LED] unable to register led %d,rc=%d\n",
						 led->id, rc);
			goto fail_id_check;
		}

		if (led->id == QPNP_ID_FLASH1_LED0 ||
			led->id == QPNP_ID_FLASH1_LED1) {
			rc = sysfs_create_group(&led->cdev.dev->kobj,
							&led_attr_group);
			if (rc) {
				printk("[+++ LED] FLASH1_LED0 or FLASH1_LED1 sysfs_create_group failed\n");
				goto fail_id_check;
			}
		}

		if (led->id == QPNP_ID_LED_MPP) {
			if (!led->mpp_cfg->pwm_cfg)
				break;
			if (led->mpp_cfg->pwm_cfg->mode == PWM_MODE) {
				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&pwm_attr_group);
				if (rc) {
					printk("[+++ LED] LED_MPP, pwm_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}
			}
			if (led->mpp_cfg->pwm_cfg->use_blink) {
				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&blink_attr_group);
				if (rc) {
					printk("[+++ LED] LED_MPP, blink_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}

				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&lpg_attr_group);
				if (rc) {
					printk("[+++ LED] LED_MPP, lpg_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}
			} else if (led->mpp_cfg->pwm_cfg->mode == LPG_MODE) {
				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&lpg_attr_group);
				if (rc) {
					printk("[+++ LED] LED_MPP, lpg_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}
			}
		} else if ((led->id == QPNP_ID_RGB_RED) ||
			(led->id == QPNP_ID_RGB_GREEN) ||
			(led->id == QPNP_ID_RGB_BLUE)) {

			if (led->rgb_cfg->pwm_cfg->mode == PWM_MODE) {
				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&pwm_attr_group);
				if (rc) {
					printk("[+++ LED] LED_RGB, pwm_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}
			}
			if (led->rgb_cfg->pwm_cfg->use_blink) {
				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&blink_attr_group);
				if (rc) {
					printk("[+++ LED] LED_RGB, blink_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}

				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&lpg_attr_group);
				if (rc) {
					printk("[+++ LED] LED_RGB, lpg_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}
			} else if (led->rgb_cfg->pwm_cfg->mode == LPG_MODE) {
				rc = sysfs_create_group(&led->cdev.dev->kobj,
					&lpg_attr_group);
				if (rc) {
					printk("[+++ LED] LED_RGB, lpg_attr sysfs_create_group failed\n");
					goto fail_id_check;
				}
			}

//++ p11309 - 2013.12.03 for LED Pattern Customization
#ifdef PAN_LED_CONTROL
			is_rgb_registered++;
#endif
//-- p11309
		}

		/* configure default state */
		if (led->default_on) {
			led->cdev.brightness = led->cdev.max_brightness;
			__qpnp_led_work(led, led->cdev.brightness);
			if (led->turn_off_delay_ms > 0)
				qpnp_led_turn_off(led);
		} else
			led->cdev.brightness = LED_OFF;

		parsed_leds++;
	}
	dev_set_drvdata(&spmi->dev, led_array);

//++ p11309 - 2013.12.03 for LED Pattern Customization
#ifdef PAN_LED_CONTROL
	if ( is_rgb_registered >= 3 ) {

		is_rgb_registered = 0;		
		INIT_WORK(&pan_led_data.wq, pan_led_process_timer_wq_func);
		init_timer(&pan_led_data.timer);
		pan_led_data.timer.function = pan_led_process_timer_func;
		pan_led_data.timer.data = 0;		

		INIT_WORK(&pan_rainbow_off_timer_work, pan_rainbow_off_timer_work_func);
		init_timer(&pan_rainbow_off_timer);
		pan_rainbow_off_timer.function = pan_rainbow_off_timer_func;
		pan_rainbow_off_timer.data = 0;
		
//++ p11309 - 2014.01.03 for Offline Charger
		pan_led_data.offline_boot = offline_charger;
		if (pan_led_data.offline_boot == 1 ) {
			INIT_WORK(&pan_led_data.offline_wq, pan_offline_timer_work_func);
			init_timer(&pan_led_data.offline_timer);
			pan_led_data.offline_timer.function = pan_offline_timer_func;
			pan_led_data.offline_timer.data = 0;			
			
			pan_led_data.led_state[RGB_RED] = PAN_LED_OFF;
			pan_led_data.led_state[RGB_GREEN] = PAN_LED_OFF;
			pan_led_data.led_state[RGB_BLUE] = PAN_LED_OFF;

			pan_led_data.batt_status = PAN_BATT_NOT_DEFINED;
			mod_timer(&pan_led_data.offline_timer, 
				jiffies + msecs_to_jiffies(PAN_OFFLINE_CHECK_INTERVAL));
		}
//-- p11309
		else{
			printk("[+++ LED] Enable Rainbow Effect for Boot up.\n");			

			pan_led_data.led_state[RGB_RED] = PAN_LED_RAINBOW;
			pan_led_data.led_state[RGB_GREEN] = PAN_LED_RAINBOW;
			pan_led_data.led_state[RGB_BLUE] = PAN_LED_RAINBOW;

			pan_rgb_pattern.is_white = 0;
			pan_do_rainbow(1, PAN_LUT_RISE_MODE_LOG, 
				PAN_RAINBOW_SETUP_TIME, PAN_RAINBOW_KEEP_TIME);
			mod_timer(&pan_rainbow_off_timer, 
				jiffies + msecs_to_jiffies(30000));
		}		
	}	
#endif
//-- p11309

	return 0;

fail_id_check:
	for (i = 0; i < parsed_leds; i++) {
		mutex_destroy(&led_array[i].lock);
		led_classdev_unregister(&led_array[i].cdev);
	}

	return rc;
}

static int __devexit qpnp_leds_remove(struct spmi_device *spmi)
{
	struct qpnp_led_data *led_array  = dev_get_drvdata(&spmi->dev);
	int i, parsed_leds = led_array->num_leds;

//++ p11309 - 2014.01.03 for Offline Charger
#ifdef PAN_LED_CONTROL
	if (pan_led_data.offline_boot == 1) {
		del_timer(&pan_led_data.offline_timer);
	}
#endif
//-- p11309
	
	for (i = 0; i < parsed_leds; i++) {
		cancel_work_sync(&led_array[i].work);
		mutex_destroy(&led_array[i].lock);
		led_classdev_unregister(&led_array[i].cdev);
		switch (led_array[i].id) {
		case QPNP_ID_WLED:
			break;
		case QPNP_ID_FLASH1_LED0:
		case QPNP_ID_FLASH1_LED1:
			if (led_array[i].flash_cfg->flash_reg_get)
				regulator_put(led_array[i].flash_cfg-> \
							flash_boost_reg);
			if (led_array[i].flash_cfg->torch_enable)
				regulator_put(led_array[i].flash_cfg->\
							torch_boost_reg);
			sysfs_remove_group(&led_array[i].cdev.dev->kobj,
							&led_attr_group);
			break;
		case QPNP_ID_RGB_RED:
		case QPNP_ID_RGB_GREEN:
		case QPNP_ID_RGB_BLUE:
			if (led_array[i].rgb_cfg->pwm_cfg->mode == PWM_MODE)
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &pwm_attr_group);
			if (led_array[i].rgb_cfg->pwm_cfg->use_blink) {
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &blink_attr_group);
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &lpg_attr_group);
			} else if (led_array[i].rgb_cfg->pwm_cfg->mode\
					== LPG_MODE)
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &lpg_attr_group);
			break;
		case QPNP_ID_LED_MPP:
			if (!led_array[i].mpp_cfg->pwm_cfg)
				break;
			if (led_array[i].mpp_cfg->pwm_cfg->mode == PWM_MODE)
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &pwm_attr_group);
			if (led_array[i].mpp_cfg->pwm_cfg->use_blink) {
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &blink_attr_group);
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &lpg_attr_group);
			} else if (led_array[i].mpp_cfg->pwm_cfg->mode\
					== LPG_MODE)
				sysfs_remove_group(&led_array[i].cdev.dev->\
					kobj, &lpg_attr_group);
			break;
		default:
			printk("[+++ LED] Invalid LED(%d)\n",
					led_array[i].id);
			return -EINVAL;
		}
	}

	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id spmi_match_table[] = {
	{ .compatible = "qcom,leds-qpnp",},
	{ },
};
#else
#define spmi_match_table NULL
#endif

static struct spmi_driver qpnp_leds_driver = {
	.driver		= {
		.name	= "qcom,leds-qpnp",
		.of_match_table = spmi_match_table,
	},
	.probe		= qpnp_leds_probe,
	.remove		= __devexit_p(qpnp_leds_remove),
};

static int __init qpnp_led_init(void)
{
	return spmi_driver_register(&qpnp_leds_driver);
}
module_init(qpnp_led_init);

static void __exit qpnp_led_exit(void)
{
	spmi_driver_unregister(&qpnp_leds_driver);
}
module_exit(qpnp_led_exit);

MODULE_DESCRIPTION("QPNP LEDs driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("leds:leds-qpnp");

