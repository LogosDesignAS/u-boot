/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 *
 * Configuration settings for the Logos Ni8 board.
 */

#ifndef __LOGOS_NI8_H__
#define __LOGOS_NI8_H__

#include <config_distro_bootcmd.h>
#include <linux/stringify.h>

#include"mx6_common.h"

// Watchdog defines
#define TIMEOUT_MAX	128000
#define TIMEOUT_MIN	500

// If SPL is enabled include the SPL header file for imx6
#ifdef CONFIG_SPL
#include "imx6_spl.h"

// Defines for booting the kernel from SPL
#define		CONFIG_SPL_FS_LOAD_KERNEL_NAME				"Nicore8.itb"//"uImage2"
#define		CONFIG_SYS_SPL_ARGS_ADDR					0x27500000
#define 	CONFIG_SPL_FS_LOAD_ARGS_NAME				"Nicore8.itb"//"uImage2"
#define 	CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR  		0x800   /* 1MB */
#define 	CONFIG_CMD_SPL_WRITE_SIZE 					0x00100000
#define 	CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS 		(CONFIG_CMD_SPL_WRITE_SIZE / 512)
#define		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR		0x1000  /* 2MB */

// For SPL to run the kernel a larger Malloc size is needed
#ifdef CONFIG_SYS_SPL_MALLOC_SIZE
#undef CONFIG_SYS_SPL_MALLOC_SIZE
#define		CONFIG_SYS_SPL_MALLOC_SIZE					0x1000000	/* 16 MB */
#endif

#endif /* CONFIG_SPL */


#define CONFIG_MXC_UART_BASE							UART4_BASE

/*
 * Undefine the following defines
 * This is needed in order to not print before the UART is powered up.
 */
#undef CONFIG_DISPLAY_CPUINFO
#undef CONFIG_DISPLAY_BOARDINFO

#define CONFIG_NR_DRAM_BANKS                1
#define CONFIG_SYS_MAX_FLASH_BANKS          1
#define CONFIG_SYS_MALLOC_LEN               (10 * SZ_1M)
#define PHYS_SDRAM                          MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE               PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR            IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE            IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
  (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* I2C Configs */
//#define CONFIG_SYS_I2C

#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1				// enable I2C bus 1
#define CONFIG_SYS_I2C_MXC_I2C2				// enable I2C bus 2
#define CONFIG_SYS_I2C_MXC_I2C3				// enable I2C bus 3
#define CONFIG_SYS_I2C_MXC_I2C4				// enable I2C bus 4
#define CONFIG_SYS_I2C_SPEED				100000
#define CONFIG_I2C_EDID


/* Bootcount Commands - Use i2C */
#define BOOTCOUNT_I2C_BUS					3
#define CONFIG_SYS_I2C_RTC_ADDR				0x51

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR           USDHC4_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM            3

#ifdef CONFIG_CMD_MMC
#define DISTRO_BOOT_DEV_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1) func(MMC, mmc, 2)
#else
#define DISTRO_BOOT_DEV_MMC(func)
#endif

/* Ethernet config */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE					RGMII
#define IMX_FEC_BASE						ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR				0x04
#define CONFIG_ETHPRIME						"FEC"

/*
#ifdef CONFIG_USB_STORAGE
#define DISTRO_BOOT_DEV_USB(func) func(USB, usb, 0) func(USB, usb, 1)
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

#define CONFIG_USBD_HS
*/

/* USB Configs */
/*
#define CONFIG_USB_MAX_CONTROLLER_COUNT     2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	// For OTG port
#define CONFIG_MXC_USB_PORTSC	            (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	            0
*/

#define BOOT_TARGET_DEVICES(func) \
	DISTRO_BOOT_DEV_MMC(func)
    /*
	DISTRO_BOOT_DEV_USB(func) \
	DISTRO_BOOT_DEV_PXE(func) \
	DISTRO_BOOT_DEV_DHCP(func)
    */

#define FDTFILE "fdtfile=imx6dl-nicore8.dtb\0"

//#define CONFIG_EXTRA_ENV_SETTINGS

#endif // __LOGOS_NI8_H__