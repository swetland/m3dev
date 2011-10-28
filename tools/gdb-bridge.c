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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <fw/types.h>
#include "rswdp.h"

struct gdbcnxn {
	int tx, rx;
	unsigned chk;
};

int gdb_getc(struct gdbcnxn *gc) {
	int r;
	unsigned char c;
	for (;;) {
		r = read(gc->rx, &c, 1);
		if (r <= 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		return c;
	}
}

int gdb_putc(struct gdbcnxn *gc, unsigned n) {
	unsigned char c = n;
	int r;
	gc->chk += c;
	for (;;) {
		r = write(gc->tx, &c, 1);
		if (r <= 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		return 0;
	}
}
int gdb_puts(struct gdbcnxn *gc, const char *s) {
	int r;
	while (*s) {
		r = gdb_putc(gc, *s++);
		if (r < 0) return r;
	}
	return 0;
}
int gdb_prologue(struct gdbcnxn *gc) {
	int n = gdb_putc(gc, '$');
	gc->chk = 0;
	return n;
}
int gdb_epilogue(struct gdbcnxn *gc) {
	char tmp[4];
	sprintf(tmp,"#%02x", gc->chk & 0xff);
	return gdb_puts(gc, tmp);
}

static char HEX[16] = "0123456789abcdef";
int gdb_puthex(struct gdbcnxn *gc, void *ptr, unsigned len) {
	unsigned char *data = ptr;
	unsigned n;
	char buf[1025];

	n = 0;
	while (len-- > 0) {
		unsigned c = *data++;
		buf[n++] = HEX[c >> 4];
		buf[n++] = HEX[c & 15];
		if (n == 1024) {
			buf[n] = 0;
			if (gdb_puts(gc, buf))
				return -1;
			n = 0;
		}
	}
	if (n) {
		buf[n] = 0;
		return gdb_puts(gc, buf);
	}
	return 0;
}

int gdb_recv(struct gdbcnxn *gc, char *buf, int max) {
	char *start = buf;
	unsigned chk;
	char tmp[3];
	int c;

again:
	do {
		c = gdb_getc(gc);
		if (c < 0) goto fail;
		if (c < 0x20)
			fprintf(stderr,"! %02x !\n",c);
	} while (c != '$');

	chk = 0;
	while (max > 1) {
		c = gdb_getc(gc);
		if (c == '#') {
			*buf++ = 0;
			c = gdb_getc(gc);
			if (c < 0) goto fail;
			tmp[0] = c;
			c = gdb_getc(gc);
			if (c < 0) goto fail;
			tmp[1] = c;
			c = strtoul(tmp, 0, 16);
			if (c != (chk & 0xff)) {
				gdb_putc(gc,'-');
				fprintf(stderr,"PKT: BAD CHECKSUM\n");
				goto again;
			} else {
				gdb_putc(gc,'+');
				return 0;
			}
		} else {
			chk += c;
			*buf++ = c;
		}
	}
	gdb_putc(gc,'-');
	fprintf(stderr,"PKT: OVERFLOW\n");
	goto again;

fail:
	*start = 0;
	return -1;
}

unsigned data[512];

void handle_command(struct gdbcnxn *gc, char *cmd)
{
	unsigned n,x,i;

	switch (cmd[0]) {
	case '?':
		gdb_puts(gc, "S00");
		swdp_core_halt();
		break;
	case 'H':
		gdb_puts(gc, "OK");
		break;
	case 'm':
		if (sscanf(cmd + 1, "%x,%x", &x, &n) != 2)
			break;
		n = ((n + 3) & (~3)) / 4;

		//fprintf(stderr,"%x %x\n", x, n);
		for (i = 0; i < n; i++) {
			swdp_ahb_read(x, data + i);
			x += 4;
		}
		gdb_puthex(gc, data, 4 * n);
		break;	
	case 'g':
		for (n = 0; n < 18; n++)
			swdp_core_read(n, &data[n]);
		gdb_puthex(gc, data, 4 * 18);
		break;
	}
}

void handler(int n) {
}

int main(int argc, char **argv) {
	struct gdbcnxn gc;
	char cmd[32768];
	gc.tx = 1;
	gc.rx = 0;
	gc.chk = 0;

	signal(SIGINT, handler);
	
	fprintf(stderr,"[ debugport v1.0 ]\n");

	if (swdp_open())
		fprintf(stderr,"error: cannot find swdp board\n");

	for (;;) {
		if (gdb_recv(&gc, cmd, sizeof(cmd))) {
			fprintf(stderr,"[ disconnect ]\n");
			return 0;
		}
		//fprintf(stderr,"PKT: %s\n", cmd);
		gdb_prologue(&gc);
		handle_command(&gc, cmd);
		gdb_epilogue(&gc);
	}
	return 0;
}
