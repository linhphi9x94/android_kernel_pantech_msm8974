/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/qpnp/pwm.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/msm_tsens.h>

#ifdef CONFIG_POWERSUSPEND
#include <linux/powersuspend.h>
#endif

#include "mdss_fb.h"
#include "mdss_dsi.h"
#include "mdss_mdp.h"
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
#include "ef63_display.h"
#include <mach/msm_smsm.h>
#endif
extern struct msm_fb_data_type * mfdmsm_fb_get_mfd(void);

#define DT_CMD_HDR 6
#define NEW_REV	1
#define OLD_REV	0

#define MIN_REFRESH_RATE 30

DEFINE_LED_TRIGGER(bl_led_trigger);

#ifdef F_LSI_VDDM_OFFSET_RD_WR 
unsigned int ldi_vddm_lut[128][2] = {
	{0, 13}, {1, 13}, {2, 14}, {3, 15}, {4, 16}, {5, 17}, {6, 18}, {7, 19}, {8, 20}, {9, 21},
	{10, 22}, {11, 23}, {12, 24}, {13, 25}, {14, 26}, {15, 27}, {16, 28}, {17, 29}, {18, 30}, {19, 31},
	{20, 32}, {21, 33}, {22, 34}, {23, 35}, {24, 36}, {25, 37}, {26, 38}, {27, 39}, {28, 40}, {29, 41},
	{30, 42}, {31, 43}, {32, 44}, {33, 45}, {34, 46}, {35, 47}, {36, 48}, {37, 49}, {38, 50}, {39, 51},
	{40, 52}, {41, 53}, {42, 54}, {43, 55}, {44, 56}, {45, 57}, {46, 58}, {47, 59}, {48, 60}, {49, 61},
	{50, 62}, {51, 63}, {52, 63}, {53, 63}, {54, 63}, {55, 63}, {56, 63}, {57, 63}, {58, 63}, {59, 63},
	{60, 63}, {61, 63}, {62, 63}, {63, 63}, {64, 12}, {65, 11}, {66, 10}, {67, 9}, {68, 8}, {69, 7},
	{70, 6}, {71, 5}, {72, 4}, {73, 3}, {74, 2}, {75, 1}, {76, 64}, {77, 65}, {78, 66}, {79, 67},
	{80, 68}, {81, 69}, {82, 70}, {83, 71}, {84, 72}, {85, 73}, {86, 74}, {87, 75}, {88, 76}, {89, 77},
	{90, 78}, {91, 79}, {92, 80}, {93, 81}, {94, 82}, {95, 83}, {96, 84}, {97, 85}, {98, 86}, {99, 87},
	{100, 88}, {101, 89}, {102, 90}, {103, 91}, {104, 92}, {105, 93}, {106, 94}, {107, 95}, {108, 96}, {109, 97},
	{110, 98}, {111, 99}, {112, 100}, {113, 101}, {114, 102}, {115, 103}, {116, 104}, {117, 105}, {118, 106}, {119, 107},
	{120, 108}, {121, 109}, {122, 110}, {123, 111}, {124, 112}, {125, 113}, {126, 114}, {127, 115},
};
#endif

void mdss_dsi_panel_pwm_cfg(struct mdss_dsi_ctrl_pdata *ctrl)
{
	ctrl->pwm_bl = pwm_request(ctrl->pwm_lpg_chan, "lcd-bklt");
	if (ctrl->pwm_bl == NULL || IS_ERR(ctrl->pwm_bl)) {
		pr_err("%s: Error: lpg_chan=%d pwm request failed",
				__func__, ctrl->pwm_lpg_chan);
	}
}

static void mdss_dsi_panel_bklt_pwm(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	int ret;
	u32 duty;
	u32 period_ns;

	if (ctrl->pwm_bl == NULL) {
		pr_err("%s: no PWM\n", __func__);
		return;
	}

	if (level == 0) {
		if (ctrl->pwm_enabled)
			pwm_disable(ctrl->pwm_bl);
		ctrl->pwm_enabled = 0;
		return;
	}

	duty = level * ctrl->pwm_period;
	duty /= ctrl->bklt_max;

	pr_debug("%s: bklt_ctrl=%d pwm_period=%d pwm_gpio=%d pwm_lpg_chan=%d\n",
			__func__, ctrl->bklt_ctrl, ctrl->pwm_period,
				ctrl->pwm_pmic_gpio, ctrl->pwm_lpg_chan);

	pr_debug("%s: ndx=%d level=%d duty=%d\n", __func__,
					ctrl->ndx, level, duty);

	if (ctrl->pwm_enabled) {
		pwm_disable(ctrl->pwm_bl);
		ctrl->pwm_enabled = 0;
	}

	if (ctrl->pwm_period >= USEC_PER_SEC) {
		ret = pwm_config_us(ctrl->pwm_bl, duty, ctrl->pwm_period);
		if (ret) {
			pr_err("%s: pwm_config_us() failed err=%d.\n",
					__func__, ret);
			return;
		}
	} else {
		period_ns = ctrl->pwm_period * NSEC_PER_USEC;
		ret = pwm_config(ctrl->pwm_bl,
				level * period_ns / ctrl->bklt_max,
				period_ns);
		if (ret) {
			pr_err("%s: pwm_config() failed err=%d.\n",
					__func__, ret);
			return;
		}
	}

	ret = pwm_enable(ctrl->pwm_bl);
	if (ret)
		pr_err("%s: pwm_enable() failed err=%d\n", __func__, ret);
	ctrl->pwm_enabled = 1;
}

static char dcs_cmd[2] = {0x54, 0x00}; /* DTYPE_DCS_READ */
static struct dsi_cmd_desc dcs_read_cmd = {
	{DTYPE_DCS_READ, 1, 0, 1, 5, sizeof(dcs_cmd)},
	dcs_cmd
};

#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
u32 mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
		char cmd1, void (*fxn)(int,char *), char *rbuf, int len)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = len;
	cmdreq.rbuf = rbuf;
	cmdreq.cb = fxn; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}
#else
u32 mdss_dsi_panel_cmd_read(struct mdss_dsi_ctrl_pdata *ctrl, char cmd0,
		char cmd1, void (*fxn)(int), char *rbuf, int len)
{
	struct dcs_cmd_req cmdreq;

	dcs_cmd[0] = cmd0;
	dcs_cmd[1] = cmd1;
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &dcs_read_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_RX | CMD_REQ_COMMIT;
	cmdreq.rlen = len;
	cmdreq.rbuf = rbuf;
	cmdreq.cb = fxn; /* call back */
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	/*
	 * blocked here, until call back called
	 */

	return 0;
}
#endif

static void mdss_dsi_panel_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
			struct dsi_panel_cmds *pcmds)
{
	struct dcs_cmd_req cmdreq;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = pcmds->cmds;
	cmdreq.cmds_cnt = pcmds->cmd_cnt;
	cmdreq.flags = CMD_REQ_COMMIT;

	/*Panel ON/Off commands should be sent in DSI Low Power Mode*/
	if (pcmds->link_state == DSI_LP_MODE)
		cmdreq.flags  |= CMD_REQ_LP_MODE;

	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}

#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
static char led_pwm1[3] = {0x51, 0x00, 0x00};	/* DTYPE_DCS_LWRITE */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_err("%s: level=%d\n", __func__, level);
#if 0//defined (CONFIG_F_SKYDISP_EF56_SS)
//PWM 22khz
	led_pwm1[1] = (unsigned char)(level >>8) & 0x0F; 
       led_pwm1[2] = (unsigned char)level & 0xFF;
#else
//PWM 10khz
       led_pwm1[2] = (unsigned char)level & 0xFF;
#endif

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

       msleep(1);
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#elif defined(CONFIG_F_SKYDISP_EF63_SS)
static void mdss_dsi_cmds_send(struct mdss_dsi_ctrl_pdata *ctrl,
				struct dcs_cmd_req * cmdreq,struct dsi_cmd_desc * cmds)
{


	memset(cmdreq, 0, sizeof(cmdreq));
	cmdreq->cmds = cmds;
	cmdreq->cmds_cnt = 1;
	cmdreq->flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq->rlen = 0;
	cmdreq->cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, cmdreq);
}

void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{

	int i;
	struct dcs_cmd_req cmdreq;
	struct tsens_device tsens_dev;
#ifdef CONFIG_F_SKYDISP_ELVSS_WORK
	int index;
	int  ret = 0;
	long temp,average_temp = 0;
	
	char G_para[2] = {0xb0, 0x05};
	char temp_set_lsi[2] = {0xb8, 0x00};	
	struct dsi_cmd_desc g_para_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(G_para)},G_para};
	struct dsi_cmd_desc temp_lsi_cmd = {{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(temp_set_lsi)},temp_set_lsi};
		
#endif	
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	if(ctrl->hbm_onoff == true)
		return;
#endif	
	if(ctrl->offline_charger == true){
		
		backlight_cmd.payload = oled_gamma;
		locking_cmd.payload = locking;
		aor_cmd.payload = aor_dim[20];
		elvss_cmd.payload = elvss_set[20];//20

	}
	else{
		backlight_cmd.payload = oled_gamma;
		locking_cmd.payload = locking;
		aor_cmd.payload = aor_dim[level];
		elvss_cmd.payload = elvss_set[level];
		
		oled_gamma[0] = 0xca;	
		oled_gamma[1] = (0x0100 & ctrl->gamma_set.gamma_table[level][0]) >> 8 ;
		oled_gamma[2] =0x0ff & ctrl->gamma_set.gamma_table[level][0] ;	
		oled_gamma[3] =  (0x0100 & ctrl->gamma_set.gamma_table[level][1]) >> 8 ;	
		oled_gamma[4] = 0x0ff & ctrl->gamma_set.gamma_table[level][1] ;
		oled_gamma[5] = (0x0100 & ctrl->gamma_set.gamma_table[level][2]) >> 8 ;
		oled_gamma[6] = 0x0ff & ctrl->gamma_set.gamma_table[level][2] ;

			
		for(i = 7; i <34;i++ ){
			if(ctrl->gamma_set.gamma_table[level][i-4] < 255){
				oled_gamma[i] = ctrl->gamma_set.gamma_table[level][i-4] ;
			}
			else{
				oled_gamma[i] =0xff; 
			}
		}
#ifdef CONFIG_F_SKYDISP_ELVSS_WORK
		for(index = 1; index <=10; index++){
			tsens_dev.sensor_num = index;
			ret = tsens_get_temp(&tsens_dev, &temp);
			if (ret) {
				pr_err(" Unable to read TSENS sensor %d\n", tsens_dev.sensor_num);
				return;
			}
			//printk("elvss_temp_work %ld\n",temp);
			average_temp += temp;
		}
		average_temp = average_temp /10;
		if(average_temp > 127)
			average_temp = 127;
		if(average_temp  < -127)
			average_temp = -127;
		
		if(average_temp <= 127 && average_temp >= 0){
			if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC)
				temp_set_lsi[1] = 0x7f & (unsigned char)average_temp;
			else
				temp_set[1] = 0x7f & (unsigned char)average_temp;
		}
		if(average_temp >= -127 && average_temp <= -1){
			if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC)
				temp_set_lsi[1] =0x80 | (~(unsigned char)average_temp);
			else	
				temp_set[1] = 0x80 | (~(unsigned char)average_temp);
		}
		if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC)
			temp_cmd.payload = temp_set_lsi;
		else
		temp_cmd.payload = temp_set;
		
#endif
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_dsi_set_tx_power_mode(0 , &ctrl->panel_data );
        msleep(2);

	//if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC)
		//mdss_dsi_cmds_send(ctrl,&cmdreq,&mtp_unlock_cmd);
	
	mdss_dsi_cmds_send(ctrl,&cmdreq,&backlight_cmd);
	mdss_dsi_cmds_send(ctrl,&cmdreq,&aor_cmd);
	mdss_dsi_cmds_send(ctrl,&cmdreq,&elvss_cmd);
	mdss_dsi_cmds_send(ctrl,&cmdreq,&locking_cmd);
#if (1) // no-feature because urgency-issue
/*
* 20140422, kkcho, Bug-Fix : Sometimes.. backlight_set fail
* Apply the below code from samsung
*/
	if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC){
		ndelay(100);
		mdss_dsi_cmds_send(ctrl,&cmdreq,&locking_disable_cmd);
	}
#endif	

#ifdef CONFIG_F_SKYDISP_ELVSS_WORK
	if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC){
		mdss_dsi_cmds_send(ctrl,&cmdreq,&g_para_cmd);
		mdss_dsi_cmds_send(ctrl,&cmdreq,&temp_lsi_cmd);
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
		if(ctrl->onflag){
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->display_on_cmds);
			ctrl->onflag = false;
		}
#endif
	}else{
		mdss_dsi_cmds_send(ctrl,&cmdreq,&temp_cmd);
	}
#endif  

	msleep(1);
	mdss_dsi_set_tx_power_mode(1 ,&ctrl->panel_data);
        mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);

	pr_err(" %d backlight level = %d AOR = 0x%x ELVSS = 0x%x TEMP = 0x%x lock = 0x%x ctrl->onflag = %d\n",
		level,gamma_level[level],aor_dim[level][4],elvss_set[level][2],temp_cmd.payload[1],locking[1],ctrl->onflag);	
#ifdef F_WA_WATCHDOG_DURING_BOOTUP
	ctrl->octa_blck_set =1;
#endif
	
//TODO
}
#else
static char led_pwm1[2] = {0x51, 0x0};	/* DTYPE_DCS_WRITE1 */
static struct dsi_cmd_desc backlight_cmd = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(led_pwm1)},
	led_pwm1
};

static void mdss_dsi_panel_bklt_dcs(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct dcs_cmd_req cmdreq;

	pr_debug("%s: level=%d\n", __func__, level);

	led_pwm1[1] = (unsigned char)level;

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &backlight_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
}
#endif

int mdss_dsi_panel_reset(struct mdss_panel_data *pdata, int enable)
{
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) ||defined (CONFIG_F_SKYDISP_EF60_SS) ||(defined (CONFIG_F_SKYDISP_EF63_SS) && (CONFIG_BOARD_VER <= CONFIG_PT20)) 
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->bl_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}
	if (!gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en_gpio)) {
		pr_debug("%s:%d, lcd vcip line not configured\n",
			   __func__, __LINE__);
		return rc;
	}
	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		if (gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcip_reg_en_gpio), 1);
              msleep(5);
#if defined (CONFIG_F_SKYDISP_EF56_SS)			  
		if (gpio_is_valid(ctrl_pdata->lcd_vcin_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcin_reg_en_gpio), 1);
              msleep(3);	
#endif			  
			//gpio_set_value((ctrl_pdata->rst_gpio),1);
              	//msleep(10); 
			//gpio_set_value((ctrl_pdata->rst_gpio),0);
			msleep(20);
			gpio_set_value((ctrl_pdata->rst_gpio),1);
              msleep(10); 
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		msleep(5);
#if defined (CONFIG_F_SKYDISP_EF56_SS)		
		if (gpio_is_valid(ctrl_pdata->lcd_vcin_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcin_reg_en_gpio), 0);	
		msleep(3);
#endif
		if (gpio_is_valid(ctrl_pdata->lcd_vcip_reg_en_gpio))
			gpio_set_value((ctrl_pdata->lcd_vcip_reg_en_gpio), 0);
		msleep(100);
	}
#elif (defined (CONFIG_F_SKYDISP_EF63_SS) && (CONFIG_BOARD_VER >= CONFIG_WS10))
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->octa_rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
#if 0		
		gpio_set_value((ctrl->octa_rst_gpio), 1);
		msleep(10);
		gpio_set_value((ctrl->octa_rst_gpio), 0);
              msleep(10); 
		gpio_set_value((ctrl->octa_rst_gpio), 1);
		msleep(10);
#endif		
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->octa_rst_gpio), 0);
	}
#else  //QUALCOMM default
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mdss_panel_info *pinfo = NULL;
	int i, rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return rc;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);
	pinfo = &(ctrl_pdata->panel_data.panel_info);

	if (enable) {
		rc = mdss_dsi_request_gpios(ctrl_pdata);
		if (rc) {
			pr_err("gpio request failed\n");
			return rc;
		}
		if (!pinfo->panel_power_on) {
			if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
				gpio_set_value((ctrl_pdata->disp_en_gpio), 1);

			for (i = 0; i < pdata->panel_info.rst_seq_len; ++i) {
				gpio_set_value((ctrl_pdata->rst_gpio),
					pdata->panel_info.rst_seq[i]);
				if (pdata->panel_info.rst_seq[++i])
					usleep(pinfo->rst_seq[i] * 1000);
			}
		}

		if (gpio_is_valid(ctrl_pdata->mode_gpio)) {
			if (pinfo->mode_gpio_state == MODE_GPIO_HIGH)
				gpio_set_value((ctrl_pdata->mode_gpio), 1);
			else if (pinfo->mode_gpio_state == MODE_GPIO_LOW)
				gpio_set_value((ctrl_pdata->mode_gpio), 0);
		}
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		if (gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
			gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
			gpio_free(ctrl_pdata->disp_en_gpio);
		}
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		gpio_free(ctrl_pdata->rst_gpio);
		if (gpio_is_valid(ctrl_pdata->mode_gpio))
			gpio_free(ctrl_pdata->mode_gpio);
	}
#endif
	return rc;
}

static char caset[] = {0x2a, 0x00, 0x00, 0x03, 0x00};	/* DTYPE_DCS_LWRITE */
static char paset[] = {0x2b, 0x00, 0x00, 0x05, 0x00};	/* DTYPE_DCS_LWRITE */

static struct dsi_cmd_desc partial_update_enable_cmd[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(caset)}, caset},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(paset)}, paset},
};

static int mdss_dsi_panel_partial_update(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct dcs_cmd_req cmdreq;
	int rc = 0;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	caset[1] = (((pdata->panel_info.roi_x) & 0xFF00) >> 8);
	caset[2] = (((pdata->panel_info.roi_x) & 0xFF));
	caset[3] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF00) >> 8);
	caset[4] = (((pdata->panel_info.roi_x - 1 + pdata->panel_info.roi_w)
								& 0xFF));
	partial_update_enable_cmd[0].payload = caset;

	paset[1] = (((pdata->panel_info.roi_y) & 0xFF00) >> 8);
	paset[2] = (((pdata->panel_info.roi_y) & 0xFF));
	paset[3] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF00) >> 8);
	paset[4] = (((pdata->panel_info.roi_y - 1 + pdata->panel_info.roi_h)
								& 0xFF));
	partial_update_enable_cmd[1].payload = paset;

	pr_debug("%s: enabling partial update\n", __func__);
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = partial_update_enable_cmd;
	cmdreq.cmds_cnt = 2;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_dsi_cmdlist_put(ctrl, &cmdreq);

	return rc;
}

