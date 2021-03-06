/* gdb-bridge.c
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
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>

#include <fw/types.h>
#include "rswdp.h"
#include <protocol/rswdp.h>
#include "debugger.h"
#include "lkdebug.h"

// useful gdb stuff
// set debug remote 1               protocol tracing
// set debug arch 1                 architecture tracing
// maint print registers-remote     check on register map

#define MAXPKT		8192

#define S_IDLE		0
#define S_RECV		1
#define S_CHK1		2
#define S_CHK2		3

#define F_ACK		0x01
#define F_RUNNING	0x04
#define F_CONSOLE	0x08
#define F_LK_THREADS	0x10

struct gdbcnxn {
	int fd;
	unsigned state;
	unsigned txsum;
	unsigned rxsum;
	unsigned flags;
	unsigned char *txptr;
	unsigned char *rxptr;
	unsigned char rxbuf[MAXPKT];
	unsigned char txbuf[MAXPKT];
	char chk[4];
	lkthread_t *threadlist;
	lkthread_t *gselected;
	lkthread_t *cselected;
};

void gdb_init(struct gdbcnxn *gc, int fd) {
	gc->fd = fd;
	gc->state = S_IDLE;
	gc->txsum = 0;
	gc->rxsum = 0;
	gc->flags = F_ACK;
	gc->txptr = gc->txbuf;
	gc->rxptr = gc->rxbuf;
	gc->chk[2] = 0;
	gc->threadlist = NULL;
}

static inline int rx_full(struct gdbcnxn *gc) {
	return (gc->rxptr - gc->rxbuf) == (MAXPKT - 1);
}

static inline int tx_full(struct gdbcnxn *gc) {
	return (gc->txptr - gc->txbuf) == (MAXPKT - 1);
}

static inline void gdb_putc(struct gdbcnxn *gc, unsigned n) {
	unsigned char c = n;
	if (!tx_full(gc)) {
		gc->txsum += c;
		*(gc->txptr++) = c;
	}
}

void gdb_puts(struct gdbcnxn *gc, const char *s) {
	while (*s) {
		gdb_putc(gc, *s++);
	}
}

void gdb_prologue(struct gdbcnxn *gc) {
	gc->txptr = gc->txbuf;
	*(gc->txptr++) = '$';
	gc->txsum = 0;
}

void gdb_epilogue(struct gdbcnxn *gc) {
	unsigned char *ptr;
	int len, r;
	char tmp[4];
	sprintf(tmp,"#%02x", gc->txsum & 0xff);
	gdb_puts(gc, tmp);

	if (tx_full(gc)) {
		xprintf(XGDB, "gdb: TX Packet Too Large\n");
		return;
	}

	ptr = gc->txbuf;
	len = gc->txptr - gc->txbuf;
	while (len > 0) {
		r = write(gc->fd, ptr, len);
		if (r <= 0) {
			if (errno == EINTR) continue;
			xprintf(XGDB, "gdb: TX Write Error\n");
			return;
		}
		ptr += r;
		len -= r;
	}
}

static char HEX[16] = "0123456789abcdef";
void gdb_puthex(struct gdbcnxn *gc, const void *ptr, unsigned len) {
	const unsigned char *data = ptr;
	while (len-- > 0) {
		unsigned c = *data++;
		gdb_putc(gc, HEX[c >> 4]);
		gdb_putc(gc, HEX[c & 15]);
	}
}

void handle_command(struct gdbcnxn *gc, unsigned char *cmd);

void gdb_recv_cmd(struct gdbcnxn *gc) {
	if (log_flags & LF_GDB) {
		xprintf(XGDB, "gdb: pkt: %s\n", gc->rxbuf);
	}
	debugger_lock();
	handle_command(gc, gc->rxbuf);
	debugger_unlock();
}

int gdb_recv(struct gdbcnxn *gc, unsigned char *ptr, int len) {
	unsigned char *start = ptr;
	unsigned char c;

	while (len > 0) {
		c = *ptr++;
		len--;
		switch (gc->state) {
		case S_IDLE:
			if (c == 3) {
				gc->rxbuf[0] = '$';
				gc->rxbuf[1] = 0;
				gdb_recv_cmd(gc);
			} else if (c == '$') {
				gc->state = S_RECV;
				gc->rxsum = 0;
				gc->rxptr = gc->rxbuf;
			}
			break;
		case S_RECV:
			if (c == '#') {
				gc->state = S_CHK1;
			} else {
				if (rx_full(gc)) {
					xprintf(XGDB, "gdb: pkt: Too Large, Discarding.");
					gc->rxptr = gc->rxbuf;
					gc->state = S_IDLE;
				} else {
					*(gc->rxptr++) = c;
					gc->rxsum += c;
				}
			}
			break;
		case S_CHK1:
			gc->chk[0] = c;
			gc->state = S_CHK2;
			break;
		case S_CHK2:
			gc->chk[1] = c;
			gc->state = S_IDLE;
			*(gc->rxptr++) = 0;
			if (strtoul(gc->chk, NULL, 16) == (gc->rxsum & 0xFF)) {
				if (gc->flags & F_ACK) {
					if (write(gc->fd, "+", 1)) ;
				}
				gdb_recv_cmd(gc);
			} else {
				if (gc->flags & F_ACK) {
					if (write(gc->fd, "-", 1)) ;
				}
			}
			break;
		}
	}
	return ptr - start;
}

unsigned unhex(char *x) {
	char tmp[3];
	unsigned n;
	tmp[0] = x[0];
	tmp[1] = x[1];
	tmp[2] = 0;
	n = strtoul(tmp, 0, 16);
 	return n;
}

int hextobin(void *_dst, char *src, int max) {
	unsigned char *dst = _dst;
	while (max > 0) {
		if (src[0] && src[1]) {
			*dst++ = unhex(src);
			src += 2;
			max -= 2;
		} else {
			break;
		}
	}
	return dst - ((unsigned char*) _dst);
}

static const char *target_xml =
"<?xml version=\"1.0\"?>"
"<target>"
"<architecture>arm</architecture>"
"<feature name=\"org.gnu.gdb.arm.m-profile\">"
"<reg name=\"r0\" bitsize=\"32\"/>"
"<reg name=\"r1\" bitsize=\"32\"/>"
"<reg name=\"r2\" bitsize=\"32\"/>"
"<reg name=\"r3\" bitsize=\"32\"/>"
"<reg name=\"r4\" bitsize=\"32\"/>"
"<reg name=\"r5\" bitsize=\"32\"/>"
"<reg name=\"r6\" bitsize=\"32\"/>"
"<reg name=\"r7\" bitsize=\"32\"/>"
"<reg name=\"r8\" bitsize=\"32\"/>"
"<reg name=\"r9\" bitsize=\"32\"/>"
"<reg name=\"r10\" bitsize=\"32\"/>"
"<reg name=\"r11\" bitsize=\"32\"/>"
"<reg name=\"r12\" bitsize=\"32\"/>"
"<reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>"
"<reg name=\"lr\" bitsize=\"32\"/>"
"<reg name=\"pc\" bitsize=\"32\" type=\"code_ptr\"/>"
"<reg name=\"xpsr\" bitsize=\"32\"/>"
"<reg name=\"msp\" bitsize=\"32\" type=\"data_ptr\"/>"
"<reg name=\"psp\" bitsize=\"32\" type=\"data_ptr\"/>"
"</feature>"
"</target>";

static void handle_query(struct gdbcnxn *gc, char *cmd, char *args) {
	if (!strcmp(cmd, "fThreadInfo")) {
		if (gc->threadlist) {
			char tmp[32];
			lkthread_t *t = gc->threadlist;
			sprintf(tmp, "m%x", t->threadptr);
			gdb_puts(gc, tmp);
			for (t = t->next; t != NULL; t = t->next) {
				sprintf(tmp, ",%x",t->threadptr);
				gdb_puts(gc, tmp);
			}
		} else {
			/* report just one thread id, #1, for now */
			gdb_puts(gc, "m1");
		}
	} else if(!strcmp(cmd, "sThreadInfo")) {
		/* no additional thread ids */
		gdb_puts(gc, "l");
	} else if(!strcmp(cmd, "ThreadExtraInfo")) {
		u32 n = strtoul(args, NULL, 16);
		lkthread_t *t = find_lk_thread(gc->threadlist, n);
		/* gdb manual suggest 'Runnable', 'Blocked on Mutex', etc */
		/* informational text shown in gdb's "info threads" listing */
		if (t) {
			char tmp[128];
			get_lk_thread_name(t, tmp, sizeof(tmp));
			gdb_puthex(gc, tmp, strlen(tmp));
		} else {
			gdb_puthex(gc, "Native", 6);
		}
	} else if(!strcmp(cmd, "C")) {
		/* current thread ID */
		if (gc->cselected) {
			char tmp[32];
			sprintf(tmp, "QC%x", gc->cselected->threadptr);
			gdb_puts(gc, tmp);
		} else {
			gdb_puts(gc, "QC1");
		}
	} else if (!strcmp(cmd, "Rcmd")) {
		char *p = args;
		cmd = p;
		while (p[0] && p[1]) {
			*cmd++ = unhex(p);
			p+=2;
		}
		*cmd = 0;
		xprintf(XGDB, "gdb: %s\n", args);
		gc->flags |= F_CONSOLE;
		debugger_unlock();
		debugger_command(args);
		debugger_lock();
		gc->flags &= ~F_CONSOLE;
		gdb_prologue(gc);
		gdb_puts(gc, "OK");
	} else if(!strcmp(cmd, "Supported")) {
		gdb_puts(gc,
			"qXfer:features:read+"
			";QStartNoAckMode+"
			";PacketSize=2004" /* size includes "$" and "#xx" */
			);
	} else if(!strcmp(cmd, "Xfer")) {
		if (!strncmp(args, "features:read:target.xml:", 25)) {
			gdb_puts(gc,"l");
			// todo: support binary format w/ escaping
			gdb_puts(gc, target_xml);
		}
	} else if(!strcmp(cmd, "TStatus")) {
		/* tracepoints unsupported. ignore. */
	} else if(!strcmp(cmd, "Attached")) {
		/* no process management. ignore */
	} else {
		xprintf(XGDB, "gdb: unsupported: q%s:%s\n", cmd, args);
	}
}

