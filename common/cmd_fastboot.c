/*
 * (C) Copyright 2008 - 2009
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
 * Part of the rx_handler were copied from the Android project. 
 * Specifically rx command parsing in the  usb_rx_data_complete 
 * function of the file bootable/bootloader/legacy/usbloader/usbloader.c
 *
 * The logical naming of flash comes from the Android project
 * Thse structures and functions that look like fastboot_flash_* 
 * They come from bootable/bootloader/legacy/libboot/flash.c
 *
 * This is their Copyright:
 * 
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <asm/byteorder.h>
#include <asm/io.h>
#include <common.h>
#include <command.h>
#include <nand.h>
#include <fastboot.h>
#include <sparse.h>
#include <environment.h>
#include <twl6030.h>

#if (CONFIG_FASTBOOT)

/* Use do_reset for fastboot's 'reboot' command */
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern void do_powerdown (void);

#if (CONFIG_MMC)
extern int do_mmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif
extern env_t *env_ptr;

#if 0
/* Use do_nand for fastboot's flash commands */
extern int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
#endif
/* Use do_setenv and do_saveenv to permenantly save data */
int do_saveenv (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_setenv ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
/* Use do_bootm and do_go for fastboot's 'boot' command */
int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/* Forward decl */
static int rx_handler (const unsigned char *buffer, unsigned int buffer_size);
static void reset_handler (void);

static struct cmd_fastboot_interface interface = 
{
	.rx_handler            = rx_handler,
	.reset_handler         = reset_handler,
	.product_name          = NULL,
	.serial_no             = NULL,
	.storage_medium        = 0,
	.nand_block_size       = 0,
	.transfer_buffer       = (unsigned char *)0xffffffff,
	.transfer_buffer_size  = 0,
};

static unsigned int  fastboot_confirmed = 0;
static unsigned long fastboot_countdown;
       unsigned int download_size;
static unsigned int download_bytes;
static unsigned int download_bytes_unpadded;
static unsigned int download_error;
static unsigned int mmc_controller_no;

static void set_env(char *var, char *val)
{
	char *setenv[4]  = { "setenv", NULL, NULL, NULL, };

	setenv[1] = var;
	setenv[2] = val;

	do_setenv(NULL, 0, 3, setenv);
}

static void save_env(struct fastboot_ptentry *ptn,
		     char *var, char *val)
{
	char start[32], length[32];
	char ecc_type[32];

	char *lock[5]    = { "nand", "lock",   NULL, NULL, NULL, };
	char *unlock[5]  = { "nand", "unlock", NULL, NULL, NULL, };
	char *ecc[4]     = { "nand", "ecc",    NULL, NULL, };
	char *saveenv[2] = { "setenv", NULL, };

	lock[2] = unlock[2] = start;
	lock[3] = unlock[3] = length;

	set_env (var, val);

	/* Some flashing requires the nand's ecc to be set */
	ecc[2] = ecc_type;
	if ((ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC) &&
	    (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC))	{
		/* Both can not be true */
		printf("Warning can not do hw and sw ecc for partition '%s'\n", ptn->name);
		printf("Ignoring these flags\n");
	} else if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC) {
		sprintf(ecc_type, "hw");
#if 0
		do_nand(NULL, 0, 3, ecc);
#endif
	} else if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC) {
		sprintf(ecc_type, "sw");
#if 0
		do_nand(NULL, 0, 3, ecc);
#endif
	}
	sprintf(start, "0x%x", ptn->start);
	sprintf(length, "0x%x", ptn->length);

	/* This could be a problem is there is an outstanding lock */
#if 0
	do_nand(NULL, 0, 4, unlock);
#endif
	do_saveenv(NULL, 0, 1, saveenv);
#if 0
	do_nand(NULL, 0, 4, lock);
#endif
}

static void save_block_values(struct fastboot_ptentry *ptn,
			      unsigned int offset,
			      unsigned int size)
{
	struct fastboot_ptentry *env_ptn;

	char var[64], val[32];
	char start[32], length[32];
	char ecc_type[32];

	char *lock[5]    = { "nand", "lock",   NULL, NULL, NULL, };
	char *unlock[5]  = { "nand", "unlock", NULL, NULL, NULL, };
	char *ecc[4]     = { "nand", "ecc",    NULL, NULL, };	
	char *setenv[4]  = { "setenv", NULL, NULL, NULL, };
	char *saveenv[2] = { "setenv", NULL, };
	
	setenv[1] = var;
	setenv[2] = val;
	lock[2] = unlock[2] = start;
	lock[3] = unlock[3] = length;

	printf ("saving it..\n");

	if (size == 0)
	{
		/* The error case, where the variables are being unset */
		
		sprintf (var, "%s_nand_offset", ptn->name);
		sprintf (val, "");
		do_setenv (NULL, 0, 3, setenv);

		sprintf (var, "%s_nand_size", ptn->name);
		sprintf (val, "");
		do_setenv (NULL, 0, 3, setenv);
	}
	else
	{
		/* Normal case */

		sprintf (var, "%s_nand_offset", ptn->name);
		sprintf (val, "0x%x", offset);

		printf ("%s %s %s\n", setenv[0], setenv[1], setenv[2]);
		
		do_setenv (NULL, 0, 3, setenv);

		sprintf (var, "%s_nand_size", ptn->name);

		sprintf (val, "0x%x", size);

		printf ("%s %s %s\n", setenv[0], setenv[1], setenv[2]);

		do_setenv (NULL, 0, 3, setenv);
	}


	/* Warning : 
	   The environment is assumed to be in a partition named 'enviroment'.
	   It is very possible that your board stores the enviroment 
	   someplace else. */
	env_ptn = fastboot_flash_find_ptn("environment");

	if (env_ptn)
	{
		/* Some flashing requires the nand's ecc to be set */
		ecc[2] = ecc_type;
		if ((env_ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC) &&
		    (env_ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC)) 
		{
			/* Both can not be true */
			printf ("Warning can not do hw and sw ecc for partition '%s'\n", ptn->name);
			printf ("Ignoring these flags\n");
		} 
		else if (env_ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC)
		{
			sprintf (ecc_type, "hw");
#if 0
			do_nand (NULL, 0, 3, ecc);
#endif
		}
		else if (env_ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC)
		{
			sprintf (ecc_type, "sw");
#if 0
			do_nand (NULL, 0, 3, ecc);
#endif
		}
		
		sprintf (start, "0x%x", env_ptn->start);
		sprintf (length, "0x%x", env_ptn->length);			
#if 0
		/* This could be a problem is there is an outstanding lock */
		do_nand (NULL, 0, 4, unlock);
#endif
	}

	do_saveenv (NULL, 0, 1, saveenv);
	
	if (env_ptn)
	{
#if 0
		do_nand (NULL, 0, 4, lock);
#endif
	}
}

