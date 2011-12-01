/*
 * (C) Copyright 2004-2006
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
#include <environment.h>
#include <command.h>

/****** DATA STRUCTURES ************/

/* Only One NAND allowed on board at a time. 
 * The GPMC CS Base for the same
 */
unsigned int nand_cs_base = 0;
unsigned int onenand_cs_base = 0;
unsigned int boot_flash_base = 0;
unsigned int boot_flash_off = 0;
unsigned int boot_flash_sec = 0;
unsigned int boot_flash_type = 0;
volatile unsigned int boot_flash_env_addr = 0;
/* help common/env_flash.c */
#ifdef ENV_IS_VARIABLE

ulong NOR_FLASH_BANKS_LIST[CFG_MAX_FLASH_BANKS];

int NOR_MAX_FLASH_BANKS = 0 ;           /* max number of flash banks */

uchar(*boot_env_get_char_spec) (int index);
int (*boot_env_init) (void);
int (*boot_saveenv) (void);
void (*boot_env_relocate_spec) (void);

/* StrataNor */
extern uchar flash_env_get_char_spec(int index);
extern int flash_env_init(void);
extern int flash_saveenv(void);
extern void flash_env_relocate_spec(void);
extern char *flash_env_name_spec;

/* 16 bit NAND */
extern uchar nand_env_get_char_spec(int index);
extern int nand_env_init(void);
extern int nand_saveenv(void);
extern void nand_env_relocate_spec(void);
extern char *nand_env_name_spec;

/* OneNAND */
extern char *onenand_env;
extern uchar onenand_env_get_char_spec(int index);
extern int onenand_env_init(void);
extern int onenand_saveenv(void);
extern void onenand_env_relocate_spec(void);
extern char *onenand_env_name_spec;

/* Global fellows */
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
u8 is_nand = 0;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_FLASH)
u8 is_flash = 0;
#endif

#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)
u8 is_onenand = 0;
#endif

char *env_name_spec = 0;
/* update these elsewhere */
env_t *env_ptr = 0;

#if ((CONFIG_COMMANDS&(CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))
extern env_t *flash_addr;
#endif

#endif				/* ENV_IS_VARIABLE */

/* SDP3430 Board CS Organization 
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

/* SDP3430 V2 Board CS organization
 * Different from SDP3430 V1. Now 4 switches used to specify CS
 */
static const unsigned char chip_sel_sdpv2[][GPMC_MAX_CS] = {
/* GPMC CS Indices (ON=0, OFF=1)*/
/* S8-1 2 3 4 IDX   CS0,       CS1,      CS2 ..                    CS7  */
/*ON ON ON ON*/{PISMO1_NOR, PISMO1_NAND, PISMO1_ONENAND, DBG_MPDB, 0, 0, 0, 0},
/*ON ON ON OFF*/{PISMO1_ONENAND, PISMO1_NAND, PISMO1_NOR, DBG_MPDB, 0, 0, 0, 0},
/*ON ON OFF ON */{PISMO1_NAND, PISMO1_ONENAND, PISMO1_NOR, DBG_MPDB, 0, 0, 0, 0},
/*ON ON OFF OFF*/{PISMO1_NOR, PISMO2_CS0, PISMO2_CS1, DBG_MPDB, 0, 0, 0, 0},
/*ON OFF ON ON*/{PISMO1_ONENAND, PISMO2_CS0, PISMO2_CS1, DBG_MPDB, 0, 0, 0, 0},
/*ON OFF ON OFF*/{PISMO1_NAND, PISMO2_CS0, PISMO2_CS1, DBG_MPDB, 0, 0, 0, 0},
/*ON OFF OFF ON*/{PISMO1_NOR, PISMO2_NAND_CS0, PISMO2_NAND_CS1, DBG_MPDB, 0, 0, 0, 0},
/*ON OFF OFF OFF*/{PISMO1_ONENAND, PISMO2_NAND_CS0, PISMO2_NAND_CS1, DBG_MPDB, 0, 0, 0, 0}
};

