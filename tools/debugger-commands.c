/* debugger.c
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
#include <fcntl.h>
#include <sys/time.h>

/* TODO
 * - fault recovery (try dw 10000000 for example)
 */

#include <fw/types.h>
#include <protocol/rswdp.h>
#include "rswdp.h"

long long now() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return ((long long) tv.tv_usec) + ((long long) tv.tv_sec) * 1000000LL;
}

#define printf __use_xprintf_in_debugger_commands_c__

extern void xprintf(const char *fmt, ...);

extern int disassemble_thumb2(u32 addr, u16 op0, u16 op1,
		char *text, int len) __attribute__ ((weak));

int disassemble(u32 addr) {
	char text[128];
	int r;
	union {
		u32 w[2];
		u16 h[4];
	} mem;

	if (!disassemble_thumb2)
		return -1;

	if (addr & 2) {
		if (swdp_ahb_read32(addr & (~3), mem.w, 2))
			return -1;
		r = disassemble_thumb2(addr, mem.h[1], mem.h[2], text, 128);
	} else {
		if (swdp_ahb_read32(addr & (~3), mem.w, 1))
			return -1;
		r = disassemble_thumb2(addr, mem.h[0], mem.h[1], text, 128);
	}
	if (r > 0)
		xprintf("%s\n", text);
	return r;
}

typedef struct {
	const char *s;
	unsigned n;
} param;

struct cmd {
	const char *name;
	const char *args;
	void (*func)(int argc, param *argv);
	const char *help;
};

void do_exit(int argc, param *argv) {
	exit(0);
}

void do_attach(int argc, param *argv) {
	swdp_reset();
}

static u32 lastregs[19];

void do_regs(int argc, param *argv) {
	swdp_core_read_all(lastregs);

	xprintf("r0 %08x r4 %08x r8 %08x ip %08x psr %08x\n",
		lastregs[0], lastregs[4], lastregs[8],
		lastregs[12], lastregs[16]);
	xprintf("r1 %08x r5 %08x r9 %08x sp %08x msp %08x\n",
		lastregs[1], lastregs[5], lastregs[9],
		lastregs[13], lastregs[17]);
	xprintf("r2 %08x r6 %08x 10 %08x lr %08x psp %08x\n",
		lastregs[2], lastregs[6], lastregs[10],
		lastregs[14], lastregs[18]);
	xprintf("r3 %08x r7 %08x 11 %08x pc %08x\n",
		lastregs[3], lastregs[7], lastregs[11],
		lastregs[15]);
	disassemble(lastregs[15]);
}

void do_stop(int argc, param *argv) {
	swdp_core_halt();
	do_regs(0, 0);
}

void do_resume(int argc, param *argv) {
	swdp_core_resume();
}

void do_step(int argc, param *argv) {
	if (argc > 0) {
		u32 pc;
		do {
			swdp_core_step();
			if (swdp_core_read(15, &pc)) {
				xprintf("error\n");
				return;
			}
			xprintf(".");
			fflush(stdout);
		} while (pc != argv[0].n);
		xprintf("\n");
	} else {
		swdp_core_step();
	}
	do_regs(0, 0);
}

struct {
	const char *name;
	unsigned n;
} core_regmap[] = {
	{ "r0", 0 },
	{ "r1", 1 },
	{ "r2", 2 },
	{ "r3", 3 },
	{ "r4", 4 },
	{ "r5", 5 },
	{ "r6", 6 },
	{ "r7", 7 },
	{ "r8", 8 },
	{ "r9", 9 },
	{ "r10", 10 },
	{ "r11", 11 },
	{ "r12", 12 },
	{ "r13", 13 }, { "sp", 13 },
	{ "r14", 14 }, { "lr", 14 },
	{ "r15", 15 }, { "pc", 15 },
	{ "psr", 16 },
	{ "msp", 17 },
	{ "psp", 18 },
};

void do_dr(int argc, param *argv) {
	unsigned n;
	u32 x = 0xeeeeeeee;
	if (argc < 1)
		return;
	for (n = 0; n < (sizeof(core_regmap) / sizeof(core_regmap[0])); n++) {
		if (!strcasecmp(argv[0].s, core_regmap[n].name)) {
			swdp_core_read(core_regmap[n].n, &x);
			xprintf("%s: %08x\n", argv[0].s, x);
			return;
		}
	}
	swdp_ahb_read(argv[0].n, &x);
	xprintf("%08x: %08x\n", argv[0].n, x);
}

void do_wr(int argc, param *argv) {
	unsigned n;
	if (argc < 2)
		return;
	for (n = 0; n < (sizeof(core_regmap) / sizeof(core_regmap[0])); n++) {
		if (!strcasecmp(argv[0].s, core_regmap[n].name)) {
			swdp_core_write(core_regmap[n].n, argv[1].n);
			xprintf("%s<<%08x\n", argv[0].s, argv[1].n);
			return;
		}
	}
	swdp_ahb_write(argv[0].n, argv[1].n);
	xprintf("%08x<<%08x\n", argv[0].n, argv[1].n);
}

static u32 lastaddr = 0x20000000;
static u32 lastcount = 0x40;

void do_dw(int argc, param *argv) {
	u32 data[4096];
	u32 addr = lastaddr;
	u32 count = lastcount;
	unsigned n;

	if (argc > 0) addr = argv[0].n;
	if (argc > 1) count = argv[1].n;

	memset(data, 0xee, 256 *4);

	/* word align */
	addr = (addr + 3) & ~3;
	count = (count + 3) & ~3;
	if (count > sizeof(data))
		count = sizeof(data);

	lastaddr = addr + count;
	lastcount = count;

	count /= 4;
	swdp_ahb_read32(addr, data, count);
	for (n = 0; n < count; n++) {
		if ((n & 3) == 0)
			xprintf("\n%08x:", addr + (n << 2));
		xprintf(" %08x", data[n]);
	}
	xprintf("\n");
}


