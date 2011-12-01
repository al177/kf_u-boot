/*
 * (C) Copyright 2004
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

#ifndef _OMAP2420_SYS_H_
#define _OMAP2420_SYS_H_

#include <asm/arch/sizes.h>

/*
 * 2420 specific Section
 */
#define OMAP24XX_L4_IO_BASE  (0x48000000)

/* L3 Firewall */
#define A_REQINFOPERM0        0x68005048
#define A_READPERM0           0x68005050
#define A_WRITEPERM0          0x68005058

/* CONTROL */
#define OMAP24XX_CTRL_BASE    (0x48000000)

/* TAP information */
#define OMAP24XX_TAP_BASE     (0x48014000)

/* GPMC */
#define OMAP24XX_GPMC_BASE    (0x6800A000)

/* SMS */
#define OMAP24XX_SMS_BASE 0x68008000

/* SDRC */
#define OMAP24XX_SDRC_BASE 0x68009000

/* UART */
#define OMAP24XX_UART1	      0x4806A000
#define OMAP24XX_UART2	      0x4806C000
#define OMAP24XX_UART3        0x4806E000

/* General Purpose Timers */
#define OMAP24XX_GPT1         0x48028000
#define OMAP24XX_GPT2         0x4802A000
#define OMAP24XX_GPT3         0x48078000
#define OMAP24XX_GPT4         0x4807A000
#define OMAP24XX_GPT5         0x4807C000
#define OMAP24XX_GPT6         0x4807E000
#define OMAP24XX_GPT7         0x48080000
#define OMAP24XX_GPT8         0x48082000
#define OMAP24XX_GPT9         0x48084000
#define OMAP24XX_GPT10        0x48086000
#define OMAP24XX_GPT11        0x48088000
#define OMAP24XX_GPT12        0x4808A000


/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE              0x48020000
#define WD2_BASE              0x48022000
#define WD3_BASE              0x48024000
#define WD4_BASE              0x48026000

/* 32KTIMER */
#define SYNC_32KTIMER         0x48004000
#define S32K_CR               (SYNC_32KTIMER+0x10)

/* PRCM */
#define OMAP24XX_CM_BASE 0x48008000

/*
 * H4 specific Section
 */

/*
 *  The 2420's chip selects are programmable.  The mask ROM
 *  does configure CS0 to 0x08000000 before dispatch.  So, if
 *  you want your code to live below that address, you have to
 *  be prepared to jump though hoops, to reset the base address.
 */
#if defined(CONFIG_OMAP24XXH4)

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
#endif  /* endif CONFIG_2420H4 */

#endif
