/*
 * (C) Copyright 2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 * Rajendra Nayak <rnayak@ti.com>
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
#include <asm/arch/clocks443x.h>
#include <environment.h>
#include <command.h>

#define CONFIG_OMAP4_SDC 1

/* Used to index into DPLL parameter tables */
struct dpll_param {
	unsigned int m;
	unsigned int n;
	unsigned int m2;
	unsigned int m3;
	unsigned int m4;
	unsigned int m5;
	unsigned int m6;
	unsigned int m7;
};

/* Tables having M,N,M2 et al values for different sys_clk speeds
 * This table is generated only for OPP100
 * The tables are organized as follows:
 * Rows : 1 - 12M, 2 - 13M, 3 - 16.8M, 4 - 19.2M, 5 - 26M, 6 - 27M, 7 - 38.4M
 */

/* MPU parameters */
struct dpll_param mpu_dpll_param[7] = {
	/* 12M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 13M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 16.8M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 19.2M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 26M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 27M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 38.4M values */
#ifdef CONFIG_OMAP4_SDC
	{0x1a, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00},
#else
	{0x7d, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00},
#endif
};

/* IVA parameters */
struct dpll_param iva_dpll_param[7] = {
	/* 12M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 13M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 16.8M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 19.2M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 26M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 27M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 38.4M values */
#ifdef CONFIG_OMAP4_SDC
	{0x61, 0x03, 0x00, 0x00, 0x04, 0x07, 0x00, 0x00},
#else
	{0x61, 0x03, 0x00, 0x00, 0x04, 0x07, 0x00, 0x00},
#endif
};

/* CORE parameters */
struct dpll_param core_dpll_param[7] = {
	/* 12M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 13M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 16.8M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 19.2M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 26M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 27M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 38.4M values */
#ifdef CONFIG_OMAP4_SDC
	{0x1b0, 0x18, 0x01, 0x05, 0x08, 0x04, 0x06, 0x08},
#else
	{0x7d, 0x05, 0x01, 0x05, 0x08, 0x04, 0x06, 0x08},
#endif
};

/* PER parameters */
struct dpll_param per_dpll_param[7] = {
	/* 12M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 13M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 16.8M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 19.2M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 26M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 27M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 38.4M values */
#ifdef CONFIG_OMAP4_SDC
	{0x0a, 0x00, 0x04, 0x03, 0x06, 0x05, 0x02, 0x03},
#else
	{0x14, 0x00, 0x08, 0x06, 0x0c, 0x09, 0x04, 0x05},
#endif
};

/* ABE parameters */
struct dpll_param abe_dpll_param[7] = {
	/* 12M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 13M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 16.8M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 19.2M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 26M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 27M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 38.4M values */
#ifdef CONFIG_OMAP4_SDC
	{0x40, 0x18, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0},
#else
	{0x40, 0x18, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0},
#endif
};

/* USB parameters */
struct dpll_param usb_dpll_param[7] = {
	/* 12M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 13M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 16.8M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 19.2M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 26M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 27M values */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* 38.4M values */
#ifdef CONFIG_OMAP4_SDC
	{0x32, 0x1, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0},
#else
	{0x32, 0x1, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0},
#endif
};

typedef struct dpll_param dpll_param;

static void configure_mpu_dpll(u32 clk_index)
{
	dpll_param *dpll_param_p;

	/* Unlock the MPU dpll */
	sr32(CM_CLKMODE_DPLL_MPU, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_MPU, LDELAY);

	/* Program MPU DPLL */
	dpll_param_p = &mpu_dpll_param[clk_index];

	sr32(CM_AUTOIDLE_DPLL_MPU, 0, 3, 0x0); /* Disable DPLL autoidle */

	/* Set M,N,M2 values */
	sr32(CM_CLKSEL_DPLL_MPU, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_MPU, 0, 6, dpll_param_p->n);
	sr32(CM_DIV_M2_DPLL_MPU, 0, 5, dpll_param_p->m2);

	/* Lock the mpu dpll */
	sr32(CM_CLKMODE_DPLL_MPU, 0, 3, PLL_LOCK | 0x10);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_MPU, LDELAY);

	return;
}