static void reset_handler ()
{
	/* If there was a download going on, bail */
	download_size = 0;
	download_bytes = 0;
	download_bytes_unpadded = 0;
	download_error = 0;
}

/* When save = 0, just parse.  The input is unchanged
   When save = 1, parse and do the save.  The input is changed */
static int parse_env(void *ptn, char *err_string, int save, int debug)
{
	int ret = 1;
	unsigned int sets = 0;
	char *var = NULL;
	char *var_end = NULL;
	char *val = NULL;
	char *val_end = NULL;
	unsigned int i;

	char *buff = (char *)interface.transfer_buffer;
	unsigned int size = download_bytes_unpadded;

	/* The input does not have to be null terminated.
	   This will cause a problem in the corner case
	   where the last line does not have a new line.
	   Put a null after the end of the input.

	   WARNING : Input buffer is assumed to be bigger
	   than the size of the input */
	if (save)
		buff[size] = 0;

	for (i = 0; i < size; i++) {
		if (NULL == var) {
			if (!((buff[i] == ' ') ||
			      (buff[i] == '\t') ||
			      (buff[i] == '\r') ||
			      (buff[i] == '\n')))
				var = &buff[i];
		} else if (((NULL == var_end) || (NULL == val)) &&
			   ((buff[i] == '\r') || (buff[i] == '\n'))) {

			/* This is the case when a variable
			   is unset. */

			if (save) {
				/* Set the var end to null so the
				   normal string routines will work

				   WARNING : This changes the input */
				buff[i] = '\0';

				save_env(ptn, var, val);

				if (debug)
					printf("Unsetting %s\n", var);
			}

			/* Clear the variable so state is parse is back
			   to initial. */
			var = NULL;
			var_end = NULL;
			sets++;
		} else if (NULL == var_end) {
			if ((buff[i] == ' ') ||
			    (buff[i] == '\t'))
				var_end = &buff[i];
		} else if (NULL == val) {
			if (!((buff[i] == ' ') ||
			      (buff[i] == '\t')))
				val = &buff[i];
		} else if (NULL == val_end) {
			if ((buff[i] == '\r') ||
			    (buff[i] == '\n')) {
				/* look for escaped cr or ln */
				if ('\\' == buff[i - 1]) {
					/* check for dos */
					if ((buff[i] == '\r') &&
					    (buff[i+1] == '\n'))
						buff[i + 1] = ' ';
					buff[i - 1] = buff[i] = ' ';
				} else {
					val_end = &buff[i];
				}
			}
		} else {
			sprintf(err_string, "Internal Error");

			if (debug)
				printf("Internal error at %s %d\n",
				       __FILE__, __LINE__);
			return 1;
		}
		/* Check if a var / val pair is ready */
		if (NULL != val_end) {
			if (save) {
				/* Set the end's with nulls so
				   normal string routines will
				   work.

				   WARNING : This changes the input */
				*var_end = '\0';
				*val_end = '\0';

				save_env(ptn, var, val);

				if (debug)
					printf("Setting %s %s\n", var, val);
			}

			/* Clear the variable so state is parse is back
			   to initial. */
			var = NULL;
			var_end = NULL;
			val = NULL;
			val_end = NULL;

			sets++;
		}
	}

	/* Corner case
	   Check for the case that no newline at end of the input */
	if ((NULL != var) &&
	    (NULL == val_end)) {
		if (save) {
			/* case of val / val pair */
			if (var_end)
				*var_end = '\0';
			/* else case handled by setting 0 past
			   the end of buffer.
			   Similar for val_end being null */
			save_env(ptn, var, val);

			if (debug) {
				if (var_end)
					printf("Trailing Setting %s %s\n", var, val);
				else
					printf("Trailing Unsetting %s\n", var);
			}
		}
		sets++;
	}
	/* Did we set anything ? */
	if (0 == sets)
		sprintf(err_string, "No variables set");
	else
		ret = 0;

	return ret;
}

static int saveenv_to_ptn(struct fastboot_ptentry *ptn, char *err_string)
{
	int ret = 1;
	int save = 0;
	int debug = 0;

	/* err_string is only 32 bytes
	   Initialize with a generic error message. */
	sprintf(err_string, "%s", "Unknown Error");

	/* Parse the input twice.
	   Only save to the enviroment if the entire input if correct */
	save = 0;
	if (0 == parse_env(ptn, err_string, save, debug)) {
		save = 1;
		ret = parse_env(ptn, err_string, save, debug);
	}
	return ret;
}

