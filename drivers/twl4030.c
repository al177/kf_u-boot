/*
 * Copyright (C) 2007-2008 Texas Instruments, Inc.
 *
 * USB
 * Imported from omap3-dev drivers/usb/twl4030_usb.c
 * This is unique part of the copyright
 *
 * twl4030_usb - TWL4030 USB transceiver, talking to OMAP OTG controller
 *
 * (C) Copyright 2009 Atin Malaviya (atin.malaviya@gmail.com)
 *
 * Based on: twl4030_usb.c in linux 2.6 (drivers/i2c/chips/twl4030_usb.c)
 * Copyright (C) 2004-2007 Texas Instruments
 * Copyright (C) 2008 Nokia Corporation
 * Contact: Felipe Balbi <felipe.balbi@nokia.com>
 *
 * Author: Atin Malaviya (atin.malaviya@gmail.com)
 *
 *
 * Keypad
 *
 * (C) Copyright 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
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
#include <config.h>
#ifdef CONFIG_TWL4030

#include <twl4030.h>
#include <asm/arch/led.h>

/*
 * Battery
 */
#define mdelay(n) ({ unsigned long msec = (n); while (msec--) udelay(1000); })

/* Functions to read and write from TWL4030 */
static inline int twl4030_i2c_write_u8(u8 chip_no, u8 val, u8 reg)
{
	return i2c_write(chip_no, reg, 1, &val, 1);
}

static inline int twl4030_i2c_read_u8(u8 chip_no, u8 *val, u8 reg)
{
	return i2c_read(chip_no, reg, 1, val, 1);
}

/*
 * Sets and clears bits on an given register on a given module
 */
static inline int clear_n_set(u8 chip_no, u8 clear, u8 set, u8 reg)
{
	int ret;
	u8 val = 0;

	/* Gets the initial register value */
	ret = twl4030_i2c_read_u8(chip_no, &val, reg);
	if (ret) {
		printf("a\n");
		return ret;
	}

	/* Clearing all those bits to clear */
	val &= ~(clear);

	/* Setting all those bits to set */
	val |= set;

	/* Update the register */
	ret = twl4030_i2c_write_u8(chip_no, val, reg);
	if (ret) {
		printf("b\n");
		return ret;
	}
	return 0;
}

/*
 * Disable/Enable AC Charge funtionality.
 */
static int twl4030_ac_charger_enable(int enable)
{
	int ret;

	if (enable) {
		/* forcing the field BCIAUTOAC (BOOT_BCI[0]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, 0,
			       (CONFIG_DONE | BCIAUTOWEN | BCIAUTOAC),
			       REG_BOOT_BCI);
		if (ret)
			return ret;
	} else {
		/* forcing the field BCIAUTOAC (BOOT_BCI[0]) to 0 */
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, BCIAUTOAC,
			       (CONFIG_DONE | BCIAUTOWEN),
			       REG_BOOT_BCI);
		if (ret)
			return ret;
	}
	return 0;
}

/*
 * Disable/Enable USB Charge funtionality.
 */
static int twl4030_usb_charger_enable(int enable)
{
	u8 value;
	int ret;

	if (enable) {
		/* enable access to BCIIREF1 */
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
			REG_BCIMFKEY);
		if (ret)
			return ret;

		/* set charging current = 852mA */
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xFF,
			REG_BCIIREF1);
		if (ret)
			return ret;

		/* forcing the field BCIAUTOUSB (BOOT_BCI[1]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, 0,
			(CONFIG_DONE | BCIAUTOWEN | BCIAUTOUSB),
			REG_BOOT_BCI);
		if (ret)
			return ret;

		/* Enabling interfacing with usb thru OCP */
		ret = clear_n_set(TWL4030_CHIP_USB, 0, PHY_DPLL_CLK,
			REG_PHY_CLK_CTRL);
		if (ret)
			return ret;

		value = 0;

		while (!(value & PHY_DPLL_CLK)) {
			udelay(10);
			ret = twl4030_i2c_read_u8(TWL4030_CHIP_USB, &value,
				REG_PHY_CLK_CTRL_STS);
			if (ret)
				return ret;
		}

		/* OTG_EN (POWER_CTRL[5]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_USB, 0, OTG_EN,
			REG_POWER_CTRL);
		if (ret)
			return ret;

		mdelay(50);

		/* forcing USBFASTMCHG(BCIMFSTS4[2]) to 1 */
		ret = clear_n_set(TWL4030_CHIP_MAIN_CHARGE, 0,
			USBFASTMCHG, REG_BCIMFSTS4);
		if (ret)
			return ret;
	} else {
		ret = clear_n_set(TWL4030_CHIP_PM_MASTER, BCIAUTOUSB,
			(CONFIG_DONE | BCIAUTOWEN), REG_BOOT_BCI);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Setup the twl4030 MADC module to measure the backup
 * battery voltage.
 */
static int twl4030_madc_setup(void)
{
	int ret = 0;

	/* turning MADC clocks on */
	ret = clear_n_set(TWL4030_CHIP_INTBR, 0,
		(MADC_HFCLK_EN | DEFAULT_MADC_CLK_EN), REG_GPBR1);
	if (ret)
		return ret;

	/* turning adc_on */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MADC, MADC_ON,
		REG_CTRL1);
	if (ret)
		return ret;

	/* setting MDC channel 9 to trigger by SW1 */
	ret = clear_n_set(TWL4030_CHIP_MADC, 0, SW1_CH9_SEL,
		REG_SW1SELECT_MSB);

	return ret;
}

