/*
 * (C) Copyright 2005
 * Texas Instruments, <www.ti.com>
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
 *
 */

#ifndef _OMAP24XX_CPU_H
#define  _OMAP24XX_CPU_H
/* CPU Specific Headers */
#ifdef CONFIG_OMAP242X
#include <asm/arch/omap2420.h>
#endif
#ifdef CONFIG_OMAP243X
#include <asm/arch/omap2430.h>
#endif

/* Register offsets of common modules */
/* Control */
#define CONTROL_STATUS        (OMAP24XX_CTRL_BASE + 0x2F8)
#define OMAP24XX_MCR          (OMAP24XX_CTRL_BASE + 0x8C)

/* Tap Information */
#define TAP_IDCODE_REG        (OMAP24XX_TAP_BASE+0x204)
#define PRODUCTION_ID         (OMAP24XX_TAP_BASE+0x208)

/* device type */
#define DEVICE_MASK          (BIT8|BIT9|BIT10)
#define TST_DEVICE	     0x0
#define EMU_DEVICE	     0x1
#define HS_DEVICE	     0x2
#define GP_DEVICE	     0x3

/* GPMC CS3/cs4/cs6 not avaliable */
#define GPMC_SYSCONFIG        (OMAP24XX_GPMC_BASE+0x10)
#define GPMC_IRQENABLE        (OMAP24XX_GPMC_BASE+0x1C)
#define GPMC_TIMEOUT_CONTROL  (OMAP24XX_GPMC_BASE+0x40)
#define GPMC_CONFIG           (OMAP24XX_GPMC_BASE+0x50)

#define GPMC_CONFIG_CS0       (OMAP24XX_GPMC_BASE+0x60)
#define GPMC_CONFIG_WIDTH     (0x30)

#define GPMC_CONFIG1          (0x00)
#define GPMC_CONFIG2          (0x04)
#define GPMC_CONFIG3          (0x08)
#define GPMC_CONFIG4          (0x0C)
#define GPMC_CONFIG5          (0x10)
#define GPMC_CONFIG6          (0x14)
#define GPMC_CONFIG7          (0x18)
#define GPMC_NAND_CMD         (0x1C)
#define GPMC_NAND_ADR         (0x20)
#define GPMC_NAND_DAT         (0x24)

/* GPMC Mapping */
# define FLASH_BASE           0x04000000  /* NOR flash (64 Meg aligned) */
# define DEBUG_BASE           0x08000000  /* debug board */
# define NAND_BASE            0x0C000000  /* NAND flash */
# define SIBLEY_MAP1          0x10000000  /* Sibley1 flash */
# define SIBLEY_MAP2          0x14000000  /* Sibley2 flash */
# define PCMCIA_BASE          0x18000000  /* PCMCIA region */
# define ONENAND_MAP          0x20000000  /* OneNand flash */

/* SMS */
#define SMS_SYSCONFIG     (OMAP24XX_SMS_BASE+0x10)
#define SMS_CLASS_ARB0    (OMAP24XX_SMS_BASE+0xD0)
#define BURSTCOMPLETE_GROUP7    BIT31

/* SDRC */
#define SDRC_SYSCONFIG     (OMAP24XX_SDRC_BASE+0x10)
#define SDRC_STATUS        (OMAP24XX_SDRC_BASE+0x14)
#define SDRC_CS_CFG        (OMAP24XX_SDRC_BASE+0x40)
#define SDRC_SHARING       (OMAP24XX_SDRC_BASE+0x44)
#define SDRC_DLLA_CTRL     (OMAP24XX_SDRC_BASE+0x60)
#define SDRC_DLLA_STATUS   (OMAP24XX_SDRC_BASE+0x64)
#define SDRC_DLLB_CTRL     (OMAP24XX_SDRC_BASE+0x68)
#define SDRC_DLLB_STATUS   (OMAP24XX_SDRC_BASE+0x6C)
#define DLLPHASE           BIT1
#define LOADDLL            BIT2
#define DLL_DELAY_MASK     0xFF00
#define DLL_NO_FILTER_MASK (BIT8|BIT9)

#define SDRC_POWER         (OMAP24XX_SDRC_BASE+0x70)
#define SDRC_MCFG_0        (OMAP24XX_SDRC_BASE+0x80)
#define SDRC_MR_0          (OMAP24XX_SDRC_BASE+0x84)
#define SDRC_ACTIM_CTRLA_0 (OMAP24XX_SDRC_BASE+0x9C)
#define SDRC_ACTIM_CTRLB_0 (OMAP24XX_SDRC_BASE+0xA0)
#define SDRC_ACTIM_CTRLA_1 (OMAP24XX_SDRC_BASE+0xC4)
#define SDRC_ACTIM_CTRLB_1 (OMAP24XX_SDRC_BASE+0xC8)
#define SDRC_RFR_CTRL      (OMAP24XX_SDRC_BASE+0xA4)
#define SDRC_MANUAL_0      (OMAP24XX_SDRC_BASE+0xA8)
#define OMAP24XX_SDRC_CS0  0x80000000
#define OMAP24XX_SDRC_CS1  0xA0000000
#define CMD_NOP            0x0
#define CMD_PRECHARGE      0x1
#define CMD_AUTOREFRESH    0x2
#define CMD_ENTR_PWRDOWN   0x3
#define CMD_EXIT_PWRDOWN   0x4
#define CMD_ENTR_SRFRSH    0x5
#define CMD_CKE_HIGH       0x6
#define CMD_CKE_LOW        0x7
#define SOFTRESET          BIT1
#define SMART_IDLE         (0x2 << 3)
#define REF_ON_IDLE        (0x1 << 6)

