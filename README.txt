HOW TO BUILD KERNEL 2.6.32 FOR SPH-M910

1. Execute 'build_kernel_Froyo.sh' or  'build_kernel_Ginger.sh'.


2. If the kernel is built successfully, you will find following files from the top directory:
	
    arch/arm/boot/zImage

3. Copy to /cache and flash

    adb push arch/arm/boot/zImage /cache
    adb shell /xbin/bml_flash boot /cache/zImage
