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

#include "usb.h"

/* TODO
 * - fault recovery (try dw 10000000 for example)
 */

#include <fw/types.h>
#include <protocol/rswdp.h>
#include "rswdp.h"

#include "linenoise.h"

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
	unsigned n;
	for (n = 0; n < 19; n++)
		swdp_core_read(n, lastregs + n);

	printf("r0 %08x r4 %08x r8 %08x ip %08x psr %08x\n",
		lastregs[0], lastregs[4], lastregs[8],
		lastregs[12], lastregs[16]);
	printf("r1 %08x r5 %08x r9 %08x sp %08x msp %08x\n",
		lastregs[1], lastregs[5], lastregs[9],
		lastregs[13], lastregs[17]);
	printf("r2 %08x r6 %08x 10 %08x lr %08x psp %08x\n",
		lastregs[2], lastregs[6], lastregs[10],
		lastregs[14], lastregs[18]);
	printf("r3 %08x r7 %08x 11 %08x pc %08x\n",
		lastregs[3], lastregs[7], lastregs[11],
		lastregs[15]);
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
				printf("error\n");
				return;
			}
			printf(".");
			fflush(stdout);
		} while (pc != argv[0].n);
		printf("\n");
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
			printf("%s: %08x\n", argv[0].s, x);
			return;
		}
	}
	swdp_ahb_read(argv[0].n, &x);
	printf("%08x: %08x\n", argv[0].n, x);
}

void do_wr(int argc, param *argv) {
	unsigned n;
	if (argc < 2)
		return;
	for (n = 0; n < (sizeof(core_regmap) / sizeof(core_regmap[0])); n++) {
		if (!strcasecmp(argv[0].s, core_regmap[n].name)) {
			swdp_core_write(core_regmap[n].n, argv[1].n);
			printf("%s<<%08x\n", argv[0].s, argv[1].n);
			return;
		}
	}
	swdp_ahb_write(argv[0].n, argv[1].n);
	printf("%08x<<%08x\n", argv[0].n, argv[1].n);
}

static u32 lastaddr = 0x20000000;
static u32 lastcount = 0x40;

void do_dw(int argc, param *argv) {
	u32 data[256];
	u32 addr = lastaddr;
	u32 count = lastcount;
	unsigned n;

	if (argc > 0) addr = argv[0].n;
	if (argc > 1) count = argv[1].n;

	memset(data, 0xee, 256 *4);

	/* word align */
	addr = (addr + 3) & ~3;
	count = (count + 3) & ~3;
	if (count > 1024)
		count = 1024;

	lastaddr = addr + count;
	lastcount = count;

	count /= 4;
	for (n = 0; n < count; n++)
		swdp_ahb_read(addr + (n << 2), data + n);

	for (n = 0; n < count; n++) {
		if ((n & 3) == 0)
			printf("\n%08x:", addr + (n << 2));
		printf(" %08x", data[n]);
	}
	printf("\n");
}

void do_download(int argc, param *argv) {
	u32 addr;
	u8 data[32768];
	u32 *w;
	int fd, r;

	if (argc != 2) {
		printf("error: usage: download <file> <addr>\n");
		return;
	}

	fd = open(argv[0].s, O_RDONLY);
	r = read(fd, data, sizeof(data));
	if ((fd < 0) || (r < 0)) {
		printf("error: cannot read '%s'\n", argv[0].s);
		return;
	}
	r = (r + 3) & ~3;

	printf("sending %d bytes...\n", r);
	w = (void*) data;
	addr = argv[1].n;
	while (r > 0) {
		u32 tmp = 0xeeeeeeee;
		if (swdp_ahb_write(addr, *w)) {
			printf("error: failed to write @ %08x\n", addr);
			return;
		}
		if (swdp_ahb_read(addr, &tmp)) {
			printf("oops\n");
			return;
		}
		if (tmp != *w) {
			printf("ACK %08x %08x @ %08x\n", tmp, *w, addr);
			return;
		}
		r++;
		w++;
		addr += 4;
		r -= 4;
	} 
}

struct cmd CMD[] = {
	{ "exit",	"", do_exit,	"" },
	{ "attach",	"", do_attach,	"attach/reattach to sw-dp" },
	{ "regs",	"", do_regs,	"show cpu registers" },
	{ "stop",	"", do_stop,	"halt cpu" },
	{ "step",	"", do_step,	"single-step cpu" },
	{ "go",		"", do_resume,	"resume cpu" },
	{ "dw",		"", do_dw,	"dump words" },
	{ "dr",		"", do_dr,	"dump register" },
	{ "wr",		"", do_wr,	"write register" },
	{ "download",	"", do_download,"download file to device" },
};

void execute(char *line) {
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
	printf("unknown command: %s\n", arg[0].s);
}

int main(int argc, char **argv) {
	char *line;

	if (swdp_open()) {
		fprintf(stderr,"could not find device\n");
		return -1;
	}

	while ((line = linenoise("debugger> ")) != NULL) {
		if (line[0] == 0)
			continue;
		linenoiseHistoryAdd(line);
		execute(line);
	}
	return 0;
}
