//--------------------------------------------------------------------------
//
// File name:      smsc9118.c
//
// Abstract:      Driver for SMSC LAN9118 ethernet controller.
//
// Start Automated RH
// *** Do not edit between "Start Automated RH" and "End Automated RH" ***
//
// Copyright 2005, Seagate Technology LLC
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// Revision History
//
// *** Do not edit between "Start Automated RH" and "End Automated RH" ***
// End Automated RH
//
//--------------------------------------------------------------------------
/*---------------------------------------------------------------------------
 * Copyright(c) 2005-2006 SMSC
 *
 *  Use of this source code is subject to the terms of the SMSC Software
 *  License Agreement (SLA) under which you licensed this software product.	 
 *  If you did not accept the terms of the SLA, you are not authorized to use
 *  this source code. 
 *  
 *  This code and information is provided as is without warranty of any kind,
 *  either expressed or implied, including but not limited to the implied
 *  warranties of merchantability and/or fitness for a particular purpose.
 *   
 *  File name   : smsc9118.c 
 *  Description : smsc9118 polled driver (non-interrupt driven)
 *  
 *  History	    :
 *  	09-27-06 MDG		v1.0 (First Release)
 *			modified for ARM platform
 *----------------------------------------------------------------------------*/

#include <common.h>
#include <command.h>
#include <config.h>
#include "smsc9118.h"
#include <net.h>

#ifdef CONFIG_DRIVER_SMSC9118

//*************************************************************************
 //  FUNCTION PROTOTYPES

//*************************************************************************
int eth_init(bd_t *bd);
void eth_halt(void);
int eth_rx(void);
int eth_send(volatile void *packet, int length);
extern void *malloc( unsigned ); // <stdlib.h>
extern void free( void * ); // <stdlib.h>

//*************************************************************************
 //  LOCAL DEFINITIONS AND MACROS

//*************************************************************************
//#define	DEBUG
#define 	GPIO_OUT(val)	  		(*GPIO_CFG = ((*GPIO_CFG & ~GPIO_CFG_GPIOD_MSK) | (val & GPIO_CFG_GPIOD_MSK)))
#define 	ENET_MAX_MTU			PKTSIZE
#define 	ENET_MAX_MTU_ALIGNED	PKTSIZE_ALIGN
#define 	NUM_RX_BUFF 	  		PKTBUFSRX
#define		ENET_ADDR_LENGTH		6
#define		TX_TIMEOUT_COUNT		30	// waiting for TX_FIFO to drain


//*************************************************************************
 // GLOBAL DATA

//*************************************************************************
static const char date_code[] = BUILD_NUMBER;

static char * txbp; 				// TX buffer pointer (only 1 buffer)
static volatile uchar * rxbp[PKTBUFSRX];   // Receiver buffer queue (IP layers)
static struct rxQue rxAvlQue[PKTBUFSRX]; // Receive buffer available queue
static int rxNdx = 0;				// Current receive buffer index
static int rxNdxIn = 0; 			// Used for input
static int rxNdxOut = 0;			// Used for output to protocol layer
static ushort lastTxTag = 0x0;
static unsigned char macAddr[ENET_ADDR_LENGTH];

// Temp variables
//#ifdef		DEBUG
ulong MaxRxFifoSz;
ulong TotalInts = 0;
ulong TotalRXE = 0;
ulong TotalRxPackets = 0;
ulong TotalBytes = 0;
ulong EmptyReads = 0;

ulong RxPacketBuf[400];
ulong SWIntTriggered = FALSE;
ulong TotalRxDrop = 0;
ulong TotalPackets = 0;
ulong TotalWords = 0;
ulong TBLower1, TBLower2;
//#endif
// Temp variables


//*************************************************************************
 // EXTERNS

//*************************************************************************
#ifdef		DEBUG
extern int use_smsc9118;
#endif

static void lan9118_udelay(unsigned long delta)	// Arg is really microseconds
{
	const unsigned long	start = *FREE_RUN,	// Start timing
						usec = delta * (25000000/1000000);

	// usec adjusted for 25MHz on-chip clock, 1 microsecond (1/1000000) scaling
	do {
		delta = *FREE_RUN;
		if (delta >= start)
			delta = (delta - start);
		else
			delta = (delta - start) + 1;	// use 0x100000000, not 0xffffffff
	} while (delta < usec);
}

