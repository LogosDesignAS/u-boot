/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2021 Logos Payment Solutions A/S.
 *
 * Configuration settings for the Logos Ni8 board.
 */

#ifndef __LOGOS_NI8_H__
#define __LOGOS_NI8_H__

#include"mx6_common.h"

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

#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
#define CONFIG_MXC_UART_BASE							UART4_BASE
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

/*
 * Undefine the following defines
 * This is needed in order to not print before the UART is powered up.
 */
#undef CONFIG_DISPLAY_CPUINFO
#undef CONFIG_DISPLAY_BOARDINFO

#define CONFIG_NR_DRAM_BANKS				1
#define CONFIG_SYS_MAX_FLASH_BANKS			1
#define CONFIG_SYS_MALLOC_LEN				(10 * SZ_1M)
#define PHYS_SDRAM							MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE				PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR			IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE			IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
  (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)


/* I2C Configs */
#ifdef CONFIG_SPL_BUILD
// For SPL use Legacy I2C Settings
#define CONFIG_SYS_I2C_LEGACY
#define CONFIG_SYS_SPL_MALLOC_START 0x18300000
#endif /* CONFIG_SPL_BUILD */
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1				// enable I2C bus 1
#define CONFIG_SYS_I2C_MXC_I2C2				// enable I2C bus 2
#define CONFIG_SYS_I2C_MXC_I2C3				// enable I2C bus 3
#define CONFIG_SYS_I2C_MXC_I2C4				// enable I2C bus 4
#define CONFIG_SYS_I2C_SPEED				100000
#define CONFIG_I2C_EDID

// Set the I2C EEPROM Address
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1

/* Bootcount Commands - Use i2C */
#define BOOTCOUNT_I2C_BUS					3
#define CONFIG_SYS_I2C_RTC_ADDR				0x51

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR			USDHC4_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM			3

/* Ethernet config */
#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE					RGMII
#define IMX_FEC_BASE						ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR				0x04
#define CONFIG_ETHPRIME						"FEC"
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

/* Environment variables */
#define CONFIG_BOOTCOMMAND "run set_defaults; run check_bootpart; run set_bootargs; run bootcmd_fit;"

// Add a different boot method depending on prod or dev
#ifdef CONFIG_TARGET_LOGOSNICORE8DEV

#define CONFIG_EXTRA_ENV_SETTINGS \
  "bootargs_base='console=ttymxc3,115200 rootwait ro printk.time=y earlyprintk rootdelay=5 panic=10 debug ignore_loglevel'\0" \
  "bootargs_a='rootfstype=squashfs root=/dev/mmcblk0gp0p2'\0" \
  "bootargs_b='rootfstype=squashfs root=/dev/mmcblk0gp1p2'\0" \
  "devtype=mmc\0" \
  "devnum=0\0" \
  "bootpart_a=4\0" \
  "bootpart_b=5\0"\
  "default_fitimage=image.itb\0" \
  "loadaddr=0x12000000\0" \
  "serverip=172.16.1.60\0" \
  "bootscr=boot.scr\0" \
  "bootenv=uEnv.txt\0 " \
  "bootcmd_fit=" \
    "if test -e ${devtype} ${devnum}.${bootpart} ${fitimage}; then " \
      "fatload ${devtype} ${devnum}.${bootpart} ${loadaddr} ${fitimage}; " \
      "bootm ${loadaddr}; echo 'reset'; " \
    "else; " \
      "echo ${devtype} ${devnum}.${bootpart} does not contain FIT image ${fitimage}; echo 'reset'; " \
    "fi;\0" \
  "set_defaults=" \
    "if test -z \"$bootpart\"; then " \
      "setenv bootpart ${bootpart_a}; " \
      "saveenv;" \
    "fi; " \
    "if test -z \"$fitimage\"; then " \
      "setenv fitimage ${default_fitimage}; " \
      "saveenv;" \
    "fi;\0" \
  "check_bootpart=" \
    "if test ${bootpart} != ${bootpart_a} && test ${bootpart} != ${bootpart_b}; then " \
      "setenv bootpart ${bootpart_a}; " \
    "fi;\0" \
  "swap_bootpart=" \
    "if test ${bootpart} -eq ${bootpart_a}; then " \
      "setenv bootpart ${bootpart_b}; " \
    "else; " \
      "setenv bootpart ${bootpart_a}; " \
    "fi; " \
    "setenv fitimage ${default_fitimage}; " \
    "saveenv; \0" \
  "set_bootargs=" \
    "if test ${bootpart} -eq ${bootpart_a}; then " \
      "setenv bootargs ${bootargs_base} ${bootargs_a}; " \
    "else " \
      "setenv bootargs ${bootargs_base} ${bootargs_b}; " \
    "fi; \0" \
  "load_and_run_env_from_tftp=" \
    "setenv autoload no; dhcp; " \
    "if tftp ${loadaddr} nicore8/scripts/${bootenv}; then " \
      "if env import -t ${loadaddr} ${filesize}; then " \
        "if tftp ${loadaddr} nicore8/scripts/${bootscr}; then " \
          "source ${loadaddr}; " \
        "else; " \
          "echo Failed to download nicore8/scripts/${bootscr} from ${serverip}.; " \
        "fi; " \
      "else; " \
        "echo Failed to import environment from ${bootenv} in memory at ${loadaddr}.; " \
      "fi; " \
    "else; " \
      "echo Failed to download nicore8/scripts/${bootenv} from ${serverip}.; " \
    "fi; \0" \
  "install_env=" \
    "setenv autoload no; dhcp; " \
    "tftp ${loadaddr} nicore8/scripts/${bootscr}; fatwrite ${devtype} ${devnum}.${bootpart_a} ${loadaddr} ${bootscr} ${filesize}; " \
    "tftp ${loadaddr} nicore8/scripts/${bootenv}; fatwrite ${devtype} ${devnum}.${bootpart_a} ${loadaddr} ${bootenv} ${filesize}; " \
    "echo Installed ${bootscr} & ${bootenv} from ${serverip} to ${devtype} ${devnum}.${bootpart_a}.;\0" \
  "load_env_from_emmc=" \
    "if fatload ${devtype} ${devnum}.${bootpart_a} ${loadaddr} ${bootenv}; then " \
      "env import -t ${loadaddr} ${filesize}" \
      "if fatload ${devtype} ${devnum}.${bootpart_a} ${loadaddr} ${bootscr}; then " \
        "source ${loadaddr}; " \
      "else; " \
        "echo Failed to load script from ${devtype} ${devnum}.${bootpart_a} ${bootscr}.; " \
      "fi; " \
    "else; " \
      "echo Failed to load environment from ${devtype} ${devnum}.${bootpart_a} ${bootenv}.; " \
    "fi; \0" \
  "altbootcmd=run check_bootpart; run swap_bootpart; run set_bootargs; run bootcmd_fit;\0" \
  "bootmenu_0=1. Boot from eMMC=run set_defaults; run check_bootpart; run bootcmd_fit;\0" \
  "bootmenu_1=2. Launch environment from tftp=run load_and_run_env_from_tftp;\0" \
  "bootmenu_2=3. Install latest user supplied environment from tftp=if run install_env; then bootmenu; fi;\0" \
  "bootmenu_3=4. Load user supplied environment from eMMC=run load_env_from_emmc;\0" \
  "bootmenu_4=5. Reset bootcount=if bootcount reset; then bootmenu; fi;\0"
