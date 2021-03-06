
#define IOCON_FUNC_0		(0 << 0)
#define IOCON_FUNC_1		(1 << 0)
#define IOCON_FUNC_2		(2 << 0)
#define IOCON_FUNC_3		(3 << 0)
#define IOCON_PULL_DOWN		(1 << 3)
#define IOCON_PULL_UP		(2 << 3)
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
