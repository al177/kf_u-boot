#include "ks885x_target.h"
#include "ks8851_registers.h"
#include "ks885x_snl.c"

void HW_WRITE_DATA_HEADER(void *phwi, USHORT *puiPacketLength)
{
	USHORT length;
	DWORD dw = 0;

	/*
	 * When do the writing, the data length writed to chip's buffer
	 * should be 32 bit data alignment
	 */
	GET_DATA_ALIGNMENT(*puiPacketLength, &length);

	if (DATA_ALIGNMENT == 2) {
		HW_WRITE_16BITDATA(phwi, TX_CTRL_INTERRUPT_ON);
		/* data length in the head should be original data length */
		HW_WRITE_16BITDATA(phwi, *puiPacketLength);
	} else if (DATA_ALIGNMENT == 4) {
		dw = ((*puiPacketLength) << 16) | TX_CTRL_INTERRUPT_ON;
		HW_WRITE_32BITDATA(phwi, dw);
	}

	/* adjusted packet length return to the caller */
	*puiPacketLength = length;

	/*
`	 * at begining of each sending packet,
	 * we must clean the m_dwPrevData and m_uPrevByte
	 */
	((PHARDWARE)phwi)->m_dwPrevData = 0;
	((PHARDWARE)phwi)->m_uPrevByte = 0;
}

/*
 * Function name HW_WRITE_DATA_BUFFER
 *
 * Parameters
 *  phwi (IN), a void pointer to HARDWARE structure
 *  pbuf (IN), a UCHAR * pointer to input data buffer
 *  ppOut (OUT), a UCHAR * * pointer to UCHAR * variable to hold current data
 *		sending pointer.
 *  buflen (IN), data length in the data buffer
 *  pPacketLen, a USHORT pointer to a USHORT address which hold a current
 *		sending packets length
 *
 * Purpose
 *  When a protocol layer send a Ethernet pack to low level driver,
 *  the whole Ethernet packet may be broken into several small data buffers.
 *  Each data buffer may hold diffetent legnth of data. For the KS8851
 *  hardware, each QMU write should be 32 bit data alignment.
 *  Because each data buffer sent by protocal layer, the data length may
 *  not be round to 32 bit alignment.
 *  For example: A 101 bytes long Ethernet packet, may be broken to 3 data
 *  buffer to send, each length is  22, 22 and 57 byte long.
 *  If KS8851 hardware is 32 bit data bus, write 22 byte (which last write
 *  is 2 byte) will cause the data broken. So we need to save last 2 byte
 *  and added next buffers first 2 byte to complete a 32 bit write.
 *  The following function is for solving this problem.
 *
 * Note
 *  The m_uPrevByte is a USHORT variable in HARDWARE structure to hold number
 *  of bytes which does not wrote to hardware at last function call.
 *  The m_dwPrevData is ULONG variable in HARDWARE structure to hold the
 *  data which not wrote to hardware at last function call.
 *  The whole packet length hold in the  *pPacketLen should has been rounded
 *  to 32 bit alignment.
 *
 * DATA_ALIGNMENT in the 16 bit data bus is defined to 2
 * DATA_ALIGNMENT in the 32 bit data bus is defined to 4
 * HW_WRITE_16BITDATA is a extern funtion for write 16 bit data to hardware
 * HW_WRITE_32BITDATA is a extern funtion for write 32 bit data to hardware
 */

