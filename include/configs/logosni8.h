/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 *
 * Configuration settings for the Logos Ni8 board.
 */

#ifndef __LOGOS_NI8_H__
#define __LOGOS_NI8_H__

#define CONFIG_MXC_UART_BASE            UART4_BASE

#include"mx6_common.h"

/* Undefine the following defines
 * This is needed in order to not print before the UART is powered up.
 */
#undef CONFIG_DISPLAY_CPUINFO
#undef CONFIG_DISPLAY_BOARDINFO

/* TODO Is set to 1 for all other iMX6 boards.
 * Our SDRAM is listed with 8 banks in the datasheet, so should it be 3 (2**3=8)?
 */
#define CONFIG_NR_DRAM_BANKS            1
#define CONFIG_SYS_MAX_FLASH_BANKS      1
#define CONFIG_SYS_MALLOC_LEN           (10 * SZ_1M)
#define CONFIG_SYS_FSL_ESDHC_ADDR       0
#define PHYS_SDRAM                      MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE           PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR        IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE        IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
  (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/*
 * The rest of this file is from Nitrogen lite 6 - TODO: Needs to be modified for Logosni8
 */

/*
 * I2C Configs
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_I2C_EDID

/*
 * SATA Configs
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		6

/* Framebuffer and LCD */
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (6 * 1024 * 1024)
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

#ifdef CONFIG_CMD_MMC
#define DISTRO_BOOT_DEV_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define DISTRO_BOOT_DEV_MMC(func)
#endif

#ifdef CONFIG_CMD_SATA
#define DISTRO_BOOT_DEV_SATA(func) func(SATA, sata, 0)
#else
#define DISTRO_BOOT_DEV_SATA(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define DISTRO_BOOT_DEV_USB(func) func(USB, usb, 0)
#else
#define DISTRO_BOOT_DEV_USB(func)
#endif

#ifdef CONFIG_CMD_PXE
#define DISTRO_BOOT_DEV_PXE(func) func(PXE, pxe, na)
#else
#define DISTRO_BOOT_DEV_PXE(func)
#endif

#ifdef CONFIG_CMD_DHCP
#define DISTRO_BOOT_DEV_DHCP(func) func(DHCP, dhcp, na)
#else
#define DISTRO_BOOT_DEV_DHCP(func)
#endif
#define CONFIG_MACH_TYPE	3769

#define CONFIG_USBD_HS



/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_I2C_EDID

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      0
#define CONFIG_SYS_FSL_USDHC_NUM       2

/*
 * SATA Configs
 */
#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		6

// Nitrogen file
/* USB Configs */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* Framebuffer and LCD */
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (6 * 1024 * 1024)
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

#ifdef CONFIG_CMD_MMC
#define DISTRO_BOOT_DEV_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define DISTRO_BOOT_DEV_MMC(func)
#endif

#ifdef CONFIG_CMD_SATA
#define DISTRO_BOOT_DEV_SATA(func) func(SATA, sata, 0)
#else
#define DISTRO_BOOT_DEV_SATA(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define DISTRO_BOOT_DEV_USB(func) func(USB, usb, 0)
#else
#define DISTRO_BOOT_DEV_USB(func)
#endif

#ifdef CONFIG_CMD_PXE
#define DISTRO_BOOT_DEV_PXE(func) func(PXE, pxe, na)
#else
#define DISTRO_BOOT_DEV_PXE(func)
#endif

#ifdef CONFIG_CMD_DHCP
#define DISTRO_BOOT_DEV_DHCP(func) func(DHCP, dhcp, na)
#else
#define DISTRO_BOOT_DEV_DHCP(func)
#endif


#if defined(CONFIG_SABRELITE)
#define FDTFILE "fdtfile=imx6q-sabrelite.dtb\0"
#else
/* FIXME: nitrogen6x covers multiple configs. Define fdtfile for each supported config. */
#define FDTFILE
#endif

#define BOOT_TARGET_DEVICES(func) \
	DISTRO_BOOT_DEV_MMC(func) \
	DISTRO_BOOT_DEV_SATA(func) \
	DISTRO_BOOT_DEV_USB(func) \
	DISTRO_BOOT_DEV_PXE(func) \
	DISTRO_BOOT_DEV_DHCP(func)

#include <config_distro_bootcmd.h>
#include <linux/stringify.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc1\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	FDTFILE \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0"  \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"ramdiskaddr=0x13000000\0" \
	"ip_dyn=yes\0" \
	"usb_pgood_delay=2000\0" \
	BOOTENV

/* Miscellaneous configurable options */

/* Physical Memory Map */
#define PHYS_SDRAM		       MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE	       PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */

/*
 * PCI express
 */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#endif

#endif				/* __LOGOS_NI8_H__*/