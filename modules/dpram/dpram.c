/****************************************************************************
**
** COPYRIGHT(C) : Samsung Electronics Co.Ltd, 2006-2010 ALL RIGHTS RESERVED
**
**                Onedram Device Driver
**
****************************************************************************/

#define _DEBUG
#define _ENABLE_ERROR_DEVICE
//#undef _ENABLE_ERROR_DEVICE
#undef _DPRAM_DEBUG_HEXDUMP

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/irq.h>
#include <linux/rtc.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>
#include <mach/spica.h>

#ifdef CONFIG_EVENT_LOGGING
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/klog.h>
#include <asm/unistd.h>
#endif	/* CONFIG_EVENT_LOGGING */

#include <linux/sched.h>

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif	/* CONFIG_PROC_FS */

#include <linux/wakelock.h>

#include "dpram.h"
#include "BML.h"
#include "OCLDReg.h"
#include "OCLD.h"
#include <mach/param.h>

#define DRIVER_ID			"$Id: dpram.c, v0.01 2008/12/29 08:00:00 $"
#define DRIVER_NAME 		"DPRAM"
#define DRIVER_PROC_ENTRY	"driver/dpram"
#define DRIVER_MAJOR_NUM	252

#ifdef CONFIG_EVENT_LOGGING
#define DPRAM_ID			3
#define DPRAM_READ			3
#define DPRAM_WRITE			4
#endif	/* CONFIG_EVENT_LOGGING */

#ifdef _DEBUG
#define dprintk(s, args...) printk("[OneDRAM] %s:%d - " s, __func__, __LINE__,  ##args)
#else
#define dprintk(s, args...)
#endif	/* _DEBUG */

#define WRITE_TO_DPRAM(dest, src, size) \
	_memcpy((void *)(DPRAM_VBASE + dest), src, size)

#define READ_FROM_DPRAM(dest, src, size) \
	_memcpy(dest, (void *)(DPRAM_VBASE + src), size)

#ifdef _ENABLE_ERROR_DEVICE
#define DPRAM_ERR_MSG_LEN			65
#define DPRAM_ERR_DEVICE			"dpramerr"
#endif	/* _ENABLE_ERROR_DEVICE */

/* Maximum PDP data length */
#define MAX_PDP_DATA_LEN		1500

/* Maximum PDP packet length including header and start/stop bytes */
#define MAX_PDP_PACKET_LEN		(MAX_PDP_DATA_LEN + 4 + 2)

#define IRQ_ONEDRAM_INT_N	IRQ_EINT(0)
#define IRQ_SIM_DETECT_N	IRQ_EINT(4)
#define IRQ_PHONE_ACTIVE	IRQ_EINT(7)

static DECLARE_WAIT_QUEUE_HEAD(dpram_wait);

static int onedram_get_semaphore(const char*);
static int return_onedram_semaphore(const char*);
static void send_interrupt_to_phone_with_semaphore(u16 irq_mask);
static unsigned char dbl_buf[MAX_DBL_IMG_SIZE];

static void __iomem *dpram_base = 0;
static volatile unsigned int *onedram_sem;
static volatile unsigned int *onedram_mailboxBA;		// send mail
static volatile unsigned int *onedram_mailboxAB;		// received mail
static volatile unsigned char *onedram_checkBA;		  // received mail

static atomic_t onedram_lock;
static int onedram_lock_with_semaphore(const char*);
static void onedram_release_lock(const char*);

static void dpram_cp_dump(dump_order);
static int register_interrupt_handler(void);

static int boot_complete = 0;
static int requested_semaphore = 0;
static int phone_sync = 0;
static int dump_on = 0;
static int phone_power_state = 0;
static int modem_wait_count = 0;

// the value for checking sim status change on sleep.
static int sim_state = 0;

static int dpram_phone_getstatus();
#define DPRAM_VBASE dpram_base
static struct tty_driver *dpram_tty_driver;
static dpram_tasklet_data_t dpram_tasklet_data[MAX_INDEX];
static dpram_device_t dpram_table[MAX_INDEX] = {
	{
		.in_head_addr = DPRAM_PHONE2PDA_FORMATTED_HEAD_ADDRESS,
		.in_tail_addr = DPRAM_PHONE2PDA_FORMATTED_TAIL_ADDRESS,
		.in_buff_addr = DPRAM_PHONE2PDA_FORMATTED_BUFFER_ADDRESS,
		.in_buff_size = DPRAM_PHONE2PDA_FORMATTED_BUFFER_SIZE,

		.out_head_addr = DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS,
		.out_tail_addr = DPRAM_PDA2PHONE_FORMATTED_TAIL_ADDRESS,
		.out_buff_addr = DPRAM_PDA2PHONE_FORMATTED_BUFFER_ADDRESS,
		.out_buff_size = DPRAM_PDA2PHONE_FORMATTED_BUFFER_SIZE,
		.out_head_saved = 0,
		.out_tail_saved = 0,

		.mask_req_ack = INT_MASK_REQ_ACK_F,
		.mask_res_ack = INT_MASK_RES_ACK_F,
		.mask_send = INT_MASK_SEND_F,
	},
	{
		.in_head_addr = DPRAM_PHONE2PDA_RAW_HEAD_ADDRESS,
		.in_tail_addr = DPRAM_PHONE2PDA_RAW_TAIL_ADDRESS,
		.in_buff_addr = DPRAM_PHONE2PDA_RAW_BUFFER_ADDRESS,
		.in_buff_size = DPRAM_PHONE2PDA_RAW_BUFFER_SIZE,

		.out_head_addr = DPRAM_PDA2PHONE_RAW_HEAD_ADDRESS,
		.out_tail_addr = DPRAM_PDA2PHONE_RAW_TAIL_ADDRESS,
		.out_buff_addr = DPRAM_PDA2PHONE_RAW_BUFFER_ADDRESS,
		.out_buff_size = DPRAM_PDA2PHONE_RAW_BUFFER_SIZE,
		.out_head_saved = 0,
		.out_tail_saved = 0,

		.mask_req_ack = INT_MASK_REQ_ACK_R,
		.mask_res_ack = INT_MASK_RES_ACK_R,
		.mask_send = INT_MASK_SEND_R,
	},
};

static struct tty_struct *dpram_tty[MAX_INDEX];
static struct ktermios *dpram_termios[MAX_INDEX];
static struct ktermios *dpram_termios_locked[MAX_INDEX];

static void res_ack_tasklet_handler(unsigned long data);
static void send_tasklet_handler(unsigned long data);

static DECLARE_TASKLET(fmt_send_tasklet, send_tasklet_handler, 0);
static DECLARE_TASKLET(raw_send_tasklet, send_tasklet_handler, 0);

static DECLARE_TASKLET(fmt_res_ack_tasklet, res_ack_tasklet_handler,
		(unsigned long)&dpram_table[FORMATTED_INDEX]);
static DECLARE_TASKLET(raw_res_ack_tasklet, res_ack_tasklet_handler,
		(unsigned long)&dpram_table[RAW_INDEX]);

static void semaphore_control_handler(unsigned long data);
static DECLARE_TASKLET(semaphore_control_tasklet, semaphore_control_handler, 0);

// DGS Info cache
static unsigned char aDGSBuf[512];

#ifdef _ENABLE_ERROR_DEVICE
static unsigned int dpram_err_len;
static char dpram_err_buf[DPRAM_ERR_MSG_LEN];

struct class *dpram_class;

static DECLARE_WAIT_QUEUE_HEAD(dpram_err_wait_q);
#endif	/* _ENABLE_ERROR_DEVICE */
static struct fasync_struct *dpram_err_async_q;

// 2008.10.20.
static DECLARE_MUTEX(write_mutex);
struct wake_lock dpram_wake_lock;

#ifdef CONFIG_EVENT_LOGGING
static inline EVENT_HEADER *getPayloadHeader(int flag, int size)
{
	EVENT_HEADER *header;
	struct timeval time_val;

	header = (EVENT_HEADER *)kmalloc(sizeof (EVENT_HEADER), GFP_ATOMIC);
	do_gettimeofday(&time_val);

	header->timeVal = time_val;
	header->class = (flag == DPRAM_READ ? DPRAM_READ : DPRAM_WRITE);
	header->repeat_count = 0;
	header->payload_length = size;

	return header;
}

static inline void dpram_event_logging(int direction, void *src, int size)
{
	EVENT_HEADER *header;
	unsigned long flags;

	header = getPayloadHeader(direction, size);

	local_irq_save(flags);
	klog(header, sizeof (EVENT_HEADER), DPRAM_ID);

	if (direction == DPRAM_WRITE) {
		klog(src, size, DPRAM_ID);
	}

	else if (direction == DPRAM_READ) {
		klog((void *)(DPRAM_VBASE + src), size, DPRAM_ID);
	}

	local_irq_restore(flags);
	kfree(header);
}
#endif	/* CONFIG_EVENT_LOGGING */

extern struct class *sec_class;
struct device *dpram_dev;

static ssize_t sim_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(sim_state) {
		sprintf(buf, "DETACHED");
	}
	else {
		sprintf(buf, "ATTACHED");
	}
		
	return sprintf(buf, "%s", buf);
}

static ssize_t sim_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	/* do noting */
	return size;
}

static DEVICE_ATTR(sim, S_IRUGO | S_IWUGO, sim_show, sim_store);

/* tty related functions. */
static inline void byte_align(unsigned long dest, unsigned long src)
{
	u16 *p_src;
	volatile u16 *p_dest;

	if (!(dest % 2) && !(src % 2)) {
		p_dest = (u16 *)dest;
		p_src = (u16 *)src;

		*p_dest = (*p_dest & (u16)0xFF00) | (*p_src & (u16)0x00FF);
	}

	else if ((dest % 2) && (src % 2)) {
		p_dest = (u16 *)(dest - 1);
		p_src = (u16 *)(src - 1);

		*p_dest = (*p_dest & (u16)0x00FF) | (*p_src & (u16)0xFF00);
	}

	else if (!(dest % 2) && (src % 2)) {
		p_dest = (u16 *)dest;
		p_src = (u16 *)(src - 1);

		*p_dest = (u16)((u16)(*p_dest & (u16)0xFF00) | (u16)((*p_src >> 8) & (u16)0x00FF));
	}

	else if ((dest % 2) && !(src % 2)) {
		p_dest = (u16 *)(dest - 1);
		p_src = (u16 *)src;

		*p_dest = (u16)((u16)(*p_dest & (u16)0x00FF) | (u16)((*p_src << 8) & (u16)0xFF00));
	}

	else {
		dprintk("oops.~\n");
	}
}

static inline void _memcpy(void *p_dest, const void *p_src, int size)
{
	int i;
	unsigned long dest = (unsigned long)p_dest;
	unsigned long src = (unsigned long)p_src;

	if (!(*onedram_sem)) {
		printk("[OneDRAM] (%s) memory access without semaphore!: %d\n", *onedram_sem);
		return;
	}
	if (size <= 0) {
		return;
	}

	if(!(size % 2) && !(dest % 2) && !(src % 2)) {
		for(i = 0; i < (size/2); i++)
			*(((u16 *)dest) + i) = *(((u16 *)src) + i);
	}
	else {
		for(i = 0; i < size; i++)
			byte_align(dest+i, src+i);
	}
}

static inline int _memcmp(u16 *dest, u16 *src, int size)
{
	int i = 0;

	if (!(*onedram_sem)) {
		printk("[OneDRAM] (%s) memory access without semaphore!: %d\n", *onedram_sem);
		return 1;
	}

	size =size >> 1;

	while (i < size) {
		if (*(dest + i) != *(src + i)) {
			return 1;
		}
		i++ ;
	}

	return 0;
}

