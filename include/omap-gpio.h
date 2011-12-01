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

#define GPIO_NUM_LOW 0
#define GPIO_NUM_HIGH 159

enum {
	GPIO_DIR_OUTPUT = 0,
	GPIO_DIR_INPUT  = 1
};

enum {
	GPIO_LOW = 0,
	GPIO_HIGH = 1
};

int omap_gpio_set_output(int gpio_num, int value);
int omap_gpio_set_direction_input(int gpio_num);
int omap_gpio_get_value(int gpio_num);