static void handle_set(struct gdbcnxn *gc, char *cmd, char *args) {
	if(!strcmp(cmd, "StartNoAckMode")) {
		gc->flags &= ~F_ACK;
		gdb_puts(gc, "OK");
	} else {
		xprintf(XGDB, "gdb: unsupported: Q%s:%s\n", cmd, args);
	}
}

// Since we're only guaranteed 32bit reads and writes by the MEM-AP
// convert non-aligned writes into a read-modify-write followed by
// an aligned multiword write, followed by a read-modify-write.
// (as necessary)
void write_memory(u32 addr, unsigned char *data, int len) {
	u32 x;

	if (len < 1) {
		return;
	}

	if (addr & 3) {
		int offset = (addr & 3);
		int count = 4 - offset;
		if (count > len) {
			count = len;
		}
		swdp_ahb_read(addr & (~3), &x);
		memcpy(((char*) &x) + offset, data, count);
		swdp_ahb_write(addr & (~3), x);
		len -= count;
		data += count;
		addr += count;
	}

	if (len > 3) {
		int count = (4 * (len / 4));
		swdp_ahb_write32(addr, (void*) data, len / 4);
		len -= count;
		data += count;
		addr += count;
	}

	if (len > 0) {
		swdp_ahb_read(addr, &x);
		memcpy(&x, data, len);
		swdp_ahb_write(addr, x);
	}
}

