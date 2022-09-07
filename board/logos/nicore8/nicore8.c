// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 */
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <common.h>
#include <env.h>
#include <fsl_esdhc_imx.h>
#include <i2c.h>
#include <linux/delay.h>

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <version.h>
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

// Watchdog
#include <wdt.h>
#include <watchdog.h>
#include <fsl_wdog.h>
#include <div64.h>

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
// Thermal Configs
#include <imx_thermal.h>
#include <thermal.h>
// Logo
#include "logosLogo.h"
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

// Bootcount
#include <bootcount.h>

#ifdef DEMO_MODE
#include "nicore8demo.h"
#endif // DEMO_MODE

// Version String
#include <version.h>
#include <version_string.h>

// ENUM for controlling the reset for I2c select for LCDs, HDMI, GP and CAM
enum I2C_RESET {
	GPIO_I2C_BUS_SEL_RESET		= IMX_GPIO_NR(2, 0)
};

// Enum for LEDs on the Logosni8 board - enum idea came from board/beckhoff/mx53cx9020
enum LED_GPIOS {
	GPIO_LED_2 					= IMX_GPIO_NR(6, 7),
	GPIO_LED_3 					= IMX_GPIO_NR(6, 9)
};

// Enum NiCore8 GPIO signals
enum GPIOS {
	GPIO0						= IMX_GPIO_NR(1,  1), // i.MX6 GPIO_1
	GPIO1						= IMX_GPIO_NR(1,  3), // i.MX6 GPIO_3
	GPIO2						= IMX_GPIO_NR(4,  5), // i.MX6 GPIO_19
	GPIO3						= IMX_GPIO_NR(1,  4), // i.MX6 GPIO_4
	GPIO4						= IMX_GPIO_NR(2, 23), // i.MX6 EIM_CS0
	GPIO5						= IMX_GPIO_NR(2, 24), // i.MX6 EIM_CS1
	GPIO6						= IMX_GPIO_NR(3, 19), // i.MX6 EIM_D19
	GPIO7						= IMX_GPIO_NR(3, 23), // i.MX6 EIM_D23
	GPIO8						= IMX_GPIO_NR(3, 24), // i.MX6 EIM_D24
	GPIO9						= IMX_GPIO_NR(3, 25), // i.MX6 EIM_D25
	GPIO10						= IMX_GPIO_NR(3, 29), // i.MX6 EIM_D29
	GPIO11						= IMX_GPIO_NR(3, 31), // i.MX6 EIM_D31
	GPIO_RESET					= IMX_GPIO_NR(6, 10), // i.MX6 NANDF_RB0
	GPIO_WDOG1_B     			= IMX_GPIO_NR(4,  7), // i.MX6 KEY_ROW0
	GPIO_MCLK					= IMX_GPIO_NR(1,  2), // i.MX6 GPIO_2
	GPIO_EMMC_RESET				= IMX_GPIO_NR(6,  8), // i.MX6 NANDF_ALE
	GPIO_CHARGER_PRSNT			= IMX_GPIO_NR(1,  7), // i.MX6 GPIO_7
	GPIO_CHARGING				= IMX_GPIO_NR(1,  8), // i.MX6 GPIO_8
	GPIO_PMIC_INT_B				= IMX_GPIO_NR(7, 13), // i.MX6 GPIO_18
	GPIO_CARRIER_PWR_ON			= IMX_GPIO_NR(6, 31), // i.MX6 EIM_BCLK
	GPIO_RGMII_nRST          	= IMX_GPIO_NR(1, 25)  // i.MX6 ENET_CRS_DV
};

// Enum for NiCore8 Alternative Function Block GPIO signals
enum AFB_GPIOS {
	AFB_GPIO0					= IMX_GPIO_NR(5, 19), // i.MX6 CSI0_MCLK
	AFB_GPIO1					= IMX_GPIO_NR(5, 18), // i.MX6 CSI0_PIXCLK
	AFB_GPIO2					= IMX_GPIO_NR(5, 21), // i.MX6 CSI0_VSYNC
	AFB_GPIO3					= IMX_GPIO_NR(5, 20), // i.MX6 CSI0_DATA_EN
	AFB_GPIO4					= IMX_GPIO_NR(5, 22), // i.MX6 CSI0_DAT4
	AFB_GPIO5					= IMX_GPIO_NR(5, 23), // i.MX6 CSI0_DAT5
	AFB_GPIO6					= IMX_GPIO_NR(5, 24), // i.MX6 CSI0_DAT6
	AFB_GPIO7					= IMX_GPIO_NR(5, 25)  // i.MX6 CSI0_DAT7
};

