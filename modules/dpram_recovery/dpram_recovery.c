
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <plat/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/hardware.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include "dpram_recovery.h"

#define _DEBUG_

#ifdef _DEBUG_
#define MSGCRIT "\x1b[1;31m"
#define MSGERR "\x1b[1;33m"
#define MSGWARN "\x1b[1;35m"
#define MSGINFO "\x1b[1;32m"
#define MSGDBG "\x1b[1;37m"

#define MSGEND "\x1b[0m \n"
#else
#define MSGCRIT 
#define MSGERR 
#define MSGWARN 
#define MSGINFO 
#define MSGDBG 
#define MSGEND 
#endif

#define DRIVER_NAME "dpram_recovery"
#define MAJOR_NUM 253

#define DPRAM_ST_ADDR_PHYS 0x5D000000
#define DPRAM_SIZE 0x1000000
#define DPRAM_MEMORY_SIZE 0xFFF800
#define DPRAM_SEM_ADDR_OFFSET 0xFFF800
#define DPRAM_MAILBOX_AB_ADDR_OFFSET (DPRAM_SEM_ADDR_OFFSET+0x20)
#define DPRAM_MAILBOX_BA_ADDR_OFFSET (DPRAM_SEM_ADDR_OFFSET+0x40)

struct dpram_dev {
    int memsize;
    int dpram_vbase;
    int dpram_sem_vaddr;
    int dpram_mailboxAB_vaddr;
    int dpram_mailboxBA_vaddr;
    struct semaphore sem;
    struct cdev cdev;
	int refcnt;
};

static struct dpram_dev *dpram;
struct class *dpram_class;


#define IRQ_ONEDRAM_INT_N	IRQ_EINT(0)
#define IRQ_PHONE_ACTIVE	IRQ_EINT(7)

static void dpram_memory_barrier(void)
{
	dmb();
	dsb();
}

static volatile u32 
read_semaphore(struct dpram_dev *dev)
{
	dpram_memory_barrier();
	
    return *(u32*)dev->dpram_sem_vaddr;	
}

static void 
return_semaphore(struct dpram_dev *dev)
{
	if(*(u32*)dev->dpram_sem_vaddr == 1)
		*(u32*)dev->dpram_sem_vaddr = 0;

	dpram_memory_barrier();
}

static volatile u32 
get_mailboxAB(struct dpram_dev *dev)
{
	dpram_memory_barrier();
	
    return *(u32*)dev->dpram_mailboxAB_vaddr;
}

static void 
set_mailboxBA(struct dpram_dev *dev, u32 data)
{
    *(u32*)dev->dpram_mailboxBA_vaddr = data;

	dpram_memory_barrier();
}

// Below __(functions) write and reads from dpram. The paramter [addr] is
// the relative address from the dpram_vbase address.
// So addr=0 means the base of the dpram.
static int __inline __writel(struct dpram_dev *dev, int addr, int data)
{
    if(*(int*)dev->dpram_sem_vaddr != 1) {
        printk(KERN_ERR "Semaphore is in modem!!\n");
        return -1;
    }

    *(int*)(dev->dpram_vbase+addr) = data;

	dpram_memory_barrier();

	return 0;
}

static int __inline __write(struct dpram_dev *dev, int addr, char *data, 
    size_t size)
{
    if(*(int*)dev->dpram_sem_vaddr != 1) {
        printk(KERN_ERR "Semaphore is in modem!!\n");
        return -1;
    }
    
    memcpy((void*)(dev->dpram_vbase+addr),data,size);

	dpram_memory_barrier();

	return 0;
}
static int __write_from_user(struct dpram_dev *dev, int addr, 
    char __user *data, size_t size)
{
    if(*(int*)dev->dpram_sem_vaddr != 1) {
        printk(KERN_ERR "Semaphore is in modem!!\n");
        return -1;
    }

    if(copy_from_user((void*)(dev->dpram_vbase+addr),data,size)<0) {
		printk(KERN_ERR "[%s:%d] Copy from user failed\n", __func__, __LINE__);
    }

	dpram_memory_barrier();

	return 0;
}

static int __inline __readl(struct dpram_dev *dev, int addr)
{
	dpram_memory_barrier();
	
    if(*(int*)dev->dpram_sem_vaddr != 1) {
        printk(KERN_ERR "Semaphore is in modem!!\n");
        return -1;
    }

    return *(int*)(dev->dpram_vbase+addr);
}

