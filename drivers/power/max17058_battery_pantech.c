// sayuss : start of header for i2c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <mach/gpio.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
// sayuss : end of header for i2c

#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/power_supply.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <linux/mfd/pm8xxx/ccadc.h>

#include <linux/bitops.h>
#include <linux/mutex.h>
#include <mach/msm_smsm.h>
#include <mach/gpiomux.h>

//sayuss check test codes
#include <linux/power/max17058_battery_pantech.h>
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
//youhw_bms
#include <linux/power_supply.h>
#endif
//MKS. charging disable when power off
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347)
#include <linux/power/smb347_charger_pantech.h>
#endif
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB349)
#include <linux/power/smb349_charger_pantech.h>
#endif

/* -------------------------------------------------------------------- */
/* max17058 i2c slave address */
/* -------------------------------------------------------------------- */
#define MAX17058_I2C_ADDR			0x36

//#define PANTECH_MAX17058_GPIO_CONTROL

/* sayuss PMIC_MAX17058 : this file added */

/* -------------------------------------------------------------------- */
/* max17058 buf max size */
/* -------------------------------------------------------------------- */
#define MAX17058_BUF_MAX			32

//#define MAX7058_DEBUG  // sayuss for debug

#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L) 
#define OEM_MAX17058_STD_FULL		(960)
#define OEM_MAX17058_STD_EMPTY	(70)
#define OEM_MAX17058_EXT_FULL		(960)
#define OEM_MAX17058_EXT_EMPTY	(70)
#elif defined(CONFIG_MACH_MSM8974_EF59S) ||  defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L)
#define OEM_MAX17058_STD_FULL		(983)
#define OEM_MAX17058_STD_EMPTY	(69)
#define OEM_MAX17058_EXT_FULL		(983)
#define OEM_MAX17058_EXT_EMPTY	(69)
#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
#define OEM_MAX17058_STD_FULL		(960)
#define OEM_MAX17058_STD_EMPTY	(42)
#define OEM_MAX17058_EXT_FULL		(960)
#define OEM_MAX17058_EXT_EMPTY	(42)
#elif defined(CONFIG_MACH_MSM8974_EF65S)
#define OEM_MAX17058_STD_FULL		(972)
#define OEM_MAX17058_STD_EMPTY	(32)
#define OEM_MAX17058_EXT_FULL		(972)
#define OEM_MAX17058_EXT_EMPTY	(32)
#else
#define OEM_MAX17058_STD_FULL		(983)
#define OEM_MAX17058_STD_EMPTY	(69)
#define OEM_MAX17058_EXT_FULL		(983)
#define OEM_MAX17058_EXT_EMPTY	(69)
#endif

#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L) 
#define OEM_BATTERY_STD_RCOMP		0x63
#define OEM_BATTERY_EXT_RCOMP		0x63 /*EXT not battery*/
#elif defined(CONFIG_MACH_MSM8974_EF59S) ||  defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L)
#define OEM_BATTERY_STD_RCOMP		0x6A
#define OEM_BATTERY_EXT_RCOMP		0x6A /*EXT not battery*/
#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
#define OEM_BATTERY_STD_RCOMP		0x51
#define OEM_BATTERY_EXT_RCOMP		0x51 /*EXT not battery*/
#elif defined(CONFIG_MACH_MSM8974_EF65S)
#define OEM_BATTERY_STD_RCOMP		0x5E
#define OEM_BATTERY_EXT_RCOMP		0x5E /*EXT not battery*/
#else
#define OEM_BATTERY_STD_RCOMP		0x6A
#define OEM_BATTERY_EXT_RCOMP		0x6A /*EXT not battery*/
#endif

