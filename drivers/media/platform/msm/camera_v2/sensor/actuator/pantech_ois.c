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

#include "pantech_ois.h"

#define RUMBA_OISCTRL_ADDR  0x0000
#define RUMBA_OISSTS_ADDR  0x0001
#define RUMBA_OISMODE_ADDR  0x0002

#define RUMBA_FWINFO_CTRL_ADDR  0x0080
#define RUMBA_GETGRX_ADDR  0x0082
#define RUMBA_GETGRY_ADDR  0x0084
#define RUMBA_GRX_ADDR  0x0086
#define RUMBA_GRY_ADDR  0x0088
#define RUMBA_HAX_ADDR  0x008E
#define RUMBA_HAY_ADDR  0x0090

#define RUMBA_OISSTS_INIT  0
#define RUMBA_OISSTS_IDLE  1
#define RUMBA_OISSTS_RUN  2

#define copy_from_le(a,b)    \
    { \
        a = (b&0xff) << 8; \
        a |= (b&0xff00) >> 8; \
    }

static struct pantech_ois_ctrl_t ois_ctrl;
static int32_t info_ctrl = 0;

EXPORT_SYMBOL_GPL(pantech_get_oisdev);

void pantech_get_oisdev(struct pantech_ois_ctrl_t **  o_ctrl)
{
    if(!ois_ctrl.a_ctrl)
        *o_ctrl = NULL;
    else
        *o_ctrl = &ois_ctrl;
}

static int32_t pantech_OIS_get_info(
	struct msm_actuator_ctrl_t *a_ctrl, struct pantech_ois_get_info_t * ois_info)
{
    int32_t rc = 0;
    uint16_t rdata = 0;

    if(info_ctrl != 1)
    {
        OIS_CERR("first, set ois info ctrl with enable !!!\n");
        return rc;
    }

    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_read(
       &a_ctrl->i2c_client,
       RUMBA_OISSTS_ADDR,
       &rdata, MSM_CAMERA_I2C_BYTE_DATA);
    if(rdata != RUMBA_OISSTS_RUN)
        return rc;

    mdelay(1);

    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_read(
       &a_ctrl->i2c_client,
       RUMBA_GRX_ADDR,
       &rdata, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0)
        goto ois_get_info_end;    
    copy_from_le(ois_info->cal_gyro_out_x, rdata);

    mdelay(1);
    
    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_read(
       &a_ctrl->i2c_client,
       RUMBA_HAX_ADDR,
       &rdata, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0)
        goto ois_get_info_end;    
    copy_from_le(ois_info->hall_out_x, rdata);

    mdelay(1);
    
    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_read(
       &a_ctrl->i2c_client,
       RUMBA_GRY_ADDR,
       &rdata, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0)
        goto ois_get_info_end;    
    copy_from_le(ois_info->cal_gyro_out_y, rdata);

    mdelay(1);
    
    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_read(
       &a_ctrl->i2c_client,
       RUMBA_HAY_ADDR,
       &rdata, MSM_CAMERA_I2C_WORD_DATA);
    if(rc < 0)
        goto ois_get_info_end;    
    copy_from_le(ois_info->hall_out_y, rdata);

    mdelay(1);

/*
        OIS_CDBG("pantech_OIS_get_info - grx=%x, gry=%x, hax=%x, hay=%x++++++++++++++\n", 
            ois_info->cal_gyro_out_x, ois_info->cal_gyro_out_y, ois_info->hall_out_x, ois_info->hall_out_y);
    */
ois_get_info_end:
    if(rc < 0)
        OIS_CERR("OIS get info read fail:%d\n", rc);

    return rc;
}

static int32_t pantech_OIS_stop(
	struct msm_actuator_ctrl_t *a_ctrl)
{
    int32_t rc = 0;
    uint16_t rdata = 0;
    int32_t i = 0;

    OIS_CDBG("pantech_OIS_stop ++++++++++++++\n");
    if(!a_ctrl)
        return -1;
    
    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_write(
        &a_ctrl->i2c_client,
        RUMBA_OISCTRL_ADDR,
        0x00, MSM_CAMERA_I2C_BYTE_DATA);
    if (rc < 0)
        goto ois_stop_end;

