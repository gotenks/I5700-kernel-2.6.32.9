/* linux/arch/arm/mach-s3c6410/mach-infobowlq.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/ctype.h>
#include <linux/reboot.h>
#include <linux/delay.h>
#include <linux/leds.h>
#include <linux/bootmem.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/spica.h>
#include <mach/map.h>
#include <mach/regs-mem.h>
#include <mach/param.h>

#include <asm/irq.h>

#include <asm/setup.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <plat/regs-rtc.h>
#include <plat/regs-clock.h>
#include <plat/regs-gpio.h>
#include <plat/iic.h>

#include <plat/nand.h>
#include <plat/partition.h>
#include <plat/s3c6410.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adcts.h>
#include <plat/ts.h>
#include <plat/adc.h>
#include <plat/reserved_mem.h>
#include <plat/pm.h>

#include <mach/gpio.h>
#include <mach/gpio-core.h>

#include <plat/gpio-cfg.h>
#include <linux/android_pmem.h>
#include <linux/i2c/pmic.h>

#include <mach/sec_headset.h>

#ifdef CONFIG_KERNEL_DEBUG_SEC
#include <linux/kernel_sec_common.h>
#endif

struct class *sec_class;
EXPORT_SYMBOL(sec_class);

struct device *switch_dev;
EXPORT_SYMBOL(switch_dev);

#ifdef CONFIG_SEC_LOG_BUF
struct device *sec_log_dev;
EXPORT_SYMBOL(sec_log_dev);
#endif

void (*sec_set_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_set_param_value);

void (*sec_get_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_get_param_value);


void infobowlq_init_gpio(void);
#ifdef DUMP_GPIO_DEBUG_INFO
void s3c_dump_gpio(void);
#endif

#define UCON S3C_UCON_DEFAULT
#define ULCON S3C_LCON_CS8 | S3C_LCON_PNONE
#define UFCON S3C_UFCON_RXTRIG8 | S3C_UFCON_FIFOMODE

#define S3C64XX_KERNEL_PANIC_DUMP_SIZE 0x8000 /* 32kbytes */
void *S3C64XX_KERNEL_PANIC_DUMP_ADDR;

//bgkim dk09 added for wifi static memory allocation
#define S3C64XX_WIFI_STATIC_MEM_SIZE 0x40000 //0x8000*8 /* 256kbytes */
void *S3C64XX_WIFI_STATIC_MEM_ADDR;
EXPORT_SYMBOL(S3C64XX_WIFI_STATIC_MEM_ADDR);

#ifndef CONFIG_HIGH_RES_TIMERS
extern struct sys_timer s3c64xx_timer;
#else
extern struct sys_timer sec_timer;
#endif /* CONFIG_HIGH_RES_TIMERS */

