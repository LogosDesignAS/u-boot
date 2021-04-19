// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <init.h>
//#include <net.h>
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
//#include <asm/mach-imx/video.h>
#include <fsl_esdhc_imx.h>
//#include <micrel.h> // KSZ9031 Gigabit ethernet transceiver driver
#include <miiphy.h>
//#include <netdev.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <input.h>
//#include <netdev.h>
//#include <usb/ehci-ci.h>

#ifdef CONFIG_CMD_I2C 		// Added for Logosni8 Testing
	#include <i2c.h>
	#include <asm/mach-imx/mxc_i2c.h>
#endif

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
	GPIO_CARRIER_PWR_ON	= IMX_GPIO_NR(6, 31)
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

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

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

#if defined(CONFIG_MX6QDL)
#define I2C_PADS_INFO_ENTRY(i2cnum, scl_pad, scl_bank, scl_gp,		\
		sda_pad, sda_bank, sda_gp, pad_ctrl)			\
	I2C_PADS_INFO_CPU(MX6Q, i2cnum, scl_pad, scl_bank, scl_gp,	\
		sda_pad, sda_bank, sda_gp, pad_ctrl),			\
	I2C_PADS_INFO_CPU(MX6DL, i2cnum, scl_pad, scl_bank, scl_gp,	\
		sda_pad, sda_bank, sda_gp, pad_ctrl)
#define I2C_PADS_INFO_ENTRY_SPACING 2

#define IOMUX_PAD_CTRL(name, pad_ctrl) \
		NEW_PAD_CTRL(MX6Q_PAD_##name, pad_ctrl),	\
		NEW_PAD_CTRL(MX6DL_PAD_##name, pad_ctrl)
#else
#define I2C_PADS_INFO_ENTRY(i2cnum, scl_pad, scl_bank, scl_gp,		\
		sda_pad, sda_bank, sda_gp, pad_ctrl)			\
	I2C_PADS_INFO_CPU(MX6, i2cnum, scl_pad, scl_bank, scl_gp,	\
		sda_pad, sda_bank, sda_gp, pad_ctrl)
#define I2C_PADS_INFO_ENTRY_SPACING 1


#define IOMUX_PAD_CTRL(name, pad_ctrl) NEW_PAD_CTRL(MX6_PAD_##name, pad_ctrl)

int dram_init(void)
{
    /* Line below requires CONFIG_DDR_MB=4096 to be set in logosni8_defconfig
	 * gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);
     */
    gd->ram_size = imx_ddr_size();
	return 0;
}

/* Configuration of UART2 for Logosni8 */
static iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PAD_CTRL(EIM_D26__UART2_TX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D27__UART2_RX_DATA, UART_PAD_CTRL),
};

