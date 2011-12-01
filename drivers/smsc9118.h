//--------------------------------------------------------------------------
//
// File name:      smsc9118.h
//
// Abstract:      Address map and register definitions for SMSC LAN9118
//          ethernet controller.
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
 *  	09-27-06 MDG		First Release
 *			modified for ARM platform
 *----------------------------------------------------------------------------*/

#ifdef CONFIG_DRIVER_SMSC9118

//*************************************************************************
 //  GLOBAL DEFINITIONS

//*************************************************************************
#define 	LAN9118_WARN(s) (printf("%s", s))

#define DRIVER_VERSION          0x101
#define	BUILD_NUMBER			"092706"

//*************************************************************************
 //  DATA STRUCTURE DEFINITIONS

//*************************************************************************


#ifndef 	_SMSC9118_H
#define 	_SMSC9118_H

#ifndef	CONFIG_SMSC9118_BASE
#error	"CONFIG_SMSC9118_BASE is not defined."
#else
#define 	SMSC9118_BASE		CONFIG_SMSC9118_BASE
#endif

#define 	MAC_TIMEOUT 		200
#define 	PHY_TIMEOUT			200
//#define	PHY_AN_TIMEOUT	  3000 * 1000 // 3 seconds
#define 	PHY_AN_TIMEOUT		10	// 3 seconds
#define 	SRST_TIMEOUT		100
#define 	TX_TIMEOUT			3000		// 3000 * 1/HZ
#define 	FFWD_TIMEOUT		100
#define 	PHY_ADDR			1
#define 	FALSE				0
#define 	TRUE				1

#define 	DELAY(n)	( {   \
						int _i = 100*n; \
						do { \
							  volatile ulong _temp; \
							  _temp = *BYTE_TEST; \
						} while (--_i); \
				  } )

struct rxQue {
	  int	index;		// Index into NetRxPackets[]
	  int	len;  		// Length of packet at this index
};

// Lan9118 memory map

// Control/Status Register Map (directly addressable registers)
#define RX_FIFO_PORT				(volatile ulong *)(SMSC9118_BASE + 0x0)
#define RX_FIFO_ALIAS_PORTS 		(volatile ulong *)(SMSC9118_BASE + 0x4)
#define TX_FIFO_PORT				(volatile ulong *)(SMSC9118_BASE + 0x20)
#define TX_FIFO_ALIAS_PORTS 		(volatile ulong *)(SMSC9118_BASE + 0x24)
#define RX_STATUS_FIFO_PORT 		(volatile ulong *)(SMSC9118_BASE + 0x40)
#define RX_STATUS_FIFO_PEEK 		(volatile ulong *)(SMSC9118_BASE + 0x44)
#define TX_STATUS_FIFO_PORT 		(volatile ulong *)(SMSC9118_BASE + 0x48)
#define TX_STATUS_FIFO_PEEK 		(volatile ulong *)(SMSC9118_BASE + 0x4C)
#define TX_STATUS_FIFO_ES 			(0x00008000)
#define TX_STATUS_FIFO_TAG_MSK		(0xffff0000)

#define ID_REV						(volatile ulong *)(SMSC9118_BASE + 0x50)
#define		ID_REV_ID_MASK			(0xFFFF0000)
#define 	ID_REV_CHIP_118   		(0x01180000)
#define 	ID_REV_CHIP_218   		(0x118A0000)
#define 	ID_REV_CHIP_211   		(0x92110000)
#define 	ID_REV_CHIP_221   		(0x92210000)
#define		ID_REV_REV_MASK			(0x0000FFFF)

#define IRQ_CFG 					(volatile ulong *)(SMSC9118_BASE + 0x54)
#define 	IRQ_CFG_MASTER_INT		(0x00001000)
#define 	IRQ_CFG_ENABLE			(0x00000100)
#define 	IRQ_CFG_IRQ_POL_HIGH	(0x00000010)
#define 	IRQ_CFG_IRQ_TYPE_PUPU	(0x00000001)

