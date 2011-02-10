/*****************************************************************************/
/*                                                                           */
/* PROJECT : RFS_1.3.1_b046-tinyBML_1.1.0p4-XSR_1.5.2p4_b122_RTM/PROJECT                                                             */
/* MODULE  : LLD                                                             */
/* NAME    : OneNAND Low-level Driver header file                            */
/* FILE    : ONLDReg.h                                                       */
/* PURPOSE : Register, Command Set, and Local Definitions for OneNAND        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2005, SAMSUNG ELECTRONICS CO., LTD.               */
/*                          ALL RIGHTS RESERVED                              */
/*                                                                           */
/*   Permission is hereby granted to licensees of Samsung Electronics        */
/*   Co., Ltd. products to use or abstract this computer program for the     */
/*   sole purpose of implementing NAND/OneNAND based on Samsung              */
/*   Electronics Co., Ltd. products. No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in whole,      */
/*   are granted.                                                            */
/*                                                                           */
/*   Samsung Electronics Co., Ltd. makes no representations or warranties    */
/*   with respect to the performance of this computer program, and           */
/*   specifically disclaims any responsibility for any damages,              */
/*   special or consequential, connected with the use of this program.       */ 
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* REVISION HISTORY                                                          */
/*                                                                           */
/* - 19/MAY/2003 [Janghwan Kim]  : First writing                             */
/* - 06/OCT/2003 [Janghwan Kim]  : Reorganization                            */
/* - 18/JAN/2006 [WooYoung Yang] : Add OneNAND SysConfig Reg Value           */
/*                                                                           */
/*****************************************************************************/

#ifndef _ONENAND_REGISTER_H_
#define _ONENAND_REGISTER_H_


/*****************************************************************************/
/* OneNAND Base Address Definitions                                          */
/*****************************************************************************/
#define REG_SHIFT  1
#define DAT_SHIFT  2
#define REG_BASE_ADDR(x)            ((x) + (0x0001E000>>REG_SHIFT))


/*****************************************************************************/
/* OneNAND Register Address Definitions                                      */
/*****************************************************************************/
#define ONLD_REG_MANUF_ID(x)        (REG_BASE_ADDR(x) + (0x0000>>REG_SHIFT))
#define ONLD_REG_DEV_ID(x)          (REG_BASE_ADDR(x) + (0x0002>>REG_SHIFT))
#define ONLD_REG_VER_ID(x)          (REG_BASE_ADDR(x) + (0x0004>>REG_SHIFT))
#define ONLD_REG_DATABUF_SIZE(x)    (REG_BASE_ADDR(x) + (0x0006>>REG_SHIFT))
#define ONLD_REG_BOOTBUF_SIZE(x)    (REG_BASE_ADDR(x) + (0x0008>>REG_SHIFT))
#define ONLD_REG_BUF_AMOUNT(x)      (REG_BASE_ADDR(x) + (0x000A>>REG_SHIFT))
#define ONLD_REG_TECH(x)            (REG_BASE_ADDR(x) + (0x000C>>REG_SHIFT))

#define ONLD_REG_START_ADDR1(x)     (REG_BASE_ADDR(x) + (0x0200>>REG_SHIFT))
#define ONLD_REG_START_ADDR2(x)     (REG_BASE_ADDR(x) + (0x0202>>REG_SHIFT))
#define ONLD_REG_START_ADDR3(x)     (REG_BASE_ADDR(x) + (0x0204>>REG_SHIFT))
#define ONLD_REG_START_ADDR4(x)     (REG_BASE_ADDR(x) + (0x0206>>REG_SHIFT))
#define ONLD_REG_START_ADDR5(x)     (REG_BASE_ADDR(x) + (0x0208>>REG_SHIFT))
#define ONLD_REG_START_ADDR6(x)     (REG_BASE_ADDR(x) + (0x020A>>REG_SHIFT))
#define ONLD_REG_START_ADDR7(x)     (REG_BASE_ADDR(x) + (0x020C>>REG_SHIFT))
#define ONLD_REG_START_ADDR8(x)     (REG_BASE_ADDR(x) + (0x020E>>REG_SHIFT))

