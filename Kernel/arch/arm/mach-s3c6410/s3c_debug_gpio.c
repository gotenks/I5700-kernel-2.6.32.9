/******************************************************************
 * Dump GPIO
 * ****************************************************************/
#define GPA_SIZE	8
#define GPB_SIZE	7
#define GPC_SIZE	8
#define GPD_SIZE	5
#define GPE_SIZE	5
#define GPF_SIZE	16
#define GPG_SIZE	7
#define GPH_SIZE	10
#define GPI_SIZE	16
#define GPJ_SIZE	12
#define GPK_SIZE	16
#define GPL_SIZE	15
#define GPM_SIZE	6
#define GPN_SIZE	16
#define GPO_SIZE	16
#define GPP_SIZE	15
#define GPQ_SIZE	9

static char* GPA_str[GPA_SIZE] = { 
	"GPA0 AP_FLM_RXD",
	"GPA1 AP_FLM_TXD",
	"GPA2 USB_SEL",
	"GPA3 MSENSE_RST",
	"GPA4 BT_UART_RXD",
	"GPA5 BT_UART_TXD",
	"GPA6 BT_UART_CTS",
	"GPA7 BT_UART_RTS",
};

static char* GPB_str[GPB_SIZE] = { 
	"GPB0 AP_RXD",
	"GPB1 AP_TXD",
	"GPB2 CAM_SCL",
	"GPB3 CAM_SDA",
	"GPB4 TOUCH_EN",
	"GPB5 AP_SCL_3.0V",
	"GPB6 AP_SDA_3.0V",
};

static char* GPC_str[GPC_SIZE] = { 
	"GPC0 HM_MISO",
	"GPC1 HM_CLK",
	"GPC1 HM_MOSI",
	"GPC3 HM_NCS",
	"GPC4 WLAN_CMD",
	"GPC5 WLAN_CLK",
	"GPC6 WLAN_WAKE",
	"GPC7 BT_WAKE",
};

static char* GPD_str[GPD_SIZE] = { 
	"GPD0 I2C_SCLK",
	"GPD1 BT_WLAN_REG_ON",
	"GPD2 I2S_SYNC",
	"GPD3 I2S_SDI",
	"GPD4 I2S_SDO",
};

static char* GPE_str[GPE_SIZE] = { 
	"GPE0 BT_RESET",
	"GPE1 BOOT",
	"GPE2 WLAN_RESET",
	"GPE3 PWR_I2C_SCL",
	"GPE4 PWR_I2C_SDA",
};

static char* GPF_str[GPF_SIZE] = { 
	"GPF0 CAM_MCLK",
	"GPF1 CAM_HSYNC",
	"GPF2 CAM_PCLK",
	"GPF3 CAM_3M_nRST",
	"GPF4 CAM_VSYNC",
	"GPF5 CAM_D(0)",
	"GPF6 CAM_D(1)",
	"GPF7 CAM_D(2)",
	"GPF8 CAM_D(3)",
	"GPF9 CAM_D(4)",
	"GPF10 CAM_D(5)",
	"GPF11 CAM_D(6)",
	"GPF12 CAM_D(7)",
	"GPF13 CAM_CIF_nRST",
	"GPF14 CAM_CIF_nSTBY",
	"GPF15 VIB_PWM",
};

static char* GPG_str[GPG_SIZE] = { 
	"GPG0 T_FLASH_CLK",
	"GPG1 T_FLASH_CMD",
	"GPG2 T_FLASH_D(0)",
	"GPG3 T_FLASH_D(1)",
	"GPG4 T_FLASH_D(2)",
	"GPG5 T_FLASH_D(3)",
	"GPG6",
};

static char* GPH_str[GPH_SIZE] = { 
	"GPH0",
	"GPH1",
	"GPH2",
	"GPH3",
	"GPH4",
	"GPH5",
	"GPH6 WLAN_DATA(0)",
	"GPH7 WLAN_DATA(1)",
	"GPH8 WLAN_DATA(2)",
	"GPH9 WLAN_DATA(3)",
};

static char* GPI_str[GPI_SIZE] = { 
	"GPI0",
	"GPI1",
	"GPI2 LCD_D(0)",
	"GPI3 LCD_D(1)",
	"GPI4 LCD_D(2)",
	"GPI5 LCD_D(3)",
	"GPI6 LCD_D(4)",
	"GPI7 LCD_D(5)",
	"GPI8",
	"GPI9",
	"GPI10 LCD_D(6)",
	"GPI11 LCD_D(7)",
	"GPI12 LCD_D(8)",
	"GPI13 LCD_D(9)",
	"GPI14 LCD_D(10)",
	"GPI15 LCD_D(11)",
};

static char* GPJ_str[GPJ_SIZE] = { 
	"GPJ0",
	"GPJ1",
	"GPJ2 LCD_D(12)",
	"GPJ3 LCD_D(13)",
	"GPJ4 LCD_D(14)",
	"GPJ5 LCD_D(15)",
	"GPJ6 LCD_D(16)",
	"GPJ7 LCD_D(17)",
	"GPJ8 LCD_HSYNC",
	"GPJ9 LCD_VSYNC",
	"GPJ10 LCD_DE",
	"GPJ11 LCD_MCLK",
};