static inline int WRITE_TO_DPRAM_VERIFY(u32 dest, void *src, int size)
{

	int cnt = 3;

	while (cnt--) {
		_memcpy((void *)(DPRAM_VBASE + dest), (void *)src, size);

		if (!_memcmp((u16 *)(DPRAM_VBASE + dest), (u16 *)src, size))
			return 0;
	}

	return -1;

}

static inline int READ_FROM_DPRAM_VERIFY(void *dest, u32 src, int size)
{

	int cnt = 1;
	
	while (cnt--) {
		_memcpy((void *)dest, (void *)(DPRAM_VBASE + src), size);

		if (!_memcmp((u16 *)dest, (u16 *)(DPRAM_VBASE + src), size))
			return 0;
	}
	
	return -1;

}

static void send_interrupt_to_phone(u16 irq_mask)
{
	*onedram_mailboxBA = irq_mask;
}

#ifdef _DPRAM_DEBUG_HEXDUMP
#define isprint(c)	((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
void hexdump(const char *buf, int len)
{
	char str[80], octet[10];
	int ofs, i, l;

	for (ofs = 0; ofs < len; ofs += 16) {
		sprintf( str, "%03d: ", ofs );

		for (i = 0; i < 16; i++) {
			if ((i + ofs) < len)
				sprintf( octet, "%02x ", buf[ofs + i] );
			else
				strcpy( octet, "   " );

			strcat( str, octet );
		}
			strcat( str, "  " );
			l = strlen( str );

		for (i = 0; (i < 16) && ((i + ofs) < len); i++)
			str[l++] = isprint( buf[ofs + i] ) ? buf[ofs + i] : '.';

		str[l] = '\0';
		printk( "%s\n", str );
	}
}

static void print_debug_current_time(void)
{
	struct rtc_time tm;
	struct timeval time;

	/* get current time */
	do_gettimeofday(&time);

	/* set current time */
	rtc_time_to_tm(time.tv_sec, &tm);

	printk(KERN_INFO"Kernel Current Time info - %02d%02d%02d%02d%02d.%ld \n", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, time.tv_usec);

}
#endif

static int dpram_write(dpram_device_t *device,
		const unsigned char *buf, int len)
{
	int retval = 0;
	int size = 0;

	u16 freesize = 0;
	u16 next_head = 0;
	
	u16 head, tail;
	u16 irq_mask = 0;
	
#ifdef _DPRAM_DEBUG_HEXDUMP
	printk("\n\n#######[dpram write : head - %04d, tail - %04d]######\n", head, tail);
	hexdump(buf, len);
#endif

	if(!onedram_get_semaphore(__func__)) 
		return -1;
		
	if(onedram_lock_with_semaphore(__func__) < 0)
		return -1;

	while(1) {

		READ_FROM_DPRAM_VERIFY(&head, device->out_head_addr, sizeof(head));
		READ_FROM_DPRAM_VERIFY(&tail, device->out_tail_addr, sizeof(tail));

		if(head < tail)
			freesize = tail - head - 1;
		else
			freesize = device->out_buff_size - head + tail -1;

		if(freesize >= len){

			next_head = head + len;

			if(next_head < device->out_buff_size) {
				size = len;
				WRITE_TO_DPRAM(device->out_buff_addr + head, buf, size);
				retval = size;
			}
			else {
				next_head -= device->out_buff_size;

				size = device->out_buff_size - head;
				WRITE_TO_DPRAM(device->out_buff_addr + head, buf, size);
				retval = size;

				size = next_head;
				WRITE_TO_DPRAM(device->out_buff_addr, buf + retval, size);
				retval += size;
			}

			head = next_head;

			WRITE_TO_DPRAM_VERIFY(device->out_head_addr, &head, sizeof(head));
				
		}
		
		/* @LDK@ send interrupt to the phone, if.. */
		irq_mask = INT_MASK_VALID;

		if (retval > 0)
			irq_mask |= device->mask_send;

		if (len > retval)
			irq_mask |= device->mask_req_ack;

		if(retval >= len) {
        	onedram_release_lock(__func__);
        	send_interrupt_to_phone_with_semaphore(irq_mask);
			break;
		}
		
	}

    // Can access head & tail info without smp.
	device->out_head_saved = head;
	device->out_tail_saved = tail;

#ifdef _DPRAM_DEBUG_HEXDUMP
	printk("#######[dpram write : head - %04d, tail - %04d]######\n\n", head, tail);
	print_debug_current_time();
#endif

	return retval;
	
}

static inline
int dpram_tty_insert_data(dpram_device_t *device, const u8 *psrc, u16 size)
{
#define CLUSTER_SEGMENT	1500

	u16 copied_size = 0;
	int retval = 0;

	if (size > CLUSTER_SEGMENT && (device->serial.tty->index == 1)) {
		while (size) {
			copied_size = (size > CLUSTER_SEGMENT) ? CLUSTER_SEGMENT : size;
			tty_insert_flip_string(device->serial.tty, psrc + retval, copied_size);

			size = size - copied_size;
			retval += copied_size;
		}

		return retval;
	}

	return tty_insert_flip_string(device->serial.tty, psrc, size);
}

static int dpram_read(dpram_device_t *device, const u16 non_cmd)
{
	int retval = 0;
	int size = 0;
	int ret = 0;
	u16 head, tail, up_tail;

	if(!onedram_get_semaphore(__func__)) 
		return -1;

	if(onedram_lock_with_semaphore(__func__) < 0)
		return -1;

	READ_FROM_DPRAM_VERIFY(&head, device->in_head_addr, sizeof(head));
	READ_FROM_DPRAM_VERIFY(&tail, device->in_tail_addr, sizeof(tail));

#ifdef _DPRAM_DEBUG_HEXDUMP
	printk("\n\n#######[dpram read : head - %04d, tail - %04d]######\n", head, tail);
#endif

	if (head != tail) {
		up_tail = 0;

		// ------- tail ++++++++++++ head -------- //
		if (head > tail) {
			size = head - tail;
			retval = dpram_tty_insert_data(device, (unsigned char *)(DPRAM_VBASE + (device->in_buff_addr + tail)), size);

		    if (retval != size)
				dprintk("Size Mismatch : Real Size = %d, Returned Size = %d\n", size, retval);

#ifdef _DPRAM_DEBUG_HEXDUMP
			hexdump((unsigned char *)(DPRAM_VBASE + (device->in_buff_addr + tail)), size);
#endif
		}

		// +++++++ head ------------ tail ++++++++ //
		else {
			int tmp_size = 0;

			// Total Size.
			size = device->in_buff_size - tail + head;

			// 1. tail -> buffer end.
			tmp_size = device->in_buff_size - tail;
			retval = dpram_tty_insert_data(device, (unsigned char *)(DPRAM_VBASE + (device->in_buff_addr + tail)), tmp_size);

			if (retval != tmp_size)
				dprintk("Size Mismatch : Real Size = %d, Returned Size = %d\n", tmp_size, retval);
			
#ifdef _DPRAM_DEBUG_HEXDUMP
			hexdump((unsigned char *)(DPRAM_VBASE + (device->in_buff_addr + tail)), tmp_size);
#endif
			// 2. buffer start -> head.
			if (size > tmp_size) {
				dpram_tty_insert_data(device, (unsigned char *)(DPRAM_VBASE + device->in_buff_addr), size - tmp_size);
				
#ifdef _DPRAM_DEBUG_HEXDUMP
				hexdump((unsigned char *)(DPRAM_VBASE + device->in_buff_addr), size - tmp_size);
#endif
				retval += (size - tmp_size);
			}
		}

		/* new tail */
		up_tail = (u16)((tail + retval) % device->in_buff_size);
		WRITE_TO_DPRAM_VERIFY(device->in_tail_addr, &up_tail, sizeof(up_tail));
	}
	
	device->out_head_saved = head;
	device->out_tail_saved = tail;

#ifdef _DPRAM_DEBUG_HEXDUMP
	printk("#######[dpram read : head - %04d, tail - %04d]######\n\n", head, up_tail);
	print_debug_current_time();
#endif

	onedram_release_lock(__func__);
	if (non_cmd & device->mask_req_ack)
		send_interrupt_to_phone_with_semaphore(INT_NON_COMMAND(device->mask_res_ack));

#ifdef CONFIG_EVENT_LOGGING
	dpram_event_logging(DPRAM_READ, (void *)&tail, size);
#endif	/* CONFIG_EVENT_LOGGING */

	return retval;
	
}

static void print_onedram_status(void)
{
	printk("onedram semaphore: %d(%s)\n", *onedram_sem, *onedram_sem ? "PDA" : "PHONE");
	printk("onedram mailboxAB: %x\n", *onedram_mailboxAB);
	printk("onedram mailboxBA: %x\n", *onedram_mailboxBA);
	printk("phone active pin : %s\n", (dpram_phone_getstatus() ? "ACTIVE" : "INACTIVE"));
}

static int onedram_get_semaphore(const char *func)
{
	int i, retry = 30;
	const u16 cmd = INT_COMMAND(INT_MASK_CMD_SMP_REQ);
	if(dump_on) return -1;

	for(i = 0; i < retry; i++) {
		if(*onedram_sem) return 1;
		*onedram_mailboxBA = cmd;
		mdelay(10);
	}

	printk("Failed to get a Semaphore (%s) sem:%d, phone status: %s\n", func, *onedram_sem,	(dpram_phone_getstatus() ? "ACTIVE" : "INACTIVE"));

	return 0;
}

static void send_interrupt_to_phone_with_semaphore(u16 irq_mask)
{
	if(dump_on) return;
	if(!atomic_read(&onedram_lock)) 
	{
		if(*onedram_sem) { 	
			*onedram_sem = 0x0;
			*onedram_mailboxBA = irq_mask;
			requested_semaphore = 0;
		}else {
			*onedram_mailboxBA = irq_mask;
		}
	}else {
		printk("[OneDRAM] (%s) lock set. can't return semaphore.\n", __func__);
	}
		

}

static int return_onedram_semaphore(const char* func)
{

	if(!atomic_read(&onedram_lock)) 
	{
		if(*onedram_sem) { 	
			*onedram_sem = 0x0;
			return 1;
		}
	}else {
		requested_semaphore++;
		printk("[OneDRAM] (%s) PDA is accessing onedram. %d\n", func, requested_semaphore);
	}

	return 0;
	
}

static int onedram_lock_with_semaphore(const char* func)
{
	int lock_value;

	if(!(lock_value = atomic_inc_return(&onedram_lock)))
		printk("[OneDRAM] (%s, lock) fail to locking onedram access. %d\n", func, lock_value);

	if(lock_value != 1)
		printk("[OneDRAM] (%s, lock) lock_value: %d\n", func, lock_value);

	if(*onedram_sem) 
		return 0;	
	else {
		printk("[OneDRAM] (%s, lock) failed.. no sem\n", func);
		if((lock_value = atomic_dec_return(&onedram_lock)) < 0)
			printk("[OneDRAM] (%s, lock) fail to unlocking onedram access. %d\n", func, lock_value);

		if(lock_value != 0)
			printk("[OneDRAM] (%s, lock) lock_value: %d\n", func, lock_value);
		return -1;
	}
}

static void onedram_release_lock(const char* func)
{
	int lock_value;

	if((lock_value = atomic_dec_return(&onedram_lock)) < 0)
		printk("[OneDRAM] (%s, release) fail to unlocking onedram access. %d\n", func, lock_value);

	if(requested_semaphore) {
		if(!atomic_read(&onedram_lock)) {
			if(*onedram_sem) { 	
				printk("[OneDRAM] (%s, release) requested semaphore(%d) return to Phone.\n", func, requested_semaphore);
				*onedram_sem = 0x0;
				requested_semaphore = 0;
			}
		}
	}

	if(lock_value != 0)
		printk("[OneDRAM] (%s, release) lock_value: %d\n", func, lock_value);

}

