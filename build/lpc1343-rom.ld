
MEMORY {
	RAM (xrw)	: ORIGIN = 0x10000000, LENGTH = 8K
	FLASH (rx)	: ORIGIN = 0x00000000, LENGTH = 32K
}

INCLUDE "build/generic-rom.ld"
