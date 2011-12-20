/* gpio.c
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
#include <fw/io.h>

#include <arch/hardware.h>

void gpio_config(unsigned n, unsigned cfg) {
	unsigned addr = 0x50008000 | (n & 0x30000);
	n &= 0xFFF;
	if (cfg) {
		writel(readl(addr) | n, addr);
	} else {
		writel(readl(addr) & (~n), addr);
	}
}

void gpio_set(unsigned n) {
	unsigned addr = 0x50000000 | (n & 0x30000) | ((n & 0xFFF) << 2);
	writel(0xFFFFFFFF, addr);
}

void gpio_clr(unsigned n) {
	unsigned addr = 0x50000000 | (n & 0x30000) | ((n & 0xFFF) << 2);
	writel(0, addr);
}

int gpio_get(unsigned n) {
	unsigned addr = 0x50003FFC | (n & 0x30000);
	n &= 0xFFF;
	return (readl(addr) & n) != 0;
}

