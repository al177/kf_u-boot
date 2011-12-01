/*
 * (C) Copyright 2004-2009 Texas Instruments, <www.ti.com>
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
#include <linux/mtd/nand_ecc.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && !defined(CFG_NAND_LEGACY)

#include <nand.h>

#if (CONFIG_FASTBOOT)
#include <fastboot.h>
#endif

unsigned char cs;
volatile unsigned long gpmc_cs_base_add;

#define GPMC_BUF_EMPTY 0
#define GPMC_BUF_FULL 1

#define ECC_P1_128_E(val)    ((val)  & 0x000000FF)         /* Bit 0 to 7 */
#define ECC_P512_2048_E(val) (((val) & 0x00000F00)>>8)     /* Bit 8 to 11 */
#define ECC_P1_128_O(val)    (((val) & 0x00FF0000)>>16)    /* Bit 16 to Bit 23 */
#define ECC_P512_2048_O(val) (((val) & 0x0F000000)>>24)    /* Bit 24 to Bit 27 */

void omap_nand_switch_ecc(struct mtd_info *, int );

int nand_unlock(struct mtd_info *mtd, unsigned long off, unsigned long size)
{
	register struct nand_chip *this = mtd->priv;
	unsigned long start_block, end_block;

	printk("\nUnlocking %x - %x. locking rest..\n", off, off + size);

	if (off + size == this->chipsize)
		size -= mtd->erasesize;
	start_block = (unsigned long) (off >> this->page_shift);
	end_block = (unsigned long) ((off + size) >> this->page_shift);

	this->cmdfunc(mtd, 0x23, -1, start_block);
	this->cmdfunc(mtd, 0x24, -1, end_block);
	ndelay (100);

	return 0;
}
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

#ifdef CFG_NAND_WIDTH_16
/**
 * omap_nand_write_buf16 - [DEFAULT] write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 *
 * Default write function for 16bit buswith
 */
static void omap_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd->priv;
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (i=0; i<len; i++){
		writew(p[i], this->IO_ADDR_W);
		while (GPMC_BUF_EMPTY == (readl(GPMC_STATUS) & GPMC_BUF_FULL));
	}
}

/**
 * nand_read_buf16 - [DEFAULT] read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store date
 * @len:	number of bytes to read
 *
 * Default read function for 16bit buswith
 */

static void omap_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd->priv;
	u16 *p = (u16 *) buf;
	len >>= 1;

	for (i=0; i<len; i++)
		p[i] = readw(this->IO_ADDR_R);
}

#else
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
	int i;
	int j=0;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++) {
		writeb(buf[i], chip->IO_ADDR_W);
		for(j=0;j<10;j++);
        }

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
	int i;
	int j=0;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++) {
		buf[i] = readb(chip->IO_ADDR_R);
		while (GPMC_BUF_EMPTY == (readl(GPMC_STATUS) & GPMC_BUF_FULL));
	}
}
#endif

/*
 * omap_hwecc_init -  Initialize the Hardware ECC for NAND flash in GPMC controller
 * @mtd:        MTD device structure
 *
 */
static void omap_hwecc_init(struct nand_chip *chip)
{
	unsigned long val = 0x0;

	/* Init ECC Control Register */
	/*       Clear all ECC  | Enable Reg1 */
	val = ( (0x00000001<<8) | 0x00000001 );
	__raw_writel(val, GPMC_BASE + GPMC_ECC_CONTROL);
	__raw_writel(0x3fcff000, GPMC_BASE + GPMC_ECC_SIZE_CONFIG);
}

/*
 * omap_correct_data - Compares the ecc read from nand spare area with ECC registers values
 *			and corrects one bit error if it has occured 
 * @mtd:		 MTD device structure
 * @dat:		 page data
 * @read_ecc:		 ecc read from nand flash
 * @calc_ecc: 		 ecc read from ECC registers
 */
static int omap_correct_data(struct mtd_info *mtd,u_char *dat,
			     u_char *read_ecc, u_char *calc_ecc)
{
	return 0;
}

