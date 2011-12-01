/*
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
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>

/* GPIO LEDs
   173 red  , bank 6, index 13
   154 blue , bank 5, index 26
   61 blue2, bank 2, index 29 */


#if defined (CONFIG_ZOOM2_LED)

void omap3_zoom2_led_red_off(void)
{
	gpio_t *gpio6_base = (gpio_t *)OMAP34XX_GPIO6_BASE;

	sr32((u32)&gpio6_base->cleardataout, 13, 1, 1); /* red off */
}

void omap3_zoom2_led_blue_off(void)
{
	gpio_t *gpio2_base = (gpio_t *)OMAP34XX_GPIO2_BASE;
	gpio_t *gpio5_base = (gpio_t *)OMAP34XX_GPIO5_BASE;

	sr32((u32)&gpio5_base->cleardataout, 26, 1, 1);   /* blue off */
	sr32((u32)&gpio2_base->cleardataout, 29, 1, 1);   /* blue 2 off */
}

void omap3_zoom2_led_red_on(void)
{
	gpio_t *gpio6_base = (gpio_t *)OMAP34XX_GPIO6_BASE;

	omap3_zoom2_led_blue_off();

	sr32((u32)&gpio6_base->setdataout, 13, 1, 1); /* red on */
}

void omap3_zoom2_led_blue_on(void)
{
	gpio_t *gpio2_base = (gpio_t *)OMAP34XX_GPIO2_BASE;
	gpio_t *gpio5_base = (gpio_t *)OMAP34XX_GPIO5_BASE;

	omap3_zoom2_led_red_off();

	sr32((u32)&gpio5_base->setdataout, 26, 1, 1);   /* blue on */
	sr32((u32)&gpio2_base->setdataout, 29, 1, 1);   /* blue 2 on */
}


#endif /* CONFIG_ZOOM2_LED */
