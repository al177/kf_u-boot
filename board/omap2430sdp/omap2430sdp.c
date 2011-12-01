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
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/mem.h>
#include <i2c.h>
#include <asm/mach-types.h>
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <linux/mtd/nand_legacy.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
#endif

void wait_for_command_complete(unsigned int wd_base);

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

/*****************************************
 * Routine: board_init
 * Description: Early hardware init.
 *****************************************/
int board_init(void)
{
	u32 rev ;
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init();		/* in SRAM or SDRM, finish GPMC */
	rev = get_board_type();
	if ((rev == BOARD_H4_SDP)) {
		gd->bd->bi_arch_number = MACH_TYPE_OMAP_H4;	/* board id for linux */
	} else {
		gd->bd->bi_arch_number = MACH_TYPE_OMAP_2430SDP;	/* board id for linux */
	}
	gd->bd->bi_boot_params = (OMAP24XX_SDRC_CS0 + 0x100);	/* adress of boot parameters */

	return 0;
}

/*****************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access 
 * (GP Device only)
 *****************************************/
void secure_unlock(void)
{
	/* Permission values for registers -Full fledged permissions to all */
	#define UNLOCK_1 0xFFFFFFFF
	#define UNLOCK_2 0x00000000
	#define UNLOCK_3 0x0000FFFF
	/* Protection Module Register Target APE (PM_RT)*/
	__raw_writel(UNLOCK_1, PM_RT_APE_BASE_ADDR_ARM + 0x68); /* REQ_INFO_PERMISSION_1 L*/
	__raw_writel(UNLOCK_1, PM_RT_APE_BASE_ADDR_ARM + 0x50);  /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_1, PM_RT_APE_BASE_ADDR_ARM + 0x58);  /* WRITE_PERMISSION_0 L*/
	__raw_writel(UNLOCK_2, PM_RT_APE_BASE_ADDR_ARM + 0x60); /* ADDR_MATCH_1 L*/


	__raw_writel(UNLOCK_3, PM_GPMC_BASE_ADDR_ARM + 0x48); /* REQ_INFO_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_GPMC_BASE_ADDR_ARM + 0x50); /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_GPMC_BASE_ADDR_ARM + 0x58); /* WRITE_PERMISSION_0 L*/

	__raw_writel(UNLOCK_3, PM_OCM_RAM_BASE_ADDR_ARM + 0x48); /* REQ_INFO_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_OCM_RAM_BASE_ADDR_ARM + 0x50); /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_OCM_RAM_BASE_ADDR_ARM + 0x58); /* WRITE_PERMISSION_0 L*/
	__raw_writel(UNLOCK_2, PM_OCM_RAM_BASE_ADDR_ARM + 0x80);  /* ADDR_MATCH_2 L*/

	/* IVA Changes */
	__raw_writel(UNLOCK_3, PM_IVA2_BASE_ADDR_ARM + 0x48); /* REQ_INFO_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_IVA2_BASE_ADDR_ARM + 0x50); /* READ_PERMISSION_0 L*/
	__raw_writel(UNLOCK_3, PM_IVA2_BASE_ADDR_ARM + 0x58); /* WRITE_PERMISSION_0 L*/
}

/**********************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP type, unlock the SRAM for
 *  general use.
 ***********************************************************/
void try_unlock_sram(void)
{
	int mode;

	/* if GP device unlock device SRAM for general use */
	/* secure code breaks for Secure/Emulation device - HS/E/T*/
	mode = get_device_type();
	if (mode == GP_DEVICE) {
		secure_unlock();
	}
	return;
}

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called path is with sram stack.
 **********************************************************/
void s_init(void)
{
	int in_sdram = running_in_sdram();
	/* u32 rev = get_cpu_rev(); unused as of now.. */

	watchdog_init();

	try_unlock_sram();	/* Do SRAM availability first - take care of permissions too */

	set_muxconf_regs();
	delay(100);

	if (!in_sdram){
		prcm_init();
	}

	peripheral_enable();
	icache_enable();
	if (!in_sdram)
		sdrc_init();
}

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r(void)
{
	ether_init();	/* better done here so timers are init'ed */
	return (0);
}

