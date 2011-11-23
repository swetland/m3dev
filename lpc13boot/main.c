/* main.c
 *
 * Copyright 2011 Brian Swetland <swetland@frotz.net>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fw/types.h>
#include <fw/lib.h>

#define GPIO_LED0 0x00004
#define GPIO_LED1 0x20080
#define GPIO_LED2 0x20100
#define GPIO_LED3 0x20002

#define RAM_BASE	0x10000000
#define RAM_SIZE	(7 * 1024)

#define ROM_BASE	0x00001000
#define ROM_SIZE	(28 * 1024)

void (*romcall)(u32 *in, u32 *out) = (void*) 0x1fff1ff1;

/* { PREPARE, startpage, endpage } */
#define OP_PREPARE	50
/* { WRITE, dstaddr, srcaddr, bytes, sysclk_khz } */
#define OP_WRITE	51
/* { ERASE, startpage, endpage, sysclk_khz } */
#define OP_ERASE	52

struct device_info {
	u8 part[16];
	u8 board[16];
	u32 version;
	u32 ram_base;
	u32 ram_size;
	u32 rom_base;
	u32 rom_size;
	u32 unused0;
	u32 unused1;
	u32 unused2;
};

struct device_info DEVICE = {
	.part = "LPC1343",
	.board = "M3DEBUG",
	.version = 0x0001000,
	.ram_base = RAM_BASE,
	.ram_size = RAM_SIZE,
	.rom_base = ROM_BASE,
	.rom_size = ROM_SIZE,
};

void iap_erase_page(unsigned addr) {
	u32 in[4];
	u32 out[5];
	in[0] = OP_PREPARE;
	in[1] = (addr >> 12) & 0xF;
	in[2] = (addr >> 12) & 0xF;
	romcall(in, out);
	in[0] = OP_ERASE;
	in[1] = (addr >> 12) & 0xF;
	in[2] = (addr >> 12) & 0xF;
	in[3] = 48000;
	romcall(in, out);
}

void iap_write_page(unsigned addr, void *src) {
	u32 in[5];
	u32 out[5];
	in[0] = OP_PREPARE;
	in[1] = (addr >> 12) & 0xF;
	in[2] = (addr >> 12) & 0xF;
	romcall(in, out);
	in[0] = OP_WRITE;
	in[1] = addr;
	in[2] = (u32) src;
	in[3] = 0x1000;
	in[4] = 48000;
	romcall(in, out);
}

static u32 buf[64];

void start_image(u32 pc, u32 sp);

void boot_image(void *img) {
        gpio_config(GPIO_LED0, 0);
        gpio_config(GPIO_LED1, 0);
        gpio_config(GPIO_LED2, 0);
        gpio_config(GPIO_LED3, 0);

	usb_stop();

	/* TODO: shut down various peripherals */
	start_image(((u32*)img)[1], ((u32*)img)[0]);
}

void handle(u32 magic, u32 cmd, u32 arg) {
	u32 reply[2];
	u32 addr, xfer;
	void *ram = (void*) RAM_BASE;

	if (magic != 0xDB00A5A5)
		return;

	reply[0] = magic;
	reply[1] = -1;

	switch (cmd) {
	case 'E':
		iap_erase_page(0x1000);
		reply[1] = 0;
		break;
	case 'W':
		if (arg > ROM_SIZE)
			break;
		reply[1] = 0;
		usb_xmit(reply, 8);
		addr = ROM_BASE;
		while (arg > 0) {
			xfer = (arg > 4096) ? 4096 : arg;
			usb_recv(ram, xfer);
			iap_erase_page(addr);
			iap_write_page(addr, ram);
			addr += 4096;
			arg -= xfer;
		}
		break;
	case 'X':
		if (arg > RAM_SIZE)
			break;
		reply[1] = 0;
		usb_xmit(reply, 8);
		usb_recv(ram, arg);
		usb_xmit(reply, 8);

		/* let last txn clear */
		usb_recv_timeout(buf, 64, 10);

		boot_image(ram);
		break;
	case 'Q':
		reply[1] = 0;
		usb_xmit(reply, 8);
		usb_xmit(&DEVICE, sizeof(DEVICE));
		return;
	default:
		break;
	}
	usb_xmit(reply, 8);
}

int main() {
	int n, x, timeout;
	u32 tmp;

	board_init();
        gpio_config(GPIO_LED0, 1);
        gpio_config(GPIO_LED1, 1);
        gpio_config(GPIO_LED2, 1);
        gpio_config(GPIO_LED3, 1);

	usb_init(0x18d1,0xdb00);

	tmp = *((u32*) 0x1000);

	if ((tmp != 0) && (tmp != 0xFFFFFFFF))
		timeout = 30;
	else
		timeout = 0;

	x = 0;
	for (;;) {
		if (x & 1) {
			gpio_set(GPIO_LED0);
			gpio_set(GPIO_LED1);
			gpio_set(GPIO_LED2);
			gpio_set(GPIO_LED3);
		} else {
			gpio_clr(GPIO_LED0);
			gpio_clr(GPIO_LED1);
			gpio_clr(GPIO_LED2);
			gpio_clr(GPIO_LED3);
		}
		x++;
		n = usb_recv_timeout(buf, 64, 100);
		if ((n == -ETIMEOUT) && timeout) {
			timeout--;
			if (timeout == 0)
				break;
		}
		if (n == 12) {
			timeout = 0;
			handle(buf[0], buf[1], buf[2]);
		}
	}

	boot_image((void*) 0x1000);

	for (;;) ;
}

