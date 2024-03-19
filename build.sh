#!/bin/bash

set -e

mkdir -p sysroot
mkdir -p logs

pushd kernel;
  make DESTDIR=../sysroot all;
  make DESTDIR=../sysroot install;
  # make DESTDIR=../sysroot install-headers;
popd

pushd tools/initrd_gen;
  make initrd_gen;
  ./initrd_gen test.txt test.txt;
popd

pushd apps/blank
  make
popd

mkdir -p sysroot/bin/

cp -f tools/initrd_gen/initrd.img sysroot/boot/luhos.initrd
cp -f apps/blank/blank.bin sysroot/bin/blank.bin