#define ONLD_REG_START_BUF(x)       (REG_BASE_ADDR(x) + (0x0400>>REG_SHIFT))

#define ONLD_REG_CMD(x)             (REG_BASE_ADDR(x) + (0x0440>>REG_SHIFT))
#define ONLD_REG_SYS_CONF1(x)       (REG_BASE_ADDR(x) + (0x0442>>REG_SHIFT))
#define ONLD_REG_SYS_CONF2(x)       (REG_BASE_ADDR(x) + (0x0444>>REG_SHIFT))

#define ONLD_REG_CTRL_STAT(x)       (REG_BASE_ADDR(x) + (0x0480>>REG_SHIFT))
#define ONLD_REG_INT(x)             (REG_BASE_ADDR(x) + (0x0482>>REG_SHIFT))

#define ONLD_REG_ULOCK_START_BA(x)  (REG_BASE_ADDR(x) + (0x0498>>REG_SHIFT))
#define ONLD_REG_ULOCK_END_BA(x)    (REG_BASE_ADDR(x) + (0x049A>>REG_SHIFT))
#define ONLD_REG_WR_PROTECT_STAT(x) (REG_BASE_ADDR(x) + (0x049C>>REG_SHIFT))

#define ONLD_REG_ECC_STAT(x)        (REG_BASE_ADDR(x) + (0x1E00>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_MB0(x)    (REG_BASE_ADDR(x) + (0x1E02>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_SB0(x)    (REG_BASE_ADDR(x) + (0x1E04>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_MB1(x)    (REG_BASE_ADDR(x) + (0x1E06>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_SB1(x)    (REG_BASE_ADDR(x) + (0x1E08>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_MB2(x)    (REG_BASE_ADDR(x) + (0x1E0A>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_SB2(x)    (REG_BASE_ADDR(x) + (0x1E0C>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_MB3(x)    (REG_BASE_ADDR(x) + (0x1E0E>>REG_SHIFT))
#define ONLD_REG_ECC_RSLT_SB3(x)    (REG_BASE_ADDR(x) + (0x1E10>>REG_SHIFT))

/*****************************************************************************/
/* OneNAND Main Buffer Address                                               */
/*****************************************************************************/
#define OCLD_MAP00_MB_ADDR          (0)
#define OCLD_MAP00_SB_ADDR          (2048)

#define ONLD_BT_MB0_ADDR(x)         ((x) + (0x00000>>REG_SHIFT))
#define ONLD_BT_MB1_ADDR(x)         ((x) + (0x00200>>REG_SHIFT))
#define ONLD_DT_MB00_ADDR(x)        ((x) + (0x00400>>REG_SHIFT))
#define ONLD_DT_MB01_ADDR(x)        ((x) + (0x00600>>REG_SHIFT))
#define ONLD_DT_MB02_ADDR(x)        ((x) + (0x00800>>REG_SHIFT))
#define ONLD_DT_MB03_ADDR(x)        ((x) + (0x00A00>>REG_SHIFT))
#define ONLD_DT_MB10_ADDR(x)        ((x) + (0x00C00>>REG_SHIFT))
#define ONLD_DT_MB11_ADDR(x)        ((x) + (0x00E00>>REG_SHIFT))
#define ONLD_DT_MB12_ADDR(x)        ((x) + (0x01000>>REG_SHIFT))
#define ONLD_DT_MB13_ADDR(x)        ((x) + (0x01200>>REG_SHIFT))

#define ONLD_BT_SB0_ADDR(x)         ((x) + (0x10000>>REG_SHIFT))
#define ONLD_BT_SB1_ADDR(x)         ((x) + (0x10010>>REG_SHIFT))
#define ONLD_DT_SB00_ADDR(x)        ((x) + (0x10020>>REG_SHIFT))
#define ONLD_DT_SB01_ADDR(x)        ((x) + (0x10030>>REG_SHIFT))
#define ONLD_DT_SB02_ADDR(x)        ((x) + (0x10040>>REG_SHIFT))
#define ONLD_DT_SB03_ADDR(x)        ((x) + (0x10050>>REG_SHIFT))
#define ONLD_DT_SB10_ADDR(x)        ((x) + (0x10060>>REG_SHIFT))
#define ONLD_DT_SB11_ADDR(x)        ((x) + (0x10070>>REG_SHIFT))
#define ONLD_DT_SB12_ADDR(x)        ((x) + (0x10080>>REG_SHIFT))
#define ONLD_DT_SB13_ADDR(x)        ((x) + (0x10090>>REG_SHIFT))