#define INT_STS 					(volatile ulong *)(SMSC9118_BASE + 0x58)
#define 	INT_STS_SW_INT			(0x80000000)
#define 	INT_STS_TXSTOP_INT		(0x02000000)
#define 	INT_STS_RXSTOP_INT		(0x01000000)
#define 	INT_STS_RXDFH_INT 		(0x00800000)
#define 	INT_STS_RXDF_INT  		(0x00400000)
#define 	INT_STS_TIOC_INT  		(0x00200000)
#define 	INT_STS_GPT_INT 		(0x00080000)
#define 	INT_STS_PHY_INT 		(0x00040000)
#define 	INT_STS_PMT_INT 		(0x00020000)
#define 	INT_STS_TXSO_INT  		(0x00010000)
#define 	INT_STS_RWT_INT 		(0x00008000)
#define 	INT_STS_RXE_INT 		(0x00004000)
#define 	INT_STS_TXE_INT 		(0x00002000)
#define 	INT_STS_ERX_INT 		(0x00001000)
#define 	INT_STS_TDFU_INT  		(0x00000800)
#define 	INT_STS_TDFO_INT  		(0x00000400)
#define 	INT_STS_TDFA_INT  		(0x00000200)
#define 	INT_STS_TSFF_INT  		(0x00000100)
#define 	INT_STS_TSFL_INT  		(0x00000080)
#define 	INT_STS_RDFO_INT  		(0x00000040)
#define 	INT_STS_RDFL_INT  		(0x00000020)
#define 	INT_STS_RSFF_INT  		(0x00000010)
#define 	INT_STS_RSFL_INT  		(0x00000008)
#define 	INT_STS_GPIO2_INT 		(0x00000004)
#define 	INT_STS_GPIO1_INT 		(0x00000002)
#define 	INT_STS_GPIO0_INT 		(0x00000001)

#define INT_EN						(volatile ulong *)(SMSC9118_BASE + 0x5C)
#define 	INT_EN_SW_INT_EN  		(0x80000000)
#define 	INT_EN_TXSTOP_INT_EN	(0x02000000)
#define 	INT_EN_RXSTOP_INT_EN	(0x01000000)
#define 	INT_EN_RXDFH_INT_EN 	(0x00800000)
#define 	INT_EN_RXDF_INT_EN		(0x00400000)
#define 	INT_EN_TIOC_INT_EN		(0x00200000)
#define 	INT_EN_GPT_INT_EN 		(0x00080000)
#define 	INT_EN_PHY_INT_EN 		(0x00040000)
#define 	INT_EN_PMT_INT_EN 		(0x00020000)
#define 	INT_EN_TXSO_INT_EN		(0x00010000)
#define 	INT_EN_RWT_INT_EN 		(0x00008000)
#define 	INT_EN_RXE_INT_EN 		(0x00004000)
#define 	INT_EN_TXE_INT_EN 		(0x00002000)
#define 	INT_EN_ERX_INT_EN 		(0x00001000)
#define 	INT_EN_TDFU_INT_EN		(0x00000800)
#define 	INT_EN_TDFO_INT_EN		(0x00000400)
#define 	INT_EN_TDFA_INT_EN		(0x00000200)
#define 	INT_EN_TSFF_INT_EN		(0x00000100)
#define 	INT_EN_TSFL_INT_EN		(0x00000080)
#define 	INT_EN_RDFO_INT_EN		(0x00000040)
#define 	INT_EN_RDFL_INT_EN		(0x00000020)
#define 	INT_EN_RSFF_INT_EN		(0x00000010)
#define 	INT_EN_RSFL_INT_EN		(0x00000008)
#define 	INT_EN_GPIO2_EN 		(0x00000004)
#define 	INT_EN_GPIO1_EN 		(0x00000002)
#define 	INT_EN_GPIO0_EN 		(0x00000001)

#define BYTE_TEST		  			(volatile ulong *)(SMSC9118_BASE + 0x64)
#define 	BYTE_TEST_VAL			(0x87654321)

#define FIFO_INT		  			(volatile ulong *)(SMSC9118_BASE + 0x68)
#define 	FIFO_INT_TDAL_MSK 		(0xFF000000)
#define 	FIFO_INT_TSL_MSK  		(0x00FF0000)
#define 	FIFO_INT_RDAL_MSK 		(0x0000FF00)
#define 	FIFO_INT_RSL_MSK  		(0x000000FF)

#define RX_CFG						(volatile ulong *)(SMSC9118_BASE + 0x6C)
#define 	RX_CFG_END_ALIGN4 		(0x00000000)
#define 	RX_CFG_END_ALIGN16		(0x40000000)
#define 	RX_CFG_END_ALIGN32		(0x80000000)
#define 	RX_CFG_FORCE_DISCARD	(0x00008000)
#define 	RX_CFG_RXDOFF_MSK 		(0x00003C00)
#define 	RX_CFG_RXBAD			(0x00000001)

