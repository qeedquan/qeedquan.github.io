#!/bin/sh

export QEMU_AUDIO_DRV="none"
#export DBG="-s -S"
$HOME/apps/qemu/bin/qemu-system-arm -M versatilepb -m 256M -kernel plan9 -serial stdio $DBG

