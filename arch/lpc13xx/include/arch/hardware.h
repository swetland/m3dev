#ifndef _LPC13XX_HARDWARE_H_
#define _LPC13XX_HARDWARE_H_

#define IOCON_FUNC_0		(0 << 0)
#define IOCON_FUNC_1		(1 << 0)
#define IOCON_FUNC_2		(2 << 0)
#define IOCON_FUNC_3		(3 << 0)
#define IOCON_PULL_UP		(1 << 3)
#define IOCON_PULL_DOWN		(2 << 3)
#define IOCON_REPEATER		(3 << 3)
#define IOCON_HYSTERESIS	(1 << 5)
#define IOCON_ANALOG		(3 << 6)
#define IOCON_DIGITAL		(2 << 6)
#define IOCON_PSEUDO_OPENDRAIN	(1 << 10)

#define IOCON_PIO0_0	0x4004400C /* RESETn / PIO */
#define IOCON_PIO0_1	0x40044010 /* PIO / CLKOUT / CT32B0_MAT2 / USB_FTOGGLE */
#define IOCON_PIO0_2	0x4004401C /* PIO / SSEL0 / CT16B0_CAP0 */
#define IOCON_PIO0_3	0x4004402C /* PIO / USB_VBUS */
#define IOCON_PIO0_4	0x40044030 /* PIO / I2C_SCL */
#define IOCON_PIO0_5	0x40044034 /* PIO / I2C_SDA */	
#define IOCON_PIO0_6	0x4004404C /* PIO / USB_CONNECTn / SCK0+ */
#define IOCON_PIO0_7	0x40044050 /* PIO / CTSn */
#define IOCON_PIO0_8	0x40044060 /* PIO / MISO0 / CT16B0_MAT0 */
#define IOCON_PIO0_9	0x40044064 /* PIO / MOSI0 / CT16B0_MAT1 / SWO */
#define IOCON_PIO0_10	0x40044068 /* SWCLK / PIO / SCK0+ / CT16B0_MAT2 */
#define IOCON_PIO0_11	0x40044074 /* reserved / PIO / AD0 / CT32B0_MAT3 */

#define IOCON_PIO1_0	0x40044078 /* reserved / PIO / AD1 / CT32B1_CAP0 */
#define IOCON_PIO1_1	0x4004407C /* reserved / PIO / AD2 / CT32B1_MAT0 */
#define IOCON_PIO1_2	0x40044080 /* reserved / PIO / AD3 / CT32B1_MAT1 */
#define IOCON_PIO1_3	0x40044090 /* SWDIO / PIO / AD4 / CT32B1_MAT2 */
#define IOCON_PIO1_4	0x40044094 /* PIO / AD5 / CT32B1_MAT3 */
#define IOCON_PIO1_5	0x400440A0 /* PIO / RTSn / CT32B0_CAP0 */
#define IOCON_PIO1_6	0x400440A4 /* PIO / RXD / CT32B0_MAT0 */
#define IOCON_PIO1_7	0x400440A8 /* PIO / TXD / CT32B0_MAT1 */
#define IOCON_PIO1_8	0x40044014 /* PIO / CT16B1_CAP0 */
#define IOCON_PIO1_9	0x40044038 /* PIO / CT16B1_MAT0 */
#define IOCON_PIO1_10	0x4004406C /* PIO / AD6 / CT16B1_MAT1 */
#define IOCON_PIO1_11	0x40044098 /* PIO / AD7 */

#define IOCON_PIO2_0	0x40044008 /* PIO / DTRn / SSEL1 */
#define IOCON_PIO2_1	0x40044028 /* PIO / DSRn+ / SCK1 */
#define IOCON_PIO2_2	0x4004405C /* PIO / DCDn+ / MISO1 */
#define IOCON_PIO2_3	0x4004408C /* PIO / RIn+ / MOSI1 */
#define IOCON_PIO2_4	0x40044040 /* PIO */
#define IOCON_PIO2_5	0x40044044 /* PIO */
#define IOCON_PIO2_6	0x40044000
#define IOCON_PIO2_7	0x40044020 /* PIO */
#define IOCON_PIO2_8	0x40044024 /* PIO */
#define IOCON_PIO2_9	0x40044054 /* PIO */
#define IOCON_PIO2_10	0x40044058 /* PIO */
#define IOCON_PIO2_11	0x40044070 /* PIO / SCK0+ */