static int MacBusy(int ReqTO)
{
	  int timeout = ReqTO;
	  int RetVal = FALSE;	  // No timeout

	  while (timeout--) {
			if (!(*MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY)) {
				  goto done;
			}
	  }
	  RetVal = TRUE;		  // Timeout
done:
	  return (RetVal);
}

static ulong
GetMacReg(int Reg)
{
	  ulong RegVal = 0xffffffff;

	  if (*MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY) {
			LAN9118_WARN("GetMacReg: previous command not complete\n");
			goto done;
	  }

	  *MAC_CSR_CMD = MAC_RD_CMD(Reg);
	  DELAY(1);

	  if (MacBusy(MAC_TIMEOUT) == TRUE) {
			LAN9118_WARN("GetMacReg: timeout waiting for response "
				  "from MAC\n");
			goto done;
	  }

	  RegVal = *MAC_CSR_DATA;
done:
	  return (RegVal);
}

static int
PhyBusy(int ReqTO)
{
	  int timeout = ReqTO;
	  int RetVal = FALSE;	  // No timeout

	  while (timeout--) {
			if (!(GetMacReg(MAC_MIIACC) & MAC_MIIACC_MII_BUSY)) {
				  goto done;
			}
	  }

	  RetVal = TRUE;		  // Timeout
done:
	  return (RetVal);
}

static int
SetMacReg(int Reg, ulong Value)
{
	  int RetVal = FALSE;

	  if (*MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY) {
			LAN9118_WARN("SetMacReg: previous command not complete\n");
			goto done;
	  }

	  *MAC_CSR_DATA = Value;
	  DELAY(1);
	  *MAC_CSR_CMD = MAC_WR_CMD(Reg);
	  DELAY(1);

	  if (MacBusy(MAC_TIMEOUT) == TRUE) {
			LAN9118_WARN("SetMacReg: timeout waiting for response "
				  "from MAC\n");
				  goto done;
	  }

	  RetVal = TRUE;
done:
	  return (RetVal);
}

static ushort
GetPhyReg(unchar Reg)
{
	  ushort RegVal = 0xffff;

	  if (GetMacReg(MAC_MIIACC) & MAC_MIIACC_MII_BUSY) {
			LAN9118_WARN("GetPhyReg: MII busy\n");
			RegVal = 0;
			goto done;
	  }

	  SetMacReg(MAC_MIIACC, MAC_MII_RD_CMD((unchar)PHY_ADDR, Reg));
	  DELAY(1);

	  if (PhyBusy(PHY_TIMEOUT) == TRUE) {
			LAN9118_WARN("GetPhyReg: timeout waiting for MII command\n");
			goto done;
	  }

	  RegVal = (ushort)GetMacReg(MAC_MIIDATA);
done:
	  return (RegVal);
}

static int
SetPhyReg(unchar Reg, ushort Value)
{
	  int RetVal = FALSE;

	  if (GetMacReg(MAC_MIIACC) & MAC_MIIACC_MII_BUSY) {
			LAN9118_WARN("SetPhyReg: MII busy\n");
			goto done;
	  }

	  SetMacReg(MAC_MIIDATA, Value);
	  DELAY(1);
	  SetMacReg(MAC_MIIACC, MAC_MII_WR_CMD((unchar)PHY_ADDR, Reg));
	  DELAY(1);

	  if (PhyBusy(PHY_TIMEOUT) == TRUE) {
			LAN9118_WARN("SetPhyReg: timeout waiting for MII command\n");
			goto done;
	  }

	  RetVal = TRUE;
done:
	  return (RetVal);
}