#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L) 
#define OEM_BATTERY_STD_RCOMP_HOT(temp) ((temp - 20) * 65 / 100)
#define OEM_BATTERY_STD_RCOMP_COLD(temp) ((temp - 20) * 83 / 10)
#define OEM_BATTERY_EXT_RCOMP_HOT(temp) ((temp - 20) * 65 / 100)
#define OEM_BATTERY_EXT_RCOMP_COLD(temp) ((temp - 20) * 83 / 10)
#elif defined(CONFIG_MACH_MSM8974_EF59S) ||  defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L)
#define OEM_BATTERY_STD_RCOMP_HOT(temp) ((temp - 20) * 20 / 100)
#define OEM_BATTERY_STD_RCOMP_COLD(temp) ((temp - 20) * 745 / 100)
#define OEM_BATTERY_EXT_RCOMP_HOT(temp) ((temp - 20) * 20 / 100)
#define OEM_BATTERY_EXT_RCOMP_COLD(temp) ((temp - 20) * 745 / 100)
#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
#define OEM_BATTERY_STD_RCOMP_HOT(temp) ((temp - 20) * 15 / 100)
#define OEM_BATTERY_STD_RCOMP_COLD(temp) ((temp - 20) * 440 / 100)
#define OEM_BATTERY_EXT_RCOMP_HOT(temp) ((temp - 20) * 15 / 100)
#define OEM_BATTERY_EXT_RCOMP_COLD(temp) ((temp - 20) * 440 / 100)
#elif defined(CONFIG_MACH_MSM8974_EF65S)
#define OEM_BATTERY_STD_RCOMP_HOT(temp) ((temp - 20) * 675 / 1000)
#define OEM_BATTERY_STD_RCOMP_COLD(temp) ((temp - 20) * 5825 / 1000)
#define OEM_BATTERY_EXT_RCOMP_HOT(temp) ((temp - 20) * 675 / 1000)
#define OEM_BATTERY_EXT_RCOMP_COLD(temp) ((temp - 20) * 5825 / 1000)
#else
#define OEM_BATTERY_STD_RCOMP_HOT(temp) ((temp - 20) * 65 / 100)
#define OEM_BATTERY_STD_RCOMP_COLD(temp) ((temp - 20) * 83 / 10)
#define OEM_BATTERY_EXT_RCOMP_HOT(temp) ((temp - 20) * 65 / 100)
#define OEM_BATTERY_EXT_RCOMP_COLD(temp) ((temp - 20) * 83 / 10)
#endif
#define OEM_SOC(msb, lsb) ((msb * 500) + (lsb * 500 / 256))
//#define OEM_SOC(msb, lsb) ((msb * 1000) + (lsb * 1000 / 256))
#define OEM_STD_ADJUSED_SOC(oem_soc) (((oem_soc - (OEM_MAX17058_STD_EMPTY * 100)) * 100) / ((OEM_MAX17058_STD_FULL * 100) - (OEM_MAX17058_STD_EMPTY * 100)))
#define OEM_EXT_ADJUSED_SOC(oem_soc) (((oem_soc - (OEM_MAX17058_EXT_EMPTY * 100)) * 100) / ((OEM_MAX17058_EXT_FULL * 100) - (OEM_MAX17058_EXT_EMPTY * 100)))


/* -------------------------------------------------------------------- */
/* max17058 Type Definition */
/* -------------------------------------------------------------------- */
struct max17058_data {
	struct i2c_client   *client;
	struct delayed_work	rcomp_work;
#if defined(CONFIG_PANTECH_PMIC_EOC) || defined(CONFIG_PANTECH_PMIC_BMS_TEST)
	int FG_SOC;
#endif /* CONFIG_PANTECH_PMIC_EOC || CONFIG_PANTECH_PMIC_BMS_TEST */
};

typedef enum {
	MAX17058_REG_VCELL_0x2      = 0x02,
	MAX17058_REG_SOC_0x4        = 0x04,
	MAX17058_REG_HIBERNATE_0x0A = 0x0A,
	MAX17058_REG_CONFIG_0x0C    = 0x0C,
	MAX17058_REG_OCV_0x0E       = 0x0E,
	MAX17058_VRESET_0x18        = 0x18,
	MAX17058_REG_UNLOCK_0x3E    = 0x3E,
	MAX17058_REG_TABLE_0x40     = 0x40,
	MAX17058_REG_TABLE_0x50     = 0x50,
	MAX17058_REG_TABLE_0x60     = 0x60,
	MAX17058_REG_TABLE_0x70     = 0x70,
	MAX17058_REG_POR_0xFE       = 0xFE,
} max17058_reg_type;

