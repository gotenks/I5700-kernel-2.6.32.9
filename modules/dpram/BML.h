/*****************************************************************************/
/*                                                                           */
/* PROJECT : AnyStore II                                                     */
/* MODULE  : XSR BML                                                         */
/* NAME    : BML header file                                                 */
/* FILE    : BML.h                                                           */
/* PURPOSE : This file contains the definition and protypes of exported      */
/*           functions for Block Management Layer.                           */
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
/*   11-JUL-2003 [SongHo Yoon]: First writing                                */
/*   05-AUG-2003 [SongHo Yoon]: added nNumOfUsBlks into BMLVolSpec structure */
/*   16-AUG-2003 [SongHo Yoon]: replaced BML_ILLEGAL_ACCESS as               */
/*                              BML_INVALID_VOLUME                           */
/*   02-DEC-2003 [SongHo Yoon]: added parameter(nFlag) in BML_Format         */
/*   02-DEC-2003 [SongHo Yoon]: added BML_INIT_FORMAT/BML_REPARTITION        */
/*   04-DEC-2003 [SongHo Yoon]: changed members of XSRPartEntry structure    */
/*   13-DEC-2003 [SongHo Yoon]: added BML_IOCTL_XXX / BML_UNSUPPORTED_IOCTL  */
/*   13-DEC-2003 [SongHo Yoon]: changed return code                          */
/*                        BML_STORE_PEXTINFO_ERROR ==> BML_STORE_PIEXT_ERROR */
/*                        BML_LOAD_PEXTINFO_ERROR  ==> BML_LOAD_PIEXT_ERROR  */
/*   15-DEC-2003 [SongHo Yoon]: removed BML_ProtectROArea()                  */
/*   27-DEC-2003 [SongHo Yoon]: reorganization of BML return value           */
/*   08-JAN-2004 [SongHo Yoon]: added BML_PREV_OP_RESULT                     */
/*   13-JAN-2004 [SongHo Yoon]: added BML_FLAG_ECC_MASK / BML_FLAG_SYNC_MASK */
/*                                    BML_FLAG_GBBM_MASK                     */
/*   16-JAN-2004 [SongHo Yoon]: added BML_IOCTL_GET_FULL_PI                  */
/*   28-JAN-2004 [SongHo Yoon]: changed PARTITION_ID_FATFILESYSTEM to        */
/*                                       PARTITION_ID_FILESYSTEM             */
/*   02-FEB-2004 [SongHo Yoon]: removed BML_VOLUME_ALREADY_OPENED            */
/*                              added BML_DEVICE_ACCESS_ERROR                */
/*   12-FEB-2004 [SongHo Yoon]: changed BML_INIT_ALREADY_CALLED to           */
/*                                      BML_ALREADY_INITIALIZED              */
/*   17-FEB-2004 [SongHo Yoon]: added BML_ALREADY_OPENED                     */
/*   21-APR-2004 [SongHo Yoon]: added CopyArg structure                      */
/*                              added BML_Copy()                             */
/*   06-MAY-2004 [SongHo Yoon]: removed BML_GetPlaneNum()                    */
/*   07-SEP-2004 [Janghwan Kim] : changed type of OpTime variables of        */
/*                                BMLVolSpec structure (UINT16->UINT32)      */
/*   07-SEP-2004 [Janghwan Kim] : added BML_MEraseBlk()                      */
/*   10-SEP-2004 [Janghwan Kim] : added nEccPos in BMLVolSpec structure      */
/*   25-MAR-2005 [MinYoung Kim] : added definitions for Partition Entry ID   */
/*	 31-JAN-2005 [ByoungYoung Ahn]	: added BML_GetDevInfo()				 */
/*	 31-JAN-2006 [ByoungYoung Ahn] 	: added BML_IOCTL_SET_BLOCK_UNLOCK 		 */
/*   14-FEB-2006 [WooYoung Yang] : added BML_GetBBInfo()                     */
/*                                                                           */
/*****************************************************************************/

