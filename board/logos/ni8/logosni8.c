// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 */
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/spi.h>
#include <asm/mach-imx/video.h>
#include <command.h>
#include <common.h>
#include <env.h>
#include <fsl_esdhc_imx.h>
#include <i2c.h>
#include <init.h>
#include <input.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <usb/ehci-ci.h>
#include <version.h>

// Watchdog
#include <wdt.h>
#include <watchdog.h>
#include <fsl_wdog.h>
#include <div64.h>
#include <dm.h>

// Thermal Configs
#include <imx_thermal.h>
#include <thermal.h>

#include "logosLogo.h"

// Bootcount
#include <bootcount.h>

#ifdef DEMO_MODE
#include "bootmelody.h"
#endif // DEMO_MODE

#ifdef CONFIG_SPL_BUILD
//#define DRAM_INIT
#endif /* CONFIG_SPL_BUILD */


// ENUM for controlling the reset for I2c select for LCDs, HDMI, GP and CAM
enum I2C_RESET {
	GPIO_I2C_BUS_SEL_RESET = IMX_GPIO_NR(2, 0)
};
// ENUM for configuring the AR8035 ethernet adapter
#ifndef CONFIG_SPL_BUILD
enum AR8035_CONFIGS {
	GPIO_RGMII_RX_DV =   IMX_GPIO_NR(6, 24),
	GPIO_ENET_RXD0_INT = IMX_GPIO_NR(1, 27),
	GPIO_RGMII_RX_D0 =   IMX_GPIO_NR(6, 25),
	GPIO_RGMII_RX_D1 =   IMX_GPIO_NR(6, 27),
	GPIO_RGMII_RX_D2 =   IMX_GPIO_NR(6, 28),
	GPIO_RGMII_RX_D3 =   IMX_GPIO_NR(6, 29),
	GPIO_RGMII_RX_CLK =  IMX_GPIO_NR(6, 30)
};
#endif /* CONFIG_SPL_BUILD */

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

// Enum for GPIOs[0-11] on the LogosNi8 board
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
#define GP_USB1_PWR		IMX_GPIO_NR(1, 0)
#define GP_USB0_PWR		IMX_GPIO_NR(4, 15)
#define GP_TEST_SMARC	IMX_GPIO_NR(4, 9)

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

// Change driving strength
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII			0x20e0768
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM				0x20e0788
#define IOMUXC_ENET_REF_CLK_SELECT_INPUT				0x20e080C

// Enable Reference CLock
#define IOMUXC_ENET_REF_CLK_SELECT_INPUT_ENABLE_ENET_REF_CLK			0x00000001

/* disable on die termination for RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE		0x00000000
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_30OHMS	0x00000400
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_40OHMS	0x00000300
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_60OHMS	0x00000200
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_120OHMS	0x00000100
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_0OHMS	0x00000000
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_17OHMS  0x00000700
/* optimised drive strength for 1.0 .. 1.3 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P2V		0x00080000
/* optimised drive strength for 1.3 .. 2.5 V signal on RGMII */
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V		0x000C0000

/* Defines above, declarations below */

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
#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_MXC_SPI
static iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS1 */
	IOMUX_PAD_CTRL(EIM_EB2__GPIO2_IO30, NO_PAD_CTRL), /* -> BOOT_CFG_30 -> SPINOR_CS0 */
	IOMUX_PAD_CTRL(EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
};
#endif // CONFIG_MXC_SPI

// HDMI Reset Pad Config
static iomux_v3_cfg_t const hdmi_reset_pads[] = {
	IOMUX_PAD_CTRL(NANDF_D0__GPIO2_IO00, WEAK_PULLUP),
	IOMUX_PAD_CTRL(NANDF_D1__GPIO2_IO01, WEAK_PULLUP),
};
#endif /* CONFIG_SPL_BUILD */

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

static struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
		{USDHC4_BASE_ADDR}, /* eMMC on Nicore8 */
		{USDHC1_BASE_ADDR}, /* SD Card Slot */
		{USDHC3_BASE_ADDR}, /* eMMC on Test Carrier */
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

#ifndef CONFIG_SPL_BUILD
static iomux_v3_cfg_t const enet_pads1[] = {
	/* MDIO */
	IOMUX_PAD_CTRL(ENET_MDIO__ENET_MDIO, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_MDC__ENET_MDC, ENET_PAD_CTRL),

	/* RGMII */
	IOMUX_PAD_CTRL(RGMII_TXC__RGMII_TXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD0__RGMII_TD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD1__RGMII_TD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD2__RGMII_TD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD3__RGMII_TD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TX_CTL__RGMII_TX_CTL, ENET_PAD_CTRL),

	/* Reference Clock */
	IOMUX_PAD_CTRL(ENET_REF_CLK__ENET_TX_CLK, ENET_PAD_CTRL),

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
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, ENET_PAD_CTRL),
};
#endif /* CONFIG_SPL_BUILD */

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
	IOMUX_PAD_CTRL(NANDF_CLE__GPIO6_IO07, OUTPUT_40OHM),  // - Configured as output 40Ohms
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
	// Mapped to a GPIO as done for the nitrogen board  inorder to enable the usb port.
	//IOMUX_PAD_CTRL(GPIO_0__USB_H1_PWR, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(GPIO_0__GPIO1_IO00, WEAK_PULLUP),

	// The datalines UBS+/- can not be multiplexed - therefore not mapped

	//USB_Micro - OTG:
	IOMUX_PAD_CTRL(ENET_RX_ER__USB_OTG_ID, WEAK_PULLUP),
	//IOMUX_PAD_CTRL(KEY_ROW4__USB_OTG_PWR, WEAK_PULLDOWN),  // This have been changed to a gpio
	// USB 0 PWR Enable
	IOMUX_PAD_CTRL(KEY_ROW4__GPIO4_IO15, WEAK_PULLUP),
	// USB 0 Over Current
	IOMUX_PAD_CTRL(KEY_COL4__USB_OTG_OC, WEAK_PULLUP),
	// USB 0 VBus Detect
	IOMUX_PAD_CTRL(NANDF_CS0__NAND_CE0_B, WEAK_PULLUP),

};
#endif

