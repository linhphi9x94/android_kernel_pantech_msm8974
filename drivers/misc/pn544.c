/*
 * Copyright (C) 2010 Trusted Logic S.A.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/pn544.h>

#define FEATURE_PANTECH_NFC
#define NXP_KR_PENDING_READ

#ifdef FEATURE_PANTECH_NFC
//#define FEATURE_PN544_KERNEL_LOG
#define FEATURE_PN544_IRQ_IN_SUSPEND
#define FEATURE_PN544_USE_DTREE
#define FEATURE_PN544_USE_PMIC_CLK
#define FEATURE_PN544_USE_SHUTDOWN

//#define FEATURE_PN544_VEN_PMIC_GPIO
#define FEATURE_PN544_FW_DL_PMIC_GPIO
#endif

#ifdef FEATURE_PN544_IRQ_IN_SUSPEND
#include <linux/wakelock.h>
#endif
#ifdef FEATURE_PN544_USE_DTREE
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
#ifdef FEATURE_PN544_USE_PMIC_CLK
#include <linux/clk.h>
#endif

#define MAX_BUFFER_SIZE	512

#ifdef FEATURE_PN544_VEN_PMIC_GPIO
#define VEN_SET_VALUE(gpio, val)    gpio_set_value_cansleep(gpio, val)
#define VEN_GET_VALUE(gpio)         gpio_get_value_cansleep(gpio)
#else
#define VEN_SET_VALUE(gpio, val)    gpio_set_value(gpio, val)
#define VEN_GET_VALUE(gpio)         gpio_get_value(gpio)
#endif

#ifdef FEATURE_PN544_FW_DL_PMIC_GPIO
#define FW_DL_SET_VALUE(gpio, val)  gpio_set_value_cansleep(gpio, val)
#define FW_DL_GET_VALUE(gpio)       gpio_get_value_cansleep(gpio)
#else
#define FW_DL_SET_VALUE(gpio, val)  gpio_set_value(gpio, val)
#define FW_DL_GET_VALUE(gpio)       gpio_get_value(gpio)
#endif



struct pn544_dev	{
	wait_queue_head_t	read_wq;
	struct mutex		read_mutex;
	struct i2c_client	*client;
	struct miscdevice	pn544_device;
	unsigned int 		ven_gpio;
	unsigned int 		firm_gpio;
	unsigned int		irq_gpio;
	bool			    irq_enabled;
	spinlock_t		    irq_enabled_lock;
#if defined(FEATURE_PN544_IRQ_IN_SUSPEND)	
	struct wake_lock    pn544_wake_lock;
#endif
};

#ifdef FEATURE_PN544_USE_PMIC_CLK
static struct clk *xo_handle_a2;
#endif

#ifdef NXP_KR_PENDING_READ
static bool irq_nfc = false;
static bool release_pending = false;
#endif

static void pn544_disable_irq(struct pn544_dev *pn544_dev)
{
	unsigned long flags;

	spin_lock_irqsave(&pn544_dev->irq_enabled_lock, flags);
	if (pn544_dev->irq_enabled) {
		disable_irq_nosync(pn544_dev->client->irq);
		pn544_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&pn544_dev->irq_enabled_lock, flags);
}

static irqreturn_t pn544_dev_irq_handler(int irq, void *dev_id)
{
	struct pn544_dev *pn544_dev = dev_id;

	pn544_disable_irq(pn544_dev);
#ifdef NXP_KR_PENDING_READ
	irq_nfc = true;
#endif
#if 0 //def FEATURE_PN544_KERNEL_LOG
     pr_info("==> %s (ven, firm, irq)=(%d, %d, %d) \n", __func__, VEN_GET_VALUE(pn544_dev->ven_gpio), FW_DL_GET_VALUE(pn544_dev->firm_gpio), gpio_get_value(pn544_dev->irq_gpio));
#endif

	/* Wake up waiting readers */
	wake_up(&pn544_dev->read_wq);

	return IRQ_HANDLED;
}