// bit 1 in each romtable entry indicates peripheral is present
#define ROMTAB_DWT	0xE00FF004
#define ROMTAB_FPB	0xE00FF008
#define ROMTAB_ITM	0xE00FF00C
#define ROMTAB_TPIU	0xE00FF010
#define ROMTAB_ETM	0xE00FF014

#define DWT_CTRL	0xE0001000

#define FP_CTRL		0xE0002000
#define FP_REMAP	0xE0002004
#define FP_COMP(n)	(0xE0002008 + ((n) * 4))

#define MAXFP 16
static u32 maxfp = 0;
static u32 fp_addr[MAXFP] = { 0, };
static u32 fp_state[MAXFP] = { 0, };

#define MAXBP 16
static u32 maxbp;
static u32 bp_addr[MAXBP] = { 0, };
static u32 bp_state[MAXBP] = { 0, };

int handle_flashpatch(int add, u32 addr, u32 kind) {
	u32 x;
	int n;
	if (swdp_ahb_read(ROMTAB_FPB, &x)) {
		xprintf(XGDB, "gdb: cannot read romtable\n");
		return -1;
	}
	if (x & 1) {
		if (swdp_ahb_read(FP_CTRL, &x)) {
			xprintf(XGDB, "gdb: cannot read flashpatch ctrl\n");
			return -1;
		}
		n = ((x & 0xF0) >> 4) | ((x & 0x7000) >> 4);
	} else {
		n = 0;
	}
	if (n != maxfp) {
		xprintf(XGDB, "gdb: %d flashpatch breakpoint registers\n", n);
		if (n > 16) n = 16;
		if (maxfp != 0) {
			xprintf(XGDB, "gdb: previously %d registers...\n", maxfp);
		}
		maxfp = n;
	}
	for (n = 0; n < maxfp; n++) {
		if (fp_state[n] && (fp_addr[n] == addr)) {
			if (add) {
				goto add0;
			} else {
				fp_state[n] = 0;
				swdp_ahb_write(FP_COMP(n), 0);
				xprintf(XGDB, "gdb: - FP BP @ %08x\n", addr);
				return 0;
			}
		}
	}
	if (!add) {
		// no breakpoint to remove
		return -1;
	}
	for (n = 0; n < maxfp; n++) {
		if (fp_state[n] == 0) {
			goto add1;
		}
	}
	return -1;
add1:
	swdp_ahb_write(FP_CTRL,3);
	if (addr & 2) {
		// breakpoint on low half-word, enable
		swdp_ahb_write(FP_COMP(n), 0x80000001 | (addr & 0x1FFFFFFC));
	} else {
		// breakpoint on high half-word, enable
		swdp_ahb_write(FP_COMP(n), 0x40000001 | (addr & 0x1FFFFFFC));
	}
	fp_state[n] = 1;
	fp_addr[n] = addr;
add0:
	xprintf(XGDB, "gdb: + FP BP @ %08x\n", addr);
	return 0;
}

