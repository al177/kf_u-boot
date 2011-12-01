/*
 *  U-Boot command for OneNAND support
 *
 *  Copyright (C) 2005 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_ONENAND)

#include <linux/mtd/onenand.h>

extern struct mtd_info onenand_mtd;
extern struct onenand_chip onenand_chip;

int do_onenand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;

	switch (argc) {
	case 0:
	case 1:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;

	case 2:
		onenand_print_device_info(onenand_chip.device_id, 1);
		return 0;

	default:
		/* At least 4 args */
		if (strncmp(argv[1], "erase", 5) == 0) {
			struct erase_info instr;
			ulong start, end;
			char *endtail;
			ulong block;

			if (strncmp(argv[2], "block", 5) == 0) {
				start = simple_strtoul(argv[3], NULL, 10);
				endtail = strchr(argv[3], '-');
				end = simple_strtoul(endtail+1, NULL, 10);
			} else {
				start = simple_strtoul(argv[2], NULL, 10);
				end = simple_strtoul(argv[3], NULL, 10);
				start -= (unsigned long) onenand_chip.base;
				end -= (unsigned long) onenand_chip.base;
			}

			if (!end || end < 0)
				end = start;

			printf("Erase block from %d to %d\n", start, end);

			for (block = start; block <= end; block++) {
				instr.addr = block << onenand_chip.erase_shift;
				instr.len = 1 << onenand_chip.erase_shift;
				ret = onenand_erase(&onenand_mtd, &instr);
				if (ret) {
					printf("erase failed %d\n", block);
					break;
				}
			}

			return 0;
		}

		if (strncmp(argv[1], "read", 4) == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong ofs  = simple_strtoul(argv[3], NULL, 16);
			size_t len = simple_strtoul(argv[4], NULL, 16);
			size_t retlen = 0;
			int oob = strncmp(argv[1], "read.oob", 8) ? 0 : 1;


			if (oob)
				onenand_read_oob(&onenand_mtd, ofs, len, &retlen, (u_char *) addr);
			else
				onenand_read(&onenand_mtd, ofs, len, &retlen, (u_char *) addr);
			printf("Done\n");

			return 0;
		}

		if (strncmp(argv[1], "write", 5) == 0) {
			int ret ;
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong ofs  = simple_strtoul(argv[3], NULL, 16);
			size_t len = simple_strtoul(argv[4], NULL, 16);
			size_t retlen = 0;

			printf("onenadwrite: addr = 0x%x, ofs = 0x%x, len = 0x%x\n", addr, ofs, len);
			ret = onenand_write(&onenand_mtd, ofs, len, &retlen, (u_char *) addr);
			if (ret)
				printf("Error writing oneNAND: ret = %d\n", ret);
			else
				printf("Done. ret = %d\n", ret);

			return 0;
		}

		if (strncmp(argv[1], "block", 5) == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong block = simple_strtoul(argv[3], NULL, 10);
			ulong page = simple_strtoul(argv[4], NULL, 10);
			size_t len = simple_strtol(argv[5], NULL, 10);
			size_t retlen = 0;
			ulong ofs;
			int oob = strncmp(argv[1], "block.oob", 9) ? 0 : 1;

			ofs = block << onenand_chip.erase_shift;
			if (page)
				ofs += page << onenand_chip.page_shift;

			if (!len) {
				if (oob)
					len = 64;
				else
					len = 512;
			}

			if (oob)
				onenand_read_oob(&onenand_mtd, ofs, len, &retlen, (u_char *) addr);
			else
				onenand_read(&onenand_mtd, ofs, len, &retlen, (u_char *) addr);
			return 0;
		}

		if (strncmp(argv[1], "unlock", 6) == 0) {
			ulong start = simple_strtoul(argv[2], NULL, 10);
			ulong ofs = simple_strtoul(argv[3], NULL, 10);

			if (!ofs)
				ofs = (1 << onenand_chip.erase_shift);

			start = start << onenand_chip.erase_shift;
			printf("start = 0x%08x, ofs = 0x%08x\n",
				start, ofs);
			onenand_unlock(&onenand_mtd, start, start + ofs);
				
			return 0;
		}

		if (strncmp(argv[1], "save", 4) == 0 &&
		    strncmp(argv[2], "bootloader", 10) == 0) {
			ulong addr = simple_strtoul(argv[3], NULL, 16);
			struct erase_info instr;
			int ofs = 0;
			int len = 0x20000;
			size_t retlen;

			printf("save bootloader...\n");

			if (!addr)
				break;

			onenand_unlock(&onenand_mtd, ofs, len);

			instr.addr = 0 << onenand_chip.erase_shift;
			instr.len = 1 << onenand_chip.erase_shift;
			onenand_erase(&onenand_mtd, &instr);

			onenand_write(&onenand_mtd, ofs, len, &retlen, (u_char *) addr);
			onenand_unlock(&onenand_mtd, CFG_ENV_ADDR, onenand_mtd.size - CFG_ENV_ADDR);
			return 0;
		}

		break;
	}

	return 0;
}

U_BOOT_CMD(
	onenand, 6, 1, do_onenand,
	"onenand - OneNAND sub-system\n",
	"info   - show available OneNAND devices\n"
	"onenand read[.oob] addr ofs len - read data at ofs with len to addr\n"
	"onenand write addr ofs len - write data at ofs with len from addr\n"
	"onenand erase block start-end - erase block from start to end\n"
	"onenand erase saddr eaddr - erase block start addr to end addr\n"
	"onenand block[.oob] addr block [page] [len] - "
		"read data with (block [, page]) to addr\n"
	"onenand unlock start-end - unlock block from start to end\n"
	"onenand save bootloader addr - save bootloader at addr\n"
);

#endif	/* (CONFIG_COMMANDS & CFG_CMD_ONENAND) */