static int dpram_shared_bank_remap(void)
{
	dpram_base = ioremap_nocache(DPRAM_START_ADDRESS_PHYS + DPRAM_SHARED_BANK, DPRAM_SHARED_BANK_SIZE);
	if (dpram_base == NULL) {
		printk("failed ioremap\n");
		return -ENOENT;
		}
		
	onedram_sem = DPRAM_VBASE + DPRAM_SMP; 
	onedram_mailboxBA = DPRAM_VBASE + DPRAM_MBX_BA;
	onedram_mailboxAB = DPRAM_VBASE + DPRAM_MBX_AB;
	onedram_checkBA = DPRAM_VBASE + DPRAM_CHECK_BA;
	atomic_set(&onedram_lock, 0);
#if 0
	printk("onedram semaphore value = 0x%x\n", *onedram_sem);
	printk("onedram mailboxAB value = 0x%x\n", *onedram_mailboxAB);
	printk("onedram mailboxBA value = 0x%x\n", *onedram_mailboxBA);
	printk("onedram lock value = %d\n", atomic_read(&onedram_lock));
	atomic_inc(&onedram_lock);
	printk("onedram lock value after inc= %d\n", atomic_read(&onedram_lock));
	atomic_dec(&onedram_lock);
	printk("onedram lock value after dec= %d\n", atomic_read(&onedram_lock));
	printk("onedram lock value inc & test = %d\n", atomic_inc_and_test(&onedram_lock));
	printk("onedram lock value dec & test = %d\n", atomic_dec_and_test(&onedram_lock));
	printk("onedram lock value inc & return = %d\n", atomic_inc_return(&onedram_lock));
	printk("onedram lock value dec & return = %d\n", atomic_dec_return(&onedram_lock));
	printk("[DPRAM] ioremap success. dpram base addr = 0x%08x\n", dpram_base);
#endif

	return 0;
}

static int ReadPhoneFile(
	unsigned char *DblBuffer, 
	unsigned char *ImageBuffer, 
	unsigned long Dbl_Length, 
	unsigned long Total_length)
{
	int   nRet;
	XSRPartEntry stPartEntry;
	unsigned int nVol = 0;
	BMLVolSpec stVolSpec;
	int PAGE_PER_BLOCK;
	int BYTE_PER_PAGE;
	int SECTOR_PER_PAGE;
	unsigned long totalPageNumber = 0;
	unsigned long DblPageNumber = 0;
	unsigned long TotalBLKNumber = 0;
	unsigned int VStartSector;
	unsigned int VStartBlock;
	unsigned int BLKNumberPartition;


	BML_Open(0);

	//block entry 
	if (BML_LoadPIEntry(nVol, PARTITION_ID_MODEM_IMG, &stPartEntry) != BML_SUCCESS) {
		printk(" # FlashRead : BML_LoadPIEntry is failed\n");
		return FALSE;
	}
	//block get information
	if (BML_GetVolInfo(nVol, &stVolSpec) != BML_SUCCESS) {
		printk(" # EOND# FlashRead : BML_GetVolInfo is failed\n");
		return FALSE;
	}

	PAGE_PER_BLOCK = stVolSpec.nPgsPerBlk;
	SECTOR_PER_PAGE = stVolSpec.nSctsPerPg;
	BYTE_PER_PAGE = SECTOR_PER_PAGE * XSR_SECTOR_SIZE;

	//이미지 길이에 맞춰 총 page수 알아낸다.
	if(Dbl_Length % BYTE_PER_PAGE)
		printk(" # EOND spare length error \n");
	else
		DblPageNumber = Dbl_Length/BYTE_PER_PAGE;
	if(Total_length % BYTE_PER_PAGE)
		totalPageNumber = (Total_length/BYTE_PER_PAGE)+1;
	else
		totalPageNumber = Total_length/BYTE_PER_PAGE;

	// page 개수에 맞춰 총 Block 수를 알아낸다.
	if(totalPageNumber % PAGE_PER_BLOCK)
		TotalBLKNumber    = (totalPageNumber/PAGE_PER_BLOCK) + 1;
	else
		TotalBLKNumber    = totalPageNumber/PAGE_PER_BLOCK;

	VStartBlock = stPartEntry.n1stVbn;
	BLKNumberPartition = stPartEntry.nNumOfBlks;

	if(TotalBLKNumber > BLKNumberPartition)
		// block read
	{
		printk(" EOND: error --too long size to read\n");
		return FALSE;
	}

#if 0
	printk("  - Phone Image Info\n");
	printk("   - SectorIndex: %d\n", VStartSector + (DblPageNumber * SECTOR_PER_PAGE));
	printk("   - SectorsNumber: %d\n", (totalPageNumber - DblPageNumber) * SECTOR_PER_PAGE);
	printk("   - BLKNumber: %d\n", TotalBLKNumber);
	printk("   - Maxsize: %d\n", Total_length);
//	printk("   - Shared memory addr: %x\n", ImageBuffer);
#endif

	VStartSector = VStartBlock * PAGE_PER_BLOCK * SECTOR_PER_PAGE; //virtual start sector

#if 0
	nRet = BML_MRead(nVol,
			VStartSector,              /* Virtual Sector Number    */
			DblPageNumber * SECTOR_PER_PAGE,                /* Number of Sectors        */
			DblBuffer,                 /* Buffer pointer for Main  */
			NULL,                   /* Buffer pointer for Dbl */
			BML_FLAG_ECC_ON | BML_FLAG_BBM_ON);     /* Operation Flag           */
	if (nRet != BML_SUCCESS)
	{
		printk(" # FlashRead : BML_Read is failed\n");
		return FALSE;
	}
	// write log
#endif
	nRet = BML_MRead(nVol,
			VStartSector + (DblPageNumber * SECTOR_PER_PAGE),      /* Virtual Sector Number    */
			(totalPageNumber - DblPageNumber) * SECTOR_PER_PAGE,     /* Number of Sectors        */
			ImageBuffer,                 /* Buffer pointer for Main  */
			NULL,                   /* Buffer pointer for Dbl */
			BML_FLAG_ECC_ON | BML_FLAG_BBM_ON);     /* Operation Flag           */
	if (nRet != BML_SUCCESS)
	{
		printk("# FlashRead : BML_Read is failed\n");
		return FALSE;
	}

	return TRUE;
}

static void dpram_clear(void)
{
	long i = 0;
	unsigned long flags;
	
	u16 value = 0;

	/* @LDK@ clear DPRAM except interrupt area */
	local_irq_save(flags);

	for (i = DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS;
			i < DPRAM_SIZE - (DPRAM_INTERRUPT_PORT_SIZE * 2);
			i += 2)
	{
		*((u16 *)(DPRAM_VBASE + i)) = 0;
	}

	local_irq_restore(flags);

	value = *onedram_mailboxAB;
}

static int dpram_init_and_report(void)
{
	const u16 magic_code = 0x00aa;
	const u16 init_start = INT_COMMAND(INT_MASK_CMD_INIT_START);
	const u16 init_end = INT_COMMAND(INT_MASK_CMD_INIT_END);
	u16 ac_code = 0;

	if (!(*onedram_sem)) {
		printk("[OneDRAM] %s semaphore: %d\n", __func__, *onedram_sem);
		if(!onedram_get_semaphore(__func__)) {
			printk("[OneDRAM] %s failed to onedram init!!! semaphore: %d\n", __func__, *onedram_sem);
			return -1;
		}
	}

	/* @LDK@ send init start code to phone */
	if(onedram_lock_with_semaphore(__func__) < 0)
		return -1;

	*onedram_mailboxBA = init_start;
	printk("[OneDRAM] Send to MailboxBA 0x%x (onedram init start).\n", init_start);

	/* @LDK@ write DPRAM disable code */
	WRITE_TO_DPRAM(DPRAM_ACCESS_ENABLE_ADDRESS, &ac_code, sizeof(ac_code));

	/* @LDK@ dpram clear */
	dpram_clear();

	/* @LDK@ write magic code */
	WRITE_TO_DPRAM(DPRAM_MAGIC_CODE_ADDRESS,
			&magic_code, sizeof(magic_code));

	/* @LDK@ write DPRAM enable code */
	ac_code = 0x0001;
	WRITE_TO_DPRAM(DPRAM_ACCESS_ENABLE_ADDRESS, &ac_code, sizeof(ac_code));

	/* @LDK@ send init end code to phone */
	onedram_release_lock(__func__);
	send_interrupt_to_phone_with_semaphore(init_end);
	printk("[OneDRAM] Send to MailboxBA 0x%x (onedram init finish).\n", init_end);

	phone_sync = 1;
	return 0;
}

static inline int dpram_get_read_available(dpram_device_t *device)
{
	return 1;
}

static void dpram_drop_data(dpram_device_t *device)
{
	u16 head;


	if(*onedram_sem) {
	READ_FROM_DPRAM_VERIFY(&head, device->in_head_addr, sizeof(head));
	WRITE_TO_DPRAM_VERIFY(device->in_tail_addr, &head, sizeof(head));
	}
}

static void dpram_phone_image_load(void)
{
	
	gpio_set_value(GPIO_USIM_BOOT, GPIO_LEVEL_LOW);
	gpio_set_value(GPIO_CP_BOOT_SEL, GPIO_LEVEL_LOW);

	// printk(" +---------------------------------------------+\n");
	// printk(" |   CHECK DBL DOWNLOAD  &  LOAD PHONE IMAGE   |\n");
	// printk(" +---------------------------------------------+\n");
	printk("  - Waiting 0x12341234 in MailboxAB ...");
	while(1) {
		if(*onedram_mailboxAB == 0x12341234)
			break;
		mdelay(10);
		printk(".");
	}
	printk("Done.\n");

    if (!dump_on)
    {
	    if(*onedram_sem)
	 	    ReadPhoneFile(dbl_buf, dpram_base, MAX_DBL_IMG_SIZE, MAX_MODEM_IMG_SIZE);
	    else
		    printk("[OneDRAM] %s failed.. sem: %d\n", __func__, *onedram_sem);
    }
	else
	{
	    dprintk("CP DUMP MODE !!! \n");
	}

}
	
