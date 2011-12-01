#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/spi.h>

/*
 * clock parameter 0=48MHZ 1=24MHZ 2=12MHZ 3=6MHZ 4=3MHz
 * below 4 is not recommended to use. Because CS force high may not work,
 * if following function force CS low immediately.
 * for example, if the clock parameter is 7, I call
 * HW_WRITE_WORD function, then call HW_READ_WORD function, the
 * HW_READ_WORD function will fail, because between write and read
 * no CS pull hign then pull down
 */

#define CLOCK_PARAM	1

/* the SPI register structure for omap */
static volatile MCSPI_REGS *gpMCSPIRegs;

/*
 * wait_txs_rxs
 *
 * Description
 *  wait for transmit / receive available
 * Parameters:
 *  int flag (IN): TXS for transmit, RXS for receive
 * Return
 *  0 if successful
 */

static int wait_txs_rxs(int flag)
{
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;
	unsigned long u, i;

	for (i = 0; i < 500; i++) {
		if (flag == TXS)
			u = CSP_BITFEXT(gpMCSPIRegs->CHxSTAT, SPI_CHxSTAT_TXS);
		else
			u = CSP_BITFEXT(gpMCSPIRegs->CHxSTAT, SPI_CHxSTAT_RXS);
		if (u)
			return 0;
	}

	if (flag == TXS)
		printf("Wait TXS time-out\n");
	else
		printf("Wait RXS time-out\n");

	return i;
}

/*
 * SPI_SetMode
 *
 * Description:
 *  Set SPI mode (POL, POA)
 * Parameters:
 *  SPIMODE mode (IN): SPI_MODE0, SPI_MODE1, SPI_MODE2 and SPI_MODE3
 * Return
 *  (None)
 */

static void SPI_SetMode(SPIMODE mode)
{

	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;
	switch (mode) {

	case SPI_MODE0:
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_POL, 0);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_PHA, 0);
		break;

	case SPI_MODE1:
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_POL, 0);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_PHA, 1);
		break;

	case SPI_MODE2:
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_POL, 1);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_PHA, 0);
		break;

	case SPI_MODE3:
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_POL, 1);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_PHA, 1);
		break;

	default:
		break;
	}
}

/*
 * SPI_SetMode
 *
 * Description:
 *  Get SPI mode.
 * Parameters:
 *  None
 * Return
 *  SPIMODE
 *
 */
static SPIMODE SPI_GetMode()
{

	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;
	unsigned long uPOL;
	unsigned long uPHA;

	uPOL = CSP_BITFEXT(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_POL);
	uPHA = CSP_BITFEXT(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_PHA);

	if (uPOL == 0 && uPHA == 0)
		return SPI_MODE0;
	else if (uPOL == 0 && uPHA == 1)
		return SPI_MODE1;
	else if (uPOL == 1 && uPHA == 0)
		return SPI_MODE2;
	else if (uPOL == 1 && uPHA == 1)
		return SPI_MODE3;
	else {
		printf("Error to get mode\n");
		return SPI_MODE0;
	}
}

/*
 * SPI_SetClockDevider
 *
 * Description
 *  Set SPI clock devider
 * Parameters
 *  unsigned long uDevider (IN)
 * Return
 *  None
 *
 */
static void SPI_SetClockDevider(unsigned long uDevider)
{
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;
	if (uDevider > 0x0C)
		return;
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_CLKD, uDevider);
}

/*
 * SPI_GetClockDevider
 *
 * Description:
 *  Set SPI clock devider
 * Parameters:
 *  None
 * Return
 *  SPI clock devider
 */
static unsigned long SPI_GetClockDevider()
{
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;
	return CSP_BITFEXT(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_CLKD);
}

/*
 * spi_init_spi1
 *
 * Description
 *  OMAP SPI1 initialization
 * Parameters
 *  None
 * Return
 *  None
 *
 */

