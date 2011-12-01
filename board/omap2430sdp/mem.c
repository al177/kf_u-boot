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
#include <asm/arch/mem.h>
#include <asm/arch/clocks.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <environment.h>
#include <command.h>

/****** DATA STRUCTURES ************/

/* Only One NAND allowed on board at a time. 
 * The GPMC CS Base for the same
 */
unsigned int nand_cs_base = 0;
unsigned int boot_flash_base = 0;
unsigned int boot_flash_off = 0;
unsigned int boot_flash_sec = 0;
unsigned int boot_flash_type = 0;
volatile unsigned int boot_flash_env_addr = 0;
/* help common/env_flash.c */
#ifdef ENV_IS_VARIABLE

/* On SDP, all the 3 NOR parts are available on all CS combinations.
 * On GDP, this is a variable set. Set the default to SDP configuration
 * and change it later on if the board is GDP.
 */
ulong NOR_FLASH_BANKS_LIST[CFG_MAX_FLASH_BANKS] = CFG_FLASH_BANKS_LIST;
int NOR_MAX_FLASH_BANKS = 4 ;           /* max number of flash banks */

uchar(*boot_env_get_char_spec) (int index);
int (*boot_env_init) (void);
int (*boot_saveenv) (void);
void (*boot_env_relocate_spec) (void);

extern uchar flash_env_get_char_spec(int index);
extern int flash_env_init(void);
extern int flash_saveenv(void);
extern void flash_env_relocate_spec(void);
extern char *flash_env_name_spec;

extern uchar nand_env_get_char_spec(int index);
extern int nand_env_init(void);
extern int nand_saveenv(void);
extern void nand_env_relocate_spec(void);
extern char *nand_env_name_spec;

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

/* Board CS Organization - 2430SDP REV 0.1->1.1 */
static const unsigned char chip_sel_sdp[][GPMC_MAX_CS] = {
/* GPMC CS Indices */
/* S8- 1   2   3 IDX    CS0,       CS1,      CS2 ..                          CS7  */
/* 0 OFF OFF OFF */ {PISMO_CS2, PISMO_CS1, PROC_NOR, 0, 0, DBG_MPDB, 0, PISMO_CS0},
/* 1 ON  OFF OFF */ {PISMO_CS1, PISMO_CS0, PROC_NOR, 0, 0, DBG_MPDB, 0, PROC_NAND},
/* 2 OFF ON  OFF */ {PISMO_CS0, PROC_NOR, PISMO_CS1, 0, 0, DBG_MPDB, 0, PISMO_CS2},
/* 3 ON  ON  OFF */ {PROC_NAND, PROC_NOR, PISMO_CS0, 0, 0, DBG_MPDB, 0, PISMO_CS1},
/* 4 OFF OFF ON  */ {PISMO_CS1, PISMO_CS2, PROC_NOR, 0, 0, DBG_MPDB, 0, PISMO_CS0},
/* 5 ON  OFF ON  */ {PISMO_CS0, PISMO_CS1, PROC_NOR, 0, 0, DBG_MPDB, 0, PROC_NAND},
/* 6 OFF ON  ON  */ {PROC_NOR, PISMO_CS0, PISMO_CS1, 0, 0, DBG_MPDB, 0, PISMO_CS2},
/* 7 ON  ON  ON  */ {PROC_NOR, PROC_NAND, PISMO_CS0, 0, 0, DBG_MPDB, 0, PISMO_CS1},
};

