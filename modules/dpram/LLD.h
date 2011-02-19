/*****************************************************************************/
/*                                                                           */
/* PROJECT : AnyStore II                                                     */
/* MODULE  : LLD                                                             */
/* NAME    : Low level Device Driver header                                  */
/* FILE    : LLD.h                                                           */
/* PURPOSE : LLD return value definitions                                    */
/*           LFT element definitions                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*          COPYRIGHT 2003-2008, SAMSUNG ELECTRONICS CO., LTD.               */
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
/*   01-OCT-2003 [Janghwan Kim] : first writing                              */
/*   06-OCT-2003 [Janghwan Kim] : code modification                          */
/*   08-JAN-2004 [Janghwan Kim] : Add LLD_PREV_OP_RESULT                     */
/*   06-MAY-2004 [SongHo Yoon]  : reorganization                             */
/* - 25/MAR/2005 [Younwon Park] : LLD code merge                             */
/*                                512M M/A-die, 1G DDP, 1G M-die, 2G DDP     */
/*                                256M M/A-die, 128M M-die                   */
/*                                                                           */
/*****************************************************************************/

#ifndef _XSR_LLD_H_
#define _XSR_LLD_H_

#include "XsrTypes.h"

/*****************************************************************************/
/* Debug Configurations                                                      */
/*****************************************************************************/
#define LLD_RTL_PRINT(x)            XSR_RTL_PRINT(x)

#if defined(LLD_DEBUG)
#define LLD_DBG_PRINT(x)            XSR_DBG_PRINT(x)
#else
#define LLD_DBG_PRINT(x)
#endif  /* LLD_DEBUG */


/*****************************************************************************/
/* External Constant Definitions                                             */
/*****************************************************************************/
#define LLD_MAIN_SIZE               512
#define LLD_SPARE_SIZE              16
#define LLD_SECTOR_SIZE             (LLD_MAIN_SIZE)

#define LLD_BW_X08                  0
#define LLD_BW_X16                  1
#define LLD_BW_X32                  2

#define LLD_ME_NO                   0
#define LLD_ME_OK                   1

#define LLD_MEBL_LEN                16

/*****************************************************************************/
/* LLD Return Values                                                         */
/*****************************************************************************/
/*--------------------*/
/* Major Return Value */
/*--------------------*/
#define	LLD_READ_DISTURBANCE		XSR_RETURN_VALUE(0, 0x0007, 0x0000)		 /* 1-Bit error by read disturbance */																			 
#define LLD_IOC_NOT_SUPPORT         XSR_RETURN_VALUE(0, 0x0006, 0x0000)      /* Not supported IO Ctrl           */
#define LLD_DEV_NOT_OPENED          XSR_RETURN_VALUE(0, 0x0005, 0x0000)      /* Device not opened               */
#define LLD_DEV_POWER_OFF           XSR_RETURN_VALUE(0, 0x0004, 0x0000)      /* Power is not supported to LLD   */
#define LLD_DEV_NO_LLD_FUNC         XSR_RETURN_VALUE(0, 0x0003, 0x0000)
#define LLD_INIT_BADBLOCK           XSR_RETURN_VALUE(0, 0x0002, 0x0000)
#define LLD_INIT_GOODBLOCK          XSR_RETURN_VALUE(0, 0x0001, 0x0000)
#define LLD_SUCCESS                 XSR_RETURN_VALUE(0, 0x0000, 0x0000)      /* success                         */
#define LLD_ERASE_ERROR             XSR_RETURN_VALUE(1, 0x0001, 0x0000)      /* erase error                     */ 
#define LLD_MERASE_ERROR            XSR_RETURN_VALUE(1, 0x0002, 0x0000)      /* merase error                    */ 
#define LLD_WRITE_ERROR             XSR_RETURN_VALUE(1, 0x0003, 0x0000)      /* write error                     */
#define LLD_READ_ERROR              XSR_RETURN_VALUE(1, 0x0005, 0x0000)      /* read error                      */
#define LLD_CRITICAL_ERROR          XSR_RETURN_VALUE(1, 0x0008, 0x0000)
#define LLD_WR_PROTECT_ERROR        XSR_RETURN_VALUE(1, 0x0009, 0x0000)
#define LLD_ILLEGAL_ACCESS          XSR_RETURN_VALUE(1, 0x000A, 0x0000)      /* Invalid op attempts or values   */
#define LLD_ALREADY_INITIALIZED		XSR_RETURN_VALUE(1, 0x000B, 0x0000)
#define LLD_SPARE_WRITE_ERROR       XSR_RETURN_VALUE(1, 0x0080, 0x0000)

