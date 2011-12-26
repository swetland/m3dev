
#include <fw/types.h>
#include <arch/hardware.h>

u32 gpio_led0 = MKGPIO(3, 0);
u32 gpio_led1 = MKGPIO(3, 1);
u32 gpio_led2 = MKGPIO(3, 2);
u32 gpio_led3 = MKGPIO(3, 3);

u8 board_name[] = "LPC-P1343";

void board_init(void) {
	core_48mhz_init();
}
