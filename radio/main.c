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
#include <fw/io.h>

#include <arch/hardware.h>

/* CC1101 module hooked up to Olimex LPC-P1343 board
 *
 * PIO2_10 (CSEL) -> CSn
 * PIO2_11 (SCK)  -> SCLK
 * PIO0_9 (MOSI)  -> SI
 * PIO0_8 (MISO)  <- SO
 * PIO1_6         <- GDO2
 *
 */

#include "cc1101.h"
#include "radio-config.h"

#define GPIO_LED0	((3<<16) | (1<<0))
#define GPIO_LED1	((3<<16) | (1<<1))
#define GPIO_LED2	((3<<16) | (1<<2))
#define GPIO_LED3	((3<<16) | (1<<3))

#define GPIO_CS		((2<<16) | (1<<10))
#define GPIO_GDO2	((1<<16) | (1<<6)

#define SSP_CFG(n) (SSP_CR0_BITS(n) | SSP_CR0_FRAME_SPI | SSP_CR0_CLK_LOW | SSP_CR0_PHASE0 | SSP_CR0_CLOCK_RATE(1))

unsigned cc_rd(unsigned reg) {
	gpio_clr(GPIO_CS);
	writel(SSP_CFG(16), SSP0_CR0);
	writel(0x8000 | (reg << 8), SSP0_DR);
	while (readl(SSP0_SR) & SSP_BUSY) ;
	gpio_set(GPIO_CS);
	return readl(SSP0_DR) & 0xFF;
}

void cc_wr(unsigned reg, unsigned val) {
	gpio_clr(GPIO_CS);
	writel(SSP_CFG(16), SSP0_CR0);
	writel(((reg & 0x3f) << 8) | (val & 0xFF), SSP0_DR);
	while (readl(SSP0_SR) & SSP_BUSY) ;
	gpio_set(GPIO_CS);
	readl(SSP0_DR);
}

unsigned cc_cmd(unsigned cmd) {
	gpio_clr(GPIO_CS);
	writel(SSP_CFG(8), SSP0_CR0);
	writel(cmd & 0x3f, SSP0_DR);
	while (readl(SSP0_SR) & SSP_BUSY) ;
	gpio_set(GPIO_CS);
	return readl(SSP0_DR) & 0xFF;
}

unsigned cc_state(void) {
	unsigned n, last;
	last = cc_cmd(CC_CMD_SNOP);
	for (;;) {
		n = cc_cmd(CC_CMD_SNOP);
		if (n == last) break;
		last = n;
	}
	return n >> 4;
}

void cc1101_init(void) {
	unsigned n;
	for (n = 0; n < (sizeof(config) / sizeof(config[0])); n++) {
		cc_wr(config[n].reg, config[n].val);
	}
}

void LED(unsigned n) {
	if (n & 1) 
		gpio_clr(GPIO_LED0);
	else
		gpio_set(GPIO_LED0);
	if (n & 2) 
		gpio_clr(GPIO_LED1);
	else
		gpio_set(GPIO_LED1);
	if (n & 4) 
		gpio_clr(GPIO_LED2);
	else
		gpio_set(GPIO_LED2);
	if (n & 8) 
		gpio_clr(GPIO_LED3);
	else
		gpio_set(GPIO_LED3);
}

volatile unsigned RX_START = 0;
volatile unsigned RX_OKAY = 0;
volatile unsigned RX_IDLE_OUT = 0;
volatile unsigned RX_OVERFLOW = 0;
volatile unsigned RX_IDLE_RESTART = 0;
volatile unsigned RX_NON16 = 0;

u8 PACKET[16];