#define LLD_INIT_FAILURE            XSR_RETURN_VALUE(1, 0x0011, 0x0000)
#define LLD_OPEN_FAILURE            XSR_RETURN_VALUE(1, 0x0012, 0x0000)
#define LLD_CLOSE_FAILURE           XSR_RETURN_VALUE(1, 0x0013, 0x0000)

/*-------------------*/
/* Minor Retun Value */
/*-------------------*/
/* Detailed Read Error Specifiations */
/* READ_U/CERROR_XX is combined with LLD_READ_ERROR as Minor Return Value    */
#define LLD_READ_CERROR_S0          XSR_RETURN_VALUE(0, 0x0000, 0x0001)
#define LLD_READ_UERROR_S0          XSR_RETURN_VALUE(0, 0x0000, 0x0002)
#define LLD_READ_CERROR_M0          XSR_RETURN_VALUE(0, 0x0000, 0x0004)
#define LLD_READ_UERROR_M0          XSR_RETURN_VALUE(0, 0x0000, 0x0008)

#define LLD_READ_CERROR_S1          XSR_RETURN_VALUE(0, 0x0000, 0x0010)
#define LLD_READ_UERROR_S1          XSR_RETURN_VALUE(0, 0x0000, 0x0020)
#define LLD_READ_CERROR_M1          XSR_RETURN_VALUE(0, 0x0000, 0x0040)
#define LLD_READ_UERROR_M1          XSR_RETURN_VALUE(0, 0x0000, 0x0080)

#define LLD_READ_CERROR_S2          XSR_RETURN_VALUE(0, 0x0000, 0x0100)
#define LLD_READ_UERROR_S2          XSR_RETURN_VALUE(0, 0x0000, 0x0200)
#define LLD_READ_CERROR_M2          XSR_RETURN_VALUE(0, 0x0000, 0x0400)
#define LLD_READ_UERROR_M2          XSR_RETURN_VALUE(0, 0x0000, 0x0800)

#define LLD_READ_CERROR_S3          XSR_RETURN_VALUE(0, 0x0000, 0x1000)
#define LLD_READ_UERROR_S3          XSR_RETURN_VALUE(0, 0x0000, 0x2000)
#define LLD_READ_CERROR_M3          XSR_RETURN_VALUE(0, 0x0000, 0x4000)
#define LLD_READ_UERROR_M3          XSR_RETURN_VALUE(0, 0x0000, 0x8000)

/* Previous Operation Flag */
/* LLD_PREV_OP_RESULT is combined with Major Return Value, and means 
   Previous Operation Error except LLD_READ_ERROR */
#define LLD_PREV_OP_RESULT          XSR_RETURN_VALUE(0, 0x0000, 0xF0F0)


/*****************************************************************************/
/* Flags                                                                     */
/*****************************************************************************/
#define LLD_FLAG_ASYNC_OP           (1 << 0)            /* Write/Erase/Copy  */
#define LLD_FLAG_SYNC_OP            (0 << 0)            /* Write/Erase/Copy  */
#define LLD_FLAG_SYNC_OP_INV        (~LLD_FLAG_ASYNC_OP)/* Write/Erase/Copy  */

#define LLD_FLAG_ECC_ON             (1 << 1)
#define LLD_FLAG_ECC_OFF            (0 << 1)
#define LLD_FLAG_ECC_OFF_INV        (~LLD_FLAG_ECC_ON)

