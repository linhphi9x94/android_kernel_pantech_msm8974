#ifndef SMB349_CHARGER_H
#define SMB349_CHARGER_H

enum pantech_cable_type {
	PANTECH_CABLE_NONE=0,
	PANTECH_OTG,
	PANTECH_USB,
	PANTECH_AC,
	PANTECH_UNKNOWN,		
	PANTECH_CABLE_MAX,
};

enum
{
	APSD_NOT_RUN	= 0,
	APSD_CDP,
	APSD_DCP,
	APSD_OCP,
	APSD_SDP,
	APSD_ACA,
	APSD_NOT_USED,
};

enum battery_thermal_trip_type {
	BATT_THERM_COLD = 0,
	BATT_THERM_COOL,
	BATT_THERM_NORMAL,
	BATT_THERM_WARM,
	BATT_THERM_HOT,
	BATT_THERM_UNKNOWN,
};

struct _smb349_external {
    int prev_state_mode;
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_TSENSOR)
    bool t_sensor_mode;
    bool t_prev_mode;
#endif 
#if defined(CONFIG_PANTECH_PMIC_CHARGING_DISABLE)
    bool smb_chg_disabled;  // sayuss charge_CMD 
#endif
    int is_factory_dummy;  // sayuss Factory
    int is_CDP;
};

int smb349_read_reg(u8 reg, unsigned char *val);
int smb349_write_reg(u8 reg, unsigned char val);
void smb349_print_all_reg(void);
void smb349_print_reg(u8 reg);
int smb349_charging_set(bool enable);
#if defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_FINGERPRINT)
void smb349_adjust_limit_use_sensor(int use_sensor);
#endif
#if defined(CONFIG_PANTECH_PMIC_AUTO_PWR_ON) || defined(CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD) //djjeon 20130610 add
int get_is_offline_charging_mode(void);
int get_auto_power_on_flag(void);
#endif /* CONFIG_PANTECH_PMIC_AUTO_PWR_ON || CONFIG_PANTECH_PMIC_CHARGE_LIMIT_WITH_LCD */

#ifdef CONFIG_PANTECH_PMIC_CHARGER_SMB349
void smb349_test_limit_up(int on);
unsigned char smb349_get_reg(unsigned char reg);
#else
inline void smb349_test_limit_up(int on) {
}

inline unsigned char smb349_get_reg(unsigned char reg) {
    return 0;
}
#endif
#endif
