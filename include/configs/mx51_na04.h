/*
 * Copyright (C) 2014, Emil Renner Berthing <u-boot@esmil.dk>
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Hercules eCAFE Slim/EX HD laptops.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

 /* High Level Configuration Options */

#define CONFIG_MX51	/* in a mx51 */

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_TEXT_BASE	0x97800000

#include <asm/arch/imx-regs.h>

#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_GENERIC_BOARD

#define CONFIG_OF_LIBFDT

#define CONFIG_MACH_TYPE	MACH_TYPE_MX51_NA04
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

#define CONFIG_BOARD_LATE_INIT

/*
 * Hardware drivers
 */
#define CONFIG_FSL_IIM
#define CONFIG_CMD_FUSE

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONFIG_MXC_GPIO

/*
 * SPI Configs
 * */
#define CONFIG_CMD_SPI

#define CONFIG_MXC_SPI

/* PMIC Controller */
#define CONFIG_POWER
#define CONFIG_POWER_SPI
#define CONFIG_POWER_FSL
#define CONFIG_FSL_PMIC_BUS	0
#define CONFIG_FSL_PMIC_CS	0
#define CONFIG_FSL_PMIC_CLK	2500000
#define CONFIG_FSL_PMIC_MODE	(SPI_MODE_0 | SPI_CS_HIGH)
#define CONFIG_FSL_PMIC_BITLEN	32
#define CONFIG_RTC_MC13XXX

/*
 * MMC Configs
 */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	MMC_SDHC1_BASE_ADDR
#define CONFIG_SYS_FSL_ESDHC_NUM	3

#define CONFIG_MMC

#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/*
 * Eth Configs
 */
#define CONFIG_MII

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE		FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR	0x1F
#define CONFIG_ETHPRIME		"FEC0"
#define CONFIG_ARP_TIMEOUT	200UL

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX5
#define CONFIG_USB_STORAGE
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	PORT_PTS_ULPI
#define CONFIG_MXC_USB_FLAGS	MXC_EHCI_POWER_PINS_ENABLED

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/***********************************************************
 * Command definition
 ***********************************************************/

#include <config_cmd_default.h>
#define CONFIG_CMD_BOOTZ
#define CONFIG_SUPPORT_RAW_INITRD
#undef CONFIG_CMD_IMLS
#ifndef CONFIG_FEC_MXC
#undef CONFIG_CMD_NET
#endif

#define CONFIG_CMD_DATE
#define CONFIG_CMD_GPIO
#define CONFIG_CMD_INI

#define CONFIG_BOOTDELAY	0
#define CONFIG_PREBOOT		"if gpio i ${powerkey};then setenv bootdelay 1;fi"

#define CONFIG_LOADADDR		0x92000000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_addr=0x91000000\0" \
	"initrd_addr=0x93000000\0" \
	"powerkey=53\0" \
	"batteryled=78\0" \
	"load=fatload mmc 1:1\0" \
	"section=default\0" \
	"linux=zImage\0" \
	"options=console=ttymxc0,115200n8 console=tty1 root=/dev/mmcblk0p2 ro rootwait\0" \
	"init=" \
		"setenv bootcmd run boot;" \
		"if ${load} ${loadaddr} boot.ini;then " \
			"ini '' ${loadaddr} ${filesize};" \
			"ini ${section} ${loadaddr} ${filesize};" \
		"fi;" \
		"if test -n ${include};then " \
			"if ${load} ${loadaddr} ${include};then " \
				"ini '' ${loadaddr} ${filesize};" \
				"ini ${section} ${loadaddr} ${filesize};" \
			"fi;" \
		"fi\0" \
	"boot=" \
		"setenv bootargs ${options};" \
		"if ${load} ${loadaddr} ${linux};then " \
			"if test -n ${devicetree};then " \
				"if ${load} ${fdt_addr} ${devicetree};then " \
					"if test -n ${initrd};then " \
						"if ${load} ${initrd_addr} ${initrd};then " \
							"bootz ${loadaddr} ${initrd_addr}:${filesize} ${fdt_addr};" \
						"fi;" \
					"else " \
						"bootz ${loadaddr} - ${fdt_addr};" \
					"fi;" \
				"fi;" \
			"else " \
				"if test -n ${initrd};then " \
					"if ${load} ${initrd_addr} ${initrd};then " \
						"bootz ${loadaddr} ${initrd_addr}:${filesize};" \
					"fi;" \
				"else " \
					"bootz ${loadaddr};" \
				"fi;" \
			"fi;" \
		"fi" \

#define CONFIG_BOOTCOMMAND \
	"if gpio i ${powerkey};then " \
		"gpio s ${batteryled};" \
		"setenv section fallback;" \
	"fi;" \
	"run init;" \
	"boot;" \
	"while true;do " \
		"gpio s ${batteryled};sleep 1;" \
		"gpio c ${batteryled};sleep 1;" \
	"done"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START       0x90000000
#define CONFIG_SYS_MEMTEST_END         0x90010000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(512 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_DDR_CLKSEL	0
#define CONFIG_SYS_CLKTL_CBCDR	0x59E35100
#define CONFIG_SYS_MAIN_PWR_ON

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE        (8 * 1024)

#define CONFIG_ENV_IS_NOWHERE
#ifndef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 1
#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#endif

#endif