/* GPIO Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_gpio_pads[] = {

	// Pin configuration for GPIO[0-3]
	IOMUX_PAD_CTRL(GPIO_1__GPIO1_IO01, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(GPIO_3__GPIO1_IO03, WEAK_PULLUP),
	IOMUX_PAD_CTRL(GPIO_19__GPIO4_IO05, OUTPUT_40OHM),
	IOMUX_PAD_CTRL(GPIO_4__GPIO1_IO04, OUTPUT_40OHM),

	// Pin configuration for GPIO[4-11]
	IOMUX_PAD_CTRL(EIM_CS0__GPIO2_IO23, WEAK_PULLUP),
	IOMUX_PAD_CTRL(EIM_CS1__GPIO2_IO24, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(EIM_D19__GPIO3_IO19, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(EIM_D23__GPIO3_IO23, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(EIM_D24__GPIO3_IO24, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(EIM_D25__GPIO3_IO25, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(EIM_D29__GPIO3_IO29, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(EIM_D31__GPIO3_IO31, WEAK_PULLDOWN),

	// Pin Configuration of GPIO_MCLK
	// Here This GPIO controls a low frequency Audio clock for the chip MAX9860ETG+T on the TC (Minimum 10MHz clock) TODO: Check if we can generate this high clk
	IOMUX_PAD_CTRL(GPIO_2__GPIO1_IO02, OUTPUT_40OHM),
	IOMUX_PAD_CTRL(NANDF_ALE__GPIO6_IO08, OUTPUT_40OHM),

	// Pin configuration for SMARC inputs - Charging and Charger_PRSNT
	IOMUX_PAD_CTRL(GPIO_7__GPIO1_IO07, WEAK_PULLUP),
	IOMUX_PAD_CTRL(GPIO_8__GPIO1_IO08, WEAK_PULLUP),

	// Pin configuration for SMARC inputs - PMIC_INT_B
	IOMUX_PAD_CTRL(GPIO_18__GPIO7_IO13, NO_PAD_CTRL),

	// Pin configuration for SMARC inputs - CARRIER_PWR_ON
	IOMUX_PAD_CTRL(EIM_BCLK__GPIO6_IO31, WEAK_PULLDOWN),

	// SMARC_Test from test carrier
	IOMUX_PAD_CTRL(KEY_ROW1__GPIO4_IO09, WEAK_PULLDOWN),
};

/* AFB_GPIO Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_afb_gpio_pads[] = {
	// Pin configuration for AFB_GPIO[0-7]
	IOMUX_PAD_CTRL(CSI0_MCLK__GPIO5_IO19, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_PIXCLK__GPIO5_IO18, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_VSYNC__GPIO5_IO21, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_DATA_EN__GPIO5_IO20, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_DAT4__GPIO5_IO22, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_DAT5__GPIO5_IO23, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_DAT6__GPIO5_IO24, WEAK_PULLDOWN),
	IOMUX_PAD_CTRL(CSI0_DAT7__GPIO5_IO25, WEAK_PULLDOWN),
};

#ifdef CONFIG_VIDEO_IPUV3
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
#endif // CONFIG_VIDEO_IPUV3

/* Functions below */
int dram_init(void)
{
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);

	return 0;
}

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
	gpio_direction_output(GPIO_CARRIER_PWR_ON, 	1);		// Carrier_PWR_ON
	gpio_direction_output(GPIO_MCLK, 			0);		// GPIO_MCLK
	gpio_direction_output(GPIO_RESET, 			0);		// GPIO_RESET - Reset Bluetooth Chip on Carrier Board
	gpio_direction_output(GPIO_0, 				0);		// GPIO_0 -> S_D_INT
	gpio_direction_output(GPIO_1, 				0);		// GPIO_1 -> AUDIO_AMP_EN
	gpio_direction_output(GPIO_2, 				0);		// GPIO_2 -> SOUND2
	gpio_direction_output(GPIO_3, 				0);		// GPIO_3 -> SOUND1

	// After setting up the GPIOs - Set one LED on and one off, to signal how fare the bootup is.
	gpio_set_value(GPIO_LED_2, 					0);
	gpio_set_value(GPIO_LED_3,					1);
	gpio_set_value(GPIO_CARRIER_PWR_ON, 		1);
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
	gpio_direction_output(AFB_GPIO_4, 			1);		// AFB_GPIO_4 -> LED6 on the Test Carrier Board
	gpio_direction_output(AFB_GPIO_5, 			0);		// AFB_GPIO_5 -> LED5 on the Test Carrier Board
	gpio_direction_output(AFB_GPIO_6, 			0);		// AFB_GPIO_6 -> LED4 on the Test Carrier Board
	gpio_direction_output(AFB_GPIO_7, 			0);		// AFB_GPIO_7 -> LED3 on the Test Carrier Board
}