/*
 * Charge backup battery through main battery
 */
static int twl4030_charge_backup_battery(void)
{
	int ret;

	ret = clear_n_set(TWL4030_CHIP_PM_RECEIVER, 0xff,
			  (BBCHEN | BBSEL_3200mV | BBISEL_150uA), REG_BB_CFG);
	if (ret)
		return ret;

	return 0;
}

/*
 * Helper function to read a 2-byte register on BCI module
 */
static int read_bci_val(u8 reg)
{
	int ret = 0, temp = 0;
	u8 val;

	/* reading MSB */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, reg + 1);
	if (ret)
		return ret;

	temp = ((int)(val & 0x03)) << 8;

	/* reading LSB */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &val, reg);
	if (ret)
		return ret;

	return temp + val;
}

/*
 * Triggers the sw1 request for the twl4030 module to measure the sw1 selected
 * channels
 */
static int twl4030_madc_sw1_trigger(void)
{
	u8 val;
	int ret;

	/* Triggering SW1 MADC convertion */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val, REG_CTRL_SW1);
	if (ret)
		return ret;

	val |= SW1_TRIGGER;

	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MADC, val, REG_CTRL_SW1);
	if (ret)
		return ret;

	/* Waiting until the SW1 conversion ends*/
	val =  BUSY;

	while (!((val & EOC_SW1) && (!(val & BUSY)))) {
		ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &val,
					  REG_CTRL_SW1);
		if (ret)
			return ret;
		mdelay(10);
	}

	return 0;
}

/*
 * Return battery voltage
 */
static int twl4030_get_battery_voltage(void)
{
	int volt;

	volt = read_bci_val(T2_BATTERY_VOLT);
	return (volt * 588) / 100;
}

/*
 * Return the battery backup voltage
 */
static int twl4030_get_backup_battery_voltage(void)
{
	int ret, temp;
	u8 volt;

	/* trigger MADC convertion */
	twl4030_madc_sw1_trigger();

	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &volt, REG_GPCH9 + 1);
	if (ret)
		return ret;

	temp = ((int) volt) << 2;

	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MADC, &volt, REG_GPCH9);
	if (ret)
		return ret;

	temp = temp + ((int) ((volt & MADC_LSB_MASK) >> 6));

	return  (temp * 441) / 100;
}

#if 0 /* Maybe used in future */
/*
 * Return the AC power supply voltage
 */
static int twl4030_get_ac_charger_voltage(void)
{
	int volt = read_bci_val(T2_BATTERY_ACVOLT);
	return (volt * 735) / 100;
}

/*
 * Return the USB power supply voltage
 */
static int twl4030_get_usb_charger_voltage(void)
{
	int volt = read_bci_val(T2_BATTERY_USBVOLT);
	return (volt * 2058) / 300;
}
#endif

/*
 * Battery charging main function called from board-specific file
 */

