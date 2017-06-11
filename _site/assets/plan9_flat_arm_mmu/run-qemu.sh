#!/bin/sh

# pass in -s -S to use gdb
qemu-system-arm -M versatilepb -m 128M -nographic -kernel plan9 $@
