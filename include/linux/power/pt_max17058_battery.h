#ifndef __PT_MAX17058_BATTERY_H
#define __PT_MAX17058_BATTERY_H

#if defined(CONFIG_PANTECH_EF63_PMIC_FUELGAUGE_MAX17058)
void pt_max17058_set_rcomp(u8 msb, u8 lsb);
int pt_max17058_get_vcell(int *batt_vol);
int pt_max17058_get_soc(int *batt_soc);
int pt_max17058_calc_rcomp(int batt_temp);
#else
int pt_max17058_calc_rcomp(int batt_temp)
{
	return -ENXIO;
}
void pt_max17058_set_rcomp(u8 msb, u8 lsb)
{
}
int pt_max17058_get_vcell(int *batt_vol)
{
	return -ENXIO;
}
int pt_max17058_get_soc(int *batt_soc)
{
	return -ENXIO;
}
#endif
#endif