int twl4030_init_battery_charging(void)
{
	u8 batstsmchg, batstspchg, hwsts, reg;
	int battery_volt = 0, charger_present = 0;
	int ret = 0;

#ifdef CONFIG_3430ZOOM2
	/* For Zoom2 enable Main charge Automatic mode:
	 * by enabling MADC clocks
	 */

	/* Red LED - on  */
	omap3_zoom2_led_red_on();

	/* Disable USB, enable AC: 0x35 defalut */
	ret = clear_n_set(TWL4030_CHIP_PM_MASTER, BCIAUTOUSB,
			       BCIAUTOAC,
			       REG_BOOT_BCI);

	/* Enable AC charging : ROM code is shutting down MADC CLK */
	ret = clear_n_set(TWL4030_CHIP_INTBR, 0,
		(MADC_HFCLK_EN | DEFAULT_MADC_CLK_EN), REG_GPBR1);

	udelay(100);

	ret = clear_n_set(TWL4030_CHIP_PM_MASTER,
				BCIAUTOAC | CVENAC,
				0,
				REG_BOOT_BCI);

	/* Change charging current */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
			REG_BCIMFKEY);
	/* set 1 Amp charging */
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0x58,
			REG_BCIIREF1);
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0xE7,
			REG_BCIMFKEY);
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_MAIN_CHARGE, 0x03,
			REG_BCIIREF2);

	/* Set CGAIN=1 */
	ret = clear_n_set(TWL4030_CHIP_MAIN_CHARGE,
				0,
				CGAIN,
				REG_BCICTL1);

	ret = clear_n_set(TWL4030_CHIP_PM_MASTER,
				0,
				BCIAUTOAC | CVENAC,
				REG_BOOT_BCI);

	/* Red LED - off  */
	omap3_zoom2_led_red_off();


	/* Done for Zoom2 */
	return 0;
#endif

	/* check for battery presence */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_MAIN_CHARGE, &batstsmchg,
				  REG_BCIMFSTS3);
	if (ret)
		return ret;

	ret = twl4030_i2c_read_u8(TWL4030_CHIP_PRECHARGE, &batstspchg,
				  REG_BCIMFSTS1);
	if (ret)
		return ret;

	if (!((batstspchg & BATSTSPCHG) || (batstsmchg & BATSTSMCHG)))
		return ret;	/* no battery */

	ret = twl4030_madc_setup();
	if (ret) {
		printf("twl4030 madc setup error %d\n", ret);
		return ret;
	}
	/* backup battery charges through main battery */
	ret = twl4030_charge_backup_battery();
	if (ret) {
		printf("backup battery charging error\n");
		return ret;
	}
	/* check for charger presence */
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER, &hwsts,
				  REG_STS_HW_CONDITIONS);
	if (ret)
		return ret;

	if (hwsts & STS_CHG) {
		printf("AC charger detected\n");
		ret = twl4030_ac_charger_enable(1);
		if (ret)
			return ret;
		charger_present = 1;
	} else {
		if (hwsts & STS_VBUS) {
			printf("USB charger detected\n");
			charger_present = 1;
		}
		/* usb charging is enabled regardless of the whether the
		 * charger is attached, otherwise the main battery voltage
		 * cannot be read
		 */
		ret = twl4030_usb_charger_enable(1);
		if (ret)
			return ret;
	}
	battery_volt = twl4030_get_battery_voltage();
	printf("Battery levels: main %d mV, backup %d mV\n",
		battery_volt, twl4030_get_backup_battery_voltage());
	if (!charger_present && (battery_volt < 3300))
		printf("Main battery charge too low!\n");

	return ret;
}

#if (defined(CONFIG_TWL4030_KEYPAD) && (CONFIG_TWL4030_KEYPAD))
/*
 * Keypad
 */
int twl4030_keypad_init(void)
{
	int ret = 0;
	u8 ctrl;
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD, &ctrl, KEYPAD_KEYP_CTRL_REG);
	if (!ret) {
		ctrl |= KEYPAD_CTRL_KBD_ON | KEYPAD_CTRL_SOFT_NRST;
		ctrl &= ~KEYPAD_CTRL_SOFTMODEN;
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD, ctrl, KEYPAD_KEYP_CTRL_REG);
	}
	return ret;
}

int twl4030_keypad_reset(void)
{
	int ret = 0;
	u8 ctrl;
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD, &ctrl, KEYPAD_KEYP_CTRL_REG);
	if (!ret) {
		ctrl &= ~KEYPAD_CTRL_SOFT_NRST;
		ret = twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD, ctrl, KEYPAD_KEYP_CTRL_REG);
	}
	return ret;
}

int twl4030_keypad_keys_pressed(unsigned char *key1, unsigned char *key2)
{
	int ret = 0;
	u8 cb, c, rb, r;
	for (cb = 0; cb < 8; cb++) {
		c = 0xff & ~(1 << cb);
		twl4030_i2c_write_u8(TWL4030_CHIP_KEYPAD, c, KEYPAD_KBC_REG);
		twl4030_i2c_read_u8(TWL4030_CHIP_KEYPAD, &r, KEYPAD_KBR_REG);
		for (rb = 0; rb < 8; rb++) {
			if (!(r & (1 << rb))) {
				if (!ret)
					*key1 = cb << 3 | rb;
				else if (1 == ret)
					*key2 = cb << 3 | rb;
				ret++;
			}
		}
	}
	return ret;
}

#endif

/* USB */

#if (defined(CONFIG_TWL4030_USB) && (CONFIG_TWL4030_USB))

