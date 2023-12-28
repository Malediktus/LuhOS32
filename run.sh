#!/bin/bash

set -e

source img.sh

# --no-shutdown
qemu-system-i386 -device piix3-ide,id=ide -drive id=disk,file=luhos.iso,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -m 256M -cpu qemu32 -net none -no-reboot -debugcon file:logs/kernel.log