static void spi_init_spi1(void)
{

	unsigned long u;

	/* set multiplexing for set mcspi1_clk */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI1_CLK);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE1, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI1_CLK) = u;

	/* set multiplexing for set mcspi1_simo */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI1_SOMI);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE0, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI1_SOMI) = u;

	/* set multiplexing for mcspi1_somi */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI1_SOMI);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE1, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI1_SOMI) = u;

	/* set multiplexing for mcspi1_cs0 */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI1_CS0);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE0, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI1_CS0) = u;

	/* set multiplexing for mcspi1_cs0 */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI1_CS0);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE1, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI1_CS0) = u;

	/* set SPI1 base address */
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI1_IO_BASE;

	/* Overwrite pin muxing for mcspi1* + gpio48  + gpio 138*/
	sr32(0x4A100130, 0, 32, 0x01100118);
	sr32(0x4A100134, 0, 32, 0x01100110);
	sr32(0x4A100138, 0, 32, 0x01130110);

	/* Output enable && dataout 1 for gpio 48 to power up ethernet */
	sr32(0x48055134, 16, 1, 0);
	sr32(0x4805513C, 16, 1, 1);

	/*
	 * Output enable && dataout 1 for gpio 138
	 * as a workaround to keep CS1 high
	 */
	sr32(0x4805B134, 10, 1, 0);
	sr32(0x4805B13C, 10, 1, 1);
	sr32(0x48098110, 0, 32, 0x00000308);

	udelay(500);

}

/*
 * spi_init_spi4
 *
 * Description
 *  OMAP SPI4 initialization
 * Parameters
 *  None
 * Return
 *  None
 *
 */

static void spi_init_spi4(void)
{

	unsigned long u;
int count=0;
#if 0
	/* set multiplexing for set mcspi4_clk */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI4_CLK);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE1, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI4_CLK) = u;
printf("spi_init_spi4 %d\n", count++);
	/* set multiplexing for mcspi4_simo */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI4_SIMO);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE1, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI4_SIMO) = u;
printf("spi_init_spi4 %d\n", count++);

	/* set multiplexing for mcspi4_somi */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI4_SOMI);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE1, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI4_SOMI) = u;
printf("spi_init_spi4 %d\n", count++);

	/* set multiplexing for mcspi4_cs0 */
	u = MIO_DWORD(CONTROL_PADCONF_MCSPI4_CS0);
	CSP_BITFINS(u, CONTROL_PADCONF_MUXMODE0, 0);
	MIO_DWORD(CONTROL_PADCONF_MCSPI4_CS0) = u;
printf("spi_init_spi4 %d\n", count++);
#endif
	/* set SPI4 base address */
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;
//printf("spi_init_spi4 %d\n", count++);

	/* Overwrite pin muxing for mcspi4 */
	sr32(0x4A100154, 0, 32, 0x01100110);
//printf("spi_init_spi4 %d\n", count++);
	sr32(0x4A100158, 0, 32, 0x01100110);
//printf("spi_init_spi4 %d\n", count++);

	udelay(500);

}

void spi_command(void){
    unsigned int w, mbid, vendor0 = 1, vendor1 = 1;

    mbid = get_mbid();
    //gpi_175
    vendor1 = (__raw_readl(0x4805D138) & 0x8000) >> 15;
    //gpi_176
    vendor0 = (__raw_readl(0x4805D138) & 0x10000) >> 16;
    //printf("LCD vendor1 = %d, vendor0 = %d\n", vendor1, vendor0);

    //Before DVT
    if (mbid < 4) {
        if ((vendor1 == 0) && (vendor0 == 0)) {
            printf("Panel of LG\n");

            w |= (0x00 << 10) | 0x21;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xa5;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x30;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0e << 10) | 0x5f;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0f << 10) | 0xa4;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0d << 10) | 0x00;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x43;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0a << 10) | 0x28;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x10 << 10) | 0x41;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xad;
            spi_otter_reg_write(w);
            w &= 0x0000;
        } else if ((vendor1 == 1) && (vendor0 == 0)) {
            printf("Panel of HYDIS\n");

            w |= (0x00 << 10) | 0x29;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0x25;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x30;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0e << 10) | 0x5f;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0f << 10) | 0xa4;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0d << 10) | 0x01;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0x2d;
            spi_otter_reg_write(w);
            w &= 0x0000;
        } else if ((vendor1 == 0) && (vendor0 == 1)) {
            printf("Panel of CMO\n");

            w |= (0x00 << 10) | 0xad;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x10;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x03 << 10) | 0x04;
            spi_otter_reg_write(w);
            w &= 0x0000;
        } else {
            printf("undefined panel, use default LGD settings\n");

            w |= (0x00 << 10) | 0x21;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xa5;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x30;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0e << 10) | 0x5f;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0f << 10) | 0xa4;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0d << 10) | 0x00;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x43;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0a << 10) | 0x28;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x10 << 10) | 0x41;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xad;
            spi_otter_reg_write(w);
            w &= 0x0000;
        }
    }
    //After DVT
    else {
        if ((vendor1 == 0) && (vendor0 == 0)) {
            printf("Panel of LG\n");

            w |= (0x00 << 10) | 0x21;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xa5;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x30;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0e << 10) | 0x5f;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0f << 10) | 0xa4;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0d << 10) | 0x00;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x43;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0a << 10) | 0x28;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x10 << 10) | 0x41;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xad;
            spi_otter_reg_write(w);
            w &= 0x0000;
        } else if ((vendor1 == 0) && (vendor0 == 1)) {
            printf("Panel of HYDIS\n");

            w |= (0x00 << 10) | 0x29;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0x25;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x30;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0e << 10) | 0x5f;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0f << 10) | 0xa4;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0d << 10) | 0x01;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0x2d;
            spi_otter_reg_write(w);
            w &= 0x0000;
        } else {
            printf("undefined panel, use default LGD settings\n");

            w |= (0x00 << 10) | 0x21;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xa5;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x01 << 10) | 0x30;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x40;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0e << 10) | 0x5f;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0f << 10) | 0xa4;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0d << 10) | 0x00;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x02 << 10) | 0x43;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x0a << 10) | 0x28;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x10 << 10) | 0x41;
            spi_otter_reg_write(w);
            w &= 0x0000;

            w |= (0x00 << 10) | 0xad;
            spi_otter_reg_write(w);
            w &= 0x0000;
        }
    }
}

