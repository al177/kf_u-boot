#include "ks885x_snl.h"
#include "ks8851_registers.h"

/*
 * GET_KS8851SNL_RWBYTE_CMD
 *
 * Description
 *  assemble a KS8851 SNL command for reading/write a byte register value
 * Parameters
 *  BYTE addr (IN), Address to read/write
 *  BYTE act (IN), Action definition (ACTION_READ/ACTION_WRITE)
 *  PBYTE pco (OUT), Pointer to first byte command.
 *  PBYTE pc1 (OUT), Pointer to second byte command.
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 * 			sucessful (TRUE)/fail (FALSE)
 * Return
 *  None
 */
static void GET_KS8851SNL_RWBYTE_CMD(BYTE addr,
				BYTE act,
				PBYTE pc0,
				PBYTE pc1,
				BOOL *pfReturn)
{

	BYTE bAlign, flag, Ac0, Ac1;

	if (act == ACTION_READ)
		flag = 0x00;
	else
		flag = 0x10;
	bAlign = addr & 0x03;
	Ac0 = (flag | (1 << bAlign)) << 2;
	Ac1 = (BYTE)addr >> 6;
	Ac0 = (BYTE)Ac0 | (BYTE)Ac1;
	Ac1 = (BYTE)addr << 2;
	*pfReturn = TRUE;
	*pc0 = Ac0;
	*pc1 = Ac1;
}

/*
 * GET_KS8851SNL_RWWORD_CMD
 *
 * Description
 *  assemble a KS8851 SNL command for reading/write a WORD register value
 * Parameters
 *  BYTE addr (IN), Address to read/write
 *  BYTE act (IN), Action definition (ACTION_READ/ACTION_WRITE)
 *  PBYTE pco (OUT), Pointer to first byte command.
 *  PBYTE pc1 (OUT), Pointer to second byte command.
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 *			sucessful (TRUE)/fail (FALSE)
 * Return (None):
 */

static void GET_KS8851SNL_RWWORD_CMD(BYTE addr,
				BYTE act,
				PBYTE pc0,
				PBYTE pc1,
				BOOL *pfReturn)
{

	BYTE bAlign, flag, Ac0 = 0, Ac1 = 0;
	BOOL f = TRUE;

	if (act == ACTION_READ)
		flag = 0x00;
	else
		flag = 0x10;
	bAlign = (BYTE)addr & 0x03;
	if (bAlign == 0)
		Ac0 = (flag | 0x3) << 2;
	else if (bAlign == 1)
		Ac0 = (flag | 0x6) << 2;
	else if (bAlign == 2)
		Ac0 = (flag | 0xC) << 2;
	else
		f = FALSE;
	Ac1 = (BYTE)addr >> 6;
	Ac0 = (BYTE)Ac0 | (BYTE)Ac1;
	Ac1 = (BYTE)addr << 2;
	*pfReturn = f;
	*pc0 = Ac0;
	*pc1 = Ac1;
}

/*
 * GET_KS8851SNL_RWDWORD_CMD
 *
 * Description
 *  assemble a KS8851 SNL command for reading/write a DWORD register value
 * Parameters
 *  BYTE addr (IN), Address to read/write
 *  BYTE act (IN), Action definition (ACTION_READ/ACTION_WRITE)
 *  PBYTE pco (OUT), Pointer to first byte command.
 *  PBYTE pc1 (OUT), Pointer to second byte command.
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 *			sucessful (TRUE)/fail (FALSE)
 * Return
 *  None
 */

static void GET_KS8851SNL_RWDWORD_CMD(BYTE addr,
				BYTE act,
				PBYTE pc0,
				PBYTE pc1,
				BOOL *pfReturn)
{

	BYTE bAlign, flag, Ac0, Ac1;
	BOOL f = TRUE;

	if (act == ACTION_READ)
		flag = 0x00;
	else
		flag = 0x10;
	bAlign = addr & 0x03;
	if (bAlign == 0)
		Ac0 = (flag | 0xF) << 2;
	else
		f = FALSE;
	Ac1 = (BYTE)addr >> 6;
	Ac0 = (BYTE)Ac0 | (BYTE)Ac1;
	Ac1 = (BYTE)addr<<2;
	*pfReturn = f;
	*pc0 = Ac0;
	*pc1 = Ac1;
}

/*
 * KS8851SNL_REG_READ
 *
 * Description
 *  Read a register value from KS8851 SNL chip
 * Parameters
 *  BYTE addr (IN), The register address.
 *  WORD * pwValue (OUT), Pointer a WORD to receive the register value.
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 *			sucessful (TRUE)/fail (FALSE)
 * Return
 *  None
 */

static void KS8851SNL_REG_READ(BYTE addr,
				WORD *pwValue,
				BOOL *pfReturn)
{
	BYTE c0 = 0, c1 = 0;
	unsigned short read_cmd;
	BOOL fReturn = FALSE;

	GET_KS8851SNL_RWWORD_CMD(addr, ACTION_READ, &c0, &c1, &fReturn);
	read_cmd = (c0 << 8) | c1;

	/* SPI master driver to get value */
	*pwValue = spi_ks8851snl_reg_read(read_cmd);
}

