/*
 * (C) Copyright 2009 Texas Instruments, <www.ti.com>
 * Kishore Kadiyala <kishore.kadiyala@ti.com>
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

#include <common.h>

#if defined(CFG_ENV_IS_IN_EMMC) /* Environment is in EMMC */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>

#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

/* References to names in env_common.c */
extern uchar default_environment[];

#ifdef ENV_IS_VARIABLE
char *mmc_env_name_spec = "EMMC";
/* update these elsewhere */
extern env_t *env_ptr;

#else /* !ENV_IS_VARIABLE */

char *env_name_spec = "EMMC";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = (env_t *)CFG_ENV_ADDR;
#endif /* ENV_IS_EMBEDDED */

#endif /* ENV_IS_VARIABLE */

#ifdef ENV_IS_VARIABLE
uchar mmc_env_get_char_spec(int index)
#else
uchar env_get_char_spec(int index)
#endif
{
	return *((uchar *)(gd->env_addr + index));
}

#ifdef ENV_IS_VARIABLE
void mmc_env_relocate_spec(void)
#else
void env_relocate_spec(void)
#endif
{
	unsigned int mmc_cont = 1;
	unsigned long env_addr;
	int use_default = 0;

	env_addr = CFG_ENV_ADDR;

	mmc_init(mmc_cont);
	mmc_read(mmc_cont, env_addr, (u_char *) env_ptr, CFG_ENV_SIZE);

	if (crc32(0, env_ptr->data, CFG_ENV_SIZE) != env_ptr->crc)
		use_default = 1;

	if (use_default) {
		memcpy(env_ptr->data, default_environment, CFG_ENV_SIZE);
		env_ptr->crc = crc32(0, env_ptr->data, CFG_ENV_SIZE);
	}
}

#ifdef ENV_IS_VARIABLE
int mmc_saveenv(void)
#else
int saveenv(void)
#endif
{
	unsigned long env_addr = CFG_ENV_ADDR;
	int     len;
	int 	mmc_cont = 1;

	len      = CFG_ENV_SIZE;
	puts("Erasing MMC...");
	mmc_erase(mmc_cont, env_addr, len);
	printf("done\n");

	/* update crc */
	env_ptr->crc = crc32(0, env_ptr->data, len);
	puts("Writing to MMC... ");
	mmc_write(mmc_cont, (u_char *) env_ptr, env_addr, len);
	printf("done\n");

	gd->env_valid = 1;
	return 0;

}

#ifdef ENV_IS_VARIABLE
int mmc_env_init(void)
#else
int  env_init(void)
#endif
{
	/* use default */
	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;
	return 0;
}

#endif /* CFG_ENV_IS_IN_EMMC */