// Display directly accessed, Control/Status Registers
static int
DumpCsrRegs(void)
{
	  printf("ID_REV:\t\t0x%0.8x\n", *ID_REV);
	  printf("IRQ_CFG:\t0x%0.8x\n", *IRQ_CFG);
	  printf("INT_STS:\t0x%0.8x\n", *INT_STS);
	  printf("INT_EN:\t\t0x%0.8x\n", *INT_EN);
	  printf("BYTE_TEST:\t0x%0.8x\n", *BYTE_TEST);
	  printf("FIFO_INT:\t0x%0.8x\n", *FIFO_INT);
	  printf("RX_CFG:\t\t0x%0.8x\n", *RX_CFG);
	  printf("TX_CFG:\t\t0x%0.8x\n", *TX_CFG);
	  printf("HW_CFG:\t\t0x%0.8x\n", *HW_CFG);
	  printf("RX_DP_CTL:\t0x%0.8x\n", *RX_DP_CTL);
	  printf("RX_FIFO_INF:\t0x%0.8x\n", *RX_FIFO_INF);
	  printf("TX_FIFO_INF:\t0x%0.8x\n", *TX_FIFO_INF);
	  printf("PWR_MGMT:\t0x%0.8x\n", *PWR_MGMT);
	  printf("GPIO_CFG:\t0x%0.8x\n", *GPIO_CFG);
	  printf("GPT_CFG:\t0x%0.8x\n", *GPT_CFG);
	  printf("GPT_CNT:\t0x%0.8x\n", *GPT_CNT);
	  printf("FPGA_REV:\t0x%0.8x\n", *FPGA_REV);
	  printf("ENDIAN:\t\t0x%0.8x\n", *ENDIAN);
	  printf("FREE_RUN\t0x%0.8x\n", *FREE_RUN);
	  printf("RX_DROP\t\t0x%0.8x\n", *RX_DROP);
	  printf("MAC_CSR_CMD\t0x%0.8x\n", *MAC_CSR_CMD);
	  printf("MAC_CSR_DATA\t0x%0.8x\n", *MAC_CSR_DATA);
	  printf("AFC_CFG\t\t0x%0.8x\n", *AFC_CFG);
	  return (0);
}

// Display Media Access Controller Registers
static int
DumpMacRegs(void)
{
	  printf("MAC_CR\t\t0x%0.8x\n", GetMacReg(MAC_CR));
	  printf("MAC_ADDRH\t0x%0.8x\n", GetMacReg(MAC_ADDRH));
	  printf("MAC_ADDRL\t0x%0.8x\n", GetMacReg(MAC_ADDRL));
	  printf("MAC_HASHH\t0x%0.8x\n", GetMacReg(MAC_HASHH));
	  printf("MAC_HASHL\t0x%0.8x\n", GetMacReg(MAC_HASHL));
	  printf("MAC_MIIACC\t0x%0.8x\n", GetMacReg(MAC_MIIACC));
	  printf("MAC_MIIDATA\t0x%0.8x\n", GetMacReg(MAC_MIIDATA));
	  printf("MAC_FLOW\t0x%0.8x\n", GetMacReg(MAC_FLOW));
	  printf("MAC_VLAN1\t0x%0.8x\n", GetMacReg(MAC_VLAN1));
	  printf("MAC_VLAN2\t0x%0.8x\n", GetMacReg(MAC_VLAN2));
	  printf("MAC_WUFF\t0x%0.8x\n", GetMacReg(MAC_WUFF));
	  printf("MAC_WUCSR\t0x%0.8x\n", GetMacReg(MAC_WUCSR));
	  return (0);
}
 
// Display PHYsical media interface registers
static int
DumpPhyRegs(void)
{
	  printf("PHY_BCR\t\t0x%0.4x\n", GetPhyReg(PHY_BCR));
	  printf("PHY_BSR\t\t0x%0.4x\n", GetPhyReg(PHY_BSR));
	  printf("PHY_ID1\t\t0x%0.4x\n", GetPhyReg(PHY_ID1));
	  printf("PHY_ID2\t\t0x%0.4x\n", GetPhyReg(PHY_ID2));
	  printf("PHY_ANAR\t0x%0.4x\n", GetPhyReg(PHY_ANAR));
	  printf("PHY_ANLPAR\t0x%0.4x\n", GetPhyReg(PHY_ANLPAR));
	  printf("PHY_ANEXPR\t0x%0.4x\n", GetPhyReg(PHY_ANEXPR));
	  printf("PHY_SILREV\t0x%0.4x\n", GetPhyReg(PHY_SILREV));
	  printf("PHY_MCSR\t0x%0.4x\n", GetPhyReg(PHY_MCSR));
	  printf("PHY_SPMODES\t0x%0.4x\n", GetPhyReg(PHY_SPMODES));
	  printf("PHY_CSIR\t0x%0.4x\n", GetPhyReg(PHY_CSIR));
	  printf("PHY_ISR\t\t0x%0.4x\n", GetPhyReg(PHY_ISR));
	  printf("PHY_IMR\t\t0x%0.4x\n", GetPhyReg(PHY_IMR));
	  printf("PHY_PHYSCSR\t0x%0.4x\n", GetPhyReg(PHY_PHYSCSR));
	  return (0);
}