static ssize_t pn544_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn544_dev *pn544_dev = filp->private_data;
	char tmp[MAX_BUFFER_SIZE];
	int ret;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	pr_debug("%s : reading %zu bytes.\n", __func__, count);

	mutex_lock(&pn544_dev->read_mutex);

	if (!gpio_get_value(pn544_dev->irq_gpio)) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto fail;
		}

		pn544_dev->irq_enabled = true;
#ifdef NXP_KR_PENDING_READ
		irq_nfc = false;
#endif
		enable_irq(pn544_dev->client->irq);
#ifdef NXP_KR_PENDING_READ
		ret = wait_event_interruptible(pn544_dev->read_wq, irq_nfc);
        if(ret != 0)
        {
            pr_info("wait_event_interruptible return : %d\n", ret);
        }
#else
		ret = wait_event_interruptible(pn544_dev->read_wq,
				gpio_get_value(pn544_dev->irq_gpio));
#endif
		pn544_disable_irq(pn544_dev);

#ifdef NXP_KR_PENDING_READ
        if(release_pending == true)
        {
            release_pending = false;
            ret = -1;
            goto fail;
        }
#endif
		if (ret)
			goto fail;

	}

#ifdef FEATURE_PN544_KERNEL_LOG
    pr_info("==> %s #2 (ven, firm, irq)=(%d, %d, %d)\n", __func__, VEN_GET_VALUE(pn544_dev->ven_gpio), FW_DL_GET_VALUE(pn544_dev->firm_gpio), gpio_get_value(pn544_dev->irq_gpio));
#endif
	/* Read data */
	ret = i2c_master_recv(pn544_dev->client, tmp, count);
	mutex_unlock(&pn544_dev->read_mutex);

	if (ret < 0) {
		pr_err("%s: i2c_master_recv returned %d\n", __func__, ret);
		return ret;
	}
	if (ret > count) {
		pr_err("%s: received too many bytes from i2c (%d)\n", __func__, ret);
		return -EIO;
	}
	if (copy_to_user(buf, tmp, ret)) {
		pr_warning("%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}
	return ret;

fail:
	mutex_unlock(&pn544_dev->read_mutex);
	return ret;
}

static ssize_t pn544_dev_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *offset)
{
	struct pn544_dev  *pn544_dev;
	char tmp[MAX_BUFFER_SIZE];
	int ret;

	pn544_dev = filp->private_data;

	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	if (copy_from_user(tmp, buf, count)) {
		pr_err("%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

#ifdef FEATURE_PN544_KERNEL_LOG
    pr_info("==> %s (ven, firm, irq)=(%d, %d, %d)\n", __func__, VEN_GET_VALUE(pn544_dev->ven_gpio), FW_DL_GET_VALUE(pn544_dev->firm_gpio), gpio_get_value(pn544_dev->irq_gpio));
#endif
	pr_debug("%s : writing %zu bytes.\n", __func__, count);
	/* Write data */
	ret = i2c_master_send(pn544_dev->client, tmp, count);
	if (ret != count) {
		pr_err("%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}

	return ret;
}

static int pn544_dev_open(struct inode *inode, struct file *filp)
{
	struct pn544_dev *pn544_dev = container_of(filp->private_data,
						struct pn544_dev,
						pn544_device);

	filp->private_data = pn544_dev;
	pr_debug("%s : %d,%d\n", __func__, imajor(inode), iminor(inode));
	return 0;
}

static long pn544_dev_ioctl( struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pn544_dev *pn544_dev = filp->private_data;

	switch (cmd) {
	case PN544_SET_PWR:
		if (arg == 2) {
			/* power on with firmware download (requires hw reset)
			 */
			pr_info("%s power on with firmware\n", __func__);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 1);
			FW_DL_SET_VALUE(pn544_dev->firm_gpio, 1);
			msleep(20);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 0);
			msleep(60);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 1);
			msleep(20);
		} else if (arg == 1) {
			/* power on */
			pr_info("%s power on\n", __func__);
			FW_DL_SET_VALUE(pn544_dev->firm_gpio, 0);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 1);
			msleep(20);
		} else  if (arg == 0) {
			/* power off */
			pr_info("%s power off\n", __func__);
			FW_DL_SET_VALUE(pn544_dev->firm_gpio, 0);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 0);
			msleep(60);
        }
#ifdef NXP_KR_PENDING_READ
		else if (arg == 3) {
			pr_info("%s release pending read\n", __func__);
            release_pending = true;
            irq_nfc = true;
        	wake_up(&pn544_dev->read_wq);
		} 
#endif
#ifdef FEATURE_PN544_USE_SHUTDOWN
        else if( arg == 0XFF) {
			pr_info("%s power down\n", __func__);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 0);
			FW_DL_SET_VALUE(pn544_dev->firm_gpio, 0);
			
			msleep(60);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 1);
			msleep(20);
			VEN_SET_VALUE(pn544_dev->ven_gpio, 0);
			msleep(20);
        }
