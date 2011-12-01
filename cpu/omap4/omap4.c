/*
 * (C) Copyright 2004-2009
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
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/clocks.h>
#include <asm/arch/mem.h>
#include <i2c.h>
#include <twl6030.h>
#include <smb347.h>
#include <asm/mach-types.h>
#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand_legacy.h>
#endif
#include <omap-gpio.h>
#include <otter.h>

#define DMM_BASE			0x4e000000
#define DMM_LISA_MAP_0 			0x0040
#define DMM_LISA_MAP_1 			0x0044

void read_all_clocks(void);
/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b" : "=r" (loops) : "0"(loops));
}

/*****************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access
 * (GP Device only)
 *****************************************/
void secure_unlock_mem(void)
{
	/* Permission values for registers -Full fledged permissions to all */
	#define UNLOCK_1 0xFFFFFFFF
	#define UNLOCK_2 0x00000000
	#define UNLOCK_3 0x0000FFFF

	/* Protection Module Register Target APE (PM_RT)*/
	__raw_writel(UNLOCK_1, RT_REQ_INFO_PERMISSION_1);
	__raw_writel(UNLOCK_1, RT_READ_PERMISSION_0);
	__raw_writel(UNLOCK_1, RT_WRITE_PERMISSION_0);
	__raw_writel(UNLOCK_2, RT_ADDR_MATCH_1);

	__raw_writel(UNLOCK_3, GPMC_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, GPMC_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, GPMC_WRITE_PERMISSION_0);

	__raw_writel(UNLOCK_3, OCM_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, OCM_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, OCM_WRITE_PERMISSION_0);
	__raw_writel(UNLOCK_2, OCM_ADDR_MATCH_2);

	/* IVA Changes */
	__raw_writel(UNLOCK_3, IVA2_REQ_INFO_PERMISSION_0);
	__raw_writel(UNLOCK_3, IVA2_READ_PERMISSION_0);
	__raw_writel(UNLOCK_3, IVA2_WRITE_PERMISSION_0);

	__raw_writel(UNLOCK_3, IVA2_REQ_INFO_PERMISSION_1);
	__raw_writel(UNLOCK_3, IVA2_READ_PERMISSION_1);
	__raw_writel(UNLOCK_3, IVA2_WRITE_PERMISSION_1);

	__raw_writel(UNLOCK_3, IVA2_REQ_INFO_PERMISSION_2);
	__raw_writel(UNLOCK_3, IVA2_READ_PERMISSION_2);
	__raw_writel(UNLOCK_3, IVA2_WRITE_PERMISSION_2);

	__raw_writel(UNLOCK_3, IVA2_REQ_INFO_PERMISSION_3);
	__raw_writel(UNLOCK_3, IVA2_READ_PERMISSION_3);
	__raw_writel(UNLOCK_3, IVA2_WRITE_PERMISSION_3);

	__raw_writel(UNLOCK_1, SMS_RG_ATT0); /* SDRC region 0 public */
}


/**********************************************************
 * Routine: secureworld_exit()
 * Description: If chip is EMU and boot type is external
 *		configure secure registers and exit secure world
 *  general use.
 ***********************************************************/

void secureworld_exit(void)
{
	unsigned long i;

	/* configrue non-secure access control register */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c1, 2" : "=r" (i));
	/* enabling co-processor CP10 and CP11 accesses in NS world */
	__asm__ __volatile__("orr %0, %0, #0xC00" : "=r"(i));
	/* allow allocation of locked TLBs and L2 lines in NS world */
	/* allow use of PLE registers in NS world also */
	__asm__ __volatile__("orr %0, %0, #0x70000" : "=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c1, 2" : "=r" (i));

	/*
	 * Enable IBE in ACR register.ASA is disabled following
	 * recommendation from ARM
	 */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1" : "=r" (i));
	__asm__ __volatile__("orr %0, %0, #0x40" : "=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1" : "=r" (i));

	/* Exiting secure world */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c1, 0" : "=r" (i));
	__asm__ __volatile__("orr %0, %0, #0x31" : "=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c1, 0" : "=r" (i));
}

/**********************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP/EMU(special) type, unlock the SRAM for
 *  general use.
 ***********************************************************/
int get_boot_type(void);
void v7_flush_dcache_all(int , int);
void setup_auxcr(int , int);

void try_unlock_memory(void)
{
	int mode;
	int in_sdram = running_in_sdram();

	/* if GP device unlock device SRAM for general use */
	/* secure code breaks for Secure/Emulation device - HS/E/T*/
	mode = get_device_type();
	if (mode == GP_DEVICE)
		secure_unlock_mem();

	/* If device is EMU and boot is XIP external booting
	 * Unlock firewalls and disable L2 and put chip
	 * out of secure world
	 */
	/* Assuming memories are unlocked by the demon who put us in SDRAM */
	if ((mode <= EMU_DEVICE) && (get_boot_type() == 0x1F)
		&& (!in_sdram)) {
		secure_unlock_mem();
		secureworld_exit();
	}

	return;
}

/**********************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 * - Called path is with SRAM stack.
 **********************************************************/