static void dpram_nvdata_load(struct _param_nv *param)
{
	// printk(" +---------------------------------------------+\n");
	printk(" |                  LOAD NVDATA                |\n");
	// printk(" +---------------------------------------------+\n");
	// printk("  - Read from File(\"/efs/nv_data.bin\").\n");
	// printk("  - Address of NV data(virt): 0x%08x.\n", param->addr);
	// printk("  - Size of NV data: %d.\n", param->size);
        
	int                   nRet;
    unsigned int          nPsn;
	unsigned int          n1stVSN;
	unsigned int          nPgsPerBlk;
	unsigned int          nSctsPerPg;
	BMLVolSpec            stVolSpec;

	if (!dump_on) 
	{
    	// printk("[DPRAM] Start getting DGS info.\n");
    
    	if (BML_Open(0) != BML_SUCCESS)
    	{
            printk("[DPRAM] BML_Open Error !!\n");
    		while(1);
    	}
    
    	if (BML_GetVolInfo(0, &stVolSpec) != BML_SUCCESS)
    	{
            printk("[DPRAM] BML_GetVolInfo Error !!\n");
    		while(1);
    	}
    
        printk("[DPRAM] Try to read DGS Info.\n");
    
    	// 4095: end of block.(4Gbit)
    	n1stVSN = 4095 * stVolSpec.nPgsPerBlk * stVolSpec.nSctsPerPg;
    	nPsn = n1stVSN + (5 * stVolSpec.nSctsPerPg); // skip 5 pages
    
    	nRet = ONLD_Read(0,                             /* Device Number    */
    	                 nPsn,                                   /* Psn              */
    	                 1,                             	 	 /* Sector Number    */
    	                 aDGSBuf,                              /* Main Buffer      */
    	                 NULL,                                  /* Spare Buffer     */
    	                 (BML_FLAG_SYNC_OP | BML_FLAG_ECC_OFF)); /* Op Flag          */ 
    
    	if(nRet != LLD_SUCCESS)
    	{
            printk("[DPRAM] Reading DGS information failed !!\n");
    		while(1);
    	}
    	if(*onedram_sem)
        {
    		WRITE_TO_DPRAM( 0xF80000 - 0x5000, param->addr, param->size);
            WRITE_TO_DPRAM( DPRAM_DGS_INFO_BLOCK_OFFSET, aDGSBuf, DPRAM_DGS_INFO_BLOCK_SIZE);   
        }
    	else
		    printk("[OneDRAM] %s failed.. sem: %d\n", __func__, *onedram_sem);
	}
	else
		dprintk("CP DUMP MODE !!! \n");
}
	
static void dpram_phone_power_on(void)
{
	if( phone_power_state ) {
		printk("[OneDram] phone off (before phone power on).\n");
		gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);
		gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);
		interruptible_sleep_on_timeout(&dpram_wait, 100);	//	mdelay(500);
		printk("[OneDram] phone rst low 500ms).\n");
	}
	
	printk("[OneDram] phone power on.\n");
	gpio_set_value(GPIO_CP_BOOT_SEL, GPIO_LEVEL_HIGH);
	gpio_set_value(GPIO_USIM_BOOT, GPIO_LEVEL_HIGH);

	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);
	interruptible_sleep_on_timeout(&dpram_wait, 40);	//	mdelay(200);

	gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_HIGH);
	interruptible_sleep_on_timeout(&dpram_wait, 6);		//	mdelay(30);
	
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_HIGH);
	interruptible_sleep_on_timeout(&dpram_wait, 100);	//	mdelay(500);

	gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);
	interruptible_sleep_on_timeout(&dpram_wait, 20);	//	mdelay(100);

	printk(" |      GPIO CONTROL FOR PHONE POWER ON        |\n");
	phone_power_state = 1;
}

static irqreturn_t dpram_irq_handler(int irq, void *dev_id);
static void dpram_phone_boot_start(void)
{
    volatile unsigned int send_mail;
	  volatile unsigned int retranmit_send_mail;
	  volatile unsigned int recieved_mail;
	  volatile unsigned int retry_recieved_mail;
	  
	  unsigned char *write_base = (unsigned char *)DPRAM_VBASE;
	  
	  int retval = 0;
	  
	  free_irq(IRQ_ONEDRAM_INT_N, NULL);
	  
	  if(!dump_on)
	      send_mail = 0x45674567;
	  else
		    send_mail = 0x34563456;
		    
	  retranmit_send_mail = 0x45674567;

	  *onedram_sem = 0x0;    
	  *onedram_mailboxBA = send_mail;

	  while(*onedram_checkBA == 0x1)
	      mdelay(10);

    if(!dump_on)
    {
	      printk(" Waiting 0xabcdabcd in MailboxAB ... \n");
		
	      while(1)
		    {
		  	    while(gpio_get_value(GPIO_ONEDRAM_INT_N))
               mdelay(10);
	      
	          recieved_mail = *onedram_mailboxAB;
	        
		        if(recieved_mail == 0xabcdabcd) 
			      {
			          // printk("Done.\n");
			          // printk(" +---------------------------------------------+\n");
			          printk(" |             PHONE BOOT COMPLETE              |\n");
			          // printk(" +---------------------------------------------+\n");
			          printk(" - MailboxAB : 0x%8x\n", *onedram_mailboxAB);
			    
			          retval = request_irq(IRQ_ONEDRAM_INT_N, dpram_irq_handler, IRQF_DISABLED, "dpram_irq", NULL);
			          if (retval)
				            printk("DPRAM interrupt handler failed.\n");
				        
			          mdelay(10);
			          boot_complete = 1;
			          break;
		        }
		        else if(recieved_mail == 0x7890abcd)
		        {
		        	  int i;
		        	  
		            /* Trying to read/write RAM */
		            printk("Recieved 0x7890abcd, Trying to read/write RAM.\n");  
		            if(*onedram_sem)
                {
                    for(i=255; i>=0; i--)
                    {
                       *write_base = i;
                       write_base++;
                    }
                    dpram_cp_dump(FIRST);
                    while(1);
                } 
                else 
                {
                	  printk("Need SMP\n");
                } 
		        }
		        else if(recieved_mail == 0x78907890)
		        {
		        	  printk("Received 0x78907890, Re-Send 0x45674567\n");
		        	  *onedram_sem = 0x0;
	              *onedram_mailboxBA = retranmit_send_mail;
		        }
		        else
		        {
		        	  mdelay(10);
		            retry_recieved_mail = *onedram_mailboxAB;
		            printk("retry_recieved_mail = 0x%x\n", retry_recieved_mail);
		            
		            if(retry_recieved_mail == 0xabcdabcd) 
		            {
		                retval = request_irq(IRQ_ONEDRAM_INT_N, dpram_irq_handler, IRQF_DISABLED, "dpram_irq", NULL);
			              if (retval)
				                printk("DPRAM interrupt handler failed.\n");
				        
			              mdelay(10);
			              boot_complete = 1;
			              break;
		            }
		            else
		            {
		            	    printk("Received Others, Re-Send 0x45674567\n");
		            	    *onedram_sem = 0x0;
	                    *onedram_mailboxBA = retranmit_send_mail;
		            }
		        }
	      }
    }
	else
	{
	    // DUMP MODE

		// FIRST DATA
	    printk("Waiting 1st Data");
	    while(gpio_get_value(GPIO_ONEDRAM_INT_N))
			printk(".");
		printk("\n");

		if(*onedram_mailboxAB == 0x23452345)
		{
		    printk("GOT DATA\n");

			if(*onedram_sem)
			{
				dpram_cp_dump(FIRST);

				*onedram_mailboxBA = send_mail;
	            *onedram_sem = 0x0;
			}
			else
			{
				printk("NEED SMP\n");
				while(1);
			}
				
		}
		else
		{
		    printk("STRANGE\n");
			while(1);
		}

        // SECOND DATA 
		printk("Waiting 2nd Data");
	    while(gpio_get_value(GPIO_ONEDRAM_INT_N))
			printk(".");
		printk("\n");

		if(*onedram_mailboxAB == 0x23452345)
		{
		    printk("GOT DATA\n");

			if(*onedram_sem)
			{
				dpram_cp_dump(SECOND);

				*onedram_mailboxBA = send_mail;
	            *onedram_sem = 0x0;
			}
			else
			{
				printk("NEED SMP\n");
				while(1);
			}
				
		}
		else
		{
		    printk("STRANGE\n");
			while(1);
		}

		// THIRD DATA 
		printk("Waiting 3rd Data");
	    while(gpio_get_value(GPIO_ONEDRAM_INT_N))
			printk(".");
		printk("\n");

		if(*onedram_mailboxAB == 0x23452345)
		{
		    printk("GOT DATA\n");

			if(*onedram_sem)
			{
				dpram_cp_dump(THIRD);
			}
			else
			{
				printk("NEED SMP\n");
				while(1);
			}
				
		}
		else
		{
		    printk("STRANGE\n");
			while(1);
		}    
	}
}

static void dpram_phone_power_off(void)
{
	dprintk("[OneDRAM] Phone power Off. - do nothing\n");
}

static int dpram_phone_getstatus(void)
{
	return gpio_get_value(GPIO_PHONE_ACTIVE);
}

static void dpram_phone_reset(void)
{
    dprintk("[OneDRAM] Phone power Reset.\n");
	if(*onedram_sem) {
		dprintk("[OneDRAM} semaphore: %d\n", *onedram_sem);
		*onedram_sem = 0x00;
	}
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);
	mdelay(100);
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_HIGH);

	// Wait until phone is stable
	mdelay(200);
}

static void dpram_mem_rw(struct _mem_param *param)
{
#if 0
	/* @LDK@ write */
	if (param->dir) {
		WRITE_TO_DPRAM(param->addr, (void *)&param->data, sizeof(param->data));
	}

	/* @LDK@ read */
	else {
		READ_FROM_DPRAM((void *)&param->data, param->addr, sizeof(param->data));
	}
#endif
}

static int dpram_phone_ramdump_on(void)
{
	const u16 rdump_flag1 = 0x554C;
	const u16 rdump_flag2 = 0x454D;
	const u16 temp1, temp2;
	
	dprintk("[OneDRAM] Ramdump ON.\n");

	if(!onedram_get_semaphore(__func__)) 
		return -1;

	if(onedram_lock_with_semaphore(__func__) < 0)
		return -1;

	WRITE_TO_DPRAM(DPRAM_MAGIC_CODE_ADDRESS,    &rdump_flag1, sizeof(rdump_flag1));
	WRITE_TO_DPRAM(DPRAM_ACCESS_ENABLE_ADDRESS, &rdump_flag2, sizeof(rdump_flag2));

#if 0
	READ_FROM_DPRAM((void *)&temp1, DPRAM_MAGIC_CODE_ADDRESS, sizeof(temp1));
	READ_FROM_DPRAM((void *)&temp2, DPRAM_ACCESS_ENABLE_ADDRESS, sizeof(temp2));
	printk("[OneDRAM] flag1: %x flag2: %x\n", temp1, temp2);
#endif

	/* @LDK@ send init end code to phone */
	onedram_release_lock(__func__);

	dump_on = 1;

	return_onedram_semaphore(__func__);
	if(*onedram_sem) {
		printk("[OneDRAM] Failed to return semaphore. try again\n");
		*onedram_sem = 0x00;
	}

	free_irq(IRQ_ONEDRAM_INT_N, NULL);
	free_irq(IRQ_PHONE_ACTIVE, NULL);
	free_irq(IRQ_SIM_DETECT_N, NULL);

	// dpram_phone_reset();
	return 0;

}

static int dpram_phone_ramdump_off(void)
{
	const u16 rdump_flag1 = 0x00aa;
	const u16 rdump_flag2 = 0x0001;
	const u16 temp1, temp2;

	dprintk("[OneDRAM] Ramdump OFF.\n");
	
	dump_on = 0;

	if(!onedram_get_semaphore(__func__)) 
		return -1;

	if(onedram_lock_with_semaphore(__func__) < 0)
		return -1;

	WRITE_TO_DPRAM(DPRAM_MAGIC_CODE_ADDRESS,    &rdump_flag1, sizeof(rdump_flag1));
	WRITE_TO_DPRAM(DPRAM_ACCESS_ENABLE_ADDRESS, &rdump_flag2, sizeof(rdump_flag2));
#if 0
	READ_FROM_DPRAM((void *)&temp1, DPRAM_MAGIC_CODE_ADDRESS, sizeof(temp1));
	READ_FROM_DPRAM((void *)&temp2, DPRAM_ACCESS_ENABLE_ADDRESS, sizeof(temp2));
	printk("[OneDRAM] flag1: %x flag2: %x\n", temp1, temp2);
#endif
	/* @LDK@ send init end code to phone */
	onedram_release_lock(__func__);
	
	phone_sync = 0;

//	*onedram_sem = 0x00;
	return_onedram_semaphore(__func__);
	if(*onedram_sem) {
		printk("[OneDRAM] Failed to return semaphore. try again\n");
		*onedram_sem = 0x00;
	}
    register_interrupt_handler();
	// dpram_phone_reset();

	return 0;

}