#endif
        else {
			pr_err("%s bad arg %lu\n", __func__, arg);
			return -EINVAL;
		}
		break;
	default:
		pr_err("%s bad ioctl %u\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}


static const struct file_operations pn544_dev_fops = {
	.owner	= THIS_MODULE,
	.llseek	= no_llseek,
	.read	= pn544_dev_read,
	.write	= pn544_dev_write,
	.open	= pn544_dev_open,
	.unlocked_ioctl  = pn544_dev_ioctl,
};

static int pn544_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	struct pn544_dev *pn544_dev;

#ifdef FEATURE_PN544_USE_PMIC_CLK
	int rc;
#endif

#ifdef FEATURE_PN544_USE_DTREE
	struct device_node *np;

	np = client->dev.of_node;
	if(np == NULL){
		pr_err("%s : pn544_probe np\n", __func__);
		return  -ENODEV;
	}
#else
	struct pn544_i2c_platform_data *platform_data;
	platform_data = client->dev.platform_data;

	if (platform_data == NULL) {
		pr_err("%s : nfc probe fail\n", __func__);
		return  -ENODEV;
	}
#endif

#ifdef FEATURE_PN544_KERNEL_LOG
	pr_info("+-----------------------------------------+\n");
	pr_info("|     NXP pn544 Driver Probe!             |\n");
	pr_info("+-----------------------------------------+\n");
#endif
	pn544_dev = kzalloc(sizeof(*pn544_dev), GFP_KERNEL);
	if (pn544_dev == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s : need I2C_FUNC_I2C\n", __func__);
		return  -ENODEV;
	}

#ifdef FEATURE_PN544_USE_DTREE
	pn544_dev->irq_gpio = (unsigned int)of_get_named_gpio_flags(np,"nxp,irq-gpio",0,NULL);
	pn544_dev->ven_gpio  = (unsigned int)of_get_named_gpio_flags(np,"nxp,ven-gpio",0,NULL);
	pn544_dev->firm_gpio  = (unsigned int)of_get_named_gpio_flags(np,"nxp,firm-gpio",0, NULL);
#else
	pn544_dev->irq_gpio = platform_data->irq_gpio;
	pn544_dev->ven_gpio  = platform_data->ven_gpio;
	pn544_dev->firm_gpio  = platform_data->firm_gpio;
#endif
	pn544_dev->client   = client;

    pr_info("%s (ven, firm, irq)=(%d, %d, %d)\n", __func__, pn544_dev->ven_gpio, pn544_dev->firm_gpio, pn544_dev->irq_gpio);

	ret = gpio_request(pn544_dev->irq_gpio, "nfc_int");
	if (ret)
	{	
		pr_err("%s : error : gpio_request nfc_int\n", __func__);
		return  -ENODEV;
	}

    ret = gpio_direction_input(pn544_dev->irq_gpio);
	if (ret)
	{
		pr_err("%s : error : gpio_direction_input irq_gpio\n", __func__);
		goto err_ven;
	}

	ret = gpio_request(pn544_dev->ven_gpio, "nfc_ven");
	if (ret)
	{
		pr_err("%s : error : gpio_request nfc_ven\n", __func__);
		goto err_ven;
	}

#if !defined(FEATURE_PN544_VEN_PMIC_GPIO)
    ret = gpio_direction_output(pn544_dev->ven_gpio, 0);
    if (ret )
    {   
		pr_err("%s : error : gpio_direction_output ven_gpio\n", __func__);
		goto err_firm;
    }
#endif
    
	ret = gpio_request(pn544_dev->firm_gpio, "nfc_firm");
	if (ret)
	{
		pr_err("%s : error : gpio_request nfc_firm\n", __func__);
		goto err_firm;
	}
#if !defined(FEATURE_PN544_FW_DL_PMIC_GPIO)
    ret = gpio_direction_output(pn544_dev->firm_gpio, 0);	
    if (ret )
    {	
		pr_err("%s : error : gpio_direction_output firm_gpio\n", __func__);
		goto err_firm_direction;
    }
#endif

	/* init mutex and queues */
	init_waitqueue_head(&pn544_dev->read_wq);
	mutex_init(&pn544_dev->read_mutex);
	spin_lock_init(&pn544_dev->irq_enabled_lock);
#if defined(FEATURE_PN544_IRQ_IN_SUSPEND)
	wake_lock_init(&pn544_dev->pn544_wake_lock, WAKE_LOCK_SUSPEND, "pn544");
#endif
	pn544_dev->pn544_device.minor = MISC_DYNAMIC_MINOR;
	pn544_dev->pn544_device.name = "pn544";
	pn544_dev->pn544_device.fops = &pn544_dev_fops;

	ret = misc_register(&pn544_dev->pn544_device);
	if (ret) {
		pr_err("%s : misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	pr_info("%s : requesting IRQ %d\n", __func__, client->irq);
	pn544_dev->irq_enabled = true;
	ret = request_irq(client->irq, pn544_dev_irq_handler,
			  IRQF_TRIGGER_HIGH, client->name, pn544_dev);
	if (ret) {
		dev_err(&client->dev, "request_irq failed\n");
		goto err_request_irq_failed;
	}
	pn544_disable_irq(pn544_dev);
	i2c_set_clientdata(client, pn544_dev);
#ifdef FEATURE_PN544_USE_PMIC_CLK
#ifdef FEATURE_PN544_KERNEL_LOG
	pr_info("[%s] device name [%s]\n", __func__,np->name);
	pr_info("[%s] device full name [%s]\n", __func__,np->full_name);
#endif
#ifdef FEATURE_PN544_USE_DTREE
	xo_handle_a2 = clk_get_sys(np->name, "xo");
#else
    xo_handle_a2 = clk_get_sys("pn544", "xo");
#endif
	if (IS_ERR(xo_handle_a2))
	{
		dev_err(&client->dev,"[%s] clk_get err \n", __func__);
	}
	rc = clk_prepare_enable(xo_handle_a2);
    if (rc)
    {
    	dev_err(&client->dev,"[%s] clk_prepare_enable fail rc[%d]\n", __func__, rc);
    }
#endif /* FEATURE_PN544_USE_PMIC_CLK */
	return 0;

err_request_irq_failed:
	misc_deregister(&pn544_dev->pn544_device);
err_misc_register:
	mutex_destroy(&pn544_dev->read_mutex);
#if defined(FEATURE_PN544_IRQ_IN_SUSPEND)
	wake_lock_destroy(&pn544_dev->pn544_wake_lock);
#endif
#if !defined(FEATURE_PN544_FW_DL_PMIC_GPIO)
err_firm_direction:
#endif
	gpio_free(pn544_dev->firm_gpio);
err_firm:
	gpio_free(pn544_dev->ven_gpio);
err_ven:
	gpio_free(pn544_dev->irq_gpio);
	kfree(pn544_dev);
err_exit:
	return ret;
}

static int pn544_remove(struct i2c_client *client)
{
	struct pn544_dev *pn544_dev;

	pn544_dev = i2c_get_clientdata(client);
	free_irq(client->irq, pn544_dev);
	misc_deregister(&pn544_dev->pn544_device);
	mutex_destroy(&pn544_dev->read_mutex);
#if defined(FEATURE_PN544_IRQ_IN_SUSPEND)
	wake_lock_destroy(&pn544_dev->pn544_wake_lock);
#endif
#ifdef FEATURE_PN544_USE_PMIC_CLK
	clk_disable_unprepare(xo_handle_a2);
#endif
	gpio_free(pn544_dev->irq_gpio);
	gpio_free(pn544_dev->ven_gpio);
	gpio_free(pn544_dev->firm_gpio);
	kfree(pn544_dev);

	return 0;
}
#if defined(FEATURE_PN544_IRQ_IN_SUSPEND)

static int pn544_suspend(struct device *dev)
{   
	struct pn544_dev *pn544_dev = dev_get_drvdata(dev);
#ifdef FEATURE_PN544_KERNEL_LOG
	pr_info("pn544_suspend*********** client IRQ[%d]\n", pn544_dev->client->irq);
	pr_info("pn544_suspend*********** gpio_to_irq IRQ[%d]\n", gpio_to_irq(pn544_dev->irq_gpio));
#endif

    if(VEN_GET_VALUE(pn544_dev->ven_gpio) == 1)
    {
        irq_set_irq_wake(pn544_dev->client->irq, 1);
    }
	return 0;
}

static int pn544_resume(struct device *dev)
{    
	struct pn544_dev *pn544_dev = dev_get_drvdata(dev);
#ifdef FEATURE_PN544_KERNEL_LOG
	pr_info("<************pn544_resume client IRQ[%d]\n", pn544_dev->client->irq);
	pr_info("<************pn544_resume gpio_to_irq IRQ[%d]\n", gpio_to_irq(pn544_dev->irq_gpio));
	pr_info("++++++pn544_resume()______pn544_resume irq_nfc [%d]\n", irq_nfc);   
#endif
	
    if(VEN_GET_VALUE(pn544_dev->ven_gpio) == 1)
 	{
	 	if(gpio_get_value(pn544_dev->irq_gpio) == 1)
	 	{
            wake_lock_timeout(&pn544_dev->pn544_wake_lock, 100);
	 	}
        irq_set_irq_wake(pn544_dev->client->irq, 0);
 	}
	return 0;
}
static struct dev_pm_ops pn544_pm_ops = 
{ 
	.suspend = pn544_suspend, 
	.resume = pn544_resume,
};
#endif

#if defined(FEATURE_PN544_USE_DTREE)
MODULE_DEVICE_TABLE(of, pn544_match_table);

static struct of_device_id pn544_match_table[] = {
	{ .compatible = "nxp,pn544",},
	{ },
};
#endif

static const struct i2c_device_id pn544_id[] = {
	{ "pn544", 0 },
	{ }
};

static struct i2c_driver pn544_driver = {
	.id_table	= pn544_id,
	.probe		= pn544_probe,
	.remove		= pn544_remove,
	.driver 	= {
		.owner	= THIS_MODULE,
		.name	= "pn544",
#if defined(FEATURE_PN544_USE_DTREE)
		.of_match_table = of_match_ptr(pn544_match_table),
#endif
#if defined(FEATURE_PN544_IRQ_IN_SUSPEND)
		.pm = &pn544_pm_ops,
#endif
	},
};

/*
 * module load/unload record keeping
 */

static int __init pn544_dev_init(void)
{
	pr_info("Loading pn544 driver\n");
	return i2c_add_driver(&pn544_driver);
}
module_init(pn544_dev_init);

static void __exit pn544_dev_exit(void)
{
	pr_info("Unloading pn544 driver\n");
	i2c_del_driver(&pn544_driver);
}
module_exit(pn544_dev_exit);

MODULE_AUTHOR("Sylvain Fonteneau");
MODULE_DESCRIPTION("NFC PN544 driver");
MODULE_LICENSE("GPL");
