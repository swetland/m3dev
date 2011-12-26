
#include <fw/types.h>
#include <arch/hardware.h>

u32 gpio_led0 = MKGPIO(0, 2);
u32 gpio_led1 = MKGPIO(2, 7);
u32 gpio_led2 = MKGPIO(2, 8);
u32 gpio_led3 = MKGPIO(2, 1);

u8 board_name[] = "M3DEBUG";

void board_init(void) {
	core_48mhz_init();
}
