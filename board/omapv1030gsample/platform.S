/*
 * G-Sample Board specific setup info
 *
 * (C) Copyright 2004-2005
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 * Jian Zhang <jzhang@ti.com>
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

#include <config.h>
#include <version.h>

#if defined(CONFIG_OMAPV1030)
#include <./configs/omap1510.h>
#endif


_TEXT_BASE:
	.word	TEXT_BASE	/* sdram load addr from config.mk */

.globl platformsetup
platformsetup:

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT1)                   *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT1
	ldr	r1,	VAL_ARM_IDLECT1
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT2)  		        *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT2
	ldr	r1,	VAL_ARM_IDLECT2
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT3)		        *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT3
	ldr	r1,	VAL_ARM_IDLECT3
	str	r1,	[r0]

	mov	r1,	#0x05		/* PER_EN bit */
	ldr	r0,	REG_ARM_RSTCT2
	strh	r1,	[r0]		/* CLKM; Peripheral reset. */

	/* check lock bit is not good enough:
	   when boot from NOR flash and control goes here
	   lock bit is set, ADPLL1 reg1 is zero. This is x2
	   mode which is not what we want. We want x16.
	  */
#if 0
	/* check if ADPLL LOCK bit (BIT 4) set or not */
	ldr	r0,	REG_ADPLL1_3
	ldrh	r1,	[r0]
	ands	r1,	r1,	#0x10	/* Check the LOCK bit.*/
	bne	skip_adpll		/* skip if already locked*/
#else
	/* Load physical SDRAM base. */
	mov 	r0, 	#0x10000000
	/* Get current execution location. */
	mov 	r1, 	pc
	/* Compare. */
	cmp 	r1, 	r0
	/* Skip over ADPLL initialization if running from SDRAM. */
	bge 	skip_adpll
#endif
	/* Set CLKM to bypass mode so DPLL multiplier/divider can be changed */
	/* Otherwise DPLL won't lock */
	/* I supposedly need to enable the dsp clock before switching */
	ldr	r1,	VAL1_ARM_SYSST
	ldr	r0,	REG_ARM_SYSST
	strh	r1,	[r0]
	mov	r0,	#0x400
1:
	subs	r0,	r0,	#0x1	/* wait for any bubbles to finish */
	bne	1b
	ldr	r1,	VAL1_ARM_CKCTL  /* for now, let arm/dsp/tc be dpll/2 */
	ldr	r0,	REG_ARM_CKCTL
	strh	r1,	[r0]
	/* a few nops to let settle */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	/* setup ADPLL1 */
	/* Ramp up the clock to 13*16Mhz */
	ldr	r0,	REG_ADPLL1_1   /* X16 */
	mov	r1,	#0x140
	strh	r1,	[r0]

	/* clear and set enable bit (BIT 14) */
	ldr	r0,	REG_ADPLL1_3
	mov	r1,	#0
	strh	r1,	[r0]
	mov	r1,	#0x4000
 	strh	r1,	[r0]
	/* set, clear, and set INITZ bit (BIT 7) */
	orr	r1,	r1,	#0x80
 	strh	r1,	[r0]
	mov	r1,	#0x4000
 	strh	r1,	[r0]
	orr	r1,	r1,	#0x80
 	strh	r1,	[r0]
	/* get all bits right */
	ldr	r1,	VAL_ADPLL1_3
 	strh	r1,	[r0]
	/* wait until LOCK bit (BIT 4) set */
2:
	ldrh	r1,	[r0]
	ands	r1,	r1,	#0x10	/*	Check the LOCK bit.*/
	beq	2b			/*	loop until bit goes hi. */
lock_end:

	/* Now set CLKM to Sync-Scalable	*/
	/* I supposedly need to enable the dsp clock before switching */
	ldr	r1,	VAL2_ARM_SYSST
	ldr	r0,	REG_ARM_SYSST
	strh	r1,	[r0]
	mov	r0,	#0x400
