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
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/bits.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <asm/arch/mux.h>
#include <asm/arch/clocks.h>
#ifdef CONFIG_OMAP3430
#include <asm/arch/led.h>
#endif
#include <environment.h>
#include <command.h>
#include <usbdcore.h>
#include "usbomap.h"
#include <fastboot.h>
#include <twl4030.h>
#include <twl6030.h>

#include <asm/io.h>
#include <i2c.h>

#if defined(CONFIG_FASTBOOT)

#define OTG_INTERFSEL 0x4A0AB40C
#define USBOTGHS_CONTROL 0x4A00233C
#define OTG_SYSCONFIG 0x4A0AB404
#define OTG_SYSSTATUS 0x4A0AB408

unsigned int fastboot_wait_power_button_abort = 0;

#include "usb_debug_macros.h"

#define CONFUSED() printf ("How did we get here %s %d ? \n", __FILE__, __LINE__)

/* memory mapped registers */
static volatile	u8  *pwr        = (volatile u8  *) OMAP34XX_USB_POWER;
static volatile u16 *csr0       = (volatile u16 *) OMAP34XX_USB_CSR0;
static volatile u8  *index      = (volatile u8  *) OMAP34XX_USB_INDEX;
static volatile u8  *txfifosz   = (volatile u8  *) OMAP34XX_USB_TXFIFOSZ;
static volatile u8  *rxfifosz   = (volatile u8  *) OMAP34XX_USB_RXFIFOSZ;
static volatile u16 *txfifoadd  = (volatile u16 *) OMAP34XX_USB_TXFIFOADD;
static volatile u16 *rxfifoadd  = (volatile u16 *) OMAP34XX_USB_RXFIFOADD;

#define BULK_ENDPOINT 1
static volatile u16 *peri_rxcsr = (volatile u16 *) OMAP34XX_USB_RXCSR(BULK_ENDPOINT);
static volatile u16 *rxmaxp     = (volatile u16 *) OMAP34XX_USB_RXMAXP(BULK_ENDPOINT);
static volatile u16 *rxcount    = (volatile u16 *) OMAP34XX_USB_RXCOUNT(BULK_ENDPOINT);
static volatile u16 *peri_txcsr = (volatile u16 *) OMAP34XX_USB_TXCSR(BULK_ENDPOINT);
static volatile u16 *txmaxp     = (volatile u16 *) OMAP34XX_USB_TXMAXP(BULK_ENDPOINT);
static volatile u8  *bulk_fifo  = (volatile u8  *) OMAP34XX_USB_FIFO(BULK_ENDPOINT);

static volatile u32 *otg_sysconfig = (volatile u32  *)OTG_SYSCONFIG;
static volatile u32 *otg_interfsel = (volatile u32  *)OTG_INTERFSEL;
static volatile u32 *otghs_control = (volatile u32  *)USBOTGHS_CONTROL;
static volatile u32 *otg_sysstatus = (volatile u32  *)OTG_SYSSTATUS;
#define DMA_CHANNEL 1
static volatile u8  *peri_dma_intr	= (volatile u8  *) OMAP_USB_DMA_INTR;
static volatile u16 *peri_dma_cntl	= (volatile u16 *) OMAP_USB_DMA_CNTL_CH(DMA_CHANNEL);
static volatile u32 *peri_dma_addr	= (volatile u32 *) OMAP_USB_DMA_ADDR_CH(DMA_CHANNEL);
static volatile u32 *peri_dma_count	= (volatile u32 *) OMAP_USB_DMA_COUNT_CH(DMA_CHANNEL);

/* This is the TI USB vendor id */
//#define DEVICE_VENDOR_ID  0x0451
/* This is the Lab USB vendor id */
//#define DEVICE_VENDOR_ID  0x1949
/* This is the Google USB vendor id.  30 fastboots agree. */
#define DEVICE_VENDOR_ID	0x18d1
/* This is just made up.. */
//#define DEVICE_PRODUCT_ID 0xD022
//#define DEVICE_PRODUCT_ID 0x0006
#define DEVICE_PRODUCT_ID 0x0100

#define DEVICE_BCD        0x0100;

/* This is used to get the serial number */
#if defined(CONFIG_4430SDP) || defined(CONFIG_4430PANDA)
#define DIE_ID_REG_BASE		(OMAP44XX_L4_IO_BASE + 0x2000)
#define DIE_ID_REG_OFFSET		0x200
#define MAX_USB_SERIAL_NUM              17
#endif

/* String 0 is the language id */
#define DEVICE_STRING_PRODUCT_INDEX       1
#define DEVICE_STRING_SERIAL_NUMBER_INDEX 2
#define DEVICE_STRING_CONFIG_INDEX        3
#define DEVICE_STRING_INTERFACE_INDEX     4
#define DEVICE_STRING_MANUFACTURER_INDEX  5
#define DEVICE_STRING_PROC_REVISION       6
#define DEVICE_STRING_PROC_TYPE           7
#define DEVICE_STRING_MAX_INDEX           DEVICE_STRING_PROC_TYPE
#define DEVICE_STRING_LANGUAGE_ID         0x0409 /* English (United States) */

/* Define this to use 1.1 / fullspeed */
/* #define CONFIG_USB_1_1_DEVICE */

/* In high speed mode packets are 512
   In full speed mode packets are 64 */
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0  (0x0200)
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1  (0x0040)
#define TX_ENDPOINT_MAXIMUM_PACKET_SIZE      (0x0200)

/* Same, just repackaged as 
   2^(m+3), 64 = 2^6, m = 3 */
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS_2_0 (6)
#define RX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS_1_1 (3)
#define TX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS (6)

#define CONFIGURATION_NORMAL      1

#define TX_LAST()						\
	*csr0 |= (MUSB_CSR0_TXPKTRDY | MUSB_CSR0_P_DATAEND);	\
	while  (*csr0 & MUSB_CSR0_RXPKTRDY)			\
		udelay(1);			      

#define NAK_REQ() *csr0 |= MUSB_CSR0_P_SENDSTALL
#define ACK_REQ() *csr0 |= MUSB_CSR0_P_DATAEND

#define ACK_RX()  *peri_rxcsr |= (MUSB_CSR0_P_SVDRXPKTRDY | MUSB_CSR0_P_DATAEND)

static u8 fastboot_fifo[MUSB_EP0_FIFOSIZE];
static u16 fastboot_fifo_used = 0;