/****************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************/
void watchdog_init(void)
{
	/* There are 4 watch dogs.  1 secure, and 3 general purpose.
	 * The ROM takes care of the secure one. Of the 3 GP ones,
	 * 1 can reset us directly, the other 2 only generate MPU interrupts.
	 */
	__raw_writel(WD_UNLOCK1, WD2_BASE + WSPR);
	wait_for_command_complete(WD2_BASE);
	__raw_writel(WD_UNLOCK2, WD2_BASE + WSPR);
}

/******************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 ******************************************************/
void wait_for_command_complete(unsigned int wd_base)
{
	int pending = 1;
	do {
		pending = __raw_readl(wd_base + WWPS);
	} while (pending);
}

/*******************************************************************
 * Routine:ether_init
 * Description: take the Ethernet controller out of reset and wait
 *  		   for the EEPROM load to complete.
 ******************************************************************/
void ether_init(void)
{
#ifdef CONFIG_DRIVER_LAN91C96
	int cnt = 20;

	/* u32 rev = get_cpu_rev(); unused as of now */


	__raw_writew(0x0, LAN_RESET_REGISTER);
	do {
		__raw_writew(0x1, LAN_RESET_REGISTER);
		udelay(100);
		if (cnt == 0)
			goto h4reset_err_out;
		--cnt;
	} while (__raw_readw(LAN_RESET_REGISTER) != 0x1);

	cnt = 20;

	do {
		__raw_writew(0x0, LAN_RESET_REGISTER);
		udelay(100);
		if (cnt == 0)
			goto h4reset_err_out;
		--cnt;
	} while (__raw_readw(LAN_RESET_REGISTER) != 0x0000);
	udelay(1000);

	*((volatile unsigned char *)ETH_CONTROL_REG) &= ~0x01;
	udelay(1000);

      h4reset_err_out:
	return;
#endif
}

/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int size0 = 0, size1 = 0;
	u32 mtype, btype;
#ifdef CONFIG_DRIVER_OMAP24XX_I2C
	u8 data;
#endif
#define NOT_EARLY 0

#ifdef CONFIG_DRIVER_OMAP24XX_I2C
	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
	select_bus(1, CFG_I2C_SPEED);	/* select bus with T2 on it */
#endif
	btype = get_board_type();
	mtype = get_mem_type();
	display_board_info(btype);
#ifdef CONFIG_DRIVER_OMAP24XX_I2C
	if (btype == BOARD_SDP_2430_T2) {		
		/* Enable VMODE following voltage switching */
		data = 0x24;  /* set the floor voltage to 1.05v */
		i2c_write(I2C_TRITON2, 0xBB, 1, &data, 1);   
		data = 0x38; /* set the roof voltage to 1.3V */
		i2c_write(I2C_TRITON2, 0xBC, 1, &data, 1);		
		data = 0x0; /* set jump mode for VDD voltage transition */
		i2c_write(I2C_TRITON2, 0xBD, 1, &data, 1);  
		data = 1; /* enable voltage scaling */
		i2c_write(I2C_TRITON2, 0xBA, 1, &data, 1); 
	}
#endif

	if ((mtype == DDR_COMBO) || (mtype == DDR_STACKED)) {
		/* init other chip select and map CS1 right after CS0 */
		do_sdrc_init(SDRC_CS1_OSET, NOT_EARLY);
	}
	size0 = get_sdr_cs_size(SDRC_CS0_OSET);
	size1 = get_sdr_cs_size(SDRC_CS1_OSET);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = size0;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_1+size0;
	gd->bd->bi_dram[1].size = size1;

	return 0;
}

#define MUX_VAL(OFFSET,VALUE)\
		__raw_writeb((VALUE), OMAP24XX_CTRL_BASE + (OFFSET));

