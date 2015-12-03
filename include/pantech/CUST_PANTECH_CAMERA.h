#ifndef __CUST_PANTECH_CAMERA_H__
#define __CUST_PANTECH_CAMERA_H__


/*------------------------------------------------------------------------------

(1) Hardware configurations
   
EF39S   : APQ8060, CE1612(8M ISP), S5K6AAFX13(1.3M)
EF40S/40K/65L   : APQ8060, CE1612(8M ISP), MT9D113(1.3M)
PRESTO  : APQ8060, MT9P111(5M SOC), MT9V113(VGA)
EF44S   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
MAGNUS   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF48S/49K/50L   : APQ8064, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF56S/57K/58L   : MSM8974, IMX135(13M bayer), S5K6B2YX(2M FHD bayer)


(2)  Delete all kernel/userspace/android logs related camera 

Modify kernel/arch/arm/config/msm8660-perf-(Model Name)_(Board Version)_defconfig

	# CONFIG_MSM_CAMERA_DEBUG is not set (default)

Do #undef F_PANTECH_CAMERA_LOG_PRINTK at CUST_PANTECH_CAMERA.h 

	#define F_PANTECH_CAMERA_LOG_PRINTK (default)

Make annotation from all source files looking for F_PANTECH_CAMERA_LOG_OEM.
	If it is declared, to activate the message using a SKYCDBG/SKYCERR macro that is used in that file.
	(user-space only)

Make annotation from all source files looking for F_PANTECH_CAMERA_LOG_CDBG.
	If it is declared, to activate the message using a CDBG macro that is used in that file.
	(user-space only)

Make annotation from all source files looking for F_PANTECH_CAMERA_LOG_VERBOSE.
	If it is declared, to activate the message using a LOGV/LOGD/LOGI macro that is used in that file.
	(user-space only)


(4) The build environment related to face recognition

	Modify vendor/qcom/android-open/libcamera2/Android.mk.
	Decide whether or not to use 3rd PARTY solution.

	PANTECH_CAMERA_FD_ENGINE := 0		Not include
	PANTECH_CAMERA_FD_ENGINE := 1		Use OllaWorks Solution
	PANTECH_CAMERA_FD_ENGINE := 2		Use other Solutions

	Do #undef F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER at CUST_PANTECH_CAMERA.h 
	Determine whether to include interface associated with the face recognition function.

Because link libOlaEngine.so existing libcamera.so, it is possible that the size of a conventional libcamera.so 
size is increased, the link error.
In this case, it is possible to secure the area libcamera.so by reducing the liboemcamera.so area to the following files.

build/core/prelink-linux-arm-2G.map (for 2G/2G)
build/core/prelink-linux-arm.map (for 1G/3G)

------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC   														  */
/*  The FEATURE list which will apply or verify only to the model             */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_SKY_EF39S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF39S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40K_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40K
#elif defined(CONFIG_PANTECH_PRESTO_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_PRESTO
#define F_PANTECH_CAMERA_PRESTO
/* Add Feature for AT&T Model */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_EF45K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF45K
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF46L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF46L
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF47S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF47S
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_SVLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SVLTE
#elif defined(T_CSFB)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CSFB
#elif defined(T_CHEETAH)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CHEETAH
#elif defined(T_STARQ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_ATT
#elif defined(T_OSCAR)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_OSCAR
#define F_PANTECH_CAMERA_ATT
#elif defined(T_VEGAPVW)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_VEGAPVW
/* Add Feature for AT&T Model */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_ZEPPLIN)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_ZEPPLIN
#elif defined(T_RACERJ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_RACERJ
#elif defined(T_SIRIUSLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#elif defined(T_EF44S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF44S
#define F_PANTECH_CAMERA_SKT
#elif defined(T_MAGNUS)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_MAGNUS
#elif defined(T_EF48S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF48S
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF49K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF49K
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF50L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF50L
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF51S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51S
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF51K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51K
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF51L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51L
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52S
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF52K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52K
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF52L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52L
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF56S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF56S
#define F_PANTECH_CAMERA_EF56S_57K_58L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF57K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF57K
#define F_PANTECH_CAMERA_EF56S_57K_58L
#elif defined(T_EF58L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF58L
#define F_PANTECH_CAMERA_EF56S_57K_58L
#elif defined(T_EF59S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59S
#define F_PANTECH_CAMERA_EF59S_59K_59L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF59K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59K
#define F_PANTECH_CAMERA_EF59S_59K_59L
#elif defined(T_EF59L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59L
#define F_PANTECH_CAMERA_EF59S_59K_59L
#elif defined(T_EF60S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF60S
#define F_PANTECH_CAMERA_EF60S_61K_62L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF61K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF61K
#define F_PANTECH_CAMERA_EF60S_61K_62L
#elif defined(T_EF62L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF62L
#define F_PANTECH_CAMERA_EF60S_61K_62L
#elif defined(T_NAMI)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_NAMI
#elif defined(T_EF63S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF63S
#define F_PANTECH_CAMERA_EF63SS
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF63K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF63K
#define F_PANTECH_CAMERA_EF63SS
#elif defined(T_EF63L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF63L
#define F_PANTECH_CAMERA_EF63SS
#elif defined(T_EF65S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF65S
#define F_PANTECH_CAMERA_EF65SS
#define F_PANTECH_CAMERA_SKT
#endif