/* Setup the LEDS on the Logosni8 board */
#ifndef CONFIG_SPL_BUILD
static void setup_iomux_leds(void)
{
	// Add a GPIO request for the two LEDS
	gpio_request(GPIO_LED_2, 				"GPIO_LED_2");
	gpio_request(GPIO_LED_3, 				"GPIO_LED_3");

	// Setup the LEDS and the corresponding padding
	SETUP_IOMUX_PADS(ni8_led_pads);

	// Setup the LEDs as Output
	gpio_direction_output(GPIO_LED_2, 1);			// LED2
	gpio_direction_output(GPIO_LED_3, 0);			// LED3
};
#endif /* CONFIG_SPL_BUILD */

static void setup_iomux_boot_config(void)
{
	// Add a GPIO request for the Bootconfigs
	gpio_request(GPIO_EIM_DA0, 				"GPIO_EIM_DA0");
	gpio_request(GPIO_EIM_DA1, 				"GPIO_EIM_DA1");
	gpio_request(GPIO_EIM_DA2, 				"GPIO_EIM_DA2");
	gpio_request(GPIO_EIM_DA3, 				"GPIO_EIM_DA3");
	gpio_request(GPIO_EIM_DA4, 				"GPIO_EIM_DA4");
	gpio_request(GPIO_EIM_DA5, 				"GPIO_EIM_DA5");
	gpio_request(GPIO_EIM_DA6, 				"GPIO_EIM_DA6");
	gpio_request(GPIO_EIM_DA7, 				"GPIO_EIM_DA7");
	gpio_request(GPIO_EIM_DA8, 				"GPIO_EIM_DA8");
	gpio_request(GPIO_EIM_DA9, 				"GPIO_EIM_DA9");
	gpio_request(GPIO_EIM_DA10, 			"GPIO_EIM_DA10");
	gpio_request(GPIO_EIM_DA11, 			"GPIO_EIM_DA11");
	gpio_request(GPIO_EIM_DA12, 			"GPIO_EIM_DA12");
 	gpio_request(GPIO_EIM_DA13, 			"GPIO_EIM_DA13");
	gpio_request(GPIO_EIM_DA14, 			"GPIO_EIM_DA14");
	gpio_request(GPIO_EIM_DA15, 			"GPIO_EIM_DA15");

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
#ifndef CONFIG_SPL_BUILD
static void setup_iomux_enet(void)
{
	gpio_request(GPIO_RGMII_RESET_LOGISNI8, "GPIO_RGMII_RESET_LOGOSNI8");
	gpio_request(GPIO_RGMII_RX_DV,			"GPIO_RGMII_RX_DV");
	gpio_request(GPIO_RGMII_RX_D0,			"GPIO_RGMII_RX_D0");
	gpio_request(GPIO_RGMII_RX_D1,			"GPIO_RGMII_RX_D1");
	gpio_request(GPIO_RGMII_RX_D2,			"GPIO_RGMII_RX_D2");
	gpio_request(GPIO_RGMII_RX_D3,			"GPIO_RGMII_RX_D3");
	gpio_request(GPIO_RGMII_RX_CLK,			"GPIO_RGMII_RX_CLK");
	gpio_request(GPIO_ENET_RXD0_INT,		"GPIO_ENET_RXD0_INT");

	// Do all the first mapping - GPIOs for Configuring the PHY and the AR8035 Mode
	SETUP_IOMUX_PADS(enet_pads1);

	/* wait until 3.3V of PHY and clock become stable - see Ar8035 datasheet */
	mdelay(10);

	// Set output for configuring AR8035
	gpio_direction_output(GPIO_RGMII_RESET_LOGISNI8, 	0); 	// Logosni8 PHY rst

	// Setup the correct mode for Ethernet chip - AR8035 - Should be 1110 - see page 8 in the datasheet
	gpio_direction_output(GPIO_RGMII_RX_DV, 			0);
	gpio_direction_output(GPIO_RGMII_RX_D0, 			0);
	gpio_direction_output(GPIO_RGMII_RX_D1, 			0);
	gpio_direction_output(GPIO_RGMII_RX_D2, 			1);
	gpio_direction_output(GPIO_RGMII_RX_D3,				1);
	gpio_direction_output(GPIO_RGMII_RX_CLK, 			1); 	// low voltage - 1.5 0 and 1.8 is 1 - for 2.5V - PULL DOWN/PULL UP (Hardwired)

	// Need delay 5ms according to AR8035 spec - to make sure the clock is stable - logosni8
	mdelay(10);
	gpio_set_value(GPIO_RGMII_RESET_LOGISNI8, 			1); 	// Logosni8 PHY reset

	SETUP_IOMUX_PADS(enet_pads2);
	mdelay(10);	// Wait 5000 us before using mii interface - and pull the reset pin low
}
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_USB		// Added for Logosni8 Testing
static iomux_v3_cfg_t const usb_pads[] = {
	// USB 1 PWR Enable
	IOMUX_PAD_CTRL(GPIO_0__GPIO1_IO00, WEAK_PULLUP),
	// USB 0 PWR Enable
	IOMUX_PAD_CTRL(KEY_ROW4__GPIO4_IO15, WEAK_PULLUP),
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
	if (port == 1)
	{
		//printf("Reseting the USB HUB 1");
		gpio_request(GP_USB1_PWR, "GP_USB1_PWR");

		// Reset USB hub
		gpio_direction_output(GP_USB1_PWR, 0);
		mdelay(2);
		gpio_set_value(GP_USB1_PWR, 1);
	}
	else
	{
		// Otherwise it is port 0
		//printf("Reseting the USB HUB 0");
		gpio_request(GP_USB0_PWR, "GP_USB0_PWR");

		// Reset USB hub
		gpio_direction_output(GP_USB0_PWR, 0);
		mdelay(2);
		gpio_set_value(GP_USB0_PWR, 1);
	}

	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	gpio_request(GP_USB0_PWR, "GP_USB0_PWR");

	return gpio_direction_output(GP_USB0_PWR, on);
}

#endif

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(2, 30)) : -1;
}
#ifndef CONFIG_SPL_BUILD
static void setup_spi(void)
{
	SETUP_IOMUX_PADS(ecspi1_pads);
}
#endif /* CONFIG_SPL_BUILD */
#endif // CONFIG_MXC_SPI


