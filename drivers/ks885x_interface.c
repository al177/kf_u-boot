#include <common.h>
#include <malloc.h>
#include <net.h>

#ifdef CONFIG_MICREL_ETH_KS8851
#include "ks885x_interface.h"
#include "ks885x_target.h"
#include "ks8851_registers.h"

/* network order */
BYTE DEFAULT_MAC_ADDRESS[] = {0x08, 0x00, 0x28, 0x01, 0x4D, 0x4D};

#define DEVICE_NAME "ks8851snl"

static HARDWARE  gHardware;

/*
 * PrintHexData
 *
 * Description
 *  print the Hex data in %02X, %04X, %08X format
 *
 */
void PrintHexData(void *p, int nSizeInByte, PRINTFFLAG PrintfFlag)
{

	int i, length;
	DWORD dw;
	LPBYTE pByte;
	LPWORD pWord;
	LPDWORD pDword;
	printf("\r\n");
	switch (PrintfFlag) {
	case PRINTF_BYTE:
		length = nSizeInByte;
		pByte = p;
		for (i = 0; i < length; i++) {
			dw = pByte[i];
			printf("0x%02X", dw);
			if ((i % 16) == 15)
				printf("\r\n");
		}
		break;

	case PRINTF_WORD:
		length = nSizeInByte / sizeof(WORD);
		pWord = p;
		for (i = 0; i < length; i++) {
			dw = pWord[i];
			printf("0x%04X", dw);
			if ((i % 8) == 7)
				printf("\r\n");
		}
		break;

	case PRINTF_DWORD:
		length = nSizeInByte / sizeof(DWORD);
		pDword = p;
		for (i = 0; i < length; i++) {
			dw = pDword[i];
			printf("0x%08X", dw);
			if ((i % 4) == 3)
				printf("\r\n");
		}
		break;

	default:
		length = 0;
	}
}

/*
 * U-Boot ethernet interface
 */

void eth_reset(void)
{
	return eth_reset();
}

void ks8851_eth_reset(void *pHardware, unsigned type)
{
	WORD w;

	/* Write 1 to active reset and wait */
	HW_WRITE_WORD(pHardware, REG_RESET_CTRL, type);
	udelay(500000);

	/* Write 0 to clear reset and wait */
	HW_WRITE_WORD(pHardware, REG_RESET_CTRL, 0);
	udelay(500000);
}

void ks8851_eth_halt(struct eth_device *dev)
{
	return;
}

