/*
 * Basic I2C functions
 *
 * Copyright (c) 2004 Texas Instruments
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author: Jian Zhang jzhang@ti.com, Texas Instruments
 *
 * Copyright (c) 2003 Wolfgang Denk, wd@denx.de
 * Rewritten to fit into the current U-Boot framework
 *
 * Adapted for OMAP2420 I2C, r-woodruff2@ti.com
 *
 */

#include <common.h>

#if defined(CONFIG_DRIVER_OMAP24XX_I2C) || defined(CONFIG_DRIVER_OMAP34XX_I2C) || defined(CONFIG_DRIVER_OMAP44XX_I2C)

#include <asm/arch/i2c.h>
#include <asm/io.h>
#include <i2c.h>

static u32 i2c_base = I2C_DEFAULT_BASE;
static u32 i2c_speed = CFG_I2C_SPEED;
static u32 i2c_bus = 0;

u32 get_i2c_speed(void)
{
  return i2c_speed;
}
u32 get_i2c_bus(void)
{
  return i2c_bus;
}

//#define DEBUG

#if DEBUG

#define DBG(ARGS...) {printf ("[%d]",__LINE__);printf(ARGS);}
#define inb(a) ({u8 v=__raw_readb(i2c_base + (a));printf("%d:Rb[%x<=%x]\n",__LINE__,a,v);v;})
#define outb(v,a) {printf("%d:Wb[%x<=%x]\n",__LINE__,a,v);__raw_writeb((v), (i2c_base + (a)));}
#define inw(a) ({u16 v=__raw_readb(i2c_base + (a));printf("%d:Rw[%x<=%x]\n",__LINE__,a,v);v;})
#define outw(v,a) {printf("%d:Ww[%x<=%x]\n",__LINE__,a,v);__raw_writew((v), (i2c_base + (a)));}

#else
#define DBG(ARGS...)
#define inb(a) __raw_readb(i2c_base + (a))
#define outb(v,a) __raw_writeb((v), (i2c_base + (a)))
#define inw(a) __raw_readw(i2c_base +(a))
#define outw(v,a) __raw_writew((v), (i2c_base + (a)))
#endif

static void wait_for_bb(void);
static u16 wait_for_pin(void);
static void flush_fifo(void);

#if defined(CONFIG_OMAP44XX)
#define I2C_NUM_IF 4
#elif (CONFIG_OMAP34XX)
#define I2C_NUM_IF 3
#else
#define I2C_NUM_IF 2
#endif

int select_bus(int bus, int speed)
{
	if ((bus < 0) || (bus >= I2C_NUM_IF)) {
		printf("Bad bus ID-%d\n", bus);
		return -1;
	}

#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
	/* Check speed */
	if ((speed != OMAP_I2C_STANDARD) && (speed != OMAP_I2C_FAST_MODE)
	    && (speed != OMAP_I2C_HIGH_SPEED)) {
		printf("Invalid Speed for i2c init-%d\n", speed);
		return -1;
	}
#else
	if ((speed != OMAP_I2C_STANDARD) && (speed != OMAP_I2C_FAST_MODE)) {
		printf("Invalid Speed for i2c init-%d\n", speed);
		return -1;
	}
#endif

#if defined(CONFIG_OMAP44XX)
	if (bus == 3)
		i2c_base = I2C_BASE4;
	else
#endif
#if defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
	if (bus == 2)
		i2c_base = I2C_BASE3;
	else 
#endif
	if (bus == 1)
		i2c_base = I2C_BASE2;
	else
		i2c_base = I2C_BASE1;

        i2c_bus = bus;
	i2c_init(speed, CFG_I2C_SLAVE);
	return 0;
}

