#!/bin/bash

set -e

mkdir -p sysroot
mkdir -p logs

pushd kernel;
  make DESTDIR=../sysroot all;
  make DESTDIR=../sysroot install;
  # make DESTDIR=../sysroot install-headers;
popd
