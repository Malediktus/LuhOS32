#!/bin/bash

set -e

source build.sh

rm -rf luhos.iso | true

mkdir -p sysroot/boot/grub
cat > sysroot/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "LuhOS 32Bit" {
    multiboot2 /boot/kernel.elf
    boot
}
EOF

grub-mkrescue -o luhos.iso sysroot
