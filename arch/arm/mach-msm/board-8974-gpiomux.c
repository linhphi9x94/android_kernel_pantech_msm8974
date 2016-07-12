/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>

//++ p11309 - 2013.12.23 for Not Used GPIO
// #define KS8851_IRQ_GPIO 94
#define KS8851_IRQ_GPIO 41

#define WLAN_CLK	40
#define WLAN_SET	39
#define WLAN_DATA0	38
#define WLAN_DATA1	37
#define WLAN_DATA2	36

static struct gpiomux_setting ap2mdm_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting mdm2ap_status_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting mdm2ap_errfatal_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting mdm2ap_pblrdy = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};


static struct gpiomux_setting ap2mdm_soft_reset_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting ap2mdm_wakeup = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct msm_gpiomux_config mdm_configs[] __initdata = {
	/* AP2MDM_STATUS */
	{
		.gpio = 105,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* MDM2AP_STATUS */
	{
		.gpio = 46,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_status_cfg,
		}
	},
	/* MDM2AP_ERRFATAL */
	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_errfatal_cfg,
		}
	},
	/* AP2MDM_ERRFATAL */
	{
		.gpio = 106,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_cfg,
		}
	},
	/* AP2MDM_SOFT_RESET, aka AP2MDM_PON_RESET_N */
	{
		.gpio = 24,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_soft_reset_cfg,
		}
	},
	/* AP2MDM_WAKEUP */
	{
		.gpio = 104,
		.settings = {
			[GPIOMUX_SUSPENDED] = &ap2mdm_wakeup,
		}
	},
	/* MDM2AP_PBL_READY*/
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mdm2ap_pblrdy,
		}
	},
};

#if defined(CONFIG_PANTECH_CONSOLE_UART10)
static struct gpiomux_setting gpio_uart_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};
#else
static struct gpiomux_setting gpio_uart_config = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
#endif

static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static struct gpiomux_setting gpio_eth_config = {
	.pull = GPIOMUX_PULL_UP,
	.drv = GPIOMUX_DRV_2MA,
	.func = GPIOMUX_FUNC_GPIO,
};
#if !defined(CONFIG_PANTECH_CONSOLE_UART1) && !defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058) && !defined(CONFIG_SKY_DMB_SPI_HW)//temp fix build error
static struct gpiomux_setting gpio_spi_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif
#if !defined(CONFIG_F_SKYDISP_EF56_SS)
static struct gpiomux_setting gpio_spi_susp_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting gpio_spi_cs1_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_UP,
};
#endif
static struct msm_gpiomux_config msm_eth_configs[] = {
	{
		.gpio = KS8851_IRQ_GPIO,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_eth_config,
		}
	},
};
#endif
#if defined(CONFIG_INPUT_FPC1080) || defined(CONFIG_INPUT_FPC1080_MODULE)//p11774 add for fpc1080's spi
static struct gpiomux_setting gpio_blsp8_spi1_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting gpio_blsp8_spi_suspend_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif
#ifdef CONFIG_SKY_DMB_SPI_HW
static struct gpiomux_setting gpio_blsp1_spi1_config = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct gpiomux_setting gpio_suspend_config[] = {
	{
		.func = GPIOMUX_FUNC_GPIO,  /* IN-NP */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},
	{
		.func = GPIOMUX_FUNC_GPIO,  /* O-LOW */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},
};

#ifndef CONFIG_PANTECH_USB_REDRIVER_EN_CONTROL
#if !defined(CONFIG_PANTECH_CAMERA) && !defined(CONFIG_PANTECH_CAMERA_EF63_SS)
static struct gpiomux_setting gpio_epm_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif
#endif

#if !defined(CONFIG_TOUCHSCREEN_FTS)
static struct gpiomux_setting gpio_epm_marker_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif

static struct gpiomux_setting wcnss_5wire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5wire_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting wcnss_5gpio_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting wcnss_5gpio_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting ath_gpio_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting ath_gpio_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting gpio_i2c_config = {
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_i2c_act_config = {
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && !defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS) 
static struct gpiomux_setting lcd_en_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting lcd_en_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

#if defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L)
#if (CONFIG_BOARD_VER == CONFIG_PT10)

static struct gpiomux_setting touch_resout_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting touch_resout_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#endif
#else
static struct gpiomux_setting touch_resout_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting touch_resout_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif
static struct gpiomux_setting touch_int_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting touch_int_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting taiko_reset = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_6MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting taiko_int = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting hap_lvl_shft_suspended_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hap_lvl_shft_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct msm_gpiomux_config hap_lvl_shft_config[] __initdata = {
	{
		.gpio = 86,
		.settings = {
			[GPIOMUX_SUSPENDED] = &hap_lvl_shft_suspended_config,
			[GPIOMUX_ACTIVE] = &hap_lvl_shft_active_config,
		},
	},
};

static struct msm_gpiomux_config msm_touch_configs[] __initdata = {

#if defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L)
#if (CONFIG_BOARD_VER == CONFIG_PT10)
	{
		.gpio      = 60,		/* TOUCH RESET */
		.settings = {
			[GPIOMUX_ACTIVE] = &touch_resout_act_cfg,
			[GPIOMUX_SUSPENDED] = &touch_resout_sus_cfg,
		},
	},
#endif
#else
	{
		.gpio      = 60,		/* TOUCH RESET */
		.settings = {
			[GPIOMUX_ACTIVE] = &touch_resout_act_cfg,
			[GPIOMUX_SUSPENDED] = &touch_resout_sus_cfg,
		},
	},

#endif
	{
		.gpio      = 61,		/* TOUCH IRQ */
		.settings = {
			[GPIOMUX_ACTIVE] = &touch_int_act_cfg,
			[GPIOMUX_SUSPENDED] = &touch_int_sus_cfg,
		},
	},
};

static struct gpiomux_setting hsic_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hsic_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_12MA,
	.pull = GPIOMUX_PULL_NONE,
};
#if !defined(CONFIG_PN544) && !defined(CONFIG_PN547)
static struct gpiomux_setting hsic_hub_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
#endif /*!CONFIG_PN544 && !CONFIG_PN547*/

static struct gpiomux_setting hsic_resume_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting hsic_resume_susp_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config msm_hsic_configs[] = {
	{
		.gpio = 144,               /*HSIC_STROBE */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 145,               /* HSIC_DATA */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_resume_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_resume_susp_cfg,
		},
	},
};

#if !defined(CONFIG_PN544) && !defined(CONFIG_PN547)
static struct msm_gpiomux_config msm_hsic_hub_configs[] = {
	{
		.gpio = 50,               /* HSIC_HUB_INT_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &hsic_hub_act_cfg,
			[GPIOMUX_SUSPENDED] = &hsic_sus_cfg,
		},
	},
};
#endif /*!CONFIG_PN544 && !CONFIG_PN547*/

#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && \
	!defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS) 
static struct gpiomux_setting mhl_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mhl_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};