static struct i2c_client *max17058_client;
static struct max17058_data *the_max17058;
static struct i2c_driver max17058_i2c_driver;
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
//youhw_bms
struct qpnp_bms_chip{
       struct device			*dev;
	struct power_supply		bms_psy;
	int				rbatt_capacitive_mohm;
	int				rbatt_mohm;
};

static char *qpnp_bms_supplicants[] = {
	"battery"
};

static enum power_supply_property msm_bms_power_props[] = {
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_RESISTANCE,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
};
int max17058_probe_ok = 0;
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
static int MAX17058_GET_SOC_FOR_OTG = 100;
#endif
#if defined(CONFIG_PANTECH_PMIC_LED)
static int MAX17058_GET_SOC_FOR_LED = 100;
#endif
extern int is_fatctory_dummy_connect(void);

#ifdef CONFIG_PANTECH_USB_SMB_OTG_DISABLE_LOW_BATTERY
extern int get_pantech_otg_enabled(void);
extern void pantech_otg_uvlo_notify(int on);
#endif
static int report_state_of_charge(void)//(struct qpnp_chg_chip *chip)
{
#ifdef CONFIG_PANTECH_USB_SMB_OTG_DISABLE_LOW_BATTERY
	int ret;
#endif

	if(is_fatctory_dummy_connect()){
		pr_debug("djjeon  factory dummy\n");
		return 50;
	}
#ifdef CONFIG_PANTECH_USB_SMB_OTG_DISABLE_LOW_BATTERY
	else if(get_pantech_otg_enabled()) {
		ret = max17058_get_soc();
		if(ret < 10)
			pantech_otg_uvlo_notify(1);
		return ret;
	}
#endif
	else
		return max17058_get_soc();
}
/* Returns capacity as a SoC percentage between 0 and 100 */
static int get_prop_bms_capacity(struct qpnp_bms_chip *chip)
{
	return report_state_of_charge();
}
#if 0
/* Returns estimated battery resistance */
static int get_prop_bms_batt_resistance(struct qpnp_chg_chip *chip)
{
	return 4000;
}
#endif
/* Returns instantaneous current in uA */
static int get_prop_bms_current_now(struct qpnp_bms_chip *chip)
{
	//int rc, result_ua;
#if 0
	rc = get_battery_current(chip, &result_ua);
	if (rc) {
		pr_err("failed to get current: %d\n", rc);
		return rc;
	}
#endif
	return 50;//result_ua;
}
#if 0
/* Returns coulomb counter in uAh */
static int get_prop_bms_charge_counter(struct qpnp_chg_chip *chip)
{

}
#endif
/* Returns full charge design in uAh */
static int get_prop_bms_charge_full_design(struct qpnp_bms_chip *chip)
{
   
       return 4000;
}
static void battery_status_check(struct qpnp_bms_chip *chip)
{

}

static void battery_insertion_check(struct qpnp_bms_chip *chip)
{

}

static void qpnp_bms_external_power_changed(struct power_supply *psy)
{
	struct qpnp_bms_chip *chip = container_of(psy, struct qpnp_bms_chip,
								bms_psy);

	battery_insertion_check(chip);
	battery_status_check(chip);
}