static void mdss_dsi_panel_switch_mode(struct mdss_panel_data *pdata,
							int mode)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct mipi_panel_info *mipi;
	struct dsi_panel_cmds *pcmds;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	mipi  = &pdata->panel_info.mipi;

	if (!mipi->dynamic_switch_enabled)
		return;

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (mode == DSI_CMD_MODE)
		pcmds = &ctrl_pdata->video2cmd;
	else
		pcmds = &ctrl_pdata->cmd2video;

	mdss_dsi_panel_cmds_send(ctrl_pdata, pcmds);

	return;
}

#ifdef CONFIG_F_SKYDISP_CABC_CONTROL
void cabc_control(struct mdss_panel_data *pdata, int state)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,panel_data);
#if defined (CONFIG_F_SKYDISP_EF56_SS) ||defined (CONFIG_F_SKYDISP_EF60_SS)
	if(state == 1){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x01;	
		ctrl_pdata->on_cmds.buf[207] = 0x01;
	}else if(state == 0){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x00;
		ctrl_pdata->on_cmds.buf[207] = 0x00;
	}else { //for err
		*(ctrl_pdata->cabc_cmds.cmds->payload+ 1) = 0x01;
		ctrl_pdata->on_cmds.buf[207] = 0x01;
	}	
#else
	if(state == 1){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x03;	
		ctrl_pdata->on_cmds.buf[178] = 0x03;
	}else if(state == 0){
		*(ctrl_pdata->cabc_cmds.cmds->payload + 1) = 0x00;
		ctrl_pdata->on_cmds.buf[178] = 0x00;
	}else { //for err
		*(ctrl_pdata->cabc_cmds.cmds->payload+ 1) = 0x03;
		ctrl_pdata->on_cmds.buf[178] = 0x03;
	}	
#endif
	mdss_dsi_panel_cmds_send(ctrl_pdata, &ctrl_pdata->cabc_cmds);
}
#endif

#ifdef CONFIG_F_SKYDISP_AMOLED_READ_DATA
void mtp_read_genernal(int data,char * read_buf)
{
	struct msm_fb_data_type *mfd = mfdmsm_fb_get_mfd();
	struct mdss_panel_info *panel_info = mfd->panel_info;
	struct mdss_panel_data * pdata =NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int index = 0;

	pdata = container_of(panel_info, struct mdss_panel_data,
				panel_info);
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	for(index = 0; index < data; index++){
		printk("0x%x ",*(read_buf));
		(read_buf)++;
	}
	printk("\n");
	
}
void pannel_read(struct mdss_panel_data *pdata, int state)
{
	struct msm_fb_data_type *mfd = mfdmsm_fb_get_mfd();
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;
	struct dcs_cmd_req cmdreq;
	int i;
	char mtp_write_data[2] = {0xb0, 0x00};

	
	struct dsi_cmd_desc mtp_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(mtp_write_data)},mtp_write_data};
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &mtp_data_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;



	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	

	printk("---------------Gamma = %d-----------------\n",gamma_level[mfd->bl_level]);	
	printk("---------------CA register--------------------\n");
	
	for(i = 0; i < 4; i++){
		*(cmdreq.cmds->payload + 1) = i*8;
		mdss_dsi_cmdlist_put(ctrl, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl,0xca,0x00,mtp_read_genernal,ctrl->rx_buf.data,8);
	
	}	

	*(cmdreq.cmds->payload + 1) = 32;
	mdss_dsi_cmdlist_put(ctrl, &cmdreq);
	mdss_dsi_panel_cmd_read(ctrl,0xca,0x00,mtp_read_genernal,ctrl->rx_buf.data,1);
		
	printk("---------------C8 register--------------------\n");

	for(i = 0; i < 12; i++){
		*(cmdreq.cmds->payload + 1) = i*8;
		mdss_dsi_cmdlist_put(ctrl, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl,0xc8,0x00,mtp_read_genernal,ctrl->rx_buf.data,8);
	
	}		
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, true);

}
#endif
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
void mtp_read_hbm_elvss(int data,char * read_buf)
{
	int index = 0;
	static int index_cnt = 1;

	for(index = 0; index < data; index++){
		elvss_hbm_cmd_backup[index_cnt] = *(read_buf);
		(read_buf)++;
		index_cnt++;
	}
	

}
void mtp_read_hbm(int data,char * read_buf)
{

	int index = 0;
	static int index_cnt = 1;
	static int first_entry = true;

	for(index = 0; index < data; index++){
		if(index_cnt == 7 && first_entry){
			elvss_hbm_cmd[17] = *(read_buf);
			index_cnt = index_cnt-1; 
			first_entry = false;
		}else{
			hbm_gamma[index_cnt] = *(read_buf);
		}
		(read_buf)++;
		index_cnt++;
	}
	

	
}
void acl_control(struct mdss_panel_data *pdata, int state)
{
	struct dcs_cmd_req cmdreq;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	char acl_data[2] = {0x55, 0x01};
	struct dsi_cmd_desc acl_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(acl_data)},acl_data};
	
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);


	if(state == 0){		
		acl_data[1] = 0x00; 
	}
	else if(state == 1)
	{
		acl_data[1] = 0x01;
	}
	else if(state == 2)
	{
		acl_data[1] = 0x02;
	}
	else if(state == 3)
	{
		acl_data[1] = 0x03;
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_dsi_set_tx_power_mode(0 , pdata );
	
	mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&acl_data_cmd);
	
	mdss_dsi_set_tx_power_mode(1 , pdata );
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);	
	pr_info("Oled acl  = %d \n",state );
}
void hbm_control_magna(struct mdss_panel_data *pdata, int state)
{

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;
	struct msm_fb_data_type * mfd = mfdmsm_fb_get_mfd();

	char hbm_data[2] = {0x53, 0xD0};
	char acl_data[2] = {0x55, 0x00};	
	struct dsi_cmd_desc hbm_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(hbm_data)},hbm_data};
	struct dsi_cmd_desc acl_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(acl_data)},acl_data};

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	
	if(!mfd->panel_power_on)
	{
		printk("[%s] panel is off state (%d).....\n",__func__,mfd->panel_power_on);
		return;
	}
	
	//if(pdata->hbm_flag == state)
		//return;
	
	if(state == 0){
		hbm_data[1] = 0x00;
		acl_data[1] = 0x01; 
		ctrl_pdata->hbm_onoff = false;
	}
	else
	{//off
		hbm_data[1] = 0xD0;
		acl_data[1] = 0x02;
		ctrl_pdata->hbm_onoff = true;
	}

	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &hbm_data_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_dsi_set_tx_power_mode(0 , pdata );
	
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	
	cmdreq.cmds = &acl_data_cmd;
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	
	mdss_dsi_set_tx_power_mode(1 , pdata );
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);
	//pdata->hbm_flag = state;
	pr_info("Oled hbm %s \n",state ? "on" : "off" );

}

void hbm_control_ddi(struct mdss_panel_data *pdata, int state)
{

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dcs_cmd_req cmdreq;
	struct msm_fb_data_type * mfd = mfdmsm_fb_get_mfd();
	
	char aor_hbm_cmd[5] = {0xB2,0x00,0x08,0x00,0x08};	
	char acl_hbm_cmd[2] = {0x55, 0x01};	
	
	struct dsi_cmd_desc hbm_gamma_cmd = {{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(hbm_gamma)},hbm_gamma};	
	struct dsi_cmd_desc hbm_aor_cmd = {{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(aor_hbm_cmd)},aor_hbm_cmd};	
	struct dsi_cmd_desc hbm_elvss_cmd = {{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(elvss_hbm_cmd)},elvss_hbm_cmd};	
	struct dsi_cmd_desc hbm_acl_cmd = {{DTYPE_DCS_LWRITE, 1, 0, 0, 1, sizeof(acl_hbm_cmd)},acl_hbm_cmd};	
	
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
#ifdef CONFIG_F_SKYDISP_EF63_DRIVER_IC_CHECK
	if(ctrl_pdata->manufacture_id_rev == NEW_REV){
		aor_hbm_cmd[2] = 0x0e;
		aor_hbm_cmd[4] = 0x0e;
	}
#endif	
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	
	if(!mfd->panel_power_on)
	{
		printk("[%s] panel is off state (%d).....\n",__func__,mfd->panel_power_on);
		return;
	}
	
	//if(pdata->hbm_flag == state)
		//return;

	
	
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_dsi_set_tx_power_mode(0 , &ctrl_pdata->panel_data );
	
	if(state == 1){
		//mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&mtp_unlock_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&hbm_gamma_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&hbm_aor_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&hbm_elvss_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&hbm_acl_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&locking_cmd);
		ctrl_pdata->hbm_onoff = true;
	}else
	{

		aor_cmd.payload = aor_dim[mfd->bl_level];	

		memcpy(elvss_hbm_cmd,elvss_hbm_cmd_backup,sizeof(elvss_hbm_cmd));
		
		elvss_hbm_cmd[1] = 	elvss_set[mfd->bl_level][1];
		elvss_hbm_cmd[2] =  elvss_set[mfd->bl_level][2];
		acl_hbm_cmd[1] = 0x01;
		//mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&mtp_unlock_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&backlight_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&aor_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&hbm_elvss_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&hbm_acl_cmd);
		mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&locking_cmd);
		ctrl_pdata->hbm_onoff = false;
	}	
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, true);
	mdss_dsi_set_tx_power_mode(1 , &ctrl_pdata->panel_data );
	//pdata->hbm_flag = state;
	pr_info("Oled hbm %s \n",state ? "on" : "off" );
}