/* CP Dump Logic */
static void dpram_cp_dump(dump_order order)
{
    int                   nErr = BML_SUCCESS;
    unsigned int          nVol = 0;
    
    unsigned int          nCPLogStartBlock;
	unsigned int          nNumCPLogBlocks;

    unsigned int          pPartBuf[2] = {0, 0};
	unsigned int          pBufIn[3] = {0, 0, 0};
	unsigned int          i;
	unsigned int          nBytesReturned = 0;

	unsigned int          nSctsPerPg;
    unsigned int          nSctsPerBlk;

	XSRPartEntry          stPartEntry;
	BMLVolSpec            stVolSpec;

	unsigned int          *pSrcAddr = (unsigned int *)DPRAM_VBASE;

	unsigned int          nNumOfLogSector0 = (((16 * 1024 * 1024) - (2 * 1024)) / XSR_SECTOR_SIZE);
	unsigned int          nNumOfLogSector1 = ((2 * 1024 * 1024) / XSR_SECTOR_SIZE);

	unsigned int          nBlkNum, nSctNum;
    unsigned int          nStartVbn, nEndVbn;
    unsigned int          nStartVsn, nEndVsn;
	
	/* BML_Init */ 
	/*
	nErr = BML_Init();
	if ((nErr != BML_SUCCESS) && (nErr != BML_ALREADY_INITIALIZED))
	{
	    dprintk("BML_Init fail!!!\n");
	    while(1);
	}
    */

	/* BML_Open */
	if (BML_Open(nVol) != BML_SUCCESS)
	{
	    dprintk("BML_Open fail!!!\n");
	    while(1);
	}
			
	/* BML_LoadPIEntry */
	nErr = BML_LoadPIEntry(nVol, TEMP_PART_ID, &stPartEntry);
	if(nErr != BML_SUCCESS)
	{
	    dprintk("BML_LoadPIEntry fail!!!\n");
	    while(1);
	}
	else
	{
	    nCPLogStartBlock = stPartEntry.n1stVbn;
	    nNumCPLogBlocks = stPartEntry.nNumOfBlks;
	}

#if 0
    /* BML_LoadPIEntry - BML_IOCTL_SET_BLOCK_UNLOCK */
	dprintk("BML_IOCtl - BML_IOCTL_SET_BLOCK_UNLOCK\n");
	pBufIn[0] = 0;
	pBufIn[1] = nCPLogStartBlock;
	pBufIn[2] = nNumCPLogBlocks;
	
	nErr = BML_IOCtl(0, BML_IOCTL_SET_BLOCK_UNLOCK, (UINT8*)pBufIn, sizeof(UINT32) * 3, NULL, 0, &nBytesReturned);
	if(nErr != BML_SUCCESS)
	{
	    dprintk("BML_IOCtl fail!!!\n");
	    while(1);		
	}
#endif

    if(order == FIRST)
    {
        dprintk("BML_IOCtl - BML_IOCTL_CHANGE_PART_ATTR\n");
        pPartBuf[0] = TEMP_PART_ID;
    	pPartBuf[1] = BML_PI_ATTR_RW;
    	
        nErr = BML_IOCtl(0, BML_IOCTL_CHANGE_PART_ATTR,
    		(unsigned char *)&pPartBuf, sizeof(unsigned int) * 2, NULL, 0, &nBytesReturned);
    	if (nErr != BML_SUCCESS) 
    	{
    		dprintk("BML_EraseBlk fail!!!\n");
    		while(1);
    	}
    	
        /* BML_EraseBlk */
    	dprintk("BML_EraseBlk\n");
    	for(i=0 ; i < nNumCPLogBlocks ; i++)
    	{
    		nErr = BML_EraseBlk(0, (i+ nCPLogStartBlock), BML_FLAG_SYNC_OP);
    		if(nErr != BML_SUCCESS)
    		{
    		    dprintk("BML_EraseBlk fail!!!\n");
    		    while(1);
    		}
    	}
    }
	
    /* BML_GetVolInfo */
	dprintk("BML_GetVolInfo\n");
	if (BML_GetVolInfo(nVol, &stVolSpec) != BML_SUCCESS)
    {
        dprintk("BML_GetVolInfo fail!!!\n");
        while(1);
    }
	else
	{
	    nSctsPerPg  = stVolSpec.nSctsPerPg;
        nSctsPerBlk = stVolSpec.nPgsPerBlk * stVolSpec.nSctsPerPg;
	}

#if 0
	/* BML_LoadPIEntry - BML_IOCTL_SET_BLOCK_UNLOCK */
	dprintk("BML_IOCtl - BML_IOCTL_SET_BLOCK_UNLOCK\n");
	pBufIn[0] = 0;
	pBufIn[1] = nCPLogStartBlock;
	pBufIn[2] = nNumCPLogBlocks;
	
	nErr = BML_IOCtl(0, BML_IOCTL_SET_BLOCK_UNLOCK, (UINT8*)pBufIn, sizeof(UINT32) * 3, NULL, 0, &nBytesReturned);
	if(nErr != BML_SUCCESS)
	{
	    dprintk("BML_IOCtl fail!!!\n");
	    while(1);		
	}
#endif
	   
    /* Write the first CP Log */
	dprintk("BML_Write\n");
    if (order == FIRST)
        nStartVsn = (nCPLogStartBlock) * nSctsPerBlk;
	else if (order == SECOND)
		nStartVsn = (nCPLogStartBlock) * nSctsPerBlk + nNumOfLogSector0;
	else
		nStartVsn = (nCPLogStartBlock) * nSctsPerBlk + (2 * nNumOfLogSector0);
	
	if (order == THIRD)
        nEndVsn   = nStartVsn + nNumOfLogSector1;
	else
        nEndVsn   = nStartVsn + nNumOfLogSector0;

	dprintk("nStartVsn = %d\n", nStartVsn);
	dprintk("nEndVsn = %d\n", nEndVsn);
	
    for (nSctNum = nStartVsn; nSctNum < nEndVsn; nSctNum += nSctsPerPg)
    {
        nErr = BML_Write(nVol,                                  /* Volum Number             */
                         nSctNum,                               /* Sector Number            */
                         nSctsPerPg,                            /* Number of Sectors to Write */
                         pSrcAddr,                              /* Pointer to Main data     */
                         NULL,                                  /* Pointer to Spare data    */
                         BML_FLAG_SYNC_OP | BML_FLAG_ECC_ON);   /* Operation Flag           */

        if (nErr != BML_SUCCESS)
        {
            dprintk("BML_Write fail!!!\n");
            while(1);
        }

        pSrcAddr += (XSR_SECTOR_SIZE * nSctsPerPg) / 4;
    }

	/* BML_Close */
	if (BML_Close(nVol) != BML_SUCCESS)
	{
	    dprintk("BML_Close fail!!!\n");
	    while(1);
	}

	dprintk("BML Operation Done!!!\n");
}

#ifdef CONFIG_PROC_FS
static int dpram_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	char *p = page;
	int len;

	u16 magic, enable;
	u16 fmt_in_head, fmt_in_tail, fmt_out_head, fmt_out_tail;
	u16 raw_in_head, raw_in_tail, raw_out_head, raw_out_tail;
	u16 in_interrupt = 0, out_interrupt = 0;

	int foh, fot, roh, rot, sem;

#ifdef _ENABLE_ERROR_DEVICE
	char buf[DPRAM_ERR_MSG_LEN];
	unsigned long flags;
#endif	/* _ENABLE_ERROR_DEVICE */
	
#if 0
	READ_FROM_DPRAM((void *)&magic, DPRAM_MAGIC_CODE_ADDRESS, sizeof(magic));
	READ_FROM_DPRAM((void *)&enable, DPRAM_ACCESS_ENABLE_ADDRESS, sizeof(enable));
	READ_FROM_DPRAM((void *)&fmt_in_head, DPRAM_PHONE2PDA_FORMATTED_HEAD_ADDRESS, 
			sizeof(fmt_in_head));
	READ_FROM_DPRAM((void *)&fmt_in_tail, DPRAM_PHONE2PDA_FORMATTED_TAIL_ADDRESS, 
		    sizeof(fmt_in_tail));
	READ_FROM_DPRAM((void *)&fmt_out_head, DPRAM_PDA2PHONE_FORMATTED_HEAD_ADDRESS, 
		    sizeof(fmt_out_head));
	READ_FROM_DPRAM((void *)&fmt_out_tail, DPRAM_PDA2PHONE_FORMATTED_TAIL_ADDRESS, 
		    sizeof(fmt_out_tail));

	READ_FROM_DPRAM((void *)&raw_in_head, DPRAM_PHONE2PDA_RAW_HEAD_ADDRESS, 
		    sizeof(raw_in_head));
	READ_FROM_DPRAM((void *)&raw_in_tail, DPRAM_PHONE2PDA_RAW_TAIL_ADDRESS, 
		    sizeof(raw_in_tail));
	READ_FROM_DPRAM((void *)&raw_out_head, DPRAM_PDA2PHONE_RAW_HEAD_ADDRESS, 
		    sizeof(raw_out_head));
	READ_FROM_DPRAM((void *)&raw_out_tail, DPRAM_PDA2PHONE_RAW_TAIL_ADDRESS, 
		    sizeof(raw_out_tail));
#endif

	foh = dpram_table[FORMATTED_INDEX].out_head_saved;
	fot = dpram_table[FORMATTED_INDEX].out_tail_saved;
	roh = dpram_table[RAW_INDEX].out_head_saved;
	rot = dpram_table[RAW_INDEX].out_tail_saved;
	sem = *onedram_sem;

	in_interrupt = *onedram_mailboxAB;
	out_interrupt = *onedram_mailboxBA;

#ifdef _ENABLE_ERROR_DEVICE
	memset((void *)buf, '\0', DPRAM_ERR_MSG_LEN);
	local_irq_save(flags);
	memcpy(buf, dpram_err_buf, DPRAM_ERR_MSG_LEN - 1);
	local_irq_restore(flags);
#endif	/* _ENABLE_ERROR_DEVICE */

	p += sprintf(p,
			"-------------------------------------\n"
			"| NAME\t\t\t| VALUE\n"
			"-------------------------------------\n"
#if 0
			"| MAGIC CODE\t\t| 0x%04x\n"
			"| ENABLE CODE\t\t| 0x%04x\n"
			"| PHONE->PDA FMT HEAD\t| %u\n"
			"| PHONE->PDA FMT TAIL\t| %u\n"
			"| PDA->PHONE FMT HEAD\t| %u\n"
			"| PDA->PHONE FMT TAIL\t| %u\n"
			"| PHONE->PDA RAW HEAD\t| %u\n"
			"| PHONE->PDA RAW TAIL\t| %u\n"
			"| PDA->PHONE RAW HEAD\t| %u\n"
			"| PDA->PHONE RAW TAIL\t| %u\n"
#endif
			"| Onedram Semaphore\t| %d\n"
			"| requested Semaphore\t| %d\n"
			"| PDA->PHONE FMT HEAD\t| %d\n"
			"| PDA->PHONE FMT TAIL\t| %d\n"
			"| PDA->PHONE RAW HEAD\t| %d\n"
			"| PDA->PHONE RAW TAIL\t| %d\n"

			"| PHONE->PDA MAILBOX\t| 0x%04x\n"
			"| PDA->PHONE MAILBOX\t| 0x%04x\n"
#ifdef _ENABLE_ERROR_DEVICE
			"| LAST PHONE ERR MSG\t| %s\n"
#endif	/* _ENABLE_ERROR_DEVICE */
			"| PHONE ACTIVE\t\t| %s\n"
			"| DPRAM INT Level\t| %d\n"
			"-------------------------------------\n",
#if 0
			magic, enable,
			fmt_in_head, fmt_in_tail, fmt_out_head, fmt_out_tail,
			raw_in_head, raw_in_tail, raw_out_head, raw_out_tail,
#endif
			sem, 
			requested_semaphore,
			foh, fot, roh, rot,
			in_interrupt, out_interrupt,

#ifdef _ENABLE_ERROR_DEVICE
			(buf[0] != '\0' ? buf : "NONE"),
#endif	/* _ENABLE_ERROR_DEVICE */

			(dpram_phone_getstatus() ? "ACTIVE" : "INACTIVE"),
				gpio_get_value(IRQ_PHONE_ACTIVE)
		);

	len = (p - page) - off;
	if (len < 0) {
		len = 0;
	}

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}
#endif /* CONFIG_PROC_FS */