static struct s3c_uartcfg infobowlq_uartcfgs[] __initdata = {
	[0] = {	/* Phone */
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {	/* Bluetooth */
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {	/* Serial */
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

#if defined(CONFIG_I2C_GPIO)
    #include "s3c_infobowlq.c"
#endif

struct platform_device sec_device_backlight = {
	.name   = "s6d05a-backlight",
	.id     = -1,
};

struct platform_device sec_device_dpram = {
	.name	= "dpram-device",
	.id		= -1,
};

#if defined(CONFIG_TOUCHSCREEN_QT5480)
struct platform_device sec_device_ts = {
	.name	= "qt5480-ts",
	.id		= -1,
};
#elif defined(CONFIG_TOUCHSCREEN_MELFAS)
struct platform_device sec_device_ts = {
	.name	= "melfas-ts",
	.id		= -1,
};
#else 
#error Platform touchscreen driver not defined !
#endif

struct platform_device sec_device_max9877 = {
	.name	= "max9877",
	.id	= -1,
};

struct platform_device sec_device_battery = {
	.name	= "spica-battery",
	.id		= -1,
};

struct platform_device sec_device_rfkill = {
	.name	= "bt_rfkill",
	.id	= -1,
};

struct platform_device sec_device_btsleep = {
	.name	= "bt_sleep",
	.id	= -1,
};

struct platform_device sec_device_eled = {
	.name	= "bd2802gue2-eled",
	.id	= -1,
};

#if 0
struct platform_device sec_device_optjoy = {
	.name	= "optjoy_device",
	.id	= -1,
};
#endif

struct platform_device sec_device_opt = {
	.name	= "gp2a-opt",
	.id	= -1,
};

static struct sec_headset_port sec_headset_port[] = {
	{
		{ // HEADSET detect info
			.eint		= IRQ_EINT(10), 
			.gpio		= GPIO_DET_35,   
			.gpio_af	= GPIO_DET_35_AF  , 
			.low_active 	= 0
		},{ // SEND/END info
			.eint		= IRQ_EINT(11), 
			.gpio		= GPIO_EAR_SEND_END, 
			.gpio_af	= GPIO_EAR_SEND_END_AF, 
			.low_active	= 1
		},{ // SEND/END info
			.eint		= IRQ_EINT(21), 
			.gpio		= GPIO_MONOHEAD_DET, 
			.gpio_af	= GPIO_MONOHEAD_DET_AF, 
			.low_active	= 0
	     	}
        }
};
 
static struct sec_headset_platform_data sec_headset_data = {
	.port           = sec_headset_port,
	.nheadsets      = ARRAY_SIZE(sec_headset_port),
};

static struct platform_device sec_device_headset = {
	.name           = "sec_headset",
	.id             = -1,
	.dev            = {
	.platform_data  = &sec_headset_data,
        },
};

static struct s3c6410_pmem_setting pmem_setting = {
        .pmem_start         = RESERVED_PMEM_START,
        .pmem_size          = RESERVED_PMEM,
#ifdef CONFIG_USE_TOM3Q_DRIVERS
        .pmem_gpu1_start    = GPU1_RESERVED_PMEM_START,
        .pmem_gpu1_size     = RESERVED_PMEM_GPU1,
#endif
        .pmem_render_start  = RENDER_RESERVED_PMEM_START,
        .pmem_render_size   = RESERVED_PMEM_RENDER,
        .pmem_stream_start  = STREAM_RESERVED_PMEM_START,
        .pmem_stream_size   = RESERVED_PMEM_STREAM,
        .pmem_preview_start = PREVIEW_RESERVED_PMEM_START,
        .pmem_preview_size  = RESERVED_PMEM_PREVIEW,
        .pmem_picture_start = PICTURE_RESERVED_PMEM_START,
        .pmem_picture_size  = RESERVED_PMEM_PICTURE,
        .pmem_jpeg_start    = JPEG_RESERVED_PMEM_START,
        .pmem_jpeg_size     = RESERVED_PMEM_JPEG,
};

#ifdef CONFIG_S3C64XX_ADCTS
static struct s3c_adcts_plat_info s3c_adcts_cfgs __initdata = {
	.channel = {
		{ /* 0 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 1 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 2 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 3 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 4 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 5 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 6 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},{ /* 7 */
			.delay = 0xFF,
			.presc = 49,
			.resol = S3C_ADCCON_RESSEL_12BIT,
		},
	},
};
#endif

struct map_desc infobowlq_iodesc[] __initdata = {
};

static struct platform_device *infobowlq_devices[] __initdata = {
#if defined(CONFIG_S3C_DMA_PL080_SOL)
	&s3c_device_dma0,
	&s3c_device_dma1,
	&s3c_device_dma2,
	&s3c_device_dma3,
#endif
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc2,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
#if defined(CONFIG_I2C_GPIO)
	&s3c_device_i2c2,
	&s3c_device_i2c3,
// definition is placed in : mach/infobowlq_gpio.h
#if defined(__FUEL_GAUGES_IC_MAX17043__) 
	&s3c_device_i2c4,
#endif
        //&s3c_device_i2c5,
        &s3c_device_i2c6,
#endif
	&sec_device_ts,
#ifdef CONFIG_S3C64XX_ADCTS
	&s3c_device_adcts,
#endif
#ifdef CONFIG_S3C_ADC
	&s3c_device_adc,
#endif
	&s3c_device_lcd,
	&s3c_device_keypad,
	&s3c_device_usbgadget,
	&s3c_device_android_usb,
	&s3c_device_usb_mass_storage,
	&s3c_device_camif,
	&s3c_device_mfc,
	&s3c_device_g3d,
	&s3c_device_2d,
	&s3c_device_rotator,
	&s3c_device_jpeg,
	&s3c_device_vpp,
	&sec_device_backlight,
	&sec_device_dpram,
	&sec_device_max9877,
	&sec_device_battery,
	&sec_device_rfkill,
	&sec_device_btsleep,  // BT_SLEEP_ENABLER
	&s3c_device_rtc, // by Anubis
	//&sec_device_keyled,
	&sec_device_eled,
	//&sec_device_optjoy,
	&sec_device_headset,
	//&sec_device_opt,
};

// local definitions of I2C addresses. Bad solution
#if defined(CONFIG_TOUCHSCREEN_QT5480)
    #define QT5480_I2C_ADDR 			0x60
    #define IRQ_TOUCH_INT 			IRQ_EINT(20)
#else
    #define MELFAS_I2C_ADDR 			0x60
    #define IRQ_TOUCH_INT 			IRQ_EINT(20)
#endif

#define E_COMPASS_ADDRESS			0x1c
#define IRQ_COMPASS_INT 			IRQ_EINT(2)

#define FSA9480UCX_ADDRESS			0x4A
#define IRQ_FSA9480_INT 			IRQ_EINT(9)

#define ACC_SENSOR_ADDRESS			0x38

#define MAX9877_ADDRESS				0x9A

static struct i2c_board_info i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO("ak8973b", E_COMPASS_ADDRESS),
		.irq = IRQ_COMPASS_INT,
	},
#if 0
	{
		I2C_BOARD_INFO("fsa9480", (FSA9480UCX_ADDRESS >> 1)),
		.irq = IRQ_FSA9480_INT,
	},
#endif
	{
		I2C_BOARD_INFO("bma020", ACC_SENSOR_ADDRESS),
	},
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{
		I2C_BOARD_INFO("s5k4ca", (S5K4CA_ID >> 1)),
	},
};

static struct i2c_board_info i2c_devs2[] __initdata = {
	{
		I2C_BOARD_INFO("MAX9877 I2C (AMP)", (MAX9877_ADDRESS >> 1)),
	},
	{
		I2C_BOARD_INFO("AK4671 I2C Codec", (0x24 >> 1)),
	},
};

static struct i2c_board_info i2c_devs3[] __initdata = {
	{
		I2C_BOARD_INFO("max8698", (0xcc >> 1)),
	},
};

static struct i2c_board_info i2c_devs6[] __initdata = {
#if defined(CONFIG_TOUCHSCREEN_QT5480)
	{
		I2C_BOARD_INFO("qt5480_i2c", (QT5480_I2C_ADDR >> 1)),
		.irq = IRQ_TOUCH_INT,
	},
#elif defined(CONFIG_TOUCHSCREEN_MELFAS)
	{
		I2C_BOARD_INFO("melfas_ts_i2c", (MELFAS_I2C_ADDR >> 1)), 
		.irq = IRQ_TOUCH_INT,
	},
#endif
};

#ifdef CONFIG_S3C_ADC
static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
	/* Support 12-bit resolution */
	.delay		= 0xff,
	.presc 		= 49,
	.resolution	= 12,
};
#endif

static void __init infobowlq_fixup(struct machine_desc *desc,
		struct tag *tags, char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	
	mi->bank[0].start = PHYS_OFFSET;
	mi->bank[0].size = PHYS_UNRESERVED_SIZE;
	mi->bank[0].node = 0;
}

static void __init s3c64xx_allocate_memory_regions(void) {
	void *addr;
	unsigned long size;

	size = S3C64XX_KERNEL_PANIC_DUMP_SIZE;
	addr = alloc_bootmem(size);
	S3C64XX_KERNEL_PANIC_DUMP_ADDR = addr;

}

//bgkim dk09 added for wifi static memory allocation
static void __init s3c64xx_allocate_wifi_static_memory(void) {
	void *addr;
	unsigned long size;

	size = S3C64XX_WIFI_STATIC_MEM_SIZE;
	addr = alloc_bootmem(size);
	S3C64XX_WIFI_STATIC_MEM_ADDR = addr;

}

static void __init infobowlq_map_io(void) {
	s3c64xx_init_io(infobowlq_iodesc, ARRAY_SIZE(infobowlq_iodesc));
	s3c64xx_gpiolib_init();
	s3c_init_clocks(12000000);
	s3c_init_uarts(infobowlq_uartcfgs, ARRAY_SIZE(infobowlq_uartcfgs));
	s3c64xx_allocate_memory_regions();
	s3c64xx_allocate_wifi_static_memory(); //bgkim dk09 added for wifi static memory allocation
}

static void infobowlq_set_qos(void) {     
	u32 reg;     							/* AXI sfr */     

	reg = (u32) ioremap((unsigned long) S3C6410_PA_AXI_SYS, SZ_4K); /* QoS override: FIMD min. latency */
	writel(0xffb6, S3C_VA_SYS + 0x128);  	    			/* AXI QoS */
	writel(0x7, reg + 0x460);   					/* (8 - MFC ch.) */
	writel(0x7ff7, reg + 0x464);      				/* Bus cacheable */
	writel(0x8ff, S3C_VA_SYS + 0x838);

	__raw_writel(0x0, S3C_AHB_CON0);
} 

/*
 *	Power Off Handler
 */

extern int get_usb_cable_state(void);

#define AV				(0x1 << 14)
#define TTY				(0x1 << 13)
#define PPD				(0x1 << 12)
#define JIG_UART_OFF			(0x1 << 11)
#define JIG_UART_ON			(0x1 << 10)
#define JIG_USB_OFF			(0x1 << 9)
#define JIG_USB_ON			(0x1 << 8)
#define USB_OTG				(0x1 << 7)
#define DEDICATED_CHARGER		(0x1 << 6)
#define USB_CHARGER			(0x1 << 5)
#define CAR_KIT				(0x1 << 4)
#define UART				(0x1 << 3)
#define USB				(0x1 << 2)
#define AUDIO_TYPE2			(0x1 << 1)
#define AUDIO_TYPE1			(0x1 << 0)

extern void arch_reset(char mode);

#ifdef CONFIG_KERNEL_DEBUG_SEC
extern void kernel_sec_clear_upload_magic_number();
#endif 

static void infobowlq_pm_power_off(void) {
	int  mode = REBOOT_MODE_NONE;
	char reset_mode = 'r';
	int  cnt = 0;

#ifdef CONFIG_KERNEL_DEBUG_SEC
	// Clear the magic number because it's normal reboot.
	kernel_sec_clear_upload_magic_number();  
#endif 

	if (!gpio_get_value(GPIO_TA_CONNECTED_N)) {	/* Reboot Charging */
		mode = REBOOT_MODE_CHARGING;
		if (sec_set_param_value)
			sec_set_param_value(__REBOOT_MODE, &mode);
		/* Watchdog Reset */
		printk(KERN_EMERG "%s: TA is connected, rebooting...\n", __func__);
		arch_reset(reset_mode);
		printk(KERN_EMERG "%s: waiting for reset!\n", __func__);
	}
	else {	/* Power Off or Reboot */
		if (sec_set_param_value)
			sec_set_param_value(__REBOOT_MODE, &mode);
		if (get_usb_cable_state() & (JIG_UART_ON | JIG_UART_OFF | JIG_USB_OFF | JIG_USB_ON)) {
			/* Watchdog Reset */
			printk(KERN_EMERG "%s: JIG is connected, rebooting...\n", __func__);
			arch_reset(reset_mode);
			printk(KERN_EMERG "%s: waiting for reset!\n", __func__);
		}
		else {
			/* POWER_N -> Input */
			if (gpio_is_valid(GPIO_POWER_N)) {
				if (gpio_request(GPIO_POWER_N, S3C_GPIO_LAVEL(GPIO_POWER_N))) 
					printk(KERN_ERR "Failed to request GPIO_POWER_N!\n");
				gpio_direction_input(GPIO_POWER_N);
			}
			/* PHONE_ACTIVE -> Input */
			if (gpio_is_valid(GPIO_PHONE_ACTIVE)) {
				if (gpio_request(GPIO_PHONE_ACTIVE, S3C_GPIO_LAVEL(GPIO_PHONE_ACTIVE))) 
					printk(KERN_ERR "Failed to request GPIO_PHONE_ACTIVE!\n");
				gpio_direction_input(GPIO_PHONE_ACTIVE);
			}
			/* Check Power Off Condition */
			if (!gpio_get_value(GPIO_POWER_N) || gpio_get_value(GPIO_PHONE_ACTIVE)) {
				/* Wait Power Button Release */
				printk(KERN_EMERG "%s: waiting for GPIO_POWER_N high.\n", __func__);
				while (!gpio_get_value(GPIO_POWER_N)); // !!! potential hang ??? !!!

				/* Wait Phone Power Off */
				printk(KERN_EMERG "%s: waiting for GPIO_PHONE_ACTIVE low.\n", __func__);
				while (gpio_get_value(GPIO_PHONE_ACTIVE)) {
					if (cnt++ < 5) {
						printk(KERN_EMERG "%s: GPIO_PHONE_ACTIVE is high(%d)\n", __func__, cnt);
						mdelay(1000);
					} else {
						printk(KERN_EMERG "%s: GPIO_PHONE_ACTIVE TIMED OUT!!!\n", __func__);
						break;
					}
				}	
			}
			/* PS_HOLD -> Output Low */
			printk(KERN_EMERG "%s: setting GPIO_PDA_PS_HOLD low.\n", __func__);

			if (gpio_is_valid(GPIO_PDA_PS_HOLD)) {
				if (gpio_request(GPIO_PDA_PS_HOLD, S3C_GPIO_LAVEL(GPIO_PDA_PS_HOLD))) 
					printk(KERN_ERR "Failed to request GPIO_PDA_PS_HOLD!\n");
				gpio_direction_output(GPIO_PDA_PS_HOLD, GPIO_LEVEL_HIGH);
			}
			s3c_gpio_setpull(GPIO_PDA_PS_HOLD, S3C_GPIO_PULL_NONE);
			gpio_set_value(GPIO_PDA_PS_HOLD, GPIO_LEVEL_LOW);

			printk(KERN_EMERG "%s: should not reach here!\n", __func__);
		}
	}

	while (1);
}

static int uart_current_owner = 0;

static ssize_t uart_switch_show(struct device *dev, struct device_attribute *attr, char *buf) {
	int switch_sel;

	if (sec_get_param_value) {
		sec_get_param_value(__SWITCH_SEL, &switch_sel);
	}
	else {
		switch_sel = (gpio_get_value(GPIO_UART_SEL)==GPIO_LEVEL_HIGH)<<1;
	}

	if ( uart_current_owner )		
		return sprintf(buf, "%s[UART Switch] Current UART owner = PDA \n", buf);
	else			
		return sprintf(buf, "%s[UART Switch] Current UART owner = MODEM \n", buf);
}

static ssize_t uart_switch_store(
		struct device *dev, 
		struct device_attribute *attr,
		const char *buf, 
		size_t size)
{	
	int switch_sel;

	if (sec_get_param_value)
		sec_get_param_value(__SWITCH_SEL, &switch_sel);

	if (strncmp(buf, "PDA", 3) == 0 || strncmp(buf, "pda", 3) == 0)	{		
		gpio_set_value(GPIO_UART_SEL, GPIO_LEVEL_HIGH);		
		uart_current_owner = 1;		
		switch_sel |= UART_SEL_MASK;
		printk("[UART Switch] Path : PDA\n");	
	}	

	if (strncmp(buf, "MODEM", 5) == 0 || strncmp(buf, "modem", 5) == 0) {		
		gpio_set_value(GPIO_UART_SEL, GPIO_LEVEL_LOW);		
		uart_current_owner = 0;		
		switch_sel &= ~UART_SEL_MASK;
		printk("[UART Switch] Path : MODEM\n");	
	}	

	if (sec_set_param_value)
		sec_set_param_value(__SWITCH_SEL, &switch_sel);

	return size;
}

static DEVICE_ATTR(uart_sel, S_IRUGO |S_IWUGO | S_IRUSR | S_IWUSR, uart_switch_show, uart_switch_store);

static int infobowlq_notifier_call(struct notifier_block *this, unsigned long code, void *_cmd) {
	int	mode = REBOOT_MODE_NONE;

	if ((code == SYS_RESTART) && _cmd) {
		if (!strcmp((char *)_cmd, "arm11_fota"))
			mode = REBOOT_MODE_ARM11_FOTA;
		else if (!strcmp((char *)_cmd, "arm9_fota"))
			mode = REBOOT_MODE_ARM9_FOTA;
		else if (!strcmp((char *)_cmd, "recovery")) {
			mode = REBOOT_MODE_RECOVERY;
#ifdef CONFIG_KERNEL_DEBUG_SEC 
			//etinum.factory.reboot disable uart msg in bootloader for
			// factory reset 2nd ack
			kernel_sec_set_upload_cause(BLK_UART_MSG_FOR_FACTRST_2ND_ACK);
#endif
		}
		else if (!strcmp((char *)_cmd, "download")) 
			mode = REBOOT_MODE_DOWNLOAD;
#ifdef CONFIG_KERNEL_DEBUG_SEC 
		// etinum.factory.reboot disable uart msg
		// in bootloader for factory reset 2nd ack
		else if (!strcmp((char *)_cmd, "factory_reboot")) { 
			mode = REBOOT_MODE_NONE;
			kernel_sec_set_upload_cause(BLK_UART_MSG_FOR_FACTRST_2ND_ACK);
		}
#endif		
	}

	if (sec_set_param_value)
		sec_set_param_value(__REBOOT_MODE, &mode);
	
	return NOTIFY_DONE;
}

static struct notifier_block infobowlq_reboot_notifier = {
	.notifier_call = infobowlq_notifier_call,
};

static void infobowlq_switch_init(void) {
	sec_class = class_create(THIS_MODULE, "sec");
	if (IS_ERR(sec_class))
		pr_err("Failed to create class(sec)!\n");

	switch_dev = device_create(sec_class, NULL, 0, NULL, "switch");
	if (IS_ERR(switch_dev))
		pr_err("Failed to create device(switch)!\n");

	if (gpio_is_valid(GPIO_UART_SEL)) {
		if (gpio_request(GPIO_UART_SEL, S3C_GPIO_LAVEL(GPIO_UART_SEL))) 
			printk(KERN_ERR "Failed to request GPIO_UART_SEL!\n");
		gpio_direction_output(GPIO_UART_SEL, gpio_get_value(GPIO_UART_SEL));
	}
	s3c_gpio_setpull(GPIO_UART_SEL, S3C_GPIO_PULL_NONE);

	if (device_create_file(switch_dev, &dev_attr_uart_sel) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_uart_sel.attr.name);
};

static void __init infobowlq_machine_init(void) {
	printk("INFOBOWLQ Machine INIT : Board REV 0x%x\n", CONFIG_INFOBOWLQ_REV);
	
	infobowlq_init_gpio();

	//MOTOR and VIBTONZE DISABLE
	if( gpio_is_valid(GPIO_VIB_EN) ) {
		if (gpio_request(GPIO_VIB_EN, S3C_GPIO_LAVEL(GPIO_VIB_EN))) 
			printk(KERN_ERR "Failed to request GPIO_VIB_EN!\n");
		gpio_direction_output(GPIO_VIB_EN, GPIO_LEVEL_LOW);
		gpio_free(GPIO_VIB_EN);
	}
	s3c_gpio_cfgpin(GPIO_VIB_EN, S3C_GPIO_SFN(GPIO_VIB_EN_AF));
	s3c_gpio_setpull(GPIO_VIB_EN, S3C_GPIO_PULL_DOWN);
	//gpio_direction_output(GPIO_VIB_EN, GPIO_LEVEL_LOW);

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);

#ifdef CONFIG_S3C64XX_ADCTS
	s3c_adcts_set_platdata (&s3c_adcts_cfgs);
#endif
#ifdef CONFIG_S3C_ADC
	s3c_adc_set_platdata(&s3c_adc_platform);
#endif

