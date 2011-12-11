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

#include "mrf89xa.h"

#define GPIO_LED0	((2<<16) | (1<<4))
#define GPIO_LED1	((2<<16) | (1<<5))
#define GPIO_LED2	((2<<16) | (1<<9))
#define GPIO_LED3	((0<<16) | (1<<7))

#define GPIO_CSCON	((0<<16) | (1<<11))
#define GPIO_CSDATA	((2<<16) | (1<<10))
#define GPIO_IRQ0	((1<<16) | (0<<6)
#define GPIO_IRQ1	((2<<16) | (2<<6)
#define GPIO_RESET	((1<<16) | (1<<10))

#define SSP_CFG(n) (SSP_CR0_BITS(n) | SSP_CR0_FRAME_SPI | SSP_CR0_CLK_LOW | SSP_CR0_PHASE0 | SSP_CR0_CLOCK_RATE(1))

void timer_init(void) {
	writel(readl(SYS_CLK_CTRL) | SYS_CLK_CT32B0, SYS_CLK_CTRL);
	writel(48, TM32B0PR); 
}
void timer_reset(void) {
	writel(TM32TCR_RESET, TM32B0TCR);
	writel(TM32TCR_ENABLE, TM32B0TCR);
}
static inline unsigned timer_read(void) {
	return readl(TM32B0TC);
}
void timer_delay(unsigned usec) {
	timer_reset();
	while (timer_read() < usec) ;
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

unsigned mrf89wr(unsigned reg, unsigned val) {
	gpio_clr(GPIO_CSCON);
	writel(SSP_CFG(16), SSP0_CR0);
	writel(((reg & 0x1F) << 9) | (val & 0xFF), SSP0_DR);
	while (readl(SSP0_SR) & SSP_BUSY) ;
	gpio_set(GPIO_CSCON);
	return readl(SSP0_DR) & 0xFF;
}

unsigned mrf89rd(unsigned reg) {
	gpio_clr(GPIO_CSCON);
	writel(SSP_CFG(16), SSP0_CR0);
	writel(0x4000 | ((reg & 0x1F) << 9), SSP0_DR);
	while (readl(SSP0_SR) & SSP_BUSY) ;
	gpio_set(GPIO_CSCON);
	return readl(SSP0_DR) & 0xFF;
}

void fifo_wr(u8 *data, unsigned len) {
	writel(SSP_CFG(8), SSP0_CR0);
	while (len-- > 0) {
		gpio_clr(GPIO_CSDATA);
		writel(*data++, SSP0_DR);
		while (readl(SSP0_SR) & SSP_BUSY) ;
		gpio_set(GPIO_CSDATA);
		readl(SSP0_DR);
	}
}

void fifo_rd(u8 *data, unsigned len) {
	writel(SSP_CFG(8), SSP0_CR0);
	while (len-- > 0) {
		gpio_clr(GPIO_CSDATA);
		writel(0, SSP0_DR);
		while (readl(SSP0_SR) & SSP_BUSY) ;
		gpio_set(GPIO_CSDATA);
		*data++ = readl(SSP0_DR);
	}
}

static void radio_init(unsigned bps) {
	/* no clock out */
	mrf89wr(CLKOREG, 0);

	mrf89wr(GCONREG, GCON_STANDBY | GCON_863_BAND | GCON_RPS_1);
	mrf89wr(DMODREG, DMOD_FSK | DMOD_IF_0DB | DMOD_PACKET);

	mrf89wr(FIFOCREG, FIFOC_64B | FIFOC_FTINT(0x3F));

	/* RX: IRQ0=SYNC, IRQ1=CRCOK TX: IRQ1=TXDONE */
	mrf89wr(FTXRXIREG, FTXRXI_P_IRQ0RX_SYNC |
		FTXRXI_P_IRQ1RX_CRCOK | FTXRXI_P_IRQ1TX_TXDONE);

	/* based on table A-1, MRF89XA datasheet */ 
	switch (bps) {
	case 10000:
		mrf89wr(FILCREG, FILC_PAS_157KHZ | FILC_BUT(X25KHZ));
		/* 12.8MHz / (64 * (19 + 1)) -> 10kbps */
		mrf89wr(BRSREG, 19);
		/* 12.8MHz / (32 * (11 + 1)) -> 33.333KHz */
		mrf89wr(FDEVREG, 11);
		break;
	case 33333:
	default:
		mrf89wr(FILCREG, FILC_PAS_378KHZ | FILC_BUT(X125KHZ));
		/* 12.8MHz / (64 * (5 + 1)) -> 33.333kbps */
		mrf89wr(BRSREG, 5);
		/* 12.8MHz / (32 * (5 + 1)) -> 66.666KHz */
		mrf89wr(FDEVREG, 5);
		break;
	case 100000:
		mrf89wr(FILCREG, FILC_PAS_987KHZ | FILC_BUT(X400KHZ));
		/* 12.8MHz / (64 * (1 + 1)) -> 100kbps */
		mrf89wr(BRSREG, 1);
		/* 12.8MHz / (32 * (3 + 1)) -> 100KHz */
		mrf89wr(FDEVREG, 3);
		break;
	}

	/* enable SYNC gen/rec, 32 bits, 0 errors accepted */
	mrf89wr(SYNCREG, SYNC_SYNCREN | SYNC_32BIT | SYNC_0ERR);

	mrf89wr(SYNCV31REG, 0xDA);
	mrf89wr(SYNCV23REG, 0xDA);
	mrf89wr(SYNCV15REG, 0xDA);
	mrf89wr(SYNCV07REG, 0xDA);

	mrf89wr(TXCONREG, TXCON_TXIPOLFV(X200KHZ) | TXCON_10DBM);
	timer_delay(500);

	mrf89wr(PKTCREG, PKTC_FIXED |
		PKTC_PREAMBLE_2B | PKTC_WHITENING |
		PKTC_ADDR_OFF | PKTC_CRC_ENABLE);

	mrf89wr(FCRCREG, FCRC_AUTO_CLEAR | FCRC_FIFO_STANDBY_READ);

	/* PLL setup
	 * 64 <= R <= 169, close to 119 preferred
	 * P + 1 > S
	 * fXtal = 12.8MHz
	 * fFSK = 9/8 * (fXtal / (R + 1)) * (75 * (P + 1) + S)
	 *
	 * R=119, P=95, S=36 -> 
         * ((9/8) * (12800000/(119+1))) * (75 * (95 + 1) + 36)
	 * -> 868.319999 MHz 
	 */
	mrf89wr(R1CREG, 119);
	mrf89wr(P1CREG, 95);
	mrf89wr(S1CREG, 36);
	mrf89wr(S1CREG, 36);
}

unsigned vcotune = 0;


unsigned volatile RSSI = 0xfff;

int packet_tx(void *data, unsigned len) {
	unsigned n;
	mrf89wr(GCONREG, (vcotune << 1) | GCON_TX |
		GCON_863_BAND | GCON_RPS_1);
	mrf89wr(FTPRIREG, FTPRI_SBO | FTPRI_PLL_LOCK_ENABLE |
		FTPRI_TX_ON_FIFO_NOT_EMPTY);
	mrf89wr(PLOADREG, len);
	fifo_wr(data, len);
	for (;;) {
		n = mrf89rd(FTPRIREG);
		if (n & FTPRI_TX_DONE) {
			mrf89wr(GCONREG, (vcotune << 1) | GCON_STANDBY |
				GCON_863_BAND | GCON_RPS_1);
			return 0;
		}
	}
}

int packet_rx(void *data, unsigned len, unsigned timeout) {
	timer_reset();
	mrf89wr(PLOADREG, len);
	mrf89wr(GCONREG, (vcotune << 1) | GCON_RX |
		GCON_863_BAND | GCON_RPS_1);
	for (;;) {
		if (mrf89rd(PKTCREG) & PKTC_CRC_OK) {
			fifo_rd(data, len);
			mrf89wr(GCONREG, (vcotune << 1) | GCON_STANDBY |
				GCON_863_BAND | GCON_RPS_1);
			return 0;
		}
		if (timer_read() > timeout) {
			return -1;
		}
	}
}

struct pkt {
	unsigned magic;
	unsigned serial;
	unsigned data0;
	unsigned data1;
};

struct pkt PX, PR;

static void radio_tx(void) {
	unsigned count = 0;

	for (;;) {	
		LED(count);

		PX.magic = 0x12345678;
		PX.serial = count;
		PX.data0 = 0xAAAAAAAA;
		PX.data1 = 0x55555555;

		packet_tx(&PX, sizeof(PX));

		if (packet_rx(&PR, sizeof(PR), 25000))
			continue;

		if (PR.serial != PX.serial)
			continue;

		count++;
	}
}

static void radio_rx(void) {
	unsigned count = 0;

	for (;;) {	
		LED(count);
		if (packet_rx(&PR, sizeof(PR), 0xffffffff))
			continue;
		packet_tx(&PR, sizeof(PR));
		count++;
	}	
}

int main() {
	unsigned n;

        board_init();
	timer_init();

	writel(IOCON_DIGITAL | IOCON_FUNC_1, IOCON_PIO1_0);
	writel(IOCON_DIGITAL | IOCON_FUNC_1, IOCON_PIO0_11);
	gpio_config(GPIO_CSCON, 1);
	gpio_config(GPIO_CSDATA, 1);
	gpio_set(GPIO_CSCON);
	gpio_set(GPIO_CSDATA);

        gpio_config(GPIO_LED0, 1);
        gpio_config(GPIO_LED1, 1);
        gpio_config(GPIO_LED2, 1);
        gpio_config(GPIO_LED3, 1);

	LED(1);
	gpio_config(GPIO_RESET, 1);
	gpio_set(GPIO_RESET);
	timer_delay(500);
	gpio_config(GPIO_RESET, 0);
	timer_delay(20000);

	radio_init(33333);

	LED(2);

	mrf89wr(FTPRIREG, FTPRI_SBO | FTPRI_PLL_LOCKED | FTPRI_PLL_LOCK_ENABLE);
	mrf89wr(GCONREG, (vcotune << 1) | GCON_FREQ_SYNTH | GCON_863_BAND | GCON_RPS_1);
	for (;;) {
		n = mrf89rd(FTPRIREG);
		if (n & FTPRI_PLL_LOCKED) {
			break;
		}
	}

#if CONFIG_RX
	radio_rx();
#endif
#if CONFIG_TX
	radio_tx();
#endif
	for (;;) ;
}