DECLARE_GLOBAL_DATA_PTR;

#define GP_TEST_SMARC IMX_GPIO_NR(4, 9)

#define ENET_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)
#define ENET_PAD_CTRL_PD (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)
#define ENET_PAD_CTRL_CLK ((PAD_CTL_PUS_100K_UP & ~PAD_CTL_PKE) | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)
#define UART_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)
#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP	| PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)
#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED |	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)
#define I2C_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_ODE | PAD_CTL_SRE_FAST)
#define WDOG_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_PKE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm)
#define WEAK_PULLUP	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)
#define WEAK_PULLDOWN (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_SLOW)
#define OUTPUT_40OHM (PAD_CTL_SPEED_MED|PAD_CTL_DSE_40ohm)

/* Prevent compiler error if gpio number 08 or 09 is used */
#define not_octal(gp) ((((0x##gp >> 4) & 0xf) * 10) + ((0x##gp & 0xf)))

#define _I2C_PADS_INFO_CPU(cpu, i2cnum, scl_pad, scl_bank, scl_gp,		\
		sda_pad, sda_bank, sda_gp, pad_ctrl, join_io) {					\
	.scl = {															\
		.i2c_mode = NEW_PAD_CTRL(cpu##_PAD_##scl_pad##__##i2cnum##_SCL,	\
					 pad_ctrl),											\
		.gpio_mode = NEW_PAD_CTRL(										\
			cpu##_PAD_##scl_pad##__GPIO##scl_bank##join_io##scl_gp,		\
			pad_ctrl),													\
		.gp = IMX_GPIO_NR(scl_bank, not_octal(scl_gp))					\
	},																	\
	.sda = {															\
		.i2c_mode = NEW_PAD_CTRL(cpu##_PAD_##sda_pad##__##i2cnum##_SDA,	\
					 pad_ctrl),											\
		.gpio_mode = NEW_PAD_CTRL(										\
			cpu##_PAD_##sda_pad##__GPIO##sda_bank##join_io##sda_gp,		\
			pad_ctrl),													\
			.gp = IMX_GPIO_NR(sda_bank, not_octal(sda_gp))				\
	}																	\
}

#define I2C_PADS_INFO_CPU(cpu, i2cnum, scl_pad, scl_bank, scl_gp,		\
		sda_pad, sda_bank, sda_gp, pad_ctrl)							\
		_I2C_PADS_INFO_CPU(cpu, i2cnum, scl_pad, scl_bank, scl_gp,		\
				sda_pad, sda_bank, sda_gp, pad_ctrl, _IO)

#define I2C_PADS_INFO_ENTRY(i2cnum, scl_pad, scl_bank, scl_gp,			\
		sda_pad, sda_bank, sda_gp, pad_ctrl)							\
	I2C_PADS_INFO_CPU(MX6, i2cnum, scl_pad, scl_bank, scl_gp,			\
		sda_pad, sda_bank, sda_gp, pad_ctrl)

#define I2C_PADS_INFO_ENTRY_SPACING 1

#define IOMUX_PAD_CTRL(name, pad_ctrl) NEW_PAD_CTRL(MX6_PAD_##name, pad_ctrl)

// Change driving strength
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII						0x20e0768
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM							0x20e0788
#define IOMUXC_ENET_REF_CLK_SELECT_INPUT							0x20e080C

// Enable Reference CLock
#define IOMUXC_ENET_REF_CLK_SELECT_INPUT_ENABLE_ENET_REF_CLK		0x00000001

// Disable on die termination for RGMII
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_DISABLE					0x00000000
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_30OHMS				0x00000400
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_40OHMS				0x00000300
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_60OHMS				0x00000200
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_120OHMS				0x00000100
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_0OHMS				0x00000000
#define IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_17OHMS 				0x00000700
// Optimised drive strength for 1.0 .. 1.3 V signal on RGMII
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P2V					0x00080000
// Optimised drive strength for 1.3 .. 2.5 V signal on RGMII
#define IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V					0x000C0000