/* Configuration of UART4 for Logosni8 */
static iomux_v3_cfg_t const uart4_pads[] = {
	IOMUX_PAD_CTRL(CSI0_DAT12__UART4_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT13__UART4_TX_DATA, UART_PAD_CTRL),
	// Configuring CTS and RTS
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

#ifdef CONFIG_CMD_I2C 		// Added for Logosni8 Testing
static struct i2c_pads_info i2c_pads[] = {
	/* I2C1, SGTL5000 */
	I2C_PADS_INFO_ENTRY(I2C1, EIM_D21, 3, 21, EIM_D28, 3, 28, I2C_PAD_CTRL),
	/* I2C2 Camera, MIPI */
	I2C_PADS_INFO_ENTRY(I2C2, KEY_COL3, 4, 12, KEY_ROW3, 4, 13,
			    I2C_PAD_CTRL),
	/* I2C3, J15 - RGB connector */
	I2C_PADS_INFO_ENTRY(I2C3, GPIO_5, 1, 05, GPIO_16, 7, 11, I2C_PAD_CTRL),
};
#endif

#define I2C_BUS_CNT    3

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PAD_CTRL(SD2_CLK__SD2_CLK, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_CMD__SD2_CMD, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT0__SD2_DATA0, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT1__SD2_DATA1, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT2__SD2_DATA2, USDHC_PAD_CTRL),
	IOMUX_PAD_CTRL(SD2_DAT3__SD2_DATA3, USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads1[] = {
	IOMUX_PAD_CTRL(ENET_MDIO__ENET_MDIO, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_MDC__ENET_MDC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TXC__RGMII_TXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD0__RGMII_TD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD1__RGMII_TD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD2__RGMII_TD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TD3__RGMII_TD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_TX_CTL__RGMII_TX_CTL, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_REF_CLK__ENET_TX_CLK, ENET_PAD_CTRL),
	/* pin 35 - 1 (PHY_AD2) on reset */
	IOMUX_PAD_CTRL(RGMII_RXC__GPIO6_IO30, NO_PAD_CTRL),
	/* pin 32 - 1 - (MODE0) all */
	IOMUX_PAD_CTRL(RGMII_RD0__GPIO6_IO25, NO_PAD_CTRL),
	/* pin 31 - 1 - (MODE1) all */
	IOMUX_PAD_CTRL(RGMII_RD1__GPIO6_IO27, NO_PAD_CTRL),
	/* pin 28 - 1 - (MODE2) all */
	IOMUX_PAD_CTRL(RGMII_RD2__GPIO6_IO28, NO_PAD_CTRL),
	/* pin 27 - 1 - (MODE3) all */
	IOMUX_PAD_CTRL(RGMII_RD3__GPIO6_IO29, NO_PAD_CTRL),
	/* pin 33 - 1 - (CLK125_EN) 125Mhz clockout enabled */
	IOMUX_PAD_CTRL(RGMII_RX_CTL__GPIO6_IO24, NO_PAD_CTRL),
	/* pin 42 PHY nRST */
	IOMUX_PAD_CTRL(EIM_D23__GPIO3_IO23, NO_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_RXD0__GPIO1_IO27, NO_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads2[] = {
	IOMUX_PAD_CTRL(RGMII_RXC__RGMII_RXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, ENET_PAD_CTRL),
};

static iomux_v3_cfg_t const misc_pads[] = {
	IOMUX_PAD_CTRL(GPIO_1__USB_OTG_ID, WEAK_PULLUP),
	IOMUX_PAD_CTRL(KEY_COL4__USB_OTG_OC, WEAK_PULLUP),
	IOMUX_PAD_CTRL(EIM_D30__USB_H1_OC, WEAK_PULLUP),
	/* OTG Power enable */
	IOMUX_PAD_CTRL(EIM_D22__GPIO3_IO22, OUTPUT_40OHM),
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

	/* Configuration of GPIO_0 to USB_H1_PWR - Here called USB_1_PWREN in schematic - see schematic page 10 - The Same padding is used for USB on Nitrogen */
	IOMUX_PAD_CTRL(GPIO_0__USB_H1_PWR, WEAK_PULLUP)
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

#define WL12XX_WL_IRQ_GP	IMX_GPIO_NR(6, 14)
#define WL12XX_WL_ENABLE_GP	IMX_GPIO_NR(6, 15)
#define WL12XX_BT_ENABLE_GP	IMX_GPIO_NR(6, 16)

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
	gpio_set_value(GPIO_LED_3, 0);
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
	gpio_direction_output(AFB_GPIO_5, 0);			// AFB_GPIO_5 -> LED5 on the Test Carrier Board
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
	gpio_direction_output(GPIO_LED_2, 0);			// LED2
	gpio_direction_output(GPIO_LED_3, 0);			// LED3
};

static void setup_iomux_enet(void)
{
	gpio_direction_output(IMX_GPIO_NR(3, 23), 0); /* SABRE Lite PHY rst */
	gpio_direction_output(IMX_GPIO_NR(1, 27), 0); /* Nitrogen6X PHY rst */
	gpio_direction_output(IMX_GPIO_NR(6, 30), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 25), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 27), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 28), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 29), 1);
	SETUP_IOMUX_PADS(enet_pads1);
	gpio_direction_output(IMX_GPIO_NR(6, 24), 1);

	/* Need delay 10ms according to KSZ9021 spec */
	udelay(1000 * 10);
	gpio_set_value(IMX_GPIO_NR(3, 23), 1); /* SABRE Lite PHY reset */
	gpio_set_value(IMX_GPIO_NR(1, 27), 1); /* Nitrogen6X PHY reset */

	SETUP_IOMUX_PADS(enet_pads2);
	udelay(100);	/* Wait 100 us before using mii interface */
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

	/* Reset USB hub */
	gpio_direction_output(IMX_GPIO_NR(7, 12), 0);
	mdelay(2);
	gpio_set_value(IMX_GPIO_NR(7, 12), 1);

	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	gpio_set_value(GP_USB_OTG_PWR, on);
	return 0;
}

#endif

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(3, 19)) : -1;
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

