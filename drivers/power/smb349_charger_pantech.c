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
#include <linux/kernel.h>
#include <linux/power/smb349_charger_pantech.h>
#include <linux/workqueue.h>  
#include <linux/qpnp/qpnp-adc.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/spmi.h>
#include <linux/rtc.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/mfd/pm8xxx/batterydata-lib.h>

#if defined CONFIG_PANTECH_PMIC_AUTO_PWR_ON
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <mach/msm_smsm.h>
#endif /* CONFIG_PANTECH_PMIC_AUTO_PWR_ON */

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

extern int cable_present_state(void);

/* Register definitions */
#define CHG_CURRENT_REG				0x00	/* Fast charge current + AC input current limit */
#define CHG_OTHER_CURRENT_REG		0x01	/* Pre-cahrge current + Term current */
#define VAR_FUNC_REG				0x02	
#define FLOAT_VOLTAGE_REG			0x03	/* Pre-charge voltage + Float voltage */
#define CHG_CTRL_REG				0x04	
#define STAT_TIMER_REG				0x05	/* charge time out */
#define PIN_ENABLE_CTRL_REG			0x06	
#define THERM_CTRL_A_REG			0x07		
#define CTRL_FUNCTIONS_REG			0x09	
#define OTG_TLIM_THERM_CNTRL_REG	0x0A	
#define TEMP_MONITOR_REG			0x0B	
#define FAULT_IRQ_REG				0x0C	
#define IRQ_ENABLE_REG				0x0D	
#define SYSOK_REG					0x0E	

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

// ????
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
int offline_boot_ok=0, boot_lcd_cnt=0;
#endif

struct _smb349_external smb_data_ext = {
    .prev_state_mode = BATT_THERM_UNKNOWN,
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
    .t_sensor_mode = false,
    .t_prev_mode = false,
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR */
#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    .smb_chg_disabled = false,
#endif /* CONFIG_PANTECH_PMIC_CHARGING_DISABLE */
    .is_factory_dummy = 0,
	.is_CDP = 0,
};
EXPORT_SYMBOL(smb_data_ext);

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
#define POWER_IS_ON(pwr)    ((pwr) <= FB_BLANK_NORMAL)
struct _smb349_lcd {
    int state;
    struct notifier_block fb_noti;
};
static int smb349_fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data);
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

struct _smb349 {
    int prev_cable_type;
    int prev_cable_current;
    int qcom_cable_type;
    int therm_boundary_min[BATT_THERM_MAX];
    int therm_boundary_max[BATT_THERM_MAX];
    int therm_boundary_init[BATT_THERM_MAX];
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
    struct _smb349_lcd lcd_state;
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */
};

