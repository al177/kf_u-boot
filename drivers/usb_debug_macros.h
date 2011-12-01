#ifndef _USB_DEBUG_MACROS
#define _USB_DEBUG_MACROS

#define PRINT_PWR(b)                                    \
  printf ("\tpower   0x%2.2x\n", b);                    \
  if (b & MUSB_POWER_ISOUPDATE)                         \
    printf ("\t\tISOUPDATE\n");                         \
  if (b & MUSB_POWER_SOFTCONN)                          \
    printf ("\t\tSOFTCONN\n");                          \
  if (b & MUSB_POWER_HSENAB)                            \
    printf ("\t\tHSENAB\n");                            \
  if (b & MUSB_POWER_HSMODE)                            \
    printf ("\t\tHSMODE\n");                            \
  if (b & MUSB_POWER_RESET)                             \
    printf ("\t\tRESET\n");                             \
  if (b & MUSB_POWER_RESUME)                            \
    printf ("\t\tRESUME\n");                            \
  if (b & MUSB_POWER_SUSPENDM)                          \
    printf ("\t\tSUSPENDM\n");                          \
  if (b & MUSB_POWER_ENSUSPEND)                         \
    printf ("\t\tENSUSPEND\n");

#define PRINT_CSR0(s)                                   \
  printf ("\tcsr0    0x%4.4x\n", s);                    \
  if (s & MUSB_CSR0_FLUSHFIFO)                          \
    printf ("\t\tFLUSHFIFO\n");                         \
  if (s & MUSB_CSR0_P_SVDSETUPEND)                      \
    printf ("\t\tSERV_SETUPEND\n");                     \
  if (s & MUSB_CSR0_P_SVDRXPKTRDY)                      \
    printf ("\t\tSERV_RXPKTRDY\n");                     \
  if (s & MUSB_CSR0_P_SENDSTALL)                        \
    printf ("\t\tSENDSTALL\n");                         \
  if (s & MUSB_CSR0_P_SETUPEND)	                        \
    printf ("\t\tSETUPEND\n");                          \
  if (s & MUSB_CSR0_P_DATAEND)                          \
    printf ("\t\tDATAEND\n");                           \
  if (s & MUSB_CSR0_P_SENTSTALL)                        \
    printf ("\t\tSENTSTALL\n");                         \
  if (s & MUSB_CSR0_TXPKTRDY)                           \
    printf ("\t\tTXPKTRDY\n");                          \
  if (s & MUSB_CSR0_RXPKTRDY)                           \
    printf ("\t\tRXPKTRDY\n");

#define PRINT_INTRUSB(b)                                \
  printf ("\tintrusb 0x%2.2x\n", b);                    \
  if (b & OMAP34XX_USB_INTRUSB_VBUSERR) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_VBUSERR\n");	\
  if (b & OMAP34XX_USB_INTRUSB_SESSREQ) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_SESSREQ\n");	\
  if (b & OMAP34XX_USB_INTRUSB_DISCON) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_DISCON\n");	\
  if (b & OMAP34XX_USB_INTRUSB_CONN) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_CONN\n");	\
  if (b & OMAP34XX_USB_INTRUSB_SOF) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_SOF\n");	\
  if (b & OMAP34XX_USB_INTRUSB_RESET_BABBLE) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_RESET_BABBLE\n");	\
  if (b & OMAP34XX_USB_INTRUSB_RESUME) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_RESUME\n");	\
  if (b & OMAP34XX_USB_INTRUSB_SUSPEND) \
    printf ("\t\tOMAP34XX_USB_INTRUSB_SUSPEND\n");

#define PRINT_INTRTX(s)                                \
  printf ("\tintrtx 0x%4.4x\n", s);                    \

#define PRINT_INTRRX(s)                                \
  printf ("\tintrx 0x%4.4x\n", s);                    \