#define TX_CFG						(volatile ulong *)(SMSC9118_BASE + 0x70)
#define 	TX_CFG_TXS_DUMP 		(0x00008000)
#define 	TX_CFG_TXD_DUMP 		(0x00004000)
#define 	TX_CFG_TXSAO			(0x00000004)
#define 	TX_CFG_TX_ON			(0x00000002)
#define 	TX_CFG_STOP_TX			(0x00000001)

#define HW_CFG						(volatile ulong *)(SMSC9118_BASE + 0x74)
#define 	HW_CFG_TTM		  		(0x00200000)
#define 	HW_CFG_SF		  		(0x00100000)
#define 	HW_CFG_TX_FIF_SZ_MSK	(0x000F0000)
#define 	HW_CFG_TR_MSK			(0x00003000)
#define 	HW_CFG_BITMD_MSK  		(0x00000004)
#define 	HW_CFG_BITMD_32 		(0x00000004)
#define 	HW_CFG_SRST_TO			(0x00000002)
#define 	HW_CFG_SRST 	  		(0x00000001)

#define RX_DP_CTL		  			(volatile ulong *)(SMSC9118_BASE + 0x78)
#define 	RX_DP_FFWD		  		(0x80000000)
#define 	RX_DP_RX_FFWD_MSK 		(0x00000FFF)

#define RX_FIFO_INF 	  			(volatile ulong *)(SMSC9118_BASE + 0x7C)
#define 	RX_FIFO_RXSUSED_MSK 	(0x00FF0000)
#define 	RX_FIFO_RXDUSED_MSK 	(0x0000FFFF)

#define TX_FIFO_INF 	  			(volatile ulong *)(SMSC9118_BASE + 0x80)
#define 	TX_FIFO_TXSUSED_MSK 	(0x00FF0000)
#define 	TX_FIFO_TDFREE_MSK		(0x0000FFFF)

#define PWR_MGMT		  			(volatile ulong *)(SMSC9118_BASE + 0x84)
#define 	PWR_MGMT_PM_MODE_MSK	(0x00030000)
#define 	PWR_MGMT_PM_MODE_MSK_LE (0x00000003)
#define 	PWR_MGMT_PM__D0 		(0x00000000)
#define 	PWR_MGMT_PM__D1 		(0x00010000)
#define 	PWR_MGMT_PM__D2 		(0x00020000)
#define 	PWR_MGMT_PHY_RST  		(0x00000400)
#define 	PWR_MGMT_WOL_EN 		(0x00000200)
#define 	PWR_MGMT_ED_EN			(0x00000100)
#define 	PWR_MGMT_PME_TYPE_PUPU	(0x00000040)
#define 	PWR_MGMT_WUPS_MSK 		(0x00000030)
#define 	PWR_MGMT_WUPS_NOWU		(0x00000000)
#define 	PWR_MGMT_WUPS_D2D0		(0x00000010)
#define 	PWR_MGMT_WUPS_D1D0		(0x00000020)
#define 	PWR_MGMT_WUPS_UNDEF 	(0x00000030)
#define 	PWR_MGMT_PME_IND_PUL	(0x00000008)
#define 	PWR_MGMT_PME_POL_HIGH	(0x00000004)
#define 	PWR_MGMT_PME_EN 		(0x00000002)
#define 	PWR_MGMT_PME_READY		(0x00000001)

#define GPIO_CFG		  			(volatile ulong *)(SMSC9118_BASE + 0x88)
#define 	GPIO_CFG_LEDx_MSK 		(0x70000000)
#define 	GPIO_CFG_LED1_EN  		(0x10000000)
#define 	GPIO_CFG_LED2_EN  		(0x20000000)
#define 	GPIO_CFG_LED3_EN  		(0x40000000)
#define 	GPIO_CFG_GPIOBUFn_MSK	(0x00070000)
#define 	GPIO_CFG_GPIOBUF0_PUPU	(0x00010000)
#define 	GPIO_CFG_GPIOBUF1_PUPU	(0x00020000)
#define 	GPIO_CFG_GPIOBUF2_PUPU	(0x00040000)
#define 	GPIO_CFG_GPDIRn_MSK 	(0x00000700)
#define 	GPIO_CFG_GPIOBUF0_OUT	(0x00000100)
#define 	GPIO_CFG_GPIOBUF1_OUT	(0x00000200)
#define 	GPIO_CFG_GPIOBUF2_OUT	(0x00000400)
#define 	GPIO_CFG_GPIOD_MSK		(0x00000007)
#define 	GPIO_CFG_GPIOD0 		(0x00000001)
#define 	GPIO_CFG_GPIOD1 		(0x00000002)
#define 	GPIO_CFG_GPIOD2 		(0x00000004)

