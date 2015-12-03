#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/spmi.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/radix-tree.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/power/pt_max17058_battery.h>
#include <linux/power_supply.h>
#include <linux/bitops.h>
#include <linux/ratelimit.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/machine.h>
#include <linux/of_batterydata.h>
#include <linux/qpnp-revid.h>
#include <linux/android_alarm.h>
#include <linux/spinlock.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <mach/gpiomux.h>
#include <linux/of_gpio.h>
#include <linux/input.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <mach/restart.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <mach/msm_smsm.h>

#define FEATURE_WORKAROUND_NC_CHARGER	1

#define CHIP_BQ2419I	0x03
#define CHIP_BQ24190	0x04
#define CHIP_BQ24192	0x05

/* BQ2419X Regs */
#define REG_IN_SRC_CTL			0x00
#define REG_PW_ON_CONFIG		0x01
#define REG_CHG_CUR_CTL		0x02
#define REG_PT_CUR_CTL			0x03
#define REG_CHG_VOL_CTL		0x04
#define REG_CHG_TIMER_CTL		0x05
#define REG_IR_TERM_CTL		0x06
#define REG_MISC_OP_CTL		0x07
#define REG_SYS_STATUS			0x08
#define REG_FAULT				0x09
#define REG_REVISION_STATUS	0x0A

#define MAX_REG_NUM			11

#define EN_HIZ_MASK				0x80
#define VINDPM_MASK			0x78
#define IINLIM_MASK				0x07
#define REG_RESET_MASK			0x80
#define CHG_CONFIG_MASK		0x30
#define BOOST_LIM_MASK			0x01
#define FAST_CHG_CUR_MASK		0xFC
#define TERM_CUR_LIM_MASK		0x0F
#define CHG_VOL_LIM_MASK		0xFC
#define WTD_TIMER_MASK		0x30
#define SAFETY_TIMER_MASK		0x08
#define AUTO_PW_ON_MASK		0x04
#define CHG_FAULT_INT_MASK	0x02
#define BAT_FAULT_INT_MASK	0x01

/* Interrupt offsets */
#define INT_RT_STS(base)			(base + 0x10)
#define INT_SET_TYPE(base)			(base + 0x11)
#define INT_POLARITY_HIGH(base)			(base + 0x12)
#define INT_POLARITY_LOW(base)			(base + 0x13)
#define INT_LATCHED_CLR(base)			(base + 0x14)
#define INT_EN_SET(base)			(base + 0x15)
#define INT_EN_CLR(base)			(base + 0x16)
#define INT_LATCHED_STS(base)			(base + 0x18)
#define INT_PENDING_STS(base)			(base + 0x19)
#define INT_MID_SEL(base)			(base + 0x1A)
#define INT_PRIORITY(base)			(base + 0x1B)

/* Peripheral register offsets */
#define CHGR_CHG_OPTION				0x08
#define CHGR_ATC_STATUS				0x0A
#define CHGR_VBAT_STATUS			0x0B
#define CHGR_IBAT_BMS				0x0C
#define CHGR_IBAT_STS				0x0D
#define CHGR_VDD_MAX				0x40
#define CHGR_VDD_SAFE				0x41
#define CHGR_VDD_MAX_STEP			0x42
#define CHGR_IBAT_MAX				0x44
#define CHGR_IBAT_SAFE				0x45
#define CHGR_VIN_MIN				0x47
#define CHGR_VIN_MIN_STEP			0x48
#define CHGR_CHG_CTRL				0x49
#define CHGR_CHG_FAILED				0x4A
#define CHGR_ATC_CTRL				0x4B
#define CHGR_ATC_FAILED				0x4C
#define CHGR_VBAT_TRKL				0x50
#define CHGR_VBAT_WEAK				0x52
#define CHGR_IBAT_ATC_A				0x54
#define CHGR_IBAT_ATC_B				0x55
#define CHGR_IBAT_TERM_CHGR			0x5B
#define CHGR_IBAT_TERM_BMS			0x5C
#define CHGR_VBAT_DET				0x5D
#define CHGR_TTRKL_MAX				0x5F
#define CHGR_TTRKL_MAX_EN			0x60
#define CHGR_TCHG_MAX				0x61
#define CHGR_CHG_WDOG_TIME			0x62
#define CHGR_CHG_WDOG_DLY			0x63
#define CHGR_CHG_WDOG_PET			0x64
#define CHGR_CHG_WDOG_EN			0x65
#define CHGR_IR_DROP_COMPEN			0x67
#define CHGR_I_MAX_REG			0x44
#define CHGR_USB_USB_SUSP			0x47
#define CHGR_USB_USB_OTG_CTL			0x48
#define CHGR_USB_ENUM_T_STOP			0x4E
#define CHGR_USB_TRIM				0xF1
#define CHGR_CHG_TEMP_THRESH			0x66
#define CHGR_BAT_IF_PRES_STATUS			0x08
#define CHGR_STATUS				0x09
#define CHGR_BAT_IF_VCP				0x42
#define CHGR_BAT_IF_BATFET_CTRL1		0x90
#define CHGR_BAT_IF_BATFET_CTRL4		0x93
#define CHGR_BAT_IF_SPARE			0xDF
#define CHGR_MISC_BOOT_DONE			0x42
#define CHGR_BUCK_PSTG_CTRL			0x73
#define CHGR_BUCK_COMPARATOR_OVRIDE_1		0xEB
#define CHGR_BUCK_COMPARATOR_OVRIDE_3		0xED
#define CHGR_BUCK_BCK_VBAT_REG_MODE		0x74
#define MISC_REVISION2				0x01
#define USB_OVP_CTL				0x42
#define USB_CHG_GONE_REV_BST			0xED
#define BUCK_VCHG_OV				0x77
#define BUCK_TEST_SMBC_MODES			0xE6
#define BUCK_CTRL_TRIM1				0xF1
#define SEC_ACCESS				0xD0
#define BAT_IF_VREF_BAT_THM_CTRL		0x4A
#define BAT_IF_BPD_CTRL				0x48
#define BOOST_VSET				0x41
#define BOOST_ENABLE_CONTROL			0x46
#define COMP_OVR1				0xEA
#define BAT_IF_BTC_CTRL				0x49
#define USB_OCP_THR				0x52
#define USB_OCP_CLR				0x53
#define BAT_IF_TEMP_STATUS			0x09

#define REG_OFFSET_PERP_SUBTYPE			0x05

/* SMBB peripheral subtype values */
#define SMBB_CHGR_SUBTYPE			0x01
#define SMBB_BUCK_SUBTYPE			0x02
#define SMBB_BAT_IF_SUBTYPE			0x03
#define SMBB_USB_CHGPTH_SUBTYPE			0x04
#define SMBB_DC_CHGPTH_SUBTYPE			0x05
#define SMBB_BOOST_SUBTYPE			0x06
#define SMBB_MISC_SUBTYPE			0x07

/* SMBB peripheral subtype values */
#define SMBBP_CHGR_SUBTYPE			0x31
#define SMBBP_BUCK_SUBTYPE			0x32
#define SMBBP_BAT_IF_SUBTYPE			0x33
#define SMBBP_USB_CHGPTH_SUBTYPE		0x34
#define SMBBP_BOOST_SUBTYPE			0x36
#define SMBBP_MISC_SUBTYPE			0x37

/* SMBCL peripheral subtype values */
#define SMBCL_CHGR_SUBTYPE			0x41
#define SMBCL_BUCK_SUBTYPE			0x42
#define SMBCL_BAT_IF_SUBTYPE			0x43
#define SMBCL_USB_CHGPTH_SUBTYPE		0x44
#define SMBCL_MISC_SUBTYPE			0x47

#define PT_BQ2419X_CHARGER_DEV_NAME	"qcom,qpnp-charger"

/* Status bits and masks */
#define CHGR_BOOT_DONE			BIT(7)
#define CHGR_CHG_EN			BIT(7)
#define CHGR_ON_BAT_FORCE_BIT		BIT(0)
#define USB_VALID_DEB_20MS		0x03
#define BUCK_VBAT_REG_NODE_SEL_BIT	BIT(0)
#define VREF_BATT_THERM_FORCE_ON	0xC0
#define BAT_IF_BPD_CTRL_SEL		0x03
#define VREF_BAT_THM_ENABLED_FSM	0x80
#define REV_BST_DETECTED		BIT(0)
#define BAT_THM_EN			BIT(1)
#define BAT_ID_EN			BIT(0)
#define BOOST_PWR_EN			BIT(7)
#define OCP_CLR_BIT			BIT(7)
#define OCP_THR_MASK			0x03
#define OCP_THR_900_MA			0x02
#define OCP_THR_500_MA			0x01
#define OCP_THR_200_MA			0x00

/* Interrupt definitions */
/* smbb_chg_interrupts */
#define CHG_DONE_IRQ			BIT(7)
#define CHG_FAILED_IRQ			BIT(6)
#define FAST_CHG_ON_IRQ			BIT(5)
#define TRKL_CHG_ON_IRQ			BIT(4)
#define STATE_CHANGE_ON_IR		BIT(3)
#define CHGWDDOG_IRQ			BIT(2)
#define VBAT_DET_HI_IRQ			BIT(1)
#define VBAT_DET_LOW_IRQ		BIT(0)

/* smbb_buck_interrupts */
#define VDD_LOOP_IRQ			BIT(6)
#define IBAT_LOOP_IRQ			BIT(5)
#define ICHG_LOOP_IRQ			BIT(4)
#define VCHG_LOOP_IRQ			BIT(3)
#define OVERTEMP_IRQ			BIT(2)
#define VREF_OV_IRQ			BIT(1)
#define VBAT_OV_IRQ			BIT(0)

/* smbb_bat_if_interrupts */
#define PSI_IRQ				BIT(4)
#define VCP_ON_IRQ			BIT(3)
#define BAT_FET_ON_IRQ			BIT(2)
#define BAT_TEMP_OK_IRQ			BIT(1)
#define BATT_PRES_IRQ			BIT(0)

/* smbb_usb_interrupts */
#define CHG_GONE_IRQ			BIT(2)
#define USBIN_VALID_IRQ			BIT(1)
#define COARSE_DET_USB_IRQ		BIT(0)

/* smbb_dc_interrupts */
#define DCIN_VALID_IRQ			BIT(1)
#define COARSE_DET_DC_IRQ		BIT(0)

/* smbb_boost_interrupts */
#define LIMIT_ERROR_IRQ			BIT(1)
#define BOOST_PWR_OK_IRQ		BIT(0)

/* smbb_misc_interrupts */
#define TFTWDOG_IRQ			BIT(0)

/* SMBB types */
#define SMBB				BIT(1)
#define SMBBP				BIT(2)
#define SMBCL				BIT(3)

/* Workaround flags */
#define CHG_FLAGS_VCP_WA		BIT(0)
#define BOOST_FLASH_WA			BIT(1)
#define POWER_STAGE_WA			BIT(2)

#ifndef MIN
#define  MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#define PM8941_CHARER_IOCTL_MAGIC 'p'
#define PM8941_CHARER_TEST_SET_CHARING_CTL        _IOW(PM8941_CHARER_IOCTL_MAGIC, 1, unsigned)
#define PM8941_CHARER_TEST_GET_CHARING_CTL        _IOW(PM8941_CHARER_IOCTL_MAGIC, 2, unsigned)
#define PM8941_CHARGER_TEST_SET_PM_CHG_TEST	_IOW(PM8941_CHARER_IOCTL_MAGIC, 3, unsigned)
#define PM8941_CHARGER_TEST_CHARGING_SETTING	_IOW(PM8941_CHARER_IOCTL_MAGIC, 4, unsigned)
#define PM8941_CHARGER_TEST_GET_AUTO_PW_ON	_IOW(PM8941_CHARER_IOCTL_MAGIC, 5, unsigned)
#define PM8941_CHARGER_TEST_SET_IINLIM		_IOW(PM8941_CHARER_IOCTL_MAGIC, 6, unsigned)
//#define CHG_ENTRY_NAME		"bq2419x"

#define DEFAULT_BATT_SOC	50
#define DEFAULT_BATT_VOL	(4000 * 1000);	// 4.0V
#define DEFAULT_BATT_TEMP	250

#define DEFAULT_UPDATE_TIME	30000
#define USB_WAITING_TIME_STEP1	3000
#define USB_WAITING_TIME_STEP2	30000

#define LOW_BATT_WARN_THR	5	// 5%

#define VREG_MAX				4400		// 4.400V
#define VREG_DEFAULT			4352		// 4.352V
#define ITERM_MAX				2048*1000	// 2048mA

#define IINLIM_MIN				500*1000	// 500mA
#define IINLIM_MAX				2000*1000 	// 2A
#define IINLIM_OCTA_ON			1200*1000	// 1200mA
#define IINLIM_OCTA_OFF			IINLIM_MAX	// 2A
#define IINLIM_DUMMY_BATT		900*1000	// 900mA
#define ITERM_DEFAULT			128*1000	// 128mA

#define ICHG_MAX				2304	// 2.3A
#define ICHG_WARM				1200	// 1.2A
#define ICHG_WARM_CRITICAL		1536	// 1.5A
#define ICHG_COOL_CRITICAL		ICHG_WARM_CRITICAL
#define ICHG_HOT_FATAL			0
#define ICHG_COLD_FATAL		ICHG_HOT_FATAL

#define FACTORY_DUMMY_ID_MIN  930
#define FACTORY_DUMMY_ID_MAX 	1230

/* BQ2419X GPIOs */
#define GPIO_SC_CE_N	4
#define GPIO_SC_OTG		5
#define GPIO_SC_INT		74
#define GPIO_SC_SDA		83
#define GPIO_SC_SCL		84
#define GPIO_SC_PSEL	85
#define GPIO_SC_STAT	86