#endif
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
void panel_gamma_sort(struct mdss_dsi_ctrl_pdata *pdata)
{
	if(pdata == NULL)
		return;

	/* 
	 * Manual Gamma Table
	 */
	printk("SHMDSSDBG: Setting OLED gamma\n");

	pdata->gamma_set.gamma_table[0][0] = 0xae;
	pdata->gamma_set.gamma_table[0][1] = 0xc2;
	pdata->gamma_set.gamma_table[0][2] = 0xaa;
	pdata->gamma_set.gamma_table[0][3] = 0x8a;
	pdata->gamma_set.gamma_table[0][4] = 0x90;
	pdata->gamma_set.gamma_table[0][5] = 0x8b;
	pdata->gamma_set.gamma_table[0][6] = 0x8b;
	pdata->gamma_set.gamma_table[0][7] = 0x90;
	pdata->gamma_set.gamma_table[0][8] = 0x8a;
	pdata->gamma_set.gamma_table[0][9] = 0x90;
	pdata->gamma_set.gamma_table[0][10] = 0x9a;
	pdata->gamma_set.gamma_table[0][11] = 0x8f;
	pdata->gamma_set.gamma_table[0][12] = 0x90;
	pdata->gamma_set.gamma_table[0][13] = 0x9b;
	pdata->gamma_set.gamma_table[0][14] = 0x8e;
	pdata->gamma_set.gamma_table[0][15] = 0x8d;
	pdata->gamma_set.gamma_table[0][16] = 0x96;
	pdata->gamma_set.gamma_table[0][17] = 0x88;
	pdata->gamma_set.gamma_table[0][18] = 0x92;
	pdata->gamma_set.gamma_table[0][19] = 0x98;
	pdata->gamma_set.gamma_table[0][20] = 0x8f;
	pdata->gamma_set.gamma_table[0][21] = 0xa9;
	pdata->gamma_set.gamma_table[0][22] = 0xa2;
	pdata->gamma_set.gamma_table[0][23] = 0xab;
	pdata->gamma_set.gamma_table[0][24] = 0xb7;
	pdata->gamma_set.gamma_table[0][25] = 0xa0;
	pdata->gamma_set.gamma_table[0][26] = 0xbb;
	pdata->gamma_set.gamma_table[0][27] = 0x0;
	pdata->gamma_set.gamma_table[0][28] = 0x0;
	pdata->gamma_set.gamma_table[0][29] = 0x0;
	pdata->gamma_set.gamma_table[1][0] = 0xb4;
	pdata->gamma_set.gamma_table[1][1] = 0xc1;
	pdata->gamma_set.gamma_table[1][2] = 0xac;
	pdata->gamma_set.gamma_table[1][3] = 0x89;
	pdata->gamma_set.gamma_table[1][4] = 0x8d;
	pdata->gamma_set.gamma_table[1][5] = 0x8b;
	pdata->gamma_set.gamma_table[1][6] = 0x8b;
	pdata->gamma_set.gamma_table[1][7] = 0x8e;
	pdata->gamma_set.gamma_table[1][8] = 0x8a;
	pdata->gamma_set.gamma_table[1][9] = 0x8c;
	pdata->gamma_set.gamma_table[1][10] = 0x95;
	pdata->gamma_set.gamma_table[1][11] = 0x8e;
	pdata->gamma_set.gamma_table[1][12] = 0x8d;
	pdata->gamma_set.gamma_table[1][13] = 0x96;
	pdata->gamma_set.gamma_table[1][14] = 0x8d;
	pdata->gamma_set.gamma_table[1][15] = 0x89;
	pdata->gamma_set.gamma_table[1][16] = 0x91;
	pdata->gamma_set.gamma_table[1][17] = 0x85;
	pdata->gamma_set.gamma_table[1][18] = 0x8f;
	pdata->gamma_set.gamma_table[1][19] = 0x95;
	pdata->gamma_set.gamma_table[1][20] = 0x8e;
	pdata->gamma_set.gamma_table[1][21] = 0xa7;
	pdata->gamma_set.gamma_table[1][22] = 0xa2;
	pdata->gamma_set.gamma_table[1][23] = 0xa4;
	pdata->gamma_set.gamma_table[1][24] = 0xa6;
	pdata->gamma_set.gamma_table[1][25] = 0x93;
	pdata->gamma_set.gamma_table[1][26] = 0xab;
	pdata->gamma_set.gamma_table[1][27] = 0x0;
	pdata->gamma_set.gamma_table[1][28] = 0x0;
	pdata->gamma_set.gamma_table[1][29] = 0x0;
	pdata->gamma_set.gamma_table[2][0] = 0xb4;
	pdata->gamma_set.gamma_table[2][1] = 0xc1;
	pdata->gamma_set.gamma_table[2][2] = 0xac;
	pdata->gamma_set.gamma_table[2][3] = 0x8a;
	pdata->gamma_set.gamma_table[2][4] = 0x8c;
	pdata->gamma_set.gamma_table[2][5] = 0x8b;
	pdata->gamma_set.gamma_table[2][6] = 0x8a;
	pdata->gamma_set.gamma_table[2][7] = 0x8c;
	pdata->gamma_set.gamma_table[2][8] = 0x8a;
	pdata->gamma_set.gamma_table[2][9] = 0x8a;
	pdata->gamma_set.gamma_table[2][10] = 0x8f;
	pdata->gamma_set.gamma_table[2][11] = 0x8b;
	pdata->gamma_set.gamma_table[2][12] = 0x8a;
	pdata->gamma_set.gamma_table[2][13] = 0x91;
	pdata->gamma_set.gamma_table[2][14] = 0x8c;
	pdata->gamma_set.gamma_table[2][15] = 0x87;
	pdata->gamma_set.gamma_table[2][16] = 0x8f;
	pdata->gamma_set.gamma_table[2][17] = 0x85;
	pdata->gamma_set.gamma_table[2][18] = 0x8c;
	pdata->gamma_set.gamma_table[2][19] = 0x91;
	pdata->gamma_set.gamma_table[2][20] = 0x87;
	pdata->gamma_set.gamma_table[2][21] = 0xa0;
	pdata->gamma_set.gamma_table[2][22] = 0x9d;
	pdata->gamma_set.gamma_table[2][23] = 0x9a;
	pdata->gamma_set.gamma_table[2][24] = 0xa1;
	pdata->gamma_set.gamma_table[2][25] = 0x92;
	pdata->gamma_set.gamma_table[2][26] = 0xa3;
	pdata->gamma_set.gamma_table[2][27] = 0x0;
	pdata->gamma_set.gamma_table[2][28] = 0x0;
	pdata->gamma_set.gamma_table[2][29] = 0x0;
	pdata->gamma_set.gamma_table[3][0] = 0xb4;
	pdata->gamma_set.gamma_table[3][1] = 0xc1;
	pdata->gamma_set.gamma_table[3][2] = 0xac;
	pdata->gamma_set.gamma_table[3][3] = 0x8a;
	pdata->gamma_set.gamma_table[3][4] = 0x8b;
	pdata->gamma_set.gamma_table[3][5] = 0x8b;
	pdata->gamma_set.gamma_table[3][6] = 0x8a;
	pdata->gamma_set.gamma_table[3][7] = 0x8b;
	pdata->gamma_set.gamma_table[3][8] = 0x8a;
	pdata->gamma_set.gamma_table[3][9] = 0x88;
	pdata->gamma_set.gamma_table[3][10] = 0x8c;
	pdata->gamma_set.gamma_table[3][11] = 0x8a;
	pdata->gamma_set.gamma_table[3][12] = 0x8a;
	pdata->gamma_set.gamma_table[3][13] = 0x8f;
	pdata->gamma_set.gamma_table[3][14] = 0x8c;
	pdata->gamma_set.gamma_table[3][15] = 0x85;
	pdata->gamma_set.gamma_table[3][16] = 0x8c;
	pdata->gamma_set.gamma_table[3][17] = 0x82;
	pdata->gamma_set.gamma_table[3][18] = 0x8a;
	pdata->gamma_set.gamma_table[3][19] = 0x8e;
	pdata->gamma_set.gamma_table[3][20] = 0x84;
	pdata->gamma_set.gamma_table[3][21] = 0x9d;
	pdata->gamma_set.gamma_table[3][22] = 0x9d;
	pdata->gamma_set.gamma_table[3][23] = 0x97;
	pdata->gamma_set.gamma_table[3][24] = 0x95;
	pdata->gamma_set.gamma_table[3][25] = 0x8a;
	pdata->gamma_set.gamma_table[3][26] = 0x96;
	pdata->gamma_set.gamma_table[3][27] = 0x0;
	pdata->gamma_set.gamma_table[3][28] = 0x0;
	pdata->gamma_set.gamma_table[3][29] = 0x0;
	pdata->gamma_set.gamma_table[4][0] = 0xb4;
	pdata->gamma_set.gamma_table[4][1] = 0xc1;
	pdata->gamma_set.gamma_table[4][2] = 0xac;
	pdata->gamma_set.gamma_table[4][3] = 0x8a;
	pdata->gamma_set.gamma_table[4][4] = 0x8b;
	pdata->gamma_set.gamma_table[4][5] = 0x8b;
	pdata->gamma_set.gamma_table[4][6] = 0x89;
	pdata->gamma_set.gamma_table[4][7] = 0x8a;
	pdata->gamma_set.gamma_table[4][8] = 0x89;
	pdata->gamma_set.gamma_table[4][9] = 0x88;
	pdata->gamma_set.gamma_table[4][10] = 0x8c;
	pdata->gamma_set.gamma_table[4][11] = 0x8a;
	pdata->gamma_set.gamma_table[4][12] = 0x86;
	pdata->gamma_set.gamma_table[4][13] = 0x8a;
	pdata->gamma_set.gamma_table[4][14] = 0x8a;
	pdata->gamma_set.gamma_table[4][15] = 0x89;
	pdata->gamma_set.gamma_table[4][16] = 0x8c;
	pdata->gamma_set.gamma_table[4][17] = 0x83;
	pdata->gamma_set.gamma_table[4][18] = 0x84;
	pdata->gamma_set.gamma_table[4][19] = 0x8a;
	pdata->gamma_set.gamma_table[4][20] = 0x80;
	pdata->gamma_set.gamma_table[4][21] = 0x99;
	pdata->gamma_set.gamma_table[4][22] = 0x99;
	pdata->gamma_set.gamma_table[4][23] = 0x92;
	pdata->gamma_set.gamma_table[4][24] = 0x99;
	pdata->gamma_set.gamma_table[4][25] = 0x8e;
	pdata->gamma_set.gamma_table[4][26] = 0x9b;
	pdata->gamma_set.gamma_table[4][27] = 0x0;
	pdata->gamma_set.gamma_table[4][28] = 0x0;
	pdata->gamma_set.gamma_table[4][29] = 0x0;
	pdata->gamma_set.gamma_table[5][0] = 0xb4;
	pdata->gamma_set.gamma_table[5][1] = 0xc1;
	pdata->gamma_set.gamma_table[5][2] = 0xac;
	pdata->gamma_set.gamma_table[5][3] = 0x89;
	pdata->gamma_set.gamma_table[5][4] = 0x8a;
	pdata->gamma_set.gamma_table[5][5] = 0x8a;
	pdata->gamma_set.gamma_table[5][6] = 0x8a;
	pdata->gamma_set.gamma_table[5][7] = 0x8b;
	pdata->gamma_set.gamma_table[5][8] = 0x8a;
	pdata->gamma_set.gamma_table[5][9] = 0x88;
	pdata->gamma_set.gamma_table[5][10] = 0x8b;
	pdata->gamma_set.gamma_table[5][11] = 0x8a;
	pdata->gamma_set.gamma_table[5][12] = 0x87;
	pdata->gamma_set.gamma_table[5][13] = 0x89;
	pdata->gamma_set.gamma_table[5][14] = 0x8b;
	pdata->gamma_set.gamma_table[5][15] = 0x85;
	pdata->gamma_set.gamma_table[5][16] = 0x89;
	pdata->gamma_set.gamma_table[5][17] = 0x80;
	pdata->gamma_set.gamma_table[5][18] = 0x86;
	pdata->gamma_set.gamma_table[5][19] = 0x8b;
	pdata->gamma_set.gamma_table[5][20] = 0x82;
	pdata->gamma_set.gamma_table[5][21] = 0x99;
	pdata->gamma_set.gamma_table[5][22] = 0x98;
	pdata->gamma_set.gamma_table[5][23] = 0x94;
	pdata->gamma_set.gamma_table[5][24] = 0x8e;
	pdata->gamma_set.gamma_table[5][25] = 0x87;
	pdata->gamma_set.gamma_table[5][26] = 0x8f;
	pdata->gamma_set.gamma_table[5][27] = 0x0;
	pdata->gamma_set.gamma_table[5][28] = 0x0;
	pdata->gamma_set.gamma_table[5][29] = 0x0;
	pdata->gamma_set.gamma_table[6][0] = 0xb5;
	pdata->gamma_set.gamma_table[6][1] = 0xc1;
	pdata->gamma_set.gamma_table[6][2] = 0xac;
	pdata->gamma_set.gamma_table[6][3] = 0x88;
	pdata->gamma_set.gamma_table[6][4] = 0x8a;
	pdata->gamma_set.gamma_table[6][5] = 0x8a;
	pdata->gamma_set.gamma_table[6][6] = 0x89;
	pdata->gamma_set.gamma_table[6][7] = 0x8a;
	pdata->gamma_set.gamma_table[6][8] = 0x89;
	pdata->gamma_set.gamma_table[6][9] = 0x87;
	pdata->gamma_set.gamma_table[6][10] = 0x8a;
	pdata->gamma_set.gamma_table[6][11] = 0x8a;
	pdata->gamma_set.gamma_table[6][12] = 0x87;
	pdata->gamma_set.gamma_table[6][13] = 0x88;
	pdata->gamma_set.gamma_table[6][14] = 0x8b;
	pdata->gamma_set.gamma_table[6][15] = 0x85;
	pdata->gamma_set.gamma_table[6][16] = 0x89;
	pdata->gamma_set.gamma_table[6][17] = 0x7f;
	pdata->gamma_set.gamma_table[6][18] = 0x85;
	pdata->gamma_set.gamma_table[6][19] = 0x8a;
	pdata->gamma_set.gamma_table[6][20] = 0x81;
	pdata->gamma_set.gamma_table[6][21] = 0x94;
	pdata->gamma_set.gamma_table[6][22] = 0x95;
	pdata->gamma_set.gamma_table[6][23] = 0x8c;
	pdata->gamma_set.gamma_table[6][24] = 0x9e;
	pdata->gamma_set.gamma_table[6][25] = 0x92;
	pdata->gamma_set.gamma_table[6][26] = 0xa0;
	pdata->gamma_set.gamma_table[6][27] = 0x0;
	pdata->gamma_set.gamma_table[6][28] = 0x0;
	pdata->gamma_set.gamma_table[6][29] = 0x0;
	pdata->gamma_set.gamma_table[7][0] = 0xb5;
	pdata->gamma_set.gamma_table[7][1] = 0xc1;
	pdata->gamma_set.gamma_table[7][2] = 0xac;
	pdata->gamma_set.gamma_table[7][3] = 0x88;
	pdata->gamma_set.gamma_table[7][4] = 0x8a;
	pdata->gamma_set.gamma_table[7][5] = 0x8a;
	pdata->gamma_set.gamma_table[7][6] = 0x89;
	pdata->gamma_set.gamma_table[7][7] = 0x8a;
	pdata->gamma_set.gamma_table[7][8] = 0x89;
	pdata->gamma_set.gamma_table[7][9] = 0x87;
	pdata->gamma_set.gamma_table[7][10] = 0x8a;
	pdata->gamma_set.gamma_table[7][11] = 0x8a;
	pdata->gamma_set.gamma_table[7][12] = 0x86;
	pdata->gamma_set.gamma_table[7][13] = 0x87;
	pdata->gamma_set.gamma_table[7][14] = 0x8b;
	pdata->gamma_set.gamma_table[7][15] = 0x88;
	pdata->gamma_set.gamma_table[7][16] = 0x8b;
	pdata->gamma_set.gamma_table[7][17] = 0x82;
	pdata->gamma_set.gamma_table[7][18] = 0x80;
	pdata->gamma_set.gamma_table[7][19] = 0x87;
	pdata->gamma_set.gamma_table[7][20] = 0x7c;
	pdata->gamma_set.gamma_table[7][21] = 0x99;
	pdata->gamma_set.gamma_table[7][22] = 0x98;
	pdata->gamma_set.gamma_table[7][23] = 0x93;
	pdata->gamma_set.gamma_table[7][24] = 0x80;
	pdata->gamma_set.gamma_table[7][25] = 0x7f;
	pdata->gamma_set.gamma_table[7][26] = 0x80;
	pdata->gamma_set.gamma_table[7][27] = 0x0;
	pdata->gamma_set.gamma_table[7][28] = 0x0;
	pdata->gamma_set.gamma_table[7][29] = 0x0;
	pdata->gamma_set.gamma_table[8][0] = 0xbb;
	pdata->gamma_set.gamma_table[8][1] = 0xc6;
	pdata->gamma_set.gamma_table[8][2] = 0xb2;
	pdata->gamma_set.gamma_table[8][3] = 0x87;
	pdata->gamma_set.gamma_table[8][4] = 0x89;
	pdata->gamma_set.gamma_table[8][5] = 0x88;
	pdata->gamma_set.gamma_table[8][6] = 0x8b;
	pdata->gamma_set.gamma_table[8][7] = 0x89;
	pdata->gamma_set.gamma_table[8][8] = 0x8a;
	pdata->gamma_set.gamma_table[8][9] = 0x86;
	pdata->gamma_set.gamma_table[8][10] = 0x88;
	pdata->gamma_set.gamma_table[8][11] = 0x88;
	pdata->gamma_set.gamma_table[8][12] = 0x87;
	pdata->gamma_set.gamma_table[8][13] = 0x88;
	pdata->gamma_set.gamma_table[8][14] = 0x8a;
	pdata->gamma_set.gamma_table[8][15] = 0x83;
	pdata->gamma_set.gamma_table[8][16] = 0x86;
	pdata->gamma_set.gamma_table[8][17] = 0x7e;
	pdata->gamma_set.gamma_table[8][18] = 0x85;
	pdata->gamma_set.gamma_table[8][19] = 0x89;
	pdata->gamma_set.gamma_table[8][20] = 0x81;
	pdata->gamma_set.gamma_table[8][21] = 0x94;
	pdata->gamma_set.gamma_table[8][22] = 0x95;
	pdata->gamma_set.gamma_table[8][23] = 0x8e;
	pdata->gamma_set.gamma_table[8][24] = 0x80;
	pdata->gamma_set.gamma_table[8][25] = 0x7f;
	pdata->gamma_set.gamma_table[8][26] = 0x80;
	pdata->gamma_set.gamma_table[8][27] = 0x0;
	pdata->gamma_set.gamma_table[8][28] = 0x0;
	pdata->gamma_set.gamma_table[8][29] = 0x0;
	pdata->gamma_set.gamma_table[9][0] = 0xc1;
	pdata->gamma_set.gamma_table[9][1] = 0xca;
	pdata->gamma_set.gamma_table[9][2] = 0xbb;
	pdata->gamma_set.gamma_table[9][3] = 0x86;
	pdata->gamma_set.gamma_table[9][4] = 0x88;
	pdata->gamma_set.gamma_table[9][5] = 0x87;
	pdata->gamma_set.gamma_table[9][6] = 0x89;
	pdata->gamma_set.gamma_table[9][7] = 0x89;
	pdata->gamma_set.gamma_table[9][8] = 0x89;
	pdata->gamma_set.gamma_table[9][9] = 0x86;
	pdata->gamma_set.gamma_table[9][10] = 0x88;
	pdata->gamma_set.gamma_table[9][11] = 0x88;
	pdata->gamma_set.gamma_table[9][12] = 0x86;
	pdata->gamma_set.gamma_table[9][13] = 0x86;
	pdata->gamma_set.gamma_table[9][14] = 0x89;
	pdata->gamma_set.gamma_table[9][15] = 0x83;
	pdata->gamma_set.gamma_table[9][16] = 0x86;
	pdata->gamma_set.gamma_table[9][17] = 0x80;
	pdata->gamma_set.gamma_table[9][18] = 0x82;
	pdata->gamma_set.gamma_table[9][19] = 0x87;
	pdata->gamma_set.gamma_table[9][20] = 0x7e;
	pdata->gamma_set.gamma_table[9][21] = 0x94;
	pdata->gamma_set.gamma_table[9][22] = 0x94;
	pdata->gamma_set.gamma_table[9][23] = 0x8e;
	pdata->gamma_set.gamma_table[9][24] = 0x87;
	pdata->gamma_set.gamma_table[9][25] = 0x83;
	pdata->gamma_set.gamma_table[9][26] = 0x88;
	pdata->gamma_set.gamma_table[9][27] = 0x0;
	pdata->gamma_set.gamma_table[9][28] = 0x0;
	pdata->gamma_set.gamma_table[9][29] = 0x0;
	pdata->gamma_set.gamma_table[10][0] = 0xc5;
	pdata->gamma_set.gamma_table[10][1] = 0xcf;
	pdata->gamma_set.gamma_table[10][2] = 0xc0;
	pdata->gamma_set.gamma_table[10][3] = 0x87;
	pdata->gamma_set.gamma_table[10][4] = 0x8a;
	pdata->gamma_set.gamma_table[10][5] = 0x88;
	pdata->gamma_set.gamma_table[10][6] = 0x87;
	pdata->gamma_set.gamma_table[10][7] = 0x86;
	pdata->gamma_set.gamma_table[10][8] = 0x86;
	pdata->gamma_set.gamma_table[10][9] = 0x85;
	pdata->gamma_set.gamma_table[10][10] = 0x88;
	pdata->gamma_set.gamma_table[10][11] = 0x87;
	pdata->gamma_set.gamma_table[10][12] = 0x87;
	pdata->gamma_set.gamma_table[10][13] = 0x87;
	pdata->gamma_set.gamma_table[10][14] = 0x89;
	pdata->gamma_set.gamma_table[10][15] = 0x83;
	pdata->gamma_set.gamma_table[10][16] = 0x85;
	pdata->gamma_set.gamma_table[10][17] = 0x81;
	pdata->gamma_set.gamma_table[10][18] = 0x82;
	pdata->gamma_set.gamma_table[10][19] = 0x84;
	pdata->gamma_set.gamma_table[10][20] = 0x7f;
	pdata->gamma_set.gamma_table[10][21] = 0x8f;
	pdata->gamma_set.gamma_table[10][22] = 0x90;
	pdata->gamma_set.gamma_table[10][23] = 0x8a;
	pdata->gamma_set.gamma_table[10][24] = 0x8f;
	pdata->gamma_set.gamma_table[10][25] = 0x88;
	pdata->gamma_set.gamma_table[10][26] = 0x90;
	pdata->gamma_set.gamma_table[10][27] = 0x0;
	pdata->gamma_set.gamma_table[10][28] = 0x0;
	pdata->gamma_set.gamma_table[10][29] = 0x0;
	pdata->gamma_set.gamma_table[11][0] = 0xca;
	pdata->gamma_set.gamma_table[11][1] = 0xd2;
	pdata->gamma_set.gamma_table[11][2] = 0xc5;
	pdata->gamma_set.gamma_table[11][3] = 0x85;
	pdata->gamma_set.gamma_table[11][4] = 0x87;
	pdata->gamma_set.gamma_table[11][5] = 0x87;
	pdata->gamma_set.gamma_table[11][6] = 0x87;
	pdata->gamma_set.gamma_table[11][7] = 0x88;
	pdata->gamma_set.gamma_table[11][8] = 0x87;
	pdata->gamma_set.gamma_table[11][9] = 0x86;
	pdata->gamma_set.gamma_table[11][10] = 0x87;
	pdata->gamma_set.gamma_table[11][11] = 0x87;
	pdata->gamma_set.gamma_table[11][12] = 0x85;
	pdata->gamma_set.gamma_table[11][13] = 0x85;
	pdata->gamma_set.gamma_table[11][14] = 0x87;
	pdata->gamma_set.gamma_table[11][15] = 0x83;
	pdata->gamma_set.gamma_table[11][16] = 0x86;
	pdata->gamma_set.gamma_table[11][17] = 0x80;
	pdata->gamma_set.gamma_table[11][18] = 0x81;
	pdata->gamma_set.gamma_table[11][19] = 0x85;
	pdata->gamma_set.gamma_table[11][20] = 0x7f;
	pdata->gamma_set.gamma_table[11][21] = 0x8f;
	pdata->gamma_set.gamma_table[11][22] = 0x91;
	pdata->gamma_set.gamma_table[11][23] = 0x8a;
	pdata->gamma_set.gamma_table[11][24] = 0x8f;
	pdata->gamma_set.gamma_table[11][25] = 0x88;
	pdata->gamma_set.gamma_table[11][26] = 0x90;
	pdata->gamma_set.gamma_table[11][27] = 0x0;
	pdata->gamma_set.gamma_table[11][28] = 0x0;
	pdata->gamma_set.gamma_table[11][29] = 0x0;
	pdata->gamma_set.gamma_table[12][0] = 0xce;
	pdata->gamma_set.gamma_table[12][1] = 0xd5;
	pdata->gamma_set.gamma_table[12][2] = 0xc9;
	pdata->gamma_set.gamma_table[12][3] = 0x84;
	pdata->gamma_set.gamma_table[12][4] = 0x87;
	pdata->gamma_set.gamma_table[12][5] = 0x84;
	pdata->gamma_set.gamma_table[12][6] = 0x89;
	pdata->gamma_set.gamma_table[12][7] = 0x87;
	pdata->gamma_set.gamma_table[12][8] = 0x88;
	pdata->gamma_set.gamma_table[12][9] = 0x84;
	pdata->gamma_set.gamma_table[12][10] = 0x87;
	pdata->gamma_set.gamma_table[12][11] = 0x87;
	pdata->gamma_set.gamma_table[12][12] = 0x86;
	pdata->gamma_set.gamma_table[12][13] = 0x85;
	pdata->gamma_set.gamma_table[12][14] = 0x88;
	pdata->gamma_set.gamma_table[12][15] = 0x83;
	pdata->gamma_set.gamma_table[12][16] = 0x84;
	pdata->gamma_set.gamma_table[12][17] = 0x80;
	pdata->gamma_set.gamma_table[12][18] = 0x81;
	pdata->gamma_set.gamma_table[12][19] = 0x85;
	pdata->gamma_set.gamma_table[12][20] = 0x7f;
	pdata->gamma_set.gamma_table[12][21] = 0x8e;
	pdata->gamma_set.gamma_table[12][22] = 0x90;
	pdata->gamma_set.gamma_table[12][23] = 0x8a;
	pdata->gamma_set.gamma_table[12][24] = 0x8f;
	pdata->gamma_set.gamma_table[12][25] = 0x88;
	pdata->gamma_set.gamma_table[12][26] = 0x90;
	pdata->gamma_set.gamma_table[12][27] = 0x0;
	pdata->gamma_set.gamma_table[12][28] = 0x0;
	pdata->gamma_set.gamma_table[12][29] = 0x0;
	pdata->gamma_set.gamma_table[13][0] = 0xd2;
	pdata->gamma_set.gamma_table[13][1] = 0xd8;
	pdata->gamma_set.gamma_table[13][2] = 0xcd;
	pdata->gamma_set.gamma_table[13][3] = 0x84;
	pdata->gamma_set.gamma_table[13][4] = 0x86;
	pdata->gamma_set.gamma_table[13][5] = 0x84;
	pdata->gamma_set.gamma_table[13][6] = 0x87;
	pdata->gamma_set.gamma_table[13][7] = 0x87;
	pdata->gamma_set.gamma_table[13][8] = 0x87;
	pdata->gamma_set.gamma_table[13][9] = 0x84;
	pdata->gamma_set.gamma_table[13][10] = 0x86;
	pdata->gamma_set.gamma_table[13][11] = 0x86;
	pdata->gamma_set.gamma_table[13][12] = 0x84;
	pdata->gamma_set.gamma_table[13][13] = 0x85;
	pdata->gamma_set.gamma_table[13][14] = 0x86;
	pdata->gamma_set.gamma_table[13][15] = 0x85;
	pdata->gamma_set.gamma_table[13][16] = 0x86;
	pdata->gamma_set.gamma_table[13][17] = 0x83;
	pdata->gamma_set.gamma_table[13][18] = 0x80;
	pdata->gamma_set.gamma_table[13][19] = 0x83;
	pdata->gamma_set.gamma_table[13][20] = 0x7e;
	pdata->gamma_set.gamma_table[13][21] = 0x90;
	pdata->gamma_set.gamma_table[13][22] = 0x91;
	pdata->gamma_set.gamma_table[13][23] = 0x8c;
	pdata->gamma_set.gamma_table[13][24] = 0x74;
	pdata->gamma_set.gamma_table[13][25] = 0x77;
	pdata->gamma_set.gamma_table[13][26] = 0x74;
	pdata->gamma_set.gamma_table[13][27] = 0x0;
	pdata->gamma_set.gamma_table[13][28] = 0x0;
	pdata->gamma_set.gamma_table[13][29] = 0x0;
	pdata->gamma_set.gamma_table[14][0] = 0xd6;
	pdata->gamma_set.gamma_table[14][1] = 0xdb;
	pdata->gamma_set.gamma_table[14][2] = 0xd2;
	pdata->gamma_set.gamma_table[14][3] = 0x84;
	pdata->gamma_set.gamma_table[14][4] = 0x87;
	pdata->gamma_set.gamma_table[14][5] = 0x85;
	pdata->gamma_set.gamma_table[14][6] = 0x87;
	pdata->gamma_set.gamma_table[14][7] = 0x86;
	pdata->gamma_set.gamma_table[14][8] = 0x85;
	pdata->gamma_set.gamma_table[14][9] = 0x85;
	pdata->gamma_set.gamma_table[14][10] = 0x87;
	pdata->gamma_set.gamma_table[14][11] = 0x87;
	pdata->gamma_set.gamma_table[14][12] = 0x83;
	pdata->gamma_set.gamma_table[14][13] = 0x84;
	pdata->gamma_set.gamma_table[14][14] = 0x85;
	pdata->gamma_set.gamma_table[14][15] = 0x83;
	pdata->gamma_set.gamma_table[14][16] = 0x85;
	pdata->gamma_set.gamma_table[14][17] = 0x81;
	pdata->gamma_set.gamma_table[14][18] = 0x82;
	pdata->gamma_set.gamma_table[14][19] = 0x84;
	pdata->gamma_set.gamma_table[14][20] = 0x7f;
	pdata->gamma_set.gamma_table[14][21] = 0x8f;
	pdata->gamma_set.gamma_table[14][22] = 0x90;
	pdata->gamma_set.gamma_table[14][23] = 0x8c;
	pdata->gamma_set.gamma_table[14][24] = 0x7c;
	pdata->gamma_set.gamma_table[14][25] = 0x7c;
	pdata->gamma_set.gamma_table[14][26] = 0x7c;
	pdata->gamma_set.gamma_table[14][27] = 0x0;
	pdata->gamma_set.gamma_table[14][28] = 0x0;
	pdata->gamma_set.gamma_table[14][29] = 0x0;
	pdata->gamma_set.gamma_table[15][0] = 0xda;
	pdata->gamma_set.gamma_table[15][1] = 0xde;
	pdata->gamma_set.gamma_table[15][2] = 0xd6;
	pdata->gamma_set.gamma_table[15][3] = 0x83;
	pdata->gamma_set.gamma_table[15][4] = 0x86;
	pdata->gamma_set.gamma_table[15][5] = 0x84;
	pdata->gamma_set.gamma_table[15][6] = 0x85;
	pdata->gamma_set.gamma_table[15][7] = 0x86;
	pdata->gamma_set.gamma_table[15][8] = 0x84;
	pdata->gamma_set.gamma_table[15][9] = 0x86;
	pdata->gamma_set.gamma_table[15][10] = 0x87;
	pdata->gamma_set.gamma_table[15][11] = 0x87;
	pdata->gamma_set.gamma_table[15][12] = 0x84;
	pdata->gamma_set.gamma_table[15][13] = 0x85;
	pdata->gamma_set.gamma_table[15][14] = 0x86;
	pdata->gamma_set.gamma_table[15][15] = 0x81;
	pdata->gamma_set.gamma_table[15][16] = 0x83;
	pdata->gamma_set.gamma_table[15][17] = 0x81;
	pdata->gamma_set.gamma_table[15][18] = 0x83;
	pdata->gamma_set.gamma_table[15][19] = 0x84;
	pdata->gamma_set.gamma_table[15][20] = 0x81;
	pdata->gamma_set.gamma_table[15][21] = 0x8a;
	pdata->gamma_set.gamma_table[15][22] = 0x8e;
	pdata->gamma_set.gamma_table[15][23] = 0x87;
	pdata->gamma_set.gamma_table[15][24] = 0x8b;
	pdata->gamma_set.gamma_table[15][25] = 0x85;
	pdata->gamma_set.gamma_table[15][26] = 0x8c;
	pdata->gamma_set.gamma_table[15][27] = 0x0;
	pdata->gamma_set.gamma_table[15][28] = 0x0;
	pdata->gamma_set.gamma_table[15][29] = 0x0;
	pdata->gamma_set.gamma_table[16][0] = 0xde;
	pdata->gamma_set.gamma_table[16][1] = 0xe1;
	pdata->gamma_set.gamma_table[16][2] = 0xdb;
	pdata->gamma_set.gamma_table[16][3] = 0x82;
	pdata->gamma_set.gamma_table[16][4] = 0x85;
	pdata->gamma_set.gamma_table[16][5] = 0x84;
	pdata->gamma_set.gamma_table[16][6] = 0x86;
	pdata->gamma_set.gamma_table[16][7] = 0x85;
	pdata->gamma_set.gamma_table[16][8] = 0x85;
	pdata->gamma_set.gamma_table[16][9] = 0x82;
	pdata->gamma_set.gamma_table[16][10] = 0x85;
	pdata->gamma_set.gamma_table[16][11] = 0x85;
	pdata->gamma_set.gamma_table[16][12] = 0x85;
	pdata->gamma_set.gamma_table[16][13] = 0x85;
	pdata->gamma_set.gamma_table[16][14] = 0x87;
	pdata->gamma_set.gamma_table[16][15] = 0x82;
	pdata->gamma_set.gamma_table[16][16] = 0x83;
	pdata->gamma_set.gamma_table[16][17] = 0x81;
	pdata->gamma_set.gamma_table[16][18] = 0x86;
	pdata->gamma_set.gamma_table[16][19] = 0x87;
	pdata->gamma_set.gamma_table[16][20] = 0x85;
	pdata->gamma_set.gamma_table[16][21] = 0x84;
	pdata->gamma_set.gamma_table[16][22] = 0x87;
	pdata->gamma_set.gamma_table[16][23] = 0x80;
	pdata->gamma_set.gamma_table[16][24] = 0x96;
	pdata->gamma_set.gamma_table[16][25] = 0x8d;
	pdata->gamma_set.gamma_table[16][26] = 0x98;
	pdata->gamma_set.gamma_table[16][27] = 0x0;
	pdata->gamma_set.gamma_table[16][28] = 0x0;
	pdata->gamma_set.gamma_table[16][29] = 0x0;
	pdata->gamma_set.gamma_table[17][0] = 0xe0;
	pdata->gamma_set.gamma_table[17][1] = 0xe4;
	pdata->gamma_set.gamma_table[17][2] = 0xdf;
	pdata->gamma_set.gamma_table[17][3] = 0x84;
	pdata->gamma_set.gamma_table[17][4] = 0x85;
	pdata->gamma_set.gamma_table[17][5] = 0x83;
	pdata->gamma_set.gamma_table[17][6] = 0x84;
	pdata->gamma_set.gamma_table[17][7] = 0x85;
	pdata->gamma_set.gamma_table[17][8] = 0x83;
	pdata->gamma_set.gamma_table[17][9] = 0x85;
	pdata->gamma_set.gamma_table[17][10] = 0x86;
	pdata->gamma_set.gamma_table[17][11] = 0x86;
	pdata->gamma_set.gamma_table[17][12] = 0x82;
	pdata->gamma_set.gamma_table[17][13] = 0x83;
	pdata->gamma_set.gamma_table[17][14] = 0x83;
	pdata->gamma_set.gamma_table[17][15] = 0x83;
	pdata->gamma_set.gamma_table[17][16] = 0x84;
	pdata->gamma_set.gamma_table[17][17] = 0x81;
	pdata->gamma_set.gamma_table[17][18] = 0x80;
	pdata->gamma_set.gamma_table[17][19] = 0x82;
	pdata->gamma_set.gamma_table[17][20] = 0x7f;
	pdata->gamma_set.gamma_table[17][21] = 0x8a;
	pdata->gamma_set.gamma_table[17][22] = 0x8d;
	pdata->gamma_set.gamma_table[17][23] = 0x88;
	pdata->gamma_set.gamma_table[17][24] = 0x8b;
	pdata->gamma_set.gamma_table[17][25] = 0x85;
	pdata->gamma_set.gamma_table[17][26] = 0x8c;
	pdata->gamma_set.gamma_table[17][27] = 0x0;
	pdata->gamma_set.gamma_table[17][28] = 0x0;
	pdata->gamma_set.gamma_table[17][29] = 0x0;
	pdata->gamma_set.gamma_table[18][0] = 0xe6;
	pdata->gamma_set.gamma_table[18][1] = 0xe7;
	pdata->gamma_set.gamma_table[18][2] = 0xe4;
	pdata->gamma_set.gamma_table[18][3] = 0x82;
	pdata->gamma_set.gamma_table[18][4] = 0x84;
	pdata->gamma_set.gamma_table[18][5] = 0x83;
	pdata->gamma_set.gamma_table[18][6] = 0x85;
	pdata->gamma_set.gamma_table[18][7] = 0x84;
	pdata->gamma_set.gamma_table[18][8] = 0x83;
	pdata->gamma_set.gamma_table[18][9] = 0x84;
	pdata->gamma_set.gamma_table[18][10] = 0x85;
	pdata->gamma_set.gamma_table[18][11] = 0x84;
	pdata->gamma_set.gamma_table[18][12] = 0x82;
	pdata->gamma_set.gamma_table[18][13] = 0x84;
	pdata->gamma_set.gamma_table[18][14] = 0x83;
	pdata->gamma_set.gamma_table[18][15] = 0x84;
	pdata->gamma_set.gamma_table[18][16] = 0x85;
	pdata->gamma_set.gamma_table[18][17] = 0x82;
	pdata->gamma_set.gamma_table[18][18] = 0x80;
	pdata->gamma_set.gamma_table[18][19] = 0x82;
	pdata->gamma_set.gamma_table[18][20] = 0x80;
	pdata->gamma_set.gamma_table[18][21] = 0x88;
	pdata->gamma_set.gamma_table[18][22] = 0x8b;
	pdata->gamma_set.gamma_table[18][23] = 0x86;
	pdata->gamma_set.gamma_table[18][24] = 0x8b;
	pdata->gamma_set.gamma_table[18][25] = 0x85;
	pdata->gamma_set.gamma_table[18][26] = 0x8c;
	pdata->gamma_set.gamma_table[18][27] = 0x0;
	pdata->gamma_set.gamma_table[18][28] = 0x0;
	pdata->gamma_set.gamma_table[18][29] = 0x0;
	pdata->gamma_set.gamma_table[19][0] = 0xe8;
	pdata->gamma_set.gamma_table[19][1] = 0xea;
	pdata->gamma_set.gamma_table[19][2] = 0xe7;
	pdata->gamma_set.gamma_table[19][3] = 0x84;
	pdata->gamma_set.gamma_table[19][4] = 0x84;
	pdata->gamma_set.gamma_table[19][5] = 0x83;
	pdata->gamma_set.gamma_table[19][6] = 0x83;
	pdata->gamma_set.gamma_table[19][7] = 0x83;
	pdata->gamma_set.gamma_table[19][8] = 0x82;
	pdata->gamma_set.gamma_table[19][9] = 0x84;
	pdata->gamma_set.gamma_table[19][10] = 0x85;
	pdata->gamma_set.gamma_table[19][11] = 0x85;
	pdata->gamma_set.gamma_table[19][12] = 0x82;
	pdata->gamma_set.gamma_table[19][13] = 0x83;
	pdata->gamma_set.gamma_table[19][14] = 0x82;
	pdata->gamma_set.gamma_table[19][15] = 0x82;
	pdata->gamma_set.gamma_table[19][16] = 0x83;
	pdata->gamma_set.gamma_table[19][17] = 0x82;
	pdata->gamma_set.gamma_table[19][18] = 0x80;
	pdata->gamma_set.gamma_table[19][19] = 0x82;
	pdata->gamma_set.gamma_table[19][20] = 0x80;
	pdata->gamma_set.gamma_table[19][21] = 0x89;
	pdata->gamma_set.gamma_table[19][22] = 0x8a;
	pdata->gamma_set.gamma_table[19][23] = 0x87;
	pdata->gamma_set.gamma_table[19][24] = 0x83;
	pdata->gamma_set.gamma_table[19][25] = 0x81;
	pdata->gamma_set.gamma_table[19][26] = 0x84;
	pdata->gamma_set.gamma_table[19][27] = 0x0;
	pdata->gamma_set.gamma_table[19][28] = 0x0;
	pdata->gamma_set.gamma_table[19][29] = 0x0;
	pdata->gamma_set.gamma_table[20][0] = 0xe8;
	pdata->gamma_set.gamma_table[20][1] = 0xea;
	pdata->gamma_set.gamma_table[20][2] = 0xe7;
	pdata->gamma_set.gamma_table[20][3] = 0x83;
	pdata->gamma_set.gamma_table[20][4] = 0x84;
	pdata->gamma_set.gamma_table[20][5] = 0x82;
	pdata->gamma_set.gamma_table[20][6] = 0x83;
	pdata->gamma_set.gamma_table[20][7] = 0x84;
	pdata->gamma_set.gamma_table[20][8] = 0x83;
	pdata->gamma_set.gamma_table[20][9] = 0x83;
	pdata->gamma_set.gamma_table[20][10] = 0x84;
	pdata->gamma_set.gamma_table[20][11] = 0x84;
	pdata->gamma_set.gamma_table[20][12] = 0x82;
	pdata->gamma_set.gamma_table[20][13] = 0x84;
	pdata->gamma_set.gamma_table[20][14] = 0x83;
	pdata->gamma_set.gamma_table[20][15] = 0x82;
	pdata->gamma_set.gamma_table[20][16] = 0x83;
	pdata->gamma_set.gamma_table[20][17] = 0x82;
	pdata->gamma_set.gamma_table[20][18] = 0x80;
	pdata->gamma_set.gamma_table[20][19] = 0x82;
	pdata->gamma_set.gamma_table[20][20] = 0x80;
	pdata->gamma_set.gamma_table[20][21] = 0x88;
	pdata->gamma_set.gamma_table[20][22] = 0x8a;
	pdata->gamma_set.gamma_table[20][23] = 0x86;
	pdata->gamma_set.gamma_table[20][24] = 0x8b;
	pdata->gamma_set.gamma_table[20][25] = 0x85;
	pdata->gamma_set.gamma_table[20][26] = 0x8c;
	pdata->gamma_set.gamma_table[20][27] = 0x0;
	pdata->gamma_set.gamma_table[20][28] = 0x0;
	pdata->gamma_set.gamma_table[20][29] = 0x0;
	pdata->gamma_set.gamma_table[21][0] = 0xe8;
	pdata->gamma_set.gamma_table[21][1] = 0xea;
	pdata->gamma_set.gamma_table[21][2] = 0xe6;
	pdata->gamma_set.gamma_table[21][3] = 0x83;
	pdata->gamma_set.gamma_table[21][4] = 0x84;
	pdata->gamma_set.gamma_table[21][5] = 0x83;
	pdata->gamma_set.gamma_table[21][6] = 0x83;
	pdata->gamma_set.gamma_table[21][7] = 0x84;
	pdata->gamma_set.gamma_table[21][8] = 0x83;
	pdata->gamma_set.gamma_table[21][9] = 0x83;
	pdata->gamma_set.gamma_table[21][10] = 0x84;
	pdata->gamma_set.gamma_table[21][11] = 0x84;
	pdata->gamma_set.gamma_table[21][12] = 0x81;
	pdata->gamma_set.gamma_table[21][13] = 0x82;
	pdata->gamma_set.gamma_table[21][14] = 0x82;
	pdata->gamma_set.gamma_table[21][15] = 0x84;
	pdata->gamma_set.gamma_table[21][16] = 0x85;
	pdata->gamma_set.gamma_table[21][17] = 0x82;
	pdata->gamma_set.gamma_table[21][18] = 0x7d;
	pdata->gamma_set.gamma_table[21][19] = 0x80;
	pdata->gamma_set.gamma_table[21][20] = 0x7d;
	pdata->gamma_set.gamma_table[21][21] = 0x88;
	pdata->gamma_set.gamma_table[21][22] = 0x89;
	pdata->gamma_set.gamma_table[21][23] = 0x85;
	pdata->gamma_set.gamma_table[21][24] = 0x87;
	pdata->gamma_set.gamma_table[21][25] = 0x83;
	pdata->gamma_set.gamma_table[21][26] = 0x88;
	pdata->gamma_set.gamma_table[21][27] = 0x0;
	pdata->gamma_set.gamma_table[21][28] = 0x0;
	pdata->gamma_set.gamma_table[21][29] = 0x0;
	pdata->gamma_set.gamma_table[22][0] = 0xe8;
	pdata->gamma_set.gamma_table[22][1] = 0xea;
	pdata->gamma_set.gamma_table[22][2] = 0xe6;
	pdata->gamma_set.gamma_table[22][3] = 0x83;
	pdata->gamma_set.gamma_table[22][4] = 0x84;
	pdata->gamma_set.gamma_table[22][5] = 0x83;
	pdata->gamma_set.gamma_table[22][6] = 0x81;
	pdata->gamma_set.gamma_table[22][7] = 0x83;
	pdata->gamma_set.gamma_table[22][8] = 0x81;
	pdata->gamma_set.gamma_table[22][9] = 0x84;
	pdata->gamma_set.gamma_table[22][10] = 0x85;
	pdata->gamma_set.gamma_table[22][11] = 0x85;
	pdata->gamma_set.gamma_table[22][12] = 0x82;
	pdata->gamma_set.gamma_table[22][13] = 0x82;
	pdata->gamma_set.gamma_table[22][14] = 0x83;
	pdata->gamma_set.gamma_table[22][15] = 0x82;
	pdata->gamma_set.gamma_table[22][16] = 0x82;
	pdata->gamma_set.gamma_table[22][17] = 0x81;
	pdata->gamma_set.gamma_table[22][18] = 0x80;
	pdata->gamma_set.gamma_table[22][19] = 0x82;
	pdata->gamma_set.gamma_table[22][20] = 0x80;
	pdata->gamma_set.gamma_table[22][21] = 0x88;
	pdata->gamma_set.gamma_table[22][22] = 0x88;
	pdata->gamma_set.gamma_table[22][23] = 0x85;
	pdata->gamma_set.gamma_table[22][24] = 0x80;
	pdata->gamma_set.gamma_table[22][25] = 0x7f;
	pdata->gamma_set.gamma_table[22][26] = 0x80;
	pdata->gamma_set.gamma_table[22][27] = 0x0;
	pdata->gamma_set.gamma_table[22][28] = 0x0;
	pdata->gamma_set.gamma_table[22][29] = 0x0;
	pdata->gamma_set.gamma_table[23][0] = 0xe7;
	pdata->gamma_set.gamma_table[23][1] = 0xea;
	pdata->gamma_set.gamma_table[23][2] = 0xe5;
	pdata->gamma_set.gamma_table[23][3] = 0x83;
	pdata->gamma_set.gamma_table[23][4] = 0x84;
	pdata->gamma_set.gamma_table[23][5] = 0x83;
	pdata->gamma_set.gamma_table[23][6] = 0x83;
	pdata->gamma_set.gamma_table[23][7] = 0x83;
	pdata->gamma_set.gamma_table[23][8] = 0x82;
	pdata->gamma_set.gamma_table[23][9] = 0x82;
	pdata->gamma_set.gamma_table[23][10] = 0x84;
	pdata->gamma_set.gamma_table[23][11] = 0x84;
	pdata->gamma_set.gamma_table[23][12] = 0x82;
	pdata->gamma_set.gamma_table[23][13] = 0x82;
	pdata->gamma_set.gamma_table[23][14] = 0x83;
	pdata->gamma_set.gamma_table[23][15] = 0x81;
	pdata->gamma_set.gamma_table[23][16] = 0x82;
	pdata->gamma_set.gamma_table[23][17] = 0x80;
	pdata->gamma_set.gamma_table[23][18] = 0x7f;
	pdata->gamma_set.gamma_table[23][19] = 0x81;
	pdata->gamma_set.gamma_table[23][20] = 0x7f;
	pdata->gamma_set.gamma_table[23][21] = 0x86;
	pdata->gamma_set.gamma_table[23][22] = 0x88;
	pdata->gamma_set.gamma_table[23][23] = 0x84;
	pdata->gamma_set.gamma_table[23][24] = 0x83;
	pdata->gamma_set.gamma_table[23][25] = 0x81;
	pdata->gamma_set.gamma_table[23][26] = 0x84;
	pdata->gamma_set.gamma_table[23][27] = 0x0;
	pdata->gamma_set.gamma_table[23][28] = 0x0;
	pdata->gamma_set.gamma_table[23][29] = 0x0;
	pdata->gamma_set.gamma_table[24][0] = 0xe7;
	pdata->gamma_set.gamma_table[24][1] = 0xea;
	pdata->gamma_set.gamma_table[24][2] = 0xe5;
	pdata->gamma_set.gamma_table[24][3] = 0x82;
	pdata->gamma_set.gamma_table[24][4] = 0x83;
	pdata->gamma_set.gamma_table[24][5] = 0x82;
	pdata->gamma_set.gamma_table[24][6] = 0x83;
	pdata->gamma_set.gamma_table[24][7] = 0x83;
	pdata->gamma_set.gamma_table[24][8] = 0x82;
	pdata->gamma_set.gamma_table[24][9] = 0x82;
	pdata->gamma_set.gamma_table[24][10] = 0x83;
	pdata->gamma_set.gamma_table[24][11] = 0x83;
	pdata->gamma_set.gamma_table[24][12] = 0x83;
	pdata->gamma_set.gamma_table[24][13] = 0x83;
	pdata->gamma_set.gamma_table[24][14] = 0x84;
	pdata->gamma_set.gamma_table[24][15] = 0x82;
	pdata->gamma_set.gamma_table[24][16] = 0x82;
	pdata->gamma_set.gamma_table[24][17] = 0x81;
	pdata->gamma_set.gamma_table[24][18] = 0x80;
	pdata->gamma_set.gamma_table[24][19] = 0x81;
	pdata->gamma_set.gamma_table[24][20] = 0x80;
	pdata->gamma_set.gamma_table[24][21] = 0x82;
	pdata->gamma_set.gamma_table[24][22] = 0x84;
	pdata->gamma_set.gamma_table[24][23] = 0x80;
	pdata->gamma_set.gamma_table[24][24] = 0x8f;
	pdata->gamma_set.gamma_table[24][25] = 0x89;
	pdata->gamma_set.gamma_table[24][26] = 0x90;
	pdata->gamma_set.gamma_table[24][27] = 0x0;
	pdata->gamma_set.gamma_table[24][28] = 0x0;
	pdata->gamma_set.gamma_table[24][29] = 0x0;
	pdata->gamma_set.gamma_table[25][0] = 0xe6;
	pdata->gamma_set.gamma_table[25][1] = 0xea;
	pdata->gamma_set.gamma_table[25][2] = 0xe3;
	pdata->gamma_set.gamma_table[25][3] = 0x82;
	pdata->gamma_set.gamma_table[25][4] = 0x83;
	pdata->gamma_set.gamma_table[25][5] = 0x83;
	pdata->gamma_set.gamma_table[25][6] = 0x83;
	pdata->gamma_set.gamma_table[25][7] = 0x83;
	pdata->gamma_set.gamma_table[25][8] = 0x83;
	pdata->gamma_set.gamma_table[25][9] = 0x84;
	pdata->gamma_set.gamma_table[25][10] = 0x83;
	pdata->gamma_set.gamma_table[25][11] = 0x83;
	pdata->gamma_set.gamma_table[25][12] = 0x83;
	pdata->gamma_set.gamma_table[25][13] = 0x83;
	pdata->gamma_set.gamma_table[25][14] = 0x84;
	pdata->gamma_set.gamma_table[25][15] = 0x81;
	pdata->gamma_set.gamma_table[25][16] = 0x82;
	pdata->gamma_set.gamma_table[25][17] = 0x81;
	pdata->gamma_set.gamma_table[25][18] = 0x80;
	pdata->gamma_set.gamma_table[25][19] = 0x81;
	pdata->gamma_set.gamma_table[25][20] = 0x80;
	pdata->gamma_set.gamma_table[25][21] = 0x83;
	pdata->gamma_set.gamma_table[25][22] = 0x83;
	pdata->gamma_set.gamma_table[25][23] = 0x84;
	pdata->gamma_set.gamma_table[25][24] = 0x8f;
	pdata->gamma_set.gamma_table[25][25] = 0x89;
	pdata->gamma_set.gamma_table[25][26] = 0x90;
	pdata->gamma_set.gamma_table[25][27] = 0x0;
	pdata->gamma_set.gamma_table[25][28] = 0x0;
	pdata->gamma_set.gamma_table[25][29] = 0x0;
	pdata->gamma_set.gamma_table[26][0] = 0xea;
	pdata->gamma_set.gamma_table[26][1] = 0xee;
	pdata->gamma_set.gamma_table[26][2] = 0xe8;
	pdata->gamma_set.gamma_table[26][3] = 0x82;
	pdata->gamma_set.gamma_table[26][4] = 0x83;
	pdata->gamma_set.gamma_table[26][5] = 0x82;
	pdata->gamma_set.gamma_table[26][6] = 0x82;
	pdata->gamma_set.gamma_table[26][7] = 0x82;
	pdata->gamma_set.gamma_table[26][8] = 0x82;
	pdata->gamma_set.gamma_table[26][9] = 0x82;
	pdata->gamma_set.gamma_table[26][10] = 0x82;
	pdata->gamma_set.gamma_table[26][11] = 0x82;
	pdata->gamma_set.gamma_table[26][12] = 0x82;
	pdata->gamma_set.gamma_table[26][13] = 0x82;
	pdata->gamma_set.gamma_table[26][14] = 0x83;
	pdata->gamma_set.gamma_table[26][15] = 0x81;
	pdata->gamma_set.gamma_table[26][16] = 0x82;
	pdata->gamma_set.gamma_table[26][17] = 0x81;
	pdata->gamma_set.gamma_table[26][18] = 0x80;
	pdata->gamma_set.gamma_table[26][19] = 0x81;
	pdata->gamma_set.gamma_table[26][20] = 0x80;
	pdata->gamma_set.gamma_table[26][21] = 0x83;
	pdata->gamma_set.gamma_table[26][22] = 0x83;
	pdata->gamma_set.gamma_table[26][23] = 0x84;
	pdata->gamma_set.gamma_table[26][24] = 0x87;
	pdata->gamma_set.gamma_table[26][25] = 0x84;
	pdata->gamma_set.gamma_table[26][26] = 0x88;
	pdata->gamma_set.gamma_table[26][27] = 0x0;
	pdata->gamma_set.gamma_table[26][28] = 0x0;
	pdata->gamma_set.gamma_table[26][29] = 0x0;
	pdata->gamma_set.gamma_table[27][0] = 0xee;
	pdata->gamma_set.gamma_table[27][1] = 0xf1;
	pdata->gamma_set.gamma_table[27][2] = 0xed;
	pdata->gamma_set.gamma_table[27][3] = 0x82;
	pdata->gamma_set.gamma_table[27][4] = 0x82;
	pdata->gamma_set.gamma_table[27][5] = 0x82;
	pdata->gamma_set.gamma_table[27][6] = 0x82;
	pdata->gamma_set.gamma_table[27][7] = 0x81;
	pdata->gamma_set.gamma_table[27][8] = 0x82;
	pdata->gamma_set.gamma_table[27][9] = 0x83;
	pdata->gamma_set.gamma_table[27][10] = 0x83;
	pdata->gamma_set.gamma_table[27][11] = 0x83;
	pdata->gamma_set.gamma_table[27][12] = 0x82;
	pdata->gamma_set.gamma_table[27][13] = 0x82;
	pdata->gamma_set.gamma_table[27][14] = 0x82;
	pdata->gamma_set.gamma_table[27][15] = 0x80;
	pdata->gamma_set.gamma_table[27][16] = 0x81;
	pdata->gamma_set.gamma_table[27][17] = 0x80;
	pdata->gamma_set.gamma_table[27][18] = 0x82;
	pdata->gamma_set.gamma_table[27][19] = 0x82;
	pdata->gamma_set.gamma_table[27][20] = 0x82;
	pdata->gamma_set.gamma_table[27][21] = 0x84;
	pdata->gamma_set.gamma_table[27][22] = 0x83;
	pdata->gamma_set.gamma_table[27][23] = 0x84;
	pdata->gamma_set.gamma_table[27][24] = 0x83;
	pdata->gamma_set.gamma_table[27][25] = 0x81;
	pdata->gamma_set.gamma_table[27][26] = 0x84;
	pdata->gamma_set.gamma_table[27][27] = 0x0;
	pdata->gamma_set.gamma_table[27][28] = 0x0;
	pdata->gamma_set.gamma_table[27][29] = 0x0;
	pdata->gamma_set.gamma_table[28][0] = 0xf3;
	pdata->gamma_set.gamma_table[28][1] = 0xf5;
	pdata->gamma_set.gamma_table[28][2] = 0xf2;
	pdata->gamma_set.gamma_table[28][3] = 0x82;
	pdata->gamma_set.gamma_table[28][4] = 0x82;
	pdata->gamma_set.gamma_table[28][5] = 0x82;
	pdata->gamma_set.gamma_table[28][6] = 0x81;
	pdata->gamma_set.gamma_table[28][7] = 0x80;
	pdata->gamma_set.gamma_table[28][8] = 0x81;
	pdata->gamma_set.gamma_table[28][9] = 0x82;
	pdata->gamma_set.gamma_table[28][10] = 0x82;
	pdata->gamma_set.gamma_table[28][11] = 0x82;
	pdata->gamma_set.gamma_table[28][12] = 0x81;
	pdata->gamma_set.gamma_table[28][13] = 0x81;
	pdata->gamma_set.gamma_table[28][14] = 0x81;
	pdata->gamma_set.gamma_table[28][15] = 0x83;
	pdata->gamma_set.gamma_table[28][16] = 0x83;
	pdata->gamma_set.gamma_table[28][17] = 0x83;
	pdata->gamma_set.gamma_table[28][18] = 0x7f;
	pdata->gamma_set.gamma_table[28][19] = 0x80;
	pdata->gamma_set.gamma_table[28][20] = 0x7f;
	pdata->gamma_set.gamma_table[28][21] = 0x7f;
	pdata->gamma_set.gamma_table[28][22] = 0x80;
	pdata->gamma_set.gamma_table[28][23] = 0x7f;
	pdata->gamma_set.gamma_table[28][24] = 0x97;
	pdata->gamma_set.gamma_table[28][25] = 0x8e;
	pdata->gamma_set.gamma_table[28][26] = 0x98;
	pdata->gamma_set.gamma_table[28][27] = 0x0;
	pdata->gamma_set.gamma_table[28][28] = 0x0;
	pdata->gamma_set.gamma_table[28][29] = 0x0;
	pdata->gamma_set.gamma_table[29][0] = 0xf7;
	pdata->gamma_set.gamma_table[29][1] = 0xf9;
	pdata->gamma_set.gamma_table[29][2] = 0xf7;
	pdata->gamma_set.gamma_table[29][3] = 0x81;
	pdata->gamma_set.gamma_table[29][4] = 0x81;
	pdata->gamma_set.gamma_table[29][5] = 0x81;
	pdata->gamma_set.gamma_table[29][6] = 0x80;
	pdata->gamma_set.gamma_table[29][7] = 0x7f;
	pdata->gamma_set.gamma_table[29][8] = 0x7f;
	pdata->gamma_set.gamma_table[29][9] = 0x82;
	pdata->gamma_set.gamma_table[29][10] = 0x82;
	pdata->gamma_set.gamma_table[29][11] = 0x82;
	pdata->gamma_set.gamma_table[29][12] = 0x82;
	pdata->gamma_set.gamma_table[29][13] = 0x82;
	pdata->gamma_set.gamma_table[29][14] = 0x82;
	pdata->gamma_set.gamma_table[29][15] = 0x81;
	pdata->gamma_set.gamma_table[29][16] = 0x81;
	pdata->gamma_set.gamma_table[29][17] = 0x81;
	pdata->gamma_set.gamma_table[29][18] = 0x80;
	pdata->gamma_set.gamma_table[29][19] = 0x80;
	pdata->gamma_set.gamma_table[29][20] = 0x80;
	pdata->gamma_set.gamma_table[29][21] = 0x82;
	pdata->gamma_set.gamma_table[29][22] = 0x81;
	pdata->gamma_set.gamma_table[29][23] = 0x82;
	pdata->gamma_set.gamma_table[29][24] = 0x83;
	pdata->gamma_set.gamma_table[29][25] = 0x81;
	pdata->gamma_set.gamma_table[29][26] = 0x84;
	pdata->gamma_set.gamma_table[29][27] = 0x0;
	pdata->gamma_set.gamma_table[29][28] = 0x0;
	pdata->gamma_set.gamma_table[29][29] = 0x0;
	pdata->gamma_set.gamma_table[30][0] = 0xfc;
	pdata->gamma_set.gamma_table[30][1] = 0xfc;
	pdata->gamma_set.gamma_table[30][2] = 0xfb;
	pdata->gamma_set.gamma_table[30][3] = 0x80;
	pdata->gamma_set.gamma_table[30][4] = 0x80;
	pdata->gamma_set.gamma_table[30][5] = 0x80;
	pdata->gamma_set.gamma_table[30][6] = 0x81;
	pdata->gamma_set.gamma_table[30][7] = 0x81;
	pdata->gamma_set.gamma_table[30][8] = 0x81;
	pdata->gamma_set.gamma_table[30][9] = 0x80;
	pdata->gamma_set.gamma_table[30][10] = 0x80;
	pdata->gamma_set.gamma_table[30][11] = 0x80;
	pdata->gamma_set.gamma_table[30][12] = 0x82;
	pdata->gamma_set.gamma_table[30][13] = 0x82;
	pdata->gamma_set.gamma_table[30][14] = 0x82;
	pdata->gamma_set.gamma_table[30][15] = 0x81;
	pdata->gamma_set.gamma_table[30][16] = 0x80;
	pdata->gamma_set.gamma_table[30][17] = 0x80;
	pdata->gamma_set.gamma_table[30][18] = 0x81;
	pdata->gamma_set.gamma_table[30][19] = 0x81;
	pdata->gamma_set.gamma_table[30][20] = 0x81;
	pdata->gamma_set.gamma_table[30][21] = 0x7f;
	pdata->gamma_set.gamma_table[30][22] = 0x7e;
	pdata->gamma_set.gamma_table[30][23] = 0x80;
	pdata->gamma_set.gamma_table[30][24] = 0x93;
	pdata->gamma_set.gamma_table[30][25] = 0x8b;
	pdata->gamma_set.gamma_table[30][26] = 0x94;
	pdata->gamma_set.gamma_table[30][27] = 0x0;
	pdata->gamma_set.gamma_table[30][28] = 0x0;
	pdata->gamma_set.gamma_table[30][29] = 0x0;
	pdata->gamma_set.gamma_table[31][0] = 0x100;
	pdata->gamma_set.gamma_table[31][1] = 0x100;
	pdata->gamma_set.gamma_table[31][2] = 0x100;
	pdata->gamma_set.gamma_table[31][3] = 0x80;
	pdata->gamma_set.gamma_table[31][4] = 0x80;
	pdata->gamma_set.gamma_table[31][5] = 0x80;
	pdata->gamma_set.gamma_table[31][6] = 0x80;
	pdata->gamma_set.gamma_table[31][7] = 0x80;
	pdata->gamma_set.gamma_table[31][8] = 0x80;
	pdata->gamma_set.gamma_table[31][9] = 0x80;
	pdata->gamma_set.gamma_table[31][10] = 0x80;
	pdata->gamma_set.gamma_table[31][11] = 0x80;
	pdata->gamma_set.gamma_table[31][12] = 0x7f;
	pdata->gamma_set.gamma_table[31][13] = 0x80;
	pdata->gamma_set.gamma_table[31][14] = 0x80;
	pdata->gamma_set.gamma_table[31][15] = 0x7f;
	pdata->gamma_set.gamma_table[31][16] = 0x7f;
	pdata->gamma_set.gamma_table[31][17] = 0x80;
	pdata->gamma_set.gamma_table[31][18] = 0x7f;
	pdata->gamma_set.gamma_table[31][19] = 0x80;
	pdata->gamma_set.gamma_table[31][20] = 0x7f;
	pdata->gamma_set.gamma_table[31][21] = 0x7f;
	pdata->gamma_set.gamma_table[31][22] = 0x80;
	pdata->gamma_set.gamma_table[31][23] = 0x7f;
	pdata->gamma_set.gamma_table[31][24] = 0x80;
	pdata->gamma_set.gamma_table[31][25] = 0x80;
	pdata->gamma_set.gamma_table[31][26] = 0x7f;
	pdata->gamma_set.gamma_table[31][27] = 0x0;
	pdata->gamma_set.gamma_table[31][28] = 0x0;
	pdata->gamma_set.gamma_table[31][29] = 0x0;
}
void mtp_read(int data,char * read_buf)
{

	struct msm_fb_data_type *mfd = mfdmsm_fb_get_mfd();
	struct mdss_panel_info *panel_info = mfd->panel_info;
	struct mdss_panel_data * pdata =NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	int index = 0;
	int temp_index = V203;
	
	pdata = container_of(panel_info, struct mdss_panel_data,
				panel_info);
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	for(index = 0; index < data; index++){
		printk("OLED Mtp Value[%d] = %d 0x%x\n",ctrl_pdata->mtp_cnt,*read_buf,*read_buf);
		ctrl_pdata->panel_read_mtp.mtp_data_RGB[ctrl_pdata->mtp_cnt] = *(read_buf);
		(read_buf)++;
		ctrl_pdata->mtp_cnt++;
	}
	
	if(ctrl_pdata->mtp_cnt == MTP_READ_MAX)
	{
		ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] = ((ctrl_pdata->panel_read_mtp.mtp_data_RGB[0] & 0x01) <<8) + ctrl_pdata->panel_read_mtp.mtp_data_RGB[1];
		ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] = ((ctrl_pdata->panel_read_mtp.mtp_data_RGB[2] & 0x01) << 8) + ctrl_pdata->panel_read_mtp.mtp_data_RGB[3];
		ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] = ((ctrl_pdata->panel_read_mtp.mtp_data_RGB[4] & 0x01) << 8)+ ctrl_pdata->panel_read_mtp.mtp_data_RGB[5];

		if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] > 255)
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][V255] -256);
		if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] > 255)
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][V255] -256);
		if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] > 255)
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][V255] -256);

		for(index = 6; index < MTP_READ_MAX; )
		{
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] = ctrl_pdata->panel_read_mtp.mtp_data_RGB[index];
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] = ctrl_pdata->panel_read_mtp.mtp_data_RGB[index + 1];
			ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] = ctrl_pdata->panel_read_mtp.mtp_data_RGB[index + 2];
			
			if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] > 127)
				ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][temp_index] - 128);
			if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] > 127)
				ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][temp_index] - 128);
			if(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] > 127)
				ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] = -(ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][temp_index] - 128);
			
			index += 3;
			temp_index--;
		}	

		//gamma_add2_mtp
		for(index = 0; index < V255_MAX; index++){
			for(temp_index = 0; temp_index < RGB_MAX; temp_index++){
				ctrl_pdata->panel_read_mtp.gamma_add2_mtp[temp_index][index] = ctrl_pdata->panel_read_mtp.mtp_RGB[temp_index][index] 
																+ ctrl_pdata->panel_read_mtp.panel_gamma_data[temp_index][index];
			}
		}	
		
