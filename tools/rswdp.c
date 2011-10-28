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

int match_18d1_6502(usb_ifc_info *ifc) {
	if (ifc->dev_vendor != 0x18d1)
		return -1;
	if (ifc->dev_product != 0x6502)
		return -1;
	return 0;
}

static u8 sequence = 1;

static usb_handle *usb;

int invoke(struct msg *msg, unsigned kind) {
	unsigned seq = sequence++;
	int r;

	msg->msg = kind;
	msg->seq = seq;

	r = usb_write(usb, msg, sizeof(struct msg));
	if (r != sizeof(struct msg)) {
		fprintf(stderr,"invoke: tx error\n");
		return -1;
	}
	for (;;) {
		r = usb_read(usb, msg, sizeof(struct msg));
		if (r != 64) {
			fprintf(stderr,"invoke: rx error\n");
			return -1;
		}
		switch (msg->msg) {
		case MSG_OKAY:
			if (msg->seq == seq)
				return 0;
			continue;
		case MSG_FAIL:
			if (msg->seq == seq)
				return -1;
			continue;
		case MSG_DEBUG: {
			char *s = (void*) msg;
			s[63] = 0;
			r = write(2, s + 4, strlen(s + 4));
			continue;
		}
		default:
			fprintf(stderr,"error: unknown msg 0x%02x\n", msg->msg);
		}
	}
}

int swdp_ap_write(u32 addr, u32 value) {
	struct msg msg;
	msg.op[0] = OP_WR | DP_SELECT;
	msg.data[0] = addr & 0xF0;
	msg.op[1] = OP_WR | OP_AP | (addr & 0xC);
	msg.data[1] = value;
	msg.pr0 = 2;
	return invoke(&msg, MSG_SWDP_OPS);
}

int swdp_ap_read(u32 addr, u32 *value) {
	struct msg msg;
	int r;
	msg.op[0] = OP_WR | DP_SELECT;
	msg.data[0] = addr & 0xF0;
	msg.op[1] = OP_RD | OP_AP | (addr & 0xC);
	msg.data[1] = 1;
	msg.op[2] = OP_RD | DP_BUFFER;
	msg.data[2] = 1;
	msg.pr0 = 3;
	r = invoke(&msg, MSG_SWDP_OPS);
	*value = msg.data[2];
	return r;
}

int swdp_ahb_read(u32 addr, u32 *value) {
	if (swdp_ap_write(AHB_TAR, addr))
		return -1;
	if (swdp_ap_read(AHB_DRW, value))
		return -1;
	return 0;
}

int swdp_ahb_write(u32 addr, u32 value) {
	if (swdp_ap_write(AHB_TAR, addr))
		return -1;
	if (swdp_ap_write(AHB_DRW, value))
		return -1;
	return 0;
}

int swdp_core_write(u32 n, u32 v) {
	if (swdp_ahb_write(CDBG_REG_DATA, v))
		return -1;
	if (swdp_ahb_write(CDBG_REG_ADDR, (n & 0x1F) | 0x10000))
		return -1;
	return 0;
}

int swdp_core_read(u32 n, u32 *v) {
	if (swdp_ahb_write(CDBG_REG_ADDR, n & 0x1F))
		return -1;
	return swdp_ahb_read(CDBG_REG_DATA, v);
}

int swdp_core_halt(void) {
	u32 n;
	if (swdp_ahb_read(CDBG_CSR, &n))
		return -1;
	if (swdp_ahb_write(CDBG_CSR, 
		CDBG_CSR_KEY | CDBG_C_HALT | CDBG_C_DEBUGEN))
		return -1;
	return 0;
}

int swdp_core_step(void) {
	u32 n;
	if (swdp_ahb_read(CDBG_CSR, &n))
		return -1;
	if (swdp_ahb_write(CDBG_CSR, 
		CDBG_CSR_KEY | CDBG_C_STEP | CDBG_C_DEBUGEN))
		return -1;
	return 0;
}

int swdp_core_resume(void) {
	u32 n;
	if (swdp_ahb_read(CDBG_CSR, &n))
		return -1;
	if (swdp_ahb_write(CDBG_CSR,
		CDBG_CSR_KEY | (n & 0x3C)))
		return -1;
	return 0;
}

int swdp_reset(void) {
	struct msg msg;
	u32 n;

	invoke(&msg, MSG_ATTACH);
	fprintf(stderr,"IDCODE: %08x\n", msg.data[0]);

 	/* clear any stale errors */
	msg.op[0] = OP_WR | DP_ABORT;
	msg.data[0] = 0x1E;

	/* power up */
	msg.op[1] = OP_WR | DP_DPCTRL;
	msg.data[1] = (1 << 28) | (1 << 30);
	msg.op[2] = OP_RD | DP_DPCTRL;
	msg.data[2] = 1;
	msg.pr0 = 3;
	if (invoke(&msg, MSG_SWDP_OPS))
		return -1;

	fprintf(stderr,"DPCTRL: %08x\n", msg.data[2]);


	/* configure for 32bit IO */
	swdp_ap_write(AHB_CSW, AHB_CSW_MDEBUG | AHB_CSW_PRIV |
		AHB_CSW_PRIV | AHB_CSW_DBG_EN | AHB_CSW_32BIT);

	swdp_ap_read(AHB_ROM_ADDR, &n);
	fprintf(stderr,"ROMADDR: %08x\n", n);
	return 0;
}

void swdp_enable_tracing(int yes) {
	struct msg msg;
	msg.pr0 = yes;
	invoke(&msg, MSG_TRACE);
}

int swdp_open(void) {
	usb = usb_open(match_18d1_6502);
	if (usb == 0) {
		fprintf(stderr,"could not find device\n");
		return -1;
	}

	swdp_enable_tracing(0);

	return swdp_reset();
}