3:
	subs	r0,	r0,	#0x1	/* wait for any bubbles to finish */
	bne	3b
	ldr	r1,	VAL2_ARM_CKCTL  /* for now, let arm/dsp/tc be dpll/2 */
	ldr	r0,	REG_ARM_CKCTL
	strh	r1,	[r0]
	/* a few nops to let settle */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

skip_adpll:
	/*------------------------------------------------------*
	 * Turn off the watchdog during init...			*
 	 *------------------------------------------------------*/
	ldr	r0,	REG_WATCHDOG
	ldr	r1,	WATCHDOG_VAL1
	str	r1,	[r0]
	ldr	r1,	WATCHDOG_VAL2
	str	r1,	[r0]
	ldr	r0,	REG_WSPRDOG
	ldr	r1,	WSPRDOG_VAL1
	str	r1,	[r0]
	ldr	r0,	REG_WWPSDOG

watch1Wait:
	ldr	r1,	[r0]
	tst	r1,	#0x10
	bne	watch1Wait

	ldr	r0,	REG_WSPRDOG
	ldr	r1,	WSPRDOG_VAL2
	str	r1,	[r0]
	ldr	r0,	REG_WWPSDOG
watch2Wait:
	ldr	r1,	[r0]
	tst	r1,	#0x10
	bne	watch2Wait


	/* Set memory timings corresponding to the new clock speed */

	/* Check execution location to determine current execution location
	 * and branch to appropriate initialization code.
	 */
	/* Load physical SDRAM base. */
	mov 	r0, 	#0x10000000
	/* Get current execution location. */
	mov 	r1, 	pc
	/* Compare. */
	cmp 	r1, 	r0
	/* Skip over EMIF-fast initialization if running from SDRAM. */
	bge 	skip_sdram

	/* Enable EMIFF TC Doubler in OMAP1710 */
	ldr	r0,	REG_EMIFF_DOUBLER
	mov	r1,	#0x1
	str	r1,	[r0]

	/*
	* Delay for SDRAM initialization.
	*/
	mov	r3,	#0x1800		/* value should be checked */
4:
	subs	r3,	r3,	#0x1	/* Decrement count */
	bne 	4b


	/*
	 * Set SDRAM control values. Disable refresh before MRS command.
	 */

	/* mobile ddr operation */
	ldr	r0,	REG_SDRAM_OPERATION	 //cc80
	mov	r2,	#07
	str	r2,	[r0]

	/* config register */
	ldr	r0,	REG_SDRAM_CONFIG    //cc20
	ldr	r1,	SDRAM_CONFIG_VAL
	str	r1,	[r0]

	/* manual command register */
	ldr	r0,	REG_SDRAM_MANUAL_CMD    //cc84
	/* issue set cke high */
	mov	r1,	#CMD_SDRAM_CKE_SET_HIGH	 //#7
	str	r1,	[r0]
	/* issue nop */
	mov	r1,	#CMD_SDRAM_NOP  //#0
	str	r1,	[r0]

	mov	r2,	#0x0100
waitMDDR1:
	subs	r2,	r2,	 #1
	bne	waitMDDR1	/* delay loop */

	/* issue precharge */
	mov	r1,	#CMD_SDRAM_PRECHARGE
	str	r1,	[r0]

	/* issue autorefresh x 2 */
	mov	r1,	#CMD_SDRAM_AUTOREFRESH
	str	r1,	[r0]
	str	r1,	[r0]

	/* mrs register ddr mobile */
	ldr	r0,	REG_SDRAM_MRS
	mov	r1,	#0x33
	str	r1,	[r0]

	/* emrs1 low-power register */
	ldr	r0,	REG_SDRAM_EMRS1
	/* self refresh on all banks */
	mov	r1,	#0
	str	r1,	[r0]

	ldr	r0,	REG_DLL_URD_CONTROL
	ldr	r1,	DLL_URD_CONTROL_VAL
	str	r1,	[r0]

	ldr	r0,	REG_DLL_LRD_CONTROL
	ldr	r1,	DLL_LRD_CONTROL_VAL
	str	r1,	[r0]

	ldr	r0,	REG_DLL_WRT_CONTROL
	ldr	r1,	DLL_WRT_CONTROL_VAL
	str	r1,	[r0]

	/* delay loop */
	mov	r2,	#0x0100