static int qpnp_bms_power_get_property(struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val)
{
    struct qpnp_bms_chip *chip = container_of(psy, struct qpnp_bms_chip,
                                              bms_psy);

    if(max17058_probe_ok == 0) {
        pr_debug("%s: Not yet complete probing\n", __func__);
        return 0;
    }
    switch (psp) {
    case POWER_SUPPLY_PROP_CAPACITY:
        val->intval = get_prop_bms_capacity(chip);
        break;
    case POWER_SUPPLY_PROP_CURRENT_NOW:
        val->intval = get_prop_bms_current_now(chip);
        break;
#if 0
    case POWER_SUPPLY_PROP_RESISTANCE:
        val->intval = get_prop_bms_batt_resistance(chip);
        break;
    case POWER_SUPPLY_PROP_CHARGE_COUNTER:
        val->intval = get_prop_bms_charge_counter(chip);
        break;
#endif
    case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
        val->intval = get_prop_bms_charge_full_design(chip);
        break;
    default:
        return -EINVAL;
    }
    return 0;
}
#endif //CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058

#if defined(CONFIG_PANTECH_PMIC_SHARED_DATA)
static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
static void smem_vendor1_init(void)
{
    smem_id_vendor1_ptr =  (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
                                                                      sizeof(oem_pm_smem_vendor1_data_type));
}

static int get_oem_battery_id(void)
{
    if(smem_id_vendor1_ptr == NULL) 
        return 1;

    return smem_id_vendor1_ptr->battery_id;
}
#endif

/* -------------------------------------------------------------------- */
/* max17058 I2C Read Write block */
/* -------------------------------------------------------------------- */
#if defined(MAX7058_DEBUG)
static s32 max17058_i2c_write(u8 reg, u16 len, u8 *val)
#else
static s32 max17058_i2c_write(u8 reg, u8 *val, u16 len)
#endif
{
#if defined(PANTECH_MAX17058_GPIO_CONTROL)
	s32 ret = 0;

	ret = i2c_smbus_write_i2c_block_data(the_max17058->client, reg,
				len, val);
	if (ret < 0)
		pr_err("max17058 failed to write register \n");

	return ret;
#else
	s32 ret = 0;
	u8 buf[MAX17058_BUF_MAX];

	struct i2c_msg msg = {
			.addr = MAX17058_I2C_ADDR, .flags = 0, .buf = buf, .len = len + 1
	};

	memset(buf, 0x0, MAX17058_BUF_MAX);

	buf[0] = reg;
	memcpy((void*)&buf[1], (void*)val, len);
	
	if (!max17058_client) {
		pr_err("max17058 no client \n");
		return -1;
	}

	if (i2c_transfer(max17058_client->adapter, &msg, 1) < 0) {
		pr_debug("max17058 failed return -EIO \n");
		return -EIO;
	}

	return ret;
#endif
}

static int max17058_i2c_read(u8 reg, u8 *val, u16 size)
{
#if defined(PANTECH_MAX17058_GPIO_CONTROL)
	s32 ret =0;

	ret = i2c_smbus_read_i2c_block_data(the_max17058->client, reg, size, val);
	if (ret < 0) {
		pr_err("max17058 failed to read register \n");
	}

	return 0;	
#else
	struct i2c_msg msg[2] = {
		{ .addr = MAX17058_I2C_ADDR, .flags = 0,        .len = 1,    .buf = &reg },
		{ .addr = MAX17058_I2C_ADDR, .flags = I2C_M_RD, .len = size, .buf = val  },
	};
	
	if (!max17058_client){
		pr_err("max17058 no client\n");
		return -1;
	}

	if (i2c_transfer(max17058_client->adapter, msg, 2) < 0){
		pr_debug("max17058 failed return -EIO \n");
		return -EIO;
	}
	
	return 0;
#endif
}

#if defined(CONFIG_PANTECH_PMIC_BMS_TEST) || defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
/* -------------------------------------------------------------------- */
/* Max17058 interface block */
/* -------------------------------------------------------------------- */
int max17058_get_voltage(void)
{
	s32 ret;
	u8 val[2] = {0};
	int uvolt = 0;

    if (get_oem_battery_id() !=0)
	{
		ret = max17058_i2c_read(MAX17058_REG_VCELL_0x2, val, 2);

		if (ret < 0)
			return uvolt;

		uvolt = (val[0] << 4) + ((val[1] & 0xF0) >> 4);
		uvolt = (uvolt * 1250);
	}

	pr_debug("max17058 uvolt=%d\n",uvolt);
	return uvolt;
}
EXPORT_SYMBOL(max17058_get_voltage);
#endif