static void configure_iva_dpll(u32 clk_index)
{
	dpll_param *dpll_param_p;

	/* Unlock the IVA dpll */
	sr32(CM_CLKMODE_DPLL_IVA, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_IVA, LDELAY);

	/* CM_BYPCLK_DPLL_IVA = CORE_X2_CLK/2 */
	sr32(CM_BYPCLK_DPLL_IVA, 0, 2, 0x1);

	/* Program IVA DPLL */
	dpll_param_p = &iva_dpll_param[clk_index];

	sr32(CM_AUTOIDLE_DPLL_IVA, 0, 3, 0x0); /* Disable DPLL autoidle */

	/* Set M,N,M4,M5 */
	sr32(CM_CLKSEL_DPLL_IVA, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_IVA, 0, 7, dpll_param_p->n);
	sr32(CM_DIV_M4_DPLL_IVA, 0, 5, dpll_param_p->m4);
	sr32(CM_DIV_M5_DPLL_IVA, 0, 5, dpll_param_p->m5);

	/* Lock the iva dpll */
	sr32(CM_CLKMODE_DPLL_IVA, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_IVA, LDELAY);

	return;
}

static void configure_per_dpll(u32 clk_index)
{
	dpll_param *dpll_param_p;

	/* Unlock the PER dpll */
	sr32(CM_CLKMODE_DPLL_PER, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_PER, LDELAY);

	/* Program PER DPLL */
	dpll_param_p = &per_dpll_param[clk_index];

	/* Disable autoidle */
	sr32(CM_AUTOIDLE_DPLL_PER, 0, 3, 0x0);

	sr32(CM_CLKSEL_DPLL_PER, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_PER, 0, 6, dpll_param_p->n);
	sr32(CM_DIV_M2_DPLL_PER, 0, 5, dpll_param_p->m2);
	sr32(CM_DIV_M3_DPLL_PER, 0, 5, dpll_param_p->m3);
	sr32(CM_DIV_M4_DPLL_PER, 0, 5, dpll_param_p->m4);
	sr32(CM_DIV_M5_DPLL_PER, 0, 5, dpll_param_p->m5);
	sr32(CM_DIV_M6_DPLL_PER, 0, 5, dpll_param_p->m6);
	sr32(CM_DIV_M7_DPLL_PER, 0, 5, dpll_param_p->m7);

	/* Lock the per dpll */
	sr32(CM_CLKMODE_DPLL_PER, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_PER, LDELAY);

	return;
}

static void configure_abe_dpll(u32 clk_index)
{
	dpll_param *dpll_param_p;

	/* Select sys_clk as ref clk for ABE dpll */
	sr32(CM_ABE_PLL_REF_CLKSEL, 0, 32, 0x0);

	/* Enable slimbus and pad clocks */
	sr32(CM_CLKSEL_ABE, 0, 32, 0x500);

	/* Unlock the ABE dpll */
	sr32(CM_CLKMODE_DPLL_ABE, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_ABE, LDELAY);

	/* Program ABE DPLL */
	dpll_param_p = &abe_dpll_param[clk_index];

	/* Disable autoidle */
	sr32(CM_AUTOIDLE_DPLL_ABE, 0, 3, 0x0);

	sr32(CM_CLKSEL_DPLL_ABE, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_ABE, 0, 6, dpll_param_p->n);

	/* Force DPLL CLKOUTHIF to stay enabled */
	sr32(CM_DIV_M2_DPLL_ABE, 0, 32, 0x500);
	sr32(CM_DIV_M2_DPLL_ABE, 0, 5, dpll_param_p->m2);
	/* Force DPLL CLKOUTHIF to stay enabled */
	sr32(CM_DIV_M3_DPLL_ABE, 0, 32, 0x100);
	sr32(CM_DIV_M3_DPLL_ABE, 0, 5, dpll_param_p->m3);

	/* Lock the abe dpll */
	sr32(CM_CLKMODE_DPLL_ABE, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_ABE, LDELAY);

	return;
}