int board_phy_config(struct phy_device *phydev)
{
	/* min rx data delay */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, 0x0);
	/* min tx data delay */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, 0x0);
	/* max rx/tx clock delay, min rx/tx control */
	ksz9021_phy_extended_write(phydev,
			MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, 0xf0f0);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(struct bd_info *bis)
{/* - Uncommented for early testing
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	gpio_request(WL12XX_WL_IRQ_GP, "wifi_irq");
	gpio_request(IMX_GPIO_NR(6, 30), "rgmii_rxc");
	gpio_request(IMX_GPIO_NR(6, 25), "rgmii_rd0");
	gpio_request(IMX_GPIO_NR(6, 27), "rgmii_rd1");
	gpio_request(IMX_GPIO_NR(6, 28), "rgmii_rd2");
	gpio_request(IMX_GPIO_NR(6, 29), "rgmii_rd3");
	gpio_request(IMX_GPIO_NR(6, 24), "rgmii_rx_ctl");
	gpio_request(IMX_GPIO_NR(3, 23), "rgmii_reset_sabrelite");
	gpio_request(IMX_GPIO_NR(1, 27), "rgmii_reset_nitrogen6x");
	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return -EINVAL;
	// scan phy 4,5,6,7
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		ret = -EINVAL;
		goto free_bus;
	}
	printf("using phy at %d\n", phydev->addr);
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret)
		goto free_phydev;
#endif

#ifdef CONFIG_CI_UDC
	// For otg ethernet //
	usb_eth_initialize(bis);
#endif
 */
	return 0;
/* - Uncommened for early testing
free_phydev:
	free(phydev);
free_bus:
	free(bus);
	return ret;
 */
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
	gpio_direction_output(LVDS_BACKLIGHT_GP, 1);
}

static void enable_rgb(struct display_info_t const *dev)
{
	SETUP_IOMUX_PADS(rgb_pads);
	gpio_direction_output(RGB_BACKLIGHT_GP, 1);
}

struct display_info_t const displays[] = {{
	.bus	= 1,
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
	gpio_direction_input(LVDS_BACKLIGHT_GP);
	gpio_direction_input(RGB_BACKLIGHT_GP);
}
#endif

#define WL12XX_WL_IRQ_GP	IMX_GPIO_NR(6, 14)

static void set_gpios(unsigned *p, int cnt, int val)
{
	int i;

	for (i = 0; i < cnt; i++)
		gpio_direction_output(*p++, val);
}


static unsigned gpios_led_logosni8[] = {
	GPIO_LED_2, /* LED 2 - LogosNi8 */
	GPIO_LED_3, /* LED 3 - LogosNi8 */
};

// TODO: Remove this demo function before merging into the main branch
static void led_logosni8_party_light(void)
{
	// This function will create a simple light demo - using the LED2 and LED3 - will run for 20 seconds
	for (int i = 0; i < 60; i++) {
		gpio_set_value(GPIO_LED_2, 1);
		gpio_set_value(GPIO_LED_3, 1);

		// Wait 1s
		mdelay(1000);

		gpio_set_value(GPIO_LED_2, 0);
		gpio_set_value(GPIO_LED_3, 0);
	}

	// After the initial heartbeat start the more serious stuff will initiate - Namely "Something - rename
	// Insert some more LED config here, to make a nice demo.
	gpio_set_value(GPIO_LED_2, 0);
	gpio_set_value(GPIO_LED_3, 1);
}

// TODO: Some of the Initialisation needs to be moved to board_init()
int board_early_init_f(void)
{
	// Setup of UART2, UART4 and UART5
	setup_iomux_uart();

	// First setting up the LED2 and LED3 on the Nicore8 for demo purposes
	setup_iomux_leds();

	// Setup of GPIOs
	setup_iomux_gpio();

	// Early setup of AFB_GPIOs - These are only valid for SMARC Version 1.1 - have changed with the new spec 2.1
	setup_iomux_afb_gpio();


#ifdef CONFIG_CMD_I2C		// Added for Logosni8 Testing
	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);
#endif

#ifdef CONFIG_USB		// Added for Logosni8 Testing
	// Early setup of USB
	SETUP_IOMUX_PADS(conf_usb_pads);
#endif

	// Early setup of Watchdog - might be needed earlier
	//SETUP_IOMUX_PADS(conf_wdog_pads);

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
	return 0;
}

int board_init(void)
{

#ifdef		CONFIG_CMD_I2C // Added for Logosni8 Testing
	// Setting up I2C and USB
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct i2c_pads_info *p = i2c_pads;
	int i;
	int stride = 1;

#if defined(CONFIG_MX6QDL)
	stride = 2;
	if (!is_mx6dq() && !is_mx6dqp())
		p += 1;
#endif
	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUXC_GPR1_OTG_ID_MASK,
			IOMUXC_GPR1_OTG_ID_GPIO1);

	SETUP_IOMUX_PADS(misc_pads);

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif
	SETUP_IOMUX_PADS(usdhc2_pads);

	for (i = 0; i < I2C_BUS_CNT; i++) {
		setup_i2c(i, CONFIG_SYS_I2C_SPEED, 0x7f, p);
		p += stride;
	}