#ifdef CONFIG_PANTECH_PMIC_CHARGER_SMB347
extern struct _smb347_external smb_data_ext;
#else
extern struct _smb349_external smb_data_ext;
#endif

#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
int max17058_get_soc_for_otg(void)
{
	return MAX17058_GET_SOC_FOR_OTG;
}
EXPORT_SYMBOL(max17058_get_soc_for_otg);
#endif

#if defined(CONFIG_PANTECH_PMIC_LED)
int max17058_get_soc_for_led(void)
{
	return MAX17058_GET_SOC_FOR_LED;
}
EXPORT_SYMBOL_GPL(max17058_get_soc_for_led);
#endif

int max17058_get_soc(void)
{
    static int last_soc = 0;
    s32 ret;
    u8 val[2] = {0};
    int soc, oem_soc, adjsoc=0;

    if(smb_data_ext.is_factory_dummy) {
        pr_debug("%s: factory dummy\n", __func__);
        return 50;
    }

    if (get_oem_battery_id() !=0) {
        ret = max17058_i2c_read(MAX17058_REG_SOC_0x4, val, 2);

        if (ret < 0) {
            pr_debug("%s: last_soc=%d\n", __func__, last_soc);	//20130814 djjeon, when sleep awake, 50% display problem
            return last_soc;
        }

        soc = ((val[0] * 256) + val[1]) / 512;
        oem_soc = OEM_SOC(val[0], val[1]);
#if defined(CONFIG_PANTECH_PMIC_EOC)
        the_max17058->FG_SOC = soc;
#endif 

        if (get_oem_battery_id() == 1)
            adjsoc = OEM_STD_ADJUSED_SOC(oem_soc);
        else if (get_oem_battery_id() == 2)
            adjsoc = OEM_EXT_ADJUSED_SOC(oem_soc);
        else
            adjsoc = OEM_STD_ADJUSED_SOC(oem_soc);

        pr_debug("%s: max17058 soc=%d, oem_soc=%d, adjsoc=%d\n", __func__, soc, oem_soc, adjsoc);

        if(0 > adjsoc)
            adjsoc = 0;
        else if( 100 < adjsoc)
            adjsoc = 100;

        last_soc = adjsoc;
    }
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
    MAX17058_GET_SOC_FOR_OTG = adjsoc;
#endif
#if defined(CONFIG_PANTECH_PMIC_LED)
    MAX17058_GET_SOC_FOR_LED = adjsoc;
#endif
    return adjsoc;
}
EXPORT_SYMBOL(max17058_get_soc);

#if defined(CONFIG_PANTECH_PMIC_EOC) || defined(CONFIG_PANTECH_PMIC_BMS_TEST)
int max17058_FG_SOC(void)
{
	return the_max17058->FG_SOC;
}
EXPORT_SYMBOL(max17058_FG_SOC);
#endif 

static int update_rcomp(u8 *val)
{
    s32 ret = 0 ;

    if (get_oem_battery_id() !=0){
#if defined(MAX7058_DEBUG) /* sayuss : fuelgauge model data write*/
        ret = max17058_i2c_write(MAX17058_REG_CONFIG_0x0C, 2, val);
#else
        ret = max17058_i2c_write(MAX17058_REG_CONFIG_0x0C, val, 2);
#endif // sayuss : fuelgauge model data write
    }

    if(ret < 0) 
        pr_err("update_rcomp fail %d\n", ret);	

    return ret;
}