static void configure_usb_dpll(u32 clk_index)
{
	dpll_param *dpll_param_p;

	/* Select the 60Mhz clock 480/8 = 60*/
	sr32(CM_CLKSEL_USB_60MHz, 0, 32, 0x1);

	/* Unlock the USB dpll */
	sr32(CM_CLKMODE_DPLL_USB, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_USB, LDELAY);

	/* Program USB DPLL */
	dpll_param_p = &usb_dpll_param[clk_index];

	/* Disable autoidle */
	sr32(CM_AUTOIDLE_DPLL_USB, 0, 3, 0x0);

	sr32(CM_CLKSEL_DPLL_USB, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_USB, 0, 6, dpll_param_p->n);

	/* Force DPLL CLKOUT to stay active */
	sr32(CM_DIV_M2_DPLL_USB, 0, 32, 0x100);
	sr32(CM_DIV_M2_DPLL_USB, 0, 5, dpll_param_p->m2);

	/* Lock the usb dpll */
	sr32(CM_CLKMODE_DPLL_USB, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_USB, LDELAY);

	/* force enable the CLKDCOLDO clock */
	sr32(CM_CLKDCOLDO_DPLL_USB, 0, 32, 0x100);

	return;
}

static void configure_core_dpll(u32 clk_index)
{
	dpll_param *dpll_param_p;

	/* Get the sysclk speed from cm_sys_clksel
	 * Set it to 38.4 MHz, in case ROM code is bypassed
	 */
	if (!clk_index)
		*(volatile int*)0x4A306110 = 0x7; //CM_SYS_CLKSEL

	/* CORE_CLK=CORE_X2_CLK/2, L3_CLK=CORE_CLK/2, L4_CLK=L3_CLK/2 */
	sr32(CM_CLKSEL_CORE, 0, 32, 0x110);

	/* Unlock the CORE dpll */
	sr32(CM_CLKMODE_DPLL_CORE, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_CORE, LDELAY);

	/* Program USB DPLL */
	dpll_param_p = &core_dpll_param[clk_index];

	/* Disable autoidle */
	sr32(CM_AUTOIDLE_DPLL_CORE, 0, 3, 0x0);

	sr32(CM_CLKSEL_DPLL_CORE, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_CORE, 0, 6, dpll_param_p->n);
	sr32(CM_DIV_M2_DPLL_CORE, 0, 5, dpll_param_p->m2);
	sr32(CM_DIV_M3_DPLL_CORE, 0, 5, dpll_param_p->m3);
	sr32(CM_DIV_M4_DPLL_CORE, 0, 5, dpll_param_p->m4);
	sr32(CM_DIV_M5_DPLL_CORE, 0, 5, dpll_param_p->m5);
	sr32(CM_DIV_M6_DPLL_CORE, 0, 5, dpll_param_p->m6);
	sr32(CM_DIV_M7_DPLL_CORE, 0, 5, dpll_param_p->m7);

	/* Lock the core dpll */
	sr32(CM_CLKMODE_DPLL_CORE, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_CORE, LDELAY);

	return;
}


