/*
 * (C) Copyright 2004-2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mem.h>	/* get mem tables */
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <i2c.h>

/**************************************************************************
 * get_cpu_type() - Read the FPGA Debug registers and provide the DIP switch
 *    settings
 * 1 is on
 * 0 is off
 * Will return Index of type of gpmc
 ***************************************************************************/
u32 get_gpmc0_type(void)
{
	/* Select configuration in chip_sel_sdpv2 */
#ifdef CONFIG_NOR_BOOT
	return 0x1;
#else
	return 0x0;
#endif
}

/****************************************************
 * get_cpu_type() - low level get cpu type
 * - no C globals yet.
 ****************************************************/
u32 get_cpu_type(void)
{
    /* fixme, need to get register defines for 4430 */
    return CPU_4430;
}

unsigned int cortex_a9_rev(void)
{

	unsigned int i;

	/* turn off I/D-cache */
	asm ("mrc p15, 0, %0, c0, c0, 0" : "=r" (i));

	return i;
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
	u32 omap_rev_reg = 0;
	u32 idcode = 0;

	idcode = cortex_a9_rev();
	if (((idcode >> 4) & 0xfff) == 0xc09) {
		idcode &= 0xf;
		switch (idcode) {
		case 1:
			return CPU_4430_ES1;
		case 2:
			omap_rev_reg = (__raw_readl(CONTROL_ID_CODE)  >> 28);
			if (omap_rev_reg == 0x3)
				return CPU_4430_ES21;
			else if (omap_rev_reg >= 0x4)
				return CPU_4430_ES22;
			else
				return CPU_4430_ES20;
		}
	}
	return CPU_4430_ES22;

}

/****************************************************
 * is_mem_sdr() - return 1 if mem type in use is SDR
 ****************************************************/
u32 is_mem_sdr(void)
{
	volatile u32 *burst = (volatile u32 *)(SDRC_MR_0 + 0x0);
	if (*burst == 0x00000031)
		return 1;
	return 0;
}

/***********************************************************
 * get_mem_type() - identify type of mDDR part used.
 ***********************************************************/
u32 get_mem_type(void)
{
	/* Current SDP4430 uses 2x16 MDDR Infenion parts */
	return DDR_DISCRETE;
}

/***********************************************************************
 * get_cs0_size() - get size of chip select 0/1
 ************************************************************************/
u32 get_sdr_cs_size(u32 offset)
{
	u32 size;

	/* get ram size field */
	size = __raw_readl(SDRC_MCFG_0 + offset) >> 8;
	size &= 0x3FF;		/* remove unwanted bits */
	size *= SZ_2M;		/* find size in MB */
	return size;
}

/******************************************************************
 * get_sysboot_value() - get init word settings
 ******************************************************************/
inline u32 get_sysboot_value(void)
{
	return 0x0000003F & __raw_readl(CONTROL_STATUS);
}

/***************************************************************************
 *  get_gpmc0_base() - Return current address hardware will be
 *     fetching from. The below effectively gives what is correct, its a bit
 *   mis-leading compared to the TRM.  For the most general case the mask
 *   needs to be also taken into account this does work in practice.
 *   - for u-boot we currently map:
 *       -- 0 to nothing,
 *       -- 4 to flash
 *       -- 8 to enent
 *       -- c to wifi
 ****************************************************************************/
u32 get_gpmc0_base(void)
{
	u32 b;

	b = __raw_readl(GPMC_CONFIG_CS0 + GPMC_CONFIG7);
	b &= 0x1F;		/* keep base [5:0] */
	b = b << 24;		/* ret 0x0b000000 */
	return b;
}

/*******************************************************************
 * get_gpmc0_width() - See if bus is in x8 or x16 (mainly for nand)
 *******************************************************************/
u32 get_gpmc0_width(void)
{
	return WIDTH_16BIT;
}

/*******************************************************************
 * load_mfg_info() - Reading APPS EEPROM, I2C4 bus for Blaze tablet and
 *                   I2C2 for Blaze/SDP.
 * Format example: 0100APPS750-2143-002    0A00092010
 *                 xxxx                       -----------> format designation
 *                     yyyy                     ---------> board type (4 letters)
 *                         zzzzzzzzzzzzzzzz      --------> board part number
 *                                         wwww    ------> board revision
 *                                             vvvvvv  --> mfg date MMYYYY format
 *******************************************************************/