/*
 *  omap_calculate_ecc - Generate non-inverted ECC bytes.
 *
 *  Using noninverted ECC can be considered ugly since writing a blank
 *  page ie. padding will clear the ECC bytes. This is no problem as 
 *  long nobody is trying to write data on the seemingly unused page.
 *  Reading an erased page will produce an ECC mismatch between
 *  generated and read ECC bytes that has to be dealt with separately.
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
*/
static int omap_calculate_ecc(struct mtd_info *mtd, const u_char *dat, 
			      u_char *ecc_code)
{
	unsigned long val = 0x0;
	unsigned long reg;

	/* Start Reading from HW ECC1_Result = 0x200 */
	reg = (unsigned long)(GPMC_BASE + GPMC_ECC1_RESULT);  
	val = __raw_readl(reg);

	*ecc_code++ = ECC_P1_128_E(val);
	*ecc_code++ = ECC_P1_128_O(val);
	*ecc_code++ = ECC_P512_2048_E(val) | ECC_P512_2048_O(val) << 4;

        return 0;
}

/*
 * omap_enable_ecc - This function enables the hardware ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 */
static void omap_enable_hwecc(struct mtd_info *mtd , int mode)
{
	struct nand_chip *chip = mtd->priv;
	unsigned int val = __raw_readl(GPMC_BASE + GPMC_ECC_CONFIG);
	unsigned int dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1; 

	switch (mode) {
		case NAND_ECC_READ    :
			__raw_writel(0x101, GPMC_BASE + GPMC_ECC_CONTROL);
			/* ECC col width) | ( CS )  | ECC Enable */
			val = (dev_width << 7) | (cs << 1) | (0x1) ;
			break;
		case NAND_ECC_READSYN :
			__raw_writel(0x100, GPMC_BASE + GPMC_ECC_CONTROL);
			/* ECC col width) | ( CS )  | ECC Enable */
			val = (dev_width << 7) | (cs << 1) | (0x1) ;
			break;
		case NAND_ECC_WRITE   :
			__raw_writel(0x101, GPMC_BASE + GPMC_ECC_CONTROL);
			/* ECC col width) | ( CS )  | ECC Enable */
			val = (dev_width << 7) | (cs << 1) | (0x1) ;
			break;
		default:
			printf("Error: Unrecognized Mode[%d]!\n", mode);
			break;
	}

	__raw_writel(val, GPMC_BASE + GPMC_ECC_CONFIG);
}

static struct nand_oobinfo hw_nand_oob_64 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 12,
	.eccpos = {	
			2, 3, 4, 5,
			6, 7, 8, 9, 
			10, 11, 12, 13
		},
	.oobfree = { {20, 50} }  /* don't care */
};

static struct nand_oobinfo sw_nand_oob_64 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 24,
	.eccpos = {
			40, 41, 42, 43, 44, 45, 46, 47,
			48, 49, 50, 51, 52, 53, 54, 55,
			56, 57, 58, 59, 60, 61, 62, 63
		},
	.oobfree = { {2, 38} }
};


void omap_nand_switch_ecc(struct mtd_info *mtd, int hardware)
{
	struct nand_chip *nand = mtd->priv;

	if (!hardware) {
		nand->eccmode           = NAND_ECC_SOFT;
		nand->autooob 		= &sw_nand_oob_64;
#if (CFG_SW_ECC_512)
		nand->eccsize		= 512;
#else
		nand->eccsize		= 256;
#endif
		nand->eccbytes		= 3;
		nand->eccsteps		= mtd->oobblock / nand->eccsize;
		nand->enable_hwecc	= 0;
		nand->calculate_ecc	= nand_calculate_ecc;
		nand->correct_data	= nand_correct_data;
	} else {
		nand->eccmode           = NAND_ECC_HW3_512;
		nand->autooob 		= &hw_nand_oob_64;
		nand->eccsize		= 512;
		nand->eccbytes		= 3;
                nand->eccsteps		= 4;
		nand->enable_hwecc	= omap_enable_hwecc;
		nand->correct_data	= omap_correct_data;
		nand->calculate_ecc	= omap_calculate_ecc;

		omap_hwecc_init(nand);
	}

	mtd->eccsize = nand->eccsize;
	nand->oobdirty = 1;

	if (nand->options & NAND_BUSWIDTH_16) {
		mtd->oobavail = mtd->oobsize - (nand->autooob->eccbytes + 2);
		if (nand->autooob->eccbytes & 0x01)
			mtd->oobavail--;
	} else
		mtd->oobavail = mtd->oobsize - (nand->autooob->eccbytes + 1);
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
	nand->options           = NAND_NO_PADDING | NAND_CACHEPRG | NAND_NO_AUTOINCR | 
					NAND_BUSWIDTH_16 | NAND_NO_AUTOINCR;
	nand->read_buf          = omap_nand_read_buf;
	nand->write_buf         = omap_nand_write_buf;
	nand->eccmode           = NAND_ECC_SOFT;
#if (CFG_SW_ECC_512)
	nand->eccsize           = 512;
#else
	nand->eccsize           = 256;
#endif
/* if RDY/BSY line is connected to OMAP then use the omap ready funcrtion
 * and the generic nand_wait function which reads the status register after
 * monitoring the RDY/BSY line. Otherwise use a standard chip delay which
 * is slightly more than tR (AC Timing) of the NAND device and read the
 * status register until you get a failure or success
 */

#if 0
	nand->dev_ready         = omap_nand_dev_ready;
#else
	nand->waitfunc		= omap_nand_wait;
	nand->chip_delay        = 50*2;
#endif

#if (CONFIG_FASTBOOT)
	/* Initialize the name of fastboot flash name mappings */
	fastboot_ptentry ptn[7] = {
		{
			.name   = "xloader",
			.start  = 0x0000000,
			.length = 0x0020000,
			/* Written into the first 4 0x20000 blocks 
			   Use HW ECC */
			.flags  = FASTBOOT_PTENTRY_FLAGS_REPEAT(4) | 
			          FASTBOOT_PTENTRY_FLAGS_WRITE_HW_ECC, 
		},

		{
			.name   = "bootloader",
			.start  = 0x0080000,
			.length = 0x0180000, /* 1.5 M */
			/* Skip bad blocks on write 
			   Use SW ECC */
			.flags  = FASTBOOT_PTENTRY_FLAGS_WRITE_NEXT_GOOD_BLOCK |
			          FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC,
		},
		{
			.name   = "environment",
			.start  = SMNAND_ENV_OFFSET,  /* set in config file */
			.length = 0x0040000,
			.flags  = FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC |
			FASTBOOT_PTENTRY_FLAGS_WRITE_ENV,
		},

		{
			.name   = "kernel",
			/* Test with start close to bad block
			   The is dependent on the individual board.
			   Change to what is required */
			/* .start  = 0x0a00000, */

			/* The real start */
			.start  = 0x0200000,
			.length = 0x1D00000, /* 30M */
			.flags  = FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC |
			FASTBOOT_PTENTRY_FLAGS_WRITE_I,
		},
#ifndef CFG_NAND_YAFFS_WRITE
#warn "CFG_NAND_YAFFS_WRITE must be defined"
#endif
		{
			.name   = "system",
			.start  = 0x2000000,
			.length = 0xA000000, /* 160M */
			.flags  = FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC |
			FASTBOOT_PTENTRY_FLAGS_WRITE_YAFFS,
		},
		{
			.name   = "userdata",
			.start  = 0xC000000,
			.length = 0x2000000, /* 32M */
			.flags  = FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC |
			FASTBOOT_PTENTRY_FLAGS_WRITE_YAFFS,
		},
		{
			.name   = "cache",
			.start  = 0xE000000,
			.length = 0x2000000, /* 32M */
			.flags  = FASTBOOT_PTENTRY_FLAGS_WRITE_SW_ECC |
			FASTBOOT_PTENTRY_FLAGS_WRITE_YAFFS,
		},
	};
	int i;
	for (i = 0; i < 7; i++)
		fastboot_flash_add_ptn (&ptn[i]);
	
#endif

}


#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */

