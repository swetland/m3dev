M_NAME := lpc13boot
M_ARCH := lpc13xx
M_LINK := build/lpc1343-boot.ld
M_OBJS := arch/arm-cm3/start.o
M_OBJS += lpc13boot/main.o
M_OBJS += lpc13boot/board.o
M_OBJS += lpc13boot/misc.o
M_OBJS += arch/lpc13xx/gpio.o
M_OBJS += arch/lpc13xx/serial.o
M_OBJS += arch/lpc13xx/usb.o
#M_OBJS += libfw/print.o
include build/target-executable.mk

