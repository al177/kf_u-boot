#ifndef __KS885X_TARGET_SPI_H__
#define __KS885X_TARGET_SPI_H__


#define BYTE_ACCESS		0x10
#define WORD_ACCESS		0x11
#define DWORD_ACCESS		0x12

#define ACTION_READ		0x13
#define ACTION_WRITE		0x14

void HW_READ_START(void *pHardware);
void HW_READ_END(void *pHardware);

void HW_WRITE_START(void *pHardware);
void HW_WRITE_END(void *pHardware);

void HW_READ_WORD(void *pHardware, unsigned short addr, unsigned short *pw);
void HW_WRITE_WORD(void *pHardware, unsigned short addr, unsigned short w);

void HW_READ_BUFFER(void *pHardware, unsigned char *pbuf, unsigned short len,
			unsigned short *pRealLen);

void KS8851SNL_READ_BUFFER(void *pHardware, LPBYTE pIn, LPBYTE *ppOut,
			USHORT uDataLen, USHORT *puReadLen);
void  KS8851SNL_DATA_WRITE(PBYTE pBuf, WORD wOrgDataLen, WORD wNewDataLen,
			BOOL *pfReturn);

extern void spi_init(void);
extern void spi_ks8851snl_reg_write(
			unsigned short write_cmd,
			unsigned short value);
extern unsigned short spi_ks8851snl_reg_read(
			unsigned short read_cmd);
extern unsigned long spi_ks8851snl_data_write(
			unsigned char *pdata,
			unsigned long len,
			int fBegin,
			int fEnd);
extern unsigned long spi_ks8851snl_data_read(
			unsigned long  readCmd,
			unsigned long cmdLen,
			unsigned char *pBuf,
			unsigned long readLen);

#endif

/* __KS885X_TARGET_SPI_H___ */