static unsigned int set_address = 0;
static u8 faddr = 0xff;
static unsigned int current_config = 0;

static unsigned int high_speed = 1;

static unsigned int deferred_rx = 0;

static struct usb_device_request req;

/* The packet size is dependend of the speed mode
   In high speed mode packets are 512
   In full speed mode packets are 64
   Set to maximum of 512 */
/* Note: The start address (written to the MUSB_DMA_ADDR_CH(n) register)
   must be word aligned */
static u8 fastboot_bulk_fifo[0x0200] __attribute__ ((aligned(0x4)));

static char *device_strings[DEVICE_STRING_MAX_INDEX+1];

static struct cmd_fastboot_interface *fastboot_interface = NULL;
#ifdef DEBUG_FASTBOOT
static void fastboot_db_regs(void) 
{
	printf("fastboot_db_regs\n");
	u8 b;
	u16 s;

	/* */
	b = inb (OMAP34XX_USB_FADDR);
	printf ("\tfaddr   0x%2.2x\n", b);

	b = inb (OMAP34XX_USB_POWER);
	PRINT_PWR(b);

	s = inw (OMAP34XX_USB_CSR0);
	PRINT_CSR0(s);

	b = inb (OMAP34XX_USB_DEVCTL);
	PRINT_DEVCTL(b);

	b = inb (OMAP34XX_USB_CONFIGDATA);
	PRINT_CONFIG(b);

	s = inw (OMAP34XX_USB_FRAME);
	printf ("\tframe   0x%4.4x\n", s);
	b = inb (OMAP34XX_USB_INDEX);
	printf ("\tindex   0x%2.2x\n", b);

	s = *rxmaxp;
	PRINT_RXMAXP(s);

	s = *peri_rxcsr;
	PRINT_RXCSR(s);

	s = *txmaxp;
	PRINT_TXMAXP(s);

	s = *peri_txcsr;
	PRINT_TXCSR(s);
}
#endif

static void fastboot_bulk_endpoint_reset (void)
{
	u8 old_index;
	/* save old index */
	old_index = *index;

	*index = 0;
	*rxfifoadd = 0;
	*txfifoadd = 0;
	*rxfifosz = 3; /* 64 bytes */
	*txfifosz = 3;

	/* set index to endpoint */
	*index = BULK_ENDPOINT;

	/* Address starts at the end of EP0 fifo, shifted right 3 (8 bytes) */
	*txfifoadd = MUSB_EP0_FIFOSIZE >> 3;
	*rxfifoadd = (MUSB_EP0_FIFOSIZE + 512) >> 3;

	/* Size depends on the mode.  Do not double buffer */
	*txfifosz = TX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS;
	/*
	 * Double buffer the rx fifo because it handles the large transfers
	 * The extent is now double and must be considered if another fifo is
	 * added to the end of this one.
	 */
#if defined(CONFIG_4430PANDA)
	if (high_speed)
		*rxfifosz =
			RX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS_2_0;
	else
		*rxfifosz =
			RX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS_1_1;
#else
	if (high_speed)
		*rxfifosz =
			RX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS_2_0 |
			MUSB_RXFIFOSZ_DPB;
	else
		*rxfifosz =
			RX_ENDPOINT_MAXIMUM_PACKET_SIZE_BITS_1_1 |
			MUSB_RXFIFOSZ_DPB;
#endif

	/* restore index */
	*index = old_index;

	/* Setup Rx endpoint for Bulk OUT */
	*rxmaxp = fastboot_fifo_size();

	/* Flush anything on fifo */
	while (*peri_rxcsr & MUSB_RXCSR_RXPKTRDY)
	{
		*peri_rxcsr |= MUSB_RXCSR_FLUSHFIFO;
		udelay(1);
	}
	/* No dma, enable bulkout,  */
	*peri_rxcsr &= ~(MUSB_RXCSR_DMAENAB | MUSB_RXCSR_P_ISO);
	/* reset endpoint data */
	*peri_rxcsr |= MUSB_RXCSR_CLRDATATOG;

	/* Setup Tx endpoint for Bulk IN */
	/* Set max packet size per usb 1.1 / 2.0 */
	*txmaxp = TX_ENDPOINT_MAXIMUM_PACKET_SIZE;

	/* Flush anything on fifo */
	while (*peri_txcsr & MUSB_TXCSR_FIFONOTEMPTY)
	{
		*peri_txcsr |= MUSB_TXCSR_FLUSHFIFO;
		udelay(1);
	}

	/* No dma, enable bulkout, no underflow */
	*peri_txcsr &= ~(MUSB_TXCSR_DMAENAB | MUSB_TXCSR_P_ISO | MUSB_TXCSR_P_UNDERRUN);
	/* reset endpoint data, shared fifo with rx */
	*peri_txcsr |= (MUSB_TXCSR_CLRDATATOG | MUSB_TXCSR_MODE);
}

static void fastboot_reset (void)
{
#ifdef CONFIG_OMAP3430
	OMAP3_LED_ERROR_ON ();
#endif
	/* Kill the power */
	*pwr &= ~MUSB_POWER_SOFTCONN;
	udelay(2 * 500000); /* 1 sec */

#ifdef CONFIG_OMAP3430
	OMAP3_LED_ERROR_OFF ();
#endif

	/* Reset address */
	faddr = 0xff;

	/* Reset */
#ifdef CONFIG_USB_1_1_DEVICE
	*pwr &= ~MUSB_POWER_HSENAB;
	*pwr |= MUSB_POWER_SOFTCONN;
#else
	*pwr |= (MUSB_POWER_SOFTCONN | MUSB_POWER_HSENAB);
#endif
	/* Bulk endpoint fifo */
	fastboot_bulk_endpoint_reset ();
	
#ifdef CONFIG_OMAP3430
	OMAP3_LED_ERROR_ON ();
#endif
	/* fastboot_db_regs(); */
}

static u8 read_fifo_8(void)
{
	u8 val;
  
	val = inb (OMAP34XX_USB_FIFO_0);
	return val;
}

static u8 read_bulk_fifo_8(void)
{
	u8 val;
  
	val = *bulk_fifo;
	return val;
}