/*
 * KS8851SNL_REG_WRITE
 *
 * Description
 *  Write a register value to KS8851 SNL chip
 * Parameters
 *  BYTE addr (IN), The register address.
 *  WORD wValue (IN), Pointer a WORD which hold the register value to write.
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 *			sucessful (TRUE)/fail (FALSE)
 * Return
 *  None
 */

void  KS8851SNL_REG_WRITE(BYTE addr,
			WORD wValue,
			BOOL *pfReturn)
{
	BOOL fReturn;
	BYTE c0 = 0, c1 = 0;

	GET_KS8851SNL_RWWORD_CMD(addr, ACTION_WRITE, &c0, &c1, &fReturn);
	if (fReturn) {
		unsigned short write_cmd;
		write_cmd = (c0 << 8) | c1;
		spi_ks8851snl_reg_write(write_cmd, wValue);
	}
}

/*
 * KS8851SNL_DATA_WRITE
 *
 * Write data to KS8851SNL slave device, before write should check the KS8851
 * TQ memory is avariable or not?
 *
 * Description
 *  Write a Ethernet data to KS8851 SNL chip
 * Parameters
 *  LPBYTE pBuf (IN), Pointer to data buffer.
 *  WORD wOrgDataLen, The data length;
 *  WORD wNewDataLen, The write length which around to 32 bit data alignment
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 *			sucessful (TRUE)/fail (FALSE)
 * Return (None):
 */

void  KS8851SNL_DATA_WRITE(PBYTE pBuf,
			WORD wOrgDataLen,
			WORD wNewDataLen,
			BOOL *pfReturn)
{

	BYTE cmd[5];
	cmd[0] = 0xC0;
	cmd[1] = 00;
	cmd[2] = 0x80;
	cmd[3] = (BYTE)(wOrgDataLen & 0x00FF);
	cmd[4] = (BYTE)(wOrgDataLen >> 8);
	*pfReturn = FALSE;
	if (spi_ks8851snl_data_write(&cmd[0], 5, TRUE, FALSE)) {
		if (spi_ks8851snl_data_write(pBuf, wNewDataLen, FALSE, TRUE))
			*pfReturn = TRUE;
	}
}

/*
 * KS8851SNL_DATA_READ
 *
 * Description
 *  Read the data from KS8851 SNL chip through KSZ8692 SPI Master driver
 *  because the KS8692 SPI master driver maximum read is 16 bytes each time,
 *  so the nDataLen in this functuion should be less 16 byte.
 * Parameters
 *  LPBYTE pBuf (IN), Pointer to data buffer to receive the data.
 *  int nDataLen, How mant data need to receive.
 *  PDWORD pdwBytesTransferred, Pointer to a DWORD to indicate how many
 *				bytes transferred.
 *  BOOL *pfReturn (OUT), Pointer to a BOOL variable to indicate function
 *			sucessful (TRUE)/fail (FALSE)
 * Return
 * None
 */

static void  KS8851SNL_DATA_READ(PBYTE pBuf,
				int nDataLen,
				PDWORD pdwBytesTransferred,
				BOOL *pfReturn)
{
	*pdwBytesTransferred = spi_ks8851snl_data_read(0x80, 1, pBuf, nDataLen);
	if (*pdwBytesTransferred != nDataLen)
		*pfReturn = FALSE;
	else
		*pfReturn = TRUE;
}

/*
 * KS8851SNL_READ_BUFFER
 *
 * Description
 *  Read a Ethernet frame from KS8851 SNL chip.
 * Parameters
 *  void * pHardware (IN), Pointer the HARDWARE instance structure
 *  LPBYTE pBuf (IN), Pointer to data buffer to receive the data.
 *  int nDataLen, How mant data need to receive.
 *  SHORT * puReadLen, Pointer to a SHORT to indicate how many bytes
 *			actully read.
 * Return
 *  None
 */