#if 0//def SMART_DIMMING_DEBUG
		for(index = 0;index < 10;index++)
			printk("mtp_data_R[%d]\n",ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_R][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("mtp_data_G[%d]\n",ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_G][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("mtp_data_B[%d]\n",ctrl_pdata->panel_read_mtp.mtp_RGB[RGB_B][index]);

		for(index = 0;index < 10;index++)
			printk("panel_gamma_data_R[%d]\n",ctrl_pdata->panel_read_mtp.panel_gamma_data[RGB_R][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("panel_gamma_data_G[%d]\n",ctrl_pdata->panel_read_mtp.panel_gamma_data[RGB_G][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("panel_gamma_data_B[%d]\n",ctrl_pdata->panel_read_mtp.panel_gamma_data[RGB_B][index]);
		
		for(index = 0;index < 10;index++)
			printk("gamma_add2_mtp[%d]\n",ctrl_pdata->panel_read_mtp.gamma_add2_mtp[RGB_R][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("gamma_add2_mtp[%d]\n",ctrl_pdata->panel_read_mtp.gamma_add2_mtp[RGB_G][index]);
		printk("======\n");
		for(index = 0;index < 10;index++)
			printk("gamma_add2_mtp[%d]\n",ctrl_pdata->panel_read_mtp.gamma_add2_mtp[RGB_B][index]);
#endif
	}

}	

#ifdef CONFIG_F_SKYDISP_EF63_DRIVER_IC_CHECK


static void ef63_octa_driver_ic_check(struct mdss_dsi_ctrl_pdata *ctrl)
{
	char id1[]= {0};
	char id3[]= {0};
	mdss_dsi_panel_cmd_read(ctrl, 0xda, 0x00, NULL, id1, 1);
		
	//mdss_dsi_panel_cmd_read(ctrl, 0xdb, 0x00, NULL, id2, 1);
	mdss_dsi_panel_cmd_read(ctrl, 0xdc, 0x00, NULL, id3, 1);

	if(id1[0] == 0x40  /*&& id2[0] == 0x40 && id3[0] == 0x24*/){
	
		if(id3[0] >= 0x28)
			ctrl->manufacture_id_rev = NEW_REV;
		else
			ctrl->manufacture_id_rev = OLD_REV;				
		ctrl->manufacture_id = SAMSUNG_DRIVER_IC;
		printk("[PANTECH_LCD]DDI_connect: id1=%x, id2=.., id3=%x\n",id1[0],id3[0] );
	}
	else if(id1[0] == 0x20 /*&& id2[0] == 0x0 && id3[0] == 0x01*/)	{			
		ctrl->manufacture_id = MAGNA_DRIVER_IC;
		printk("[PANTECH_LCD]MaganaIC_connect: id1=%x, id2=.., id3=.\n",id1[0]);
	}
	else
		ctrl->manufacture_id = NO_CONNECT;		

	/* Set EF63 OLED Panel Gamma */
	panel_gamma_sort(ctrl);
}
#endif

#ifdef F_LSI_VDDM_OFFSET_RD_WR 
static void ef63_octa_vddm_offset(struct mdss_dsi_ctrl_pdata *ctrl)
{
	int vddm_offset;
	char vddm_offset_read[]={0};
	
	mdss_dsi_panel_cmd_read(ctrl, 0xD7, 0x00, NULL, vddm_offset_read, 1);
	vddm_offset=(unsigned int)(vddm_offset_read[0] & 0x7F);
	pr_info("%s:vddm_offset = %d , ldi_vddm_lut[%d][1] = %d \n", __func__, vddm_offset, vddm_offset, ldi_vddm_lut[vddm_offset][1]);
	ctrl->vddm_offset_write_cmds.buf[27] = ldi_vddm_lut[vddm_offset][1];
}
#endif
void read_reg_chipdependency(struct mdss_dsi_ctrl_pdata *ctrl_pdata,int base_addr,int offset)
{
	struct dcs_cmd_req cmdreq;
	int quotient,remainder,index;
	
	char mtp_write_data[2] = {0xb0, 0x00};	
	struct dsi_cmd_desc mtp_data_cmd = {{DTYPE_DCS_WRITE1, 1, 0, 0, 1, sizeof(mtp_write_data)},mtp_write_data};
	mtp_write_data[1] = base_addr;	
	quotient  = offset / 8;
	if(offset >= 8)
		remainder = offset % 8;
	else
		remainder = offset;
	

	
	//if(ctrl_pdata->manufacture_id == SAMSUNG_DRIVER_IC)
		//mdss_dsi_cmds_send(ctrl_pdata,&cmdreq,&mtp_unlock_cmd);
	
	memset(&cmdreq, 0, sizeof(cmdreq));
	cmdreq.cmds = &mtp_data_cmd;
	cmdreq.cmds_cnt = 1;
	cmdreq.flags = CMD_REQ_COMMIT | CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;
	
	for(index = 0; index < quotient; index++){
		*(cmdreq.cmds->payload + 1) = 8 * index  + base_addr;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);	
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read,ctrl_pdata->rx_buf.data,8);
		printk("Reg Offset  = 0x%x index = %d mtp_write_data = %d\n",*(cmdreq.cmds->payload + 1),index, mtp_write_data[1]);
	}

	*(cmdreq.cmds->payload + 1) = quotient * 8 + base_addr;
	mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
	mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read,ctrl_pdata->rx_buf.data,remainder);
	printk("Reg Offset  = 0x%x index = %d mtp_write_data = %d\n",*(cmdreq.cmds->payload + 1),index, mtp_write_data[1]);

	
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	if(ctrl_pdata->manufacture_id == SAMSUNG_DRIVER_IC){//read for hbm
		*(cmdreq.cmds->payload + 1) = 33;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read_hbm,ctrl_pdata->rx_buf.data,7);
		*(cmdreq.cmds->payload + 1) = 72;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read_hbm,ctrl_pdata->rx_buf.data,8);
		*(cmdreq.cmds->payload + 1) = 80;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xc8,0x00,mtp_read_hbm,ctrl_pdata->rx_buf.data,7);

		*(cmdreq.cmds->payload + 1) = 0;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);//elvss 6reg
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xb6,0x00,mtp_read_hbm_elvss,ctrl_pdata->rx_buf.data,8);
		*(cmdreq.cmds->payload + 1) = 8;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xb6,0x00,mtp_read_hbm_elvss,ctrl_pdata->rx_buf.data,8);
		
		*(cmdreq.cmds->payload + 1) = 16;
		mdss_dsi_cmdlist_put(ctrl_pdata, &cmdreq);
		mdss_dsi_panel_cmd_read(ctrl_pdata,0xb6,0x00,mtp_read_hbm_elvss,ctrl_pdata->rx_buf.data,1);
}
#endif
}
void mtp_read_work(struct work_struct *work)
{
	struct msm_fb_data_type *mfd = mfdmsm_fb_get_mfd();
	struct mdss_panel_info *panel_info = mfd->panel_info;
	struct mdss_panel_data * pdata =NULL;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	
	pdata = container_of(panel_info, struct mdss_panel_data,
				panel_info);
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	mdss_dsi_sw_reset(pdata);
	mdss_dsi_host_init(pdata);
	mdss_dsi_op_mode_config(panel_info->mipi.mode, pdata);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON, false);
	mdss_dsi_set_tx_power_mode(0 , pdata );

