#
# Makefile for the vibrator driver ICs with vibrator driver
#

obj-m += vibrator.o

vibrator-objs += vibetonz.o 

all:
	@echo "$(MAKE) --no-print-directory -C $(KDIR) SUBDIRS=$(CURDIR) modules"
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules

clean:
	rm -f *.o *.ko *.mod.c *.mod.o *~ .*.cmd .*.o.cmd Module.symvers modules.order 
	[ -d .tmp_versions ] && ( rm -rf .tmp_versions/*; rmdir .tmp_versions; )

install:
	@$(MAKE) --no-print-directory -C $(KDIR) \
		SUBDIRS=$(CURDIR) modules_install

