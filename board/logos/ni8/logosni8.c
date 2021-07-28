// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <malloc.h>
#include <asm/arch/mx6-pins.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/spi.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/video.h>
#include <fsl_esdhc_imx.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <input.h>
#include <netdev.h>
#include <usb/ehci-ci.h>
#include "logosLogo.h"
#include "bootmelody.h"

#ifdef CONFIG_CMD_I2C 		// Added for Logosni8 Testing
	#include <i2c.h>
	#include <asm/mach-imx/mxc_i2c.h>
#endif


//Uncomment to enable the demo
//#define DEMO_MODE

// ENUM for controlling the reset for I2c select for LCDs, HDMI, GP and CAM
enum I2C_RESET {
	GPIO_I2C_BUS_SEL_RESET = IMX_GPIO_NR(2, 0)
};
// ENUM for configuring the AR8035 ethernet adapter
enum AR8035_CONFIGS {
	GPIO_RGMII_RX_DV =   IMX_GPIO_NR(6, 24),
	GPIO_ENET_RXD0_INT = IMX_GPIO_NR(1, 27),
	GPIO_RGMII_RX_D0 =   IMX_GPIO_NR(6, 25),
	GPIO_RGMII_RX_D1 =   IMX_GPIO_NR(6, 27),
	GPIO_RGMII_RX_D2 =   IMX_GPIO_NR(6, 28),
	GPIO_RGMII_RX_D3 =   IMX_GPIO_NR(6, 29),
	GPIO_RGMII_RX_CLK =  IMX_GPIO_NR(6, 30)
};

// ENUM for bootconfigs
enum BOOT_CONFIGS {
	GPIO_EIM_DA0 = IMX_GPIO_NR(3, 0),
	GPIO_EIM_DA1 = IMX_GPIO_NR(3, 1),
	GPIO_EIM_DA2 = IMX_GPIO_NR(3, 2),
	GPIO_EIM_DA3 = IMX_GPIO_NR(3, 3),
	GPIO_EIM_DA4 = IMX_GPIO_NR(3, 4),
	GPIO_EIM_DA5 = IMX_GPIO_NR(3, 5),
	GPIO_EIM_DA6 = IMX_GPIO_NR(3, 6),
	GPIO_EIM_DA7 = IMX_GPIO_NR(3, 7),
	GPIO_EIM_DA8 = IMX_GPIO_NR(3, 8),
	GPIO_EIM_DA9 = IMX_GPIO_NR(3, 9),
	GPIO_EIM_DA10 = IMX_GPIO_NR(3, 10),
	GPIO_EIM_DA11 = IMX_GPIO_NR(3, 11),
	GPIO_EIM_DA12 = IMX_GPIO_NR(3, 12),
	GPIO_EIM_DA13 = IMX_GPIO_NR(3, 13),
	GPIO_EIM_DA14 = IMX_GPIO_NR(3, 14),
	GPIO_EIM_DA15 = IMX_GPIO_NR(3, 15)

};

// Enum for LEDs on the Logosni8 board - enum idea came from board/beckhoff/mx53cx9020
enum LED_GPIOS {
	GPIO_LED_2 = IMX_GPIO_NR(6, 7),
	GPIO_LED_3 = IMX_GPIO_NR(6, 9)
};

// Enum for GPIOs[0-11] on the Logosni8 board
enum GPIOS {
	GPIO_0				= IMX_GPIO_NR(1,  1),
	GPIO_1				= IMX_GPIO_NR(4,  5),
	GPIO_2				= IMX_GPIO_NR(1,  3),
	GPIO_3				= IMX_GPIO_NR(1,  4),
	GPIO_4				= IMX_GPIO_NR(2, 23),
	GPIO_5				= IMX_GPIO_NR(2, 24),
	GPIO_6				= IMX_GPIO_NR(3, 19),
	GPIO_7				= IMX_GPIO_NR(3, 23),
	GPIO_8				= IMX_GPIO_NR(3, 24),
	GPIO_9				= IMX_GPIO_NR(3, 25),
	GPIO_10				= IMX_GPIO_NR(3, 29),
	GPIO_11				= IMX_GPIO_NR(3, 31),
	GPIO_RESET			= IMX_GPIO_NR(6,  8),
	GPIO_MCLK			= IMX_GPIO_NR(1,  2),
	GPIO_CHARGER_PRSNT	= IMX_GPIO_NR(1,  7),
	GPIO_CHARGING		= IMX_GPIO_NR(1,  8),
	GPIO_PMIC_INT_B		= IMX_GPIO_NR(7, 13),
	GPIO_CARRIER_PWR_ON	= IMX_GPIO_NR(6, 31),
	GPIO_RGMII_RESET_LOGISNI8 = IMX_GPIO_NR(1, 25)
};

// Enum for AFB_GPIOs[0-7] on the Logosni8 board
enum AFB_GPIOS {
	AFB_GPIO_0				= IMX_GPIO_NR(5, 19),
	AFB_GPIO_1				= IMX_GPIO_NR(5, 18),
	AFB_GPIO_2				= IMX_GPIO_NR(5, 21),
	AFB_GPIO_3				= IMX_GPIO_NR(5, 20),
	AFB_GPIO_4				= IMX_GPIO_NR(5, 22),
	AFB_GPIO_5				= IMX_GPIO_NR(5, 23),
	AFB_GPIO_6				= IMX_GPIO_NR(5, 24),
	AFB_GPIO_7				= IMX_GPIO_NR(5, 25)
};

// Enum for SD_GPIOs on the Logosni8 board
enum SD_GPIOS {
	SDIO_CD				= IMX_GPIO_NR(6, 14),
	SDIO_WP				= IMX_GPIO_NR(6, 15),
	SDIO_PWR_EN			= IMX_GPIO_NR(6, 16)
};


DECLARE_GLOBAL_DATA_PTR;
#define GP_USB_OTG_PWR	IMX_GPIO_NR(3, 22)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define ENET_PAD_CTRL_PD  (PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define ENET_PAD_CTRL_CLK  ((PAD_CTL_PUS_100K_UP & ~PAD_CTL_PKE) | \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define BUTTON_PAD_CTRL (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define RGB_PAD_CTRL	PAD_CTL_DSE_120ohm

#define WEAK_PULLUP	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_SRE_SLOW)

#define WEAK_PULLDOWN	(PAD_CTL_PUS_100K_DOWN |		\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

#define OUTPUT_40OHM (PAD_CTL_SPEED_MED|PAD_CTL_DSE_40ohm)

// Added a define for the Watchdog Padding - from freescale
#define WDOG_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_PKE | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_40ohm)

/* Prevent compiler error if gpio number 08 or 09 is used */
#define not_octal(gp) ((((0x##gp >> 4) & 0xf) * 10) + ((0x##gp & 0xf)))

#define _I2C_PADS_INFO_CPU(cpu, i2cnum, scl_pad, scl_bank, scl_gp,	       \
		sda_pad, sda_bank, sda_gp, pad_ctrl, join_io) {		       \
	.scl = {							       \
		.i2c_mode = NEW_PAD_CTRL(cpu##_PAD_##scl_pad##__##i2cnum##_SCL,\
					 pad_ctrl),			       \
		.gpio_mode = NEW_PAD_CTRL(				       \
			cpu##_PAD_##scl_pad##__GPIO##scl_bank##join_io##scl_gp,\
			pad_ctrl),					       \
		.gp = IMX_GPIO_NR(scl_bank, not_octal(scl_gp))		       \
	},								       \
	.sda = {							       \
		.i2c_mode = NEW_PAD_CTRL(cpu##_PAD_##sda_pad##__##i2cnum##_SDA,\
					 pad_ctrl),			       \
		.gpio_mode = NEW_PAD_CTRL(				       \
			cpu##_PAD_##sda_pad##__GPIO##sda_bank##join_io##sda_gp,\
			pad_ctrl),					       \
			.gp = IMX_GPIO_NR(sda_bank, not_octal(sda_gp))	       \
	}								       \
}

