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

#include <protocol/rswdp.h>
#include "swdp.h"

extern unsigned swdp_trace;

void clocks_to_72mhz() {
	writel(readl(RCC_CR) | 0x00010001, RCC_CR);
	while ((readl(RCC_CR) & 0x00020000) == 0) ;

	writel(readl(RCC_CFGR) | 0x001D0400, RCC_CFGR);
	writel(readl(RCC_CR) | 0x01000000, RCC_CR);
	while ((readl(RCC_CR) & 0x03000000) == 0) ;

	writel(readl(RCC_CFGR) | 2, RCC_CFGR);
	while ((readl(RCC_CFGR) & 8) == 0) ;
}

extern u8 __bss_start__;
extern u8 __bss_end__;
void bss_init() {
	u32 *bss, *end;
	bss = (void*) &__bss_start__;
	end = (void*) &__bss_end__;
	while (bss < end)
		*bss++ = 0;
}

int online = 0;
int check_target(void) { 
	if (!online) {
		printx("swdp_reset()\n");
		if (swdp_reset() != IDCODE_M3)
			return -1;
		online = 1;
		printx("online\n");
	}
	return 0;
}

#define DEBUG_MAX 59

static struct raw pmsg = {
	.msg = MSG_DEBUG,
	.seq = 0xFF,
	.pr0 = 0x00,
	.pr1 = 0x01,
};
static unsigned poff = 0;

void flushu(void) {
	pmsg.data[poff] = 0;
	usb_xmit(&pmsg, sizeof(pmsg));
	poff = 0;
}

void _putu(unsigned c) {
	pmsg.data[poff++] = c;
	if (poff == DEBUG_MAX)
		flushu();
}

void printu(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintx(_putu, fmt, ap);
	va_end(ap);
	if (poff)
		flushu();
}

static u8 optable[16] = {
	[OP_RD | OP_DP | OP_X0] = RD_IDCODE,
	[OP_RD | OP_DP | OP_X4] = RD_DPCTRL,
	[OP_RD | OP_DP | OP_X8] = RD_RESEND,
	[OP_RD | OP_DP | OP_XC] = RD_BUFFER,
	[OP_WR | OP_DP | OP_X0] = WR_ABORT,
	[OP_WR | OP_DP | OP_X4] = WR_DPCTRL,
	[OP_WR | OP_DP | OP_X8] = WR_SELECT,
	[OP_WR | OP_DP | OP_XC] = WR_BUFFER,
	[OP_RD | OP_AP | OP_X0] = RD_AP0,
	[OP_RD | OP_AP | OP_X4] = RD_AP1,
	[OP_RD | OP_AP | OP_X8] = RD_AP2,
	[OP_RD | OP_AP | OP_XC] = RD_AP3,
	[OP_WR | OP_AP | OP_X0] = WR_AP0,
	[OP_WR | OP_AP | OP_X4] = WR_AP1,
	[OP_WR | OP_AP | OP_X8] = WR_AP2,
	[OP_WR | OP_AP | OP_XC] = WR_AP3,
};

void process_swdp_message(struct msg *msg) {
	unsigned count = msg->pr0;
	unsigned n;

	if (count > 12) {
		printx("error: >12 swdp ops\n");
		return;
	}
	for (n = 0; n < count; n++) {
		unsigned op = msg->op[n];
		if (op > 15) {
			printx("error: swdp opcode invalid\n");
			return;
		}
		if (op & OP_WR) {
			swdp_write(optable[op], msg->data[n]);
		} else {
			msg->data[n] = swdp_read(optable[op]);
		}
	}
	msg->msg = MSG_OKAY;
}

static struct msg msg;

#define GPIO_LED 5

int main() {
	writel(readl(RCC_APB2ENR) |
		RCC_APB2_GPIOA | RCC_APB2_USART1,
		RCC_APB2ENR);

	gpio_config(9, GPIO_OUTPUT_10MHZ | GPIO_ALT_PUSH_PULL);
	gpio_config(GPIO_LED, GPIO_OUTPUT_10MHZ | GPIO_OUT_PUSH_PULL);

	clocks_to_72mhz();
	bss_init();
	serial_init(72000000, 115200);
	printx("[ rswdp agent v0.9 ]\n");

#if 0
	irq_set_base(0x20001000);
	irq_enable(i_usb_lp);
	irq_enable(i_usb_hp);
#endif

	usb_init();

	for (;;) {
		gpio_clr(GPIO_LED);
		if (usb_recv(&msg) != 64) {
			printx("error: runt message\n");
			continue;
		}
		gpio_set(GPIO_LED);
		switch (msg.msg) {
		case MSG_NULL:
			msg.msg = MSG_OKAY;
			break;
		case MSG_DEBUG:
			((char*) &msg)[63] = 0;
			printx(((char*) &msg) + 4);
			continue;
		case MSG_TRACE:
			msg.msg = MSG_OKAY;
			swdp_trace = msg.pr0;
			break;
		case MSG_ATTACH:
			msg.msg = MSG_OKAY;
			msg.data[0] = swdp_reset();
			break;
		case MSG_SWDP_OPS:
			msg.msg = MSG_FAIL;
			process_swdp_message(&msg);
			break;
		default:
			msg.msg = MSG_FAIL;
		}
		usb_xmit(&msg, 64);
	}
}