int handle_breakpoint(int add, u32 addr, u32 kind) {
	u32 x;
	int n;

	if ((addr < 0x20000000) && (!handle_flashpatch(add,addr,kind))) {
		return 0;
	}
	if (swdp_ahb_read(ROMTAB_DWT, &x)) {
		xprintf(XGDB, "gdb: cannot read romtable\n");
		return -1;
	}
	if (x & 1) {
		if (swdp_ahb_read(DWT_CTRL, &x)) {
			xprintf(XGDB, "gdb: cannot read dwt ctrl\n");
			return -1;
		}
		n = x >> 28;
	} else {
		n = 0;
	}
	if (n != maxbp) {
		xprintf(XGDB, "gdb: %d dwt breakpoint registers\n", n);
		if (maxbp != 0) {
			xprintf(XGDB, "gdb: previously %d registers...\n", maxbp);
		}
		maxbp = n;
	}

	if (add) {
		for (n = 0; n < maxbp; n++) {
			if (bp_state[n] && (bp_addr[n] == addr)) {
				// already setup
				return 0;
			}
		}
		for (n = 0; n < maxbp; n++) {
			if (bp_state[n] == 0) {
				bp_addr[n] = addr;
				bp_state[n] = 1;
				swdp_watchpoint_pc(n, addr);
				xprintf(XGDB, "gdb: + HW BP @ %08x\n", addr);
				return 0;
			}
		}
		if (n == maxbp) {
			xprintf(XGDB, "gdb: Out of hardware breakpoints.\n");
			return 1;
		}
		return 0;
	} else {
		for (n = 0; n < maxbp; n++) {
			if (bp_state[n] && (bp_addr[n] == addr)) {
				bp_state[n] = 0;
				swdp_watchpoint_disable(n);
				break;
			}
		}
		xprintf(XGDB, "gdb: - HW BP @ %08x\n", addr);
		return 0;
	}
}

void gdb_update_threads(struct gdbcnxn *gc) {
	xprintf(XGDB, "gdb: sync threadlist\n");
	free_lk_threads(gc->threadlist);
	if (gc->flags & F_LK_THREADS) {
		if ((gc->threadlist = find_lk_threads(0)) == NULL) {
			xprintf(XGDB, "gdb: problem syncing threadlist\n");
		}
		gc->cselected = gc->threadlist;
		gc->gselected = gc->threadlist;
	} else {
		gc->threadlist = NULL;
		gc->cselected = NULL;
		gc->gselected = NULL;
	}
}