/* Board CS Organization - 2430GDP REV 0.1->1.1 */
static const unsigned char chip_sel_gdp[][GPMC_MAX_CS] = {
/* GPMC CS Indices */
/* S8- 1   2   3 IDX    CS0,       CS1,      CS2 ..                          CS7  */
/* 0 OFF OFF OFF */ {PISMO_CS0, PISMO_CS1, PROC_NAND, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 1 ON  OFF OFF */ {PISMO_CS2, PROC_NAND, PROC_NOR, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 2 OFF ON  OFF */ {PROC_NAND, PISMO_CS0, PISMO_CS1, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 3 ON  ON  OFF */ {PROC_NAND, PROC_NOR, PISMO_CS2, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 4 OFF OFF ON  */ {PISMO_CS0, PISMO_CS1, PROC_NOR, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 5 ON  OFF ON  */ {PISMO_CS2, PROC_NOR, PROC_NAND, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 6 OFF ON  ON  */ {PROC_NOR, PISMO_CS0, PISMO_CS1, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
/* 7 ON  ON  ON  */ {PROC_NOR, PROC_NAND, PISMO_CS2, 0, 0, DBG_MPDB, 0, PISMO_PCMCIA},
};

/* Map the number of NORs present and NOR flash locations on GDP */
static const ulong norfl_loc_gdp[][CFG_MAX_FLASH_BANKS + 1] = {
/* 0 OFF OFF OFF*/ {2,SIBLEY_MAP1, SIBLEY_MAP2, 0, 0 },
/* 1 ON  OFF OFF*/ {2,FLASH_BASE, FLASH_BASE + PHYS_FLASH_SIZE, 0, 0},
/* 2 OFF ON  OFF*/ {2,SIBLEY_MAP1, SIBLEY_MAP2, 0, 0 }, 
/* 3 ON  ON  OFF*/ {2,FLASH_BASE, FLASH_BASE + PHYS_FLASH_SIZE, 0, 0}, 
/* 4 OFF OFF ON */ {4,FLASH_BASE, FLASH_BASE + PHYS_FLASH_SIZE, SIBLEY_MAP1, SIBLEY_MAP2}, 
/* 5 ON  OFF ON */ {2,FLASH_BASE, FLASH_BASE + PHYS_FLASH_SIZE, 0, 0}, 
/* 6 OFF ON  ON */ {4, FLASH_BASE, FLASH_BASE + PHYS_FLASH_SIZE, SIBLEY_MAP1, SIBLEY_MAP2},
/* 7 ON  ON  ON */ {2,FLASH_BASE, FLASH_BASE + PHYS_FLASH_SIZE, 0, 0},
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
static u32 gpmc_stnor[GPMC_MAX_REG] = {
	STNOR_GPMC_CONFIG1,
	STNOR_GPMC_CONFIG2,
	STNOR_GPMC_CONFIG3,
	STNOR_GPMC_CONFIG4,
	STNOR_GPMC_CONFIG5,
	STNOR_GPMC_CONFIG6, 0
};
static u32 gpmc_smnand[GPMC_MAX_REG] = {
	SMNAND_GPMC_CONFIG1,
	SMNAND_GPMC_CONFIG2,
	SMNAND_GPMC_CONFIG3,
	SMNAND_GPMC_CONFIG4,
	SMNAND_GPMC_CONFIG5,
	SMNAND_GPMC_CONFIG6, 0
};
static u32 gpmc_sibnor[GPMC_MAX_REG] = {
	SIBNOR_GPMC_CONFIG1,
	SIBNOR_GPMC_CONFIG2,
	SIBNOR_GPMC_CONFIG3,
	SIBNOR_GPMC_CONFIG4,
	SIBNOR_GPMC_CONFIG5,
	SIBNOR_GPMC_CONFIG6, 0
};
static u32 gpmc_onenand[GPMC_MAX_REG] = {
	ONENAND_GPMC_CONFIG1,
	ONENAND_GPMC_CONFIG2,
	ONENAND_GPMC_CONFIG3,
	ONENAND_GPMC_CONFIG4,
	ONENAND_GPMC_CONFIG5,
	ONENAND_GPMC_CONFIG6, 0
};
static u32 gpmc_pcmcia[GPMC_MAX_REG] = {
	PCMCIA_GPMC_CONFIG1,
	PCMCIA_GPMC_CONFIG2,
	PCMCIA_GPMC_CONFIG3,
	PCMCIA_GPMC_CONFIG4,
	PCMCIA_GPMC_CONFIG5,
	PCMCIA_GPMC_CONFIG6, 0
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

/*************************************************************
 * get_sys_clk_speed - determine reference oscillator speed
 *  based on known 32kHz clock and gptimer.
 *************************************************************/
u32 get_osc_clk_speed(u32 * shift)
{
#define GPT_EN	((0<<2)|BIT1|BIT0)	/* enable sys_clk NO-prescale /1 */
#define GPT_CTR	OMAP24XX_GPT2+TCRR	/* read counter address */
	u32 start, cstart, cend, cdiff, val;

	if (__raw_readl(PRCM_CLKSRC_CTRL) & BIT7) {	/* if currently /2 */
		*shift = 1;
	} else {
		*shift = 0;
	}

	/* enable timer2 */
	val = __raw_readl(CM_CLKSEL2_CORE) | 0x4;	/* mask for sys_clk use */
	__raw_writel(val, CM_CLKSEL2_CORE);	/* timer2 source to sys_clk */
	__raw_writel(BIT4, CM_ICLKEN1_CORE);	/* timer2 interface clock on */
	__raw_writel(BIT4, CM_FCLKEN1_CORE);	/* timer2 function clock on */
	__raw_writel(0, OMAP24XX_GPT2 + TLDR);	/* start counting at 0 */
	__raw_writel(GPT_EN, OMAP24XX_GPT2 + TCLR);	/* enable clock */
	/* enable 32kHz source *//* enabled out of reset */
	/* determine sys_clk via gauging */

	start = 20 + __raw_readl(S32K_CR);	/* start time in 20 cycles */
	while (__raw_readl(S32K_CR) < start) ;	/* dead loop till start time */
	cstart = __raw_readl(GPT_CTR);	/* get start sys_clk count */
	while (__raw_readl(S32K_CR) < (start + 20)) ;	/* wait for 40 cycles */
	cend = __raw_readl(GPT_CTR);	/* get end sys_clk count */
	cdiff = cend - cstart;	/* get elapsed ticks */

	/* based on number of ticks assign speed */
	if (cdiff > (19000 >> *shift))
		return (S38_4M);
	else if (cdiff > (15200 >> *shift))
		return (S26M);
	else if (cdiff > (13000 >> *shift))
		return (S24M);
	else if (cdiff > (9000 >> *shift))
		return (S19_2M);
	else if (cdiff > (7600 >> *shift))
		return (S13M);
	else
		return (S12M);
}

/************************************************************
 * sdelay() - simple spin loop.  Will be constant time as
 *  its generally used in 12MHz bypass conditions only.  This
 *  is necessary until timers are accessible.
 *
 *  not inline to increase chances its in cache when called
 *************************************************************/
void sdelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

/*********************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h (config II default).
 *   -- called from SRAM, or Flash (using temp SRAM stack).
 *********************************************************************************/
void prcm_init(void)
{
	u32 div, speed, val, div_by_2;
	void (*f_lock_pll) (u32, u32, u32, u32);
	extern void *_end_vect, *_start;
	/* u32 rev = get_cpu_rev(); unused as of now */

	f_lock_pll =
	    (void *)((u32) & _end_vect - (u32) & _start + SRAM_VECT_CODE);

	val = __raw_readl(PRCM_CLKSRC_CTRL) & ~(BIT1 | BIT0);
#if defined(OMAP2430_SQUARE_CLOCK_INPUT)
	__raw_writel(val, PRCM_CLKSRC_CTRL);
#else
	__raw_writel((val | BIT0), PRCM_CLKSRC_CTRL);
#endif
	/* skip clock gauging if warm reset. For some unknown reason,
	   GPT2 stalls after warm reset until DPLL has been setup and
	   GPT2 F/I clocks are enabled.
	 */
	if (__raw_readl(RM_RSTST_MPU) & BIT1) {
		speed = S13M;
		if (((BIT23 | BIT24 | BIT25) & __raw_readl(CM_CLKSEL1_PLL)) ==
		    (0x3 << 23))
			speed = S12M;
		else if (((BIT23 | BIT24 | BIT25) & __raw_readl(CM_CLKSEL1_PLL))
			 == (0x0 << 23))
			speed = S19_2M;
		div_by_2 =
		    (((BIT6 | BIT7) & __raw_readl(PRCM_CLKSRC_CTRL)) == 0x2);
		if (div_by_2)
			speed <<= 1;
	} else
		speed = get_osc_clk_speed(&div_by_2);

	if ((speed > S19_2M) && (!div_by_2)) {	/* if fast && /2 off, enable it */
		val = ~(BIT6 | BIT7) & __raw_readl(PRCM_CLKSRC_CTRL);
		val |= (0x2 << 6);	/* divide by 2 if (24,26,38.4) -> (12/13/19.2)  */
		__raw_writel(val, PRCM_CLKSRC_CTRL);
	}
	__raw_writel(0, CM_FCLKEN1_CORE);	/* stop all clocks to reduce ringing */
	__raw_writel(0, CM_FCLKEN2_CORE);	/* may not be necessary */
	__raw_writel(0, CM_ICLKEN1_CORE);
	__raw_writel(0, CM_ICLKEN2_CORE);

	__raw_writel(DPLL_OUT, CM_CLKSEL2_PLL);	/* set DPLL out */
	__raw_writel(MPU_DIV, CM_CLKSEL_MPU);	/* set MPU divider */
	__raw_writel(DSP_DIV, CM_CLKSEL_DSP);	/* set dsp and iva dividers */
	__raw_writel(GFX_DIV, CM_CLKSEL_GFX);	/* set gfx dividers */
	__raw_writel(MDM_DIV, CM_CLKSEL_MDM);	/* set mdm dividers */

	div = BUS_DIV;
	__raw_writel(div, CM_CLKSEL1_CORE);	/* set L3/L4/USB/Display/SSi dividers */
	sdelay(1000);

	if (running_in_flash()) {
		/* if running from flash, need to jump to small relocated code area in SRAM.
		 * This is the only safe spot to do configurations from.
		 */
		(*f_lock_pll) (PRCM_CLKCFG_CTRL, CM_CLKEN_PLL, DPLL_LOCK,
			       CM_IDLEST_CKGEN);
	}

	/* set up APLLS_CLKIN per crystal */
	if (speed > S19_2M)
		speed >>= 1;	/* if fast shift to /2 range */
	val = (0x2 << 23);	/* default to 13Mhz for 2430c */
	if (speed == S12M)
		val = (0x3 << 23);
	else if (speed == S19_2M)
		val = (0x0 << 23);
	val |= (~(BIT23 | BIT24 | BIT25) & __raw_readl(CM_CLKSEL1_PLL));
	__raw_writel(val, CM_CLKSEL1_PLL);

	__raw_writel(DPLL_LOCK | APLL_LOCK, CM_CLKEN_PLL);	/* enable apll */
	wait_on_value(BIT8, BIT8, CM_IDLEST_CKGEN, LDELAY);	/* wait for apll lock */

	sdelay(1000);
}

/**************************************************************************
 * make_cs1_contiguous() - for es2 and above remap cs1 behind cs0 to allow
 *  command line mem=xyz use all memory with out discontigious support
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
 *             for a part. Helps in gussing which part
 *             we are currently using.
 *******************************************************/
u32 mem_ok(void)
{
	u32 val1, val2, orig1, orig2, addr;
	u32 pattern = 0x12345678;

	addr = OMAP24XX_SDRC_CS0;

	orig1 = __raw_readl(addr + 0x400);	/* try to save original value */
	orig2 = __raw_readl(addr);
	__raw_writel(0x0, addr + 0x400);	/* clear pos A */
	__raw_writel(pattern, addr);		/* pattern to pos B */
	__raw_writel(0x0, addr + 4);		/* remove pattern off the bus */
	val1 = __raw_readl(addr + 0x400);	/* get pos A value */
	val2 = __raw_readl(addr);		/* get val2 */
	if ((val1 != 0) || (val2 != pattern))	/* see if pos A value changed */
		return (0);
	else {
		/* restore original values and return pass */
		__raw_writel(orig1, addr + 0x400);
		__raw_writel(orig2, addr);
		return (1);
	}
}

/********************************************************
 *  sdrc_init() - init the sdrc chip selects CS0 and CS1
 *  - early init routines, called from flash or
 *  SRAM.
 *******************************************************/
void sdrc_init(void)
{
#define EARLY_INIT 1
	do_sdrc_init(SDRC_CS0_OSET, EARLY_INIT);	/* only init up first bank here */
}

/*************************************************************************
 * do_sdrc_init(): initialize the SDRAM for use.
 *  -called from low level code with stack only.
 *  -code sets up SDRAM timing and muxing for 2422 or 2420.
 *  -optimal settings can be placed here, or redone after i2c
 *      inspection of board info
 *
 *  This is a bit ugly, but should handle all memory modules
 *   used with the SDP. The first time though this code from s_init()
 *   we configure the first chip select.  Later on we come back and
 *   will configure the 2nd chip select if it exists.
 *
 **************************************************************************/
void do_sdrc_init(u32 offset, u32 early)
{
	u32 cpu, dev, dllstat, dllctrl = 0, rev, common = 0, cs0 = 0, pmask =
	    0, pass_type, mtype;
	sdrc_data_t *sdata;	/* do not change type */
	u32 dllx = 0, mono = 0;
	void init_dcdl(u32 cpu);

	/* the following structure has a lot data shared with 2420 H4. Only 3 2430SDP
	   parameters. This needs to be cleaned after wakeup */
	static const sdrc_data_t sdrc_2430 = {
		H4_2420_SDRC_SHARING, SDP_2430_SDRC_MDCFG_0_DDR,
		H4_2420_SDRC_MDCFG_0_SDR,
		SDP_2430_SDRC_ACTIM_CTRLA_0, H4_2420_SDRC_ACTIM_CTRLB_0,
		H4_2420_SDRC_RFR_CTRL, H4_2420_SDRC_MR_0_DDR,
		H4_2420_SDRC_MR_0_SDR,
		SDP_2430_SDRC_DLLAB_CTRL
	};

	if (offset == SDRC_CS0_OSET)
		cs0 = common = 1;	/* int regs shared between both chip select */

	cpu = get_cpu_type();
	dev = get_device_type();
	rev = get_cpu_rev();

	/* warning generated, though code generation is correct. this may bite later,
	 * but is ok for now. there is only so much C code you can do on stack only
	 * operation.
	 */
	sdata = (sdrc_data_t *) & sdrc_2430;
	pass_type = IP_DDR;

	__asm__ __volatile__("":::"memory");	/* limit compiler scope */

	/* u-boot is compiled to run in DDR or SRAM at 8xxxxxxx or 4xxxxxxx.
	 * If we are running in flash prior to relocation and we use data
	 * here which is not pc relative we need to get the address correct.
	 * We need to find the current flash mapping to dress up the initial
	 * pointer load.  As long as this is const data we should be ok.
	 */
	if ((early) && running_in_flash()) {
		sdata =
		    (sdrc_data_t *) (((u32) sdata & 0x0003FFFF) |
				     get_gpmc0_base());
		if (running_from_internal_boot()) {
			if (dev != GP_DEVICE)
				/* NOR internal boot for HS device offset is 
				 * 0x4000 from xloader signature.
				 */
				sdata = (sdrc_data_t *) ((u32) sdata + 0x4000);
			else {
				/* GP device, the image may or may not signed.
				 * If signed, there are 8 bytes pending so the
				 * u-boot is at offset 0x8.
				 */
				if (sdata->sdrc_sharing != H4_2420_SDRC_SHARING)
					sdata =
					    (sdrc_data_t *) ((u32) sdata + 0x8);
			}
		}
	}

	if (!early && (((mtype = get_mem_type()) == DDR_COMBO)
		       || (mtype == DDR_STACKED))) {
		if (mtype == DDR_COMBO) {
			pmask = BIT2;	/* combo part has a shared CKE signal, can't use feature */
			pass_type = COMBO_DDR;	/* CS1 config */
			__raw_writel((__raw_readl(SDRC_POWER)) & ~pmask,
				     SDRC_POWER);
		}
		if (rev != CPU_242X_ES1)	/* for es2 and above smooth things out */
			make_cs1_contiguous();
	}

      next_mem_type:
	if (common) {		/* do a SDRC reset between types to clear regs */
		__raw_writel(SOFTRESET, SDRC_SYSCONFIG);	/* reset sdrc */
		wait_on_value(BIT0, BIT0, SDRC_STATUS, 12000000);	/* wait till reset done set */
		__raw_writel(0, SDRC_SYSCONFIG);	/* clear soft reset */
		__raw_writel(sdata->sdrc_sharing, SDRC_SHARING);
#ifdef POWER_SAVE
		__raw_writel(__raw_readl(SMS_SYSCONFIG) | SMART_IDLE,
			     SMS_SYSCONFIG);
		__raw_writel(sdata->sdrc_sharing | SMART_IDLE, SDRC_SHARING);
		__raw_writel((__raw_readl(SDRC_POWER) | BIT6), SDRC_POWER);
#endif
	}

	if ((pass_type == IP_DDR) || (pass_type == STACKED)) {	/* (IP ddr-CS0),(2422-CS0/CS1) */
		__raw_writel(sdata->sdrc_mdcfg_0_ddr, SDRC_MCFG_0 + offset);
		if (mono)
			__raw_writel(H4_2422_SDRC_MDCFG_MONO_DDR, SDRC_MCFG_0 + offset);	/* 2422.es2.05-CS1 */
	} else if (pass_type == COMBO_DDR) {	/* (combo-CS0/CS1) */
		__raw_writel(H4_2420_COMBO_MDCFG_0_DDR, SDRC_MCFG_0 + offset);
	} else if (pass_type == IP_SDR) {	/* ip sdr-CS0 */
		__raw_writel(sdata->sdrc_mdcfg_0_sdr, SDRC_MCFG_0 + offset);
	}

	if (cs0) {
		__raw_writel(sdata->sdrc_actim_ctrla_0, SDRC_ACTIM_CTRLA_0);
		__raw_writel(sdata->sdrc_actim_ctrlb_0, SDRC_ACTIM_CTRLB_0);
	} else {
		__raw_writel(sdata->sdrc_actim_ctrla_0, SDRC_ACTIM_CTRLA_1);
		__raw_writel(sdata->sdrc_actim_ctrlb_0, SDRC_ACTIM_CTRLB_1);
	}
	__raw_writel(sdata->sdrc_rfr_ctrl, SDRC_RFR_CTRL + offset);

	/* init sequence for mDDR/mSDR using manual commands (DDR is a bit different) */
	__raw_writel(CMD_NOP, SDRC_MANUAL_0 + offset);
	sdelay(5000);		/* supposed to be 100us per design spec for mddr/msdr */
	__raw_writel(CMD_PRECHARGE, SDRC_MANUAL_0 + offset);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0 + offset);
	__raw_writel(CMD_AUTOREFRESH, SDRC_MANUAL_0 + offset);

	/*
	 * CSx SDRC Mode Register
	 * Burst length = (4 - DDR) (2-SDR)
	 * Serial mode
	 * CAS latency = x
	 */
	if (pass_type == IP_SDR)
		__raw_writel(sdata->sdrc_mr_0_sdr, SDRC_MR_0 + offset);
	else
		__raw_writel(sdata->sdrc_mr_0_ddr, SDRC_MR_0 + offset);

	dllctrl = BIT0; /* flag to clear bit0 (90deg for < 133MHz) */

#ifdef PRCM_CONFIG_2
	/* flag clear bit1 (set phase to 72 for > 150MHz) */
	dllctrl |= DLLPHASE;	/* phase to 72 for > 150MHz) */
#endif

	/* If DDR enable DLL to get a value, then move to unlock mode for SDRC mod16 errata */
	if (common && (pass_type != IP_SDR)) {
		__raw_writel(sdata->sdrc_dllab_ctrl, SDRC_DLLA_CTRL);
		__raw_writel(sdata->sdrc_dllab_ctrl & ~(LOADDLL | dllctrl),
			     SDRC_DLLA_CTRL);
		__raw_writel(sdata->sdrc_dllab_ctrl, SDRC_DLLB_CTRL);
		__raw_writel(sdata->sdrc_dllab_ctrl & ~(LOADDLL | dllctrl),
			     SDRC_DLLB_CTRL);

		init_dcdl(cpu);	/* fix errata for possible bad init state */

		sdelay(0x2000);	/* give time to lock, at least 1000 L3 */

		/* work around DCDL MOD16 bug */
		dllctrl = __raw_readl(SDRC_DLLA_CTRL + dllx);	/* get cur ctrl value */
		dllctrl &= ~(DLL_DELAY_MASK);	/* prepare for load new value */
		dllctrl |= LOADDLL;	/* prepare for load + unlock mode */
		dllstat =
		    (__raw_readl(SDRC_DLLA_STATUS + dllx) & DLL_DELAY_MASK);
		dllctrl |= dllstat;	/* prepare new dll load delay */
		dllctrl |= DLL_NO_FILTER_MASK;	/* make sure filter is off */
		__raw_writel(dllctrl, SDRC_DLLA_CTRL);	/* go to unlock modeA */
		__raw_writel(dllctrl, SDRC_DLLB_CTRL);	/* go to unlock modeB */
	}
	sdelay(0x1000);

	if (mono)		/* Used if Stacked memory is on CS1 only */
		make_cs1_contiguous();	/* make CS1 appear at CS0 */

	if (mem_ok())
		return;		/* STACKED, other configured type */
	++pass_type;		/* IPDDR->COMBODDR->IPSDR for CS0 */
	goto next_mem_type;
}

/*****************************************************
 * init_dcdl(): Fix errata - unitilized flip-flop.
 *****************************************************/
void init_dcdl(u32 cpu)
{
	volatile u8 *adqs[4];
	u8 vdqs[4], idx, i;
	u32 base = OMAP24XX_CTRL_BASE;
#define CONTROL_SDRC_DQS0 0x50

	adqs[0] = ((volatile u8 *)(base + CONTROL_SDRC_DQS0 + 0x0));
	adqs[1] = ((volatile u8 *)(base + CONTROL_SDRC_DQS0 + 0x1));
	adqs[2] = ((volatile u8 *)(base + CONTROL_SDRC_DQS0 + 0x2));
	adqs[3] = ((volatile u8 *)(base + CONTROL_SDRC_DQS0 + 0x3));
	idx = 4;

	for (i = 0; i < idx; ++i)	/* save origional state */
		vdqs[i] = *adqs[i];

	for (i = 0; i < idx; ++i)
		*adqs[i] = ((~0x10 & vdqs[i]) | 0x8);	/* enable/activate pull down */

	sdelay(0x400);

	for (i = 0; i < idx; ++i)
		*adqs[i] = (vdqs[i] | 0x10);	/* enable/activate pull up */

	sdelay(0x400);

	for (i = 0; i < idx; ++i)	/* restore state */
		*adqs[i] = vdqs[i];
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
	/* For readability */
	u32 mux = 0, mwidth;
	u32 *gpmc_config = NULL;
	u32 gpmc_base = 0;
	u32 base = 0;
	u8 idx = 0;
	u8 cnt = 0;
	u32 size = 0;
	u32 f_off = CFG_MONITOR_LEN;
	u32 f_sec = 0;
	u32 board_type = 0;
	unsigned char *config_sel = NULL;

	mux = BIT9;
	mwidth = get_gpmc0_width();

	/* global settings */
	__raw_writel(0x10, GPMC_SYSCONFIG);	/* smart idle */
	__raw_writel(0x0, GPMC_IRQENABLE);	/* isr's sources masked */
	__raw_writel(0, GPMC_TIMEOUT_CONTROL);	/* timeout disable */

	/* Disable the GPMC0 config set by ROM code
	 * It conflicts with our MPDB (both at 0x08000000)
	 */
	__raw_writel(0 , GPMC_CONFIG7 + GPMC_CONFIG_CS0);
	sdelay(1000);

	/* GPMC5 is always MPDB.. need to know the chip info */
	gpmc_base = GPMC_CONFIG_CS0 + (5 * GPMC_CONFIG_WIDTH);
	gpmc_mpdb[0] |= mux;
	enable_gpmc_config(gpmc_mpdb, gpmc_base, DEBUG_BASE, DBG_MPDB_SIZE);
	idx = get_gpmc0_type();
	/* invalid chip, assume nor boot */
	if (idx > 0x7) {
		idx = 7;
	}

	/* The board type information was supposed to be dynamically
	* pinged from the board revision id. Due to the problem with
	* fpga, this doesn't happen fast enough, hence reverting back
	* to hardcoding the fact.
	*/
#ifdef OMAP2430_SDP_GDP_CONFIG
	board_type = BOARD_GDP_2430_T2;
#else
	board_type = BOARD_SDP_2430_T2;
#endif

	if (board_type == BOARD_GDP_2430_T2) {

		config_sel = (unsigned char *)(chip_sel_gdp[idx]);

		/* GPMC7 is always PCMCIA.. need to know the chip info */
		gpmc_base = GPMC_CONFIG_CS0 + (7 * GPMC_CONFIG_WIDTH);
		gpmc_pcmcia[0] |= mux;
		__raw_writel(0x0, GPMC_CONFIG);	/* set nWP, disable limited addr */
		enable_gpmc_config(gpmc_pcmcia, gpmc_base, PCMCIA_BASE,PISMO_PCMCIA_SIZE );

		/* On GDP, all the 4 flashes are not present by default 
		 * Configure only those flashes that are present.
		 */
		NOR_MAX_FLASH_BANKS = norfl_loc_gdp[idx][0];
		
		for (cnt = 1; cnt <= NOR_MAX_FLASH_BANKS ; cnt++) {
			NOR_FLASH_BANKS_LIST[cnt - 1] = norfl_loc_gdp[idx][cnt];
		}
		
	} else {
		config_sel = (unsigned char *)(chip_sel_sdp[idx]);
	}

	/* For Nand based boot only..OneNand?? */
	if (config_sel[0] == PROC_NAND) {
		__raw_writel(0x001, GPMC_CONFIG);	/* set nWP, disable limited addr */
	}
	/* reuse idx */
	for (idx = 0; idx < GPMC_MAX_CS; idx++) {
		if (!config_sel[idx]) {
			continue;
		}
		gpmc_base = GPMC_CONFIG_CS0 + (idx * GPMC_CONFIG_WIDTH);
		switch (config_sel[idx]) {
		case PROC_NOR:
			gpmc_config = gpmc_stnor;
			gpmc_config[0] |= mux | TYPE_NOR | mwidth;
			base = PROC_NOR_BASE;
			size = PROC_NOR_SIZE;
			f_sec = SZ_128K;
			is_flash = 1;
			break;
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
		case PROC_NAND:
			base = PROC_NAND_BASE;
			size = PROC_NAND_SIZE;
			gpmc_config = gpmc_smnand;
			gpmc_config[0] |= mux | TYPE_NAND | mwidth;
			/* Either OneNand or Normal Nand at a time!! */
			nand_cs_base = gpmc_base;
			f_off = SMNAND_ENV_OFFSET;
			is_nand = 1;
			break;
#endif
		case PISMO_SIBLEY0:
		case PISMO_SIBLEY1:
			if (config_sel[idx] == PISMO_SIBLEY0) {
				base = PISMO_SIB0_BASE;
				size = PISMO_SIB0_SIZE;
			} else {
				base = PISMO_SIB1_BASE;
				size = PISMO_SIB1_SIZE;
			}
			f_sec = SZ_256K;
			gpmc_config = gpmc_sibnor;
			gpmc_config[0] |= mux | TYPE_NOR | mwidth;
			is_flash = 1;
			break;
		case PISMO_ONENAND:
			base = PISMO_ONEN_BASE;
			size = PISMO_ONEN_SIZE;
			gpmc_config = gpmc_onenand;
			f_off = ONENAND_ENV_OFFSET;
			is_onenand = 1;
			break;
		default:
			/* MPDB/Unsupported/Corrupt config- try Next GPMC CS!!!! */
			continue;
		}
		if (0 == idx) {
			boot_flash_base = base;
			boot_flash_off = f_off;
			boot_flash_sec = f_sec;
			boot_flash_type = config_sel[idx];
			boot_flash_env_addr = f_off;
#ifdef ENV_IS_VARIABLE
			switch (config_sel[0]) {
			case PROC_NOR:
			case PISMO_SIBLEY0:
			case PISMO_SIBLEY1:
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
#if 0
			case PROC_NAND:
				boot_env_get_char_spec = nand_env_get_char_spec;
				boot_env_init = nand_env_init;
				boot_saveenv = nand_saveenv;
				boot_env_relocate_spec = nand_env_relocate_spec;
				env_ptr = 0;	/* This gets filled elsewhere!! */
				env_name_spec = nand_env_name_spec;
				break;
#endif
			case PISMO_ONENAND:
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
			default:
				/* unknown variant!! */
				puts("Unknown Boot chip!!!\n");
				break;
			}
#endif				/* ENV_IS_VARIABLE */
		}
		enable_gpmc_config(gpmc_config, gpmc_base, base, size);
	}
}