	//TOUCH_EN HIGH(Rq. HW)
#if !defined( CONFIG_TOUCHSCREEN_QT5480 )
	s3c_gpio_cfgpin(GPIO_TOUCH_EN, S3C_GPIO_SFN(GPIO_TOUCH_EN_AF));
	s3c_gpio_setpull(GPIO_TOUCH_EN, S3C_GPIO_PULL_NONE);
	
	if( gpio_is_valid(GPIO_TOUCH_EN) ) {
		if (gpio_request(GPIO_TOUCH_EN, S3C_GPIO_LAVEL(GPIO_TOUCH_EN))) 
			printk(KERN_ERR "Failed to request GPIO_TOUCH_EN in INIT !\n");
		gpio_direction_output(GPIO_TOUCH_EN, GPIO_LEVEL_HIGH);
		gpio_free(GPIO_TOUCH_EN);
	}
#endif
 
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));
	i2c_register_board_info(3, i2c_devs3, ARRAY_SIZE(i2c_devs3));
	i2c_register_board_info(6, i2c_devs6, ARRAY_SIZE(i2c_devs6));

	platform_add_devices(infobowlq_devices, ARRAY_SIZE(infobowlq_devices));
        s3c6410_add_mem_devices (&pmem_setting);

	s3c6410_pm_init();

	infobowlq_set_qos();

	pm_power_off = infobowlq_pm_power_off;

	register_reboot_notifier(&infobowlq_reboot_notifier);

	infobowlq_switch_init();

