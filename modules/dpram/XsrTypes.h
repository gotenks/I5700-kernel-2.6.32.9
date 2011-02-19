/*****************************************************************************/
/*                                                                           */
/* PROJECT : AnyStore II                                                     */
/* MODULE  : XSR types definition heade file                                 */
/* NAME    : XSR types definition                                            */
/* FILE    : XsrTypes.h                                                      */
/* PURPOSE : Types definition for XSR                                        */
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
/*   01-JUL-2003 [HyoJun Kim]   : first writing                              */
/*   23-JUL-2003 [HyoJun Kim]   : reorganization                             */
/*   24-JUL-2003 [SongHo Yoon]  : added return value MACRO definition        */
/*   28-JUL-2003 [SongHo Yoon]  : added #ifndef VOID, #endif VOID            */
/*                                to avoid the syntax error when using VC++  */
/*   28-JUL-2003 [SongHo Yoon]  : added XSR_UID_SIZE                         */
/*   13-AUG-2003 [SongHo Yoon]  : removed NULL definition                    */
/*   21-AUG-2003 [SongHo Yoon]  : added XSR_IOCTL_CODE                       */
/*   22-DEC-2003 [SongHo Yoon]  : added XSR_MAX_VOL/XSR_MAX_DEV              */
/*   14-JAN-2004 [SongHo Yoon]  : added XSR_MAX_SEMAPHORE                    */
/*   12-FEB-2004 [SongHo Yoon]  : added XSR_NO_BASIC_TYPES                   */
/*   01-SEP-2004 [MinYoung Kim] : added MEBArg structure                     */
/*   07-APR-2005 [MinYoung Kim] : modified defect of XSR_IOCTL_CODE          */
/*                                                                           */
/*****************************************************************************/

#ifndef _XSR_TYPES_H_
#define _XSR_TYPES_H_

/*****************************************************************************/
/* Basic Types                                                               */
/* if the XSR basic types is defined in the typedef header file of OS and    */ 
/*    OAM.cpp includes both the typedef header file of OS and XsrTypes.h,    */
/* XSR_NO_BASIC_TYPES should be defined in OAM.cpp to avoid the duplicate    */
/* typedef error.                                                            */
/*****************************************************************************/
#if !defined(XSR_NO_BASIC_TYPES)

typedef     unsigned int        UINT32;
typedef     signed int          INT32;
typedef     unsigned short      UINT16;
typedef     signed short        INT16;
typedef     unsigned char       UINT8;
typedef     signed char         INT8;

#endif /* XSR_NO_BASIC_TYPES */

#ifndef     VOID
typedef     void                VOID;
#endif      //VOID
typedef     UINT32              BOOL32;
typedef     UINT32              SM32;

/*****************************************************************************/
/* Basic Constants                                                           */
/*****************************************************************************/

#define     FALSE32             (BOOL32) 0
#define     TRUE32              (BOOL32) 1

/*****************************************************************************/
/* Global Definition which should be shared by STL, BML, FIL                 */
/*****************************************************************************/
#define     XSR_MAX_VOL         2
#define     XSR_MAX_DEV         8

#define     XSR_MAX_MEBLK       16

#define     XSR_SECTOR_SIZE     (512)
#define     XSR_SPARE_SIZE      (16)
#define     XSR_MAX_REQSEC      (4)     /* the maximum number of request sec */
#define     XSR_UID_SIZE        (16)    /* the size of unique ID             */

#define     XSR_MAX_SEMAPHORE   (10)    /* the maximum number of semaphore   */

#define     XSR_INT_ID_NAND_0   (0)     /* Interrupt ID : 1st NAND            */
#define     XSR_INT_ID_NAND_1   (1)     /* Interrupt ID : 2nd NAND            */
#define     XSR_INT_ID_NAND_2   (2)     /* Interrupt ID : 3rd NAND            */
#define     XSR_INT_ID_NAND_3   (3)     /* Interrupt ID : 4th NAND            */
#define     XSR_INT_ID_NAND_4   (4)     /* Interrupt ID : 5th NAND            */
#define     XSR_INT_ID_NAND_5   (5)     /* Interrupt ID : 6th NAND            */
#define     XSR_INT_ID_NAND_6   (6)     /* Interrupt ID : 7th NAND            */
#define     XSR_INT_ID_NAND_7   (7)     /* Interrupt ID : 8th NAND            */