//U_BOOT_CMD(spi_command, 1, 0, spi_command, "spi_command - LG pannel initialized.\n", NULL);


static u16 lg_lcd[] = {
	0x00ad, //A00 = adh
	0x0430, //A01 = 30h
	0x0840, //A02 = 40h

	0x385f, //A0e = 5fh
	0x3Ca4, //A0f = a4h
	0x3400, //A0d = 00h
	0x0843, //A02 = 43h
	0x2828, //A0a = 28h
	0x4041, //A10 = 41h
};

#if 0
#define CONTROL_PADCONF_MCSPI4_CLK	0x4A100154 //gpio 151
#define CONTROL_PADCONF_MCSPI4_SIMO	0x4a100156 //gpio 152
#define CONTROL_PADCONF_MCSPI4_SOMI	0x4A100158 //gpio 153
#define CONTROL_PADCONF_MCSPI4_CS0	0x4A10015A //gpio 154
#endif
void spi_write(void)
{
	int count = sizeof(lg_lcd);
	int i, j, bitData;
	u16 writeData;

//gpio_151 152
printf("The gpio 151 and gpio 152 value is %8x!\n",__raw_readl(0x4a100154));
__raw_writel(__raw_readl(0x4a100154) & 0x00000000,0x4a100154);
__raw_writel(__raw_readl(0x4a100154) | 0x001B001B,0x4a100154);
printf("The gpio 151 and gpio 152 value is %8x!\n",__raw_readl(0x4a100154));
//gpio_153 154
printf("The gpio 153 and gpio 154 value is %8x!\n",__raw_readl(0x4a100158));
__raw_writel(__raw_readl(0x4a100158) & 0x00000000,0x4a100158);
__raw_writel(__raw_readl(0x4a100158) | 0x001B001B,0x4a100158);
printf("The gpio 153 and gpio 154 value is %8x!\n",__raw_readl(0x4a100158));


__raw_writel(__raw_readl(0x4805B134) & 0xF87FFFFF, 0x4805B134);//gpio_OE gpio_151 152 153 154
printf("The gpio 151 152 153 154 GPIO_OE value is %x!\n",__raw_readl(0x4805B134));
//__raw_writew(__raw_readw(0x4805B13C) | 0x07800000, 0x4805B13C);//gpio_dataout gpio_151
//printf("The gpio 151 152 153 154 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805B13C));


	udelay(10000);

	for (i = 0; i < count; i++)
	{
		writeData = lg_lcd[i];
		printf("writeData=0x%04x\n", writeData);
__raw_writel(__raw_readl(0x4805B13C) & 0xFBFFFFFF, 0x4805B13C);//gpio_dataout gpio_154 MCSPI4_CS 0
//gpio_set_value(gpio_cs, 0);
		for (j = 0; j < 16; j++)
		{
			bitData = (writeData & (0x8000 >> j)) >> (15 - j);
			//printk("%s, j = %d, bitData=%d\n", __func__, j, bitData);

__raw_writel(__raw_readl(0x4805B13C) & 0xFF7FFFFF, 0x4805B13C);//gpio_dataout gpio_151 MCSPI4_CLK 0
//gpio_set_value(gpio_clk, 0);

if(bitData == 0) __raw_writel(__raw_readl(0x4805B13C) & 0xFEFFFFFF, 0x4805B13C);//gpio_dataout gpio_152 MCSPI4_SIMO 0
if(bitData == 1) __raw_writel(__raw_readl(0x4805B13C) | 0x01000000, 0x4805B13C);//gpio_dataout gpio_152 MCSPI4_SIMO 1
//gpio_set_value(gpio_do, bitData);

			udelay(1);

__raw_writel(__raw_readl(0x4805B13C) | 0x00800000, 0x4805B13C);//gpio_dataout gpio_151 MCSPI4_CLK 1
//gpio_set_value(gpio_clk, 1);

			udelay(1);
		}

__raw_writel(__raw_readl(0x4805B13C) | 0x04000000, 0x4805B13C);//gpio_dataout gpio_154 MCSPI4_CS 1
//gpio_set_value(gpio_cs, 1);

__raw_writel(__raw_readl(0x4805B13C) & 0xFEFFFFFF, 0x4805B13C);//gpio_dataout gpio_152 MCSPI4_SIMO
//gpio_set_value(gpio_do, 0);

__raw_writel(__raw_readl(0x4805B13C) & 0xFF7FFFFF, 0x4805B13C);//gpio_dataout gpio_151 MCSPI4_CLK 0
//gpio_set_value(gpio_clk, 0);

		udelay (10);
	}
}
/*
 * spi_init
 *
 * Description
 *  Initialize OMAP SPI interface
 * Parameters
 *  None
 * Return
 *  None
 */
