
M_NAME := debugger
M_OBJS := tools/debugger.o
M_OBJS += tools/debugger-commands.o
M_OBJS += tools/rswdp.o
M_OBJS += tools/linenoise.o
M_OBJS += tools/usb-linux.o
include build/host-executable.mk

M_NAME := gdb-bridge
M_OBJS := tools/gdb-bridge.o
M_OBJS += tools/debugger-commands.o
M_OBJS += tools/rswdp.o
M_OBJS += tools/linenoise.o
M_OBJS += tools/usb-linux.o
include build/host-executable.mk

M_NAME := stm32boot
M_OBJS := tools/stm32boot.o
include build/host-executable.mk

M_NAME := usbmon
M_OBJS := tools/usbmon.o
include build/host-executable.mk

M_NAME := usbtest
M_OBJS := tools/usbtest.o
M_OBJS += tools/usb-linux.o
include build/host-executable.mk

M_NAME := bless-lpc
M_OBJS := tools/bless-lpc.o
include build/host-executable.mk

M_NAME := lpcboot
M_OBJS := tools/lpcboot.o
M_OBJS += tools/usb-linux.o
include build/host-executable.mk