static int read_bulk_fifo_dma(u8 *buf, u32 size)
{
	int ret = 0;

	*peri_dma_cntl = 0;

	/* Set the address */
	*peri_dma_addr = (u32) buf;
	/* Set the transfer size */
	*peri_dma_count = size;
	/*
	 * Set the control parts,
	 * The size is either going to be 64 or 512 which
	 * is ok for burst mode 3 which does increment by 16.
	 */
	*peri_dma_cntl =
		MUSB_DMA_CNTL_BUSRT_MODE_3 |
		MUSB_DMA_CNTL_END_POINT(BULK_ENDPOINT) |
		MUSB_DMA_CNTL_WRITE;

	*peri_dma_cntl |= MUSB_DMA_CNTL_ENABLE;

	while (1) {

		if (MUSB_DMA_CNTL_ERR & *peri_dma_cntl) {
			ret = 1;
			break;
		}

		if (0 == *peri_dma_count)
			break;
	}

	return ret;
}

static void write_fifo_8(u8 val)
{
	outb (val, OMAP34XX_USB_FIFO_0);
}

static void write_bulk_fifo_8(u8 val)
{
	*bulk_fifo = val;
}

static void read_request(void)
{
	int i;
  
	for (i = 0; i < 8; i++)
		fastboot_fifo[i] = read_fifo_8 ();
	memcpy (&req, &fastboot_fifo[0], 8);
	fastboot_fifo_used = 0;

	*csr0 |= MUSB_CSR0_P_SVDRXPKTRDY;

	while  (*csr0 & MUSB_CSR0_RXPKTRDY)
		udelay(1);

}

static int do_usb_req_set_interface(void)
{
	int ret = 0;

	/* Only support interface 0, alternate 0 */
	if ((0 == req.wIndex) &&
	    (0 == req.wValue))
	{
		fastboot_bulk_endpoint_reset ();
		ACK_REQ();
	}
	else
	{
		NAK_REQ();
	}

	return ret;
}

static int do_usb_req_set_address(void)
{
	int ret = 0;
  
	if (0xff == faddr) 
	{
		faddr = (u8) (req.wValue & 0x7f);
		set_address = 1;

		/* Check if we are in high speed mode */
		if (*pwr & MUSB_POWER_HSMODE)
		  high_speed = 1;
		else
		  high_speed = 0;

		ACK_REQ();
	}
	else
	{
		NAK_REQ();
	}

	return ret;
}


static int do_usb_req_set_configuration(void)
{
	int ret = 0;

	if (0xff == faddr) {
		NAK_REQ(); 
	} else {
		if (0 == req.wValue) {
			/* spec says to go to address state.. */
			faddr = 0xff;
			current_config = req.wValue;
			ACK_REQ();
		} else if (CONFIGURATION_NORMAL == req.wValue) {
			/* This is the one! */
			/* Bulk endpoint fifo */
			fastboot_bulk_endpoint_reset();
			current_config = req.wValue;

			ACK_REQ();
		} else {
			/* Only support 1 configuration so nak anything else */
			NAK_REQ();
		}
	}

	return ret;
}

static int do_usb_req_set_feature(void)
{
	int ret = 0;
  
	NAK_REQ();

	return ret;
}