static int twl4030_usb_write(u8 address, u8 data)
{
	int ret = 0;
	ret = twl4030_i2c_write_u8(TWL4030_CHIP_USB, data, address);
	if (ret != 0)
		printf("TWL4030:USB:Write[0x%x] Error %d\n", address, ret);

	return ret;
}

static int twl4030_usb_read(u8 address)
{
	u8 data;
	int ret = 0;
	ret = twl4030_i2c_read_u8(TWL4030_CHIP_USB, &data, address);
	if (ret == 0)
		ret = data;
	else
		printf("TWL4030:USB:Read[0x%x] Error %d\n", address, ret);

	return ret;
}

static int twl4030_usb_set_bits(u8 reg, u8 bits)
{
	return twl4030_usb_write(reg + 1, bits);
}

static int twl4030_usb_clear_bits(u8 reg, u8 bits)
{
	return twl4030_usb_write(reg + 2, bits);
}

static void twl4030_usb_ldo_init(void)
{
	/* Enable writing to power configuration registers */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0xC0, PM_MASTER_PROTECT_KEY);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x0C, PM_MASTER_PROTECT_KEY);

	/* put VUSB3V1 LDO in active state */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00, PM_RECEIVER_VUSB_DEDICATED2);

	/* input to VUSB3V1 LDO is from VBAT, not VBUS */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x14, PM_RECEIVER_VUSB_DEDICATED1);

	/* turn on 3.1V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20, PM_RECEIVER_VUSB3V1_DEV_GRP);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00, PM_RECEIVER_VUSB3V1_TYPE);

	/* turn on 1.5V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20, PM_RECEIVER_VUSB1V5_DEV_GRP);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00, PM_RECEIVER_VUSB1V5_TYPE);

	/* turn on 1.8V regulator */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x20, PM_RECEIVER_VUSB1V8_DEV_GRP);
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_RECEIVER, 0x00, PM_RECEIVER_VUSB1V8_TYPE);

	/* disable access to power configuration registers */
	twl4030_i2c_write_u8(TWL4030_CHIP_PM_MASTER, 0x00, PM_MASTER_PROTECT_KEY);
}

static void twl4030_phy_power(void)
{
	u8 pwr;

	pwr = twl4030_usb_read(USB_PHY_PWR_CTRL);
	pwr &= ~USB_PHYPWD;
	twl4030_usb_write(USB_PHY_PWR_CTRL, pwr);
	twl4030_usb_write(USB_PHY_CLK_CTRL, twl4030_usb_read(USB_PHY_CLK_CTRL) |
				(USB_CLOCKGATING_EN | USB_CLK32K_EN));
}

int twl4030_usb_init(void)
{
	unsigned long timeout;

	/* twl4030 ldo init */
	twl4030_usb_ldo_init();

	/* Enable the twl4030 phy */
	twl4030_phy_power();

	/* enable DPLL to access PHY registers over I2C */
	twl4030_usb_write(USB_PHY_CLK_CTRL, twl4030_usb_read(USB_PHY_CLK_CTRL) |
			  USB_REQ_PHY_DPLL_CLK);
	timeout = 1000 * 1000; /* 1 sec */
	while (!(twl4030_usb_read(USB_PHY_CLK_CTRL_STS) & USB_PHY_DPLL_CLK) &&
		0 < timeout) {
		udelay(10);
		timeout -= 10;
	}
	if (!(twl4030_usb_read(USB_PHY_CLK_CTRL_STS) & USB_PHY_DPLL_CLK)) {
		printf("Timeout setting T2 HSUSB PHY DPLL clock\n");
		return -1;
	}

	/* Enable ULPI mode */
	twl4030_usb_clear_bits(USB_IFC_CTRL, USB_CARKITMODE);
	twl4030_usb_set_bits(USB_POWER_CTRL, USB_OTG_ENAB);
	twl4030_usb_clear_bits(USB_FUNC_CTRL, USB_XCVRSELECT_MASK | USB_OPMODE_MASK);
	/* let ULPI control the DPLL clock */
	twl4030_usb_write(USB_PHY_CLK_CTRL, twl4030_usb_read(USB_PHY_CLK_CTRL) &
				~USB_REQ_PHY_DPLL_CLK);
	return 0;
}

#endif

/*
 * Power Reset
 */
void twl4030_power_reset_init(void)
{
#ifdef CONFIG_3430ZOOM2
	clear_n_set(TWL4030_CHIP_PM_MASTER, 0, SW_EVENTS_STOPON_PWRON,
		    PM_MASTER_P1_SW_EVENTS);
#endif
}

#endif