#define PRINT_DEVCTL(b)                                 \
  printf ("\tdevctl  0x%2.2x\n", b);                    \
  if (b & MUSB_DEVCTL_BDEVICE)                          \
    printf ("\t\tB device\n");                          \
  else                                                  \
    printf ("\t\tA device\n");                          \
  if (b & MUSB_DEVCTL_FSDEV)                            \
    printf ("\t\tFast Device - (host mode)\n");         \
  if (b & MUSB_DEVCTL_LSDEV)                            \
    printf ("\t\tSlow Device - (host mode)\n");         \
  if (b & MUSB_DEVCTL_HM)                               \
    printf ("\t\tHost mode\n");                         \
  else                                                  \
    printf ("\t\tPeripherial mode\n");                  \
  if (b & MUSB_DEVCTL_HR)                               \
    printf ("\t\tHost request started (B device)\n");   \
  else                                                  \
    printf ("\t\tHost request finished (B device)\n");  \
  if (b & MUSB_DEVCTL_BDEVICE)                          \
    {                                                   \
      if (b & MUSB_DEVCTL_SESSION)                      \
	printf ("\t\tStart of session (B device)\n");   \
      else                                              \
	printf ("\t\tEnd of session (B device)\n");     \
    }                                                   \
  else                                                  \
    {                                                   \
      if (b & MUSB_DEVCTL_SESSION)                      \
	printf ("\t\tStart of session (A device)\n");   \
      else                                              \
	printf ("\t\tEnd of session (A device)\n");     \
    }

#define PRINT_CONFIG(b)                                 \
  printf ("\tconfig 0x%2.2x\n", b);                     \
  if (b & MUSB_CONFIGDATA_MPRXE)                        \
    printf ("\t\tAuto combine rx bulk packets\n");	\
  if (b & MUSB_CONFIGDATA_MPTXE)                        \
    printf ("\t\tAuto split tx bulk packets\n");	\
  if (b & MUSB_CONFIGDATA_BIGENDIAN)                    \
    printf ("\t\tBig Endian ordering\n");	        \
  else                                                  \
    printf ("\t\tLittle Endian ordering\n");	        \
  if (b & MUSB_CONFIGDATA_HBRXE)                        \
    printf ("\t\tHigh speed rx iso endpoint\n");	\
  if (b & MUSB_CONFIGDATA_HBTXE)                        \
    printf ("\t\tHigh speed tx iso endpoint\n");	\
  if (b & MUSB_CONFIGDATA_DYNFIFO)                      \
    printf ("\t\tDynamic fifo sizing\n");	        \
  if (b & MUSB_CONFIGDATA_SOFTCONE)                     \
    printf ("\t\tSoft Connect\n");                      \
  if (b & MUSB_CONFIGDATA_UTMIDW)                       \
    printf ("\t\t16 bit data width\n");		        \
  else                                                  \
    printf ("\t\t8 bit data width\n");

#define PRINT_REQ(r)                                                             \
  printf ("usb request \n");				                         \
  printf ("\tbmRequestType 0x%2.2x\n", r.bmRequestType);	                 \
  if (0 == (r.bmRequestType & USB_REQ_DIRECTION_MASK))                           \
    printf ("\t\tDirection : To device\n");				         \
  else                                                                           \
    printf ("\t\tDirection : To host\n");				         \
  if (USB_TYPE_STANDARD == (r.bmRequestType & USB_TYPE_STANDARD))                \
    printf ("\t\tType      : Standard\n");				         \
  if (USB_TYPE_CLASS == (r.bmRequestType & USB_TYPE_CLASS))                      \
    printf ("\t\tType      : Standard\n");				         \
  if (USB_TYPE_VENDOR == (r.bmRequestType & USB_TYPE_VENDOR))                    \
    printf ("\t\tType      : Standard\n");				         \
  if (USB_TYPE_RESERVED == (r.bmRequestType & USB_TYPE_RESERVED))                \
    printf ("\t\tType      : Standard\n");				         \
  if (USB_REQ_RECIPIENT_DEVICE == (r.bmRequestType & USB_REQ_RECIPIENT_MASK))    \
    printf ("\t\tRecipient : Device\n");				         \
  if (USB_REQ_RECIPIENT_INTERFACE == (r.bmRequestType & USB_REQ_RECIPIENT_MASK)) \
    printf ("\t\tRecipient : Interface\n");				         \
  if (USB_REQ_RECIPIENT_ENDPOINT == (r.bmRequestType & USB_REQ_RECIPIENT_MASK))  \
    printf ("\t\tRecipient : Endpoint\n");				         \
  if (USB_REQ_RECIPIENT_OTHER == (r.bmRequestType & USB_REQ_RECIPIENT_MASK))     \
    printf ("\t\tRecipient : Other\n");				                 \
  printf ("\tbRequest      0x%2.2x\n", r.bRequest);		                 \
  if (r.bRequest == USB_REQ_GET_STATUS)                                          \
    printf ("\t\tGET_STATUS\n");	                                         \
  else if (r.bRequest == USB_REQ_SET_ADDRESS)                                    \
    printf ("\t\tSET_ADDRESS\n");	                                         \
  else if (r.bRequest == USB_REQ_SET_FEATURE)                                    \
    printf ("\t\tSET_FEATURE\n");	                                         \
  else if (r.bRequest == USB_REQ_GET_DESCRIPTOR)                                 \
    printf ("\t\tGET_DESCRIPTOR\n");	                                         \
  else if (r.bRequest == USB_REQ_GET_CONFIGURATION)				 \
    printf ("\t\tGET_CONFIGURATION\n");						 \
  else if (r.bRequest == USB_REQ_SET_CONFIGURATION)                              \
    printf ("\t\tSET_CONFIGURATION\n");	                                         \
  else if (r.bRequest == USB_REQ_SET_INTERFACE)                                  \
    printf ("\t\tUSB_REQ_SET_INTERFACE\n");	                                 \
  else                                                                           \
    printf ("\tUNKNOWN\n");	                                                 \
  printf ("\twValue        0x%4.4x\n", r.wValue);		                 \
  printf ("\twIndex        0x%4.4x\n", r.wIndex);		                 \
  printf ("\twLength       0x%4.4x\n", r.wLength);