static void radio_init(void) {
	unsigned n;

	cc_cmd(CC_CMD_SRES);
	for (n = 0; n < 100000; n++) asm("nop");
	cc_cmd(CC_CMD_SRES);
	for (n = 0; n < 100000; n++) asm("nop");

	cc1101_init();

	/* active low, assert on start-of-packet, deassert on success or fail */
	cc_wr(CC_IOCFG2, (1<<6) | 6);
	/* hi-z */
	cc_wr(CC_IOCFG1, 46);
	cc_wr(CC_IOCFG0, 46);

	cc_wr(CC_PKTLEN, 16);
	/* qual thres = 0, crc-autoflush = 1, no address check */
	cc_wr(CC_PKTCTRL1, (1<<3));
	/* data whitening, crc, fixed len, normal packet mode */
	cc_wr(CC_PKTCTRL0, (1<<6) | (1<<2));
	/* MDMCFG1 defaults to FEC=0 preamble bytes = 4 */

	/* PA TABLE 0 */
	cc_wr(0x3e, 0xC0);

	cc_wr(CC_CHANNR, 0);

	cc_cmd(CC_CMD_SCAL);
	while (cc_state() != CC_S_IDLE) ;
}

static void radio_tx(void) {
	unsigned n, m;

	m = 0;
	for (;;) {
		LED(m++);
		cc_wr(0x3F, 0xAA);
		for (n = 0; n < 10; n++) {
			cc_wr(0x3F, 0x42);
		}
		cc_wr(0x3F, ((m >> 24) & 0xFF));
		cc_wr(0x3F, ((m >> 16) & 0xFF));
		cc_wr(0x3F, ((m >> 8) & 0xFF));
		cc_wr(0x3F, ((m >> 0) & 0xFF));
		cc_wr(0x3F, 0xFF);
		cc_cmd(CC_CMD_STX);
		while (cc_state() != CC_S_IDLE);
	}
}

static void radio_rx2(void) {
	unsigned n, m;

	/* pos edge */
	writel(readl(GPIOPOLARITY(1)) | (1<<6), GPIOPOLARITY(1));

	m = 1;
	for (;;) {
		LED(m);
		RX_START++;
		writel((1<<6), GPIOICR(1));
		cc_cmd(CC_CMD_SRX);

		/* wait for IRQ */
		while (!(readl(GPIORAWISR(1)) & (1<<6))) ;

		do {
			n = cc_rd(CC_PKTSTATUS);
		} while (n != cc_rd(CC_PKTSTATUS));

		if (n & 0x80) {
			if (cc_rd(CC_RXBYTES) == 16) {
				for(n = 0; n < 16; n++)
					PACKET[n] = cc_rd(0x3F);
				RX_OKAY++;
				m++;
			} else {
				RX_NON16++;
			}
		} else {
			RX_IDLE_OUT++;
		}

		/* reset to idle in the event of an overflow */
		if (cc_state() == CC_S_RX_OVF) {
			RX_OVERFLOW++;
			cc_cmd(CC_S_IDLE);
		}
	}
}

static void radio_rx(void) {
	unsigned s, n, m;

	m = 0;
restart:
	RX_START++;
	cc_cmd(CC_CMD_SRX);
	for (;;) {
		LED(m);
		s = cc_state();
		switch (s) {
		case CC_S_CALIBRATE:
		case CC_S_SETTLING:
			continue;
		case CC_S_RX:
			break;
		case CC_S_IDLE:
			do {
				n = cc_rd(CC_PKTSTATUS);
			} while (n != cc_rd(CC_PKTSTATUS));
			if (n & 0x80) {
				int i;
				n = cc_rd(CC_RXBYTES);
				if (n != 16) {
					RX_NON16++;
					continue;
				}
				for(i = 0; i < 16; i++)
					PACKET[i] = cc_rd(0x3F);
				while (cc_state() != CC_S_IDLE) ;
				m++;
				RX_OKAY++;
				goto restart;
			}
			RX_IDLE_OUT++;
			goto restart;
		case CC_S_RX_OVF:
			RX_OVERFLOW++;
			cc_cmd(CC_CMD_SFRX);
			while (cc_state() != CC_S_IDLE) ;
			goto restart;
		default:
			/* should not be possible */
			for (;;) ;
		}
	}
}

int main() {
        board_init();

	gpio_config(GPIO_CS, 1);
	gpio_set(GPIO_CS);

        gpio_config(GPIO_LED0, 1);
        gpio_config(GPIO_LED1, 1);
        gpio_config(GPIO_LED2, 1);
        gpio_config(GPIO_LED3, 1);
	LED(0);

	radio_init();

#if CONFIG_TX
	radio_tx();
#elif CONFIG_RX
	radio_rx2();
#else
#error not configured
#endif
}
