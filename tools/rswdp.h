/* rswdp.h
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

#ifndef _RSWDP_H__
#define _RSWDP_H__

int swdp_ahb_read(u32 addr, u32 *value);
int swdp_ahb_write(u32 addr, u32 value);

int swdp_ahb_read32(u32 addr, u32 *out, int count);

int swdp_core_halt(void);
int swdp_core_step(void);
int swdp_core_resume(void);

/* access to CPU registers */
int swdp_core_read(u32 n, u32 *v);
int swdp_core_read_all(u32 *v);
int swdp_core_write(u32 n, u32 v);

int swdp_reset(void);

int swdp_open(void);

void swdp_enable_tracing(int yes);

void swdp_target_reset(int enable);

#endif