enum iinlim {
	IINLIM_100MA = 0,
	IINLIM_150MA = 1,
	IINLIM_500MA = 2,
	IINLIM_900MA = 3,
	IINLIM_1200MA = 4,
	IINLIM_1500MA = 5,
	IINLIM_2000MA = 6,
	IINLIM_3000MA = 7,
};

static int bq2419x_iinlim_val[] = {100, 150, 500, 900, 1200, 1500, 2000, 3000};

enum battery_thermal_trip_type {
	BATT_THERM_FATAL_COLD = 0,
	BATT_THERM_CRITICAL_COLD,
	BATT_THERM_NORMAL,
	BATT_THERM_WARM,
	BATT_THERM_CRITICAL_HOT,
	BATT_THERM_FATAL_HOT,
	BATT_THERM_UNKNOWN,
};

struct therm_trip_info {
	int batt_temp;
	int therm_trip;
	unsigned int ichg;
};

static struct therm_trip_info batt_therm_table[] = {
		/* {temperature , battery thermal trip, fast chg current} */
		{-70, BATT_THERM_FATAL_COLD, ICHG_COLD_FATAL},
		{30, BATT_THERM_CRITICAL_COLD, ICHG_COOL_CRITICAL},
		{381, BATT_THERM_NORMAL, ICHG_MAX},
		{431, BATT_THERM_WARM, ICHG_MAX},
		{601, BATT_THERM_CRITICAL_HOT, ICHG_WARM_CRITICAL},
		{681, BATT_THERM_FATAL_HOT, ICHG_HOT_FATAL},
};

enum chg_mode_type {
	OFFLINE_CHARGING = 0,
	ONLINE_CHARGING,
};

static struct gpiomux_setting bq2419x_gpio_in_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting bq2419x_ce_n_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting bq2419x_psel_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting bq2419x_otg_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct msm_gpiomux_config bq2419x_gpio_configs[] = {
	{
		.gpio = GPIO_SC_CE_N,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bq2419x_ce_n_cfg,
		},
	},
	{
		.gpio = GPIO_SC_OTG,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bq2419x_otg_cfg,
		},
	},
	{
		.gpio = GPIO_SC_INT,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bq2419x_gpio_in_cfg,
		},
	},
	{
		.gpio = GPIO_SC_PSEL,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bq2419x_psel_cfg,
		},
	},
	{
		.gpio = GPIO_SC_STAT,
		.settings = {
			[GPIOMUX_ACTIVE]    = &bq2419x_gpio_in_cfg,
		},
	},
	
};

static struct of_device_id pt_bq2419x_charger_match_table[] = {
	{ .compatible = PT_BQ2419X_CHARGER_DEV_NAME, },
	{}
};

struct qpnp_chg_irq {
	int		irq;
	unsigned long		disabled;
};

struct qpnp_chg_regulator {
	struct regulator_desc			rdesc;
	struct regulator_dev			*rdev;
};

struct pt_bq2419x_chg_chip {
	struct device			*dev;
	struct spmi_device		*spmi;
	struct i2c_client 		*bq2419x_client;
	unsigned int 		otg_gpio;
	unsigned int		stat_gpio;
	unsigned int		int_gpio;
	unsigned int		ce_n_gpio;
	unsigned int		psel_gpio;
	/* This vin_gpio is used to indicate when VBUS has collapsed */
	unsigned int		vin_gpio;
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	unsigned int		pg_gpio;
	unsigned int		pw_good_irq;
#endif
	unsigned int		revision;
	unsigned int		bq2419x_irq;
	unsigned int		current_max;
	unsigned int		update_time;
	unsigned int		rcomp;
	unsigned int		batt_soc;
	unsigned int		batt_uvolt;
	unsigned int		type;
	unsigned int		online;
	int				fb_stat;
	int				batt_temp;
	int				host_mode;
	int				user_iinlim;
	u8				bq2419x_regs[MAX_REG_NUM];
	u16				chgr_base;
	u16				buck_base;
	u16				bat_if_base;
	u16				usb_chgpth_base;
	u16				dc_chgpth_base;
	u16				boost_base;
	u16				misc_base;
	u16				freq_base;
	uint32_t			flags;
	bool				charging_disabled;
	bool				cable_present;
	bool				aux_present;
	bool				vbus_active;
	bool				chg_f_dis;				// force disable charging
	bool				auto_pw_on;
	bool				charging_setting;
	bool				dummy_batt_present;	// factory dummy battery
	atomic_t 			bms_input_flag;
	unsigned int	 	therm_type;
	oem_pm_smem_vendor1_data_type *smem_data;
	struct notifier_block fb_notif;
	struct wake_lock		heartbeat_wake_lock;
	struct wake_lock		chg_wake_lock;
	enum power_supply_type		chg_type;
	struct qpnp_chg_regulator	boost_vreg;
	struct qpnp_chg_irq		batt_pres;
	struct qpnp_chg_irq		batt_temp_ok;
	struct power_supply		*usb_psy;
	struct power_supply		*bms_psy;
	struct power_supply		batt_psy;
	struct qpnp_adc_tm_btm_param	adc_param;
	struct delayed_work		update_heartbeat;
#if defined(FEATURE_WORKAROUND_NC_CHARGER)
	struct delayed_work		nc_chg_check_work;
#endif
	struct delayed_work 		vbus_check_work;
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	struct work_struct 		pg_irq_work;
#endif
	struct work_struct 		irq_work;
	struct qpnp_vadc_chip		*vadc_dev;
	struct input_dev *bms_input_dev;
	struct proc_dir_entry *bq2419x_charger_dir;
};

static struct i2c_client 		*chip_client;
static struct pt_bq2419x_chg_chip *testmenu;
static struct platform_device *bms_input_attr_dev;
static struct mutex			masked_write_lock;

#if defined(FEATURE_WORKAROUND_NC_CHARGER)
extern int composite_get_udc_state(char *udc_state);
#endif

#ifdef CONFIG_PANTECH_USB_BLOCKING_MDMSTATE
extern int get_pantech_mdm_state(void);
#endif

static int
qpnp_chg_read(struct pt_bq2419x_chg_chip *chip, u8 *val,
			u16 base, int count)
{
	int rc = 0;
	struct spmi_device *spmi = chip->spmi;

	if (base == 0) {
		pr_err("base cannot be zero base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return -EINVAL;
	}

	rc = spmi_ext_register_readl(spmi->ctrl, spmi->sid, base, val, count);
	if (rc) {
		pr_err("SPMI read failed base=0x%02x sid=0x%02x rc=%d\n", base,
				spmi->sid, rc);
		return rc;
	}
	return 0;
}

static int
qpnp_chg_write(struct pt_bq2419x_chg_chip *chip, u8 *val,
			u16 base, int count)
{
	int rc = 0;
	struct spmi_device *spmi = chip->spmi;

	if (base == 0) {
		pr_err("base cannot be zero base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return -EINVAL;
	}

	rc = spmi_ext_register_writel(spmi->ctrl, spmi->sid, base, val, count);
	if (rc) {
		pr_err("write failed base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return rc;
	}

	return 0;
}

static int
qpnp_chg_masked_write(struct pt_bq2419x_chg_chip *chip, u16 base,
						u8 mask, u8 val, int count)
{
	int rc;
	u8 reg;

	rc = qpnp_chg_read(chip, &reg, base, count);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n", base, rc);
		return rc;
	}
	pr_debug("addr = 0x%x read 0x%x\n", base, reg);

	reg &= ~mask;
	reg |= val & mask;

	pr_debug("Writing 0x%x\n", reg);

	rc = qpnp_chg_write(chip, &reg, base, count);
	if (rc) {
		pr_err("spmi write failed: addr=%03X, rc=%d\n", base, rc);
		return rc;
	}

	return 0;
}

static int
qpnp_chg_charge_en(struct pt_bq2419x_chg_chip *chip, int enable)
{
	return qpnp_chg_masked_write(chip, chip->chgr_base + CHGR_CHG_CTRL,
			CHGR_CHG_EN,
			enable ? CHGR_CHG_EN : 0, 1);
}

static int
qpnp_chg_is_batt_present(struct pt_bq2419x_chg_chip *chip)
{
	u8 batt_pres_rt_sts;
	int rc;

	rc = qpnp_chg_read(chip, &batt_pres_rt_sts,
				 INT_RT_STS(chip->bat_if_base), 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->bat_if_base), rc);
		return rc;
	}

	return (batt_pres_rt_sts & BATT_PRES_IRQ) ? 1 : 0;
}

static int
qpnp_chg_force_run_on_batt(struct pt_bq2419x_chg_chip *chip, int disable)
{
	/* Don't force on battery if battery is not present */
	if (!qpnp_chg_is_batt_present(chip))
		return 0;

	/* This bit forces the charger to run off of the battery rather
	 * than a connected charger */
	return qpnp_chg_masked_write(chip, chip->chgr_base + CHGR_CHG_CTRL,
			CHGR_ON_BAT_FORCE_BIT,
			disable ? CHGR_ON_BAT_FORCE_BIT : 0, 1);
}

static int
qpnp_chg_setup_flags(struct pt_bq2419x_chg_chip *chip)
{
	if (chip->revision > 0 && chip->type == SMBB)
		chip->flags |= CHG_FLAGS_VCP_WA;
	if (chip->type == SMBB)
		chip->flags |= BOOST_FLASH_WA;
	if (chip->type == SMBBP) {
		struct device_node *revid_dev_node;
		struct pmic_revid_data *revid_data;

		chip->flags |=  BOOST_FLASH_WA;

		revid_dev_node = of_parse_phandle(chip->spmi->dev.of_node,
						"qcom,pmic-revid", 0);
		if (!revid_dev_node) {
			pr_err("Missing qcom,pmic-revid property\n");
			return -EINVAL;
		}
		revid_data = get_revid_data(revid_dev_node);
		if (IS_ERR(revid_data)) {
			pr_err("Couldnt get revid data rc = %ld\n",
						PTR_ERR(revid_data));
			return PTR_ERR(revid_data);
		}

		if (revid_data->rev4 < PM8226_V2P1_REV4
			|| ((revid_data->rev4 == PM8226_V2P1_REV4)
				&& (revid_data->rev3 <= PM8226_V2P1_REV3))) {
			chip->flags |= POWER_STAGE_WA;
		}
	}
	return 0;
}

#define BOOST_MIN_UV	4200000
#define BOOST_MAX_UV	5500000
#define BOOST_STEP_UV	50000
#define BOOST_MIN	16
#define N_BOOST_V	((BOOST_MAX_UV - BOOST_MIN_UV) / BOOST_STEP_UV + 1)
static int
qpnp_boost_vset(struct pt_bq2419x_chg_chip *chip, int voltage)
{
	u8 reg = 0;

	if (voltage < BOOST_MIN_UV || voltage > BOOST_MAX_UV) {
		pr_err("invalid voltage requested %d uV\n", voltage);
		return -EINVAL;
	}

	reg = DIV_ROUND_UP(voltage - BOOST_MIN_UV, BOOST_STEP_UV) + BOOST_MIN;

	pr_debug("voltage=%d setting %02x\n", voltage, reg);
	return qpnp_chg_write(chip, &reg, chip->boost_base + BOOST_VSET, 1);
}

static int
qpnp_boost_vget_uv(struct pt_bq2419x_chg_chip *chip)
{
	int rc;
	u8 boost_reg;

	rc = qpnp_chg_read(chip, &boost_reg,
		 chip->boost_base + BOOST_VSET, 1);
	if (rc) {
		pr_err("failed to read BOOST_VSET rc=%d\n", rc);
		return rc;
	}

	if (boost_reg < BOOST_MIN) {
		pr_err("Invalid reading from 0x%x\n", boost_reg);
		return -EINVAL;
	}

	return BOOST_MIN_UV + ((boost_reg - BOOST_MIN) * BOOST_STEP_UV);
}

static int
qpnp_chg_is_boost_en_set(struct pt_bq2419x_chg_chip *chip)
{
	u8 boost_en_ctl;
	int rc;

	rc = qpnp_chg_read(chip, &boost_en_ctl,
		chip->boost_base + BOOST_ENABLE_CONTROL, 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				chip->boost_base + BOOST_ENABLE_CONTROL, rc);
		return rc;
	}

	pr_debug("boost en 0x%x\n", boost_en_ctl);

	return (boost_en_ctl & BOOST_PWR_EN) ? 1 : 0;
}

static int
qpnp_chg_regulator_boost_enable(struct regulator_dev *rdev)
{
	struct pt_bq2419x_chg_chip *chip = rdev_get_drvdata(rdev);

	return qpnp_chg_masked_write(chip,
		chip->boost_base + BOOST_ENABLE_CONTROL,
		BOOST_PWR_EN,
		BOOST_PWR_EN, 1);
}

/* Boost regulator operations */
#define ABOVE_VBAT_WEAK		BIT(1)
static int
qpnp_chg_regulator_boost_disable(struct regulator_dev *rdev)
{
	struct pt_bq2419x_chg_chip *chip = rdev_get_drvdata(rdev);
	int rc;
	u8 vbat_sts;

	rc = qpnp_chg_masked_write(chip,
		chip->boost_base + BOOST_ENABLE_CONTROL,
		BOOST_PWR_EN,
		0, 1);
	if (rc) {
		pr_err("failed to disable boost rc=%d\n", rc);
		return rc;
	}

	rc = qpnp_chg_read(chip, &vbat_sts,
			chip->chgr_base + CHGR_VBAT_STATUS, 1);
	if (rc) {
		pr_err("failed to read bat sts rc=%d\n", rc);
		return rc;
	}

	if (!(vbat_sts & ABOVE_VBAT_WEAK) && (chip->flags & BOOST_FLASH_WA)) {
		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);
		if (rc) {
			pr_err("failed to write SEC_ACCESS rc=%d\n", rc);
			return rc;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + COMP_OVR1,
			0xFF,
			0x20, 1);
		if (rc) {
			pr_err("failed to write COMP_OVR1 rc=%d\n", rc);
			return rc;
		}

		usleep(2000);

		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + SEC_ACCESS,
			0xFF,
			0xA5, 1);
		if (rc) {
			pr_err("failed to write SEC_ACCESS rc=%d\n", rc);
			return rc;
		}

		rc = qpnp_chg_masked_write(chip,
			chip->chgr_base + COMP_OVR1,
			0xFF,
			0x00, 1);
		if (rc) {
			pr_err("failed to write COMP_OVR1 rc=%d\n", rc);
			return rc;
		}
	}

	return rc;
}

