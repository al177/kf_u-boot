/*
 * (C) Copyright 2004-2006
 * Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
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

#include <asm/io.h>

#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && !defined(CFG_NAND_LEGACY)

#include <nand.h>

#define GPMC_CHUNK_SHIFT	24		/* 16 MB */
#define GPMC_SECTION_SHIFT	28		/* 128 MB */

#define CS_NUM_SHIFT		24
#define ENABLE_PREFETCH		7
#define DMA_MPU_MODE		2

#define OMAP_NAND_GPMC_PREFETCH	1

unsigned char cs;
volatile unsigned long gpmc_cs_base_add;

#ifdef OMAP_NAND_GPMC_PREFETCH
volatile unsigned long nand_fifo_add;
#endif

/*
 * omap_nand_hwcontrol - Set the address pointers corretly for the
 *			following address/data/command operation
 * @mtd:        MTD device structure
 * @ctrl:	Says whether Address or Command or Data is following.
 */

static void omap_nand_hwcontrol(struct mtd_info *mtd, int ctrl)
{
	register struct nand_chip *this = mtd->priv;


/*
 * Point the IO_ADDR to DATA and ADDRESS registers instead of chip address
 */
	switch (ctrl) {
	 case NAND_CTL_SETCLE:
		this->IO_ADDR_W = (void *) gpmc_cs_base_add + GPMC_NAND_CMD;
		this->IO_ADDR_R = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
		break;
	 case NAND_CTL_SETALE:
		this->IO_ADDR_W = (void *) gpmc_cs_base_add + GPMC_NAND_ADR;
		this->IO_ADDR_R = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
		break;
	 case NAND_CTL_CLRCLE:
		this->IO_ADDR_W = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
		this->IO_ADDR_R = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
		break;
	 case NAND_CTL_CLRALE:
		this->IO_ADDR_W = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
		this->IO_ADDR_R = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
		break;
	}
}

/*
 * omap_nand_wait - called primarily after a program/erase operation
 *			so that we access NAND again only after the device
 *			is ready again.
 * @mtd:        MTD device structure
 * @chip:	nand_chip structure
 * @state:	State from which wait function is being called i.e write/erase.
 */
static int omap_nand_wait(struct mtd_info *mtd, struct nand_chip *chip, int state)
{
	register struct nand_chip *this = mtd->priv;
	int status = 0;

	this->IO_ADDR_W = (void *) gpmc_cs_base_add + GPMC_NAND_CMD;
	this->IO_ADDR_R = (void *) gpmc_cs_base_add + GPMC_NAND_DAT;
	/* Send the status command and loop until the device is free */
	while(!(status & 0x40)){
		__raw_writeb(NAND_CMD_STATUS & 0xFF, this->IO_ADDR_W);
		status = __raw_readb(this->IO_ADDR_R);
	}
	return status;
}

#ifdef OMAP_NAND_GPMC_PREFETCH
/*
 * gpmc_cs_get_memconf - Get memory configs of device
 * @cs:		Chip selected
 * @base:	Base address pointer for device
 * @size:	Size of device
 */
static void gpmc_cs_get_memconf(int cs, u32 *base, u32 *size)
{
	u32 l;
	u32 mask;

	l = __raw_readl(gpmc_cs_base_add + GPMC_CONFIG7);
	*base = (l & 0x3f) << GPMC_CHUNK_SHIFT;
	mask = (l >> 8) & 0x0f;
	*size = (1 << GPMC_SECTION_SHIFT) - (mask << GPMC_CHUNK_SHIFT);
}

/*
 * gpmc_prefetch_init - configures default configuration for prefetch engine
 */
static void gpmc_prefetch_init(void)
{
	/* Setting the default threshold to 64 */
	__raw_writel(0x0, GPMC_BASE + GPMC_PREFETCH_CONTROL);
	__raw_writel(0x40  << 8, GPMC_BASE + GPMC_PREFETCH_CONFIG1);
	__raw_writel(0x0, GPMC_BASE + GPMC_PREFETCH_CONFIG2);
}

/*
 * gpmc_prefetch_start - configures and starts prefetch transfer
 * @cs - nand cs (chip select) number
 * @dma_mode: dma mode enable (1) or disable (0)
 * @u32_count: number of bytes to be transferred
 * @is_write: prefetch read(0) or write post(1) mode
 */