#define GPT_CFG 					(volatile ulong *)(SMSC9118_BASE + 0x8C)
#define 	GPT_CFG_TIMER_EN  		(0x20000000)
#define 	GPT_CFG_GPT_LOAD_MSK	(0x0000FFFF)

#define	GPT_CNT 					(volatile ulong *)(SMSC9118_BASE + 0x90)
#define 	GPT_CNT_MSK 	  		(0x0000FFFF)

#define FPGA_REV		  			(volatile ulong *)(SMSC9118_BASE + 0x94)

#define ENDIAN						(volatile ulong *)(SMSC9118_BASE + 0x98)
#define 	ENDIAN_BIG		  		(0xFFFFFFFF)

#define FREE_RUN		  			(volatile ulong *)(SMSC9118_BASE + 0x9C)
#define 	FREE_RUN_FR_CNT_MSK 	(0xFFFFFFFF)

#define RX_DROP 					(volatile ulong *)(SMSC9118_BASE + 0xA0)
#define 	RX_DROP_RX_DFC_MSK		(0xFFFFFFFF)

#define MAC_CSR_CMD 	  			(volatile ulong *)(SMSC9118_BASE + 0xA4)
#define 	MAC_CSR_CMD_CSR_BUSY	(0x80000000)
#define 	MAC_CSR_CMD_RNW 		(0x40000000)
#define 	MAC_RD_CMD(Reg)   		((Reg & 0x000000FF) | \
									 (MAC_CSR_CMD_CSR_BUSY | MAC_CSR_CMD_RNW))
#define 	MAC_WR_CMD(Reg)   		((Reg & 0x000000FF) | \
									 (MAC_CSR_CMD_CSR_BUSY))

#define MAC_CSR_DATA				(volatile ulong *)(SMSC9118_BASE + 0xA8)

#define AFC_CFG 					(volatile ulong *)(SMSC9118_BASE + 0xAC)
#define 	AFC_CFG_AFC_HI_MSK		(0x00FF0000)
#define 	AFC_CFG_AFC_LO_MSK		(0x0000FF00)

#define E2P_CMD 					(volatile ulong *)(SMSC9118_BASE + 0xB0)
#define E2P_DATA		  			(volatile ulong *)(SMSC9118_BASE + 0xB4)

// MAC Control and Status Registers (accessed through MAC_CSR_CMD/_DATA regs)
#define MAC_CR						(0x1)
#define 	MAC_CR_RXALL			(0x80000000)
#define 	MAC_CR_HBDIS			(0x10000000)
#define 	MAC_CR_RCVOWN			(0x00800000)
#define 	MAC_CR_LOOPBK			(0x00200000)
#define 	MAC_CR_FDPX 	  		(0x00100000)
#define 	MAC_CR_MCPAS			(0x00080000)
#define 	MAC_CR_PRMS 	  		(0x00040000)
#define 	MAC_CR_INVFILT			(0x00020000)
#define 	MAC_CR_PASSBAD			(0x00010000)
#define 	MAC_CR_HFILT			(0x00008000)
#define 	MAC_CR_HPFILT			(0x00002000)
#define 	MAC_CR_LCOLL			(0x00001000)
#define 	MAC_CR_BCAST			(0x00000800)
#define 	MAC_CR_DISRTY			(0x00000400)
#define 	MAC_CR_PADSTR			(0x00000100)
#define 	MAC_CR_BOLMT_MSK  		(0x000000C0)
#define 	MAC_CR_BOLMT_10 		(0x00000000)
#define 	MAC_CR_BOLMT_8			(0x00000040)
#define 	MAC_CR_BOLMT_4			(0x00000080)
#define 	MAC_CR_BOLMT_1			(0x000000C0)
#define 	MAC_CR_DFCHK			(0x00000020)
#define 	MAC_CR_TXEN 	  		(0x00000008)
#define 	MAC_CR_RXEN 	  		(0x00000004)

#define MAC_ADDRH		  			(0x2)
#define 	MAC_ADDRH_MSK			(0x0000FFFF)

#define MAC_ADDRL		  			(0x3)
#define 	MAC_ADDRL_MSK			(0xFFFFFFFF)