void spi_init(void)
{
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;

	unsigned long u, n;

	/* initialize the multipad and interface clock */
	//spi_init_spi1();
	spi_init_spi4();
//printf("spi_init 1\n");
	/* soft reset */
	CSP_BITFINS(gpMCSPIRegs->SYSCONFIG, SPI_SYSCONFIG_SOFTRESET, 1);
	for (n = 0; n < 100; n++) {
		u = CSP_BITFEXT(gpMCSPIRegs->SYSSTATUS,
				SPI_SYSSTATUS_RESETDONE);
		if (u)
			break;
	}

	/* disable the channel */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 0);

	/* disable all interrupts */
	gpMCSPIRegs->IRQENABLE = 0x0;

	/* clear all interrupt status bits */
	gpMCSPIRegs->IRQSTATUS = 0xFFFFFFFF;

	/* set DPE0 to 1 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_DPE0, 1);

	/* set DPE1 to 0 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_DPE1, 0);

	/* set IS to 0 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_IS, 0);

	/* CS hold low at active state */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_EPOL, 1);

	/* set clock devider
	 * Note: the clock devider must big than 4. otherwise the
	 * CS force high will not work, if following function force
	 * CS low immediately. for example, if the clock devider is 7, I call
	 * HW_WRITE_WORD function, then call HW_READ_WORD function, the
	 * HW_READ_WORD function will fail, because between write and read
	 * no CS pull hign then pull down.
	 */

	SPI_SetClockDevider(CLOCK_PARAM);

	/* set mode 0 */
	SPI_SetMode(SPI_MODE0);

	/*
	 * set SPI word length 16 bit for ks8851SNL
	 * CSP_BITFINS(gpMCSPIRegs->CHxCONF,SPI_CHxCONF_WL,0xF);
	 *
	 * set delay clock
	 * CSP_BITFINS(gpMCSPIRegs->CHxCONF,SPI_CHxCONF_TCS,1);
	 *
	 * set FIFO for transmit
	 * CSP_BITFINS(gpMCSPIRegs->CHxCONF,SPI_CHxCONF_FFEW,1);
	 *
	 * set FIFO for receive
	 * CSP_BITFINS(gpMCSPIRegs->CHxCONF,SPI_CHxCONF_FFER ,1);
	 */

	/*
	 * set module to master
	 * drive CS to high
	 */
	CSP_BITFINS(gpMCSPIRegs->MODULECTRL, SPI_MODULCTRL_MS, 0);
	CSP_BITFINS(gpMCSPIRegs->MODULECTRL, SPI_MODULCTRL_SINGLE, 1);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 0);
}


/*
 * spi_otter_reg_write
 *
 * Description
 *  Write value to ks8851 register
 *
 * Parameters
 *  unsigned short value (IN)
 *  value to write
 * Return
 *  None
 */