/******************************************************************************
  OneNANC Control Register Address Map 
*******************************************************************************/
#define REGu32(x)                   *(volatile UINT32 *)(x)
#define OCLD_REG_MEM_CFG(n)  (*(volatile UINT32*)(n + 0x0))
	#define BIT_ECC		(1<<8)
	#define BIT_RDYPOL		(1<<7)
	#define BIT_INTPOL		(1<<6)
	#define BIT_IOBE		(1<<5)
#define OCLD_REG_BURST_LEN(n)  (*(volatile UINT32*)(n + 0x10))
	#define BL_4WORDS		4
	#define BL_8WORDS		8
	#define BL_16WORDS		16
	#define BL_32WORDS		32
#define OCLD_REG_MEM_RESET(n)  (*(volatile UINT32*)(n + 0x20))
	#define WARMRESET		(1)
	#define CORERESET		(2)
	#define HOTRESET		(3)
#define OCLD_REG_INT_ERR_STAT(n)  (*(volatile UINT32*)(n + 0x30))
#define OCLD_REG_INT_ERR_MASK(n)  (*(volatile UINT32*)(n + 0x40))
#define OCLD_REG_INT_ERR_ACK(n)  (*(volatile UINT32*)(n + 0x50))
	#define BIT_CACHE_OP_ERR		(1<<13)
	#define BIT_RST_CMP			(1<<12)
	#define BIT_RDY_ACT			(1<<11)
	#define BIT_INT_ACT			(1<<10)
	#define BIT_UNSUP_CMD			(1<<9)
	#define BIT_LOCKED_BLK		(1<<8)
	#define BIT_BLK_RW_CMP		(1<<7)
	#define BIT_ERS_CMP			(1<<6)
	#define BIT_PGM_CMP			(1<<5)
	#define BIT_LOAD_CMP			(1<<4)
	#define BIT_ERS_FAIL			(1<<3)
	#define BIT_PGM_FAIL			(1<<2)
	#define BIT_INT_TO				(1<<1)
	#define BIT_LD_FAIL_ECC_ERR	(1<<0)
#define OCLD_REG_ECC_ERR_STAT(n)  (*(volatile UINT32*)(n + 0x60))
#define OCLD_REG_MANUFACT_ID(n)  (*(volatile UINT32*)(n + 0x70))
#define OCLD_REG_DEVICE_ID(n)  (*(volatile UINT32*)(n + 0x80))
#define OCLD_REG_DATA_BUF_SIZE(n)  (*(volatile UINT32*)(n + 0x90))
#define OCLD_REG_BOOT_BUF_SIZE(n)  (*(volatile UINT32*)(n + 0xA0))
#define OCLD_REG_BUF_AMOUNT(n)  (*(volatile UINT32*)(n + 0xB0))
#define OCLD_REG_TECH(n)  (*(volatile UINT32*)(n + 0xC0))
#define OCLD_REG_FBA_WIDTH(n)  (*(volatile UINT32*)(n + 0xD0))
#define OCLD_REG_FPA_WIDTH(n)  (*(volatile UINT32*)(n + 0xE0))
#define OCLD_REG_FSA_WIDTH(n)  (*(volatile UINT32*)(n + 0xF0))
#define OCLD_REG_REVISION(n)  (*(volatile UINT32*)(n + 0x100))
#define OCLD_REG_DATARAM0(n)  (*(volatile UINT32*)(n + 0x110))
#define OCLD_REG_DATARAM1(n)  (*(volatile UINT32*)(n + 0x120))
#define OCLD_REG_SYNC_MODE(n)  (*(volatile UINT32*)(n + 0x130))
	#define OCLD_ASYNC		0
	#define OCLD_SYNC		1
#define OCLD_REG_TRANS_SPARE(n)  (*(volatile UINT32*)(n + 0x140))
	#define TSRF_DATA_ONLY		0
	#define TSRF_DATA_SPARE		1