#define MAC_HASHH		  			(0x4)
#define 	MAC_HASHH_MSK			(0xFFFFFFFF)

#define MAC_HASHL		  			(0x5)
#define 	MAC_HASHL_MSK			(0xFFFFFFFF)

#define MAC_MIIACC		  			(0x6)
#define 	MAC_MIIACC_MII_WRITE	(0x00000002)
#define 	MAC_MIIACC_MII_BUSY 	(0x00000001)
#define 	MAC_MII_RD_CMD(Addr,Reg)	(((Addr & 0x1f) << 11) | \
										 ((Reg & 0x1f)) << 6)
#define 	MAC_MII_WR_CMD(Addr,Reg)	(((Addr & 0x1f) << 11) | \
							  			 ((Reg & 0x1f) << 6) | \
							  			 MAC_MIIACC_MII_WRITE)

#define MAC_MIIDATA 	  			(0x7)
#define 	MAC_MIIDATA_MSK 		(0x0000FFFF)
#define 	MAC_MII_DATA(Data)		(Data & MAC_MIIDATA_MSK)

#define MAC_FLOW		  			(0x8)
#define 	MAC_FLOW_FCPT_MSK 		(0xFFFF0000)
#define 	MAC_FLOW_FCPASS 		(0x00000004)
#define 	MAC_FLOW_FCEN			(0x00000002)
#define 	MAC_FLOW_FCBSY			(0x00000001)

#define MAC_VLAN1		  			(0x9)
#define MAC_VLAN2		  			(0xA)
#define MAC_WUFF		  			(0xB)

#define MAC_WUCSR		  			(0xC)
#define 	MAC_WUCSR_GUE			(0x00000200)
#define 	MAC_WUCSR_WUFR			(0x00000040)
#define 	MAC_WUCSR_MPR			(0x00000020)
#define 	MAC_WUCSR_WUEN			(0x00000004)
#define 	MAC_WUCSR_MPEN			(0x00000002)

// PHY Control and Status Registers (accessed through MAC_MIIACC/_MIIDATA regs)
#define PHY_BCR 					(0x0)
#define 	PHY_BCR_RST 	  		(0x8000)
#define 	PHY_BCR_LOOPBK			(0x4000)
#define 	PHY_BCR_SS		  		(0x2000)
#define 	PHY_BCR_ANE 	  		(0x1000)
#define 	PHY_BCR_PWRDN			(0x0800)
#define 	PHY_BCR_RSTAN			(0x0200)
#define 	PHY_BCR_FDPLX			(0x0100)
#define 	PHY_BCR_COLLTST 		(0x0080)

#define PHY_BSR 					(0x1)
#define 	PHY_BSR_100_T4_ABLE 	(0x8000)
#define 	PHY_BSR_100_TX_FDPLX	(0x4000)
#define 	PHY_BSR_100_TX_HDPLX	(0x2000)
#define 	PHY_BSR_10_FDPLX  		(0x1000)
#define 	PHY_BSR_10_HDPLX  		(0x0800)
#define 	PHY_BSR_ANC 	  		(0x0020)
#define 	PHY_BSR_REM_FAULT 		(0x0010)
#define 	PHY_BSR_AN_ABLE 		(0x0008)
#define 	PHY_BSR_LINK_STATUS 	(0x0004)
#define 	PHY_BSR_JAB_DET 		(0x0002)
#define 	PHY_BSR_EXT_CAP 		(0x0001)

#define PHY_ID1 					(0x2)
#define 	PHY_ID1_MSK 	  		(0xFFFF)
#define 	PHY_ID1_LAN9118 		(0x0007)
#define 	PHY_ID1_LAN9218 		(PHY_ID1_LAN9118)

#define PHY_ID2 					(0x3)
#define 	PHY_ID2_MSK 	  		(0xFFFF)
#define 	PHY_ID2_MODEL_MSK 		(0x03F0)
#define 	PHY_ID2_REV_MSK 		(0x000F)
#define 	PHY_ID2_LAN9118 		(0xC0D1)
#define 	PHY_ID2_LAN9218 		(0xC0C3)