#ifdef CONFIG_F_SKYDISP_EF63_DRIVER_IC_CHECK
	ef63_octa_driver_ic_check(ctrl_pdata);	
#endif

#ifdef F_LSI_VDDM_OFFSET_RD_WR 
	if(ctrl_pdata->manufacture_id == SAMSUNG_DRIVER_IC)
		ef63_octa_vddm_offset(ctrl_pdata);
#endif

	if(ctrl_pdata->manufacture_id == SAMSUNG_DRIVER_IC){
		read_reg_chipdependency(ctrl_pdata,0x00,33);
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	ctrl_pdata->onflag = false;
	ctrl_pdata->panel_data.hbm_control = hbm_control_ddi;
	ctrl_pdata->panel_data.acl_control = acl_control;
#endif	
	}
	else if(ctrl_pdata->manufacture_id == MAGNA_DRIVER_IC){
		read_reg_chipdependency(ctrl_pdata,0x00,33);
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	ctrl_pdata->panel_data.hbm_control = hbm_control_magna;
#endif	
	}
	else
		pr_err("not connected");
	
	mdss_dsi_set_tx_power_mode(1 , pdata );
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF, false);

	if(ctrl_pdata->manufacture_id == SAMSUNG_DRIVER_IC)
	{
		if(ctrl_pdata->manufacture_id_rev == NEW_REV){
			memcpy(aor_dim,aor_dim_lsi_rev,sizeof(aor_dim));
			memcpy(elvss_set,elvss_set_lsi_rev,sizeof(elvss_set));
			memcpy(gamma_level,gamma_level_lsi,sizeof(gamma_level));
			memcpy(locking,locking_lsi,sizeof(locking));
			memcpy(oled_gamma,oled_gamma_lsi,sizeof(oled_gamma));
			printk("Samsung LCD is New Rev Ver\n");
		}
		else{
			memcpy(aor_dim,aor_dim_lsi,sizeof(aor_dim));
			memcpy(elvss_set,elvss_set_lsi,sizeof(elvss_set));
			memcpy(gamma_level,gamma_level_lsi,sizeof(gamma_level));
			memcpy(locking,locking_lsi,sizeof(locking));
			memcpy(oled_gamma,oled_gamma_lsi,sizeof(oled_gamma));
			printk("Samsung LCD is Old Rev Ver\n");
		}
	}

	
	
}
#endif
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
bool first_enable = false;
#endif
static void mdss_dsi_panel_bl_ctrl(struct mdss_panel_data *pdata,
							u32 bl_level)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct msm_fb_data_type * mfd = mfdmsm_fb_get_mfd();
	
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}
	
	if(!mfd->panel_power_on)
	{
		printk("[%s] panel is off state (%d).....\n",__func__,mfd->panel_power_on);
		return;
	}
	
	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
	if(bl_level == 0)
	{
		gpio_set_value((ctrl_pdata->bl_en_gpio), 0);	
		first_enable = false;
		printk("%s:bl_en_gpio off\n",__func__);
	}
	else
	{     
	       if(first_enable ==false)
	       {
			gpio_set_value((ctrl_pdata->bl_en_gpio), 1);		
			first_enable = true;
			printk("%s:bl_en_gpio on\n",__func__);
	       }
	}