/* timer regs offsets (32 bit regs) */
#define TIDR       0x0      /* r */
#define TIOCP_CFG  0x10     /* rw */
#define TISTAT     0x14     /* r */
#define TISR       0x18     /* rw */
#define TIER       0x1C     /* rw */
#define TWER       0x20     /* rw */
#define TCLR       0x24     /* rw */
#define TCRR       0x28     /* rw */
#define TLDR       0x2C     /* rw */
#define TTGR       0x30     /* rw */
#define TWPS       0x34     /* r */
#define TMAR       0x38     /* rw */
#define TCAR1      0x3c     /* r */
#define TSICR      0x40     /* rw */
#define TCAR2      0x44     /* r */

/* Watchdog */
#define WWPS       0x34     /* r */
#define WSPR       0x48     /* rw */
#define WD_UNLOCK1 0xAAAA
#define WD_UNLOCK2 0x5555

/* PRCM */
#define PRCM_CLKSRC_CTRL (OMAP24XX_CM_BASE+0x060)
#define PRCM_CLKOUT_CTRL (OMAP24XX_CM_BASE+0x070)
#define PRCM_CLKEMUL_CTRL (OMAP24XX_CM_BASE+0x078)
#define PRCM_CLKCFG_CTRL (OMAP24XX_CM_BASE+0x080)
#define PRCM_CLKCFG_STATUS (OMAP24XX_CM_BASE+0x084)
#define CM_CLKSEL_MPU    (OMAP24XX_CM_BASE+0x140)
#define RM_RSTST_MPU     (OMAP24XX_CM_BASE+0x158)
#define CM_FCLKEN1_CORE  (OMAP24XX_CM_BASE+0x200)
#define CM_FCLKEN2_CORE  (OMAP24XX_CM_BASE+0x204)
#define CM_ICLKEN1_CORE  (OMAP24XX_CM_BASE+0x210)
#define CM_ICLKEN2_CORE  (OMAP24XX_CM_BASE+0x214)
#define CM_CLKSEL1_CORE  (OMAP24XX_CM_BASE+0x240)
#define CM_CLKSEL_WKUP   (OMAP24XX_CM_BASE+0x440)
#define CM_CLKSEL2_CORE  (OMAP24XX_CM_BASE+0x244)
#define CM_FCLKEN_GFX    (OMAP24XX_CM_BASE+0x300)
#define CM_ICLKEN_GFX    (OMAP24XX_CM_BASE+0x310)
#define CM_CLKSEL_GFX    (OMAP24XX_CM_BASE+0x340)
#define RM_RSTCTRL_GFX    (OMAP24XX_CM_BASE+0x350)
#define CM_FCLKEN_WKUP    (OMAP24XX_CM_BASE+0x400)
#define CM_ICLKEN_WKUP    (OMAP24XX_CM_BASE+0x410)
#define CM_CLKSEL_WKUP   (OMAP24XX_CM_BASE+0x440)
#define PM_RSTCTRL_WKUP  (OMAP24XX_CM_BASE+0x450)
#define CM_CLKEN_PLL     (OMAP24XX_CM_BASE+0x500)
#define CM_IDLEST_CKGEN  (OMAP24XX_CM_BASE+0x520)
#define CM_CLKSEL1_PLL   (OMAP24XX_CM_BASE+0x540)
#define CM_CLKSEL2_PLL   (OMAP24XX_CM_BASE+0x544)
#define CM_CLKSEL_DSP    (OMAP24XX_CM_BASE+0x840)
#define CM_CLKSEL_MDM    (OMAP24XX_CM_BASE+0xC40)

/* SMX-APE */
#define PM_RT_APE_BASE_ADDR_ARM  (SMX_APE_BASE + 0x10000)
#define PM_GPMC_BASE_ADDR_ARM    (SMX_APE_BASE + 0x12400)
#define PM_OCM_RAM_BASE_ADDR_ARM (SMX_APE_BASE + 0x12800)
#define PM_OCM_ROM_BASE_ADDR_ARM (SMX_APE_BASE + 0x12C00)

/* IVA2 */
#define PM_IVA2_BASE_ADDR_ARM    (SMX_APE_BASE + 0x14000)

/* I2C base */
#define I2C_BASE1               (OMAP24XX_L4_IO_BASE + 0x70000) 
#define I2C_BASE2               (OMAP24XX_L4_IO_BASE + 0x72000) 

#endif
