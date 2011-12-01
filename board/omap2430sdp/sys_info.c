/*
 * (C) Copyright 2004-2005
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

/****************************************************************************
 * check_fpga_revision number: the rev number should be a or b
 * 0xA203/5 variant did not have it, but the B101 variant has EEPROM update facility
 ***************************************************************************/
static inline u16 check_fpga_rev(void)
{
	return __raw_readw(FPGA_REV_REGISTER);
}

/****************************************************************************
 * check_eeprom_avail: Check FPGA Availability
 * OnBoard DEBUG FPGA registers need to be ready for us to proceed
 * Required to retrieve the bootmode also.
 ***************************************************************************/
int check_eeprom_avail(u32 offset)
{
	u16 memst_reg;
	u16 bit;

	int count = 10000;
	/* old fpga revs dont seem to have this update */
	if (check_fpga_rev() < 0xB000) {
		return 0;
	}
	/* Check if UI revision Name is already updated.
	 * if this is not done, we wait a bit to give a chance
	 * to update. This is nice to do as the Main board FPGA
	 * gets a chance to know off all it's components and we 
	 * can continue to work normally
	 * Currently taking 269* udelay(1000) to update this on 
	 * poweron from flash!!
	 */

	if (offset == EEPROM_MAIN_BRD ) {
		bit = BIT0;
	}
	else if (offset == EEPROM_UI_BRD ) {
		bit = BIT2;
	}

	memst_reg = __raw_readw(I2C2_MEMORY_STATUS_REG);

	while ( (memst_reg & bit) && count ) {
		sdelay(100);
		memst_reg = __raw_readw(I2C2_MEMORY_STATUS_REG);
		count--;
	}
	/* Timed out count will be 0? */
	return count;
}

/**************************************************************************
 * get_cpu_type() - Read the FPGA Debug registers and provide the DIP switch 
 *    settings
 * 1 is on
 * 0 is off
 * Will return Index of type of gpmc
 ***************************************************************************/
u32 get_gpmc0_type(void)
{
	u8 cs;
	if (!check_fpga_rev()) {
		/* we dont have an DEBUG FPGA??? */
		/* Depend on #defines!! default to strata boot return param */
		return 0x07;
	}
	cs = (u8) __raw_readw(DIP_SWITCH_INPUT_REG2);
	/* The bits are inverted- S8 0-2 define the CS0 select */
	return ((~cs) & 0x07);
}

/**************************************************************************
 * get_cpu_type() - low level get cpu type
 * - no C globals yet.
 * - just looking to say if this is a 2422 or 2420 or ...
 * - to start with we will look at switch settings..
 * - 2422 id's same as 2420 for ES1 will rely on H4 board characteristics
 *   (mux for 2420, non-mux for 2422).
 ***************************************************************************/
u32 get_cpu_type(void)
{
	u32 v;
	v = __raw_readl(TAP_IDCODE_REG);
	v &= CPU_24XX_ID_MASK;

	if (v == CPU_2430_CHIPID) {
		return (CPU_2430);
	} else
		return (-1);	/* don't know,return invalid val */
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
	u32 v;
	v = __raw_readl(TAP_IDCODE_REG);
	v = v >> 28;
	return (v + 1);
}

/****************************************************
 * is_mem_sdr() - return 1 if mem type in use is SDR
 ****************************************************/
u32 is_mem_sdr(void)
{
	volatile u32 *burst = (volatile u32 *)(SDRC_MR_0 + SDRC_CS0_OSET);
	if (*burst == H4_2420_SDRC_MR_0_SDR)
		return (1);
	return (0);
}

/***********************************************************
 * get_mem_type() - identify type of mDDR part used.
 * 2422 uses stacked DDR, 2 parts CS0/CS1.
 * 2420 may have 1 or 2, no good way to know...only init 1...
 * when eeprom data is up we can select 1 more.
 *************************************************************/