#ifdef F_PANTECH_CAMERA

#ifndef CONFIG_PANTECH_CAMERA
#define CONFIG_PANTECH_CAMERA
/* #define CONFIG_PANTECH_CAMERA_TUNER */
#endif

/*******************************************************************
 * 
 *           SENSOR ( sensor name or external ISP name)
 *
 *******************************************************************/

#ifdef F_PANTECH_CAMERA_TARGET_EF39S
/* ISP backend camera ISP */
#if (CONFIG_BOARD_VER == CONFIG_PT10)
#define F_PANTECH_CAMERA_ICP_HD
#else
#define F_PANTECH_CAMERA_CE1612
#endif
/* 1.3M front camera sensor */
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif

#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#if (CONFIG_BOARD_VER >= CONFIG_WS20)
#define F_PANTECH_CAMERA_S5K4ECGX
#else
#define F_PANTECH_CAMERA_MT9P111
#endif
#define F_PANTECH_CAMERA_MT9V113
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF40S) || \
    defined(F_PANTECH_CAMERA_TARGET_EF40K)
#if (CONFIG_BOARD_VER >= CONFIG_WS20)
#define F_PANTECH_CAMERA_MT9D113
#else
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif
#define F_PANTECH_CAMERA_CE1612
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF45K) || \
    	defined(F_PANTECH_CAMERA_TARGET_EF46L) || \
	defined(F_PANTECH_CAMERA_TARGET_EF47S) || \
	defined(F_PANTECH_CAMERA_TARGET_OSCAR) || \
	defined(F_PANTECH_CAMERA_TARGET_VEGAPVW)
#define F_PANTECH_CAMERA_OV8820	
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif	

#if defined(F_PANTECH_CAMERA_TARGET_CHEETAH) || \
	defined(F_PANTECH_CAMERA_TARGET_ZEPPLIN) 
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_MT9V113	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SVLTE) || \
    defined(F_PANTECH_CAMERA_TARGET_CSFB)
#define F_PANTECH_CAMERA_CE1612	
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF44S) || defined(F_PANTECH_CAMERA_EF48S_49K_50L)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF51S) || defined(F_PANTECH_CAMERA_TARGET_EF51K) || defined(F_PANTECH_CAMERA_TARGET_EF51L) || defined(F_PANTECH_CAMERA_TARGET_EF52S) || defined(F_PANTECH_CAMERA_TARGET_EF52K) || defined(F_PANTECH_CAMERA_TARGET_EF52L) || defined(F_PANTECH_CAMERA_TARGET_EF52W)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_AS0260
#endif

#if defined(F_PANTECH_CAMERA_TARGET_MAGNUS)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SIRIUSLTE)
#define F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_EF56S_57K_58L) || defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || (defined(F_PANTECH_CAMERA_EF63SS) && (CONFIG_BOARD_VER == CONFIG_PT10))
#define F_PANTECH_CAMERA_IMX135
#endif

#if defined(F_PANTECH_CAMERA_EF56S_57K_58L) || defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || defined(F_PANTECH_CAMERA_EF63SS) || defined(F_PANTECH_CAMERA_EF65SS)
#define F_PANTECH_CAMERA_S5K6B2YX
#endif

#if (defined(F_PANTECH_CAMERA_EF63SS) && (CONFIG_BOARD_VER > CONFIG_PT10)) || defined(F_PANTECH_CAMERA_EF65SS)
#define F_PANTECH_CAMERA_IMX214
#endif


/*******************************************************************
 * 
 *           ACTUATOR
 *
 *******************************************************************/
 
