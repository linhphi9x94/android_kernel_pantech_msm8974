#ifndef __MAX17058_BATTERY_H
#define __MAX17058_BATTERY_H

#include <linux/errno.h>

#ifdef CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058
int max17058_get_voltage(void);
int max17058_get_soc_for_otg(void);
int max17058_get_soc_for_led(void);
int max17058_get_soc(void);
int max17058_FG_SOC(void);
void max17058_update_rcomp(int temp);
#else
inline int max17058_get_voltage(void) {
    return 0;
}

inline int max17058_get_soc_for_otg(void) {
    return 0;
}

int max17058_get_soc_for_led(void) {
    return 0;
}

inline int max17058_get_soc(void) {
    return 0;
}

inline int max17058_FG_SOC(void) {
    return 0;
}

inline void max17058_update_rcomp(int temp) {
}
#endif

#endif /* __MAX17058_BATTERY_H */
