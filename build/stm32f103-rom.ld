
MEMORY {
	RAM (xrw)	: ORIGIN = 0x20000000, LENGTH = 20K
	FLASH (rx)	: ORIGIN = 0x08000000, LENGTH = 128K
}

INCLUDE "build/generic-rom.ld"
