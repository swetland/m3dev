/* Deviation = 19.042969 */
/* Base frequency = 914.999969 */
/* Carrier frequency = 914.999969 */
/* Channel number = 0 */
/* Carrier frequency = 914.999969 */
/* Modulated = true */
/* Modulation format = 2-FSK */
/* Manchester enable = false */
/* Sync word qualifier mode = 30/32 sync word bits detected */
/* Preamble count = 4 */
/* Channel spacing = 199.951172 */
/* Carrier frequency = 914.999969 */
/* Data rate = 38.3835 */
/* RX filter BW = 101.562500 */
/* Data format = Normal mode */
/* CRC enable = true */
/* Whitening = true */
/* Device address = 0 */
/* Address config = No address check */
/* CRC autoflush = false */
/* PA ramping = false */
/* TX power = 0 */
struct cc1101_cfg {
    u8 reg, val;
};

struct cc1101_cfg config[] = {
    { CC_IOCFG0, 0x06 },
    { CC_FIFOTHR, 0x47 },
    { CC_FSCTRL1, 0x06 },
    { CC_FREQ2, 0x23 },
    { CC_FREQ1, 0x31 },
    { CC_FREQ0, 0x3B },
    { CC_MDMCFG4, 0xCA },
    { CC_MDMCFG3, 0x83 },
    { CC_MDMCFG2, 0x03 },
    { CC_DEVIATN, 0x34 },
    { CC_MCSM0, 0x18 },
    { CC_FOCCFG, 0x16 },
    { CC_WORCTRL, 0xFB },
    { CC_FSCAL3, 0xE9 },
    { CC_FSCAL2, 0x2A },
    { CC_FSCAL1, 0x00 },
    { CC_FSCAL0, 0x1F },
    { CC_TEST2, 0x81 },
    { CC_TEST1, 0x35 },
    { CC_TEST0, 0x09 },
};