static int
lan9118_open(bd_t *bis)
{
	  int RetVal = TRUE;
	  int timeout;
	  int i;
	  static unsigned mac_addrh = 0, mac_addrl = 0;

#ifdef		DEBUG
	  TotalInts = 0;
	  TotalRXE = 0;
	  TotalBytes = 0;

	  printf("DRIVER_VERSION : %X, ", DRIVER_VERSION);
	  printf("DATECODE : %s\r\n", BUILD_NUMBER);

	  if (bis->bi_bootflags & 0x40000000) {
			use_smsc9118 = 1;
	  }
#endif		//DEBUG

	  // Because we just came out of h/w reset we can't be sure that
	  // the chip has completed reset and may have to implement the
	  // workaround for Errata 5, stepping A0.	Therefore we need to
	  // check the ID_REV in little endian, the reset default.
	  if (((*ID_REV & ID_REV_ID_MASK) == ID_REV_CHIP_118) ||
	  	  ((*ID_REV & ID_REV_ID_MASK) == ID_REV_CHIP_218) ||
		  ((*ID_REV & ID_REV_ID_MASK) == ID_REV_CHIP_211) ||
		  ((*ID_REV & ID_REV_ID_MASK) == ID_REV_CHIP_221))
	  {
			printf("LAN9x18 (0x%08x) detected.\n", *ID_REV);
	  }
	  else
	  {
			printf("Failed to detect LAN9118. ID_REV = 0x%08x\n", *ID_REV);
		    RetVal = FALSE;
		    goto done;
	  }

	  // Does SoftReset to 118
	  *HW_CFG = HW_CFG_SRST;
	  DELAY(10);

	  // Is the internal PHY running?
	  if ((*PWR_MGMT & PWR_MGMT_PM_MODE_MSK) != 0) {
			// Apparently not...
			*BYTE_TEST = 0x0; // Wake it up
			DELAY(1);
			timeout = PHY_TIMEOUT;
			while (timeout-- && ((*PWR_MGMT & PWR_MGMT_PME_READY) == 0)) {
				  lan9118_udelay(1);
			}
			if ((*PWR_MGMT & PWR_MGMT_PME_READY) == 0) {
				  LAN9118_WARN("LAN9118: PHY not ready");
				  LAN9118_WARN(" - aborting\n");
				  RetVal = FALSE;
				  goto done;
			}
	  }

	  // Setup TX and RX resources.

	  // There is one TX buffer.
	  if ((txbp = (char *)malloc(ENET_MAX_MTU_ALIGNED)) == NULL) {
			LAN9118_WARN("lan9118_open: can't get TX buffer\n");
			goto cleanup;
	  }

	  // The receive buffers are allocated and aligned by upper layer
	  // software.
	 for (i = 0; i < PKTBUFSRX; i++) {
			rxbp[i] = NetRxPackets[i];
			rxAvlQue[i].index = -1;
	  }

	  rxNdx = 0;
	  rxNdxIn = 0;
	  rxNdxOut = 0;
	  lastTxTag = 0x0;

	  // Set TX Fifo Size
	  *HW_CFG = 0x00040000;   // 4K for TX

	  // This value is dependent on TX Fifo Size since there's a limited
	  // amount of Fifo space.
	  MaxRxFifoSz = 13440;			// Decimal

	  // Set automatic flow control.
	  *AFC_CFG = 0x008c46af;

	  // Flash LEDs.
	  *GPIO_CFG = 0x70700000;

	  // Disable interrupts until the rest of initialization is complete.
	  *INT_EN = 0x0;				// Clear interrupt enables
	  *INT_STS = 0xffffffff;		// Clear pending interrupts
	  *IRQ_CFG = 0x00000001;		// IRQ disable

	  // Enable flow control and pause frame time
	  SetMacReg(MAC_FLOW, 0xffff0002);

	  // Set MAC address, if octet 0 is non-null assume it's all good.
	  memcpy(macAddr, bis->bi_enetaddr, ENET_ADDR_LENGTH);
	  if (mac_addrh != 0 || mac_addrl != 0) {
		printf("Setting mac address: %02x:%02x:%02x:%02x:%02x:%02x\n",
			macAddr[0], macAddr[1], macAddr[2],
			macAddr[3], macAddr[4], macAddr[5]);
		mac_addrh = macAddr[5] << 8 | macAddr[4];
		mac_addrl = macAddr[3] << 24 | macAddr[2] << 16 |
			    macAddr[1] << 8 | macAddr[0];
		SetMacReg(MAC_ADDRH, mac_addrh);
		SetMacReg(MAC_ADDRL, mac_addrl);
	  } else {
		char s_env_mac[64];
		char *env_var;

		mac_addrl = *((unsigned int *)macAddr) = GetMacReg(MAC_ADDRL);
		mac_addrh = *((unsigned int *)macAddr + 1) = GetMacReg(MAC_ADDRH);

		if ( (GetMacReg(MAC_ADDRL) == 0xffffffff) && (GetMacReg(MAC_ADDRH) == 0xffff) ) {

			/* Case: No Mac id in EEPROM : Bad case */
			env_var = getenv("ethaddr");

			if (env_var == NULL) {
				/* No ethaddr env */
				printf("\n*** ERROR: Mac id is not programmed in EEPROM\n");
				printf("\tsetenv ethaddr 'xx:xx:xx:xx:xx:xx';saveenv");
				printf("\n Then reboot u-boot for NFS to work\n\n");
				RetVal = -1;
				goto done;
			}
			else {
				/* Get env var and populate macAddr */
				unsigned char len, i;

				len = strlen(env_var);
				strcpy(s_env_mac,env_var);
				// Format xx:xx:xx:xx:xx:xx
				// Convert to digits: back to school days
				for(i=0; i<len; i++)
				{
					if (s_env_mac[i] == ':')
						continue;
					if ( (s_env_mac[i] >= 'A') && (s_env_mac[i] <= 'F') ) {
						s_env_mac[i] -= 'A';
						s_env_mac[i] += 0xa;
					} else if ( (s_env_mac[i] >= 'a') && (s_env_mac[i] <= 'f') ){
						s_env_mac[i] -= 'a';
						s_env_mac[i] += 0xa;
					} else if ( (s_env_mac[i] >= '0') && (s_env_mac[i] <= '9') ){
						s_env_mac[i] -= '0';
					} else {
						printf("\n wrong hex digit %c\n", s_env_mac[i]);
						RetVal = -1;
						goto done;
					}
				}

				macAddr[0] = 16*s_env_mac[0] + s_env_mac[1];
				macAddr[1] = 16*s_env_mac[3] + s_env_mac[4];
				macAddr[2] = 16*s_env_mac[6] + s_env_mac[7];
				macAddr[3] = 16*s_env_mac[9] + s_env_mac[10];
				macAddr[4] = 16*s_env_mac[12] + s_env_mac[13];
				macAddr[5] = 16*(s_env_mac[15]) + s_env_mac[16];
			}
		}

		sprintf (s_env_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
			 macAddr[0], macAddr[1], macAddr[2], macAddr[3],
			macAddr[4], macAddr[5]);
		printf("Read mac address: %s\n", s_env_mac);
		setenv ("ethaddr", s_env_mac);
	  	memcpy(bis->bi_enetaddr, macAddr, ENET_ADDR_LENGTH);
	  }

	  // Dump old status and data
	  *TX_CFG = (TX_CFG_TXS_DUMP | TX_CFG_TXD_DUMP);
	  *RX_CFG = (RX_CFG_FORCE_DISCARD);

	  // Initialize Tx parameters
	  *HW_CFG = ((*HW_CFG & HW_CFG_TX_FIF_SZ_MSK) | HW_CFG_SF);
	  *FIFO_INT = FIFO_INT_TDAL_MSK;	  // Max out value
	  *INT_EN |= INT_EN_TDFA_INT_EN;
	  {
			// Disable MAC heartbeat SQE and enable MAC transmitter
			ulong macCR = GetMacReg(MAC_CR);
			macCR |= (MAC_CR_TXEN | MAC_CR_HBDIS);
			macCR &= ~MAC_CR_PRMS;	// Turn off promiscuous mode
			macCR |= MAC_CR_BCAST;	// Don't accept broadcast frames
			SetMacReg(MAC_CR, macCR);
	  }

	  // Initialize Rx parameters
	  *RX_CFG = 0x00000000;			// 4byte end-alignment
	  {
			// Enable receiver.
			ulong macCR = GetMacReg(MAC_CR);
			SetMacReg(MAC_CR, (macCR | MAC_CR_RXEN));
	  }
	  *FIFO_INT = ((*FIFO_INT & 0xffff0000) | 0x00000101);
	  *INT_EN |= (INT_EN_RSFL_INT_EN | INT_EN_RXE_INT_EN);
	  *INT_EN |= INT_EN_RXDFH_INT_EN;

	  // Initialize PHY parameters
#if 1
	  if (((GetPhyReg(PHY_ID1) == PHY_ID1_LAN9118) && 
		   (GetPhyReg(PHY_ID2) == PHY_ID2_LAN9118)) ||
	      ((GetPhyReg(PHY_ID1) == PHY_ID1_LAN9218) && 
		   (GetPhyReg(PHY_ID2) == PHY_ID2_LAN9218)))
#else
   if(1)
#endif
	   {
		  // Reset the PHY
		  SetPhyReg(PHY_BCR, PHY_BCR_RST);
		  timeout = PHY_TIMEOUT;
		  lan9118_udelay(50*1000);	// > 50ms
		  while(timeout-- && (GetPhyReg(PHY_BCR) & PHY_BCR_RST))
		  {
				lan9118_udelay(10);
		  }
		  if (timeout == 0)
		  {
				LAN9118_WARN("PHY reset incomplete\n");
				RetVal = FALSE;
				goto done;
		  }

		  // Setup and start auto negotiation
		  {
				ushort anar;
				ushort bcr;
				char * spddplx;

				anar = GetPhyReg(PHY_ANAR);
				anar &= ~PHY_ANAR_PAUSE_OP_MSK;
				anar |= PHY_ANAR_PAUSE_OP_BOTH;
				anar |= (PHY_ANAR_10_FDPLX | PHY_ANAR_10_ABLE |
					  PHY_ANAR_100_TX_FDPLX | PHY_ANAR_100_TX_ABLE);
				SetPhyReg(PHY_ANAR, anar);

				DELAY(2);
				bcr = GetPhyReg(PHY_BCR);
				bcr |= (PHY_BCR_SS | PHY_BCR_FDPLX);
				SetPhyReg(PHY_BCR, bcr);
				DELAY(2);

				printf("start Auto negotiation... (take ~2sec)\n");
				bcr = GetPhyReg(PHY_BCR);
				bcr |= (PHY_BCR_ANE | PHY_BCR_RSTAN);
				SetPhyReg(PHY_BCR, bcr);
				DELAY(2);

				timeout = PHY_AN_TIMEOUT;
				while((timeout--) && ((GetPhyReg(PHY_BSR) & PHY_BSR_ANC) == 0)) {
					  lan9118_udelay(500000);
				}
				if ((GetPhyReg(PHY_BSR) & PHY_BSR_ANC) == 0) {
					  LAN9118_WARN("Auto negotiation failed\n");
					  RetVal = FALSE;
					  goto done;
				}

				if ((GetPhyReg(PHY_BSR) & PHY_BSR_LINK_STATUS) == 0) {
					  LAN9118_WARN("Link down\n");
					  RetVal = FALSE;
					  goto done;
				}

				switch ((GetPhyReg(PHY_PHYSCSR) & PHY_PHYSCSR_SPEED_MSK)>>2) {
					  case 0x01:
							spddplx = "10BaseT, half duplex";
							break;
					  case 0x02:
							spddplx = "100BaseTX, half duplex";
							break;
					  case 0x05:
							spddplx = "10BaseT, full duplex";
							break;
					  case 0x06:
							spddplx = "100BaseTX, full duplex";
							break;
					  default:
							spddplx = "Unknown";
							break;
				}
				printf("Auto negotiation complete, %s\n", spddplx);
		  }

		  // If PHYs auto negotiated for full duplex, enable full duplex in MAC.
		  if ((GetPhyReg(PHY_ANAR) & GetPhyReg(PHY_ANLPAR)) & 0x0140) {
				SetMacReg(MAC_CR, (GetMacReg(MAC_CR) | 0x00100000));
		  }
		  // correct PHY_ID is detected
		  goto done;
	  }
	  else
	  {
			printf("Unknown PHY ID : 0x%x, 0x%x\n", GetPhyReg(PHY_ID1), GetPhyReg(PHY_ID2));
	  }

	  goto done;

cleanup:
	  if (txbp != NULL) {
			free(txbp);
	  }

done:
	  return (RetVal);
}

