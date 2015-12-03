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
* TITLE		: MTV319 porting source file.
*
* FILENAME	: mtv319_port.c
*
* DESCRIPTION	: User-supplied Routines for RAONTECH TV Services.
*
******************************************************************************/
/******************************************************************************
* REVISION HISTORY
*
*    DATE         NAME          REMARKS
* ----------  -------------    ------------------------------------------------
* 07/12/2012  Ko, Kevin        Created.
******************************************************************************/

#include "mtv319.h"
#include "mtv319_internal.h"
//#include "tdmb.h"
//#include "tdmb_gpio.h"

#ifdef RTV_IF_SPI
#include "../../dmb_interface.h"
#include <linux/spi/spi.h>
#endif

/* Declares a variable of gurad object if neccessry. */
#if defined(RTV_IF_SPI) || defined(RTV_IF_EBI2)\
|| defined(RTV_FIC_I2C_INTR_ENABLED)
	#if defined(__KERNEL__)
	struct mutex raontv_guard;
	#elif defined(WINCE) || defined(WINDOWS) || defined(WIN32)
	CRITICAL_SECTION raontv_guard;
    #else
	/* non-OS and RTOS. */
	#endif
#endif

#ifdef RTV_IF_SPI
struct spi_device *mtv_spi=NULL;

int mtv319_spi_init(void)
{
  mtv_spi = dmb_spi_setup();
  
  RTV_DBGMSG1("[%s] SPI mode[%d] speed[%d]\n", __func__, mtv_spi->mode, mtv_spi->max_speed_hz);
  return 1;
}

unsigned char tdmb_spi_read(unsigned char page, unsigned char reg)
{
	int ret;
	u8 out_buf[4], in_buf[4];
	struct spi_message msg;
	struct spi_transfer msg_xfer = {
		.tx_buf = out_buf,
		.rx_buf = in_buf,
		.len = 4,
		.cs_change = 0,
		.delay_usecs = 0
	};

	spi_message_init(&msg);
	out_buf[0] = 0x90 | page;
	out_buf[1] = reg;
	out_buf[2] = 1; /* Read size */

	spi_message_add_tail(&msg_xfer, &msg);

	ret = spi_sync(mtv_spi, &msg);
	if (ret) {
		RTV_DBGMSG1("MTV319 SPI Read error: %d\n", ret);
		return 0xFF;
	}

#if 0
	RTV_DBGMSG1("0x%02X 0x%02X 0x%02X 0x%02X\n",
			in_buf[0], in_buf[1], in_buf[2], in_buf[3]);
#endif

	return in_buf[MTV319_SPI_CMD_SIZE];
}

void tdmb_spi_read_burst(unsigned char page, unsigned char reg,
			unsigned char *buf, int size)
{
	int ret;
	u8 out_buf[MTV319_SPI_CMD_SIZE];
	struct spi_message msg;
	struct spi_transfer xfer0 = {
		.tx_buf = out_buf,
		.rx_buf = buf,
		.len = MTV319_SPI_CMD_SIZE,
		.cs_change = 0,
		.delay_usecs = 0,
	};

	struct spi_transfer xfer1 = {
		.tx_buf = buf,
		.rx_buf = buf,
		.len = size,
		.cs_change = 0,
		.delay_usecs = 0,
	};

	out_buf[0] = 0xA0; /* Memory read */
	out_buf[1] = 0x00;
	out_buf[2] = 188; /* Fix */

	spi_message_init(&msg);
	spi_message_add_tail(&xfer0, &msg);
	spi_message_add_tail(&xfer1, &msg);

	ret = spi_sync(mtv_spi, &msg);
	if (ret) {
		RTV_DBGMSG1("MTV319 SPI Read Burst 0 error: %d\n", ret);
		return;
	}

}

void tdmb_spi_write(unsigned char page, unsigned char reg, unsigned char val)
{
	u8 out_buf[4];
	u8 in_buf[4];
	struct spi_message msg;
	struct spi_transfer msg_xfer = {
		.tx_buf = out_buf,
		.rx_buf = in_buf,
		.len = 4,
		.cs_change = 0,
		.delay_usecs = 0
	};
	int ret;

	spi_message_init(&msg);
	out_buf[0] = 0x80 | page;
	out_buf[1] = reg;
	out_buf[2] = 1; /* size */
	out_buf[3] = val;
	spi_message_add_tail(&msg_xfer, &msg);

	ret = spi_sync(mtv_spi, &msg);
	if (ret)
		RTV_DBGMSG1("MTV319 SPI Write error: %d\n", ret);
}

void tdmb_spi_recover(unsigned int size)
{
	static unsigned char temp_buf[MTV319_SPI_CMD_SIZE + (32 * 188)];
	int ret;
	struct spi_message msg;
	struct spi_transfer msg_xfer = {
		.tx_buf = temp_buf,
		.rx_buf = temp_buf,
		.len = size,
		.cs_change = 0,
		.delay_usecs = 0,
	};

	memset(temp_buf, 0xFF, size);

	spi_message_init(&msg);
	spi_message_add_tail(&msg_xfer, &msg);

	ret = spi_sync(mtv_spi, &msg);
	if (ret)
		RTV_DBGMSG1("error: %d\n", ret);
}
#endif

void rtvOEM_PowerOn(int on)
{
	if (on) {
		/* Set the GPIO of MTV_EN pin to high. */
		//gpio_set_value(MTV_PWR_EN, 1);

		/* In case of SPI interface or FIC interrupt mode for T-DMB,
		we should lock the register page. */
		RTV_GUARD_INIT;
	} else {
		/* Set the GPIO of MTV_EN pin to low. */
		//gpio_set_value(MTV_PWR_EN, 0);

		RTV_GUARD_DEINIT;
	}
}