static int do_usb_req_get_descriptor(void)
{
	int ret = 0;
  
	if (0 == req.wLength)
	{
		ACK_REQ();
	}
	else
	{
		unsigned int byteLoop;

		if (USB_DT_DEVICE == (req.wValue >> 8))
		{
			struct usb_device_descriptor d;
			d.bLength = MIN(req.wLength, sizeof (d));
	      
			d.bDescriptorType    = USB_DT_DEVICE;
#ifdef CONFIG_USB_1_1_DEVICE
			d.bcdUSB             = 0x110;
#else
			d.bcdUSB             = 0x200;
#endif
			d.bDeviceClass       = 0x00;
			d.bDeviceSubClass    = 0x00;
			d.bDeviceProtocol    = 0x00;
			d.bMaxPacketSize0    = 0x40;
			d.idVendor           = DEVICE_VENDOR_ID;
			d.idProduct          = DEVICE_PRODUCT_ID;
			d.bcdDevice          = DEVICE_BCD;
			d.iManufacturer      = DEVICE_STRING_MANUFACTURER_INDEX;
			d.iProduct           = DEVICE_STRING_PRODUCT_INDEX;
			d.iSerialNumber      = DEVICE_STRING_SERIAL_NUMBER_INDEX;
			d.bNumConfigurations = 1;
	  
			memcpy (&fastboot_fifo, &d, d.bLength);
			for (byteLoop = 0; byteLoop < d.bLength; byteLoop++) 
				write_fifo_8 (fastboot_fifo[byteLoop]);

			TX_LAST();
		}
		else if (USB_DT_CONFIG == (req.wValue >> 8))
		{
			struct usb_configuration_descriptor c;
			struct usb_interface_descriptor i;
			struct usb_endpoint_descriptor e1, e2;
			unsigned char bytes_remaining = req.wLength;
			unsigned char bytes_total = 0;

			c.bLength             = MIN(bytes_remaining, sizeof (c));
			c.bDescriptorType     = USB_DT_CONFIG;
			/* Set this to the total we want */
			c.wTotalLength = sizeof (c) + sizeof (i) + sizeof (e1) + sizeof (e2); 
			c.bNumInterfaces      = 1;
			c.bConfigurationValue = CONFIGURATION_NORMAL;
			c.iConfiguration      = DEVICE_STRING_CONFIG_INDEX;
			c.bmAttributes        = 0xc0;
			//c.bMaxPower           = 0x32;
          c.bMaxPower           = 0xfa;
			bytes_remaining -= c.bLength;
			memcpy (&fastboot_fifo[0], &c, c.bLength);
			bytes_total += c.bLength;
	  
			i.bLength             = MIN (bytes_remaining, sizeof(i));
			i.bDescriptorType     = USB_DT_INTERFACE;	
			i.bInterfaceNumber    = 0x00;
			i.bAlternateSetting   = 0x00;
			i.bNumEndpoints       = 0x02;
			i.bInterfaceClass     = FASTBOOT_INTERFACE_CLASS;
			i.bInterfaceSubClass  = FASTBOOT_INTERFACE_SUB_CLASS;
			i.bInterfaceProtocol  = FASTBOOT_INTERFACE_PROTOCOL;
			i.iInterface          = DEVICE_STRING_INTERFACE_INDEX;

			bytes_remaining -= i.bLength;
			memcpy (&fastboot_fifo[bytes_total], &i, i.bLength);
			bytes_total += i.bLength;

			e1.bLength            = MIN (bytes_remaining, sizeof (e1));
			e1.bDescriptorType    = USB_DT_ENDPOINT;
			e1.bEndpointAddress   = 0x80 | BULK_ENDPOINT; /* IN */
			e1.bmAttributes       = USB_ENDPOINT_XFER_BULK;
			e1.wMaxPacketSize     = TX_ENDPOINT_MAXIMUM_PACKET_SIZE;
			e1.bInterval          = 0x00;

			bytes_remaining -= e1.bLength;
			memcpy (&fastboot_fifo[bytes_total], &e1, e1.bLength);
			bytes_total += e1.bLength;

			e2.bLength            = MIN (bytes_remaining, sizeof (e2));
			e2.bDescriptorType    = USB_DT_ENDPOINT;
			e2.bEndpointAddress   = BULK_ENDPOINT; /* OUT */
			e2.bmAttributes       = USB_ENDPOINT_XFER_BULK;
			if (high_speed)
				e2.wMaxPacketSize = RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0;
			else
				e2.wMaxPacketSize = RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1;
			e2.bInterval          = 0x00;

			bytes_remaining -= e2.bLength;
			memcpy (&fastboot_fifo[bytes_total], &e2, e2.bLength);
			bytes_total += e2.bLength;

			for (byteLoop = 0; byteLoop < bytes_total; byteLoop++) 
				write_fifo_8 (fastboot_fifo[byteLoop]);

			TX_LAST();
		}
		else if (USB_DT_STRING == (req.wValue >> 8))
		{
			unsigned char bLength;
			unsigned char string_index = req.wValue & 0xff;
	  
			if (string_index > DEVICE_STRING_MAX_INDEX)
			{
				/* Windows XP asks for an invalid string index. 
				   Fail silently instead of doing
				
				   NAK_REQ(); 
				*/
			}
			else if (0 == string_index) 
			{
				/* Language ID */
				bLength = MIN(4, req.wLength);
		  
				fastboot_fifo[0] = bLength;        /* length */
				fastboot_fifo[1] = USB_DT_STRING;  /* descriptor = string */
				fastboot_fifo[2] = DEVICE_STRING_LANGUAGE_ID & 0xff;
				fastboot_fifo[3] = DEVICE_STRING_LANGUAGE_ID >> 8;

				for (byteLoop = 0; byteLoop < bLength; byteLoop++) 
					write_fifo_8 (fastboot_fifo[byteLoop]);

				TX_LAST();
			}
			else
			{
				/* Size of string in chars */
				unsigned char s;
				unsigned char sl = strlen (&device_strings[string_index][0]);
				/* Size of descriptor
				   1    : header
				   2    : type
				   2*sl : string */
				unsigned char bLength = 2 + (2 * sl);
				unsigned char numbytes_to_send;

				numbytes_to_send = MIN(bLength, req.wLength);

				fastboot_fifo[0] = bLength;        /* length */
				fastboot_fifo[1] = USB_DT_STRING;  /* descriptor = string */
	      
				/* Copy device string to fifo, expand to simple unicode */
				for (s = 0; s < sl; s++)
				{
					fastboot_fifo[2+ 2*s + 0] = device_strings[string_index][s];
					fastboot_fifo[2+ 2*s + 1] = 0;
				}

				for (byteLoop = 0; byteLoop < numbytes_to_send; byteLoop++)
					write_fifo_8 (fastboot_fifo[byteLoop]);

				TX_LAST();
			}
		} else if (USB_DT_DEVICE_QUALIFIER == (req.wValue >> 8)) {

#ifdef CONFIG_USB_1_1_DEVICE
			/* This is an invalid request for usb 1.1, nak it */
			NAK_REQ();
#else
			struct usb_qualifier_descriptor d;
			d.bLength = MIN(req.wLength, sizeof(d));
			d.bDescriptorType    = USB_DT_DEVICE_QUALIFIER;
			d.bcdUSB             = 0x200;
			d.bDeviceClass       = 0xff;
			d.bDeviceSubClass    = 0xff;
			d.bDeviceProtocol    = 0xff;
			d.bMaxPacketSize0    = 0x40;
			d.bNumConfigurations = 1;
			d.bRESERVED          = 0;

			memcpy(&fastboot_fifo, &d, d.bLength);
			for (byteLoop = 0; byteLoop < d.bLength; byteLoop++)
				write_fifo_8(fastboot_fifo[byteLoop]);

			TX_LAST();
#endif
		}
		else
		{
			NAK_REQ();
		}
	}
  
	return ret;
}

static int do_usb_req_get_configuration(void)
{
	int ret = 0;

	if (0 == req.wLength) {
		printf ("Get config with length 0 is unexpected\n");
		NAK_REQ();
	} else {
		write_fifo_8 (current_config);
		TX_LAST();
	}

	return ret;
}

static int do_usb_req_get_status(void)
{
	int ret = 0;

	if (0 == req.wLength)
	{
		ACK_REQ();
	}
	else
	{
		/* See 9.4.5 */
		unsigned int byteLoop;
		unsigned char bLength;

		bLength = MIN (req.wValue, 2);
      
		fastboot_fifo[0] = USB_STATUS_SELFPOWERED;
		fastboot_fifo[1] = 0;

		for (byteLoop = 0; byteLoop < bLength; byteLoop++) 
			write_fifo_8 (fastboot_fifo[byteLoop]);

		TX_LAST();
	}
  
	return ret;
}