// Function for increasing Boot Count
static inline void bootcount_inc_logos(void) {
	unsigned long bootcount = bootcount_load();
	puts("Increase Bootcount\n");
	bootcount_store(++bootcount);
}
#ifndef CONFIG_SPL_BUILD
int board_phy_config(struct phy_device *phydev)
{
	// Setting RGMII_ID makes driver enable RX and TX delays, all other options breaks everything.
	phydev->interface = PHY_INTERFACE_MODE_RGMII_ID;

	phy_init();
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	setup_iomux_enet();
	return cpu_eth_init(bis);
}
#endif /* CONFIG_SPL_BUILD */

#ifdef CONFIG_VIDEO_IPUV3
static void do_enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

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
	.bus	= 1,// logos is bus 2
	.addr	= 0x50,
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

#ifdef CONFIG_CMD_I2C
static int detect_i2c(struct display_info_t const *dev)
{
    return ((0 == i2c_set_bus_num(dev->bus)) && (0 == i2c_probe(dev->addr)));
}
#endif // CONFIG_CMD_I2C
#endif // CONFIG_VIDEO_IPUV3

#ifdef DEMO_MODE
static unsigned gpios_led_logosni8[] = {
	GPIO_LED_2, /* LED 2 - LogosNi8 */
	GPIO_LED_3, /* LED 3 - LogosNi8 */
};

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

#ifndef CONFIG_SPL_BUILD
static int setup_fec(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* Clear gpr1[ENET_CLK_SEL] for external clock  - see page 2032 in reference manual */
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	// Change the drive strength
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_0OHMS,
				 (void *)IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V,
				 (void *)IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);

	// Daisy Chain
	__raw_writel(IOMUXC_ENET_REF_CLK_SELECT_INPUT_ENABLE_ENET_REF_CLK,
				 (void *)IOMUXC_ENET_REF_CLK_SELECT_INPUT);

	return 0;
}
#endif /* CONFIG_SPL_BUILD */

// TODO: Some of the Initialisation needs to be moved to board_init()
int board_early_init_r(void)
{
	// Setup of UART2, UART4 and UART5
	setup_iomux_uart();

	// Config environment variables TODO Test if this is needed
	env_set("ethact", "FEC");

#ifdef CONFIG_CMD_I2C
	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);
#endif

#ifdef CONFIG_USB
	// Early setup of USB
	SETUP_IOMUX_PADS(conf_usb_pads);
#endif

	// Early setup of Watchdog
	SETUP_IOMUX_PADS(conf_wdog_pads);

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
#ifndef CONFIG_SPL_BUILD
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
#endif /* CONFIG_SPL_BUILD */

#ifdef DEMO_MODE
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
#endif // DEMO_MODE

#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_CMD_BMODE // TODO Adapt to our board or remove
static const struct boot_mode board_boot_modes[] = {
	/* 8 bit bus width */
	{"sd1", MAKE_CFGVAL(0x42, 0x28, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc0", MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc1", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL, 0},
};
#endif // CONFIG_CMD_BMODE
#endif /* CONFIG_SPL_BUILD */

// Card detected function for seeing if a card is present
int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	if (cfg->esdhc_base == USDHC1_BASE_ADDR) {
		gpio_request(SDIO_CD, "SDIO_CD");
		ret = !(gpio_get_value(SDIO_CD));
		return ret;
	}
	else
	{
		ret = -1; // If it is not the SD card - it should be marked as present
		return ret;
	}

	return -1;
}

// I2c Functions for the full u-boot
#ifndef CONFIG_SPL_BUILD
int i2c_read(uint8_t chip, unsigned int addr, int alen,
			 uint8_t *buffer, int len)

			 // Call I2c DM read ..
{
	struct udevice *dev;
	int err;

	err = i2c_get_chip_for_busnum(3, chip, 1, &dev);
	if (err) {
		printf("%s: Cannot find FRAM \n", __func__);
		return err;
	}
	// Reading from FRAM
	return dm_i2c_read(dev, 0x0, buffer, alen);
}

int i2c_write(uint8_t chip, unsigned int addr, int alen,
			  uint8_t *buffer, int len)
{
	struct udevice *dev;
	int err;

	err = i2c_get_chip_for_busnum(3, chip, 1, &dev);
	if (err) {
		printf("%s: Cannot find FRAM\n", __func__);
		return err;
	}
	// Write to the I2c Device
	return dm_i2c_write(dev, 0x00, buffer, alen);
}
#endif /* CONFIG_SPL_BUILD */

