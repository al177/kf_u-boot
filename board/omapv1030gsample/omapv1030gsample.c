/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
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
#if defined(CONFIG_OMAPV1030)
#include <./configs/omap1510.h>
#endif
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
#endif


void flash__init (void);
void ether__init (void);
void set_muxconf_regs (void);
void peripheral_power_enable (void);

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* arch number of OMAPV10300 G-Sample */
	gd->bd->bi_arch_number = 998;  /* a temp one */

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x10000100;

	/* Configure MUX settings */
	set_muxconf_regs ();
	peripheral_power_enable ();

/* this speeds up your boot a quite a bit.  However to make it
 *  work, you need make sure your kernel startup flush bug is fixed.
 *  ... rkw ...
 */
	icache_enable ();

	flash__init ();
	ether__init ();
	return 0;
}


int misc_init_r (void)
{
	/* currently empty */
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
void flash__init (void)
{
#define EMIFS_GlB_Config_REG 0xfffecc0c
	unsigned int regval;
	regval = *((volatile unsigned int *) EMIFS_GlB_Config_REG);
	/* Turn off write protection for flash devices. */
	regval = regval | 0x0001;
	*((volatile unsigned int *) EMIFS_GlB_Config_REG) = regval;
}
/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
	  		   for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
	#define LAN_RESET_REGISTER 0x0840001c
	#define ETH_CONTROL_REG 0x0840030b
	
	int timeout;

	timeout = 1000;
	*((volatile unsigned short *) LAN_RESET_REGISTER) = 0x0000;
	do {
		*((volatile unsigned short *) LAN_RESET_REGISTER) = 0x0001;
		udelay (3);
	} while (*((volatile unsigned short *) LAN_RESET_REGISTER) != 0x0001 && --timeout);
	if (!timeout)
		printf("timed out when resettimg LAN.\n");
		
	timeout = 1000;
	do {
		*((volatile unsigned short *) LAN_RESET_REGISTER) = 0x0000;
		udelay (3);
	} while (*((volatile unsigned short *) LAN_RESET_REGISTER) != 0x0000 && --timeout);
	if (!timeout)
		printf("timed out when resettimg LAN.\n");

	*((volatile unsigned char *) ETH_CONTROL_REG) &= ~0x01;
	udelay (3);
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/******************************************************
 Routine: set_muxconf_regs
 Description: Setting up the configuration Mux registers
 			  specific to the hardware
*******************************************************/
/* OMAPV1030 has a different way to config mux */
void set_muxconf_regs (void)
{
	volatile unsigned int *MuxConfReg;
	/* set each registers to its reset value; */

	/* SPARE Register setting at Configuration level */
	MuxConfReg =
		(volatile unsigned int *) ((unsigned int) 0xFFFE102C);
	*MuxConfReg = 1;
 
	/* Select emifs_nfcs_1 instead of gpio19 */
	MuxConfReg =
		(volatile unsigned int *) ((unsigned int) 0xFFFE11E8);
	*MuxConfReg = 1;

	/* Select emifs_nfcs_2 */
	MuxConfReg =
		(volatile unsigned int *) ((unsigned int) 0xFFFE14B8);
	*MuxConfReg = 2;

	/* Select wire_1 for TEST_NEMU1 */
	//MuxConfReg =
	//	(volatile unsigned int *) ((unsigned int) 0xFFFE12D8);
	//*MuxConfReg = 5;

	/* TBD: add more pin mux here */


	MuxConfReg =
		(volatile unsigned int *) ((unsigned int)COMP_MODE_CTRL_0);
	*MuxConfReg = COMP_MODE_ENABLE;
}

/******************************************************
 Routine: peripheral_power_enable
 Description: Enable the power for UART1
*******************************************************/
void peripheral_power_enable (void)
{
/* OMAPV1030 has a different ULPDR */
#define UART2_48MHZ_ENABLE	((unsigned short)0x0040)
#define SW_CLOCK_REQUEST	((volatile unsigned short *)0xFFFB101A)

	*SW_CLOCK_REQUEST |= UART2_48MHZ_ENABLE;
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
    flash_info[CFG_JFFS2_FIRST_BANK].size = 1024*1024*2;      /* only read kernel single meg partition */             
	flash_info[CFG_JFFS2_FIRST_BANK].sector_count = 1024;   /* 1024 blocks in 16meg chip (use less for raw/copied partition) */
    flash_info[CFG_JFFS2_FIRST_BANK].start[0] = 0x10200000; /* ?, ram for now, open question, copy to RAM or adapt for NAND */
#endif
}
#endif