#ifdef CONFIG_SEC_LOG_BUF
	sec_log_buf_init();
#endif
#ifdef DUMP_GPIO_DEBUG_INFO
	s3c_dump_gpio();
#endif
}

MACHINE_START(INFOBOWLQ, "SPICA") //mrahn80 DJ26 : apply new codec libs.
	/* Maintainer: Ben Dooks <ben@fluff.org> */
	.phys_io		= S3C_PA_UART & 0xfff00000,
	.io_pg_offst		= (((u32)S3C_VA_UART) >> 18) & 0xfffc,
	.boot_params		= S3C64XX_PA_SDRAM + 0x100,

	.init_irq		= s3c6410_init_irq,
	.fixup			= infobowlq_fixup,
	.map_io			= infobowlq_map_io,
	.init_machine		= infobowlq_machine_init,
#ifndef CONFIG_HIGH_RES_TIMERS
	.timer			= &s3c64xx_timer,
#else
	.timer			= &sec_timer,
#endif /* CONFIG_HIGH_RES_TIMERS */
MACHINE_END

#if defined(CONFIG_RTC_DRV_S3C)
/* RTC common Function for samsung APs*/
unsigned int s3c_rtc_set_bit_byte(void __iomem *base, uint offset, uint val) {
	writeb(val, base + offset);
	return 0;
}

unsigned int s3c_rtc_read_alarm_status(void __iomem *base) {
	return 1;
}

