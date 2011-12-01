/*
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 * Author: Vikram Pandita <vikram.pandita@ti.com>
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

#include <config.h>
#include <common.h>
#include <sparse.h>
#include <mmc.h>

//#define DEBUG

#define SPARSE_HEADER_MAJOR_VER 1

int mmc_compare(unsigned mmcc, unsigned char *src, unsigned sector, unsigned len)
{
	u8 data[512];

	while (len > 0) {
		if (mmc_read(mmcc, sector, data, 512) != 1) {
			printf("mmc read error sector %d\n", sector);
			return -1;
		}
		if (memcmp(data, src, 512)) {
			printf("mmc data mismatch sector %d\n", sector);
			return -1;
		}
		len -= 512;
		sector++;
		src += 512;
	}
	return 0;
}


int _unsparse(unsigned char *source, u32 sector, u32 section_size,
	      unsigned mmcc,
	      int (*WRITE)(unsigned mwcc, unsigned char *src,
			   unsigned sector, unsigned len))
{
	sparse_header_t *header = (void*) source;
	u32 i, outlen = 0;

	printf("sparse: write to mmc slot[%d] @ %d\n", mmcc, sector);

	if ((header->total_blks * header->blk_sz) > section_size) {
		printf("sparse: section size %d MB limit: exceeded\n",
				section_size/(1024*1024));
		return 1;
	}

	if (header->magic != SPARSE_HEADER_MAGIC) {
		printf("sparse: bad magic\n");
		return 1;
	}

	if ((header->major_version != SPARSE_HEADER_MAJOR_VER) ||
	    (header->file_hdr_sz != sizeof(sparse_header_t)) ||
	    (header->chunk_hdr_sz != sizeof(chunk_header_t))) {
		printf("sparse: incompatible format\n");
		return 1;
	}
	/* todo: ensure image will fit */

	/* Skip the header now */
	source += header->file_hdr_sz;

	for (i=0; i < header->total_chunks; i++) {
		unsigned int len = 0;
		int r;
		chunk_header_t *chunk = (void*) source;

		/* move to next chunk */
		source += sizeof(chunk_header_t);

		switch (chunk->chunk_type) {
		case CHUNK_TYPE_RAW:
			len = chunk->chunk_sz * header->blk_sz;

			if (chunk->total_sz != (len + sizeof(chunk_header_t))) {
				printf("sparse: bad chunk size for chunk %d, type Raw\n", i);
				return 1;
			}

			outlen += len;
			if (outlen > section_size) {
				printf("sparse: section size %d MB limit: exceeded\n", section_size/(1024*1024));
				return 1;
			}
#ifdef DEBUG
			printf("sparse: RAW blk=%d bsz=%d: write(sector=%d,len=%d)\n",
			       chunk->chunk_sz, header->blk_sz, sector, len);
#endif
			r = WRITE(mmcc, source, sector, len);
			if (r < 0) {
				printf("sparse: mmc write failed\n");
				return 1;
			}

			sector += (len / 512);
			source += len;
			break;

		case CHUNK_TYPE_DONT_CARE:
			if (chunk->total_sz != sizeof(chunk_header_t)) {
				printf("sparse: bogus DONT CARE chunk\n");
				return 1;
			}
			len = chunk->chunk_sz * header->blk_sz;
#ifdef DEBUG
			printf("sparse: DONT_CARE blk=%d bsz=%d: skip(sector=%d,len=%d)\n",
			       chunk->chunk_sz, header->blk_sz, sector, len);
#endif

			outlen += len;
			if (outlen > section_size) {
				printf("sparse: section size %d MB limit: exceeded\n", section_size/(1024*1024));
				return 1;
			}
			sector += (len / 512);
			break;

		default:
			printf("sparse: unknown chunk ID %04x\n", chunk->chunk_type);
			return 1;
		}
	}

	printf("\nsparse: out-length-0x%d MB\n", outlen/(1024*1024));
	return 0;
}

u8 do_unsparse(unsigned char *source, u32 sector, u32 section_size, char *slot_no)
{
	unsigned mmcc = simple_strtoul(slot_no, NULL, 16);
	if (_unsparse(source, sector, section_size, mmcc, mmc_write))
		return 1;
#if DEBUG
	if (_unsparse(source, sector, section_size, mmcc, mmc_compare))
		return 1;
#endif
	return 0;
}
			