void do_db(int argc, param *argv) {
	u32 addr, count;
	u8 data[1024];
	unsigned n;
	if (argc < 2)
		return;

	addr = argv[0].n;
	count = argv[1].n;

	if (count > 1024)
		count = 1024;

	memset(data, 0xee, 1024);


	swdp_ahb_write(AHB_CSW, AHB_CSW_MDEBUG | AHB_CSW_PRIV |
		AHB_CSW_DBG_EN | AHB_CSW_8BIT);
	for (n = 0; n < count; n++) {
		u32 tmp;
		if (swdp_ahb_read(addr + n, &tmp)) {
			swdp_reset();
			break;
		}
		data[n] = tmp >> (8 * (n & 3));
	}

	swdp_ahb_write(AHB_CSW, AHB_CSW_MDEBUG | AHB_CSW_PRIV |
		AHB_CSW_DBG_EN | AHB_CSW_32BIT);

	for (n = 0; n < count; n++) {
		if ((n & 15) == 0)
			xprintf("\n%08x:", addr + n);
		xprintf(" %02x", data[n]);
	}
	xprintf("\n");
}

void do_download(int argc, param *argv) {
	u32 addr;
	u8 data[32768];
	u8 vrfy[32768];
	int fd, r;

	long long t0, t1;

	if (argc != 2) {
		xprintf("error: usage: download <file> <addr>\n");
		return;
	}

	fd = open(argv[0].s, O_RDONLY);
	r = read(fd, data, sizeof(data));
	if ((fd < 0) || (r < 0)) {
		xprintf("error: cannot read '%s'\n", argv[0].s);
		return;
	}
	r = (r + 3) & ~3;
	addr = argv[1].n;

	xprintf("sending %d bytes...\n", r);
	t0 = now();
	if (swdp_ahb_write32(addr, (void*) data, r / 4)) {
		xprintf("error: failed to write data\n");
		return;
	}
	t1 = now();
	xprintf("%lld uS -> %lld B/s\n", (t1 - t0), 
		(((long long)r) * 1000000LL) / (t1 - t0));

#if 0
	t0 = now();
	if (swdp_ahb_read32(addr, (void*) vrfy, r / 4)) {
		xprintf("error: verify read failed\n");
		return;
	}
	t1 = now();
	xprintf("%lld uS. %lld B/s.\n", (t1 - t0), 
		(((long long)r) * 1000000LL) / (t1 - t0));
	if (memcmp(data,vrfy,r)) {
		xprintf("error: verify failed\n");
		return;
	}
#endif
}

void do_reset(int argc, param *argv) {
	if (argc > 0) {
		swdp_target_reset(argv[0].n);
	} else {
		swdp_target_reset(1);
		usleep(10000);
		swdp_target_reset(0);
	}
}

void do_reset_stop(int argc, param *argv) {
	swdp_core_halt();
	swdp_ahb_write(CDBG_EMCR, 1);
	swdp_target_reset(1);
	usleep(10000);
	swdp_target_reset(0);
	usleep(10000);
	do_stop(0,0);
}

void do_watch_pc(int argc, param *argv) {
	if (argc < 1)
		return;
	swdp_watchpoint_pc(0, argv[0].n);
}

void do_watch_rw(int argc, param *argv) {
	if (argc < 1)
		return;
	swdp_watchpoint_rw(0, argv[0].n);
}

struct cmd CMD[] = {
	{ "exit",	"", do_exit,	"" },
	{ "attach",	"", do_attach,	"attach/reattach to sw-dp" },
	{ "regs",	"", do_regs,	"show cpu registers" },
	{ "stop",	"", do_stop,	"halt cpu" },
	{ "step",	"", do_step,	"single-step cpu" },
	{ "go",		"", do_resume,	"resume cpu" },
	{ "dw",		"", do_dw,	"dump words" },
	{ "db",		"", do_db,	"dump bytes" },
	{ "dr",		"", do_dr,	"dump register" },
	{ "wr",		"", do_wr,	"write register" },
	{ "download",	"", do_download,"download file to device" },
	{ "reset",	"", do_reset,	"reset target" },
	{ "reset-stop",	"", do_reset_stop, "reset target and halt cpu" },
	{ "watch-pc",	"", do_watch_pc, "set watchpoint at addr" },
	{ "watch-rw",	"", do_watch_rw, "set watchpoint at addr" },
};

void debugger_command(char *line) {
	param arg[32];
	unsigned c, n = 0;

	while ((c = *line)) {
		if (c <= ' ') {
			line++;
			continue;
		}
		arg[n].s = line;
		for (;;) {
			if (c == 0) {
				n++;
				break;
			} else if (c <= ' ') {
				*line++ = 0;
				n++;
				break;
			}
			c = *++line;
		}
	}

	if (n == 0)
		return;

	for (c = 0; c < n; c++)
		arg[c].n = strtoul(arg[c].s, 0, 16);

	for (c = 0; c < (sizeof(CMD) / sizeof(CMD)[0]); c++) {
		if (!strcasecmp(arg[0].s, CMD[c].name)) {
			CMD[c].func(n - 1, arg + 1);
			return;
		}
	}
	xprintf("unknown command: %s\n", arg[0].s);
}

