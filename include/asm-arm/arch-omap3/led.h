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

#ifndef _OMAP3_LED_H
#define _OMAP3_LED_H

#if defined (CONFIG_ZOOM2_LED)

extern void omap3_zoom2_led_red_on(void);
extern void omap3_zoom2_led_red_off(void);
extern void omap3_zoom2_led_blue_on(void);
extern void omap3_zoom2_led_blue_off(void);

#define OMAP3_LED_OK_ON() omap3_zoom2_led_blue_on ()
#define OMAP3_LED_OK_OFF() omap3_zoom2_led_blue_off ()
#define OMAP3_LED_ERROR_ON() omap3_zoom2_led_red_on ()
#define OMAP3_LED_ERROR_OFF() omap3_zoom2_led_red_on ()

#endif /* CONFIG_ZOOM2_LED */

#ifndef OMAP3_LED_OK_ON
#define OMAP3_LED_OK_ON()
#endif
#ifndef OMAP3_LED_OK_OFF
#define OMAP3_LED_OK_OFF()
#endif
#ifndef OMAP3_LED_ERROR_ON
#define OMAP3_LED_ERROR_ON()
#endif
#ifndef OMAP3_LED_ERROR_OFF
#define OMAP3_LED_ERROR_OFF()
#endif

#endif /* _OMAP3_LED_H */
