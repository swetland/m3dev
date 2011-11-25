/* rswdp.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "usb.h"

#include <fw/types.h>
#include <protocol/rswdp.h>
#include "rswdp.h"

static volatile int ATTN;

void swdp_interrupt(void) {
	ATTN++;
	write(2, "\b\b*INTERRUPT*\n", 16);
}

static int match_18d1_db03(usb_ifc_info *ifc) {
	if (ifc->dev_vendor != 0x18d1)
		return -1;
	if (ifc->dev_product != 0xdb03)
		return -1;
	return 0;
}

static u16 sequence = 1;

static usb_handle *usb;

struct txn {
	/* words to transmit */
	u32 tx[1024];
	/* pointers to words for reply data */
	u32 *rx[1024];

	/* words to send and receive */
	unsigned txc;
	unsigned rxc;

	/* cached state */
	u32 cache_apaddr;
	u32 cache_ahbtar;

	unsigned magic;
};

static void process_async(u32 *data, unsigned count) {
	unsigned msg, op, n;
	u32 tmp;

	while (count-- > 0) {
		msg = *data++;
		op = RSWD_MSG_OP(msg);
		n = RSWD_MSG_ARG(msg);
		switch (RSWD_MSG_CMD(msg)) {
		case CMD_NULL:
			continue;
		case CMD_DEBUG_PRINT:
			if (n > count)
				return;
			tmp = data[n];
			data[n] = 0;
			printf("%s",(char*) data);
			data[n] = tmp;
			data += n;
			count -= n;
			break;
		default:
			return;
		}
	}	
}

static int process_reply(struct txn *t, u32 *data, int count) {
	unsigned msg, op, n, rxp, rxc;

	rxc = t->rxc;
	rxp = 0;

	while (count-- > 0) {
		msg = *data++;
		op = RSWD_MSG_OP(msg);
		n = RSWD_MSG_ARG(msg);

		//fprintf(stderr,"[ %02x %02x %04x ]\n",RSWD_MSG_CMD(msg), op, n);
		switch (RSWD_MSG_CMD(msg)) {
		case CMD_NULL:
			continue;
		case CMD_SWD_DATA:
			if (n > rxc) {
				fprintf(stderr,"reply overrun (%d > %d)\n", n, rxc);
				return -1;
			}
			while (n-- > 0) {
				//printf("data %08x -> %p\n", data[0], t->rx[rxp]);
				*(t->rx[rxp++]) = *data++;
				rxc--;
			}
			continue;
		case CMD_STATUS:
			return op ? -op : 0;
		default:
			fprintf(stderr,"unknown command 0x%02x\n", RSWD_MSG_CMD(msg));
			return -1;
		}
	}
	return 0;
}

static int q_exec(struct txn *t) {
	unsigned data[1028];
	unsigned seq = sequence++;
	int r;
	u32 id;

	if (t->magic != 0x12345678) {
		fprintf(stderr,"FATAL: bogus txn magic\n");
		exit(1);
	}
	t->magic = 0;

	/* If we are a multiple of 64, and not exactly 4K,
	 * add padding to ensure the target can detect the end of txn 
	 */
	if (((t->txc % 16) == 0) && (t->txc != 1024))
		t->tx[t->txc++] = RSWD_MSG(CMD_NULL, 0, 0);

	id = RSWD_TXN_START(seq);
	t->tx[0] = id;

	r = usb_write(usb, t->tx, t->txc * sizeof(u32));
	if (r != (t->txc * sizeof(u32))) {
		fprintf(stderr,"invoke: tx error\n");
		return -1;
	}
	for (;;) {
		r = usb_read(usb, data, 4096);
		if (r <= 0) {
			fprintf(stderr,"invoke: rx error\n");
			return -1;
		}
		if (r & 3) {
			fprintf(stderr,"invoke: framing error\n");
			return -1;
		}

		if (data[0] == id) {
			return process_reply(t, data + 1, (r / 4) - 1);
		} else if (data[0] == RSWD_TXN_ASYNC) {
			process_async(data + 1, (r / 4) - 1);
		} else {
			fprintf(stderr,"invoke: unexpected txn %08x (%d)\n", data[0], r);
		}
	}
}

static void q_check(struct txn *t, int n) {
	if ((t->txc + n) >= 1024) {
		fprintf(stderr,"FATAL: txn buffer overflow\n");
		exit(1);
	}

}

