#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mod_devicetable.h>
#include <linux/power_supply.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <mach/gpiomux.h>

#define MAX17058_VCELL_MSB	0x02
#define MAX17058_VCELL_LSB	0x03
#define MAX17058_SOC_MSB	0x04
#define MAX17058_SOC_LSB	0x05
#define MAX17058_MODE_MSB	0x06
#define MAX17058_MODE_LSB	0x07
#define MAX17058_VER_MSB	0x08
#define MAX17058_VER_LSB	0x09
#define MAX17058_RCOMP_MSB	0x0C
#define MAX17058_RCOMP_LSB	0x0D

#define SKY_SOC_EMPTY	2550	// 2.550 X 1000
#define SKY_SOC_FULL	97700	// 97.70 X 1000
#define SKY_DEFAULT_RCOMP	0x5E
#define SKY_TEMP_CO_HOT		675 // -0.675
#define SKY_TEMP_CO_COLD	-5825 // -5.825

#define SKY_MULT_1000(x) 	(x*1000)

struct pt_max17058_chip {
	struct i2c_client *client;
	unsigned int batt_soc;
	unsigned int batt_vol;
};

static struct pt_max17058_chip *pt_fg;

static int max17058_write_reg(struct i2c_client *client, u8 addr, u16 val)
{
	int rc;
	struct i2c_msg msg[1];
	unsigned char data[3];

	if (!client->adapter)
		return -ENODEV;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 3;
	msg->buf = data;
	data[0] = addr;
	data[1] = (val>>8);
	data[2] = (val&0xFF);

	rc = i2c_transfer(client->adapter, msg, 1);
	if (rc >= 0)
		return 0;
	
	return rc;
}

static int max17058_read_reg(struct i2c_client *client, uint8_t addr,
	uint8_t *data, int length)
{
	int rc=0;

	struct i2c_msg msgs[] = {
		{
			.addr  = client->addr,
			.flags = 0,
			.len   = 1,
			.buf   = &addr,
		},
		{
			.addr  = client->addr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = data,
		},
	};

	rc = i2c_transfer(client->adapter, msgs, 2);	
	return rc;
}

void pt_max17058_set_rcomp(u8 msb, u8 lsb)
{	
	u16 data;
	
	data = (msb<<8)|lsb;
	max17058_write_reg(pt_fg->client, MAX17058_RCOMP_MSB, data);
}
EXPORT_SYMBOL(pt_max17058_set_rcomp);

int pt_max17058_get_vcell(int *batt_vol)
{
	int rc=0;
	u8 msb;
	u8 lsb;
	u8 buf[2] = {0,};
	int uvolt=0;
	int voltage=0;
	int retry=20;
	
	if(pt_fg->client == NULL)
		return -EIO;

	do {
		rc = max17058_read_reg(pt_fg->client, MAX17058_VCELL_MSB, buf, 2);
		if(rc == -EIO)	
			msleep(10);
	}while(rc < 0 && retry-- > 0);
	
	if(rc < 0) {
		pr_err("pt_max17058_get_vcell failed, rc=%d\n", rc);
		return rc;
	}
	
	msb = buf[0];
	lsb = buf[1];
	
	voltage=(msb<<4)|((lsb&0xf0)>>4);
	uvolt=(voltage*1250);

	*batt_vol = uvolt;
	
	return rc;
}
EXPORT_SYMBOL(pt_max17058_get_vcell);