#define IOCON_PIO3_0	0x40044084 /* PIO / DTRn */
#define IOCON_PIO3_1	0x40044088 /* PIO / DSRn+ */
#define IOCON_PIO3_2	0x4004409C /* PIO / DCDn+ */
#define IOCON_PIO3_3	0x400440AC /* PIO / RIn+ */
#define IOCON_PIO3_4	0x4004403C /* PIO */
#define IOCON_PIO3_5	0x40044048 /* PIO */

#define IOCON_SCK0_LOC		0x400440B0 /* + where is SCK0? */
#define IOCON_SCK0_PIO0_10	0
#define IOCON_SCK0_PIO2_11	1
#define IOCON_SCK0_PIO0_6	2
#define IOCON_DSR_LOC		0x400440B4
#define IOCON_DSR_PIO2_1	0
#define IOCON_DSR_PIO3_1	1
#define IOCON_DCD_LOC		0x400440B8
#define IOCON_DCD_PIO2_2	0
#define IOCON_DCD_PIO3_2	1
#define IOCON_RI_LOC		0x400440BC
#define IOCON_RI_PIO2_3		0
#define IOCON_RI_PIO3_3		1

#define SYS_CLK_CTRL		0x40048080
#define SYS_CLK_SYS		(1 << 0)
#define SYS_CLK_ROM		(1 << 1)
#define SYS_CLK_RAM		(1 << 2)
#define SYS_CLK_FLASHREG	(1 << 3)
#define SYS_CLK_FLASHARRAY	(1 << 4)
#define SYS_CLK_I2C		(1 << 5)
#define SYS_CLK_GPIO		(1 << 6)
#define SYS_CLK_CT16B0		(1 << 7)
#define SYS_CLK_CT16B1		(1 << 8)
#define SYS_CLK_CT32B0		(1 << 9)
#define SYS_CLK_CT32B1		(1 << 10)
#define SYS_CLK_SSP0		(1 << 11)
#define SYS_CLK_UART		(1 << 12) /* MUST CONFIG IOCON FIRST */
#define SYS_CLK_ADC		(1 << 13)
#define SYS_CLK_USB_REG		(1 << 14)
#define SYS_CLK_WDT		(1 << 15)
#define SYS_CLK_IOCON		(1 << 16)
#define SYS_CLK_SSP1		(1 << 18)

#define SYS_DIV_AHB		0x40048078
#define SYS_DIV_SSP0		0x40048094 /* 0 = off, 1... = PCLK/n */
#define SYS_DIV_UART		0x40048098
#define SYS_DIV_SSP1		0x4004809C
#define SYS_DIV_TRACE		0x400480AC
#define SYS_DIV_SYSTICK		0x400480B0

#define CLKOUT_SELECT		0x400480E0
#define CLKOUT_IRC		0
#define CLKOUT_SYSTEM		1
#define CLKOUT_WATCHDOG		2
#define CLKOUT_MAIN		3
#define CLKOUT_UPDATE		0x400480E4 /* write 0, then 1 to update */

#define PRESETCTRL		0x40048004
#define SSP0_RST_N		(1 << 0)
#define I2C_RST_N		(1 << 1)
#define SSP1_RST_N		(1 << 2)


#define USB_INT_STATUS		0x40020000
#define USB_INT_ENABLE		0x40020004
#define USB_INT_CLEAR		0x40020008
#define USB_INT_SET		0x4002000C
#define USB_CMD_CODE		0x40020010
#define USB_CMD_DATA		0x40020014
#define USB_RX_DATA		0x40020018
#define USB_TX_DATA		0x4002001C
#define USB_RX_PLEN		0x40020020
#define USB_TX_PLEN		0x40020024
#define USB_CTRL		0x40020028
#define USB_FIQ_SELECT		0x4002002C

