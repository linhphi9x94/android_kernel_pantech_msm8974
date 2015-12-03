//=============================================================================
// File       : dmb_spi.c
//
// Description: 
//
// Revision History:
//
// Version         Date           Author        Description of Changes
//-----------------------------------------------------------------------------
//  1.0.0       2011/04/20       yschoi         Create
//  1.1.0       2011/09/29       yschoi         dmb_spi.c => dmb_spi.c
//=============================================================================

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#ifdef CONFIG_ARCH_MSM
#include <mach/board.h>
#endif
#include <mach/gpio.h>

#include "dmb_comdef.h"
#include "dmb_spi.h"
#include "dmb_hw.h"

#ifdef FEATURE_DMB_USE_DTS
#include <linux/of_gpio.h>
#endif

/*================================================================== */
/*==============        DMB SPI Driver Definition     =============== */
/*================================================================== */


struct spi_device *dmb_spi_dev = NULL;
 
#ifdef CONFIG_SKY_DMB_SPI_GPIO
#define DMB_SPI_MOSI 82
#define DMB_SPI_MISO 83
#define DMB_SPI_CS   84
#define DMB_SPI_CLK  85


#ifdef CONFIG_ARCH_MSM
static uint32 dmb_spi_gpio_table[] = {
  GPIO_CFG(DMB_SPI_CS, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
  GPIO_CFG(DMB_SPI_CLK, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
  GPIO_CFG(DMB_SPI_MOSI, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
  GPIO_CFG(DMB_SPI_MISO, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

#elif defined(CONFIG_ARCH_TEGRA)
static uint32 dmb_spi_gpio_table[][3] = { //{GPIO, IS_INPUT, VALUE}
  {DMB_SPI_CS, 0, 0},
  {DMB_SPI_CLK, 0, 0},
  {DMB_SPI_MOSI, 0, 0},
  {DMB_SPI_MISO, 1, 0},
};
#endif
#endif /* CONFIG_SKY_DMB_SPI_GPIO */



/*================================================================== */
/*==============        DMB SPI Driver Function      =============== */
/*================================================================== */

#ifdef CONFIG_SKY_DMB_SPI_GPIO
/*====================================================================
FUNCTION       dmb_spi_gpio_init  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void dmb_spi_gpio_init(void)
{
  int i;
#ifdef CONFIG_ARCH_MSM
  int rc;
#endif

  DMB_MSG_SPI("[%s] \n", __func__);
  
  for(i = 0; i < ARRAY_SIZE(dmb_spi_gpio_table); i ++)
  {
#ifdef CONFIG_ARCH_MSM
    rc = gpio_tlmm_config(dmb_spi_gpio_table[i], GPIO_CFG_ENABLE);
    if (rc)
    {
      DMB_MSG_SPI("[%s] error!!! index=%d, rc=%d\n",__func__, i  , rc);
      break;
    }
#elif defined(CONFIG_ARCH_TEGRA)
    tegra_gpio_enable(dmb_spi_gpio_table[i][0]);
    tegra_gpio_init_configure(dmb_spi_gpio_table[i][0],dmb_spi_gpio_table[i][1],dmb_spi_gpio_table[i][2]);
    //DMB_MSG_HW("[%s] dmb_gpio_init gpio[%d], is_input[%d], value[%d]\n", __func__, dmb_spi_gpio_table[i][0], dmb_spi_gpio_table[i][1], dmb_spi_gpio_table[i][2]);
#endif
  }

  DMB_MSG_SPI("[%s] end cnt[%d]!!!\n",__func__, i);
}
#endif /* CONFIG_SKY_DMB_SPI_GPIO */


/*====================================================================
FUNCTION       dmb_spi_write_then_read  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
int dmb_spi_write_then_read(u8 *txbuf, unsigned n_tx, u8 *rxbuf, unsigned n_rx)
{
  int ret;
  
  //DMB_MSG_SPI("[%s] \n", __func__);

  //DMB_MSG_SPI("[%s] dmb_spi_dev[0x%x]\n", __func__, (unsigned int) dmb_spi_dev);

#if 0//def CONFIG_SKY_TDMB_FCI_BB_FC8050
  //ret = fc8050_spi_write_then_read(dmb_spi_dev, txbuf, n_tx, rxbuf, n_rx);
  ret = fc8050_spi_write_then_read_burst(dmb_spi_dev, txbuf, n_tx, rxbuf, n_rx);
#else
  ret = spi_write_then_read(dmb_spi_dev, txbuf, n_tx, rxbuf, n_rx);
#endif

  if(ret)
  {
    DMB_MSG_SPI("[%s] fail : %d\n", __func__, ret);
    return 1;
  }

  return 0;
}

#ifdef FEATURE_TDMB_USE_FCI_FC8050
/*====================================================================
FUNCTION       dmb_spi_clear_int  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void dmb_spi_clear_int(void)
{
  //DMB_MSG_SPI("[%s] \n", __func__);

  FC8050_BUFFER_RESET(VIDEO_MASK);

  return;
}
#endif


#if 0
/*====================================================================
FUNCTION       find_spi_device  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
static struct spi_device* find_spi_device(int bus_num)
{
  struct spi_master *spi_master;
  struct spi_device *spi_device;
  struct device *pdev;
  char buff[64];

  spi_master = spi_busnum_to_master(bus_num);
  if (!spi_master) {
    DMB_MSG_SPI("[%s] spi_busnum_to_master(%d) returned NULL\n", __func__, bus_num);
    return NULL;
  }

  spi_device = spi_alloc_device(spi_master);
  if (!spi_device) {
    put_device(&spi_master->dev);
    DMB_MSG_SPI("[%s] spi_alloc_device() failed\n", __func__);
    return NULL;
  }

  /* specify a chip select line */
  spi_device->chip_select = 0;

  snprintf(buff, sizeof(buff), "%s.%u",
      dev_name(&spi_device->master->dev),
      spi_device->chip_select);

  pdev = bus_find_device_by_name(spi_device->dev.bus, NULL, buff);

  //if (pdev) {
  //  TCDBG("spi_device :0x%X\n", (unsigned int)spi_device);
  //}

  put_device(&spi_master->dev);
  return spi_device;
}
#endif


/*====================================================================
FUNCTION       dmb_spi_setup  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void* dmb_spi_setup(void)
{
  int rc;

  DMB_MSG_SPI("[%s] \n", __func__);

#ifdef CONFIG_SKY_DMB_SPI_GPIO
  dmb_spi_gpio_init();
#endif

  if(dmb_spi_dev == NULL)
  {
    DMB_MSG_SPI("[%s] dmb_spi_dev is NULL \n",__func__);
    return 0;
  }
  else
  {
    DMB_MSG_SPI("[%s] dmb_spi_dev[0x%x]\n", __func__, (unsigned int) dmb_spi_dev);
  }

#ifdef CONFIG_SKY_DMB_SPI_GPIO
#if defined(CONFIG_SKY_TDMB_FCI_BB)
  //dmb_spi_dev->max_speed_hz = 24000000; // 24M
  //dmb_spi_dev->max_speed_hz = 5400000; // 5.4M
  dmb_spi_dev->max_speed_hz = 10800000; // 10.8M
  dmb_spi_dev->mode = SPI_MODE_0;
#elif defined(CONFIG_SKY_TDMB_TCC_BB)
  dmb_spi_dev->max_speed_hz = 20000000;
  dmb_spi_dev->mode = SPI_MODE_2;
#elif defined(CONFIG_SKY_TDMB_INC_BB)
  dmb_spi_dev->max_speed_hz = 5400000;
  dmb_spi_dev->mode = SPI_MODE_0;
#else
  ##error
#endif
  //dmb_spi_dev->chip_select    = 0;
  //dmb_spi_dev->bits_per_word  = 8;
#endif

  rc = spi_setup(dmb_spi_dev);

  if (rc < 0)
  {
    DMB_MSG_SPI("[%s] rc2 [%d]\n", __func__,rc);
  }
  else
  {
    if(dmb_spi_dev == NULL)
    {
      DMB_MSG_SPI("[%s] dmb_spi_dev NULL \n",__func__);
      return 0;
    }
    DMB_MSG_SPI("[%s] spi_setup OK [%d]\n", __func__,rc);
  }

  return dmb_spi_dev;
}


/*====================================================================
FUNCTION       dmb_spi_probe  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
static int __devinit dmb_spi_probe(struct spi_device *spi)
{
#ifdef FEATURE_DMB_USE_DTS
    int irq, gpois[3], cs, cpol, cpha, cs_high;//intr, freq, 
    int i;
#ifdef FEATURE_DMB_PMIC_GPIO
    dmb_ext_dev *ext_dev = get_dmb_pmic_data();
    memset(ext_dev, 0xff, sizeof(dmb_ext_dev));
#endif
#endif
 
   DMB_MSG_SPI("[%s] \n", __func__);

#ifdef FEATURE_DMB_USE_DTS
    if((spi == NULL) || (spi->dev.of_node == NULL))
    {
      DMB_MSG_SPI("%s no device",__func__);
      return -ENODEV;
    }
  
    //Parse data using dt.
    irq = of_get_named_gpio_flags(spi->dev.of_node, "dmb,irq-gpio", 0, NULL);
    //intr = of_get_named_gpio_flags(spi->dev.of_node, "dmb,interrupts", 0, NULL);
    //freq= of_get_named_gpio_flags(spi->dev.of_node, "dmb,spi-max-frequency", 0, NULL);
    for(i=0;i<ARRAY_SIZE(gpois);i++)
    {
      gpois[i]= of_get_named_gpio_flags(spi->dev.of_node, "dmb,gpios", i, NULL);
    }
    cs= of_get_named_gpio_flags(spi->dev.of_node, "dmb,cs-gpios", 0, NULL);

#ifdef FEATURE_DMB_PMIC_GPIO
    for(i=0; i<DMB_PMIC_GPIO_NUM;i++)
    {
      ext_dev->pmic_gpio[i] = of_get_named_gpio_flags(spi->dev.of_node, "dmb,pmic_gpio", i, NULL);
    }
    DMB_MSG_SPI("DMB SPI CS[%d] MISO[%d] MOSI[%d] CLK[%d]  irq[%d] pmic_gpio1[%d]\n",cs,gpois[1],gpois[2],gpois[0],irq, ext_dev->pmic_gpio[0]);
#else
    DMB_MSG_SPI("DMB SPI CS[%d] MISO[%d] MOSI[%d] CLK[%d]  irq[%d]\n",cs,gpois[1],gpois[2],gpois[0],irq);
#endif
 
    cpha = ( spi->mode & SPI_CPHA ) ? 1:0;
    cpol = ( spi->mode & SPI_CPOL ) ? 1:0;
    cs_high = ( spi->mode & SPI_CS_HIGH ) ? 1:0;
    DMB_MSG_SPI("        mode[%d]  ph[%d] pol[%d] cs high[%d]\n", spi->mode,cpha,cpol,cs_high);
#endif

  dmb_spi_dev = spi;
  
  if(dmb_spi_dev == NULL)
  {
    DMB_MSG_SPI("[%s] SPI device is null\n", __func__);
    return -ENODEV;
  }
  else 
  {
    DMB_MSG_SPI("[%s] SPI  Max speed [%d] CS[%d]  Mode[%d] \n", __func__,
    dmb_spi_dev->max_speed_hz, dmb_spi_dev->chip_select, dmb_spi_dev->mode);
  }

  DMB_MSG_SPI("%s End !!",__func__);

  return 0;
}


/*====================================================================
FUNCTION       dmb_spi_remove  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
static int __devexit dmb_spi_remove (struct spi_device *spi)
{
  //int rc;

   DMB_MSG_SPI("[%s] \n", __func__);
   return 0;
}


/*====================================================================

======================================================================*/

#ifdef FEATURE_DMB_USE_DTS
#ifdef CONFIG_OF //Open firmware must be defined for dts useage
static struct of_device_id dmb_spi_table[] = {
{ .compatible = "dmb,dmb_spi",}, //Compatible node must match dts
{ },
};
#else
#define dmb_spi_table NULL
#endif
#endif

/* spi driver data */
static struct spi_driver dmb_spi_driver = {
  .probe = dmb_spi_probe,
  .remove = __devexit_p(dmb_spi_remove),
  .driver = {
    .name = "dmb_spi",//DMB_SPI_DEV_NAME,
    .bus = &spi_bus_type,
#ifdef FEATURE_DMB_USE_DTS
    .of_match_table = dmb_spi_table,
#endif
    .owner = THIS_MODULE,
  },
};


/*====================================================================
FUNCTION       dmb_spi_init  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void dmb_spi_init(void)
{
  int rc;

  DMB_MSG_SPI("[%s] \n", __func__);

#ifdef CONFIG_SKY_DMB_SPI_GPIO
  //dmb_spi_gpio_init();
#endif

  rc = spi_register_driver(&dmb_spi_driver);

  if(rc)
  {
    DMB_MSG_SPI("[%s] spi_probe fail !!!, rc [%d]", __func__,rc);
  }

#if 0
  dmb_spi_dev = find_spi_device(5);

  if(dmb_spi_dev == NULL)
  {
    DMB_MSG_SPI("[%s] SPI device is null\n", __func__);
    return;
  }
  else 
  {
    DMB_MSG_SPI("[%s] SPI  Max speed [%d] CS[%d]  Mode[%d] \n", __func__,
    dmb_spi_dev->max_speed_hz, dmb_spi_dev->chip_select, dmb_spi_dev->mode);
  }
#endif

  return;
}


/*====================================================================
FUNCTION       dmb_spi_exit  
DESCRIPTION 
DEPENDENCIES
RETURN VALUE
SIDE EFFECTS
======================================================================*/
void dmb_spi_exit(void)
{
  DMB_MSG_SPI("[%s] \n", __func__);
  spi_unregister_driver(&dmb_spi_driver);
}