waitMDDR2:
	subs	r2,	r2,	#1
	bne	waitMDDR2

	/*
	 * Delay for SDRAM initialization.
	 */
	mov	r3,	#0x1800
5:
	subs	r3,	r3,	#1	/* Decrement count. */
	bne	5b
	b	common_tc

skip_sdram:

	ldr	r0,	REG_SDRAM_CONFIG
	ldr	r1,	SDRAM_CONFIG_VAL
	str	r1,	[r0]

common_tc:
	/* slow interface */
	ldr	r1,	VAL_TC_EMIFS_CONFIG
	ldr	r0,	REG_TC_EMIFS_CONFIG
	str	r1,	[r0]

	ldr	r1,	VAL_TC_EMIFS_CS0_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS0_CONFIG
	str	r1,	[r0] /* Chip Select 0 */

	ldr	r1,	VAL_TC_EMIFS_CS1_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS1_CONFIG
	str	r1,	[r0] /* Chip Select 1 */

	ldr	r1,	VAL_TC_EMIFS_CS2_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS2_CONFIG
	str	r1,	[r0] /* Chip Select 2 */

	ldr	r1,	VAL_TC_EMIFS_CS3_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS3_CONFIG
	str	r1,	[r0] /* Chip Select 3 */

        /* inserting additional 2 clock cycle hold time for testing LAN */
	ldr     r0,     REG_TC_EMIFS_CS2_ADVANCED
	ldr    	r1,     VAL_TC_EMIFS_CS2_ADVANCED
	str     r1,     [r0]

        /* Start MPU Timer 1 */
        ldr     r0,     REG_MPU_LOAD_TIMER
        ldr     r1,     VAL_MPU_LOAD_TIMER
        str     r1,     [r0]

        ldr     r0,     REG_MPU_CNTL_TIMER
        ldr     r1,     VAL_MPU_CNTL_TIMER
        str     r1,     [r0]

	/* back to arch calling code */
	mov	pc,	lr

	/* the literal pools origin */
	.ltorg


REG_TC_EMIFS_CONFIG:		/* 32 bits */
	.word 0xfffecc0c
REG_TC_EMIFS_CS0_CONFIG:	/* 32 bits */
	.word 0xfffecc10
REG_TC_EMIFS_CS1_CONFIG:	/* 32 bits */
	.word 0xfffecc14
REG_TC_EMIFS_CS2_CONFIG:	/* 32 bits */
	.word 0xfffecc18
REG_TC_EMIFS_CS3_CONFIG:	/* 32 bits */
	.word 0xfffecc1c
REG_TC_EMIFS_CS2_ADVANCED:	/* 32 bits */
	.word 0xfffecc58

/* MPU clock/reset/power mode control registers */
REG_ARM_CKCTL:			/* 16 bits */
	.word 0xfffece00

REG_ARM_IDLECT3:		/* 16 bits */
	.word 0xfffece24
REG_ARM_IDLECT2:		/* 16 bits */
	.word 0xfffece08
REG_ARM_IDLECT1:		/* 16 bits */
	.word 0xfffece04

REG_ARM_RSTCT2:			/* 16 bits */
	.word 0xfffece14
REG_ARM_SYSST:			/* 16 bits */
	.word 0xfffece18
/* ADPLL registers */
REG_ADPLL1_1:			/* 16 bits */
	.word 0xfffecf02
REG_ADPLL1_3:			/* 16 bits */
	.word 0xfffecf06
VAL_ADPLL1_3:
	.word 0x0000dc98

/* Watch Dog register */
/* secure watchdog stop */
REG_WSPRDOG:
	.word 0xfffeb048
