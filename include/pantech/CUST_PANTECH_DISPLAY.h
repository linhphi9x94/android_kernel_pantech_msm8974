#ifndef F_SKYDISP_FRAMEWORK_FEATURE
#define F_SKYDISP_FRAMEWORK_FEATURE

/****************************************************/
/*                             PANTECH FEARTURE                             */
/****************************************************/

 /*****************************************************
* owner  : p13832          
* date    : 2013.04.11 
* PLM    : 
* Case  :       
* Description : it's for cabc control
* kernel : used
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L)
#undef CONFIG_F_SKYDISP_CABC_CONTROL
#define CONFIG_F_SKYDISP_CABC_CONTROL
#endif

#undef CONFIG_F_SKYDISP_SHARPNESS_CTRL
#define CONFIG_F_SKYDISP_SHARPNESS_CTRL


 /*****************************************************
* owner  : p13832          
* date    : 2013.12.04
* PLM    : 
* Case  :       
* Description : power on off sequence
* kernel : none
* user    : used
******************************************************/

#define CONFIG_F_SKYDISP_SHUTDOWN_BUGFIX

 /*****************************************************
* owner  : p13832          
* date    : 2013.04.23
* PLM    : 
* Case  :       
* Description : it's for lcd onff
* kernel : none
* user    : used
******************************************************/

#define CONFIG_F_SKYDISP_NEW_LCD_ONOFF

 /*****************************************************
* owner  : p13832          
* date    : 2013.07.02
* PLM    : 
* Case  :       
* Description : it's for lcd cmds test
* kernel : used
* user    : used
******************************************************/

#define CONFIG_F_SKYDISP_CMDS_CONTROL

 /*****************************************************
* owner  : p12452          
* date    : 2014.01.27
* PLM    : 
* Case  :       
* Description : DRM contents is not shown to external device , (ex HDMI , MiraCast) 
*                    But, used HDCP2.x from EF63-series 
* kernel : none
* user    : used
******************************************************/
#if defined(T_EF56S) || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) || defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L)
#define CONFIG_F_SKYDISP_DRM_ISNOT_SHOWN_TO_MIRACAST
#endif

 /*****************************************************
* owner  : p13832          
* date    : 2013.08.19
* PLM    : 
* Case  :       
* Description : it's for lcd backlight
* kernel : used
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_BACKLIGHT_CMDS_CTL


#if defined(T_EF59L) || defined(T_EF62L) || defined(T_EF63L)

/*****************************************************
* owner  :?
* date   : 2014.01.20
* PLM    :
* Case   :
* Description : UVS Service related, Implement checkFramebufferUpdate() API.Only for LGU+
* kernel : none
* user   : used
******************************************************/
#define CONFIG_F_SKYDISP_UVS
#endif

/*****************************************************
* owner  : p13042
* date   : 2014.01.21
* PLM    :
* Case   :
* Description : It shows partial logos on the bootloader.
*               But it doesn't use thread to display bootloader's logo
* kernel : none
* user   : used
******************************************************/
#define DISPLAY_SKYDISP_PARTIAL_LOGO

#if defined(T_EF63S) ||defined(T_EF63K) || defined(T_EF63L)

/*****************************************************
* owner  : p13832
* date   : 2014.01.23
* PLM    :
* Case   :
* Description : oled hbm feature
* kernel : use
* user   : used
******************************************************/
#define CONFIG_F_SKYDISP_HBM_FOR_AMOLED
#endif

#if defined(T_EF63S) ||defined(T_EF63K) || defined(T_EF63L)