static int fastboot_poll_h (void)
{
	int ret = 0;
	u16 count0;

	if (*csr0 & MUSB_CSR0_RXPKTRDY) 
	{
		count0 = inw (OMAP34XX_USB_COUNT0);

		if (count0 != 8) 
		{
#ifdef CONFIG_OMAP3430
			OMAP3_LED_ERROR_ON ();
#endif
			CONFUSED();
			ret = 1;
		}
		else
		{
			read_request();

			/* Check data */
			if (USB_REQ_TYPE_STANDARD == (req.bmRequestType & USB_REQ_TYPE_MASK))
			{
				/* standard */
				if (0 == (req.bmRequestType & USB_REQ_DIRECTION_MASK))
				{
					/* host-to-device */
					if (USB_RECIP_DEVICE == (req.bmRequestType & USB_REQ_RECIPIENT_MASK))
					{
						/* device */
						switch (req.bRequest)
						{
						case USB_REQ_SET_ADDRESS:
							ret = do_usb_req_set_address();
							break;

						case USB_REQ_SET_FEATURE:
							ret = do_usb_req_set_feature();
							break;

						case USB_REQ_SET_CONFIGURATION:
							ret = do_usb_req_set_configuration();
							break;
			  
						default:
							NAK_REQ();
							ret = -1;
							break;
						}
					}
					else if (USB_RECIP_INTERFACE == (req.bmRequestType & USB_REQ_RECIPIENT_MASK))
					{
						switch (req.bRequest)
						{
						case USB_REQ_SET_INTERFACE:
							ret = do_usb_req_set_interface();
							break;
			      
						default:
							NAK_REQ();
							ret = -1;
							break;
						}
					}
					else if (USB_RECIP_ENDPOINT == (req.bmRequestType & USB_REQ_RECIPIENT_MASK))
					{
						switch (req.bRequest)
						{
						case USB_REQ_CLEAR_FEATURE:
							ACK_REQ();
							ret = 0;
							break;

						default:
							NAK_REQ();
							ret = -1;
							break;
						}
					}
					else
					{
						NAK_REQ();
						ret = -1;
					}
				}
				else
				{
					/* device-to-host */
					if (USB_RECIP_DEVICE == (req.bmRequestType & USB_REQ_RECIPIENT_MASK))
					{
						switch (req.bRequest)
						{
						case USB_REQ_GET_DESCRIPTOR:
							ret = do_usb_req_get_descriptor();
							break;

						case USB_REQ_GET_STATUS:
							ret = do_usb_req_get_status();
							break;

						case USB_REQ_GET_CONFIGURATION:
							ret = do_usb_req_get_configuration();
							break;

						default:
							NAK_REQ();
							ret = -1;
							break;
						}
					}
					else
					{
						NAK_REQ();
						ret = -1;
					}
				}
			}
			else
			{
				/* Non-Standard Req */
				NAK_REQ();
				ret = -1;
			}
		}
		if (0 > ret)
		{
			printf ("Unhandled req\n");
			PRINT_REQ (req);
		}
	}
  
	return ret;
}

static int fastboot_resume (void)
{
	/* Here because of stall was sent */
	if (*csr0 & MUSB_CSR0_P_SENTSTALL)
	{
		*csr0 &= ~MUSB_CSR0_P_SENTSTALL;
		return 0;
	}

	/* Host stopped last transaction */
	if (*csr0 & MUSB_CSR0_P_SETUPEND)
	{
		/* This should be enough .. */
		*csr0 |= MUSB_CSR0_P_SVDSETUPEND;

#if 0
		if (0xff != faddr)
			fastboot_reset ();

		/* Let the cmd layer to reset */
		if (fastboot_interface &&
		    fastboot_interface->reset_handler) 
		  {
			  fastboot_interface->reset_handler();
		  }

		/* If we were not resetting, dropping through and handling the
		   poll would be fine.  As it is returning now is the 
		   right thing to do here.  */
		return 0;
#endif

	}

	/* Should we change the address ? */
	if (set_address) 
	{
		outb (faddr, OMAP34XX_USB_FADDR);
		set_address = 0;
#ifdef CONFIG_OMAP3430
		/* If you have gotten here you are mostly ok */
		OMAP3_LED_OK_ON ();
#endif
	}
  
	return fastboot_poll_h();
}

static void fastboot_rx_error(void)
{
	/* Clear the RXPKTRDY bit */
	*peri_rxcsr &= ~MUSB_RXCSR_RXPKTRDY;

	/* Send stall */
	*peri_rxcsr |= MUSB_RXCSR_P_SENDSTALL;

	/* Wait till stall is sent.. */
	while (!(*peri_rxcsr & MUSB_RXCSR_P_SENTSTALL))
		udelay(1);

	/* Clear stall */
	*peri_rxcsr &= ~MUSB_RXCSR_P_SENTSTALL;

}

static int rx_nothing;

static int fastboot_rx (void)
{
	if (*peri_rxcsr & MUSB_RXCSR_RXPKTRDY) {
		u16 count = *rxcount;
		int fifo_size = fastboot_fifo_size();

		if (0 == *rxcount) {
			/* Clear the RXPKTRDY bit */
			*peri_rxcsr &= ~MUSB_RXCSR_RXPKTRDY;
		} else if (fifo_size < count) {
			fastboot_rx_error();
		} else {
			int i = 0;
			int err = 1;

			/*
			 * If the fifo is full, it is likely we are going to
			 * do a multiple packet transfere.  To speed this up
			 * do a DMA for full packets.  To keep the handling
			 * of the end packet simple, just do it by manually
			 * reading the fifo
			 */
			if (fifo_size == count) {
				/* Mode 1
				 *
				 * The setup is not as simple as
				 * *peri_rxcsr |=
				 * (MUSB_RXCSR_DMAENAB | MUSB_RXCSR_DMAMODE)
				 *
				 * There is a special sequence needed to
				 * enable mode 1.  This was take from
				 * musb_gadget.c in the 2.6.27 kernel
				 */
				*peri_rxcsr &= ~MUSB_RXCSR_AUTOCLEAR;
				*peri_rxcsr |= MUSB_RXCSR_DMAENAB;
				*peri_rxcsr |= MUSB_RXCSR_DMAMODE;
				*peri_rxcsr |= MUSB_RXCSR_DMAENAB;

				if (read_bulk_fifo_dma
				    (fastboot_bulk_fifo, fifo_size)) {
					/* Failure */
					fastboot_rx_error();
				}

				/* Disable DMA in peri_rxcsr */
				*peri_rxcsr &= ~(MUSB_RXCSR_DMAENAB |
						 MUSB_RXCSR_DMAMODE);

			} else {
				for (i = 0; i < count; i++)
					fastboot_bulk_fifo[i] =
						read_bulk_fifo_8();
			}
			/* Clear the RXPKTRDY bit */
			*peri_rxcsr &= ~MUSB_RXCSR_RXPKTRDY;

			/* Pass this up to the interface's handler */
			if (fastboot_interface &&
			    fastboot_interface->rx_handler) {
				if (!fastboot_interface->rx_handler
				    (&fastboot_bulk_fifo[0], count))
					err = 0;
				rx_nothing = 0;
			}

			/* Since the buffer is not null terminated,
			 * poison the buffer */
			memset(&fastboot_bulk_fifo[0], 0, fifo_size);

			/* If the interface did not handle the command */
			if (err) {
#ifdef CONFIG_OMAP3430
				OMAP3_LED_ERROR_ON ();
#endif
				CONFUSED();
			}
		}
	}
	return 0;
}