#define PRINT_RXMAXP(s)                                                          \
  printf ("\trxmaxp  0x%4.4x\n", s)

#define PRINT_RXCSR(s)                                                           \
  printf ("\trxcsr   0x%4.4x\n", s);			                         \
  if (s & MUSB_RXCSR_AUTOCLEAR)                                                  \
    printf ("\t\tautclear\n");	                                                 \
  if (s & MUSB_RXCSR_DMAENAB)                                                    \
    printf ("\t\tdma enable\n");		                                 \
  if (s & MUSB_RXCSR_DISNYET)                                                    \
    printf ("\t\tdisable nyet\n");		                                 \
  if (s & MUSB_RXCSR_PID_ERR)                                                    \
    printf ("\t\tpid error\n");                                                  \
  if (s & MUSB_RXCSR_DMAMODE)                                                    \
    printf ("\t\tdma mode should be 0 \n");		                         \
  if (s & MUSB_RXCSR_CLRDATATOG)                                                 \
    printf ("\t\tclear data\n");                                                 \
  if (s & MUSB_RXCSR_FLUSHFIFO)                                                  \
    printf ("\t\tflush fifo\n");		                                 \
  if (s & MUSB_RXCSR_DATAERROR)                                                  \
    printf ("\t\tdata error\n");		                                 \
  if (s & MUSB_RXCSR_FIFOFULL)                                                   \
    printf ("\t\tfifo full\n");			\
  if (s & MUSB_RXCSR_RXPKTRDY) \
    printf ("\t\trx packet ready\n");		\
  if (s & MUSB_RXCSR_P_ISO) \
    printf ("\t\tiso mode\n");			\
  else \
    printf ("\t\tbulk mode\n");			\
  if (s & MUSB_RXCSR_P_SENTSTALL) \
    printf ("\t\tsent stall\n");  \
  if (s & MUSB_RXCSR_P_SENDSTALL) \
    printf ("\t\tsend stall\n");  \
  if (s & MUSB_RXCSR_P_OVERRUN) \
    printf ("\t\toverrun\n")

#define PRINT_TXMAXP(s)                                    \
  printf ("\ttxmaxp  0x%4.4x\n", s)


#define PRINT_TXCSR(s)                                                                        \
  printf ("\ttxcsr   0x%4.4x\n", s);			                                      \
  if (s & MUSB_TXCSR_TXPKTRDY) \
    printf ("\t\ttxpktrdy\n"); \
  if (s & MUSB_TXCSR_FIFONOTEMPTY) \
    printf ("\t\tfifo not empty\n");	\
  if (s & MUSB_TXCSR_FLUSHFIFO) \
    printf ("\t\tflush fifo\n");	\
  if (s & MUSB_TXCSR_CLRDATATOG) \
    printf ("\t\tclear data toggle\n");	\
  if (s & MUSB_TXCSR_MODE) \
    printf ("\t\tTX mode\n");	\
  else \
    printf ("\t\tRX mode\n");  \
  if (s & MUSB_TXCSR_P_UNDERRUN) \
    printf ("\t\tunderrun\n");	\
  if (s & MUSB_TXCSR_P_SENTSTALL) \
    printf ("\t\tsent stall\n");	\
  if (s & MUSB_TXCSR_P_SENDSTALL) \
    printf ("\t\tsend stall\n");	\


#endif