void handle_command(struct gdbcnxn *gc, unsigned char *cmd) {
	union {
		u32 w[256+2];
		u16 h[512+4];
		u8 b[1024+8];
	} tmp;
	unsigned n,x;

	/* silent (no-response) commands */
	switch (cmd[0]) {
	case 'c':
		if (cmd[1]) {
			x = strtoul((char*) cmd + 1, NULL, 16) | 1;
			swdp_core_write(15, x);
		}
		swdp_core_resume();
		gc->flags |= F_RUNNING;
		return;
	// single step
	case 's':
		if (cmd[1]) {
			x = strtoul((char*) cmd + 1, NULL, 16) | 1;
			swdp_core_write(15, x);
		}
		swdp_core_step();
		gc->flags |= F_RUNNING;
		return;
	}

	gdb_prologue(gc);
	switch (cmd[0]) {
	case '?':
		gdb_puts(gc, "S05");
		gc->flags &= (~F_RUNNING);
		swdp_core_halt();
		gdb_update_threads(gc);
		break;
	case 'H': {
		// select thread
		lkthread_t *t = NULL;
		if ((cmd[2] == '-') && (cmd[3] == '1')) {
			t = gc->threadlist;
		} else {
			n = strtoul((char*) cmd + 2, NULL, 16);
			if ((t = find_lk_thread(gc->threadlist, n)) == NULL) {
				t = gc->threadlist;
			}
		}
		if (cmd[1] == 'g') {
			gc->gselected = t;
		} else if (cmd[1] == 'c') {
			gc->cselected = t;
		} else {
			xprintf(XGDB, "gdb: selecting '%c' thread?!\n", cmd[1]);
		}
		gdb_puts(gc, "OK");
		break;
	}
	// is thread alive?
	case 'T':
		n = strtoul((char*) cmd + 1, NULL, 16);
		if (find_lk_thread(gc->threadlist, n)) {
			gdb_puts(gc, "OK");
		} else {
			gdb_puts(gc, "E00");
		}
		break;
	// m hexaddr , hexcount
	// read from memory
	case 'm':
		if (sscanf((char*) cmd + 1, "%x,%x", &x, &n) != 2) {
			break;
		}
		if (n > 1024) {
			n = 1024;
		}
		swdp_ahb_read32(x & (~3), tmp.w, ((n + 3) & (~3)) / 4);
		gdb_puthex(gc, tmp.b + (x & 3), n);
		break;
	// M hexaddr , hexcount : hexbytes
	// write to memory
	case 'M': {
		char *data = strchr((char*) cmd + 1, ':');
		int len;
		if (!data) {
			break;
		}
		*data++ = 0;
		if (sscanf((char*) cmd + 1, "%x,%x", &x, &n) != 2) {
			break;
		}
		len = hextobin(gc->rxbuf, data, MAXPKT);
		write_memory(x, gc->rxbuf, len);
		gdb_puts(gc, "OK");
		break;
		}
	// g
	// read registers 0...
	case 'g':  {
		u32 regs[19];
		if (gc->gselected && !gc->gselected->active) {
			memset(regs, 0, sizeof(regs));
			memcpy(regs, gc->gselected->regs, sizeof(gc->gselected->regs)); 
		} else {
			swdp_core_read_all(regs);
		}
		gdb_puthex(gc, regs, sizeof(regs));
		break;
	}
	// G hexbytes
	// write registers 0...
	case 'G': {
		if (gc->gselected && !gc->gselected->active) {
			xprintf(XGDB, "gdb: attempting to write to inactive registers\n");
			break;
		}
		int len = hextobin(gc->rxbuf, (char*) cmd + 1, MAXPKT);
		for (n = 0; n < len / 4; n++) {
			memcpy(&x, gc->rxbuf + (n * 4), sizeof(x));
			swdp_core_write(n, x);
		}
		gdb_puts(gc, "OK");
		break;
	}
	// p reghex
	// read from register
	case 'p': {
		u32 v;
		u32 n = strtoul((char*) cmd + 1, NULL, 16);
		if (gc->gselected && !gc->gselected->active) {
			if (n > 16) {
				v = 0xeeeeeeee;
			} else {
				v = gc->gselected->regs[n];
			}
		} else {
			swdp_core_read(n, &v);
		}
		gdb_puthex(gc, &v, sizeof(v));
		break;
	}
	// P reghex = hexbytes
	// write to register
	case 'P': {
		int len;
		char *data = strchr((char*) cmd + 1, '=');
		if (gc->gselected && !gc->gselected->active) {
			xprintf(XGDB, "gdb: attempting to write to inactive registers\n");
			break;
		}
		if (data) {
			*data++ = 0;
			n = strtoul((char*) cmd + 1, NULL, 16);
			len = hextobin(gc->rxbuf, data, MAXPKT);
			if (len != 4) break;
			memcpy(&x, gc->rxbuf, sizeof(x));
			swdp_core_write(n, x);
			gdb_puts(gc, "OK");
		}
		break;
	}
	// halt (^c)
	case '$':
		swdp_core_halt();
		gdb_update_threads(gc);
		gc->flags &= (~F_RUNNING);
		gdb_puts(gc, "S05");
		break;
	// extended query and set commands
	case 'q': 
	case 'Q': {
		char *args = (char*) (cmd + 1);
		while (*args) {
			if ((*args == ':') || (*args == ',')) {
				*args++ = 0;
				break;
			}
			args++;
		}
		if (cmd[0] == 'q') {
			handle_query(gc, (char*) (cmd + 1), args);
		} else {
			handle_set(gc, (char*) (cmd + 1), args);
		}
		break;
	}
	case 'z':
	case 'Z': {
		u32 type, addr, kind;
		if (sscanf((char*) cmd + 1, "%x,%x,%x", &type, &addr, &kind) != 3) {
			break;
		}
		if (type != 1) {
			// only support hw breakpoints
		}
		if (handle_breakpoint(cmd[0] == 'Z', addr, kind)) {
			gdb_puts(gc, "E1");
		} else {
			gdb_puts(gc, "OK");
		}
		break;
	}
	default:
		xprintf(XGDB, "gdb: unknown command: %c\n", cmd[0]);
	}
	gdb_epilogue(gc);
}

