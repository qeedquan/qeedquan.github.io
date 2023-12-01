#!/bin/sh

export QEMU_AUDIO_DRV="none"
qemu-system-arm -M versatilepb -m 128M -kernel plan9 -serial stdio