#if defined(F_PANTECH_CAMERA_EF56S_57K_58L) || defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || (defined(F_PANTECH_CAMERA_EF63SS) && (CONFIG_BOARD_VER == CONFIG_PT10))
#define F_PANTECH_CAMERA_ACT_WV560
#endif

#if defined(F_PANTECH_CAMERA_EF63SS) || defined(F_PANTECH_CAMERA_EF65SS)
#define F_PANTECH_CAMERA_ACT_RUMBA_SA
#endif

/*******************************************************************
 * 
 *           SENSOR TYPE
 *
 *******************************************************************/

#if defined(F_PANTECH_CAMERA_IMX135) || defined(F_PANTECH_CAMERA_S5K6B2YX) || defined(F_PANTECH_CAMERA_IMX214)
#define F_PANTECH_CAMERA_SENSOR_BAYER
#endif

/*******************************************************************
 * 
 *           SENSOR SPECIFICs
 *
 *******************************************************************/

#ifdef F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_CFG_WDR
#define F_PANTECH_CAMERA_ADD_CFG_UPDATE_ISP
#define F_PANTECH_CAMERA_ADD_CFG_READ_REG

#define F_PANTECH_CAMERA_CFG_STOP_CAPTURE
#endif

#if !defined(F_PANTECH_CAMERA_OV8820)
#define F_PANTECH_CAMERA_BACKFACE_YUV
#endif

#ifdef F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_CFG_GET_FRAME_INFO
#define F_PANTECH_CAMERA_CFG_YUV_ZSL_FLASH
#define F_PANTECH_CAMERA_ADD_CFG_OJT
#endif

#if defined(F_PANTECH_CAMERA_IMX135) || defined(F_PANTECH_CAMERA_IMX214)
/* 
 * EEPROM feature code 
*/
#define F_PANTECH_CAMERA_EEPROM
#endif

#if defined(F_PANTECH_CAMERA_IMX214)
#define F_PANTECH_CAMERA_ADD_OIS
#ifdef F_PANTECH_CAMERA_ADD_OIS
#define CONFIG_PANTECH_CAMERA_ADD_OIS
#endif
#endif

#ifndef T_BUILD_USER
/*#ifndef FEATURE_PANTECH_RELEASE */
/* enable SKYCDBG/SKYCERR in kernel
 * kernel/arch/arm/mach-msm/include/mach/camera.h 
 * by enabling #define F_PANTECH_CAMERA_LOG_PRINTK in kernel/arch/arm/mach-msm/include/mach/camera.h , logs can be printed
*/
/* enable SKYCDBG/SKYCERR in user space(vendor) */
#define F_PANTECH_CAMERA_LOG_OEM		

 /****************************************************************
 * Description : The following files, it is possible to define #define F_PANTECH_CAMERA_LOG_PRINTK, open the log.
 On/Off CDBG message of user area(vendor).
 For debugging, it is used to open the log file for each.
 It is possible to print out a log in accordance with the log level for each file.
 
   vendor/qcom/proprietary/mm-camera/mm-camera2/include/camera_dbg.h 
   vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/sensors/module/sensor_dbg.h 
   vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/imglib/components/include/img_dbg.h 
   camera_dbg.h : CDBG_LEVEL (0 ~ 2)
   sensor_dbg.h : SLOG_HIGH, SLOG_LOW
   img_dbg.h : IDBG_LOG_LEVEL (1 ~ 4)
   
 * Creation Date : 
 * Added By : 
 ****************************************************************/
