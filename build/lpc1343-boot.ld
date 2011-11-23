
MEMORY {
	RAM (xrw)	: ORIGIN = 0x10001c00, LENGTH = 1K
	FLASH (rx)	: ORIGIN = 0x00000000, LENGTH = 4K
}

INCLUDE "build/generic-rom.ld"
