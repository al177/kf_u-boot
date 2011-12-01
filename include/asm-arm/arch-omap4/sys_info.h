/*
 * (C) Copyright 2006-2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
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

#ifndef _OMAP44XX_SYS_INFO_H_
#define _OMAP44XX_SYS_INFO_H_

#define XDR_POP		5      /* package on package part */
#define SDR_DISCRETE	4      /* 128M memory SDR module*/
#define DDR_STACKED	3      /* stacked part on 2422 */
#define DDR_COMBO	2      /* combo part on cpu daughter card (menalaeus) */
#define DDR_DISCRETE	1      /* 2x16 parts on daughter card */

#define DDR_100		100    /* type found on most mem d-boards */
#define DDR_111		111    /* some combo parts */
#define DDR_133		133    /* most combo, some mem d-boards */
#define DDR_165		165    /* future parts */

#define CPU_4430	0x4430

/* 343x real hardware:
 *  ES1     = rev 0
 */

/* 343x code defines:
 * ES1     = 0+1 = 1
 * ES1     = 1+1 = 1
 */
#define CPU_4430_ES1		1
#define CPU_4430_ES20		2
#define CPU_4430_ES21		3
#define CPU_4430_ES22		4

#define CPU_4430_GP		3
#define CPU_4430_HS		2
#define CPU_4430_EMU		1

/* Currently Virtio models this one */
#define CPU_4430_CHIPID		0x0B68A000

#define GPMC_MUXED		1
#define GPMC_NONMUXED		0

#define TYPE_NAND		0x800	/* bit pos for nand in gpmc reg */
#define TYPE_NOR		0x000
#define TYPE_ONENAND		0x800

#define WIDTH_8BIT		0x0000
#define WIDTH_16BIT		0x1000	/* bit pos for 16 bit in gpmc */

#define I2C_MENELAUS		0x72	/* i2c id for companion chip */
#define I2C_TRITON2		0x4B	/* addres of power group */

#define BOOT_FAST_XIP		0x1f

/* SDP definitions according to FPGA Rev. Is this OK?? */
#define SDP_4430_VIRTIO		0x1
#define SDP_4430_V1		0x2

#define BOARD_4430_LABRADOR	0x80
#define BOARD_4430_LABRADOR_V1	0x1

#endif