/*#define F_PANTECH_CAMERA_LOG_CDBG*/

  /****************************************************************
 * Description : On/Off message of LOGV/LOGD/LOGI at user area.  
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_LOG_VERBOSE
#endif

  /****************************************************************
 * Description : Delete the logs that have been repeated in the user area of the camera preview.
 This log is deleted because it is difficult by the log of unnecessary repetition of during preview 
 to see the log of other and debugging.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_ERASE_PREVIEW_LOGS

 /****************************************************************
 * Description : This function that is to be applied by loading a chromatix files for Bayer tuning.
 Need to be removed after the completion of tuning.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#ifdef F_PANTECH_CAMERA_SENSOR_BAYER
#define LOAD_CHROMATIX_DATA_FROM_FILE
#define F_PANTECH_CAMERA_TUNING

 /****************************************************************
 * Description : To apply the code to use with digital and analog gain when using the Bayer Camera sensor.
 It must be set in the user driver of the sensor with digital gain and actual analog gain, from real gain.
 In EF56/59/60 series, applied the digital gain only IMX135 sensor.
 * Creation Date : 2013.11.01.
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_TUNING_CFG_DIGITAL_GAIN
#endif



/*----------------------------------------------------------------------------*/
/*  PANTECH CUSTOMIZATION                                                     */
/*  Optimization or modify it to suit PANTECH Model or Device                 */
/*----------------------------------------------------------------------------*/

 /****************************************************************
 * Description : To modify a table supported recording resolution.
HAL manages available recording resolution in the form of a table, 
and the resolution setting other than those included in the table, it is treated as an error.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_PICTURE_SIZES

 /****************************************************************
 * Description : Of the EXIF TAG information of photos taken at the device, 
 modify the information of the manufacturer-specific.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_OEM_EXIF_TAG

  /****************************************************************
 * Description : To take burst shot "num-snaps-per-shutter" parameter is set at UI.
 And after it is set to burst shot, so that you can burst shot through takePicture.
 Shooting sound so as to play at Camera Service in the same way as single shot.
 Request other than the resolution setting are process as an error.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_BURSTSHOT

#ifdef F_PANTECH_CAMERA_CUST_BURSTSHOT

 /****************************************************************
 * Description : Receiving and processing stop commands from UI during burst shot.
 UI can call stopBurstShot(int numBurst) in Camera.java.
 Complete the jpeg callback as the numBurst if you receive StopBurstShot command.
 And then you can end Burstshot by calling cancelPicture.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_STOP_BURSTSHOT

 /****************************************************************
 * Description : For burst shot, photo-shutter sound is not generated by CameraService, 
 thereby to play a shooting sound in the UI.
 If set to "pantech-burst-sound" "disable", in the case of burst shot, it does not play a shooting sound.
 However it generate shutter callback.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_BURST_UI_SOUND
#endif

 /****************************************************************
 * Description : Added to processing command flash test mode : FLASH_MODE_TORCH_FLASH 
 It is possible to test either torch mode and flash mode.
 After the lightning of any flash mode, and to flash off so automatically after a certain period of time.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_TORCH_FLASH

  /****************************************************************
 * Description : Set the initial value of the ISP (VFE) from CameraHAL.
 It is part of modification to match the default specification of Pantech setting initDefaultParameters().
 For correct operation and optimization of camera enter speed,
 default value of the UI and the setting of the default ISP (VFE) must be match.
 Make sure it matches the initial value of the ISP (VFE).
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_INIT_DEFAULT_PARAM

  /****************************************************************
 * Description : In previous versions of Android Gingerbread, "preview-frame-rate" to fixed fps was set.
Later version of  Android Gingerbread, "preview-fps-range-values" added. Also min_fps and max_fps must be set.
"preview-fps-range-values" can set variable frame rate,if min and max are the same fixed fps is set.
Therefore, it is recommended that you use the "preview-fps-range-values". 
However we support the existing "preview-frame-rate" 
and modify setting in order that there are no conflicts of using the two parameters. 
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_FPS


#define F_PANTECH_CAMERA_CUST_VT_TUNING

/* 2014.03.18. 
 * This feature make Autofocus run before taking picture when focus mode is "continuous-picture" and if flash would be fired.
 * For it, Application have to call Autofocus before calling takePicture and call CancelAutofocus after get Callback with Compressed Image.
 * CameraHAL send Callback for both CAMERA_MSG_FOCUS and CAMERA_MSG_FOCUS_MOVE to application if getting AutoFocus command 
 * when focus-mode is continuous-picture.
 * In case of supporting this feature, "pantech-led-assisted-af" is set "enable" to notify to application.
 * This Feature is available from MSM8974 KK.
*/
#define F_PANTECH_CAMERA_QBUG_LED_ASSISTED_AF


 /****************************************************************
 * Description : Define additional preview resolution that using in the VT preveiw.  
 To create and use a table separately without adding to the existing resolution 
 only to use the resolution of the direction perpendicular to the LCD only VT.
 So use a resolution of VT app in only pantech device.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_FIX_VT_PREVIEW

  /****************************************************************
 * Description : Solution for problem which cannot be sent in KT videos that are encoded in in stagefright.
 Can not be analyzed in the KT server part of the header in the video track of "pasp".
 SKT and LG is working properly.
 Since there is no problem even if there is no pasp, regardless of the telephone company, it can remove
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_FIX_MMS_PASP

/*
 * Moving the location calling releaseSound() from ~Client() to removeClientByRemote().
 * The pid of caller on Client is different from ~Client. so Sound pool can't remove the shutter sound object because pids are different.
 * the pid of Client is application's but the pid of ~Client is MediaServer's.
*/
#define F_PANTECH_CAMERA_FIX_MEDIAPLAYER_CALLING_PID

  /****************************************************************
 * Description : An error occurred when checking in Qparser recorded files of SKY video camera.(cannot decode thumbnail)
 It is a mMaxFileDurationUs value of the difference between 
 SKY Camcorder and video camera recording when Google Camcorder is come down from app.
(SKY Camcorder: 3600000000(us)=1hour / Google Camcorder: 600000000(us)=10minute.)
 Because of difference of mMaxFileDurationUs ,SKY Camcorder is use 64bitfileoffset 
 and Google Camcorder is use 32bitfileoffset in Mpeg4write. 
 To set equal to 32bitfileoffset, it is fix that part.
 Because it is a part to be changed temporarily, monitoring and ongoing additional confirmation is required.
 The EF52 and later was changed to 64bitfileoffset to increase to 4GB in max recording size.
 But in the case of MMS, the recording in 32bitfileoffset.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_VIDEO_REC_FILEOFFSET

/****************************************************************
 * Description : Move to camera service not camera client at the point of 
 loading for improvement of snapshot speed.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_MOVE_LOAD_SOUND

/****************************************************************
 * Description : Applying in case of using of WFD. 
 camera shutter sound must be sounded from local device.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_PLAYSOUND_IN_WFD


/****************************************************************
 * Description : Support for MDM 
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_MDM_CHECK
/*----------------------------------------------------------------------------*/
/*  PANTECH ADD FEATURE                                                                                                */
/*  Please define feature or add API base on pantech device or model                                    */
/*----------------------------------------------------------------------------*/