void s3c_rtc_set_pie(void __iomem *base, uint to) {
	unsigned int tmp;
	tmp = readw(base + S3C_RTCCON) & ~S3C_RTCCON_TICEN;
        if (to)
                tmp |= S3C_RTCCON_TICEN;
        writew(tmp, base + S3C_RTCCON);
}

void s3c_rtc_set_freq_regs(void __iomem *base, uint freq, uint s3c_freq) {
	unsigned int tmp;

	tmp = readw(base + S3C_RTCCON) & (S3C_RTCCON_TICEN | S3C_RTCCON_RTCEN );
	writew(tmp, base + S3C_RTCCON);
	s3c_freq = freq;
	tmp = (32768 / freq)-1;
	writel(tmp, base + S3C_TICNT);
}

void s3c_rtc_enable_set(struct platform_device *pdev,void __iomem *base, int en) {
	unsigned int tmp;

	if (!en) {
		tmp = readw(base + S3C_RTCCON);
		writew(tmp & ~ (S3C_RTCCON_RTCEN | S3C_RTCCON_TICEN), base + S3C_RTCCON);
	} else {
		/* re-enable the device, and check it is ok */
		if ((readw(base+S3C_RTCCON) & S3C_RTCCON_RTCEN) == 0) {
			dev_info(&pdev->dev, "rtc disabled, re-enabling\n");
                    
			tmp = readw(base + S3C_RTCCON);
			writew(tmp|S3C_RTCCON_RTCEN, base+S3C_RTCCON);
		}

		if ((readw(base + S3C_RTCCON) & S3C_RTCCON_CNTSEL)) {
			dev_info(&pdev->dev, "removing RTCCON_CNTSEL\n");

			tmp = readw(base + S3C_RTCCON);
			writew(tmp& ~S3C_RTCCON_CNTSEL, base+S3C_RTCCON);
		}

		if ((readw(base + S3C_RTCCON) & S3C_RTCCON_CLKRST)) {
			dev_info(&pdev->dev, "removing RTCCON_CLKRST\n");

			tmp = readw(base + S3C_RTCCON);
			writew(tmp & ~S3C_RTCCON_CLKRST, base+S3C_RTCCON);
		}
	}
}
#endif

