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

/* TODO Is set to 1 for all other iMX6 boards.
 * Our SDRAM is listed with 8 banks in the datasheet, so should it be 3 (2**3=8)?
 */
//#define CONFIG_NR_DRAM_BANKS            1
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
#endif