static int pipefds[2] = { -1, -1 };

void signal_gdb_server(void) {
	if (pipefds[1] >= 0) {
		char x = 0;
		if (write(pipefds[1], &x, 1) < 0) ;
	}
}

static struct gdbcnxn *active_gc = NULL;

void gdb_console_puts(const char *msg) {
	if (active_gc == NULL) return;
	if (!(active_gc->flags & F_CONSOLE)) return;
	gdb_prologue(active_gc);
	gdb_putc(active_gc, 'O');
	gdb_puthex(active_gc, msg, strlen(msg));
	gdb_epilogue(active_gc);
}

void gdb_server(int fd) {
	struct pollfd fds[2];
	struct gdbcnxn gc;
	unsigned char iobuf[32768];
	unsigned char *ptr;
	int r, len;

	gdb_init(&gc, fd);

	debugger_lock();
	active_gc = &gc;
	if (pipefds[0] == -1) {
		if (pipe(pipefds)) ;
	}
	xprintf(XGDB,"[ gdb connected ]\n");
	debugger_unlock();

	gc.flags |= F_LK_THREADS;

	for (;;) {

		fds[0].fd = fd;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		fds[1].fd = pipefds[0];
		fds[1].events = POLLIN;
		fds[1].revents = 0;

		// check ahead of the poll, since we may have
		// just resume'd a single-step that will have
		// halted again by now
		debugger_lock();
		if (gc.flags & F_RUNNING) {
			u32 csr;
			if (swdp_ahb_read(CDBG_CSR, &csr) == 0) {
				if (csr & CDBG_S_HALT) {
					gc.flags &= (~F_RUNNING);
					// todo: indicate specific halt reason
					gdb_prologue(&gc);
					gdb_puts(&gc, "S05");
					gdb_epilogue(&gc);
					gdb_update_threads(&gc);
				}
			}
		}
		debugger_unlock();

		r = poll(fds, 2, (gc.flags & F_RUNNING) ? 10 : -1);
		if (r < 0) {
			if (errno == EINTR) continue;
			break;
		}
		if (r == 0) {
			continue;
		}
		if (fds[0].revents & POLLIN) {
			r = read(fd, iobuf, sizeof(iobuf));
			if (r <= 0) {
				if (errno == EINTR) continue;
				break;
			}
			len = r;
			ptr = iobuf;
			while (len > 0) {
				r = gdb_recv(&gc, ptr, len);
				ptr += r;
				len -= r;
			}
		}
		if (fds[1].revents & POLLIN) {
			char x;
			if (read(fds[1].fd, &x, 1) < 0) ;
		}
	}

	debugger_lock();
	active_gc = NULL;
	xprintf(XGDB, "[ gdb connected ]\n");
	debugger_unlock();
}