#ifndef _XSR_BML_H_
#define _XSR_BML_H_

#include "LLD.h"

/*****************************************************************************/
/* Common Constant definitions                                               */
/*****************************************************************************/
#define     BML_MAX_REQSEC                  (4)
#define     BML_MAX_PARTENTRY               ((512 - 16) / 16)
#define     BML_MAX_PART_SIG                (8)
#define		BML_MAX_ERL_PROCESS				(16)

/*****************************************************************************/
/* nFlag value of BML_ReadSector, BML_WriteSector,                           */
/*                BML_CopyBack, BML_EraseBlk                                 */
/*****************************************************************************/
#define     BML_FLAG_SYNC_MASK              (1 << 0)
#define     BML_FLAG_ASYNC_OP               (1 << 0)    /* Write/Erase/Copy  */
#define     BML_FLAG_SYNC_OP                (0 << 0)    /* Write/Erase/Copy  */
#define     BML_FLAG_SYNC_OP_INV            (~BML_FLAG_ASYNC_OP) 

#define     BML_FLAG_ECC_MASK               (1 << 1)
#define     BML_FLAG_ECC_ON                 (1 << 1)    /* Read/Write/Copy   */
#define     BML_FLAG_ECC_OFF                (0 << 1)    /* Read/Write/Copy   */
#define     BML_FLAG_ECC_OFF_INV            (~BML_FLAG_ECC_ON)

#define     BML_FLAG_BBM_MASK               (1 << 2)
#define     BML_FLAG_BBM_OFF                (1 << 2)    /* Read              */
#define     BML_FLAG_BBM_ON                 (0 << 2)    /* Read              */
#define     BML_FLAG_BBM_ON_INV             (~BML_FLAG_BBM_OFF)

/*****************************************************************************/
/* nFlag value of BML_FlushOp                                                */
/*****************************************************************************/
#define     BML_NORMAL_MODE                 (0 << 3)
#define     BML_EMERGENCY_MODE              (1 << 3)

/*****************************************************************************/
/* Return value of BML_XXX()                                                 */
/*****************************************************************************/
#define     BML_SUCCESS                     XSR_RETURN_VALUE(0, 0x0000, 0x0000)
#define     BML_CRITICAL_ERROR              XSR_RETURN_VALUE(1, 0x0001, 0x0000)
#define     BML_READ_ERROR                  XSR_RETURN_VALUE(1, 0x0002, 0x0000)
#define     BML_INVALID_DATA_ERROR          XSR_RETURN_VALUE(1, 0x0003, 0x0000)
#define     BML_ALREADY_OPENED              XSR_RETURN_VALUE(1, 0x0004, 0x0000)
#define     BML_WR_PROTECT_ERROR            XSR_RETURN_VALUE(1, 0x0005, 0x0000)
#define     BML_INVALID_PARAM               XSR_RETURN_VALUE(1, 0x0006, 0x0000)
#define     BML_NO_PIENTRY                  XSR_RETURN_VALUE(1, 0x0007, 0x0000)
#define     BML_STORE_PIEXT_ERROR           XSR_RETURN_VALUE(1, 0x0008, 0x0000)
#define     BML_MAKE_RSVR_ERROR             XSR_RETURN_VALUE(1, 0x0009, 0x0000)
#define     BML_DEVICE_ACCESS_ERROR         XSR_RETURN_VALUE(1, 0x000A, 0x0000)
#define     BML_UNFORMATTED                 XSR_RETURN_VALUE(1, 0x000B, 0x0000)
#define     BML_UNSUPPORTED_IOCTL           XSR_RETURN_VALUE(1, 0x000C, 0x0000)
#define     BML_CANT_LOCK_FOREVER           XSR_RETURN_VALUE(1, 0x000D, 0x0000)
#define     BML_CANT_UNLOCK_WHOLEAREA       XSR_RETURN_VALUE(1, 0x000E, 0x0000)
#define     BML_VOLUME_ALREADY_LOCKTIGHT    XSR_RETURN_VALUE(1, 0x000F, 0x0000)
#define     BML_CANT_CHANGE_PART_ATTR       XSR_RETURN_VALUE(1, 0x0010, 0x0000)
#define     BML_VOLUME_NOT_OPENED           XSR_RETURN_VALUE(1, 0x0011, 0x0000)
#define     BML_ALREADY_INITIALIZED         XSR_RETURN_VALUE(1, 0x0012, 0x0000)
#define     BML_ACQUIRE_SM_ERROR            XSR_RETURN_VALUE(1, 0x0013, 0x0000)
#define     BML_RELEASE_SM_ERROR            XSR_RETURN_VALUE(1, 0x0014, 0x0000)
#define     BML_OAM_ACCESS_ERROR            XSR_RETURN_VALUE(1, 0x0015, 0x0000)
#define     BML_PAM_ACCESS_ERROR            XSR_RETURN_VALUE(1, 0x0016, 0x0000)
#define     BML_CANT_LOCK_BLOCK             XSR_RETURN_VALUE(1, 0x0017, 0x0000)
#define		BML_ERASE_REFRESH_FAIL			XSR_RETURN_VALUE(1, 0x0018, 0x0000)
#define		BML_CANT_UNLOCK_BLOCK			XSR_RETURN_VALUE(1, 0x0019, 0x0000)
#define		BML_SKIP_ERASE_REFRESH			XSR_RETURN_VALUE(1, 0x001A, 0x0000)