/* dpram tty file operations. */
static int dpram_tty_open(struct tty_struct *tty, struct file *file)
{
	dpram_device_t *device = &dpram_table[tty->index];

	device->serial.tty = tty;
	device->serial.open_count++;

	if (device->serial.open_count > 1) {
		device->serial.open_count--;
		return -EBUSY;
	}


	tty->driver_data = (void *)device;
	tty->low_latency = 1;
	return 0;
}

static void dpram_tty_close(struct tty_struct *tty, struct file *file)
{
	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (device && (device == &dpram_table[tty->index])) {
		down(&device->serial.sem);
		device->serial.open_count--;
		device->serial.tty = NULL;
		up(&device->serial.sem);
	}
}

static int dpram_tty_write(struct tty_struct *tty,
		const unsigned char *buffer, int count)
{
	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (!device) {
		return 0;
	}

	return dpram_write(device, buffer, count);
}

static int dpram_tty_write_room(struct tty_struct *tty)
{
	int avail;
	u16 head, tail;

	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (device != NULL) {
#if 0
		onedram_lock_with_semaphore();

		READ_FROM_DPRAM_VERIFY(&head, device->out_head_addr, sizeof(head));
		READ_FROM_DPRAM_VERIFY(&tail, device->out_tail_addr, sizeof(tail));

		onedram_release_lock();
#else
		head = device->out_head_saved;
		tail = device->out_tail_saved;
#endif
		avail = (head < tail) ? tail - head - 1 :
			device->out_buff_size + tail - head - 1;

		return avail;
	}

	return 0;
}


static int dpram_tty_ioctl(struct tty_struct *tty, struct file *file,
		unsigned int cmd, unsigned long arg)
{
	unsigned int val;
	
	switch (cmd) {
		case DPRAM_PHONE_ON:
			// printk(" +---------------------------------------------+\n");
			printk(" |   INIT ONEDRAM  &  READY TO TRANSFER DATA   |\n");
			// printk(" +---------------------------------------------+\n");
			// printk("  - received DPRAM_PHONE_ON ioctl.\n");

			dump_on = 0;
			gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);

			if(boot_complete) {
				if(dpram_init_and_report() < 0) {
					printk("  - Failed.. unexpected error when ipc transfer start.\n");
					return -1;
				}
				printk("  - OK! IPC TRANSER START..!\n");
			}else {
				printk("  - Failed.. plz.. booting modem first.\n");
			}
			return 0;

		case DPRAM_PHONE_GETSTATUS:
			val = dpram_phone_getstatus();
			return copy_to_user((unsigned int *)arg, &val, sizeof(val));
			
		case DPRAM_PHONE_POWON:
			phone_sync = 0;
			dpram_phone_power_on();
			return 0;

		case DPRAM_PHONEIMG_LOAD:
			dpram_phone_image_load();
			return 0;

		case DPRAM_PHONE_BOOTSTART:
			dpram_phone_boot_start();
			return 0;

		case DPRAM_NVDATA_LOAD:
		{
			struct _param_nv param;

			val = copy_from_user((void *)&param, (void *)arg, sizeof(param));
			dpram_nvdata_load(&param);
			return 0;
		}
        case DPRAM_GET_DGS_INFO:
        {
            // Copy data to user
            printk("[DPRAM] Sending DGS info.\n");
            val = copy_to_user((void __user *)arg, aDGSBuf, DPRAM_DGS_INFO_BLOCK_SIZE);
			return 0;
        }
		case DPRAM_PHONE_RESET:
			dpram_phone_reset();
			return 0;

		case DPRAM_PHONE_OFF:
			dpram_phone_power_off();
			return 0;

		case DPRAM_PHONE_RAMDUMP_ON:
			dpram_phone_ramdump_on();
			return 0;

		case DPRAM_PHONE_RAMDUMP_OFF:
			dpram_phone_ramdump_off();
			return 0;

		default:
			// dprintk("Unknown Cmd !!!\n");
			break;
	}

	return -ENOIOCTLCMD;
}

static int dpram_tty_chars_in_buffer(struct tty_struct *tty)
{
	int data;
	u16 head, tail;

	dpram_device_t *device = (dpram_device_t *)tty->driver_data;

	if (device != NULL) {
#if 0
		onedram_lock_with_semaphore();

		READ_FROM_DPRAM_VERIFY(&head, device->out_head_addr, sizeof(head));
		READ_FROM_DPRAM_VERIFY(&tail, device->out_tail_addr, sizeof(tail));

		onedram_release_lock();
#else
		head = device->out_head_saved;
		tail = device->out_tail_saved;
#endif
		data = (head > tail) ? head - tail - 1 :
			device->out_buff_size - tail + head;

		return data;
	}

	return 0;
}

#ifdef _ENABLE_ERROR_DEVICE
static int dpram_err_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
{
	DECLARE_WAITQUEUE(wait, current);

	unsigned long flags;
	ssize_t ret;
	size_t ncopy;

	add_wait_queue(&dpram_err_wait_q, &wait);
	set_current_state(TASK_INTERRUPTIBLE);

	while (1) {
		local_irq_save(flags);

		if (dpram_err_len) {
			ncopy = min(count, dpram_err_len);

			if (copy_to_user(buf, dpram_err_buf, ncopy)) {
				ret = -EFAULT;
			}

			else {
				ret = ncopy;
			}

			dpram_err_len = 0;
			
			local_irq_restore(flags);
			break;
		}

		local_irq_restore(flags);

		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			break;
		}

		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			break;
		}

		schedule();
	}

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&dpram_err_wait_q, &wait);

	return ret;
}

static int dpram_err_fasync(int fd, struct file *filp, int mode)
{
	return fasync_helper(fd, filp, mode, &dpram_err_async_q);
}

static unsigned int dpram_err_poll(struct file *filp,
		struct poll_table_struct *wait)
{
	poll_wait(filp, &dpram_err_wait_q, wait);
	return ((dpram_err_len) ? (POLLIN | POLLRDNORM) : 0);
}
#endif	/* _ENABLE_ERROR_DEVICE */

/* handlers. */
static void res_ack_tasklet_handler(unsigned long data)
{
	dpram_device_t *device = (dpram_device_t *)data;

	if (device && device->serial.tty) {
		struct tty_struct *tty = device->serial.tty;

		if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
				tty->ldisc->ops->write_wakeup) {
			(tty->ldisc->ops->write_wakeup)(tty);
		}

		wake_up_interruptible(&tty->write_wait);
	}
}

static void send_tasklet_handler(unsigned long data)
{
	dpram_tasklet_data_t *tasklet_data = (dpram_tasklet_data_t *)data;

	dpram_device_t *device = tasklet_data->device;
	u16 non_cmd = tasklet_data->non_cmd;

	int ret = 0;

	if (device != NULL && device->serial.tty) {
		struct tty_struct *tty = device->serial.tty;
      do {
         ret = dpram_read(device,non_cmd); 
         if (ret == -1){
             if (non_cmd & INT_MASK_SEND_F) {
                 tasklet_schedule(&fmt_send_tasklet);
             }
             if (non_cmd & INT_MASK_SEND_R) {
                 tasklet_schedule(&raw_send_tasklet);
             }
             return;
         }
      } while(ret); 

      tty_flip_buffer_push(tty);
	}
	else {
		dpram_drop_data(device);
	}
}

static void cmd_req_active_handler(void)
{
#if 0
	send_interrupt_to_phone(INT_COMMAND(INT_MASK_CMD_RES_ACTIVE));
#else
	send_interrupt_to_phone_with_semaphore(INT_COMMAND(INT_MASK_CMD_RES_ACTIVE));

#endif
}

static void cmd_error_display_handler(void)
{

#ifdef _ENABLE_ERROR_DEVICE
	char buf[DPRAM_ERR_MSG_LEN];
	unsigned long flags;

	phone_sync = 0;

	if(*onedram_sem) {
		memset((void *)buf, 0, sizeof (buf));
		buf[0] = '1';
		buf[1] = ' ';

		READ_FROM_DPRAM((buf + 2), DPRAM_PHONE2PDA_FORMATTED_BUFFER_ADDRESS,
				sizeof (buf) - 3);


		printk("[PHONE ERROR] ->> %s\n", buf);

		local_irq_save(flags);
		memcpy(dpram_err_buf, buf, DPRAM_ERR_MSG_LEN);
		dpram_err_len = 64;
		local_irq_restore(flags);

		wake_up_interruptible(&dpram_err_wait_q);
		kill_fasync(&dpram_err_async_q, SIGIO, POLL_IN);
	}
#endif	/* _ENABLE_ERROR_DEVICE */

}

static void cmd_phone_start_handler(void)
{
#if 0
	static int phone_start = 0;
	if(!phone_start) {
		printk("  - Received a Phone start command.\n  - Waiting for Shared Memory Init.\n");
		phone_start = 1;
	}
	if(!phone_sync) {
        dpram_init_and_report();
    }
#else
	printk("  - Received 0xc8 from MailboxAB (Phone Boot OK).\n");
#endif
}

static void cmd_req_time_sync_handler(void)
{
	/* TODO: add your codes here.. */
}

static void cmd_phone_deep_sleep_handler(void)
{
	/* TODO: add your codes here.. */
}

static void cmd_nv_rebuilding_handler(void)
{
	/* TODO: add your codes here.. */
}

static void cmd_emer_down_handler(void)
{
	/* TODO: add your codes here.. */
}

static void cmd_smp_req_handler(void)
{
	const u16 cmd = INT_COMMAND(INT_MASK_CMD_SMP_REP);
	if(return_onedram_semaphore(__func__))
		*onedram_mailboxBA = cmd;
}

static void cmd_smp_rep_handler(void)
{
	/* TODO: add your codes here.. */
}

static void semaphore_control_handler(unsigned long data)
{
	const u16 cmd = INT_COMMAND(INT_MASK_CMD_SMP_REP);

	if(return_onedram_semaphore(__func__))
		*onedram_mailboxBA = cmd;
}


