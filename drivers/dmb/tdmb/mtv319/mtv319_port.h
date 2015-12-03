/******************************************************************************
* (c) COPYRIGHT 2012 RAONTECH, Inc. ALL RIGHTS RESERVED.
*
* This software is the property of RAONTECH and is furnished under license
* by RAONTECH.
* This software may be used only in accordance with the terms of said license.
* This copyright noitce may not be remoced, modified or obliterated without
* the prior written permission of RAONTECH, Inc.
*
* This software may not be copied, transmitted, provided to or otherwise
* made available to any other person, company, corporation or other entity
* except as specified in the terms of said license.
*
* No right, title, ownership or other interest in the software is hereby
* granted or transferred.
*
* The information contained herein is subject to change without notice
* and should not be construed as a commitment by RAONTECH, Inc.
*
* TITLE		: MTV319 configuration header file.
*
* FILENAME	: mtv319_port.h
*
* DESCRIPTION	:
*		Configuration for RAONTECH MTV319 Services.
*
******************************************************************************/
/******************************************************************************
* REVISION HISTORY
*
*    DATE         NAME          REMARKS
* ----------  -------------    ------------------------------------------------
* 07/12/2012  Ko, Kevin        Created.
******************************************************************************/

#ifndef __MTV319_PORT_H__
#define __MTV319_PORT_H__

/*=============================================================================
 * Includes the user header files if neccessry.
 *===========================================================================*/
#if defined(__KERNEL__) /* Linux kernel */
	#include <linux/io.h>
	#include <linux/kernel.h>
	#include <linux/delay.h>
	#include <linux/mm.h>
	#include <linux/mutex.h>
	#include <linux/uaccess.h>
	#include <linux/string.h>
	#include <linux/jiffies.h>

#elif defined(WINCE) || defined(WINDOWS) || defined(WIN32)
	#include <windows.h>
	#include <winbase.h>
	#include <string.h>
	#ifdef WINCE
		#include <drvmsg.h>
	#endif
#else
	#include <stdio.h>
	#include <string.h>
#endif

#include "../tdmb_comdef.h"
#include "mtv319_bb.h"