static void
lan9118_close(void)
{
	  // Release the TX buffer.
	  if (txbp != NULL) {
			free(txbp);
	  }
	  txbp = NULL;
}

static int
lan9118_read(void)
{
	  int curBufNdx;
	  int loopCount = 0;
	  ulong rxStatus;
	  ulong count;
	  ulong len;
	  int ffwdOk = TRUE;
	  int timeout;
	  int handled = 0;

	  while((*RX_FIFO_INF & 0x00ff0000) != 0) {
			if (loopCount >= NUM_RX_BUFF) {
//printf("read: loopCount exceeded\n");
				  break;	  // Packet buffers full
			}

			curBufNdx = rxNdx;
			loopCount++;
			if (++rxNdx >= NUM_RX_BUFF) {
				  rxNdx = 0;  // Wrap buffer slot #
			}

			rxStatus = *RX_STATUS_FIFO_PORT;
			len = count = rxStatus >> 16;

			if (count >= 4*sizeof(ulong)) {
				  ffwdOk = TRUE;	// Use h/w to toss packet
			} else {
				  ffwdOk = FALSE;	// Have to empty manually on error
			}

			if (count != 0) {
				  if (count > ENET_MAX_MTU) {
						count = 0;
				  } else {
						if ((rxStatus & TX_STATUS_FIFO_ES) != 0) {
							  count = 0;
						}
				  }
			}

			if (count == 0) {
				  if (ffwdOk == TRUE) {
						// Drain it the fast way
						*RX_DP_CTL = RX_DP_FFWD;
						timeout = FFWD_TIMEOUT;
						while (timeout-- && (*RX_DP_CTL & RX_DP_FFWD)) {
							  lan9118_udelay(1);
						}
						if ((*RX_DP_CTL & RX_DP_FFWD) != 0) {
							  LAN9118_WARN("lan9118_read: fast "
									"forward op failed\n");
							  break;
						}
				  } else {
						// Drain it manually
						while (len--) {
							volatile ulong tmp;
							tmp = *RX_FIFO_PORT;
						}
				  }
			} else if (rxAvlQue[rxNdxIn].index != -1) {
				  LAN9118_WARN("lan9118_read: read buffers full!\n");
				  break;
			} else {
				  register ulong *rxbpl;
				  int ndx;

				  TotalRxPackets++;
				  TotalBytes += count;
				  rxAvlQue[rxNdxIn].index = curBufNdx;
				  rxAvlQue[rxNdxIn].len = count;
				  if (++rxNdxIn >= NUM_RX_BUFF) {
						rxNdxIn = 0;
				  }

				  // Copy this packet to a NetRxPacket buffer
				  handled = 1;
//printf("read: %d empty reads prior to this one\n", EmptyReads);
				  EmptyReads = 0;
				  rxbpl = (ulong *)rxbp[curBufNdx];
				  for (ndx = (count+3)/sizeof(ulong); ndx > 0; ndx--) {
						*rxbpl++ = *RX_FIFO_PORT;
				  }
#if 0
{
	printf("Received: packet contents follows.\n");
	int i;
	for (i = 1; i <= count; i++) {
		  printf("0x%02x ", rxbp[curBufNdx][i-1]);
		  if (!(i%16))
			  printf("\n");
	}
	printf("\n");
}
#endif
				  DELAY(3);
			}
	  }

	  if (handled) {
			for (;;) {
				  curBufNdx = rxAvlQue[rxNdxOut].index;
				  if (curBufNdx == -1) {
						len = -1;	// Nothing else received
//printf("read: nothing else received: rxNdxOut: %d curBufNdx: %d\n", rxNdxOut, curBufNdx);
						break;
				  }
				  len = rxAvlQue[rxNdxOut].len;
//printf("read: sending a packet up: rxNdxOut: %d curBufNdx: %d\n", rxNdxOut, curBufNdx);
				  NetReceive(NetRxPackets[curBufNdx], len - 4);
				  rxAvlQue[rxNdxOut].index = -1;	  // Free buffer
				  if (++rxNdxOut >= NUM_RX_BUFF) {
						rxNdxOut = 0;	  // Handle wrap
				  }
			}
	  } else {
			EmptyReads++;
			return (-1);	  // Nothing was received
	  }

	  return (len);
}


