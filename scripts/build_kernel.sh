#!/bin/bash

#Set CPU Environment Variable
if [ "$CPU_JOB_NUM" = "" ] ; then
        CPU_JOB_NUM=8
fi

Usage()
{
echo "build_kernel.sh - building script android kernel"
echo "  Usage: ./build_kernel.sh <r880 | r880tom3q>"
echo

exit 1
}

OPTION=-k
#PRODUCT=r880
PRODUCT=$1

case "$PRODUCT" in
	r880*)
		MODULES="multipdp dpram vibetonz dpram_recovery"

		case "$PRODUCT" in
			r880)
				KERNEL_DEF_CONFIG=r880_android_defconfig
			;;
			r880tom3q)
				KERNEL_DEF_CONFIG=r880_tom3q_defconfig
			;;
		esac 
		;;
		
	*)
		Usage
		;;
esac 

if [ ! $PWD_DIR ] ; then
	PWD_DIR=$(pwd)
fi

KERNEL_DIR=$PWD_DIR/Kernel
MODULES_DIR=$PWD_DIR/modules
CTNG_BIN_DIR=/opt/ctng/bin


prepare_kernel()
{
	echo "*************************************"
	echo "*          prepare kernel           *"
	echo "*************************************"
	echo

	make -C $KERNEL_DIR ARCH=arm $KERNEL_DEF_CONFIG
	if [ $? != 0 ] ; then
	    exit 1
	fi
	make -C $KERNEL_DIR ARCH=arm prepare
	if [ $? != 0 ] ; then
	    exit 1
	fi
}

build_modules()
{
	echo "*************************************"
	echo "*           build modules           *"
	echo "*************************************"
	echo

	#make -C $KERNEL_DIR ARCH=arm
	if [ $? != 0 ] ; then
	    exit 1
	fi
	make -C $KERNEL_DIR ARCH=arm KBUILD_MODPOST_WARN=1 modules
	if [ $? != 0 ] ; then
	    exit 1
	fi
	#exit 1

	for module in $MODULES
	do
		echo cd $MODULES_DIR/$module
		cd $MODULES_DIR/$module
		make KDIR=$KERNEL_DIR
		if [ -e ./*.ko ]
		then
		    cp ./*.ko  $KERNEL_DIR/../initramfs/lib/modules
		fi
	done 

}


build_kernel()
{
	if [ ! -f $KERNEL_DIR/.config ] ; then
        echo "kernel config missing, using default"
		if [ ! -f $KERNEL_DIR/scripts/mod/modprobe ] ; then
			prepare_kernel
		fi
	fi

	#echo "make " -C $KERNEL_DIR ARCH=arm
	#make -C $KERNEL_DIR ARCH=arm
	if [ $? != 0 ] ; then
	    exit 1
	fi

	build_modules

	echo "*************************************"
	echo "*           build kernel            *"
	echo "*************************************"
	echo
	
	cd $KERNEL_DIR

	#make -j$CPU_JOB_NUM CONFIG_DEBUG_SECTION_MISMATCH=y
	make -j$CPU_JOB_NUM
	if [ $? != 0 ] ; then
		exit $?
	fi

#
# 	add some modification for optimize
#
#	cp $KERNEL_DIR/drivers/net/wireless/bcm4325/dhd.ko   $KERNEL_DIR/../initramfs/lib/modules
#	cp $KERNEL_DIR/net/netfilter/xt_TCPMSS.ko            $KERNEL_DIR/../initramfs/lib/modules
#	cp $KERNEL_DIR/drivers/net/tun.ko                    $KERNEL_DIR/../initramfs/lib/modules

	$CTNG_BIN_DIR/arm-spica-linux-uclibcgnueabi-strip -g $KERNEL_DIR/../initramfs/lib/modules/dpram.ko
	$CTNG_BIN_DIR/arm-spica-linux-uclibcgnueabi-strip -g $KERNEL_DIR/../initramfs/lib/modules/dhd.ko
	$CTNG_BIN_DIR/arm-spica-linux-uclibcgnueabi-strip -g $KERNEL_DIR/../initramfs/lib/modules/xt_TCPMSS.ko
	$CTNG_BIN_DIR/arm-spica-linux-uclibcgnueabi-strip -g $KERNEL_DIR/../initramfs/lib/modules/tun.ko
#	add old vibrator
	$CTNG_BIN_DIR/arm-spica-linux-uclibcgnueabi-strip -g $KERNEL_DIR/../initramfs/lib/modules/vibrator.ko
 
	make
}



case "$OPTION" in
	-k)
		build_kernel
		;;
	*)
		Usage
		;;
esac 

exit 0