static int write_to_ptn(struct fastboot_ptentry *ptn)
{
	int ret = 1;
	char start[32], length[32];
	char wstart[32], wlength[32], addr[32];
	char ecc_type[32], write_type[32];
	int repeat, repeat_max;

	char *lock[5]   = { "nand", "lock",   NULL, NULL, NULL, };
	char *unlock[5] = { "nand", "unlock", NULL, NULL, NULL,	};
	char *write[6]  = { "nand", "write",  NULL, NULL, NULL, NULL, };
	char *ecc[4]    = { "nand", "ecc",    NULL, NULL, };
	char *erase[5]  = { "nand", "erase",  NULL, NULL, NULL, };

	lock[2] = unlock[2] = erase[2] = start;
	lock[3] = unlock[3] = erase[3] = length;

	write[1] = write_type;
	write[2] = addr;
	write[3] = wstart;
	write[4] = wlength;

	printf("flashing '%s'\n", ptn->name);

	/* Which flavor of write to use */
	if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_I)
		sprintf(write_type, "write.i");
#ifdef CFG_NAND_YAFFS_WRITE
	else if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_YAFFS)
		sprintf(write_type, "write.yaffs");
#endif
	else
		sprintf(write_type, "write");


	/* Some flashing requires the nand's ecc to be set */
	ecc[2] = ecc_type;
	if ((ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC) &&
	    (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC)) {
		/* Both can not be true */
		printf("Warning can not do hw and sw ecc for partition '%s'\n",
		       ptn->name);
		printf("Ignoring these flags\n");
	} else if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC) {
		sprintf(ecc_type, "hw");
#if 0
		do_nand(NULL, 0, 3, ecc);
#endif
	} else if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC) {
		sprintf(ecc_type, "sw");
#if 0
		do_nand(NULL, 0, 3, ecc);
#endif
	}

	/* Some flashing requires writing the same data in multiple,
	   consecutive flash partitions */
	repeat_max = 1;
	if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_REPEAT_MASK) {
		if (ptn->flags &
		    FASTBOOT_PTENTRY_FLAGS_WRITE_CONTIGUOUS_BLOCK) {
			printf("Warning can not do both 'contiguous block' and 'repeat' writes for for partition '%s'\n", ptn->name);
			printf("Ignoring repeat flag\n");
		} else {
			repeat_max = ptn->flags &
				FASTBOOT_PTENTRY_FLAGS_REPEAT_MASK;
		}
	}

	/* Unlock the whole partition instead of trying to
	   manage special cases */
	sprintf(length, "0x%x", ptn->length * repeat_max);

	for (repeat = 0; repeat < repeat_max; repeat++) {
		sprintf(start, "0x%x", ptn->start + (repeat * ptn->length));
#if 0
		do_nand(NULL, 0, 4, unlock);
		do_nand(NULL, 0, 4, erase);
#endif

		if ((ptn->flags &
		     FASTBOOT_PTENTRY_FLAGS_WRITE_NEXT_GOOD_BLOCK) &&
		    (ptn->flags &
		     FASTBOOT_PTENTRY_FLAGS_WRITE_CONTIGUOUS_BLOCK)) {
			/* Both can not be true */
			printf("Warning can not do 'next good block' and 'contiguous block' for partition '%s'\n", ptn->name);
			printf("Ignoring these flags\n");
		} else if (ptn->flags &
			   FASTBOOT_PTENTRY_FLAGS_WRITE_NEXT_GOOD_BLOCK) {
			/* Keep writing until you get a good block
			   transfer_buffer should already be aligned */
			if (interface.nand_block_size) {
				unsigned int blocks = download_bytes /
					interface.nand_block_size;
				unsigned int i = 0;
				unsigned int offset = 0;

				sprintf(wlength, "0x%x",
					interface.nand_block_size);
				while (i < blocks) {
					/* Check for overflow */
					if (offset >= ptn->length)
						break;

					/* download's address only advance
					   if last write was successful */
					sprintf(addr, "0x%x",
						interface.transfer_buffer +
						(i * interface.nand_block_size));

					/* nand's address always advances */
					sprintf(wstart, "0x%x",
						ptn->start + (repeat * ptn->length) + offset);
#if 0
					ret = do_nand(NULL, 0, 5, write);
#endif
					if (ret)
						break;
					else
						i++;

					/* Go to next nand block */
					offset += interface.nand_block_size;
				}
			} else {
				printf("Warning nand block size can not be 0 when using 'next good block' for partition '%s'\n", ptn->name);
				printf("Ignoring write request\n");
			}
		} else if (ptn->flags &
			 FASTBOOT_PTENTRY_FLAGS_WRITE_CONTIGUOUS_BLOCK) {
#if 0
			/* Keep writing until you get a good block
			   transfer_buffer should already be aligned */
			if (interface.nand_block_size) {
				if (0 == nand_curr_device) {
					nand_info_t *nand;
					unsigned long off;
					unsigned int ok_start;

					nand = &nand_info[nand_curr_device];

					printf("\nDevice %d bad blocks:\n",
					       nand_curr_device);

					/* Initialize the ok_start to the
					   start of the partition
					   Then try to find a block large
					   enough for the download */
					ok_start = ptn->start;

					/* It is assumed that the start and
					   length are multiples of block size */
					for (off = ptn->start;
					     off < ptn->start + ptn->length;
					     off += nand->erasesize) {
						if (nand_block_isbad(nand, off)) {
							/* Reset the ok_start
							   to the next block */
							ok_start = off +
								nand->erasesize;
						}

						/* Check if we have enough
						   blocks */
						if ((ok_start - off) >=
						    download_bytes)
							break;
					}

					/* Check if there is enough space */
					if (ok_start + download_bytes <=
					    ptn->start + ptn->length) {
						sprintf(addr,    "0x%x", interface.transfer_buffer);
						sprintf(wstart,  "0x%x", ok_start);
						sprintf(wlength, "0x%x", download_bytes);

						ret = do_nand(NULL, 0, 5, write);

						/* Save the results into an
						   environment variable on the
						   format
						   ptn_name + 'offset'
						   ptn_name + 'size'  */
						if (ret) {
							/* failed */
							save_block_values(ptn, 0, 0);
						} else {
							/* success */
							save_block_values(ptn, ok_start, download_bytes);
						}
					} else {
						printf("Error could not find enough contiguous space in partition '%s' \n", ptn->name);
						printf("Ignoring write request\n");
					}
				} else {
					/* TBD : Generalize flash handling */
					printf("Error only handling 1 NAND per board");
					printf("Ignoring write request\n");
				}
			} else {
				printf("Warning nand block size can not be 0 when using 'continuous block' for partition '%s'\n", ptn->name);
				printf("Ignoring write request\n");
			}
#endif
		} else {
			/* Normal case */
			sprintf(addr,    "0x%x", interface.transfer_buffer);
			sprintf(wstart,  "0x%x", ptn->start +
				(repeat * ptn->length));
			sprintf(wlength, "0x%x", download_bytes);
#ifdef CFG_NAND_YAFFS_WRITE
			if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_YAFFS)
				sprintf(wlength, "0x%x",
					download_bytes_unpadded);
#endif

#if 0
			ret = do_nand(NULL, 0, 5, write);
#endif

			if (0 == repeat) {
				if (ret) /* failed */
					save_block_values(ptn, 0, 0);
				else     /* success */
					save_block_values(ptn, ptn->start,
							  download_bytes);
			}
		}