/*****************************************************************************/
/* LLD IO Ctrl Code                                                          */
/*****************************************************************************/
/* IOC Code */
#define IOC_SET_SECURE_LT           0x01    /* Set Lock-Tighten     */
#define IOC_GET_SECURE_STAT         0x02    /* Get Secure Status    */
#define IOC_RESET_NAND_DEV          0x03    /* Reset NAND Device    */
#define IOC_SET_BLOCK_LOCK          0x04    /* Block Lock           */

/* IOC Return Value */
#define LLD_IOC_SECURE_LT           0x01    /* Lock-Tight           */
#define LLD_IOC_SECURE_LS           0x02    /* Locked Status        */
#define LLD_IOC_SECURE_US           0x04    /* Unlocked Status      */


/* Set Device lock-tight */
#define LLD_IOC_SET_SECURE_LT       XSR_IOCTL_CODE(XSR_MODULE_LLD,          \
                                                   IOC_SET_SECURE_LT,       \
                                                   XSR_METHOD_INOUT_DIRECT, \
                                                   XSR_ANY_ACCESS)

/* Set Device lock-tight */
#define LLD_IOC_SET_BLOCK_LOCK      XSR_IOCTL_CODE(XSR_MODULE_LLD,          \
                                                   IOC_SET_BLOCK_LOCK,      \
                                                   XSR_METHOD_INOUT_DIRECT, \
                                                   XSR_ANY_ACCESS)
/* Get Device security status 
   return value 
      DEV_LT : Device is lock-tighten
      DEV_AL : All blocks of device are locked
      DEV_UL : There is an unlocked block       */
#define LLD_IOC_GET_SECURE_STAT     XSR_IOCTL_CODE(XSR_MODULE_LLD,          \
                                                   IOC_GET_SECURE_STAT,     \
                                                   XSR_METHOD_INOUT_DIRECT, \
                                                   XSR_READ_ACCESS)
/* Reset Nand Device */
#define LLD_IOC_RESET_NAND_DEV      XSR_IOCTL_CODE(XSR_MODULE_LLD,          \
                                                   IOC_RESET_NAND_DEV,      \
                                                   XSR_METHOD_INOUT_DIRECT, \
                                                   XSR_ANY_ACCESS)

/*****************************************************************************/
/* Data Structures                                                           */
/*****************************************************************************/
typedef struct {                /*                               offset size */ 
    UINT8   nMCode;             /* Manufacturer Code              0x00   1   */
    UINT8   nDCode;             /* Device Code                    0x01   1   */
    UINT16  nNumOfBlks;         /* The Number of Blocks           0x02   2   */
                                                                           
    UINT16  nPgsPerBlk;         /* The Number of Pages per Block  0x04   2   */
    UINT16  nBlksInRsv;         /* The Number of Blocks           0x06   2  
                                   in Reserved Block Pool                    */

    UINT8   nSctsPerPg;         /* The Number of Sectors per Page 0x08   1   */
    UINT8   nNumOfPlane;        /* The Number of Plane            0x09   1   */
    UINT8   nMEFlag;            /* Multiple Erase Flag            0x0A   1   */
                                                                           
    UINT8   nBWidth;            /* Device Bus Width               0x0C   1   */
    UINT8   nBadPos;            /* BadBlock Information Position  0x0E   1   */
    UINT8   nLsnPos;            /* LSN Position                   0x0E   1   */
    UINT8   nEccPos;            /* ECC Value Position             0x0F   1   */

    UINT8   aUID[XSR_UID_SIZE]; /* UID                            0x10   16, 
                                   0xFF(absence case of UID)                 */

                                /* Op Time based on nano second              */
    UINT32  nTrTime;            /* Physical Read Operation Time   0x20   2   */
    UINT32  nTwTime;            /* Physical Write Operation Time  0x22   2   */
    UINT32  nTeTime;            /* Physical Erase Operation Time  0x24   2   */
    UINT32  nTfTime;            /* Data Transfer Time             0x26   2   */
} LLDSpec;

