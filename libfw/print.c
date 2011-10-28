/* print.c
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

#include <fw/lib.h>

static char hex[16] = "0123456789ABCDEF";

void vprintx(void (*_putc)(unsigned), const char *fmt, va_list ap) {
	unsigned c, n;
	const char *s;
	char tmp[9];

	while ((c = *fmt++)) {
		if (c != '%') {
			_putc(c);
			continue;
		}
		switch((c = *fmt++)) {
		case 0: return;
		case 'x': c = 8; goto printhex;
		case 'h': c = 4; goto printhex;
		case 'b': c = 2; goto printhex;
		case 's': s = va_arg(ap, const char *); goto printstr;
		case 'c': c = va_arg(ap, unsigned); /* fall through */
		default: _putc(c); continue;
		}
printhex:
		n = va_arg(ap, unsigned);
		tmp[c] = 0;
		while (c-- > 0) {
			tmp[c] = hex[n & 0x0F];
			n >>= 4;
		}
		s = tmp;
printstr:
		while (*s)
			_putc(*s++);
	}
}

void printx(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintx(serial_putc, fmt, ap);
	va_end(ap);
}

