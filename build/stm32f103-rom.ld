
MEMORY {
	RAM (xrw)	: ORIGIN = 0x20000000, LENGTH = 20K
	FLASH (rx)	: ORIGIN = 0x08000000, LENGTH = 128K
}

SECTIONS {
	.text : {
		. = ALIGN(4);
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(4);
		__data_init__ = . ;
	} >FLASH
	.data : AT(__data_init__) {
		. = ALIGN(4);
		__data_start__ = . ;
		*(.data)
		*(.data.*)
		. = ALIGN(4);
		__data_end__ = . ;
	} >RAM
	.bss : {
		. = ALIGN(4);
		__bss_start__ = . ;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = . ;
	} >RAM
}
