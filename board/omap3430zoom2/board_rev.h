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
#ifndef _ZOOM2_BOARD_REV
#define _ZOOM2_BOARD_REV

#define ZOOM2_BOARD_REVISION_UNKNOWN       0
#define ZOOM2_BOARD_REVISION_ALPHA         1
#define ZOOM2_BOARD_REVISION_BETA          2
#define ZOOM2_BOARD_REVISION_PRODUCTION_1  3

#if (defined(CONFIG_BOARD_REVISION) && (CONFIG_BOARD_REVISION))

int zoom2_board_revision (void);

#else

#define zoom2_board_revision()  ZOOM2_BOARD_REVISION_UNKNOWN

#endif /* CONFIG_BOARD_REVISION */

#define ZOOM2_BOARD_REVISION_STRING() \
	((ZOOM2_BOARD_REVISION_ALPHA == zoom2_board_revision()) ? "Alpha" : \
	 (ZOOM2_BOARD_REVISION_BETA == zoom2_board_revision()) ? "Beta" : \
	 (ZOOM2_BOARD_REVISION_PRODUCTION_1 == zoom2_board_revision()) ? "Production" : \
	 "Unknown")
						\
#endif /* _ZOOM2_BOARD_REV */