static int
qpnp_chg_regulator_boost_is_enabled(struct regulator_dev *rdev)
{
	struct pt_bq2419x_chg_chip *chip = rdev_get_drvdata(rdev);

	return qpnp_chg_is_boost_en_set(chip);
}

static int
qpnp_chg_regulator_boost_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV, unsigned *selector)
{
	int uV = min_uV;
	int rc;
	struct pt_bq2419x_chg_chip *chip = rdev_get_drvdata(rdev);

	if (uV < BOOST_MIN_UV && max_uV >= BOOST_MIN_UV)
		uV = BOOST_MIN_UV;


	if (uV < BOOST_MIN_UV || uV > BOOST_MAX_UV) {
		pr_err("request %d uV is out of bounds\n", uV);
		return -EINVAL;
	}

	*selector = DIV_ROUND_UP(uV - BOOST_MIN_UV, BOOST_STEP_UV);
	if ((*selector * BOOST_STEP_UV + BOOST_MIN_UV) > max_uV) {
		pr_err("no available setpoint [%d, %d] uV\n", min_uV, max_uV);
		return -EINVAL;
	}

	rc = qpnp_boost_vset(chip, uV);

	return rc;
}

static int
qpnp_chg_regulator_boost_get_voltage(struct regulator_dev *rdev)
{
	struct pt_bq2419x_chg_chip *chip = rdev_get_drvdata(rdev);

	return qpnp_boost_vget_uv(chip);
}

static int
qpnp_chg_regulator_boost_list_voltage(struct regulator_dev *rdev,
			unsigned selector)
{
	if (selector >= N_BOOST_V)
		return 0;

	return BOOST_MIN_UV + (selector * BOOST_STEP_UV);
}

static struct regulator_ops qpnp_chg_boost_reg_ops = {
	.enable			= qpnp_chg_regulator_boost_enable,
	.disable		= qpnp_chg_regulator_boost_disable,
	.is_enabled		= qpnp_chg_regulator_boost_is_enabled,
	.set_voltage		= qpnp_chg_regulator_boost_set_voltage,
	.get_voltage		= qpnp_chg_regulator_boost_get_voltage,
	.list_voltage		= qpnp_chg_regulator_boost_list_voltage,
};

static int
qpnp_chg_hwinit(struct pt_bq2419x_chg_chip *chip, u8 subtype,
				struct spmi_resource *spmi_resource)
{
	int rc = 0;
	struct regulator_init_data *init_data;
	struct regulator_desc *rdesc;
	
	switch (subtype) {
		case SMBB_CHGR_SUBTYPE:
		case SMBBP_CHGR_SUBTYPE:
		case SMBCL_CHGR_SUBTYPE:
			break;
		case SMBB_BUCK_SUBTYPE:
		case SMBBP_BUCK_SUBTYPE:
		case SMBCL_BUCK_SUBTYPE:
			break;
		case SMBB_BAT_IF_SUBTYPE:
		case SMBBP_BAT_IF_SUBTYPE:
		case SMBCL_BAT_IF_SUBTYPE:
			/* Select battery presence detection */
			rc = qpnp_chg_masked_write(chip,
				chip->bat_if_base + BAT_IF_BPD_CTRL,
				BAT_IF_BPD_CTRL_SEL, BAT_ID_EN, 1);
			if (rc) {
				pr_debug("failed to chose BPD rc=%d\n", rc);
				return rc;
			}
			/* Force on VREF_BAT_THM */
			rc = qpnp_chg_masked_write(chip,
				chip->bat_if_base + BAT_IF_VREF_BAT_THM_CTRL,
				VREF_BATT_THERM_FORCE_ON,
				VREF_BATT_THERM_FORCE_ON, 1);
			if (rc) {
				pr_debug("failed to force on VREF_BAT_THM rc=%d\n", rc);
				return rc;
			}
			break;
		case SMBB_USB_CHGPTH_SUBTYPE:
		case SMBBP_USB_CHGPTH_SUBTYPE:
		case SMBCL_USB_CHGPTH_SUBTYPE:
			break;
		case SMBB_DC_CHGPTH_SUBTYPE:
			break;
		case SMBB_BOOST_SUBTYPE:
		case SMBBP_BOOST_SUBTYPE:	
			init_data = of_get_regulator_init_data(chip->dev,
					       spmi_resource->of_node);
			if (!init_data) {
				pr_err("unable to allocate memory\n");
				return -ENOMEM;
			}

			if (init_data->constraints.name) {
				if (of_get_property(chip->dev->of_node,
							"boost-parent-supply", NULL))
					init_data->supply_regulator = "boost-parent";

				rdesc			= &(chip->boost_vreg.rdesc);
				rdesc->owner		= THIS_MODULE;
				rdesc->type		= REGULATOR_VOLTAGE;
				rdesc->ops		= &qpnp_chg_boost_reg_ops;
				rdesc->name		= init_data->constraints.name;

				init_data->constraints.valid_ops_mask
					|= REGULATOR_CHANGE_STATUS
						| REGULATOR_CHANGE_VOLTAGE;

				chip->boost_vreg.rdev = regulator_register(rdesc,
						chip->dev, init_data, chip,
						spmi_resource->of_node);
				if (IS_ERR(chip->boost_vreg.rdev)) {
					rc = PTR_ERR(chip->boost_vreg.rdev);
					chip->boost_vreg.rdev = NULL;
					if (rc != -EPROBE_DEFER)
						pr_err("boost reg failed, rc=%d\n", rc);
					return rc;
				}
			}
			break;
		case SMBB_MISC_SUBTYPE:
		case SMBBP_MISC_SUBTYPE:
		case SMBCL_MISC_SUBTYPE:
			pr_debug("Setting BOOT_DONE\n");
			rc = qpnp_chg_masked_write(chip,
				chip->misc_base + CHGR_MISC_BOOT_DONE,
				CHGR_BOOT_DONE, CHGR_BOOT_DONE, 1);
			break;
		default:
			pr_err("Invalid peripheral subtype\n");
			break;
	}

	return rc;
}

#define BATT_PRES_BIT BIT(7)
static int
get_prop_batt_present(struct pt_bq2419x_chg_chip *chip)
{
	u8 batt_present;
	int rc;

	rc = qpnp_chg_read(chip, &batt_present,
				chip->bat_if_base + CHGR_BAT_IF_PRES_STATUS, 1);
	if (rc) {
		pr_err("Couldn't read battery status read failed rc=%d\n", rc);
		return 0;
	};
	return (batt_present & BATT_PRES_BIT) ? 1 : 0;
}

#define BATT_ID_PRES_BIT BIT(0)
static int
qpnp_chg_is_batt_id_present(struct pt_bq2419x_chg_chip *chip)
{
	u8 batt_id_pres;
	int rc;
	
	rc = qpnp_chg_read(chip, &batt_id_pres,
				chip->bat_if_base + CHGR_BAT_IF_PRES_STATUS, 1);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n",
				INT_RT_STS(chip->bat_if_base), rc);
		return rc;
	}
	return (batt_id_pres & BATT_ID_PRES_BIT) ? 1 : 0;
}

static irqreturn_t
bat_if_batt_pres_irq_handler(int irq, void *_chip)
{
	struct pt_bq2419x_chg_chip *chip = _chip;
	int batt_present;
	int batt_id_present;

	batt_present = qpnp_chg_is_batt_present(chip);
	batt_id_present = qpnp_chg_is_batt_id_present(chip);

	if(!batt_present && !batt_id_present) {
		// if battery removed, bq24192 will generate 257us INT pulse continuously.
		disable_irq_nosync(chip->bq2419x_irq);
		pr_err("Good bye ~~~~\n");
		msm_restart(0, "oem-34");
	}
	
	return IRQ_HANDLED;
}

static int
qpnp_chg_request_irqs(struct pt_bq2419x_chg_chip *chip)
{
	int rc = 0;
	struct resource *resource;
	struct spmi_resource *spmi_resource;
	u8 subtype;
	struct spmi_device *spmi = chip->spmi;

	spmi_for_each_container_dev(spmi_resource, chip->spmi) {
		if (!spmi_resource) {
				pr_err("qpnp_chg: spmi resource absent\n");
			return rc;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			return rc;
		}

		rc = qpnp_chg_read(chip, &subtype,
				resource->start + REG_OFFSET_PERP_SUBTYPE, 1);
		if (rc) {
			pr_err("Peripheral subtype read failed rc=%d\n", rc);
			return rc;
		}

		switch (subtype) {
		case SMBB_CHGR_SUBTYPE:
		case SMBBP_CHGR_SUBTYPE:
		case SMBCL_CHGR_SUBTYPE:
			break;
		case SMBB_BAT_IF_SUBTYPE:
		case SMBBP_BAT_IF_SUBTYPE:
		case SMBCL_BAT_IF_SUBTYPE:
			chip->batt_pres.irq = spmi_get_irq_byname(spmi,
						spmi_resource, "batt-pres");
			if (chip->batt_pres.irq < 0) {
				pr_err("Unable to get batt-pres irq\n");
				return rc;
			}
			rc = devm_request_irq(chip->dev, chip->batt_pres.irq,
				bat_if_batt_pres_irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING
#if 1 // sayuss : we use below
				,"batt-pres", chip);
#else
				| IRQF_SHARED | IRQF_ONESHOT,
				"batt-pres", chip);
#endif
			if (rc < 0) {
				pr_err("Can't request %d batt-pres irq: %d\n",
						chip->batt_pres.irq, rc);
				return rc;
			}

			enable_irq_wake(chip->batt_pres.irq);

			break;
		case SMBB_BUCK_SUBTYPE:
		case SMBBP_BUCK_SUBTYPE:
		case SMBCL_BUCK_SUBTYPE:
			break;
		case SMBB_USB_CHGPTH_SUBTYPE:
		case SMBBP_USB_CHGPTH_SUBTYPE:
		case SMBCL_USB_CHGPTH_SUBTYPE:
			break;
		case SMBB_DC_CHGPTH_SUBTYPE:
			break;
		}
	}

	return rc;
}

static int bq2419x_i2c_rxdata(struct i2c_client *client, uint8_t addr,
	uint8_t *data, int length)
{
	int rc=0;
	struct i2c_msg msgs[] = {
		{
			.addr  = client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = &addr,
		},
		{
			.addr  = client->addr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = data,
		},
	};

	rc = i2c_transfer(client->adapter, msgs, 2);	
	return rc;
}

static int bq2419x_i2c_txdata(struct i2c_client *client, uint8_t addr,
	uint8_t data, int length)
{
	int rc=0;
	u8 buf[2];
	
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = length+1,
			.buf = buf,
		 },
	};

	buf[0] = addr;
	buf[1] = data;

	rc = i2c_transfer(client->adapter, msg, 1);
	
	return rc;
}

static int bq2419x_read_reg(struct i2c_client *client, unsigned short raddr, u8 *rdata)
{
	int32_t rc = 0;
	int retry=20;
	
	if(client == NULL)
		return -EIO;

	do {
		rc = bq2419x_i2c_rxdata(client, raddr, rdata, 1);
		if(rc == -EIO)	
			msleep(10);
	}while(rc < 0 && retry-- > 0);
	
	if (rc < 0) 
		pr_err("bq2419x_i2c_read failed!rc=%d\n", rc);
		
	return rc;
}

static int32_t bq2419x_write_reg(struct i2c_client *client, unsigned short waddr, u8 wdata)
{
	int32_t rc = -EFAULT;
	int retry=20;
	
//	if(client == NULL)
//		return -EIO;

	do {
		rc = bq2419x_i2c_txdata(client, waddr, wdata, 1);
		if(rc == -EIO)	
			msleep(10);
	}while(rc < 0 && retry-- > 0);
	
	if (rc < 0) {
		pr_err("bq2419x_i2c_write failed, addr = 0x%x, val = 0x%x!, rc=%d\n",
			waddr, wdata, rc);
	}
	
	return rc;
}

static int bq2419x_masked_write(struct i2c_client *client, int reg,
		u8 mask, u8 val)
{
	s32 rc = 0;
	u8 temp = 0;

	if(client == NULL)
		return -EIO;

	mutex_lock(&masked_write_lock);
	
	rc = bq2419x_read_reg(client, reg, &temp);
	if (rc < 0) {
		pr_err("bq2419x_read_reg failed: reg=%d, rc=%d\n", reg, rc);
		goto i2c_mutex_unlock;
	}
	
	temp &= ~mask;
	temp |= val & mask;
//	printk("masked write:0x%x\n", temp);
	rc = bq2419x_write_reg(client, reg, temp);
	if (rc < 0) 
		pr_err("bq2419x_write failed: reg=%d, rc=%d\n", reg, rc);

i2c_mutex_unlock:
	mutex_unlock(&masked_write_lock);
	return rc;
}

#if 0
static void print_regs(struct pt_bq2419x_chg_chip *chip)
{
	int i;

	pr_err("====> BQ24192 Regs Info <====\n");
	for(i=0; i<=REG_REVISION_STATUS; i++) {
		bq2419x_read_reg(chip->bq2419x_client, i, 
		&chip->bq2419x_regs[i]);
		pr_err("REG%02d => 0x%x\n", i, chip->bq2419x_regs[i]);
	}
}

