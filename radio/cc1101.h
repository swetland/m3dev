/* cc1101.h
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

#ifndef _CC1101_H_
#define _CC1101_H_

#define CC_CMD_SRES	0x30 /* reset */
#define CC_CMD_SFSTXON	0x31 /* enable and calibrate freq synth */
#define CC_CMD_SXOFF	0x32 /* disable crystal osc */
#define CC_CMD_SCAL	0x33 /* calibrate frequency syntheisze */
#define CC_CMD_SRX	0x34 /* enable rx */
#define CC_CMD_STX	0x35 /* idle: ->tx, rx: ->tx if !CCA | clear */
#define CC_CMD_SIDLE	0x36 /* exit rx/tx/wor, turn off freq synth */
#define CC_CMD_SWOR	0x37 /* start wake-on-radio polling */
#define CC_CMD_SPWD	0x38 /* power down when CSn goes high */
#define CC_CMD_SFRX	0x3A /* flush rx fifo */
#define CC_CMD_SFTX	0x3B /* flush tx fifo */
#define CC_CMD_SWORRST	0x3C /* reset RTC to Event1 */
#define CC_CMD_SNOP	0x3D

#define CC_IOCFG2	0x00
#define CC_IOCFG1	0x01
#define CC_IOCFG0	0x02
#define CC_FIFOTHR	0x03
#define CC_SYNC1	0x04
#define CC_SYNC2	0x05
#define CC_PKTLEN	0x06
#define CC_PKTCTRL1	0x07
#define CC_PKTCTRL0	0x08
#define CC_ADDR		0x09
#define CC_CHANNR	0x0A
#define CC_FSCTRL1	0x0B
#define CC_FSCTRL0	0x0C
#define CC_FREQ2	0x0D
#define CC_FREQ1	0x0E
#define CC_FREQ0	0x0F
#define CC_MDMCFG4	0x10
#define CC_MDMCFG3	0x11
#define CC_MDMCFG2	0x12
#define CC_MDMCFG1	0x13
#define CC_MDMCFG0	0x14
#define CC_DEVIATN	0x15
#define CC_MCSM2	0x16
#define CC_MCSM1	0x17
#define CC_MCSM0	0x18
#define CC_FOCCFG	0x19
#define CC_BSCFG	0x1A
#define CC_AGCTRL2	0x1B
#define CC_AGCTRL1	0x1C
#define CC_AGCTRL0	0x1D
#define CC_WOREVT1	0x1E
#define CC_WOREVT0	0x1F
#define CC_WORCTRL	0x20
#define CC_FREND1	0x21
#define CC_FREND0	0x22
#define CC_FSCAL3	0x23
#define CC_FSCAL2	0x24
#define CC_FSCAL1	0x25
#define CC_FSCAL0	0x26
#define CC_RCCTRL1	0x27
#define CC_RCCTRL0	0x28
#define CC_FSTEST	0x29
#define CC_PTEST	0x2A
#define CC_AGCTEST	0x2B
#define CC_TEST2	0x2C
#define CC_TEST1	0x2D
#define CC_TEST0	0x2E

/* status regs - read only */
#define CC_PARTNUM	0xF0
#define CC_VERSION	0xF1
#define CC_FREQEST	0xF2
#define CC_LQI		0xF3
#define CC_RSSI		0xF4
#define CC_MARCSTATE	0xF5
#define CC_WORTIME1	0xF6
#define CC_WORTIME0	0xF7
#define CC_PKTSTATUS	0xF8
#define CC_VCO_VC_DAC	0xF9
#define CC_TXBYTES	0xFA
#define CC_RXBYTES	0xFB
#define CC_RCCTRL1_STAT	0xFC
#define CC_RCCTRL0_STAT	0xFD


#define CC_S_IDLE	0
#define CC_S_RX		1
#define CC_S_TX		2
#define CC_S_FSTXON	3
#define CC_S_CALIBRATE	4
#define CC_S_SETTLING	5
#define CC_S_RX_OVF	6
#define CC_S_TX_OVF	7
#define CC_S_MASK	8

#endif
