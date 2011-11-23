/* board.c
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
#include <fw/io.h>
#include <fw/lib.h>

#include <arch/hardware.h>

void board_init(void) {
	/* switch to IRC if we aren't on it already */
	if ((readl(0x40048070) & 3) != 0) {
		writel(0, 0x40048070);
		writel(0, 0x40048074);
		writel(1, 0x40048074);
	}

	/* power down SYS PLL */
	writel(readl(0x40048238) | (1 << 7), 0x40048238);

	/* configure SYS PLL */
	writel(0x23, 0x40048008); /* MSEL=4, PSEL=2, OUT=48MHz */
//	writel(0x25, 0x40048008); /* MSEL=6, PSEL=2, OUT=72MHz */

	/* power up SYS PLL */
	writel(readl(0x40048238) & (~(1 << 7)), 0x40048238);

	/* wait for SYS PLL to lock */
	while(!(readl(0x4004800c) & 1)) ;

	/* select SYS PLL OUT for MAIN CLK */
	writel(3, 0x40048070);
	writel(0, 0x40048074);
	writel(1, 0x40048074);

	/* select Main clock for USB CLK */
	writel(1, 0x400480C0);
	writel(0, 0x400480C4);
	writel(1, 0x400480C4);

	/* set USB clock divider to 1 */
	writel(1, 0x400480C8);

	/* assert reset, disable clock */
	writel(readl(PRESETCTRL) & (~SSP0_RST_N), PRESETCTRL);
	writel(0, SYS_DIV_SSP0);

	/* enable SSP0 clock */
	writel(readl(SYS_CLK_CTRL) |
		SYS_CLK_SSP0 | SYS_CLK_IOCON | SYS_CLK_GPIO,
		SYS_CLK_CTRL);

	serial_init(48000000,115200);

	/* SSP0 PCLK = SYSCLK / 1 (8MHz) */
	writel(2, SYS_DIV_SSP0);

	/* deassert reset */
	writel(readl(PRESETCTRL) | SSP0_RST_N, PRESETCTRL);

	writel(2, SSP0_CPSR); /* prescale = PCLK/2 */
	writel(SSP_CR0_BITS(16) | SSP_CR0_FRAME_SPI |
		SSP_CR0_CLK_HIGH | SSP_CR0_PHASE1 |
		SSP_CR0_CLOCK_RATE(1),
		SSP0_CR0);
	writel(SSP_CR1_ENABLE | SSP_CR1_MASTER, SSP0_CR1);

	writel(IOCON_SCK0_PIO2_11, IOCON_SCK0_LOC);
	writel(IOCON_FUNC_1 | IOCON_DIGITAL, IOCON_PIO0_8); /* MISO */
	writel(IOCON_FUNC_1 | IOCON_DIGITAL, IOCON_PIO0_9); /* MOSI */
	writel(IOCON_FUNC_1 | IOCON_DIGITAL, IOCON_PIO2_11); /* SCK */
}