static struct gpiomux_setting hdmi_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_16MA,
	.pull = GPIOMUX_PULL_DOWN,
};
#endif

#ifdef CONFIG_PANTECH_USB_REDRIVER_EN_CONTROL
static struct gpiomux_setting usb3_redriver_enable_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting usb3_redriver_enable_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
#endif

//++ p13106 ls4 For Touch Pen
#if defined (CONFIG_MACH_MSM8974_EF59S) || defined (CONFIG_MACH_MSM8974_EF59K) || defined (CONFIG_MACH_MSM8974_EF59L)
static struct gpiomux_setting touch_pen_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting touch_pen_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config touch_pen_configs[] __initdata = {
	{	/* melfas INTERRUPT */
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &touch_pen_act_cfg,
			[GPIOMUX_SUSPENDED] = &touch_pen_sus_cfg,
		},
	},
};

#endif
//-- p13106 ls2

#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && \
	!defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS)
static struct msm_gpiomux_config msm_mhl_configs[] __initdata = {
	{
		/* mhl-sii8334 pwr */
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mhl_suspend_config,
			[GPIOMUX_ACTIVE]    = &mhl_active_1_cfg,
		},
	},
	{
		/* mhl-sii8334 intr */
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &mhl_suspend_config,
			[GPIOMUX_ACTIVE]    = &mhl_active_1_cfg,
		},
	},
};

