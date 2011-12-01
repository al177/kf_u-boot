#ifndef __KS885X_TARGET__H__
#define __KS885X_TARGET__H__

#include "ks885x_snl.h"

#define DATA_ALIGNMENT	4  /* WORD boundary */

#define MIO_DWORD(x)  (*((volatile unsigned int*)(x)))
#define MIO_WORD(x)   (*((volatile unsigned short*)(x)))
#define MIO_BYTE(x)   (*((volatile unsigned char*)(x)))

#define HW_READ_START(phwi) \
{ \
	HW_WRITE_WORD(phwi, REG_RX_ADDR_PTR, ADDR_PTR_AUTO_INC); \
	HW_WRITE_WORD(phwi, REG_RXQ_CMD, (RXQ_CMD_CNTL | RXQ_START)); \
}

#define HW_READ_END(phwi) \
{ \
	HW_WRITE_WORD(phwi, REG_RXQ_CMD, RXQ_CMD_CNTL); \
}

#define HW_WRITE_START(phwi) \
{ \
	WORD w; \
	HW_READ_WORD(phwi, REG_RXQ_CMD, &w); \
	w |= RXQ_CMD_CNTL | RXQ_START; \
	HW_WRITE_WORD(phwi, REG_RXQ_CMD, w); \
}

#define HW_WRITE_END(phwi) \
{ \
    WORD w; \
    HW_READ_WORD(phwi, REG_RXQ_CMD, &w); \
    w &= ~RXQ_START; \
    w |= RXQ_CMD_CNTL; \
    HW_WRITE_WORD(phwi, REG_RXQ_CMD, w); \
}

/* Chip data aligment is 4 byte */

#define  GET_DATA_ALIGNMENT(in, pout) \
{ \
	unsigned short wMask = (4-1); \
	if ((in & wMask) != 0) \
		*pout = (in + wMask) & ~wMask; \
	else \
		*pout = in;\
}

void HW_WRITE_DATA_HEADER(void *phwi, USHORT *puiPacketLength);

/*
 * before call this macro, we has start the DMA and wrote the DATA header
 * data length should have been adjusted to 16 or 32 bit alignment.
 */

void HW_WRITE_DATA_BUFFER(void *phwi, LPBYTE pbuf, LPBYTE * ppOut, \
			SHORT buflen, \
			USHORT *pPacketLen);

/*
 * Read single frame from KS8851MQL QMU RxQ packet memory
 * at one DMA interval. Frame size (len) must be DWORD alignment
 * (1). reset RX frame pointer.
 * (2). start transfer.
 * (3). dummy read.
 * (4). read 4-byte frame header.
 * (5). read frame data.
 * (6). stop transfer.
 */

void HW_READ_BUFFER(void *phwi, unsigned char *data, unsigned short len, \
			unsigned short *pRealLen);

#endif

/*__KS885X_TAREGT__H__ */