int ks8851_get_ethaddr (bd_t * bd)
{

	PHARDWARE pHardware = &gHardware;

	int ethaddr_env_size = 0;
	int ethaddr_env_set = 0;
	int ethaddr_rom_size = 0;
	int ethaddr_rom_set = 0;

	int i;

	char *s = NULL;
	char *t = NULL;
	
	char ethaddr_tmp_data[64];
	char ethaddr_env_data[64];	
	
	char ethaddr_rom_data[6];
	char ethaddr_tmp[6];

	char *ethaddr_data, ethaddr_pattern[] = "00:00:00:00:00:00";

	unsigned short *pw;
  
	/* Check if we have ethaddr set in environment */
	ethaddr_env_size = getenv_r ("ethaddr", ethaddr_tmp_data,
					sizeof (ethaddr_tmp_data));
	if (ethaddr_env_size == sizeof(ethaddr_pattern)) {
		ethaddr_env_set = 1;
		s = ethaddr_tmp_data;
		for (i = 0; i < 6; ++i) {
			ethaddr_tmp[i] = s ? simple_strtoul (s, &t, 16) : 0;
			if (s)
				s = (*t) ? t + 1 : t;
		}
    ethaddr_data = (char *)ethaddr_tmp;
		sprintf(ethaddr_env_data, "%02X:%02X:%02X:%02X:%02X:%02X\n",
			ethaddr_tmp[0], ethaddr_tmp[1],
			ethaddr_tmp[2], ethaddr_tmp[3],
			ethaddr_tmp[4], ethaddr_tmp[5]);
	}

  /* Check if we have ethaddr set in ethernet rom memory */
	HW_READ_WORD(pHardware, REG_MAC_ADDR_4, &ethaddr_rom_data[4]);
	HW_READ_WORD(pHardware, REG_MAC_ADDR_2, &ethaddr_rom_data[2]);
	HW_READ_WORD(pHardware, REG_MAC_ADDR_0, &ethaddr_rom_data[0]);
	
	printf("ethaddr rom: %02X:%02X:%02X:%02X:%02X:%02X\n",
			ethaddr_rom_data[5], ethaddr_rom_data[4],
			ethaddr_rom_data[3], ethaddr_rom_data[2],
			ethaddr_rom_data[1], ethaddr_rom_data[0]);

	if (ethaddr_rom_data != sizeof(ethaddr_pattern)) {
		ethaddr_rom_set = 1;
	}
	
	/* ethaddr, both environment & rom invalid */
	if (!ethaddr_env_set && !ethaddr_rom_set) {
		printf ("%s: ethaddr env & rom are not valid! cannot continue\n",
				DEVICE_NAME);
		return -1;

	/* ethaddr, environment invalid & rom valid */	
	} else if (!ethaddr_env_set && ethaddr_rom_set) {
		printf ("%s: ethaddr env is not valid! cannot continue\n",
				DEVICE_NAME);
		return -1;

	/* ethaddr, environment valid & rom invalid */
 	} else if (ethaddr_env_set && !ethaddr_rom_set) {
		printf ("%s: ethaddr rom is not valid! using env ethaddr\n",
				DEVICE_NAME);

#if 0
	/* Write QMU MAC address */
	pw = (unsigned short *)&DEFAULT_MAC_ADDRESS[0];
	HW_WRITE_WORD(pHardware, REG_MAC_ADDR_4, *pw++);
	HW_WRITE_WORD(pHardware, REG_MAC_ADDR_2, *pw++);
	HW_WRITE_WORD(pHardware, REG_MAC_ADDR_0, *pw++);

	sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
		DEFAULT_MAC_ADDRESS[0], DEFAULT_MAC_ADDRESS[1],
		DEFAULT_MAC_ADDRESS[2], DEFAULT_MAC_ADDRESS[3],
		DEFAULT_MAC_ADDRESS[4], DEFAULT_MAC_ADDRESS[5]);

	printf("set environment from HW MAC addr = \"%s\"\n", ethaddr);
	setenv("ethaddr", ethaddr);
#endif				
  
  /* ethaddr, both environment & rom valid, continue */
	}
	
  	
  return 0;	

}

