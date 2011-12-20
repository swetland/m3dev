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

#define EFAIL		1
#define EBUSY		2
#define ENODEV		3
#define ETIMEOUT	4

#include <stdarg.h>

void vprintx(void (*_putchar_)(unsigned), const char *fmt, va_list ap);
void printx(const char *fmt, ...);

void serial_init(unsigned sysclk_mhz, unsigned baud);
void serial_putc(unsigned c);

void gpio_config(unsigned n, unsigned cfg);
void gpio_set(unsigned n);
void gpio_clr(unsigned n);
int gpio_get(unsigned n);

/* init USB and attach */
void usb_init(unsigned vid, unsigned pid);

/* detach from USB */
void usb_stop(void);

/* read up to len bytes on bulk in
 * - stops on a short packet
 * - returns bytes read
 * - returns -ENODEV if USB went offline
 */
int usb_recv(void *data, int len);

/* send len bytes on bulk out
 * - returns bytes written
 * - returns -ENODEV if USB went offline
 */
int usb_xmit(void *data, int len);

/* same as usb_recv but returns -ETIMEOUT
 * if msec milliseconds pass. 
 * wait forever if msec == 0
 */
int usb_recv_timeout(void *data, int len, unsigned msec);

/* check for host, return nonzero if we found it */
int usb_online(void);


/* low level interrupt-based USB interface */

extern void (*usb_ep1_rx_full_cb)(void);
extern void (*usb_ep1_tx_empty_cb)(void);
extern void (*usb_online_cb)(int online);

int usb_ep1_read(void *data, int max);
int usb_ep1_write(void *data, int len);

void usb_mask_ep1_rx_full(void);
void usb_unmask_ep1_rx_full(void);
void usb_mask_ep1_tx_empty(void);
void usb_unmask_ep1_tx_empty(void);

#endif