// These function are not needed by SPL
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
	gpio_request(SDIO_PWR_EN, 			"SDIO_PWR_EN,");
	gpio_request(SDIO_WP, 				"SDIO_WP");
	gpio_request(SDIO_CD, 				"SDIO_CD");

	// Enable power to SDCARD
	gpio_direction_output(SDIO_PWR_EN, 	1);
	gpio_direction_output(SDIO_WP, 		1);
	gpio_direction_output(SDIO_CD, 		1);

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	if(fsl_esdhc_initialize(bis, &usdhc_cfg[0]))
		puts("WARNING: failed to initialize SD\n");
	if(fsl_esdhc_initialize(bis, &usdhc_cfg[1]))
		puts("WARNING: failed to initialize eMMC on Test Carrier\n");
	if(fsl_esdhc_initialize(bis, &usdhc_cfg[2]))
		puts("WARNING: failed to initialize eMMC on Nicore8\n");

	return 0;
}
/*
 * i2c_multiplexer
 * This function can control the multiplexer PCA9546ABS, which control I2c3 .
 * I2c3 can be connected to four different i2c busses using this multiplexer.
 * @param - select
 * 0x01 - selects I2C4_SDA_LCD
 * 0x02 - selects I2C4_SDA_HDMI
 * 0x04 - selects I2C4_SDA_GP
 * 0x08 - selects I2C4_SDA_CAM
 */
#ifndef CONFIG_SPL_BUILD
int i2c_multiplexer(uint8_t select)
{
	struct udevice *dev;
	u8 addr = 0x00;
	int err;

	err = i2c_get_chip_for_busnum(2, 0x70, 1, &dev);
	if (err) {
		printf("%s: Cannot find I2C Multiplexer \n", __func__);
		return err;
	}

	// Write to the I2c Device
	if (dm_i2c_write(dev, addr, &select, 1)) {
		printf("dm_i2c_write: error sending\n");
	}
	return 0;

};
#endif /* CONFIG_SPL_BUILD */

int board_mmc_init_dts(void) {
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
	gpio_request(SDIO_PWR_EN, 			"SDIO_PWR_EN,");
	gpio_request(SDIO_WP, 				"SDIO_WP");
	gpio_request(SDIO_CD, 				"SDIO_CD");

	// Enable power to SDCARD
	gpio_direction_output(SDIO_PWR_EN, 	1);
	gpio_direction_output(SDIO_WP, 		1);
	gpio_direction_output(SDIO_CD, 		1);

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	return 0;
}

#ifndef CONFIG_SPL_BUILD
int board_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);

	// First setting up the LED2 and LED3 on the Nicore8 for demo purposes
	setup_iomux_leds();

	// Setup of GPIOs
	setup_iomux_gpio();

	// Early setup of AFB_GPIOs - These are only valid for SMARC Version 1.1 - have changed with the new spec 2.1
	setup_iomux_afb_gpio();

	// Set Boot Configs as GPIOs - such that they can be validated with u-boot
	setup_iomux_boot_config();

	// Set reset high for IC2 Bus select - Chip is PCA954 - IC2 address 0x70 - (Reset is active low)
	gpio_request(GPIO_I2C_BUS_SEL_RESET, "GPIO_I2C_BUS_SEL_RESET ");

	// Map HDMI Reset - I2c bus select
	SETUP_IOMUX_PADS(hdmi_reset_pads);

	// Set output high - reset disabled
	gpio_direction_output(GPIO_I2C_BUS_SEL_RESET, 1);

	// Config the I2c Multiplexer
	i2c_multiplexer( 0x02 );

#ifdef CONFIG_VIDEO_IPUV3
	setup_display();
#endif // CONFIG_VIDEO_IPUV3

	// Setup Clocks for Ethernet
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#if defined(CONFIG_OF_CONTROL)
	// Init mmc - Ontop of Device tree - Enable power for SD Card
	board_mmc_init_dts();

	// Init USB
#ifdef CONFIG_USB_EHCI_MX6
	board_ehci_hcd_init(0);
	board_ehci_hcd_init(1);
	board_ehci_power(0, 1);
#endif

	// ETH init
	setup_iomux_enet();

	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);
#endif

	// Setting up USB OTG - We have ENET_RX_ER connected to OTG_ID TODO Verify if this is needed
	clrbits_le32(&iomux->gpr[1], IOMUXC_GPR1_OTG_ID_MASK);

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif

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
#endif /* CONFIG_SPL_BUILD */
/*
 * The board_late_init function is called late during the bootloader initialisation
 * Therefore, all the functionality needed late during the bootup should be added here - this is e.g. the UART printing
 * Which is first available late in the bootup, because the Test Carrier Board needs to be powered up.
 */
#ifndef CONFIG_SPL_BUILD
int board_late_init(void)
{
	// The test carrier board is now powered up and the UART is ready - make a startup screen
	print_Logos_Logo();
	printf("\n%s\nNiCore8 HW id: %s - Logos Payment Solutions A/S.\n", U_BOOT_VERSION_STRING, env_get("serial#"));

#ifdef CONFIG_IMX_THERMAL
	struct udevice *thermal_dev;
	int cpu_tmp, ret;

	// Get the device
	ret = uclass_get_device(UCLASS_THERMAL, 0, &thermal_dev);

	// Get some information about the temperatur of the CPU
	//ret = imx_thermal_get_temp(thermal_dev, &cpu_tmp);
	ret = thermal_get_temp(thermal_dev, &cpu_tmp);

	if (!ret)
		printf("CPU temperature at %d Celsius\n", cpu_tmp);
	else
		printf("Error Getting the CPU Temperature\n");
#endif /* CONFIG_IMX_THERMAL */

#ifdef DEMO_MODE
	// Boot up Song
	bootup_Song_Star_Wars();

	// This function creates a short demo of LED2 and LED3 on the Ni8 board - No udelay in board_early_init - use cpurelax()
	led_logosni8_party_light();
#endif

	// Set i2c bus to 3 - Boot Counter
	struct udevice *dev;
	int err;

	err = i2c_get_chip_for_busnum(BOOTCOUNT_I2C_BUS, 0x51, 1, &dev);
	if (err) {
		puts("Error switching I2C bus\n");
		return err;
	 }

	// Increase bootcount Manually
	bootcount_inc_logos();


	return 0;

}
#endif /* CONFIG_SPL_BUILD */
//#endif /* CONFIG_SPL_BUILD */


