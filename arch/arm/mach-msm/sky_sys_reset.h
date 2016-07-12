#ifndef __ARCH_ARM_MACH_MSM_SKY_SYS_RESET_H
#define __ARCH_ARM_MACH_MSM_SKY_SYS_RESET_H

#define SYS_RESTART_REASON_ADDR 0x2A05F65C

#define SYS_RESET_REASON_NORMAL       0x776655DD
#define SYS_RESET_REASON_EXCEPTION	  0x776655E0
#define SYS_RESET_REASON_ASSERT		  0x776655E1
#define SYS_RESET_REASON_LINUX        0x776655E2
#define SYS_RESET_REASON_ANDROID      0x776655E3
#define SYS_RESET_REASON_VENUS        0x776655E4
#define SYS_RESET_REASON_ADSP         0x776655E5
#define SYS_RESET_REASON_RIVA         0x776655E6
#define SYS_RESET_REASON_UNKNOWN      0x776655E7
#define SYS_RESET_REASON_WATCHDOG     0x776655E8
#define SYS_RESET_REASON_USERDATA_FS  0x776655EA //add for auto repair userdata filesystem
#define SYS_RESET_REASON_ABNORMAL     0x77236d34
#define SYS_RESET_BACKLIGHT_OFF_FLAG  0x08000000

#define SYS_RESET_PDL_DOWNLOAD_ENTRY		0xCC33CC33
#define SYS_RESET_PDL_IDLE_DOWNLOAD_ENTRY	0x33CC33CC

#define PANTECH_SHARED_RAM_BASE 0x07A00000 //CPL_r1031330B.1
#define PANTECH_SHARED_RAM_SIZE  0x100000

extern uint8_t sky_sys_rst_is_silent_boot_mode(void);
extern uint8_t sky_sys_rst_is_backlight_off(void);
extern void  sky_sys_rst_is_silent_boot_backlight(int backlight);
extern void sky_sys_rst_is_silent_boot_for_test(int silent_mode,int backlight);

extern int sky_reset_reason;
#endif

