/* lpcboot.c
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
#include <stdint.h>

#include "usb.h"

static int match_18d1_db05(usb_ifc_info *ifc) {
	if (ifc->dev_vendor != 0x18d1)
		return -1;
	if (ifc->dev_product != 0xdb05)
		return -1;
	return 0;
}

int main(int argc, char **argv) {
	int r, fd, once;
	usb_handle *usb;
	char buf[64];

	once = 1;
	for (;;) {		
		usb = usb_open(match_18d1_db05);
		if (usb == 0) {
			if (once) {
				fprintf(stderr,"waiting for device...\n");
				once = 0;
			}
		} else {
			break;
		}
	}

	for (;;) {
		r = usb_read(usb, buf, 64);
		if (r < 0)
			break;
		write(1, buf, r);
	}

	return 0;
}