void HW_WRITE_DATA_BUFFER(
			void *phwi,
			UCHAR *pbuf,
			UCHAR **ppOut,
			SHORT buflen,
			USHORT *pPacketLen)
{

	/* first we check if there some data left before */
	int len;
	UCHAR *pByte, *p;
	*ppOut = pbuf;
	while (((PHARDWARE)phwi)->m_uPrevByte) {
		pByte = (UCHAR *)&((PHARDWARE)phwi)->m_dwPrevData;
		pByte += ((PHARDWARE)phwi)->m_uPrevByte;

		if ((buflen-(DATA_ALIGNMENT - ((
				PHARDWARE)phwi)->m_uPrevByte)) > 0) {
			memmove(pByte, pbuf,
			(DATA_ALIGNMENT-((PHARDWARE)phwi)->m_uPrevByte));
		} else {
			/* it should only happen at last sent portion */
			break;
		}

		if (DATA_ALIGNMENT == 2) {
			USHORT w = (USHORT)((PHARDWARE)phwi)->m_dwPrevData;
			HW_WRITE_16BITDATA(phwi, w);
		} else if (DATA_ALIGNMENT == 4) {
			HW_WRITE_32BITDATA(phwi,
				((PHARDWARE)phwi)->m_dwPrevData);
		}

		/* RETAILMSG(TRUE, (TEXT("there is a previous data\n"))); */
		pbuf += (DATA_ALIGNMENT - ((PHARDWARE)phwi)->m_uPrevByte);
		*ppOut = (*ppOut) + (DATA_ALIGNMENT -
			((PHARDWARE)phwi)->m_uPrevByte);
		buflen -= (DATA_ALIGNMENT - ((PHARDWARE)phwi)->m_uPrevByte);
		*pPacketLen -= DATA_ALIGNMENT;
		((PHARDWARE)phwi)->m_uPrevByte = 0;
		break;
	}

	len = buflen/DATA_ALIGNMENT;

	if (len == 0) {

		/*
		 * it should only happen on the last data to send
		 * because we send a data by first change the original
		 * length to a DWORD alignment;
		 * at this case we can not use current buffer, because current
		 * buffer length may not include the DWORD alignment,
		 * for example current buffer length is 10, but DWORD
		 * alignment is 12. Now the buffer point to byte 8, so pass
		 * to this function's buflen=2. We need to send last data with
		 * DWORD alignment is 4 byte. But we can not use current buffer
		 *  pointer, because current buffer pointer + 4 will excess
		 * the valid buffer size, will cause system memory failure.
		 */

		if ((*pPacketLen/DATA_ALIGNMENT) > 0)
			len = (*pPacketLen / DATA_ALIGNMENT);
		else
			len = 1;

		memmove(&((PHARDWARE)phwi)->m_dwPrevData, pbuf, buflen);
		pbuf = (UCHAR *)&((PHARDWARE)phwi)->m_dwPrevData;
		*pPacketLen = 0;

	} else {

		*ppOut = (*ppOut) + (len * DATA_ALIGNMENT);
		((PHARDWARE)phwi)->m_uPrevByte = buflen % DATA_ALIGNMENT;
		*pPacketLen -= (len * DATA_ALIGNMENT);

	}

	if (DATA_ALIGNMENT == 2) {

		USHORT *pw = (USHORT *)pbuf;
		while (len--)
			HW_WRITE_16BITDATA(phwi, *pw++);
		p = (UCHAR *)pw;

	} else if (DATA_ALIGNMENT == 4) {

		ULONG *pdw = (ULONG *)pbuf;
		while (len--)
			HW_WRITE_32BITDATA(phwi, *pdw++);
		p = (UCHAR *)pdw;
	}

	pByte = (UCHAR *)&((PHARDWARE)phwi)->m_dwPrevData;
	if (((PHARDWARE)phwi)->m_uPrevByte)
		memmove(pByte, p, ((PHARDWARE)phwi)->m_uPrevByte);
}

/*
 * HW_READ_BUFFER
 *
 * Description
 *  This function read the data from the hardware
 * Parameters
 *  PHARDWARE pHardware (IN), Pointer to hardware instance
 *  unsigned char * data (IN), Pointer to a receive buffer
 *  unsigned short len (IN), the receving data length got from header before
 *  unsigned short * pRealLen (OUT), Actually received data length
 * Return
 *  None
 */

void HW_READ_BUFFER(
		void *phw,
		unsigned char *data,
		unsigned short len,
		unsigned short *pRealLen)
{

	USHORT length;
	USHORT *pwData;
	ULONG *pdwData;
	HARDWARE *phwi = (HARDWARE *)phw;
	GET_DATA_ALIGNMENT(len, &length);
	HW_READ_START(phwi);

	length = length / DATA_ALIGNMENT;

	if (DATA_ALIGNMENT == 2) {
		pwData = (PUSHORT)data;
		/*
		 * for the 16 bit data bus, there are 2 byte
		 * dummy data at begining
		 */

		/* we need to skip them */
		HW_READ_16BITDATA(phwi, pwData);

		/* 4 byte header */
		HW_READ_16BITDATA(phwi, pwData);
		HW_READ_16BITDATA(phwi, pwData);
		*pRealLen = *pwData;

		/* now is valid data */
		while (length--)
			HW_READ_16BITDATA(phwi, pwData++);

	} else if (DATA_ALIGNMENT == 4) {

		pdwData = (ULONG *)data;
		/*
		 * for the 32 bit data bus, there are 4 byte dummy data
		 * at begining
		 */

		/* we need to skip them */
		HW_READ_32BITDATA(phwi, pdwData);

		/* 4 byte header */
		HW_READ_32BITDATA(phwi, pdwData);
		*pRealLen = (USHORT)((*pdwData) >> 16);

		/* now is valid data */
		while (length--)
			HW_READ_32BITDATA(phwi, pdwData++);
	}
	HW_READ_END(phwi);
}