static struct msm_gpiomux_config msm_hdmi_configs[] __initdata = {
	{
		.gpio = 31,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 32,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_cfg,
		},
	},
};
#endif
static struct gpiomux_setting gpio_uart7_active_cfg = {
	.func = GPIOMUX_FUNC_3,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting gpio_uart7_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config msm_blsp2_uart7_configs[] __initdata = {
	{
		.gpio	= 41,	/* BLSP2 UART7 TX */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
	{
		.gpio	= 42,	/* BLSP2 UART7 RX */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
	{
		.gpio	= 43,	/* BLSP2 UART7 CTS */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
	{
		.gpio	= 44,	/* BLSP2 UART7 RFR */
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_uart7_active_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_uart7_suspend_cfg,
		},
	},
};
#if !defined(CONFIG_INPUT_FPC1080) || !defined(CONFIG_INPUT_FPC1080_MODULE)//p11774 blocked for fpc1080's spi
static struct msm_gpiomux_config msm_rumi_blsp_configs[] __initdata = {
	{
		.gpio      = 45,	/* BLSP2 UART8 TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 46,	/* BLSP2 UART8 RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
};
#endif

#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && \
	!defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS) 
static struct msm_gpiomux_config msm_lcd_configs[] __initdata = {
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_en_sus_cfg,
		},
	},
};
#elif defined(CONFIG_F_SKYDISP_EF56_SS)

static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_det_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,	
};
static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_det_suspend_cfg,
		},
	},		
};
#if (CONFIG_BOARD_VER == CONFIG_WS20)
static struct gpiomux_setting lcd_vci_i2c_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vci_i2c_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
#endif
static struct gpiomux_setting lcd_vcin_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct gpiomux_setting lcd_vcin_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {
#if (CONFIG_BOARD_VER == CONFIG_WS20)
	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vci_i2c_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vci_i2c_suspend_cfg,
		},
	},
	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vci_i2c_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vci_i2c_suspend_cfg,
		},
	},
#endif	
	{
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE] = &lcd_vcin_en_active_cfg,		
			[GPIOMUX_SUSPENDED] = &lcd_vcin_en_suspend_cfg,
		},
	},	
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_bl_swire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
	{
		.gpio = 89,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_bl_swire_suspend_cfg,
		},
	},		
};


#elif defined(CONFIG_F_SKYDISP_EF59_SS)

static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_det_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,	
};
static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_det_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_bl_swire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_bl_swire_suspend_cfg,
		},
	},		
};
#elif defined(CONFIG_F_SKYDISP_EF60_SS)

static struct gpiomux_setting lcd_rst_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,		
};
static struct gpiomux_setting lcd_rst_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_det_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,	
};
static struct msm_gpiomux_config lcd_common_configs[] __initdata = {
	{
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_rst_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_rst_suspend_cfg ,
		},
	},
	{
		.gpio = 12,
		.settings = {
			[GPIOMUX_SUSPENDED] = &lcd_det_suspend_cfg,
		},
	},		
};
static struct gpiomux_setting lcd_vcin_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct gpiomux_setting lcd_vcin_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vcip_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vcip_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,		
};

static struct gpiomux_setting lcd_vddio_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct gpiomux_setting lcd_vddio_en_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_OUT_HIGH,	
};
static struct msm_gpiomux_config lcd_ext_power_configs[] __initdata = {
	
	{
		.gpio = 8,
		.settings = {
			[GPIOMUX_ACTIVE] = &lcd_vcin_en_active_cfg,		
			[GPIOMUX_SUSPENDED] = &lcd_vcin_en_suspend_cfg,
		},
	},	
	{
		.gpio = 34,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vcip_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vcip_en_suspend_cfg,
		},
	},		
	{
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_vddio_en_active_cfg,
			[GPIOMUX_SUSPENDED] = &lcd_vddio_en_suspend_cfg,
		},
	},		
};

static struct gpiomux_setting lcd_bl_en_active_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,	
};

static struct msm_gpiomux_config lcd_bl_configs[] __initdata = {
	{
		.gpio = 57,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lcd_bl_en_active_cfg,
		},
	},	
	
};

#elif defined(CONFIG_F_SKYDISP_EF63_SS) 




#endif


static struct msm_gpiomux_config msm_epm_configs[] __initdata = {
#if !defined(CONFIG_PANTECH_CAMERA) && !defined(CONFIG_PANTECH_CAMERA_EF63_SS)
	{
		.gpio      = 81,		/* EPM enable */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_epm_config,
		},
	},
#endif
#if !defined(CONFIG_PANTECH_PMIC_CHARGER_BQ2419X)
	{
		.gpio      = 85,		/* EPM MARKER2 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_epm_marker_config,
		},
	},
#endif	

#if !defined(CONFIG_TOUCHSCREEN_FTS)
	{
		.gpio      = 96,		/* EPM MARKER1 */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_epm_marker_config,
		},
	},
#endif

};

#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)  /* actuator qup */
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L) || defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)       /* MKS. */
static struct gpiomux_setting gpio_sc_en_susp_config = {	// GPIO_4
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_sc_sysok_susp_config = {	// GPIO_5
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_fuelalrt_n_susp_cfg = {	// GPIO_27
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
#if defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
static struct gpiomux_setting piezo_en_active_cfg = { // GPIO_68
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	//.pull = GPIOMUX_PULL_NONE, //p12911
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif
static struct gpiomux_setting gpio_sc_susp_susp_cfg = {	// GPIO_78
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
static struct gpiomux_setting gpio_sc_stat_susp_cfg = {	// GPIO_85
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	//.pull = GPIOMUX_PULL_NONE,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
#endif
#if ((defined (CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L)) && (CONFIG_BOARD_VER > CONFIG_WS20)) || defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) /*|| defined (T_NAMI)*/
static struct gpiomux_setting gpio_i2c_sda_config = {
	.func = GPIOMUX_FUNC_4,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting gpio_i2c_scl_config = {
	.func = GPIOMUX_FUNC_5,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_sc_i2c_sda_susp_cfg = {	// GPIO_83
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
static struct gpiomux_setting gpio_sc_i2c_scl_susp_cfg = {	// GPIO_84
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif
#endif

#if defined(CONFIG_PANTECH_EF63_PMIC_FUELGAUGE_MAX17058)
static struct gpiomux_setting gpio_sc_en_susp_config = {	// GPIO_4
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_sc_sysok_susp_config = {	// GPIO_5
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_fuelalrt_n_susp_cfg = {	// GPIO_27
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_i2c_sda_config = {
	.func = GPIOMUX_FUNC_4,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting gpio_i2c_scl_config = {
	.func = GPIOMUX_FUNC_5,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting gpio_sc_susp_susp_cfg = {	// GPIO_31
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
static struct gpiomux_setting gpio_sc_stat_susp_cfg = {	// GPIO_86
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	//.pull = GPIOMUX_PULL_NONE,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};
static struct gpiomux_setting gpio_sc_i2c_sda_susp_cfg = {	// GPIO_83
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_OUT_HIGH,
};
static struct gpiomux_setting gpio_sc_i2c_scl_susp_cfg = {	// GPIO_84
	.func = GPIOMUX_FUNC_3,
	/*
	 * Please keep I2C GPIOs drive-strength at minimum (2ma). It is a
	 * workaround for HW issue of glitches caused by rapid GPIO current-
	 * change.
	 */
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_OUT_HIGH,
};
#endif
#if defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L)
static struct gpiomux_setting pan_tm_key_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting pan_tm_key_sus_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

#endif

static struct msm_gpiomux_config msm_blsp_configs[] __initdata = {
#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
#if !defined(CONFIG_PANTECH_CONSOLE_UART1) && !defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058) && !defined(CONFIG_SKY_DMB_SPI_HW) /* p13250 jaegyu.jun */
	{
		.gpio      = 0,		/* BLSP1 QUP SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_spi_susp_config,
		},
	},
	{
		.gpio      = 1,		/* BLSP1 QUP SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_spi_susp_config,
		},
	},
#endif
#if !defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058) && !defined(CONFIG_SKY_DMB_SPI_HW) /* sayuss */ /* p13250 jaegyu.jun */
	{
		.gpio      = 3,		/* BLSP1 QUP SPI_CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_spi_susp_config,
		},
	},
#endif
//++ p16088 - 2014.01.07 To prevent conflict of GPIO pin number with Hall IC for KK upgrade model(EF56S, EF59/EF60 series).
#if defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L)
	{
		.gpio      = 9,		/* BLSP1 QUP SPI_CS2A_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &pan_tm_key_active_config,
			[GPIOMUX_SUSPENDED] = &pan_tm_key_sus_config,
		},
	},
#endif
//-- p16088
#if !defined(CONFIG_F_SKYDISP_EF56_SS)
	{
		.gpio      = 8,		/* BLSP1 QUP SPI_CS1_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_cs1_config,
			[GPIOMUX_SUSPENDED] = &gpio_spi_susp_config,
		},
	},
#endif
#endif
#if defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)  /* sayuss */
#if defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L) 
	{
		.gpio      = 4,			/* SC_EN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_en_susp_config,
		},
	},
	{
		.gpio      = 5,			/* SC_SYSOK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_sysok_susp_config,
		},
	},
	{
		.gpio     = 27,			/* FUEL_ALRT_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
		},
	},
#if (CONFIG_BOARD_VER <= CONFIG_WS20)
	{
		.gpio     = 78,			/* SC_SUSP */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
		},
	},
	{
		.gpio     = 85,			/* SC_STAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
		},
	},
#else 
    {
		.gpio     = 31,			/* SC_SUSP */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
		},
	},
	{
		.gpio     = 86,			/* SC_STAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
		},
	},	
#endif /* CONFIG_BOARD_VER <= CONFIG_WS20 */
#elif defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio      = 4,			/* SC_EN */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_en_susp_config,
		},
	},
	{
		.gpio      = 5,			/* SC_SYSOK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_sysok_susp_config,
		},
	},
	{
		.gpio     = 27,			/* FUEL_ALRT_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
		},
	},
	{
		.gpio     = 31,			/* SC_SUSP */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
			[GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
		},
	},
	{
		.gpio     = 86,			/* SC_STAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
		},
	},
#endif
#if (defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L)) && (CONFIG_BOARD_VER <= CONFIG_WS20)
	{
		.gpio      = 2,         /* FUEL_I2C_SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 3,         /* FUEL_I2C_SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#elif ((defined(CONFIG_MACH_MSM8974_EF56S) || defined(CONFIG_MACH_MSM8974_EF57K) || defined(CONFIG_MACH_MSM8974_EF58L)) && (CONFIG_BOARD_VER > CONFIG_WS20)) || defined(CONFIG_MACH_MSM8974_EF59S) || defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) /* 2013.05.23 sayuss change i2c port */ 
    {
        .gpio = 25,             /* FUEL_I2C_SDA */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gpio_i2c_sda_config,
        },
    },
    {
        .gpio = 26,             /* FUEL_I2C_SCL */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gpio_i2c_scl_config,
        },
    },
#endif
#endif /* CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058 */

#if defined(CONFIG_PANTECH_EF63_PMIC_FUELGAUGE_MAX17058)
    {
        .gpio      = 4,			/* SC_EN */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gpio_sc_en_susp_config,
        },
    },
    {
        .gpio      = 5,			/* SC_SYSOK */
        .settings = {
        	[GPIOMUX_SUSPENDED] = &gpio_sc_sysok_susp_config,
        },
    },
    {
        .gpio = 25, /* FUEL_SDA */
        .settings = {
            [GPIOMUX_ACTIVE] = &gpio_i2c_sda_config,
            [GPIOMUX_SUSPENDED] = &gpio_i2c_sda_config, //
        },
    },
    {
        .gpio = 26, /* FUEL_SCL */
        .settings = {
            [GPIOMUX_ACTIVE] = &gpio_i2c_scl_config,
            [GPIOMUX_SUSPENDED] = &gpio_i2c_scl_config, //
        },
    },
    {
        .gpio     = 27,			/* FUEL_ALRT_N */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
        },
    },
    {
        .gpio     = 31,			/* SC_SUSP */
        .settings = {
            [GPIOMUX_ACTIVE] = &gpio_sc_susp_susp_cfg,
            [GPIOMUX_SUSPENDED] = &gpio_sc_susp_susp_cfg,
        },
    },
    {
        .gpio     = 86,			/* SC_STAT */
        .settings = {
            [GPIOMUX_SUSPENDED] = &gpio_sc_stat_susp_cfg,
        },
    },
#endif

#ifdef CONFIG_SKY_DMB_SPI_HW
	{
      .gpio      = 0,   /* BLSP1 QUP0 SPI_DATA_MOSI */
      .settings = {
        [GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
      },
    },
    {
      .gpio      = 1,   /* BLSP1 QUP0 SPI_DATA_MISO */
      .settings = {
        [GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
      },
    },
    {
      .gpio      = 3,   /* BLSP1 QUP0 SPI_CLK */
      .settings = {
        [GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
      },
    },
    {
      .gpio      = 2,   /* BLSP1 QUP0 SPI_CS0_N */
      .settings = {
        [GPIOMUX_SUSPENDED] = &gpio_blsp1_spi1_config,
      },
    },
#endif
	{
		.gpio      = 6,		/* BLSP1 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
			[GPIOMUX_ACTIVE] = &gpio_i2c_act_config,
		},
	},
	{
		.gpio      = 7,		/* BLSP1 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
			[GPIOMUX_ACTIVE] = &gpio_i2c_act_config,
		},
	},
#if defined(CONFIG_INPUT_FPC1080) || defined(CONFIG_INPUT_FPC1080_MODULE)//p11774 add for fpc1080's spi
	{
		.gpio      = 45,		/* BLSP2 QUP2 SPI MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
	{
		.gpio      = 46,		/* BLSP2 QUP2 SPI MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
	{
		.gpio      = 47,		/* BLSP2 QUP2 SPI CS */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
	{
		.gpio      = 48,		/* BLSP2 QUP2 SPI CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_blsp8_spi1_config,
			[GPIOMUX_SUSPENDED] = &gpio_blsp8_spi_suspend_config,
		},
	},
#endif

#if defined(CONFIG_PANTECH_EF63_PMIC_FUELGAUGE_MAX17058) ||defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)
	{
		.gpio      = 83,		/* SC_I2C_SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_i2c_sda_susp_cfg,
		},
	},
	{
		.gpio      = 84,		/* SC_I2C_SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_sc_i2c_scl_susp_cfg,
		},
	},
#else
	{
		.gpio      = 83,		/* BLSP11 QUP I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio      = 84,		/* BLSP11 QUP I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#endif

#if defined(CONFIG_PANTECH_CONSOLE_UART1) && !defined(CONFIG_SKY_DMB_SPI_HW)
	{
                .gpio      = 0,                 /* BLSP2 UART TX */
                .settings = {
                        [GPIOMUX_SUSPENDED] = &gpio_uart_config,
                },
        },
        {
                .gpio      = 1,                 /* BLSP2 UART RX */
                .settings = {
                        [GPIOMUX_SUSPENDED] = &gpio_uart_config,
                },
        },
#elif defined(CONFIG_PANTECH_CONSOLE_UART10)
	{
		.gpio      = 53,		/* BLSP2 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 54,		/* BLSP2 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
#else
	{
		.gpio      = 4,			/* BLSP2 UART TX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{
		.gpio      = 5,			/* BLSP2 UART RX */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_uart_config,
		},
	},
	{                           /* NFC */
		.gpio      = 29,		/* BLSP1 QUP6 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{                           /* NFC */
		.gpio      = 30,		/* BLSP1 QUP6 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#endif
/* EF63 TM TOUCH KEY. P13106*/
#if defined(CONFIG_KEYBOARD_TC370)
	{
		.gpio      = 56,		/* BLSP1 QUP2 I2C_DAT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
			[GPIOMUX_ACTIVE] = &gpio_i2c_act_config,
		},
	},
	{
		.gpio      = 55,		/* BLSP1 QUP2 I2C_CLK */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
			[GPIOMUX_ACTIVE] = &gpio_i2c_act_config,
		},
	},
#endif

#if !defined(CONFIG_PANTECH_CONSOLE_UART10) && !defined(CONFIG_KEYBOARD_TC370)
	{
		.gpio      = 53,		/* BLSP2 QUP4 SPI_DATA_MOSI */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio      = 54,		/* BLSP2 QUP4 SPI_DATA_MISO */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio      = 56,		/* BLSP2 QUP4 SPI_CLK */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio      = 55,		/* BLSP2 QUP4 SPI_CS0_N */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_spi_config,
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
#endif	

#if defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE] = &piezo_en_active_cfg,
			//[GPIOMUX_SUSPENDED] = &gpio_fuelalrt_n_susp_cfg,
			[GPIOMUX_SUSPENDED] = &piezo_en_active_cfg,
		},
	},
#endif

};

#ifdef CONFIG_PANTECH_USB_REDRIVER_EN_CONTROL
static struct msm_gpiomux_config msm8974_usb3_redriveric_configs[] __initdata = {
	{
		.gpio      = 81,		/* USB3.0 redriver IC  enable */
		.settings = {
			[GPIOMUX_ACTIVE] = &usb3_redriver_enable_active_cfg,
			[GPIOMUX_SUSPENDED] = &usb3_redriver_enable_suspend_cfg,
		},
	},
};
#endif

static struct msm_gpiomux_config msm8974_slimbus_config[] __initdata = {
	{
		.gpio	= 70,		/* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio	= 71,		/* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};

static struct gpiomux_setting cam_settings[] = {
	{
		.func = GPIOMUX_FUNC_1, /*active 1*/ /* 0 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_1, /*suspend*/ /* 1 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*i2c suspend*/ /* 2 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_KEEPER,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 0*/ /* 3 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend 0*/ /* 4 */
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
	},
};

static struct gpiomux_setting sd_card_det_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir = GPIOMUX_IN,
};

static struct gpiomux_setting sd_card_det_sleep_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
	.dir = GPIOMUX_IN,
};

static struct msm_gpiomux_config sd_card_det __initdata = {
	.gpio = 62,
	.settings = {
		[GPIOMUX_ACTIVE]    = &sd_card_det_active_config,
		[GPIOMUX_SUSPENDED] = &sd_card_det_sleep_config,
	},
};

static struct msm_gpiomux_config msm_sensor_configs[] __initdata = {
#ifdef CONFIG_PANTECH_CAMERA
#if defined(CONFIG_PANTECH_CAMERA_EF56_SS) || defined(CONFIG_PANTECH_CAMERA_EF59_SS) || defined(CONFIG_PANTECH_CAMERA_EF60_SS) || (defined(CONFIG_PANTECH_CAMERA_EF63_SS) && (CONFIG_BOARD_VER < CONFIG_WS10))
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* FRONT RESET */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},	
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 90, /* REAR RESET */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if !defined(CONFIG_TOUCHSCREEN_FTS)
	{
		.gpio = 96, /* REAR WP */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
#endif

#elif defined(CONFIG_PANTECH_CAMERA_EF63_SS)
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* FRONT RESET */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},	
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 57, /* CAM0_SVDD0_EN */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 58, /* CAM0_AVDD_EN */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 81, /* CAM0_SVDD1_EN */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 90, /* REAR RESET */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
				
#elif defined(CONFIG_PANTECH_CAMERA_EF65_SS)
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* FRONT RESET */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},	
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 95, /* CAM0_SVDD0_EN */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 96, /* CAM0_AVDD_EN */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 81, /* CAM0_SVDD1_EN */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	
	{
		.gpio = 90, /* REAR RESET */
		.settings = {
			[GPIOMUX_ACTIVE]	= &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},	

#endif
#else
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 16, /* CAM_MCLK1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* WEBCAM1_RESET_N / CAM_MCLK3 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[4],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 23, /* FLASH_LED_EN */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if !defined(CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058)	
	{
		.gpio = 25, /* WEBCAM2_RESET_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 26, /* CAM_IRQ */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
#endif /* CONFIG_PANTECH_PMIC_FUELGAUGE_MAX17058) */
#if !defined(CONFIG_PANTECH_EF63_PMIC_FUELGAUGE_MAX17058)	
	{
		.gpio = 27, /* OIS_SYNC */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif	
	{
		.gpio = 28, /* WEBCAM1_STANDBY */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 89, /* CAM1_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 90, /* CAM1_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if !defined(CONFIG_SKY_DMB_SPI_HW)
	{
		.gpio = 91, /* CAM2_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 92, /* CAM2_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
#endif
};

static struct msm_gpiomux_config msm_sensor_configs_dragonboard[] __initdata = {
	{
		.gpio = 15, /* CAM_MCLK0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 16, /* CAM_MCLK1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 17, /* CAM_MCLK2 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
	{
		.gpio = 18, /* WEBCAM1_RESET_N / CAM_MCLK3 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[4],
		},
	},
	{
		.gpio = 19, /* CCI_I2C_SDA0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 20, /* CCI_I2C_SCL0 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 21, /* CCI_I2C_SDA1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 22, /* CCI_I2C_SCL1 */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[0],
		},
	},
	{
		.gpio = 23, /* FLASH_LED_EN */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 24, /* FLASH_LED_NOW */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 25, /* WEBCAM2_RESET_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 26, /* CAM_IRQ */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &cam_settings[1],
		},
	},
#if !defined(CONFIG_PANTECH_EF63_PMIC_FUELGAUGE_MAX17058)	
	{
		.gpio = 27, /* OIS_SYNC */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[0],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif	
	{
		.gpio = 28, /* WEBCAM1_STANDBY */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 89, /* CAM1_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
	{
		.gpio = 90, /* CAM1_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#if !defined(CONFIG_SKY_DMB_SPI_HW)
	{
		.gpio = 91, /* CAM2_STANDBY_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
#endif
	{
		.gpio = 94, /* CAM2_RST_N */
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &gpio_suspend_config[1],
		},
	},
};

static struct gpiomux_setting auxpcm_act_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct gpiomux_setting auxpcm_sus_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

/* Primary AUXPCM port sharing GPIO lines with Primary MI2S */
static struct msm_gpiomux_config msm8974_pri_pri_auxpcm_configs[] __initdata = {
#if !(defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L) || defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L))  //p11774 blocked for HallIC 
	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
#endif
	{
		.gpio = 66,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 67,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
#if !defined(CONFIG_MACH_MSM8974_EF60S) && !defined(CONFIG_MACH_MSM8974_EF65S) && !defined(CONFIG_MACH_MSM8974_EF61K) && !defined(CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
#endif
};

/* Primary AUXPCM port sharing GPIO lines with Tertiary MI2S */
static struct msm_gpiomux_config msm8974_pri_ter_auxpcm_configs[] __initdata = {
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 76,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 77,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
};

static struct msm_gpiomux_config msm8974_sec_auxpcm_configs[] __initdata = {
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 80,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_SUSPENDED] = &auxpcm_sus_cfg,
			[GPIOMUX_ACTIVE] = &auxpcm_act_cfg,
		},
	},
};

static struct msm_gpiomux_config wcnss_5wire_interface[] = {
	{
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
};

static struct msm_gpiomux_config wcnss_5gpio_interface[] = {
	{
		.gpio = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5gpio_suspend_cfg,
		},
	},
	{
		.gpio = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5gpio_suspend_cfg,
		},
	},
	{
		.gpio = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5gpio_suspend_cfg,
		},
	},
	{
		.gpio = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5gpio_suspend_cfg,
		},
	},
	{
		.gpio = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5gpio_suspend_cfg,
		},
	},
};
/* EF63 TM TOUCH KEY. P13106*/
#if defined(CONFIG_KEYBOARD_TC370)
static struct gpiomux_setting coreriver_touch_key_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,	
};

static struct msm_gpiomux_config cr_touch_key_configs[] __initdata = {
	{
		.gpio      = 28,		/* TOUCH RESET */
		.settings = {
			[GPIOMUX_ACTIVE] = &coreriver_touch_key_act_cfg,
			[GPIOMUX_SUSPENDED] = &coreriver_touch_key_act_cfg,
		},
	},
	{
		.gpio      = 16,		/* TOUCH IRQ */
		.settings = {
			[GPIOMUX_ACTIVE] = &coreriver_touch_key_act_cfg,
			[GPIOMUX_SUSPENDED] = &coreriver_touch_key_act_cfg,
		},
	},
	{
		.gpio      = 9,		/* TOUCH IRQ */
		.settings = {
			[GPIOMUX_ACTIVE] = &coreriver_touch_key_act_cfg,
			[GPIOMUX_SUSPENDED] = &coreriver_touch_key_act_cfg,
		},
	},

};


#endif

// For EF59 & EF60 LED Driver. LS4 P13106
#if defined(CONFIG_LEDS_LP5523) 

static struct gpiomux_setting blsp9_i2c_config = {
	.func = GPIOMUX_FUNC_4,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting lp5523_led_en_act_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,	
};

static struct gpiomux_setting lp5523_led_en_sus_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config lp5523_led_configs[] __initdata = {
	{
		.gpio = 16,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lp5523_led_en_act_cfg,
			[GPIOMUX_SUSPENDED] = &lp5523_led_en_act_cfg,
		},
	},
//++ p11309 - 2013.05.21 not used port.
	{
		.gpio = 28,
		.settings = {
			[GPIOMUX_ACTIVE]    = &lp5523_led_en_sus_cfg,
			[GPIOMUX_SUSPENDED] = &lp5523_led_en_sus_cfg,
		},
	},
//-- p11309
	{
		.gpio      = 51,	/* BLSP2 QUP3 I2C_DAT */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp9_i2c_config,
		},
	},
	{
		.gpio      = 52,	/* BLSP2 QUP3 I2C_CLK */
		.settings = {			
			[GPIOMUX_SUSPENDED] = &blsp9_i2c_config,
		},
	},
};
#endif



static struct msm_gpiomux_config ath_gpio_configs[] = {
	{
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE]    = &ath_gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &ath_gpio_suspend_cfg,
		},
	},
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &ath_gpio_active_cfg,
			[GPIOMUX_SUSPENDED] = &ath_gpio_suspend_cfg,
		},
	},
};