static void q_init(struct txn *t) {
	t->magic = 0x12345678;
	t->txc = 1;
	t->rxc = 0;
	t->cache_apaddr = 0xffffffff;
	t->cache_ahbtar = 0xffffffff;
}

#define SWD_WR(a,n) RSWD_MSG(CMD_SWD_WRITE, OP_WR | (a), (n))
#define SWD_RD(a,n) RSWD_MSG(CMD_SWD_READ, OP_RD | (a), (n))
#define SWD_RX(a,n) RSWD_MSG(CMD_SWD_DISCARD, OP_RD | (a), (n))

static void q_ap_select(struct txn *t, u32 addr) {
	addr &= 0xF0;
	if (t->cache_apaddr != addr) {
		t->tx[t->txc++] = SWD_WR(DP_SELECT, 1);
		t->tx[t->txc++] = addr;
		t->cache_apaddr = addr;
	}
}

static void q_ap_write(struct txn *t, u32 addr, u32 value) {
	q_check(t, 3);
	q_ap_select(t, addr);
	t->tx[t->txc++] = SWD_WR(OP_AP | (addr & 0xC), 1);
	t->tx[t->txc++] = value;
}

static void q_ap_read(struct txn *t, u32 addr, u32 *value) {
	q_check(t, 4);
	q_ap_select(t, addr);
	t->tx[t->txc++] = SWD_RX(OP_AP | (addr & 0xC), 1);
	t->tx[t->txc++] = SWD_RD(DP_BUFFER, 1);
	t->rx[t->rxc++] = value;
}

static void q_ahb_write(struct txn *t, u32 addr, u32 value) {
	if (t->cache_ahbtar != addr) {
		q_ap_write(t, AHB_TAR, addr);
		t->cache_ahbtar = addr;
	}
	q_ap_write(t, AHB_DRW, value);
}

static void q_ahb_read(struct txn *t, u32 addr, u32 *value) {
	if (t->cache_ahbtar != addr) {
		q_ap_write(t, AHB_TAR, addr);
		t->cache_ahbtar = addr;
	}
	q_ap_read(t, AHB_DRW, value);
}

int swdp_ap_write(u32 addr, u32 value) {
	struct txn t;
	q_init(&t);
	q_ap_write(&t, addr, value);
	return q_exec(&t);
}

int swdp_ap_read(u32 addr, u32 *value) {
	struct txn t;
	q_init(&t);
	q_ap_read(&t, addr, value);
	return q_exec(&t);
}

int swdp_ahb_read(u32 addr, u32 *value) {
	struct txn t;
	q_init(&t);
	q_ahb_read(&t, addr, value);
	return q_exec(&t);
}

int swdp_ahb_write(u32 addr, u32 value) {
	struct txn t;
	q_init(&t);
	q_ahb_write(&t, addr, value);
	return q_exec(&t);
}

#if 0
/* simpler but far less optimal. keeping against needing to debug */
int swdp_ahb_read32(u32 addr, u32 *out, int count) {
	struct txn t;
	while (count > 0) {
		int xfer = (count > 128) ? 128: count;
		count -= xfer;
		q_init(&t);
		while (xfer-- > 0) {
			q_ahb_read(&t, addr, out++);
			addr += 4;
		}
		if (q_exec(&t))
			return -1;
	}
	return 0;
}

int swdp_ahb_write32(u32 addr, u32 *in, int count) {
	struct txn t;
	while (count > 0) {
		int xfer = (count > 128) ? 128: count;
		count -= xfer;
		q_init(&t);
		while (xfer-- > 0) {
			q_ahb_write(&t, addr, *in++);
			addr += 4;
		}
		if (q_exec(&t))
			return -1;
	}
	return 0;
}
#else
/* 10 txns overhead per 128 read txns - 126KB/s on 72MHz STM32F
 * 8 txns overhead per 128 write txns - 99KB/s on 72MHz STM32F
 */