#if (CONFIG_2430SDP)
#define MUX_DEFAULT()\
	/* SDRC */\
	MUX_VAL(0x0054, 0x1B)		/* sdrc_a14 - EN, HI, 3, ->gpio_0 */\
	MUX_VAL(0x0055, 0x1B)		/* sdrc_a13 - EN, HI, 3, ->gpio_1 */\
	MUX_VAL(0x0056, 0x00)		/* sdrc_a12 - Dis, 0 */\
	MUX_VAL(0x0046, 0x00)		/* sdrc_ncs1 - Dis, 0 */\
	MUX_VAL(0x0048, 0x00)		/* sdrc_cke1 - Dis, 0 */\
	/* GPMC */\
	MUX_VAL(0x0030, 0x00)		/* gpmc_clk - Dis, 0 */\
	MUX_VAL(0x0032, 0x00)		/* gpmc_ncs1- Dis, 0 */\
	MUX_VAL(0x0033, 0x00)		/* gpmc_ncs2- Dis, 0 */\
	MUX_VAL(0x0034, 0x03)		/* gpmc_ncs3- Dis, 3, ->gpio_24 */\
	MUX_VAL(0x0035, 0x03)		/* gpmc_ncs4- Dis, 3, ->gpio_25 */\
	MUX_VAL(0x0036, 0x00)		/* gpmc_ncs5- Dis, 0 */\
	MUX_VAL(0x0037, 0x03)		/* gpmc_ncs6- Dis, 3, ->gpio_27 */\
	MUX_VAL(0x0038, 0x00)		/* gpmc_ncs7- Dis, 0 */\
	MUX_VAL(0x0040, 0x18)		/* gpmc_wait1- Dis, 0 */\
	MUX_VAL(0x0041, 0x18)		/* gpmc_wait2- Dis, 0 */\
	MUX_VAL(0x0042, 0x1B)		/* gpmc_wait3- EN, HI, 3, ->gpio_35 */\
	MUX_VAL(0x0085, 0x1B)		/* gpmc_a10- EN, HI, 3, ->gpio_3 */\
	/* GPMC mux for NAND access */\
        MUX_VAL(0x0086, 0x18)		/* gpmc_a9 - EN, HI, 0*/\
        MUX_VAL(0x0087, 0x18)		/* gpmc_a8 - EN, HI, 0*/\
        MUX_VAL(0x0088, 0x18)		/* gpmc_a7 - EN, HI, 0*/\
        MUX_VAL(0x0089, 0x18)		/* gpmc_a6 - EN, HI, 0*/\
        MUX_VAL(0x008A, 0x18)		/* gpmc_a5 - EN, HI, 0*/\
        MUX_VAL(0x008B, 0x18)		/* gpmc_a4 - EN, HI, 0*/\
        MUX_VAL(0x008C, 0x18)		/* gpmc_a3 - EN, HI, 0*/\
        MUX_VAL(0x008D, 0x18)		/* gpmc_a2 - EN, HI, 0*/\
        MUX_VAL(0x008E, 0x18)		/* gpmc_a1 - EN, HI, 0*/\
        MUX_VAL(0x008F, 0x18)		/* gpmc_d15 - EN,HI, 0*/\
        MUX_VAL(0x0090, 0x18)		/* gpmc_d14 - EN, HI, 0*/\
        MUX_VAL(0x0091, 0x18)		/* gpmc_d13 - EN, HI, 0*/\
        MUX_VAL(0x0092, 0x18)		/* gpmc_d12 - EN, HI, 0*/\
        MUX_VAL(0x0093, 0x18)		/* gpmc_d11 - EN, HI, 0*/\
        MUX_VAL(0x0094, 0x18)		/* gpmc_d10 - EN, HI, 0*/\
        MUX_VAL(0x0095, 0x18)		/* gpmc_d9 - EN, HI, 0 */\
        MUX_VAL(0x0096, 0x18)		/* gpmc_d8 - EN, HI, 0*/\
	/* DSS */\
	MUX_VAL(0x009F, 0x00)		/* dss_data0- Dis, 0 */\
	MUX_VAL(0x00A0, 0x00)		/* dss_data1- Dis, 0 */\
	MUX_VAL(0x00A1, 0x00)		/* dss_data2- Dis, 0 */\
	MUX_VAL(0x00A2, 0x00)		/* dss_data3- Dis, 0 */\
	MUX_VAL(0x00A3, 0x00)		/* dss_data4- Dis, 0 */\
	MUX_VAL(0x00A4, 0x00)		/* dss_data5- Dis, 0 */\
	MUX_VAL(0x00A5, 0x00)		/* dss_data6- Dis, 0 */\
	MUX_VAL(0x00A6, 0x00)		/* dss_data7- Dis, 0 */\
	MUX_VAL(0x00A7, 0x00)		/* dss_data8- Dis, 0 */\
	MUX_VAL(0x00A8, 0x00)		/* dss_data9- Dis, 0 */\
	MUX_VAL(0x00A9, 0x00)		/* dss_data10- Dis, 0 */\
	MUX_VAL(0x00AA, 0x00)		/* dss_data11- Dis, 0 */\
	MUX_VAL(0x00AB, 0x00)		/* dss_data12- Dis, 0 */\
	MUX_VAL(0x00AC, 0x00)		/* dss_data13- Dis, 0 */\
	MUX_VAL(0x00AD, 0x00)		/* dss_data14- Dis, 0 */\
	MUX_VAL(0x00AE, 0x00)		/* dss_data15- Dis, 0 */\
	MUX_VAL(0x00AF, 0x00)		/* dss_data16- Dis, 0 */\
	MUX_VAL(0x00B0, 0x00)		/* dss_data17- Dis, 0 */\
	MUX_VAL(0x00B9, 0x00)		/* dss_hsync- Dis, 0 */\
	MUX_VAL(0x00BA, 0x00)		/* dss_acbias- Dis, 0 */\
	MUX_VAL(0x00B1, 0x1B)		/* uart1_cts- EN, HI, 3, ->gpio_32 */\
	MUX_VAL(0x00B2, 0x1B)		/* uart1_rts- EN, HI, 3, ->gpio_8 */\
	MUX_VAL(0x00B3, 0x1B)		/* uart1_tx- EN, HI, 3, ->gpio_9 */\
	MUX_VAL(0x00B4, 0x1B)		/* uart1_rx- EN, HI, 3, ->gpio_10 */\
	MUX_VAL(0x00B5, 0x1B)		/* mcbsp2_dr- EN, HI, 3, ->gpio_11 */\
	MUX_VAL(0x00B6, 0x1B)		/* mcbsp2_clkx- EN, HI, 3, ->gpio_12 */\
	/* CONTROL */\
	MUX_VAL(0x00BB, 0x00)		/* sys_nrespwron- Dis, 0 */\
	MUX_VAL(0x00BC, 0x00)		/* sys_nreswarm- Dis, 0 */\
	MUX_VAL(0x00BD, 0x18)		/* sys_nirq0- EN, HI, 0 */\
	/*MUX_VAL(0x00BD, 0x1B)*/	/* sys_nirq0- EN, HI, 3, ->gpio_56 */\
	MUX_VAL(0x00BE, 0x18)		/* sys_nirq1- EN, HI, 0 */\
	MUX_VAL(0x00C7, 0x00)		/* gpio_132- Dis, 0, ->gpio132 */\
	MUX_VAL(0x00CB, 0x00)		/* gpio_133- Dis, 0, ->gpio133 */\
	MUX_VAL(0x00C9, 0x18)		/* sys_clkout- Dis, 0 */\
	/*MUX_VAL(0x00C9, 0x1B)*/	/* sys_clkout- EN, HI, 3, ->gpio_111 */\
	MUX_VAL(0x00CC, 0x18)		/* jtag_emu1- EN, HI, 0 */\
	MUX_VAL(0x00CD, 0x18)		/* jtag_emu0- EN, HI, 0 */\
	/* CAMERA */\
	MUX_VAL(0x00DD, 0x02)		/* cam_d0- Dis, 2, sti_dout */\
	MUX_VAL(0x00DC, 0x02)		/* cam_d1- Dis, 2, sti_din */\
	MUX_VAL(0x00DB, 0x1B)		/* cam_d2- EN, HI, 3, ->gpio_129 */\
	MUX_VAL(0x00DA, 0x1B)		/* cam_d3- EN, HI, 3, ->gpio_128 */\
	MUX_VAL(0x00D9, 0x00)		/* cam_d4- Dis, 0 */\
	MUX_VAL(0x00D8, 0x00)		/* cam_d5- Dis, 0 */\
	MUX_VAL(0x00D7, 0x00)		/* cam_d6- Dis, 0 */\
	MUX_VAL(0x00D6, 0x00)		/* cam_d7- Dis, 0 */\
	MUX_VAL(0x00D5, 0x00)		/* cam_d8- Dis, 0 */\
	MUX_VAL(0x00D4, 0x00)		/* cam_d9- Dis, 0 */\
	MUX_VAL(0x00E3, 0x00)		/* cam_d10- Dis, 0 */\
	MUX_VAL(0x00E2, 0x00)		/* cam_d11- Dis, 0 */\
	MUX_VAL(0x00DE, 0x00)		/* cam_hs- Dis, 0 */\
	MUX_VAL(0x00DF, 0x00)		/* cam_vs- Dis, 0 */\
	MUX_VAL(0x00E0, 0x00)		/* cam_lclk- Dis, 0 */\
	MUX_VAL(0x00E1, 0x00)		/* cam_xclk- Dis, 0 */\
	MUX_VAL(0x00E4, 0x01)		/* gpio_134- Dis, 1, ->ccp_datn */\
	MUX_VAL(0x00E5, 0x01)		/* gpio_135- Dis, 1, ->ccp_datp */\
	MUX_VAL(0x00E6, 0x01)		/* gpio_136- Dis, 1, ->ccp_clkn */\
	MUX_VAL(0x00E7, 0x01)		/* gpio_137- Dis, 1, ->ccp_clkp */\
	MUX_VAL(0x00E8, 0x01)		/* gpio_138- Dis, 1, ->spi3_clk */\
	MUX_VAL(0x00E9, 0x01)		/* gpio_139- Dis, 1, ->spi3_cs0 */\
	MUX_VAL(0x00EA, 0x01)		/* gpio_140- Dis, 1, ->spi3_simo */\
	MUX_VAL(0x00EB, 0x01)		/* gpio_141- Dis, 1, ->spi3_somi */\
	MUX_VAL(0x00EC, 0x18)		/* gpio_142- EN, HI, 0, ->gpio_142 */\
	MUX_VAL(0x00ED, 0x18)		/* gpio_154- EN, HI, 0, ->gpio_154 */\
	MUX_VAL(0x00EE, 0x18)		/* gpio_148- EN, HI, 0, ->gpio_148 */\
	MUX_VAL(0x00EF, 0x18)		/* gpio_149- EN, HI, 0, ->gpio_149 */\
	MUX_VAL(0x00F0, 0x18)		/* gpio_150- EN, HI, 0, ->gpio_150 */\
	MUX_VAL(0x00F1, 0x18)		/* gpio_152- EN, HI, 0, ->gpio_152 */\
	MUX_VAL(0x00F2, 0x18)		/* gpio_153- EN, HI, 0, ->gpio_153 */\
	/* MMC1 */\
	MUX_VAL(0x00F3, 0x00)		/* mmc1_clko- Dis, 0 */\
	MUX_VAL(0x00F4, 0x18)		/* mmc1_cmd- EN, HI, 0 */\
	MUX_VAL(0x00F5, 0x18)		/* mmc1_dat0- EN, HI, 0 */\
	MUX_VAL(0x00F6, 0x18)		/* mmc1_dat1- EN, HI, 0 */\
	MUX_VAL(0x00F7, 0x18)		/* mmc1_dat2- EN, HI, 0 */\
	MUX_VAL(0x00F8, 0x18)		/* mmc1_dat3- EN, HI, 0 */\
	/* MMC2 */\
	MUX_VAL(0x00F9, 0x00)		/* mmc2_clko- Dis, 0 */\
	MUX_VAL(0x00FA, 0x18)		/* mmc2_cmd- EN, HI, 0 */\
	MUX_VAL(0x00FB, 0x18)		/* mmc2_dat0- EN, HI, 0 */\
	MUX_VAL(0x00FC, 0x18)		/* mmc2_dat1- EN, HI, 0 */\
	MUX_VAL(0x00FD, 0x18)		/* mmc2_dat2- EN, HI, 0 */\
	MUX_VAL(0x00FE, 0x18)		/* mmc2_dat3- EN, HI, 0 */\
	/* UART2 */\
	MUX_VAL(0x00FF, 0x00)		/* uart2_cts- Dis, 0 */\
	MUX_VAL(0x0100, 0x00)		/* uart2_rts- Dis, 0 */\
	MUX_VAL(0x0101, 0x00)		/* uart2_tx- Dis, 0 */\
	MUX_VAL(0x0102, 0x00)		/* uart2_rx- Dis, 0 */\
	/* MCBSP3 */\
	MUX_VAL(0x0103, 0x00)		/* mcbsp3_clkx- Dis, 0 */\
	MUX_VAL(0x0104, 0x00)		/* mcbsp3_fsx- Dis, 0 */\
	MUX_VAL(0x0105, 0x00)		/* mcbsp3_dr- Dis, 0 */\
	MUX_VAL(0x0106, 0x00)		/* mcbsp3_dx- Dis, 0 */\
	/* SSI1 */\
	MUX_VAL(0x0107, 0x01)		/* ssi1_dat_tx- Dis, 1, ->uart1_tx */\
	MUX_VAL(0x0108, 0x01)		/* ssi1_flag_tx- Dis, 1, ->uart1_rts */\
	MUX_VAL(0x0109, 0x01)		/* ssi1_rdy_tx- Dis, 1, ->uart1_cts */\
	MUX_VAL(0x010A, 0x01)		/* ssi1_dat_rx- Dis, 1, ->uart1_rx */\
	MUX_VAL(0x010B, 0x01)		/* gpio_63- Dis, 1, ->mcbsp4_clkx */\
	MUX_VAL(0x010C, 0x01)		/* ssi1_flag_rx- Dis, 1, ->mcbsp4_dr */\
	MUX_VAL(0x010D, 0x01)		/* ssi1_rdy_rx- Dis, 1, ->mcbsp4_dx */\
	MUX_VAL(0x010E, 0x01)		/* ssi1_wake- Dis, 1, ->mcbsp4_fsx */\
	/* SPI1 */\
	MUX_VAL(0x010F, 0x00)		/* spi1_clk- Dis, 0 */\
	MUX_VAL(0x0110, 0x00)		/* spi1_simo- Dis, 0 */\
	MUX_VAL(0x0111, 0x00)		/* spi1_somi- Dis, 0 */\
	MUX_VAL(0x0112, 0x00)		/* spi1_cs0- Dis, 0 */\
	MUX_VAL(0x0113, 0x00)		/* spi1_cs1- Dis, 0 */\
	MUX_VAL(0x0114, 0x00)		/* spi1_cs2- Dis, 0 */\
	MUX_VAL(0x0115, 0x00)		/* spi1_cs3- Dis, 0 */\
	/* SPI2 */\
	MUX_VAL(0x0116, 0x1B)		/* spi2_clk- EN, HI, 3, ->gpio_88 */\
	MUX_VAL(0x0117, 0x1B)		/* spi2_simo- EN, HI, 3, ->gpio_89 */\
	MUX_VAL(0x0118, 0x1B)		/* spi2_somi- EN, HI, 3, ->gpio_90 */\
	MUX_VAL(0x0119, 0x1B)		/* spi2_cs0- EN, HI, 3, ->gpio_91 */\
	/* MCBSP1 */\
	MUX_VAL(0x011A, 0x00)		/* mcbsp1_clkr- Dis, 0 */\
	MUX_VAL(0x011B, 0x00)		/* mcbsp1_fsr- Dis, 0 */\
	MUX_VAL(0x011C, 0x00)		/* mcbsp1_dx- Dis, 0 */\
	MUX_VAL(0x011D, 0x00)		/* mcbsp1_dr- Dis, 0 */\
	MUX_VAL(0x011E, 0x00)		/* mcbsp1_clks- Dis, 0 */\
	MUX_VAL(0x011F, 0x00)		/* mcbsp1_fsx- Dis, 0 */\
	MUX_VAL(0x0120, 0x00)		/* mcbsp1_clkx- Dis, 0 */\
	/* HDQ */\
	MUX_VAL(0x0125, 0x00)		/* hdq_sio- Dis, 0 */\
	/* UART3 */\
	MUX_VAL(0x0126, 0x00)		/* uart3_cts_rctx- Dis, 0 */\
	MUX_VAL(0x0127, 0x00)		/* uart3_rts_sd- Dis, 0 */\
	MUX_VAL(0x0128, 0x00)		/* uart3_tx_irtx- Dis, 0 */\
	MUX_VAL(0x0129, 0x00)		/* uart3_rx_irrx- Dis, 0 */\
	/* OTHERS */\
	MUX_VAL(0x012B, 0x1B)		/* gpio_78- EN, HI, 3, ->gpio_78 */\
	MUX_VAL(0x012C, 0x01)		/* gpio_79- Dis, 1, ->secure_indicator */\
	MUX_VAL(0x012D, 0x1B)		/* gpio_80- EN, HI, 3, ->gpio_80 */\
	/* MCBSP2 */\
	MUX_VAL(0x012E, 0x01)		/* gpio_113- Dis, 1, ->mcbsp2_clkx */\
	MUX_VAL(0x012F, 0x01)		/* gpio_114- Dis, 1, ->mcbsp2_fsx */\
	MUX_VAL(0x0130, 0x01)		/* gpio_115- Dis, 1, ->mcbsp2_dr */\
	MUX_VAL(0x0131, 0x01)		/* gpio_116- Dis, 1, ->mcbsp2_dx */\
	/* GPIO7-AUDIOENVDD */\
	MUX_VAL(0x012A, 0x18)		/* gpio_7- EN, HI, 3, ->gpio_7 */\