/****************************************************************
 * Description : Delivering recording size to app during video recording. 
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_ADD_EVT_RECSIZE

/****************************************************************
 * Description : Add to information of jpeg file exif.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_ADD_EXIF_DATA

/****************************************************************
 * Description : Position of lens must move to specific position on UI 
 as add manual mode. focus-mode set to manual, 
 lens can move as set to pantech-focus-step of 0 -9.
 focus step consists of 10 steps based on max 80.
 the focus of camera for EF60 bumper case add on UI using manual focus feature.
 * Creation Date : 2014.01.02
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_ADD_MANUAL_FOCUS

#ifndef F_PANTECH_CAMERA_TARGET_NAMI
/****************************************************************
 * Description : Applying for HDR.
 HDR multi processing with 3 pictures when setting to HDR on.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CFG_HDR

/****************************************************************
 * Description : HDR checker
 evaluate preview and set aebracket
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_HDR_CHECKER

/****************************************************************
 * Description : Applying for HDR thumbnail.
 thumbnail created by main image. 
 fixed because of difference between main and thumbnail in normal process.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_HDR_THUMBNAIL

/****************************************************************
 * Description : During effects recording, set mIsRecording to 1
 * Creation Date : 2014/06/10
 * Added By : Ha Dong Jin
 ****************************************************************/
#define F_PANTECH_CAMERA_EFFECTS_RECORDING
/****************************************************************
 * Description : Fixed for closing camera when clicking cancel button after 
 recording pause.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_RECORDING_PAUSE

/****************************************************************
 * Description : Fixed for thumbnail with rotation of 90, 270 and with brightness 
 during taking a picture as HDR.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_HDR_THUMBNAIL_KK

#if defined(F_PANTECH_CAMERA_EF63SS) || defined(F_PANTECH_CAMERA_EF65SS)
/****************************************************************
 * Description : Applying for LLS (LOW LIGHT SHOT)
 LLS multi processing with 5 pictures when setting to LLS on.
 * Creation Date : 2014/2/14
 * Added By : Leehwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_CFG_LLS 
#endif
#endif

/****************************************************************
 * Description : Applying for SKT only and fined on UI Vob 
 (Device Control Management Object), 
 must have "pantech/development/sky_fota/sky_fota.h" file.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
/* #define F_PANTECH_CAMERA_FOTA_DCMO_CAMERA_LOCK */