#ifdef CONFIG_MXC_SPI
static iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS1 */
	IOMUX_PAD_CTRL(EIM_EB2__GPIO2_IO30, NO_PAD_CTRL), /* -> BOOT_CFG_30 -> SPINOR_CS0 */
	IOMUX_PAD_CTRL(EIM_D17__ECSPI1_MISO, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D18__ECSPI1_MOSI, SPI_PAD_CTRL),
	IOMUX_PAD_CTRL(EIM_D16__ECSPI1_SCLK, SPI_PAD_CTRL),
};
#endif // CONFIG_MXC_SPI

// I2C MUX Reset Pad Config
static iomux_v3_cfg_t const mux_reset_pads[] = {
		IOMUX_PAD_CTRL(NANDF_D0__GPIO2_IO00, WEAK_PULLUP)
};

// Configuration of UART4 for Logosni8
static iomux_v3_cfg_t const uart4_pads[] = {
		IOMUX_PAD_CTRL(CSI0_DAT12__UART4_TX_DATA, UART_PAD_CTRL),
		IOMUX_PAD_CTRL(CSI0_DAT13__UART4_RX_DATA, UART_PAD_CTRL),
		// Configuring CTS and RTSl
		IOMUX_PAD_CTRL(CSI0_DAT16__UART4_RTS_B, UART_PAD_CTRL),
		IOMUX_PAD_CTRL(CSI0_DAT17__UART4_CTS_B, UART_PAD_CTRL),
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

		// EMMC Reset Core Board
		IOMUX_PAD_CTRL(NANDF_ALE__SD4_RESET,	WEAK_PULLDOWN),
};

static struct fsl_esdhc_cfg usdhc_cfg[CONFIG_SYS_FSL_USDHC_NUM] = {
		{USDHC4_BASE_ADDR}
};

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
static iomux_v3_cfg_t const enet_pads[] = {
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

	/* pin 1 PHY nRST */
	IOMUX_PAD_CTRL(ENET_CRS_DV__GPIO1_IO25, WEAK_PULLUP),
	/* Interrupt pin */
	IOMUX_PAD_CTRL(ENET_RXD0__GPIO1_IO27, ENET_PAD_CTRL),

	IOMUX_PAD_CTRL(RGMII_RXC__RGMII_RXC, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD0__RGMII_RD0, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD1__RGMII_RD1, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD2__RGMII_RD2, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RD3__RGMII_RD3, ENET_PAD_CTRL),
	IOMUX_PAD_CTRL(RGMII_RX_CTL__RGMII_RX_CTL, ENET_PAD_CTRL),
};

#endif // CONFIG_TARGET_LOGOSNICORE8DEV

/* LED2 and LED3 pads on logosni8 */
static iomux_v3_cfg_t const ni8_led_pads[] = {
		IOMUX_PAD_CTRL(NANDF_CLE__GPIO6_IO07, OUTPUT_40OHM),  // - Configured as output 40Ohms
		IOMUX_PAD_CTRL(NANDF_WP_B__GPIO6_IO09, OUTPUT_40OHM), // - Configured as output 40Ohm
};

#ifdef CONFIG_CMD_I2C
/* I2C Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_i2c_pads[] = {
	IOMUX_PAD_CTRL(GPIO_5__I2C3_SCL, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(GPIO_6__I2C3_SDA, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_ROW3__I2C2_SDA, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(KEY_COL3__I2C2_SCL, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_TX_EN__I2C4_SCL, I2C_PAD_CTRL),
	IOMUX_PAD_CTRL(ENET_TXD1__I2C4_SDA, I2C_PAD_CTRL),
};
#endif // CONFIG_CMD_I2C

/* WatchDog Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_wdog_pads[] = {
		// Pin configuration for the Watchdog

		/* Configuration of KEY_ROW0 to WDOG1_B (GPIO4_IO07)- Here called WDOG1_B in schematic  - see schematic page 10 */
		IOMUX_PAD_CTRL(KEY_ROW0__GPIO4_IO07, OUTPUT_40OHM),
};