u32 get_mem_type(void)
{
#if 1
	/* 2340SDP only uses Infineon DDR for now. */
	return (DDR_DISCRETE);
#else
	u32 cpu, sdr = is_mem_sdr();

	cpu = get_cpu_type();
	if (cpu == CPU_2422 || cpu == CPU_2423)
		return (DDR_STACKED);

	if (is_this_24xx_pop())
		return (XDR_POP);

	if (get_board_type() == BOARD_H4_MENELAUS)
		if (sdr)
			return (SDR_DISCRETE);
		else
			return (DDR_COMBO);
	else if (sdr)		/* SDP + SDR kit */
		return (SDR_DISCRETE);
	else
		return (DDR_DISCRETE);	/* origional SDP */
#endif
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
	return (size);
}

/***********************************************************************
 * get_board_type() - get board type based on current production stats.
 *  - NOTE-1-: 2 I2C EEPROMs will someday be populated with proper info.
 *    when they are available we can get info from there.  This should
 *    be correct of all known boards up until today.
 *  - NOTE-2- EEPROMs are populated but they are updated very slowly.  To
 *    avoid waiting on them we will use ES version of the chip to get info.
 *    A later version of the FPGA migth solve their speed issue.
 ************************************************************************/
u32 get_board_type(void)
{
	u32 cpu = get_cpu_rev();

	/* we have sdp/we timed out.. base on 2430 rev */
	/* > ES2 can voltage scale and is bundled w/ T2 */ 
	if(cpu >= CPU_2430_ES2)
		return BOARD_SDP_2430_T2;
	else
		return BOARD_SDP_2430_M1;
}


/***********************************************************************
get_sdp_type() - get the sdp type : SDP or GDP based on the board id name tag.
		On any failure conditions - due to lack of fpga information or
		timeout, always default to SDP.
***********************************************************************/
u32 get_sdp_type(void)
{
	if (check_eeprom_avail(EEPROM_MAIN_BRD)) {
		volatile char *m_brd_name = (char *)(EEPROM_MAIN_BRD+0x08);
		char t_brd_name[] = GDP_MB_EE_NAME;
		int count=0;

		/* scan to check if all the characters match */
		/* Move ahead to name location */
		count = sizeof(t_brd_name) -2;
		while ((m_brd_name[count] == t_brd_name[count]) && count) {
			count--;
		}
		/* Match?? */
		if (!count) {
			/* GDP!! */
			return BOARD_GDP_2430_T2;
		}
		else  {
			return BOARD_SDP_2430_T2;
		}
	}
	else return BOARD_SDP_2430_T2;
}

/******************************************************************
 * get_sysboot_value() - get init word settings (dip switch on h4)
 ******************************************************************/
inline u32 get_sysboot_value(void)
{
	return (0x00000FFF & __raw_readl(CONTROL_STATUS));
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
	return (b);
}

/*******************************************************************
 * get_gpmc0_width() - See if bus is in x8 or x16 (mainly for nand)
 *******************************************************************/
