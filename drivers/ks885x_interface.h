#ifndef __KS8851_UBOOTETH__H__
#define __KS8851_UBOOTETH__H__

#undef TRUE
#undef FALSE
#define TRUE			1
#define FALSE			0
#define MAX_RECV_FRAMES         32
#define MAX_BUF_SIZE            2048
#define TX_BUF_SIZE             2000
#define RX_BUF_SIZE             2000

typedef void *HANDLE;
typedef unsigned char BYTE, *LPBYTE, *PBYTE;
typedef unsigned long DWORD, *LPDWORD, *PDWORD;
typedef unsigned short WORD, *LPWORD, *PWORD;
typedef unsigned short USHORT, *PUSHORT;
typedef unsigned long  ULONG, *PULONG;
typedef int BOOL, *LPBOOL;
typedef short SHORT;
typedef unsigned char UCHAR;
typedef void *LPVOID;

extern void spi_init(void);
extern void NetReceive(volatile uchar *, int);

typedef enum {
	SPI_RAED_BURST_4BYTES = 0,
	SPI_RAED_BURST_8BYTES,
	SPI_RAED_BURST_16BYTES,
	SPI_RAED_BURST_32BYTES,
	SPI_RAED_BURST_ALL,
} SPI_RAED_BURST;

/* Tramsmit multiplex frame length info */
typedef struct {
	BYTE  *txData;		/* tx data pointer */
	USHORT  txLength;	/* Byte count */
} TX_FRAME_INFO;

/* Receive multiplex framer header info */
typedef struct {
	USHORT  rxStatus;	/* Frame status */
	USHORT  rxLength;	/* Byte count */
} FR_HEADER_INFO;

typedef struct {
	/* a variable holder for multi buffer write */
	ULONG m_dwPrevData;
	USHORT m_uPrevByte;
	FR_HEADER_INFO *m_pRevHdrInfo;
	ULONG m_uCurFrameIndex;
	ULONG m_uFramesRemained;
	ULONG m_uCurRecvFrams;
	BYTE m_recvBuffer[TX_BUF_SIZE];
} HARDWARE, *PHARDWARE;

typedef enum {
	PRINTF_BYTE = 0,
	PRINTF_WORD,
	PRINTF_DWORD,
} PRINTFFLAG;

#endif
/* __KS8851_UBOOTETH__H__ */