void s_init(void)
{
	int external_boot = 0;
	int in_sdram = running_in_sdram();


#ifdef CONFIG_4430VIRTIO
	in_sdram = 0;  /* allow setup from memory for Virtio */
#endif
	/* TODO: Disable Watchdog's here if needed. */
	watchdog_init();
	
#if 0
	external_boot = (get_boot_type() == 0x1F) ? 1 : 0;
	/* Right now flushing at low MPU speed. Need to move after clock init */
	v7_flush_dcache_all(get_device_type(), external_boot);

	try_unlock_memory();
#endif
#ifndef CONFIG_ICACHE_OFF
	icache_enable();
#endif
    //read_all_clocks();
	/* Clock and Mux configerations is already done in x-loader */
	if(!in_sdram) {
		set_muxconf_regs();
		delay(100);
		prcm_init();
	}
}

void spi_command(void);

void chech_low_bat();
void initialize_lcd(int);

/*******************************************************
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 ********************************************************/
int misc_init_r(void)
{
   int power_source=0;
   int input_limit=0;
   int voltage=0;
   int temperature=0;
   int ms=0;
   int sec=0;
   int pre_boot=0;
   pre_boot=fastboot_preboot();
   //SUSP pin UART4_TX/GPIO156
   __raw_writel(0x30003,0x4A10015C);
   __raw_writel(__raw_readl(0x4805B134) & 0xf7ffffff, 0x4805B134); //GPIO 5 OE
   __raw_writel(__raw_readl(0x4805B13C) | 0x8000000, 0x4805B13C); //GPIO 5 DATAOUT 
//    omap_gpio_set_output(156,GPIO_HIGH);

   __raw_writel(__raw_readl(0x4A100090) | 3,0x4A100090);
   __raw_writel(__raw_readl(0x48059134) & 0xffffffdf, 0x48059134);
   __raw_writel(__raw_readl(0x4805913C) | 0x00000000, 0x4805913C); //GPIO 4 DATAOUT 0x4805913C
//   omap_gpio_set_output(134,GPIO_LOW);
#ifdef CONFIG_DRIVER_OMAP44XX_I2C
    i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
    if (pre_boot==0){
        check_low_bat();
    }
#endif

    /*
    * Summit charger has finished all the detection at
    *  this point, so we are safe to turn on the LCD
    */
    initialize_lcd(OTTER_LCD_DISPLAY_SPLASH);

    //twl6030_kc1_settings();

	return 0;
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

/****************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************/
void watchdog_init(void)
{
	/* There are 3 watch dogs WD1=Secure, WD2=MPU, WD3=IVA. WD1 is
	 * either taken care of by ROM (HS/EMU) or not accessible (GP).
	 * We need to take care of WD2-MPU or take a PRCM reset.  WD3
	 * should not be running and does not generate a PRCM reset.
	 */
	
	__raw_writel(WD_UNLOCK1, WD2_BASE + WSPR);
	wait_for_command_complete(WD2_BASE);
	__raw_writel(WD_UNLOCK2, WD2_BASE + WSPR);
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

/*
 * This function finds the SDRAM size available in the system
 * based on DMM section configurations
 * This is needed because the size of memory installed may be
 * different on different versions of the board
 */
u32 sdram_size(void)
{
	u32 section, i, total_size = 0, size, addr;
	for (i = 0; i < 4; i++) {
		section	= __raw_readl(DMM_LISA_MAP + i*4);
		addr = section & DMM_LISA_MAP_SYS_ADDR_MASK;
		/* See if the address is valid */
		if ((addr >= OMAP44XX_DRAM_ADDR_SPACE_START) &&
		    (addr < OMAP44XX_DRAM_ADDR_SPACE_END)) {
			size	= ((section & DMM_LISA_MAP_SYS_SIZE_MASK) >>
				    DMM_LISA_MAP_SYS_SIZE_SHIFT);
			size	= 1 << size;
			size	*= SZ_16M;
			total_size += size;
		}
	}
	return total_size;
}

/**********************************************
 * Routine: dram_init
 * Description: sets uboots idea of sdram size
 **********************************************/
int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	u32 mtype, btype;

	btype = get_board_type();
	mtype = get_mem_type();
	/* fixme... dont know why this func is crashing in ZeBu */
	//display_board_info(btype);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = sdram_size();

	/*SW WA for DMM errata (Errata Id i614) for unmapped
	  (or at the end of mapped section if wrap access) access
	  issue on 4430 ES2.x.
	  Perform this after computing the dram size, so that the
      size computation is correct - if u-boot performs an unmapped
	  access before this point, we will hang. But this is very
	  unlikely to happen */
	__raw_writel(0x80540300, DMM_BASE + DMM_LISA_MAP_1);
	__raw_writel(0x80760300, DMM_BASE + DMM_LISA_MAP_0);

	printf("Load address: 0x%x\n", TEXT_BASE);
	return 0;
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)
/**********************************************************
 * Routine: nand+_init
 * Description: Set up nand for nand and jffs2 commands
 *********************************************************/
void nand_init(void)
{
	/* REVISIT */
	return;
}
#endif

