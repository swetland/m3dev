M_NAME := m3debug 
M_ARCH := lpc13xx
#M_LINK := build/lpc1343-ram.ld
M_LINK := build/lpc1343-app.ld
M_OBJS := arch/arm-cm3/start.o
M_OBJS += m3debug/main.o
M_OBJS += m3debug/swdp.o
M_OBJS += m3debug/board.o
M_OBJS += libfw/print.o
M_OBJS += arch/lpc13xx/gpio.o
M_OBJS += arch/lpc13xx/serial.o
M_OBJS += arch/lpc13xx/usb.o
M_OBJS += arch/lpc13xx/reboot.o
include build/target-executable.mk

