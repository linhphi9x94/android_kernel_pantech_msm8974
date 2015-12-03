#ifndef LINUX_SPI_FPC1080_H
#define LINUX_SPI_FPC1080_H

struct fpc1080_nav_settings {

     // Added by Joshua on 2012-10-29 Mon PM  1:22
     u8 p_sensitivity_key;
     u8 p_sensitivity_ptr;
     u8 p_multiplier_x;
     u8 p_multiplier_y;

     u8 multiplier_key_accel;
     u8 multiplier_ptr_accel;
     int sum_x;
     int sum_y;
     u8 threshold_key_start;
     u8 threshold_key_accel;
     u8 threshold_key_dispatch;
     u8 threshold_ptr_start;
     u8 threshold_ptr_accel;
     u8 duration_key_clear;
     u8 duration_ptr_clear;

     // Added by Sungchan
     u8 mouse_enable;
     u8 mouse_scroll;
     u8 mouse_scroll_skip_frame;
     u8 btp_input_mode;
};

struct fpc1080_adc_setup {
	u8 gain;
	u8 offset;
	u8 pxl_setup;
};

struct fpc1080_platform_data {
	int power_gpio;//p14696 add 2013_03_21
	int irq_gpio;
	int reset_gpio;
	struct fpc1080_adc_setup adc_setup;
	struct fpc1080_nav_settings nav;
};
#if defined(CONFIG_MACH_MSM8974_EF63S) || defined(CONFIG_MACH_MSM8974_EF63K) || defined(CONFIG_MACH_MSM8974_EF63L) //p11774 for support module
void set_finger_cover_state(int state);
#endif
#endif
