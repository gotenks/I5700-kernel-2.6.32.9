#
#   Makefile S3C Camera driver
#
#   Copyright(c) 2004-2006, Samsung Electronics, Co., Ltd.
#

ifeq ($(CONFIG_MACH_INFOBOWLQ),y)
EXTRA_CFLAGS 	+= -DCONFIG_VIDEO_SAMSUNG_S5K4CA
endif

SENSOR_DIR		= samsung

#
# S3C Camera Interface
#

obj-y			+= s3c_camera.o

s3c_camera-y	+= s3c_camif.o s3c_camera_driver.o

#
# 3M Camera Sensor
#

obj-$(CONFIG_VIDEO_SAMSUNG)	+= samsung/
