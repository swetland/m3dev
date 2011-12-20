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

M_NAME := $(strip $(M_NAME))
M_ARCH := $(strip $(M_ARCH))
M_ARCH_CFLAGS := $(ARCH_$(M_ARCH)_CFLAGS)

# sanity check
ifeq "$(M_NAME)" ""
$(error No module name specified)
endif
ifeq "$(M_ARCH_CFLAGS)" ""
$(error Module $(M_NAME): Unknown Architecture: $(M_ARCH))
endif

M_OBJS := $(addprefix $(OUT_TARGET_OBJ)/$(M_NAME)/,$(M_OBJS))
DEPS += $(M_OBJS:%o=%d)

ALL += $(OUT)/$(M_NAME).bin
ALL += $(OUT)/$(M_NAME).lst

$(OUT_TARGET_OBJ)/$(M_NAME)/%.o: %.c
	@$(MKDIR)
	@echo compile $<
	$(QUIET)$(TARGET_CC) $(TARGET_CFLAGS) $(_CFLAGS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)

$(OUT_TARGET_OBJ)/$(M_NAME)/%.o: %.S
	@$(MKDIR)
	@echo assemble $<
	$(QUIET)$(TARGET_CC) $(TARGET_CFLAGS) $(_CFLAGS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)

# apply our flags to our objects
$(M_OBJS): _CFLAGS := $(M_ARCH_CFLAGS) $(M_CFLAGS)

$(OUT)/$(M_NAME).bin: _SRC := $(OUT)/$(M_NAME)
$(OUT)/$(M_NAME).bin: $(OUT)/$(M_NAME)
	@echo create $@
	$(QUIET)$(TARGET_OBJCOPY) --gap-fill=0xee -O binary $(_SRC) $@

$(OUT)/$(M_NAME).lst: _SRC := $(OUT)/$(M_NAME)
$(OUT)/$(M_NAME).lst: $(OUT)/$(M_NAME)
	@echo create $@
	$(QUIET)$(TARGET_OBJDUMP) -d $(_SRC) > $@

$(OUT)/$(M_NAME): _OBJS := $(M_OBJS)
$(OUT)/$(M_NAME): _LIBS := $(M_LIBS)
$(OUT)/$(M_NAME): _LINK := $(M_LINK)
$(OUT)/$(M_NAME): $(M_OBJS)
	@echo link $@
	$(QUIET)$(TARGET_LD) $(TARGET_LFLAGS) -Bstatic -T $(_LINK) $(_OBJS) $(_LIBS) -o $@

$(info module $(M_NAME))

M_OBJS :=
M_NAME :=
M_BASE :=
M_LIBS :=
M_ARCH :=
M_CFLAGS :=
