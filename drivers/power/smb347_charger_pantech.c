 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regulator/consumer.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/kernel.h>
#include <linux/power/smb347_charger_pantech.h>
#include <linux/workqueue.h>  
#include <linux/qpnp/qpnp-adc.h>
#include <linux/delay.h>
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
#include <linux/mutex.h>
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
#include <linux/types.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/spmi.h>
#include <linux/rtc.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/mfd/pm8xxx/batterydata-lib.h>
/* CONFIG_PANTECH_PMIC_CHARGER_SMB347 */
#include <mach/gpio.h>
#include <mach/gpiomux.h>

#if defined(CONFIG_PANTECH_PMIC_AUTO_PWR_ON)
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <mach/msm_smsm.h>
#endif /* CONFIG_PANTECH_PMIC_AUTO_PWR_ON */
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
#include <linux/msm_tsens.h>
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR */

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

/*CONFIG_PANTECH_PMIC_CHARGER_SMB347*/
enum
{
	GPIO_LOW_VALUE = 0,
	GPIO_HIGH_VALUE,
};
enum
{
	WR_COMMAND = 0x0,
	RD_COMMAND,
};

extern int cable_present_state(void);

/*CONFIG_PANTECH_PMIC_CHARGER_SMB347*/
#define SMB347_CHIP_ADDR		0x0c
#define SMB_SCL_PIN			84
#define SMB_SDA_PIN			83
#define SMB_CHG_SUSP            31
#define SMB_I2C_SDA_OUT	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE)
#define SMB_I2C_SDA_IN		gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE)

/* Register definitions */
#define CHG_CTRL_REG				0x04	


/* Command registers */
#define CMD_A						0x30
#define CMD_A_CHG_ENABLED			BIT(1)
#define CMD_A_SUSPEND_ENABLED		BIT(2)
#define CMD_A_ALLOW_WRITE			BIT(7)
#if defined(CONFIG_PANTECH_PMIC_EOC)
#define CHG_CTRL_TERM_ENABLE		BIT(6)
#endif

#define BATT_THERM_MAX 5
#define CHANGED 1
#define NOT_CHANGED 0
#define TEMP_HYSTERESIS_DEGC 30

static int charger_ready = false;

struct _smb347_external smb_data_ext = {
    .prev_state_mode = BATT_THERM_UNKNOWN,
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
    .t_sensor_mode = false,
    .t_prev_mode = false,
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR */
#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    .smb_chg_disabled = false,
#endif /* CONFIG_PANTECH_PMIC_CHARGING_DISABLE */
    .is_factory_dummy = 0,
    .is_CDP = 0, // not used. It's used on smb349 chip. dummy value.
};
EXPORT_SYMBOL(smb_data_ext);

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
#define POWER_IS_ON(pwr)    ((pwr) <= FB_BLANK_NORMAL)
struct _smb347_lcd {
    int state;
    struct notifier_block fb_noti;
};
static int smb349_fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data);
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

struct _smb347 {
    int prev_cable_type;
    int prev_cable_current;
    int prev_state_temp_mode;
    int qcom_cable_type;
    int therm_boundary_min[BATT_THERM_MAX];
    int therm_boundary_max[BATT_THERM_MAX];
    int therm_boundary_init[BATT_THERM_MAX];
	struct rt_mutex reg_lock;
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
    struct _smb347_lcd lcd_state;
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
    struct mutex otg_lock_mutex;
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
};

static struct _smb347 smb_data = {
    .prev_cable_type = PANTECH_CABLE_NONE,
    .prev_cable_current = 0,
    .prev_state_temp_mode = BATT_THERM_UNKNOWN,
    .qcom_cable_type = PANTECH_CABLE_NONE,
    .therm_boundary_min = {
        -2000, 
        -80, 
        20, 
        400, 
        550
    },
    .therm_boundary_max = {
        -50, 
        50, 
        430, 
        580, 
        2000
    },
    .therm_boundary_init = {
        -2000, 
        -80, 
        20, 
        430, 
        580
    },
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
    .lcd_state = {
        .state = 0,
        .fb_noti = {
            .notifier_call = smb349_fb_notifier_callback,
        },
    },
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */
};