#endif

#ifdef CONFIG_SATA
	setup_sata();
#endif

	return 0;
}

int checkboard(void)
{
/*
	int ret = gpio_get_value(WL12XX_WL_IRQ_GP);

	if (ret < 0) {
		// The gpios have not been probed yet. Read it myself
		struct gpio_regs *regs = (struct gpio_regs *)GPIO6_BASE_ADDR;
		int gpio = WL12XX_WL_IRQ_GP & 0x1f;

		ret = (readl(&regs->gpio_psr) >> gpio) & 0x01;
	}
	if (ret)
		puts("Board: Nitrogen6X\n");
	else
		puts("Board: SABRE Lite\n");
*/
	// Print the correct board name out
	puts("Board: NiCore8 - Logosni8\n");

	return 0;
}

struct button_key {
	char const	*name;
	unsigned	gpnum;
	char		ident;
};

static struct button_key const buttons[] = {
	{"back",	IMX_GPIO_NR(2, 2),	'B'},
	{"home",	IMX_GPIO_NR(2, 4),	'H'},
	{"menu",	IMX_GPIO_NR(2, 1),	'M'},
	{"search",	IMX_GPIO_NR(2, 3),	'S'},
	{"volup",	IMX_GPIO_NR(7, 13),	'V'},
	{"voldown",	IMX_GPIO_NR(4, 5),	'v'},
};

/*
 * generate a null-terminated string containing the buttons pressed
 * returns number of keys pressed
 */
static int read_keys(char *buf)
{
	int i, numpressed = 0;
	for (i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (!gpio_get_value(buttons[i].gpnum))
			buf[numpressed++] = buttons[i].ident;
	}
	buf[numpressed] = '\0';
	return numpressed;
}

static int do_kbd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char envvalue[ARRAY_SIZE(buttons)+1];
	int numpressed = read_keys(envvalue);
	env_set("keybd", envvalue);
	return numpressed == 0;
}

U_BOOT_CMD(
	kbd, 1, 1, do_kbd,
	"Tests for keypresses, sets 'keybd' environment variable",
	"Returns 0 (true) to shell if key is pressed."
);

#ifdef CONFIG_PREBOOT
static char const kbd_magic_prefix[] = "key_magic";
static char const kbd_command_prefix[] = "key_cmd";

static void preboot_keys(void)
{
	int numpressed;
	char keypress[ARRAY_SIZE(buttons)+1];
	numpressed = read_keys(keypress);
	if (numpressed) {
		char *kbd_magic_keys = env_get("magic_keys");
		char *suffix;
		/*
		 * loop over all magic keys
		 */
		for (suffix = kbd_magic_keys; *suffix; ++suffix) {
			char *keys;
			char magic[sizeof(kbd_magic_prefix) + 1];
			sprintf(magic, "%s%c", kbd_magic_prefix, *suffix);
			keys = env_get(magic);
			if (keys) {
				if (!strcmp(keys, keypress))
					break;
			}
		}
		if (*suffix) {
			char cmd_name[sizeof(kbd_command_prefix) + 1];
			char *cmd;
			sprintf(cmd_name, "%s%c", kbd_command_prefix, *suffix);
			cmd = env_get(cmd_name);
			if (cmd) {
				env_set("preboot", cmd);
				return;
			}
		}
	}
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{NULL,		0},
};
#endif

int misc_init_r(void)
{
/*
	gpio_request(RGB_BACKLIGHT_GP, "lvds backlight");
	gpio_request(LVDS_BACKLIGHT_GP, "lvds backlight");
	gpio_request(GP_USB_OTG_PWR, "usbotg power");
	gpio_request(IMX_GPIO_NR(7, 12), "usbh1 hub reset");
	gpio_request(IMX_GPIO_NR(2, 2), "back");
	gpio_request(IMX_GPIO_NR(2, 4), "home");
	gpio_request(IMX_GPIO_NR(2, 1), "menu");
	gpio_request(IMX_GPIO_NR(2, 3), "search");
	gpio_request(IMX_GPIO_NR(7, 13), "volup");
	gpio_request(IMX_GPIO_NR(4, 5), "voldown");
*/
#ifdef CONFIG_PREBOOT
	preboot_keys();
#endif

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	env_set_hex("reset_cause", get_imx_reset_cause());
	return 0;
}

int board_late_init(void)
{
	// First setting up the LED2 and LED3 on the Nicore8 for demo purposes
	setup_iomux_leds();

	// This function creates a short demo of LED2 and LED3 on the Ni8 board - No udelay in board_early_init - use cpurelax()
	//led_logosni8_party_light();
	return 0;
}