#define I2C_PADS_INFO_CPU(cpu, i2cnum, scl_pad, scl_bank, scl_gp,	       \
		sda_pad, sda_bank, sda_gp, pad_ctrl)			       \
		_I2C_PADS_INFO_CPU(cpu, i2cnum, scl_pad, scl_bank, scl_gp,     \
				sda_pad, sda_bank, sda_gp, pad_ctrl, _IO)

#define I2C_PADS_INFO_ENTRY(i2cnum, scl_pad, scl_bank, scl_gp,		\
		sda_pad, sda_bank, sda_gp, pad_ctrl)			\
	I2C_PADS_INFO_CPU(MX6, i2cnum, scl_pad, scl_bank, scl_gp,	\
		sda_pad, sda_bank, sda_gp, pad_ctrl)
#define I2C_PADS_INFO_ENTRY_SPACING 1

#define IOMUX_PAD_CTRL(name, pad_ctrl) NEW_PAD_CTRL(MX6_PAD_##name, pad_ctrl)

int dram_init(void)
{
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);

	return 0;
}

/* Configuration of UART2 for Logosni8 */
static iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PAD_CTRL(EIM_D26__UART2_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D27__UART2_RX_DATA, UART_PAD_CTRL),
};

/* Configuration of UART4 for Logosni8 */
static iomux_v3_cfg_t const uart4_pads[] = {
	IOMUX_PAD_CTRL(CSI0_DAT12__UART4_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT13__UART4_RX_DATA, UART_PAD_CTRL),
	// Configuring CTS and RTSl
	IOMUX_PAD_CTRL(CSI0_DAT16__UART4_RTS_B, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT17__UART4_CTS_B, UART_PAD_CTRL),
};

/* Configuration of UART5 for Logosni8 */
static iomux_v3_cfg_t const uart5_pads[] = {
	IOMUX_PAD_CTRL(CSI0_DAT14__UART5_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT15__UART5_RX_DATA, UART_PAD_CTRL),
	// Configuring CTS and RTS
	IOMUX_PAD_CTRL(CSI0_DAT18__UART5_RTS_B, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT19__UART5_CTS_B, UART_PAD_CTRL),
};

#ifdef CONFIG_CMD_I2C
static struct i2c_pads_info i2c_pads[] = {
    I2C_PADS_INFO_ENTRY(I2C2, KEY_COL3, 4, 12, KEY_ROW3, 4, 13, I2C_PAD_CTRL),
	// Not used I2C_PADS_INFO_ENTRY(I2C3, GPIO_5, 1, 05, GPIO_16, 7, 11, I2C_PAD_CTRL),
	I2C_PADS_INFO_ENTRY(I2C4, ENET_TX_EN, 1, 28, ENET_TXD1, 1, 29, I2C_PAD_CTRL),
};
#endif

#define I2C_BUS_CNT 2

// HDMI Reset Pad Config
static iomux_v3_cfg_t const hdmi_reset_pads[] = {
	IOMUX_PAD_CTRL(NANDF_D0__GPIO2_IO00, WEAK_PULLUP),
};

// Logosni8 - Map the onboard eMMC
static iomux_v3_cfg_t const usdhc4_pads[] = {
	IOMUX_PAD_CTRL(SD4_CLK__SD4_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_CMD__SD4_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT0__SD4_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT1__SD4_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT2__SD4_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT3__SD4_DATA3, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT4__SD4_DATA4, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT5__SD4_DATA5, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT6__SD4_DATA6, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD4_DAT7__SD4_DATA7, USDHC_PAD_CTRL),
};
//Logosni8 - Map the SD CARD on the Test Carrier Board
static iomux_v3_cfg_t const sdmmc_pads[] = {
	IOMUX_PAD_CTRL(SD1_CLK__SD1_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_CMD__SD1_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT0__SD1_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT1__SD1_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT2__SD1_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD1_DAT3__SD1_DATA3, USDHC_PAD_CTRL),
	// Map GPIO to enable SD CARD
	IOMUX_PAD_CTRL(NANDF_CS3__GPIO6_IO16, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NANDF_CS2__GPIO6_IO15, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NANDF_CS1__GPIO6_IO14, WEAK_PULLUP),
};
// Logosni8 - Map eMMC on Test Carrier
static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PAD_CTRL(SD3_CLK__SD3_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_CMD__SD3_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_RST__SD3_RESET, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT0__SD3_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT1__SD3_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT2__SD3_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT3__SD3_DATA3, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT4__SD3_DATA4, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT5__SD3_DATA5, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT6__SD3_DATA6, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD3_DAT7__SD3_DATA7, USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads1[] = {
	/* MDIO */
	IOMUX_PAD_CTRL(ENET_MDIO__ENET_MDIO, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_MDC__ENET_MDC, ENET_PAD_CTRL),

	/* RGMII */
	IOMUX_PAD_CTRL(RGMII_TXC__RGMII_TXC, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD0__RGMII_TD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD1__RGMII_TD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD2__RGMII_TD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD3__RGMII_TD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TX_CTL__RGMII_TX_CTL, ENET_PAD_CTRL),

	/* GPIO16 -> AR8035 25MHz */
	IOMUX_PAD_CTRL(GPIO_16__ENET_REF_CLK , NO_PAD_CTRL),

	/* Reference Clock */
	IOMUX_PAD_CTRL(ENET_REF_CLK__ENET_TX_CLK, ENET_PAD_CTRL_CLK),

	/* First use these pins to config the AR8035 to the correct mode
	* Should be in the RGMII, PLLON , INT mode - meaning Mode[3..0] = 1110  */
	/* pin 31 - RX_CLK */
	IOMUX_PAD_CTRL(RGMII_RXC__GPIO6_IO30, WEAK_PULLUP),
	/* pin 29 - Value: 0 - PHY ADDDRES0 */
	IOMUX_PAD_CTRL(RGMII_RD0__GPIO6_IO25, WEAK_PULLDOWN),
	/* pin 28 - Value: 0 - PHY ADDDRES1 */
	IOMUX_PAD_CTRL(RGMII_RD1__GPIO6_IO27, WEAK_PULLDOWN),
	/* pin 26 - Value: 1 - (MODE1) all */
	IOMUX_PAD_CTRL(RGMII_RD2__GPIO6_IO28, WEAK_PULLUP),
	/* pin 25 - Value: 1 - (MODE3) all */
	IOMUX_PAD_CTRL(RGMII_RD3__GPIO6_IO29, WEAK_PULLUP),
	/* pin 30 - Value: 0 - (MODE0) all */
	IOMUX_PAD_CTRL(RGMII_RX_CTL__GPIO6_IO24, WEAK_PULLUP),
	/* pin 1 PHY nRST */
	IOMUX_PAD_CTRL(ENET_CRS_DV__GPIO1_IO25, WEAK_PULLUP),
	/* Interrupt pin */
	IOMUX_PAD_CTRL(ENET_RXD0__GPIO1_IO27, ENET_PAD_CTRL),
};
/* Ethernet Pad Initialisation for Logosni8 */
static iomux_v3_cfg_t const enet_pads2[] = {
	IOMUX_PAD_CTRL(RGMII_RXC__RGMII_RXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, ENET_PAD_CTRL_PD),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, ENET_PAD_CTRL_PD),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, ENET_PAD_CTRL_PD),
};