int swdp_ahb_read32(u32 addr, u32 *out, int count) {
	struct txn t;

	while (count > 0) {
		int xfer;

		/* auto-inc wraps at page boundaries... */
		if ((addr & 0xFFF) > 0x800) {
			xfer = (0x1000 - (addr & 0xFFF)) / 4;
			if (xfer > count)
				xfer = count;
		} else {
			xfer = (count > 512) ? 512 : count;
		}

		count -= xfer;
		q_init(&t);

		/* setup before initial txn */
		q_ap_write(&t, AHB_CSW,
			AHB_CSW_MDEBUG | AHB_CSW_PRIV | AHB_CSW_INC_SINGLE |
			AHB_CSW_DBG_EN | AHB_CSW_32BIT);

		/* initial address */
		q_ap_write(&t, AHB_TAR, addr);
		addr += xfer * 4;

		/* kick off first read, ignore result, as the
		 * real result will show up during the *next* read
		 */
		t.tx[t.txc++] = SWD_RX(OP_AP | (AHB_DRW & 0xC), 1);
		t.tx[t.txc++] = SWD_RD(OP_AP | (AHB_DRW & 0xC), xfer -1);
		while (xfer-- > 1)
			t.rx[t.rxc++] = out++;
		t.tx[t.txc++] = SWD_RD(DP_BUFFER, 1);
		t.rx[t.rxc++] = out++;

		/* restore state after last batch */
		if (count == 0)
			q_ap_write(&t, AHB_CSW,
				AHB_CSW_MDEBUG | AHB_CSW_PRIV |
				AHB_CSW_DBG_EN | AHB_CSW_32BIT);

		if (q_exec(&t))
			return -1;
	}
	return 0;
}

int swdp_ahb_write32(u32 addr, u32 *in, int count) {
	struct txn t;

	while (count > 0) {
		int xfer;

		/* auto-inc wraps at page boundaries... */
		if ((addr & 0xFFF) > 0x800) {
			xfer = (0x1000 - (addr & 0xFFF)) / 4;
			if (xfer > count)
				xfer = count;
		} else {
			xfer = (count > 512) ? 512 : count;
		}

		count -= xfer;
		q_init(&t);

		/* setup before initial txn */
		q_ap_write(&t, AHB_CSW,
			AHB_CSW_MDEBUG | AHB_CSW_PRIV | AHB_CSW_INC_SINGLE |
			AHB_CSW_DBG_EN | AHB_CSW_32BIT);

		/* initial address */
		q_ap_write(&t, AHB_TAR, addr);

		t.tx[t.txc++] = SWD_WR(OP_AP | (AHB_DRW & 0xC), xfer);
		addr += xfer * 4;
		while (xfer-- > 0) 
			t.tx[t.txc++] = *in++;

		/* restore state after last batch */
		if (count == 0)
			q_ap_write(&t, AHB_CSW,
				AHB_CSW_MDEBUG | AHB_CSW_PRIV |
				AHB_CSW_DBG_EN | AHB_CSW_32BIT);

		if (q_exec(&t))
			return -1;
	}
	return 0;
}
#endif

int swdp_core_write(u32 n, u32 v) {
	struct txn t;
	q_init(&t);
	q_ahb_write(&t, CDBG_REG_DATA, v);
	q_ahb_write(&t, CDBG_REG_ADDR, (n & 0x1F) | 0x10000);
	return q_exec(&t);
}

int swdp_core_read(u32 n, u32 *v) {
	struct txn t;
	q_init(&t);
	q_ahb_write(&t, CDBG_REG_ADDR, n & 0x1F);
	q_ahb_read(&t, CDBG_REG_DATA, v);
	return q_exec(&t);
}

int swdp_core_read_all(u32 *v) {
	struct txn t;
	unsigned n;
	q_init(&t);
	for (n = 0; n < 19; n++) {
		q_ahb_write(&t, CDBG_REG_ADDR, n & 0x1F);
		q_ahb_read(&t, CDBG_REG_DATA, v++);
	}
	return q_exec(&t);
}

int swdp_core_halt(void) {
	return swdp_ahb_write(CDBG_CSR, CDBG_CSR_KEY | CDBG_C_HALT | CDBG_C_DEBUGEN);
}

int swdp_core_step(void) {
	return swdp_ahb_write(CDBG_CSR, CDBG_CSR_KEY | CDBG_C_STEP | CDBG_C_DEBUGEN);
}

int swdp_core_wait_for_halt(void) {
	int last = ATTN;
	u32 csr;
	for (;;) {
		if (swdp_ahb_read(CDBG_CSR, &csr))
			return -1;
		if (csr & CDBG_S_HALT)
			return 0;
		if (ATTN != last)
			return -2;
	}
}

int swdp_core_resume(void) {
	/* must leave DEBUGEN on to halt on vector catch, breakpoints, etc */
	return swdp_ahb_write(CDBG_CSR, CDBG_CSR_KEY | CDBG_C_DEBUGEN);
}