static int sendToNet(uchar * txbp, int len)
{
	  ulong tx_cmd_a, tx_cmd_b;
	  int i;
	  ulong * txbpl = (ulong *)txbp;

	  lastTxTag++;

#if   DEBUG
	  {
			printf("sendToNet: packet contents follows.\n");
			int i;
			int j = 0;
			for (i = 0; i < len; i++) {
				  if (++j == 20) {
						j = 0;
						printf("\n");
				  }
				  printf("%0.1x ", txbp[i]);
			}
			printf("\n");

//			printf("sendToNet: peek TX status: 0x%0.8x\n",
//				  *TX_STATUS_FIFO_PEEK);
	  }
#endif		// DEBUG

	  tx_cmd_a = (((ulong)txbp & 0x3) << 16) | 0x00003000 | len;
	  tx_cmd_b = (lastTxTag << 16) | len;

#if   DEBUG
	  printf("sendToNet: tx_cmd_a: 0x%0.8x tx_cmd_b: 0x%0.8x\n",
			tx_cmd_a, tx_cmd_b);
#endif		// DEBUG

	  *TX_FIFO_PORT = tx_cmd_a;
	  *TX_FIFO_PORT = tx_cmd_b;

	  for (i = (len+3)/sizeof(ulong); i > 0; i--) {
			*TX_FIFO_PORT = *txbpl++; 
	  }

	  *TX_CFG = TX_CFG_TX_ON;		// Enable transmitter

	  return (TRUE);
}