/* Values for each of the chips */
static u32 gpmc_mpdb[GPMC_MAX_REG] = {
	MPDB_GPMC_CONFIG1,
	MPDB_GPMC_CONFIG2,
	MPDB_GPMC_CONFIG3,
	MPDB_GPMC_CONFIG4,
	MPDB_GPMC_CONFIG5,
	MPDB_GPMC_CONFIG6, 0
};
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
static u32 gpmc_sibnor[GPMC_MAX_REG] = {
        SIBNOR_GPMC_CONFIG1,
        SIBNOR_GPMC_CONFIG2,
        SIBNOR_GPMC_CONFIG3,
        SIBNOR_GPMC_CONFIG4,
        SIBNOR_GPMC_CONFIG5,
        SIBNOR_GPMC_CONFIG6, 0
};
static u32 gpmc_smnand[GPMC_MAX_REG] = {
	SMNAND_GPMC_CONFIG1,
	SMNAND_GPMC_CONFIG2,
	SMNAND_GPMC_CONFIG3,
	SMNAND_GPMC_CONFIG4,
	SMNAND_GPMC_CONFIG5,
	SMNAND_GPMC_CONFIG6, 0
};
static u32 gpmc_pismo2[GPMC_MAX_REG] = {
	P2_GPMC_CONFIG1,
	P2_GPMC_CONFIG2,
	P2_GPMC_CONFIG3,
	P2_GPMC_CONFIG4,
	P2_GPMC_CONFIG5,
	P2_GPMC_CONFIG6, 0
};
static u32 gpmc_onenand[GPMC_MAX_REG] = {
	ONENAND_GPMC_CONFIG1,
	ONENAND_GPMC_CONFIG2,
	ONENAND_GPMC_CONFIG3,
	ONENAND_GPMC_CONFIG4,
	ONENAND_GPMC_CONFIG5,
	ONENAND_GPMC_CONFIG6, 0
};

/********** Functions ****/

/* ENV Functions */
#ifdef ENV_IS_VARIABLE
uchar env_get_char_spec(int index)
{
	if (!boot_env_get_char_spec) {
		puts("ERROR!! env_get_char_spec not available\n");
	} else
		return boot_env_get_char_spec(index);
	return 0;
}
int env_init(void)
{
	if (!boot_env_init) {
		puts("ERROR!! boot_env_init not available\n");
	} else
		return boot_env_init();
	return -1;
}
int saveenv(void)
{
	if (!boot_saveenv) {
		puts("ERROR!! boot_saveenv not available\n");
	} else
		return boot_saveenv();
	return -1;
}
void env_relocate_spec(void)
{
	if (!boot_env_relocate_spec) {
		puts("ERROR!! boot_env_relocate_spec not available\n");
	} else
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

	size = get_sdr_cs_size(SDRC_CS0_OSET);
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

	addr = OMAP34XX_SDRC_CS0;

	__raw_writel(0x0, addr + 0x400);	/* clear pos A */
	__raw_writel(pattern, addr);	/* pattern to pos B */
	__raw_writel(0x0, addr + 4);	/* remove pattern off the bus */
	val1 = __raw_readl(addr + 0x400);	/* get pos A value */
	val2 = __raw_readl(addr);	/* get val2 */

	if ((val1 != 0) || (val2 != pattern))	/* see if pos A value changed */
		return (0);
	else
		return (1);
}

/********************************************************
 *  sdrc_init() - init the sdrc chip selects CS0 and CS1
 *  - early init routines, called from flash or
 *  SRAM.
 *******************************************************/
void sdrc_init(void)
{
#define EARLY_INIT 1
	do_sdrc_init(SDRC_CS0_OSET, EARLY_INIT); /* only init up first bank here */
}

/*************************************************************************
 * do_sdrc_init(): initialize the SDRAM for use.
 *  -code sets up SDRAM basic SDRC timings for CS0
 *  -optimal settings can be placed here, or redone after i2c
 *      inspection of board info
 *
 *  - code called ones in C-Stack only context for CS0 and a possible 2nd
 *      time depending on memory configuration from stack+global context
 **************************************************************************/