static iomux_v3_cfg_t const ni8_boot_flags[] = {
	IOMUX_PAD_CTRL(EIM_DA0__GPIO3_IO00, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA1__GPIO3_IO01, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA2__GPIO3_IO02, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA3__GPIO3_IO03, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA4__GPIO3_IO04, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA5__GPIO3_IO05, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA6__GPIO3_IO06, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA7__GPIO3_IO07, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA8__GPIO3_IO08, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA9__GPIO3_IO09, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA10__GPIO3_IO10, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA11__GPIO3_IO11, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA12__GPIO3_IO12, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA13__GPIO3_IO13, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA14__GPIO3_IO14, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_DA15__GPIO3_IO15, NO_PAD_CTRL),

};

/* LED2 and LED3 pads on logosni8 */
static iomux_v3_cfg_t const ni8_led_pads[] = {

	/* Configuration of LED1 (GREEN)  - On the board this is LED2 - see schematic page 10 */
	IOMUX_PAD_CTRL(NANDF_CLE__GPIO6_IO07, OUTPUT_40OHM),  // - Configured as output 40Ohms

	/* Configuration of LED2 (GREEN)  - On the board this is LED3 - see schematic page 10
	 * Here it should be noted that there might be an small error on the schematic, where this is called NANDF_WP
	 * But should be NANDF_WP_B as on the nitrogen 6 lite schematic page 2
	 */
	IOMUX_PAD_CTRL(NANDF_WP_B__GPIO6_IO09, OUTPUT_40OHM), // - Configured as output 40Ohm
};

#ifdef CONFIG_CMD_I2C 		// Added for Logosni8 Testing
/* I2C Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_i2c_pads[] = {
	// Pin configuration for I2C

	/* Configuration of GPIO_5 to I2C3_SCL - Here called I2C3_SCL in schematic - see schematic page 10  - Here the I2C pad is used*/
	IOMUX_PAD_CTRL(GPIO_5__I2C3_SCL, I2C_PAD_CTRL),
	/* Configuration of GPIO_6 to I2C3_SDA - Here called I2C3_SDA in schematic - see schematic page 10  - Here the I2C pad is used*/
	IOMUX_PAD_CTRL(GPIO_6__I2C3_SDA, I2C_PAD_CTRL),

	//new I2C bus for the EEPROM
	IOMUX_PAD_CTRL(KEY_ROW3__I2C2_SDA, I2C_PAD_CTRL),
	/* see schematic page 10  - Here the I2C pad is used*/
	IOMUX_PAD_CTRL(KEY_COL3__I2C2_SCL, I2C_PAD_CTRL),

	
	IOMUX_PAD_CTRL(ENET_TX_EN__I2C4_SCL, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_TXD1__I2C4_SDA, I2C_PAD_CTRL),

};
#endif

/* WatchDog Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_wdog_pads[] = {
	// Pin configuration for the Watchdog

	/* Configuration of GPIO_9 to WDOG1_B - Here called WDOG1_B in schematic  - see schematic page 10 */
	// TODO: Make sure the the watch dog initialisation doesnt reset the device - goes to timeout on TC
	IOMUX_PAD_CTRL(GPIO_9__WDOG1_B, WDOG_PAD_CTRL),
};

#ifdef CONFIG_USB		// Added for Logosni8 Testing
/* USB Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_usb_pads[] = {
	// Pin configuration for USB

	//USB1:
	IOMUX_PAD_CTRL(EIM_D30__USB_H1_OC, WEAK_PULLUP),
	/* Configuration of GPIO_0 to USB_H1_PWR - Here called USB_1_PWREN in schematic - see schematic page 10 - The Same padding is used for USB on Nitrogen */
	IOMUX_PAD_CTRL(GPIO_0__USB_H1_PWR, WEAK_PULLDOWN),

	//USB_Micro:
	IOMUX_PAD_CTRL(ENET_RX_ER__USB_OTG_ID, WEAK_PULLUP),
	IOMUX_PAD_CTRL(KEY_ROW4__USB_OTG_PWR, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(KEY_COL4__USB_OTG_OC, WEAK_PULLUP),

};
#endif
/*
 * Pin Configurations have been done such at all pins have a start configuration - such that none of them are floating
 * This idea comes from Henning.
 *
 * Secondarily, All inputs that have to read a low values should be pulled up and vice versa.
 * Third rule, All output signals need to be configured either pull up or pull down, if not done on the schematic
 * Otherwise, simply set no pad setting or 40Ohm.
 */
/* GPIO Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_gpio_pads[] = {

	// Pin configuration for GPIO[0-3]

	/* Configuration of GPIO_1 to GPIO1_IO01 - Here called GPIO0 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(GPIO_1__GPIO1_IO01, WEAK_PULLDOWN),	// Output for S_D_INT on TC - disabled on startup
	/* Configuration of GPIO_3 to GPIO1_IO03 - Here called GPIO1 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(GPIO_3__GPIO1_IO03, WEAK_PULLUP),	// Output for Audio_AMP_EN on TC disable Audio amp on startup
	/* Configuration of GPIO_19 to GPIO4_IO05 - Here called GPIO2 on schematic- see schematic page 10 */
	IOMUX_PAD_CTRL(GPIO_19__GPIO4_IO05, OUTPUT_40OHM),	// Output for SOUND2 on TC - Pulled up with hardware- therefore either as 40Ohm or no pad setting
	/* Configuration of GPIO_4 to GPIO1_IO04 - Here called GPIO3 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(GPIO_4__GPIO1_IO04, OUTPUT_40OHM), 	// Output for SOUND1 on TC- Pulled up with hardware- therefore either as 40Ohm or no pad setting

	// Pin configuration for GPIO[4-11]

	/* Configuration of EIM_CS0 to GPIO2_IO23 - Here called GPIO4 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_CS0__GPIO2_IO23, WEAK_PULLUP), // Used as input for Audio_IRQ on TC - active low - meaning 0 activates it.
	/* Configuration of EIM_CS1 to GPIO2_IO24 - Here called GPIO5 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_CS1__GPIO2_IO24, WEAK_PULLDOWN), // SMART_IO on TC - Output
	/* Configuration of EIM_D19 to GPIO3_IO19 - Here called GPIO6 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_D19__GPIO3_IO19, WEAK_PULLDOWN), // SMART_INT on TC - Input - pulled high - therefore set to pull  down
	/* Configuration of EIM_D23 to GPIO3_IO23 - Here called GPIO7 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_D23__GPIO3_IO23, WEAK_PULLDOWN), // Used as GPIO - set low to have no floating pins
	/* Configuration of EIM_D24 to GPIO3_IO24 - Here called GPIO8 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_D24__GPIO3_IO24, WEAK_PULLDOWN), // Used as GPIO - set low to have no floating pins
	/* Configuration of EIM_D25 to GPIO3_IO25 - Here called GPIO9 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_D25__GPIO3_IO25, WEAK_PULLDOWN), // Used as GPIO - set low to have no floating pins
	/* Configuration of EIM_D29 to GPIO3_IO29 - Here called GPIO10 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_D29__GPIO3_IO29, WEAK_PULLDOWN), // Used as GPIO - set low to have no floating pins
	/* Configuration of EIM_D31 to GPIO3_IO31 - Here called GPIO11 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_D31__GPIO3_IO31, WEAK_PULLDOWN), // Used as GPIO - set low to have no floating pins


	// Pin Configuration of GPIO_MCLK

	/* Configuration of GPIO_2 to GPIO1_IO02 - Here called GPIO_MCLK on schematic - see schematic page 10 */
	// Here This GPIO controls a low frequency Audio clock for the chip MAX9860ETG+T on the TC (Minimum 10MHz clock) TODO: Check if we can generate this high clk
	IOMUX_PAD_CTRL(GPIO_2__GPIO1_IO02, OUTPUT_40OHM),

	// Pin Configuration of GPIO_RESET

	/* Configuration of GPIO_1 to GPIO6_IO08 - Here called GPIO_RESET in schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(NANDF_ALE__GPIO6_IO08, OUTPUT_40OHM),	// Output - Pulled up by hardware, therefore no padding is needed - here just set to 40 Ohm

	// Pin configuration for SMARC inputs - Charging and Charger_PRSNT

	/* Configuration of GPIO_7 to GPIO1_IO07 - Here called Charger_PRSNT# in schematic (This is pull up in the schematic) - see schematic page 10 */

	IOMUX_PAD_CTRL(GPIO_7__GPIO1_IO07, WEAK_PULLUP),	// Set to pull up, because it has to read a low value on TC
	/* Configuration of GPIO_8 to GPIO1_IO08 - Here called Charging# in schematic (This is pull up in the schematic) - see schematic page 10 */

	IOMUX_PAD_CTRL(GPIO_8__GPIO1_IO08, WEAK_PULLUP),	// Set to pull up, because it has to read a low value on TC

	// Pin configuration for SMARC inputs - PMIC_INT_B

	/* Configuration of GPIO_18 to GPIO7_IO13 - Here called PMIC_INIT_B in schematic  - see schematic page 10 */
	IOMUX_PAD_CTRL(GPIO_18__GPIO7_IO13, NO_PAD_CTRL), // Input - Pulled high by the hardware - therefore no pad control

	// Pin configuration for SMARC inputs - CARRIER_PWR_ON

	/* Configuration of EIM_BCLK to GPIO6_IO31 - Here called CARRIER_PWR_ON in schematic  - see schematic page 10 */
	IOMUX_PAD_CTRL(EIM_BCLK__GPIO6_IO31, WEAK_PULLDOWN), // Disable the 4g module carrier at startup on TC
};

