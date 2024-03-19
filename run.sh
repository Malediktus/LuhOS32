#!/bin/bash

set -e

source img.sh

qemu-system-i386 --no-shutdown -device piix3-ide,id=ide -drive id=disk,file=luhos.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -m 256M -cpu qemu32 -net user -net nic,model=pcnet -no-reboot -debugcon file:logs/kernel.log