void do_sdrc_init(u32 offset, u32 early)
{
	u32 common = 0, cs0 = 0, pmask = 0, pass_type, mtype, mono = 0;

	if (offset == SDRC_CS0_OSET)
		cs0 = common = 1;	/* int regs shared between both chip select */

    pass_type = IP_DDR;

    /* If this is a 2nd pass init of a CS1, make it contiguous with CS0 */
    if (!early && (((mtype = get_mem_type()) == DDR_COMBO)
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
	if (common) { /* do a SDRC reset between types to clear regs */
		__raw_writel(SOFTRESET, SDRC_SYSCONFIG);	/* reset sdrc */
		wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000);	/* wait on reset */
		__raw_writel(0, SDRC_SYSCONFIG);	/* clear soft reset */
		__raw_writel(SDP_SDRC_SHARING, SDRC_SHARING);
		/* If its a 3430 ES1.0 silicon, configure WAKEUPPROC to 1 as
		per Errata 1.22 */
		/* Need to change the condition to silicon and rev check */
		if(1)
			__raw_writel((__raw_readl(SDRC_POWER)) | WAKEUPPROC
							, SDRC_POWER);
#ifdef POWER_SAVE
		__raw_writel(__raw_readl(SMS_SYSCONFIG) | SMART_IDLE,
			     SMS_SYSCONFIG);
		__raw_writel(SDP_SDRC_SHARING | SMART_IDLE, SDRC_SHARING);
		__raw_writel((__raw_readl(SDRC_POWER) | BIT6), SDRC_POWER);
#endif
	}

    /* set MDCFG_0 values */
    if ((pass_type == IP_DDR) || (pass_type == STACKED)) {
		__raw_writel(SDP_SDRC_MDCFG_0_DDR, SDRC_MCFG_0 + offset);
		if (mono) /* Stacked with memory on CS1 only */
			__raw_writel(SDP_SDRC_MDCFG_MONO_DDR, SDRC_MCFG_0 + offset);
	} else if (pass_type == COMBO_DDR) {	/* (combo-CS0/CS1) */
		__raw_writel(SDP_COMBO_MDCFG_0_DDR, SDRC_MCFG_0 + offset);
	} else if (pass_type == IP_SDR) {	/* ip sdr-CS0 */
		__raw_writel(SDP_SDRC_MDCFG_0_SDR, SDRC_MCFG_0 + offset);
	}

    /* Set ACTIM values */
    if (cs0) {
		__raw_writel(SDP_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_0);
		__raw_writel(SDP_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_0);
	} else {
		__raw_writel(SDP_SDRC_ACTIM_CTRLA_0, SDRC_ACTIM_CTRLA_1);
		__raw_writel(SDP_SDRC_ACTIM_CTRLB_0, SDRC_ACTIM_CTRLB_1);
	}
	__raw_writel(SDP_SDRC_RFR_CTRL, SDRC_RFR_CTRL_0 + offset);

	/* init sequence for mDDR/mSDR using manual commands (DDR is different) */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0 + offset);
	sdelay(5000);	/* supposed to be 100us per design spec for mddr/msdr */
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0 + offset);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0 + offset);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0 + offset);

    /* Set MR0 values */
    if (pass_type == IP_SDR)
		__raw_writel(SDP_SDRC_MR_0_SDR, SDRC_MR_0 + offset);
	else
		__raw_writel(SDP_SDRC_MR_0_DDR, SDRC_MR_0 + offset);

    /* setup 343x DLL values (DDR only) */
    if (common && (pass_type != IP_SDR)) {
		__raw_writel(SDP_SDRC_DLLAB_CTRL, SDRC_DLLA_CTRL);
		sdelay(0x2000);	/* give time to lock, at least 1000 L3 */
	}
	sdelay(0x1000);

	if (mono)		/* Used if Stacked memory is on CS1 only */
		make_cs1_contiguous();	/* make CS1 appear at CS0 */

	if (mem_ok())
		return;		/* STACKED, other configured type */
	++pass_type;		/* IPDDR->COMBODDR->IPSDR for CS0 */
	goto next_mem_type;
}

void enable_gpmc_config(u32 * gpmc_config, u32 gpmc_base, u32 base, u32 size)
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
 * Init GPMC for x16, MuxMode (SDRAM in x32).
 * This code can only be executed from SRAM or SDRAM.
 *****************************************************/