/*****************************************************************************/
/* Minor Return value of BML_READ_ERROR and BML_INVALID_DATA_ERROR           */
/*****************************************************************************/
#define     BML_READ_ERROR_S0               XSR_RETURN_VALUE(0, 0x0000, 0x0002)
#define     BML_READ_ERROR_M0               XSR_RETURN_VALUE(0, 0x0000, 0x0008)
#define     BML_READ_ERROR_S1               XSR_RETURN_VALUE(0, 0x0000, 0x0020)
#define     BML_READ_ERROR_M1               XSR_RETURN_VALUE(0, 0x0000, 0x0080)
#define     BML_READ_ERROR_S2               XSR_RETURN_VALUE(0, 0x0000, 0x0200)
#define     BML_READ_ERROR_M2               XSR_RETURN_VALUE(0, 0x0000, 0x0800)
#define     BML_READ_ERROR_S3               XSR_RETURN_VALUE(0, 0x0000, 0x2000)
#define     BML_READ_ERROR_M3               XSR_RETURN_VALUE(0, 0x0000, 0x8000)

/* Previous Operation Flag */
/* BML_PREV_OP_RESULT is combined with BML_WR_PROTECT_ERROR, and means 
   Previous Operation Error of BML_WR_PROTECT_ERROR */
#define     BML_PREV_OP_RESULT              XSR_RETURN_VALUE(0, 0x0000, 0xF0F0)

/*****************************************************************************/
/* Partition Entry ID of BML_LoadPIEntry()                                   */
/* Partition Entry ID from 0 to 0x0FFFFFFF is reserved in BML                */
/* Following ID is the pre-defined value and User can use Partition Entry ID */
/* from PARTITION_USER_DEF_BASE                                              */
/*****************************************************************************/
#define     PARTITION_ID_NBL1               0  /* NAND bootloader stage 1    */
#define     PARTITION_ID_NBL2               1  /* NAND bootloader stage 2    */
#define     PARTITION_ID_NBL3               2  /* NAND bootloader stage 3    */
#define     PARTITION_ID_COPIEDOS           3  /* OS image copied from NAND 
                                                  flash memory to RAM        */
#define     PARTITION_ID_DEMANDONOS         4  /* OS image loaded on demand  */