u32 get_gpmc0_width(void)
{
	return (WIDTH_16BIT);
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
{
	u32 i = 0, val;
	do {
		++i;
		val = __raw_readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return (1);
		if (i == bound)
			return (0);
	} while (1);
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * returns:(bit[0-3] sub version, higher bit[7-4] is higher version)
 * 0x00 = old H4 SDP
 * 0x01 = 0.1. wakeup
 * 0x10 = 1.0. Adds OneNAND, Sibley NOR, new camera
 * 0x11 = 1.1. Adds T2
 * 0x20 = 2.0. ES2.0 Silicon
 * TODO: Figure a way out to differentiate b/w various board versions 
 *************************************************************************/
u32 get_board_rev(void)
{
	/* Currently reading EEPROM reg to get UI board version and try it out */
	volatile char *ui_brd_name = (char *)EEPROM_UI_BRD;
	char enhanced_ui_brd_name[] = ENHANCED_UI_EE_NAME;
	int count = 0;
	if (!check_eeprom_avail(EEPROM_UI_BRD)) {
		/* timed out OR fpga rev not found!! */
		/* Assume 1.0 */
		return 0x01;
	}
	/* Move ahead to name location */
	ui_brd_name += 0x08;
	count = sizeof(enhanced_ui_brd_name) - 2;
	while ((enhanced_ui_brd_name[count] == ui_brd_name[count]) && count) {
		count--;
	}
	/* Match?? */
	if (!count) {
		/* Enhanced UI board.. SDP1.1 */
		return 0x11;
	}
	/* Legacy UI - hope they are all 1.0 boards.. */
	return (0x10);
}

/*********************************************************************
 *  display_board_info() - print banner with board info.
 *********************************************************************/
void display_board_info(u32 btype)
{
	char *bootmode[] = {
		"ONND",
		"SIB1",
		"SIB0",
		"NAND",
		"SIB1",
		"SIB0",
		"NOR",
		"NOR",
	};
	u32 brev = get_board_rev();
	char cpu_2430s[] = "2430C";
	char db_ver[] = "0.0";	/* board type */
	char mem_sdr[] = "mSDR";	/* memory type */
	char mem_ddr[] = "mDDR";
	char t_tst[] = "TST";	/* security level */
	char t_emu[] = "EMU";
	char t_hs[] = "HS";
	char t_gp[] = "GP";
	char unk[] = "?";
	char t_sdp[] = "2430SDP";
	char t_gdp[] = "2430GDP";
#ifdef CONFIG_LED_INFO
	char led_string[CONFIG_LED_LEN] = { 0 };
#endif

#if defined(PRCM_CONFIG_2)
	char prcm[] = "#2";
#elif defined(PRCM_CONFIG_3)
	char prcm[] = "#3";
#elif defined(PRCM_CONFIG_5A)
	char prcm[] = "#5A";
#elif defined(PRCM_CONFIG_5B)
	char prcm[] = "#5B"
#endif
	char *cpu_s, *db_s, *mem_s, *sec_s, *sdp;
	u32 cpu, rev, sec;

	rev = get_cpu_rev();
	cpu = get_cpu_type();
	sec = get_device_type();

	if (is_mem_sdr())
		mem_s = mem_sdr;
	else
		mem_s = mem_ddr;

	cpu_s = cpu_2430s;

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

	if (get_sdp_type() == BOARD_GDP_2430_T2) {
		sdp=t_gdp;
	} else {
		sdp=t_sdp;
	}
	printf("OMAP%s-%s revision %d, PRCM %s\n", cpu_s, sec_s, rev, prcm);
	printf("TI %s %s Version + %s (Boot %s)\n",sdp, db_s,
	       mem_s, bootmode[get_gpmc0_type()]);
#ifdef CONFIG_LED_INFO
	/* Format: 0123456789ABCDEF
	 *         2430C GP#5A NAND
	 */
	sprintf(led_string, "%5s%3s%3s %4s", cpu_s, sec_s, prcm,
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
	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return (val);
}

/********************************************************
 *  running_in_flash() - tell if currently running in
 *   flash.
 *******************************************************/
u32 running_in_flash(void)
{
	if (get_base() < 4)
		return (1);	/* in flash */
	return (0);		/* running in SRAM or SDRAM */
}

/********************************************************
 *  running_in_sram() - tell if currently running in
 *   sram.
 *******************************************************/
u32 running_in_sram(void)
{
	if (get_base() == 4)
		return (1);	/* in SRAM */
	return (0);		/* running in FLASH or SDRAM */
}

/********************************************************
 *  running_in_sdram() - tell if currently running in
 *   flash.
 *******************************************************/
u32 running_in_sdram(void)
{
	if (get_base() > 4)
		return (1);	/* in sdram */
	return (0);		/* running in SRAM or FLASH */
}

/*************************************************************
 *  running_from_internal_boot() - am I boot through mask rom.
 *************************************************************/
u32 running_from_internal_boot(void)
{
	u32 v;

	v = get_sysboot_value() & (BIT2 | BIT1 | BIT0);
	/* external boot settings bit1 == bit2 */
	if (((v & BIT1) && (v & BIT2)) || (!(v & BIT1) && !(v & BIT2)))
		v = 0;
	else			/* all other defined combos are internal */
		v = 1;
	return v;
}

/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
	int mode;
	mode = __raw_readl(CONTROL_STATUS) & (DEVICE_MASK);
	return (mode >>= 8);
}