static void get_chip_revision(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;
	u8 *pn;
	u8 rData;
	
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_REVISION_STATUS, 
		&rData);
	if(rc < 0)
		pr_err("bq2419x read failed: reg=0x%x, rc=%d\n", REG_REVISION_STATUS, rc);

	chip->revision = ((rData&(0x7<<3))>>3);
	
	switch(chip->revision)
	{
		case CHIP_BQ2419I:
			pn = "bq2419I";
			break;
		case CHIP_BQ24190:
			pn = "bq24190";
			break;
		case CHIP_BQ24192:
			pn = "bq24192/bq24193";
			break;
		default:
			pn = "unknown";
	}
	
	printk("=== bq2419x Part name : %s ===\n", pn);
}

static void bq2419x_disable_wtd_and_safe_timer(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;
	
	/* WATCHDOG : disable, CHG_TIMER : disable */
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_CHG_TIMER_CTL, 
		WTD_TIMER_MASK|SAFETY_TIMER_MASK, 0);
	if(rc < 0) 
		pr_err("bq241x_disable_wtd_and_safe_timer failed: rc=%d\n", rc);
}
#endif

#define CHRG_FAULT_MASK	BIT(1)
#define BAT_FAULT_MASK		BIT(0)
static void bq2419x_fault_irq_set(struct pt_bq2419x_chg_chip *chip, bool all_en)
{
	int rc=0;
	u8 int_mask=0;

	if(all_en)
		int_mask = CHRG_FAULT_MASK|BAT_FAULT_MASK;
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_MISC_OP_CTL, 
		CHG_FAULT_INT_MASK|BAT_FAULT_INT_MASK, int_mask);
	if(rc < 0) 
		pr_err("bq2419x_fault_irq_set failed: rc=%d\n", rc);
}

static u8
bq2419x_get_sys_status(struct pt_bq2419x_chg_chip *chip)
{
	u8 rData=0;
	
	bq2419x_read_reg(chip->bq2419x_client, REG_SYS_STATUS, 
		&rData);

	return rData;
}

static int is_chg_done(u8 sys_stat)
{
	return ((sys_stat & (BIT(4)|BIT(5))) == (BIT(4)|BIT(5))) ? 1 : 0;
}

static void bq2419x_en_hiz_set(struct pt_bq2419x_chg_chip *chip, unsigned int en)
{
	int rc=0;
	u8 en_hiz=0;
	
	if(en)
		en_hiz = BIT(7);
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_IN_SRC_CTL,
		EN_HIZ_MASK, en_hiz);
	if(rc < 0)
		pr_err("%s failed: rc=%d\n", __func__, rc);
}

static void
bq2419x_force_start_recharging(struct pt_bq2419x_chg_chip *chip)
{
	bq2419x_en_hiz_set(chip, 1);
	msleep(2);
	bq2419x_en_hiz_set(chip, 0);
}

static int bq2419x_chg_en(struct pt_bq2419x_chg_chip *chip, bool en)
{
	int rc=0;
	u8 wdata=0;
	u8 sys_stat;
	
	if(en && !chip->chg_f_dis) {
		wdata = BIT(4);
		sys_stat = bq2419x_get_sys_status(chip);
		if(is_chg_done(sys_stat)) {
			/* if VBAT > VRECHG,  then use HIZ toggle with 1ms delay in between set and clear HIZ bit. 
			    if VBAT < VRECHG, then either charger should automatically recharge or user can toggle disable
			    and enable charge to start charging */
			bq2419x_force_start_recharging(chip);
		}
	}
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_PW_ON_CONFIG,
		CHG_CONFIG_MASK, wdata);
	if(rc < 0) 
		pr_err("bq2419x_chg_en failed: rc=%d\n", rc);
	
	return rc;
}

#if 0
static int bq2419x_chg_term_en(struct pt_bq2419x_chg_chip *chip, bool en)
{
	int rc=0;
	u8 wdata=0;

	if(en)
		wdata = BIT(7);

	printk("chg_term_en : %d\n", en);
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_CHG_TIMER_CTL,
		0x80, wdata);
	if(rc < 0) 
		pr_err("bq2419x_chg_term_en failed: rc=%d\n", rc);

	return rc;
}
#endif

static int bq2419x_force_batt_chg_ctl(struct pt_bq2419x_chg_chip *chip, bool chg_f_dis)
{
	int rc=0;

	chip->chg_f_dis = chg_f_dis;
	rc =	bq2419x_chg_en(chip, !chg_f_dis);
	
	return rc;
}

static int bq2419x_get_batt_chg_ctl(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;
	u8 rData = 0;
	
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_PW_ON_CONFIG, 
		&rData);

	return (rData&0x30)? 0 : 1;
}

//#define DEFAULT_FAULT_MASK	0x7F
static int bq2419x_check_fault_status(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;
	u8 fault_val=0;
	u8 rData = 0;
	
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_FAULT, 
		&rData);
	if(rc < 0) {
		pr_err("bq2419x_check_fault_status failed: rc=%d\n", rc);
		return fault_val;
	}
	
	fault_val = (rData/* & DEFAULT_FAULT_MASK*/);
	return fault_val;
}

#if (CONFIG_BOARD_VER < CONFIG_TP10)
static int bq2419x_power_status(u8 sys_stat)
{	
	return ((sys_stat & BIT(2))>>2);
}

static int bq2419x_get_dpm_stat(u8 sys_stat)
{
	return ((sys_stat & BIT(3))>>3);
}

static int bq2419x_get_vsys_stat(u8 sys_stat)
{
	return (sys_stat & BIT(0));
}
#endif

static int bq2419x_iinlim_set(struct pt_bq2419x_chg_chip *chip, unsigned int mA)
{
	int rc=0;
	u8 iinlim;
	unsigned int lim_mA;
	
	lim_mA = mA;
	
	if(lim_mA > bq2419x_iinlim_val[IINLIM_3000MA])
		lim_mA = bq2419x_iinlim_val[IINLIM_3000MA];

	if(lim_mA < bq2419x_iinlim_val[IINLIM_100MA])
		lim_mA = bq2419x_iinlim_val[IINLIM_100MA];

	if(lim_mA < bq2419x_iinlim_val[IINLIM_500MA])
		gpio_set_value(chip->otg_gpio, 0);
	else
		gpio_set_value(chip->otg_gpio, 1);
	
	for(iinlim=IINLIM_3000MA; iinlim>=0; iinlim--) {
		if(bq2419x_iinlim_val[iinlim] <= lim_mA)
			break;
	}
	
	printk("IN Current %dmA -> %dmA[%d]\n", lim_mA, bq2419x_iinlim_val[iinlim], iinlim);
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_IN_SRC_CTL,
		IINLIM_MASK|EN_HIZ_MASK, iinlim);
	if(rc < 0)
		pr_err("bq2419x_iinlim_set failed: rc=%d\n", rc);

	return rc;
}

static void 
set_appropriate_input_current(struct pt_bq2419x_chg_chip *chip, unsigned int uA)
{
	unsigned int mA;

	mA = uA/1000;

	if(chip->vbus_active) {
		if(chip->fb_stat == FB_BLANK_UNBLANK)
			mA = (IINLIM_OCTA_ON/1000);
		else if(chip->fb_stat == FB_BLANK_POWERDOWN)
			mA = (IINLIM_OCTA_OFF/1000);

		if(chip->aux_present)
			mA = MIN(mA, chip->user_iinlim);
		
		mA = MIN(mA, (uA/1000));
		
		if(chip->dummy_batt_present)
			mA = (IINLIM_DUMMY_BATT/1000);
	}
	else
		mA = IINLIM_MIN;
	
	bq2419x_iinlim_set(chip, mA);
}

void set_user_iinlim(bool en)
{
	testmenu->aux_present = en;
	set_appropriate_input_current(testmenu, testmenu->current_max);
}
EXPORT_SYMBOL_GPL(set_user_iinlim);

#define BQ2419X_ICHG_OFFSET		512		// 512 mA
#define BQ2419X_ICHG_STEP_MA		64		// 64 mA
#define BQ2419X_ICHG_REG_BIT_NUM	6		// Bit[7:2]
static int bq2419x_ichg_set(struct pt_bq2419x_chg_chip *chip, unsigned int mA)
{
	int rc=0;
	int i, start;
	unsigned int ichg;
	bool chg_en=true;
	
	start = BIT(BQ2419X_ICHG_REG_BIT_NUM)-1;	 
	
	for(i=start; i>=0; i--) {
		ichg = (BQ2419X_ICHG_STEP_MA*i)+BQ2419X_ICHG_OFFSET;
		if(ichg <= mA)
			break;
	}

	printk("%s : req ichg:%d mA => ichg:%d mA, val:%d\n", __func__, mA, ichg, i);

	if(i < 0)
		i = 0;
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_CHG_CUR_CTL,
		FAST_CHG_CUR_MASK, (u8) (i<<2));
	if(rc < 0)
		pr_err("bq2419x_ichg_set failed: rc=%d\n", rc);

	if(mA == 0)
		chg_en = false;
	
	bq2419x_chg_en(chip, chg_en);
	return rc;
}

static void 
set_appropriate_battery_current(struct pt_bq2419x_chg_chip *chip)
{
	unsigned int mA;

	if(!chip->vbus_active)
		return;

	mA = batt_therm_table[chip->therm_type].ichg;
	
	bq2419x_ichg_set(chip, mA);
}

#if 0
#define BQ2419X_ITERM_OFFSET		128		// 128 mA
#define BQ2419X_ITERM_STEP_MA		128		// 64 mA
#define BQ2419X_ITERM_REG_BIT_NUM	4	// Bit[7:4]
static int bq2419x_iterm_set(struct pt_bq2419x_chg_chip *chip, unsigned int uA)
{
	int rc=0;
	int i, start;
	unsigned int mA;
	unsigned int iterm;		

	if(uA > ITERM_MAX)
		uA = ITERM_MAX;
	
	mA = uA/1000;
	start = BIT(BQ2419X_ITERM_REG_BIT_NUM)-1;

	for(i=start; i>=0; i--) {
		iterm = (BQ2419X_ITERM_STEP_MA*i)+BQ2419X_ITERM_OFFSET;
		if(iterm <= mA)
			break;
	}

	printk("%s : req iterm:%d mA => iterm:%d mA, val:%d\n", __func__, mA, iterm, i);

	if(i < 0)
		i = 0;
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_PT_CUR_CTL,
		TERM_CUR_LIM_MASK, (u8) (i<<0));
	if(rc < 0) 
		pr_err("bq2419x_iterm_set failed: rc=%d\n", rc);

	return rc;
}

#define BQ2419X_VREG_OFFSET		3504	// 3.504V
#define BQ2419X_VREG_STEP_MA		16		// 64 mA
#define BQ2419X_VREG_REG_BIT_NUM	6	// Bit[7:2]
static int bq2419x_vreg_set(struct pt_bq2419x_chg_chip *chip, unsigned int mV)
{
	int rc=0;
	int i, start;
	unsigned int vreg;		

	if(mV > VREG_MAX)
		mV = VREG_MAX;
	
	start = BIT(BQ2419X_VREG_REG_BIT_NUM)-1;

	for(i=start; i>=0; i--) {
		vreg = (BQ2419X_VREG_STEP_MA*i)+BQ2419X_VREG_OFFSET;
		if(vreg <= mV)
			break;
	}

	printk("%s : req vreg:%d mV => vreg:%d mA, val:%d\n", __func__, mV, vreg, i);

	if(i < 0)
		i = 0;
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_CHG_VOL_CTL,
		CHG_VOL_LIM_MASK, (u8) (i<<2));
	if(rc < 0) 
		pr_err("bq2419x_vreg_set failed: rc=%d\n", rc);

	return rc;
}

static int is_en_hiz_enabled(struct pt_bq2419x_chg_chip *chip)
{
	int rc;
	u8 rData;
	
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_IN_SRC_CTL, 
		&rData);
	
	return (rData & BIT(7)) ? 1 : 0 ;	
}
#endif

#define AUTO_RECHARGE_THRESHOLD_SOC		100
static int can_recharging_start(struct pt_bq2419x_chg_chip *chip, u8 sys_stat)
{	
	if(chip->vbus_active && chip->batt_soc <= AUTO_RECHARGE_THRESHOLD_SOC
		&& is_chg_done(sys_stat)) {
			printk("Charger will start recharging\n");
			return 1;
	}

	return 0;
}

static int
chg_is_otg_en_set(struct pt_bq2419x_chg_chip *chip)
{
	int rdata=0;

	rdata = bq2419x_get_sys_status(chip);
	return ((rdata&0xC0) == 0xC0) ? 1 : 0;
}

static int
switch_usb_to_charge_mode(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;

	printk("++ %s ++\n", __func__);
	if (!chg_is_otg_en_set(chip))
		return 0;

	if(chip->therm_type != BATT_THERM_FATAL_COLD
		|| chip->therm_type != BATT_THERM_FATAL_HOT)
		bq2419x_chg_en(chip, true);
	else
		bq2419x_chg_en(chip, false);

	gpio_set_value(chip->otg_gpio, 0);

	return rc;
}

static int
switch_usb_to_host_mode(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;

	printk("++ %s ++\n", __func__);
	if (chg_is_otg_en_set(chip))
		return 0;

	rc = bq2419x_masked_write(chip->bq2419x_client, REG_PW_ON_CONFIG,
		CHG_CONFIG_MASK, BIT(5));

	gpio_set_value(chip->otg_gpio, 1);
	
	return rc;
}

int chg_force_switching_scope(bool u2c)
{
	int rc=0;
	
	if(!testmenu)
		return -ENXIO;
	
	if(u2c)
		rc = switch_usb_to_charge_mode(testmenu);
	else
		rc = switch_usb_to_host_mode(testmenu);

	return rc;
}
EXPORT_SYMBOL_GPL(chg_force_switching_scope);