typedef struct {
    UINT16 nMEListSbn;          /* MEList Semi-physical Block Number         */
    UINT16 nMEListPbn;          /* MEList Physical Block Number              */
} LLDMEList;

typedef struct {
    LLDMEList  *pstMEList;      /* Pointer to LLDMEList                      */
    UINT16      nNumOfMList;    /* Number of Entries of LLDMEList            */
    UINT16      nBitMapErr;     /* Error Bitmap Position of MEList           */
    BOOL32      bFlag;          /* Valid Flag                                */
} LLDMEArg;

typedef struct
{
    UINT32  nType;              /* 0 : OneNAND, 1 : read ID from NAND directly  */
                                /* 2 : read ID from register of NAND controller */
    UINT32  nAddrOfCmdReg;      /* Address of command register                  */
    UINT32  nAddrOfAdrReg;      /* Address of address register                  */
    UINT32  nAddrOfReadIDReg;   /* Address of register for reading ID           */
    UINT32  nAddrOfStatusReg;   /* Address of status register                   */
    UINT32  nCmdOfReadID;       /* Command of reading Device ID                 */
    UINT32  nCmdOfReadPage;     /* Command of read page                         */ 
    UINT32  nCmdOfReadStatus;   /* Command of read status                       */
    UINT32  nMaskOfRnB;         /* Mask value for Ready or Busy status          */
} LLDPlatformInfo;

/*****************************************************************************/
/* LLD Function Table Data Structures                                        */
/*****************************************************************************/
typedef struct {
    INT32 (*Init)           (VOID  *pParm);
    INT32 (*Open)           (UINT32 nDev);   
    INT32 (*Close)          (UINT32 nDev);   
    INT32 (*Read)           (UINT32 nDev,  UINT32 nPsn,    UINT32 nScts, 
                             UINT8 *pMBuf, UINT8 *pSBuf,   UINT32 nFlag);
    INT32 (*Write)          (UINT32 nDev,  UINT32 nPsn,    UINT32 nScts, 
                             UINT8 *pMBuf, UINT8 *pSBuf,   UINT32 nFlag);
    INT32 (*MRead)          (UINT32 nDev,  UINT32 nPsn,    UINT32 nScts, 
                             SGL  *pstSGL, UINT8 *pSBuf,   UINT32 nFlag);
    INT32 (*MWrite)         (UINT32 nDev,  UINT32 nPsn,    UINT32 nScts, 
                             SGL  *pstSGL, UINT8 *pSBuf,   UINT32 nFlag,
							 UINT32 *pErrPsn);
	INT32 (*CopyBack)       (UINT32 nDev,  CpBkArg *pstCpArg, 
                             UINT32 nFlag);
    INT32 (*Erase)          (UINT32 nDev,  UINT32 Pbn,     UINT32 nFlag);
    INT32 (*GetDevInfo)     (UINT32 nDev,  LLDSpec *pstDevInfo);
    INT32 (*ChkInitBadBlk)  (UINT32 nDev,  UINT32 Pbn);
    INT32 (*FlushOp)        (UINT32 nDev); 
    INT32 (*SetRWArea)      (UINT32 nDev,  UINT32 nSUbn,   UINT32 nUBlks);
    INT32 (*GetPrevOpData)  (UINT32 nDev,  UINT8 *pMBuf,   UINT8 *pSBuf);
    INT32 (*IOCtl)          (UINT32 nDev,  UINT32 nCmd,
                             UINT8 *pBufI, UINT32 nLenI,
                             UINT8 *pBufO, UINT32 nLenO,
                             UINT32 *pByteRet);

    INT32 (*MErase)         (UINT32 nDev,  LLDMEArg *pstMEArg, 
                             UINT32 nFlag);
    INT32 (*EraseVerify)    (UINT32 nDev,  LLDMEArg *pstMEArg, 
                             UINT32 nFlag);
    VOID  (*GetPlatformInfo)(LLDPlatformInfo*   pstLLDPlatformInfo);
} LowFuncTbl;

#endif /* _XSR_LLD_H_ */