#define USB_INT_FRAME		(1 << 0)
#define USB_INT_EP0		(1 << 1)
#define USB_INT_EP1		(1 << 2)
#define USB_INT_EP2		(1 << 3)
#define USB_INT_EP3		(1 << 4)
#define USB_INT_EP4		(1 << 5)
#define USB_INT_EP5		(1 << 6)
#define USB_INT_EP6		(1 << 7)
#define USB_INT_EP7		(1 << 8)
#define USB_INT_DEV_STAT	(1 << 9) /* RESET, SUSPEND, CONNECT */
#define USB_INT_CC_EMPTY	(1 << 10) /* can write CMD_CODE */
#define USB_INT_CD_FULL		(1 << 11) /* can read CMD_DATA */
#define USB_INT_RX_END		(1 << 12)
#define USB_INT_TX_END		(1 << 13)

#define USB_CTRL_RD_EN		(1 << 0)
#define USB_CTRL_WR_EN		(1 << 1)
#define USB_CTRL_EP_NUM(n)	((n & 0xF) << 2)

#define USB_OP_WRITE		0x0100
#define USB_OP_READ		0x0200
#define USB_OP_COMMAND		0x0500

#define USB_CC_SET_ADDR		0xD00000
#define USB_CC_CONFIG_DEV	0xD80000
#define USB_CC_SET_MODE		0xF30000
#define USB_CC_RD_INT_STATUS	0xF40000
#define USB_CC_RD_FRAME_NUM	0xF50000
#define USB_CC_RD_CHIP_ID	0xFD0000
#define USB_CC_SET_DEV_STATUS	0xFE0000
#define USB_CC_GET_DEV_STATUS	0xFE0000
#define USB_CC_GET_ERROR_CODE	0xFF0000
#define USB_CC_SEL_EPT(n)	((n & 0xF) << 16)
#define USB_CC_CLR_EPT(n)	(((n & 0xF) | 0x40) << 16)
#define USB_CC_SET_EPT(n)	(((n & 0xF) | 0x40) << 16)
#define USB_CC_CLR_BUFFER	0xF20000
#define USB_CC_VAL_BUFFER	0xFA0000


#define SSP0_CR0		0x40040000
#define SSP0_CR1		0x40040004
#define SSP0_DR			0x40040008 /* data */
#define SSP0_SR			0x4004000C /* status */
#define SSP0_CPSR		0x40040010 /* clock prescale 2..254 bit0=0 always */
#define SSP0_IMSC		0x40040014 /* IRQ mask set/clear */
#define SSP0_RIS		0x40040018 /* IRQ raw status */
#define SSP0_MIS		0x4004001C /* IRQ masked status */
#define SSP0_ICR		0x40040020 /* IRQ clear */

#define SSP_CR0_BITS(n)		((n) - 1) /* valid for n=4..16 */
#define SSP_CR0_FRAME_SPI	(0 << 4)
#define SSP_CR0_FRAME_TI	(1 << 4)
#define SSP_CR0_FRAME_MICROWIRE	(2 << 4)
#define SSP_CR0_CLK_LOW		(0 << 6) /* clock idle is low */
#define SSP_CR0_CLK_HIGH	(1 << 6) /* clock idle is high */
#define SSP_CR0_PHASE0		(0 << 7) /* capture on clock change from idle */
#define SSP_CR0_PHASE1		(1 << 7) /* capture on clock change to idle */
#define SSP_CR0_CLOCK_RATE(n)	(((n) - 1) << 8)

#define SSP_CR1_LOOPBACK	(1 << 0)
#define SSP_CR1_ENABLE		(1 << 1)
#define SSP_CR1_MASTER		(0 << 2)
#define SSP_CR1_SLAVE		(1 << 2)
#define SSP_CR1_OUTPUT_DISABLE	(1 << 3) /* only valid in SLAVE mode */

#define SSP_XMIT_EMPTY		(1 << 0)
#define SSP_XMIT_NOT_FULL	(1 << 1)
#define SSP_RECV_NOT_EMPTY	(1 << 2)
#define SSP_RECV_FULL		(1 << 3)
#define SSP_BUSY		(1 << 4)

/* SSP bitrate = SYSCLK / SYS_DIV_SSPn / SSP_CR0_CLOCK_RATE */

