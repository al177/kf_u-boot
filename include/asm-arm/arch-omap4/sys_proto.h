/*
 * (C) Copyright 2004-2009
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _OMAP44XX_SYS_PROTO_H_
#define _OMAP44XX_SYS_PROTO_H_
#include <asm/types.h>
#include <environment.h>

extern uchar(*boot_env_get_char_spec) (int index);
extern int (*boot_env_init) (void);
extern int (*boot_saveenv) (void);
extern void (*boot_env_relocate_spec) (void);
extern char *mmc_env_name_spec;
extern env_t *env_ptr;
extern char *env_name_spec;

/* StrataNor */
uchar mmc_env_get_char_spec(int index);
int mmc_env_init(void);
int mmc_saveenv(void);
void mmc_env_relocate_spec(void);


void prcm_init(void);
void per_clocks_enable(void);

void memif_init(void);
void sdrc_init(void);
void do_sdrc_init(u32, u32);
void gpmc_init(void);

void ether_init(void);
void watchdog_init(void);
void set_muxconf_regs(void);

u32 get_cpu_type(void);
u32 get_cpu_rev(void);
u32 get_mem_type(void);
u32 get_sysboot_value(void);
u32 get_gpmc0_base(void);
u32 is_gpmc_muxed(void);
u32 get_gpmc0_type(void);
u32 get_gpmc0_width(void);
u32 get_board_type(void);
void display_board_info(u32);
void update_mux(u32, u32);
u32 get_sdr_cs_size(u32 offset);
u32 running_in_sdram(void);
u32 running_in_sram(void);
u32 running_in_flash(void);
u32 running_from_internal_boot(void);
u32 get_device_type(void);

void sr32(u32 addr, u32 start_bit, u32 num_bits, u32 value);
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound);
void sdelay(unsigned long loops);

#endif