u32 load_mfg_info(void)
{
	int j;
	uint   addr=12;                  /* offset */
	uint   alen=1;
	uint   length=4;
	u_char chip=0x50;               /*EEPROM addr = 0x50*/
	uint   bus=3;
	uint   speed=400;               /* OMAP_I2C_FAST_MODE */
	unsigned char	linebuf[length];
	unsigned char	*cp;
	u32	omap4_board_revision = 0x10; /* default = 1.0 */

	if (select_bus(bus,speed) == 0 ) { /* configure I2C4 */
		if (i2c_read(chip, addr, alen, linebuf, length) == 0) {
			cp = linebuf;
			/* Read eeprom is so slow, so only significant bytes are considered */
			printf("Blaze Tablet Board: 0100APPS750-");
			for (j=0; j<length; j++) {
				printf("%c", *cp);
				cp++;
			}
			putc ('\n');
			/* revision board is changing continously, in the time it is defined,
			board part number is used instead */
			omap4_board_revision = simple_strtoul(linebuf, NULL, 10);
		}
		else {
			/* TODO. Because apps eeprom for Blaze is connected to I2C2
			   and tablet to I2C4, an error here will indicated sw is
			   running on a Blaze/SDP board. It is a temporal solution
			   in the time apps eeprom for tablet is connected to I2C2
			   or a better decision is taken */
			puts ("Blaze/SDP Board.\n");
			omap4_board_revision = 0x10;
		}
	}
	else {
		printf("Setting bus[%d] to Speed[%d]: ", bus, speed);
		printf("FAILED\n");
	}

	/* recover default status */
	if (select_bus(CFG_I2C_BUS,CFG_I2C_SPEED) != 0) {
		printf("Setting bus[%d] to Speed[%d]: ", CFG_I2C_BUS, CFG_I2C_SPEED);
		printf("FAILED\n");
	}

	return omap4_board_revision;
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * returns:(bit[0-3] sub version, higher bit[7-4] is higher version)
 *************************************************************************/
u32 get_board_rev(void)
{
	return load_mfg_info();
}

/*********************************************************************
 *  display_board_info() - print banner with board info.
 *********************************************************************/
void display_board_info(u32 btype)
{
	char *bootmode[] = {
		"NAND",
		"NOR",
		"ONND",
		"P2a",
		"NOR",
		"NOR",
		"P2a",
		"P2b",
	};
	u32 brev = get_board_rev();
	char cpu_4430s[] = "4430";
	char db_ver[] = "0.0";	 /* board type */
	char mem_sdr[] = "mSDR"; /* memory type */
	char mem_ddr[] = "lpDDR2";
	char t_tst[] = "TST";	 /* security level */
	char t_emu[] = "EMU";
	char t_hs[] = "HS";
	char t_gp[] = "GP";
	char unk[] = "?";
#ifdef CONFIG_LED_INFO
	char led_string[CONFIG_LED_LEN] = { 0 };
#endif

#if defined(L3_165MHZ)
	char p_l3[] = "165";
#elif defined(L3_110MHZ)
	char p_l3[] = "110";
#elif defined(L3_133MHZ)
	char p_l3[] = "133";
#elif defined(L3_100MHZ)
	char p_l3[] = "100"
#endif

#if defined(PRCM_PCLK_OPP1)
	char p_cpu[] = "1";
#elif defined(PRCM_PCLK_OPP2)
	char p_cpu[] = "2";
#elif defined(PRCM_PCLK_OPP3)
	char p_cpu[] = "3";
#elif defined(PRCM_PCLK_OPP4)
	char p_cpu[] = "4"
#endif
	char *cpu_s, *db_s, *mem_s, *sec_s;
	u32 cpu, rev, sec;

	rev = get_cpu_rev();
	cpu = get_cpu_type();
	sec = get_device_type();

	mem_s = mem_ddr;

	cpu_s = cpu_4430s;

	db_s = db_ver;
	db_s[0] += (brev >> 4) & 0xF;
	db_s[2] += brev & 0xF;

	switch (sec) {
	case TST_DEVICE:
		sec_s = t_tst;
		break;
	case EMU_DEVICE:
		sec_s = t_emu;
		break;
	case HS_DEVICE:
		sec_s = t_hs;
		break;
	case GP_DEVICE:
		sec_s = t_gp;
		break;
	default:
		sec_s = unk;
	}

	printf("OMAP%s-%s rev %d, CPU-OPP%s L3-%sMHz\n", cpu_s, sec_s, rev,
		p_cpu, p_l3);
	printf("TI 4430SDP %s Version + %s (Boot %s)\n", db_s,
		mem_s, bootmode[get_gpmc0_type()]);
#ifdef CONFIG_LED_INFO
	/* Format: 0123456789ABCDEF
	 *         4430C GP L3-100 NAND
	 */
	sprintf(led_string, "%5s%3s%3s %4s", cpu_s, sec_s, p_l3,
		bootmode[get_gpmc0_type()]);
	/* reuse sec */
	for (sec = 0; sec < CONFIG_LED_LEN; sec += 2) {
		/* invert byte loc */
		u16 val = led_string[sec] << 8;
		val |= led_string[sec + 1];
		__raw_writew(val, LED_REGISTER + sec);
	}
#endif

}

/********************************************************
 *  get_base(); get upper addr of current execution
 *******************************************************/
u32 get_base(void)
{
	u32 val;
	__asm__ __volatile__("mov %0, pc \n" : "=r"(val) : : "memory");
	val &= 0xF0000000;
	val >>= 28;
	return val;
}

/********************************************************
 *  running_in_flash() - tell if currently running in
 *   flash.
 *******************************************************/
u32 running_in_flash(void)
{
	if (get_base() < 4)
		return 1;	/* in flash */
	return 0;		/* running in SRAM or SDRAM */
}

/********************************************************
 *  running_in_sram() - tell if currently running in
 *   sram.
 *******************************************************/
u32 running_in_sram(void)
{
	if (get_base() == 4)
		return 1;	/* in SRAM */
	return 0;		/* running in FLASH or SDRAM */
}

/********************************************************
 *  running_in_sdram() - tell if currently running in
 *   sdram.
 *******************************************************/
u32 running_in_sdram(void)
{
	if (get_base() > 4)
		return 1;	/* in sdram */
	return 0;		/* running in SRAM or FLASH */
}

/***************************************************************
 *  get_boot_type() - Is this an XIP type device or a stream one
 *   bits 4-0 specify type.  Bit 5 sys mem/perif
 ***************************************************************/
u32 get_boot_type(void)
{
	u32 v;

    v = get_sysboot_value() & (BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
	return v;
}

/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
	int mode;
	mode = __raw_readl(SECURE_ID_CODE) & (DEVICE_MASK);
	return mode >>= 8;
}