#define DWT_COMP(n) (0xE0001020 + (n) * 0x10)
#define DWT_MASK(n) (0xE0001024 + (n) * 0x10)
#define DWT_FUNC(n) (0xE0001028 + (n) * 0x10)

#define FUNC_DISABLED	0x0
#define FUNC_WATCH_PC	0x4
#define FUNC_WATCH_RD	0x5
#define FUNC_WATCH_WR	0x6
#define FUNC_WATCH_RW	0x7

int swdp_watchpoint(unsigned n, u32 addr, u32 func) {
	struct txn t;

	if (n > 3)
		return -1;

	q_init(&t);
	/* enable DWT, enable all exception traps */
	q_ahb_write(&t, CDBG_EMCR, 0x010007F1);
	q_ahb_write(&t, DWT_FUNC(n), FUNC_DISABLED);
	if (func != FUNC_DISABLED) {
		q_ahb_write(&t, DWT_COMP(n), addr);
		q_ahb_write(&t, DWT_MASK(n), 0);
		q_ahb_write(&t, DWT_FUNC(n), func);
	}
	return q_exec(&t);
}

int swdp_watchpoint_pc(unsigned n, u32 addr) {
	return swdp_watchpoint(n, addr, FUNC_WATCH_PC);
}

int swdp_watchpoint_rd(unsigned n, u32 addr) {
	return swdp_watchpoint(n, addr, FUNC_WATCH_RD);
}

int swdp_watchpoint_wr(unsigned n, u32 addr) {
	return swdp_watchpoint(n, addr, FUNC_WATCH_WR);
}

int swdp_watchpoint_rw(unsigned n, u32 addr) {
	return swdp_watchpoint(n, addr, FUNC_WATCH_RW);
}

int swdp_bootloader(void) {
	struct txn t;
	q_init(&t);
	t.tx[t.txc++] = RSWD_MSG(CMD_BOOTLOADER, 0, 0);
	return q_exec(&t);
}

int swdp_reset(void) {
	struct txn t;
	u32 n, idcode;

	q_init(&t);
	t.tx[t.txc++] = RSWD_MSG(CMD_ATTACH, 0, 0);
	t.tx[t.txc++] = SWD_RD(DP_IDCODE, 1);
	t.rx[t.rxc++] = &idcode;
	q_exec(&t);

	fprintf(stderr,"IDCODE: %08x\n", idcode);

	q_init(&t);
 	/* clear any stale errors */
	t.tx[t.txc++] = SWD_WR(DP_ABORT, 1);
	t.tx[t.txc++] = 0x1E;

	/* power up */
	t.tx[t.txc++] = SWD_WR(DP_DPCTRL, 1);
	t.tx[t.txc++] = (1 << 28) | (1 << 30);
	t.tx[t.txc++] = SWD_RD(DP_DPCTRL, 1);
	t.rx[t.rxc++] = &n;

	/* configure for 32bit IO */
	q_ap_write(&t, AHB_CSW,
		AHB_CSW_MDEBUG | AHB_CSW_PRIV |
		AHB_CSW_PRIV | AHB_CSW_DBG_EN | AHB_CSW_32BIT);
	if (q_exec(&t))
		return -1;

	fprintf(stderr,"DPCTRL: %08x\n", n);
	return 0;
}

void swdp_enable_tracing(int yes) {
	struct txn t;
	q_init(&t);
	t.tx[t.txc++] = RSWD_MSG(CMD_TRACE, yes, 0);
	q_exec(&t);
}

void swdp_target_reset(int enable) {
	struct txn t;
	q_init(&t);
	t.tx[t.txc++] = RSWD_MSG(CMD_RESET, 0, enable);
	q_exec(&t);
}

int swdp_set_clock(unsigned khz) {
	struct txn t;
	if (khz > 0xFFFF)
		return -1;
	if (khz < 1000)
		khz = 1000;
	q_init(&t);
	t.tx[t.txc++] = RSWD_MSG(CMD_SET_CLOCK, 0, khz);
	return q_exec(&t);
}
	
int swdp_open(void) {
	usb = usb_open(match_18d1_db03);
	if (usb == 0) {
		fprintf(stderr,"could not find device\n");
		return -1;
	}

	swdp_enable_tracing(0);
	swdp_reset();
	return 0;
}