    mdelay(1);
    for(i = 0; i < 10; i++)
    {
        rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_read(
           &a_ctrl->i2c_client,
           RUMBA_OISSTS_ADDR,
           &rdata, MSM_CAMERA_I2C_BYTE_DATA);
        if(rdata == RUMBA_OISSTS_IDLE || rdata == RUMBA_OISSTS_INIT)
            break;
        else
            rc = -1;
        mdelay(10);
    }
    
ois_stop_end:    
    if(rc < 0)
        OIS_CERR("OIS stop fail:%d\n", rc);

    return rc;
}

static int32_t pantech_OIS_start(
	struct msm_actuator_ctrl_t *a_ctrl)
{
    int32_t rc = 0;
    
    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_write(
        &a_ctrl->i2c_client,
        RUMBA_OISCTRL_ADDR,
        0x01, MSM_CAMERA_I2C_BYTE_DATA);
        
    if (rc < 0)
        OIS_CERR("OIS start fail:%d\n", rc);

    return rc;
}

static int32_t pantech_OIS_set_mode(
	struct msm_actuator_ctrl_t *a_ctrl, uint16_t mode)
{
    int32_t rc = 0;

    rc = pantech_OIS_stop(a_ctrl);
    if(rc < 0)
        goto ois_set_mode_end;
    
    switch(mode)
    {
    case 0:
        OIS_CDBG("pantech_OIS_mode set - centering mode\n");

        rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_write(
            &a_ctrl->i2c_client,
            RUMBA_OISMODE_ADDR,
            5, MSM_CAMERA_I2C_BYTE_DATA);
        if(rc < 0)
            break;
        break;
    case 1:
    case 2:
        OIS_CDBG("pantech_OIS_mode set - %d\n", mode);

        rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_write(
            &a_ctrl->i2c_client,
            RUMBA_OISMODE_ADDR,
            mode-1, MSM_CAMERA_I2C_BYTE_DATA);
        if(rc < 0)
            break;

        break;
    default:
        break;        
    }

        OIS_CDBG("pantech_OIS_start\n");
		
        rc = pantech_OIS_start(a_ctrl);
    
ois_set_mode_end:
    if (rc < 0)
        OIS_CERR("pantech_OIS_set_mode fail:%d\n", mode);

    return rc;
}

static int32_t pantech_OIS_info_ctrl(
	struct msm_actuator_ctrl_t *a_ctrl, int32_t ctrl)
{
    int32_t rc = 0;
    
    if(!a_ctrl)
        return -1;

    if(ctrl != 0 && ctrl != 1)
        return -2;
    
    rc = a_ctrl->i2c_client.i2c_func_tbl->i2c_write(
        &a_ctrl->i2c_client,
        RUMBA_FWINFO_CTRL_ADDR,
        ctrl, MSM_CAMERA_I2C_BYTE_DATA);

    if(rc >= 0)
        info_ctrl = ctrl;
    
    return rc;
}
static void pantech_OIS_ctrl(int32_t ctrl)
{
    struct msm_actuator_ctrl_t *a_ctrl = ois_ctrl.a_ctrl;
    if(!a_ctrl)
        return;

    pantech_OIS_set_mode(a_ctrl, ctrl);
}


static struct pantech_ois_func_tbl ois_func_tbl = {
    .ois_stop = pantech_OIS_stop,
    .ois_set_mode = pantech_OIS_set_mode,
    .ois_start = pantech_OIS_start,
    .ois_ctrl = pantech_OIS_ctrl,
    .ois_get_info = pantech_OIS_get_info,
    .ois_info_ctrl = pantech_OIS_info_ctrl,
};

void pantech_OIS_init(struct msm_actuator_ctrl_t *a_ctrl)
{
    OIS_CDBG("pantech_OIS_init >> +++++++++++++++++\n");
    ois_ctrl.a_ctrl = a_ctrl;
    if(!ois_ctrl.a_ctrl)
        return;

    OIS_CDBG("pantech OIS function table set >> +++++++++++++++++\n");
    ois_ctrl.func_tbl = &ois_func_tbl;
    info_ctrl = 0;
}