#define     PARTITION_ID_FILESYSTEM         8  /* file system 0              */
#define     PARTITION_ID_FILESYSTEM1        9  /* file system 1              */
#define     PARTITION_ID_FILESYSTEM2        10 /* file system 2              */
#define     PARTITION_ID_FILESYSTEM3        11 /* file system 3              */
#define     PARTITION_ID_FILESYSTEM4        12 /* file system 4              */
#define     PARTITION_ID_FILESYSTEM5        13 /* file system 5              */
#define     PARTITION_ID_FILESYSTEM6        14 /* file system 6              */
#define     PARTITION_ID_FILESYSTEM7        15 /* file system 7              */
#define     PARTITION_ID_FILESYSTEM8        16 /* file system 8              */
#define     PARTITION_ID_FILESYSTEM9        17 /* file system 9              */


#define     PARTITION_USER_DEF_BASE         0x10000000 /* partition id base for
                                                  user definition            */

/*****************************************************************************/
/* nFlag of BML_Format()                                                     */
/*****************************************************************************/
#define     BML_INIT_FORMAT                 0
#define     BML_REPARTITION                 1

/*****************************************************************************/
/* value of nAttr of XSRPartEntry structure                                  */
/* nAttr can be 'BML_PI_ATTR_FROZEN + BML_PI_ATTR_RO' or                     */
/*              'BML_PI_ATTR_RO'                      or                     */
/*              'BML_PI_ATTR_RW'.                                            */
/* other value is invalid attribute.                                         */
/*****************************************************************************/
#define     BML_PI_ATTR_FROZEN              0x00000020
#define     BML_PI_ATTR_RO                  0x00000002
#define     BML_PI_ATTR_RW                  0x00000001

/*****************************************************************************/
/* maximum nSizeOfData of XSRPIExt structure                                 */
/*****************************************************************************/
#define     BML_MAX_PIEXT_DATA              (504)

/*****************************************************************************/
/* BML IO Control Code                                                       */
/*****************************************************************************/

#if defined(XSR_NW)
/*****************************************************************************/
/*  BML_IOCtl(nVol, BML_IOCTL_UNLOCK_WHOLEAREA, NULL, 0, NULL, 0, &nRet)     */
/*****************************************************************************/
#define     BML_IOCTL_UNLOCK_WHOLEAREA      XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    0,                      \
                                                    XSR_METHOD_IN_DIRECT,   \
                                                    XSR_WRITE_ACCESS)

#endif /* XSR_NW */
/*****************************************************************************/
/*  UINT32  nSbn;                                                            */
/*  UINT32  nBlks;                                                           */
/*  UINT32  nRet;                                                            */
/*  UINT32  nVol;                                                            */
/*  UINT8  *pBufIn;                                                          */
/*  UINT32  nDevIdx;                                                         */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nDevIdx   = 0;                                                           */
/*                                                                           */
/*  *(UINT32*)pBufIn         = nDevIdx;                                      */
/*  *((UINT32*)(pBufIn + 4)) = nSbn;                                         */
/*  *((UINT32*)(pBufIn + 8)) = nBlks;                                        */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_SET_BLOCK_UNLOCK, pBufIn, 0,                   */
/*                                              NULL, 0, &nRet)              */
/*****************************************************************************/
#define     BML_IOCTL_SET_BLOCK_UNLOCK        XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    1,                      \
                                                    XSR_METHOD_BUFFERED,    \
                                                    XSR_WRITE_ACCESS)

/*****************************************************************************/
/*  UINT32  nSbn;                                                            */
/*  UINT32  nBlks;                                                           */
/*  UINT32  nRet;                                                            */
/*  UINT32  nVol;                                                            */
/*  UINT8  *pBufIn;                                                          */
/*  UINT32  nDevIdx;                                                         */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nDevIdx   = 0;                                                           */
/*                                                                           */
/*  *(UINT32*)pBufIn         = nDevIdx;                                      */
/*  *((UINT32*)(pBufIn + 4)) = nSbn;                                         */
/*  *((UINT32*)(pBufIn + 8)) = nBlks;                                        */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_LOCK_FOREVER, pBufIn, 0,                       */
/*                                              NULL, 0, &nRet)              */
/*****************************************************************************/
#define     BML_IOCTL_LOCK_FOREVER          XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    2,                      \
                                                    XSR_METHOD_IN_DIRECT,   \
                                                    XSR_WRITE_ACCESS)