#if 0
		do_nand(NULL, 0, 4, lock);
#endif

		if (ret)
			break;
	}

	return ret;
}

static int rx_handler (const unsigned char *buffer, unsigned int buffer_size)
{
	int ret = 1;

	/* Use 65 instead of 64
	   null gets dropped  
	   strcpy's need the extra byte */
	char response[65];

	if (download_size) 
	{
		/* Something to download */

		if (buffer_size)
		{
			/* Handle possible overflow */
			unsigned int transfer_size = 
				download_size - download_bytes;

			if (buffer_size < transfer_size)
				transfer_size = buffer_size;
			
			/* Save the data to the transfer buffer */
			memcpy (interface.transfer_buffer + download_bytes, 
				buffer, transfer_size);

			download_bytes += transfer_size;
			
			/* Check if transfer is done */
			if (download_bytes >= download_size) {
				/* Reset global transfer variable,
				   Keep download_bytes because it will be
				   used in the next possible flashing command */
				download_size = 0;

				if (download_error) {
					/* There was an earlier error */
					sprintf(response, "ERROR");
				} else {
					/* Everything has transferred,
					   send the OK response */
					sprintf(response, "OKAY");
				}
				fastboot_tx_status(response, strlen(response));

				printf ("\ndownloading of %d bytes finished\n",
					download_bytes);

				/* Padding is required only if storage medium is NAND */
				if (interface.storage_medium == NAND) {
					/* Pad to block length
					   In most cases, padding the download to be
					   block aligned is correct. The exception is
					   when the following flash writes to the oob
					   area.  This happens when the image is a
					   YAFFS image.  Since we do not know what
					   the download is until it is flashed,
					   go ahead and pad it, but save the true
					   size in case if should have
					   been unpadded */
					download_bytes_unpadded = download_bytes;
					if (interface.nand_block_size)
					{
						if (download_bytes %
						    interface.nand_block_size)
						{
							unsigned int pad = interface.nand_block_size - (download_bytes % interface.nand_block_size);
							unsigned int i;

							for (i = 0; i < pad; i++)
							{
								if (download_bytes >= interface.transfer_buffer_size)
									break;

								interface.transfer_buffer[download_bytes] = 0;
								download_bytes++;
							}
						}
					}
				}
			}

			/* Provide some feedback */
			if (download_bytes &&
			    0 == (download_bytes %
				  (16 * interface.nand_block_size)))
			{
				/* Some feeback that the
				   download is happening */
				if (download_error)
					printf("X");
				else
					printf(".");
				if (0 == (download_bytes %
					  (80 * 16 *
					   interface.nand_block_size)))
					printf("\n");
				
			}
		}
		else
		{
			/* Ignore empty buffers */
			printf ("Warning empty download buffer\n");
			printf ("Ignoring\n");
		}
		ret = 0;
	}
	else
	{
		/* A command */
                fastboot_confirmed=1;


		/* Cast to make compiler happy with string functions */
		const char *cmdbuf = (char *) buffer;

		/* Generic failed response */
		sprintf(response, "FAIL");

		/* reboot 
		   Reboot the board. */
		if(memcmp(cmdbuf, "reboot-bootloader", 17) == 0)
		{
			sprintf(response,"OKAY");
			fastboot_tx_status(response, strlen(response));

			/* Clear all reset reasons */
			__raw_writel(0xfff, PRM_RSTST);

			strcpy(PUBLIC_SAR_RAM_1_FREE, "reboot-bootloader");

			/* now warm reset the silicon */
			__raw_writel(PRM_RSTCTRL_RESET_WARM_BIT,
					PRM_RSTCTRL);
			return 0;
		}

		if(memcmp(cmdbuf, "reboot", 6) == 0) 
		{
			sprintf(response,"OKAY");
			fastboot_tx_status(response, strlen(response));

			do_reset (NULL, 0, 0, NULL);
			
			/* This code is unreachable,
			   leave it to make the compiler happy */
			return 0;
		}
		
		/* getvar
		   Get common fastboot variables
		   Board has a chance to handle other variables */
		if(memcmp(cmdbuf, "getvar:", 7) == 0) 
		{
			int get_var_length = strlen("getvar:");

			strcpy(response,"OKAY");

			if(!strcmp(cmdbuf + get_var_length, "version")) {
				strcpy(response + 4, FASTBOOT_VERSION);

			} else if(!strcmp(cmdbuf + get_var_length, "product")) {
				if (interface.product_name)
					strcpy(response + 4, interface.product_name);

			} else if(!strcmp(cmdbuf + get_var_length, "serialno")) {
				if (interface.serial_no)
					strcpy(response + 4, interface.serial_no);

			} else if(!strcmp(cmdbuf + get_var_length, "downloadsize")) {
				if (interface.transfer_buffer_size) 
					sprintf(response + 4, "%08x", interface.transfer_buffer_size);
			} else if(!strcmp(cmdbuf + get_var_length, "cpurev")) {
				if (interface.proc_rev)
					strcpy(response + 4, interface.proc_rev);
			} else if(!strcmp(cmdbuf + get_var_length, "secure")) {
				if (interface.proc_type)
					strcpy(response + 4, interface.proc_type);
			} else {
				fastboot_getvar(cmdbuf + 7, response + 4);
			}
			ret = 0;

		}

		/* %fastboot oem <cmd> */
		if (memcmp(cmdbuf, "oem ", 4) == 0) {

			ret = 0;
			cmdbuf += 4;

			if (memcmp(cmdbuf, "shutdown", 8) == 0) {
				sprintf(response, "SHUTDOWN");
				fastboot_tx_status(response, strlen(response));
				do_powerdown ();
			}
            if (memcmp(cmdbuf, "idme ", 5) == 0) {
				ret = fastboot_idme(cmdbuf + 5);
                if (ret) {
					strcpy(response,"FAIL");
    			} else {
    				strcpy(response,"OKAY");
    			}
    			ret = 0;
    			goto done;
			}
			/* fastboot oem format */
			if(memcmp(cmdbuf, "format", 6) == 0){
				ret = fastboot_oem(cmdbuf);
				if (ret < 0) {
					strcpy(response,"FAIL");
				} else {
					strcpy(response,"OKAY");
				}
				goto done;
			}

			/* fastboot oem recovery */
			if(memcmp(cmdbuf, "recovery", 8) == 0){
				sprintf(response,"OKAY");
				fastboot_tx_status(response, strlen(response));

				/* Clear all reset reasons */
				__raw_writel(0xfff, PRM_RSTST);
				strcpy(PUBLIC_SAR_RAM_1_FREE, "recovery");
				/* now warm reset the silicon */
				__raw_writel(PRM_RSTCTRL_RESET_WARM_BIT,
					PRM_RSTCTRL);
				/* Never returns */
				while(1);
			}

			/* fastboot oem unlock */
			if(memcmp(cmdbuf, "unlock", 6) == 0){
				sprintf(response,"FAIL");
				printf("\nfastboot: oem unlock "\
						"not implemented yet!!\n");
				goto done;
			}

			/* fastboot oem [xxx] */
			printf("\nfastboot: do not understand oem %s\n", cmdbuf);
			strcpy(response,"FAIL");
			goto done;
		} /* end: %fastboot oem <cmd> */

		/* erase
		   Erase a register flash partition
		   Board has to set up flash partitions */

		if(memcmp(cmdbuf, "erase:", 6) == 0){

			if (interface.storage_medium == NAND) {
				/* storage medium is NAND */

				struct fastboot_ptentry *ptn;

				ptn = fastboot_flash_find_ptn(cmdbuf + 6);
				if (ptn == 0)
				{
					sprintf(response, "FAILpartition does not exist");
				}
				else
				{
					char start[32], length[32];
					int status = 0, repeat, repeat_max;

					printf("erasing '%s'\n", ptn->name);

					char *lock[5]   = { "nand", "lock",   NULL, NULL, NULL, };
					char *unlock[5] = { "nand", "unlock", NULL, NULL, NULL,	};
					char *erase[5]  = { "nand", "erase",  NULL, NULL, NULL, };

					lock[2] = unlock[2] = erase[2] = start;
					lock[3] = unlock[3] = erase[3] = length;

					repeat_max = 1;
					if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_REPEAT_MASK)
						repeat_max = ptn->flags & FASTBOOT_PTENTRY_FLAGS_REPEAT_MASK;

					sprintf (length, "0x%x", ptn->length);
					for (repeat = 0; repeat < repeat_max; repeat++)
					{
						sprintf (start, "0x%x", ptn->start + (repeat * ptn->length));
#if 0
						do_nand (NULL, 0, 4, unlock);
						status = do_nand (NULL, 0, 4, erase);
						do_nand (NULL, 0, 4, lock);
#endif

						if (status)
							break;
					}

					if (status)
					{
						sprintf(response, "FAILfailed to erase partition");
					}
					else
					{
						printf("partition '%s' erased\n", ptn->name);
						sprintf(response, "OKAY");
					}
				}
			} else if (interface.storage_medium == EMMC) {
				/* storage medium is EMMC */

				struct fastboot_ptentry *ptn;

				/* Save the MMC controller number */
#if  defined(CONFIG_4430PANDA)
				/* panda board does not have eMMC on mmc1 */
				mmc_controller_no = 0;
#else
				/* blaze has emmc on mmc1 */
				mmc_controller_no = 1;
#endif

				/* Find the partition and erase it */
				ptn = fastboot_flash_find_ptn(cmdbuf + 6);

				if (ptn == 0) {
					sprintf(response,
					"FAIL: partition doesn't exist");
				} else {
					/* Call MMC erase function here */
					char start[32], length[32];
					char slot_no[32];

					char *erase[5]  = { "mmc", NULL, "erase",
							NULL, NULL, };
					char *mmc_init[2] = {"mmcinit", NULL,};

					mmc_init[1] = slot_no;
					erase[1] = slot_no;
					erase[3] = start;
					erase[4] = length;

					sprintf(slot_no, "%d", mmc_controller_no);
					sprintf(length, "0x%x", ptn->length);
					sprintf(start, "0x%x", ptn->start);

					printf("Initializing '%s'\n", ptn->name);
					if (do_mmc(NULL, 0, 2, mmc_init))
						sprintf(response, "FAIL: Init of MMC card");
					else
						sprintf(response, "OKAY");


					printf("Erasing '%s'\n", ptn->name);
					if (do_mmc(NULL, 0, 5, erase)) {
						printf("Erasing '%s' FAILED!\n", ptn->name);
						sprintf(response, "FAIL: Erase partition");
					} else {
						printf("Erasing '%s' DONE!\n", ptn->name);
						sprintf(response, "OKAY");
					}
				}
			}

			ret = 0;
		}


		/* EMMC Erase
		   Erase a register flash partition on MMC
		   Board has to set up flash partitions */
		if (memcmp(cmdbuf, "mmcerase:", 9) == 0) {
			struct fastboot_ptentry *ptn;

			/* Save the MMC controller number */
			mmc_controller_no = simple_strtoul(cmdbuf + 9,
								NULL, 10);
			/* Find the partition and erase it */
			ptn = fastboot_flash_find_ptn(cmdbuf + 11);

			if (ptn == 0) {
				sprintf(response,
					"FAIL: partition doesn't exist");
			} else {
				/* Call MMC erase function here */
				/* This is not complete */
				char start[32], length[32];
				char slot_no[32];

				char *erase[5]  = { "mmc", NULL, "erase",
								NULL, NULL, };
				char *mmc_init[2] = {"mmcinit", NULL,};

				mmc_init[1] = slot_no;
				erase[1] = slot_no;
				erase[3] = start;
				erase[4] = length;

				sprintf(slot_no, "%d", mmc_controller_no);
				sprintf(length, "0x%x", ptn->length);
				sprintf(start, "0x%x", ptn->start);

				printf("Initializing '%s'\n", ptn->name);
				if (do_mmc(NULL, 0, 2, mmc_init)) {
					sprintf(response, "FAIL: Init of MMC card");
				} else {
					sprintf(response, "OKAY");
				}

				printf("Erasing '%s'\n", ptn->name);
				if (do_mmc(NULL, 0, 5, erase)) {
					sprintf(response, "FAIL: Erase partition");
				} else {
					sprintf(response, "OKAY");
				}
			}
		}

		/* download
		   download something .. 
		   What happens to it depends on the next command after data */
		if(memcmp(cmdbuf, "download:", 9) == 0) {

			/* save the size */
			download_size = simple_strtoul(cmdbuf + 9, NULL, 16);
			/* Reset the bytes count, now it is safe */
			download_bytes = 0;
			/* Reset error */
			download_error = 0;

			printf("Starting download of %d bytes\n",
							download_size);

			if (0 == download_size)
			{
				/* bad user input */
				sprintf(response, "FAILdata invalid size");
			} else if (download_size >
						interface.transfer_buffer_size)
			{
				/* set download_size to 0
				 * because this is an error */
				download_size = 0;
				sprintf(response, "FAILdata too large");
			}
			else
			{
				/* The default case, the transfer fits
				   completely in the interface buffer */
				sprintf(response, "DATA%08x", download_size);
			}
			ret = 0;
		}

		/* boot
		   boot what was downloaded
		   **
		   ** +-----------------+
		   ** | boot header     | 1 page
		   ** +-----------------+
		   ** | kernel          | n pages
		   ** +-----------------+
		   ** | ramdisk         | m pages
		   ** +-----------------+
		   ** | second stage    | o pages
		   ** +-----------------+
		   **
		   Pagesize has default value of
		   CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE
		*/

		if(memcmp(cmdbuf, "boot", 4) == 0) {

			if ((download_bytes) &&
			    (CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE < download_bytes))
			{
				char start[32];
				char *booti_args[4] = { "booti", NULL, "boot", NULL };

				/* Skip the mkbootimage header */
				//boot_img_hdr *hdr =
				//	(boot_img_hdr *)
				//	&interface.transfer_buffer[CFG_FASTBOOT_MKBOOTIMAGE_PAGE_SIZE];

				booti_args[1] = start;
				sprintf (start, "0x%x", interface.transfer_buffer);

				/* Execution should jump to kernel so send the response
				   now and wait a bit.  */
				sprintf(response, "OKAY");
				fastboot_tx_status(response, strlen(response));

				printf ("Booting kernel..\n");

				/* For Future use
				 *	if (strlen ((char *) &fb_hdr->cmdline[0]))
				 *		set_env ("bootargs", (char *) &fb_hdr->cmdline[0]);
				 */
				/* boot the boot.img */
				do_booti (NULL, 0, 3, booti_args);
			}
			sprintf(response, "FAILinvalid boot image");
			ret = 0;
		}

		/* mmcwrite
		   write what was downloaded on MMC*/
				/* Write to MMC whatever was downloaded */
		if (memcmp(cmdbuf, "mmcwrite:", 9) == 0) {

			if (download_bytes) {

				struct fastboot_ptentry *ptn;

				/* Save the MMC controller number */
				mmc_controller_no = simple_strtoul(cmdbuf + 9, NULL, 10);

				/* Next is the partition name */
				ptn = fastboot_flash_find_ptn(cmdbuf + 11);

				if (ptn == 0) {
					sprintf(response, "FAILpartition does not exist");
				} else {
					char source[32], dest[32], length[32];
					char slot_no[32];

					printf("writing to partition '%s'\n", ptn->name);
					char *mmc_write[6]  = {"mmc", NULL, "write", NULL, NULL, NULL};
					char *mmc_init[2] = {"mmcinit", NULL,};

					mmc_init[1] = slot_no;
					mmc_write[1] = slot_no;
					mmc_write[3] = source;
					mmc_write[4] = dest;
					mmc_write[5] = length;

					sprintf(slot_no, "%d", mmc_controller_no);
					sprintf(source, "0x%x",	interface.transfer_buffer);
					sprintf(dest, "0x%x", ptn->start);
					sprintf(length, "0x%x", download_bytes);

					printf("Initializing '%s'\n", ptn->name);
					if (do_mmc(NULL, 0, 2, mmc_init)) {
						sprintf(response, "FAIL:Init of MMC card");
					} else {
						sprintf(response, "OKAY");
					}

					printf("Writing '%s'\n", ptn->name);
					if (do_mmc(NULL, 0, 6, mmc_write)) {
						sprintf(response, "FAIL: Write partition");
					} else {
						sprintf(response, "OKAY");
					}
				}

			} else {
				sprintf(response, "FAILno image downloaded");
			}
		}


		/* flash
		   Flash what was downloaded */

		if(memcmp(cmdbuf, "flash:", 6) == 0) {

			if (interface.storage_medium == NAND) {
				/* storage medium is NAND */

				if (download_bytes)
				{
					struct fastboot_ptentry *ptn;

					ptn = fastboot_flash_find_ptn(cmdbuf + 6);
					if (ptn == 0) {
						sprintf(response, "FAILpartition does not exist");
					} else if ((download_bytes > ptn->length) &&
						   !(ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_ENV)) {
						sprintf(response, "FAILimage too large for partition");
						/* TODO : Improve check for yaffs write */
					} else {
						/* Check if this is not really a flash write
						   but rather a saveenv */
						if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_ENV) {
							/* Since the response can only be 64 bytes,
							   there is no point in having a large error message. */
							char err_string[32];
							if (saveenv_to_ptn(ptn, &err_string[0])) {
								printf("savenv '%s' failed : %s\n", ptn->name, err_string);
								sprintf(response, "FAIL%s", err_string);
							} else {
								printf("partition '%s' saveenv-ed\n", ptn->name);
								sprintf(response, "OKAY");
							}
						} else {
							/* Normal case */
							if (write_to_ptn(ptn)) {
								printf("flashing '%s' failed\n", ptn->name);
								sprintf(response, "FAILfailed to flash partition");
							} else {
								printf("partition '%s' flashed\n", ptn->name);
								sprintf(response, "OKAY");
							}
						}
					}
				}
				else
				{
					sprintf(response, "FAILno image downloaded");
				}
			} else if (interface.storage_medium == EMMC) {
				/* storage medium is EMMC */

				if (download_bytes) {

					struct fastboot_ptentry *ptn;
					struct fastboot_ptentry tmpptn;
					char *argv[2] = {NULL, "-f"};

					/* Save the MMC controller number */
#if  defined(CONFIG_4430PANDA)
					/* panda board does not have eMMC on mmc1 */
					mmc_controller_no = 0;
#else
					/* blaze has emmc on mmc1 */
					mmc_controller_no = 1;
#endif

					/* Next is the partition name */
					if(memcmp(cmdbuf+6, "all:", 4) == 0){
						printf("Factory image testing");
						ptn=&tmpptn;
						sprintf(ptn->name,"%s","all");
						ptn->length=download_bytes;
						ptn->start=simple_strtoul(cmdbuf + 10, NULL, 16);
						printf("name=%s length=%x offset=%x\n",ptn->name,ptn->length,ptn->start);
					}else if(memcmp(cmdbuf+6, "bootpart:", 9) == 0){
						printf("boot partition testing\n");
						ptn=&tmpptn;
						sprintf(ptn->name,"bootpart%d",simple_strtoul(cmdbuf + 15, NULL, 10));
						ptn->length=download_bytes;
						ptn->start=0;
						printf("name=%s length=%x offset=%x\n",ptn->name,ptn->length,ptn->start);

					}else{
						ptn = fastboot_flash_find_ptn(cmdbuf + 6);
					}

					if (ptn == 0) {
						printf("Partition:'%s' does not exist\n", ptn->name);
						sprintf(response, "FAILpartition does not exist");
					} else if ((download_bytes > ptn->length) &&
						!(ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_ENV)) {
						printf("Image too large for the partition\n");
						sprintf(response, "FAILimage too large for partition");
						} else if (ptn->flags & FASTBOOT_PTENTRY_FLAGS_WRITE_ENV) {
						/* Check if this is not really a flash write,
						 * but instead a saveenv
						 */
						unsigned int i = 0;
						/* Env file is expected with a NULL delimeter between
						 * env variables So replace New line Feeds (0x0a) with
						 * NULL (0x00)
						 */
						for (i = 0; i < download_bytes; i++) {
							if (interface.transfer_buffer[i] == 0x0a)
								interface.transfer_buffer[i] = 0x00;
						}
						memset(env_ptr->data, 0, ENV_SIZE);
						memcpy(env_ptr->data, interface.transfer_buffer, download_bytes);
						do_saveenv(NULL, 0, 2, argv);
						printf("saveenv to '%s' DONE!\n", ptn->name);
						sprintf(response, "OKAY");
					} else {
						/* Normal case */

						char source[32], dest[32], length[32];
						char slot_no[32];

						printf("writing to partition '%s'\n", ptn->name);
						char *mmc_write[6]  = {"mmc", NULL, "write", NULL, NULL, NULL};
						char *mmc_sw_part[4]  = {"mmc", NULL, "sw_part", NULL};
						char *mmc_init[2] = {"mmcinit", NULL,};

						mmc_init[1] = slot_no;
						mmc_write[1] = slot_no;
						mmc_write[3] = source;
						mmc_write[4] = dest;
						mmc_write[5] = length;

						sprintf(slot_no, "%d", mmc_controller_no);
						sprintf(source, "0x%x", interface.transfer_buffer);
						sprintf(dest, "0x%x", ptn->start);
						sprintf(length, "0x%x", download_bytes);

						printf("Initializing '%s'\n", ptn->name);
						if (do_mmc(NULL, 0, 2, mmc_init)) {
							sprintf(response, "FAIL:Init of MMC card");
							goto done;
						} else
							sprintf(response, "OKAY");

						if(strncmp(ptn->name, "bootpart",8) == 0){
							mmc_sw_part[1] = slot_no;
						    mmc_sw_part[3] = source;
							sprintf(slot_no, "%d", mmc_controller_no);
							strncpy(source, &(ptn->name)+8, 1);
							if (do_mmc(NULL, 0, 4, mmc_sw_part)) {
								printf("sw_part '%s' FAILED!\n", ptn->name);
								sprintf(response, "FAIL: change partition");
							} else {
								printf("sw_part '%s' DONE!\n", ptn->name);
                                sprintf(source, "0x%x", interface.transfer_buffer);
							}
						}
						/* Check if we have sparse compressed image */
						if ( ((sparse_header_t *)interface.transfer_buffer)->magic
								== SPARSE_HEADER_MAGIC) {
							printf("fastboot: %s is in sparse format\n", ptn->name);
							if (!do_unsparse(interface.transfer_buffer,
									ptn->start,
									ptn->length,
									slot_no)) {
								printf("Writing sparsed: '%s' DONE!\n", ptn->name);
							} else {
								printf("Writing sparsed '%s' FAILED!\n", ptn->name);
								sprintf(response, "FAIL: Sparsed Write");
							}
						} else {
							/* Normal image: no sparse */
							printf("Writing '%s'\n", ptn->name);
							if (do_mmc(NULL, 0, 6, mmc_write)) {
								printf("Writing '%s' FAILED!\n", ptn->name);
								sprintf(response, "FAIL: Write partition");
							} else {
								printf("Writing '%s' DONE!\n", ptn->name);
							}
						}
					} /* Normal Case */

				} else {
					sprintf(response, "FAILno image downloaded");
				}
			} /* EMMC */

			ret = 0;
		} /* fastboot flash ... */
