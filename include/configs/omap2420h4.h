/*
 * (C) Copyright 2004
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Kshitij Gupta <kshitij@ti.com>
 *
 * Configuration settings for the 242x TI H4 board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARM1136           1    /* This is an arm1136 CPU core */
#define CONFIG_OMAP              1    /* in a TI OMAP core */
#define CONFIG_OMAP24XX	         1    /* which is a 24XX Processor */
#define CONFIG_OMAP242X	         1    /* which is in a 242X */
#define CONFIG_OMAP24XXH4        1    /* and on a H4 board */

/* Clock config to target*/
/* #define PRCM_CONFIG_II	1 */
#define PRCM_CONFIG_III	1

#include <asm/arch/cpu.h>        /* get chip and board defs */

/* On H4, NOR and NAND flash are mutual exclusive.
   Define this if you want to use NAND
 */

/*
#define CFG_NAND_BOOT
#define CFG_NAND_2420
*/

#define V_SCLK                   12000000

/* input clock of PLL */
/* the OMAP24XX H4 has 12MHz, 13MHz, or 19.2Mhz crystal input */
#define CONFIG_SYS_CLK_FREQ      V_SCLK

#undef CONFIG_USE_IRQ                 /* no support for IRQs */
#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG       1    /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG        1
#define CONFIG_REVISION_TAG      1

/*
 * Size of malloc() pool
 */
#define CFG_ENV_SIZE             SZ_128K     /* Total Size of Environment Sector */
#define CFG_MALLOC_LEN           (CFG_ENV_SIZE + SZ_128K)
#define CFG_GBL_DATA_SIZE        128  /* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

/*
 * SMC91c96 Etherent
 */
#define CONFIG_DRIVER_LAN91C96
#define CONFIG_LAN91C96_BASE     (DEBUG_BASE+0x300)
#define CONFIG_LAN91C96_EXT_PHY

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK            (48000000)  /* 48MHz (APLL96/2) */

#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE     (-4)
#define CFG_NS16550_CLK          V_NS16550_CLK   /* 3MHz (1.5MHz*2) */
#define CFG_NS16550_COM1         OMAP24XX_UART1

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1           1    /* UART1 on H4 */


/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX        1
#define CONFIG_BAUDRATE          115200
#define CFG_BAUDRATE_TABLE       {9600, 19200, 38400, 57600, 115200}

#ifdef CFG_NAND_BOOT
#define CONFIG_COMMANDS          (CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_I2C | CFG_CMD_NAND | CFG_CMD_JFFS2)
#else
#define CONFIG_COMMANDS          ((CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_I2C | CFG_CMD_JFFS2) & ~CFG_CMD_AUTOSCRIPT)
#endif
#define CONFIG_BOOTP_MASK        CONFIG_BOOTP_DEFAULT

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

  /*
   * I2C configuration
   */
#if (CONFIG_COMMANDS & CFG_CMD_I2C)
#define CONFIG_HARD_I2C
#define CFG_I2C_SPEED          100
#define CFG_I2C_SLAVE          1
#define CFG_I2C_BUS            0
#define CFG_I2C_BUS_SELECT     1
#define CONFIG_DRIVER_OMAP24XX_I2C 1
#endif

/*
 *  Board NAND Info.
 */
#define CFG_NAND_ADDR NAND_BASE  /* physical address to access nand at CS0*/
#define CFG_NAND_BASE NAND_BASE  /* physical address to access nand at CS0*/

#define CFG_MAX_NAND_DEVICE 1	/* Max number of NAND devices */
#define SECTORSIZE          512

#define ADDR_COLUMN         1
#define ADDR_PAGE           2
#define ADDR_COLUMN_PAGE    3

#define NAND_ChipID_UNKNOWN 0x00
#define NAND_MAX_FLOORS     1
#define NAND_MAX_CHIPS      1

#define NAND_WAIT_READY(nand)  udelay(10)
#define NAND_NO_RB          1

#define CFG_NAND_WP

#define NAND_CTL_CLRALE(nandptr)
#define NAND_CTL_SETALE(nandptr)
#define NAND_CTL_CLRCLE(nandptr)
#define NAND_CTL_SETCLE(nandptr)
#define NAND_DISABLE_CE(nand)
#define NAND_ENABLE_CE(nand)


#define CONFIG_BOOTDELAY         3

#ifdef NFS_BOOT_DEFAULTS
#define CONFIG_BOOTARGS	         "mem=32M console=ttyS0,115200n8 noinitrd root=/dev/nfs rw nfsroot=128.247.77.158:/home/a0384864/wtbu/rootfs ip=dhcp"
#else
#define CONFIG_BOOTARGS          "root=/dev/ram0 rw mem=32M console=ttyS0,115200n8 initrd=0x80600000,8M ramdisk_size=8192"
#endif

#define CONFIG_NETMASK           255.255.254.0
#define CONFIG_IPADDR            128.247.77.90
#define CONFIG_SERVERIP          128.247.77.158
#define CONFIG_BOOTFILE          "uImage"

/*
 * Miscellaneous configurable options
 */
#define V_PROMPT                 "OMAP242x H4 # "