static void __inline __read(struct dpram_dev *dev, int addr, char *data, 
    size_t size)
{
    if(data == NULL) return;

	dpram_memory_barrier();
	
    if(*(int*)dev->dpram_sem_vaddr != 1) {
        printk(KERN_ERR "Semaphore is in modem!!\n");
        return;
    }
    
    memcpy(data, (void*)(dev->dpram_vbase+addr), size);
}
static void __read_to_user(struct dpram_dev *dev, int addr, 
    char __user *data, size_t size)
{
    if(data == NULL) return;

	dpram_memory_barrier();
	
    if(*(int*)dev->dpram_sem_vaddr != 1) {
        printk(KERN_ERR "Semaphore is in modem!!\n");
        return;
    }

    if(copy_to_user(data, (void*)(dev->dpram_vbase+addr), size)<0) {
		printk(KERN_ERR "[%s] Copy to user failed\n", __func__);
    }
}

static int 
dpram_recovery_ioremap(struct dpram_dev *dev)
{
    int i;
    
	dev->dpram_vbase = (int)ioremap_nocache(DPRAM_ST_ADDR_PHYS, DPRAM_SIZE);
	if (dev->dpram_vbase == NULL) {
		printk("failed ioremap\n");
		return -ENOENT;
	}

    printk(KERN_DEBUG "dpram vbase=0x%8x\n", dev->dpram_vbase);
    
    dev->dpram_sem_vaddr = dev->dpram_vbase + DPRAM_SEM_ADDR_OFFSET;
    
	dev->dpram_mailboxAB_vaddr = dev->dpram_vbase + DPRAM_MAILBOX_AB_ADDR_OFFSET;
	dev->dpram_mailboxBA_vaddr = dev->dpram_vbase + DPRAM_MAILBOX_BA_ADDR_OFFSET;

    dev->memsize = DPRAM_MEMORY_SIZE;
	
    // clear dpram 
    for(i=0;i<DPRAM_SIZE;i=i+4) {
        if(__writel(dev, i, 0xffffffff)<0)
			break;
    }
	return 0;
}

static int
modem_pwr_status(struct dpram_dev *dev)
{
	return gpio_get_value(GPIO_PHONE_ACTIVE);
}

static int 
modem_pwroff(struct dpram_dev *dev)
{
	gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);
	mdelay(100);
	
	printk(MSGINFO "Modem power off sem(%d)" MSGEND, *((int*)dev->dpram_sem_vaddr));
	
	return 0;
}

static int 
modem_pwron(struct dpram_dev *dev)
{
	int err = -1;
	int msec;
	
	printk(MSGINFO "Modem power on. sem(%d)" MSGEND, *((int*)dev->dpram_sem_vaddr));

    // toss semaphore to modem
	*((int*)dev->dpram_sem_vaddr)= 0x00;
    
	printk(MSGDBG "semaphore tossed to modem(%d)" MSGEND, *((int*)dev->dpram_sem_vaddr));

	gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_HIGH);
	mdelay(50);
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_LOW);
	mdelay(100);
	gpio_set_value(GPIO_PHONE_RST_N, GPIO_LEVEL_HIGH);
	msleep(500);
	//gpio_set_value(GPIO_PHONE_ON, GPIO_LEVEL_LOW);

	for(msec=0;msec<100000;msec++) {
	    if(modem_pwr_status(dev)) {
	        err = 0;
			printk(MSGINFO "Phone is active %d" MSGEND, modem_pwr_status(dev));
			break;
	    }
		msleep(1);
	}
	
    return err;
}


int
dpram_recovery_write_modem_firmware(
    struct dpram_dev *dev, char __user *firmware, int size)
{
    int ret = 0;

    // write firmware size
    __writel(dev, DPRAM_FIRMWARE_SIZE_ADDR, size);
    
    // write firmware
    __write_from_user(dev, DPRAM_FIRMWARE_ADDR, firmware, size);
    
	return ret;
}