/* AFB_GPIO Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_afb_gpio_pads[] = {

	// Pin configuration for AFB_GPIO[0-7]

	/* Configuration of CSI0_MCLK to GPIO5_IO19 - Here called AFB_GPIO0 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_MCLK__GPIO5_IO19, WEAK_PULLDOWN),
	/* Configuration of CSI0_PIXCLK to GPIO5_IO18 - Here called AFB_GPIO1 on schematic  - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_PIXCLK__GPIO5_IO18, WEAK_PULLDOWN),
	/* Configuration of CSI0_VSYNC to GPIO5_IO21 - Here called AFB_GPIO2 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_VSYNC__GPIO5_IO21, WEAK_PULLDOWN),
	/* Configuration of CSI0_DATA_EN to GPIO5_IO20 - Here called AFB_GPIO3 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_DATA_EN__GPIO5_IO20, WEAK_PULLDOWN), // Give initial value - no floating values
	/* Configuration of CSI0_DAT4 to GPIO5_IO22 - Here called AFB_GPIO4 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_DAT4__GPIO5_IO22, WEAK_PULLDOWN),  // Controls LED6 on the Test Carrier - Connected to mosfet - pull down to set to zero(LED OFF)
	/* Configuration of CSI0_DAT5 to GPIO5_IO23 - Here called AFB_GPIO5 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_DAT5__GPIO5_IO23, WEAK_PULLDOWN), // Controls LED5 on the Test Carrier	- Connected to mosfet - pull down to set to zero(LED OFF)
	/* Configuration of CSI0_DAT6 to GPIO5_IO24 - Here called AFB_GPIO6 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_DAT6__GPIO5_IO24, WEAK_PULLDOWN), // Controls LED4 on the Test Carrier	- Connected to mosfet - pull down to set to zero(LED OFF)
	/* Configuration of CSI0_DAT7 to GPIO5_IO25 - Here called AFB_GPIO07 on schematic - see schematic page 10 */
	IOMUX_PAD_CTRL(CSI0_DAT7__GPIO5_IO25, WEAK_PULLDOWN), // Controls LED3 on the Test Carrier	- Connected to mosfet - pull down to set to zero(LED OFF)
};