// Enable watchdog when device tree is enabled for SPL
#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_WDT
#if defined(CONFIG_IMX_WATCHDOG)
static void imx_watchdog_reset(struct watchdog_regs *wdog)
{
#ifndef CONFIG_WATCHDOG_RESET_DISABLE
	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);
#endif /* CONFIG_WATCHDOG_RESET_DISABLE*/
}

static void imx_watchdog_init(struct watchdog_regs *wdog, bool ext_reset,
				u64 timeout)
{
	u16 wcr;
	/*
	 * The timer watchdog can be set between
	 * 0.5 and 128 Seconds. If not defined
	 * in configuration file, sets 128 Seconds
	 */
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 128000
#endif

	timeout = max_t(u64, timeout, TIMEOUT_MIN);
	timeout = min_t(u64, timeout, TIMEOUT_MAX);
	timeout = lldiv(timeout, 500) - 1;

#ifdef CONFIG_FSL_LSCH2
	wcr = (WCR_WDA | WCR_SRS | WCR_WDE) << 8 | timeout;
#else
	wcr = WCR_WDZST | WCR_WDBG | WCR_WDE | WCR_SRS |
			WCR_WDA | SET_WCR_WT(timeout);
	if (ext_reset)
		wcr |= WCR_WDT;
#endif /* CONFIG_FSL_LSCH2*/
	writew(wcr, &wdog->wcr);
	imx_watchdog_reset(wdog);
}

static int imx_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_init(wdog, true, CONFIG_WATCHDOG_TIMEOUT_MSECS);

	while(1)
	{
		//Wait for reboot
	}
	return 0;
}

/*
 * Board specific reset that is system reset.
 */

void reset_cpu(void)
{ 	struct udevice *dev = 0;
	u64 timeout = 0;
	ulong flags = 0;

	// TODO: ADD some reset needed features - Have GPIO to PMIC to reset - This is not a true reset
 	// Reset CPU
 	imx_wdt_start(dev, timeout, flags);
}

#endif /* CONFIG_IMX_WATCHDOG */
#endif /* CONFIG_WDT */
#endif /* CONFIG_SPL_BUILD */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------- SPL Mode Code ------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <linux/libfdt.h>
#include "asm/arch/crm_regs.h"
#include "asm/arch/mx6dl-ddr.h"

void reset_cpu(void)
{
}

//#define IOMUX_PAD_CTRL(name, pad_ctrl) NEW_PAD_CTRL(MX6_PAD_##name | pad_ctrl)

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	// Here look at the Test Pin from the DIP switch in order to choose the boot up method for now.

	// If it is low - then boot u-boot
	gpio_request(GP_TEST_SMARC, "GP_TEST_SMARC");
	gpio_direction_input(GP_TEST_SMARC);

	/* Only enter in Falcon mode if GP_TEST_SMARC is enabled */
	if ( gpio_get_value(GP_TEST_SMARC) == 0)
	{
		puts("Booting U-Boot\n");
		return 1;
	}
	else
	{
		puts("Booting OS\n");
		return 0;
	}
}
#endif /* CONFIG_SPL_OS_BOOT */

/* called from board_init_r after gd setup if CONFIG_SPL_BOARD_INIT defined */
/* its our chance to print info about boot device */
void spl_board_init(void)
{
	//printf("%d\n",spl_start_uboot());
	/* determine boot device from SRC_SBMR1 (BOOT_CFG[4:1]) or SRC_GPR9 */
	u32 boot_device = BOOT_DEVICE_MMC1;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		puts("Booting from MMC\n");
		break;
	case BOOT_DEVICE_NAND:
		puts("Booting from NAND\n");
		break;
	case BOOT_DEVICE_SATA:
		puts("Booting from SATA\n");
		break;
	default:
		puts("Unknown boot device\n");
	}

	/* PMIC init */
	//setup_pmic();
}


void board_boot_order(u32 *spl_boot_list)
{
	/*
	* Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD3
	 * 0x3                  SD4
	 */

	// Map MMC 1
	SETUP_IOMUX_PADS(sdmmc_pads);

	SETUP_IOMUX_PADS(usdhc3_pads);

	// Map MMC 2
	SETUP_IOMUX_PADS(usdhc4_pads);

	// Request GPIOs
	gpio_request(SDIO_PWR_EN, 			"SDIO_PWR_EN,");
	gpio_request(SDIO_WP, 				"SDIO_WP");
	gpio_request(SDIO_CD, 				"SDIO_CD");

	// Enable power to SDCARD
	gpio_direction_output(SDIO_PWR_EN, 	1);
	gpio_direction_output(SDIO_WP, 		1);
	gpio_direction_output(SDIO_CD, 		1);

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	// We need to decide which mmc to boot from - for now we are using the SD card
	spl_boot_list[0] = BOOT_DEVICE_MMC1;
	spl_boot_list[1] = BOOT_DEVICE_MMC2_2;
	spl_boot_list[2] = BOOT_DEVICE_MMC2;
}


static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFFFF3, &ccm->CCGR2);
	writel(0x3FF0300F, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000F3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}
