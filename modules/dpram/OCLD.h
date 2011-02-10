/*****************************************************************************/
/*                                                                           */
/* PROJECT : RFS_1.3.1_b046-tinyBML_1.1.0p4-XSR_1.5.2p4_b122_RTM/PROJECT                                                             */
/* MODULE  : LLD                                                             */
/* NAME    : OneNAND LLD header                                              */
/* FILE    : ONLD.h                                                          */
/* PURPOSE : This file implements the exported function declarations and     */
/*           the exported values return values, macros, types,...            */
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
/* - 03/JUN/2003 [Janghwan Kim] : first writing                              */
/* - 04/OCT/2003 [Janghwan Kim] : reorganization                             */
/* - 11/DEC/2003 [Janghwan Kim] : Add ONLD_IOCtl() function                  */
/* - 11/DEC/2003 [Janghwan Kim] : Change parmameter of ONLD_Init()           */
/*                                                                           */
/*****************************************************************************/

#ifndef _ONENAND_H_
#define _ONENAND_H_

/*****************************************************************************/
/* ONLD External Function declarations                                       */
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

INT32 ONLD_Init         (VOID  *pParm);
INT32 ONLD_Open         (UINT32 nDev);   
INT32 ONLD_Close        (UINT32 nDev);   
INT32 ONLD_Read         (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts, 
                         UINT8 *pMBuf,   UINT8   *pSBuf,    UINT32 nFlag);
INT32 ONLD_Write        (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts, 
                         UINT8 *pMBuf,   UINT8   *pSBuf,    UINT32 nFlag);
INT32 ONLD_Erase        (UINT32 nDev,    UINT32   nPbn,     UINT32 nFlag);
INT32 ONLD_CopyBack     (UINT32 nDev,    CpBkArg *pstCpArg, UINT32 nFlag);
INT32 ONLD_ChkInitBadBlk(UINT32 nDev,    UINT32   nPbn);
INT32 ONLD_SetRWArea    (UINT32 nDev,    UINT32   nSUbn,    UINT32 nUBlks);
INT32 ONLD_FlushOp      (UINT32 nDev);   
INT32 ONLD_GetUID       (UINT8 * aUID);
INT32 ONLD_GetDevInfo   (UINT32 nDev,    LLDSpec *pstLLDDev);
INT32 ONLD_GetPrevOpData(UINT32 nDev,    UINT8   *pMBuf,    UINT8 *pSBuf);
INT32 ONLD_IOCtl        (UINT32 nDev,    UINT32   nCmd,
                         UINT8 *pBufI,   UINT32   nLenI, 
                         UINT8 *pBufO,   UINT32   nLenO,
                         UINT32 *pByteRet);
INT32 ONLD_MRead        (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts, 
                         SGL  *pstSGL,   UINT8   *pSBuf,    UINT32 nFlag);
INT32 ONLD_MWrite       (UINT32 nDev,    UINT32   nPsn,     UINT32 nScts, 
                         SGL  *pstSGL,   UINT8   *pSBuf,    UINT32 nFlag,
						 UINT32 *pErrPsn);
INT32 ONLD_EraseVerify  (UINT32 nDev,    LLDMEArg *pstMEArg,
                         UINT32 nFlag);
INT32 ONLD_MErase       (UINT32 nDev,    LLDMEArg *pstMEArg, 
                         UINT32 nFlag);
VOID  ONLD_GetPlatformInfo
                        (LLDPlatformInfo* pstLLDPlatformInfo);                         
INT32 ONLD_PowerUp(UINT32 nVol, UINT32 n1stVbn, UINT32 nNumOfBlks);
UINT32 _SetRWBlock(UINT32 nDev, UINT32 nSUbn, UINT32 nUBlks);

VOID _EnterOTP(UINT32 nDev);
VOID _ExitOTP(UINT32 nDev);
INT32 ONLD_OTPRead(UINT32 nDev, UINT32 nPsn, UINT32 nScts, UINT8 *pMBuf, UINT8 *pSBuf, UINT32 nFlag);
INT32 ONLD_OTPWrite(UINT32 nDev, UINT32 nPsn, UINT32 nScts, UINT8 *pMBuf, UINT8 *pSBuf, UINT32 nFlag);
INT32 ONLD_OTPLock             (UINT32  nDev,       UINT32  nLockFlag);
INT32 ONLD_GetOTPLockInfo      (UINT32  nDev,       UINT32* pLockInfo);

#ifdef __cplusplus
};
#endif // __cplusplus

#endif /*  _ONENAND_H_ */
