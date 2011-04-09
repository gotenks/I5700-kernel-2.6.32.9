#!/bin/bash

[ -d ./initramfs   -a   -h ./initramfs ] && rm ./initramfs
[ "$1" = "prepare" -a ! -d ./initramfs ] && ln -s ././initramfs-A22 ./initramfs
