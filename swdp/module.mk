M_NAME := swdp
M_ARCH := stm32f1xx
M_LINK := build/stm32f103-rom.ld
M_OBJS := arch/arm-cm3/start.o
M_OBJS += swdp/main.o
M_OBJS += swdp/swdp.o
M_OBJS += arch/stm32f1xx/gpio.o
M_OBJS += arch/stm32f1xx/serial.o
M_OBJS += arch/stm32f1xx/usb.o
M_OBJS += libfw/print.o
M_OBJS += libfw/serialconsole.o
include build/target-executable.mk