void configure_core_dpll_no_lock(void)
{
	dpll_param *dpll_param_p;
	u32 clk_index;

	/* Get the sysclk speed from cm_sys_clksel
	 * Set it to 38.4 MHz, in case ROM code is bypassed
	 */
	clk_index = readl(CM_SYS_CLKSEL);
	if (!clk_index)
		*(volatile int*)0x4A306110 = 0x7; //CM_SYS_CLKSEL

	clk_index = clk_index - 1;
	/* CORE_CLK=CORE_X2_CLK/2, L3_CLK=CORE_CLK/2, L4_CLK=L3_CLK/2 */
	sr32(CM_CLKSEL_CORE, 0, 32, 0x110);

	/* Unlock the CORE dpll */
	sr32(CM_CLKMODE_DPLL_CORE, 0, 3, PLL_MN_POWER_BYPASS);
	wait_on_value(BIT0, 0, CM_IDLEST_DPLL_CORE, LDELAY);

	/* Program USB DPLL */
	dpll_param_p = &core_dpll_param[clk_index];

	/* Disable autoidle */
	sr32(CM_AUTOIDLE_DPLL_CORE, 0, 3, 0x0);

	sr32(CM_CLKSEL_DPLL_CORE, 8, 11, dpll_param_p->m);
	sr32(CM_CLKSEL_DPLL_CORE, 0, 6, dpll_param_p->n);
	sr32(CM_DIV_M2_DPLL_CORE, 0, 5, dpll_param_p->m2);
	sr32(CM_DIV_M3_DPLL_CORE, 0, 5, dpll_param_p->m3);
	sr32(CM_DIV_M4_DPLL_CORE, 0, 5, dpll_param_p->m4);
	sr32(CM_DIV_M5_DPLL_CORE, 0, 5, dpll_param_p->m5);
	sr32(CM_DIV_M6_DPLL_CORE, 0, 5, dpll_param_p->m6);
	sr32(CM_DIV_M7_DPLL_CORE, 0, 5, dpll_param_p->m7);

	return;
}

void lock_core_dpll(void)
{
	/* Lock the core dpll */
	sr32(CM_CLKMODE_DPLL_CORE, 0, 3, PLL_LOCK);
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_CORE, LDELAY);

	return;
}

void lock_core_dpll_shadow(void)
{
	/* Lock the core dpll using freq update method */
	*(volatile int*)0x4A004120 = 10;	//(CM_CLKMODE_DPLL_CORE)

	/* CM_SHADOW_FREQ_CONFIG1DLL_OVERRIDE = 0, DLL_RESET = 1,
	 * DPLL_CORE_M2_DIV =1, DPLL_CORE_DPLL_EN = 0x7, FREQ_UPDATE = 1
	 */
	*(volatile int*)0x4A004260 = 0x1709;

	/* Wait for Freq_Update to get cleared: CM_SHADOW_FREQ_CONFIG1 */
	while( ( (*(volatile int*)0x4A004260) & 0x1) == 0x1 );

	/* Wait for DPLL to Lock : CM_IDLEST_DPLL_CORE */
	wait_on_value(BIT0, 1, CM_IDLEST_DPLL_CORE, LDELAY);

	return;
}

