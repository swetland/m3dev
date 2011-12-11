/* mrf89xa.h
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

#ifndef _MRF89XA_H_
#define _MRF89XA_H_

#define GCONREG		0x00
#define DMODREG		0x01
#define FDEVREG		0x02
#define BRSREG		0x03
#define FLTHREG		0x04
#define FIFOCREG	0x05
#define R1CREG		0x06
#define P1CREG		0x07
#define S1CREG		0x08
#define R2CREG		0x09
#define P2CREG		0x0A
#define S2CREG		0x0B
#define PACREG		0x0C
#define FTXRXIREG	0x0D
#define FTPRIREG	0x0E
#define RSTHIREG	0x0F
#define FILCREG		0x10
#define PFCREG		0x11
#define SYNCREG		0x12
#define RSTSREG		0x13
#define RSVREG		0x14
#define OOKCREG		0x15
#define SYNCV31REG	0x16
#define SYNCV23REG	0x17
#define SYNCV15REG	0x18
#define SYNCV07REG	0x19
#define TXCONREG	0x1A
#define CLKOREG		0x1B
#define PLOADREG	0x1C
#define NADDSREG	0x1D
#define PKTCREG		0x1E
#define FCRCREG		0x1F

#define GCON_TX		(4 << 5)
#define GCON_RX		(3 << 5)
#define GCON_FREQ_SYNTH	(2 << 5)
#define GCON_STANDBY	(1 << 5)
#define GCON_SLEEP	(0 << 5)
#define GCON_950_BAND	(2 << 3)
#define GCON_863_BAND	(2 << 3)
#define GCON_915_BAND	(1 << 3)
#define GCON_902_BAND	(0 << 3)
#define GCON_RPS_1	(0 << 0) /* use R1/P1/S1 */
#define GCON_RPS_2	(1 << 0) /* use R2/P2/S2 */

#define DMOD_FSK	(2 << 6) /* modulation: FSK */
#define DMOD_OOK	(1 << 6) /* modulation: OOK */
#define DMOD_OOK_AVG	(2 << 3)
#define DMOD_OOK_PEAK	(1 << 3)
#define DMOD_OOK_FIXED	(0 << 3)
#define DMOD_IF_N13_5DB	(3 << 0) /* IF gain: -13.5dB */
#define DMOD_IF_N9DB	(2 << 0) /* IF gain: -9dB */
#define DMOD_IF_N4_5DB	(1 << 0) /* IF gain: -4.5dB */
#define DMOD_IF_0DB	(0 << 0) /* IF gain: 0dB */
#define DMOD_CONTINUOUS	(0x00) /* data mode: continuous */
#define DMOD_BUFFER	(0x20) /* data mode: buffer */
#define DMOD_PACKET	(0x04) /* data mode: packet */

/* fDev = fXtal / (32 * (FDEVREG + 1))    FDEVREG = 0..255 */
/* bitrate = fXtal / (64 * (BRSREG + 1))  BRSREG = 0..127  */

#define FIFOC_64B	(3 << 6) /* fifo size: 64 bytes */
#define FIFOC_48B	(2 << 6) /* fifo size: 48 bytes */
#define FIFOC_32B	(1 << 6) /* fifo size: 32 bytes */
#define FIFOC_16B	(0 << 6) /* fifo size: 16 bytes */
#define FIFOC_FTINT(n)	((n) & 0x3F) /* fifo int threshold */

#define PACREG_23US	(0x58) /* OOK TX PA ramp: 23 uS */
#define PACREG_15US	(0x50)
#define PACREG_8_5US	(0x48)
#define PACREG_3US	(0x40)

#define FTXRXI_P_IRQ0RX_SYNC		(3 << 6)
#define FTXRXI_P_IRQ0RX_NFIFOEMPTY	(2 << 6)
#define FTXRXI_P_IRQ0RX_WRITEBYTE	(1 << 6)
#define FTXRXI_P_IRQ0RX_PLREADY		(0 << 6)
#define FTXRXI_P_IRQ1RX_FIFO_THRESHOLD	(3 << 4)
#define FTXRXI_P_IRQ1RX_RSSI		(2 << 4)
#define FTXRXI_P_IRQ1RX_FIFOFULL	(1 << 4)
#define FTXRXI_P_IRQ1RX_CRCOK		(0 << 4)
#define FTXRXI_P_IRQ1TX_TXDONE		(1 << 3)
#define FTXRXI_P_IRQ1TX_FIFOFULL	(0 << 3)
#define FTXRXI_FIFO_FULL		(1 << 2)
#define FTXRXI_FIFO_EMPTY		(1 << 1)
#define FTXRXI_FIFO_OVERRUN		(1 << 0)