int
dpram_recovery_start_firmware_update(struct dpram_dev *dev)
{
	int err = 0;
	int msec = 0;
	u32 val;

    // write boot magic
    __writel(dev, DPRAM_BOOT_MAGIC_ADDR, DPRAM_BOOT_MAGIC_RECOVERY_FOTA);
    __writel(dev, DPRAM_BOOT_TYPE_ADDR, DPRAM_BOOT_TYPE_DPRAM_DELTA);

	modem_pwroff(dev);
	
    // write boot magic
    __writel(dev, DPRAM_BOOT_MAGIC_ADDR, DPRAM_BOOT_MAGIC_RECOVERY_FOTA);
    __writel(dev, DPRAM_BOOT_TYPE_ADDR, DPRAM_BOOT_TYPE_DPRAM_DELTA);

    // boot modem
    err = modem_pwron(dev);
    if(err < 0) {
        printk(KERN_ERR MSGCRIT "Modem power on failed. phone active is %d" MSGEND, 
            modem_pwr_status(dev));
		goto out;
    }

	// clear mailboxBA
	set_mailboxBA(dev, 0xFFFFFFFF);
	
	// wait for job sync message
	while(true) {
		val = get_mailboxAB(dev);
		if((val&STATUS_JOB_MAGIC_M) == STATUS_JOB_MAGIC_CODE) {
			err = 0;
			break;
		}
		
		msleep(1);
		if(++msec > 100000) {
			err = -2;
			printk(MSGCRIT "Failed to sync with modem (%x)" MSGEND, val);
			goto out;
		}

		if((msec%1000) == 0) {
			printk(MSGINFO "Waiting for sync message... 0x%08x (pwr:%s)" MSGEND,
				val, modem_pwr_status(dev)?"ON":"OFF");
		}
	}

	if(err == 0) {
		printk(MSGINFO "Modem ready to start the firmware update" MSGEND);
	
		// let modem start the job
		set_mailboxBA(dev, STATUS_JOB_MAGIC_CODE);
		// if we have the semaphore, return it.
		return_semaphore(dev);
	}
	
out:
    return err;

}

static int
dpram_recovery_check_status(struct dpram_dev *dev, 
	int __user *pct, char __user *msg)
{
	u32 status;
	static u32 prev_status = 0;
	int percent = 0;
	int err = 0;
	char buf[DPRAM_MODEM_MSG_SIZE];
	int debugprint = false;
	int wait = 0;
	
    // check mailboxAB for the modem status
    status = get_mailboxAB(dev);

	debugprint = (prev_status != status);
	
	if(debugprint) printk("Job status : 0x%08x (pwr:%s)\n", status,
		modem_pwr_status(dev)?"ON":"OFF");

	if((status & STATUS_JOB_MAGIC_M) != STATUS_JOB_MAGIC_CODE) {
		if(debugprint) printk("Job not accepted yet\n");
		err = 1;
		percent = 0;
		strncpy(buf, "Job not accepted yet", DPRAM_MODEM_MSG_SIZE);
		goto out;
	}

	if((status & STATUS_JOB_STARTED_M) == STATUS_JOB_STARTED_M) {
		return_semaphore(dev);
		percent = status & STATUS_JOB_PROGRESS_M;
		if(debugprint) printk("Job progress pct=%d\n", percent);
		err = 3;
	}
	else {
		percent = 0;
		if(debugprint) printk("Job NOT started yet...\n");
		err = 2;
	}

	if(status & STATUS_JOB_ENDED_M) {
		percent = status & STATUS_JOB_PROGRESS_M;
		
		// wait till we have semaphore
		printk(MSGWARN "Wait for semaphore" MSGEND);
		while(true) {
			msleep(10);
			if(read_semaphore(dev) == 1) {
				printk(MSGINFO "We have semaphore" MSGEND);
				break;
			}

			if(wait++ > 1000) {
				printk(MSGWARN "Proceeding without semaphore" MSGEND);
				break;
			}
			
		}
		
		__read(dev, DPRAM_MODEM_STRING_MSG_ADDR, buf, DPRAM_MODEM_MSG_SIZE);

		if(status & STATUS_JOB_ERROR_M) {
			err = -1;
			printk(KERN_ERR "Job ended with error msg : %s\n", buf);
		}
		else if((status & STATUS_JOB_COMPLETE_M) == STATUS_JOB_COMPLETE_M) {
			err = 0;
			printk(KERN_DEBUG "Job completed successfully : %s\n", buf);
		}
	}
out:
	prev_status = status;
	if(copy_to_user((void*)pct, (void*)&percent, sizeof(int))<0) {
		printk(KERN_ERR "[%s:%d] Copy to user failed\n", __func__, __LINE__);
	}
	
	if(copy_to_user((void*)msg, (void*)&buf[0], DPRAM_MODEM_MSG_SIZE)<0) {
		printk(KERN_ERR "[%s:%d] Copy to user failed\n", __func__, __LINE__);
	}
	
	return err;
}

