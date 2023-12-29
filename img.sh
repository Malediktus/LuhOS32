#!/bin/bash

set -e

source build.sh

rm -rf luhos.iso | true
rm -rf luhos.img | true

mkdir -p sysroot/boot/grub
cat > sysroot/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "LuhOS 32Bit" {
    multiboot2 /boot/kernel.elf
    boot
}
EOF

#grub-mkrescue -o luhos.iso sysroot

dd if=/dev/zero of=luhos.img bs=512 count=131072

fdisk luhos.img << EOF
o
n
p
1


a
w
EOF

sudo losetup /dev/loop0 luhos.img
sudo losetup /dev/loop1 luhos.img -o 1048576

sudo mkdosfs -F32 -f 2 /dev/loop1

sudo mount /dev/loop1 /mnt
sudo cp -rf sysroot/* /mnt

sudo grub-install --root-directory=/mnt --target=i386-pc --no-floppy --modules="normal part_msdos multiboot2" /dev/loop0

sudo umount /mnt
sudo losetup -d /dev/loop0
sudo losetup -d /dev/loop1