void max17058_update_rcomp(int temp)
{
	int start_rcomp;
	int new_rcomp = 0; // sayuss error initialize
	u8 val[2] = {0};
	s32 ret;

	if (get_oem_battery_id() == 1)
		start_rcomp = OEM_BATTERY_STD_RCOMP;
	else if (get_oem_battery_id() == 2)
		start_rcomp = OEM_BATTERY_EXT_RCOMP;
	else
		start_rcomp = OEM_BATTERY_STD_RCOMP;

	if (!the_max17058) {
		/*
		schedule_delayed_work(&the_max17058->rcomp_work,
	        round_jiffies_relative(msecs_to_jiffies
	        (30000)));
	        */
		return;
	}
	temp = temp / 10;

    pr_debug("%s: start_rcomp = %d, temp = %d, \n", __func__, start_rcomp, temp);

	ret = max17058_i2c_read(0x0C, val, 2);
	if (ret < 0){
		printk("[MAX17058] fuelgauge: 0Ch read error\n");
		return;
	}

	if (get_oem_battery_id() == 1) {
		if (temp > 20)
			new_rcomp = start_rcomp - OEM_BATTERY_STD_RCOMP_HOT(temp);
		else if (temp < 20)
			new_rcomp = start_rcomp - OEM_BATTERY_STD_RCOMP_COLD(temp);
		else
			new_rcomp = start_rcomp;		
	} else if (get_oem_battery_id() == 2) {
		if (temp > 20)
			new_rcomp = start_rcomp - OEM_BATTERY_EXT_RCOMP_HOT(temp);
		else if (temp < 20)
			new_rcomp = start_rcomp - OEM_BATTERY_EXT_RCOMP_COLD(temp);
		else
			new_rcomp = start_rcomp;
	} else {
		if (temp > 20)
			new_rcomp = start_rcomp - OEM_BATTERY_STD_RCOMP_HOT(temp);
		else if (temp < 20)
			new_rcomp = start_rcomp - OEM_BATTERY_STD_RCOMP_COLD(temp);
		else
			new_rcomp = start_rcomp;
	}


	if (new_rcomp > 255)
		new_rcomp = 255;
	else if (new_rcomp < 0)
		new_rcomp = 0;

	pr_debug("%s: new_rcomp = %d\n", __func__, new_rcomp);

	val[0] = (u8)new_rcomp;

	if (update_rcomp(val) < 0)
		pr_err("update_rcomp failed\n");

#if 0
	schedule_delayed_work(&the_max17058->rcomp_work,
        round_jiffies_relative(msecs_to_jiffies
        (30000)));
#endif
}

static int __devinit max17058_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
    struct max17058_data *max17058;
    struct i2c_adapter *adapter;
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
    struct qpnp_bms_chip *chip;
    int rc;   

    pr_info("%s: start of %s\n", __func__, __func__);	

    chip = kzalloc(sizeof *chip, GFP_KERNEL);
    if (chip == NULL) {
        pr_err("%s: Failed to allocate memory\n", __func__);
        return -ENOMEM;
    }

    /* setup & register the battery power supply */
    chip->bms_psy.name = "bms";
    chip->bms_psy.type = POWER_SUPPLY_TYPE_BMS;
    chip->bms_psy.properties = msm_bms_power_props;
    chip->bms_psy.num_properties = ARRAY_SIZE(msm_bms_power_props);
    chip->bms_psy.get_property = qpnp_bms_power_get_property;
    chip->bms_psy.external_power_changed =
        qpnp_bms_external_power_changed;
    chip->bms_psy.supplied_to = qpnp_bms_supplicants;
    chip->bms_psy.num_supplicants = ARRAY_SIZE(qpnp_bms_supplicants);

    rc = power_supply_register(chip->dev, &chip->bms_psy);

    if (rc < 0) {
        pr_err("power_supply_register bms failed rc = %d\n", rc);
        goto unregister_dc;
    }	