#ifdef DRAM_INIT
static int mx6dl_dcd_table[] = {
	/* ddr-setup.cfg */

	/* SDQS */
	MX6_IOM_DRAM_SDQS0, 0x00000030,
	MX6_IOM_DRAM_SDQS1, 0x00000030,
	MX6_IOM_DRAM_SDQS2, 0x00000030,
	MX6_IOM_DRAM_SDQS3, 0x00000030,
	MX6_IOM_DRAM_SDQS4, 0x00000030,
	MX6_IOM_DRAM_SDQS5, 0x00000030,
	MX6_IOM_DRAM_SDQS6, 0x00000030,
	MX6_IOM_DRAM_SDQS7, 0x00000030,
	// BDS
	MX6_IOM_GRP_B0DS, 0x00000030,
	MX6_IOM_GRP_B1DS, 0x00000030,
	MX6_IOM_GRP_B2DS, 0x00000030,
	MX6_IOM_GRP_B3DS, 0x00000030,
	MX6_IOM_GRP_B4DS, 0x00000030,
	MX6_IOM_GRP_B5DS, 0x00000030,
	MX6_IOM_GRP_B6DS, 0x00000030,
	MX6_IOM_GRP_B7DS, 0x00000030,
	// Adds
	MX6_IOM_GRP_ADDDS, 0x00000030,

	/* 40 Ohm drive strength for cs0/1,sdba2,cke0/1,sdwe */
	MX6_IOM_GRP_CTLDS, 0x00000030,

	// DQM0
	MX6_IOM_DRAM_DQM0, 0x00020030,
	MX6_IOM_DRAM_DQM1, 0x00020030,
	MX6_IOM_DRAM_DQM2, 0x00020030,
	MX6_IOM_DRAM_DQM3, 0x00020030,
	MX6_IOM_DRAM_DQM4, 0x00020030,
	MX6_IOM_DRAM_DQM5, 0x00020030,
	MX6_IOM_DRAM_DQM6, 0x00020030,
	MX6_IOM_DRAM_DQM7, 0x00020030,


	MX6_IOM_DRAM_CAS, 0x00020030,
	MX6_IOM_DRAM_RAS, 0x00020030,
	MX6_IOM_DRAM_SDCLK_0, 0x00020030,
	MX6_IOM_DRAM_SDCLK_1, 0x00020030,

	// DRAM RESET
	MX6_IOM_DRAM_RESET, 0x00020030,
	MX6_IOM_DRAM_SDCKE0, 0x00003000,
	MX6_IOM_DRAM_SDCKE1, 0x00003000,

 	// Different than for the Colibri MX6
	MX6_IOM_DRAM_SDODT0, 0x00020030,
	MX6_IOM_DRAM_SDODT1, 0x00020030,

	/* (differential input) */
	MX6_IOM_DDRMODE_CTL, 0x00020000,
	/* (differential input) */
	MX6_IOM_GRP_DDRMODE, 0x00020000,
	/* disable ddr pullups */
	MX6_IOM_GRP_DDRPKE, 0x00000000,
	MX6_IOM_DRAM_SDBA2, 0x00000000,
	/* 40 Ohm drive strength for cs0/1,sdba2,cke0/1,sdwe */
	MX6_IOM_GRP_DDR_TYPE, 0x000C0000,

	/* Read data DQ Byte0-3 delay */
	MX6_MMDC_P0_MPRDDQBY0DL, 0x33333333,
	MX6_MMDC_P0_MPRDDQBY1DL, 0x33333333,
	MX6_MMDC_P0_MPRDDQBY2DL, 0x33333333,
	MX6_MMDC_P0_MPRDDQBY3DL, 0x33333333,
	MX6_MMDC_P1_MPRDDQBY0DL, 0x33333333,
	MX6_MMDC_P1_MPRDDQBY1DL, 0x33333333,
	MX6_MMDC_P1_MPRDDQBY2DL, 0x33333333,
	MX6_MMDC_P1_MPRDDQBY3DL, 0x33333333,

	/*
	 * MDMISC	mirroring	interleaved (row/bank/col)
	 */
	MX6_MMDC_P0_MDMISC, 0x00001740,

	/*
	 * MDSCR	con_req
	 */
	MX6_MMDC_P0_MDSCR, 0x00008000,
	// From U-Boot 800mhz_2x128mx16.cfg
	MX6_MMDC_P0_MDPDC, 0x0002002D,

	//Not define in Nitrogen lite board 800mhz
	MX6_MMDC_P0_MDSCR, 0x00008000,
	MX6_MMDC_P0_MDCFG0, 0x3F435333,
	MX6_MMDC_P0_MDCFG1, 0xB50D0AA4,
	MX6_MMDC_P0_MDCFG2, 0x01FF00DB,
	MX6_MMDC_P0_MDRWD, 0x000026D2,
	MX6_MMDC_P0_MDOR, 0x00431023,
	MX6_MMDC_P0_MDOTC, 0x1B444040,
	MX6_MMDC_P0_MDPDC, 0x0002556D,

	/* CS0 End: 7MSB of ((0x10000000, + 512M) -1) >> 25 */
	MX6_MMDC_P0_MDASP, 0x00000017,
	MX6_MMDC_P0_MDCTL, 0x83190000,

	/* Write commands to DDR */
	/* Load Mode Registers - we have the setting for the extended temperature */
	MX6_MMDC_P0_MDSCR, 0x04088032,
	MX6_MMDC_P0_MDSCR, 0x00008033,
	MX6_MMDC_P0_MDSCR, 0x00048031,
	MX6_MMDC_P0_MDSCR, 0x15208030,

	/* ZQ calibration */
	MX6_MMDC_P0_MDSCR, 0x04008040,

	MX6_MMDC_P0_MPZQHWCTRL, 0xA1390003,
	MX6_MMDC_P1_MPZQHWCTRL, 0xA1390003,
	MX6_MMDC_P0_MDREF, 0x00005800,

	MX6_MMDC_P0_MPODTCTRL, 0x00011117,
	MX6_MMDC_P1_MPODTCTRL, 0x00011117,

// Different sequence
/*
	MX6_MMDC_P0_MPDGCTRL0, 0x4220021F,
	0x021b483c, 0x4201020C,
	0x021b0840, 0x0207017E,
	0x021b4840, 0x01660172,
	0x021b0848, 0x4A4D4E4D,
	0x021b4848, 0x4A4F5049,
	0x021b0850, 0x3F3C3D31,
	0x021b4850, 0x3238372B,
 */
// Colibri Version
	MX6_MMDC_P0_MPDGCTRL0, 0x4220021F,
	MX6_MMDC_P0_MPDGCTRL1, 0x0207017E,
	MX6_MMDC_P1_MPDGCTRL0, 0x4201020C,
	MX6_MMDC_P1_MPDGCTRL1, 0x01660172,
	MX6_MMDC_P0_MPRDDLCTL, 0x4A4D4E4D,
	MX6_MMDC_P1_MPRDDLCTL, 0x3F3C3D31,
	MX6_MMDC_P0_MPWRDLCTL, 0x4A4F5049,
	MX6_MMDC_P1_MPWRDLCTL, 0x3238372B,

	MX6_MMDC_P0_MPWLDECTRL0, 0x001F001F,
	MX6_MMDC_P0_MPWLDECTRL1, 0x001F001F,
	MX6_MMDC_P1_MPWLDECTRL0, 0x001F001F,
	MX6_MMDC_P1_MPWLDECTRL1, 0x001F001F,

	MX6_MMDC_P0_MPMUR0, 0x00000800,
	MX6_MMDC_P1_MPMUR0, 0x00000800,
	MX6_MMDC_P0_MDSCR, 0x00000000,
	MX6_MMDC_P0_MAPSR, 0x00011006,
};