void spi_otter_reg_write(unsigned int value)
{

	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI4_IO_BASE;

	//printf("reg_write, value=0x%4x\n", value);
	/*
	 * use 32 bit length to do transmit
	 * set transmit mode only
	 * drive CS to low
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_WL, 0xF);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 2);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 1);

	/* enable transmit */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 1);
	if (wait_txs_rxs(TXS) != 0)
		return;

	/*
	 * Because the data clock out from register high byte
	 * so we need to change the order here
	 */
	gpMCSPIRegs->TXx = value;
	if (wait_txs_rxs(TXS) != 0)
		return;

	/*
	 * disable channel
	 * driver CS to high
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 0);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 0);
}


/*
 * spi_ks8851snl_reg_write
 *
 * Description
 *  Write value to ks8851 register
 *
 * Parameters
 *  unsigned short write_cmd (IN)
 *  ks8851snl slave drvice register write command
 *  unsigned short value (IN)
 *  value to write
 * Return
 *  None
 */

void spi_ks8851snl_reg_write(unsigned short write_cmd, unsigned short value)
{

	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI1_IO_BASE;

#ifdef DEBUG
	printf("reg_write, cmd=0x%04x value=0x%4x\n", write_cmd, value);
#endif

	/*
	 * use 32 bit length to do transmit
	 * set transmit mode only
	 * drive CS to low
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_WL, 0xF);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 2);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 1);

	/* enable transmit */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 1);
	if (wait_txs_rxs(TXS) != 0)
		return;

	gpMCSPIRegs->TXx = write_cmd;
	if (wait_txs_rxs(TXS) != 0)
		return;

	/*
	 * Because the data clock out from register high byte
	 * so we need to change the order here
	 */
	gpMCSPIRegs->TXx = ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
	if (wait_txs_rxs(TXS) != 0)
		return;

	/*
	 * disable channel
	 * driver CS to high
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 0);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 0);
}

/*
 * spi_ks8851snl_reg_read
 *
 * Description
 *  Read the ks8851 register value
 * Parameters
 *  unsigned short read_cmd (IN): ks8851snl slave drvice register read command
 * Return
 *  read value
 */
unsigned short spi_ks8851snl_reg_read(unsigned short read_cmd)
{
	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI1_IO_BASE;

	unsigned long u;
	unsigned short ush;

	/*
	 * set 16 bit transmit length
	 * set transit mode only
	 * drive CS to low
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_WL, 0xF);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 2);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 1);

	/* enable channel and do transmit */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 1);
	if (wait_txs_rxs(TXS) != 0)
		return 0;

	gpMCSPIRegs->TXx = read_cmd;
	if (wait_txs_rxs(TXS) != 0)
		return 0;

	/* change to receive mode only */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 1);
	if (wait_txs_rxs(RXS) != 0)
		return 0;

	/*
	 * When clock rate less than 12MHZ.
	 * By using 16 bit read, the first read is a dummy data
	 * I do not know why???, this is a experiment result.
	 */

#if (CLOCK_PARAM > 2)
	/* this is dummy data, we need to through away */
	u = gpMCSPIRegs->RXx;
#ifdef DEBUG
	printf("reg_read first data, dummy data = 0x%x\n", u);
#endif
	if (wait_txs_rxs(RXS) != 0)
		return 0;
#endif

	/* this is real data */
	u = gpMCSPIRegs->RXx;
#ifdef DEBUG
	printf("reg_read second data, real data = 0x%x\n", u);
#endif
	/*
	 * when datd clock in, the low byte will be shift to rigster
	 * high order because it is litter endian; we need to change
	 * the order
	 */
	ush = ((u & 0xFF) << 8) | ((u >> 8) & 0xFF);

	/*
	 * disable channel first
	 * drive CS to high
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 0);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 0);

#ifdef DEBUG
	printf("reg_read, cmd=0x%04x value=0x%4x\n", read_cmd, ush);
#endif
	return ush;
}

/*
 * spi_ks8851snl_data_read
 *
 * Description
 *  Read ks8851 QMU data
 * Parameters
 *  unsigned long read_cmd (IN): ks8851snl QMU read commond
 *  unsigned long cmdLen (IN)  : ks8851snl QMU read commond length in byte
 *  unsigned char * pBuf (IN)  : Point to a receive buffer, this buffer
 *				 should can hold all read data
 *  unsigned long readLen (IN) : How may data in byte need to be read
 * Return
 *  the number of bytes read
 */