/*CONFIG_PANTECH_PMIC_CHARGER_SMB347*/
void smb347_i2c_startcondition(void)
{
    gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
    gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
    udelay(5);	
    gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);
    udelay(4);
    gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
}

void smb347_i2c_stopcondition(void)
{
    gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);
    gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
    udelay(4);
    gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
    mdelay(5);
}

void smb347_i2c_ack(void)
{
    gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);	
    gpio_tlmm_config(GPIO_CFG(SMB_SCL_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_DISABLE);
    gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
    mdelay(4);
    gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
    mdelay(4);
}

void smb347_i2c_nack(void)
{
    gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);	
    gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
    udelay(4);
    gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
    udelay(4);
}

int smb347_i2c_sendbyte(unsigned char data)
{
    int i = 0;
    int timeout = 1;
    int ret = 0;

    gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    for(i=0;i<8;i++) {
        if (data & 0x80)
            gpio_direction_output(SMB_SDA_PIN, GPIO_HIGH_VALUE);
        else
            gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);	
        udelay(2);
        gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
        udelay(4);
        gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);
        udelay(5);
        data = data << 1;
    }
    gpio_direction_output(SMB_SDA_PIN, GPIO_LOW_VALUE);	
    gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    mdelay(4);	
    gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);	

    i = 0;
    while((((gpio_get_value(SMB_SDA_PIN)) ? 1 : 0) == 1) && timeout) {
        i++;
        udelay(1);		
        if (i >= 100)
        {
            timeout = 0;
            ret = 1;
        }
    }
    gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);

    return ret;
}

int smb347_i2c_readbyte(unsigned char *data)
{
    int i = 0;
    int ret = 1;
    unsigned char temp = 0;

    gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    for(i=0;i<8;i++) {
        temp <<= 1;
        udelay(5);
        gpio_direction_output(SMB_SCL_PIN, GPIO_HIGH_VALUE);
        udelay(4);
        temp |= ((((gpio_get_value(SMB_SDA_PIN)) ? 1 : 0)) & 0x01);
        gpio_direction_output(SMB_SCL_PIN, GPIO_LOW_VALUE);		

    }
    *data = temp;

    return ret;
}

int smb349_read_reg(u8 reg, unsigned char *val)
{
    int status = 0;

    rt_mutex_lock(&smb_data.reg_lock);
    *val = 0;
    smb347_i2c_startcondition();
    status = smb347_i2c_sendbyte(SMB347_CHIP_ADDR | WR_COMMAND);
    if (status == 0) {	
        status = smb347_i2c_sendbyte(reg);
        if (status == 0) {
            smb347_i2c_startcondition();
            status = smb347_i2c_sendbyte(SMB347_CHIP_ADDR | RD_COMMAND);
            if (status == 0)
                smb347_i2c_readbyte(val);
            smb347_i2c_nack();
        }
    }
    smb347_i2c_stopcondition();
    rt_mutex_unlock(&smb_data.reg_lock);

    return 0;
}

int smb349_write_reg(u8 reg, unsigned char val)
{
    int status = 0;

    rt_mutex_lock(&smb_data.reg_lock);
    smb347_i2c_startcondition();
    status = smb347_i2c_sendbyte((SMB347_CHIP_ADDR) | WR_COMMAND);
    if (status == 0) {	
        status = smb347_i2c_sendbyte(reg);
        if (status == 0)
            status = smb347_i2c_sendbyte (val);
    } else {
        smb347_i2c_stopcondition();
        pr_err("smb349 i2c write fail: can't write %02x to %02x: %d\n",
               val, reg, status);
        return status;
    }
    smb347_i2c_stopcondition();
    rt_mutex_unlock(&smb_data.reg_lock);

    return 0;
}

