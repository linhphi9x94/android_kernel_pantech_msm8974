/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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
/*
 * Qualcomm PMIC QPNP Charger driver header file by Pantech Co.
 *
 */

#ifndef __QPNP_CHARGER_H
#define __QPNP_CHARGER_H

enum nonstandard_state {
    NONSTANDARD_READY = 0,
    NONSTANDARD_WORKING,
    NONSTANDARD_USBIN,
    NONSTANDARD_ACIN,
    NONSTANDARD_COMPLETED,
};

#ifdef CONFIG_PANTECH_PMIC_ABNORMAL
int qpnp_chg_get_nonstandard_state(void);
void qpnp_chg_notify_charger_type(int type);
#else
int qpnp_chg_get_nonstandard_state(void)
{
}
void qpnp_chg_notify_charger_type(int type)
{
}
#endif

#endif /* __QPNP_CHARGER_H */