static struct msm_gpiomux_config msm_taiko_config[] __initdata = {
	{
		.gpio	= 63,		/* SYS_RST_N */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_reset,
		},
	},
	{
		.gpio	= 72,		/* CDC_INT */
		.settings = {
			[GPIOMUX_SUSPENDED] = &taiko_int,
		},
	},
};

static struct gpiomux_setting sdc3_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc3_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc3_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc3_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8974_sdc3_configs[] __initdata = {
	{
		/* DAT3 */
		.gpio      = 35,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT2 */
		.gpio      = 36,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* DAT1 */
		.gpio      = 37,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_data_1_suspend_cfg,
		},
	},
	{
		/* DAT0 */
		.gpio      = 38,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* CMD */
		.gpio      = 39,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
	{
		/* CLK */
		.gpio      = 40,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc3_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc3_suspend_cfg,
		},
	},
};

static void msm_gpiomux_sdc3_install(void)
{
	msm_gpiomux_install(msm8974_sdc3_configs,
			    ARRAY_SIZE(msm8974_sdc3_configs));
}

#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
static struct gpiomux_setting sdc4_clk_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting sdc4_cmd_data_0_3_actv_cfg = {
	.func = GPIOMUX_FUNC_2,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting sdc4_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting sdc4_data_1_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config msm8974_sdc4_configs[] __initdata = {
#if !defined(CONFIG_SKY_DMB_SPI_HW)
	{
		/* DAT3 */
		.gpio      = 92,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
#endif
	{
		/* DAT2 */
		.gpio      = 94,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
	{
		/* DAT1 */
		.gpio      = 95,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_data_1_suspend_cfg,
		},
	},
#if !defined(CONFIG_TOUCHSCREEN_FTS)
	{
		/* DAT0 */
		.gpio      = 96,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
#endif

#if !defined(CONFIG_SKY_DMB_SPI_HW)
	{
		/* CMD */
		.gpio      = 91,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_cmd_data_0_3_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
#endif
	{
		/* CLK */
		.gpio      = 93,
		.settings = {
			[GPIOMUX_ACTIVE]    = &sdc4_clk_actv_cfg,
			[GPIOMUX_SUSPENDED] = &sdc4_suspend_cfg,
		},
	},
};

static void msm_gpiomux_sdc4_install(void)
{
	msm_gpiomux_install(msm8974_sdc4_configs,
			    ARRAY_SIZE(msm8974_sdc4_configs));
}
#else
static void msm_gpiomux_sdc4_install(void) {}
#endif /* CONFIG_MMC_MSM_SDC4_SUPPORT */

static struct msm_gpiomux_config apq8074_dragonboard_ts_config[] __initdata = {
#if !defined(CONFIG_SKY_DMB_SPI_HW)
	{
		/* BLSP1 QUP I2C_DATA */
		.gpio      = 2,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		/* BLSP1 QUP I2C_CLK */
		.gpio      = 3,
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
#endif
};

#if defined(CONFIG_PN544) ||  defined(CONFIG_PN547)
static struct gpiomux_setting gpio_nfc_en_active_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct gpiomux_setting gpio_nfc_en_suspend_config = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_KEEPER,
};
static struct msm_gpiomux_config nfc_gpio_configs[] __initdata = {
	{
		.gpio	= 29,  /*BLSP1_QUP5(BLSP6) I2C DATA*/
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio	= 30,  /*BLSP1_QUP5(BLSP6) I2C CLK*/
		.settings = {
			[GPIOMUX_SUSPENDED] = &gpio_i2c_config,
		},
	},
	{
		.gpio	= 50,  /* NFC Enable pin */
		.settings = {
			[GPIOMUX_ACTIVE] = &gpio_nfc_en_active_config,
			[GPIOMUX_SUSPENDED] = &gpio_nfc_en_suspend_config,
		},
	}
};
#endif /* CONFIG_PN544 || CONFIG_PN547*/

#ifdef CONFIG_PANTECH_GPIO_SLEEP_CONFIG
static struct gpiomux_setting msm8974_gpio_suspend_in_pd_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir = GPIOMUX_IN,
};

#if defined(CONFIG_MACH_MSM8974_EF56S)
static struct msm_gpiomux_config msm8974_sleep_gpio_gpio_configs[] __initdata = {
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	{
		.gpio = 16,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 28,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
#if (CONFIG_BOARD_VER <= CONFIG_WS20)
	{
		.gpio = 31,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 51,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 52,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
#if (CONFIG_BOARD_VER <= CONFIG_WS20)
	{
		.gpio = 58,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},

	},
#endif
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if !defined (CONFIG_MACH_MSM8974_EF60S)&& !defined (CONFIG_MACH_MSM8974_EF65S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
#if (CONFIG_BOARD_VER <= CONFIG_WS20)
	{
		.gpio = 69,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	{
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 79,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 81,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (CONFIG_BOARD_VER >= CONFIG_TP10)
	{
		.gpio = 100,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 103,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 105,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 106,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if (CONFIG_BOARD_VER <= CONFIG_WS20)
	{
		.gpio = 108,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 109,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 113,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 114,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 115,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 117,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 118,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 119,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 123,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 126,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 127,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 129,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 131,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 132,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 137,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 144,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 145,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

}; /* CONFIG_MACH_MSM8974_EF56S */

#elif defined(CONFIG_MACH_MSM8974_EF59S) ||defined(CONFIG_MACH_MSM8974_EF59K) || defined(CONFIG_MACH_MSM8974_EF59L) || defined(CONFIG_MACH_MSM8974_EF60S) || defined(CONFIG_MACH_MSM8974_EF65S) || defined(CONFIG_MACH_MSM8974_EF61K) || defined(CONFIG_MACH_MSM8974_EF62L)
static struct msm_gpiomux_config msm8974_sleep_gpio_gpio_configs[] __initdata = {
#if ((defined (CONFIG_MACH_MSM8974_EF65S) && (CONFIG_BOARD_VER == CONFIG_WS10)) || defined (CONFIG_MACH_MSM8974_EF60S) && (CONFIG_BOARD_VER == CONFIG_PT10)) || (defined (CONFIG_MACH_MSM8974_EF61K) && (CONFIG_BOARD_VER == CONFIG_PT10)) || (defined (CONFIG_MACH_MSM8974_EF62L) && (CONFIG_BOARD_VER == CONFIG_PT10))
	{
		.gpio = 8,	
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif 
	{
		.gpio = 33,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 41,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 42,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 55,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 56,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
	{
		.gpio = 64,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 65,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#if !defined (CONFIG_MACH_MSM8974_EF60S) && !defined (CONFIG_MACH_MSM8974_EF65S) && !defined (CONFIG_MACH_MSM8974_EF61K) && !defined (CONFIG_MACH_MSM8974_EF62L)
	{
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
#endif
	{
		.gpio = 74,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 75,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 78,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 82,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 100,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 102,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 103,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 105,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 106,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 108,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 109,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 113,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 114,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 115,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 117,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 118,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 119,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 123,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 126,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 127,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 129,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 130,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 131,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 132,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 137,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 144,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},

	{
		.gpio = 145,
		.settings = {
			[GPIOMUX_ACTIVE]    = &msm8974_gpio_suspend_in_pd_cfg,
			[GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
		},
	},
}; /* CONFIG_MACH_MSM8974_EF59S || CONFIG_MACH_MSM8974_EF59K || CONFIG_MACH_MSM8974_EF59L || CCONFIG_MACH_MSM8974_EF65S || ONFIG_MACH_MSM8974_EF60S || CONFIG_MACH_MSM8974_EF61K || CONFIG_MACH_MSM8974_EF62L*/

#elif defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L)	
static struct msm_gpiomux_config msm8974_sleep_gpio_gpio_configs[] __initdata = {
        {
                .gpio = 8,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },
	
        {
                .gpio = 31,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 33,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 34,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 41,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },
	
        {
                .gpio = 42,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 49,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },
	
        {
                .gpio = 74,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 78,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 82,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },
	
        {
                .gpio = 89,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 95,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 100,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 102,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 104,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 105,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 106,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },

        },

        {
                .gpio = 107,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 108,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 109,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 110,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 111,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 112,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 113,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 114,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 115,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 123,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },
	
        {
                .gpio = 131,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },

        },

        {
                .gpio = 132,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 135,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 137,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },

        },

        {
                .gpio = 144,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },

        {
                .gpio = 145,
                .settings = {
			[GPIOMUX_ACTIVE]	= &msm8974_gpio_suspend_in_pd_cfg,
                        [GPIOMUX_SUSPENDED] = &msm8974_gpio_suspend_in_pd_cfg,
                },
        },
};
#endif /* (CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L) */
#endif /* CONFIG_PANTECH_GPIO_SLEEP_CONFIG */


void __init msm_8974_init_gpiomux(void)
{
	int rc;

	rc = msm_gpiomux_init_dt();
	if (rc) {
		pr_err("%s failed %d\n", __func__, rc);
		return;
	}

	pr_err("%s:%d socinfo_get_version %x\n", __func__, __LINE__,
		socinfo_get_version());
	if (socinfo_get_version() >= 0x20000)
		msm_tlmm_misc_reg_write(TLMM_SPARE_REG, 0xf);

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
	if (!(of_board_is_dragonboard() && machine_is_apq8074()))
		msm_gpiomux_install(msm_eth_configs, \
			ARRAY_SIZE(msm_eth_configs));
#endif

	msm_gpiomux_install(msm_blsp_configs, ARRAY_SIZE(msm_blsp_configs));
	msm_gpiomux_install(msm_blsp2_uart7_configs,
			 ARRAY_SIZE(msm_blsp2_uart7_configs));
	msm_gpiomux_install(wcnss_5wire_interface,
				ARRAY_SIZE(wcnss_5wire_interface));
	if (of_board_is_liquid())
		msm_gpiomux_install_nowrite(ath_gpio_configs,
					ARRAY_SIZE(ath_gpio_configs));
	msm_gpiomux_install(msm8974_slimbus_config,
			ARRAY_SIZE(msm8974_slimbus_config));

	msm_gpiomux_install(msm_touch_configs, ARRAY_SIZE(msm_touch_configs));
		msm_gpiomux_install(hap_lvl_shft_config,
				ARRAY_SIZE(hap_lvl_shft_config));

	if (of_board_is_dragonboard() && machine_is_apq8074())
		msm_gpiomux_install(msm_sensor_configs_dragonboard, \
				ARRAY_SIZE(msm_sensor_configs_dragonboard));
	else
		msm_gpiomux_install(msm_sensor_configs, \
				ARRAY_SIZE(msm_sensor_configs));

	msm_gpiomux_install(&sd_card_det, 1);

	if (machine_is_apq8074() && (of_board_is_liquid() || \
	    of_board_is_dragonboard()))
		msm_gpiomux_sdc3_install();

	if (!(of_board_is_dragonboard() && machine_is_apq8074()))
		msm_gpiomux_sdc4_install();

	msm_gpiomux_install(msm_taiko_config, ARRAY_SIZE(msm_taiko_config));

	msm_gpiomux_install(msm_hsic_configs, ARRAY_SIZE(msm_hsic_configs));

#if !defined(CONFIG_PN544) && !defined(CONFIG_PN547)
	msm_gpiomux_install(msm_hsic_hub_configs,
				ARRAY_SIZE(msm_hsic_hub_configs));
#endif /*!CONFIG_PN544 && !CONFIG_PN547*/
#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && \
	!defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS) 
	msm_gpiomux_install(msm_hdmi_configs, ARRAY_SIZE(msm_hdmi_configs));
#endif

//++ p13106 ls4 For Touch Pen
#if defined (CONFIG_MACH_MSM8974_EF59S) || defined (CONFIG_MACH_MSM8974_EF59K) || defined (CONFIG_MACH_MSM8974_EF59L)
    msm_gpiomux_install(touch_pen_configs, ARRAY_SIZE(touch_pen_configs));
#endif

#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && \
	!defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS)
	if (of_board_is_fluid())
		msm_gpiomux_install(msm_mhl_configs,
				    ARRAY_SIZE(msm_mhl_configs));
#endif

	if (of_board_is_liquid() ||
	    (of_board_is_dragonboard() && machine_is_apq8074()))
		msm_gpiomux_install(msm8974_pri_ter_auxpcm_configs,
				 ARRAY_SIZE(msm8974_pri_ter_auxpcm_configs));
	else
		msm_gpiomux_install(msm8974_pri_pri_auxpcm_configs,
				 ARRAY_SIZE(msm8974_pri_pri_auxpcm_configs));

	if (of_board_is_cdp())
		msm_gpiomux_install(msm8974_sec_auxpcm_configs,
				 ARRAY_SIZE(msm8974_sec_auxpcm_configs));
	else if (of_board_is_liquid() || of_board_is_fluid() ||
						of_board_is_mtp())
		msm_gpiomux_install(msm_epm_configs,
				ARRAY_SIZE(msm_epm_configs));
	
#if !defined(CONFIG_F_SKYDISP_EF56_SS) && !defined(CONFIG_F_SKYDISP_EF59_SS) && \
	!defined(CONFIG_F_SKYDISP_EF60_SS) && !defined(CONFIG_F_SKYDISP_EF63_SS) 
	msm_gpiomux_install_nowrite(msm_lcd_configs,
			ARRAY_SIZE(msm_lcd_configs));

#elif defined(CONFIG_F_SKYDISP_EF56_SS) || defined(CONFIG_F_SKYDISP_EF59_SS) \
	||defined(CONFIG_F_SKYDISP_EF60_SS)
//display CONFIG
	msm_gpiomux_install(lcd_common_configs,
			ARRAY_SIZE(lcd_common_configs));
	msm_gpiomux_install(lcd_ext_power_configs,
			ARRAY_SIZE(lcd_ext_power_configs));
	msm_gpiomux_install(lcd_bl_configs,
			ARRAY_SIZE(lcd_bl_configs));

#elif defined(CONFIG_F_SKYDISP_EF63_SS) 

#endif

#if !defined(CONFIG_INPUT_FPC1080) || !defined(CONFIG_INPUT_FPC1080_MODULE)//p11774 add for fpc1080's spi
	if (of_board_is_rumi())
		msm_gpiomux_install(msm_rumi_blsp_configs,
				    ARRAY_SIZE(msm_rumi_blsp_configs));
#endif

	if (socinfo_get_platform_subtype() == PLATFORM_SUBTYPE_MDM)
		msm_gpiomux_install(mdm_configs,
			ARRAY_SIZE(mdm_configs));

	if (of_board_is_dragonboard() && machine_is_apq8074())
		msm_gpiomux_install(apq8074_dragonboard_ts_config,
			ARRAY_SIZE(apq8074_dragonboard_ts_config));
#ifdef CONFIG_PANTECH_USB_REDRIVER_EN_CONTROL
	msm_gpiomux_install(msm8974_usb3_redriveric_configs, 
			ARRAY_SIZE(msm8974_usb3_redriveric_configs));
#endif

#if defined(CONFIG_LEDS_LP5523) // For EF59 & EF60 LED Driver. LS4 P13106
    msm_gpiomux_install(lp5523_led_configs, ARRAY_SIZE(lp5523_led_configs));
#endif
#if defined(CONFIG_KEYBOARD_TC370)
/* EF63 TM TOUCH KEY. P13106*/
  msm_gpiomux_install(cr_touch_key_configs, ARRAY_SIZE(cr_touch_key_configs));
#endif

#if defined(CONFIG_PN544) || defined(CONFIG_PN547)
	msm_gpiomux_install(nfc_gpio_configs, ARRAY_SIZE(nfc_gpio_configs));
#endif /*CONFIG_PN544 || CONFIG_PN547*/






/* For GPIO SLEEP CONFFIG, 
If you want to add another msm_gpiomux_install, plz add to it above this function, p14978 */
#ifdef CONFIG_PANTECH_GPIO_SLEEP_CONFIG
        msm_gpiomux_install(msm8974_sleep_gpio_gpio_configs,
                        ARRAY_SIZE(msm8974_sleep_gpio_gpio_configs));
#endif /* CONFIG_PANTECH_GPIO_SLEEP_CONFIG */
/* For GPIO SLEEP CONFFIG, 
If you want to add another msm_gpiomux_install, plz add to it above this function */

}

static void wcnss_switch_to_gpio(void)
{
	/* Switch MUX to GPIO */
	msm_gpiomux_install(wcnss_5gpio_interface,
			ARRAY_SIZE(wcnss_5gpio_interface));

	/* Ensure GPIO config */
	gpio_direction_input(WLAN_DATA2);
	gpio_direction_input(WLAN_DATA1);
	gpio_direction_input(WLAN_DATA0);
	gpio_direction_output(WLAN_SET, 0);
	gpio_direction_output(WLAN_CLK, 0);
}

static void wcnss_switch_to_5wire(void)
{
	msm_gpiomux_install(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));
}

u32 wcnss_rf_read_reg(u32 rf_reg_addr)
{
	int count = 0;
	u32 rf_cmd_and_addr = 0;
	u32 rf_data_received = 0;
	u32 rf_bit = 0;

	wcnss_switch_to_gpio();

	/* Reset the signal if it is already being used. */
	gpio_set_value(WLAN_SET, 0);
	gpio_set_value(WLAN_CLK, 0);

	/* We start with cmd_set high WLAN_SET = 1. */
	gpio_set_value(WLAN_SET, 1);

	gpio_direction_output(WLAN_DATA0, 1);
	gpio_direction_output(WLAN_DATA1, 1);
	gpio_direction_output(WLAN_DATA2, 1);

	gpio_set_value(WLAN_DATA0, 0);
	gpio_set_value(WLAN_DATA1, 0);
	gpio_set_value(WLAN_DATA2, 0);

	/* Prepare command and RF register address that need to sent out.
	 * Make sure that we send only 14 bits from LSB.
	 */
	rf_cmd_and_addr  = (((WLAN_RF_READ_REG_CMD) |
		(rf_reg_addr << WLAN_RF_REG_ADDR_START_OFFSET)) &
		WLAN_RF_READ_CMD_MASK);

	for (count = 0; count < 5; count++) {
		gpio_set_value(WLAN_CLK, 0);

		rf_bit = (rf_cmd_and_addr & 0x1);
		gpio_set_value(WLAN_DATA0, rf_bit ? 1 : 0);
		rf_cmd_and_addr = (rf_cmd_and_addr >> 1);

		rf_bit = (rf_cmd_and_addr & 0x1);
		gpio_set_value(WLAN_DATA1, rf_bit ? 1 : 0);
		rf_cmd_and_addr = (rf_cmd_and_addr >> 1);

		rf_bit = (rf_cmd_and_addr & 0x1);
		gpio_set_value(WLAN_DATA2, rf_bit ? 1 : 0);
		rf_cmd_and_addr = (rf_cmd_and_addr >> 1);

		/* Send the data out WLAN_CLK = 1 */
		gpio_set_value(WLAN_CLK, 1);
	}

	/* Pull down the clock signal */
	gpio_set_value(WLAN_CLK, 0);

	/* Configure data pins to input IO pins */
	gpio_direction_input(WLAN_DATA0);
	gpio_direction_input(WLAN_DATA1);
	gpio_direction_input(WLAN_DATA2);

	for (count = 0; count < 2; count++) {
		gpio_set_value(WLAN_CLK, 1);
		gpio_set_value(WLAN_CLK, 0);
	}

	rf_bit = 0;
	for (count = 0; count < 6; count++) {
		gpio_set_value(WLAN_CLK, 1);
		gpio_set_value(WLAN_CLK, 0);

		rf_bit = gpio_get_value(WLAN_DATA0);
		rf_data_received |= (rf_bit << (count * 3 + 0));

		if (count != 5) {
			rf_bit = gpio_get_value(WLAN_DATA1);
			rf_data_received |= (rf_bit << (count * 3 + 1));

			rf_bit = gpio_get_value(WLAN_DATA2);
			rf_data_received |= (rf_bit << (count * 3 + 2));
		}
	}

	gpio_set_value(WLAN_SET, 0);
	wcnss_switch_to_5wire();

	return rf_data_received;
}
