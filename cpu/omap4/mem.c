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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include <asm/arch/clocks.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <command.h>

/****** DATA STRUCTURES ************/

/* Only One NAND allowed on board at a time.
 * The GPMC CS Base for the same
 */
unsigned int nand_cs_base;
unsigned int onenand_cs_base;
unsigned int boot_flash_base;
unsigned int boot_flash_off;
unsigned int boot_flash_sec;
unsigned int boot_flash_type;
volatile unsigned int boot_flash_env_addr;
/* help common/env_flash.c */
#ifndef CFG_ENV_IS_NOWHERE
char *env_name_spec;
/* update these elsewhere */
env_t *env_ptr;

ulong NOR_FLASH_BANKS_LIST[CFG_MAX_FLASH_BANKS];

int NOR_MAX_FLASH_BANKS;           /* max number of flash banks */

uchar(*boot_env_get_char_spec) (int index);
int (*boot_env_init) (void);
int (*boot_saveenv) (void);
void (*boot_env_relocate_spec) (void);


/* Global fellows */
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
u8 is_nand;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_FLASH)
u8 is_flash;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)
u8 is_onenand;
#endif

#endif	/* #ifndef CFG_ENV_IS_NOWHERE */

/* SDP4430 Board CS Organization
 * Two PISMO connections are specified. PISMO1 is first and default PISMO board
 * PISMO2 is a 2nd stacked PISMOv2 board and is meant for vendor extensions.
 */
static const unsigned char chip_sel[][GPMC_MAX_CS] = {
/* GPMC CS Indices (ON=0, OFF=1)*/
/* S8- 1   2   3 IDX    CS0,       CS1,      CS2 ..                    CS7  */
/*ON  ON  ON */{PISMO1_NOR, PISMO1_NAND, PISMO1_ONENAND, DBG_MPDB, 0, 0, 0, 0},
/*ON  ON OFF */{PISMO1_ONENAND, PISMO1_NAND, PISMO1_NOR, DBG_MPDB, 0, 0, 0, 0},
/*ON  OFF ON */{PISMO1_NAND, PISMO1_ONENAND, PISMO1_NOR, DBG_MPDB, 0, 0, 0, 0},
/*ON OFF OFF */{PISMO2_CS0, PISMO1_ONENAND, PISMO1_NOR, DBG_MPDB, 0, 0, 0, 0},
/*OFF ON  ON*/{PISMO1_NOR, PISMO2_CS1, PISMO1_ONENAND, DBG_MPDB, 0, 0, 0, 0},
/*OFF ON  OFF*/{PISMO1_NOR, PISMO2_CS1, PISMO2_CS0, DBG_MPDB, 0, 0, 0, 0},
/*OFF OFF ON*/{PISMO2_CS0, PISMO1_NOR, PISMO2_CS1, DBG_MPDB, 0, 0, 0, 0},
/*OFF OFF OFF*/{PISMO2_CS1, PISMO1_NOR,  PISMO2_CS0, DBG_MPDB, 0, 0, 0, 0}
};

/* SDP4430 Virtio Board CS organization
 * Need to be configured in user parameters
 */
static const unsigned char chip_sel_sdpv2[][GPMC_MAX_CS] = {
/* GPMC CS Indices (ON=0, OFF=1)*/
/* S8-1 2 3 4 IDX   CS0,       CS1,      CS2 ..                    CS7  */
{PISMO1_NAND, PISMO1_ONENAND, PISMO1_NOR, 0 , 0, 0, 0, 0},
{PISMO1_NOR, PISMO1_ONENAND, 0, 0, 0, 0, 0, 0},
};

/* Values for each of the chips */
static u32 gpmc_mpdb_v2[GPMC_MAX_REG] = {
	SDPV2_MPDB_GPMC_CONFIG1,
	SDPV2_MPDB_GPMC_CONFIG2,
	SDPV2_MPDB_GPMC_CONFIG3,
	SDPV2_MPDB_GPMC_CONFIG4,
	SDPV2_MPDB_GPMC_CONFIG5,
	SDPV2_MPDB_GPMC_CONFIG6, 0
};
static u32 gpmc_stnor[GPMC_MAX_REG] = {
	STNOR_GPMC_CONFIG1,
	STNOR_GPMC_CONFIG2,
	STNOR_GPMC_CONFIG3,
	STNOR_GPMC_CONFIG4,
	STNOR_GPMC_CONFIG5,
	STNOR_GPMC_CONFIG6, 0
};
static u32 gpmc_pismo2[GPMC_MAX_REG] = {
	P2_GPMC_CONFIG1,
	P2_GPMC_CONFIG2,
	P2_GPMC_CONFIG3,
	P2_GPMC_CONFIG4,
	P2_GPMC_CONFIG5,
	P2_GPMC_CONFIG6, 0
};
/********** Functions ****/

/* ENV Functions */
#ifndef CFG_ENV_IS_NOWHERE
uchar env_get_char_spec(int index)
{
	if (!boot_env_get_char_spec)
		puts("ERROR!! env_get_char_spec not available\n");
	else
		return boot_env_get_char_spec(index);
	return 0;
}
int env_init(void)
{
	if (!boot_env_init)
		puts("ERROR!! boot_env_init not available\n");
	else
		return boot_env_init();
	return -1;
}
int saveenv(void)
{
	if (!boot_saveenv)
		puts("ERROR!! boot_saveenv not available\n");
	else
		return boot_saveenv();
	return -1;
}
void env_relocate_spec(void)
{
	if (!boot_env_relocate_spec)
		puts("ERROR!! boot_env_relocate_spec not available\n");
	else
		boot_env_relocate_spec();
}
#endif