#else
/* For all other platforms */
#define MUX_DEFAULT()\
	/* SDRC */\
	MUX_VAL(0x0054, 0x08)		/* sdrc_a14 - EN, LO, 0 */\
	MUX_VAL(0x0055, 0x08)		/* sdrc_a13 - EN, LO, 0 */\
	MUX_VAL(0x0056, 0x08)		/* sdrc_a12 - EN, LO, 0 */\
	MUX_VAL(0x0045, 0x18)		/* sdrc_ncs1 - EN, HI, 0 */\
	MUX_VAL(0x0046, 0x18)		/* sdrc_ncs2 - EN, HI, 0 */\
	/* GPMC */\
	MUX_VAL(0x0030, 0x08)		/* gpmc_clk - EN, LO, 0 */\
	MUX_VAL(0x0032, 0x18)		/* gpmc_ncs1- EN, HI, 0 */\
	MUX_VAL(0x0033, 0x18)		/* gpmc_ncs2- EN, HI, 0 */\
	MUX_VAL(0x0034, 0x18)		/* gpmc_ncs3- EN, HI, 0 */\
	/* UART1 */\
	MUX_VAL(0x00B1, 0x18)		/* uart1_cts- EN, HI, 0 */\
	MUX_VAL(0x00B2, 0x18)		/* uart1_rts- EN, HI, 0 */\
	MUX_VAL(0x00B3, 0x18)		/* uart1_tx- EN, HI, 0 */\
	MUX_VAL(0x00B4, 0x18)		/* uart1_rx- EN, HI, 0 */\
	/* UART2 */\
	MUX_VAL(0x00FF, 0x18)		/* uart2_cts- EN, HI, 0 */\
	MUX_VAL(0x0100, 0x18)		/* uart2_rts- EN, HI, 0 */\
	MUX_VAL(0x0101, 0x18)		/* uart2_tx- EN, HI, 0 */\
	MUX_VAL(0x0102, 0x18)		/* uart2_rx- EN, HI, 0 */\
	/* UART3 */\
	MUX_VAL(0x0126, 0x18)		/* uart3_cts_rctx- EN, HI, 0 */\
	MUX_VAL(0x0127, 0x18)		/* uart3_rts_sd- EN, HI, 0 */\
	MUX_VAL(0x0127, 0x18)		/* uart3_tx_irtx- EN, HI, 0 */\
	MUX_VAL(0x0127, 0x18)		/* uart3_rx_irrx- EN, HI, 0 */\
	/* I2C1 */\
	MUX_VAL(0x0111, 0x00)		/* i2c1_scl - DIS, NA, 0 */\
	MUX_VAL(0x0112, 0x00)		/* i2c1_sda - DIS, NA, 0 */\

