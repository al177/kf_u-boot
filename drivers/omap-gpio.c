/*
 * OMAP4 basic GPIO functions
 *
 * Copyright (c) 2011 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/omap4430.h>
#include <omap-gpio.h>

#define OMAP_GPIO_OE_OFFSET         0x134
#define OMAP_GPIO_IN_OFFSET         0x138
#define OMAP_GPIO_OUT_OFFSET        0x13C
#define OMAP_GPIO_OUT_CLEAR_OFFSET  0x190
#define OMAP_GPIO_OUT_SET_OFFSET    0x194

#define OMAP_GPIO_PER_MODULE 32

struct omap_gpio_module {
	int module;
	int address_oe;
	int address_set_out;
	int address_clear_out;
	int address_in;
	int address_out;
};

const unsigned int gpio_addresses[6] = {
	OMAP44XX_GPIO1_BASE,
	OMAP44XX_GPIO2_BASE,
	OMAP44XX_GPIO3_BASE,
	OMAP44XX_GPIO4_BASE,
	OMAP44XX_GPIO5_BASE,
	OMAP44XX_GPIO6_BASE};

static int get_gpio_module(struct omap_gpio_module *gpio, int gpio_num)
{
	int gpio_module;

	if (gpio_num < GPIO_NUM_LOW || gpio_num > GPIO_NUM_HIGH) {
		return -1;
	}

	gpio_module             = gpio->module = gpio_num / OMAP_GPIO_PER_MODULE;
	gpio->address_oe        = gpio_addresses[gpio_module] + OMAP_GPIO_OE_OFFSET;
	gpio->address_set_out   = gpio_addresses[gpio_module] + OMAP_GPIO_OUT_SET_OFFSET;
	gpio->address_clear_out = gpio_addresses[gpio_module] + OMAP_GPIO_OUT_CLEAR_OFFSET;
	gpio->address_in        = gpio_addresses[gpio_module] + OMAP_GPIO_IN_OFFSET;
	gpio->address_out       = gpio_addresses[gpio_module] + OMAP_GPIO_OUT_OFFSET;

	return 0;
}

int omap_gpio_set_output(int gpio_num, int value)
{
	struct omap_gpio_module gpio;
	int oe, gpio_offset;

	if ((value != GPIO_LOW) && (value != GPIO_HIGH)) {
		return -1;
	}

	if (get_gpio_module(&gpio, gpio_num)) {
		return -1;
	}

	gpio_offset = gpio_num - (gpio.module * OMAP_GPIO_PER_MODULE);
	if (value == GPIO_LOW) {
		__raw_writel(1 << gpio_offset, gpio.address_clear_out);
	} else {
		__raw_writel(1 << gpio_offset, gpio.address_set_out);
	}
	oe = __raw_readl(gpio.address_oe);
	oe &= ~(1 << gpio_offset);
	__raw_writel(oe, gpio.address_oe);

	return 0;
}

int omap_gpio_set_direction_input(int gpio_num)
{
	struct omap_gpio_module gpio;
	int oe, gpio_offset;

	if (get_gpio_module(&gpio, gpio_num)) {
		return -1;
	}

	gpio_offset = gpio_num - (gpio.module * OMAP_GPIO_PER_MODULE);
	oe = __raw_readl(gpio.address_oe);
	oe |= (1 << gpio_offset);
	__raw_writel(oe, gpio.address_oe);

	return 0;
}

int omap_gpio_get_value(int gpio_num)
{
	struct omap_gpio_module gpio;
	int gpio_offset, value;

	if (get_gpio_module(&gpio, gpio_num)) {
		return -1;
	}

	gpio_offset = gpio_num - (gpio.module * OMAP_GPIO_PER_MODULE);
	value = __raw_readl(gpio.address_oe);
	if (value & (1 << gpio_offset)) { /* GPIO is input. Read input register */
		value = __raw_readl(gpio.address_in);
	} else { /* GPIO is output. Read the level we force */
		value = __raw_readl(gpio.address_out);
	}
	return (value & (1 << gpio_offset)) >> gpio_offset;
}

