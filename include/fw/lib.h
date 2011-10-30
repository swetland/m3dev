/* lib.h
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

#ifndef _FW_LIB_H_
#define _FW_LIB_H_

#define EFAIL   1
#define EBUSY   2
#define ENODEV  3

#include <stdarg.h>

void vprintx(void (*_putchar_)(unsigned), const char *fmt, va_list ap);
void printx(const char *fmt, ...);

void serial_init(unsigned sysclk_mhz, unsigned baud);
void serial_putc(unsigned c);

void gpio_config(unsigned n, unsigned cfg);
void gpio_set(unsigned n);
void gpio_clr(unsigned n);

void usb_init(void);
int usb_recv(void *data, int len);
int usb_xmit(void *data, int len);

#endif