static int fastboot_suspend (void)
{
	/* No suspending going on here! 
	   We are polling for all its worth */

	return 0;
}

extern unsigned int download_size;
static char last_status[65];
static unsigned int last_bsize;

int fastboot_poll(void) 
{
	int ret = 0;

	u8 intrusb;
	u16 intrtx;
	u16 intrrx;
	static u32 blink = 0;
	u32 reg = 0x4A326000;
	u8 pull = 0;

#define OMAP44XX_WKUP_CTRL_BASE 0x4A31E000
#define OMAP44XX_CTRL_BASE 0x4A100000
#if 0
#if defined(CONFIG_4430PANDA)
	reg = OMAP44XX_WKUP_CTRL_BASE + CONTROL_WKUP_PAD1_FREF_CLK4_REQ;
	pull = PTU;
#else
	reg = OMAP44XX_CTRL_BASE + CONTROL_PADCONF_MCSPI1_CS2;
	pull = (1<<4);
#endif

	/* On panda blink the D1 LED in fastboot mode */
	#define PRECEPTION_FACTOR 100000
	if (blink  == 0x7fff + PRECEPTION_FACTOR){
		__raw_writew(__raw_readw(reg) | (pull), reg);
	}
	if (blink  == (0xffff + PRECEPTION_FACTOR)) {
		__raw_writew(__raw_readw(reg) & (~pull), reg);
		blink = 0;
	}
	blink ++ ;
#endif

	/* Look at the interrupt registers */
	intrusb = inb (OMAP34XX_USB_INTRUSB);

	/* A disconnect happended, this signals that the cable
	   has been disconnected, return immediately */
	if (intrusb & OMAP34XX_USB_INTRUSB_DISCON)
	{
		return 1;
	}

	if (intrusb & OMAP34XX_USB_INTRUSB_RESUME)
	{
		ret = fastboot_resume ();
		if (ret)
			return ret;
	}
	else 
	{
		if (intrusb & OMAP34XX_USB_INTRUSB_SOF)
		{
			if (++rx_nothing > 5000) {
				rx_nothing = 1;
				if (download_size) {
					printf("Nothing seen in 5000 SOFs, resending last status\n");
					fastboot_tx_status(last_status, last_bsize);
				};
			};
			ret = fastboot_resume ();
			if (ret)
				return ret;

			/* The fastboot client blocks of read and 
			   intrrx is not reliable. 
			   Really poll */
			if (deferred_rx)
				ret = fastboot_rx ();
			deferred_rx = 0;
			if (ret)
				return ret;
			

		}
		if (intrusb & OMAP34XX_USB_INTRUSB_SUSPEND)
		{
			ret = fastboot_suspend ();
			if (ret)
				return ret;
		}

		intrtx = inw (OMAP34XX_USB_INTRTX);
		if (intrtx)
		{
			/* TX interrupts happen when a packet has been sent
			   We already poll the csr register for this when 
			   something is sent, so do not do it twice 

			*/
		}

		intrrx = inw (OMAP34XX_USB_INTRRX);
		if (intrrx)
		{
			/* Defer this to SOF */
			deferred_rx = 1;
		}
	}

	return ret;
}



void fastboot_shutdown(void)
{
	/* Clear the SOFTCONN bit to disconnect */
	*pwr &= ~MUSB_POWER_SOFTCONN;

	/* Reset some globals */
	faddr = 0xff;
	fastboot_interface = NULL;
	high_speed = 0;
}

int fastboot_is_highspeed(void)
{
	int ret = 0;
	if (*pwr & MUSB_POWER_HSMODE)
		ret = 1;
	return ret;
}

int fastboot_fifo_size(void)
{
	return high_speed ? RX_ENDPOINT_MAXIMUM_PACKET_SIZE_2_0 : RX_ENDPOINT_MAXIMUM_PACKET_SIZE_1_1;
}

int fastboot_tx_status(const char *buffer, unsigned int buffer_size)
{
	int ret = 1;
	unsigned int i;
	/* fastboot client only reads back at most 64 */
	unsigned int transfer_size = MIN(64, buffer_size);

	/* save a copy, for blocked-send retries */
	strncpy(last_status, buffer, last_bsize = transfer_size);

	while  (*peri_txcsr & MUSB_TXCSR_TXPKTRDY)
		udelay(1);

	for (i = 0; i < transfer_size; i++)
		write_bulk_fifo_8 (buffer[i]);

	*peri_txcsr |= MUSB_TXCSR_TXPKTRDY;

	while  (*peri_txcsr & MUSB_TXCSR_TXPKTRDY)
		udelay(1);

	/* Send an empty packet to signal that we are done */
	TX_LAST();

	ret = 0;

	return ret;
}

int fastboot_getvar(const char *rx_buffer, char *tx_buffer)
{
	/* Place board specific variables here */
	return 0;
}