static int readSingleFrame(FR_HEADER_INFO *prxFrameHeader)
{
	WORD wDataLen, w;
	BYTE *pOut;
	PHARDWARE pHardware = &gHardware;
	pHardware->m_uFramesRemained--;
	pHardware->m_uCurFrameIndex++;

#if DEBUG
	printf("ks8851recv: remained frames =%d\n",
				pHardware->m_uFramesRemained);
#endif

	if (prxFrameHeader->rxLength == 0) {
#if DEBUG	
		printf("ks8851_recv error: the receive length is zero\n");
#endif		
		HW_READ_WORD(pHardware, REG_RXQ_CMD, &w);
		w |= RXQ_CMD_FREE_PACKET;
		HW_WRITE_WORD(pHardware, REG_RXQ_CMD, w);
#if 0
		HW_READ_START(pHardware);
		HW_READ_END(pHardware);
#endif
		return 0;
	}

	if (!(prxFrameHeader)->rxStatus & (RX_VALID | RX_ICMP_ERROR | \
					RX_IP_ERROR | RX_TCP_ERROR | \
					RX_UDP_ERROR)) {
#if DEBUG					
		printf("ks8851_recv error: received packet is invalid\n");
#endif		
		HW_READ_WORD(pHardware, REG_RXQ_CMD, &w);
		w |= RXQ_CMD_FREE_PACKET;
		HW_WRITE_WORD(pHardware, REG_RXQ_CMD, w);
#if 0
		HW_READ_START(pHardware);
		HW_READ_END(pHardware);
#endif
		return 0;
	}

	if (prxFrameHeader->rxLength > RX_BUF_SIZE) {
#if DEBUG	
		printf("ks8851_recv error: the received packet \
			length=%d is bigger than buffer size=%d, \
			can not handle it\n",
			prxFrameHeader->rxLength,
			RX_BUF_SIZE);
#endif
		HW_READ_WORD(pHardware, REG_RXQ_CMD, &w);
		w |= RXQ_CMD_FREE_PACKET;
		HW_WRITE_WORD(pHardware, REG_RXQ_CMD, w);
#if 0
		HW_READ_START(pHardware);
		HW_READ_END(pHardware);
#endif
		return 0;
	}

	KS8851SNL_READ_BUFFER(pHardware,
			pHardware->m_recvBuffer,
			&pOut,
			prxFrameHeader->rxLength,
			&wDataLen);
	NetReceive(pOut, prxFrameHeader->rxLength);

#if DEBUG
	printf("\nrxLength=%d wDataLen=%d\n",
			FrameHeader->rxLength,
			wDataLen);
	PrintHexData(pOut, prxFrameHeader->rxLength, PRINTF_BYTE);
	printf("***********************************************\n");
#endif

	return wDataLen;

}

int ks8851_eth_recv(struct eth_device *dev)
{
	unsigned short uh, i, IntStatus;
	DWORD dwDataLen = 0;
	PHARDWARE pHardware = &gHardware;
	ULONG rxFrameCount;
	FR_HEADER_INFO *prxFrameHeader = NULL;

	/* Are there any frames pending to be processed? */
	if (pHardware->m_uFramesRemained > 0) {

		prxFrameHeader = pHardware->m_pRevHdrInfo;
		dwDataLen = readSingleFrame(
			(prxFrameHeader+pHardware->m_uCurFrameIndex));

	} else {

		/* check if we have more packets based on rx interrupt */
		HW_READ_WORD(pHardware, REG_INT_STATUS, &IntStatus);

		if (IntStatus == 0) {
#if DEBUG
			printf("ks8851: no pending interrupt\n");
#endif
			return 0;
		}

		/* Acknowledge the rx interrupt */
		HW_WRITE_WORD(pHardware, REG_INT_STATUS, IntStatus);

		if (IntStatus & INT_RX) {

			/*
			 * Read current total amount of received
			 * frame count from RXFCTR
			 */

			HW_READ_WORD(pHardware, REG_RX_FRAME_CNT_THRES, &uh);
			rxFrameCount = (uh & 0xFF00) >> 8;

#if DEBUG
			printf("frame count = %d\n", rxFrameCount);
#endif

			pHardware->m_uFramesRemained = rxFrameCount;

			if (rxFrameCount == 0)
				return 0;

			if (rxFrameCount >= pHardware->m_uCurRecvFrams) {

				free(pHardware->m_pRevHdrInfo);
				pHardware->m_uCurRecvFrams = rxFrameCount + 5;
				pHardware->m_pRevHdrInfo =
					(FR_HEADER_INFO *)malloc(
					sizeof(FR_HEADER_INFO)*(
					rxFrameCount + 5));
				if (!pHardware->m_pRevHdrInfo) {
					printf("ks8851_eth_recv: fail \
						to allocate memory\n");
					pHardware->m_uCurRecvFrams = 0;
					return 0;
				}
			}

			prxFrameHeader = pHardware->m_pRevHdrInfo;

			/* read all head information */
			for (i = 0; i < rxFrameCount; i++) {

				/* Checking Received packet status */
				HW_READ_WORD(pHardware,
					REG_RX_FHR_STATUS,
					&(prxFrameHeader+i)->rxStatus);

				/*
				 * Get received packet length from hardware
				 * packet memory
				 */
				HW_READ_WORD(pHardware,
					REG_RX_FHR_BYTE_CNT,
					&(prxFrameHeader+i)->rxLength);

				/*
				 * Record read packet length in DWORD
				 * alignment
				 */
				(prxFrameHeader+i)->rxLength &=
							RX_BYTE_CNT_MASK;
			}

			/* read a single frame */
			dwDataLen = readSingleFrame(prxFrameHeader);

		} else {
#if DEBUG
			printf("the interrupt is not a receiving\n");
#endif
		}
	}
	return dwDataLen;
}

