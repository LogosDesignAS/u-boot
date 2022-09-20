/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Logos Payment Solutions A/S.
 *
 * Configuration settings for the Logos Ni8 board.
 */

#ifndef __LOGOS_NI8_H__
#define __LOGOS_NI8_H__

#include"mx6_common.h"

// Watchdog defines
#define TIMEOUT_MAX	128000
#define TIMEOUT_MIN	500

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
#define PHYS_SDRAM							MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE				PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR			IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE			IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
  (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
  (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)


/* I2C Configs */
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1				// enable I2C bus 1
#define CONFIG_SYS_I2C_MXC_I2C2				// enable I2C bus 2
#define CONFIG_SYS_I2C_MXC_I2C3				// enable I2C bus 3
#define CONFIG_SYS_I2C_MXC_I2C4				// enable I2C bus 4
#define CONFIG_SYS_I2C_SPEED				100000
#define CONFIG_I2C_EDID

// Set the I2C EEPROM Address
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR			USDHC4_BASE_ADDR
#define CONFIG_SYS_FSL_USDHC_NUM			3

/* Ethernet config */
#ifdef CONFIG_TARGET_LOGOSNICORE8DEV
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE						ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR				0x04
#define CONFIG_ETHPRIME						"FEC"
#endif // CONFIG_TARGET_LOGOSNICORE8DEV

/* Environment variables */
#define CONFIG_BOOTCOMMAND "run bootcmd_fit;"

// Add a different boot method depending on prod or dev
#ifdef CONFIG_TARGET_LOGOSNICORE8DEV

#define CONFIG_EXTRA_ENV_SETTINGS \
  "BOOT_ORDER=A B\0" \
  "BOOT_A_LEFT=3\0" \
  "BOOT_B_LEFT=3\0" \
  "DEVTYPE=mmc\0" \
  "DEVNUM=0\0" \
  "BOOTPART_A=4\0" \
  "BOOTPART_B=5\0"\
  "FITIMAGE=image.itb\0" \
  "FITCONFIG_BASE=\0" \
  "loadaddr=0x12000000\0" \
  "serverip=172.16.1.60\0" \
  "bootenv=uEnv.txt\0 " \
  "bootcmd_fit=" \
    "setenv FITCONFIG; " \
    "if test \"x${FITCONFIG_BASE}\" = \"x\"; then " \
      "setenv FITCONFIG_BASE \"#config-core\"; " \
      "echo \"No FITCONFIG_BASE set, fallback to ${FITCONFIG_BASE}\"; " \
    "fi; " \
    "for BOOT_SLOT in ${BOOT_ORDER}; do " \
      "if test \"x${FITCONFIG}\" != \"x\"; then " \
        "echo Skip remaining; " \
      "elif test \"x${BOOT_SLOT}\" = \"xA\"; then " \
        "if test ${BOOT_A_LEFT} -gt 0; then " \
          "echo Found valid slot A, ${BOOT_A_LEFT} attempts remaining; " \
          "setexpr BOOT_A_LEFT ${BOOT_A_LEFT} - 1; " \
		  "if fatload ${DEVTYPE} ${DEVNUM}.${BOOTPART_A} ${loadaddr} ${FITIMAGE}; then " \
            "setenv FITCONFIG \"${FITCONFIG_BASE}-a\"; " \
            "echo \"Loaded ${FITIMAGE} from ${DEVTYPE} ${DEVNUM}.${BOOTPART_A}, set fit config to ${FITCONFIG}\"; " \
		  "fi; " \
        "fi; " \
      "elif test \"x${BOOT_SLOT}\" = \"xB\"; then " \
        "if test ${BOOT_B_LEFT} -gt 0; then " \
          "echo Found valid slot B, ${BOOT_B_LEFT} attempts remaining; " \
          "setexpr BOOT_B_LEFT ${BOOT_B_LEFT} - 1; " \
		  "if fatload ${DEVTYPE} ${DEVNUM}.${BOOTPART_B} ${loadaddr} ${FITIMAGE}; then " \
            "setenv FITCONFIG \"${FITCONFIG_BASE}-b\"; " \
            "echo \"Loaded ${FITIMAGE} from ${DEVTYPE} ${DEVNUM}.${BOOTPART_B}, set fit config to ${FITCONFIG}\"; " \
		  "fi; " \
        "fi; " \
      "fi; " \
    "done; " \
    "if test -n \"${FITCONFIG}\"; then " \
      "saveenv; " \
    "else; " \
      "echo No valid slot found, resetting tries to 3; " \
      "setenv BOOT_A_LEFT 3; " \
      "setenv BOOT_B_LEFT 3; " \
      "saveenv; " \
    "fi; " \
    "bootm ${loadaddr}${FITCONFIG}; \0" \
  "load_env_from_tftp=" \
    "setenv autoload no; dhcp; " \
    "if tftp ${loadaddr} nicore8/scripts/${bootenv}; then " \
      "if env import -t ${loadaddr} ${filesize}; then " \
        "bootmenu; " \
      "else; " \
        "echo Failed to import environment from ${bootenv} in memory at ${loadaddr}.; " \
      "fi; " \
    "else; " \
      "echo Failed to download nicore8/scripts/${bootenv} from ${serverip}.; " \
    "fi; \0" \
  "install_env_from_tftp=" \
    "setenv autoload no; dhcp; " \
    "if tftp ${loadaddr} nicore8/scripts/${bootenv}; then " \
      "if env import -t ${loadaddr} ${filesize}; then " \
        "saveenv; " \
        "bootmenu; " \
      "else; " \
        "echo Failed to import environment from ${bootenv} in memory at ${loadaddr}.; " \
      "fi; " \
    "else; " \
      "echo Failed to download nicore8/scripts/${bootenv} from ${serverip}.; " \
    "fi; \0" \
  "bootmenu_0=1. Boot from eMMC=boot;\0" \
  "bootmenu_1=2. Launch environment from tftp=run load_env_from_tftp;\0" \
  "bootmenu_2=3. Install environment from tftp=run install_env_from_tftp;\0" \
  "bootmenu_3=4. Reset environment=env default -a -f; saveenv; bootmenu;\0" \
  "bootmenu_4=5. <placeholder>=bootmenu;\0"
#else

// If CONFIG_ENV_WRITEABLE_LIST is se, we explicitly define (whitelist) the set of mutable variables below.
// #define CONFIG_ENV_FLAGS_LIST_STATIC "BOOT_ORDER:sw,BOOT_A_LEFT:dw,BOOT_B_LEFT:dw,FITCONFIG_BASE:sw,FITCONFIG:sw"

// Defaults to booting FIT image 'image.itb' file from FAT fs from eMMC 0.
// RAUC slot A is eMMC GP partition 0 (hardware partition 4).
// RAUC slot B is eMMV GP partition 1 (hardware partition 5).
#define CONFIG_EXTRA_ENV_SETTINGS \
  "BOOT_ORDER=A B\0" \
  "BOOT_A_LEFT=3\0" \
  "BOOT_B_LEFT=3\0" \
  "DEVTYPE=mmc\0" \
  "DEVNUM=0\0" \
  "BOOTPART_A=4\0" \
  "BOOTPART_B=5\0"\
  "FITIMAGE=image.itb\0" \
  "FITCONFIG_BASE=\0" \
  "loadaddr=0x12000000\0" \
  "bootcmd_fit=" \
    "setenv FITCONFIG; " \
    "if test \"x${FITCONFIG_BASE}\" = \"x\"; then " \
      "setenv FITCONFIG_BASE \"#config-core\"; " \
    "fi; " \
    "for BOOT_SLOT in ${BOOT_ORDER}; do " \
      "if test \"x${FITCONFIG}\" != \"x\"; then " \
        "echo Skip remaining; " \
      "elif test \"x${BOOT_SLOT}\" = \"xA\"; then " \
        "if test ${BOOT_A_LEFT} -gt 0; then " \
          "setexpr BOOT_A_LEFT ${BOOT_A_LEFT} - 1; " \
		  "if fatload ${DEVTYPE} ${DEVNUM}.${BOOTPART_A} ${loadaddr} ${FITIMAGE}; then " \
            "setenv FITCONFIG \"${FITCONFIG_BASE}-a\"; " \
		  "fi; " \
        "fi; " \
      "elif test \"x${BOOT_SLOT}\" = \"xB\"; then " \
        "if test ${BOOT_B_LEFT} -gt 0; then " \
          "setexpr BOOT_B_LEFT ${BOOT_B_LEFT} - 1; " \
		  "if fatload ${DEVTYPE} ${DEVNUM}.${BOOTPART_B} ${loadaddr} ${FITIMAGE}; then " \
            "setenv FITCONFIG \"${FITCONFIG_BASE}-b\"; " \
		  "fi; " \
        "fi; " \
      "fi; " \
    "done; " \
    "if test -n \"${FITCONFIG}\"; then " \
      "saveenv; " \
    "else; " \
      "setenv BOOT_A_LEFT 3; " \
      "setenv BOOT_B_LEFT 3; " \
      "saveenv; " \
      "reset; " \
    "fi; " \
    "bootm ${loadaddr}${FITCONFIG}; reset;\0"

#endif // CONFIG_TARGET_LOGOSNICORE8DEV

#endif // __LOGOS_NI8_H__