#define OCLD_REG_LOCK_BIT(n)  (*(volatile UINT32*)(n + 0x150))  // spec-out, not used.
#define OCLD_REG_DBS_DFS_WIDTH(n)  (*(volatile UINT32*)(n + 0x160))
#define OCLD_REG_PAGE_CNT(n)  (*(volatile UINT32*)(n + 0x170))
#define OCLD_REG_ERR_PAGE_ADDR(n)  (*(volatile UINT32*)(n + 0x180))
#define OCLD_REG_BURST_RD_LAT(n)  (*(volatile UINT32*)(n + 0x190))
#define OCLD_REG_INT_PIN_ENABLE(n)  (*(volatile UINT32*)(n + 0x1A0))
	#define USE_STATUS_REG		0
	#define USE_INT_PIN			1
#define OCLD_REG_INT_MON_CYC(n)  (*(volatile UINT32*)(n + 0x1B0))
#define OCLD_REG_ACC_CLOCK(n)  (*(volatile UINT32*)(n + 0x1C0))
#define OCLD_REG_SLOW_RD_PATH(n)  (*(volatile UINT32*)(n + 0x1D0))
#define OCLD_REG_ERR_BLK_ADDR(n)  (*(volatile UINT32*)(n + 0x1E0))
#define OCLD_REG_FLASH_VER_ID(n)  (*(volatile UINT32*)(n + 0x1F0))
#define OCLD_REG_FLASH_AUX_CNTRL(n)	(*(volatile UINT32*)(n + 0x300))	// added on S3C6410
#define OCLD_REG_FLASH_AFIFO_CNT(n) (*(volatile UINT32*)(n + 0x310))


/*****************************************************************************/
/* OneNAND Register Masking values                                           */
/*****************************************************************************/
#define MASK_DFS                       0x8000
#define MASK_FBA                       0x7FFF
#define MASK_DBS                       0x8000
#define MASK_FCBA                      0x7FFF
#define MASK_FCPA                      0x00FC
#define MASK_FCSA                      0x0003
#define MASK_FPA                       0x00FC
#define MASK_BSA                       0x0F00
#define MASK_BSC                       0x0003
#define MASK_FPC                       0x00FF


/*****************************************************************************/
/* OneNAND MAP10 Register Command Set                                        */
/*****************************************************************************/
#define INPROGRESS					1


/*****************************************************************************/
/* OneNAND System Configureation1 Register Values                            */
/*****************************************************************************/
#define SYNC_READ_MODE              0x8000
#define ASYNC_READ_MODE             0x0000

#define BST_RD_LATENCY_8            0x0000      /*   N/A   */
#define BST_RD_LATENCY_9            0x1000      /*   N/A   */
#define BST_RD_LATENCY_10           0x2000      /*   N/A   */
#define BST_RD_LATENCY_3            0x3000      /*   min   */
#define BST_RD_LATENCY_4            0x4000      /* default */
#define BST_RD_LATENCY_5            0x5000      
#define BST_RD_LATENCY_6            0x6000
#define BST_RD_LATENCY_7            0x7000

#define BST_LENGTH_CONT             0x0000      /* default */
#define BST_LENGTH_4WD              0x0200
#define BST_LENGTH_8WD              0x0400
#define BST_LENGTH_16WD             0x0600
#define BST_LENGTH_32WD             0x0800      /* N/A on spare */
#define BST_LENGTH_1KWD             0x0A00      /* N/A on spare, sync. burst block read only */

#define CONF1_ECC_ON                0xFEFF
#define CONF1_ECC_OFF               0x0100      //(~CONF1_ECC_ON)   //0x0100

#define RDY_POLAR                   0x0080
#define INT_POLAR                   0x0040
#define IOBE_ENABLE                 0x0020

#define BWPS_UNLOCKED               0x0001

#define HF_ON                0x0004
#define HF_OFF               0xFFFB
#define RDY_CONF             0x0010