/* GPIO Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_gpio_pads[] = {

		// Pin configuration for GPIO[0-3]
		IOMUX_PAD_CTRL(GPIO_1__GPIO1_IO01, 		WEAK_PULLUP),
		IOMUX_PAD_CTRL(GPIO_3__GPIO1_IO03, 		WEAK_PULLUP),
		IOMUX_PAD_CTRL(GPIO_19__GPIO4_IO05, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(GPIO_4__GPIO1_IO04, 		WEAK_PULLUP),

		// Pin configuration for GPIO[4-11]
		IOMUX_PAD_CTRL(EIM_CS0__GPIO2_IO23, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_CS1__GPIO2_IO24, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_D19__GPIO3_IO19, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_D23__GPIO3_IO23, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_D24__GPIO3_IO24, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_D25__GPIO3_IO25, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_D29__GPIO3_IO29, 	WEAK_PULLUP),
		IOMUX_PAD_CTRL(EIM_D31__GPIO3_IO31, 	WEAK_PULLUP),

		IOMUX_PAD_CTRL(GPIO_2__GPIO1_IO02, 		OUTPUT_40OHM),

		// Pin configuration for SMARC inputs - CHARGING and CHARGER_PRSNT
		IOMUX_PAD_CTRL(GPIO_7__GPIO1_IO07, 		WEAK_PULLUP),
		IOMUX_PAD_CTRL(GPIO_8__GPIO1_IO08, 		WEAK_PULLUP),

		// Pin configuration for SMARC inputs - PMIC_INT_B
		IOMUX_PAD_CTRL(GPIO_18__GPIO7_IO13, 	NO_PAD_CTRL),

		// PIN Configuration for GPIO_RESET
		IOMUX_PAD_CTRL(NANDF_RB0__GPIO6_IO10, 	WEAK_PULLDOWN),

		// Pin configuration for SMARC inputs - CARRIER_PWR_ON
		IOMUX_PAD_CTRL(EIM_BCLK__GPIO6_IO31, 	WEAK_PULLDOWN),

		// SMARC_TESTn from carrier
		IOMUX_PAD_CTRL(KEY_ROW1__GPIO4_IO09, 	WEAK_PULLUP),
};

/* AFB_GPIO Pin Configuration on logosni8 */
static iomux_v3_cfg_t const conf_afb_gpio_pads[] = {
		// Pin configuration for AFB_GPIO[0-7]
		IOMUX_PAD_CTRL(CSI0_MCLK__GPIO5_IO19, 	    WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_PIXCLK__GPIO5_IO18,     WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_VSYNC__GPIO5_IO21, 	    WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_DATA_EN__GPIO5_IO20,    WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_DAT4__GPIO5_IO22, 	    WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_DAT5__GPIO5_IO23, 	    WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_DAT6__GPIO5_IO24, 	    WEAK_PULLUP),
		IOMUX_PAD_CTRL(CSI0_DAT7__GPIO5_IO25, 	    WEAK_PULLUP),
};

/* Functions below */
int dram_init(void)
{
#ifdef CONFIG_TEE
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024) - 0x2000000;
#else
	gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);
#endif // CONFIG_TEE
	return 0;
}

// Setup the GPIO pins on the Logosni8 board
static void setup_iomux_gpio(void)
{
	// Add a GPIO request for all the GPIOs - without requesting a gpio the driver will not let us use the GPIOs
	gpio_request(GPIO0,				    "GPIO0");
	gpio_request(GPIO1,				    "GPIO1");
	gpio_request(GPIO2,				    "GPIO2");
	gpio_request(GPIO3,				    "GPIO3");
	gpio_request(GPIO4,				    "GPIO4");
	gpio_request(GPIO5,				    "GPIO5");
	gpio_request(GPIO6,				    "GPIO6");
	gpio_request(GPIO7,				    "GPIO7");
	gpio_request(GPIO8,				    "GPIO8");
	gpio_request(GPIO9,				    "GPIO9");
	gpio_request(GPIO10,			    "GPIO10");
	gpio_request(GPIO11,			    "GPIO11");
	gpio_request(GPIO_RESET,		    "GPIO_RESET");
	gpio_request(GPIO_WDOG1_B,		    "GPIO_WDOG1_B");
	gpio_request(GPIO_MCLK,			    "GPIO_MCLK");
	gpio_request(GPIO_CHARGING,		    "GPIO_CHARGING");
	gpio_request(GPIO_EMMC_RESET,	    "GPIO_EMMC_RESET");
	gpio_request(GPIO_PMIC_INT_B, 	    "GPIO_PMIC_INT_B");
	gpio_request(GPIO_CHARGER_PRSNT,    "GPIO_CHARGER_PRSNT");
	gpio_request(GPIO_CARRIER_PWR_ON,	"GPIO_CARRIER_PWR_ON");

	// Setup the rest of the GPIO pins and the corresponding padding for the i.MX6U -
	SETUP_IOMUX_PADS(conf_gpio_pads);

	// Setup the GPIOs as Input if specified on the Schematic and carrier board
	gpio_direction_input(GPIO_CHARGER_PRSNT);
	gpio_direction_input(GPIO_CHARGING);
	gpio_direction_input(GPIO_PMIC_INT_B);
    gpio_direction_input(GPIO0);
    gpio_direction_input(GPIO1);
    gpio_direction_input(GPIO3);
	gpio_direction_input(GPIO7);
    gpio_direction_input(GPIO8);
    gpio_direction_input(GPIO9);
    gpio_direction_input(GPIO10);
    gpio_direction_input(GPIO11);

	// Setup the GPIOs as Output if specified on the Schematic and Test Carrier board
	gpio_direction_output(GPIO_CARRIER_PWR_ON, 0);
	gpio_direction_output(GPIO_MCLK, 0);
	gpio_direction_output(GPIO_EMMC_RESET, 0);
	gpio_direction_output(GPIO_RESET, 0);
	gpio_direction_output(GPIO_WDOG1_B, 1);
	gpio_direction_output(GPIO2, 0);
    gpio_direction_output(GPIO4, 1);
    gpio_direction_output(GPIO5, 1);

	// After setting up the GPIOs - Set one LED on and one off, to signal how fare the bootup is.
	gpio_set_value(GPIO_LED_2, 0);
	gpio_set_value(GPIO_LED_3, 1);
};