// Setup the GPIO pins on the Logosni8 board
static void setup_iomux_gpio(void)
{
	// Add a GPIO request for all the GPIOs - without requesting a gpio the driver will not let us use the GPIOs
	gpio_request(GPIO_0,				"GPIO_0");
	gpio_request(GPIO_1,				"GPIO_1");
	gpio_request(GPIO_2,				"GPIO_2");
	gpio_request(GPIO_3,				"GPIO_3");
	gpio_request(GPIO_4,				"GPIO_4");
	gpio_request(GPIO_5,				"GPIO_5");
	gpio_request(GPIO_6,				"GPIO_6");
	gpio_request(GPIO_7,				"GPIO_7");
	gpio_request(GPIO_8,				"GPIO_8");
	gpio_request(GPIO_9,				"GPIO_9");
	gpio_request(GPIO_10,				"GPIO_10");
	gpio_request(GPIO_11,				"GPIO_11");
	gpio_request(GPIO_RESET,			"GPIO_RESET");
	gpio_request(GPIO_MCLK,				"GPIO_MCLK");
	gpio_request(GPIO_CHARGING,			"GPIO_CHARGING");
	gpio_request(GPIO_PMIC_INT_B, 		"GPIO_PMIC_INT_B");
	gpio_request(GPIO_CHARGER_PRSNT,	"GPIO_CHARGER_PRSNT");
	gpio_request(GPIO_CARRIER_PWR_ON,	"GPIO_CARRIER_PWR_ON");

	// Setup the rest of the GPIO pins and the corresponding padding for the i.MX6U -
	SETUP_IOMUX_PADS(conf_gpio_pads);

	// Setup the GPIOs as Input if specified on the Schematic and Test Carrier board
	gpio_direction_input(GPIO_CHARGER_PRSNT);			// CHARGER_PRNST#
	gpio_direction_input(GPIO_CHARGING);				// CHARGING#
	gpio_direction_input(GPIO_PMIC_INT_B);				// PMIC_INT_B
	gpio_direction_input(GPIO_4);						// GPIO_4 -> AUDIO_IRQ
	gpio_direction_input(GPIO_7);						// GPIO_7 -> SMART_INT_1V8 -> SMART_INT

	// Setup the GPIOs as Output if specified on the Schematic and Test Carrier board
	gpio_direction_output(GPIO_CARRIER_PWR_ON, 1);		// Carrier_PWR_ON
	gpio_direction_output(GPIO_MCLK, 0);				// GPIO_MCLK
	gpio_direction_output(GPIO_RESET, 0);				// GPIO_RESET
	gpio_direction_output(GPIO_0, 0);					// GPIO_0 -> S_D_INT
	gpio_direction_output(GPIO_1, 0);					// GPIO_1 -> AUDIO_AMP_EN
	gpio_direction_output(GPIO_2, 0);					// GPIO_2 -> SOUND2
	gpio_direction_output(GPIO_3, 0);					// GPIO_3 -> SOUND1

	// After setting up the GPIOs - Set one LED on and one off, to signal how fare the bootup is.
	gpio_set_value(GPIO_LED_2, 0);
	gpio_set_value(GPIO_LED_3, 1);
	gpio_set_value(GPIO_CARRIER_PWR_ON, 1);
};
// Setup AFB_GPIOs - which goes to AFB[0-7] on the SMARC interface- Are mapped to GPIOs on the Test Carrier board
static void setup_iomux_afb_gpio(void)
{
	// Add a GPIO request for all the GPIOs - without requesting a gpio the driver will not let us use the GPIOs
	gpio_request(AFB_GPIO_0,				"AFB_GPIO_0");
	gpio_request(AFB_GPIO_1,				"AFB_GPIO_1");
	gpio_request(AFB_GPIO_2,				"AFB_GPIO_2");
	gpio_request(AFB_GPIO_3,				"AFB_GPIO_3");
	gpio_request(AFB_GPIO_4,				"AFB_GPIO_4");
	gpio_request(AFB_GPIO_5,				"AFB_GPIO_5");
	gpio_request(AFB_GPIO_6,				"AFB_GPIO_6");
	gpio_request(AFB_GPIO_7,				"AFB_GPIO_7");

	// Setup the rest of the AFB_GPIO pins and the corresponding padding for the i.MX6U -
	SETUP_IOMUX_PADS(conf_afb_gpio_pads);

	// Setup the AFB GPIOs as Output if specified on the Schematic - Also configured for the Test Carrier
	gpio_direction_output(AFB_GPIO_4, 1);			// AFB_GPIO_4 -> LED6 on the Test Carrier Board
	gpio_direction_output(AFB_GPIO_5, 1);			// AFB_GPIO_5 -> LED5 on the Test Carrier Board
	gpio_direction_output(AFB_GPIO_6, 1);			// AFB_GPIO_6 -> LED4 on the Test Carrier Board
	gpio_direction_output(AFB_GPIO_7, 0);			// AFB_GPIO_7 -> LED3 on the Test Carrier Board
}
// Setup the LEDS on the Logosni8 board
static void setup_iomux_leds(void)
{
	// Add a GPIO request for the two LEDS
	gpio_request(GPIO_LED_2, "GPIO_LED_2");
	gpio_request(GPIO_LED_3, "GPIO_LED_3");

	// Setup the LEDS and the corresponding padding
	SETUP_IOMUX_PADS(ni8_led_pads);

	// Setup the LEDs as Output
	gpio_direction_output(GPIO_LED_2, 1);			// LED2
	gpio_direction_output(GPIO_LED_3, 0);			// LED3
};
static void setup_iomux_boot_config(void)
{
	// Add a GPIO request for the Bootconfigs
	gpio_request(GPIO_EIM_DA0, "GPIO_EIM_DA0");
	gpio_request(GPIO_EIM_DA1, "GPIO_EIM_DA1");
	gpio_request(GPIO_EIM_DA2, "GPIO_EIM_DA2");
	gpio_request(GPIO_EIM_DA3, "GPIO_EIM_DA3");
	gpio_request(GPIO_EIM_DA4, "GPIO_EIM_DA4");
	gpio_request(GPIO_EIM_DA5, "GPIO_EIM_DA5");
	gpio_request(GPIO_EIM_DA6, "GPIO_EIM_DA6");
	gpio_request(GPIO_EIM_DA7, "GPIO_EIM_DA7");
	gpio_request(GPIO_EIM_DA8, "GPIO_EIM_DA8");
	gpio_request(GPIO_EIM_DA9, "GPIO_EIM_DA9");
	gpio_request(GPIO_EIM_DA10, "GPIO_EIM_DA10");
	gpio_request(GPIO_EIM_DA11, "GPIO_EIM_DA11");
	gpio_request(GPIO_EIM_DA12, "GPIO_EIM_DA12");
 	gpio_request(GPIO_EIM_DA13, "GPIO_EIM_DA13");
	gpio_request(GPIO_EIM_DA14, "GPIO_EIM_DA14");
	gpio_request(GPIO_EIM_DA15, "GPIO_EIM_DA15");

	// Setup the LEDS and the corresponding padding
	SETUP_IOMUX_PADS(ni8_boot_flags);

	// Setup the boot configs as input
	gpio_direction_input(GPIO_EIM_DA0);
	gpio_direction_input(GPIO_EIM_DA1);
	gpio_direction_input(GPIO_EIM_DA2);
	gpio_direction_input(GPIO_EIM_DA3);
	gpio_direction_input(GPIO_EIM_DA4);
	gpio_direction_input(GPIO_EIM_DA5);
	gpio_direction_input(GPIO_EIM_DA6);
	gpio_direction_input(GPIO_EIM_DA7);
	gpio_direction_input(GPIO_EIM_DA8);
	gpio_direction_input(GPIO_EIM_DA9);
	gpio_direction_input(GPIO_EIM_DA10);
	gpio_direction_input(GPIO_EIM_DA11);
	gpio_direction_input(GPIO_EIM_DA12);
	gpio_direction_input(GPIO_EIM_DA13);
	gpio_direction_input(GPIO_EIM_DA14);
	gpio_direction_input(GPIO_EIM_DA15);

};

static void setup_iomux_enet(void)
{
	gpio_request(GPIO_RGMII_RESET_LOGISNI8, "GPIO_RGMII_RESET_LOGISNI8");
	gpio_request(GPIO_RGMII_RX_DV,"GPIO_RGMII_RX_DV");
	gpio_request(GPIO_RGMII_RX_D0,"GPIO_RGMII_RX_D0");
	gpio_request(GPIO_RGMII_RX_D1,"GPIO_RGMII_RX_D1");
	gpio_request(GPIO_RGMII_RX_D2,"GPIO_RGMII_RX_D2");
	gpio_request(GPIO_RGMII_RX_D3,"GPIO_RGMII_RX_D3");
	gpio_request(GPIO_RGMII_RX_CLK,"GPIO_RGMII_RX_CLK");
	gpio_request(GPIO_ENET_RXD0_INT,"GPIO_ENET_RXD0_INT");

	// DO all the first mapping - GPIOs for Configuring the PHY and the AR8035 Mode
	SETUP_IOMUX_PADS(enet_pads1);

	// set Output for configuring AR8035
	gpio_direction_output(GPIO_RGMII_RESET_LOGISNI8, 0); /* Logosni8 PHY rst */

	// Setup the correct mode for Ethernet chip - AR8035 - Should be 1110 - see page 8 in the datasheet
	gpio_direction_output(GPIO_RGMII_RX_DV, 0);
	gpio_direction_output(GPIO_RGMII_RX_D0, 0);
	gpio_direction_output(GPIO_RGMII_RX_D1, 0);
	gpio_direction_output(GPIO_RGMII_RX_D2, 1);
	gpio_direction_output(GPIO_RGMII_RX_D3, 1);
	gpio_direction_output(GPIO_RGMII_RX_CLK, 1);  // low voltage - 1.5 0 and 1.8 is 1 - for 2.5V - PULL DOWN/PULL UP (Hardwired)
	//gpio_direction_output(GPIO_ENET_RXD0_INT, 1); // Active low - but is a output and is the interrupt pin.

	// For Debug Purpose Set the LED 2 and LED 3 LOW
	gpio_set_value(GPIO_LED_2, 0);
	gpio_set_value(GPIO_LED_3, 0);

	/* Need delay 2ms according to AR8035 spec - to make sure the clock is stable - logosni8 */
	mdelay(2);
	gpio_set_value(GPIO_RGMII_RESET_LOGISNI8, 1); /* Logosni8 PHY reset */

	// Turn LEDs off for debug
	gpio_set_value(GPIO_LED_2, 1);
	gpio_set_value(GPIO_LED_3, 1);

	mdelay(2); // This delay is used for testing - TODO: REMOVE This Delay when the Ethernet is working.
	SETUP_IOMUX_PADS(enet_pads2);
	mdelay(1);	/* Wait 1000 us before using mii interface - and pull the reset pin low */
}

