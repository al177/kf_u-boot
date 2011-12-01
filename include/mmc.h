/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#ifndef _MMC_H_
#define _MMC_H_
#include <asm/arch/mmc.h>

int mmc_init(int slot);
int mmc_read(int mmc_cont, unsigned int src, unsigned char *dst, int size);
int mmc_write(int mmc_cont, unsigned char *src, unsigned long dst, int size);
int mmc_erase(int mmc_cont, unsigned int start, int size);
int mmc_info(int mmc_count, unsigned int *blksize, unsigned int *sectors);

#endif /* _MMC_H_ */