/****************************************************************
 * Description : Applying for AOT
 Add function of isRunning for checking operating state.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_AOT_ISRUNNING

/****************************************************************
 * Description : Applying for reconition of motion
 to avoid to put frame into preview buffer
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#if defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || defined(F_PANTECH_CAMERA_EF63SS) || defined(F_PANTECH_CAMERA_EF65SS)
#define F_PANTECH_CAMERA_VTS
#endif

/****************************************************************
 * Description : Change mode for signle isp.
 raw capture is not working in dual isp, so change mode to single isp in case of 
 raw capture.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_SINGLE_ISP

/****************************************************************
 * Description : Apply for thumbnail filp.
 thumbnail image is saved with flipped when taking a picture in from camera.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_THUMBNAIL_FLIP

/****************************************************************
 * Description : Apply for filp cover.
 this support to smart filp camera feature in flip cover. 
 because size and ratio of window is not used nomal case, must used in smart filp 
 camera app not adding resolution to table. do not check resolution discussed from UI 
 because size of window can be changed in the middle of developing.
 parameter - "pantech-window-camera" : "enable"/"disable"
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_FLIP_COVER_CAMERA

/****************************************************************
 * Description : Apply for connect fail .
 when new camera  try to connect to open, have to disconnect to previous camera.
 and close flash when trying to connect to open in flash app.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_DEVICE_CONNECT_FAIL_FIXED

/****************************************************************
 *--------------------------------------------------------------- 
 * Qualcomm BUGS  
 * Define in below in case of SR, patch as Qulcomm bus.
 * Need to look around about applying for Qualcomm main stream
 *----------------------------------------------------------------
 ****************************************************************/

/****************************************************************
 * Description : Fixed for not selecting optimized mode in sensor mode 
 because of QC bug.
 * Creation Date : 2014.01.28
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_SENSOR_PICK_RES

/****************************************************************
 * Description : case#01169476
 front and rear camera can use simultaneously as applied dual vfe
 reference for kernel : #if(n)def CONFIG_PANTECH_CAMERA // case#01169476 simultaneous camera
 * Creation Date :
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_SIMULTANEOUS_CAM

/****************************************************************
 * Description : case#01187023
 feature for fix side effect occurred when applying 13M@24fps ZSL (dual vfe)
 fixed for frame not synced with VFE0 and VFE1 on ZSL on/off 
 * Creation Date : 2013-05-14
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_HIGH_PERFORMANCE

/****************************************************************
 * Description : fixed for broken image captured from zsl mode
 this is case that saved as one frame with chopped between previous and next frame
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_FIX_QBUG_BROCKEN_IMAGE

/****************************************************************
 * Description : fixed for decording error for broken png file in skia decoder
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_FIX_PNG_DECORDING_ERROR_WIH_BROKEN_DATA

/****************************************************************
 * Description : fixed that snapshot dump image is not seen nomally when taking
 a picture verically
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_DUMP_IMAGE

/****************************************************************
 * Description : case # 01466765
 fix wrong image capture problem, the cause was wrong buffer count.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_WRONG_IMAGE_CAPTURE

/****************************************************************
 * Description : case#01473686
 stop preview error during dual camera recording stop.
 fix SIGSEGV in mct_pipeline_check_stream.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_STOP_PREVIEW

/****************************************************************
 * Description : case#01473540
 increasing meta buffer cnt from 8 to 10.
 * Creation Date : 2014/03/11
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_PREPARE_SNAPSHOT

/****************************************************************
 * Description : case#01504004
fix prepare snapshot done problem
 * Creation Date : 2014/04/21
 * Added By : Ha Dong Jin
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_PREPARE_SNAPSHOT_DONE

/****************************************************************
 * Description : EF61K PLM#664, CASE#1349270, CR#522939
 Clean-up the msm camera generic buff queue on userspace
 crash to avoid invalid memory access in next session
 which might lead to corruption in other modules
 and system stability issues and also avoid memory leak.
 * Creation Date : 2013/12/02
 * Added By : Ha Dong Jin
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_CLEANUP_MSM_GENERIC_BUF

/****************************************************************
 * Description : case# 01215481
 fixed for recording with no effect when starting effect recording
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_EFFECT_RECORDING

/****************************************************************
 * Description : fixed that continuous af does not stop when focus mode changes
 "continuous-video" to "auto".
 caf does not stop because of caf_stat as loaded alog type abnomally from camcorder caf file.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_CONTINUOUS_VIDEO_STOP

#ifdef F_PANTECH_CAMERA_IMX135
/****************************************************************
 * Description : fixed that imx135_fill_awb_hdr_array is not called when applying
 movie HDR supported on IMX135 sensor.
 fixed for recording with no effect when starting effect recording
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_MOVIE_HDR

/****************************************************************
 * Description : customizing for optimization for movie HDR supported on IMX135 sensor.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_CUST_MOVIE_HDR
#endif

/****************************************************************
 * Description : work around code for shutter callback
 must return callback only for 1 time after determine sound play when shutter callback
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_SHUTTER_CALLBACK

/****************************************************************
 * Description : fixed for halt when pressed the home key button during snapshot.
 this is issue for halt for OMX Jpeg encoder releas.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_STOP_ON_SNAPSHOT

/****************************************************************
 * Description : workaround for not applying flash or movie mode before start preview.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_LED_MODE

/****************************************************************
 * Description : fixed for halt that mediaplayer operated abnomally when starting/finishing a
 recording rapidly, trying to continous shot with earjack.
 this issue occured 100% when trying to replay as same music file, so 
 must play after check if previous music file is done.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
//#define F_PANTECH_CAMERA_QBUG_SKIP_CAMERA_SOUND  

/****************************************************************
 * Description : fixed for not applying sharpness during intialization
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_SHARPNESS

/****************************************************************
 * Description : case# 01438139
 applying QC patch code for thumbnail roation from MSM8974 KK
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_THUMB_ROTATION

/****************************************************************
 * Description : 
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_FIX_GOOFY_FACE_RECORD

/****************************************************************
 * Description : fixed for not applying effect when starting a recording.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_EFFECT

/****************************************************************
 * Description : fixed for hold during map/unmap in case of  starting/stopping preview 
 after applying CPL patch.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_HOLD_DURING_MAP_UNMAP

/****************************************************************
 * Description : The camera mode is picked up according to resolution and
 fps but the snapshot chromatix in the orignal code is just set with MSM_SENSOR_RES_FULL is 0. 
 So, modified code is selecte snapshot chromatix according to camera mode selected.
 after applying CPL patch.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_WRONG_SNAPSHOT_CHROMATIX

/****************************************************************
 * Description : resolution of live shot must use recording size not picture size.
 * Creation Date : 
 * Added By : 
 ****************************************************************/
 #define F_PANTECH_CAMERA_CUST_LIVESNAPSHOT

