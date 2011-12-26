
#include <fw/types.h>
#include <arch/hardware.h>

const u32 gpio_led0 = MKGPIO(0, 2);
const u32 gpio_led1 = MKGPIO(2, 7);
const u32 gpio_led2 = MKGPIO(2, 8);
const u32 gpio_led3 = MKGPIO(2, 1);

const u32 gpio_reset_n = MKGPIO(2, 10);

const u8 board_name[] = "M3DEBUG";

void board_init(void) {
	core_48mhz_init();
}