/* watchdog write pending */
REG_WWPSDOG:
	.word 0xfffeb034

WSPRDOG_VAL1:
	.word 0x0000aaaa
WSPRDOG_VAL2:
	.word 0x00005555

/* SDRAM config is: auto refresh enabled, 16 bit 4 bank,
 counter @8192 rows, 10 ns, 8 burst */
REG_SDRAM_CONFIG:
	.word 0xfffecc20

/* Operation register */
REG_SDRAM_OPERATION:
	.word 0xfffecc80

REG_EMIFF_DOUBLER:
	.word 0xfffecc60

/* Manual command register */
REG_SDRAM_MANUAL_CMD:
	.word 0xfffecc84

/* SDRAM MRS (New) config is: CAS latency is 2, burst length 8 */
REG_SDRAM_MRS:
	.word 0xfffecc70

/* SDRAM MRS (New) config is: CAS latency is 2, burst length 8 */
REG_SDRAM_EMRS1:
	.word 0xfffecc78

/* WRT DLL register */
REG_DLL_WRT_CONTROL:
	.word 0xfffecc64
DLL_WRT_CONTROL_VAL:
	.word 0x00000006

/* URD DLL register */
REG_DLL_URD_CONTROL:
	.word 0xfffeccc0
DLL_URD_CONTROL_VAL:
	.word 0x00000006

/* LRD DLL register */
REG_DLL_LRD_CONTROL:
	.word 0xfffecccc

REG_WATCHDOG:
	.word 0xfffec808

REG_MPU_LOAD_TIMER:
        .word 0xfffec600
REG_MPU_CNTL_TIMER:
        .word 0xfffec500

/* 96 MHz Samsung Mobile DDR */
SDRAM_CONFIG_VAL:
	.word 0x0003f7f6

DLL_LRD_CONTROL_VAL:
	.word 0x00000006

VAL1_ARM_CKCTL:
	.word 0x1555
VAL2_ARM_CKCTL:
	.word 0x350e
VAL1_ARM_SYSST:
	.word 0x2800
VAL2_ARM_SYSST:
	.word 0x1001

VAL_DPLL1_CTL:
	.word 0x2810

VAL_TC_EMIFS_CONFIG:
	.word 0x00000010

VAL_TC_EMIFS_CS0_CONFIG:
	.word 0x0017000c
VAL_TC_EMIFS_CS1_CONFIG:
	.word 0x0010fffb
/* EMIFS CS2 Configuration Register: A/D Multiplexed, 8 RWS, 8 WWS,
   WELEN = 4, 1 BT WST, Asynchronous Read mode & Ref_Clk = TC_Clock */
VAL_TC_EMIFS_CS2_CONFIG:
	.word 0x0040488b 
VAL_TC_EMIFS_CS3_CONFIG:	
	.word 0xffc0fff3
/* EMIFS CS2 Advanced Configuration Register: ADV hold = 2 Ref_Clk cycles,
   OE SETUP = 3 */
VAL_TC_EMIFS_CS2_ADVANCED:
	.word 0x00000103

VAL_TC_EMIFF_SDRAM_CONFIG:
	.word 0x010290fc
VAL_TC_EMIFF_MRS:
	.word 0x00000027

VAL_ARM_IDLECT1:
	.word 0x000014c6

VAL_ARM_IDLECT2:
	.word 0x000009ff
VAL_ARM_IDLECT3:
	.word 0x0000003f

WATCHDOG_VAL1:
	.word 0x000000f5
WATCHDOG_VAL2:
	.word 0x000000a0

VAL_MPU_LOAD_TIMER:
        .word 0xffffffff
VAL_MPU_CNTL_TIMER:
        .word 0xffffffa1

/* command values */
.equ CMD_SDRAM_NOP,		0x00000000
.equ CMD_SDRAM_PRECHARGE,	0x00000001
.equ CMD_SDRAM_AUTOREFRESH,	0x00000002
.equ CMD_SDRAM_CKE_SET_HIGH,	0x00000007
