#ifndef _USB_H_INCLUDED
#define _USB_H_INCLUDED

void InitUSB(void);
void HandleUSBEvents(void);
void SendControlResponse(int Size);
//void SendDataEP0(WORD Size, BYTE Offset);
void SendDataEndpoint(WORD Size, BYTE Offset, WORD EndpointNumber);
void SetDMA(BYTE p5, BYTE p3, BYTE px);

/*extern BYTE	bmRequestType;
extern BYTE	bRequest;
extern WORD	wValue;
extern WORD	wIndex;
extern WORD	wLength;*/

extern BYTE	bmRequestType;
extern BYTE	bRequest;
extern BYTE	wValue;
extern BYTE	wIndex;
extern WORD	wLength;

extern BYTE	UsbSpeed;
extern __xdata __at UsbBufferVA volatile BYTE UsbBuffer[1024];
extern __xdata volatile BYTE UsbReceivedDataReady;
extern __xdata volatile BYTE UsbHaveCswReady;

#endif