extern int
do_booti(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

int fastboot_preboot(void)
{
	char *cmd[3];
    unsigned long val;
#if 0
#if (defined(CONFIG_TWL4030_KEYPAD) && (CONFIG_TWL4030_KEYPAD))
	int i;
	unsigned char key1, key2;
	int keys;
	udelay(CFG_FASTBOOT_PREBOOT_INITIAL_WAIT);
	for (i = 0; i < CFG_FASTBOOT_PREBOOT_LOOP_MAXIMUM; i++) {
		key1 = key2 = 0;
		keys = twl4030_keypad_keys_pressed(&key1, &key2);
		if ((1 == CFG_FASTBOOT_PREBOOT_KEYS) &&
		    (1 == keys)) {
			if (CFG_FASTBOOT_PREBOOT_KEY1 == key1)
				return 1;
		} else if ((2 == CFG_FASTBOOT_PREBOOT_KEYS) &&
			   (2 == keys)) {
			if ((CFG_FASTBOOT_PREBOOT_KEY1 == key1) &&
			    (CFG_FASTBOOT_PREBOOT_KEY2 == key2))
				return 1;
		}
		udelay(CFG_FASTBOOT_PREBOOT_LOOP_WAIT);
	}
#endif

#if defined(CONFIG_OMAP44XX)
#define KBD_IRQSTATUS		(0x4a31c018)
#define KBD_STATEMACHINE	(0x4a31c038)
#define KBD_FULLCODE31_0	(0x4a31c044)
	/* Any key kept pressed does auto-fastboot */
	if (__raw_readl(KBD_STATEMACHINE)) {

		switch (__raw_readl(KBD_FULLCODE31_0)) {
			case 0x800: /* Blaze GREEN key pressed */
				printf("\n Green key press == Recovery mode \n");
				/* Clear any key status */
				while (__raw_readl(KBD_IRQSTATUS))
					__raw_writel(0xf, KBD_IRQSTATUS);
				goto start_recovery;
			break;
			default:
				return 1;
			break;
		}
	}

	/* On Panda: GPIO_121 button pressed causes to enter fastboot */
#if defined(CONFIG_4430PANDA)
	if (!(__raw_readl(OMAP44XX_GPIO4_BASE + DATA_IN_OFFSET) & (1<<25))){
		printf("Panda: GPIO_121 pressed: entering fastboot....\n");
		return 1;
	}
#endif

#if defined(OMAP44XX_TABLET_CONFIG)
	/* Home key == Enter Fastboot */
	if (!(__raw_readl(OMAP44XX_GPIO2_BASE + DATA_IN_OFFSET) & (1<<14))){
		printf("Tablet: HOME key pressed: entering fastboot....\n");
		return 1;
	}
	/* Back key == Enter Recovery */
	if (!(__raw_readl(OMAP44XX_GPIO2_BASE + DATA_IN_OFFSET) & (1<<11))){
		printf("Tablet: Back key pressed: entering recovery....\n");
		goto start_recovery;
	}
#endif
	if (__raw_readl(PRM_RSTST) & PRM_RSTST_RESET_WARM_BIT) {

		printf("\n reboot command [%s]\n", PUBLIC_SAR_RAM_1_FREE);
		/* Warm reset case:
		* %adb reboot recovery
		*/
		if (!strcmp(PUBLIC_SAR_RAM_1_FREE, "recovery")) {

			printf("\n Case: \%reboot recovery\n");
start_recovery:
			printf("\n Starting recovery img.....\n");
			cmd[0] = malloc(10);
			cmd[1] = malloc(10);
			cmd[2] = malloc(10);

			/* pass: booti mmci<N> recovery */
			strcpy(cmd[0], "booti");
#if defined(CONFIG_4430PANDA)
			strcpy(cmd[1], "mmc0");
#else
			strcpy(cmd[1], "mmc1");
#endif
			strcpy(cmd[2], "recovery");

			do_booti(NULL, 0, 3, cmd);
			/* returns if recovery.img is bad
			 * Default to normal boot
			 */
			free(cmd[0]);
			free(cmd[1]);
			free(cmd[2]);

			printf("\nfastboot: Error: Invalid recovery img\n");
			return 0;
		}

		/* Warm reset case
		 * Case: %fastboot reboot-bootloader
		 * Case: %adb reboot bootloader
		 * Case: %adb reboot reboot-bootloader
		 */
		return 1;
	}

#endif
#endif
//Michael 0426
//    val = getbootmode();
//	if((0x4002 == val) || (0x4003 == val)){
//		//printf("\n Entering FASTBOOT ...\n");
//		return 1;
//	}
////gpio_52 set configuration to MUX3 and INPUT_ENABLE
//__raw_writew(0x10B,0x4A100078);
//udelay(100000); /* 1 sec */
//
////gpio_52 read GPIO_DATAIN
//	if(__raw_readl(0x48055138) & 0x00100000){
//		printf("\nSPECIAL USB CABLE DETECTED: enter FASTBOOT now\n");
//		return 1;
//	}
//	return 0;
//
	fastboot_wait_power_button_abort = 0;
	return 1; // always enter fastboot, delay gets us out
}

int fastboot_init(struct cmd_fastboot_interface *interface) 
{
	int ret,ms = 1;
	u8 devctl;
	int cpu_rev = 0;
	int cpu_type = 0;

	twl6030_disable_vusb();
	twl6030_init_vusb();
	__raw_writel(0x101, 0x4A0093E0);    //enable ocp2scp_usb_phy_ick
	__raw_writel(0x100, 0x4A008640);    //enable ocp2scp_usb_phy_phy_48m
	for (ms=0;ms<100;ms++)
		udelay(100);//10ms
	__raw_writel(~(1), 0x4A002300);          //power up the usb phy

	//device_strings[DEVICE_STRING_MANUFACTURER_INDEX]  = "Texas Instruments";
	device_strings[DEVICE_STRING_MANUFACTURER_INDEX]  = "Amazon";
#if defined (CONFIG_3430ZOOM2)
	device_strings[DEVICE_STRING_PRODUCT_INDEX]       = "Zoom2";
#elif defined (CONFIG_3430LABRADOR)
	device_strings[DEVICE_STRING_PRODUCT_INDEX]       = "Zoom";
#elif defined(CONFIG_4430SDP)
//	if (get_board_rev() != 0x10)
//		device_strings[DEVICE_STRING_PRODUCT_INDEX] = "Blaze_Tablet";
//	else
//		device_strings[DEVICE_STRING_PRODUCT_INDEX] = "Blaze";
		device_strings[DEVICE_STRING_PRODUCT_INDEX] = "kindle";
#elif defined(CONFIG_4430PANDA)
	device_strings[DEVICE_STRING_PRODUCT_INDEX]       = "panda";
#else
	/* Default, An error message to prompt user */
#error "Need a product name for fastboot"

#endif

#if 0//defined(CONFIG_4430SDP) || defined(CONFIG_4430PANDA)
	unsigned int val[4] = { 0 };
	unsigned int reg;
	static char device_serial[MAX_USB_SERIAL_NUM];

	reg = DIE_ID_REG_BASE + DIE_ID_REG_OFFSET;

	val[0] = __raw_readl(reg);
	val[1] = __raw_readl(reg + 0x8);
	val[2] = __raw_readl(reg + 0xC);
	val[3] = __raw_readl(reg + 0x10);
	printf("Device Serial Number: %08X%08X\n", val[3], val[2]);
	sprintf(device_serial, "%08X%08X", val[3], val[2]);

	device_strings[DEVICE_STRING_SERIAL_NUMBER_INDEX] = device_serial;
#else
	/* These are just made up */
	//device_strings[DEVICE_STRING_SERIAL_NUMBER_INDEX] = "00123";
	device_strings[DEVICE_STRING_SERIAL_NUMBER_INDEX] = "0123456789ABCDEF";
#endif
	device_strings[DEVICE_STRING_CONFIG_INDEX]        = "Android Fastboot";
	device_strings[DEVICE_STRING_INTERFACE_INDEX]     = "Android Fastboot";

#if defined(CONFIG_4430SDP) || defined(CONFIG_4430PANDA)
	cpu_rev = get_cpu_rev();
	switch (cpu_rev) {
		case CPU_4430_ES1:
			device_strings[DEVICE_STRING_PROC_REVISION]  = "ES1.0";
			break;
		case CPU_4430_ES20:
			device_strings[DEVICE_STRING_PROC_REVISION]  = "ES2.0";
			break;
		case CPU_4430_ES21:
			device_strings[DEVICE_STRING_PROC_REVISION]  = "ES2.1";
			break;
		case CPU_4430_ES22:
			device_strings[DEVICE_STRING_PROC_REVISION]  = "ES2.2";
			break;
		default:
			device_strings[DEVICE_STRING_PROC_REVISION]  = "Unknown";
			break;
	}
	cpu_type = get_device_type();
	switch (cpu_type) {
		case CPU_4430_GP:
			device_strings[DEVICE_STRING_PROC_TYPE]  = "GP";
			break;
		case  CPU_4430_EMU:
			device_strings[DEVICE_STRING_PROC_TYPE]  = "EMU";
			break;
		case  CPU_4430_HS:
			device_strings[DEVICE_STRING_PROC_TYPE]  = "HS";
			break;
		default:
			device_strings[DEVICE_STRING_PROC_TYPE]  = "Unknown";
			break;
	}

#endif

	/* The interface structure */
	fastboot_interface = interface;
	fastboot_interface->product_name                  = device_strings[DEVICE_STRING_PRODUCT_INDEX];
	fastboot_interface->serial_no                     = device_strings[DEVICE_STRING_SERIAL_NUMBER_INDEX];
#if defined(CONFIG_4430SDP) || defined(CONFIG_4430PANDA)
	fastboot_interface->storage_medium                = EMMC;
	fastboot_interface->proc_rev			  = device_strings[DEVICE_STRING_PROC_REVISION];
	fastboot_interface->proc_type			  = device_strings[DEVICE_STRING_PROC_TYPE];
#else
	fastboot_interface->storage_medium                = NAND;
#endif
	fastboot_interface->nand_block_size               = 2048;
	fastboot_interface->transfer_buffer               = (unsigned char *) CFG_FASTBOOT_TRANSFER_BUFFER;
	fastboot_interface->transfer_buffer_size          = CFG_FASTBOOT_TRANSFER_BUFFER_SIZE;

    //printf("fastbootinit: Reset Mentor USB block otg_sysconfig=0x%x (0x%x)\n",*otg_sysconfig,otg_sysconfig);
	/* Reset Mentor USB block */
	/* 1)soft reset */
    //printf("fastbootinit: otg_sysconfig=0x%x\n",*otg_sysstatus);
	*otg_sysconfig |= (1<<1);
    //while(*otg_sysstatus){
    //    printf("fastbootinit: otg_sysconfig=0x%x\n",*otg_sysstatus);
    //}
    //printf("fastbootinit: Power Management Configuration\n");
	/* 2)Power Management Configuration .now set better defaults MIDLEMODE=No standby mode ,SIDLEMODE=No idle mode*/
	*otg_sysconfig = (0x1008);

    /* 3)PHY interface is 8-bit, UTMI+ level 3*/
    *otg_interfsel &= 0;
    /* 4)*Enable functional PHY clock (OTG_60M_FCLK)*/
    __raw_writel(0x101, 0x4A0093E0);	//Enable ocp2scp_usb_phy_ick
    __raw_writel(0x100, 0x4A008640);	//Enable ocp2scp_usb_phy_phy_48m

    //printf("Enable functional PHY clock _CLKCTRL=0x%x\n",__raw_readl(CM_L3INIT_HSUSBOTG_CLKCTRL));
    //__raw_writel(0x101,CM_L3INIT_HSUSBOTG_CLKCTRL);
    //sr32(CM_L3INIT_HSUSBOTG_CLKCTRL, 0, 24, 0x1);
    //printf("Enable functional PHY clock _CLKCTRL=0x%x\n",__raw_readl(CM_L3INIT_HSUSBOTG_CLKCTRL));
    //wait_on_value(BIT17|BIT16, 1, CM_L3INIT_HSUSBOTG_CLKCTRL , LDELAY);
    //printf("!!CM_L3INIT_HSUSBOTG_CLKCTRL=0x%x\n",__raw_readl(CM_L3INIT_HSUSBOTG_CLKCTRL));
    //sr32(CM_L3INIT_HSUSBOTG_CLKCTRL, 0, 32, 0x1);


    __raw_writel(~(1), 0x4A002300);    //power up the usb phy

    //printf("fasboot:otghs_control= 0x%x\n",*otghs_control);
    if (*otghs_control != 0x15){

		fastboot_reset();

		*otg_interfsel &= 0;

		/* Program Phoenix registers VUSB_CFG_STATE and MISC2 */
		twl6030_usb_device_settings();

		/* Program the control module register */
		*otghs_control = 0x15;

	} else {
		/* HACK */
		fastboot_bulk_endpoint_reset();

		*otg_interfsel &= 0;

		/* Keeping USB cable attached and booting causes
		 * ROM code to reconfigure USB, and then
		 * re-enumeration never happens
		 * Setting this SRP bit helps - Cannot see why !!
		 * MUSB spec says : Session bit is used only for SRP
		 */
		outb(0x1,OMAP34XX_USB_DEVCTL);
	}

	/* Check if device is in b-peripheral mode */
	devctl = inb (OMAP34XX_USB_DEVCTL);
	if (!(devctl & MUSB_DEVCTL_BDEVICE) || (devctl & MUSB_DEVCTL_HM)) {
		printf ("ERROR : Unsupport USB mode\n");
		printf ("Check that mini-B USB cable is attached to the device\n");
	} else
		ret = 0;

	return ret;
}

#endif /* CONFIG_FASTBOOT */