static void command_handler(u16 cmd)
{
	switch (cmd) {
		case INT_MASK_CMD_REQ_ACTIVE:
			cmd_req_active_handler();
			break;

		case INT_MASK_CMD_ERR_DISPLAY:
			cmd_error_display_handler();
			break;

		case INT_MASK_CMD_PHONE_START:
			cmd_phone_start_handler();
			break;

		case INT_MASK_CMD_REQ_TIME_SYNC:
			cmd_req_time_sync_handler();
			break;

		case INT_MASK_CMD_PHONE_DEEP_SLEEP:
			cmd_phone_deep_sleep_handler();
			break;

		case INT_MASK_CMD_NV_REBUILDING:
			cmd_nv_rebuilding_handler();
			break;

		case INT_MASK_CMD_EMER_DOWN:
			cmd_emer_down_handler();
			break;

		case INT_MASK_CMD_SMP_REQ:
			tasklet_schedule(&semaphore_control_tasklet);
//			cmd_smp_req_handler();
			break;

		case INT_MASK_CMD_SMP_REP:
			cmd_smp_rep_handler();
			break;

		default:
			// dprintk("Unknown command.. %x\n", cmd);
			break;
	}
}

static void non_command_handler(u16 non_cmd)
{
	u16 head, tail;

	/* @LDK@ formatted check. */
/*
	if(!(*onedram_sem)) {
		printk("[OneDRAM] %s failed! no sem. cmd: %x\n", __func__, non_cmd);
		return;
	}
*/
    if(!onedram_get_semaphore(__func__)) 
		return -1;

	READ_FROM_DPRAM_VERIFY(&head, DPRAM_PHONE2PDA_FORMATTED_HEAD_ADDRESS, sizeof(head));
	READ_FROM_DPRAM_VERIFY(&tail, DPRAM_PHONE2PDA_FORMATTED_TAIL_ADDRESS, sizeof(tail));

	if (head != tail)
		non_cmd |= INT_MASK_SEND_F;

	/* @LDK@ raw check. */
	READ_FROM_DPRAM_VERIFY(&head, DPRAM_PHONE2PDA_RAW_HEAD_ADDRESS, sizeof(head));
	READ_FROM_DPRAM_VERIFY(&tail, DPRAM_PHONE2PDA_RAW_TAIL_ADDRESS, sizeof(tail));

	if (head != tail)
		non_cmd |= INT_MASK_SEND_R;

	/* @LDK@ +++ scheduling.. +++ */
	if (non_cmd & INT_MASK_SEND_F) {
		wake_lock_timeout(&dpram_wake_lock, HZ/2);
		dpram_tasklet_data[FORMATTED_INDEX].device = &dpram_table[FORMATTED_INDEX];
		dpram_tasklet_data[FORMATTED_INDEX].non_cmd = non_cmd;
		
		fmt_send_tasklet.data = (unsigned long)&dpram_tasklet_data[FORMATTED_INDEX];
		tasklet_schedule(&fmt_send_tasklet);
	}

	if (non_cmd & INT_MASK_SEND_R) {
		wake_lock_timeout(&dpram_wake_lock, 4*HZ);
		dpram_tasklet_data[RAW_INDEX].device = &dpram_table[RAW_INDEX];
		dpram_tasklet_data[RAW_INDEX].non_cmd = non_cmd;

		raw_send_tasklet.data = (unsigned long)&dpram_tasklet_data[RAW_INDEX];
		/* @LDK@ raw buffer op. -> soft irq level. */
		tasklet_hi_schedule(&raw_send_tasklet);
	}

	if (non_cmd & INT_MASK_RES_ACK_F) {
		wake_lock_timeout(&dpram_wake_lock, HZ/2);
		tasklet_schedule(&fmt_res_ack_tasklet);
	}

	if (non_cmd & INT_MASK_RES_ACK_R) {
		wake_lock_timeout(&dpram_wake_lock, 4*HZ);
		tasklet_hi_schedule(&raw_res_ack_tasklet);
	}

}

static inline
void check_int_pin_level(void)
{
	u16 mask = 0, cnt = 0;

	while (cnt++ < 3) {
		mask = *onedram_mailboxAB;
		if (gpio_get_value(GPIO_ONEDRAM_INT_N))
			break;
	}
}

/* @LDK@ interrupt handlers. */
static irqreturn_t dpram_irq_handler(int irq, void *dev_id)
{
	u16 irq_mask = 0;

	irq_mask = *onedram_mailboxAB;
	check_int_pin_level();

	/* valid bit verification. @LDK@ */
	if (!(irq_mask & INT_MASK_VALID)) {
		if(irq_mask == 0x1234) {
			printk("  - Received 0x12341234 from MailboxAB (DBL download OK).\n");
//			return_onedram_semaphore(__func__);
			return IRQ_HANDLED;
		}
		else {
			printk("Invalid interrupt mask: 0x%04x\n", irq_mask);
			return IRQ_NONE;
		}
	}

	/* command or non-command? @LDK@ */
	if (irq_mask & INT_MASK_COMMAND) {
		irq_mask &= ~(INT_MASK_VALID | INT_MASK_COMMAND);
		wake_lock_timeout(&dpram_wake_lock, HZ/2);
		command_handler(irq_mask);
	}
	else {
		irq_mask &= ~INT_MASK_VALID;
		non_command_handler(irq_mask);
	}

	return IRQ_HANDLED;
}

static irqreturn_t phone_active_irq_handler(int irq, void *dev_id)
{
	// printk("  - [IRQ_CHECK] IRQ_PHONE_ACTIVE is Detected, level: %s\n", gpio_get_value(GPIO_PHONE_ACTIVE)?"HIGH":"LOW");

#ifdef _ENABLE_ERROR_DEVICE
	if((phone_sync) && (!gpio_get_value(GPIO_PHONE_ACTIVE)))
	{
		char buf[DPRAM_ERR_MSG_LEN];
		unsigned long flags;

		phone_sync = 0;

		memset((void *)buf, 0, sizeof (buf));
	  buf[0] = '9';
	  buf[1] = ' ';
    
	  memcpy(buf+2, "$PHONE-RESET", sizeof("$PHONE-RESET"));
	  
	  printk("[PHONE ERROR] ->> %s\n", buf);

		local_irq_save(flags);
		memcpy(dpram_err_buf, buf, DPRAM_ERR_MSG_LEN);
		dpram_err_len = 64;
		local_irq_restore(flags);

		wake_up_interruptible(&dpram_err_wait_q);
		kill_fasync(&dpram_err_async_q, SIGIO, POLL_IN);
	}
#endif
	return IRQ_HANDLED;
}

static irqreturn_t sim_detect_irq_handler(int irq, void *dev_id)
{
#ifdef _ENABLE_ERROR_DEVICE
	char buf[DPRAM_ERR_MSG_LEN];
#endif
	unsigned long flags;

	int new_sim_state = 0;
	int i;
	
	new_sim_state = gpio_get_value(GPIO_SIM_DETECT_N);

	if (new_sim_state != sim_state)
	{
	
#ifdef _ENABLE_ERROR_DEVICE
    	if (!new_sim_state)
    	{
    	    memset((void *)buf, 0, sizeof (buf));
    	    buf[0] = '3';
    	    buf[1] = ' ';
    
    	    memcpy(buf+2, "$SIM-ATTACHED", sizeof("$SIM-ATTACHED"));
    
    	    printk("[SIM ATTACHED] ->> %s\n", buf);
    
    	    local_irq_save(flags);
    	    memcpy(dpram_err_buf, buf, DPRAM_ERR_MSG_LEN);
    	    dpram_err_len = 64;
    	    local_irq_restore(flags);
    
    	    wake_up_interruptible(&dpram_err_wait_q);
    	    kill_fasync(&dpram_err_async_q, SIGIO, POLL_IN);
    	}
        else
#endif        
        {
            for( i = 0 ; i < 3 ; i++ ) {
				mdelay(7);

				if (!gpio_get_value(GPIO_SIM_DETECT_N))
				    return IRQ_HANDLED;

				mdelay(200);

				if (!gpio_get_value(GPIO_SIM_DETECT_N))
					return IRQ_HANDLED;
            }
			
#ifdef _ENABLE_ERROR_DEVICE
            memset((void *)buf, 0, sizeof (buf));
    	    buf[0] = '2';
    	    buf[1] = ' ';
    
    	    memcpy(buf+2, "$SIM-DETACHED", sizeof("$SIM-DETACHED"));
    
    	    printk("[SIM DETACHED] ->> %s\n", buf);
    
    	    local_irq_save(flags);
    	    memcpy(dpram_err_buf, buf, DPRAM_ERR_MSG_LEN);
    	    dpram_err_len = 64;
    	    local_irq_restore(flags);
    
    	    wake_up_interruptible(&dpram_err_wait_q);
    	    kill_fasync(&dpram_err_async_q, SIGIO, POLL_IN);
#endif
        }

		sim_state = new_sim_state;
	}
	
	return IRQ_HANDLED;
}