static struct _smb349 smb_data = {
    .prev_cable_type = PANTECH_CABLE_NONE,
    .prev_cable_current = 0,
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

struct smb349_data {
	struct i2c_client   *client;
    struct _smb349 *smb_data;
};
static struct smb349_data *the_smb349;

int smb349_read(struct smb349_data *smb, u8 reg)
{
	int ret;

	ret = i2c_smbus_read_byte_data(smb->client, reg);
	if (ret < 0)
		dev_warn(&smb->client->dev, "failed to read reg 0x%x: %d\n",
			 reg, ret);
	return ret;
}

int smb349_write(struct smb349_data *smb, u8 reg, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(smb->client, reg, val);
	if (ret < 0)
		dev_warn(&smb->client->dev, "failed to write reg 0x%x: %d\n",
			 reg, ret);
	return ret;
}

int smb349_read_reg(u8 reg, unsigned char *val)
{
	s32 ret;
	if(!the_smb349->client)
		return -EIO;
	ret = i2c_smbus_read_byte_data(the_smb349->client, reg);
	if (ret < 0) {
		pr_err("smb349 i2c read fail: can't read from %02x: %d\n", reg, ret);
		return ret;
	} else{
		*val = ret;
	}	
	return 0;
}


int smb349_write_reg(u8 reg, unsigned char val)
{
	s32 ret;

	if(!the_smb349->client)
		return -EIO;
	
	ret = i2c_smbus_write_byte_data(the_smb349->client, reg, val); 
	if (ret < 0) {
		pr_err("smb349 i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, ret);
		return ret;
	}
	
	return 0;
}

void smb349_otg_power(int on)
{
	pr_debug("%s: on=%d\n", __func__, on);
	if (on){		 
		smb349_write_reg(0x30,0x91); // OTG: enable, STAT: disable
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5
		the_smb349->smb_data->prev_cable_type = 0;
	}
	else {		
		smb349_write_reg(0x30,0xC0); // OTG: disable, STAT: enable, disable charging
		smb349_write_reg(0x31,0x00); // USB1 or USB1.5	
	}
}
EXPORT_SYMBOL_GPL(smb349_otg_power);

/*56S TP10 board apply*/
#if defined(CONFIG_PANTECH_PMIC_OTG)
void smb349_otg_power_current(int mode)
{
    pr_debug("%s: mode=%d\n", __func__, mode);
    switch(mode){
    case 1:
        smb349_write_reg(0x0A,0x33);  //250mA
        break;	
    case 2:
        smb349_write_reg(0x0A,0x37); //500mA
        break;	
    case 3:
        smb349_write_reg(0x0A,0x3B); //750mA
        break;	
    case 4:
        smb349_write_reg(0x0A,0x3F); //1000mA
        break;	
    default:
        break;
    }
}
EXPORT_SYMBOL_GPL(smb349_otg_power_current);
#endif 

void smb349_print_reg(u8 reg)
{
	u8 is_invalid_temp;	
	smb349_read_reg(reg, &is_invalid_temp);
	pr_err("0x%x reg value = %.2X\n", reg, is_invalid_temp);
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
    int term_satatus = 0;
    u8 is_invalid_temp;	
    term_satatus =  smb349_read(the_smb349, CHG_CTRL_REG);
    if (term_satatus < 0)
        return;
    if(enable){
        term_satatus &= ~CHG_CTRL_TERM_ENABLE;
    }else{
        term_satatus |= CHG_CTRL_TERM_ENABLE;		
    }	
    term_satatus = smb349_write(the_smb349, CHG_CTRL_REG, term_satatus);
    smb349_read_reg(0x04, &is_invalid_temp);
    pr_debug("0x04 value = %.2X\n", is_invalid_temp);
}
EXPORT_SYMBOL_GPL(end_recharging_enable);
#endif

void smb349_charging_enable(bool enable)
{
    int charging_status;
    u8 is_invalid_temp;	
    charging_status = smb349_read(the_smb349, CMD_A);
    if (charging_status < 0)
        return;
    if(enable){
        charging_status |= CMD_A_CHG_ENABLED;
        charging_status = smb349_write(the_smb349, CMD_A, charging_status);	
        smb349_read_reg(0x30, &is_invalid_temp);
        pr_debug("0x30 value = %.2X\n", is_invalid_temp);
    }else{
        charging_status &= ~CMD_A_CHG_ENABLED;
        charging_status = smb349_write(the_smb349, CMD_A, charging_status);
        smb349_read_reg(0x30, &is_invalid_temp);
        pr_debug("0x30 value = %.2X\n", is_invalid_temp);
    }
}
EXPORT_SYMBOL_GPL(smb349_charging_enable);	

static void smb349_usb_mode(int current_ma)
{
    // modified by skkim (p14200@LS1 / 2013.08.16)
    if (!get_is_offline_charging_mode()) {
        if(current_ma == 100){
            smb349_write_reg(0x30, 0x83); //charging enable
            smb349_write_reg(0x31,0x00); //usb 100 
        }else if(current_ma == 900){	
            smb349_write_reg(0x30, 0x83); //charging enable
            smb349_write_reg(0x31,0x06); //usb 900mA
        }else{
            smb349_write_reg(0x30, 0x83); //charging enable
            smb349_write_reg(0x31,0x02); //usb 500
        }
    }
    else {
        // Only Offline Charging Mode (for TEST)
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31,0x02); //usb 500 
    }
}

#if defined(CONFIG_PANTECH_PMIC_AUTO_PWR_ON)
int get_auto_power_on_flag(void)
{
    static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;

    smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
                                                                     sizeof(oem_pm_smem_vendor1_data_type));
    if(smem_id_vendor1_ptr) {
        pr_debug("%s: auto_power_on_soc_check=%d\n", __func__, smem_id_vendor1_ptr->auto_power_on_soc_check);
        return smem_id_vendor1_ptr->auto_power_on_soc_check;
    }

