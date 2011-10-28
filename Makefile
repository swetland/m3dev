
## Copyright 2011 Brian Swetland <swetland@frotz.net>
## 
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

what_to_build:: all

-include local.mk

TOOLCHAIN ?= arm-none-eabi-

TARGET_CC := $(TOOLCHAIN)gcc
TARGET_LD := $(TOOLCHAIN)ld
TARGET_OBJCOPY := $(TOOLCHAIN)objcopy
TARGET_OBJDUMP := $(TOOLCHAIN)objdump

TARGET_CFLAGS := -g -Os -Wall
TARGET_CFLAGS += -mcpu=cortex-m3 -mthumb -mthumb-interwork
TARGET_CFLAGS += -ffunction-sections -fdata-sections 
TARGET_CFLAGS += -fno-builtin -nostdlib
#TARGET_CFLAGS += -ffreestanding
TARGET_CFLAGS += -Wl,--gc-sections
TARGET_CFLAGS += -I. -Iinclude

# config?
TARGET_CFLAGS += -Iarch/stm32f1xx/include
TARGET_CFLAGS += -Iarch/arm-cm3/include
#TARGET_LIBGCC := $(shell $(TARGET_CC) $(TARGET_CFLAGS) -print-libgcc-file-name)

HOST_CFLAGS := -g -O1 -Wall
HOST_CFLAGS += -Itools -Iinclude

#OUT := out/$(BOARD)
OUT := out
OUT_HOST_OBJ := $(OUT)/host-obj
OUT_TARGET_OBJ := $(OUT)/target-obj

ALL :=

include build/rules.mk

M_NAME := debugger
M_OBJS := tools/debugger.o
M_OBJS += tools/rswdp.o
M_OBJS += tools/linenoise.o
M_OBJS += tools/usb-linux.o
include build/host-executable.mk

M_NAME := gdb-bridge
M_OBJS := tools/gdb-bridge.o
M_OBJS += tools/rswdp.o
M_OBJS += tools/linenoise.o
M_OBJS += tools/usb-linux.o
include build/host-executable.mk

M_NAME := stm32boot
M_OBJS := tools/stm32boot.o
include build/host-executable.mk

M_NAME := swdp
M_BASE := 0x20001000
M_DATA := 0x20002000
M_OBJS := arch/stm32f1xx/start.o
M_OBJS += swdp/main.o
M_OBJS += swdp/swdp.o
M_OBJS += arch/stm32f1xx/gpio.o
M_OBJS += arch/stm32f1xx/serial.o
M_OBJS += arch/stm32f1xx/usb.o
M_OBJS += libfw/print.o

#M_LIBS := $(TARGET_LIBGCC)
include build/target-executable.mk

clean::
	@echo clean
	@rm -rf $(OUT)

all:: $(ALL)

# we generate .d as a side-effect of compiling. override generic rule:
%.d:
-include $(DEPS)