/****************************************************************
 * Description : case# 01413457
 fixed for setting value of VFE MAX clock
 * Creation Date : 2014/01/09
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_VFE_MAX_CLOCK_SETTING

/****************************************************************
 * Description : case# 01388126
Qualcomm fixes for known FD leakage issues in camera side
 * Creation Date : 2013/12/19
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_FD_LEAKAGE

/****************************************************************
 * Description : case# 01388126
Qualcomm fix memory leak in luma adaptation 
 * Creation Date : 2013/12/19
 * Added By : 
 ****************************************************************/
/* #define F_PANTECH_CAMERA_QBUG_LUMA_LEAKAGE */

/****************************************************************
 * Description : case# 01428626
issue for camera stability, occured fail when stream on, refer to error log as below
mm-camera stats_port_event: Failure posting to the bus!
mm-camera isp_ch_util_streamon_ack: error, ISP_HW_ACTION_CODE_STREAM_START_ACK, sessid = 1, vfe_id = 0, rc = -1
apply QC patch code because ISP Buffer manager has error for handling regarding to mutex.
we fixed to apply patch as same issue for MSM8974 JB, but we didn't apply JB patch because of some change on KK.
applied patch code based on LINUX build ID LNX.LA.3.5-07510-8x74.0-1.
 * Creation Date : 2014.01.23
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_MUTEX_FOR_ISP_BUFMGR

/****************************************************************
 * Description : case# 01452485
 fix module_imglib_clear_session_params fail.
 * Creation Date : 2014/2/18
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_IMGLIB_CLEAR

/****************************************************************
 * Description : case# 01484820
 destroy super buf cmd thread.
 * Creation Date : 2014/2/18
 * Added By : 
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_RELEASE_THREAD

/*
* 2014.03.27. case:01475905
* stream on fail issue when preview is started
* in issue case, below fail log is occured.
* "QCameraStream Failed to config stream, rc = -1"
* Additionally, you can see only the log for enterance of preview_start without ending of start_preview.
*/
#define F_PANTECH_CAMERA_QBUG_STREM_ON_FAIL


#if defined(F_PANTECH_CAMERA_IMX214)
/*
* 2014/03/28 
* Add video size 4k x 2k(UHD) to supported preview and video list, if "pantech-4k-res" is set with "enable"
*/
/****************************************************************
 * Description : video size 4k x 2k(UHD) support is enabled.
 * Creation Date : 2014/02/13
 * Added By : lee sangwon
 ****************************************************************/
#define F_PANTECH_CAMERA_SUPPORT_4K2K_UHD
#endif