#define PHY_ANAR		  			(0x4)
#define 	PHY_ANAR_NXTPG_CAP		(0x8000)
#define 	PHY_ANAR_REM_FAULT		(0x2000)
#define 	PHY_ANAR_PAUSE_OP_MSK	(0x0C00)
#define 	PHY_ANAR_PAUSE_OP_NONE	(0x0000)
#define 	PHY_ANAR_PAUSE_OP_ASLP	(0x0400)
#define 	PHY_ANAR_PAUSE_OP_SLP	(0x0800)
#define 	PHY_ANAR_PAUSE_OP_BOTH	(0x0C00)
#define 	PHY_ANAR_100_T4_ABLE	(0x0200)
#define 	PHY_ANAR_100_TX_FDPLX	(0x0100)
#define 	PHY_ANAR_100_TX_ABLE	(0x0080)
#define 	PHY_ANAR_10_FDPLX 		(0x0040)
#define 	PHY_ANAR_10_ABLE  		(0x0020)

#define PHY_ANLPAR		  			(0x5)
#define 	PHY_ANLPAR_NXTPG_CAP	(0x8000)
#define 	PHY_ANLPAR_ACK			(0x4000)
#define 	PHY_ANLPAR_REM_FAULT	(0x2000)
#define 	PHY_ANLPAR_PAUSE_CAP	(0x0400)
#define 	PHY_ANLPAR_100_T4_ABLE	(0x0200)
#define 	PHY_ANLPAR_100_TX_FDPLX (0x0100)
#define 	PHY_ANLPAR_100_TX_ABLE	(0x0080)
#define 	PHY_ANLPAR_10_FDPLX 	(0x0040)
#define 	PHY_ANLPAR_10_ABLE		(0x0020)

#define PHY_ANEXPR		  			(0x6)
#define 	PHY_ANEXPR_PARDET_FAULT (0x0010)
#define 	PHY_ANEXPR_LP_NXTPG_CAP (0x0008)
#define 	PHY_ANEXPR_NXTPG_CAP	(0x0004)
#define 	PHY_ANEXPR_NEWPG_REC	(0x0002)
#define 	PHY_ANEXPR_LP_AN_ABLE	(0x0001)

#define PHY_SILREV		  			(0x10)

#define PHY_MCSR		  			(0x11)
#define 	PHY_MCSR_FASTRIP  		(0x4000)
#define 	PHY_MCSR_EDPWRDOWN		(0x2000)
#define 	PHY_MCSR_LOWSQEN  		(0x0800)
#define 	PHY_MCSR_MDPREBP  		(0x0400)
#define 	PHY_MCSR_FASTEST  		(0x0100)
#define 	PHY_MCSR_PHYADBP  		(0x0008)
#define 	PHY_MCSR_FGLS			(0x0004)
#define 	PHY_MCSR_ENERGYON 		(0x0002)

#define PHY_SPMODES 	  			(0x12)

#define PHY_CSIR		  			(0x1B)
#define 	PHY_CSIR_SQEOFF 		(0x0800)
#define 	PHY_CSIR_FEFIEN 		(0x0020)
#define 	PHY_CSIR_XPOL			(0x0010)

#define PHY_ISR 					(0x1C)
#define 	PHY_ISR_INT7			(0x0080)
#define 	PHY_ISR_INT6			(0x0040)
#define 	PHY_ISR_INT5			(0x0020)
#define 	PHY_ISR_INT4			(0x0010)
#define 	PHY_ISR_INT3			(0x0008)
#define 	PHY_ISR_INT2			(0x0004)
#define 	PHY_ISR_INT1			(0x0002)

#define PHY_IMR 					(0x1E)
#define 	PHY_IMR_INT7			(0x0080)
#define 	PHY_IMR_INT6			(0x0040)
#define 	PHY_IMR_INT5			(0x0020)
#define 	PHY_IMR_INT4			(0x0010)
#define 	PHY_IMR_INT3			(0x0008)
#define 	PHY_IMR_INT2			(0x0004)
#define 	PHY_IMR_INT1			(0x0002)

#define PHY_PHYSCSR 	  			(0x1F)
#define 	PHY_PHYSCSR_ANDONE		(0x1000)
#define 	PHY_PHYSCSR_4B5B_EN 	(0x0040)
#define 	PHY_PHYSCSR_SPEED_MSK	(0x001C)
#define 	PHY_PHYSCSR_SPEED_10HD	(0x0004)
#define 	PHY_PHYSCSR_SPEED_10FD	(0x0014)
#define 	PHY_PHYSCSR_SPEED_100HD (0x0008)
#define 	PHY_PHYSCSR_SPEED_100FD (0x0018)
#endif		// #ifndef _SMSC9118_H

#endif		// CONFIG_DRIVER_SMSC9118