#endif /* End of Mux Mapping */

#if 0
/**********************************************************
 * Routine: mux_pwr_save
 * Description: Set pins to optimal power savings state.
 *
 * NOTE ES1 Failures for Errata, Do NOT include:
 *  - set D0-D31 (boot failure, u-boot ok)
 *  - set D16-D31 (boot failue, u-boot ok)
 *  - set D0-D15 (boot with memory errors, u-boot ok)
 *  - set DQS3 (boot failures, u-boot start some failures).
 *  - set DQS0-2 (no apparent problems).
 *
 *********************************************************/
static void mux_pwr_save(void)
{
	#define SDRAM_WIDTH	32
	#define OPTIMZE_FOR_DDR	1
	#define NUM_DQS		4

	u32 addr, val, offset, base = OMAP24XX_CTRL_BASE;

	/* Activate DDR/SDR-SDRAM signal pull-ups on DQ signals to save power */
	for(offset = 0x65; offset < (0x65 + SDRAM_WIDTH); offset++){
		addr = base + offset;			/* addr of pin config */
		val = __raw_readb(addr) | (BIT4|BIT3);	/* mask for pull-up on */
		__raw_writeb(val, addr);		/* update config */
	}
#if OPTIMZE_FOR_DDR
	/* Activate DDR-SDRAM signal pull-ups on DQS signals to save power */
	for(offset = 0x50; (offset < 0x50 + NUM_DQS); offset++){
		addr = base + offset;			/* addr of pin config */
		val = __raw_readb(addr) | (BIT4|BIT3);	/* mask for pull-up on */
		__raw_writeb(val, addr);		/* update config */
	}
#endif
}
#endif

