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
#include <command.h>
#include "omap-gpio.h"

int do_gpio (cmd_tbl_t *cmdtp, int flag, int argc, char **argv)
{
	int gpio_value, gpio_num;

	if ((argc < 2) || (argc > 3)) {
		goto gpio_cmd_usage;
	} else {
		gpio_num = simple_strtoul(argv[1], NULL, 10);
		if (argc == 2) {
			gpio_value = omap_gpio_get_value(gpio_num);
			if (gpio_value == -1) {
				printf("Error getting value for gpio %d\n", gpio_num);
				return -1;
			}
			printf ("%d\n", gpio_value);
			return 0;
		}
		if (argc == 3) {
			gpio_value = simple_strtoul(argv[2], NULL, 10);
			if (omap_gpio_set_output(gpio_num, gpio_value)) {
				printf("Error setting gpio %d to %d\n", gpio_num, gpio_value);
				return -1;
			}
			return 0;
		}
	}
gpio_cmd_usage:
	printf("gpio - sets, resets or reads a GPIO\n");
	return 0;
}

U_BOOT_CMD(gpio, 3, 1, do_gpio,
	"gpio    - sets, resets or reads a GPIO\n",
	"gpio <number> <value> - Set gpio <number> to <value>.\n"
	"gpio <number>         - returns the value of gpio <number>.\n");