/*****************************************************************************/
/*  UINT32  nPartI[2];                                                       */
/*  UINT32  nRet;                                                            */
/*  UINT32  nVol;                                                            */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nPartI[0] = PARTITION_ID_COPIEDOS;                                       */
/*  nPartI[1] = BML_PI_ATTR_RO;                                              */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_CHANGE_PART_ATTR, (UINT8 *) &nPartI,           */
/*            sizeof(UINT32) * 2, NULL, 0, &nRet)                            */
/*****************************************************************************/
#define     BML_IOCTL_CHANGE_PART_ATTR      XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    3,                      \
                                                    XSR_METHOD_IN_DIRECT,   \
                                                    XSR_WRITE_ACCESS)

/*****************************************************************************/
/*  UINT32  nDevIdx;                                                         */
/*  UINT16  nBMF[512];                                                       */
/*  UINT32  nRet;                                                            */
/*  UINT32  nVol;                                                            */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nDevIdx   = 0;                                                           */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_GET_BMI, (UINT8 *) &nDevIdx, sizeof(UINT32),   */
/*            (UINT8 *) nBMI, sizeof(UINT32) * 256, &nRet)                   */
/*****************************************************************************/
#define     BML_IOCTL_GET_BMI               XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    4,                      \
                                                    XSR_METHOD_BUFFERED,    \
                                                    XSR_READ_ACCESS)

/*****************************************************************************/
/*  UINT32  nWaitTime;                                                       */
/*  UINT32  nVol;                                                            */
/*  UINT32  nRet;                                                            */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nWaitTime = 50;                                                          */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_SET_WTIME_FOR_ERR, (UINT8 *) &nWaitTime,       */
/*            sizeof(UINT32), NULL, 0, &nRet)                                */
/*****************************************************************************/
#define     BML_IOCTL_SET_WTIME_FOR_ERR     XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    5,                      \
                                                    XSR_METHOD_IN_DIRECT,   \
                                                    XSR_WRITE_ACCESS)

/*****************************************************************************/
/*  UINT32   nVol;                                                           */
/*  UINT32   nRet;                                                           */
/*  XSRPartI stPI;                                                           */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_GET_FULL_PI, NULL, 0,                          */
/*            stPI, sizeof(XSRPartI), &nRet)                                 */
/*****************************************************************************/
#define     BML_IOCTL_GET_FULL_PI           XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    6,                      \
                                                    XSR_METHOD_BUFFERED,    \
                                                    XSR_READ_ACCESS)

/*****************************************************************************/
/*  UINT32  nDevIdx;                                                         */
/*  UINT32  nRet;                                                            */
/*  UINT32  nVol;                                                            */
/*  UINT32  nCnt;                                                            */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nDevIdx   = 0;                                                           */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_INVALID_BLOCK_CNT, (UINT8 *) &nDevIdx,         */
/*            sizeof(UINT32), (UINT8 *) &nCnt, sizeof(UINT32), &nRet)        */
/*****************************************************************************/
#define     BML_IOCTL_INVALID_BLOCK_CNT     XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    7,                      \
                                                    XSR_METHOD_IN_DIRECT,   \
                                                    XSR_READ_ACCESS)