done:
		fastboot_tx_status(response, strlen(response));

	} /* End of command */
	
	return ret;
}


	
int do_fastboot (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret = 1;
	unsigned long val;
	/* Initialize the board specific support */
	if (0 == fastboot_init(&interface))
	{
		printf ("Fastboot entered...\n");
		run_command("setgreenled 50", 0);
		fastboot_countdown = CFG_FASTBOOT_COUNTDOWN;
		/* If we got this far, we are a success */
		ret = 0;
		val = getbootmode();
		/* On disconnect or error, polling returns non zero */
		/* Always delay for fastboot unless we're in bootmode 4002 
		 * or using the "factory" cable */
		if((0x4002 != val) && !(__raw_readl(0x48055138) & 0x00100000)){
			while (fastboot_countdown)
			{
	            if (!fastboot_confirmed) {
                   fastboot_countdown--;
                }
				if (fastboot_poll())
					break;
				/* if we're holding down the button to get into
				 * recovery, don't wait for the fastboot timeout so we don't
				 * accidentally power off.  short circuit 49999/50000 times
				 * through to keep from overwhelming the twl6030 */

				if (!(fastboot_countdown % 50000) &&
						(0 == twl6030_get_power_button_status())) {
					fastboot_wait_power_button_abort = 1;
					break;
				}
			}
		}else{
			while (1)
			{
				if (fastboot_poll())
					break;
			}
		}
	}

	/* Reset the board specific support */
	fastboot_shutdown();
	/* If we're in 4003, go on to OMAP boot over USB */
	if(0x4003 == val){
        printf ("setting boot sequence first to USB.\nreboot...\n");
        set_SWBootingCfg();		
	}
	return ret;
}

