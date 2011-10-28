/* cm3.h
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

#ifndef _M3_H_
#define _M3_H_

enum {
        v_reset = 1,
        v_nmi,
        v_hardfault,
        v_mmu,
        v_busfault,
        v_usagefault,
        v_reserved_a,
        v_reserved_b,
        v_reserved_c,
        v_reserved_d,
        v_svc,
        v_debugmon,
        v_reserved_e,
        v_pendsv,
        v_systick,
};

#endif