static int lan9118_write(volatile void *ptr, int len)
{
	  ulong startTime;
	  ulong timeout;
	  char statusStr[64];

	  if (len > ENET_MAX_MTU) {
			len = ENET_MAX_MTU;
	  }

	  // Copy the packet.
	  memcpy((void *)txbp, (void *)ptr, len);

	  // Drain the TX status fifo just in case there are old (good) statuses.
	  for (timeout=0; timeout<TX_TIMEOUT_COUNT; timeout++)
	  {
		  if ((*TX_FIFO_INF & TX_FIFO_TXSUSED_MSK) == 0) {
				break;
		  }
		  printf("lan9118_write: discarded old TX status\n");
	  }
	  if (timeout == TX_TIMEOUT_COUNT)	// timed out?  Yes--
	  {
		DumpCsrRegs();
		DumpMacRegs();
		DumpPhyRegs();
	  }

	  if (sendToNet((uchar *)txbp, len) == FALSE) {
			return (-1);
	  }

//printf("write: sent packet out: len: %d\n", len);

	  startTime = get_timer(0);
	  while (1) {
			if ((*TX_FIFO_INF & TX_FIFO_TXSUSED_MSK) == 0) {
				  // No status yet
				  if ((get_timer(0) - startTime) > TX_TIMEOUT) {
						return (-1);
				  }
			} else {
				  ulong txStatus = *TX_STATUS_FIFO_PORT;

				  if ((txStatus & TX_STATUS_FIFO_ES) == TX_STATUS_FIFO_ES) {
						sprintf(statusStr, "lan9118_write: error "
							  "status: 0x%0.8x\n", txStatus);
						LAN9118_WARN(statusStr);
						return (-1);
				  } else {
						*TX_CFG |= TX_CFG_STOP_TX; // Stop transmitter
						return (len);				// successful send
				  }
			}
	  }
}

int	eth_init(bd_t *bd)
{
	return lan9118_open(bd);
}

void eth_halt(void)
{
	lan9118_close();
}

int	eth_rx(void)
{
	int	r;

	r = lan9118_read();

	return r;
}

int	eth_send(volatile void *packet, int length)
{
	return lan9118_write(packet, length);
}

#endif		// #ifdef CONFIG_DRIVER_SMSC9118