static void bq2419x_sel_pw_src(struct pt_bq2419x_chg_chip *chip, 
	enum power_supply_type src_type)
{
	if((src_type == POWER_SUPPLY_TYPE_USB_DCP 
		|| src_type == POWER_SUPPLY_TYPE_USB_CDP
		|| src_type == POWER_SUPPLY_TYPE_USB_ACA)
		|| chip->dummy_batt_present)
		gpio_set_value(chip->psel_gpio, 0);
	else
		gpio_set_value(chip->psel_gpio, 1);
}

static void auto_power_on_set(struct pt_bq2419x_chg_chip *chip, bool flag_en)
{
	int rc=0;
	u8 flag=0;

	if(chip->auto_pw_on)
		return;
	
	if(flag_en)
		flag = BIT(2);
	
	rc = bq2419x_masked_write(chip->bq2419x_client, REG_MISC_OP_CTL,
		AUTO_PW_ON_MASK, flag);
	if(rc < 0)
		pr_err("auto_power_on_set failed: rc=%d\n", rc);
}

static int auto_pw_on_get(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;
	u8 flag=0;
	
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_MISC_OP_CTL, &flag);
	if(rc < 0)
		pr_err("auto_pw_on_get failed: rc=%d\n", rc);

	auto_power_on_set(chip, false);
	
	return (flag&BIT(2)) ? 1 : 0;
}

static void get_smem_data(struct pt_bq2419x_chg_chip *chip)
{
	chip->smem_data = (oem_pm_smem_vendor1_data_type*) smem_alloc(SMEM_ID_VENDOR1,
		sizeof(oem_pm_smem_vendor1_data_type));
	if(!chip->smem_data)
		pr_err("%s: No VENDOR1 data in SMEM\n", __func__);
}

static int fb_notifier_callback(struct notifier_block *self,
				 unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;
	struct pt_bq2419x_chg_chip *chip =
		container_of(self, struct pt_bq2419x_chg_chip, fb_notif);
	
	if (evdata && evdata->data) {
		if (event == FB_EVENT_BLANK) {
			blank = evdata->data;
			chip->fb_stat = *blank;
			if(!chip->vbus_active)
				return 0;
			
			if (*blank == FB_BLANK_UNBLANK || *blank == FB_BLANK_POWERDOWN)
				set_appropriate_input_current(chip, chip->current_max);
		}
	}
	
	return 0;
}

#define CHG_STAT_NOT_CHG		0
#define CHG_STAT_PRE_CHG		1
#define CHG_STAT_FAST_CHG		2
#define CHG_STAT_CHG_DONE		3
static int get_prop_charge_type(struct pt_bq2419x_chg_chip *chip)
{	
	int rc=0;
	int chg_stat;
	u8 rData = 0;
	
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_SYS_STATUS, 
		&rData);
	if(rc < 0) {
		pr_err("get_prop_charge_type failed: rc=%d\n", rc);
		return POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
	}

	chg_stat = ((rData & (3<<4))>>4);
	switch(chg_stat) {
		case CHG_STAT_NOT_CHG:
			rc =  POWER_SUPPLY_CHARGE_TYPE_NONE;
			break;
		case CHG_STAT_PRE_CHG:
		case CHG_STAT_FAST_CHG:
			rc = POWER_SUPPLY_CHARGE_TYPE_FAST;
			break;
		case CHG_STAT_CHG_DONE:
			rc = POWER_SUPPLY_CHARGE_TYPE_FAST;
			break;
		default:
			rc = POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
			break;
	}
	
	return rc;
}

static int get_prop_batt_status(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;
	int chg_stat;
	u8 rData = 0;
	
	if(!chip->cable_present)
		return POWER_SUPPLY_STATUS_DISCHARGING;
		
	if(chip->batt_soc >= 100 && chip->cable_present)
		return POWER_SUPPLY_STATUS_FULL;
		
	rc = bq2419x_read_reg(chip->bq2419x_client, REG_SYS_STATUS, 
		&rData);
	if(rc < 0) {
		pr_err("get_prop_batt_status failed: rc=%d\n", rc);
		return POWER_SUPPLY_STATUS_UNKNOWN;
	}

	chg_stat = ((rData & (3<<4))>>4);
	switch(chg_stat) {
		case CHG_STAT_NOT_CHG:
			rc =  POWER_SUPPLY_STATUS_NOT_CHARGING;
			break;
		case CHG_STAT_PRE_CHG:
		case CHG_STAT_FAST_CHG:
			rc = POWER_SUPPLY_STATUS_CHARGING;
			break;
		case CHG_STAT_CHG_DONE:
			rc = POWER_SUPPLY_STATUS_FULL;
			break;
		default:
			rc = POWER_SUPPLY_STATUS_UNKNOWN;
			break;
	}
	
	return rc;
}

int get_batt_status(void)
{
	struct power_supply *batt_psy;
	union power_supply_propval ret = {0,};
	
	batt_psy = power_supply_get_by_name("battery");
	if (batt_psy == NULL) {
		pr_err("usb supply not found deferring probe\n");
	}
	
	batt_psy->get_property(batt_psy, POWER_SUPPLY_PROP_STATUS, &ret);
	if (!batt_psy) {
		pr_err("usb supply not found deferring probe\n");
	}
	
    	return ret.intval;
}
EXPORT_SYMBOL_GPL(get_batt_status);

static int
get_prop_batt_health(struct pt_bq2419x_chg_chip *chip)
{
	int batt_health;
	
	switch(chip->therm_type)
	{
		case BATT_THERM_FATAL_COLD:
			batt_health = POWER_SUPPLY_HEALTH_COLD;
			break;
		case BATT_THERM_CRITICAL_COLD:
		case BATT_THERM_CRITICAL_HOT:
		case BATT_THERM_NORMAL:
		case BATT_THERM_WARM:
			batt_health = POWER_SUPPLY_HEALTH_GOOD;
			break;

		case BATT_THERM_FATAL_HOT:
			batt_health = POWER_SUPPLY_HEALTH_OVERHEAT;
			break;
		default:
			batt_health = POWER_SUPPLY_HEALTH_GOOD;
			break;
	}

	return batt_health;
}

#define DEFAULT_TEMP		250
static int get_prop_batt_temp(struct pt_bq2419x_chg_chip *chip)
{
	return chip->batt_temp;
}

static int get_batt_temp(struct pt_bq2419x_chg_chip *chip)
{
#if (CONFIG_BOARD_VER == CONFIG_PT10)  
	return DEFAULT_TEMP;
#else
	int rc = 0;
	struct qpnp_vadc_result results;

	if (!get_prop_batt_present(chip) || chip->dummy_batt_present)
		return DEFAULT_TEMP;

	rc = qpnp_vadc_read(chip->vadc_dev, LR_MUX1_BATT_THERM, &results);
	if (rc) {
		pr_debug("Unable to read batt temperature rc=%d\n", rc);
		return 0;
	}
	pr_debug("get_bat_temp %d %lld\n",
		results.adc_code, results.physical);

	return (int)results.physical;	
#endif
}

static int get_batt_id(struct pt_bq2419x_chg_chip *chip)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (!get_prop_batt_present(chip))
		return 1800;

	rc = qpnp_vadc_read(chip->vadc_dev, LR_MUX2_BAT_ID, &results);
	if (rc) {
		pr_debug("Unable to read batt ID rc=%d\n", rc);
		return 0;
	}
	pr_debug("get_bat_temp %d %lld\n",
		results.adc_code, results.physical);

	return ((int)results.physical) / 1000;	
}

static bool is_dummy_batt_present(struct pt_bq2419x_chg_chip *chip)
{
	int batt_id_adc;
	
	batt_id_adc = get_batt_id(chip);
	
	return ((batt_id_adc > FACTORY_DUMMY_ID_MIN) && (batt_id_adc  < FACTORY_DUMMY_ID_MAX));
}

static int get_prop_battery_voltage_now(struct pt_bq2419x_chg_chip *chip)
{
	return (int) chip->batt_uvolt;
}

static int get_prop_capacity(struct pt_bq2419x_chg_chip *chip)
{
#if (CONFIG_BOARD_VER >= CONFIG_WS10)
	int adj_soc=0;

	if(chip->dummy_batt_present)
		return 50;
	
	adj_soc = chip->batt_soc;
	
	if(adj_soc < 0)
		adj_soc = 0;

	if(adj_soc >= 100)
		adj_soc = 100;
	
	return adj_soc;
#else
	return 50;
#endif
}

#define TEMP_TRIP_HYSTERISIS_DEGC		30
static int get_curr_therm_trip(struct pt_bq2419x_chg_chip *chip, int temp)
{
	int i;

	for(i=0; i < ARRAY_SIZE(batt_therm_table); i++) {
		if(temp < batt_therm_table[i].batt_temp)
			return batt_therm_table[i].therm_trip;
	}

	return BATT_THERM_UNKNOWN;
}