static void enable_all_clocks(void)
{
	/* Enable Ducati clocks */
	sr32(CM_DUCATI_DUCATI_CLKCTRL, 0, 32, 0x1);
	sr32(CM_DUCATI_CLKSTCTRL, 0, 32, 0x2);

	wait_on_value(BIT8, BIT8, CM_DUCATI_CLKSTCTRL, LDELAY);

	/* Enable ivahd and sl2 clocks */
	sr32(IVAHD_IVAHD_CLKCTRL, 0, 32, 0x1);
	sr32(IVAHD_SL2_CLKCTRL, 0, 32, 0x1);
	sr32(IVAHD_CLKSTCTRL, 0, 32, 0x2);

	wait_on_value(BIT8, BIT8, IVAHD_CLKSTCTRL, LDELAY);

	/* Enable Tesla clocks */
	sr32(DSP_DSP_CLKCTRL, 0, 32, 0x1);
	sr32(DSP_CLKSTCTRL, 0, 32, 0x2);

	wait_on_value(BIT8, BIT8, DSP_CLKSTCTRL, LDELAY);

	/* TODO: Some hack needed by MM: Clean this */
	#if 0 /* Doesn't work on Zebu */
	*(volatile int*)0x4a306910 = 0x00000003;
	*(volatile int*)0x550809a0 = 0x00000001;
	*(volatile int*)0x55080a20 = 0x00000007;
	#endif

	/* ABE clocks */
	sr32(CM1_ABE_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM1_ABE_AESS_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_PDM_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_DMIC_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_MCASP_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_MCBSP1_CLKCTRL, 0, 32, 0x08000002);
	sr32(CM1_ABE_MCBSP2_CLKCTRL, 0, 32, 0x08000002);
	sr32(CM1_ABE_MCBSP3_CLKCTRL, 0, 32, 0x08000002);

	sr32(CM1_ABE_SLIMBUS_CLKCTRL, 0, 32, 0xf02);
	sr32(CM1_ABE_TIMER5_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_TIMER6_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_TIMER7_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_TIMER8_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_WDT3_CLKCTRL, 0, 32, 0x2);
	/* Disable sleep transitions */
	sr32(CM1_ABE_CLKSTCTRL, 0, 32, 0x0);

	/* L4PER clocks */
	sr32(CM_L4PER_CLKSTCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER10_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER11_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER2_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER3_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER4_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER9_CLKCTRL, 0, 32, 0x2);

	/* GPIO clocks */
	sr32(CM_L4PER_GPIO2_CLKCTRL, 0 ,32, 0x1);
	sr32(CM_L4PER_GPIO3_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L4PER_GPIO4_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L4PER_GPIO5_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L4PER_GPIO6_CLKCTRL, 0, 32, 0x1);

	sr32(CM_L4PER_HDQ1W_CLKCTRL, 0, 32, 0x2);

	/* I2C clocks */
	sr32(CM_L4PER_I2C1_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_I2C2_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_I2C3_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_I2C4_CLKCTRL, 0, 32, 0x2);

	sr32(CM_L4PER_MCBSP4_CLKCTRL, 0, 32, 0x2);

	/* MCSPI clocks */
	sr32(CM_L4PER_MCSPI1_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_MCSPI2_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_MCSPI3_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_MCSPI4_CLKCTRL, 0, 32, 0x2);

	/* MMC clocks */
	sr32(CM_L3INIT_HSMMC1_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L3INIT_HSMMC2_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_MMCSD3_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_MMCSD4_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_MMCSD5_CLKCTRL, 0, 32, 0x2);

	/* UART clocks */
	sr32(CM_L4PER_UART1_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_UART2_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_UART3_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_UART4_CLKCTRL, 0, 32, 0x2);

	/* WKUP clocks */
	sr32(CM_WKUP_GPIO1_CLKCTRL, 0, 32, 0x1);
	sr32(CM_WKUP_TIMER1_CLKCTRL, 0, 32, 0x01000002);

	sr32(CM_WKUP_KEYBOARD_CLKCTRL, 0, 32, 0x2);

	sr32(CM_SDMA_CLKSTCTRL, 0, 32, 0x0);
	sr32(CM_MEMIF_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_MEMIF_EMIF_1_CLKCTRL, 0, 32, 0x1);
	sr32(CM_MEMIF_EMIF_2_CLKCTRL, 0, 32, 0x1);
	sr32(CM_D2D_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_L3_2_GPMC_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L3INSTR_L3_3_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L3INSTR_L3_INSTR_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L3INSTR_OCP_WP1_CLKCTRL, 0, 32, 0x1);

	/* WDT clocks */
	sr32(CM_WKUP_WDT2_CLKCTRL, 0, 32, 0x2);

	/* Enable Camera clocks */
	sr32(CM_CAM_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_CAM_ISS_CLKCTRL, 0, 32, 0x102);
	sr32(CM_CAM_FDIF_CLKCTRL, 0, 32, 0x2);
	sr32(CM_CAM_CLKSTCTRL, 0, 32, 0x0);

	/* Enable DSS clocks */
	sr32(CM_DSS_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_DSS_DSS_CLKCTRL, 0, 32, 0x2);
	sr32(CM_DSS_DEISS_CLKCTRL, 0, 32, 0x2);
	sr32(CM_DSS_CLKSTCTRL, 0, 32, 0x0);

	/* Enable SGX clocks */
	sr32(CM_SGX_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_SGX_SGX_CLKCTRL, 0, 32, 0x2);
	sr32(CM_SGX_CLKSTCTRL, 0, 32, 0x0);

	/* Enable hsi/unipro/usb clocks */
	sr32(CM_L3INIT_HSI_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L3INIT_UNIPRO1_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L3INIT_HSUSBHOST_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L3INIT_HSUSBOTG_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L3INIT_HSUSBTLL_CLKCTRL, 0, 32, 0x1);
	sr32(CM_L3INIT_FSUSB_CLKCTRL, 0, 32, 0x2);
	sr32(CM_L3INIT_USBPHY_CLKCTRL, 0, 32, 0x1);
	return;
}