int pt_max17058_get_soc(int *batt_soc)
{
	int rc=0;
	u8 msb;
	u8 lsb;
	u8 buf[2] = {0,};
	int retry=20;
	int adj_soc=0;
	int soc=0;
	
	if(pt_fg->client == NULL)
		return -EIO;

	do {
		rc = max17058_read_reg(pt_fg->client, MAX17058_SOC_MSB, buf, 2);
		if(rc == -EIO)
			msleep(10);
	}while(rc < 0 && retry-- > 0);

	if(rc < 0) {
		pr_err("pt_max17058_get_soc failed, rc=%d\n", rc);
		*batt_soc = 1; // if i2c failed, return 1%
		return rc;
	}
	
	msb = buf[0];
	lsb = buf[1];
	
	soc = SKY_MULT_1000(((msb * 256) +lsb)) / 512;
	adj_soc = (soc-SKY_SOC_EMPTY)*100 / (SKY_SOC_FULL-SKY_SOC_EMPTY);

	*batt_soc = adj_soc;
	pt_fg->batt_soc = adj_soc;
	
	return rc;
}
EXPORT_SYMBOL(pt_max17058_get_soc);

int max17058_get_soc_for_led(void)
{
	int adj_soc=0;

	adj_soc = pt_fg->batt_soc;
	
	if(adj_soc < 0)
		adj_soc = 0;

	if(adj_soc >= 100)
		adj_soc = 100;
	
	return adj_soc;
}
EXPORT_SYMBOL_GPL(max17058_get_soc_for_led);

int pt_max17058_calc_rcomp(int batt_temp)
{
	int newRcomp;

	if(batt_temp > 20) {
		newRcomp = SKY_DEFAULT_RCOMP - (((batt_temp-20) * SKY_TEMP_CO_HOT)/1000);
		}
	else if(batt_temp < 20) {
		newRcomp = SKY_DEFAULT_RCOMP + (((batt_temp-20) * SKY_TEMP_CO_COLD)/1000);
		}
	else
		newRcomp = SKY_DEFAULT_RCOMP;

	if(newRcomp > 255)
		newRcomp = 255;
	else if(newRcomp < 0)
		newRcomp = 0;

	return newRcomp;
}
EXPORT_SYMBOL(pt_max17058_calc_rcomp);

static int __devinit pt_max17058_battery_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct pt_max17058_chip *chip;
	
	printk("%s enter\n", __func__);

	chip = kzalloc(sizeof(struct pt_max17058_chip), GFP_KERNEL);
	if (!chip) {
		pr_err("Cannot allocate pt_max17058_chip\n");
		return -ENOMEM;
	}
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
	        pr_err("i2c_check_functionality failed\n");
	        kfree(chip);
	        return -EIO;
	}
	
	chip->client = client;
	i2c_set_clientdata(client, chip);
	
	pt_fg = chip;
	
	pt_max17058_set_rcomp(SKY_DEFAULT_RCOMP, 0x1F);

	return 0;
}

static int __devexit pt_max17058_battery_remove(struct i2c_client *client)
{
	struct pt_max17058_chip *chip = i2c_get_clientdata(client);

	kfree(chip);
	
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id pt_max17058_dt_match[] = {
	{ .compatible = "maxim,max17058-i2c" },
	{ },
};
#else
#define pt_max17058_dt_match NULL
#endif

static const struct i2c_device_id pt_max17058_id[] = {
	{ "max17058_i2c", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pt_max17058_id);

static struct i2c_driver pt_max17058_i2c_driver = {
	.driver	= {
		.name	= "max17058_i2c",
		.owner	= THIS_MODULE,
		.of_match_table = pt_max17058_dt_match,
	},
	.probe		= pt_max17058_battery_probe,
	.remove		= __devexit_p(pt_max17058_battery_remove),
	.id_table	= pt_max17058_id,
};

static int __init pt_max17058_init(void)
{
	int rc;

	rc = i2c_add_driver(&pt_max17058_i2c_driver);
	if(rc) {
		printk(KERN_ERR "max17058_battery driver add failed.\n");
	}
	
	return rc;
}

static void __exit pt_max17058_exit(void)
{
	i2c_del_driver(&pt_max17058_i2c_driver);
}

module_init(pt_max17058_init);
module_exit(pt_max17058_exit);

MODULE_DESCRIPTION("Pantech MAX17058 Fuel Gauge driver");
MODULE_LICENSE("GPL");