    return 0;
}
#endif //CONFIG_PANTECH_PMIC_AUTO_PWR_ON

int get_is_offline_charging_mode(void)
{
	static oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
	
	smem_id_vendor1_ptr = (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,
		sizeof(oem_pm_smem_vendor1_data_type));

    if(smem_id_vendor1_ptr) {
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

	if( BATT_THERM_UNKNOWN == index ) {
		index--;
		for( ; index > -1 ; index--) {
			if(temp > the_smb349->smb_data->therm_boundary_init[index])
				break;
		}
	} else if(temp < the_smb349->smb_data->therm_boundary_min[*state]) {
		index--;
		for( ; index > -1 ; index--) {
			if(temp < the_smb349->smb_data->therm_boundary_max[index] && temp >= the_smb349->smb_data->therm_boundary_min[index])
				break;
		}
	} else if(temp >= the_smb349->smb_data->therm_boundary_max[*state]) {
		index++;
		for( ; index < BATT_THERM_MAX ; index++) {
			if(temp < the_smb349->smb_data->therm_boundary_max[index] && temp >= the_smb349->smb_data->therm_boundary_min[index])
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


void smb349_current_jeita (int mode, int cable_type)
{
#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    // sayuss charge_CMD 
    if(smb_data_ext.smb_chg_disabled) 
        return;
#endif

    if(cable_present_state() == 0)
        return;

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
    if(get_is_offline_charging_mode()) {
        pr_debug("%s: offling charging mode\n", __func__);
        if(10 != boot_lcd_cnt)
            return;
    }
#endif

    pr_debug("%s: mode=%d cable_type=%d\n", __func__, mode, cable_type);

    switch(mode){
    case BATT_THERM_COLD:
        pr_debug("%s: jeita mode (%d) set cold\n", __func__, mode);
        smb349_write_reg(0x30,0x81); //charging disable
        smb349_write_reg(0x31,0x00); //mode change 
        break;

    case BATT_THERM_COOL: 
        pr_debug("%s: jeita mode (%d) set cool\n", __func__, mode);
        if(cable_type == PANTECH_USB){
            smb349_usb_mode(the_smb349->smb_data->prev_cable_current);
            break;
        }
        smb349_write_reg(0x30, 0x81); //charging disable
        smb349_write_reg(0x31, 0x01); //hc mode
		if(smb_data_ext.is_CDP)
			smb349_write_reg(0x00, 0x02); // fast 1A_input 1A 
		else
        	smb349_write_reg(0x00,0x09); // fast 1A_input 1.8A
        smb349_write_reg(0x02, 0x8F);  //AICL disable, AICL Detection Threshold 4.5V
        smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31, 0x01); //hc mode
        break;	

    case BATT_THERM_NORMAL:
        pr_debug("%s: jeita mode (%d) set mormal 1\n", __func__, mode);
        if(cable_type == PANTECH_USB){
            smb349_usb_mode(the_smb349->smb_data->prev_cable_current);
            break;
        }
        smb349_write_reg(0x30, 0x81); //charging disable
        smb349_write_reg(0x31, 0x01); //hc mode
#if defined(CONFIG_PANTECH_PMIC_LIMIT_WITH_EF56S)
		if(smb_data_ext.is_CDP)
			smb349_write_reg(0x00, 0x02); // fast 1A_input 1A
		else
        	smb349_write_reg(0x00,0x59); // 2.5A_2A 
#else
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
        if(!the_smb349->lcd_state)
            smb349_write_reg(0x00,0xAE); // 3A_3A 
        else
            smb349_write_reg(0x00,0x6A); // 2.5A_2A 
#else	
        smb349_write_reg(0x00,0x6A); // 2.5A_2A
#endif
#endif
        smb349_write_reg(0x02, 0x8F);  //AICL enable, AICL Detection Threshold 4.5V
        smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31, 0x01); //hc mode
        break;

    case BATT_THERM_WARM:
        pr_debug("%s: jeita mode (%d) set warm\n", __func__, mode);
        if(cable_type == PANTECH_USB){
            smb349_usb_mode(the_smb349->smb_data->prev_cable_current);
            break;
        }
        smb349_write_reg(0x30, 0x81); //charging disable
        smb349_write_reg(0x31, 0x01); //hc mode
		if(smb_data_ext.is_CDP)
			smb349_write_reg(0x00, 0x02); // fast 1A_input 1A
		else
        	smb349_write_reg(0x00,0x09); // fast 1A_input 1.8A
        smb349_write_reg(0x02, 0x8F);  //AICL disable, AICL Detection Threshold 4.5V
        smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
        smb349_write_reg(0x30, 0x83); //charging enable
        smb349_write_reg(0x31, 0x01); //hc mode
        break;

    case BATT_THERM_HOT:
        pr_debug("%s: jeita mode (%d) set hot\n", __func__, mode);
        smb349_write_reg(0x30,0x81); //charging disable
        smb349_write_reg(0x31,0x00); //mode change 
        break;

    default:

        break;
    }
    //smb349_print_all_reg();
}
EXPORT_SYMBOL_GPL(smb349_current_jeita);

#if defined(CONFIG_MACH_MSM8974_EF59L) && (CONFIG_BOARD_VER == CONFIG_WS20)
// 2013.07.25. LS1. MKS.
// setting the registers of smb349 to default. only for EF59L WS20.
void smb349_B_register_init(void){
	smb349_write_reg(0x30, 0x81);   //disable charging
   	smb349_write_reg(0x31, 0x00);   //usb mode
	smb349_write_reg(0x06, 0x08);  // Register Control
	smb349_write_reg(0x00, 0x00);  // fast : 1000mA , input : 500mA
	smb349_write_reg(0x01, 0x39);  //pre_current 300mA, term current 100mA
	smb349_write_reg(0x02, 0x9F);  //AICL enable, AICL Detection Threshold 4.5V
	smb349_write_reg(0x03, 0xED);  //Pre-Charge To Fast Charge Threshold : 3.0V , Float Voltage : 4.36V
	smb349_write_reg(0x04, 0xE0);  //auto recharge disable, recharge Threshold : 50mV ->eoc recharging
	smb349_write_reg(0x05, 0x12); //Complete Charge Timeout : 382min, Pre-Charge Timeout : 191min 	
	smb349_write_reg(0x07, 0xD5);
	smb349_write_reg(0x09, 0x04); //GF 15msec 
	smb349_write_reg(0x0A, 0x33); //Thermal Loop Temperature Select : 130C, OTG Output Current Limit : 250mA, OTG Battery UVLO Threshold : 3.3V 
	smb349_write_reg(0x0B, 0xED);
	smb349_write_reg(0x0C, 0x00);
	smb349_write_reg(0x0D, 0x00);
	smb349_write_reg(0x0E, 0x38);
	smb349_write_reg(0x10, 0x40);
}
#endif

void smb349_current_set(int cable_type, int current_ma)
{ 
    the_smb349->smb_data->qcom_cable_type = cable_type;

#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    if(smb_data_ext.smb_chg_disabled)// sayuss charge_CMD 
        return;
#endif

    if(cable_present_state() == 0)
        the_smb349->smb_data->prev_cable_type = PANTECH_CABLE_NONE;

    if(the_smb349->smb_data->prev_cable_current == current_ma){
        if(the_smb349->smb_data->prev_cable_type == cable_type || cable_present_state() == 0)	
            return;
    }

    the_smb349->smb_data->prev_cable_current = current_ma;
    the_smb349->smb_data->prev_cable_type  = cable_type;	

    if(cable_type == PANTECH_USB){
#if defined(CONFIG_MACH_MSM8974_EF59L) && (CONFIG_BOARD_VER == CONFIG_WS20)
        smb349_B_register_init();
#endif
        if(smb_data_ext.is_factory_dummy){
            pr_debug("%s: is_factory_dummy=%d", __func__, smb_data_ext.is_factory_dummy);
            smb349_write_reg(0x30, 0x83); //charging enable
            smb349_write_reg(0x31,0x06); //usb 900mA
        }else{
            smb349_current_jeita(smb_data_ext.prev_state_mode, the_smb349->smb_data->prev_cable_type);
        }
    }else if (cable_type == PANTECH_AC){ 
#if defined(CONFIG_MACH_MSM8974_EF59L) && (CONFIG_BOARD_VER == CONFIG_WS20)
        smb349_B_register_init();
#endif
        smb349_current_jeita(smb_data_ext.prev_state_mode, the_smb349->smb_data->prev_cable_type);
    }
}
EXPORT_SYMBOL_GPL(smb349_current_set);

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
static int smb349_fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
    struct fb_event *evdata = data;
    int power;
    //20140204 youhw block reset entering suspend
    if(cable_present_state() == 0)
		return 0;

    power = *(int *)evdata->data;
    the_smb349->smb_data->lcd_state.state = POWER_IS_ON(power) ? 1 : 0;

#if 1
	if(!the_smb349->smb_data->lcd_state.state) {
		if(boot_lcd_cnt < 4)
			boot_lcd_cnt++;
		if(3<boot_lcd_cnt)
			offline_boot_ok = 1;
	} else {
		offline_boot_ok=0; 
	}
#else

	if((PANTECH_AC != the_smb349->smb_data->qcom_cable_type) || (cable_present_state() == 0))
		return;

	if(the_smb349->smb_data->lcd_state.state) {
		smb349_current_jeita(smb_data_ext.prev_state_mode, the_smb349->smb_data->qcom_cable_type);
		// sayuss : Never block or delete this message
		pr_debug("%s: LCD On\n", __func__);

		// To do
	} else {
		smb349_current_jeita(smb_data_ext.prev_state_mode, the_smb349->smb_data->qcom_cable_type);
		// sayuss : Never block or delete this message
		pr_debug("%s: LCD Off\n", __func__);
		// To do
	}
#endif
    return 0;
}
#endif

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_FINGERPRINT)
void smb349_adjust_limit_use_sensor(int use_sensor)
{
	if((PANTECH_AC != the_smb349->smb_data->qcom_cable_type) || (cable_present_state() == 0))
		return;
	
	pr_debug("%s: sensor mode limitation use_sensor=%d prev_cable_type=%d\n", __func__, use_sensor, the_smb349->smb_data->prev_cable_type );
	if(use_sensor) {
		// sayuss : Never block or delete this message
		pr_debug("%s: sensor mode limitation On\n", __func__);
		smb349_usb_mode(500);
	} else {
		// sayuss : Never block or delete this message
		pr_debug("%s: sensor mode limitation Off\n", __func__);
		smb349_current_jeita(smb_data_ext.prev_state_mode, the_smb349->smb_data->qcom_cable_type);	
	}
}
EXPORT_SYMBOL_GPL(smb349_adjust_limit_use_sensor);
#endif