static void ddr_init(int *table, int size)
{
	int i;

	for (i = 0; i < size / 2 ; i++)
		writel(table[2 * i + 1], table[2 * i]);
}

static void spl_dram_init(void)
{
	ddr_init(mx6dl_dcd_table, ARRAY_SIZE(mx6dl_dcd_table));
}
#endif /* DRAM_INIT */

/* - This function looks for the name of the U-Boot binary
 * This is not a perfect match. Add the name when building the fit image
 */
int board_fit_config_name_match(const char *name)
{	// TODO: Change the Description when building the FIT Image
	char tmp_name[20] = "Standard Boot\0";

	return strcmp(name, tmp_name);
}

int board_early_init_f(void)
{
	
	// Setup of UART2, UART4 and UART5
	setup_iomux_uart();//this is also int boar_early_init_f()

	// Setup early value initialisation - power up carrier board - set GPIO_CARRIER_PWR_ON high
	gpio_direction_output(IMX_GPIO_NR(6, 31), 1); // Doesnt set the Pin high early enough

	// Add a GPIO request for the two LEDS
	gpio_request(GPIO_LED_2, "GPIO_LED_2");
	gpio_request(GPIO_LED_3, "GPIO_LED_3");

	// Setup the LEDS and the corresponding padding
	SETUP_IOMUX_PADS(ni8_led_pads);

	// Setup the LEDs as Output
	gpio_direction_output(GPIO_LED_2, 0);			// LED2
	gpio_direction_output(GPIO_LED_3, 0);			// LED3

	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);

	return 0;
}

// Dummy function for I2C support
int gpio_request_by_name_nodev(ofnode node, const char *list_name, int index,
			struct gpio_desc *desc, int flags)
{

	return 0;
}

void board_init_f(ulong dummy)
{
	/* DDR initialization */
	//spl_dram_init(); //outcommented, uses DCD tabe to initilze ram

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* setup GP timer */
	timer_init();

	/* iomux */
	board_early_init_f();

	/* Enable device tree and early DM support*/
	spl_early_init();

	/* UART clocks enabled and gd valid - init serial console */
#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif /*	CONFIG_SPL_SERIAL_SUPPORT	*/

	// Run our code
	// Add a GPIO request for the two LEDS
	gpio_request(GPIO_LED_2, "GPIO_LED_2");
	gpio_request(GPIO_LED_3, "GPIO_LED_3");

	// Setup the LEDS and the corresponding padding
	SETUP_IOMUX_PADS(ni8_led_pads);

	// Setup the LEDs as Output
	gpio_direction_output(GPIO_LED_2, 1);			// LED2
	gpio_direction_output(GPIO_LED_3, 0);			// LED3

	// Setup of GPIOs
	setup_iomux_gpio();

	// Early setup of AFB_GPIOs - These are only valid for SMARC Version 1.1 - have changed with the new spec 2.1
	setup_iomux_afb_gpio();

	// Set Boot Configs as GPIOs - such that they can be validated with u-boot
	setup_iomux_boot_config();

	/* Clear the BSS. */ 
	memset(__bss_start, 0, __bss_end - __bss_start);
	//comment out clearing of BSS should be done bi crt0

	// Set environment variable for OS Boot
	env_set("falcon_args_file", "Nicore8");

	// Set i2c bus to 3 - Boot Counter
	int err;

	err = i2c_set_bus_num(BOOTCOUNT_I2C_BUS);
	if (err) {
		puts("Error switching I2C bus\n");
	}

	// Increase bootcount Manually
	bootcount_inc_logos();

	/* load/boot image from boot device */
	board_init_r(NULL, 0);

	gpio_direction_output(GPIO_LED_2, 1);			// LED2
	gpio_direction_output(GPIO_LED_3, 1);			// LED3
}
#endif