/**************************************************************************
 * make_cs1_contiguous() - for es2 and above remap cs1 behind cs0 to allow
 *  command line mem=xyz use all memory with out discontinuous support
 *  compiled in.  Could do it at the ATAG, but there really is two banks...
 * Called as part of 2nd phase DDR init.
 **************************************************************************/
void make_cs1_contiguous(void)
{
	u32 size, a_add_low, a_add_high;

	size = get_sdr_cs_size(0x0);
	size /= SZ_32M;		/* find size to offset CS1 */
	a_add_high = (size & 3) << 8;	/* set up low field */
	a_add_low = (size & 0x3C) >> 2;	/* set up high field */
	__raw_writel((a_add_high | a_add_low), SDRC_CS_CFG);

}

/********************************************************
 *  mem_ok() - test used to see if timings are correct
 *             for a part. Helps in guessing which part
 *             we are currently using.
 *******************************************************/
u32 mem_ok(void)
{
	u32 val1, val2, addr;
	u32 pattern = 0x12345678;

	addr = 0x80000000;

	__raw_writel(0x0, addr + 0x400);	/* clear pos A */
	__raw_writel(pattern, addr);	/* pattern to pos B */
	__raw_writel(0x0, addr + 4);	/* remove pattern off the bus */
	val1 = __raw_readl(addr + 0x400);	/* get pos A value */
	val2 = __raw_readl(addr);	/* get val2 */

	if ((val1 != 0) || (val2 != pattern))	/* see if pos A value changed */
		return 0;
	else
		return 1;
}

/********************************************************
 *  dmm_init() - init the sdrc chip selects CS0 and CS1
 *  - early init routines, called from flash or
 *  SRAM.
 *******************************************************/
void dmm_init(void)
{
#define EARLY_INIT 1
	do_dmm_init(0x0, EARLY_INIT); /* only init up first bank here */
}

/*************************************************************************
 * do_dmm_init(): initialize the SDRAM for use.
 *  -code sets up SDRAM basic SDRC timings for CS0
 *  -optimal settings can be placed here, or redone after i2c
 *      inspection of board info
 *
 *  - code called once in C-Stack only context for CS0 and a possible 2nd
 *      time depending on memory configuration from stack+global context
 **************************************************************************/
void do_dmm_init(u32 offset, u32 early)
{
	u32 common = 0, cs0 = 0, pmask = 0, pass_type, mtype, mono = 0;

	if (offset == 0x0)
		cs0 = common = 1; /* int regs shared between both chip select*/

	pass_type = IP_DDR;
	mtype = get_mem_type();

	/* If this is a 2nd pass init of a CS1, make it contiguous with CS0 */
	if (!early && ((mtype == DDR_COMBO)
		       || (mtype == DDR_STACKED))) {
		if (mtype == DDR_COMBO) {
			pmask = BIT2;	/* if shared CKE don't use */
			pass_type = COMBO_DDR;	/* CS1 config */
			__raw_writel((__raw_readl(SDRC_POWER)) & ~pmask,
				     SDRC_POWER);
		}
		make_cs1_contiguous();
	}

next_mem_type:
	if (common) {
		__raw_writel(0x80301000, DMM_LISA_MAP);
#ifdef POWER_SAVE
	__raw_writel(__raw_readl(DMM_SYSCONFIG) | SMART_IDLE, DMM_SYSCONFIG);
#endif
	}

	if (mono)		/* Used if Stacked memory is on CS1 only */
		make_cs1_contiguous();	/* make CS1 appear at CS0 */

	if (mem_ok())
		return;		/* STACKED, other configured type */
	++pass_type;		/* IPDDR->COMBODDR->IPSDR for CS0 */
	goto next_mem_type;
}

void enable_gpmc_config(u32 *gpmc_config, u32 gpmc_base, u32 base, u32 size)
{
	__raw_writel(0, GPMC_CONFIG7 + gpmc_base);
	sdelay(1000);
	/* Delay for settling */
	__raw_writel(gpmc_config[0], GPMC_CONFIG1 + gpmc_base);
	__raw_writel(gpmc_config[1], GPMC_CONFIG2 + gpmc_base);
	__raw_writel(gpmc_config[2], GPMC_CONFIG3 + gpmc_base);
	__raw_writel(gpmc_config[3], GPMC_CONFIG4 + gpmc_base);
	__raw_writel(gpmc_config[4], GPMC_CONFIG5 + gpmc_base);
	__raw_writel(gpmc_config[5], GPMC_CONFIG6 + gpmc_base);
	/* Enable the config */
	__raw_writel((((size & 0xF) << 8) | ((base >> 24) & 0x3F) |
		      (1 << 6)), GPMC_CONFIG7 + gpmc_base);
	sdelay(2000);
}

/*****************************************************
 * gpmc_init(): init gpmc bus
 *****************************************************/
void gpmc_init(void)
{
        u32 cs_base = GPMC_CONFIG_CS0;
        u8 i;

        /* Disable all Chip Selects as there is
         * no device on gpmc right now
         */
        for (i = 0; i < 8; i++)
                sr32(cs_base + GPMC_CONFIG7 + 0x30*i, 6, 1, 0);
}
