
#include <fw/types.h>
#include <arch/hardware.h>

u32 gpio_led0 = MKGPIO(2, 4);
u32 gpio_led1 = MKGPIO(2, 5);
u32 gpio_led2 = MKGPIO(2, 9);
u32 gpio_led3 = MKGPIO(2, 7);

u8 board_name[] = "M3RADIO1";

void board_init(void) {
	core_48mhz_init();
}
