/*
 * (C) Copyright 2009
 * Texas Instruments, <www.ti.com>
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
#include <i2c.h>

/* I2C chip addresses */
#define TWL6030_CHIP_PM		0x48

#define TWL6030_CHIP_USB	0x49
#define TWL6030_CHIP_ADC	0x49
#define TWL6030_CHIP_CHARGER	0x49
#define TWL6030_CHIP_PWM	0x49

/* Battery CHARGER REGISTERS */
#define CONTROLLER_INT_MASK	0xE0
#define CONTROLLER_CTRL1	0xE1
#define CONTROLLER_WDG		0xE2
#define CONTROLLER_STAT1	0xE3
#define CHARGERUSB_INT_STATUS	0xE4
#define CHARGERUSB_INT_MASK	0xE5
#define CHARGERUSB_STATUS_INT1	0xE6
#define CHARGERUSB_STATUS_INT2	0xE7
#define CHARGERUSB_CTRL1	0xE8
#define CHARGERUSB_CTRL2	0xE9
#define CHARGERUSB_CTRL3	0xEA
#define CHARGERUSB_STAT1	0xEB
#define CHARGERUSB_VOREG	0xEC
#define CHARGERUSB_VICHRG	0xED
#define CHARGERUSB_CINLIMIT	0xEE
#define CHARGERUSB_CTRLLIMIT1	0xEF

#define TWL6030_PHONIX_DEV_ON 0x25

#define REG_START_CONDITION 0x1F
/* START_CONDITION_MASK*/
#define STRT_ON_PLUG_DET      (1 << 3)
#define STRT_ON_USB_ID        (1 << 2)
#define STRT_ON_PWRON         (1)

#define REG_HW_CONDITION 0x21
#define STS_PLUG_DET          (1 << 3)
#define STS_USB_ID            (1 << 2)
#define STS_PWRON             (1)

/* CHARGERUSB_VICHRG */
#define CHARGERUSB_VICHRG_500		0x4
#define CHARGERUSB_VICHRG_1500		0xE
/* CHARGERUSB_CINLIMIT */
#define CHARGERUSB_CIN_LIMIT_100	0x1
#define CHARGERUSB_CIN_LIMIT_300	0x5
#define CHARGERUSB_CIN_LIMIT_500	0x9
#define CHARGERUSB_CIN_LIMIT_NONE	0xF
/* CONTROLLER_INT_MASK */
#define MVAC_FAULT		(1 << 6)
#define MAC_EOC			(1 << 5)
#define MBAT_REMOVED		(1 << 4)
#define MFAULT_WDG		(1 << 3)
#define MBAT_TEMP		(1 << 2)
#define MVBUS_DET		(1 << 1)
#define MVAC_DET		(1 << 0)
/* CHARGERUSB_INT_MASK */
#define MASK_MCURRENT_TERM		(1 << 3)
#define MASK_MCHARGERUSB_STAT		(1 << 2)
#define MASK_MCHARGERUSB_THMREG		(1 << 1)
#define MASK_MCHARGERUSB_FAULT		(1 << 0)
/* CHARGERUSB_VOREG */
#define CHARGERUSB_VOREG_3P52		0x01
#define CHARGERUSB_VOREG_4P0		0x19
#define CHARGERUSB_VOREG_4P2		0x23
#define CHARGERUSB_VOREG_4P76		0x3F
/* CHARGERUSB_CTRL2 */
#define CHARGERUSB_CTRL2_VITERM_50	(0 << 5)
#define CHARGERUSB_CTRL2_VITERM_100	(1 << 5)
#define CHARGERUSB_CTRL2_VITERM_150	(2 << 5)
/* CONTROLLER_CTRL1 */
#define CONTROLLER_CTRL1_EN_CHARGER	(1 << 4)
#define CONTROLLER_CTRL1_SEL_CHARGER	(1 << 3)

#define VUSB_CFG_STATE		0xA2
#define MISC2			0xE5

#define PHOENIX_MSK_TRANSITION 0x20
#define PHOENIX_SENS_TRANSITION 0x2A

#define VMEM_CFG_GRP 0x64
#define VMEM_CFG_TRANS 0x65
#define VMEM_CFG_STATE 0x66
#define VMEM_CFG_VOLTAGE 0x68

#define V1V29_CFG_GRP 0x40
#define V1V29_CFG_TRANS 0x41
#define V1V29_CFG_STATE 0x42
#define V1V29_CFG_VOLTAGE 0x44

#define V2V1_CFG_GRP 0x4c
#define V2V1_CFG_TRANS 0x4d
#define V2V1_CFG_STATE 0x4e
#define V2V1_CFG_VOLTAGE 0x50

#define VMEM_CFG_GRP 0x64
#define VMEM_CFG_TRANS 0x65
#define VMEM_CFG_STATE 0x66
#define VMEM_CFG_VOLTAGE 0x68

#define VAUX1_CFG_GRP 0x84
#define VAUX1_CFG_TRANS 0x85
#define VAUX1_CFG_STATE 0x86
#define VAUX1_CFG_VOLTAGE 0x87

#define VUSB_CFG_GRP 0xa0
#define VUSB_CFG_TRANS 0xa1
#define VUSB_CFG_STATE 0xa2
#define VUSB_CFG_VOLTAGE 0xa3

#define VAUX2_CFG_GRP 0x88
#define VAUX2_CFG_TRANS 0x89
#define VAUX2_CFG_STATE 0x8a
#define VAUX2_CFG_VOLTAGE 0x8b

#define VAUX3_CFG_GRP 0x8c
#define VAUX3_CFG_TRANS 0X8D
#define VAUX3_CFG_STATE 0x8E
#define VAUX3_CFG_VOLTAGE 0x8f

#define VCXIO_CFG_GRP 0x90
#define VCXIO_CFG_TRANS 0x91
#define VCXIO_CFG_STATE 0x92
#define VCXIO_CFG_VOLTAGE 0x93

#define VDAC_CFG_GRP 0x94
#define VDAC_CFG_TRANS 0x95
#define VDAC_CFG_STATE 0x96
#define VDAC_CFG_VOLTAGE 0x97

#define VMMC_CFG_GRP 0x98
#define VMMC_CFG_TRANS 0x99
#define VMMC_CFG_STATE 0x9A
#define VMMC_CFG_VOLTAGE 0x9B

#define VUSIM_CFG_GRP 0xa4
#define VUSIM_CFG_TRANS 0xa5
#define VUSIM_CFG_STATE 0xa6
#define VUSIM_CFG_VOLTAGE 0xa7

#define CLK32KG_CFG_GRP 0xbc
#define CLK32KG_CFG_TRANS 0xbd
#define CLK32KG_CFG_STATE 0xbe

#define BBSPOR_CFG 0xE6

void twl6030_disable_vusb();
int twl6030_get_vbus_status();
void twl6030_init_battery_charging(void);
void twl6030_usb_device_settings(void);
void twl6030_shutdown();