void gpmc_prefetch_start(int cs, unsigned int u32_count, int is_write)
{
	uint32_t prefetch_config1;
	if (is_write) {
		/* Set the amount of bytes to be prefetched */
		 __raw_writel(u32_count, GPMC_BASE + GPMC_PREFETCH_CONFIG2);

		/* Set mpu mode, the post write and enable the engine
		 * Set which cs is using the post write
		 */
		prefetch_config1 = __raw_readl(GPMC_BASE + GPMC_PREFETCH_CONFIG1);
		prefetch_config1 |= (((cs << CS_NUM_SHIFT) |
					(1 << ENABLE_PREFETCH) | 0x1) &
					~(1 << DMA_MPU_MODE));
		 __raw_writel(prefetch_config1, GPMC_BASE + GPMC_PREFETCH_CONFIG1);
	} else {
		/* Set the amount of bytes to be prefetched */
		__raw_writel(u32_count, GPMC_BASE + GPMC_PREFETCH_CONFIG2);

		/* Set dma/mpu mode, the prefech read and enable the engine
		 * Set which cs is using the prefetch
		 */
		prefetch_config1 = __raw_readl(GPMC_BASE + GPMC_PREFETCH_CONFIG1);
		prefetch_config1 |= (((cs << CS_NUM_SHIFT) |
					(1 << ENABLE_PREFETCH)) &
					~((1 << DMA_MPU_MODE) | 0x1));
		__raw_writel(prefetch_config1, GPMC_BASE + GPMC_PREFETCH_CONFIG1);
	}
	/*  Start the prefetch engine */
	__raw_writel(0x1, GPMC_BASE + GPMC_PREFETCH_CONTROL);
}

/*
 * gpmc_prefetch_stop - disables and stops the prefetch engine
 */
void gpmc_prefetch_stop(void)
{
	uint32_t prefetch_config1;
	/* stop the PFPW engine */
	__raw_writel(0x0, GPMC_BASE + GPMC_PREFETCH_CONTROL);

	/* Disable the PFPW engine */
	prefetch_config1 = __raw_readl(GPMC_BASE + GPMC_PREFETCH_CONFIG1);
	prefetch_config1 &= ~((0x07 << CS_NUM_SHIFT) |
				(1 << ENABLE_PREFETCH) |
					(1 << DMA_MPU_MODE) | 0x1);
	__raw_writel(prefetch_config1, GPMC_BASE + GPMC_PREFETCH_CONFIG1);
}

/*
 * gpmc_prefetch_status - reads prefetch status of engine
 */
int  gpmc_prefetch_status(void)
{
	return __raw_readl(GPMC_BASE + GPMC_PREFETCH_STATUS);
}
#endif

/*
 * omap_nand_write_buf -  write buffer to NAND controller
 * @mtd:        MTD device structure
 * @buf:        data buffer
 * @len:        number of bytes to write
 *
 */

static void omap_nand_write_buf(struct mtd_info *mtd, const uint8_t * buf,
				int len)
{
	uint8_t *p = (uint8_t *)buf;
	uint32_t prefetch_status = 0;
	int i, bytes_to_write = 0;

#ifdef OMAP_NAND_GPMC_PREFETCH
	/*  configure and start prefetch transfer */
	gpmc_prefetch_start(cs, len, 0x1);

	prefetch_status = gpmc_prefetch_status();
	while (prefetch_status & 0x3FFF) {
		bytes_to_write = (prefetch_status >> 24) & 0x7F;
		for (i = 0; ((i < bytes_to_write) && (len)); i++, len--)
			*(volatile uint8_t *)(nand_fifo_add) = *p++;
		prefetch_status = gpmc_prefetch_status();
	}
	/* disable and stop the PFPW engine */
	gpmc_prefetch_stop();
#else
	for (i = 0; i < len; i++) {
		writeb(p[i], chip->IO_ADDR_W);
		for(j=0;j<10;j++);
        }
#endif
}

/*
 * omap_nand_read_buf - read data from NAND controller into buffer
 * @mtd:        MTD device structure
 * @buf:        buffer to store date
 * @len:        number of bytes to read
 *
 */

