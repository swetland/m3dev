
M_NAME := agent-lpc15xx
M_CHIP := lpc1547-agt
M_START := agents/lpc-header.o
M_OBJS := agents/lpc-main.o
$(call build-target-executable)

M_NAME := agent-lpc13xx
M_CHIP := lpc1343-agt
M_START := agents/lpc-header.o
M_OBJS := agents/lpc-main.o
$(call build-target-executable)

