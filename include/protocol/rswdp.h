/* rswdp.h - remote serial wire debug protocol
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

/* Remote Serial Wire Debug Protocol */

#ifndef _RSWDP_PROTOCOL_H_
#define _RSWDP_PROTOCOL_H_

struct msg {
	u8 msg;
	u8 seq;
	u8 pr0;
	u8 pr1;
	u8 op[12];
	u32 data[12];
};

struct raw {
	u8 msg;
	u8 seq;
	u8 pr0;
	u8 pr1;
	u8 data[60];
};

/* target to host messages */
#define MSG_OKAY	0x00	/* response to MSG with matching seq */
#define MSG_FAIL	0x01	/* failure response to MSG with matching seq */
#define MSG_DEBUG	0x02	/* debug print */
#define MSG_TRACE	0x03	/* pr0 = 1 -> enable tracing */

/* host to target messages */
#define MSG_NULL	0x04
#define MSG_ATTACH	0x05	/* execute sw reset + jtag handover */
#define MSG_SWDP_OPS	0x06	/* pr0 (up to 12) SWDP operations */ 
#define MSG_RESET_N	0x07	/* pr0 1=assert 0=release target RESET */

/* host to target softload to 0x20001000 */
#define MSG_DOWNLOAD	0x08	/* pr0 pr1 = LE word offset */
#define MSG_EXECUTE	0x09

/* low level ops - combine for direct AP/DP io */
#define OP_RD 0x00
#define OP_WR 0x01
#define OP_DP 0x00
#define OP_AP 0x02
#define OP_X0 0x00
#define OP_X4 0x04
#define OP_X8 0x08
#define OP_XC 0x0C

/* DP registers */
#define DP_IDCODE	(OP_DP|OP_X0)
#define DP_ABORT	(OP_DP|OP_X0)
#define DP_DPCTRL	(OP_DP|OP_X4)
#define DP_RESEND	(OP_DP|OP_X8)
#define DP_SELECT	(OP_DP|OP_X8)
#define DP_BUFFER	(OP_DP|OP_XC)

/* AHB AP registers */
#define AHB_CSW		0x00
#define AHB_TAR		0x04 
#define AHB_DRW		0x0C
#define AHB_BD0		0x10
#define AHB_BD1		0x14
#define AHB_BD2		0x18
#define AHB_BD3		0x20
#define AHB_ROM_ADDR	0xF8
#define AHB_IDR		0xFC

#define AHB_CSW_MCORE	(0 << 29)
#define AHB_CSW_MDEBUG	(1 << 29)
#define AHB_CSW_USER	(0 << 25)
#define AHB_CSW_PRIV	(1 << 25)
#define AHB_CSW_DBG_EN	(1 << 6)
#define AHB_CSW_8BIT	(0 << 0)
#define AHB_CSW_16BIT	(1 << 0)
#define AHB_CSW_32BIT	(2 << 0)

/* Core Debug registers */
#define CDBG_CSR		0xE000EDF0
#define CDBG_REG_ADDR		0xE000EDF4
#define CDBG_REG_DATA		0xE000EDF8
#define CDBG_EMCR		0xE000EDFC

#define CDBG_CSR_KEY		0xA05F0000
#define CDBG_S_RESET_ST		(1 << 25)
#define CDBG_S_RETIRE_ST	(1 << 24)
#define CDBG_S_LOCKUP		(1 << 19)
#define CDBG_S_SLEEP		(1 << 18)
#define CDBG_S_HALT		(1 << 17)
#define CDBG_S_REGRDY		(1 << 16)
#define CDBG_C_SNAPSTALL	(1 << 5)
#define CDBG_C_MASKINTS		(1 << 3)
#define CDBG_C_STEP		(1 << 2)
#define CDBG_C_HALT		(1 << 1)
#define CDBG_C_DEBUGEN		(1 << 0)

#define IDCODE_M3 0x1BA01477

#endif
