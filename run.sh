#!/bin/bash

set -e

source img.sh

# --no-shutdown
qemu-system-x86_64 -drive id=disk,file=luhos.iso,format=raw,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -m 256M -cpu qemu64 -net none -no-reboot --no-shutdown -debugcon file:logs/kernel.log
