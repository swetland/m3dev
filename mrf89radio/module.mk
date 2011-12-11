
R_OBJS := arch/arm-cm3/start.o
R_OBJS += mrf89radio/main.o
R_OBJS += mrf89radio/board.o
R_OBJS += arch/lpc13xx/gpio.o
R_OBJS += arch/lpc13xx/serial.o
R_OBJS += libfw/print.o

M_NAME := mrf89rx
M_CFLAGS := -DCONFIG_RX=1
M_ARCH := lpc13xx
M_LINK := build/lpc1343-app.ld
M_OBJS += $(R_OBJS)
include build/target-executable.mk

M_NAME := mrf89tx
M_CFLAGS := -DCONFIG_TX=1
M_ARCH := lpc13xx
M_LINK := build/lpc1343-app.ld
M_OBJS += $(R_OBJS)
include build/target-executable.mk