/* the send function will send a complete ethernet frame */
int ks8851_eth_send(struct eth_device *dev,
		volatile void *packet,
		int length)
{
	PHARDWARE pHardware = &gHardware;
	ULONG uiPacketLength;
	BOOL fReturn = FALSE;

	uiPacketLength = length;
	GET_DATA_ALIGNMENT(length, &uiPacketLength);
	HW_WRITE_START(pHardware);

#ifdef KS8851_SNL
	KS8851SNL_DATA_WRITE((PBYTE)packet, length, uiPacketLength, &fReturn);
#else
{
	LPBYTE pOut = NULL;
	USHORT uiPacketLength = legnth;
	HW_WRITE_DATA_HEADER(pHardware, ((USHORT *)&uiPacketLength));
	HW_WRITE_DATA_BUFFER(pHardware,
				packet,
				&pOut,
				length,
				((USHORT *)&uiPacketLength));
}
#endif

	HW_WRITE_END(pHardware);
	/*
	 * Issue TXQ Command (Enqueue Tx frame from TX buffer into
	 * TXQ for transmit)
	 */
	HW_WRITE_WORD(pHardware, REG_TXQ_CMD, TXQ_ENQUEUE);
	return 1;
}

int ks8851_eth_init(bd_t *bd)
{

	WORD txCntl, rxCntl, uRegData, w, intMask;
	char ethaddr[64];
	unsigned short *pw;
	PHARDWARE pHardware = &gHardware;

	spi_init();
	printf("\nks8851snl ethernet initialized\n");

	pHardware->m_uCurFrameIndex = 0;
	pHardware->m_uFramesRemained = 0;
	pHardware->m_uCurRecvFrams = MAX_RECV_FRAMES;
	pHardware->m_pRevHdrInfo = (FR_HEADER_INFO *)malloc(
				sizeof(FR_HEADER_INFO)*MAX_RECV_FRAMES);

	/* Reset eth device */
	ks8851_eth_reset(pHardware, GLOBAL_SOFTWARE_RESET);

	/* Read the device chip ID */
	HW_READ_WORD(pHardware, REG_CHIP_ID, &uRegData);
	if ((uRegData & 0xFFF0) != 0x8870) {
		printf("ks8851 check chip id fail, \
			readback value=0x%x \n", uRegData);
		return 0;
	}
	
	/* Verify if ethaddr from environment is set */
	ks8851_get_ethaddr (bd);	

	/* Setup Transmit Frame Data Pointer Auto-Increment (TXFDPR) */
	HW_WRITE_WORD(pHardware, REG_TX_ADDR_PTR, ADDR_PTR_AUTO_INC);

	/* Enable QMU TxQ Auto-Enqueue frame */
	HW_WRITE_WORD(pHardware, REG_TXQ_CMD, TXQ_AUTO_ENQUEUE);

	/* Configure the QMU transmit module function */
	txCntl = (
		TX_CTRL_ICMP_CHECKSUM |
		/* TX_CTRL_UDP_CHECKSUM | */
		TX_CTRL_TCP_CHECKSUM |
		TX_CTRL_IP_CHECKSUM |
		TX_CTRL_FLOW_ENABLE |
		TX_CTRL_PAD_ENABLE |
		TX_CTRL_CRC_ENABLE
	);

	HW_WRITE_WORD(pHardware, REG_TX_CTRL, txCntl);

	/* Setup Receive Frame Data Pointer Auto-Increment */
	HW_WRITE_WORD(pHardware, REG_RX_ADDR_PTR, ADDR_PTR_AUTO_INC);

	/* Setup Receive Frame Threshold - 1 frame (RXFCTFC) */
	HW_WRITE_WORD(pHardware, REG_RX_FRAME_CNT_THRES,
			1 & RX_FRAME_THRESHOLD_MASK);

	/* Configure the receive function */
	rxCntl = (
		RX_CTRL_UDP_CHECKSUM |
		RX_CTRL_TCP_CHECKSUM |
		RX_CTRL_IP_CHECKSUM |
		RX_CTRL_MAC_FILTER |
		RX_CTRL_FLOW_ENABLE |
		RX_CTRL_BAD_PACKET |
		RX_CTRL_ALL_MULTICAST |
		RX_CTRL_UNICAST |
		RX_CTRL_PROMISCUOUS
#if 0
		RX_CTRL_MULTICAST |
		RX_CTRL_BROADCAST |
		RX_CTRL_STRIP_CRC |
		RX_CTRL_INVERSE_FILTER
#endif
	);

	HW_WRITE_WORD(pHardware, REG_RX_CTRL1, rxCntl);

	/* Setup RxQ Command Control (RXQCR) */
	HW_WRITE_WORD(pHardware, REG_RXQ_CMD, RXQ_CMD_CNTL);

#if 0
	/* Restart Port 1 auto-negotiation */
	HW_WRITE_WORD(pHardware, REG_PORT_CTRL, PORT_AUTO_NEG_RESTART);

	/* Disable Port 1 auto-negotiation */
	HW_READ_WORD(pHardware, REG_PORT_CTRL, &txCntl);
#endif

	/* FixMe for now force 10BT half duplex */
	HW_WRITE_WORD(pHardware, REG_PORT_CTRL, 0x0040);
	HW_READ_WORD(pHardware, REG_PORT_STATUS, &txCntl);
	printf("port status information = 0x%x \n", txCntl);

	/* Clear the interrupts status */
	HW_WRITE_WORD(pHardware, REG_INT_STATUS, 0xFFFF);

	/* Enables QMU Transmit (TXCR). */
	HW_READ_WORD(pHardware, REG_TX_CTRL, &txCntl);
	txCntl |= TX_CTRL_ENABLE;
	HW_WRITE_WORD(pHardware, REG_TX_CTRL, txCntl);

	/* Enables QMU Receive (RXCR1). */
	HW_READ_WORD(pHardware, REG_RX_CTRL1, &rxCntl);
	rxCntl |= RX_CTRL_ENABLE;
	HW_WRITE_WORD(pHardware, REG_RX_CTRL1, rxCntl);

	intMask = (
		INT_PHY |
		INT_RX |
		INT_TX |
		INT_RX_OVERRUN |
		INT_TX_STOPPED |
		INT_RX_STOPPED |
#ifdef KS8851_SNL
		INT_RX_SPI_ERROR |
#endif
		INT_TX_SPACE
	);

	intMask = INT_RX;
	HW_WRITE_WORD(pHardware, REG_INT_MASK, intMask);

	return 0;

	/* RX Frame Count Threshold Enable and Auto-Dequeue RXQ Frame Enable */
	HW_READ_WORD(pHardware, REG_RXQ_CMD, &w);
	w |= RXQ_FRAME_CNT_INT;
	HW_WRITE_WORD(pHardware, REG_RXQ_CMD, w);

}

#include "ks885x_target.c"

#endif

