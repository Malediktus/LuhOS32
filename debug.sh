#!/bin/bash

set -e

source img.sh

gdb --command=debug.gdb