#define FTPRI_TX_DONE			(1 << 5)
#define FTPRI_TX_ON_FIFO_NOT_EMPTY	(1 << 4)
#define FTPRI_TX_ON_FIFO_THRESHOLD	(0 << 4)
#define FTPRI_SBO			(1 << 3) /* must be one */
#define FTPRI_RSSI_ABOVE_THRESHOLD	(1 << 2)
#define FTPRI_PLL_LOCKED		(1 << 1)
#define FTPRI_PLL_LOCK_ENABLE		(1 << 0)

#define FILC_PAS_987KHZ			(15 << 4)
#define FILC_PAS_676KHZ			(14 << 4)
#define FILC_PAS_514KHZ			(13 << 4)
#define FILC_PAS_458KHZ			(12 << 4)
#define FILC_PAS_414KHZ			(11 << 4)
#define FILC_PAS_378KHZ			(10 << 4)
#define FILC_PAS_321KHZ			(9 << 4)
#define FILC_PAS_262KHZ			(8 << 4)
#define FILC_PAS_234KHZ			(7 << 4)
#define FILC_PAS_211KHZ			(6 << 4)
#define FILC_PAS_184KHZ			(5 << 4)
#define FILC_PAS_157KHZ			(4 << 4)
#define FILC_PAS_137KHZ			(3 << 4)
#define FILC_PAS_109KHZ			(2 << 4)
#define FILC_PAS_82KHZ			(1 << 4)
#define FILC_PAS_65KHZ			(0 << 4)
#define FILC_BUT(n)			((n) & 0xF)

#define SYNC_POLFILEN			(1 << 7)
#define SYNC_BSYNCEN			(1 << 6)
#define SYNC_SYNCREN			(1 << 5)
#define SYNC_32BIT			(3 << 3)
#define SYNC_24BIT			(2 << 3)
#define SYNC_16BIT			(1 << 3)
#define SYNC_8BIT			(0 << 3)
#define SYNC_3ERR			(3 << 1)
#define SYNC_2ERR			(2 << 1)
#define SYNC_1ERR			(1 << 1)
#define SYNC_0ERR			(0 << 1)

#define TXCON_TXIPOLFV(n)		(((n) & 15) << 4)
#define TXCON_N8DBM			(7 << 1) /* -8dBm */
#define TXCON_N5DBM			(6 << 1) /* -5dBm */
#define TXCON_N2DBM			(5 << 1) /* -2dBm */
#define TXCON_1DBM			(4 << 1) /*  1dBm */
#define TXCON_4DBM			(3 << 1) /*  4dBm */
#define TXCON_7DBM			(2 << 1) /*  7dBm */
#define TXCON_10DBM			(1 << 1) /* 10dBm */
#define TXCON_13DBM			(0 << 1) /* 13dBm */

#define PLOAD_MCHSTREN			(1 << 7)

#define PKTC_VARIABLE			(1 << 7)
#define PKTC_FIXED			(0 << 7)
#define PKTC_PREAMBLE_4B		(3 << 5)
#define PKTC_PREAMBLE_3B		(2 << 5)
#define PKTC_PREAMBLE_2B		(1 << 5)
#define PKTC_PREAMBLE_1B		(0 << 5)
#define PKTC_WHITENING			(1 << 4)
#define PKTC_CRC_ENABLE			(1 << 3)
#define PKTC_ADDR_NODE_OR_00_OR_FF	(3 << 2)
#define PKTC_ADDR_NODE_OR_00		(2 << 2)
#define PKTC_ADDR_NODE			(1 << 2)
#define PKTC_ADDR_OFF			(0 << 2)
#define PKTC_CRC_OK			(1 << 0)

#define FCRC_AUTO_CLEAR			(1 << 7) /* clear if CRC bad */
#define FCRC_FIFO_STANDBY_READ		(1 << 6)
#define FCRC_FIFO_STANDBY_WRITE		(0 << 5)

/* common freq values for 12.8 MHz crystal */
/* freq = 200kHz * (fXtal / 12.8 MHz) * ((val + 1) / 8) */
#define X400KHZ				(0xF)
#define X375KHZ				(0xE)
#define X350KHZ				(0xD)
#define X325KHZ				(0xC)
#define X300KHZ				(0xB)
#define X275KHZ				(0xA)
#define X250KHZ				(0x9)
#define X225KHZ				(0x8)
#define X200KHZ				(0x7)
#define X175KHZ				(0x6)
#define X150KHZ				(0x5)
#define X125KHZ				(0x4)
#define X100KHZ				(0x3)
#define X75KHZ				(0x2)
#define X50KHZ				(0x1)
#define X25KHZ				(0x0)

#endif