void i2c_init(int speed, int slaveadd)
{
	int scl_lh = 0;
	int psc = 0;
	int iclk = 0;
	int reset_timeout = 10;

	/* assume clock settings done */
	/* write to clock regs to enable if and fun clks for board */
#if defined(CONFIG_OMAP243X)
	{
		u32 v = 0;

		v = __raw_readl(CM_ICLKEN1_CORE) | (0x3 << 19);	/* Interface clocks on */
		__raw_writel(v, CM_ICLKEN1_CORE);
		v = __raw_readl(CM_FCLKEN1_CORE) & ~(0x3 << 19);
		__raw_writel(v, CM_FCLKEN1_CORE);
		v = __raw_readl(CM_FCLKEN2_CORE) | (0x3 << 19);	/* Functional Clocks on */
		__raw_writel(v, CM_FCLKEN2_CORE);
	}
#endif				/* End of 243x code */

	if (inw(I2C_CON) & I2C_CON_EN) {
		outw(0, I2C_CON);
		udelay(50000);
	}
	outw(I2C_SYSC_SRST, I2C_SYSC);	/* for ES2 after soft reset */
	udelay(1000);
	/* compute divisors - dynamic decision based on i/p clock */
	psc = I2C_PSC_MAX;
	while (psc >= I2C_PSC_MIN) {
		iclk = I2C_IP_CLK / (psc + 1);
		switch (speed) {
		case OMAP_I2C_STANDARD:
			scl_lh = (iclk * 10 / (OMAP_I2C_STANDARD * 2));
			break;
		case OMAP_I2C_HIGH_SPEED:
			/* PSC ignored for HS */
		case OMAP_I2C_FAST_MODE:
			scl_lh = (iclk * 10 / (OMAP_I2C_FAST_MODE * 2));
			break;
			/* no default case  - fall thru */
		}
		DBG("Search- speed= %d SysClk=%d, iclk=%d,psc=0x%x[%d],scl_lh=0x%x[%d]\n",
	       speed, I2C_IP_CLK, iclk, psc, psc, scl_lh, scl_lh);
		/* Check for decimal places.. if yes, we ignore it */
		if (scl_lh % 10) {
			scl_lh = -1;
		} else {
			scl_lh /= 10;
			scl_lh -= 7;
		}
		if (scl_lh >= 0) {
			break;
		}
		psc--;
	}
	/* Did not find an optimal config */
	if (psc < I2C_PSC_MIN) {
		printf
		    ("Unable to set Prescalar for i2c_clock=%d syI2C_IP_CLK=%d\n",
		     speed, I2C_IP_CLK);
		psc = 0;
		return;

	}
	iclk = I2C_IP_CLK / (psc + 1);
	/* Initialize the I2C clock timers to generate an I2C bus clock
	 * frequency of i2c_clock kilohertz (default is 100 KHz).
	 */
	switch (speed) {
	case OMAP_I2C_STANDARD:
		scl_lh =
		    (((iclk / (OMAP_I2C_STANDARD * 2)) - 7) &
		     I2C_SCLL_SCLL_M) << I2C_SCLL_SCLL;
		break;
	case OMAP_I2C_HIGH_SPEED:
		scl_lh =
		    (((I2C_IP_CLK / (OMAP_I2C_HIGH_SPEED * 2)) - 7) &
		     I2C_SCLH_HSSCLL_M) << I2C_SCLL_HSSCLL;
		/* Fall through for the FS settings */
	case OMAP_I2C_FAST_MODE:
		scl_lh |=
		    (((iclk / (OMAP_I2C_FAST_MODE * 2)) - 7) &
		     I2C_SCLL_SCLL_M) << I2C_SCLL_SCLL;
		break;
		/* no default case */
	}

	DBG(" speed= %d SysClk=%d, iclk=%d,psc=0x%x[%d],scl_lh=0x%x[%d]\n",
	       speed, I2C_IP_CLK, iclk, psc, psc, scl_lh, scl_lh);
	outw(I2C_CON_EN, I2C_CON);
	while (!(inw(I2C_SYSS) & I2C_SYSS_RDONE) && reset_timeout--) {
		if (reset_timeout <= 0)
			printf("ERROR: Timeout while waiting for soft-reset to complete\n");
		udelay(1000);
	}

	outw(0, I2C_CON);  /* Disable I2C controller before writing
                                        to PSC and SCL registers */
	outw(psc, I2C_PSC);
	outw(scl_lh, I2C_SCLL);
	outw(scl_lh, I2C_SCLH);
	/* own address */
	outw(slaveadd, I2C_OA);
	outw(I2C_CON_EN, I2C_CON);

	/* have to enable intrrupts or OMAP i2c module doesn't work */
	outw(I2C_IE_XRDY_IE | I2C_IE_RRDY_IE | I2C_IE_ARDY_IE |
	     I2C_IE_NACK_IE | I2C_IE_AL_IE, I2C_IE);
	udelay(1000);
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	i2c_speed = speed;
}

