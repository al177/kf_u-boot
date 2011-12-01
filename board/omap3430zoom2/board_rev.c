/*
 * Copyright (c) 2009 Wind River Systems, Inc.
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
#include "board_rev.h"

#if (defined(CONFIG_BOARD_REVISION) && (CONFIG_BOARD_REVISION))

static int board_revision = ZOOM2_BOARD_REVISION_UNKNOWN;

static void zoom2_board_revision_detect (void)
{
	unsigned int val;
	/*
	 * GPIO 94 to query for board revision
	 * production vs preproduction
	 * 94 is bank bank 3, index 30
	 */
	gpio_t *gpio3_base = (gpio_t *) OMAP34XX_GPIO3_BASE;

	val = __raw_readl (&gpio3_base->datain);

	/* Check the bit for gpio 94 */
	if (!(val & (1 << 30)))
		board_revision = ZOOM2_BOARD_REVISION_PRODUCTION_1;
	else
		board_revision = ZOOM2_BOARD_REVISION_BETA;
}

int zoom2_board_revision (void)
{
	if (ZOOM2_BOARD_REVISION_UNKNOWN == board_revision) {
		zoom2_board_revision_detect ();
	}
	return board_revision;
}

#endif /* CONFIG_BOARD_REVISION */