/**********************************************************
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers
 *              specific to the hardware. Many pins need
 *              to be moved from protect to primary mode.
 *********************************************************/
void set_muxconf_regs(void)
{
	u32 cpu;
	cpu = get_cpu_type();
	/* Incase we have to handle multiple processors such as 2430 and 2430C */
	if (cpu == CPU_2430) {
		MUX_DEFAULT();
		// mux_pwr_save();  /* this fails on 2430-ES1 dispite recommendation */
	} else
		return;

}

/*****************************************************************
 * Routine: peripheral_enable
 * Description: Enable the clks & power for perifs (GPT2, UART1,...)
 ******************************************************************/
void peripheral_enable(void)
{

	unsigned int v, if_clks1 = 0, func_clks1 = 0, if_clks2 = 0, func_clks2 = 0;
	/* ALERT STATUS 10000 */
	/* Enable GP2 timer. */
	if_clks1 |= BIT4;
	func_clks1 |= BIT4;
	v = __raw_readl(CM_CLKSEL2_CORE) | 0x4;	/* Sys_clk input OMAP24XX_GPT2 */
	__raw_writel(v, CM_CLKSEL2_CORE);
	__raw_writel(0x1, CM_CLKSEL_WKUP);

#ifdef CFG_NS16550
	/* Enable UART1 clock */
	func_clks1 |= BIT21;
	if_clks1 |= BIT21;
#endif
#ifdef CONFIG_DRIVER_OMAP24XX_I2C
	/* 2430 requires only the hs clock */
	func_clks2 |= BIT20|BIT19; /* i2c1 and 2 96 meg clock input */
	if_clks1 |= BIT20|BIT19;
#endif

	v = __raw_readl(CM_ICLKEN1_CORE) | if_clks1;	/* Interface clocks on */
	__raw_writel(v, CM_ICLKEN1_CORE);
	v = __raw_readl(CM_ICLKEN2_CORE) | if_clks2;	/* Interface clocks on */
	__raw_writel(v, CM_ICLKEN2_CORE);
	v = __raw_readl(CM_FCLKEN1_CORE) | func_clks1;	/* Functional Clocks on */
	__raw_writel(v, CM_FCLKEN1_CORE);
	v = __raw_readl(CM_FCLKEN2_CORE) | func_clks2;	/* Functional Clocks on */
	__raw_writel(v, CM_FCLKEN2_CORE);
	delay(1000);
}

/*****************************************************************************
 * Routine: update_mux()
 * Description: Update balls which are different beween boards.  All should be
 *              updated to match functionaly.  However, I'm only updating ones
 *              which I'll be using for now.  When power comes into play they
 *              all need updating.
 *****************************************************************************/
void update_mux(u32 btype, u32 mtype)
{
	/* NOTHING as of now... */
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
void nand_init(void)
{
	extern flash_info_t flash_info[];

	nand_probe(CFG_NAND_ADDR);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
#ifdef CFG_JFFS2_MEM_NAND
	flash_info[CFG_JFFS2_FIRST_BANK].flash_id = nand_dev_desc[0].id;
	flash_info[CFG_JFFS2_FIRST_BANK].size = 1024 * 1024 * 2;	/* only read kernel single meg partition */
	flash_info[CFG_JFFS2_FIRST_BANK].sector_count = 1024;	/* 1024 blocks in 16meg chip (use less for raw/copied partition) */
	flash_info[CFG_JFFS2_FIRST_BANK].start[0] = 0x80200000;	/* ?, ram for now, open question, copy to RAM or adapt for NAND */
#endif
}
#endif