static ssize_t 
dpram_recovery_read(struct file *filp, char __user *buff, 
    size_t count, loff_t *fpos)
{
    struct dpram_dev *dev = filp->private_data;

    if(*fpos > dev->memsize) {
        printk(KERN_ERR "Request offset overflow!\n");
        return -EOVERFLOW;
    }
	
	// users can only read/write from DPRAM_FIRMWARE_ADDR address onwards..
    __read_to_user(dev, (int)*fpos+DPRAM_FIRMWARE_ADDR, buff, count);

	*fpos = *fpos + count;

	return count;
}

static ssize_t 
dpram_recovery_write(struct file *filp, const char __user *buff,
    size_t size, loff_t *fpos)
{
    struct dpram_dev *dev = filp->private_data;

	
    if(*fpos > dev->memsize) {
        printk(KERN_ERR "Request offset overflow!\n");
        return -EOVERFLOW;
    }
	
	// users can only read/write from DPRAM_FIRMWARE_ADDR address onwards..
    __write_from_user(dev, (int)*fpos+DPRAM_FIRMWARE_ADDR, (char*)buff, size);
	
	*fpos = *fpos + size;

	return size;
}

static int
dpram_recovery_ioctl(struct inode *inode, struct file *filp,
	   unsigned int cmd, unsigned long arg)
    
{
    struct dpram_dev *dev;
	int ret = 0;
    
    dev = container_of(inode->i_cdev, struct dpram_dev, cdev);
    
	switch (cmd) {
		
        case IOCTL_ST_FW_UPDATE:
            printk(KERN_ERR "%s %d\n", __func__, __LINE__);
			
			if(arg == NULL) {
				printk(MSGWARN "Firmware should be written prior to this call" MSGEND);
			}
			else {
				struct dpram_firmware fw;
				
    			if(copy_from_user((void *)&fw, (void *)arg, sizeof(fw)) < 0) {
                    printk("[IOCTL_ST_FW_UPDATE]copy from user failed!");
                    ret = -1;
    			}
                
    			if (dpram_recovery_write_modem_firmware(dev, fw.firmware, fw.size) < 0) {
    				printk("firmware write failed\n");
    				ret = -2;
    			}
			}
			
			if(dpram_recovery_start_firmware_update(dev) < 0) {
				printk("Firmware update failed\n");
				ret = -3;
			}
			break;

    	case IOCTL_CHK_STAT:
			{
				struct stat_info *pst;
				pst = (struct stat_info*)arg;
	    		ret = dpram_recovery_check_status(dev, &(pst->pct), pst->msg);
    		}
    		break;
			
        case IOCTL_MOD_PWROFF:
			modem_pwroff(dev);
			break;
			
        default:
            printk(KERN_ERR "Unknown command");
			break;
	}

	return ret;
}

static int 
dpram_recovery_open(struct inode *inode, struct file *filp)
{
    struct dpram_dev *dev;
	static int ref = 0;

	
    printk(KERN_DEBUG "dpram recovery device opened.\n");
    
    dev = container_of(inode->i_cdev, struct dpram_dev, cdev);
    filp->private_data = (void*)dev;
	
	if(dev->refcnt > 1) {
		printk(KERN_ERR "dpram is already in use.\n");
		return -EMFILE;
	}

	dev->refcnt++;
	
    return 0;
}

static int 
dpram_recovery_release(struct inode *inode, struct file *filp)
{
    struct dpram_dev *dev;
	
    dev = container_of(inode->i_cdev, struct dpram_dev, cdev);
	
	if(dev->refcnt > 0) 
		dev->refcnt--;
	
    printk(KERN_DEBUG "dpram recovery device released.\n");
	
    return 0;
}

static struct file_operations dpram_recovery_fops = {
    .owner = THIS_MODULE,
    .read = dpram_recovery_read,
    .write = dpram_recovery_write,
    .ioctl = dpram_recovery_ioctl,
    .open = dpram_recovery_open,
    .release = dpram_recovery_release,
};

static irqreturn_t
dpram_irq_handler(int irq, void *dev_id)
{
	u32 mbAB;
	mbAB = get_mailboxAB(dpram);
	printk(MSGDBG "Dpram irq received : 0x%8x" MSGEND, mbAB);

	return IRQ_HANDLED;
}

