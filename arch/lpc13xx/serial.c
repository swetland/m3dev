/* serial.c
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

/* only works for multiples of 12MHz and 115.2kbaud right now */
void serial_init(unsigned sysclk_mhz, unsigned baud) {

	if (sysclk_mhz % 12000000)
		return;
	if (baud != 115200)
		return;

	writel(IOCON_FUNC_1 | IOCON_DIGITAL, IOCON_PIO1_6); /* RXD */
	writel(IOCON_FUNC_1 | IOCON_DIGITAL, IOCON_PIO1_7); /* TXD */
	writel(sysclk_mhz / 12000000, SYS_DIV_UART); /* SYS / n -> 12MHz */
	writel(readl(SYS_CLK_CTRL) | SYS_CLK_UART, SYS_CLK_CTRL);

	writel(0x80, 0x4000800C); /* DLAB=1 */
	writel(0x04, 0x40008000); /* DLL=4 */
	writel(0x00, 0x40008004); /* DLM=0 */
	writel(0x85, 0x40008028); /* DIVADDVAL=5, MULVAL=8 */
	writel(0x03, 0x4000800C); /* DLAB=0, 8N1 */
	writel(1, 0x40008008); /* enable FIFO */
	writel(1, 0x40008008); /* enable FIFO */
}

void serial_putc(unsigned c) {
	/* wait until xmit holding register is empty */
	while (!(readl(0x40008014) & (1 << 5))) ;
	writel(c, 0x40008000);
}