#endif

	/*
	 * Some backlight controllers specify a minimum duty cycle
	 * for the backlight brightness. If the brightness is less
	 * than it, the controller can malfunction.
	 */

	if ((bl_level < pdata->panel_info.bl_min) && (bl_level != 0))
		bl_level = pdata->panel_info.bl_min;

	switch (ctrl_pdata->bklt_ctrl) {
	case BL_WLED:
		led_trigger_event(bl_led_trigger, bl_level);
		break;
	case BL_PWM:
		mdss_dsi_panel_bklt_pwm(ctrl_pdata, bl_level);
		break;
	case BL_DCS_CMD:
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
		mdss_dsi_set_tx_power_mode(0 , pdata );
		msleep(2);
#endif
		mdss_dsi_panel_bklt_dcs(ctrl_pdata, bl_level);
#if defined (CONFIG_F_SKYDISP_EF56_SS) || defined (CONFIG_F_SKYDISP_EF59_SS) || defined (CONFIG_F_SKYDISP_EF60_SS)
		msleep(1);
		mdss_dsi_set_tx_power_mode(1 , pdata);
#endif
		if (mdss_dsi_is_master_ctrl(ctrl_pdata)) {
			struct mdss_dsi_ctrl_pdata *sctrl =
				mdss_dsi_get_slave_ctrl();
			if (!sctrl) {
				pr_err("%s: Invalid slave ctrl data\n",
					__func__);
				return;
			}
			mdss_dsi_panel_bklt_dcs(sctrl, bl_level);
		}
		break;
	default:
		pr_err("%s: Unknown bl_ctrl configuration\n",
			__func__);
		break;
	}
}

static int mdss_dsi_panel_on(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	mipi  = &pdata->panel_info.mipi;

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);
#if defined (CONFIG_F_SKYDISP_EF63_SS) && (CONFIG_BOARD_VER >= CONFIG_WS10)
	gpio_set_value((ctrl->octa_rst_gpio), 1);
	msleep(10);
	gpio_set_value((ctrl->octa_rst_gpio), 0);
	msleep(10);
	gpio_set_value((ctrl->octa_rst_gpio), 1);
	msleep(10);