#if defined(CONFIG_PANTECH_PMIC_MONITOR_TEST)
void smb349_test_limit_up(int on)
{
	if ((PANTECH_USB != the_smb349->smb_data->qcom_cable_type) || (cable_present_state() == 0))
		return;

    pr_debug("%s: on=%d\n", __func__, on);

	if(on) {
		smb349_write_reg(0x30,0x81); //charging disable
		smb349_write_reg(0x00,0x36); // 1.5A
		smb349_write_reg(0x31,0x01); //hc mode
		smb349_write_reg(0x30,0x83); //charging enable
		smb349_write_reg(0x31,0x01); //hc mode
		smb349_write_reg(0x02,0x8F);
	}
	else
	{
		smb349_write_reg(0x02,0x9F);
		smb349_usb_mode(500);
	}
}
EXPORT_SYMBOL_GPL(smb349_test_limit_up);

int charger_ready_status(void)
{
	return charger_ready;
}
EXPORT_SYMBOL(charger_ready_status);

unsigned char smb349_get_reg(unsigned char reg)
{
    unsigned char value;
    smb349_read_reg(reg, &value);
    return value;
}
EXPORT_SYMBOL_GPL(smb349_get_reg);
#endif /* CONFIG_PANTECH_PMIC_MONITOR_TEST */

