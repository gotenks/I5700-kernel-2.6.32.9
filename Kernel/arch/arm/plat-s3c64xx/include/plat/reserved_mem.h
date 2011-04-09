#ifndef _ASM_ARM_ARCH_RESERVED_MEM_H
#define _ASM_ARM_ARCH_RESERVED_MEM_H

#include <linux/types.h>
#include <linux/list.h>
#include <asm/setup.h>

#define DRAM_END_ADDR 				(PHYS_OFFSET + PHYS_SIZE)



#ifdef CONFIG_SEC_LOG_BUF

/* Reserved 1MB for Frame & Log Buffer & Android RAM console */
#define RESERVED_PMEM_END_ADDR 			(DRAM_END_ADDR - (1 * 1024 * 1024))



#define SEC_LOG_BUF_DATA_SIZE			(1 << CONFIG_LOG_BUF_SHIFT)

#define SEC_LOG_BUF_FLAG_SIZE			(4 * 1024)		

#define SEC_LOG_BUF_SIZE			(SEC_LOG_BUF_FLAG_SIZE + SEC_LOG_BUF_DATA_SIZE)

#define SEC_LOG_BUF_START			(DRAM_END_ADDR - SEC_LOG_BUF_SIZE)

#define SEC_LOG_BUF_MAGIC			0x404C4F47	/* @LOG */



#ifdef CONFIG_ANDROID_RAM_CONSOLE
#define SEC_RAM_CONSOLE_DATA_SIZE		(1 << CONFIG_LOG_BUF_SHIFT)

#define SEC_RAM_CONSOLE_FLAG_SIZE		(0/*4*/ * 1024)

#define SEC_RAM_CONSOLE_BUF_SIZE		(SEC_RAM_CONSOLE_DATA_SIZE + SEC_RAM_CONSOLE_FLAG_SIZE)

#define SEC_RAM_CONSOLE_BUF_START		(SEC_LOG_BUF_START - SEC_RAM_CONSOLE_BUF_SIZE)

//#define RAM_CONSOLE_SIG (0x43474244) /* DBGC */

#if SEC_RAM_CONSOLE_BUF_START < RESERVED_PMEM_END_ADDR
    #error RAM Console consumes to much memory
#endif
#endif

struct sec_log_buf {
	unsigned int *flag;	
	unsigned int *count;
	char *data;
};

extern void sec_log_buf_init(void);

#else

#define RESERVED_PMEM_END_ADDR 		(DRAM_END_ADDR)

#endif

#define RESERVED_MEM_CMM		(8 * 1024 * 1024)
#define RESERVED_MEM_MFC		(6 * 1024 * 1024)
#define RESERVED_PMEM_PICTURE		(6 * 1024 * 1024)	/* PMEM_PIC and MFC use share area */
#define RESERVED_PMEM_JPEG		(3 * 1024 * 1024)
#define RESERVED_PMEM_PREVIEW		(2 * 1024 * 1024)
#define RESERVED_PMEM_RENDER  		(2 * 1024 * 1024)
#define RESERVED_PMEM_STREAM		(4 * 1024 * 1024) // 0 -> 4
#define RESERVED_G3D			(32 * 1024 * 1024) 	/* G3D is shared with uppper memory areas */
#ifdef CONFIG_USE_TOM3Q_DRIVERS
    #define RESERVED_PMEM_GPU1		(RESERVED_G3D)
#endif
#define RESERVED_PMEM			(8 * 1024 * 1024)

#define RESERVED_G3D_UI			(4 * 1024 * 1024)
#define RESERVED_G3D_SHARED		(RESERVED_MEM_CMM + RESERVED_MEM_MFC + RESERVED_PMEM_STREAM + RESERVED_PMEM_JPEG + RESERVED_PMEM_PREVIEW + RESERVED_PMEM_RENDER)
#define RESERVED_G3D_APP		(RESERVED_G3D - RESERVED_G3D_UI - RESERVED_G3D_SHARED)

#if defined(CONFIG_RESERVED_MEM_CMM_JPEG_MFC_POST_CAMERA)
#define CMM_RESERVED_MEM_START		(RESERVED_PMEM_END_ADDR - RESERVED_MEM_CMM)
#define MFC_RESERVED_MEM_START		(CMM_RESERVED_MEM_START - RESERVED_MEM_MFC)
#define PICTURE_RESERVED_PMEM_START	(MFC_RESERVED_MEM_START)
#define JPEG_RESERVED_PMEM_START	(MFC_RESERVED_MEM_START - RESERVED_PMEM_JPEG)
#define PREVIEW_RESERVED_PMEM_START	(JPEG_RESERVED_PMEM_START - RESERVED_PMEM_PREVIEW)
#define RENDER_RESERVED_PMEM_START	(PREVIEW_RESERVED_PMEM_START - RESERVED_PMEM_RENDER)
#define STREAM_RESERVED_PMEM_START	(RENDER_RESERVED_PMEM_START - RESERVED_PMEM_STREAM)
#define G3D_RESERVED_START		(RESERVED_PMEM_END_ADDR - RESERVED_G3D)		 /* G3D is shared */
#ifdef CONFIG_USE_TOM3Q_DRIVERS
    #define GPU1_RESERVED_PMEM_START	(G3D_RESERVED_START)
#endif
#define RESERVED_PMEM_START		(G3D_RESERVED_START - RESERVED_PMEM)
#define PHYS_UNRESERVED_SIZE		(RESERVED_PMEM_START - PHYS_OFFSET)

#else
#define PHYS_UNRESERVED_SIZE		(RESERVED_PMEM_END_ADDR - PHYS_OFFSET)

#endif 

struct s3c6410_pmem_setting{
        resource_size_t pmem_start;
        resource_size_t pmem_size;
#ifdef CONFIG_USE_TOM3Q_DRIVERS
        resource_size_t pmem_gpu1_start;
        resource_size_t pmem_gpu1_size;
#endif
        resource_size_t pmem_render_start;
        resource_size_t pmem_render_size;
        resource_size_t pmem_render_pic_start;
        resource_size_t pmem_render_pic_size;
        resource_size_t pmem_stream_start;
        resource_size_t pmem_stream_size;
        resource_size_t pmem_preview_start;
        resource_size_t pmem_preview_size;
        resource_size_t pmem_picture_start;
        resource_size_t pmem_picture_size;
        resource_size_t pmem_jpeg_start;
        resource_size_t pmem_jpeg_size;
};
 
void s3c6410_add_mem_devices (struct s3c6410_pmem_setting *setting);

#endif /* _ASM_ARM_ARCH_RESERVED_MEM_H */