/*****************************************************************************/
/*  UINT32  nSbn;                                                            */
/*  UINT32  nBlks;                                                           */
/*  UINT32  nRet;                                                            */
/*  UINT32  nVol;                                                            */
/*  UINT8  *pBufIn;                                                          */
/*  UINT32  nDevIdx                                                          */
/*                                                                           */
/*  nVol      = 0;                                                           */
/*  nDevIdx   = 0;                                                           */
/*                                                                           */
/*  *(UINT32*)pBufIn         = nDevIdx;                                      */
/*  *((UINT32*)(pBufIn + 4)) = nSbn;                                         */
/*  *((UINT32*)(pBufIn + 8)) = nBlks;                                        */
/*                                                                           */
/*  BML_IOCtl(nVol, BML_IOCTL_SET_BLOCK_LOCK, pBufIn, 0,                     */
/*                                              NULL, 0, &nRet)              */
/*****************************************************************************/
#define     BML_IOCTL_SET_BLOCK_LOCK        XSR_IOCTL_CODE(XSR_MODULE_BML,  \
                                                    8,                      \
                                                    XSR_METHOD_BUFFERED,    \
                                                    XSR_WRITE_ACCESS)
                                                                                                                                                            
/*****************************************************************************/
/* device specification structure in Volume                                  */
/*****************************************************************************/
typedef struct {
    UINT16       nPgsPerBlk;    /* the number of pages per block             */
    UINT8        nSctsPerPg;    /* the number of sectors per page            */
    UINT8        nLsnPos;       /* offset for storing LSN in spare area      */

    UINT8        nEccPos;       /* offset for storing ECC value in spare area*/
    UINT8        nReserved8;    /* Reserved                                  */
    UINT16       nReserved16;   /* Reserved                                  */

    UINT32       nTrTime;       /* read-time of NAND device (unit: nsec)     */
    UINT32       nTwTime;       /* write-time of NAND device (unit : nsec)   */
    UINT32       nTeTime;       /* erase-time of NAND device (unit : nsec)   */
    UINT32       nTfTime;       /* transfer-time from NAND device to host    
                                   memory (unit : nsec)                      */

    UINT32       nNumOfUsBlks;  /* the number of usable blocks in the volume */
    BOOL32       bMErasePol;    /* MErase Policy                             */

    UINT32       nEccPol;       /* ECC policy                                */
    
    UINT8        aUID[XSR_UID_SIZE]; /* Uniqure ID in OTP area of NAND device*/
} BMLVolSpec;

/*****************************************************************************/
/* partition entry structure                                                 */
/*****************************************************************************/
typedef struct {
    UINT32       nID;           /* partition entry ID                        */
    UINT32       nAttr;         /* Attribute                                 */
    UINT32       n1stVbn;       /* 1st virtual block number                  */
    UINT32       nNumOfBlks;    /* # of blocks                               */
} XSRPartEntry;

/*****************************************************************************/
/* partition information structure                                           */
/*****************************************************************************/
typedef struct {
    UINT8         aSig[BML_MAX_PART_SIG];   /* signature of partition 
                                               information                   */
    UINT32        nVer;                     /* version of partition 
                                               information                   */
    UINT32        nNumOfPartEntry;          /* # of partition entry          */
    XSRPartEntry  stPEntry[BML_MAX_PARTENTRY];
} XSRPartI;

/*****************************************************************************/
/* partition extension information structure                                 */
/*****************************************************************************/
typedef struct {
    UINT32       nID;           /* nID of partition information extension    */
    UINT32       nSizeOfData;   /* size of pData                             */
    VOID        *pData;         /* pointer of user-defined data structure    */
} XSRPIExt;


typedef struct {
    UINT32       nSrcVsn;       /* source virtual sector number              */
    UINT16       nRndInOffset;  /* spare area offset (0~15) for random-in    */
                                /* Random-In of ScrArg is only used          */
                                /* for spare area                            */
    UINT16       nRndInBytes;   /* number of bytes for random-in             */
    UINT8       *pRndInBuf;     /* buffer pointer for random-in              */
} SrcArg;

typedef struct {
    SrcArg       aSrcArg[XSR_MAX_REQSEC];
    UINT32       nDstVsn;       /* destination virtual sector number         */
    UINT32       nCopyCnt;      /* number of CopyArg                         */
} CopyArg;