static int __devinit smb349_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct smb349_data *smb349;  
	struct i2c_adapter *adapter;

	pr_info("%s: start of %s\n", __func__, __func__);

	adapter = to_i2c_adapter(client->dev.parent);	
	smb349 = kzalloc(sizeof(struct smb349_data), GFP_KERNEL);

	if (!smb349) {
		pr_err("%s: failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	smb349->client = client;
	i2c_set_clientdata(client, smb349);

#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD)
    if(fb_register_client(&smb_data.lcd_state.fb_noti))
        pr_err("%s: Unabled to register fb_notifier\n", __func__);
#endif /* CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

    smb349->smb_data = &smb_data;
	the_smb349 = smb349;

	smb349_print_all_reg();	

	pr_info("%s: end of %s\n", __func__, __func__);

	charger_ready = true;

	return 0;
}
static int __devexit smb349_remove(struct i2c_client *client)
{
	struct smb349_data *smb349 = i2c_get_clientdata(client);
	charger_ready = false;
	kfree(smb349);
	return 0;
}

static int smb349_suspend(struct device *dev)
{
	return 0;
}

static int smb349_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops smb349_pm_ops = {
	.suspend	= smb349_suspend,
	.resume		= smb349_resume,
};

static void smb349_shutdown(struct i2c_client *client)
{
	smb349_write_reg(0x30,0x81); //charging disable
	smb349_write_reg(0x31,0x00); //mode change 
}

static const struct i2c_device_id smb349_id[] = {
        {"smb349-i2c", 0},
        {},
};
MODULE_DEVICE_TABLE(i2c, smb349_device_id);

#ifdef CONFIG_OF
static struct of_device_id smb349_i2c_table[] = {
	{ .compatible = "smb349,smb349-i2c",}, // Compatible node must match dts
	{ },
};
#else
#define smb349_i2c_table NULL
#endif

static struct i2c_driver smb349_i2c_driver={
	.id_table = smb349_id ,
	.probe = smb349_i2c_probe ,
	.remove		= __devexit_p(smb349_remove),
	.shutdown = smb349_shutdown,
	.driver = {
		.name = "smb349-i2c",
		.pm	= &smb349_pm_ops,
		.owner = THIS_MODULE ,
		.of_match_table = smb349_i2c_table ,
	},
};

static int __init smb349_init(void)
{
	int rc;

	pr_info(KERN_ERR "%s: start of %s\n", __func__, __func__);
	rc = i2c_add_driver(&smb349_i2c_driver);
	if(rc) {
		printk(KERN_ERR "smb349_i2c driver add failed.\n");
	}
	pr_info(KERN_ERR "%s: end of %s\n", __func__, __func__);
	
	return rc;
}

static void __exit smb349_exit(void)
{
	i2c_del_driver(&smb349_i2c_driver);
}

module_init(smb349_init);
module_exit(smb349_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smb349 charger/battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" smb349_charger);

