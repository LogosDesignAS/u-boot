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
#include <asm/mach-imx/mxc_i2c.h>
//#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/spi.h>
#include <asm/mach-imx/boot_mode.h>
//#include <asm/mach-imx/video.h>
#include <fsl_esdhc_imx.h>
//#include <micrel.h> // KSZ9031 Gigabit ethernet transceiver driver
#include <miiphy.h>
//#include <netdev.h>
#include <asm/arch/crm_regs.h>
//#include <asm/arch/mxc_hdmi.h>
#include <i2c.h>
#include <input.h>
//#include <netdev.h>
//#include <usb/ehci-ci.h>

DECLARE_GLOBAL_DATA_PTR;

/* TODO Verify values below */
#define UART_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

#define IOMUX_PAD_CTRL(name, pad_ctrl) NEW_PAD_CTRL(MX6_PAD_##name, pad_ctrl)

int dram_init(void)
{
    /* Line below requires CONFIG_DDR_MB=4096 to be set in logosni8_defconfig
	 * gd->ram_size = ((ulong)CONFIG_DDR_MB * 1024 * 1024);
     */
    gd->ram_size = imx_ddr_size();
	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
    // error: 'MX6_PAD_CSI0_DAT12__UART1_RX_DATA' undeclared here (not in a function); did you mean 'MX6_PAD_CSI0_DAT12__UART4_RX_DATA'?
	IOMUX_PAD_CTRL(CSI0_DAT12__UART4_RX_DATA, UART_PAD_CTRL),
	IOMUX_PAD_CTRL(CSI0_DAT13__UART4_TX_DATA, UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	return 0;
}
