#ifndef __CUST_PANTECH_GOTA_H
#define __CUST_PANTECH_GOTA_H
/* ========================================================================
FILE: cust_pantech_gota.h

Copyright (c) 2010 by PANTECH Incorporated.  All Rights Reserved.

=========================================================================== */

/*************************************************************************/
/*                           FEATURE DESCRIPTION                         */
/*************************************************************************/
/* ========================================================================
FEATURE_GOTA_UPGRADE
	GOTA main FEATURE
FEATURE_GOTA_1BLK_PASS_PATCH
	desc
		SKY Station 1 block pass writing patch
	source
		mtdutils/mtdutils.c
		updater/install.c
FEATURE_GOTA_MEMORY_LEAK_PATCH
	desc
		Memory Leak patch during applypatch in recovery mode.
	source
		applypatch/applypatch.c
FEATURE_GOTA_CONSOLE_LOG
	desc
		Print log to seial in recovery mode
	source
		recovery_log.c
		recovery_log.h
	usage
		#include "recovery_log.h"
		Console_log_init();
		CONSOLE_LOG(const char *fmt, ...);
		Console_log_close();
FEATURE_GOTA_SBL_POWER_LOSS
	desc
		Load each image from FOTA partition if the image is damaged due to power loss when the SBL or LK updating in the recovery mode.
	source
		modem_proc/core/.../sky_rawdata.h
		pantech/frameworks/.../sky_rawdata.h
		boot_images/core/boot/secboot3/common/boot_loader.h
		boot_images/core/boot/secboot3/common/boot_loader.c
		boot_images/core/boot/secboot3/common/boot_gpt_partition_id.c
		boot_images/core/boot/secboot3/common/boot_config.c
		applypatch/applypatch.c
FEATURE_GOTA_PANTECH_DISPLAY
	desc
		Show string and percent when updating in the recovery mode.
	source
		recovery_ui.h
		ui.c
		recovery.c
FEATURE_GOTA_UPDATE_INFO
	desc
		Store update info for AT&T
	source
		modem_proc/core/.../sky_rawdata.h
		pantech/frameworks/.../sky_rawdata.h
		bootable/bootloader/lk/app/pdl/pdl.h
		bootable/bootloader/lk/app/pdl/dloadstatus.c
		pantech/frameworks/pam_proc/
		gota_jni/GotaJni.java
		gota_jni/gota_jni.cpp
		fota_rawdata/fota_rawdata.c
		gota_status.h
		recovery.c
FEATURE_GOTA_PERMANENT_MEMORY
	desc
		Functions to deal with permanent memory.
		It is used only by the SKT models
	source
		pantech/frameworks/pam_proc/pam_proc.c
		pantech/frameworks/pam_proc/pam_proc.h
FEATURE_GOTA_BATTERY_WARNING
	desc 
		Functions to display battery warning image during update in recovery mode.
		it is used only by dormitory devices.
	source
		screen_ui.h
		screen_ui.cpp

=========================================================================== */


/*************************************************************************/
/*                           PANTECH GOTA                                */
/*************************************************************************/

#if 1 /* Common */
#define FEATURE_GOTA_UPGRADE
#define FEATURE_GOTA_1BLK_PASS_PATCH
#define FEATURE_GOTA_MEMORY_LEAK_PATCH
#define FEATURE_GOTA_CONSOLE_LOG
#define FEATURE_GOTA_SDCARD_UPDATE
#define FEATURE_GOTA_SBL_POWER_LOSS
#define FEATURE_GOTA_PANTECH_DISPLAY
#define FEATURE_GOTA_BINARY_MD5_CHECK
#endif

#if defined(T_EF58L) || defined(T_EF59L) || defined(T_EF62L) || defined(T_EF63L)
// #define FEATURE_GOTA_UPDATE_INFO
// #define FEATURE_GOTA_PERMANENT_MEMORY
#define FEATURE_AIR_LOGGING
#define FEATURE_GOTA_BATTERY_WARNING
#endif

#if defined(T_EF57K) || defined(T_EF59K) || defined(T_EF61K) || defined(T_EF63K)
// #define FEATURE_GOTA_UPDATE_INFO
// #define FEATURE_GOTA_PERMANENT_MEMORY
#define FEATURE_AIR_LOGGING
#define FEATURE_GOTA_BATTERY_WARNING
#endif

#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF60S) || defined(T_EF65S) || defined(T_NAMI) || defined(T_EF63S)
#define FEATURE_GOTA_PERMANENT_MEMORY
// #define FEATURE_GOTA_UPDATE_INFO
#define FEATURE_AIR_LOGGING
#define FEATURE_GOTA_BATTERY_WARNING
#endif

#if defined(T_EF57K) || defined(T_EF58L) \
        || defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) \
        || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L) \
        || defined(T_EF63S) || defined(T_EF63K) || defined(T_EF63L) \
        || defined(T_NAMI)
#define FEATURE_GOTA_AUTO_TEST_MODE
#endif

/* VZW
#if defined(T_STARQ)
#define FEATURE_GOTA_UPGRADE
#define FEATURE_GOTA_1BLK_PASS_PATCH
#define FEATURE_GOTA_MEMORY_LEAK_PATCH
#define FEATURE_GOTA_CONSOLE_LOG
#define FEATURE_GOTA_SBL_POWER_LOSS
#define FEATURE_GOTA_PANTECH_DISPLAY
#define FEATURE_GOTA_SDCARD_UPDATE
#define FEATURE_GOTA_NO_BOOT_SOUND
// #define FEATURE_GOTA_UPDATE_INFO
// #define FEATURE_GOTA_PERMANENT_MEMORY
#endif
*/
/* AT&T
#if defined(T_OSCAR)
#define FEATURE_GOTA_UPGRADE
#define FEATURE_GOTA_1BLK_PASS_PATCH
#define FEATURE_GOTA_MEMORY_LEAK_PATCH
#define FEATURE_GOTA_CONSOLE_LOG
#define FEATURE_GOTA_SBL_POWER_LOSS
#define FEATURE_GOTA_PANTECH_DISPLAY
#define FEATURE_GOTA_UPDATE_INFO
#define FEATURE_GOTA_SDCARD_UPDATE
// #define FEATURE_GOTA_PERMANENT_MEMORY
#endif
*/
/* KDDI
#if defined(T_VEGAPKDDI)
#define FEATURE_GOTA_UPGRADE
#define FEATURE_GOTA_1BLK_PASS_PATCH
#define FEATURE_GOTA_MEMORY_LEAK_PATCH
#define FEATURE_GOTA_CONSOLE_LOG
#define FEATURE_GOTA_SBL_POWER_LOSS
#define FEATURE_GOTA_PANTECH_DISPLAY
// #define FEATURE_GOTA_SDCARD_UPDATE
// #define FEATURE_GOTA_UPDATE_INFO
// #define FEATURE_GOTA_PERMANENT_MEMORY
#endif
*/


#endif /* __CUST_PANTECH_GOTA_H */