#define     XSR_MAX_SGL_ENTRIES	 9
/*****************************************************************************/
/* Return value MACRO definition                                             */
/*****************************************************************************/
#define     XSR_RETURN_MAJOR(err)           (INT32)((err) & 0xFFFF0000)
#define     XSR_RETURN_MINOR(err)           (INT32)((err) & 0x0000FFFF)
#define     XSR_RETURN_VALUE(err, maj, min) (INT32)(((UINT32)((err) & 0x00000001) << 31) | \
                                                    ((UINT32)((maj) & 0x00007FFF) << 16) | \
                                                     (UINT32)((min) & 0x0000FFFF))

/*****************************************************************************/
/* Module field of XSR_IOCTL_CODE                                            */
/*****************************************************************************/
#define XSR_MODULE_STL                      0
#define XSR_MODULE_BML                      1
#define XSR_MODULE_LLD                      2

/*****************************************************************************/
/* Method field of XSR_IOCTL_CODE                                            */
/*****************************************************************************/
#define XSR_METHOD_BUFFERED                 0
#define XSR_METHOD_IN_DIRECT                1
#define XSR_METHOD_OUT_DIRECT               2
#define XSR_METHOD_INOUT_DIRECT             3

/*****************************************************************************/
/* Access field of XSR_IOCTL_CODE                                            */
/*****************************************************************************/
#define XSR_ANY_ACCESS                      0
#define XSR_READ_ACCESS                     1
#define XSR_WRITE_ACCESS                    2

/*****************************************************************************/
/* Macro definition for defining IOCTL control codes.                        */
/* The range of Module field is from bit 31 to bit 18. the size is 14 bits   */
/* The range of Function field is from bit 17 to bit 4. the size is 14 bits  */
/* The range of Method field is from bit 3 to bit 2. the size is 2 bits      */
/* The range of Access field is from bit 1 to bit 0. the size is 2 bits      */
/*****************************************************************************/
#define XSR_IOCTL_CODE(Module, Function, Method, Access)    (               \
    ((Module) << 18 | (Function) << 4 | (Method) << 2 | (Access))           \
)

/*****************************************************************************/
/* SGL Entry Flag (User data or meta data)                                   */
/*****************************************************************************/
typedef enum
{
	SGL_ENTRY_USER_DATA = 0,
	SGL_ENTRY_BISCT_VALID_DATA = 1,
	SGL_ENTRY_BISCT_INVALID_DATA = 2,
	SGL_ENTRY_META_DATA = 3
} SGLEntryFlag;

/*****************************************************************************/
/* Global Structure Types which should be shared by STL, BML, FIL            */
/*****************************************************************************/

typedef struct
{
    UINT16    nOffset;      /* nOffset : sector offset(0 - 3) * 1024
                                         + Main(0 - 511) + Spare(512 - 527) */
    UINT16    nNumOfBytes;  /* Random In Bytes                              */
    UINT8    *pBuf;         /* Data Buffer Pointer                          */
} RndInArg;

typedef struct
{
    UINT32    nSrcSn;       /* Copy Back Source Vsn, should be page aligned */
    UINT32    nDstSn;       /* Copy Back Dest.  Vsn, should be page aligned */
    UINT32    nRndInCnt;    /* Random In Count                              */
    RndInArg *pstRndInArg;  /* RndInArg Array pointer                       */
} CpBkArg;

typedef struct
{
    UINT32  *pMEBList;       /* Multi-block erase list pointer               */
                             /* which indicates blocks (virtual)             */ 
                             /* to be erased by multi-block erase            */             
    UINT32   nNumOfMEBList;  /* Total number of blocks to be erased          */     
} MEBArg;

/*****************************************************************************/
/* SGL entry                                                                 */
/*****************************************************************************/
typedef struct
{
	UINT8*			pBuf;		/* Buffer for data							 */
	UINT16			nSectors;	/* Number of sectors this entry represents   */
	UINT8			nFlag;		/* user data or meta data					 */
								/* SGL_ENTRY_USER_DATA 						 */
								/* SGL_ENTRY_BISCT_DATA						 */
								/* SGL_ENTRY_META_DATA						 */
} SGLEntry;

/*****************************************************************************/
/* SGL (Scatter-gather list)                                                 */
/*****************************************************************************/
typedef struct
{
	UINT8			nElements;
	SGLEntry		stSGLEntry[XSR_MAX_SGL_ENTRIES];
} SGL;

#endif /* _XSR_TYPES_H_ */
