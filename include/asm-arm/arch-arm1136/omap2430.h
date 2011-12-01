/*
 * (C) Copyright 2004-2005
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _OMAP2430_SYS_H_
#define _OMAP2430_SYS_H_

#include <asm/arch/sizes.h>

/*
 * 2430 specific Section
 */

/* Stuff on L3 Interconnect */
#define SMX_APE_BASE 0x68000000

/* L3 Firewall */
#define A_REQINFOPERM0        (SMX_APE_BASE + 0x05048)
#define A_READPERM0           (SMX_APE_BASE + 0x05050)
#define A_WRITEPERM0          (SMX_APE_BASE + 0x05058)

/* GPMC */
#define OMAP24XX_GPMC_BASE    (0x6E000000)

/* SMS */
#define OMAP24XX_SMS_BASE 0x6C000000

/* SDRC */
#define OMAP24XX_SDRC_BASE 0x6D000000

/*
 * L4 Peripherals - L4 Wakeup and L4 Core now
 */
#define OMAP243X_CORE_L4_IO_BASE        0x48000000

#define OMAP243X_WAKEUP_L4_IO_BASE      0x49000000

#define OMAP24XX_L4_IO_BASE	OMAP243X_CORE_L4_IO_BASE

/* CONTROL */
#define OMAP24XX_CTRL_BASE    (OMAP243X_WAKEUP_L4_IO_BASE+0x2000)

/* TAP information */
#define OMAP24XX_TAP_BASE     (OMAP243X_WAKEUP_L4_IO_BASE+0xA000)

/* UART */
#define OMAP24XX_UART1	      (OMAP24XX_L4_IO_BASE+0x6a000)
#define OMAP24XX_UART2	       (OMAP24XX_L4_IO_BASE+0x6c000)
#define OMAP24XX_UART3        (OMAP24XX_L4_IO_BASE+0x6e000)

/* General Purpose Timers */
#define OMAP24XX_GPT1         (OMAP243X_WAKEUP_L4_IO_BASE+0x18000)
#define OMAP24XX_GPT2         (OMAP24XX_L4_IO_BASE+0x2A000)
#define OMAP24XX_GPT3         (OMAP24XX_L4_IO_BASE+0x78000)
#define OMAP24XX_GPT4         (OMAP24XX_L4_IO_BASE+0x7A000)
#define OMAP24XX_GPT5         (OMAP24XX_L4_IO_BASE+0x7C000)
#define OMAP24XX_GPT6         (OMAP24XX_L4_IO_BASE+0x7E000)
#define OMAP24XX_GPT7         (OMAP24XX_L4_IO_BASE+0x80000)
#define OMAP24XX_GPT8         (OMAP24XX_L4_IO_BASE+0x82000)
#define OMAP24XX_GPT9         (OMAP24XX_L4_IO_BASE+0x84000)
#define OMAP24XX_GPT10        (OMAP24XX_L4_IO_BASE+0x86000)
#define OMAP24XX_GPT11        (OMAP24XX_L4_IO_BASE+0x88000)
#define OMAP24XX_GPT12        (OMAP24XX_L4_IO_BASE+0x8A000)


/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE              (OMAP243X_WAKEUP_L4_IO_BASE+0x14000)
#define WD2_BASE              (OMAP243X_WAKEUP_L4_IO_BASE+0x16000)
#define WD3_BASE              (OMAP24XX_L4_IO_BASE+0x24000) /* not present */
#define WD4_BASE              (OMAP24XX_L4_IO_BASE+0x26000)

/* 32KTIMER */
#define SYNC_32KTIMER_BASE    (OMAP243X_WAKEUP_L4_IO_BASE+0x20000)
#define S32K_CR               (SYNC_32KTIMER_BASE+0x10)

/* PRCM */
#define OMAP24XX_CM_BASE (OMAP243X_WAKEUP_L4_IO_BASE+0x06000)

/*
 * SDP2430 specific Section
 */

/*
 *  The 243x's chip selects are programmable.  The mask ROM
 *  does configure CS0 to 0x08000000 before dispatch.  So, if
 *  you want your code to live below that address, you have to
 *  be prepared to jump though hoops, to reset the base address.
 *  Same as in SDP2430
 */
#if (CONFIG_2430SDP) 

/* base address for indirect vectors (internal boot mode) */
#define SRAM_OFFSET0          0x40000000
#define SRAM_OFFSET1          0x00200000
#define SRAM_OFFSET2          0x0000F800
#define SRAM_VECT_CODE       (SRAM_OFFSET0|SRAM_OFFSET1|SRAM_OFFSET2)

#define LOW_LEVEL_SRAM_STACK  0x4020FFFC

#define PERIFERAL_PORT_BASE   0x480FE003

/* FPGA on Debug board.*/
#define ETH_CONTROL_REG       (DEBUG_BASE+0x30b)
#define LAN_RESET_REGISTER    (DEBUG_BASE+0x1c)
#define DIP_SWITCH_INPUT_REG2 (DEBUG_BASE+0x60)
#define LED_REGISTER          (DEBUG_BASE+0x40)
#define FPGA_REV_REGISTER     (DEBUG_BASE+0x10)
#define EEPROM_MAIN_BRD       (DEBUG_BASE+0x10000+0x1800)
#define EEPROM_CONN_BRD       (DEBUG_BASE+0x10000+0x1900)
#define EEPROM_UI_BRD         (DEBUG_BASE+0x10000+0x1A00)
#define EEPROM_MCAM_BRD       (DEBUG_BASE+0x10000+0x1B00)
#define I2C2_MEMORY_STATUS_REG  (DEBUG_BASE+0x10000+0xA)
#define ENHANCED_UI_EE_NAME   "750-2038"
#define GDP_MB_EE_NAME   "750-2031-3"

#endif  /* endif (CONFIG_2430SDP)  */

#endif
