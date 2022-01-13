/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 *
 * Configuration settings for the Logos Ni8 board.
 */

#ifndef __LOGOS_NI8_H__
#define __LOGOS_NI8_H__

#include"mx6_common.h"

//#include <config_distro_bootcmd.h>
//#include <linux/stringify.h>

// Watchdog defines
#define TIMEOUT_MAX	128000
#define TIMEOUT_MIN	500

// If SPL is enabled include the SPL header file for imx6
#ifdef CONFIG_SPL
#include "imx6_spl.h"

// Parameters below is for SPL loading FIT image from FS on partition.
/*
#define		CONFIG_SPL_FS_LOAD_KERNEL_NAME				  "nicore8br_initrd.itb"
#define 	CONFIG_SPL_FS_LOAD_ARGS_NAME				  "Nicore8.itb"
*/

//Parameters below is for SPL loading FIT image from raw partition.
#define		CONFIG_SYS_SPL_ARGS_ADDR                      0x1ffe5000
#define 	CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR         0x6000// Block offset for Arguments - fdt
#define 	CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS        0x76
#define		CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	      0x0 // 0MB at partition 4 in MMC dev 2 - offset of kernel
#define     CONFIG_SYS_MMCSD_RAW_MODE_EMMC_BOOT_PARTITION 4

// Add possibilities to adjust the Malloc size - which is needed with SPL and large kernels
#ifdef CONFIG_SYS_SPL_MALLOC_SIZE
#undef CONFIG_SYS_SPL_MALLOC_SIZE
#define	CONFIG_SYS_SPL_MALLOC_SIZE						  0x1000000 // 16 MB
#endif

#endif /* CONFIG_SPL */

// Mach Type
// 'MACH_TYPE_NITROGEN6X 4296' from arch/arm/include/asm/mach-types.h
#define CONFIG_MACH_TYPE 4296

#define CONFIG_MXC_UART_BASE							UART4_BASE

/*
 * Undefine the following defines
 * This is needed in order to not print before the UART is powered up.
 */
#undef CONFIG_DISPLAY_CPUINFO
#undef CONFIG_DISPLAY_BOARDINFO

#define CONFIG_NR_DRAM_BANKS				1
#define CONFIG_SYS_MAX_FLASH_BANKS			1
#define PHYS_SDRAM							MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE				PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR			IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE			IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
  (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* I2C Configs */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1				// enable I2C bus 1
#define CONFIG_SYS_I2C_MXC_I2C2				// enable I2C bus 2
#define CONFIG_SYS_I2C_MXC_I2C3				// enable I2C bus 3
#define CONFIG_SYS_I2C_MXC_I2C4				// enable I2C bus 4
#define CONFIG_SYS_I2C_SPEED				100000
#define CONFIG_I2C_EDID
#else
#define CONFIG_SYS_I2C_MXC
#define I2C_DEFAULT_SLAVE_ADDR 				0x00
#define CONFIG_SYS_I2C_MXC_I2C1												// Enable I2C bus 1
#define CONFIG_SYS_MXC_I2C1_SPEED			100000							// Set speed
#define CONFIG_SYS_MXC_I2C1_SLAVE			I2C_DEFAULT_SLAVE_ADDR			// Set slave
#define CONFIG_SYS_I2C_MXC_I2C2												// Enable I2C bus 2
#define CONFIG_SYS_MXC_I2C2_SPEED			100000							// Set speed
#define CONFIG_SYS_MXC_I2C2_SLAVE			I2C_DEFAULT_SLAVE_ADDR			// Set slave
#define CONFIG_SYS_I2C_MXC_I2C3												// Enable I2C bus 3
#define CONFIG_SYS_MXC_I2C3_SPEED			100000							// Set speed
#define CONFIG_SYS_MXC_I2C3_SLAVE			I2C_DEFAULT_SLAVE_ADDR			// Set slave
#define CONFIG_SYS_I2C_MXC_I2C4												// Enable I2C bus 4
#define CONFIG_SYS_MXC_I2C4_SPEED			100000							// Set speed
#define CONFIG_SYS_MXC_I2C4_SLAVE			I2C_DEFAULT_SLAVE_ADDR			// Set slave
#define CONFIG_SYS_I2C_SPEED				100000
#define CONFIG_I2C_EDID
#endif /* CONFIG_SPL_BUILD */

/* Bootcount Commands - Use i2C */
#define BOOTCOUNT_I2C_BUS					3
#define CONFIG_SYS_I2C_RTC_ADDR				0x51

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR			USDHC4_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM			3

/* Ethernet config */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE					RGMII
#define IMX_FEC_BASE						ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR				0x04
#define CONFIG_ETHPRIME						"FEC"

/* Environment variables below can't be changed */
#define CONFIG_BOOTCOMMAND "run mmc_boot"

#define CONFIG_EXTRA_ENV_SETTINGS \
  "devtype=mmc\0" \
  "devnum=2\0" \
  "bootpart_a=4\0" \
  "bootpart_b=5\0"\
  "fitimage=image.itb\0" \
  "loadaddr=0x12000000\0" \
  "bootcmd_fit="\
    "if test -e ${devtype} ${devnum}.${bootpart} ${fitimage}; then " \
      "fatload ${devtype} ${devnum}.${bootpart} ${loadaddr} ${fitimage}; " \
      "bootm ${loadaddr}; " \
    "else; " \
      "echo fisk; " \
    "fi;\0" \
  "bootcmd=run bootcmd_fit;\0"

#endif // __LOGOS_NI8_H__
