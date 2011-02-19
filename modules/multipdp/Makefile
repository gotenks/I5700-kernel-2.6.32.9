##############################################################################
# COPYRIGHT(C)	: Samsung Electronics Co.Ltd, 2006-2011 ALL RIGHTS RESERVED
# AUTHOR		: KyoungHOON Kim (khoonk)                                @LDK@
#########################################################################@LDK@
# VERSION&DATE	: Version 1.00	2006/06/02 (khoonk)                      @LDK@
##############################################################################

MOD_NAME = multipdp

ifneq ($(KERNELRELEASE), )

EXTRA_CFLAGS += -I$(PRJROOT)/modules/include

obj-m := $(MOD_NAME).o

else

all:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.o *.ko *.mod.c *.mod.o *~ .*.cmd .*.o.cmd Module.symvers modules.order $(obj-m)

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install

endif