#define CFG_LONGHELP             /* undef to save memory */
#define CFG_PROMPT               V_PROMPT
#define CFG_CBSIZE               256  /* Console I/O Buffer Size */
/* Print Buffer Size */
#define CFG_PBSIZE               (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS              16          /* max number of command args */
#define CFG_BARGSIZE             CFG_CBSIZE  /* Boot Argument Buffer Size */

#define CFG_MEMTEST_START        (OMAP24XX_SDRC_CS0)  /* memtest works on */
#define CFG_MEMTEST_END          (OMAP24XX_SDRC_CS0+SZ_31M)

#undef	CFG_CLKS_IN_HZ           /* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR            (OMAP24XX_SDRC_CS0) /* default load address */

/* The 2420 has 12 GP timers, they can be driven by the SysClk (12/13/19.2) or by
 * 32KHz clk, or from external sig. This rate is divided by a local divisor.
 */
#define V_PVT                    7  /* use with 12MHz/128 */
#define CFG_TIMERBASE            OMAP24XX_GPT2
#define CFG_PVT                  V_PVT  /* 2^(pvt+1) */
#define CFG_HZ	                 ((CONFIG_SYS_CLK_FREQ)/(2 << CFG_PVT))

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE         SZ_128K /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ     SZ_4K   /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ     SZ_4K   /* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS     2                 /* CS1 may or may not be populated */
#define PHYS_SDRAM_1             OMAP24XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE        SZ_32M            /* at least 32 meg */
#define PHYS_SDRAM_2             OMAP24XX_SDRC_CS1

#define PHYS_FLASH_SECT_SIZE     SZ_128K
#define PHYS_FLASH_1             FLASH_BASE	   /* Flash Bank #1 */
#define PHYS_FLASH_SIZE_1        SZ_32M
#define PHYS_FLASH_2             (FLASH_BASE+SZ_32M) /* same cs, 2 chips in series */
#define PHYS_FLASH_SIZE_2        SZ_32M

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_FLASH_BASE           PHYS_FLASH_1
#define CFG_MAX_FLASH_BANKS      2           /* max number of memory banks */
#define CFG_MAX_FLASH_SECT       (259)	     /* max number of sectors on one chip */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE /* Monitor at beginning of flash */
#define CFG_MONITOR_LEN		SZ_256K      /* Reserve 2 sectors */
#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE, CFG_FLASH_BASE + PHYS_FLASH_SIZE_1 }

#ifdef CFG_NAND_BOOT
#define CFG_ENV_IS_IN_NAND	1
#define CFG_ENV_OFFSET	0x80000	/* environment starts here  */
#else
#define CFG_ENV_ADDR             (CFG_FLASH_BASE + SZ_256K)
#define	CFG_ENV_IS_IN_FLASH      1
#define CFG_ENV_SECT_SIZE	PHYS_FLASH_SECT_SIZE
#define CFG_ENV_OFFSET	( CFG_MONITOR_BASE + CFG_MONITOR_LEN ) /* Environment after Monitor */
#endif




/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
#define CFG_FLASH_CFI		1	/* Flash memory is CFI compliant */
#define CFG_FLASH_CFI_DRIVER	1	/* Use drivers/cfi_flash.c */
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* Use buffered writes (~10x faster) */
#define CFG_FLASH_PROTECTION	1	/* Use hardware sector protection */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT     (100*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT     (100*CFG_HZ) /* Timeout for Flash Write */

/* Flash banks JFFS2 should use */
#define CFG_MAX_MTD_BANKS	(CFG_MAX_FLASH_BANKS+CFG_MAX_NAND_DEVICE)
#define CFG_JFFS2_MEM_NAND
#define CFG_JFFS2_FIRST_BANK	1		/* use flash_info[1] */
#define CFG_JFFS2_NUM_BANKS     1

/* GPMC Settings */
#ifdef CFG_NAND_BOOT
/* NAND */
#define FLASH_CONFIGURATION_IDX  1
#else
/* NOR */
#define FLASH_CONFIGURATION_IDX  0 
#endif
#define PROC_NOR_SIZE   GPMC_SIZE_64M
#define PROC_NAND_SIZE  GPMC_SIZE_64M
#define DBG_MPDB_SIZE   GPMC_SIZE_16M


/* Other NAND Access APIs */
#ifdef CFG_NAND_BOOT
#define WRITE_NAND_COMMAND(d, adr) do {*(volatile u16 *)(0x6800A07C)= (d);} while(0)
#define WRITE_NAND_ADDRESS(d, adr) do {*(volatile u16 *)(0x6800A080) = (d);} while(0)
#define WRITE_NAND(d, adr) do {*(volatile u16 *)(0x6800A084)= (d);} while(0)
#define READ_NAND(adr) (*(volatile u16 *)(0x6800A084))
#define NAND_WP_OFF()  do {*(volatile u32 *)(GPMC_CONFIG) |= 0x00000010;} while(0)
#define NAND_WP_ON()  do {*(volatile u32 *)(GPMC_CONFIG) &= ~0x00000010;} while(0)
#define NAND_CTL_CLRALE(nandptr)
#define NAND_CTL_SETALE(nandptr)
#define NAND_CTL_CLRCLE(nandptr)
#define NAND_CTL_SETCLE(nandptr)
#define NAND_DISABLE_CE(nand)
#define NAND_ENABLE_CE(nand)
#define NAND_WAIT_READY(nand)  udelay(10)
#endif /* NAND Commands */

#endif							/* __CONFIG_H */