#ifdef __cplusplus
extern "C"{
#endif

/*############################################################################
#
# COMMON configurations
#
############################################################################*/
/*============================================================================
* The slave address for I2C and SPI.
*===========================================================================*/
#define RTV_CHIP_ADDR	0x86

/*============================================================================
* Modifies the basic data types if neccessry.
*===========================================================================*/
typedef int			BOOL;
typedef signed char		S8;
typedef unsigned char		U8;
typedef signed short		S16;
typedef unsigned short		U16;
typedef signed int		S32;
typedef unsigned int		U32;

typedef int			INT;
typedef unsigned int		UINT;
typedef long			LONG;
typedef unsigned long		ULONG;

typedef volatile U8		VU8;
typedef volatile U16		VU16;
typedef volatile U32		VU32;


#if defined(__GNUC__)
	#define INLINE	inline
#elif defined(WINCE) || defined(WINDOWS) || defined(WIN32)
	#define INLINE	__inline
#elif defined(__ARMCC_VERSION)
	#define INLINE	__inline
#else
    /* Need to modified */
    #define INLINE	inline
#endif

/*============================================================================
* Defines the package type of chip to target product.
*===========================================================================*/
#define RTV_CHIP_PKG_CSP
//#define RTV_CHIP_PKG_QFN

/*============================================================================
* Defines the external source freqenecy in KHz.
* Ex> #define RTV_SRC_CLK_FREQ_KHz	36000 // 36MHz
*===========================================================================*/
#ifdef FEATURE_DMB_CLK_24576
#define RTV_SRC_CLK_FREQ_KHz		24576
#elif defined(FEATURE_DMB_CLK_19200)
#define RTV_SRC_CLK_FREQ_KHz		19200
#endif
/*============================================================================
* Defines the Host interface.
*===========================================================================*/
#ifdef FEATURE_DMB_SPI_IF
#define RTV_IF_SPI /* AP: SPI Master Mode */
#else
//#define RTV_IF_TSIF /* I2C + TSIF Master Mode*/
//#define RTV_IF_SPI_SLAVE /* AP: SPI Slave Mode*/
//#define RTV_IF_EBI2 /* AP: Master Mode */
#endif
/*============================================================================
* Defines the polarity of interrupt if necessary.
*===========================================================================*/
#define RTV_INTR_POLARITY_LOW_ACTIVE
//#define RTV_INTR_POLARITY_HIGH_ACTIVE

/*============================================================================
* Defines the delay macro in milliseconds.
*===========================================================================*/
#if defined(__KERNEL__) /* Linux kernel */
	#define RTV_DELAY_MS(ms)    mdelay(ms)

#elif defined(WINCE)
	#define RTV_DELAY_MS(ms)    Sleep(ms)

#else
	void mtv_delay_ms(int ms);
	#define RTV_DELAY_MS(ms)    mtv_delay_ms(ms) /* TODO */
#endif

/*============================================================================
* Defines the debug message macro.
*===========================================================================*/
#if 1
    #define RTV_DBGMSG0(fmt, arg...)                   TDMB_MSG_RTV_BB(fmt, ##arg)//printk(fmt)
    #define RTV_DBGMSG1(fmt, arg...)             TDMB_MSG_RTV_BB(fmt, ##arg)//printk(fmt, arg1)
    #define RTV_DBGMSG2(fmt, arg...)       TDMB_MSG_RTV_BB(fmt, ##arg)//printk(fmt, arg1, arg2)
    #define RTV_DBGMSG3(fmt, arg...) TDMB_MSG_RTV_BB(fmt, ##arg)//printk(fmt, arg1, arg2, arg3)
#else
    /* To eliminates the debug messages. */
    #define RTV_DBGMSG0(fmt)			do {} while (0)
    #define RTV_DBGMSG1(fmt, arg1)		do {} while (0)
    #define RTV_DBGMSG2(fmt, arg1, arg2)	do {} while (0)
    #define RTV_DBGMSG3(fmt, arg1, arg2, arg3)	do {} while (0)
#endif
/*#### End of Common ###########*/


/*#############################################################################
#
# T-DMB specific configurations
#
#############################################################################*/
#define RTV_REMOVE_NOISE_MSC

/* Determine if the FIC is not handled by interrupt. */
//#define RTV_FIC_POLLING_MODE

/* Defines the number of sub channel to be open simultaneously. (AV + DATA) */
#define RTV_MAX_NUM_USE_SUBCHANNEL	1

#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
  #ifdef  FEATURE_TDMB_MULTI_CHANNEL_ENABLE
	#define RTV_SPI_FIC_DECODE_IN_PLAY
  #endif

	/* The size of interrupt level for CIF mode of SPI interface. */
	#define RTV_SPI_CIF_MODE_INTERRUPT_SIZE	(16 * 188)

#elif defined(RTV_IF_TSIF) || defined(RTV_IF_SPI_SLAVE)
	/*========================================================*/
	/* Selects FIC transmission path when SCAN and PLAY state */
	/* NA: Not Applicable                                     */
	/*========================================================*/
	//#define RTV_FIC__SCAN_I2C__PLAY_NA /* Polling or Intrrupt. */
	//#define RTV_FIC__SCAN_I2C__PLAY_I2C /* Polling or Intrrupt */
	#define RTV_FIC__SCAN_I2C__PLAY_TSIF /* Polling or Intrrupt */
	//#define RTV_FIC__SCAN_TSIF__PLAY_NA /* Polling meaningless */
	//#define RTV_FIC__SCAN_TSIF__PLAY_I2C /* Polling or Intrrupt */
	//#define RTV_FIC__SCAN_TSIF__PLAY_TSIF /* Polling meaningless */
#endif

/* Determine if the output of error-tsp is disable. */
#define RTV_ERROR_TSP_OUTPUT_DISABLE

#ifndef RTV_ERROR_TSP_OUTPUT_DISABLE
	/* Determine if the NULL PID will generated for error-tsp. */
	//#define RTV_NULL_PID_GENERATE

	/* Determine if the sync-byte was insert for error-tsp forcely. */
	#define RTV_FORCE_INSERT_SYNC_BYTE
#endif /* RTV_ERROR_TSP_OUTPUT_DISABLE */


/* Determine if the CIF decoder is compiled with RAONTECH driver. */
#define RTV_CIFDEC_IN_DRIVER

#ifdef RTV_CIFDEC_IN_DRIVER
	/* Select the copying method of CIF decoded data(FIC and MSC).
	CIF decoder copy the decoded data into user space buffer direcly
	to fast operation.
	NOTE: Only applicable in RTV_CIF_MODE_ENABLED defined. */
	//#define RTV_CIFDEC_DIRECT_COPY_USER_SPACE
#endif


/*############################################################################
#
# Host Interface specific configurations
#
############################################################################*/
#if defined(RTV_IF_SPI)
	/*=================================================================
	* Defines the register I/O macros.
	*================================================================*/
	int mtv319_spi_init(void); //wgon build error
	U8 tdmb_spi_read(U8 page, U8 reg);
	void tdmb_spi_read_burst(U8 page, U8 reg, U8 *buf, int size);
	void tdmb_spi_write(U8 page, U8 reg, U8 val);
	extern U8 g_bRtvPage;

	static INLINE U8 RTV_REG_GET(U8 reg)
	{
		if (g_bRtvPage == 0xF)
			reg ^= 0x10;

		return (U8)tdmb_spi_read(g_bRtvPage, (U8)(reg));
	}

	#define RTV_REG_BURST_GET(reg, buf, size)\
		tdmb_spi_read_burst(g_bRtvPage, (U8)(reg), buf, (size))

	static INLINE void RTV_REG_SET(U8 reg, U8 val)
	{
		if (g_bRtvPage == 0xF)
			reg ^= 0x10;	

		tdmb_spi_write(g_bRtvPage, (U8)(reg), (U8)(val));
	}

	#define RTV_REG_MASK_SET(reg, mask, val)\
	do {					\
		U8 tmp;				\
		tmp = (RTV_REG_GET(reg)|(U8)(mask))\
				& (U8)((~(mask))|(val));\
		RTV_REG_SET(reg, tmp);		\
	} while (0)

	#define RTV_TSP_XFER_SIZE	188

#elif defined(RTV_IF_TSIF) || defined(RTV_IF_SPI_SLAVE)
	/*=================================================================
	* Defines the TS format.
	*================================================================*/
	//#define RTV_TSIF_FORMAT_0 /* EN_high, CLK_rising */
	#define RTV_TSIF_FORMAT_1 /* EN_high, CLK_falling */ // MV210
	//#define RTV_TSIF_FORMAT_2 /* EN_low, CLK_rising */
	//#define RTV_TSIF_FORMAT_3 /* EN_low, CLK_falling */
	//#define RTV_TSIF_FORMAT_4 /* EN_high, CLK_rising + 1CLK add */
	//#define RTV_TSIF_FORMAT_5 /* EN_high, CLK_falling + 1CLK add */
	//#define RTV_TSIF_FORMAT_6 /* Parallel: EN_high, CLK_falling */

	/*=================================================================
	* Defines the TSIF speed.
	*================================================================*/
	//#define RTV_TSIF_SPEED_3_Mbps  /* 2.41MHz */
	#define RTV_TSIF_SPEED_4_Mbps   /* 3.62MHz */
	//#define RTV_TSIF_SPEED_5_Mbps /* 4.8MHz */
	//#define RTV_TSIF_SPEED_6_Mbps /* 5.8MHz */
	//#define RTV_TSIF_SPEED_8_Mbps /* 7.2MHz */
	//#define RTV_TSIF_SPEED_10_Mbps /* 9.6MHz */
	//#define RTV_TSIF_SPEED_15_Mbps /* 14.5MHz */
	//#define RTV_TSIF_SPEED_30_Mbps /* 28.8MHz */
	//#define RTV_TSIF_SPEED_60_Mbps /* 58.5MHz */

	/*=================================================================
	* Defines the TSP size. 188 or 204
	*================================================================*/
	#define RTV_TSP_XFER_SIZE	188

	/*=================================================================
	* Defines the register I/O macros.
	*================================================================*/
	U8 tdmb_i2c_read(U8 chipid, U8 reg);
	void tdmb_i2c_read_burst(U8 reg, U8 *buf, int size);
	void tdmb_i2c_write(U8 chipid, U8 reg, U8 val);
	extern U8 g_bRtvPage;

	static INLINE U8 RTV_REG_GET(U8 reg)
	{
		if (g_bRtvPage != 0xF)
			return (U8)tdmb_i2c_read((U8)RTV_CHIP_ADDR, (U8)(reg));
		else {
			tdmb_i2c_write(RTV_CHIP_ADDR, 0x02, (0x62|0x80));
			return (U8)tdmb_i2c_read((U8)(0x62), (U8)(reg));
		}
	}

	#define	RTV_REG_BURST_GET(reg, buf, size)\
			tdmb_i2c_read_burst((U8)(reg), buf, size)

	static INLINE void RTV_REG_SET(U8 reg, U8 val)
	{
		if (g_bRtvPage != 0xF)
			tdmb_i2c_write(RTV_CHIP_ADDR, (U8)(reg), (U8)(val));
		else {
			   tdmb_i2c_write(RTV_CHIP_ADDR, 0x02, (0x62|0x80));
			   tdmb_i2c_write(0x62, (U8)(reg), (U8)(val));
		}
	}

	#define	RTV_REG_MASK_SET(reg, mask, val)\
		do {					\
			U8 tmp;				\
			tmp = (RTV_REG_GET(reg)|(U8)(mask))\
					& (U8)((~(mask))|(val));\
			RTV_REG_SET(reg, tmp);		\
		} while (0)

#elif defined(RTV_IF_EBI2)
	/*=================================================================
	* Defines the register I/O macros.
	*================================================================*/
	U8 tdmb_ebi2_read(U8 page, U8 reg);
	void tdmb_ebi2_read_burst(U8 page, U8 reg, U8 *buf, int size);
	void tdmb_ebi2_write(U8 page, U8 reg, U8 val);
	extern U8 g_bRtvPage;

	static INLINE U8 RTV_REG_GET(U8 reg)
	{
		if (g_bRtvPage == 0xF)
			reg ^= 0x10;

		return (U8)tdmb_ebi2_read(g_bRtvPage, (U8)(reg));
	}

	#define RTV_REG_BURST_GET(reg, buf, size)\
		tdmb_ebi2_read_burst(g_bRtvPage, (U8)(reg), buf, (size))

	static INLINE void RTV_REG_SET(U8 reg, U8 val)
	{
		if (g_bRtvPage == 0xF)
			reg ^= 0x10;	

		tdmb_ebi2_write(g_bRtvPage, (U8)(reg), (U8)(val));
	}

	#define RTV_REG_MASK_SET(reg, mask, val)\
	do {					\
		U8 tmp;				\
		tmp = (RTV_REG_GET(reg)|(U8)(mask))\
				& (U8)((~(mask))|(val));\
		RTV_REG_SET(reg, tmp);		\
	} while (0)

	#define RTV_TSP_XFER_SIZE	188
#else
	#error "Must define the interface definition !"
#endif


/*############################################################################
#
# Pre-definintion by RAONTECH.
#
############################################################################*/
#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	#ifndef RTV_FIC_POLLING_MODE
		#define RTV_FIC_SPI_INTR_ENABLED /* FIC SPI Interrupt use. */
	#endif
	#ifdef FEATURE_TDMB_MULTI_CHANNEL_ENABLE
	  #define RTV_MSC_CIFMODE_ENABLED
	  #define RTV_FIC_CIFMODE_ENABLED
	#endif
#else
	#if !defined(RTV_FIC_POLLING_MODE)\
	&& !defined(RTV_FIC__SCAN_TSIF__PLAY_NA)\
	&& !defined(RTV_FIC__SCAN_TSIF__PLAY_TSIF)
		#define RTV_FIC_I2C_INTR_ENABLED /* FIC I2C Interrupt use. */
	#endif
#endif

/* Defines the FIC recevied in play state or not. */
#if defined(RTV_SPI_FIC_DECODE_IN_PLAY)\
|| defined(RTV_FIC__SCAN_I2C__PLAY_TSIF)\
|| defined(RTV_FIC__SCAN_TSIF__PLAY_TSIF)
	#define RTV_FIC_CIFMODE_ENABLED
#endif

#if (RTV_MAX_NUM_USE_SUBCHANNEL >= 2)
	#define RTV_MSC_CIFMODE_ENABLED
#endif

#if defined(RTV_MSC_CIFMODE_ENABLED) || defined(RTV_FIC_CIFMODE_ENABLED)
	#define RTV_CIF_MODE_ENABLED /* CIF decoder enabled */
#endif


/*############################################################################
#
# Defines the critical object and macros.
#
############################################################################*/
#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)\
|| defined(RTV_FIC_I2C_INTR_ENABLED)
    #if defined(__KERNEL__)
	extern struct mutex raontv_guard;
	#define RTV_GUARD_INIT		mutex_init(&raontv_guard)
	#define RTV_GUARD_LOCK		mutex_lock(&raontv_guard)
	#define RTV_GUARD_FREE		mutex_unlock(&raontv_guard)
	#define RTV_GUARD_DEINIT	((void)0)

    #elif defined(WINCE) || defined(WINDOWS) || defined(WIN32)
	extern CRITICAL_SECTION		raontv_guard;
	#define RTV_GUARD_INIT		InitializeCriticalSection(&raontv_guard)
	#define RTV_GUARD_LOCK		EnterCriticalSection(&raontv_guard)
	#define RTV_GUARD_FREE		LeaveCriticalSection(&raontv_guard)
	#define RTV_GUARD_DEINIT	DeleteCriticalSection(&raontv_guard)
    #else
	/* temp: TODO */
	#define RTV_GUARD_INIT		((void)0)
	#define RTV_GUARD_LOCK		((void)0)
	#define RTV_GUARD_FREE		((void)0)
	#define RTV_GUARD_DEINIT	((void)0)
    #endif
#else
	#define RTV_GUARD_INIT		((void)0)
	#define RTV_GUARD_LOCK		((void)0)
	#define RTV_GUARD_FREE		((void)0)
	#define RTV_GUARD_DEINIT	((void)0)
#endif


/*############################################################################
#
# Check erros by user-configurations.
#
############################################################################*/
#if !defined(RTV_CHIP_PKG_CSP) && !defined(RTV_CHIP_PKG_QFN)
	#error "Must define the package type !"
#endif

#if defined(RTV_IF_TSIF) || defined(RTV_IF_SPI_SLAVE)\
|| defined(RTV_IF_SPI)
    #if (RTV_CHIP_ADDR >= 0xFF)
	#error "Invalid chip address"
    #endif

#elif defined(RTV_IF_EBI2)

#else
	#error "Must define the interface definition !"
#endif


#ifndef RTV_MAX_NUM_USE_SUBCHANNEL
	#error "Should be define number of subchannel!"
#endif

#if (RTV_MAX_NUM_USE_SUBCHANNEL < 0) || (RTV_MAX_NUM_USE_SUBCHANNEL > 4)
	#error "Must 0 or 4 for subchannel. TDMB(1ea), DAB(2ea), DABP(1ea)"
#endif


#if defined(RTV_IF_TSIF) || defined(RTV_IF_SPI_SLAVE)
	#if !defined(RTV_FIC__SCAN_I2C__PLAY_NA)\
	&& !defined(RTV_FIC__SCAN_I2C__PLAY_I2C)\
	&& !defined(RTV_FIC__SCAN_I2C__PLAY_TSIF)\
	&& !defined(RTV_FIC__SCAN_TSIF__PLAY_NA)\
	&& !defined(RTV_FIC__SCAN_TSIF__PLAY_I2C)\
	&& !defined(RTV_FIC__SCAN_TSIF__PLAY_TSIF)
		#error "No FIC path was defined for TSIF!"
	#endif

	#if defined(RTV_FIC__SCAN_I2C__PLAY_NA)\
		&& defined(RTV_FIC__SCAN_I2C__PLAY_I2C)\
		&& defined(RTV_FIC__SCAN_I2C__PLAY_TSIF)\
		&& defined(RTV_FIC__SCAN_TSIF__PLAY_NA)\
		&& defined(RTV_FIC__SCAN_TSIF__PLAY_I2C)\
		&& defined(RTV_FIC__SCAN_TSIF__PLAY_TSIF)
		#error "Should select only one FIC path for TSIF!"
	#endif
#endif


#ifndef RTV_TSP_XFER_SIZE
	#error "Must define the RTV_TSP_XFER_SIZE definition !"
#endif

#if (RTV_TSP_XFER_SIZE != 188) && (RTV_TSP_XFER_SIZE != 204)
	#error "Must 188 or 204 for TS size"
#endif


#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)
	#ifdef RTV_CIF_MODE_ENABLED
		#ifdef RTV_SPI_CIF_MODE_INTERRUPT_SIZE
			#if ((RTV_SPI_CIF_MODE_INTERRUPT_SIZE % 188) != 0)
				#error "Must multiple of 188"
			#endif
		#else
			#error "Should defined"
		#endif
	#endif

	#if defined(RTV_FIC_POLLING_MODE) && defined(RTV_SPI_FIC_DECODE_IN_PLAY)
		#error "Not support!"
	#endif
#endif

#if defined(RTV_IF_TSIF) || defined(RTV_IF_SPI_SLAVE)
#endif

void rtvOEM_PowerOn(int on);

#ifdef __cplusplus
}
#endif

#endif /* __MTV319_PORT_H__ */