/*****************************************************
* owner  : p13832
* date   : 2014.02.06
* PLM    :
* Case   :
* Description : oled smart dimming
* kernel : use
* user   : used
******************************************************/
#define CONFIG_F_SKYDISP_SMARTDIMMING
#endif

 /*****************************************************
* owner  : p16603       
* date   : 2014.02.26
* PLM    : 
* Case   :        
* Description : Fix miscalculation in Layer.cpp when has not intersect rect activeRect and transparentRect
* kernel : none
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_FIX_MISCALCULATION_WHEN_HAS_NOT_INTERSECT_RECT

/*****************************************************
* owner  : p16603       
* date   : 2014.03.04
* PLM    : 
* Case   : 01469205  
* Description : 720p wfd support.
* kernel : none
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_SUPPORT_720P_WFD


/*****************************************************
* owner  : p16603
* date   : 2014.02.11
* PLM    : EF60S_KK 356
* Case   : 01444586
* Description : hwc_utils.cpp bugfix.
*				about layer's rect calculate(prev : float -> int non casting. after : float -> int with casting)
* kernel : none
* user   : used
******************************************************/
#define DISPLAY_SKYDISP_FIX_RECT_CALCULATE


#if defined(T_EF56S)  || defined(T_EF59S) || defined(T_EF59K) || defined(T_EF59L) ||\
	defined(T_EF60S) || defined(T_EF65S) || defined(T_EF61K) || defined(T_EF62L)  || defined(T_EF63S) ||\
	defined(T_EF63K) || defined(T_EF63L)
#undef QUALCOMMM_BUGFIX_
#define QUALCOMMM_BUGFIX_
#endif

/****************************************************/
/*                             QUALCOMM FEARTURE                             */
/****************************************************/


/*****************************************************
* owner  : p10443       
* date    : 2013.10.29
* PLM    : 
* Case  : 01321798       
* Description : UHD(3840*2160, 2160*3840) contents underrun in video play and AOT 
* kernel : none
* user    : used
******************************************************/
#define QUALCOMM_BUGFIX_UHD_UNDERRUN


 /*****************************************************
* owner  : p10443      
* date    : 2014.03.05
* PLM    : 
* Case  : 01451879     
* Description : display broken in dual window - video player/gallery setting  recent apps -> vido play(repeat)
* kernel : none
* user    : used
* TODO : official patch release check 
******************************************************/
#define QUALCOMM_BUGFIX_DUALWINDOW_VIDEO_PLAYER_HALT

 /*****************************************************
* owner  : p13832      
* date    : 2014.04.03
* PLM    : 
* Case  :   01502968 
* Description : the font disappear(HardwareRenderer.java)
* kernel : none
* user    : used
* TODO : (JB patch)
******************************************************/
#define QUALCOMM_BUGFIX_FONT_ERROR


/*****************************************************
* owner  : p16603       
* date   : 2014.04.10
* PLM    : 
* Case   : 01505067  
* Description : Image distortion when play 4k video with wfd
* kernel : none
* user    : used
******************************************************/
#define CONFIG_F_SKYDISP_DO_NOT_INCREASE_DECIMATOR_WB_PANEL

 /*****************************************************
* owner  : p13832      
* date    : 2014.06.11
* PLM    : 
* Case  :   01561678 
* Description : the hwui cache error(image corruption)
* kernel : none
* user    : used
* TODO : (JB patch)
******************************************************/
#define GOOGLE_BUG_FIX_GRAPHICS_CORRUPTION_BY_HWUI_CACHES

 /*****************************************************
* owner  : p13832     
* date    : 2014.07.16
* PLM    : EF63S_3508
* Case  :   01628717  
* Description : Fix the texture ID reuse issue in HWUI
                      Issue: When the layer of previous frame is destroyed, it doesn't clear the
			 texture id in mBoundTextures[mTextureUnit], so in the next frame, if
			 glGenTexture returns same texture ID of the previous frame,
			 the new texture is not bound.

			 CRs-fixed: 671736
* kernel : none
* user    : used
* TODO : 
******************************************************/
#define GOOGLE_BUGFIX_HWUI_CLEAR_ID

#endif  /* SKY_FRAMEWORK_FEATURE */