#endif //CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058
#if defined(PANTECH_MAX17058_GPIO_CONTROL)
    pr_info("%s: start of %s\n", __func__, __func__);	

    max17058 = kzalloc(sizeof(struct max17058_data), GFP_KERNEL);
    if (!max17058) {
        pr_err("%s: Failed to allocate memory\n", __func__);
        return -ENOMEM;
    }

    //INIT_DELAYED_WORK(&max17058->rcomp_work, rcomp_handler);

    max17058->client = client;
    i2c_set_clientdata(client, max17058);
    max17058_client = client;

    the_max17058 = max17058;

    i2c_set_clientdata(client, max17058);

    //schedule_delayed_work(&the_max17058->rcomp_work, round_jiffies_relative(msecs_to_jiffies(30000)));

    // Alert Intterupt Add.
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)		
    max17058_probe_ok =1;
#endif	   

    pr_info("%s: end of %s\n", __func__, __func__);	
    return 0;	
#else
    adapter = to_i2c_adapter(client->dev.parent);	

    max17058 = kzalloc(sizeof(struct max17058_data), GFP_KERNEL);

    if (!max17058) {
        pr_err("max17058 failed to alloc memory\n");
        return -ENOMEM;
    }

    if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
        return -EIO;

    //INIT_DELAYED_WORK(&max17058->rcomp_work, rcomp_handler);

    max17058->client = client;
    max17058_client = client;

    the_max17058 = max17058;

    i2c_set_clientdata(client, max17058);

    //schedule_delayed_work(&the_max17058->rcomp_work,round_jiffies_relative(msecs_to_jiffies(30000)));

    // Alert Intterupt Add.
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)	
    max17058_probe_ok =1;
#endif	   

    pr_info("%s: end of %s\n", __func__, __func__);	
    return 0;
#endif
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
    //youhw_bms
unregister_dc:	
    power_supply_unregister(&chip->bms_psy);
    return rc;
#endif
}

static int __devexit max17058_remove(struct i2c_client *client)
{
	struct max17058_data *max17058 = i2c_get_clientdata(client);
	kfree(max17058);
	
	return 0;
}

static int max17058_suspend(struct device *dev)
{
	return 0;
}

static int max17058_resume(struct device *dev)
{
	return 0;
}

static void max17058_shutdown(struct i2c_client *client)
{
#if defined(CONFIG_PANTECH_PMIC_CHARGER_SMB347) //LS1. MKS. disabling the OTG devices when phone power off. (only for SMB347)
	smb349_write_reg(0x30,0x81); //charging disable
	smb349_write_reg(0x31,0x00); //mode change 
#endif
}

static const struct dev_pm_ops max17058_pm_ops = {
	.suspend	= max17058_suspend,
	.resume		= max17058_resume,
};

/* sayuss PMIC_MAX17058 */
#ifdef CONFIG_OF // Open firmware must be defined for dts useage
static struct of_device_id max17058_i2c_table[] = {
	{ .compatible = "maxim,max17058-i2c",}, // Compatible node must match dts
	{ },
};
#else
#define max17058_i2c_table NULL
#endif

// I2C slave id supported by driver
static const struct i2c_device_id max17058_id[]={
	{ "max17058_i2c", 0},
	{ }
};

static struct i2c_driver max17058_i2c_driver={
	.driver = {
		.name = "max17058_i2c",
		.pm	= &max17058_pm_ops,
		.owner = THIS_MODULE ,
		.of_match_table = max17058_i2c_table ,
	},
	.probe = max17058_probe ,
	.remove		= __devexit_p(max17058_remove),
	.shutdown = max17058_shutdown,
	.id_table = max17058_id ,
};
// Easy wrapper to do driver init

static int __init max17058_init(void)
{
    int rc;

    pr_info("%s: start of %s\n", __func__, __func__);	
    smem_vendor1_init();
    rc = i2c_add_driver(&max17058_i2c_driver);
    if(rc) {
        printk(KERN_ERR "max17058_battery driver add failed.\n");
    }
    pr_info("%s: end of %s\n", __func__, __func__);	

    return rc;
}

static void __exit max17058_exit(void)
{
	i2c_del_driver(&max17058_i2c_driver);
}

module_init(max17058_init);
module_exit(max17058_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MAX17058 battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" "max17058_battery");
