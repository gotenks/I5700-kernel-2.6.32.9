static struct i2c_gpio_platform_data i2c2_platdata = {
	.sda_pin		= GPIO_FM_I2C_SDA,
	.scl_pin		= GPIO_FM_I2C_SCL,
	.udelay			= 2,	/* 250KHz */		
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device s3c_device_i2c2 = {
	.name				= "i2c-gpio",
	.id					= 2,
	.dev.platform_data	= &i2c2_platdata,
};

static struct i2c_gpio_platform_data i2c3_platdata = {
	.sda_pin		= GPIO_PWR_I2C_SDA,
	.scl_pin		= GPIO_PWR_I2C_SCL,
	.udelay			= 2,	/* 250KHz */		
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device s3c_device_i2c3 = {
	.name				= "i2c-gpio",
	.id					= 3,
	.dev.platform_data	= &i2c3_platdata,
};

#if defined(__FUEL_GAUGES_IC_MAX17043__)
static struct i2c_gpio_platform_data i2c4_platdata = {
	.sda_pin		= GPIO_FUEL_SDA,
	.scl_pin		= GPIO_FUEL_SCL,
	.udelay			= 3,	/* 250KHz */		
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device s3c_device_i2c4 = {
	.name				= "i2c-gpio",
	.id					= 4,
	.dev.platform_data	= &i2c4_platdata,
};
#endif

// dgahn.smd: move to rev01
static struct i2c_gpio_platform_data i2c5_platdata = {
	.sda_pin		= GPIO_AP_SDA/*GPIO_TOUCH_I2C_SDA*/,
	.scl_pin		= GPIO_AP_SCL/*GPIO_TOUCH_I2C_SCL*/,
	.udelay			= 3,	/* 166KHz */		
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device s3c_device_i2c5 = {
	.name				= "i2c-gpio",
	.id					= 5,
	.dev.platform_data	= &i2c5_platdata,
};

static struct i2c_gpio_platform_data i2c6_platdata = {
	.sda_pin		= GPIO_TOUCH_I2C_SDA,
	.scl_pin		= GPIO_TOUCH_I2C_SCL,
	.udelay			= 6,	/* KSS : Change I2C speed to avoid error in TSP */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device s3c_device_i2c6 = {
	.name				= "i2c-gpio",
	.id					= 6,
	.dev.platform_data	= &i2c6_platdata,
};

#if 0
static struct i2c_gpio_platform_data i2c7_platdata = {
	.sda_pin		= GPIO_AP_SDA,
	.scl_pin		= GPIO_AP_SCL,
	.udelay			= 2,	/* 250KHz */		
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 1,
};

static struct platform_device s3c_device_i2c7 = {
	.name				= "i2c-gpio",
	.id					= 7,
	.dev.platform_data	= &i2c7_platdata,
};
#endif