#ifdef CONFIG_USB		// Added for Logosni8 Testing
static iomux_v3_cfg_t const usb_pads[] = {
	IOMUX_PAD_CTRL(GPIO_17__GPIO7_IO12, NO_PAD_CTRL),
};
#endif

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart2_pads);
	SETUP_IOMUX_PADS(uart4_pads);
	SETUP_IOMUX_PADS(uart5_pads);
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	SETUP_IOMUX_PADS(usb_pads);
	gpio_request(IMX_GPIO_NR(7, 12), "GPIO_RESET_USB_HUB");

	/* Reset USB hub */
	gpio_direction_output(IMX_GPIO_NR(7, 12), 0);
	mdelay(2);
	gpio_set_value(IMX_GPIO_NR(7, 12), 1);

	return 0;
}

int board_ehci_power(int port, int on)
{
	gpio_request(GP_USB_OTG_PWR, "GP_USB_OTG_PWR");
	if (port)
		return 0;
	gpio_set_value(GP_USB_OTG_PWR, on);
	return 0;
}

#endif

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(2, 30)) : -1;
}

static iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS1 */
	IOMUX_PAD_CTRL(EIM_EB2__GPIO2_IO30, NO_PAD_CTRL), /* -> BOOT_CFG_30 -> SPINOR_CS0 */
	IOMUX_PAD_CTRL(EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
};

static void setup_spi(void)
{
	SETUP_IOMUX_PADS(ecspi1_pads);
}
#endif

#define BMC_PDOWN 0x0800

static int ar8035_phy_fixup(struct phy_device *phydev)
{
	unsigned short val;

	/* from linux/arch/arm/mach-imx/mach-imx6q.c :
	 * Ar803x phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
 	*/

	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x805d);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4003);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val & ~(1 << 8));


	// Device 7 of the phy
	// Enable AR8035 to output a 125MHz clk from CLK_25M to IMX6 ENET_REF_CLK
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe7;
	val |= 0x18;
	if ( phy_write(phydev, MDIO_DEVAD_NONE, 0xe,val) < 0 )
		printf("Enabling the 125MHz Clk from CLK 25M failed.\n");


	// Introduce tx clock delay
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	if(phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val) < 0)
		printf("Enabling TX Clock Delay failed\n");

	// Introduce rx clock delay
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x0);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x8000;
	if (phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val) < 0)
		printf("Enabling RX Clock delay failed\n");

	// Introduce rgmii gtx clock delay - 3.4 ns - Default 2.4ns - TODO: Should be tweaked when the ethernet works - to get optimal performance
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0xB);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0060;
	if(phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val) < 0)
		printf("Enabling GTX delay of 3.4 ns failed\n");

	// Read all the values to verify that they are correctly written
	// Firstly reading the clock back.
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	ar8035_phy_fixup(phydev);

	printf("Initalised the AR8035\n");

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}


int board_eth_init(struct bd_info *bis)
{
	// struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	setup_iomux_enet();

	// Config environment variables
	//env_set("ethaddr", "00:19:b8:04:42:1b");


#ifdef CONFIG_FEC_MXC

	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;
	// scan phy 4,5,6,7
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		free(bus);
		return 0;
	}
	printf("Using phy at %d\n", phydev->addr);

	printf("Resetting the AR8035\n");

	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}

	// Use 125MHz anatop loopback REF_CLK1 for ENET0
	//clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC_MASK, 0);

	//enable_fec_anatop_clock(0, ENET_125MHZ);

#endif

#ifdef CONFIG_CI_UDC
	// For otg ethernet //
	usb_eth_initialize(bis);
#endif

	return ret;
}


#if defined(CONFIG_VIDEO_IPUV3)

static iomux_v3_cfg_t const backlight_pads[] = {
	/* Backlight on RGB connector: J15 */
	IOMUX_PAD_CTRL(SD1_DAT3__GPIO1_IO21, NO_PAD_CTRL),
#define RGB_BACKLIGHT_GP IMX_GPIO_NR(1, 21)

	/* Backlight on LVDS connector: J6 */
	IOMUX_PAD_CTRL(SD1_CMD__GPIO1_IO18, NO_PAD_CTRL),
#define LVDS_BACKLIGHT_GP IMX_GPIO_NR(1, 18)
};

static iomux_v3_cfg_t const rgb_pads[] = {
	IOMUX_PAD_CTRL(DI0_DISP_CLK__IPU1_DI0_DISP_CLK, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DI0_PIN15__IPU1_DI0_PIN15, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DI0_PIN2__IPU1_DI0_PIN02, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DI0_PIN3__IPU1_DI0_PIN03, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DI0_PIN4__GPIO4_IO20, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT0__IPU1_DISP0_DATA00, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT1__IPU1_DISP0_DATA01, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT2__IPU1_DISP0_DATA02, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT3__IPU1_DISP0_DATA03, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT4__IPU1_DISP0_DATA04, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT5__IPU1_DISP0_DATA05, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT6__IPU1_DISP0_DATA06, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT7__IPU1_DISP0_DATA07, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT8__IPU1_DISP0_DATA08, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT9__IPU1_DISP0_DATA09, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT10__IPU1_DISP0_DATA10, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT11__IPU1_DISP0_DATA11, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT12__IPU1_DISP0_DATA12, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT13__IPU1_DISP0_DATA13, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT14__IPU1_DISP0_DATA14, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT15__IPU1_DISP0_DATA15, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT16__IPU1_DISP0_DATA16, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT17__IPU1_DISP0_DATA17, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT18__IPU1_DISP0_DATA18, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT19__IPU1_DISP0_DATA19, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT20__IPU1_DISP0_DATA20, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT21__IPU1_DISP0_DATA21, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT22__IPU1_DISP0_DATA22, RGB_PAD_CTRL),
	IOMUX_PAD_CTRL(DISP0_DAT23__IPU1_DISP0_DATA23, RGB_PAD_CTRL),
};

static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

#ifdef CONFIG_CMD_I2C 		// Added for Logosni8 Testing
static int detect_i2c(struct display_info_t const *dev)
{
	return ((0 == i2c_set_bus_num(dev->bus))
		&&
		(0 == i2c_probe(dev->addr)));
}
#endif

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;
	u32 reg = readl(&iomux->gpr[2]);
	reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT;
	writel(reg, &iomux->gpr[2]);
	gpio_request(LVDS_BACKLIGHT_GP, "LVDS_BACKLIGHT_GP");
	gpio_direction_output(LVDS_BACKLIGHT_GP, 1);
}

static void enable_lvds_jeida(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;
	u32 reg = readl(&iomux->gpr[2]);
	reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	     |IOMUXC_GPR2_BIT_MAPPING_CH0_JEIDA;
	writel(reg, &iomux->gpr[2]);
	gpio_request(LVDS_BACKLIGHT_GP, "LVDS_BACKLIGHT_GP");
	gpio_direction_output(LVDS_BACKLIGHT_GP, 1);
}

static void enable_rgb(struct display_info_t const *dev)
{
	SETUP_IOMUX_PADS(rgb_pads);
	gpio_request(RGB_BACKLIGHT_GP, "RGB_BACKLIGHT_GP");
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
}