#if defined(F_PANTECH_CAMERA_EF63SS) || defined(F_PANTECH_CAMERA_EF65SS)
/****************************************************************
 * Description : Case#01460192
 Added the QCom Patch about High Speed Recording (CR605405)
 * Creation Date : 2014/02/28
 * Added By : lee sangwon
 ****************************************************************/
#define F_PANTECH_CAMERA_SUPPORT_HSR
/****************************************************************
 * Description : Tunning for uhd recording with dark preview
 * Creation Date : 2014/04/10
 * Added By : lee hwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_UHD_PREVIEW_TUNNING
#endif

#ifdef F_PANTECH_CAMERA_SUPPORT_HSR
/****************************************************************
 * Description : Case#01460192
 support for the slow motion
 * Creation Date : 2014/03/05
 * Added By : lee sangwon
 ****************************************************************/
#define F_PANTECH_CAMERA_SUPPORT_SLOW_MOTION
#endif

/****************************************************************
 * Description : video size 4k x 2k(UHD) support is enabled.
 Fix Camcoder Recording File Audio not Sync
 Because Add H/W ACC Encoder use code in StageFrightRecorder.cpp by Post CR Patch.
 * Creation Date : 2014/02/28
 * Added By : lee junhee
 ****************************************************************/
#define F_PANTECH_CAMERA_FIX_CAM_AUDIO_SYNC

/****************************************************************
 * Description : Fix mms recording problem. The track header size is too big 
 in mms recording case, So set it to 6KB.
 * Creation Date : 2014/03/10
 * Added By : lee sangwon
 ****************************************************************/
#define F_PANTECH_CAMERA_MMS_RECORDING

/****************************************************************
 * Description : fixed issue that jpeg exif data is not seen as flash fired in torch mode.
 * Creation Date : 2014/03/24
 * Added By : lee hwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_JPEG_EXIF_DATA_IN_TORCH_MODE


/****************************************************************
 * Description : support hfr preview recording with 60fps
 * Creation Date : 2014/04/25
 * Added By : lee hwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_SUPPORT_HFR_PREVIEW_60FPS

/****************************************************************
 * Description : bug fixed for recording stopping during liveshot
 * Creation Date : 2014/04/21
 * Added By : lee hwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_RECORDING_STOP_DURING_LIVESHOT

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC CONSTANTS                                                                                      */
/*  Define enum type for model                                                                                           */
/*----------------------------------------------------------------------------*/

/* camera select enum */
#define C_PANTECH_CAMERA_SELECT_MAIN_CAM 		0
#define C_PANTECH_CAMERA_SELECT_SUB_CAM 		1

#ifndef F_PANTECH_CAMERA_TARGET_NAMI
//P11232 US4 CAMERA
#define F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#ifdef F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#define F_PANTECH_CAMERA_CFG_CAMNOTE
#define F_PANTECH_CAMERA_CFG_MINIATURE
#define F_PANTECH_CAMERA_CFG_COLOREXTRACTION
#endif
#endif

#ifdef F_PANTECH_CAMERA_CUST_OEM_EXIF_TAG
#define C_PANTECH_CAMERA_EXIF_MAKE		"PANTECH"
#define C_PANTECH_CAMERA_EXIF_MAKE_LEN		8		/* including NULL */
#ifdef F_PANTECH_CAMERA_TARGET_EF39S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A800S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#define C_PANTECH_CAMERA_EXIF_MODEL		"PRESTO"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		7		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF45K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF46L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF47S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SVLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SVLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CSFB
#define C_PANTECH_CAMERA_EXIF_MODEL		"CSFB"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CHEETAH
#define C_PANTECH_CAMERA_EXIF_MODEL		"CHEETAH"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define C_PANTECH_CAMERA_EXIF_MODEL		"STARQ"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_OSCAR
#define C_PANTECH_CAMERA_EXIF_MODEL		"OSCAR"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_VEGAPVW
#define C_PANTECH_CAMERA_EXIF_MODEL		"ADR930L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_ZEPPLIN
#define C_PANTECH_CAMERA_EXIF_MODEL		"ZEPPLIN"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SIRIUSLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		10		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF44S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A840S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL		"P9090"		//MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF48S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF49K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF50L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52W
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF56S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF57K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF58L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF60S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF61K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF62L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_NAMI
#define C_PANTECH_CAMERA_EXIF_MODEL		"NAMI"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF63S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF63K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF63L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF65S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif

#endif
#endif /* F_PANTECH_CAMERA */

#endif /* CUST_PANTECH_CAMERA.h */