static int i2c_read_byte(u8 devaddr, u8 regoffset, u8 * value)
{
	int i2c_error = 0;
	u16 status;

	/* wait until bus not busy */
	wait_for_bb();

	/* one byte only */
	outw(1, I2C_CNT);
	/* set slave address */
	outw(devaddr, I2C_SA);
	/* no stop bit needed here */
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX, I2C_CON);

	status = wait_for_pin();

	if (status & I2C_STAT_XRDY) {
		/* Important: have to use byte access */
		outb(regoffset, I2C_DATA);
		udelay(20000);
		if (inw(I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}

	if (!i2c_error) {
		int err = 10;
		while (inw(I2C_STAT) || (inw(I2C_CON) & I2C_CON_MST)) {
			udelay(10000);
			/* Have to clear pending interrupt to clear I2C_STAT */
			outw(0xFFFF, I2C_STAT);
			if (!err--) {
				break;
			}
		}

		/* set slave address */
		outw(devaddr, I2C_SA);
		/* read one byte from slave */
		outw(1, I2C_CNT);
		/* need stop bit here */
		outw(I2C_CON_EN |
		     ((i2c_speed ==
		       OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) | I2C_CON_MST |
		     I2C_CON_STT | I2C_CON_STP, I2C_CON);

		status = wait_for_pin();
		if (status & I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
			*value = inb(I2C_DATA);
#else
			*value = inw(I2C_DATA);
#endif
			udelay(20000);
		} else {
			i2c_error = 1;
		}

		if (!i2c_error) {
			int err = 10;
			outw(I2C_CON_EN, I2C_CON);
			while (inw(I2C_STAT)
			       || (inw(I2C_CON) & I2C_CON_MST)) {
				udelay(10000);
				outw(0xFFFF, I2C_STAT);
				if (!err--) {
					break;
				}
			}
		}
	}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	return i2c_error;
}

static int i2c_write_byte(u8 devaddr, u8 regoffset, u8 value)
{
	int i2c_error = 0;
	u16 status, stat;

	/* wait until bus not busy */
	wait_for_bb();

	/* two bytes */
	outw(2, I2C_CNT);
	/* set slave address */
	outw(devaddr, I2C_SA);
	/* stop bit needed here */
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP, I2C_CON);

	/* wait until state change */
	status = wait_for_pin();

	if (status & I2C_STAT_XRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
		/* send out 1 byte */
		outb(regoffset, I2C_DATA);
		outw(I2C_STAT_XRDY, I2C_STAT);
		status = wait_for_pin();
		if ((status & I2C_STAT_XRDY)) {
			/* send out next 1 byte */
			outb(value, I2C_DATA);
			outw(I2C_STAT_XRDY, I2C_STAT);
		} else {
			i2c_error = 1;
		}
#else
		/* send out 2 bytes */
		outw((value << 8) | regoffset, I2C_DATA);
#endif
		/* must have enough delay to allow BB bit to go low */
		udelay(50000);
		if (inw(I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}
	if (!i2c_error) {
		int eout = 200;

		outw(I2C_CON_EN, I2C_CON);
		while ((stat = inw(I2C_STAT)) || (inw(I2C_CON) & I2C_CON_MST)) {
			udelay(1000);
			/* have to read to clear intrrupt */
			outw(0xFFFF, I2C_STAT);
			if (--eout == 0)	/* better leave with error than hang */
				break;
		}
	}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	return i2c_error;
}

static void flush_fifo(void)
{
	u16 stat;

	/* note: if you try and read data when its not there or ready
	 * you get a bus error
	 */
	while (1) {
		stat = inw(I2C_STAT);
		if (stat == I2C_STAT_RRDY) {
#if defined(CONFIG_OMAP243X) || defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)
			inb(I2C_DATA);
#else
			inw(I2C_DATA);
#endif
			outw(I2C_STAT_RRDY, I2C_STAT);
			udelay(1000);
		} else
			break;
	}
}

int i2c_probe(uchar chip)
{
	int res = 1;		/* default = fail */

	if (chip == inw(I2C_OA)) {
		return res;
	}

	/* wait until bus not busy */
	wait_for_bb();

	/* try to read one byte */
	outw(1, I2C_CNT);
	/* set slave address */
	outw(chip, I2C_SA);
	/* stop bit needed here */
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_STP, I2C_CON);
	/* enough delay for the NACK bit set */
	udelay(50000);

	if (!(inw(I2C_STAT) & I2C_STAT_NACK)) {
		res = 0;	/* success case */
		flush_fifo();
		outw(0xFFFF, I2C_STAT);
	} else {
		outw(0xFFFF, I2C_STAT);	/* failue, clear sources */
		outw(inw(I2C_CON) | I2C_CON_STP, I2C_CON);	/* finish up xfer */
		udelay(20000);
		wait_for_bb();
	}
	flush_fifo();
	outw(0, I2C_CNT);	/* don't allow any more data in...we don't want it. */
	outw(0xFFFF, I2C_STAT);
	return res;
}

int i2c_read(uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	int i;

	if (alen > 1) {
		printf("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte(chip, addr + i, &buffer[i])) {
			printf("I2C read: I/O error\n");
			i2c_init(i2c_speed, CFG_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	int i;

	if (alen > 1) {
		printf("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if (addr + len > 256) {
		printf("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_write_byte(chip, addr + i, buffer[i])) {
			printf("I2C read: I/O error\n");
			i2c_init(i2c_speed, CFG_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write_addr(uchar devaddr, uint addr)
{
	int i2c_error = 0;
	u16 status, stat;

	/* wait until bus not busy */
	wait_for_bb();

	/* one byte */
	outw(1, I2C_CNT);
	/* set slave address */
	outw(devaddr, I2C_SA);
	/* stop bit needed here */
	outw(I2C_CON_EN | ((i2c_speed == OMAP_I2C_HIGH_SPEED) ? 0x1 << 12 : 0) |
	     I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP, I2C_CON);

	/* wait until state change */
	status = wait_for_pin();

	if (status & I2C_STAT_XRDY) {
		/* send out 1 byte */
		outb(addr, I2C_DATA);
		/* must have enough delay to allow BB bit to go low */
		udelay(50000);
		if (inw(I2C_STAT) & I2C_STAT_NACK) {
			i2c_error = 1;
		}
	} else {
		i2c_error = 1;
	}
	if (!i2c_error) {
		int eout = 200;

		outw(I2C_CON_EN, I2C_CON);
		while ((stat = inw(I2C_STAT)) || (inw(I2C_CON) & I2C_CON_MST)) {
			udelay(1000);
			/* have to read to clear intrrupt */
			outw(0xFFFF, I2C_STAT);
			if (--eout == 0)	/* better leave with error than hang */
				break;
		}
	}
	flush_fifo();
	outw(0xFFFF, I2C_STAT);
	outw(0, I2C_CNT);
	return i2c_error;
}

static void wait_for_bb(void)
{
	int timeout = 10;
	u16 stat;

	outw(0xFFFF, I2C_STAT);	/* clear current interruts... */
	while ((stat = inw(I2C_STAT) & I2C_STAT_BB) && timeout--) {
		outw(stat, I2C_STAT);
		udelay(50000);
	}

	if (timeout <= 0) {
		printf("timed out in wait_for_bb: I2C_STAT=%x\n",
		       inw(I2C_STAT));
	}
	outw(0xFFFF, I2C_STAT);	/* clear delayed stuff */
}

static u16 wait_for_pin(void)
{
	u16 status;
	int timeout = 10;

	do {
		udelay(1000);
		status = inw(I2C_STAT);
	} while (!(status &
		   (I2C_STAT_ROVR | I2C_STAT_XUDF | I2C_STAT_XRDY |
		    I2C_STAT_RRDY | I2C_STAT_ARDY | I2C_STAT_NACK |
		    I2C_STAT_AL)) && timeout--);

	if (timeout <= 0) {
		printf("timed out in wait_for_pin: I2C_STAT=%x\n",
		       inw(I2C_STAT));
		outw(0xFFFF, I2C_STAT);
	}
	return status;
}

#endif				/* CONFIG_DRIVER_OMAP24XX_I2C */