static char* GPK_str[GPK_SIZE] = { 
	"GPK0 TA_EN",
	"GPK1 AUDIO_EN",
	"GPK2 PHONE_ON",
	"GPK3 MICBIAS_EN",
	"GPK4 UART_SEL",
	"GPK5 FM_nRST",
	"GPK6 CAM_EN",
	"GPK7 nMSM_RST",
	"GPK8 KBR(0)",
	"GPK9 KBR(1)",
	"GPK10 KBR(2)",
	"GPK11 KBR(3)",
	"GPK12 KBR(4)",
	"GPK13 KBR(5)",
	"GPK14 KBR(6)",
	"GPK15 VREG_MSMP_2.6V",
};

static char* GPL_str[GPL_SIZE] = { 
	"GPL0 KBC(0)",
	"GPL1 KBC(1)",
	"GPL2 KBC(2)",
	"GPL3 KBC(3)",
	"GPL4 KBC(4)",
	"GPL5 KBC(5)",
	"GPL6 KBC(6)",
	"GPL7 USIM_BOOT",
	"GPL8 CAM_3M_nSTBY",
	"GPL9 HM_INT",
	"GPL10 FM_INT",
	"GPL11 TA_nCONNECTED",
	"GPL12 TOUCH_INT",
	"GPL13 CP_BOOT_SEL",
	"GPL14 BT_HOST_WAKE",
};

static char* GPM_str[GPM_SIZE] = { 
	"GPM0 FM_SCL",
	"GPM1 FM_SDA",
	"GPM2 TA_nCHG",
	"GPM3 PDA_ACTIVE",
	"GPM4 TOUCH_SCL",
	"GPM5 TOUCH_SDA",
};

static char* GPN_str[GPN_SIZE] = { 
	"GPN0 nONED_INT_AP",
	"GPN1 WLAN_HOST_WAKE",
	"GPN2 MSENSE_IRQ",
	"GPN3 ACC_INT",
	"GPN4 SLIDE_INT",
	"GPN5 nPOWER",
	"GPN6 T_FLASH_DETECT",
	"GPN7 PHONE_ACTIVE",
	"GPN8 PM_INT_N",
	"GPN9 INTB",
	"GPN10 DET_3.5",
	"GPN11 EAR_SEND_END",
	"GPN12 RESOUT_N",
	"GPN13 BOOT_EINT13",
	"GPN14 BOOT_EINT14",
	"GPN15 BOOT_EINT15",
};

static char* GPO_str[GPO_SIZE] = { 
	"GPO0 ONENAND_CS",
	"GPO1",
	"GPO2 LCD_nRST",
	"GPO3",
	"GPO4",
	"GPO5",
	"GPO6 LCD_nCS",
	"GPO7 LCD_SI",
	"GPO8",
	"GPO9",
	"GPO10",
	"GPO11",
	"GPO12 LCD_VENDOR",
	"GPO13 LCD_CLK",
	"GPO14",
	"GPO15",
};

static char* GPP_str[GPP_SIZE] = { 
	"GPP0",
	"GPP1",
	"GPP2",
	"GPP3",
	"GPP4",
	"GPP5",
	"GPP6",
	"GPP7",
	"GPP8",
	"GPP9",
	"GPP10",
	"GPP11",
	"GPP12",
	"GPP13 AP_PSHOLD",
	"GPP14",
};

static char* GPQ_str[GPQ_SIZE] = { 
	"GPQ0",
	"GPQ1",
	"GPQ2",
	"GPQ3 HMLDO_EN",
	"GPQ4 MIC_SEL",
	"GPQ5 VIB_EN",
	"GPQ6 HM_EN",
	"GPQ7",
	"GPQ8",
};

void s3c_dump_gpio(void)
{
#define CHECK_GPIO(pin, name)						\
	do {								\
		unsigned int pinfun = s3c_gpio_get_cfgpin(pin);		\
		unsigned int pinpud = s3c_gpio_getpull(pin);		\
		if (pinfun != 0 || pinpud  != 1)			\
			printk(KERN_INFO "%s->function:%x, pull up/down:%x\n",\
					name, pinfun, pinpud);		\
	} while (0)

#define CHECK_GPIO_PORT(port)						\
	do {								\
		int i;							\
		for (i = 0; i < GP##port##_SIZE; i++)			\
			CHECK_GPIO(S3C64XX_GP##port(i), GP##port##_str[i]);\
	} while (0)

	printk("******************************************\n");	
	CHECK_GPIO_PORT(A);
	CHECK_GPIO_PORT(B);
	CHECK_GPIO_PORT(C);
	CHECK_GPIO_PORT(D);
	CHECK_GPIO_PORT(E);
	CHECK_GPIO_PORT(F);
	CHECK_GPIO_PORT(G);
	CHECK_GPIO_PORT(H);
	CHECK_GPIO_PORT(I);
	CHECK_GPIO_PORT(J);
	CHECK_GPIO_PORT(K);
	CHECK_GPIO_PORT(L);
	CHECK_GPIO_PORT(M);
	CHECK_GPIO_PORT(N);
	CHECK_GPIO_PORT(O);
	CHECK_GPIO_PORT(P);
	CHECK_GPIO_PORT(Q);
	printk("******************************************\n");	
}
EXPORT_SYMBOL(s3c_dump_gpio);
