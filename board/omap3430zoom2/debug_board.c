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
 *
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/mux.h>

#define DEBUG_BOARD_CONNECTED     1
#define DEBUG_BOARD_NOT_CONNECTED 0

static int debug_board_connected = DEBUG_BOARD_CONNECTED;

static void zoom2_debug_board_detect(void)
{
	unsigned int val;
	/* GPIO to query for debug board
	   158 db board query, bank 5, index 30 */
	gpio_t *gpio5_base = (gpio_t *)OMAP34XX_GPIO5_BASE;

	val = __raw_readl(&gpio5_base->datain);

	/* Check the bit for gpio 158 */
	if (!(val & (1 << 30)))
		debug_board_connected = DEBUG_BOARD_NOT_CONNECTED;
}

int zoom2_debug_board_connected(void)
{
	static int first_time = 1;

	if (first_time) {
		zoom2_debug_board_detect();
		first_time = 0;
	}
	return debug_board_connected;
}