void read_all_clocks(void)
{
	/* Enable Ducati clocks */
	printf("CM_DUCATI_DUCATI_CLKCTRL=0x%x \n",__raw_readl(CM_DUCATI_DUCATI_CLKCTRL));
	printf("CM_DUCATI_CLKSTCTRL=0x%x \n",__raw_readl(CM_DUCATI_CLKSTCTRL));

	/* Enable ivahd and sl2 clocks */
    printf("IVAHD_IVAHD_CLKCTRL=0x%x \n",__raw_readl(IVAHD_IVAHD_CLKCTRL));
	printf("IVAHD_SL2_CLKCTRL=0x%x \n",__raw_readl(IVAHD_SL2_CLKCTRL));
	printf("IVAHD_CLKSTCTRL=0x%x \n",__raw_readl(IVAHD_CLKSTCTRL));

	/* Enable Tesla clocks */
	printf("DSP_DSP_CLKCTRL=0x%x \n",__raw_readl(DSP_DSP_CLKCTRL));
	printf("DSP_CLKSTCTRL=0x%x \n",__raw_readl(DSP_CLKSTCTRL));

	/* TODO: Some hack needed by MM: Clean this */
	#if 0 /* Doesn't work on Zebu */
	*(volatile int*)0x4a306910 = 0x00000003;
	*(volatile int*)0x550809a0 = 0x00000001;
	*(volatile int*)0x55080a20 = 0x00000007;
	#endif

	/* ABE clocks */
	printf("CM1_ABE_CLKSTCTRL=0x%x \n",__raw_readl(CM1_ABE_CLKSTCTRL));
	printf("CM1_ABE_AESS_CLKCTRL=0x%x \n",__raw_readl(CM1_ABE_AESS_CLKCTRL));
	sr32(CM1_ABE_PDM_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_DMIC_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_MCASP_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_MCBSP1_CLKCTRL, 0, 32, 0x08000002);
	sr32(CM1_ABE_MCBSP2_CLKCTRL, 0, 32, 0x08000002);
	sr32(CM1_ABE_MCBSP3_CLKCTRL, 0, 32, 0x08000002);

	sr32(CM1_ABE_SLIMBUS_CLKCTRL, 0, 32, 0xf02);
	sr32(CM1_ABE_TIMER5_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_TIMER6_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_TIMER7_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_TIMER8_CLKCTRL, 0, 32, 0x2);
	sr32(CM1_ABE_WDT3_CLKCTRL, 0, 32, 0x2);
	

	/* L4PER clocks */
    printf("CM_L4PER_CLKSTCTRL=0x%x \n",__raw_readl(CM_L4PER_CLKSTCTRL));
	printf("CM_L4PER_DMTIMER10_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_DMTIMER10_CLKCTRL));
	printf("CM_L4PER_DMTIMER11_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_DMTIMER11_CLKCTRL));
	printf("CM_L4PER_DMTIMER2_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_DMTIMER2_CLKCTRL));
	printf("CM_L4PER_DMTIMER3_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_DMTIMER3_CLKCTRL));
    printf("CM_L4PER_DMTIMER4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_DMTIMER4_CLKCTRL));
	printf("CM_L4PER_DMTIMER9_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_DMTIMER9_CLKCTRL));

	/* GPIO clocks */
	printf("CM_L4PER_GPIO2_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_GPIO2_CLKCTRL));
	printf("CM_L4PER_GPIO3_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_GPIO3_CLKCTRL));
	printf("CM_L4PER_GPIO4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_GPIO4_CLKCTRL));
	printf("CM_L4PER_GPIO5_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_GPIO5_CLKCTRL));
	printf("CM_L4PER_GPIO6_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_GPIO6_CLKCTRL));
    printf("CM_L4PER_HDQ1W_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_HDQ1W_CLKCTRL));

	/* I2C clocks */
	printf("CM_L4PER_I2C1_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_I2C1_CLKCTRL));
	printf("CM_L4PER_I2C2_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_I2C2_CLKCTRL));
	printf("CM_L4PER_I2C3_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_I2C3_CLKCTRL));
	printf("CM_L4PER_I2C4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_I2C4_CLKCTRL));

	printf("CM_L4PER_MCBSP4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MCBSP4_CLKCTRL));

	/* MCSPI clocks */
	printf("CM_L4PER_MCSPI1_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MCSPI1_CLKCTRL));
	printf("CM_L4PER_MCSPI2_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MCSPI2_CLKCTRL));
	printf("CM_L4PER_MCSPI3_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MCSPI3_CLKCTRL));
	printf("CM_L4PER_MCSPI4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MCSPI4_CLKCTRL));

	/* MMC clocks */
	printf("CM_L3INIT_HSMMC1_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_HSMMC1_CLKCTRL));
	printf("CM_L3INIT_HSMMC2_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_HSMMC2_CLKCTRL));
	printf("CM_L4PER_MMCSD3_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MMCSD3_CLKCTRL));
	printf("CM_L4PER_MMCSD4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MMCSD4_CLKCTRL));
	printf("CM_L4PER_MMCSD5_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_MMCSD5_CLKCTRL));

	/* UART clocks */
	printf("CM_L4PER_UART1_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_UART1_CLKCTRL));
	printf("CM_L4PER_UART2_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_UART2_CLKCTRL));
	printf("CM_L4PER_UART3_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_UART3_CLKCTRL));
	printf("CM_L4PER_UART4_CLKCTRL=0x%x \n",__raw_readl(CM_L4PER_UART4_CLKCTRL));

	/* WKUP clocks */
	printf("CM_WKUP_GPIO1_CLKCTRL=0x%x \n",__raw_readl(CM_WKUP_GPIO1_CLKCTRL));
	printf("CM_WKUP_TIMER1_CLKCTRL=0x%x \n",__raw_readl(CM_WKUP_TIMER1_CLKCTRL));

	printf("CM_WKUP_KEYBOARD_CLKCTRL=0x%x \n",__raw_readl(CM_WKUP_KEYBOARD_CLKCTRL));

	printf("CM_SDMA_CLKSTCTRL=0x%x \n",__raw_readl(CM_SDMA_CLKSTCTRL));
	printf("CM_MEMIF_CLKSTCTRL=0x%x \n",__raw_readl(CM_MEMIF_CLKSTCTRL));
	printf("CM_MEMIF_EMIF_1_CLKCTRL=0x%x \n",__raw_readl(CM_MEMIF_EMIF_1_CLKCTRL));
	printf("CM_MEMIF_EMIF_2_CLKCTRL=0x%x \n",__raw_readl(CM_MEMIF_EMIF_2_CLKCTRL));
	printf("CM_D2D_CLKSTCTRL=0x%x \n",__raw_readl(CM_D2D_CLKSTCTRL));
	printf("CM_L3_2_GPMC_CLKCTRL=0x%x \n",__raw_readl(CM_L3_2_GPMC_CLKCTRL));
	printf("CM_L3INSTR_L3_3_CLKCTRL=0x%x \n",__raw_readl(CM_L3INSTR_L3_3_CLKCTRL));
	printf("CM_L3INSTR_L3_INSTR_CLKCTRL=0x%x \n",__raw_readl(CM_L3INSTR_L3_INSTR_CLKCTRL));
	printf("CM_L3INSTR_OCP_WP1_CLKCTRL=0x%x \n",__raw_readl(CM_L3INSTR_OCP_WP1_CLKCTRL));

	/* WDT clocks */
    printf("CM_WKUP_WDT2_CLKCTRL=0x%x \n",__raw_readl(CM_WKUP_WDT2_CLKCTRL));

	/* Enable Camera clocks */
	printf("CM_CAM_CLKSTCTRL=0x%x \n",__raw_readl(CM_CAM_CLKSTCTRL));
	printf("CM_CAM_ISS_CLKCTRL=0x%x \n",__raw_readl(CM_CAM_ISS_CLKCTRL));
	printf("CM_CAM_FDIF_CLKCTRL=0x%x \n",__raw_readl(CM_CAM_FDIF_CLKCTRL));
	printf("CM_CAM_CLKSTCTRL=0x%x \n",__raw_readl(CM_CAM_CLKSTCTRL));

	/* Enable DSS clocks */
	printf("CM_DSS_CLKSTCTRL=0x%x \n",__raw_readl(CM_DSS_CLKSTCTRL));
	printf("CM_DSS_DSS_CLKCTRL=0x%x \n",__raw_readl(CM_DSS_DSS_CLKCTRL));
	printf("CM_DSS_DEISS_CLKCTRL=0x%x \n",__raw_readl(CM_DSS_DEISS_CLKCTRL));
	printf("CM_DSS_CLKSTCTRL=0x%x \n",__raw_readl(CM_DSS_CLKSTCTRL));

	/* Enable SGX clocks */
	printf("CM_SGX_CLKSTCTRL=0x%x \n",__raw_readl(CM_SGX_CLKSTCTRL));
	printf("CM_SGX_SGX_CLKCTRL=0x%x \n",__raw_readl(CM_SGX_SGX_CLKCTRL));
	printf("CM_SGX_CLKSTCTRL=0x%x \n",__raw_readl(CM_SGX_CLKSTCTRL));

	/* Enable hsi/unipro/usb clocks */
	printf("CM_L3INIT_HSI_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_HSI_CLKCTRL));
	printf("CM_L3INIT_UNIPRO1_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_UNIPRO1_CLKCTRL));
	printf("CM_L3INIT_HSUSBHOST_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_HSUSBHOST_CLKCTRL));
	printf("CM_L3INIT_HSUSBOTG_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_HSUSBOTG_CLKCTRL));
	printf("CM_L3INIT_HSUSBTLL_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_HSUSBTLL_CLKCTRL));
	printf("CM_L3INIT_FSUSB_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_FSUSB_CLKCTRL));
	printf("CM_L3INIT_USBPHY_CLKCTRL=0x%x \n",__raw_readl(CM_L3INIT_USBPHY_CLKCTRL));
	return;
}
/******************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 *   -- called from SRAM, or Flash (using temp SRAM stack).
 *****************************************************************************/
void prcm_init(void)
{
	u32 clk_index;

	/* Get the sysclk speed from cm_sys_clksel */
	clk_index = readl(CM_SYS_CLKSEL);
	if (!clk_index)
		return; /* Sys clk uninitialized */

	/* Configure all DPLL's at 100% OPP */
	configure_core_dpll(clk_index - 1);
//	configure_mpu_dpll(clk_index - 1);
	configure_iva_dpll(clk_index - 1);
	configure_per_dpll(clk_index - 1);
	configure_abe_dpll(clk_index - 1);
	configure_usb_dpll(clk_index - 1);

#ifdef CONFIG_OMAP4_SDC
	/* Enable all clocks */
	enable_all_clocks();
    read_all_clocks();
#endif

	return;
}