void smb349_otg_power(int on)
{
	pr_debug("%s: on=%d\n", __func__, on);
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
	mutex_lock(&smb_data.otg_lock_mutex);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
	if(on) {		 
		smb349_write_reg(0x30,0x91); // OTG: enable, STAT: disable
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5
		smb_data.prev_cable_type = PANTECH_CABLE_NONE;
	} else {
		smb349_write_reg(0x30,0xC0); // OTG: disable, STAT: enable, disable charging
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5	
	}
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
	mutex_unlock(&smb_data.otg_lock_mutex);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
}
EXPORT_SYMBOL_GPL(smb349_otg_power);

/*56S TP10 board apply*/
#ifdef CONFIG_PANTECH_PMIC_OTG 
void smb349_otg_power_current(int mode)
{
    printk("set otg index %d\n", mode);
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
    mutex_lock(&smb_data.otg_lock_mutex);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
    switch(mode){
    case 1:
        smb349_write_reg(0x0A,0xB7);  //250mA 
        break;	
    case 2:
        smb349_write_reg(0x0A,0xBB); //500mA
        break;	
    case 3:
        smb349_write_reg(0x0A,0xBF); //750mA
        break;	
    default:
        break;
    }
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
    mutex_unlock(&smb_data.otg_lock_mutex);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
}
EXPORT_SYMBOL_GPL(smb349_otg_power_current);
#endif 

void smb349_print_reg(u8 reg)
{
	u8 is_invalid_temp;	
	smb349_read_reg(reg, &is_invalid_temp);
	pr_debug("%s: 0x%x reg value = %.2X\n", __func__, reg, is_invalid_temp);
	return ;
}

void smb349_print_all_reg(void)
{
	u8 is_invalid_temp;	
    int loop_count = 0;
    u8 regs[] = { 0x30, 0x31, 0x00, 0x01, 0x02, 
        0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0E, 0x34, 
        0x3F };

    for(;loop_count<ARRAY_SIZE(regs);loop_count++) {
        smb349_read_reg(regs[loop_count], &is_invalid_temp);
        pr_info("%s: 0x%02x value = %.2X\n", __func__, regs[loop_count], is_invalid_temp);
    }
}
	
#if defined(CONFIG_PANTECH_PMIC_EOC)
void end_recharging_enable(bool enable)
{
	u8 term_satatus = 0;
	u8 is_invalid_temp;
	
	smb349_read_reg(CHG_CTRL_REG, &term_satatus);
	if (term_satatus < 0)
			return;
	if(enable) {
		term_satatus &= ~CHG_CTRL_TERM_ENABLE;
	} else {
		term_satatus |= CHG_CTRL_TERM_ENABLE;		
	}	
	term_satatus = smb349_write_reg(CHG_CTRL_REG, term_satatus);
	smb349_read_reg(0x04, &is_invalid_temp);
	pr_debug("%s: 0x04 value = %.2X\n", __func__, is_invalid_temp);
}
EXPORT_SYMBOL_GPL(end_recharging_enable);
#endif

void smb349_charging_enable(bool enable)
{
    u8 charging_status;
    u8 is_invalid_temp;	

    smb349_read_reg(CMD_A, &charging_status);
    if (charging_status < 0)
        return;

    if(enable) {
        charging_status |= CMD_A_CHG_ENABLED;
        charging_status = smb349_write_reg(CMD_A, charging_status);	
        smb349_read_reg(0x30, &is_invalid_temp);
        pr_err("0x30 value = %.2X\n", is_invalid_temp);
    } else {
        charging_status &= ~CMD_A_CHG_ENABLED;
        charging_status = smb349_write_reg(CMD_A, charging_status);
        smb349_read_reg(0x30, &is_invalid_temp);
        pr_err("0x30 value = %.2X\n", is_invalid_temp);
    }
}
EXPORT_SYMBOL_GPL(smb349_charging_enable);	

static void smb349_usb_mode(int current_ma)
{
    if (!get_is_offline_charging_mode()) {
        if(current_ma == 100) {
            smb349_write_reg(0x30,0x83); //charging enable
            smb349_write_reg(0x31,0x00); //usb 100 
        } else if(current_ma == 900) {
            smb349_write_reg(0x30,0x83); //charging enable
            smb349_write_reg(0x31,0x02); //usb 900
            smb349_write_reg(0x08, 0x38); //USB3.0
        } else {	
            smb349_write_reg(0x30,0x83); //charging enable
            smb349_write_reg(0x31,0x02); //usb 500
            smb349_write_reg(0x08, 0x18); //USB2.0		
        }
    } else {
        smb349_write_reg(0x30,0x83); //charging enable
        smb349_write_reg(0x31,0x02); //usb 500
        smb349_write_reg(0x08, 0x18); //USB2.0		
    }
}