#endif

#ifdef CONFIG_F_SKYDISP_CMDS_CONTROL	
	if(ctrl->lcd_cmds_check == false){
#endif

		if (ctrl->on_cmds.cmd_cnt){
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_F_SKYDISP_EF60_SS)
	       	if(ctrl->lcd_on_skip_during_bootup)
				ctrl->on_cmds.buf[189] = 0x00;
#elif defined(CONFIG_F_SKYDISP_EF59_SS)
	      		 if(ctrl->lcd_on_skip_during_bootup)
				ctrl->on_cmds.buf[160] = 0x00;
#endif

#ifdef F_SKYDISP_MAGNAIC_OPERATING_BEFORE_TP20
			if(ctrl->manufacture_id == MAGNA_DRIVER_IC)
				mdss_dsi_panel_cmds_send(ctrl, &ctrl->magnaic_on_cmds);
			else				
#endif
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds);
		}
#ifdef CONFIG_F_SKYDISP_CMDS_CONTROL		
	}else if(ctrl->lcd_cmds_check == true){
		if (ctrl->on_cmds_user.cmd_cnt)
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->on_cmds_user);
		pr_info("user LCD on cmds---------------->\n");
	}
#endif

#ifdef F_LSI_VDDM_OFFSET_RD_WR
		if(ctrl->manufacture_id == SAMSUNG_DRIVER_IC){
			mdss_dsi_panel_cmds_send(ctrl, &ctrl->vddm_offset_write_cmds);
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
			ctrl->onflag = true;
#endif
		}
#endif
	pr_err("%s:-\n", __func__);

#ifdef CONFIG_POWERSUSPEND
	set_power_suspend_state_panel_hook(POWER_SUSPEND_INACTIVE);
#endif

	return 0;
}

static int mdss_dsi_panel_off(struct mdss_panel_data *pdata)
{
	struct mipi_panel_info *mipi;
	struct mdss_dsi_ctrl_pdata *ctrl = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	pr_debug("%s: ctrl=%p ndx=%d\n", __func__, ctrl, ctrl->ndx);

	mipi  = &pdata->panel_info.mipi;

	if (ctrl->off_cmds.cmd_cnt)
		mdss_dsi_panel_cmds_send(ctrl, &ctrl->off_cmds);
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	ctrl->onflag = false;
#endif
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_F_SKYDISP_EF60_SS) || \
	defined(CONFIG_F_SKYDISP_EF59_SS)
	if(!ctrl->lcd_on_skip_during_bootup)
		ctrl->lcd_on_skip_during_bootup = true;
#endif
#ifdef CONFIG_F_SKYDISP_HBM_FOR_AMOLED
	//pdata->hbm_flag = 0;
#endif
	pr_err("%s:-\n", __func__);

#ifdef CONFIG_POWERSUSPEND
	set_power_suspend_state_panel_hook(POWER_SUSPEND_ACTIVE);
#endif

	return 0;
}

static void mdss_dsi_parse_lane_swap(struct device_node *np, char *dlane_swap)
{
	const char *data;

	*dlane_swap = DSI_LANE_MAP_0123;
	data = of_get_property(np, "qcom,mdss-dsi-lane-map", NULL);
	if (data) {
		if (!strcmp(data, "lane_map_3012"))
			*dlane_swap = DSI_LANE_MAP_3012;
		else if (!strcmp(data, "lane_map_2301"))
			*dlane_swap = DSI_LANE_MAP_2301;
		else if (!strcmp(data, "lane_map_1230"))
			*dlane_swap = DSI_LANE_MAP_1230;
		else if (!strcmp(data, "lane_map_0321"))
			*dlane_swap = DSI_LANE_MAP_0321;
		else if (!strcmp(data, "lane_map_1032"))
			*dlane_swap = DSI_LANE_MAP_1032;
		else if (!strcmp(data, "lane_map_2103"))
			*dlane_swap = DSI_LANE_MAP_2103;
		else if (!strcmp(data, "lane_map_3210"))
			*dlane_swap = DSI_LANE_MAP_3210;
	}
}

static void mdss_dsi_parse_trigger(struct device_node *np, char *trigger,
		char *trigger_key)
{
	const char *data;

	*trigger = DSI_CMD_TRIGGER_SW;
	data = of_get_property(np, trigger_key, NULL);
	if (data) {
		if (!strcmp(data, "none"))
			*trigger = DSI_CMD_TRIGGER_NONE;
		else if (!strcmp(data, "trigger_te"))
			*trigger = DSI_CMD_TRIGGER_TE;
		else if (!strcmp(data, "trigger_sw_seof"))
			*trigger = DSI_CMD_TRIGGER_SW_SEOF;
		else if (!strcmp(data, "trigger_sw_te"))
			*trigger = DSI_CMD_TRIGGER_SW_TE;
	}
}


static int mdss_dsi_parse_dcs_cmds(struct device_node *np,
		struct dsi_panel_cmds *pcmds, char *cmd_key, char *link_key)
{
	const char *data;
	int blen = 0, len;
	char *buf, *bp;
	struct dsi_ctrl_hdr *dchdr;
	int i, cnt;

	data = of_get_property(np, cmd_key, &blen);
	if (!data) {
		pr_err("%s: failed, key=%s\n", __func__, cmd_key);
		return -ENOMEM;
	}

	buf = kzalloc(sizeof(char) * blen, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	memcpy(buf, data, blen);

	/* scan dcs commands */
	bp = buf;
	len = blen;
	cnt = 0;
	while (len > sizeof(*dchdr)) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		dchdr->dlen = ntohs(dchdr->dlen);
		if (dchdr->dlen > len) {
			pr_err("%s: dtsi cmd=%x error, len=%d",
				__func__, dchdr->dtype, dchdr->dlen);
			goto exit_free;
		}
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}

	if (len != 0) {
		pr_err("%s: dcs_cmd=%x len=%d error!",
				__func__, buf[0], blen);
		goto exit_free;
	}

	pcmds->cmds = kzalloc(cnt * sizeof(struct dsi_cmd_desc),
						GFP_KERNEL);
	if (!pcmds->cmds)
		goto exit_free;

	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;

	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dchdr = *dchdr;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}

	/*Set default link state to LP Mode*/
	pcmds->link_state = DSI_LP_MODE;

	if (link_key) {
		data = of_get_property(np, link_key, NULL);
		if (data && !strcmp(data, "dsi_hs_mode"))
			pcmds->link_state = DSI_HS_MODE;
		else
			pcmds->link_state = DSI_LP_MODE;
	}

	pr_debug("%s: dcs_cmd=%x len=%d, cmd_cnt=%d link_state=%d\n", __func__,
		pcmds->buf[0], pcmds->blen, pcmds->cmd_cnt, pcmds->link_state);

	return 0;

exit_free:
	kfree(buf);
	return -ENOMEM;
}


int mdss_panel_get_dst_fmt(u32 bpp, char mipi_mode, u32 pixel_packing,
				char *dst_format)
{
	int rc = 0;
	switch (bpp) {
	case 3:
		*dst_format = DSI_CMD_DST_FORMAT_RGB111;
		break;
	case 8:
		*dst_format = DSI_CMD_DST_FORMAT_RGB332;
		break;
	case 12:
		*dst_format = DSI_CMD_DST_FORMAT_RGB444;
		break;
	case 16:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB565;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB565;
			break;
		}
		break;
	case 18:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB666;
			break;
		default:
			if (pixel_packing == 0)
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666;
			else
				*dst_format = DSI_VIDEO_DST_FORMAT_RGB666_LOOSE;
			break;
		}
		break;
	case 24:
		switch (mipi_mode) {
		case DSI_VIDEO_MODE:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		case DSI_CMD_MODE:
			*dst_format = DSI_CMD_DST_FORMAT_RGB888;
			break;
		default:
			*dst_format = DSI_VIDEO_DST_FORMAT_RGB888;
			break;
		}
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}


static int mdss_dsi_parse_fbc_params(struct device_node *np,
				struct mdss_panel_info *panel_info)
{
	int rc, fbc_enabled = 0;
	u32 tmp;

	fbc_enabled = of_property_read_bool(np,	"qcom,mdss-dsi-fbc-enable");
	if (fbc_enabled) {
		pr_debug("%s:%d FBC panel enabled.\n", __func__, __LINE__);
		panel_info->fbc.enabled = 1;
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bpp", &tmp);
		panel_info->fbc.target_bpp =	(!rc ? tmp : panel_info->bpp);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-packing",
				&tmp);
		panel_info->fbc.comp_mode = (!rc ? tmp : 0);
		panel_info->fbc.qerr_enable = of_property_read_bool(np,
			"qcom,mdss-dsi-fbc-quant-error");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-bias", &tmp);
		panel_info->fbc.cd_bias = (!rc ? tmp : 0);
		panel_info->fbc.pat_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-pat-mode");
		panel_info->fbc.vlc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-vlc-mode");
		panel_info->fbc.bflc_enable = of_property_read_bool(np,
				"qcom,mdss-dsi-fbc-bflc-mode");
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-h-line-budget",
				&tmp);
		panel_info->fbc.line_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-budget-ctrl",
				&tmp);
		panel_info->fbc.block_x_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-block-budget",
				&tmp);
		panel_info->fbc.block_budget = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossless-threshold", &tmp);
		panel_info->fbc.lossless_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-threshold", &tmp);
		panel_info->fbc.lossy_mode_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np, "qcom,mdss-dsi-fbc-rgb-threshold",
				&tmp);
		panel_info->fbc.lossy_rgb_thd = (!rc ? tmp : 0);
		rc = of_property_read_u32(np,
				"qcom,mdss-dsi-fbc-lossy-mode-idx", &tmp);
		panel_info->fbc.lossy_mode_idx = (!rc ? tmp : 0);
	} else {
		pr_debug("%s:%d Panel does not support FBC.\n",
				__func__, __LINE__);
		panel_info->fbc.enabled = 0;
		panel_info->fbc.target_bpp =
			panel_info->bpp;
	}
	return 0;
}

static void mdss_panel_parse_te_params(struct device_node *np,
				       struct mdss_panel_info *panel_info)
{

	u32 tmp;
	int rc = 0;
	/*
	 * TE default: dsi byte clock calculated base on 70 fps;
	 * around 14 ms to complete a kickoff cycle if te disabled;
	 * vclk_line base on 60 fps; write is faster than read;
	 * init == start == rdptr;
	 */
	panel_info->te.tear_check_en =
		!of_property_read_bool(np, "qcom,mdss-tear-check-disable");
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-cfg-height", &tmp);
	panel_info->te.sync_cfg_height = (!rc ? tmp : 0xfff0);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-init-val", &tmp);
	panel_info->te.vsync_init_val = (!rc ? tmp : panel_info->yres);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-threshold-start", &tmp);
	panel_info->te.sync_threshold_start = (!rc ? tmp : 4);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-sync-threshold-continue", &tmp);
	panel_info->te.sync_threshold_continue = (!rc ? tmp : 4);
	rc = of_property_read_u32(np, "qcom,mdss-tear-check-start-pos", &tmp);
	panel_info->te.start_pos = (!rc ? tmp : panel_info->yres);
	rc = of_property_read_u32
		(np, "qcom,mdss-tear-check-rd-ptr-trigger-intr", &tmp);
	panel_info->te.rd_ptr_irq = (!rc ? tmp : panel_info->yres + 1);
	rc = of_property_read_u32(np, "qcom,mdss-tear-check-frame-rate", &tmp);
	panel_info->te.refx100 = (!rc ? tmp : 6000);
}


static int mdss_dsi_parse_reset_seq(struct device_node *np,
		u32 rst_seq[MDSS_DSI_RST_SEQ_LEN], u32 *rst_len,
		const char *name)
{
	int num = 0, i;
	int rc;
	struct property *data;
	u32 tmp[MDSS_DSI_RST_SEQ_LEN];
	*rst_len = 0;
	data = of_find_property(np, name, &num);
	num /= sizeof(u32);
	if (!data || !num || num > MDSS_DSI_RST_SEQ_LEN || num % 2) {
		pr_debug("%s:%d, error reading %s, length found = %d\n",
			__func__, __LINE__, name, num);
	} else {
		rc = of_property_read_u32_array(np, name, tmp, num);
		if (rc)
			pr_debug("%s:%d, error reading %s, rc = %d\n",
				__func__, __LINE__, name, rc);
		else {
			for (i = 0; i < num; ++i)
				rst_seq[i] = tmp[i];
			*rst_len = num;
		}
	}
	return 0;
}

static void mdss_dsi_parse_roi_alignment(struct device_node *np,
		struct mdss_panel_info *pinfo)
{
	int len = 0;
	u32 value[6];
	struct property *data;
	data = of_find_property(np, "qcom,panel-roi-alignment", &len);
	len /= sizeof(u32);
	if (!data || (len != 6)) {
		pr_err("%s: Panel roi alignment not found", __func__);
	} else {
		int rc = of_property_read_u32_array(np,
				"qcom,panel-roi-alignment", value, len);
		if (rc)
			pr_err("%s: Error reading panel roi alignment values",
					__func__);
		else {
			pinfo->xstart_pix_align = value[0];
			pinfo->width_pix_align = value[1];
			pinfo->ystart_pix_align = value[2];
			pinfo->height_pix_align = value[3];
			pinfo->min_width = value[4];
			pinfo->min_height = value[5];
		}

		pr_info("%s: ROI alignment: [%d, %d, %d, %d, %d, %d]",
				__func__, pinfo->xstart_pix_align,
				pinfo->width_pix_align, pinfo->ystart_pix_align,
				pinfo->height_pix_align, pinfo->min_width,
				pinfo->min_height);
	}
}

static int mdss_dsi_parse_panel_features(struct device_node *np,
	struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct mdss_panel_info *pinfo;

	if (!np || !ctrl) {
		pr_err("%s: Invalid arguments\n", __func__);
		return -ENODEV;
	}

	pinfo = &ctrl->panel_data.panel_info;

	pinfo->cont_splash_enabled = of_property_read_bool(np,
		"qcom,cont-splash-enabled");

	pinfo->partial_update_enabled = of_property_read_bool(np,
		"qcom,partial-update-enabled");
	pr_info("%s:%d Partial update %s\n", __func__, __LINE__,
		(pinfo->partial_update_enabled ? "enabled" : "disabled"));
	if (pinfo->partial_update_enabled)
		ctrl->partial_update_fnc = mdss_dsi_panel_partial_update;

	pinfo->ulps_feature_enabled = of_property_read_bool(np,
		"qcom,ulps-enabled");
	pr_info("%s: ulps feature %s", __func__,
		(pinfo->ulps_feature_enabled ? "enabled" : "disabled"));
	pinfo->esd_check_enabled = of_property_read_bool(np,
		"qcom,esd-check-enabled");

	pinfo->mipi.dynamic_switch_enabled = of_property_read_bool(np,
		"qcom,dynamic-mode-switch-enabled");

	if (pinfo->mipi.dynamic_switch_enabled) {
		mdss_dsi_parse_dcs_cmds(np, &ctrl->video2cmd,
			"qcom,video-to-cmd-mode-switch-commands", NULL);

		mdss_dsi_parse_dcs_cmds(np, &ctrl->cmd2video,
			"qcom,cmd-to-video-mode-switch-commands", NULL);

		if (!ctrl->video2cmd.cmd_cnt || !ctrl->cmd2video.cmd_cnt) {
			pr_warn("No commands specified for dynamic switch\n");
			pinfo->mipi.dynamic_switch_enabled = 0;
		}
	}

	pr_info("%s: dynamic switch feature enabled: %d", __func__,
		pinfo->mipi.dynamic_switch_enabled);

	return 0;
}

static int mdss_dsi_set_refresh_rate_range(struct device_node *pan_node,
		struct mdss_panel_info *pinfo)
{
	int rc = 0;
	rc = of_property_read_u32(pan_node,
			"qcom,mdss-dsi-min-refresh-rate",
			&pinfo->min_fps);
	if (rc) {
		pr_warn("%s:%d, Unable to read min refresh rate\n",
				__func__, __LINE__);

		/*
		 * Since min refresh rate is not specified when dynamic
		 * fps is enabled, using minimum as 30
		 */
		pinfo->min_fps = MIN_REFRESH_RATE;
		rc = 0;
	}

	rc = of_property_read_u32(pan_node,
			"qcom,mdss-dsi-max-refresh-rate",
			&pinfo->max_fps);
	if (rc) {
		pr_warn("%s:%d, Unable to read max refresh rate\n",
				__func__, __LINE__);

		/*
		 * Since max refresh rate was not specified when dynamic
		 * fps is enabled, using the default panel refresh rate
		 * as max refresh rate supported.
		 */
		pinfo->max_fps = pinfo->mipi.frame_rate;
		rc = 0;
	}

	pr_info("dyn_fps: min = %d, max = %d\n",
			pinfo->min_fps, pinfo->max_fps);
	return rc;
}

static void mdss_dsi_parse_dfps_config(struct device_node *pan_node,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	const char *data;
	bool dynamic_fps;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	dynamic_fps = of_property_read_bool(pan_node,
			"qcom,mdss-dsi-pan-enable-dynamic-fps");

	if (!dynamic_fps)
		return;

	pinfo->dynamic_fps = true;
	data = of_get_property(pan_node, "qcom,mdss-dsi-pan-fps-update", NULL);
	if (data) {
		if (!strcmp(data, "dfps_suspend_resume_mode")) {
			pinfo->dfps_update = DFPS_SUSPEND_RESUME_MODE;
			pr_debug("dfps mode: suspend/resume\n");
		} else if (!strcmp(data, "dfps_immediate_clk_mode")) {
			pinfo->dfps_update = DFPS_IMMEDIATE_CLK_UPDATE_MODE;
			pr_debug("dfps mode: Immediate clk\n");
		} else if (!strcmp(data, "dfps_immediate_porch_mode")) {
			pinfo->dfps_update = DFPS_IMMEDIATE_PORCH_UPDATE_MODE;
			pr_debug("dfps mode: Immediate porch\n");
		} else {
			pinfo->dfps_update = DFPS_SUSPEND_RESUME_MODE;
			pr_debug("default dfps mode: suspend/resume\n");
		}
		mdss_dsi_set_refresh_rate_range(pan_node, pinfo);
	} else {
		pinfo->dynamic_fps = false;
		pr_debug("dfps update mode not configured: disable\n");
	}
	pinfo->new_fps = pinfo->mipi.frame_rate;

	return;
}