/*****************************************************************************/
/* OneNAND Controller Status Register Values                                 */
/*****************************************************************************/
#define CTRL_ONGO                   0x8000
#define LOCK_STATE                  0x4000
#define LOAD_STATE                  0x2000
#define PROG_STATE                  0x1000
#define ERASE_STATE					0x0800
#define ERROR_STATE					0x0400
#define SUSPEND_STATE				0x0200
#define RESET_STATE					0x0080
#define OTPL_STATE					0x0040
#define TIME_OUT                    0x0001

#define PROG_LOCK					(LOCK_STATE | PROG_STATE | ERROR_STATE)
#define ERASE_LOCK					(LOCK_STATE | ERASE_STATE | ERROR_STATE)
#define PROG_FAIL					(PROG_STATE | ERROR_STATE)
#define ERASE_FAIL					(ERASE_STATE | ERROR_STATE)

/*****************************************************************************/
/* OneNAND Controller Interrupt Status Register Values                       */
/*****************************************************************************/
#define CINT_CLEAR                   0x0000
#define CINT_MASK                    0xFFFF

#define CPEND_INT                    (BIT_INT_ACT)
#define CPEND_READ                   (BIT_INT_ACT | BIT_BLK_RW_CMP | BIT_LOAD_CMP)
#define CPEND_WRITE                  (BIT_INT_ACT | BIT_BLK_RW_CMP | BIT_PGM_CMP)
#define CPEND_ERASE                  (BIT_INT_ACT | BIT_ERS_CMP)
#define CPEND_RESET                  (BIT_INT_ACT | BIT_RST_CMP)
#define CPEND_LOAD                   (BIT_INT_ACT | BIT_LOAD_CMP)

/*****************************************************************************/
/* OneNAND Write Protection Status Register Values                           */
/*****************************************************************************/
#define UNLOCKED_STAT               0x0004
#define LOCKED_STAT                 0x0002
#define LOCK_TIGHTEN_STAT           0x0001


/*****************************************************************************/
/* OneNAND ECC Status Register Valuies                                       */
/*****************************************************************************/
#define ECC_SB0_NO_ERR              0x0000
#define ECC_SB0_1BIT_ERR            0x0001
#define ECC_SB0_2BIT_ERR            0x0002
#define ECC_MB0_NO_ERR              0x0000
#define ECC_MB0_1BIT_ERR            0x0004
#define ECC_MB0_2BIT_ERR            0x0008

#define ECC_SB1_NO_ERR              0x0000
#define ECC_SB1_1BIT_ERR            0x0010
#define ECC_SB1_2BIT_ERR            0x0020
#define ECC_MB1_NO_ERR              0x0000
#define ECC_MB1_1BIT_ERR            0x0040
#define ECC_MB1_2BIT_ERR            0x0080

#define ECC_SB2_NO_ERR              0x0000
#define ECC_SB2_1BIT_ERR            0x0100
#define ECC_SB2_2BIT_ERR            0x0200
#define ECC_MB2_NO_ERR              0x0000
#define ECC_MB2_1BIT_ERR            0x0400
#define ECC_MB2_2BIT_ERR            0x0800

#define ECC_SB3_NO_ERR              0x0000
#define ECC_SB3_1BIT_ERR            0x1000
#define ECC_SB3_2BIT_ERR            0x2000
#define ECC_MB3_NO_ERR              0x0000
#define ECC_MB3_1BIT_ERR            0x4000
#define ECC_MB3_2BIT_ERR            0x8000

#define ECC_ANY_2BIT_ERR            0xAAAA
#define ECC_ANY_BIT_ERR             0xFFFF
#define ECC_MAIN_BIT_ERR            0xCCCC
#define ECC_SPARE_BIT_ERR           0x3333
#define ECC_REG_CLEAR               0x0000


/*****************************************************************************/
/* OneNAND Misc Values                                                       */
/*****************************************************************************/
#define SECTOR0                     0x0000
#define SECTOR1                     0x0001
#define SECTOR2                     0x0002
#define SECTOR3                     0x0003

#define DATA_BUF0                   0x0000
#define DATA_BUF1                   0x0001

#define SECTOR0_OFFSET              0x0000
#define SECTOR1_OFFSET              0x0200
#define SECTOR2_OFFSET              0x0400
#define SECTOR3_OFFSET              0x0600

#define VALID_BLK_MARK              0xFFFF

#endif /* _ONENAND_REGISTER_H_ */
