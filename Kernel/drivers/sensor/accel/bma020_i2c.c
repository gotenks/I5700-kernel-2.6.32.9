#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

//#include <asm/hardware.h>
//#include <asm/arch/gpio.h>
//add by inter.park
#include <mach/hardware.h>
#include <linux/gpio.h>

#include <linux/irq.h>
#include <linux/i2c.h>

#include "bma020_i2c.h"





#define	ACC_SENSOR_ADDRESS		0x38

#define I2C_M_WR			0x00
#define I2C_DF_NOTIFY			0x01





static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { I2C_CLIENT_END };
static unsigned short probe_addr[] = { 0, ACC_SENSOR_ADDRESS, I2C_CLIENT_END };


static struct i2c_client_address_data bma020_addr_data = {
	.normal_i2c		= normal_addr,
	.probe			= probe_addr,
	.ignore			= ignore,
};





static int bma020_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id);
static int __devexit bma020_i2c_remove(struct i2c_client *client);

static int bma020_probe(struct platform_device *);
static int bma020_remove(struct platform_device *);
static int bma020_suspend(pm_message_t mesg);


static const struct i2c_device_id bma020_id[] = {
	{ "bma020_i2c", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, bma020_id);

static struct i2c_driver acc_bma020_i2c_driver = {
    .driver = {
        .name   = "bma020",
        .owner  = THIS_MODULE,
    },
    .id_table     = bma020_id,
    .probe        = bma020_i2c_probe,
    .remove       = __devexit_p(bma020_i2c_remove),
    .address_data = &bma020_addr_data
};

static struct i2c_client* bma020_i2c_client;

static int bma020_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	if ( !i2c_check_functionality(client->adapter,I2C_FUNC_SMBUS_BYTE_DATA) ) {
		printk(KERN_INFO "byte op is not permited.\n");
		printk("i2c_acc_bma020_probe_client() ERROR0\n");
		return -ENODEV;
	}

	client->flags = I2C_DF_NOTIFY | I2C_M_IGNORE_NAK;

	bma020_i2c_client = client;
	i2c_set_clientdata(client, bma020_i2c_client);
	return 0;
}

static int __devexit bma020_i2c_remove(struct i2c_client *client) {
	bma020_i2c_client = i2c_get_clientdata(client);
	kfree(bma020_i2c_client);
	return 0;
}




int i2c_acc_bma020_init(void)
{
	int ret;

	if ( (ret = i2c_add_driver(&acc_bma020_i2c_driver)) ) 
	{
		printk("Driver registration failed, module not inserted.\n");
		return ret;
	}

	return 0;
}

void i2c_acc_bma020_exit(void)
{
	i2c_del_driver(&acc_bma020_i2c_driver); 
}


char i2c_acc_bma020_read(u8 reg, u8 *val, unsigned int len )
{
	int 	 err;
	struct 	 i2c_msg msg[1];
	unsigned char data[1];
	
	if( (bma020_i2c_client == NULL) || (!bma020_i2c_client->adapter) )
	{
		return -ENODEV;
	}
	
	msg->addr 	= bma020_i2c_client->addr;
	msg->flags 	= I2C_M_WR;
	msg->len 	= 1;
	msg->buf 	= data;
	*data       = reg;

	err = i2c_transfer(bma020_i2c_client->adapter, msg, 1);

	if (err >= 0) 
	{
		msg->flags = I2C_M_RD;
		msg->len   = len;
		msg->buf   = val;
		err = i2c_transfer(bma020_i2c_client->adapter, msg, 1);
	}

	if (err >= 0) 
	{
		return 0;
	}
	printk("%s %d i2c transfer error\n", __func__, __LINE__);/* add by inter.park */

	return err;

}

char i2c_acc_bma020_write( u8 reg, u8 *val )
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[2];

	if( (bma020_i2c_client == NULL) || (!bma020_i2c_client->adapter) ){
		return -ENODEV;
	}
	
	data[0] = reg;
	data[1] = *val;

	msg->addr = bma020_i2c_client->addr;
	msg->flags = I2C_M_WR;
	msg->len = 2;
	msg->buf = data;
	
	err = i2c_transfer(bma020_i2c_client->adapter, msg, 1);

	if (err >= 0) return 0;

	printk("%s %d i2c transfer error\n", __func__, __LINE__);/* add by inter.park */
	return err;
}