void gpmc_init(void)
{
/* putting a blanket check on GPMC based on ZeBu for now */
#ifndef CONFIG_3430ZEBU
	u32 mux = 0, mwidth;
	u32 *gpmc_config = NULL;
	u32 gpmc_base = 0;
	u32 base = 0;
	u8 idx = 0;
	u32 size = 0;
	u32 f_off = CFG_MONITOR_LEN;
	u32 f_sec = 0;
	u32 config = 0;
	unsigned char *config_sel = NULL;
	u32 i=0;
	
	mux = BIT9;
	mwidth = get_gpmc0_width();

	/* global settings */
	__raw_writel(0x10, GPMC_SYSCONFIG);	/* smart idle */
	__raw_writel(0x0, GPMC_IRQENABLE);	/* isr's sources masked */
	__raw_writel(0, GPMC_TIMEOUT_CONTROL);	/* timeout disable */

	/* For SDPV2, FPGA uses WAIT1 line in active low mode */
	if(get_board_type() == SDP_3430_V2) {
		config = __raw_readl(GPMC_CONFIG);
		config &= (~0xf00);
		__raw_writel(config, GPMC_CONFIG);
	}

	/* Disable the GPMC0 config set by ROM code
	 * It conflicts with our MPDB (both at 0x08000000)
	 */
	__raw_writel(0 , GPMC_CONFIG7 + GPMC_CONFIG_CS0);
	sdelay(1000);
	
	if(get_board_type() == SDP_3430_V2)
		gpmc_config = gpmc_mpdb_v2;
	else
		gpmc_config = gpmc_mpdb;

	/* GPMC3 is always MPDB.. need to know the chip info */
	gpmc_base = GPMC_CONFIG_CS0 + (3 * GPMC_CONFIG_WIDTH);	
	gpmc_config[0] |= mux;
	enable_gpmc_config(gpmc_config, gpmc_base, DEBUG_BASE, DBG_MPDB_SIZE);

	/* Look up chip select map */
	idx = get_gpmc0_type();
	if(get_board_type() == SDP_3430_V2)
		config_sel = (unsigned char *)(chip_sel_sdpv2[idx]);
	else
		config_sel = (unsigned char *)(chip_sel[idx]);

	/* Initialize each chip selects timings (may be to 0) */
	for (idx = 0; idx < GPMC_MAX_CS; idx++) {
		gpmc_base = GPMC_CONFIG_CS0 + (idx * GPMC_CONFIG_WIDTH);
		switch (config_sel[idx]) {
		case PISMO1_NOR:
			if(get_board_type() == SDP_3430_V2) {
				gpmc_config = gpmc_sibnor;
				f_sec = SZ_256K;
				NOR_MAX_FLASH_BANKS = 1;
				size = PISMO1_NOR_SIZE_SDPV2;
				for(i=0; i < NOR_MAX_FLASH_BANKS; i++)
					NOR_FLASH_BANKS_LIST[i] = 
						FLASH_BASE_SDPV2 + PHYS_FLASH_SIZE_SDPV2*i;
			}
			else {
				gpmc_config = gpmc_stnor;
				f_sec = SZ_128K;
				NOR_MAX_FLASH_BANKS = 2;
				size = PISMO1_NOR_SIZE;
				for(i=0; i < NOR_MAX_FLASH_BANKS; i++)
					NOR_FLASH_BANKS_LIST[i] = 
						FLASH_BASE_SDPV1 + PHYS_FLASH_SIZE*i;
			}
			gpmc_config[0] |= mux | TYPE_NOR | mwidth;
			base = NOR_FLASH_BANKS_LIST[0];
			is_flash = 1;
			break;
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
		case PISMO1_NAND:
			base = PISMO1_NAND_BASE;
			size = PISMO1_NAND_SIZE;
			gpmc_config = gpmc_smnand;
			nand_cs_base = gpmc_base;
			f_off = SMNAND_ENV_OFFSET;
			is_nand = 1;
			break;
#endif
		case PISMO2_CS0:
		case PISMO2_CS1:
			base = PISMO2_BASE;
			size = PISMO2_SIZE;
			gpmc_config = gpmc_pismo2;
			gpmc_config[0] |= mux | TYPE_NOR | mwidth;
			break;
/* Either OneNand or Normal Nand at a time!! */
#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)
		case PISMO1_ONENAND:
			base = PISMO1_ONEN_BASE;
			size = PISMO1_ONEN_SIZE;
			gpmc_config = gpmc_onenand;
			onenand_cs_base = gpmc_base;
			f_off = ONENAND_ENV_OFFSET;
			is_onenand = 1;
			break;
#endif
		default:
			/* MPDB/Unsupported/Corrupt config- try Next GPMC CS!!!! */
			continue;
		}

		/* handle boot CS0 */
		if (idx == 0) {
			boot_flash_base = base;
			boot_flash_off = f_off;
			boot_flash_sec = f_sec;
			boot_flash_type = config_sel[idx];
			boot_flash_env_addr = f_off;
#ifdef ENV_IS_VARIABLE
			switch (config_sel[0]) {
			case PISMO1_NOR:
				boot_env_get_char_spec =
				    flash_env_get_char_spec;
				boot_env_init = flash_env_init;
				boot_saveenv = flash_saveenv;
				boot_env_relocate_spec =
				    flash_env_relocate_spec;
				flash_addr = env_ptr =
				    (env_t *) (boot_flash_base +
					       boot_flash_off);
				env_name_spec = flash_env_name_spec;
				boot_flash_env_addr = (u32) flash_addr;
				break;
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
			case PISMO1_NAND:
				boot_env_get_char_spec = nand_env_get_char_spec;
				boot_env_init = nand_env_init;
				boot_saveenv = nand_saveenv;
				boot_env_relocate_spec = nand_env_relocate_spec;
				env_ptr = 0;	/* This gets filled elsewhere!! */
				env_name_spec = nand_env_name_spec;
				break;
#endif
#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)
			case PISMO1_ONENAND:
				boot_env_get_char_spec =
				onenand_env_get_char_spec;
				boot_env_init = onenand_env_init;
				boot_saveenv = onenand_saveenv;
				boot_env_relocate_spec =
				onenand_env_relocate_spec;
				env_ptr =
				(env_t *) onenand_env;
				env_name_spec = onenand_env_name_spec;
				break;
#endif
			default:
				/* unknown variant!! */
				puts("Unknown Boot chip!!!\n");
				break;
			}
#endif				/* ENV_IS_VARIABLE */
		}
		enable_gpmc_config(gpmc_config, gpmc_base, base, size);
	}
#endif
}