static void setup_iomux_afb_gpio(void)
{
    // Setup AFB_GPIOs - which goes to AFB[0-7] on the SMARC interface
    gpio_request(AFB_GPIO0,				"AFB_GPIO0");
    gpio_request(AFB_GPIO1,				"AFB_GPIO1");
    gpio_request(AFB_GPIO2,				"AFB_GPIO2");
    gpio_request(AFB_GPIO3,				"AFB_GPIO3");
    gpio_request(AFB_GPIO4,				"AFB_GPIO4");
    gpio_request(AFB_GPIO5,				"AFB_GPIO5");
    gpio_request(AFB_GPIO6,				"AFB_GPIO6");
    gpio_request(AFB_GPIO7,				"AFB_GPIO7");

    SETUP_IOMUX_PADS(conf_afb_gpio_pads);

    gpio_direction_output(AFB_GPIO0, 0);
    gpio_direction_output(AFB_GPIO1, 0);
    gpio_direction_output(AFB_GPIO2, 0);

    gpio_direction_input(AFB_GPIO3);
    gpio_direction_input(AFB_GPIO4);
    gpio_direction_input(AFB_GPIO5);

    gpio_direction_output(AFB_GPIO6, 0);
    gpio_direction_output(AFB_GPIO7, 0);
}

// Set up the LED's on the NiCore8 board
static void setup_iomux_leds(void)
{
	// Add a GPIO request for the two LEDS
	gpio_request(GPIO_LED_2, "GPIO_LED_2");
	gpio_request(GPIO_LED_3, "GPIO_LED_3");

	// Set up the LED's and the corresponding padding
	SETUP_IOMUX_PADS(ni8_led_pads);

	// Set up the LED's as output
	gpio_direction_output(GPIO_LED_2, 1);			// LED2
	gpio_direction_output(GPIO_LED_3, 0);			// LED3
};

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
static void setup_iomux_enet(void)
{
    gpio_request(GPIO_RGMII_nRST, "GPIO_RGMII_nRST");

	// Do all the first mapping - GPIOs for Configuring the PHY and the AR8035 Mode
	SETUP_IOMUX_PADS(enet_pads);

	// Set output for configuring AR8035 - Reset the AR8035
	gpio_direction_output(GPIO_RGMII_nRST, 0);

	// Wait 20ms
	mdelay(20);

	gpio_direction_output(GPIO_RGMII_nRST, 1);
}
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart4_pads);
}

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(2, 30)) : -1;
}

static void setup_spi(void)
{
	SETUP_IOMUX_PADS(ecspi1_pads);
}
#endif // CONFIG_MXC_SPI