#if defined(CONFIG_PANTECH_PMIC_AUTO_PWR_ON)
int get_auto_power_on_flag(void)
{
    static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;

    smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
                                                                     sizeof(oem_pm_smem_vendor1_data_type));

    if(smem_id_vendor1_ptr != NULL) {
        pr_debug("%s: auto_power_on_soc_check=%d\n", __func__, smem_id_vendor1_ptr->auto_power_on_soc_check);
        return smem_id_vendor1_ptr->auto_power_on_soc_check;
    }

    return 0;
}
#endif /* CONFIG_PANTECH_PMIC_AUTO_PWR_ON */

int get_is_offline_charging_mode(void)
{
    static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;

    smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1, 
                                                                     sizeof(oem_pm_smem_vendor1_data_type));

    if(smem_id_vendor1_ptr != NULL) {
        pr_debug("%s: power_on_mode=%d\n", __func__, smem_id_vendor1_ptr->power_on_mode);
        return !smem_id_vendor1_ptr->power_on_mode;
    }

    // default online boot mode
    return 1;
}

int is_temp_state_changed(int* state, int64_t temp)
{
	int index = *state;

	smb_data_ext.prev_state_mode = index;	

    pr_debug("%s: index=%d\n", __func__, index);

	if( BATT_THERM_UNKNOWN == index ) {
		index--;
		for( ; index > -1 ; index--) {
			if(temp > smb_data.therm_boundary_init[index])
				break;
		}
	} else if(temp < smb_data.therm_boundary_min[*state] ) {
		index--;
		for( ; index > -1 ; index--) {
			if(temp < smb_data.therm_boundary_max[index] && temp >= smb_data.therm_boundary_min[index])
				break;
		}
		
	} else if(temp >= smb_data.therm_boundary_max[*state]) {
		index++;
		for( ; index < BATT_THERM_MAX ; index++) {
			if(temp < smb_data.therm_boundary_max[index] && temp >= smb_data.therm_boundary_min[index])
				break;
		}
	} else {
		return NOT_CHANGED;
    }
	
	if( index >= 0 && index < BATT_THERM_MAX) {
		*state = index;
        pr_debug("%s: searched state=%dn", __func__, *state);
        return CHANGED;
	}

    pr_debug("%s: What happend to me!!", __func__);
    return NOT_CHANGED;
}
EXPORT_SYMBOL_GPL(is_temp_state_changed);


