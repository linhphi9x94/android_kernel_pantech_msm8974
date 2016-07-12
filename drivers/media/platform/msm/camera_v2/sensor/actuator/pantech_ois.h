/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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
#ifndef PANTECH_OIS_H
#define PANTECH_OIS_H

#include <linux/module.h>
#include "msm_sd.h"
#include "msm_actuator.h"

#define OIS_DEBUG_MSG

#define OIS_CERR(fmt, args...) pr_err(fmt, ##args)
#ifdef OIS_DEBUG_MSG
#define OIS_CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define OIS_CDBG(fmt, args...) do { } while (0)
#endif

struct pantech_ois_ctrl_t;

struct pantech_ois_func_tbl {
    int32_t (*ois_stop)(struct msm_actuator_ctrl_t *a_ctrl);
    int32_t (*ois_set_mode)(struct msm_actuator_ctrl_t *a_ctrl, uint16_t mode);
    int32_t (*ois_start)(struct msm_actuator_ctrl_t *a_ctrl);
    void (*ois_ctrl) (int32_t ctrl);
    int32_t (*ois_get_info)(struct msm_actuator_ctrl_t *a_ctrl, struct pantech_ois_get_info_t * ois_info);
    int32_t (*ois_info_ctrl)(struct msm_actuator_ctrl_t *a_ctrl, int32_t ctrl);
};

struct pantech_ois_ctrl_t {
    struct msm_actuator_ctrl_t * a_ctrl;
    struct pantech_ois_func_tbl *func_tbl;
};

void pantech_get_oisdev(struct pantech_ois_ctrl_t **  o_ctrl);
void pantech_OIS_init(struct msm_actuator_ctrl_t *a_ctrl);

#endif //PANTECH_OIS_H