static irqreturn_t pm_irq_handler(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

/* basic functions. */
#ifdef _ENABLE_ERROR_DEVICE
static struct file_operations dpram_err_ops = {
	.owner = THIS_MODULE,
	.read = dpram_err_read,
	.fasync = dpram_err_fasync,
	.poll = dpram_err_poll,
	.llseek = no_llseek,

	/* TODO: add more operations */
};
#endif	/* _ENABLE_ERROR_DEVICE */

static struct tty_operations dpram_tty_ops = {
	.open 		= dpram_tty_open,
	.close 		= dpram_tty_close,
	.write 		= dpram_tty_write,
	.write_room = dpram_tty_write_room,
	.ioctl 		= dpram_tty_ioctl,
	.chars_in_buffer = dpram_tty_chars_in_buffer,

	/* TODO: add more operations */
};

#ifdef _ENABLE_ERROR_DEVICE

static void unregister_dpram_err_device(void)
{
	unregister_chrdev(DRIVER_MAJOR_NUM, DPRAM_ERR_DEVICE);
	class_destroy(dpram_class);
}

static int register_dpram_err_device(void)
{
	/* @LDK@ 1 = formatted, 2 = raw, so error device is '0' */
	struct device *dpram_err_dev_t;
	int ret = register_chrdev(DRIVER_MAJOR_NUM, DPRAM_ERR_DEVICE, &dpram_err_ops);

	if ( ret < 0 )
	{
		return ret;
	}

	dpram_class = class_create(THIS_MODULE, "err");

if (IS_ERR(dpram_class))
	{
		unregister_dpram_err_device();
		return -EFAULT;
	}

	dpram_err_dev_t = device_create(dpram_class, NULL,
			MKDEV(DRIVER_MAJOR_NUM, 0), NULL, DPRAM_ERR_DEVICE);

	if (IS_ERR(dpram_err_dev_t))
	{
		unregister_dpram_err_device();
		return -EFAULT;
	}

	return 0;
}
#endif	/* _ENABLE_ERROR_DEVICE */

static int register_dpram_driver(void)
{
	int retval = 0;

	/* @LDK@ allocate tty driver */
	dpram_tty_driver = alloc_tty_driver(MAX_INDEX);

	if (!dpram_tty_driver) {
		return -ENOMEM;
	}

	/* @LDK@ initialize tty driver */
	dpram_tty_driver->owner = THIS_MODULE;
	dpram_tty_driver->magic = TTY_DRIVER_MAGIC;
	dpram_tty_driver->driver_name = DRIVER_NAME;
	dpram_tty_driver->name = "dpram";
	dpram_tty_driver->major = DRIVER_MAJOR_NUM;
	dpram_tty_driver->minor_start = 1;
	dpram_tty_driver->num = 2;
	dpram_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	dpram_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	dpram_tty_driver->flags = TTY_DRIVER_REAL_RAW;
	dpram_tty_driver->init_termios = tty_std_termios;
	dpram_tty_driver->init_termios.c_cflag =
		(B115200 | CS8 | CREAD | CLOCAL | HUPCL);

	tty_set_operations(dpram_tty_driver, &dpram_tty_ops);

	dpram_tty_driver->ttys = dpram_tty;
	dpram_tty_driver->termios = dpram_termios;
	dpram_tty_driver->termios_locked = dpram_termios_locked;

	/* @LDK@ register tty driver */
	retval = tty_register_driver(dpram_tty_driver);

	if (retval) {
		dprintk("tty_register_driver error\n");
		put_tty_driver(dpram_tty_driver);
		return retval;
	}

	return 0;
}

static void unregister_dpram_driver(void)
{
	tty_unregister_driver(dpram_tty_driver);
}

static void init_devices(void)
{
	int i;

	for (i = 0; i < MAX_INDEX; i++) {
		init_MUTEX(&dpram_table[i].serial.sem);

		dpram_table[i].serial.open_count = 0;
		dpram_table[i].serial.tty = NULL;
	}
}

static void init_hw_setting(void)
{
	u32 mask;

	/* initial pin settings - dpram driver control */
	s3c_gpio_cfgpin(GPIO_PHONE_ACTIVE, S3C_GPIO_SFN(GPIO_PHONE_ACTIVE_AF));
	s3c_gpio_setpull(GPIO_PHONE_ACTIVE, S3C_GPIO_PULL_NONE); 
	set_irq_type(IRQ_PHONE_ACTIVE, IRQ_TYPE_EDGE_BOTH);

	s3c_gpio_cfgpin(GPIO_ONEDRAM_INT_N, S3C_GPIO_SFN(GPIO_ONEDRAM_INT_N_AF));
	s3c_gpio_setpull(GPIO_ONEDRAM_INT_N, S3C_GPIO_PULL_UP); 
//	s3c_gpio_setpull(GPIO_ONEDRAM_INT_N, S3C_GPIO_PULL_NONE); 
#if 0
	set_irq_type(IRQ_ONEDRAM_INT_N, IRQ_TYPE_LEVEL_LOW);
#else
	set_irq_type(IRQ_ONEDRAM_INT_N, IRQ_TYPE_EDGE_FALLING);
//	mask = __raw_readl(S3C64XX_EINT0FLTCON0);
//	mask |= 0x000000c3;
//	__raw_writel(mask, S3C64XX_EINT0FLTCON0);
#endif

	s3c_gpio_cfgpin(GPIO_SIM_DETECT_N, S3C_GPIO_SFN(GPIO_SIM_DETECT_N_AF));
	s3c_gpio_setpull(GPIO_SIM_DETECT_N, S3C_GPIO_PULL_NONE); 
	set_irq_type(IRQ_SIM_DETECT_N, IRQ_TYPE_EDGE_BOTH);

	s3c_gpio_cfgpin(GPIO_FLM_RXD, S3C_GPIO_SFN(GPIO_FLM_RXD_AF));
	s3c_gpio_setpull(GPIO_FLM_RXD, S3C_GPIO_PULL_NONE); 

	s3c_gpio_cfgpin(GPIO_FLM_TXD, S3C_GPIO_SFN(GPIO_FLM_TXD_AF));
	s3c_gpio_setpull(GPIO_FLM_TXD, S3C_GPIO_PULL_NONE); 

	if (gpio_is_valid(GPIO_PHONE_ON)) {
		if (gpio_request(GPIO_PHONE_ON, S3C_GPIO_LAVEL(GPIO_PHONE_ON)))
			printk(KERN_ERR "Filed to request GPIO_PHONE_ON!\n");
		gpio_direction_output(GPIO_PHONE_ON, GPIO_LEVEL_LOW);
	}
	s3c_gpio_setpull(GPIO_PHONE_ON, S3C_GPIO_PULL_NONE); 

	if (gpio_is_valid(GPIO_CP_BOOT_SEL)) {
		if (gpio_request(GPIO_CP_BOOT_SEL, S3C_GPIO_LAVEL(GPIO_CP_BOOT_SEL)))
			printk(KERN_ERR "Filed to request GPIO_CP_BOOT_SEL!\n");
		gpio_direction_output(GPIO_CP_BOOT_SEL, GPIO_LEVEL_LOW);
	}
	s3c_gpio_setpull(GPIO_CP_BOOT_SEL, S3C_GPIO_PULL_NONE); 

	if (gpio_is_valid(GPIO_USIM_BOOT)) {
		if (gpio_request(GPIO_USIM_BOOT, S3C_GPIO_LAVEL(GPIO_USIM_BOOT)))
			printk(KERN_ERR "Filed to request GPIO_USIM_BOOT!\n");
		gpio_direction_output(GPIO_USIM_BOOT, GPIO_LEVEL_LOW);
	}
	s3c_gpio_setpull(GPIO_USIM_BOOT, S3C_GPIO_PULL_NONE); 

	if (gpio_is_valid(GPIO_PHONE_RST_N)) {
		if (gpio_request(GPIO_PHONE_RST_N, S3C_GPIO_LAVEL(GPIO_PHONE_RST_N)))
			printk(KERN_ERR "Filed to request GPIO_PHONE_RST_N!\n");
		gpio_direction_output(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);
	}
	s3c_gpio_setpull(GPIO_PHONE_RST_N, S3C_GPIO_PULL_NONE); 

	if (gpio_is_valid(GPIO_PDA_ACTIVE)) {
		if (gpio_request(GPIO_PDA_ACTIVE, S3C_GPIO_LAVEL(GPIO_PDA_ACTIVE)))
			printk(KERN_ERR "Filed to request GPIO_PDA_ACTIVE!\n");
		gpio_direction_output(GPIO_PDA_ACTIVE, GPIO_LEVEL_HIGH);
	}
	s3c_gpio_setpull(GPIO_PDA_ACTIVE, S3C_GPIO_PULL_NONE); 

}

static void kill_tasklets(void)
{
	tasklet_kill(&fmt_res_ack_tasklet);
	tasklet_kill(&raw_res_ack_tasklet);

	tasklet_kill(&fmt_send_tasklet);
	tasklet_kill(&raw_send_tasklet);
}

static int register_interrupt_handler(void)
{

	unsigned int dpram_irq, phone_active_irq, sim_detect_irq;
	int retval = 0;
	
	dpram_irq = IRQ_ONEDRAM_INT_N;
	phone_active_irq = IRQ_PHONE_ACTIVE;
	sim_detect_irq = IRQ_SIM_DETECT_N;

	/* @LDK@ interrupt area read - pin level will be driven high. */
	dpram_clear();

	/* @LDK@ dpram interrupt */
	retval = request_irq(dpram_irq, dpram_irq_handler, IRQF_DISABLED, "dpram irq", NULL);

	if (retval) {
		dprintk("DPRAM interrupt handler failed.\n");
		unregister_dpram_driver();
		return -1;
	}

	/* @LDK@ phone active interrupt */
	retval = request_irq(phone_active_irq, phone_active_irq_handler, IRQF_DISABLED, "Phone Active", NULL);

	if (retval) {
		dprintk("Phone active interrupt handler failed.\n");
		free_irq(phone_active_irq, NULL);
		unregister_dpram_driver();
		return -1;
	}

	retval = request_irq(sim_detect_irq, sim_detect_irq_handler, IRQF_DISABLED, "Sim Detect", NULL);

	if (retval) {
		dprintk("sim detect interrupt handler failed.\n");
		free_irq(sim_detect_irq, NULL);
		unregister_dpram_driver();
		return -1;
	}

	return 0;
}

static void check_miss_interrupt(void)
{
	unsigned long flags;

	if (gpio_get_value(GPIO_PHONE_ACTIVE) &&
			(!gpio_get_value(GPIO_ONEDRAM_INT_N))) {
		dprintk("there is a missed interrupt. try to read it!\n");

		if (!(*onedram_sem)) {
			printk("[OneDRAM] (%s) semaphore: %d\n", __func__, *onedram_sem);
			onedram_get_semaphore(__func__);
		}

		local_irq_save(flags);
		dpram_irq_handler(IRQ_ONEDRAM_INT_N, NULL);
		local_irq_restore(flags);
	}

	local_irq_save(flags);
	sim_detect_irq_handler(IRQ_SIM_DETECT_N, NULL);
	local_irq_restore(flags);

}

static int dpram_suspend(struct platform_device *dev, pm_message_t state)
{
	gpio_set_value(GPIO_PDA_ACTIVE, GPIO_LEVEL_LOW);
	return 0;
}

static int dpram_resume(struct platform_device *dev)
{
	gpio_set_value(GPIO_PDA_ACTIVE, GPIO_LEVEL_HIGH);
	check_miss_interrupt();
	return 0;
}

static int __devinit dpram_probe(struct platform_device *dev)
{
	int retval;

	/* @LDK@ register dpram (tty) driver */
	retval = register_dpram_driver();
	if (retval) {
		dprintk("Failed to register dpram (tty) driver.\n");
		return -1;
	}

#ifdef _ENABLE_ERROR_DEVICE
	/* @LDK@ register dpram error device */
	retval = register_dpram_err_device();
	if (retval) {
		dprintk("Failed to register dpram error device.\n");

		unregister_dpram_driver();
		return -1;
	}

	memset((void *)dpram_err_buf, '\0', sizeof dpram_err_buf);
#endif /* _ENABLE_ERROR_DEVICE */

	/* @LDK@ H/W setting */
	init_hw_setting();

	dpram_shared_bank_remap();

	sim_state = gpio_get_value(GPIO_SIM_DETECT_N);

	dprintk("sime_state = %d\n", sim_state);

	/* @LDK@ register interrupt handler */
	if ((retval = register_interrupt_handler()) < 0) {
		return -1;
	}

	/* @LDK@ initialize device table */
	init_devices();

#ifdef CONFIG_PROC_FS
	create_proc_read_entry(DRIVER_PROC_ENTRY, 0, 0, dpram_read_proc, NULL);
#endif	/* CONFIG_PROC_FS */

	/* @LDK@ check out missing interrupt from the phone */
//	check_miss_interrupt();

	return 0;
}

static int __devexit dpram_remove(struct platform_device *dev)
{
	/* @LDK@ unregister dpram (tty) driver */
	unregister_dpram_driver();

	/* @LDK@ unregister dpram error device */
#ifdef _ENABLE_ERROR_DEVICE
	unregister_dpram_err_device();
#endif
	/* @LDK@ unregister irq handler */
	
	free_irq(IRQ_ONEDRAM_INT_N, NULL);
	free_irq(IRQ_PHONE_ACTIVE, NULL);
	free_irq(IRQ_SIM_DETECT_N, NULL);

	kill_tasklets();

	return 0;
}

static struct platform_driver platform_dpram_driver = {
	.probe		= dpram_probe,
	.remove		= __devexit_p(dpram_remove),
	.suspend	= dpram_suspend,
	.resume 	= dpram_resume,
	.driver	= {
		.name	= "dpram-device",
	},
};

/* init & cleanup. */
static int __init dpram_init(void)
{
	dpram_dev = device_create(sec_class, NULL, 0, NULL, "dpram");
	if (IS_ERR(dpram_dev))
		dprintk("Failed to create device(dpram)!\n");
	if (device_create_file(dpram_dev, &dev_attr_sim) < 0)
		dprintk("Failed to create device file(%s)!\n", dev_attr_sim.attr.name);

	wake_lock_init(&dpram_wake_lock, WAKE_LOCK_SUSPEND, "DPRAM");


	return platform_driver_register(&platform_dpram_driver);
}

static void __exit dpram_exit(void)
{
	wake_lock_destroy(&dpram_wake_lock);
	platform_driver_unregister(&platform_dpram_driver);
}

module_init(dpram_init);
module_exit(dpram_exit);

MODULE_AUTHOR("SAMSUNG ELECTRONICS CO., LTD");
MODULE_DESCRIPTION("Onedram Device Driver.");
MODULE_LICENSE("GPL");