#if defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) 
#define WARM_COOL_USBIN 0x15  //USBIN 1.5A
#define WARM_COOL_FAST  0x59  // FAST 1.2A, term 50mA
#define NORMAL_USBIN 0x16  //USBIN 1.8A
#define NORMAL_FAST  0xB9  // FAST 2A, term 50mA
#elif defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
#define WARM_COOL_USBIN 0x16 //USBIN 1.8A
#define WARM_COOL_FAST  0x39 // FAST 900mA, term 50mA
#define NORMAL_USBIN 0x16 //USBIN 1.8A
#define NORMAL_FAST  0xB9 // FAST 2A, term 50mA
#else
#define WARM_COOL_USBIN 0x15
#define WARM_COOL_FAST  0x59
#define NORMAL_USBIN 0x16
#define NORMAL_FAST  0xBA
#endif
void smb349_current_jeita (int mode, int cable_type)
{
#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    // sayuss charge_CMD 
    if(smb_data_ext.smb_chg_disabled) 
        return;
#endif /* CONFIG_PANTECH_PMIC_CHARGING_DISABLE */

    if(cable_present_state() == 0)
        return;

    switch(mode){
    case BATT_THERM_COLD:
        pr_debug("%s: jeita mode (%d) set cold\n", __func__, mode);
        smb349_write_reg(0x30,0x81); //charging disable
        smb349_write_reg(0x31,0x00); //mode change 
        smb_data.prev_state_temp_mode = BATT_THERM_COLD;
        break;

    case BATT_THERM_COOL: 
        pr_debug("%s: jeita mode (%d) set cool\n", __func__, mode);
        if(cable_type == PANTECH_USB) {
            smb349_usb_mode(smb_data.prev_cable_current);
            break;
        }
        if(smb_data.prev_state_temp_mode == BATT_THERM_COLD) {
            smb349_write_reg(0x30, 0x81); //charging disable
            smb349_write_reg(0x31, 0x02); //usb mode
        }
        smb349_write_reg(0x01, WARM_COOL_USBIN); 
        smb349_write_reg(0x00, WARM_COOL_FAST); 
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31, 0x01); //hc mode	
        smb_data.prev_state_temp_mode = BATT_THERM_COOL;
        break;	

    case BATT_THERM_NORMAL:
        pr_debug("%s: jeita mode (%d) set mormal 1\n", __func__, mode);
        if(cable_type == PANTECH_USB) {
            smb349_usb_mode(smb_data.prev_cable_current);
            break;
        }
        if(smb_data.prev_state_temp_mode == BATT_THERM_COOL || smb_data.prev_state_temp_mode == BATT_THERM_WARM){
            smb349_write_reg(0x30, 0x81); //charging disable
            smb349_write_reg(0x31, 0x02);  //usb mode
        }
        smb349_write_reg(0x01, NORMAL_USBIN); 
        smb349_write_reg(0x00, NORMAL_FAST);  
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31, 0x01);  //hc mode	
        smb_data.prev_state_temp_mode = BATT_THERM_NORMAL;
        break;

    case BATT_THERM_WARM:
        pr_debug("%s: jeita mode (%d) set warm\n", __func__, mode);
        if(cable_type == PANTECH_USB) {
            smb349_usb_mode(smb_data.prev_cable_current);
            break;
        }
        if(smb_data.prev_state_temp_mode == BATT_THERM_HOT) {
            smb349_write_reg(0x30, 0x81); //charging disable
            smb349_write_reg(0x31,0x02); //usb mode
        }			
        smb349_write_reg(0x01, WARM_COOL_USBIN); 
        smb349_write_reg(0x00, WARM_COOL_FAST); 
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31, 0x01);  //hc mode	
        smb_data.prev_state_temp_mode = BATT_THERM_WARM;
        break;

    case BATT_THERM_HOT:
        pr_debug("%s: jeita mode (%d) set hot\n", __func__, mode);
        smb349_write_reg(0x30,0x81); //charging disable
        smb349_write_reg(0x31,0x00); //mode change 
        smb_data.prev_state_temp_mode = BATT_THERM_HOT;

    default:
        break;
    }
    //smb349_print_all_reg();
}
EXPORT_SYMBOL_GPL(smb349_current_jeita);

void smb349_current_set(int cable_type, int current_ma)
{ 
    smb_data.qcom_cable_type = cable_type;

#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    if(smb_data_ext.smb_chg_disabled)// sayuss charge_CMD 
        return;
#endif /* CONFIG_PANTECH_PMIC_CHARGING_DISABLE */

    if(cable_present_state() == 0)
        smb_data.prev_cable_type = PANTECH_CABLE_NONE;

    if(smb_data.prev_cable_current == current_ma) {
        if(smb_data.prev_cable_type == cable_type || cable_present_state() == 0)	
            return;
    }
    smb_data.prev_cable_current = current_ma;
    smb_data.prev_cable_type = cable_type;

    if(cable_type == PANTECH_USB) {
        if(smb_data_ext.is_factory_dummy) {
            pr_debug("%s: is_factory_dummy=%d", __func__, smb_data_ext.is_factory_dummy);
            smb349_write_reg(0x30, 0x83); //charging enable
            smb349_write_reg(0x31, 0x02); //USB 900mA
            smb349_write_reg(0x08, 0x38); //USB 3.0 mode 
        } else {
            smb349_current_jeita(smb_data_ext.prev_state_mode, smb_data.prev_cable_type);
        }
    } else if (cable_type == PANTECH_AC) { 
        smb349_current_jeita(smb_data_ext.prev_state_mode, smb_data.prev_cable_type);
    }
}
EXPORT_SYMBOL_GPL(smb349_current_set);

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
static int smb349_fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
    struct fb_event *evdata = data;
    int power;

	if((PANTECH_AC != smb_data.qcom_cable_type) || (cable_present_state() == 0))
		return 0;
    
    power = *(int *)evdata->data;
    smb_data.lcd_state.state = POWER_IS_ON(power) ? 1 : 0;
	if(smb_data.lcd_state.state) {
		smb349_current_jeita(smb_data_ext.prev_state_mode, smb_data.qcom_cable_type);
		// sayuss : Never block or delete this message
		pr_debug("%s: LCD On\n", __func__);
		// To do
	} else {
		smb349_current_jeita(smb_data_ext.prev_state_mode, smb_data.qcom_cable_type);
		// sayuss : Never block or delete this message
		pr_debug("%s: LCD Off\n", __func__);
		// To do
	}
    return 0;
}
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_FINGERPRINT)
int g_use_sensor = 0;