static int battery_temp_trip_changed(struct pt_bq2419x_chg_chip *chip, int temp)
{
	int curr_trip = get_curr_therm_trip(chip, temp);
	int trip_changed=0;
	
	if(curr_trip == chip->therm_type || curr_trip > BATT_THERM_UNKNOWN)
		return 0;

	switch(chip->therm_type) {
		case BATT_THERM_FATAL_COLD:
			if(temp >= (batt_therm_table[chip->therm_type].batt_temp + TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_CRITICAL_COLD:
			if(temp >= (batt_therm_table[chip->therm_type].batt_temp + TEMP_TRIP_HYSTERISIS_DEGC)
				|| temp < batt_therm_table[chip->therm_type-1].batt_temp) 
				trip_changed = 1;
			
			break;
		case BATT_THERM_NORMAL:
			if(temp >= batt_therm_table[chip->therm_type].batt_temp
				|| temp < batt_therm_table[chip->therm_type-1].batt_temp) 
				trip_changed = 1;
			break;
		case BATT_THERM_WARM:
			if(temp >= batt_therm_table[chip->therm_type].batt_temp
				|| temp < (batt_therm_table[chip->therm_type-1].batt_temp-TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_CRITICAL_HOT:
			if(temp >= batt_therm_table[chip->therm_type].batt_temp
				|| temp < (batt_therm_table[chip->therm_type-1].batt_temp-TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_FATAL_HOT:
			if(temp < (batt_therm_table[chip->therm_type-1].batt_temp - TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_UNKNOWN:
			chip->therm_type = curr_trip;
			trip_changed = 1;
			break;
		default:
			return 0;
			break;
	}

	if(trip_changed) {
		printk("batt thermal trip changed: %d -> %d\n", chip->therm_type, curr_trip);
		chip->therm_type = curr_trip;
	}
		
	return trip_changed;
}

static int
bq2419x_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{

	struct pt_bq2419x_chg_chip *chip = container_of(psy, struct pt_bq2419x_chg_chip,
								batt_psy);
	
	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = get_prop_batt_status(chip);
			break;
		case POWER_SUPPLY_PROP_CHARGE_TYPE:
			val->intval = get_prop_charge_type(chip);
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = get_prop_batt_present(chip);
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = get_prop_batt_health(chip);
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
			val->intval = 4350 * 1000;	// 4.35V
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
			val->intval = 4350 * 1000;	// 4.35V
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = get_prop_battery_voltage_now(chip);
			break;
		case POWER_SUPPLY_PROP_CAPACITY:			
			val->intval = get_prop_capacity(chip);
			break;
		case POWER_SUPPLY_PROP_TEMP:
			val->intval = get_prop_batt_temp(chip);
			break;
		default:
			return -EINVAL;
	}

//	pr_err("[Battery get] psp:%d, intval:%d\n", psp, val->intval);
	return 0;
}

static int
bq2419x_batt_power_set_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  const union power_supply_propval *val)
{
	return 0;
}

static int
bq2419x_batt_property_is_writeable(struct power_supply *psy,
						enum power_supply_property psp)
{
	return 0;
}

static void
bq2419x_batt_external_power_changed(struct power_supply *psy)
{
	struct pt_bq2419x_chg_chip *chip = container_of(psy, struct pt_bq2419x_chg_chip,
								batt_psy);
	struct power_supply *ext;
	union power_supply_propval ret = {0,};
	int i;

	ext = chip->usb_psy;
	for (i = 0; i < ext->num_properties; i++) {
		enum power_supply_property prop;
		prop = ext->properties[i];

		if (ext->get_property(ext, prop, &ret))
			continue;

		switch (prop) {
		case POWER_SUPPLY_PROP_PRESENT:
			if(chip->vbus_active == ret.intval)
				break;
			
			chip->vbus_active = ret.intval;
			if(chip->vbus_active) {
#if defined(FEATURE_WORKAROUND_NC_CHARGER)
				if(!delayed_work_pending(&chip->nc_chg_check_work))
					schedule_delayed_work(&chip->nc_chg_check_work,
								msecs_to_jiffies(USB_WAITING_TIME_STEP1));
#endif				
			}
			break;
		case POWER_SUPPLY_PROP_ONLINE:
			if(chip->online == ret.intval)
				break;
			chip->online = ret.intval;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MAX:
			break;
		case POWER_SUPPLY_PROP_CURRENT_MAX:
			if(chip->current_max == ret.intval)
				break;
			
			chip->current_max = ret.intval;
			set_appropriate_input_current(chip, chip->current_max);
			set_appropriate_battery_current(chip);
			break;
		case POWER_SUPPLY_PROP_TYPE:
			if(chip->chg_type == ret.intval)
				break;
			chip->chg_type = ret.intval;
			bq2419x_sel_pw_src(chip, chip->chg_type);

#if defined(FEATURE_WORKAROUND_NC_CHARGER)
			if(chip->chg_type == POWER_SUPPLY_TYPE_BATTERY 
				&& !chip->vbus_active && chip->online) {
				power_supply_set_online(chip->usb_psy, false);
				power_supply_set_current_limit(chip->usb_psy, 0);
			}
#endif			
			break;
		case POWER_SUPPLY_PROP_SCOPE:
			if((chip->host_mode == ret.intval) || (ret.intval == POWER_SUPPLY_SCOPE_UNKNOWN))
				break;
			chip->host_mode = ret.intval;
			if ((chip->host_mode == POWER_SUPPLY_SCOPE_SYSTEM)
				&& !chg_is_otg_en_set(chip))
				switch_usb_to_host_mode(chip);

			if ((chip->host_mode == POWER_SUPPLY_SCOPE_DEVICE)
					&& chg_is_otg_en_set(chip))
				switch_usb_to_charge_mode(chip);
			return;			
		default:
			break;
		}		
	}
	printk("vbus:%d, online:%d, current:%d, type:%d, scope:%d\n", chip->vbus_active, chip->online
		,chip->current_max, chip->chg_type, chip->host_mode);
	
	power_supply_changed(&chip->batt_psy);

}

static enum power_supply_property bq_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,	
};

//static char *pm_power_supplied_to[] = {
//	"battery",
//};

static char *pm_batt_supplied_to[] = {
	"bms",
};

static void bq2419x_regs_init(struct pt_bq2419x_chg_chip *chip) 
{		
	bq2419x_fault_irq_set(chip, false);
	
	/* clear fault register */
	bq2419x_check_fault_status(chip);
}

static void bq2419x_hw_init(struct pt_bq2419x_chg_chip *chip)
{
	int rc;

	msm_gpiomux_install(bq2419x_gpio_configs, ARRAY_SIZE(bq2419x_gpio_configs));
	
	chip->otg_gpio = GPIO_SC_OTG;
	chip->stat_gpio  = GPIO_SC_STAT;
	chip->ce_n_gpio  = GPIO_SC_CE_N;
	chip->psel_gpio = GPIO_SC_PSEL;
#if (CONFIG_BOARD_VER >= CONFIG_WS10)
	chip->int_gpio =
			of_get_named_gpio(chip->bq2419x_client->dev.of_node, "bq2419x,int-gpio", 0);
#else
	chip->int_gpio  = GPIO_SC_INT;
#endif

	chip->vin_gpio = 
			of_get_named_gpio(chip->bq2419x_client->dev.of_node, "bq2419x,vin-gpio", 0);

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	chip->pg_gpio = 
			of_get_named_gpio(chip->bq2419x_client->dev.of_node, "bq2419x,pg-gpio", 0);
#endif

	/* config SC_OTG pin */
	rc = gpio_request(chip->otg_gpio, "sc_otg");
	if (rc)
		pr_err("%s : error : gpio_request sc_otg\n", __func__);

	rc = gpio_direction_output(chip->otg_gpio, 1);
	if (rc )
		pr_err("%s : error : gpio_direction_output sc_otg\n", __func__);

	gpio_set_value(chip->otg_gpio, 1);
	
	/* config SC_STAT pin */
	rc = gpio_request(chip->stat_gpio, "sc_stat");
	if (rc)
		pr_err("%s : error : gpio_request sc_stat\n", __func__);

	rc = gpio_direction_input(chip->stat_gpio);
	if (rc )
		pr_err("%s : error : gpio_direction_input stat_gpio\n", __func__);
	
	/* config SC_INT pin */
	rc = gpio_request(chip->int_gpio, "sc_int");
	if (rc)
		pr_err("%s : error : gpio_request sc_int\n", __func__);

	/* config SC_CE_N pin */
	rc = gpio_request(chip->ce_n_gpio, "sc_ce_n");
	if (rc)
		pr_err("%s : error : gpio_request sc_ce_n\n", __func__);

	rc = gpio_direction_output(chip->ce_n_gpio, 0);
	if (rc )
		pr_err("%s : error : gpio_direction_output sc_ce_n\n", __func__);

	/* enable charge device */
	gpio_set_value(chip->ce_n_gpio, 0);

	/* config SC_PSEL pin */
	rc = gpio_request(chip->psel_gpio, "sc_psel");
	if (rc)
		pr_err("%s : error : gpio_request sc_psel\n", __func__);

	rc = gpio_direction_output(chip->psel_gpio, 1);
	if (rc )
		pr_err("%s : error : gpio_direction_output sc_psel\n", __func__);
	
	/* USB host source */
	gpio_set_value(chip->psel_gpio, 1);

	rc = gpio_request(chip->vin_gpio, "vbus_int");
	if (rc)
		pr_err("%s : error : gpio_request vin_gpio\n", __func__);

	rc = gpio_direction_input(chip->vin_gpio);
	if (rc )
		pr_err("%s : error : gpio_direction_input vin_gpio\n", __func__);
	
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	rc = gpio_request(chip->pg_gpio, "power_good");
	if (rc)
		pr_err("%s : error : gpio_request pg_gpio\n", __func__);
#endif

	bq2419x_regs_init(chip);
}

#if defined(FEATURE_WORKAROUND_NC_CHARGER)
static void
non_conforming_chg_check_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct pt_bq2419x_chg_chip *chip = container_of(dwork,
				struct pt_bq2419x_chg_chip, nc_chg_check_work);
	int rc=0;
	int usb_event_waiting_time=USB_WAITING_TIME_STEP1;
	union power_supply_propval ret = {0,};
	struct power_supply *ext;
	char udc_state[128] = {0,};
	unsigned int charger_online;
	int usb_configured;

	printk("%s enter\n", __func__);
	
	ext = chip->usb_psy;
	
	ext->get_property(chip->usb_psy,
			  POWER_SUPPLY_PROP_PRESENT, &ret);
	chip->vbus_active = ret.intval;

#ifdef CONFIG_PANTECH_USB_BLOCKING_MDMSTATE
	if(!chip->vbus_active || 0 < get_pantech_mdm_state())
		return;
#else
	if(!chip->vbus_active)
		return;
#endif


	rc = composite_get_udc_state(udc_state);
	if(rc<0) {
	    	pr_err("%s: Failed to get udc_state. rc=%d\n", __func__, rc);

		usb_event_waiting_time = USB_WAITING_TIME_STEP1;	
		goto waiting_usb_event;
	}

	usb_configured = (strcmp(udc_state, "CONFIGURED") ? 0 : 1);
	
	if(usb_configured) {
		printk("USB external changed. [UDC_STATE:%s]\n", udc_state);
		return;
	}

	if(chip->chg_type == POWER_SUPPLY_TYPE_USB
		|| chip->chg_type == POWER_SUPPLY_TYPE_BATTERY) {
		printk("set charger type\n");
		power_supply_set_supply_type(chip->usb_psy, POWER_SUPPLY_TYPE_USB_DCP);
	}
	
	ext->get_property(chip->usb_psy,
			  POWER_SUPPLY_PROP_ONLINE, &ret);
	charger_online = ret.intval;
	if(!charger_online) {
		printk("set charger online\n");
		power_supply_set_online(chip->usb_psy, true);
	}
	else {
		printk("set charger current limit => iinlim max\n");
		power_supply_set_current_limit(chip->usb_psy, IINLIM_MAX);
		return;
	}

	ext->get_property(chip->usb_psy,
			  POWER_SUPPLY_PROP_CURRENT_MAX, &ret);
	if(ret.intval <= IINLIM_MIN) {
		printk("set charger current limit => iinlim min\n");
		power_supply_set_current_limit(chip->usb_psy, IINLIM_MIN); 
	}
	
	usb_event_waiting_time = USB_WAITING_TIME_STEP2;

waiting_usb_event:
	schedule_delayed_work(&chip->nc_chg_check_work,
					msecs_to_jiffies(usb_event_waiting_time));
}
#endif

#if defined(CONFIG_PANTECH_USB_TI_OTG_DISABLE_LOW_BATTERY)
extern int get_pantech_otg_enabled(void);
extern int pantech_otg_uvlo_notify(int on);
#endif

static void
update_heartbeat_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct pt_bq2419x_chg_chip *chip = container_of(dwork,
				struct pt_bq2419x_chg_chip, update_heartbeat);
	int curr_soc=50, curr_vcell=0, curr_temp=0, curr_rcomp=0;
	int test_monitor_enable;
	int batt_changed=0;
	int poll_time= DEFAULT_UPDATE_TIME;	// 30s
	u8 sys_stat;
	
	wake_lock(&chip->heartbeat_wake_lock);

#if 0
	if(is_en_hiz_enabled(chip)) {
		printk("EN_HIZ is enabled\n");
		bq2419x_disable_en_hiz(chip);
	}
#endif

	if(get_prop_batt_present(chip)) {
		if(pt_max17058_get_soc(&curr_soc) < 0)
			curr_soc = chip->batt_soc;
		
#if defined(CONFIG_PANTECH_USB_TI_OTG_DISABLE_LOW_BATTERY)
		if ((curr_soc < 10) && get_pantech_otg_enabled())
			pantech_otg_uvlo_notify(1);
#endif
		
		if(pt_max17058_get_vcell(&curr_vcell) < 0)
			curr_vcell = DEFAULT_BATT_VOL;
		
		chip->batt_uvolt = curr_vcell;
		
		curr_temp = get_batt_temp(chip);
		
		curr_rcomp = pt_max17058_calc_rcomp((curr_temp/10));
		if(chip->rcomp != curr_rcomp) {
			chip->rcomp = curr_rcomp;
			pt_max17058_set_rcomp((u8)chip->rcomp, 0x1F);
		}

		if(curr_soc != chip->batt_soc || curr_temp != chip->batt_temp) {
			chip->batt_soc = curr_soc;
			chip->batt_temp = curr_temp;
			batt_changed = 1;
		}

		sys_stat = bq2419x_get_sys_status(chip);
		if(battery_temp_trip_changed(chip, chip->batt_temp) || can_recharging_start(chip, sys_stat))
			set_appropriate_battery_current(chip);

#if 0
#if (CONFIG_BOARD_VER >= CONFIG_TP10)		
		printk("[SOC:%d, VCELL:%d, TEMP:%d, chg state:%s, pg:%s, vin:%s, REG08:0x%x]\n", curr_soc, 
				curr_vcell, curr_temp, (gpio_get_value(chip->stat_gpio)) ? "discharging" : "charging", 
				(gpio_get_value(chip->pg_gpio)) ? "disconnected" : "connected",
				(gpio_get_value(chip->vin_gpio)) ? "not powered" : "powered", sys_stat);	
#else
		printk("[SOC:%d, VCELL:%d, TEMP:%d, chg state:%s, vin:%s, REG08:0x%x]\n", curr_soc, 
				curr_vcell, curr_temp, (gpio_get_value(chip->stat_gpio)) ? "discharging" : "charging", 
				(gpio_get_value(chip->vin_gpio)) ? "not powered" : "powered", sys_stat);	
#endif
#endif

		/* Test menu */
		test_monitor_enable = atomic_read(&chip->bms_input_flag);
		if(test_monitor_enable)
		{            							
			input_report_rel(chip->bms_input_dev, REL_RX, chip->batt_soc);
			input_report_rel(chip->bms_input_dev, REL_RY, (chip->batt_uvolt/1000));                
			input_report_rel(chip->bms_input_dev, REL_RZ, chip->batt_temp);	
//			input_report_rel(chip->bms_input_dev, REL_Y, bq2419x_get_dpm_stat(bq2419x_get_sys_status(chip)));
			input_sync(chip->bms_input_dev);
		}
	}

	if(!chip->cable_present && chip->batt_soc <= LOW_BATT_WARN_THR)
		poll_time = (poll_time/10);		// 3s

//	if(test_monitor_enable)
//		poll_time = 1000;		// 1s
	
	chip->update_time = poll_time;
	
	if(batt_changed)
		power_supply_changed(&chip->batt_psy);
	
	schedule_delayed_work(&chip->update_heartbeat,
					msecs_to_jiffies(chip->update_time));

	wake_unlock(&chip->heartbeat_wake_lock);
}

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
static void
power_good_irq_work(struct work_struct *work)
{
	struct pt_bq2419x_chg_chip *chip = container_of(work,
				struct pt_bq2419x_chg_chip, pg_irq_work);
	
	chip->cable_present = (gpio_get_value(chip->pg_gpio)) ? 0 : 1;
	
	if(!chip->cable_present) {
#if defined(FEATURE_WORKAROUND_NC_CHARGER)		
		if(delayed_work_pending(&chip->nc_chg_check_work)) 
				cancel_delayed_work(&chip->nc_chg_check_work);
#endif					
		power_supply_set_present(chip->usb_psy, false);

		wake_unlock(&chip->chg_wake_lock);
	}
	else {
		wake_lock(&chip->chg_wake_lock);
		
		power_supply_set_present(chip->usb_psy, true);
	}

	printk("Charger online : %d\n", chip->cable_present);
}
#endif

static void
bq2419x_irq_work(struct work_struct *work)
{
	struct pt_bq2419x_chg_chip *chip = container_of(work,
				struct pt_bq2419x_chg_chip, irq_work);
	int last_INT=0, curr_fault=0;
#if (CONFIG_BOARD_VER < CONFIG_TP10)	
	u8 sys_stat=0;
#endif

#if (CONFIG_BOARD_VER >= CONFIG_TP10)	
	if(!chip->vbus_active)
		return;
#endif

	last_INT = bq2419x_check_fault_status(chip);
	curr_fault = bq2419x_check_fault_status(chip);
	if(last_INT || curr_fault)
		pr_err("last INT : 0x%x, current fault register status : 0x%x\n", last_INT, curr_fault);
	
#if (CONFIG_BOARD_VER < CONFIG_TP10)
	sys_stat = bq2419x_get_sys_status(chip);
	if(chip->cable_present) {
		if(bq2419x_get_dpm_stat(sys_stat)) {
			pr_err("VINDPM or IINDPM mode\n");
		}
		
		if(bq2419x_get_vsys_stat(sys_stat)) {
			pr_err("In VSYSMIN regulation(BAT<VSYSMIN)\n");
		}

		if(!bq2419x_power_status(sys_stat)) {
#if defined(FEATURE_WORKAROUND_NC_CHARGER)		
			if(delayed_work_pending(&chip->nc_chg_check_work)) 
				cancel_delayed_work(&chip->nc_chg_check_work);
#endif			
			chip->cable_present = false;
			wake_unlock(&chip->chg_wake_lock);
		}
		else
			return;
	}
	else {
		if(bq2419x_power_status(sys_stat)) {
			wake_lock(&chip->chg_wake_lock);
			chip->cable_present = true;
		}
		else
			return;
	}
	
	printk("Charger online : %d\n", chip->cable_present);
	power_supply_set_present(chip->usb_psy, chip->cable_present);
#endif	
}

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
static irqreturn_t power_good_irq_handler(int irq, void *_chip)
{
	struct pt_bq2419x_chg_chip *chip = _chip;

	schedule_work(&chip->pg_irq_work);
	
	return IRQ_HANDLED;
}
#endif

static irqreturn_t bq2419x_isr(int irq, void *_chip)
{
	struct pt_bq2419x_chg_chip *chip = _chip;

	schedule_work(&chip->irq_work);
	
	return IRQ_HANDLED;
}

static ssize_t bms_input_show_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
	int enable;
	enable = atomic_read(&testmenu->bms_input_flag);
	return sprintf(buf, "%d\n", enable);
}

static ssize_t bms_input_store_flag(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u8 scale = (u8)simple_strtoul(buf, NULL, 10);	
	atomic_set(&testmenu->bms_input_flag, scale);
	return count;
}

#ifdef CONFIG_PANTECH_USER_BUILD
static DEVICE_ATTR(setflag, S_IRUGO, bms_input_show_flag, bms_input_store_flag);
#else
static DEVICE_ATTR(setflag, S_IWUGO | S_IRUGO, bms_input_show_flag, bms_input_store_flag);
#endif

static int chg_test_open(struct inode *inode, struct file *file)
{
	return 0;
}

static long chg_test_ioctl(struct file *file,
		    unsigned int cmd, unsigned long arg)
{
    	int rc;
	int data;
    	uint32_t n;

	if (!testmenu) {
		pr_debug("%s: testmenu is NULL\n", __func__);
		return 0;
	}
	
    	pr_info("%s: cmd = [%x]\n", __func__, cmd);

	switch (cmd) {
	case PM8941_CHARER_TEST_SET_CHARING_CTL:
		rc = copy_from_user(&n, (void *)arg, sizeof(n));
		if (!rc) {
		    	bq2419x_force_batt_chg_ctl(testmenu, (bool) n);
		}
		break;

	case PM8941_CHARER_TEST_GET_CHARING_CTL:
		data = bq2419x_get_batt_chg_ctl(testmenu);
		if (copy_to_user((void *)arg, &data, sizeof(int)))
		    	rc = -EINVAL;
		else
		    	rc = 0;
		break;

	case PM8941_CHARGER_TEST_SET_PM_CHG_TEST:
	    rc = copy_from_user(&n, (void *)arg, sizeof(n));
	    if (!rc) {
	        	bq2419x_force_batt_chg_ctl(testmenu, (bool) n);
	    }	
	    break;

	case PM8941_CHARGER_TEST_CHARGING_SETTING:
		rc = copy_from_user(&n, (void *)arg, sizeof(n));
		pr_err("PM8941_CHARGER_TEST_CHARGING_SETTING rc: %d, n: %d\n", rc, n);
		
		if (!rc) {
			testmenu->charging_setting = n;
			if(n) {
				bq2419x_sel_pw_src(testmenu, POWER_SUPPLY_TYPE_USB_DCP);
				set_appropriate_input_current(testmenu, IINLIM_MAX);
			}
			else{
				bq2419x_sel_pw_src(testmenu, testmenu->chg_type);
				set_appropriate_input_current(testmenu, testmenu->current_max);
			}
		}
		break;

	case PM8941_CHARGER_TEST_GET_AUTO_PW_ON:
		data = testmenu->auto_pw_on;
		if (copy_to_user((void *)arg, &data, sizeof(int)))
		    	rc = -EINVAL;
		else
		    	rc = 0;
		break;
	case PM8941_CHARGER_TEST_SET_IINLIM:
		rc = copy_from_user(&n, (void *)arg, sizeof(n));
		pr_info("PM8941_CHARGER_TEST_SET_IINLIM rc: %d, n: %d\n", rc, n);
		testmenu->user_iinlim = n;
		if(testmenu->aux_present)
			set_appropriate_input_current(testmenu, testmenu->current_max);
		
		break;
	default:
	    rc = -EINVAL;
	}

	return rc;
}

static int chg_test_release(struct inode *inode, struct file *file)
{
	return 0;
}

static struct attribute *bms_input_attrs[] = {
	&dev_attr_setflag.attr,
	NULL,
};

static struct attribute_group bms_input_attr_group = {
	.attrs = bms_input_attrs,
};

static struct file_operations chg_test_fops = {
	.owner = THIS_MODULE,
	.open = chg_test_open,
	.unlocked_ioctl	= chg_test_ioctl,
	.release	= chg_test_release,
};

struct miscdevice chg_test_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "qcom,qpnp-charger",
	.fops	= &chg_test_fops,
};

static int bq2419x_subdevices_register(struct pt_bq2419x_chg_chip *chip)
{
	int rc=0;

	atomic_set(&chip->bms_input_flag, 0);

	bms_input_attr_dev = platform_device_register_simple("bms_app_attr", -1, NULL, 0);

	if (!bms_input_attr_dev) {
		pr_debug("BMS: Unable to register platform_device_register_simple device\n");
	}

	rc = sysfs_create_group(&bms_input_attr_dev->dev.kobj, &bms_input_attr_group);	
	if (rc) {
		pr_debug("[BMS] failed: sysfs_create_group  [ERROR]\n");	
		rc = -ENOENT;
	    	return rc;
	} 

	chip->bms_input_dev = input_allocate_device();
	if (!chip->bms_input_dev) {
	    	pr_err("BMS: Unable to input_allocate_device \n");
	    	rc = -ENXIO;
	    	return rc;
	}

	set_bit(EV_REL, chip->bms_input_dev->evbit);
	input_set_capability(chip->bms_input_dev, EV_REL, REL_RX);	// SOC
	input_set_capability(chip->bms_input_dev, EV_REL, REL_RY); 	// Volt
	input_set_capability(chip->bms_input_dev, EV_REL, REL_RZ);    // TEMP
	input_set_capability(chip->bms_input_dev, EV_REL, REL_Y);	// DPM status
	chip->bms_input_dev->name="bms_app";
	rc =input_register_device(chip->bms_input_dev);
	if (rc) {
	    	pr_err("BMS: Unable to register input_register_device device\n");
	    	return rc;
	}

	rc = misc_register(&chg_test_device);
	
	return rc;
}

static char *str_cable_type[] = {
		"NO_CABLE", "STANDARD_CABLE", "FACTORY_CABLE", "TA_CABLE", "WIRELESS_CABLE",
		"UNKNOWN_CABLE", "INVALID_CABLE",
};

static int proc_debug_pm_chg_get_CableType(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int cable_index = 5;
	struct power_supply *usb_psy;
	union power_supply_propval ret = {0,};
	
	usb_psy = power_supply_get_by_name("usb");
	if (usb_psy == NULL) {
		pr_err("usb supply not found deferring probe\n");
	}
	usb_psy->get_property(usb_psy, POWER_SUPPLY_PROP_TYPE, &ret);
	if (!usb_psy) {
		pr_err("usb supply not found deferring probe\n");
		return sprintf(page, "%s\n", str_cable_type[cable_index]);
	}
	
	switch(ret.intval) {
		case POWER_SUPPLY_TYPE_USB:
		    cable_index = 1;
		    break;
		case POWER_SUPPLY_TYPE_USB_DCP:
		case POWER_SUPPLY_TYPE_USB_CDP:
		case POWER_SUPPLY_TYPE_USB_ACA:		
		    cable_index = 3;
		    break;
	}

	*eof = 1;  

	return sprintf(page, "%s\n", str_cable_type[cable_index]); //imsi  charging type 
}

static int proc_debug_pm_chg_get_CableID(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int64_t rc;
	struct qpnp_vadc_result results;	
	
	*eof = 1;  

	if (!testmenu) {
		pr_debug("%s: testmenu is NULL\n", __func__);
		return 0;
	}
	
	if (testmenu->cable_present)
		rc = qpnp_vadc_read(testmenu->vadc_dev, LR_MUX10_PU2_AMUX_USB_ID_LV, &results);
	else
		results.physical = 0;  
	
	return sprintf(page, "%lld\n", results.physical );  //imsi charging cable ID 
}

static int proc_debug_pm_chg_get_BattID(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int64_t rc;
	struct qpnp_vadc_result results;

	if (!testmenu) {
		pr_debug("%s: the_chip is NULL\n", __func__);
		return 0;
	}

	rc = qpnp_vadc_read(testmenu->vadc_dev, LR_MUX2_BAT_ID, &results);//LR_MUX2_PU1_PU2_BAT_ID, &results);		20130604 djjeon channel modify
	*eof = 1;  

	return sprintf(page, "%lld\n", results.physical);
}

static int proc_debug_pm_chg_get_IInLim(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int64_t rc = 0;
	int iin_lim = 0;
	u8 rData = 0;

	if (!testmenu) {
		pr_debug("%s: the_chip is NULL\n", __func__);
		return 0;
	}

	rc =bq2419x_read_reg(testmenu->bq2419x_client, REG_IN_SRC_CTL, &rData);
	iin_lim = (rData & 7);

	*eof = 1;  
	return sprintf(page, "%d\n",bq2419x_iinlim_val[iin_lim]);
}

static int bq2419x_ichg_val[] = {64, 128, 256, 512, 1024, 2048};
static int proc_debug_pm_chg_get_Ichg(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int64_t rc;
	int ichg,i=0;
	u8 rData;
	int chg[6]={0,};
	if (!testmenu) {
		pr_debug("%s: the_chip is NULL\n", __func__);
		return 0;
	}

	rc =bq2419x_read_reg(testmenu->bq2419x_client, REG_CHG_CUR_CTL, &rData);
	ichg = ((rData & (63<<2))>>2);
	
	for(i =0 ; i<=5 ; i++)
	{

		chg[i] = ichg%2;
		ichg /=2;
		if(!ichg)	break;
	}		

	for(i=5; i>=0 ; i--)
		ichg += bq2419x_ichg_val[i]*chg[i];
	ichg += BQ2419X_ICHG_OFFSET;

	*eof = 1;  
	return sprintf(page, "%d\n",ichg);
}

static int proc_debug_pm_chg_get_SetIInLim(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	if (!testmenu) {
		pr_debug("%s: the_chip is NULL\n", __func__);
		return 0;
	}
	*eof = 1;

	return sprintf(page, "%d\n", testmenu->user_iinlim);
}

static int proc_debug_pm_chg_get_charging_setting(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	if (!testmenu) {
		pr_debug("%s: the_chip is NULL\n", __func__);
		return 0;
	}

	*eof = 1;

	return sprintf(page, "%d\n", testmenu->charging_setting);
}

static void create_testmenu_entries(struct pt_bq2419x_chg_chip *chip)
{
	struct proc_dir_entry *ent;
	
	chip->bq2419x_charger_dir = proc_mkdir(PT_BQ2419X_CHARGER_DEV_NAME, NULL);

	if (chip->bq2419x_charger_dir == NULL) {
		pr_err("Unable to create /proc/%s directory\n", PT_BQ2419X_CHARGER_DEV_NAME);
	}

	testmenu = chip;
	
	/* cable type */
	ent = create_proc_entry("cable_type", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/cable_type entry\n", __func__);
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_CableType;

	/* cable ID */
	ent = create_proc_entry("cable_id", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/cable_id entry\n", __func__);
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_CableID;

	/* battery ID */
	ent = create_proc_entry("batt_id", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/batt_id entry\n", __func__);
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_BattID;

	/* Input Current Limit */
	ent = create_proc_entry("iinlim", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/iinlim entry\n", __func__);
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_IInLim;


	/* Fast Charge Current Limit */
	ent = create_proc_entry("ichg", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/ichg entry\n", __func__);
		return;
	}	
	ent->read_proc = proc_debug_pm_chg_get_Ichg;
			
	/* Charging setting for Stability test. */
	ent = create_proc_entry("charging_setting", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/charging_setting entry\n", __func__);
		return;
	}
	ent->read_proc = proc_debug_pm_chg_get_charging_setting;      


	ent = create_proc_entry("setiinlim", 0, chip->bq2419x_charger_dir);
	if (!ent) {
		pr_err("%s: Unable to create /proc/setiinlim entry\n", __func__);
		return;
}
	ent->read_proc = proc_debug_pm_chg_get_SetIInLim;
}

static int __devinit bq2419x_i2c_probe(struct i2c_client *client,
                                  const struct i2c_device_id *id)
{		
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
	        pr_err("i2c_check_functionality failed\n");
	        return -EIO;
	}
	
	chip_client = client;	
	
	return 0;
}

static int __devexit	bq2419x_i2c_remove(struct i2c_client *client)
{
        struct pt_bq2419x_chg_chip *chip;

	chip = i2c_get_clientdata(client);

	chip->bq2419x_client = NULL;
	
        return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id bq2419x_i2c_dt_match[] = {
	{ .compatible = "bq2419x,bq2419x-i2c" },
	{ },
};
#else
#define bq2419x_i2c_dt_match NULL
#endif


static const struct i2c_device_id bq2419x_id[] = {
        {"bq2419x-i2c", 0},
        {},
};

static struct i2c_driver bq2419x_i2c_driver = {
        .driver = {
                   .name = "bq2419x-i2c",
		     .owner	= THIS_MODULE,
	   	     .of_match_table = bq2419x_i2c_dt_match,
        },
        .probe = bq2419x_i2c_probe,
        .remove = __devexit_p(bq2419x_i2c_remove),
        .id_table = bq2419x_id,
};

static int __devinit
pt_bq2419x_charger_probe(struct spmi_device *spmi)
{
	int rc = 0;
	u8 subtype;
#if (CONFIG_BOARD_VER < CONFIG_TP10)	
	u8 sys_stat=0;
#endif
	struct pt_bq2419x_chg_chip *chip;
	struct resource *resource;
	struct spmi_resource *spmi_resource;
	
	printk("++ %s enter\n", __func__);

	chip = kzalloc(sizeof(struct pt_bq2419x_chg_chip),
					GFP_KERNEL);
	if (!chip) {
		pr_err("Cannot allocate pt_bq2419x_chg_chip\n");
		return -ENOMEM;
	}

	chip->current_max = IINLIM_MIN;	
	chip->dev = &(spmi->dev);
	chip->spmi = spmi;

	chip->host_mode = POWER_SUPPLY_SCOPE_DEVICE;
	chip->cable_present = false;
	chip->batt_soc = DEFAULT_BATT_SOC;
	chip->batt_uvolt = DEFAULT_BATT_VOL;
	chip->batt_temp = DEFAULT_BATT_TEMP;
	chip->therm_type = BATT_THERM_NORMAL;
	chip->fb_stat = -1;
	chip->aux_present = false;
	
	chip->usb_psy = power_supply_get_by_name("usb");
	if (!chip->usb_psy) {
		pr_err("usb supply not found deferring probe\n");
		rc = -EPROBE_DEFER;
		goto fail_chg_enable;
	}

	spmi_for_each_container_dev(spmi_resource, spmi) {
		if (!spmi_resource) {
			pr_err("qpnp_chg: spmi resource absent\n");
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		rc = qpnp_chg_read(chip, &subtype,
				resource->start + REG_OFFSET_PERP_SUBTYPE, 1);
		if (rc) {
			pr_err("Peripheral subtype read failed rc=%d\n", rc);
			goto fail_chg_enable;
		}

		if (subtype == SMBB_BAT_IF_SUBTYPE ||
			subtype == SMBBP_BAT_IF_SUBTYPE ||
			subtype == SMBCL_BAT_IF_SUBTYPE) {
			chip->vadc_dev = qpnp_get_vadc(chip->dev, "chg");
			if (IS_ERR(chip->vadc_dev)) {
				rc = PTR_ERR(chip->vadc_dev);
				if (rc != -EPROBE_DEFER)
					pr_err("vadc property missing\n");
				goto fail_chg_enable;
			}
		}
	}
	
	spmi_for_each_container_dev(spmi_resource, spmi) {
		if (!spmi_resource) {
			pr_err("qpnp_chg: spmi resource absent\n");
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			rc = -ENXIO;
			goto fail_chg_enable;
		}

		rc = qpnp_chg_read(chip, &subtype,
				resource->start + REG_OFFSET_PERP_SUBTYPE, 1);
		if (rc) {
			pr_err("Peripheral subtype read failed rc=%d\n", rc);
			goto fail_chg_enable;
		}

		switch (subtype) {
			case SMBB_CHGR_SUBTYPE:
			case SMBBP_CHGR_SUBTYPE:
			case SMBCL_CHGR_SUBTYPE:
				chip->chgr_base = resource->start;
				break;

			case SMBB_BUCK_SUBTYPE:
			case SMBBP_BUCK_SUBTYPE:
			case SMBCL_BUCK_SUBTYPE:
				chip->buck_base = resource->start;
				break;

			case SMBB_BAT_IF_SUBTYPE:
			case SMBBP_BAT_IF_SUBTYPE:
			case SMBCL_BAT_IF_SUBTYPE:
				chip->bat_if_base = resource->start;
				rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
				if (rc) {
					pr_err("Failed to init subtype 0x%x rc=%d\n",
							subtype, rc);
					goto fail_chg_enable;
				}
				break;

			case SMBB_USB_CHGPTH_SUBTYPE:
			case SMBBP_USB_CHGPTH_SUBTYPE:
			case SMBCL_USB_CHGPTH_SUBTYPE:
				chip->usb_chgpth_base = resource->start;
				break;

			case SMBB_DC_CHGPTH_SUBTYPE:
				chip->dc_chgpth_base = resource->start;
				break;

			case SMBB_BOOST_SUBTYPE:
			case SMBBP_BOOST_SUBTYPE:
				chip->boost_base = resource->start;			
				rc = qpnp_chg_hwinit(chip, subtype, spmi_resource);
				if (rc) {
					pr_err("Failed to init subtype 0x%x rc=%d\n",
							subtype, rc);
					goto fail_chg_enable;
				}
				break;

			case SMBB_MISC_SUBTYPE:
			case SMBBP_MISC_SUBTYPE:
			case SMBCL_MISC_SUBTYPE:
				chip->misc_base = resource->start;
				break;
				
			default:
				pr_err("Invalid peripheral subtype=0x%x\n", subtype);
				rc = -EINVAL;
				goto fail_chg_enable;
		}
	}

	mutex_init(&masked_write_lock);
	
	rc = i2c_add_driver(&bq2419x_i2c_driver);
	if (rc < 0) {
		pr_err("bq2419x I2C add driver failed %d", rc);
	}

	chip->bq2419x_client = chip_client;

	dev_set_drvdata(&spmi->dev, chip);
	device_init_wakeup(&spmi->dev, 1);
	
	bq2419x_hw_init(chip);

	get_smem_data(chip);
	
	chip->user_iinlim = chip->smem_data->auto_power_on_soc_check;
	if(chip->user_iinlim < bq2419x_iinlim_val[IINLIM_100MA] || chip->user_iinlim > bq2419x_iinlim_val[IINLIM_3000MA])
		chip->user_iinlim = bq2419x_iinlim_val[IINLIM_2000MA];
	
	chip->auto_pw_on = auto_pw_on_get(chip);
	chip->dummy_batt_present = is_dummy_batt_present(chip);
	chip->chg_f_dis = chip->dummy_batt_present;
	printk("user iinlim : %d, auto pw on : %d, dummy batt : %d\n", chip->user_iinlim, chip->auto_pw_on, chip->dummy_batt_present);
	
	rc = qpnp_chg_request_irqs(chip);
	if (rc) {
		pr_err("failed to request interrupts %d\n", rc);
		goto fail_chg_enable;
	}

	if(chip->smem_data->power_on_mode != OFFLINE_CHARGING) {
		chip->fb_notif.notifier_call = fb_notifier_callback;
		rc = fb_register_client(&chip->fb_notif);
		if (rc)
			pr_err("Unable to register fb_notifier: %d\n", rc);
	}
	
	wake_lock_init(&chip->chg_wake_lock, WAKE_LOCK_SUSPEND, "bq2419x_chg");
	wake_lock_init(&chip->heartbeat_wake_lock, WAKE_LOCK_SUSPEND, "bq2419x_heartbeat");

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	INIT_WORK(&chip->pg_irq_work, power_good_irq_work);
#endif

	INIT_WORK(&chip->irq_work, bq2419x_irq_work);
	INIT_DELAYED_WORK(&chip->update_heartbeat,
			update_heartbeat_work);
	
#if defined(FEATURE_WORKAROUND_NC_CHARGER)
	INIT_DELAYED_WORK(&chip->nc_chg_check_work,
			non_conforming_chg_check_work);
#endif

	chip->batt_psy.name = "battery";
	chip->batt_psy.type = POWER_SUPPLY_TYPE_BATTERY;
	chip->batt_psy.properties = bq_batt_power_props;
	chip->batt_psy.num_properties =
		ARRAY_SIZE(bq_batt_power_props);
	chip->batt_psy.get_property = bq2419x_batt_power_get_property;
	chip->batt_psy.set_property = bq2419x_batt_power_set_property;
	chip->batt_psy.property_is_writeable =
			bq2419x_batt_property_is_writeable;
	chip->batt_psy.external_power_changed =
			bq2419x_batt_external_power_changed;
	chip->batt_psy.supplied_to = pm_batt_supplied_to;
	chip->batt_psy.num_supplicants =
			ARRAY_SIZE(pm_batt_supplied_to);

	rc = power_supply_register(chip->dev, &chip->batt_psy);
	if (rc < 0) {
		pr_err("batt failed to register rc = %d\n", rc);
		goto fail_chg_enable;
	}

	/* Turn on appropriate workaround flags */
	rc = qpnp_chg_setup_flags(chip);
	if (rc < 0) {
		pr_err("failed to setup flags rc=%d\n", rc);
	}
	
	/* Disable QPNP charging */
	chip->charging_disabled = true;
	qpnp_chg_charge_en(chip, !chip->charging_disabled);
	qpnp_chg_force_run_on_batt(chip, chip->charging_disabled);

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	chip->pw_good_irq = gpio_to_irq(chip->pg_gpio);
	rc = request_irq(chip->pw_good_irq, power_good_irq_handler, 
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "bq2419x_pw_good", chip);
	if(rc) {
		pr_err("%s request_threaded_irq failed for %d rc =%d\n", __func__, chip->pw_good_irq, rc);
		goto unregister_batt;
	}
#endif

	chip->bq2419x_irq = gpio_to_irq(chip->int_gpio);
	rc = request_irq(chip->bq2419x_irq, bq2419x_isr, 
			IRQF_TRIGGER_LOW, "bq2419x_int", chip);
	if(rc) {
		pr_err("%s request_threaded_irq failed for %d rc =%d\n", __func__, chip->bq2419x_irq, rc);
		goto unregister_batt;
	}

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	chip->cable_present = (gpio_get_value(chip->pg_gpio)) ? 0 : 1;
#else
	sys_stat = bq2419x_get_sys_status(chip);
	chip->cable_present = (bool) bq2419x_power_status(sys_stat);
#endif
	if(chip->cable_present) {
		wake_lock(&chip->chg_wake_lock);
	}
	
	power_supply_set_present(chip->usb_psy, chip->cable_present);

	schedule_delayed_work(&chip->update_heartbeat,
					msecs_to_jiffies(0));

	create_testmenu_entries(chip);
	bq2419x_subdevices_register(chip);
	
	
	return 0;

unregister_batt:
	if (chip->bat_if_base)
		power_supply_unregister(&chip->batt_psy);	
fail_chg_enable:
	regulator_unregister(chip->boost_vreg.rdev);
	kfree(chip);
	dev_set_drvdata(&spmi->dev, NULL);
	return rc;
}

static int __devexit
pt_bq2419x_charger_remove(struct spmi_device *spmi)
{
	struct pt_bq2419x_chg_chip *chip = dev_get_drvdata(&spmi->dev);

	if (fb_unregister_client(&chip->fb_notif))
		pr_err("Error occurred while unregistering fb_notifier.\n");
	mutex_destroy(&masked_write_lock);
	regulator_unregister(chip->boost_vreg.rdev);
	dev_set_drvdata(&spmi->dev, NULL);
	kfree(chip);
	
	return 0;
}

static void 
pt_bq2419x_charger_shutdown(struct spmi_device *spmi)
{
	struct pt_bq2419x_chg_chip *chip = dev_get_drvdata(&spmi->dev);

	switch_usb_to_charge_mode(chip);
	auto_power_on_set(chip, chip->cable_present);
}

static int pt_bq2419x_chg_resume(struct device *dev)
{
	struct pt_bq2419x_chg_chip *chip = dev_get_drvdata(dev);
	int rc = 0;

#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	disable_irq_wake(chip->pw_good_irq);
#else
	disable_irq_wake(chip->bq2419x_irq);
#endif

	if (chip->bat_if_base) {
		rc = qpnp_chg_masked_write(chip,
			chip->bat_if_base + BAT_IF_VREF_BAT_THM_CTRL,
			VREF_BATT_THERM_FORCE_ON,
			VREF_BATT_THERM_FORCE_ON, 1);
		if (rc)
			pr_debug("failed to force on VREF_BAT_THM rc=%d\n", rc);
	}

	schedule_delayed_work(&chip->update_heartbeat,
        	round_jiffies_relative(msecs_to_jiffies(0)));
	
	return rc;
}

static int pt_bq2419x_chg_suspend(struct device *dev)
{
	struct pt_bq2419x_chg_chip *chip = dev_get_drvdata(dev);
	int rc = 0;

	cancel_delayed_work(&chip->update_heartbeat);
	
	if (chip->bat_if_base) {
		rc = qpnp_chg_masked_write(chip,
			chip->bat_if_base + BAT_IF_VREF_BAT_THM_CTRL,
			VREF_BATT_THERM_FORCE_ON,
			VREF_BAT_THM_ENABLED_FSM, 1);
		if (rc)
			pr_debug("failed to set FSM VREF_BAT_THM rc=%d\n", rc);
	}
		
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	enable_irq_wake(chip->pw_good_irq);
#else
	enable_irq_wake(chip->bq2419x_irq);
#endif
	
	return rc;
}


static const struct dev_pm_ops pt_bq2419x_chg_pm_ops = {
	.resume		= pt_bq2419x_chg_resume,
	.suspend	= pt_bq2419x_chg_suspend,
};

static struct spmi_driver pt_bq2419x_charger_driver = {
	.probe		= pt_bq2419x_charger_probe,
	.remove		= __devexit_p(pt_bq2419x_charger_remove),
	.shutdown 	= pt_bq2419x_charger_shutdown,
	.driver		= {
		.name		= PT_BQ2419X_CHARGER_DEV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= pt_bq2419x_charger_match_table,
		.pm		= &pt_bq2419x_chg_pm_ops,
	},
};

int __init
pt_bq2419x_chg_init(void)
{
	return spmi_driver_register(&pt_bq2419x_charger_driver);
}
module_init(pt_bq2419x_chg_init);

static void __exit
pt_bq2419x_chg_exit(void)
{
	spmi_driver_unregister(&pt_bq2419x_charger_driver);
}
module_exit(pt_bq2419x_chg_exit);


MODULE_DESCRIPTION("Pantech BQ2419X charger driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" PT_BQ2419X_CHARGER_DEV_NAME);

