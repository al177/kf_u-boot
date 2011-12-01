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
#include <config.h>
#ifdef CONFIG_TWL6030
#include <bq27541.h>
#include <twl6030.h>
#include <asm/io.h>
/* Functions to read and write from TWL6030 */
static inline int twl6030_i2c_write_u8(u8 chip_no, u8 val, u8 reg)
{
    int mbid=0;
    mbid=get_mbid();
    if(mbid>=4)
        select_bus(0,100);
	return i2c_write(chip_no, reg, 1, &val, 1);
}

static inline int twl6030_i2c_read_u8(u8 chip_no, u8 *val, u8 reg)
{
    int mbid=0;
    mbid=get_mbid();
    if(mbid>=4)
        select_bus(0,100);
	return i2c_read(chip_no, reg, 1, val, 1);
}

void twl6030_start_usb_charging(void)
{
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CHARGERUSB_VICHRG_500,
							CHARGERUSB_VICHRG);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CHARGERUSB_CIN_LIMIT_500,
							CHARGERUSB_CINLIMIT);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, MBAT_TEMP,
							CONTROLLER_INT_MASK);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, MASK_MCHARGERUSB_THMREG,
							CHARGERUSB_INT_MASK);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CHARGERUSB_VOREG_4P76,
							CHARGERUSB_VOREG);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CHARGERUSB_CTRL2_VITERM_100,
							CHARGERUSB_CTRL2);
	/* Enable USB charging */
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CONTROLLER_CTRL1_EN_CHARGER,
							CONTROLLER_CTRL1);

	return;
}
void twl6030_reset_wd()
{
    get_bat_voltage();
    get_bat_current();
    twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, 0xa0,CONTROLLER_WDG);
}
void twl6030_shutdown()
{
    twl6030_i2c_write_u8(TWL6030_CHIP_PM,7,TWL6030_PHONIX_DEV_ON);
}

int twl6030_get_vbus_status()
{   
    u8   data;
    twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, &data, CONTROLLER_STAT1);//49,
    //printf("VBUS=%d 0x%x\n",((data&0x4) >>2),data);
    return ((data&0x4) >>2);
}
void twl6030_init_battery_charging(void)
{
    printf("twl6030_init_battery_charging\n");
    get_bat_voltage();
    get_bat_current();
	twl6030_start_usb_charging();
	return;
}
void twl6030_init_vusb()
{
    u8   data;
    /* Select APP Group and set state to ON */
    twl6030_i2c_read_u8(TWL6030_CHIP_PM, &data, MISC2);
    data |= 0x10;
    /* Select the input supply for VBUS regulator */
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, data, MISC2);

    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x18,VUSB_CFG_VOLTAGE);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_TRANS);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x01,VUSB_CFG_STATE);
}
void twl6030_disable_vusb()
{
    u8   data;
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x18,VUSB_CFG_VOLTAGE);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_TRANS);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_STATE);

    /* Select APP Group and set state to ON */
    twl6030_i2c_read_u8(TWL6030_CHIP_PM, &data, MISC2);
    data &= ~0x18;
    /* Select the input supply for VBUS regulator */
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, data, MISC2);
}
void twl6030_kc1_settings()
{
    u8 data = 0;

    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,VMEM_CFG_GRP);
    //printf("VMEM_CFG_GRP=0x%x \n",data);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VMEM_CFG_GRP);
    //rtc off mode low power,BBSPOR_CFG,VRTC_EN_OFF_STS
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x72,0xe6);

    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0X0,VMEM_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VMEM_CFG_STATE);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VMEM_CFG_TRANS);

    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0xC0, PHOENIX_MSK_TRANSITION);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x01, VCXIO_CFG_TRANS);

    //printf("VMEM_CFG_GRP=0x%x \n",data);
    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,VMEM_CFG_STATE);
    //printf("VMEM_CFG_STATE=0x%x \n",data);

    //twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_GRP);
    //twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_TRANS);
    //twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSB_CFG_STATE);

    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VDAC_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VDAC_CFG_TRANS);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VDAC_CFG_STATE);
    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,VDAC_CFG_GRP);
    //printf("VDAC_CFG_GRP=0x%x \n",data);
    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,VDAC_CFG_STATE);
    //printf("VDAC_CFG_STATE=0x%x \n",data);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VMMC_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VMMC_CFG_TRANS);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VMMC_CFG_STATE);



    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VAUX3_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VAUX3_CFG_STATE);

    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VAUX2_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VAUX2_CFG_STATE);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x9,VAUX2_CFG_VOLTAGE);

    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,0xec);
    //printf("SIMCTRL=0x%x\n",data);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM,0x10,0xec);
    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,0xec);
    //printf("SIMCTRL=0x%x\n",data);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSIM_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,VUSIM_CFG_STATE);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x14,VUSIM_CFG_VOLTAGE );
    //twl6030_i2c_read_u8(TWL6030_CHIP_PM,&data,0xee);
    //printf("MMCCTRL=0x%x\n",data);
    /*For Wifi test*/
    //For test,It should move to wifi driver to control power.
    //V2V1_CFG_VOLTAGE
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x3c,V2V1_CFG_VOLTAGE);
    //32k
    //twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,CLK32KG_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x1,CLK32KG_CFG_STATE);
}
void tw6030_kc1_clk32kg()
{
    //32k
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x0,CLK32KG_CFG_GRP);
    twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x1,CLK32KG_CFG_STATE);
}
void twl6030_usb_device_settings()
{
    u8 data = 0;
    twl6030_init_vusb();
#if 0
	/* Select APP Group and set state to ON */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, 0x21, VUSB_CFG_STATE);

	twl6030_i2c_read_u8(TWL6030_CHIP_PM, &data, MISC2);
	data |= 0x10;

	/* Select the input supply for VBUS regulator */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, data, MISC2);
#endif
}

#define PHOENIX_START_CONDITION		0x1F
#define PHOENIX_MSK_TRANSITION 		0x20
#define PHOENIX_STS_HW_CONDITIONS 	0x21
#define PHOENIX_LAST_TURNOFF_STS 	0x22
int twl6030_get_power_button_status()
{   
    volatile u8   data;
    twl6030_i2c_read_u8 (TWL6030_CHIP_PM, &data, PHOENIX_STS_HW_CONDITIONS);
    return (data&0x1);
}
void twl6030_print_boot_reason ()
{
	volatile u8 data1, data2, data3;
	volatile u32 data4 = 0;
	data1 = 0;
	data2 = 0;
	data3 = 0;
	printf ("PMIC TWL 6030 start conditions\n");
	twl6030_i2c_read_u8 (TWL6030_CHIP_PM, &data1, PHOENIX_START_CONDITION);
	twl6030_i2c_read_u8 (TWL6030_CHIP_PM, &data2, PHOENIX_LAST_TURNOFF_STS);
	twl6030_i2c_read_u8 (TWL6030_CHIP_PM, &data3, PHOENIX_STS_HW_CONDITIONS);
	printf ("PHOENIX_START_CONDITION  :0x%02x\n", data1);
	printf ("PHOENIX_LAST_TURNOFF_STS :0x%02x\n", data2);
	printf ("PHOENIX_STS_HW_CONDITIONS:0x%02x\n", data3);
	data4 = __raw_readl(PRM_RSTST);
	/* PRM_RSTST 10:31 are reserved. Mask them off */
	data4 = (0x3ff & data4);
	printf ("OMAP4 PRM_RSTST          :0x%08x\n", data4);
	return;
}

#endif