U_BOOT_CMD(
	fastboot,	1,	1,	do_fastboot,
	"fastboot- use USB Fastboot protocol\n",
	NULL
);


/* To support the Android-style naming of flash */
#define MAX_PTN 16

static fastboot_ptentry ptable[MAX_PTN];
static unsigned int pcount = 0;

void fastboot_flash_reset_ptn(void)
{
	pcount = 0;
}

void fastboot_flash_add_ptn(fastboot_ptentry *ptn)
{
    if(pcount < MAX_PTN){
        memcpy(ptable + pcount, ptn, sizeof(*ptn));
        pcount++;
    }
}

void fastboot_flash_dump_ptn(void)
{
    unsigned int n;
    for(n = 0; n < pcount; n++) {
        fastboot_ptentry *ptn = ptable + n;
        printf("ptn %d name='%s' start=%d len=%d\n",
                n, ptn->name, ptn->start, ptn->length);
    }
}


fastboot_ptentry *fastboot_flash_find_ptn(const char *name)
{
    unsigned int n;
    
    for(n = 0; n < pcount; n++) {
	    /* Make sure a substring is not accepted */
	    if (strlen(name) == strlen(ptable[n].name))
	    {
		    if(0 == strcmp(ptable[n].name, name))
			    return ptable + n;
	    }
    }
    return 0;
}

fastboot_ptentry *fastboot_flash_get_ptn(unsigned int n)
{
    if(n < pcount) {
        return ptable + n;
    } else {
        return 0;
    }
}

unsigned int fastboot_flash_get_ptn_count(void)
{
    return pcount;
}



#endif	/* CONFIG_FASTBOOT */