// Function for increasing Boot Count
static inline void bootcount_inc_logos(void) {
	unsigned long bootcount = bootcount_load();

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
	puts("Increase Bootcount\n");
#endif // CONFIG_TARGET_LOGOSNICORE8DEV
	bootcount_store(++bootcount);
}

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
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
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
static int setup_fec(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	// Clear gpr1[ENET_CLK_SEL] for external clock  - see page 2032 in reference manual
	clrbits_le32(&iomuxc_regs->gpr[1], IOMUXC_GPR1_ENET_CLK_SEL_MASK);

	// Change the drive strength
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM_ENABLE_0OHMS, (void *)IOMUX_SW_PAD_CTRL_GRP_RGMII_TERM);
	__raw_writel(IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII_1P5V, (void *)IOMUX_SW_PAD_CTRL_GRP_DDR_TYPE_RGMII);

	// Daisy Chain
	__raw_writel(IOMUXC_ENET_REF_CLK_SELECT_INPUT_ENABLE_ENET_REF_CLK, (void *)IOMUXC_ENET_REF_CLK_SELECT_INPUT);

	return 0;
}
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

int board_early_init_r(void)
{
	setup_iomux_uart();

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
	// Config environment variables
	env_set("ethact", "FEC");
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

#ifdef CONFIG_CMD_I2C
	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);
#endif // CONFIG_CMD_I2C

	// Early setup of Watchdog
	SETUP_IOMUX_PADS(conf_wdog_pads);

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
int overwrite_console(void)
{
	return 1;
}

int print_logos_logo(void)
{
	printf("\n");
	for(int h = 0; h < LOGOS_LOGO_ROWS; h++)
	{
		printf("%s\n", logosLogo[h]);
	}
	return 0;
}
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

// I2c Functions for the full u-boot
int i2c_read(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len)
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

int i2c_write(uint8_t chip, unsigned int addr, int alen, uint8_t *buffer, int len)
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

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

    if(fsl_esdhc_initialize(bis, &usdhc_cfg[0]))
		puts("WARNING: failed to initialize eMMC on Nicore8\n");

	return 0;
}

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

	// Initialise all mmc - Define clocks first
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	return 0;
}

int board_init(void)
{
	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);

	// First setting up the LED2 and LED3 on the Nicore8 for demo purposes
	setup_iomux_leds();

	// Setup of GPIOs
	setup_iomux_gpio();
    setup_iomux_afb_gpio();

	// Map the Reset for the I2C MUX
	SETUP_IOMUX_PADS(mux_reset_pads);

	// Set reset high for IC2 Bus select - chip is PCA954 - IC2 address 0x70 - reset is active low
	gpio_request(GPIO_I2C_BUS_SEL_RESET, "GPIO_I2C_BUS_SEL_RESET ");

	// Set output high - reset disabled
	gpio_direction_output(GPIO_I2C_BUS_SEL_RESET, 1);

	// Setup Clocks for Ethernet
#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif // CONFIG_FEC_MXC

#if defined(CONFIG_OF_CONTROL)
	// Init mmc - Ontop of Device tree - Enable power for SD Card
	board_mmc_init_dts();

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
	// Ethernet init
	setup_iomux_enet();
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

	// Early setup of I2C
	SETUP_IOMUX_PADS(conf_i2c_pads);
#endif // CONFIG_OF_CONTROL

#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif // CONFIG_MXC_SPI

	return 0;
}

int misc_init_r(void)
{
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
#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
	// The carrier board is now powered up and the UART is ready - make a startup screen
	print_logos_logo();
	printf("\n%s\nNiCore8 HW id: %s - Logos Payment Solutions A/S.\n", version_string, env_get("serial#"));

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
#endif // DEMO_MODE
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

	// OP-TEE Environment variable
	env_set("tee", "no");
#ifdef CONFIG_OPTEE
	env_set("tee", "yes");
#endif // CONFIG_OPTEE

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

	// Turn on the LEDS on the Core Board to verify that the Production Image Works and have finished all initialisation
	gpio_set_value(GPIO_LED_2, 0);
	gpio_set_value(GPIO_LED_3, 0);

	return 0;
}

// Enable the watchdog
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

	// Reset CPU

	// Turn on WDOG LED
	gpio_direction_output(GPIO_WDOG1_B, 0);

	// Reset EMMC
	gpio_direction_output(GPIO_EMMC_RESET,		1);		// GPIO_EMMC_RESET - Active high

	// Watchdog with timeout
	imx_wdt_start(dev, timeout, flags);
}

#endif /* CONFIG_IMX_WATCHDOG */
#endif /* CONFIG_WDT */