static irqreturn_t
phone_active_irq_handler(int irq, void *dev_id)
{
	printk(MSGDBG "Phone active irq : %d" MSGEND, modem_pwr_status(dpram));
	return IRQ_HANDLED;
}

static int 
register_interrupt_handler(void)
{
	int err = 0;

	err = request_irq(IRQ_ONEDRAM_INT_N, dpram_irq_handler, 
						IRQF_DISABLED, "Dpram irq", NULL);

	if (err) {
		printk("DPRAM interrupt handler failed.\n");
		return -1;
	}

	err = request_irq(IRQ_PHONE_ACTIVE, phone_active_irq_handler, 
						IRQF_DISABLED, "Phone Active", NULL);

	if (err) {
		printk("Phone active interrupt handler failed.\n");
		free_irq(IRQ_ONEDRAM_INT_N, NULL);
		return -1;
	}

	printk(MSGINFO "Interrupt handler registered." MSGEND);
	
	return err;
}

static int
hw_init(void)
{
	s3c_gpio_cfgpin(GPIO_PHONE_ACTIVE, S3C_GPIO_SFN(GPIO_PHONE_ACTIVE_AF));
	s3c_gpio_setpull(GPIO_PHONE_ACTIVE, S3C_GPIO_PULL_NONE); 
	set_irq_type(IRQ_PHONE_ACTIVE, IRQ_TYPE_EDGE_BOTH);

	s3c_gpio_cfgpin(GPIO_ONEDRAM_INT_N, S3C_GPIO_SFN(GPIO_ONEDRAM_INT_N_AF));
	s3c_gpio_setpull(GPIO_ONEDRAM_INT_N, S3C_GPIO_PULL_NONE); 
	set_irq_type(IRQ_ONEDRAM_INT_N, IRQ_TYPE_EDGE_FALLING);

	return 0;
}


static int __init 
dpram_recovery_init(void)
{
    int err = 0;
    int devno = MKDEV(MAJOR_NUM, 0);
	struct device *dpram_dev;

    printk(KERN_ERR "dpram recovery module init!!\n");
    
    dpram = kmalloc(sizeof(struct dpram_dev), GFP_KERNEL);
        
    if(dpram == NULL) {
        printk(KERN_ERR "dpram recovery device allocation failed!!\n");
        err = -ENOMEM;
        goto out;
    }

	dpram->refcnt = 0;
	
    if(dpram_recovery_ioremap(dpram)<0) {
        printk(KERN_ERR "Dpram recovery ioremap failed!\n");
        goto out_err1;
    }

	hw_init();
	
	register_interrupt_handler();
	
    if(register_chrdev_region(devno, 1, DRIVER_NAME) < 0) {
        printk(KERN_DEBUG "chrdev region register failed\n");
        err = -1;
        goto out_err1;
    }
        
    cdev_init(&dpram->cdev, &dpram_recovery_fops);
    dpram->cdev.owner = THIS_MODULE;
    dpram->cdev.ops = &dpram_recovery_fops;
        
    err = cdev_add(&dpram->cdev, devno, 1);
        
    if(err < 0) {
        printk(KERN_ERR "Dpram recovery device failed to register!\n");
        goto out_err1;
    }

	dpram_class = class_create(THIS_MODULE, "dpram");

	if (IS_ERR(dpram_class))
	{
		err = -EFAULT;
		goto out_err2;
	}
	
	dpram_dev = device_create(dpram_class, NULL, MKDEV(MAJOR_NUM, 0), 
		NULL, DRIVER_NAME);

	if (IS_ERR(dpram_dev))
	{
		err = -EFAULT;
		goto out_err2;
	}

    
    printk(KERN_ERR "%s %d\n", __func__, __LINE__);
    
out:
	return err;

out_err2:
    cdev_del(&dpram->cdev);
    
out_err1:
    kfree(dpram);
    return err;
    
}

static void __exit 
dpram_recovery_exit(void)
{
    cdev_del(&dpram->cdev);
	class_destroy(dpram_class);
    kfree(dpram);
	printk (KERN_DEBUG "Dpram recovery char device is unregistered\n");
}

module_init(dpram_recovery_init);
module_exit(dpram_recovery_exit);

MODULE_AUTHOR("Samsung Electronics Co., LTD");
MODULE_DESCRIPTION("Onedram Device Driver for recovery.");
MODULE_LICENSE("GPL");