struct display_info_t const displays[] = {{
	.bus	= 2,
	.addr	= 0x70,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_i2c,
	.enable	= do_enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 0,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= NULL,
	.enable	= enable_lvds_jeida,
	.mode	= {
		.name           = "LDB-WXGA",
		.refresh        = 60,
		.xres           = 1280,
		.yres           = 800,
		.pixclock       = 14065,
		.left_margin    = 40,
		.right_margin   = 40,
		.upper_margin   = 3,
		.lower_margin   = 80,
		.hsync_len      = 10,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 0,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= NULL,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "LDB-WXGA-S",
		.refresh        = 60,
		.xres           = 1280,
		.yres           = 800,
		.pixclock       = 14065,
		.left_margin    = 40,
		.right_margin   = 40,
		.upper_margin   = 3,
		.lower_margin   = 80,
		.hsync_len      = 10,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 2,
	.addr	= 0x4,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "Hannstar-XGA",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 0,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= NULL,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "LG-9.7",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385, /* ~65MHz */
		.left_margin    = 480,
		.right_margin   = 260,
		.upper_margin   = 16,
		.lower_margin   = 6,
		.hsync_len      = 250,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 2,
	.addr	= 0x38,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "wsvga-lvds",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 600,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 2,
	.addr	= 0x10,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= detect_i2c,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "fusion7",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 33898,
		.left_margin    = 96,
		.right_margin   = 24,
		.upper_margin   = 3,
		.lower_margin   = 10,
		.hsync_len      = 72,
		.vsync_len      = 7,
		.sync           = 0x40000002,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 0,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= NULL,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "svga",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 600,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 2,
	.addr	= 0x41,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "amp1024x600",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 600,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 0,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= 0,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "wvga-lvds",
		.refresh        = 57,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 2,
	.addr	= 0x48,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= detect_i2c,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "wvga-rgb",
		.refresh        = 57,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 37037,
		.left_margin    = 40,
		.right_margin   = 60,
		.upper_margin   = 10,
		.lower_margin   = 10,
		.hsync_len      = 20,
		.vsync_len      = 10,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 0,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= NULL,
	.enable	= enable_rgb,
	.mode	= {
		.name           = "qvga",
		.refresh        = 60,
		.xres           = 320,
		.yres           = 240,
		.pixclock       = 37037,
		.left_margin    = 38,
		.right_margin   = 37,
		.upper_margin   = 16,
		.lower_margin   = 15,
		.hsync_len      = 30,
		.vsync_len      = 3,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

int board_cfb_skip(void)
{
	return NULL != env_get("novideo");
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();
	/* Turn on LDB0,IPU,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      |(3<<MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<<MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     |IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	     |IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     |IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     |IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     |IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	     |IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK
			|IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       <<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	/* backlights off until needed */
	SETUP_IOMUX_PADS(backlight_pads);
	gpio_request(RGB_BACKLIGHT_GP, "RGB_BACKLIGHT_GP");
	gpio_request(LVDS_BACKLIGHT_GP, "LVDS_BACKLIGHT_GP");
	gpio_direction_input(LVDS_BACKLIGHT_GP);
	gpio_direction_input(RGB_BACKLIGHT_GP);
}
#endif

#ifdef DEMO_MODE
static unsigned gpios_led_logosni8[] = {
	GPIO_LED_2, /* LED 2 - LogosNi8 */
	GPIO_LED_3, /* LED 3 - LogosNi8 */
};

// TODO: Remove this demo function before merging into the main branch
static void led_logosni8_party_light(void)
{
	// This function will create a simple light demo - using the LED2 and LED3 - will run for 20 seconds
	for (int i = 0; i < 30; i++) {
		gpio_set_value(GPIO_LED_2, 1);
		gpio_set_value(GPIO_LED_3, 1);

		// Wait 0.5s
		mdelay(500);

		gpio_set_value(GPIO_LED_2, 0);
		gpio_set_value(GPIO_LED_3, 0);

		// Wait 0.5s
		mdelay(500);
	}

	// After the initial heartbeat start the more serious stuff will initiate - Namely "Something - rename
	// Insert some more LED config here, to make a nice demo.
	gpio_set_value(GPIO_LED_2, 0);
	gpio_set_value(GPIO_LED_3, 1);
}
#endif

static int setup_fec(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	// Not removed for now - this is not needed because the clock is generated by the PHY

	/* set gpr1[21] to select anatop clock */
	//clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK,
	//				IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	/* Clear gpr1[ENET_CLK_SEL] for external clock  - see page 2032 in reference manual */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);
	//return enable_fec_anatop_clock(0, ENET_125MHZ);

	return 0;
}

// TODO: Some of the Initialisation needs to be moved to board_init()
int board_early_init_r(void)
{
	// Setup of UART2, UART4 and UART5
	setup_iomux_uart();

	// Setup early value initialisation - power up carrier board - set GPIO_CARRIER_PWR_ON high
	gpio_direction_output(IMX_GPIO_NR(6, 31), 1); // Doesnt set the Pin high early enough

	// Config environment variables
	env_set("ethact", "FEC");


#ifdef CONFIG_CMD_I2C		// Added for Logosni8 Testing
	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);
#endif


#ifdef CONFIG_USB		// Added for Logosni8 Testing
	// Early setup of USB
	SETUP_IOMUX_PADS(conf_usb_pads);
#endif

	// Early setup of Watchdog
	SETUP_IOMUX_PADS(conf_wdog_pads);

//#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
//#endif
	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int print_Logos_Logo(void)
{
	printf("\n");
	for(int h = 0; h < LOGOS_LOGO_ROWS; h++)
	{
		printf("%s\n", logosLogo[h]);
	}
	return 0;
}

/*
 * This function generate beeps using the Buzzer on the Test Carrier board and takes in two parameters
 * note     - This is the frequency of the requested note - Hz
 * duration - For how long should this tone be played     - ms
 *
 * The function generates a square wave that activates the buzzer
 */
int beep(int note, int duration)
{
	// Determine the Time Period
	int T_sound = 1000000 / note;
	// Determine the number of time periods to run to get the wanted duration.
	int runTime = (duration * 1000)/T_sound;

	for (int u = 0; u < runTime; u++) {
		gpio_direction_output(GPIO_2, 1);					// GPIO_2 -> SOUND2
		gpio_direction_output(GPIO_3, 0);					// GPIO_3 -> SOUND1
		udelay(T_sound >> 1);								// Divide by two

		gpio_direction_output(GPIO_2, 0);					// GPIO_2 -> SOUND2
		gpio_direction_output(GPIO_3, 1);					// GPIO_3 -> SOUND1
		udelay(T_sound >> 1);								// Divide by two
	}
	return 0;
}

int firstSection(void)
{
	beep(a,  500);
	beep(a,  500);
	beep(a,  500);
	beep(f,  350);
	beep(cH, 150);
	beep(a,  500);
	beep(f,  350);
	beep(cH, 150);
	beep(a,  650);

	mdelay(500);

	beep(eH, 500);
	beep(eH, 500);
	beep(eH, 500);
	beep(fH, 350);
	beep(cH, 150);
	beep(gS, 500);
	beep(f,  350);
	beep(cH, 150);
	beep(a,  650);

	mdelay(500);

	return 0;
}

int secondSection(void)
{
	beep(aH, 500);
	beep(a,  300);
	beep(a,  150);
	beep(aH, 500);
	beep(gSH,325);
	beep(gH, 175);
	beep(fSH,125);
	beep(fH, 125);
	beep(fSH,250);

	mdelay(325);

	beep(aS, 250);
	beep(dSH,500);
	beep(dH, 325);
	beep(cSH,175);
	beep(cH, 125);
	beep(b,  125);
	beep(cH, 250);

	mdelay(350);

	return 0;
}

int bootup_Song_Star_Wars(void)
{

	//Play first section
	firstSection();

	//Play second section
	secondSection();

	//Variant 1
	beep(f,  250);
	beep(gS, 500);
	beep(f,  350);
	beep(a,  125);
	beep(cH, 500);
	beep(a,  375);
	beep(cH, 125);
	beep(eH, 650);

    mdelay(500);

	//Repeat second section
	secondSection();

	//Variant 2
	beep(f,  250);
	beep(gS, 500);
	beep(f,  375);
	beep(cH, 125);
	beep(a,  500);
	beep(f,  375);
	beep(cH, 125);
	beep(a,  650);

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 8 bit bus width */
	{"sd1", MAKE_CFGVAL(0x42, 0x28, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc0", MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc1", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL, 0},
};
#endif /* CONFIG_CMD_BMODE */

static struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
		{USDHC1_BASE_ADDR}, /* SD Card Slot */
		{USDHC3_BASE_ADDR}, /* eMMC on Test Carrier */
		{USDHC4_BASE_ADDR}, /* eMMC on Nicore8 */
};

// Card detected function for seeing if a card is present
int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC1_BASE_ADDR)
		ret = !gpio_get_value(SDIO_CD);
	else
		ret = 1; // If it is not the SD card - it should be marked as present

	return ret;
}

int board_mmc_init(struct bd_info *bis) {
	/*
	  * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD3
	 * 0x3                  SD4
	 */

	// Configure Pins for eMMC
	SETUP_IOMUX_PADS(usdhc4_pads);

	// Configure Pins for eMMC on Test Carrier
	SETUP_IOMUX_PADS(usdhc3_pads);

	// Configure and Map Pins for SD Card on Test Carrier
	SETUP_IOMUX_PADS(sdmmc_pads);

	// Request GPIOs
	gpio_request(SDIO_PWR_EN, "SDIO_PWR_EN,");
	gpio_request(SDIO_WP, "SDIO_WP");
	gpio_request(SDIO_CD, "SDIO_CD");

	// Enable power to SDCARD
	gpio_direction_output(SDIO_PWR_EN, 1);
	gpio_direction_output(SDIO_WP, 1);
	gpio_direction_output(SDIO_CD, 1);

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	if(fsl_esdhc_initialize(bis, &usdhc_cfg[0]))
		puts("WARNING: failed to initialize SD\n");
	if(fsl_esdhc_initialize(bis, &usdhc_cfg[1]))
		puts("WARNING: failed to initialize eMMC on Test Carrier\n");
	if(fsl_esdhc_initialize(bis, &usdhc_cfg[2]))
		puts("WARNING: failed to initialize eMMC on Nicore8\n");

	return 0;
}

int board_mmc_init_dts() {
	/*
	  * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD2
	 * 0x3                  SD4
	 */

	// Configure Pins for eMMC
	SETUP_IOMUX_PADS(usdhc4_pads);

	// Configure Pins for eMMC on Test Carrier
	SETUP_IOMUX_PADS(usdhc3_pads);

	// Configure and Map Pins for SD Card on Test Carrier
	SETUP_IOMUX_PADS(sdmmc_pads);

	// Request GPIOs
	gpio_request(SDIO_PWR_EN, "SDIO_PWR_EN,");
	gpio_request(SDIO_WP, "SDIO_WP");
	gpio_request(SDIO_CD, "SDIO_CD");

	// Enable power to SDCARD
	gpio_direction_output(SDIO_PWR_EN, 1);
	gpio_direction_output(SDIO_WP, 1);
	gpio_direction_output(SDIO_CD, 1);

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	return 0;
}

int board_init(void)
{
	// First setting up the LED2 and LED3 on the Nicore8 for demo purposes
	setup_iomux_leds();

	// Setup of GPIOs
	setup_iomux_gpio();

	// Early setup of AFB_GPIOs - These are only valid for SMARC Version 1.1 - have changed with the new spec 2.1
	setup_iomux_afb_gpio();

	// Set Boot Configs as GPIOs - such that they can be validated with u-boot
	setup_iomux_boot_config();


	// Map HDMI Reset - I2c bus select
	SETUP_IOMUX_PADS(hdmi_reset_pads);
	// Set reset high for IC2 Bus select - Chpi is PCA954 - IC2 address 0x70 - (Reset is active low)
	gpio_request(GPIO_I2C_BUS_SEL_RESET, "GPIO_I2C_BUS_SEL_RESET ");
	// Set output high - reset disabled
	gpio_direction_output(GPIO_I2C_BUS_SEL_RESET, 1);


	// Setup Clocks for Ethernet
#if defined(CONFIG_FEC_MXC)
	setup_fec();
#endif

#if defined(CONFIG_OF_CONTROL)
	// Init mmc - Ontop of Device tree
	board_mmc_init_dts();
#endif

/*
#ifdef CONFIG_CMD_I2C
	// Setting up I2C
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct i2c_pads_info *p = i2c_pads;
	int i;
	int stride = 1;

	for (i = 0; i < I2C_BUS_CNT; i++)
	{
		setup_i2c(i, CONFIG_SYS_I2C_SPEED, 0x7f, p);
		p += stride;
	}
#endif
*/
/*
	// Setting up USB
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_OTG_ID_MASK, IOMUXC_GPR1_OTG_ID_GPIO1);

	SETUP_IOMUX_PADS(misc_pads);
*/
	/* address of boot parameters */
	//gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_MXC_SPI
    setup_spi();
#endif

#ifdef CONFIG_SATA
	setup_sata();
#endif

	return 0;
}

/*
void cpuinfo()
{
    if (is_cpu_type(MXC_CPU_MX6SOLO))
    {
        u32 rev = get_cpu_rev();
        u32 freq = get_cpu_speed_grade_hz();

        int min = 0;
        int max = 0;
        u32 grade = get_cpu_temp_grade(&min, &max);

        printf("CPU: 1 GHz i.MX 6Solo 1x ARM Cortex-A9 (rev: %d, speed grade: %d Hz, temp grade: %d to %d degree Celsius)\n", rev, freq, min, max);

        // CTP TODO: Actual speed? and uuid Nice features in cpu.h under imx seems not to work with imx6s
    }
    else
    {
        puts("CPU: Unknown\n");
    }
}
*/

/*
 * Simple function for printing the CPU information
 * This is hardcoded for now - Only on CPU using this bootloader.
 */
int print_cpuinfo(void)
{
	printf("CPU:   NXP MX6S Rev 5 with a ARM Cortex-A9 core running at 1 GHz - 512MB RAM\n");
	return 0;
}


int checkboard(void)
{
	printf("Board: NiCore8  \nDeveloped and Designed by Logos Payment Solutions\n\n\n");

	return 0;
}

int initial_Printing(void)
{
	print_Logos_Logo();
	print_cpuinfo();
	checkboard();

	return 0;
}

#ifdef CONFIG_PREBOOT
// Here was the setup of the Preboot keys for Nitrogen 6 - see board/boundary/nitrogen6x.c
#endif

int misc_init_r(void)
{

#ifdef CONFIG_PREBOOT
	//preboot_keys();
#endif

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	env_set_hex("reset_cause", get_imx_reset_cause());
	return 0;
}
/*
 * The board_late_init function is called late during the bootloader initialisation
 * Therefore, all the functionality needed late during the bootup should be added here - this is e.g. the UART printing
 * Which is first available late in the bootup, because the Test Carrier Board needs to be powered up.
 */
int board_late_init(void)
{
	// The test carrier board is now powered up and the UART is ready - make a startup screen
	initial_Printing();

	// TODO: Remove, can be read using u-boot command 'fuse read 0 1 and fuse read 0 2' or 'env print'
	const char* sn = env_get("serial#");
	if (sn)
	{
		printf("HW ID: %s\n", sn);
	}

	// Write commands to the PCA9546ABS to control the correct I2c bus
	//i2c_reg_write( 2, 0x002aab190, 0x01);

#ifdef DEMO_MODE
	// Boot up Song
	bootup_Song_Star_Wars();

	// This function creates a short demo of LED2 and LED3 on the Ni8 board - No udelay in board_early_init - use cpurelax()

	led_logosni8_party_light();
#endif
	return 0;
}