static int mdss_panel_parse_dt(struct device_node *np,
			struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	u32 tmp;
	int rc, i, len;
	const char *data;
	static const char *pdest;
	struct mdss_panel_info *pinfo = &(ctrl_pdata->panel_data.panel_info);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-width", &tmp);
	if (rc) {
		pr_err("%s:%d, panel width not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	pinfo->xres = (!rc ? tmp : 640);

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-height", &tmp);
	if (rc) {
		pr_err("%s:%d, panel height not specified\n",
						__func__, __LINE__);
		return -EINVAL;
	}
	pinfo->yres = (!rc ? tmp : 480);

	rc = of_property_read_u32(np,
		"qcom,mdss-pan-physical-width-dimension", &tmp);
	pinfo->physical_width = (!rc ? tmp : 0);
	rc = of_property_read_u32(np,
		"qcom,mdss-pan-physical-height-dimension", &tmp);
	pinfo->physical_height = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-left-border", &tmp);
	pinfo->lcdc.xres_pad = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-right-border", &tmp);
	if (!rc)
		pinfo->lcdc.xres_pad += tmp;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-top-border", &tmp);
	pinfo->lcdc.yres_pad = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-bottom-border", &tmp);
	if (!rc)
		pinfo->lcdc.yres_pad += tmp;
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bpp", &tmp);
	if (rc) {
		pr_err("%s:%d, bpp not specified\n", __func__, __LINE__);
		return -EINVAL;
	}
	pinfo->bpp = (!rc ? tmp : 24);
	pinfo->mipi.mode = DSI_VIDEO_MODE;
	data = of_get_property(np, "qcom,mdss-dsi-panel-type", NULL);
	if (data && !strncmp(data, "dsi_cmd_mode", 12))
		pinfo->mipi.mode = DSI_CMD_MODE;
	tmp = 0;
	data = of_get_property(np, "qcom,mdss-dsi-pixel-packing", NULL);
	if (data && !strcmp(data, "loose"))
		pinfo->mipi.pixel_packing = 1;
	else
		pinfo->mipi.pixel_packing = 0;
	rc = mdss_panel_get_dst_fmt(pinfo->bpp,
		pinfo->mipi.mode, pinfo->mipi.pixel_packing,
		&(pinfo->mipi.dst_format));
	if (rc) {
		pr_debug("%s: problem determining dst format. Set Default\n",
			__func__);
		pinfo->mipi.dst_format =
			DSI_VIDEO_DST_FORMAT_RGB888;
	}
	pdest = of_get_property(np,
		"qcom,mdss-dsi-panel-destination", NULL);

	if (pdest) {
		if (strlen(pdest) != 9) {
			pr_err("%s: Unknown pdest specified\n", __func__);
			return -EINVAL;
		}
		if (!strcmp(pdest, "display_1"))
			pinfo->pdest = DISPLAY_1;
		else if (!strcmp(pdest, "display_2"))
			pinfo->pdest = DISPLAY_2;
		else {
			pr_debug("%s: incorrect pdest. Set Default\n",
				__func__);
			pinfo->pdest = DISPLAY_1;
		}
	} else {
		pr_debug("%s: pdest not specified. Set Default\n",
				__func__);
		pinfo->pdest = DISPLAY_1;
	}
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-front-porch", &tmp);
	pinfo->lcdc.h_front_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-back-porch", &tmp);
	pinfo->lcdc.h_back_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-pulse-width", &tmp);
	pinfo->lcdc.h_pulse_width = (!rc ? tmp : 2);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-h-sync-skew", &tmp);
	pinfo->lcdc.hsync_skew = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-back-porch", &tmp);
	pinfo->lcdc.v_back_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-front-porch", &tmp);
	pinfo->lcdc.v_front_porch = (!rc ? tmp : 6);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-v-pulse-width", &tmp);
	pinfo->lcdc.v_pulse_width = (!rc ? tmp : 2);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-underflow-color", &tmp);
	pinfo->lcdc.underflow_clr = (!rc ? tmp : 0xff);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-border-color", &tmp);
	pinfo->lcdc.border_clr = (!rc ? tmp : 0);
	pinfo->bklt_ctrl = UNKNOWN_CTRL;
	data = of_get_property(np, "qcom,mdss-dsi-bl-pmic-control-type", NULL);
	if (data) {
		if (!strncmp(data, "bl_ctrl_wled", 12)) {
			led_trigger_register_simple("bkl-trigger",
				&bl_led_trigger);
			pr_debug("%s: SUCCESS-> WLED TRIGGER register\n",
				__func__);
			ctrl_pdata->bklt_ctrl = BL_WLED;
		} else if (!strncmp(data, "bl_ctrl_pwm", 11)) {
			ctrl_pdata->bklt_ctrl = BL_PWM;
			rc = of_property_read_u32(np,
				"qcom,mdss-dsi-bl-pmic-pwm-frequency", &tmp);
			if (rc) {
				pr_err("%s:%d, Error, panel pwm_period\n",
						__func__, __LINE__);
				return -EINVAL;
			}
			ctrl_pdata->pwm_period = tmp;
			rc = of_property_read_u32(np,
				"qcom,mdss-dsi-bl-pmic-bank-select", &tmp);
			if (rc) {
				pr_err("%s:%d, Error, dsi lpg channel\n",
						__func__, __LINE__);
				return -EINVAL;
			}
			ctrl_pdata->pwm_lpg_chan = tmp;
			tmp = of_get_named_gpio(np,
				"qcom,mdss-dsi-pwm-gpio", 0);
			ctrl_pdata->pwm_pmic_gpio = tmp;
		} else if (!strncmp(data, "bl_ctrl_dcs", 11)) {
			ctrl_pdata->bklt_ctrl = BL_DCS_CMD;
		}
	}
	rc = of_property_read_u32(np, "qcom,mdss-brightness-max-level", &tmp);
	pinfo->brightness_max = (!rc ? tmp : MDSS_MAX_BL_BRIGHTNESS);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bl-min-level", &tmp);
	pinfo->bl_min = (!rc ? tmp : 0);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-bl-max-level", &tmp);
	pinfo->bl_max = (!rc ? tmp : 255);
	ctrl_pdata->bklt_max = pinfo->bl_max;

	rc = of_property_read_u32(np, "qcom,mdss-dsi-interleave-mode", &tmp);
	pinfo->mipi.interleave_mode = (!rc ? tmp : 0);

	pinfo->mipi.vsync_enable = of_property_read_bool(np,
		"qcom,mdss-dsi-te-check-enable");
	pinfo->mipi.hw_vsync_mode = of_property_read_bool(np,
		"qcom,mdss-dsi-te-using-te-pin");

	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-h-sync-pulse", &tmp);
	pinfo->mipi.pulse_mode_hsa_he = (!rc ? tmp : false);

	pinfo->mipi.hfp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hfp-power-mode");
	pinfo->mipi.hsa_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hsa-power-mode");
	pinfo->mipi.hbp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-hbp-power-mode");
	pinfo->mipi.last_line_interleave_en = of_property_read_bool(np,
		"qcom,mdss-dsi-last-line-interleave");
	pinfo->mipi.bllp_power_stop = of_property_read_bool(np,
		"qcom,mdss-dsi-bllp-power-mode");
	pinfo->mipi.eof_bllp_power_stop = of_property_read_bool(
		np, "qcom,mdss-dsi-bllp-eof-power-mode");
	pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_PULSE;
	data = of_get_property(np, "qcom,mdss-dsi-traffic-mode", NULL);
	if (data) {
		if (!strcmp(data, "non_burst_sync_event"))
			pinfo->mipi.traffic_mode = DSI_NON_BURST_SYNCH_EVENT;
		else if (!strcmp(data, "burst_mode"))
			pinfo->mipi.traffic_mode = DSI_BURST_MODE;
	}
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-dcs-command", &tmp);
	pinfo->mipi.insert_dcs_cmd =
			(!rc ? tmp : 1);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-wr-mem-continue", &tmp);
	pinfo->mipi.wr_mem_continue =
			(!rc ? tmp : 0x3c);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-wr-mem-start", &tmp);
	pinfo->mipi.wr_mem_start =
			(!rc ? tmp : 0x2c);
	rc = of_property_read_u32(np,
		"qcom,mdss-dsi-te-pin-select", &tmp);
	pinfo->mipi.te_sel =
			(!rc ? tmp : 1);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-virtual-channel-id", &tmp);
	pinfo->mipi.vc = (!rc ? tmp : 0);
	pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RGB;
	data = of_get_property(np, "qcom,mdss-dsi-color-order", NULL);
	if (data) {
		if (!strcmp(data, "rgb_swap_rbg"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_RBG;
		else if (!strcmp(data, "rgb_swap_bgr"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BGR;
		else if (!strcmp(data, "rgb_swap_brg"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_BRG;
		else if (!strcmp(data, "rgb_swap_grb"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GRB;
		else if (!strcmp(data, "rgb_swap_gbr"))
			pinfo->mipi.rgb_swap = DSI_RGB_SWAP_GBR;
	}
	pinfo->mipi.data_lane0 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-0-state");
	pinfo->mipi.data_lane1 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-1-state");
	pinfo->mipi.data_lane2 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-2-state");
	pinfo->mipi.data_lane3 = of_property_read_bool(np,
		"qcom,mdss-dsi-lane-3-state");

	rc = of_property_read_u32(np, "qcom,mdss-dsi-t-clk-pre", &tmp);
	pinfo->mipi.t_clk_pre = (!rc ? tmp : 0x24);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-t-clk-post", &tmp);
	pinfo->mipi.t_clk_post = (!rc ? tmp : 0x03);

	pinfo->mipi.rx_eot_ignore = of_property_read_bool(np,
		"qcom,mdss-dsi-rx-eot-ignore");
	pinfo->mipi.tx_eot_append = of_property_read_bool(np,
		"qcom,mdss-dsi-tx-eot-append");

	rc = of_property_read_u32(np, "qcom,mdss-dsi-stream", &tmp);
	pinfo->mipi.stream = (!rc ? tmp : 0);

	data = of_get_property(np, "qcom,mdss-dsi-panel-mode-gpio-state", NULL);
	if (data) {
		if (!strcmp(data, "high"))
			pinfo->mode_gpio_state = MODE_GPIO_HIGH;
		else if (!strcmp(data, "low"))
			pinfo->mode_gpio_state = MODE_GPIO_LOW;
	} else {
		pinfo->mode_gpio_state = MODE_GPIO_NOT_VALID;
	}

	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-framerate", &tmp);
	pinfo->mipi.frame_rate = (!rc ? tmp : 60);
	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-clockrate", &tmp);
	pinfo->clk_rate = (!rc ? tmp : 0);
	data = of_get_property(np, "qcom,mdss-dsi-panel-timings", &len);
	if ((!data) || (len != 12)) {
		pr_err("%s:%d, Unable to read Phy timing settings",
		       __func__, __LINE__);
		goto error;
	}
	for (i = 0; i < len; i++)
		pinfo->mipi.dsi_phy_db.timing[i] = data[i];

	pinfo->mipi.lp11_init = of_property_read_bool(np,
					"qcom,mdss-dsi-lp11-init");
	rc = of_property_read_u32(np, "qcom,mdss-dsi-init-delay-us", &tmp);
	pinfo->mipi.init_delay = (!rc ? tmp : 0);

	mdss_dsi_parse_fbc_params(np, pinfo);
	mdss_dsi_parse_roi_alignment(np, pinfo);

	mdss_dsi_parse_trigger(np, &(pinfo->mipi.mdp_trigger),
		"qcom,mdss-dsi-mdp-trigger");

	mdss_dsi_parse_trigger(np, &(pinfo->mipi.dma_trigger),
		"qcom,mdss-dsi-dma-trigger");

	mdss_dsi_parse_lane_swap(np, &(pinfo->mipi.dlane_swap));

	mdss_dsi_parse_reset_seq(np, pinfo->rst_seq, &(pinfo->rst_seq_len),
		"qcom,mdss-dsi-reset-sequence");
	mdss_panel_parse_te_params(np, pinfo);

	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->on_cmds,
		"qcom,mdss-dsi-on-command", "qcom,mdss-dsi-on-command-state");

#ifdef F_SKYDISP_MAGNAIC_OPERATING_BEFORE_TP20
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->magnaic_on_cmds,
		"qcom,mdss-dsi-magnaic-on-command", "qcom,mdss-dsi-on-command-state");
#endif

	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->off_cmds,
		"qcom,mdss-dsi-off-command", "qcom,mdss-dsi-off-command-state");
#ifdef CONFIG_F_SKYDISP_CABC_CONTROL
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->cabc_cmds,
		"qcom,mdss-dsi-cabc-command", "qcom,mdss-dsi-cabc-command-state");
#endif	
#ifdef F_LSI_VDDM_OFFSET_RD_WR
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->display_on_cmds,
		"qcom,mdss-dsi-display-on-cmds", "qcom,mdss-dsi-on-command-state");
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->read_enable_cmds,
		"qcom,mdss-dsi-panel-read-enable-cmds", "qcom,mdss-dsi-on-command-state");
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->read_disable_cmds,
		"qcom,mdss-dsi-panel-read-disable-cmds", "qcom,mdss-dsi-on-command-state");	
	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->vddm_offset_write_cmds,
		"qcom,mdss-dsi-panel-ldi-vddm-offset-write-cmds", "qcom,mdss-dsi-on-command-state");
#endif

	mdss_dsi_parse_dcs_cmds(np, &ctrl_pdata->status_cmds,
			"qcom,mdss-dsi-panel-status-command",
				"qcom,mdss-dsi-panel-status-command-state");
	rc = of_property_read_u32(np, "qcom,mdss-dsi-panel-status-value", &tmp);
	ctrl_pdata->status_value = (!rc ? tmp : 0);


	ctrl_pdata->status_mode = ESD_MAX;
	rc = of_property_read_string(np,
				"qcom,mdss-dsi-panel-status-check-mode", &data);
	if (!rc) {
		if (!strcmp(data, "bta_check"))
			ctrl_pdata->status_mode = ESD_BTA;
		else if (!strcmp(data, "reg_read"))
			ctrl_pdata->status_mode = ESD_REG;
	}

	rc = mdss_dsi_parse_panel_features(np, ctrl_pdata);
	if (rc) {
		pr_err("%s: failed to parse panel features\n", __func__);
		goto error;
	}

	mdss_dsi_parse_dfps_config(np, ctrl_pdata);

	return 0;

error:
	return -EINVAL;
}

int mdss_dsi_panel_init(struct device_node *node,
	struct mdss_dsi_ctrl_pdata *ctrl_pdata,
	bool cmd_cfg_cont_splash)
{
	int rc = 0;
	static const char *panel_name;
	struct mdss_panel_info *pinfo;
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
	int i = 0;
	oem_pm_smem_vendor1_data_type *smem_id_vendor1_ptr;
#endif

	if (!node || !ctrl_pdata) {
		pr_err("%s: Invalid arguments\n", __func__);
		return -ENODEV;
	}

	pinfo = &ctrl_pdata->panel_data.panel_info;

	pr_debug("%s:%d\n", __func__, __LINE__);
	panel_name = of_get_property(node, "qcom,mdss-dsi-panel-name", NULL);
	if (!panel_name)
		pr_info("%s:%d, Panel name not specified\n",
						__func__, __LINE__);
	else
		pr_info("%s: Panel Name = %s\n", __func__, panel_name);

	rc = mdss_panel_parse_dt(node, ctrl_pdata);
	if (rc) {
		pr_err("%s:%d panel dt parse failed\n", __func__, __LINE__);
		return rc;
	}

	if (!cmd_cfg_cont_splash)
		pinfo->cont_splash_enabled = false;
	pr_info("%s: Continuous splash %s", __func__,
		pinfo->cont_splash_enabled ? "enabled" : "disabled");

	pinfo->dynamic_switch_pending = false;
	pinfo->is_lpm_mode = false;

	ctrl_pdata->on = mdss_dsi_panel_on;
	ctrl_pdata->off = mdss_dsi_panel_off;
	ctrl_pdata->panel_data.set_backlight = mdss_dsi_panel_bl_ctrl;
	ctrl_pdata->switch_mode = mdss_dsi_panel_switch_mode;

#ifdef CONFIG_F_SKYDISP_AMOLED_READ_DATA
	ctrl_pdata->panel_data.panel_read = pannel_read;
#endif
#ifdef CONFIG_F_SKYDISP_SMARTDIMMING
	INIT_DELAYED_WORK_DEFERRABLE(&ctrl_pdata->panel_read_work, mtp_read_work);
	schedule_delayed_work(&ctrl_pdata->panel_read_work, msecs_to_jiffies(3000));

	for(ctrl_pdata->mtp_cnt = 0;ctrl_pdata->mtp_cnt < V255_MAX; ctrl_pdata->mtp_cnt++){
		for(i = 0 ; i < RGB_MAX; i++){
			if(ctrl_pdata->mtp_cnt == VT)
				ctrl_pdata->panel_read_mtp.panel_gamma_data[i][ctrl_pdata->mtp_cnt] = 0;
			else if(ctrl_pdata->mtp_cnt > VT && ctrl_pdata->mtp_cnt < V255)
				ctrl_pdata->panel_read_mtp.panel_gamma_data[i][ctrl_pdata->mtp_cnt] = 128;
			else if(ctrl_pdata->mtp_cnt == V255)
				ctrl_pdata->panel_read_mtp.panel_gamma_data[i][ctrl_pdata->mtp_cnt] = 256;
		}
	}
	//panel_gamma_data
	ctrl_pdata->mtp_cnt = 0;	
	ctrl_pdata->gamma_sort = panel_gamma_sort;
       ctrl_pdata->gamma_buf = oled_gamma;

	smem_id_vendor1_ptr =  (oem_pm_smem_vendor1_data_type*)smem_alloc(SMEM_ID_VENDOR1,sizeof(oem_pm_smem_vendor1_data_type));
	if(smem_id_vendor1_ptr->power_on_mode == 0){
		ctrl_pdata->offline_charger=1;
	}

	pr_err(" Boot Mode: %s\n",ctrl_pdata->offline_charger ? "Offline": "Online");

#endif
	return 0;
}