void KS8851SNL_READ_BUFFER(void *pHardware,
			LPBYTE pIn,
			LPBYTE *ppOut,
			USHORT uDataLen,
			USHORT *puReadLen)
{
	USHORT length;
	USHORT uLeftLen;
	BOOL  fReturn;
	DWORD dwBytesTransferred = 0;
	WORD w, spiBurstFlag, spiBurstLen;
	DWORD *pdw;
	LPBYTE pMove = pIn;
	*ppOut = pIn;
	*puReadLen = 0;

	HW_READ_WORD(pHardware, REG_RX_CTRL2, &w);
	w &= 0xFF0F;

	/* now set the read burst rate according to user defined */
	spiBurstFlag = SPI_RAED_BURST_ALL;

	/* get real read length */
	GET_DATA_ALIGNMENT(uDataLen, &length);

	switch (spiBurstFlag) {

	case SPI_RAED_BURST_4BYTES:
		w |= 0x0000;
		spiBurstLen = 4;
		break;

	case SPI_RAED_BURST_8BYTES:
		w |= 0x0020;
		spiBurstLen = 8;
		break;

	case SPI_RAED_BURST_16BYTES:
		w |= 0x0040;
		spiBurstLen = 16;
		break;

	case SPI_RAED_BURST_32BYTES:
		w |= 0x0060;
		spiBurstLen = 32;
		break;

	case SPI_RAED_BURST_ALL:
		w |= 0x0080;
		spiBurstLen = length;
		break;

	default:
		*puReadLen = 0;
		printf("ks8851snl error: wrong spi read burst value\n");
		return;
	}

	HW_WRITE_WORD(pHardware, REG_RX_CTRL2, w);
	HW_READ_START(pHardware);

	/* each DMA transfer, there 4byte dummy + 4 byte header */
	if (spiBurstFlag == SPI_RAED_BURST_4BYTES) {

		KS8851SNL_DATA_READ(pMove, spiBurstLen, &dwBytesTransferred,
					&fReturn);
		KS8851SNL_DATA_READ(pMove, spiBurstLen, &dwBytesTransferred,
					&fReturn);
		pdw = (LPDWORD)pMove;
		*puReadLen = (USHORT)((*pdw) >> 16);

	} else if (spiBurstFlag == SPI_RAED_BURST_8BYTES) {

		KS8851SNL_DATA_READ(pMove, spiBurstLen, &dwBytesTransferred,
					&fReturn);
		pdw = (LPDWORD)pMove;
		pdw++;
		*puReadLen = (USHORT)((*pdw) >> 16);

	} else if (spiBurstFlag == SPI_RAED_BURST_16BYTES) {

		KS8851SNL_DATA_READ(pMove, spiBurstLen, &dwBytesTransferred,
					&fReturn);
		pdw = (LPDWORD)pMove;
		pdw++;
		*puReadLen = (USHORT)((*pdw) >> 16);
		*ppOut = pIn + 8;
		pMove += 16;
		length -= 8;

	} else if (spiBurstFlag == SPI_RAED_BURST_32BYTES) {
		KS8851SNL_DATA_READ(pMove, spiBurstLen, &dwBytesTransferred,
					&fReturn);
		pdw = (LPDWORD)pMove;
		pdw++;
		*puReadLen = (USHORT)((*pdw) >> 16);
		*ppOut = pIn + 8;
		pMove += 32;
		length -= 24;
	} else {

		/* spiBurstFlag==SPI_RAED_BURST_ALL
		 * read length should add another 8 byte to include 4 byte
		 * dummy data and 4 byte header
		 */
		KS8851SNL_DATA_READ(pMove, spiBurstLen+8, &dwBytesTransferred,
					&fReturn);
		pdw = (LPDWORD)pMove;
		pdw++;
		*puReadLen = (USHORT)((*pdw) >> 16);
		*ppOut = pIn + 8;
	}

	if (spiBurstFlag != SPI_RAED_BURST_ALL) {

		/*
		 * for a Ethernet packet, minimum length should bigger than
		 * 16 byte so the length=length/SPI_READ_BURST should not be
		 * zero
		 */
		uLeftLen = length%spiBurstLen;
		length = length/spiBurstLen;
		while (length--) {
			KS8851SNL_DATA_READ(pMove, spiBurstLen,
						&dwBytesTransferred, &fReturn);
			pMove += spiBurstLen;
		}

		if (uLeftLen) {
			/*
			 * last one, still should use same burst rate,
			 * otherwise error will happen
			 */
			KS8851SNL_DATA_READ(pMove, spiBurstLen,
					&dwBytesTransferred, &fReturn);
		}
	}

	HW_READ_END(pHardware);
}

/*
 * this function should not be called in a KS8851_SNL compiler flag
 * enabled. here only is a dummy function.
 */

void HW_WRITE_16BITDATA(void *phwi, unsigned short data)
{
	BOOL fReturn;
	KS8851SNL_DATA_WRITE((PBYTE)&data, 2, 2, &fReturn);
}

void HW_WRITE_32BITDATA(void *phwi, unsigned long data)
{
	BOOL fReturn = FALSE;
	KS8851SNL_DATA_WRITE((PBYTE)&data, 4, 4, &fReturn);
}


void HW_READ_16BITDATA(void *phwi, unsigned short *data)
{
	BOOL fReturn;
	DWORD dwBytesTransferred = 0;
	KS8851SNL_DATA_READ((LPBYTE)data, 2, &dwBytesTransferred, &fReturn);
}

void HW_READ_32BITDATA(void *phwi, unsigned long *data)
{
	BOOL fReturn;
	DWORD dwBytesTransferred = 0;
	KS8851SNL_DATA_READ((LPBYTE)data, 4, &dwBytesTransferred, &fReturn);
}

void HW_READ_WORD(void *pHardware, unsigned short addr, unsigned short *pw)
{
	BOOL fReturn = FALSE;
	KS8851SNL_REG_READ((BYTE)addr, pw, &fReturn);
}


void HW_WRITE_WORD(void *pHardware, unsigned short addr, unsigned short w)
{
	BOOL fReturn = FALSE;
	KS8851SNL_REG_WRITE((BYTE)addr, w, &fReturn);
}

