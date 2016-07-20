<<<<<<< HEAD
/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
=======
/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
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

#ifndef __NCP6335D_H__
#define __NCP6335D_H__

enum {
	NCP6335D_VSEL0,
	NCP6335D_VSEL1,
};

struct ncp6335d_platform_data {
	struct regulator_init_data *init_data;
	int default_vsel;
	int slew_rate_ns;
	int discharge_enable;
	bool sleep_enable;
};

<<<<<<< HEAD
=======
#ifdef CONFIG_REGULATOR_ONSEMI_NCP6335D
int __init ncp6335d_regulator_init(void);
#else
static inline int __init ncp6335d_regulator_init(void) { return 0; }
#endif

>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
#endif