#if defined(CONFIG_KEYPAD_S3C) || defined (CONFIG_KEYPAD_S3C_MODULE)
void s3c_setup_keypad_cfg_gpio(int rows, int columns) {
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPK(8 + rows);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S3C64XX_GPK(8); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	end = S3C64XX_GPL(0 + columns);

	/* Set all the necessary GPL pins to special-function 0 */
	for (gpio = S3C64XX_GPL(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}

EXPORT_SYMBOL(s3c_setup_keypad_cfg_gpio);
#endif

void s3c_setup_uart_cfg_gpio(unsigned char port) {
	if (port == 0) {
		s3c_gpio_cfgpin(GPIO_FLM_RXD, S3C_GPIO_SFN(GPIO_FLM_RXD_AF));
		s3c_gpio_setpull(GPIO_FLM_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_FLM_TXD, S3C_GPIO_SFN(GPIO_FLM_TXD_AF));
		s3c_gpio_setpull(GPIO_FLM_TXD, S3C_GPIO_PULL_NONE);
	}
	else if (port == 1) {
		s3c_gpio_cfgpin(GPIO_BT_RXD, S3C_GPIO_SFN(GPIO_BT_RXD_AF));
		s3c_gpio_setpull(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_TXD, S3C_GPIO_SFN(GPIO_BT_TXD_AF));
		s3c_gpio_setpull(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_CTS, S3C_GPIO_SFN(GPIO_BT_CTS_AF));
		s3c_gpio_setpull(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_RTS, S3C_GPIO_SFN(GPIO_BT_RTS_AF));
		s3c_gpio_setpull(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
	}
	else if (port == 2) {
		s3c_gpio_cfgpin(GPIO_PDA_RXD, S3C_GPIO_SFN(GPIO_PDA_RXD_AF));
		s3c_gpio_setpull(GPIO_PDA_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_PDA_TXD, S3C_GPIO_SFN(GPIO_PDA_TXD_AF));
		s3c_gpio_setpull(GPIO_PDA_TXD, S3C_GPIO_PULL_NONE);
	}
}
EXPORT_SYMBOL(s3c_setup_uart_cfg_gpio);

void s3c_reset_uart_cfg_gpio(unsigned char port)
{
#if 0  // dgahn.temp
	if (port == 0) {
		s3c_gpio_cfgpin(GPIO_FLM_RXD, S3C_GPIO_SFN(GPIO_FLM_RXD_AF));
		s3c_gpio_setpull(GPIO_FLM_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_FLM_TXD, S3C_GPIO_SFN(GPIO_FLM_TXD_AF));
		s3c_gpio_setpull(GPIO_FLM_TXD, S3C_GPIO_PULL_NONE);
	}
#endif
	if (port == 1) {
		s3c_gpio_cfgpin(GPIO_BT_RXD, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_BT_RXD, S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(GPIO_BT_TXD, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_BT_TXD, S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(GPIO_BT_CTS, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_BT_CTS, S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(GPIO_BT_RTS, S3C_GPIO_OUTPUT);
		gpio_set_value(GPIO_BT_RTS, GPIO_LEVEL_LOW);
		s3c_gpio_setpull(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
	}
	else if (port == 2) {
		s3c_gpio_cfgpin(GPIO_PDA_RXD, S3C_GPIO_SFN(GPIO_PDA_RXD_AF));
		s3c_gpio_setpull(GPIO_PDA_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_PDA_TXD, S3C_GPIO_SFN(GPIO_PDA_TXD_AF));
		s3c_gpio_setpull(GPIO_PDA_TXD, S3C_GPIO_PULL_NONE);
	}
}
EXPORT_SYMBOL(s3c_reset_uart_cfg_gpio);

static int infobowlq_gpio_table[][6] = {
	/** OFF PART **/
	/* GPA */
#if defined(__FUEL_GAUGES_IC_MAX17043__) // is defined in : mach/infobowlq_gpio.h
	{ GPIO_FUEL_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_FUEL_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },	
#endif
	{ GPIO_FLM_RXD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_FLM_TXD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	/* { GPIO_USB_SEL, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE }, */
	{ GPIO_MSENSE_RST, 1, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT1, S3C_GPIO_PULL_NONE }, 
	{ GPIO_BT_RXD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_BT_TXD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_BT_CTS, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_BT_RTS, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	/* GPB */
	{ GPIO_PDA_RXD, GPIO_PDA_RXD_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_PDA_TXD, GPIO_PDA_TXD_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
#if (CONFIG_SPICA_REV >= CONFIG_SPICA_TEST_REV02)
	{ GPIO_I2C1_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_I2C1_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
#else
	{ GPIO_I2C1_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_I2C1_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
#endif
	{ GPIO_TOUCH_EN, 1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT1,  S3C_GPIO_PULL_NONE },
	{ GPIO_I2C0_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_I2C0_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	/* GPC */
	{ GPIO_PM_SET1, GPIO_PM_SET1_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },	
	{ GPIO_PM_SET2, GPIO_PM_SET2_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },	
	{ GPIO_PM_SET3, GPIO_PM_SET3_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },	
	{ GPIO_WLAN_CMD,  0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },	
	{ GPIO_WLAN_CLK,  0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_WLAN_WAKE, 1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_BT_WAKE,   1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	/* GPD */
	{ GPIO_I2S_CLK,        0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_BT_WLAN_REG_ON, 1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_I2S_LRCLK,      0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_I2S_DI,         0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_I2S_DO,         0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	/* GPE */
	{ GPIO_BT_RST_N,    1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_BOOT,        0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_WLAN_RST_N,  1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_PWR_I2C_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_PWR_I2C_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	/* GPF */
	{ GPIO_CAM_MCLK,   0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_HSYNC,  0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_PCLK,   0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_MCAM_RST_N, 1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_CAM_VSYNC,  0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_4, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_5, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_6, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_CAM_D_7, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_VIBTONE_PWM, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	/* GPG */
	{ GPIO_TF_CLK, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_TF_CMD, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_TF_D_0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_TF_D_1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_TF_D_2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_TF_D_3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	/* GPH */
	{ GPIO_TOUCH_I2C_SCL, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },	
	{ GPIO_TOUCH_I2C_SDA, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_FM_I2C_SCL,    0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_FM_I2C_SDA,    0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_VIB_EN,        1, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_WLAN_D_0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_WLAN_D_1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_WLAN_D_2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_WLAN_D_3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	/* GPI */
	{ GPIO_LCD_B_0, GPIO_LCD_B_0_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_B_1, GPIO_LCD_B_1_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_B_2, GPIO_LCD_B_2_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_B_3, GPIO_LCD_B_3_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_B_4, GPIO_LCD_B_4_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_B_5, GPIO_LCD_B_5_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_G_0, GPIO_LCD_G_0_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_G_1, GPIO_LCD_G_1_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_G_2, GPIO_LCD_G_2_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_G_3, GPIO_LCD_G_3_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_G_4, GPIO_LCD_G_4_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_G_5, GPIO_LCD_G_5_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	/* GPJ */
	{ GPIO_LCD_R_0, GPIO_LCD_R_0_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_R_1, GPIO_LCD_R_1_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_R_2, GPIO_LCD_R_2_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_R_3, GPIO_LCD_R_3_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_R_4, GPIO_LCD_R_4_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_R_5, GPIO_LCD_R_5_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_HSYNC, GPIO_LCD_HSYNC_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_VSYNC, GPIO_LCD_VSYNC_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_DE,  GPIO_LCD_DE_AF,  GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	{ GPIO_LCD_CLK, GPIO_LCD_CLK_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN },
	/** ALIVE PART **/
	/* GPK */
	{ GPIO_TA_EN,      GPIO_TA_EN_AF,      GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ GPIO_AUDIO_EN,   GPIO_AUDIO_EN_AF,   GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_PHONE_ON,   GPIO_PHONE_ON_AF,   GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_MICBIAS_EN, GPIO_MICBIAS_EN_AF, GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, 0, 0 },
	/* { GPIO_UART_SEL, GPIO_UART_SEL_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 }, */
	{ GPIO_TOUCH_RST,   GPIO_TOUCH_RST_AF,   GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_CAM_EN,      GPIO_CAM_EN_AF,      GPIO_LEVEL_LOW,  S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_PHONE_RST_N, GPIO_PHONE_RST_N_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPK(12), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPK(13), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPK(14), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_VREG_MSMP_26V, GPIO_VREG_MSMP_26V_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	/* GPL */
	{ GPIO_KEYSENSE_0, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSENSE_1, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSENSE_2, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSENSE_3, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_USIM_BOOT,     GPIO_USIM_BOOT_AF,     GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_CAM_3M_STBY_N, GPIO_CAM_3M_STBY_N_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
#if (CONFIG_SPICA_REV >= CONFIG_SPICA_TEST_REV02)
	{ GPIO_HOLD_KEY_N, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
#else
	{ S3C64XX_GPL(9),  1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
#endif
	{ S3C64XX_GPL(10),     1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_TA_CONNECTED_N, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_TOUCH_INT_N,    0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_CP_BOOT_SEL,    GPIO_CP_BOOT_SEL_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_BT_HOST_WAKE,   GPIO_BT_HOST_WAKE_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, 0, 0 },
	/* GPM */
	{ S3C64XX_GPM(0), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPM(1), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_TA_CHG_N,  0, GPIO_LEVEL_NONE,  S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_PDA_ACTIVE, GPIO_PDA_ACTIVE_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	{ S3C64XX_GPM(4), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPM(5), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	/* GPN */
	{ GPIO_ONEDRAM_INT_N,  0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_WLAN_HOST_WAKE, GPIO_WLAN_HOST_WAKE_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_DOWN, 0, 0 },
	{ GPIO_MSENSE_INT,     0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_ACC_INT,        0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_SIM_DETECT_N,   0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_POWER_N,        0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_TF_DETECT,      0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_PHONE_ACTIVE,   0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_PMIC_INT_N,     0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_JACK_INT_N,     0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_DET_35,         0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_EAR_SEND_END,   0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_RESOUT_N,       0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_BOOT_EINT13,    0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_BOOT_EINT14,    0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_BOOT_EINT15,    0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	/** MEMORY PART **/
	/* GPO */
	{ S3C64XX_GPO(4), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPO(5), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	/* GPP */
	{ S3C64XX_GPP(8),  1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPP(10), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPP(14), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	/* GPQ */
	{ S3C64XX_GPQ(2), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPQ(3), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPQ(4), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPQ(5), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
	{ S3C64XX_GPQ(6), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE },
};

void s3c_config_gpio_table(int array_size, int (*gpio_table)[6]) {
	u32 i, gpio;

	pr_debug("%s: ++\n", __func__);
	for (i = 0; i < array_size; i++) {
		gpio = gpio_table[i][0];
		if (gpio < S3C64XX_GPIO_ALIVE_PART_BASE) { /* Off Part */
			pr_debug("%s: Off gpio=%d,%d\n", __func__, gpio, 
					S3C64XX_GPIO_ALIVE_PART_BASE);
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
			s3c_gpio_setpull(gpio, gpio_table[i][3]);
			s3c_gpio_slp_cfgpin(gpio, gpio_table[i][4]);
			s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][5]);
			if (gpio_table[i][2] != GPIO_LEVEL_NONE)
				gpio_set_value(gpio, gpio_table[i][2]);
		} else if (gpio < S3C64XX_GPIO_MEM_PART_BASE) { /* Alive Part */
			pr_debug("%s: Alive gpio=%d\n", __func__, gpio);
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
			s3c_gpio_setpull(gpio, gpio_table[i][3]);
			if (gpio_table[i][2] != GPIO_LEVEL_NONE)
				gpio_set_value(gpio, gpio_table[i][2]);
		} else { /* Memory Part */
			pr_debug("%s: Memory gpio=%d\n", __func__, gpio);
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
			s3c_gpio_setpull(gpio, gpio_table[i][3]);
			s3c_gpio_slp_cfgpin(gpio, gpio_table[i][4]);
			s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][5]);
			if (gpio_table[i][2] != GPIO_LEVEL_NONE)
				gpio_set_value(gpio, gpio_table[i][2]);
		}
	}
	pr_debug("%s: --\n", __func__);
}
EXPORT_SYMBOL(s3c_config_gpio_table);

void infobowlq_init_gpio(void) {
	s3c_config_gpio_table(ARRAY_SIZE(infobowlq_gpio_table),
			infobowlq_gpio_table);
}

static int infobowlq_sleep_gpio_table[][6] = {
	/** ALIVE PART **/
	/* GPK */
	{ GPIO_TOUCH_RST, GPIO_TOUCH_RST_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_CAM_EN, GPIO_CAM_EN_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_PHONE_RST_N, GPIO_PHONE_RST_N_AF, GPIO_LEVEL_HIGH, S3C_GPIO_PULL_NONE, 0, 0 },
	// [ SEC Kernel2 : Enable Anykey Wakeup
#if 0
	{ GPIO_KEYSCAN_0, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_1, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_2, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSCAN_3, 0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
#endif
	// ] SEC Kernel2 : Enable Anykey Wakeup
	{ S3C64XX_GPK(12), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPK(13), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPK(14), 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_VREG_MSMP_26V, GPIO_VREG_MSMP_26V_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	/* GPL */
	{ GPIO_KEYSENSE_0, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSENSE_1, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSENSE_2, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_KEYSENSE_3, 1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPL(4),  1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPL(5),  1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ S3C64XX_GPL(6),  1, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_USIM_BOOT,     GPIO_USIM_BOOT_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_CAM_3M_STBY_N, GPIO_CAM_3M_STBY_N_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 },
	{ GPIO_TOUCH_INT_N,   0, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, 0, 0 },
	/* GPM */
	{ GPIO_PDA_ACTIVE, GPIO_PDA_ACTIVE_AF, GPIO_LEVEL_LOW, S3C_GPIO_PULL_NONE, 0, 0 }, 
	/** MEMORY PART **/
	/* GPO */
	{ GPIO_LCD_RST_N, GPIO_LCD_RST_N_AF, GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_CS_N,  GPIO_LCD_CS_N_AF,  GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_SDI,   GPIO_LCD_SDI_AF,   GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_ID,    GPIO_LCD_ID_AF,    GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE },
	{ GPIO_LCD_SCLK,  GPIO_LCD_SCLK_AF,  GPIO_LEVEL_NONE, S3C_GPIO_PULL_NONE, S3C_GPIO_SLP_OUT0,  S3C_GPIO_PULL_NONE },
};

static void check_pmic(void) {
	unsigned char reg_buff = 0;

#if defined(CONFIG_MACH_INFOBOWLQ) || defined(CONFIG_MACH_MAX)
		if (Get_MAX8698_PM_REG(EN1, &reg_buff)) {
			pr_info("  %s: BUCK1 (%d)\n", __func__, reg_buff);
			if (reg_buff)
				Set_MAX8698_PM_REG(EN1, 0);
		}
		if (Get_MAX8698_PM_REG(EN2, &reg_buff)) {
			pr_info("  %s: BUCK2 (%d)\n", __func__, reg_buff);
			if (reg_buff)
				Set_MAX8698_PM_REG(EN2, 0);
		}
#endif

	if (Get_MAX8698_PM_REG(ELDO3, &reg_buff)) {
		pr_info("  %s: OTGI 1.2V (%d)\n", __func__, reg_buff);
	}
	if (Get_MAX8698_PM_REG(ELDO4, &reg_buff)) {
		pr_info("  %s: BT WL 3.3V (%d)\n", __func__, reg_buff);
	}
	if (Get_MAX8698_PM_REG(ELDO5, &reg_buff)) {
		pr_info("  %s: MMC 3.0V (%d)\n", __func__, reg_buff);
		if (reg_buff)
			Set_MAX8698_PM_REG(ELDO5, 0);
	}
	if (Get_MAX8698_PM_REG(ELDO6, &reg_buff)) {
		pr_info("  %s: VLCD 1.8V (%d)\n", __func__, reg_buff);
	}
	if (Get_MAX8698_PM_REG(ELDO7, &reg_buff)) {
		pr_info("  %s: VLCD 3.0V (%d)\n", __func__, reg_buff);
	}
	if (Get_MAX8698_PM_REG(ELDO8, &reg_buff)) {
		pr_info("  %s: OTG 3.3V (%d)\n", __func__, reg_buff);
	}
}

void s3c_config_sleep_gpio(void) {
	int spcon_val;

	pr_info("%s: calling check_pmic()", __func__);
	check_pmic();
	s3c_config_gpio_table(ARRAY_SIZE(infobowlq_sleep_gpio_table),
			infobowlq_sleep_gpio_table);

	spcon_val = __raw_readl(S3C64XX_SPCON);
	spcon_val = spcon_val & (~0xFFEC0000);
	__raw_writel(spcon_val, S3C64XX_SPCON);
	__raw_writel(0x20, S3C64XX_SPCONSLP);

	/* mem interface reg config in sleep mode */
	__raw_writel(0x00005000, S3C64XX_MEM0CONSLP0);
	__raw_writel(0x01041595, S3C64XX_MEM0CONSLP1);
	__raw_writel(0x10055000, S3C64XX_MEM1CONSLP);	

}
EXPORT_SYMBOL(s3c_config_sleep_gpio);

void s3c_config_wakeup_gpio(void) {
	unsigned char reg_buff = 0;
	if (Get_MAX8698_PM_REG(ELDO5, &reg_buff)) {
		pr_info("%s: MMC 3.0V (%d)\n", __func__, reg_buff);
		if (!reg_buff)
			Set_MAX8698_PM_REG(ELDO5, 1);
	}
}
EXPORT_SYMBOL(s3c_config_wakeup_gpio);

extern unsigned char ftm_sleep;

#ifdef CONFIG_SEC_HEADSET
extern short int get_headset_status();
#else
short int get_headset_status(){return 1;}
#endif



#if 0
void s3c_config_wakeup_source(void) {
	unsigned int eint0pend_val;

	/* Power key (GPN5) */
	s3c_gpio_cfgpin(S3C64XX_GPN(5), S3C64XX_GPN5_EINT5);
	s3c_gpio_setpull(S3C64XX_GPN(5), S3C_GPIO_PULL_NONE);
	__raw_writel((__raw_readl(S3C64XX_EINT0CON0) & ~(0x7 << 8)) |
			(S3C64XX_EXTINT_BOTHEDGE << 8), S3C64XX_EINT0CON0);

	/* Wake-up source 
	 * ONEDRAM_INT(EINT0), WLAN_HOST_WAKE(EINT1) HALL_SW(EINT4), Power key(EINT5),
	 * T_FLASH(EINT6), DET_35(EINT10), EAR_SEND_END(EINT11), Hold key(EINT17), 
	 * TA_CONNECTED(EINT19),PROXIMITY_SENSOR(EINT20),
	 * BT_HOST_WAKE(EINT22), CHG_ING(EINT25)
	 */

	//SEC_BP_WONSUK_20090811
	//register INTB(EINT9) with wakeup source 
	eint0pend_val = __raw_readl(S3C64XX_EINT0PEND);

       //[[HYH_20100512
	if(system_rev < 0x80)
	{
		if(get_headset_status()==0)//hw bug
			eint0pend_val |= (0x1 << 25) | (0x1 << 22) |(0x1 << 20) |  (0x1 << 19) | (0x1 << 17) |
       				(0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) |  0x1;
		else
			eint0pend_val |= (0x1 << 25) | (0x1 << 22) |(0x1 << 20) |  (0x1 << 19) | 
       				(0x1 << 11) | (0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) | 0x1;
	}
	else
	{
		if(get_headset_status()==0)//hw bug
			eint0pend_val |= (0x1 << 25) | (0x1 << 22) |(0x1 << 20) |  (0x1 << 19) | (0x1 << 17) |
				(0x1 << 10) | (0x1 << 9) |(0x1 << 6) |  (0x1 << 5) | (0x1 << 4) | (0x1 << 1) |  0x1;
		else
			eint0pend_val |= (0x1 << 25) | (0x1 << 22) |(0x1 << 21) |(0x1 << 20) |  (0x1 << 19) | 
				(0x1 << 11) | (0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) | 0x1;
	}
	//]]HYH_20100512
	
	__raw_writel(eint0pend_val, S3C64XX_EINT0PEND);

	//[[HYH_20100512
	if(system_rev < 0x80)
	{
		if(get_headset_status()==0)//hw bug
			eint0pend_val = (0x1 << 25) | (0x1 << 22) | (0x1 << 20) | (0x1 << 19) | (0x1 << 17) |
				(0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) | 0x1;
		else
			eint0pend_val = (0x1 << 25) | (0x1 << 22) | (0x1 << 20) | (0x1 << 19) |
				(0x1 << 11) | (0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) | 0x1;
	}
	else
	{
		if(get_headset_status()==0)//hw bug
			eint0pend_val = (0x1 << 25) | (0x1 << 22) | (0x1 << 20) | (0x1 << 19) | (0x1 << 17) | 
				(0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) | 0x1;
		else
			eint0pend_val = (0x1 << 25) | (0x1 << 22) |(0x1 << 21) | (0x1 << 20) | (0x1 << 19) | 
				(0x1 << 11) | (0x1 << 10) | (0x1 << 9) |(0x1 << 6) | (0x1 << 5) | (0x1 << 4) | (0x1 << 1) | 0x1;
	}
	//]]HYH_20100512
	
	__raw_writel(~eint0pend_val, S3C64XX_EINT0MASK);

	__raw_writel((0x0FFFFFFF & ~eint0pend_val), S3C_EINT_MASK);

	/* Alarm Wakeup Enable */
	if (!ftm_sleep)
		__raw_writel((__raw_readl(S3C_PWR_CFG) & ~(0x1 << 10)), S3C_PWR_CFG);
	else {
		pr_info("%s: RTC alarm is disabled\n", __func__);
		__raw_writel((__raw_readl(S3C_PWR_CFG) | (0x1 << 10)), S3C_PWR_CFG);
	}
}
#else
void s3c_config_wakeup_source(void) {
	unsigned int eint0pend_val;

	/* Power key (GPN5) */
	s3c_gpio_cfgpin(GPIO_POWER_N, S3C64XX_GPN5_EINT5);
	s3c_gpio_setpull(GPIO_POWER_N, S3C_GPIO_PULL_NONE);
	__raw_writel((__raw_readl(S3C64XX_EINT0CON0) & ~(0x7 << 8)) |
			(S3C64XX_EXTINT_BOTHEDGE << 8), S3C64XX_EINT0CON0);

	/* Wake-up source 
	 * ONEDRAM_INT    (EINT0), 
	 * WLAN_HOST_WAKE (EINT1), 
	 * SIM_nDETECT    (EINT4),
	 * Power key      (EINT5), 
	 * T_FLASH_DETECT (EINT6),
	 * DET_3.5        (EINT10), 
	 * EAR_SEND_END   (EINT11), 
	 * Hold key       (EINT17), 
	 * TA_CONNECTED   (EINT19),
	 * BT_HOST_WAKE   (EINT22), 
	 * CHG_ING        (EINT25)
	 */

	//SEC_BP_WONSUK_20090811
	//register INTB(EINT9) with wakeup source 
	
	// WLAN_HOST_WAKE(EINT1) Wake-up Source disable temporary by hskang
	eint0pend_val= __raw_readl(S3C64XX_EINT0PEND);
	eint0pend_val |= (0x1 << 25) | (0x1 << 22) | (0x1 << 19) |
		(0x1 << 17) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) /*| (0x1 << 1)*/ | 0x1;
	__raw_writel(eint0pend_val, S3C64XX_EINT0PEND);

	eint0pend_val  = (0x1 << 25) | (0x1 << 22) | (0x1 << 19) |
		(0x1 << 17) | (0x1 << 11) | (0x1 << 10) | (0x1 << 9) | (0x1 << 6) | (0x1 << 5) | (0x1 << 4) /*| (0x1 << 1)*/ | 0x1;
	__raw_writel(~eint0pend_val, S3C64XX_EINT0MASK);

	__raw_writel((0x0FFFFFFF & ~eint0pend_val), S3C_EINT_MASK);

	/* Alarm Wakeup Enable */
#if 0
	__raw_writel((__raw_readl(S3C_PWR_CFG) & ~(0x1 << 10)), S3C_PWR_CFG);
#else
	if (!ftm_sleep)
		__raw_writel((__raw_readl(S3C_PWR_CFG) & ~(0x1 << 10)), S3C_PWR_CFG);
	else {
		pr_info("%s: RTC alarm is disabled\n", __func__);
		__raw_writel((__raw_readl(S3C_PWR_CFG) | (0x1 << 10)), S3C_PWR_CFG);
	}
#endif
}
#endif
EXPORT_SYMBOL(s3c_config_wakeup_source);

#ifdef DUMP_GPIO_DEBUG_INFO
    #include "s3c_debug_gpio.c"
#endif