static void omap_nand_read_buf(struct mtd_info *mtd, uint8_t * buf, int len)
{
	uint32_t prefetch_status = 0;
	int i, bytes_to_read = 0;

#ifdef OMAP_NAND_GPMC_PREFETCH
		/* configure and start prefetch transfer */
		gpmc_prefetch_start(cs, len, 0x0);

		prefetch_status = gpmc_prefetch_status();
		while (len) {
			bytes_to_read  = (prefetch_status >> 24) & 0x7F;
			for (i = 0; (i < bytes_to_read) && (len); i++, len--)
				*buf++ = *(volatile uint8_t *)(nand_fifo_add);
			prefetch_status = gpmc_prefetch_status();
		}
		/* disable and stop the PFPW engine */
		gpmc_prefetch_stop();
#else
	for (i = 0; i < len; i++) {
		buf[i] = readb(chip->IO_ADDR_R);
		for(j=0;j<10;j++);
	}
#endif
}

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific (per include/linux/mtd/nand_new.h):
 * - IO_ADDR_R?: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W?: address to write the 8 I/O lines of the flash device
 * - hwcontrol: hardwarespecific function for accesing control-lines
 * - dev_ready: hardwarespecific function for  accesing device ready/busy line
 * - enable_hwecc?: function to enable (reset)  hardware ecc generator. Must
 *   only be provided if a hardware ECC is available
 * - eccmode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 * Members with a "?" were not set in the merged testing-NAND branch,
 * so they are not set here either.
 */
void board_nand_init(struct nand_chip *nand)
{
	int gpmc_config=0;
	u32 size;
        cs = 0;
	while (cs <= GPMC_MAX_CS) {
		/* Each GPMC set for a single CS is at offset 0x30 */
		/* already remapped for us */
		gpmc_cs_base_add = (GPMC_CONFIG_CS0 + (cs*0x30));
		/* xloader/Uboot would have written the NAND type for us
		 * -NOTE This is a temporary measure and cannot handle ONENAND.
		 * The proper way of doing this is to pass the setup of u-boot up to kernel
		 * using kernel params - something on the lines of machineID
		 */
		/* Check if NAND type is set */
		if ((__raw_readl(gpmc_cs_base_add + GPMC_CONFIG1) & 0xC00)==0x800) {
		/* Found it!! */
			break;
		}
		cs++;
	}
	if (cs > GPMC_MAX_CS) {
		printk ("NAND: Unable to find NAND settings in GPMC Configuration - quitting\n");
	}
	gpmc_config = __raw_readl(GPMC_CONFIG);
	/* Disable Write protect */
	gpmc_config |= 0x10;
	__raw_writel(gpmc_config, GPMC_CONFIG);

	nand->IO_ADDR_R		= (void *)gpmc_cs_base_add + GPMC_NAND_DAT;
	nand->IO_ADDR_W		= (void *)gpmc_cs_base_add + GPMC_NAND_CMD;

	nand->hwcontrol         = omap_nand_hwcontrol;
	nand->options           = NAND_SAMSUNG_LP_OPTIONS;
	nand->read_buf          = omap_nand_read_buf;
	nand->write_buf         = omap_nand_write_buf;
	nand->eccmode           = NAND_ECC_SOFT;
#if (CFG_SW_ECC_512)
	nand->eccsize		= 512;
#else
	nand->eccsize		= 256;
#endif
/* if RDY/BSY line is connected to OMAP then use the omap ready funcrtion
 * and the generic nand_wait function which reads the status register after
 * monitoring the RDY/BSY line. Otherwise use a standard chip delay which
 * is slightly more than tR (AC Timing) of the NAND device and read the
 * status register until you get a failure or success
 */
#ifdef OMAP_NAND_GPMC_PREFETCH
	gpmc_cs_get_memconf(cs, (u32 *)&nand_fifo_add, &size);
	gpmc_prefetch_init();
#endif

#if 0
	nand->dev_ready         = omap_nand_dev_ready;
#else
	nand->waitfunc		= omap_nand_wait;
	nand->chip_delay        = 50;
#endif
}


#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */

