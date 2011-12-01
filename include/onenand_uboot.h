/*
 *  Header file for OneNAND support for U-Boot
 *
 *  Adaptation from kernel to U-Boot
 *
 *  Copyright (C) 2005 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __UBOOT_ONENAND_H
#define __UBOOT_ONENAND_H

#ifndef __iomem
#define __iomem
#endif

#ifndef CONFIG_OMAP
#define __mem_pci(x)			(x)
#endif

#define ONENAND_DEBUG
#undef ONENAND_DEBUG

#define DEBUG_LEVEL                    1

#define MTD_DEBUG_LEVEL0		0
#define MTD_DEBUG_LEVEL1		1
#define MTD_DEBUG_LEVEL2		2
#define MTD_DEBUG_LEVEL3		3

#ifdef ONENAND_DEBUG
#define DEBUG(level, args...)                                           \
do {                                                                    \
        if (level <= DEBUG_LEVEL) {                                     \
                printf(args);                                           \
        }                                                               \
} while (0)
#else
#define DEBUG(level, args...)           do { } while (0)
#endif

enum {
        EAGAIN,
        EIO,
        EBADMSG,
        ENXIO,
        EINVAL,
	EPERM,
	ENOMEM,
};

#define KERN_INFO
#define KERN_ERR
#define KERN_WARNING
#define KERN_DEBUG

#define min_t(type,x,y) ({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })

#define unlikely(x)             (x)

struct mtd_info {
        int             size;
        int             oobblock;
        int             oobsize;
        int             erasesize;

        void            *priv;
};

#define MTD_ERASE_PENDING       0x01
#define MTD_ERASING             0x02
#define MTD_ERASE_SUSPEND       0x04
#define MTD_ERASE_DONE          0x08
#define MTD_ERASE_FAILED        0x10

struct erase_info {
	struct mtd_info *mtd;
	u_int32_t addr;
	u_int32_t len;
	u_int32_t fail_addr;
	u_char state;
};

struct nand_oobinfo {
};

struct kvec {
	void *iov_base;
	size_t iov_len;
};

typedef int 			spinlock_t;
typedef int 			wait_queue_head_t;

#define printk(args...)		printf(args)

#define mtd_erase_callback(x)	do { } while (0)

/* Functions */
extern int onenand_read(struct mtd_info *mtd, loff_t from, size_t len,
        size_t *retlen, u_char *buf);
extern int onenand_read_oob(struct mtd_info *mtd, loff_t from, size_t len,
        size_t *retlen, u_char *buf);
extern int onenand_write(struct mtd_info *mtd, loff_t from, size_t len,
        size_t *retlen, const u_char *buf);
extern int onenand_erase(struct mtd_info *mtd, struct erase_info *instr);

extern int onenand_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);

extern void onenand_print_device_info(int device, int verbose);


#endif	/* __UBOOT_ONENAND_H */