void smb349_adjust_limit_use_sensor(int use_sensor)
{
	if((PANTECH_AC != smb_data.qcom_cable_type) || (cable_present_state() == 0))
		return;
	
	g_use_sensor = use_sensor;
	pr_debug("%s: sensor mode limitation g_use_sensor %d, cable_type_value %d\n", __func__, use_sensor, smb_data.prev_cable_type );

	if(g_use_sensor) {
		// sayuss : Never block or delete this message
		pr_debug("%s: sensor mode limitation On\n", __func__);
//		smb349_usb_mode(500);
	} else {
		// sayuss : Never block or delete this message
		pr_debug("%s: sensor mode limitation Off\n", __func__);
//		smb349_current_jeita(smb_data_ext.prev_state_mode, smb_data.qcom_cable_type);	

	}
}
EXPORT_SYMBOL_GPL(smb349_adjust_limit_use_sensor);
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_FINGERPRINT */

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
#define T_THERM_MAX   53
#define T_THERM_MIN	  48
void smb349_t_sense_limit(int mode)
{
	if((PANTECH_AC != smb_data.qcom_cable_type) || (cable_present_state() == 0) || smb_data_ext.t_prev_mode == mode)
		return;
	
	smb_data_ext.t_prev_mode = mode;
	if(smb_data_ext.t_prev_mode) {
		pr_debug("%s: T_sensor mode limitation On\n", __func__);
#if defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
		smb349_write_reg(0x00, 0x3A); // FAST 900mA
#else
		smb349_write_reg(0x01, 0x15); //USBIN 1.5A
		smb349_write_reg(0x00, 0x59); // FAST 1.2A
#endif		
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31, 0x01);  //hc mode	
		smb_data_ext.t_sensor_mode = true;
	} else {
		pr_debug("%s: T_sensor mode limitation Off\n", __func__);
		if(smb_data.prev_state_temp_mode == BATT_THERM_NORMAL) {
			smb349_write_reg(0x30, 0x81); //charging disable
			smb349_write_reg(0x31, 0x02); //usb mode
		}
		smb349_current_jeita(smb_data_ext.prev_state_mode, smb_data.qcom_cable_type);
		smb_data_ext.t_sensor_mode = false;
	}
}
EXPORT_SYMBOL_GPL(smb349_t_sense_limit);



int t_sensor_value(void)
{
	struct tsens_device tsens_dev;
	long T_sense_2;
	tsens_dev.sensor_num = 2; 
	tsens_get_temp(&tsens_dev, &T_sense_2);
	pr_debug("%s: T_sense_2 %ld, t_sensor_mode %d \n", __func__, T_sense_2, smb_data_ext.t_sensor_mode);

	if(T_sense_2 > T_THERM_MAX) {
		return T_SENSE_UP;
	} else if(smb_data_ext.t_sensor_mode == true && T_sense_2 < T_THERM_MIN) {
		return T_SENSE_DOWN;
	}

	return T_SENSE_NORMAL;
	
}
EXPORT_SYMBOL_GPL(t_sensor_value);
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR */

int charger_ready_status(void)
{
	return charger_ready;
}
EXPORT_SYMBOL_GPL(charger_ready_status);

#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
int smb347_get_otg_status(void)
{
	u8 reg,rc, ret;

	mutex_lock(&smb_data.otg_lock_mutex);
	rc =  smb349_read_reg(0x0C,&reg);
	if (!rc)
		ret = (0x20 &reg) >> 5;
	mutex_unlock(&smb_data.otg_lock_mutex);

	pr_debug("%s: smb347_get_otg_status = 0x%03X\n", __func__, reg);

	return ret;
}
EXPORT_SYMBOL_GPL(smb347_get_otg_status);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */

//+++ 20130806, P14787, djjeon, charging setting stability test
#if defined(CONFIG_PANTECH_PMIC_MONITOR_TEST)
void smb349_test_limit_up(int on)
{
	if ((PANTECH_USB != smb_data.qcom_cable_type) || (cable_present_state() == 0)) {
		return;
	}
	
	if(on) {
		smb349_write_reg(0x30, 0x81); //charging disable
		smb349_write_reg(0x31, 0x01);  //hc mode
		smb349_write_reg(0x00, 0x79);  // FAST 1.5A
		smb349_write_reg(0x30, 0x83); //charging enable
		smb349_write_reg(0x31, 0x01);  //hc mode	
		pr_debug("%s: test_limit_up On\n", __func__);
    } else {	
		smb349_usb_mode(500);
		pr_debug("%s: test_limit_up Off\n", __func__);
	}
}
EXPORT_SYMBOL_GPL(smb349_test_limit_up);

unsigned char smb349_get_reg(unsigned char reg)
{
    unsigned char value;
    smb349_read_reg(reg, &value);
    return value;
}
EXPORT_SYMBOL_GPL(smb349_get_reg);
#endif /* CONFIG_PANTECH_PMIC_MONITOR_TEST */

//--- 20130806, P14787, djjeon, charging setting stability test
static int __init smb349_init(void)
{
	pr_info("%s: start of %s\n", __func__, __func__);
    
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
    if(fb_register_client(&smb_data.lcd_state.fb_noti)) {
        pr_err("%s: Unabled to reigster fb_notifier\n", __func__);
    }
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

#if defined(CONFIG_PANTECH_PMIC_I2C_LOCK)    
    rt_mutex_init(&smb_data.reg_lock);
#endif /* CONFIG_PANTECH_PMIC_I2C_LOCK */
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
    mutex_init(&smb_data.otg_lock_mutex);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
	gpio_request(SMB_SCL_PIN, "smb_scl");
	gpio_request(SMB_SDA_PIN, "smb_sda");
	
	gpio_tlmm_config(GPIO_CFG(SMB_SCL_PIN,0,GPIO_CFG_OUTPUT,GPIO_CFG_NO_PULL,GPIO_CFG_2MA),GPIO_CFG_DISABLE);		
	gpio_tlmm_config(GPIO_CFG(SMB_SDA_PIN,0,GPIO_CFG_OUTPUT,GPIO_CFG_NO_PULL,GPIO_CFG_2MA),GPIO_CFG_DISABLE);
	
	mdelay(10);

	smb349_print_all_reg();

	pr_info("%s: end of %s\n", __func__, __func__);
        charger_ready = true;
	return 0;
}

static void __exit smb349_exit(void)
{
//	i2c_del_driver(&smb349_i2c_driver);
#if defined(CONFIG_PANTECH_PMIC_I2C_LOCK)
    rt_mutex_destroy(&smb_data.reg_lock);
#endif /* CONFIG_PANTECH_PMIC_I2C_LOCK */
#if defined(CONFIG_PANTECH_PMIC_OTG_UVLO)
    mutex_destroy(&smb_data.otg_lock_mutex);
#endif /* CONFIG_PANTECH_PMIC_OTG_UVLO */
}

module_init(smb349_init);
module_exit(smb349_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smb349 charger/battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" smb349_charger);