#define GPIOBASE(n)		(0x50000000 + ((n) << 16))
#define GPIODATA(n)		(GPIOBASE(n) + 0x3FFC)
#define GPIODIR(n)		(GPIOBASE(n) + 0x8000) /* 0 = input, 1 = output */
#define GPIOIER(n)		(GPIOBASE(n) + 0x8010) /* 1 = irq enabled */
#define GPIOLEVEL(n)		(GPIOBASE(n) + 0x8004) /* 0 = edge, 1 = level irq */
#define GPIOBOTHEDGES(n)	(GPIOBASE(n) + 0x8008) /* 1 = trigger on both edges */
#define GPIOPOLARITY(n)		(GPIOBASE(n) + 0x800C) /* 0 = low/falling, 1 = high/rising */
#define GPIORAWISR(n)		(GPIOBASE(n) + 0x8014) /* 1 if triggered */
#define GPIOISR(n)		(GPIOBASE(n) + 0x8018) /* 1 if triggered and enabled */
#define GPIOICR(n)		(GPIOBASE(n) + 0x801C) /* write 1 to clear, 2 clock delay */

/* these registers survive powerdown / warm reboot */
#define GPREG0			0x40038004
#define GPREG1			0x40038008
#define GPREG2			0x4003800C
#define GPREG3			0x40038010
#define GPREG4			0x40038014


#define TM32B0TCR		0x40014004
#define TM32B0TC		0x40014008 /* increments every PR PCLKs */
#define TM32B0PR		0x4001400C
#define TM32B0PC		0x40014010 /* increments every PCLK */
#define TM32B0MCR		0x40014014
#define TM32B0MR0		0x40014018
#define TM32B0MR1		0x4001401C
#define TM32B0MR2		0x40014020
#define TM32B0MR3		0x40014024
#define TM32B0CCR		0x40014028
#define TM32B0CR0		0x4001402C
#define TM32B0EMR		0x4001403C

#define TM32B1TCR		0x40018004
#define TM32B1TC		0x40018008 /* increments every PR PCLKs */
#define TM32B1PR		0x4001800C
#define TM32B1PC		0x40018010 /* increments every PCLK */
#define TM32B1MCR		0x40018014
#define TM32B1MR0		0x40018018
#define TM32B1MR1		0x4001801C
#define TM32B1MR2		0x40018020
#define TM32B1MR3		0x40018024
#define TM32B1CCR		0x40018028
#define TM32B1CR0		0x4001802C
#define TM32B1EMR		0x4001803C

#define TM32TCR_ENABLE		1
#define TM32TCR_RESET		2

/* Match Control Register (MCR) actions for each Match Register */
#define TM32_M0_IRQ		(1 << 0)
#define TM32_M0_RESET		(1 << 1)
#define TM32_M0_STOP		(1 << 2)
#define TM32_M1_IRQ		(1 << 3)
#define TM32_M1_RESET		(1 << 4)
#define TM32_M1_STOP		(1 << 5)
#define TM32_M2_IRQ		(1 << 6)
#define TM32_M2_RESET		(1 << 7)
#define TM32_M2_STOP		(1 << 8)
#define TM32_M3_IRQ		(1 << 9)
#define TM32_M3_RESET		(1 << 10)
#define TM32_M3_STOP		(1 << 11)

/* External Match Control (EMC) actions for each Match Register */
#define TM32_EMC0_CLEAR		(1 << 4)
#define TM32_EMC0_SET		(2 << 4)
#define TM32_EMC0_TOGGLE	(3 << 4)
#define TM32_EMC1_CLEAR		(1 << 6)
#define TM32_EMC1_SET		(2 << 6)
#define TM32_EMC1_TOGGLE	(3 << 6)
#define TM32_EMC2_CLEAR		(1 << 8)
#define TM32_EMC2_SET		(2 << 8)
#define TM32_EMC2_TOGGLE	(3 << 8)
#define TM32_EMC3_CLEAR		(1 << 10)
#define TM32_EMC3_SET		(2 << 10)
#define TM32_EMC3_TOGGLE	(3 << 10)

#endif