/*****************************************************************************/
/* exported function prototype of BML                                        */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*****************************************************************************/
/* APIs for pre-programming                                                  */
/*****************************************************************************/
#if !defined(XSR_NBL2)
INT32   BML_Format           (UINT32  nVol,  XSRPartI *pstPart, UINT32 nFlag);
#endif /* XSR_NBL2 */

/*****************************************************************************/
/* Major APIs                                                                */
/*****************************************************************************/
INT32   BML_Init             (VOID);
INT32   BML_Open             (UINT32  nVol);

#if !defined(XSR_NBL2)
INT32   BML_Close            (UINT32  nVol);
#endif /* XSR_NBL2 */
INT32   BML_Read             (UINT32  nVol,  UINT32  nVsn,  UINT32 nNumOfScts,
                              UINT8  *pMBuf, UINT8  *pSBuf, UINT32 nFlag);
INT32   BML_MRead            (UINT32  nVol,  UINT32 nVsn,   UINT32 nNumOfScts,
                              UINT8  *pMBuf, UINT8 *pSBuf,  UINT32 nFlag);
INT32   BML_SGLRead          (UINT32  nVol,  UINT32 nVsn,   UINT32 nNumOfScts,
                              SGL   *pstSGL, UINT8 *pSBuf,  UINT32 nFlag);
#if !defined(XSR_NBL2)
INT32   BML_Write            (UINT32  nVol,  UINT32  nVsn,  UINT32 nNumOfScts,
                              UINT8  *pMBufs, UINT8  *pSBuf, UINT32 nFlag);
INT32   BML_MWrite           (UINT32  nVol,  UINT32  nVsn,  UINT32 nNumOfScts,
                              UINT8  *pMBuf, UINT8  *pSBuf, UINT32 nFlag);
INT32   BML_SGLWrite         (UINT32  nVol,  UINT32  nVsn,  UINT32 nNumOfScts,
                              SGL   *pstSGL, UINT8  *pSBuf, UINT32 nFlag);
INT32   BML_EraseBlk         (UINT32  nVol,  UINT32  nVbn,  UINT32 nFlag);
INT32   BML_MEraseBlk        (UINT32  nVol,  MEBArg *pstMEBArg, UINT32 nFlag);
INT32   BML_FlushOp          (UINT32  nVol,  UINT32   nFlag);
INT32   BML_GetBBInfo        (UINT32 nVol, UINT8 nDevNo, UINT8* pSRBInfo, UINT8* nBBCnt);
#endif /* XSR_NBL2 */

INT32   BML_GetVolInfo       (UINT32  nVol,  BMLVolSpec  *pstVolSpec);
INT32   BML_LoadPIEntry      (UINT32  nVol,  UINT32 nID,
                              XSRPartEntry  *pstPartEntry);
INT32	BML_GetDevInfo		 (UINT32  nVol, LLDSpec  *pstLLDSpec);                              
/*****************************************************************************/
/* APIs only used in STL                                                     */
/*****************************************************************************/
#if !defined(XSR_NBL2)
INT32   BML_CopyBack         (UINT32  nVol, CpBkArg *pstCpArg, UINT32  nFlag);
INT32   BML_Copy             (UINT32  nVol, CopyArg *pstCpArg, UINT32  nFlag);
#endif /* XSR_NBL2 */

/*****************************************************************************/
/* Extra APIs                                                                */
/*****************************************************************************/
#if !defined(XSR_NBL2)
INT32   BML_StorePIExt       (UINT32  nVol,  XSRPIExt *pExtInfo);
#endif /* XSR_NBL2 */
INT32   BML_LoadPIExt        (UINT32  nVol,  UINT32    nID, XSRPIExt *pExtInfo);
INT32   BML_IOCtl            (UINT32  nVol,  UINT32    nCode,
                                             UINT8    *pBufIn,   UINT32  nLenIn,
                                             UINT8    *pBufOut,  UINT32  nLenOut,
                                             UINT32   *pBytesReturned);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* _XSR_BML_H_ */