#else

// CONFIG_ENV_WRITEABLE_LIST is defined in production,
// we explicitly define (whitelist) the set of mutable variables below.
#define CONFIG_ENV_FLAGS_LIST_STATIC "bootpart:dw,fitimage:sw"

// Defaults to booting FIT image 'image.itb' file from FAT fs from eMMC 0.
// GP partition 0 (hardware partition 4) with fallback to GP partition 1 (hardware partition 5).
// Alternative boot will swap between partition 4 and 5. This means that if bootcount is reached
// trying to boot partition 4 then alternative boot will try to boot partition 5 or vice versa.
// bootpart variable can be set from a running Linux system to change the current active partition
// after a firmware update. Alternative boot will always fall back to default_image.
// 'reset' should be added after all final commands, to avoid falling back to consol in case
// of any error scenario.
#define CONFIG_EXTRA_ENV_SETTINGS \
  "bootargs_base='console=ttymxc3,115200 rootwait ro quiet'\0" \
  "bootargs_a='rootfstype=squashfs root=/dev/mmcblk0gp0p2'\0" \
  "bootargs_b='rootfstype=squashfs root=/dev/mmcblk0gp1p2'\0" \
  "devtype=mmc\0" \
  "devnum=0\0" \
  "bootpart_a=4\0" \
  "bootpart_b=5\0"\
  "default_fitimage=image.itb\0" \
  "loadaddr=0x12000000\0" \
  "bootcmd_fit=" \
    "if test -e ${devtype} ${devnum}.${bootpart} ${fitimage}; then " \
      "fatload ${devtype} ${devnum}.${bootpart} ${loadaddr} ${fitimage}; " \
      "bootm ${loadaddr}; reset; " \
    "else; " \
      "echo ${devtype} ${devnum}.${bootpart} does not contain FIT image ${fitimage}; reset; " \
    "fi;\0" \
  "set_defaults=" \
    "if test -z \"$bootpart\"; then " \
      "setenv bootpart ${bootpart_a}; " \
      "saveenv;" \
    "fi; " \
    "if test -z \"$fitimage\"; then " \
      "setenv fitimage ${default_fitimage}; " \
      "saveenv;" \
    "fi;\0" \
  "check_bootpart=" \
    "if test ${bootpart} != ${bootpart_a} && test ${bootpart} != ${bootpart_b}; then " \
      "setenv bootpart ${bootpart_a}; " \
    "fi;\0" \
  "swap_bootpart=" \
    "if test ${bootpart} -eq ${bootpart_a}; then " \
      "setenv bootpart ${bootpart_b}; " \
    "else; " \
      "setenv bootpart ${bootpart_a}; " \
    "fi; " \
    "setenv fitimage ${default_fitimage}; " \
    "saveenv; \0" \
  "set_bootargs=" \
    "if test ${bootpart} -eq ${bootpart_a}; then " \
      "setenv bootargs ${bootargs_base} ${bootargs_a}; " \
    "else " \
      "setenv bootargs ${bootargs_base} ${bootargs_b}; " \
    "fi; \0" \                                  \
  "altbootcmd=run check_bootpart; run swap_bootpart; run set_bootargs; run bootcmd_fit;\0"

#endif // CONFIG_TARGET_LOGOSNICORE8DEV

#endif // __LOGOS_NI8_H__