unsigned long spi_ks8851snl_data_read(unsigned long  readCmd,
				unsigned long cmdLen, unsigned char *pBuf,
				unsigned long readLen)
{

	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI1_IO_BASE;

	/* for ks8851 the read data cmd is  1 byte long */
	unsigned long u, i;
	unsigned char *pb;
	unsigned long *pMove = (unsigned long *)pBuf;

#ifdef DEBUG
	static unsigned char dbgBuf[2048];
	unsigned long *pul = (unsigned long *)&dbgBuf[0];
	unsigned long *pulBegin = pul;
#endif

	/*
	 * set 8 bit transmit length
	 * set transmit mode only
	 * drive CS to low
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_WL, 0x7);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 2);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 1);

	/* enable channel and do transmit */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 1);
	if (wait_txs_rxs(TXS) != 0)
		return 0;

	/* the ks8851 QMU read command is 1 byte long */
	gpMCSPIRegs->TXx = readCmd & 0xFF;
	if (wait_txs_rxs(TXS) != 0)
		return 0;

	/*
	 * set receive mode only
	 * because KS8851 QMU data read length is always 32 bit alignment
	 * so we use 32 bit read
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_WL, 0x1F);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 1);

	/*
	 * When clock rate less than 6MHZ. By using 32 bit read
	 * the first read is a dummy data
	 * I do not know why???, this is a experiment result.
	 * If you using 8 bit read, the first 2 read are dummy data.
	 * It also is a experiment result.
	 */

#if (CLOCK_PARAM > 3)
	/* this is dummy data, we need to through away */
	u = gpMCSPIRegs->RXx;
	if (wait_txs_rxs(RXS) != 0)
		return 0;
#endif

#ifdef DEBUG
	*pul++ = u;
#endif

	for (i = 0; i < readLen / 4; i++) {
		if (wait_txs_rxs(RXS) != 0)
			break;;
		u = gpMCSPIRegs->RXx;
#ifdef DEBUG
		*(pul++) = u;
#endif
		pb = (unsigned char *)pMove;
		*(pb + 3) = u & 0xFF;
		*(pb + 2) = (u >> 8) & 0xFF;
		*(pb + 1) = (u >> 16) & 0xFF;
		*(pb) = (u >> 24) & 0xFF;
		pMove++;
}

	/*
	 * disable channel
	 * drive CS to high
	 */
	CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 0);
	CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 0);

#ifdef DEBUG
	i = 0;
	while (pulBegin < pul) {
		if (i % 8 == 0 && i != 0)
			printf("%08x \n", *pulBegin);
		else
			printf("%08x ", *pulBegin);
		pulBegin++;
		i++;
	}
#endif
	return i * 4;
}

/*
 * spi_ks8851snl_data_write
 *
 * Description:
 *  Write data to ks8851 QMU
 *
 * Parameters:
 *  unsigned char * pdata (IN)	: Point to a data buffer.
 *  unsigned long len (IN)	: Write data length.
 *  int fBegin (IN)		: Tell the function, this is a begin of
 *				  ethernet frame.
 *  int fEnd (IN)		: Tell the function, this is a end of
 *				  ethernet frame.
 *
 * Note: Because user may call this function several times to complete
 * a Ethernet frams transmit. So we need to know when is first call for
 * beginning a frame tansmit and when is a last call for frame end.
 *
 * Return
 *  The number of bytes write
 */

unsigned long spi_ks8851snl_data_write(unsigned char *pdata, unsigned long len,
					int fBegin, int fEnd)
{

	gpMCSPIRegs = (MCSPI_REGS *)MCSPI_SPI1_IO_BASE;
	unsigned long i;

	if (fBegin) {

		/*
		 * set 8 bit transmit length
		 * set transmit mode only
		 * drive CS to low
		 */
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_WL, 0x7);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_TRM, 2);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 1);

		/* enable transmit */
		CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 1);
	}

	for (i = 0; i < len; i++) {
		gpMCSPIRegs->TXx = *(pdata + i);
		if (wait_txs_rxs(TXS))
			return 0;
	}
#if 0
	if (wait_txs_rxs(TXS))
		return 0;
#endif

	if (fEnd) {
		/*
		 * disable channel
		 * driver CS to high
		 */
		CSP_BITFINS(gpMCSPIRegs->CHxCTRL, SPI_CHxCTRL_EN, 0);
		CSP_BITFINS(gpMCSPIRegs->CHxCONF, SPI_CHxCONF_FORCE, 0);
	}

	return len;
}

