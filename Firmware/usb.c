#include "defs.h"
#include "string.h"
#include "timers.h"

__xdata __at UsbBufferVA volatile BYTE UsbBuffer[1024];

BYTE	bmRequestType;
BYTE	bRequest;
BYTE	wValue;
BYTE	wIndex;
WORD	wLength;

//BYTE IsDeviceReady = 0;

static __xdata BYTE	UsbIrq;

static __xdata BYTE	UsbIntStsF080;
static __xdata BYTE UsbIntStsF082;
static __xdata BYTE	UsbIntStsF086;
static __xdata BYTE	UsbIntStsF087;

BYTE UsbSpeed;

__xdata volatile BYTE UsbReceivedDataReady;
__xdata volatile BYTE UsbHaveCSWReady;

extern BYTE	ScsiStatus;
extern DWORD ScsiDataResidue;
extern DWORD ScsiTransferSize;
extern BYTE	ScsiTag[4];
extern BYTE	ScsiDirIn;
extern BYTE	ScsiCdb[16];
extern BYTE	ScsiLun;
extern BYTE	ScsiCdbSize;
extern volatile BYTE SendKeysEnabled;

extern BYTE HandleCDB(void);

extern BYTE HandleStandardRequest(void);
extern BYTE HandleClassRequest(void);
extern BYTE HandleVendorRequest(void);

//Gives access to DMA protocol (DMA (Direct Memory Access) - memory operations in which the processor is not involved)
void SetDMA(BYTE p5, BYTE p3, BYTE px)
{
	XVAL(0xF80B) = 0;
	XVAL(0xF80C) = p5-1;

	switch(px)
	{
		case 0:
		{
			XVAL(0xF80D) = p3;
			XVAL(0xF80E) = p3;
			break;
		}
		case 1:
		{
			XVAL(0xF80D) = p3;
			break;
		}
		case 2:
		{
			XVAL(0xF80E) = p3;
			break;
		}
		default:
		{
			break;
		}
	}
}

void SendControlResponse(int Size)
{
	Endpoint_0.LengthL = LSB(Size);
	Endpoint_0.LengthM = MSB(Size);
	Endpoint_0.LengthH = 0;
	Endpoint_0.CStatus = 0x40;
	while (Endpoint_0.CStatus & 0x40);
	EP0CS = 0x05;
}

/*void SendDataEP0(WORD Size, BYTE Offset)
{
	if (Size > 0)
	{
		SetDMA(0x20, 0, 0);
		SetDMA(0x20, 0x80, 1);
		EP0.ptr_l = UsbBufferPA>>8;
		EP0.ptr_m = UsbBufferPA>>16;
		EP0.ptr_h = UsbBufferPA>>24;
		EP0.offset = Offset;
		EP0.len_l = LSB(Size);
		EP0.len_m = MSB(Size);
		EP0.len_h = 0;
		EP0.cs = 0x88;		

		while(EP0.cs & 0x80);	
	}
}*/

void SendDataEndpoint(WORD Size, BYTE Offset, WORD EndpointNumber)
{
	EndpointRegisters* Endpoint;

	if (EndpointNumber == 0) 
	{
		*Endpoint = Endpoint_0;
	}
	else if (EndpointNumber == 1)
	{
		*Endpoint = Endpoint_1;
	}
	else if (EndpointNumber == 2)
	{
		*Endpoint = Endpoint_2;
	}
	else if (EndpointNumber == 3)
	{
		*Endpoint = Endpoint_3;
	}
	else if (EndpointNumber == 4)
	{
		*Endpoint = Endpoint_4;
	}
	else
	{
		return;
	}

	if (Size > 0) 
	{
		SetDMA(0x20, 0, 0);
		SetDMA(0x20, 0x80, 1);
		Endpoint->PtrL = UsbBufferPA>>8;
		Endpoint->PtrM = UsbBufferPA>>16;
		Endpoint->PtrH = UsbBufferPA>>24;
		Endpoint->Offset = Offset;
		Endpoint->LengthL = LSB(Size);
		Endpoint->LengthM = MSB(Size);
		Endpoint->LengthH = 0;
		Endpoint->CStatus = 0x88;

		while(Endpoint->CStatus & 0x80);
	}
}

// Func (number 1) for send CSW (Command Status Wrapper) container
static void SendCSW()
{
	UsbBuffer[0] = 'U';
	UsbBuffer[1] = 'S';
	UsbBuffer[2] = 'B';
	UsbBuffer[3] = 'S';
	UsbBuffer[4] = ScsiTag[0];
	UsbBuffer[5] = ScsiTag[1];
	UsbBuffer[6] = ScsiTag[2];
	UsbBuffer[7] = ScsiTag[3];
	UsbBuffer[8] = ScsiDataResidue;
	UsbBuffer[9] = ScsiDataResidue>>8;
	UsbBuffer[10] = ScsiDataResidue>>16;
	UsbBuffer[11] = ScsiDataResidue>>24;
	UsbBuffer[12] = ScsiStatus;

	SendDataEndpoint(13, 0, 1);
	UsbHaveCSWReady = 0;
	ScsiDataResidue = 0;
}

// Func (number 2) for send CSW (Command Status Wrapper) container
static void SendCSW2()
{
	while(Endpoint_1.CStatus & bmSTALL);

	while((Endpoint_1.r17 & 0x80)==0)
	{
		if ((XVAL(0xF010) & 0x20)==0)
		{
			UsbHaveCSWReady = 0;
			return;
		}
	}

	while(Endpoint_1.CStatus & 0x40);
	while(Endpoint_2.CStatus & 0x40);
	while(Endpoint_3.CStatus & 0x40);
	while(Endpoint_4.CStatus & 0x40);

	Endpoint_1.FIFORegister = 'U';
	Endpoint_1.FIFORegister = 'S';
	Endpoint_1.FIFORegister = 'B';
	Endpoint_1.FIFORegister = 'S';
	Endpoint_1.FIFORegister = ScsiTag[0];
	Endpoint_1.FIFORegister = ScsiTag[1];
	Endpoint_1.FIFORegister = ScsiTag[2];
	Endpoint_1.FIFORegister = ScsiTag[3];
	Endpoint_1.FIFORegister = ScsiDataResidue;
	Endpoint_1.FIFORegister = ScsiDataResidue>>8;
	Endpoint_1.FIFORegister = ScsiDataResidue>>16;
	Endpoint_1.FIFORegister = ScsiDataResidue>>24;
	Endpoint_1.FIFORegister = ScsiStatus;
	Endpoint_1.LengthL = 13;
	Endpoint_1.LengthM = 0;
	Endpoint_1.LengthH = 0;
	Endpoint_1.CStatus = 0x40;
	UsbHaveCSWReady = 0;
	ScsiDataResidue = 0;
}


// Func for init USB
void InitUSB(void)  
{
	UsbIrq = 0;
	UsbReceivedDataReady = 0;
	UsbHaveCSWReady = 0;
	UsbSpeed = 0;

	Endpoint_1.PtrL = UsbBufferPA>>8;
	Endpoint_1.PtrM = UsbBufferPA>>16;
	Endpoint_1.PtrH = UsbBufferPA>>24;
	Endpoint_1.r8 = 0x10;
	Endpoint_1.Offset = 0;

	Endpoint_2.PtrL = UsbBufferPA>>8;
	Endpoint_2.PtrM = UsbBufferPA>>16;
	Endpoint_2.PtrH = UsbBufferPA>>24;
	Endpoint_2.r8 = 0x10;
	Endpoint_2.Offset = 0;

	if (WARMSTATUS & 2)					//USB warm start
	{
		if ((USBSTAT & bmSpeed) == bmSuperSpeed)
		{
			UsbSpeed = bmSuperSpeed;	// (?) Super speed set
		}
		else if ((USBSTAT & bmSpeed) == bmHighSpeed)
		{
			UsbSpeed = bmHighSpeed;		// (?) High speed set
		}
		else if ((USBSTAT & bmSpeed) == bmFullSpeed)
		{
			UsbSpeed = bmFullSpeed;		// (?) Full speed set
		}
		else
		{
			UsbSpeed = 0;				// (?) Low speed set
		}

		EX1 = 1;						// (?) Enable external_1 interrupts
		EX0 = 1;						// (?) Enable external_0 interrupts
		EPIE = bmEP2IRQ | bmEP4IRQ;		// (?) Enable EP2 and EP4 interrupts
		ScsiDataResidue = 0;	
		ScsiStatus = 0;
		SendCSW();
	}
	else								//USB cold start
	{
		REGBANK = 6;
		__asm
		.db 0x90
		.db 0xF2
		.db 0x83
		.db 0xE0
		.db 0x44
		.db 0x10
		.db 0xF0
		.db 0x90
		.db 0xF2
		.db 0x8C
		.db 0x74
		.db 0xC2
		.db 0xF0
		.db 0xA3
		.db 0x74
		.db 0xE0
		.db 0xF0
		.db 0xA3
		.db 0x74
		.db 0x9C
		.db 0xF0
		.db 0x90
		.db 0xF2
		.db 0x90
		.db 0x74
		.db 0x17
		.db 0xF0
		.db 0x90
		.db 0xF2
		.db 0x93
		.db 0x74
		.db 0x0C
		.db 0xF0
		.db 0x90
		.db 0xF3
		.db 0x20
		.db 0x74
		.db 0xC0
		.db 0xF0
		.db 0xA3
		.db 0x74
		.db 0xE3
		.db 0xF0
		.db 0xA3
		.db 0x74
		.db 0x8B
		.db 0xF0
		.db 0xA3
		.db 0x74
		.db 0x2E
		.db 0xF0
		.db 0x90
		.db 0xF3
		.db 0x26
		.db 0x74
		.db 0x01
		.db 0xF0
		.db 0xA3
		.db 0x74
		.db 0x05
		.db 0xF0
		.db 0x90
		.db 0xF3
		.db 0x0D
		.db 0x74
		.db 0x10
		.db 0xF0
		.db 0x90
		.db 0xF3
		.db 0x0C
		.db 0x74
		.db 0x02
		.db 0xF0
		.db 0x90
		.db 0xF3
		.db 0x0A
		.db 0x74
		.db 0x30
		.db 0xF0
		.db 0x90
		.db 0xF0
		.db 0x00
		.db 0x74
		.db 0x07
		.db 0xF0
		.db 0x90
		.db 0xF2
		.db 0x80
		.db 0x74
		.db 0xB5
		.db 0xF0
		.db 0x90
		.db 0xFA
		.db 0x6F
		.db 0xE0
		.db 0x54
		.db 0xEF
		.db 0xF0
		.db 0xE4
		.db 0x90
		.db 0x42
		.db 0x2B
		.db 0xF0
		.db 0x90
		.db 0x42
		.db 0x2B
		.db 0xE0
		.db 0x04
		.db 0xF0
		.db 0xE0
		.db 0xB4
		.db 0xFA
		.db 0xF6
		.db 0x90
		.db 0xFA
		.db 0x6F
		.db 0xE0
		.db 0x44
		.db 0x10
		.db 0xF0
		.db 0x90
		.db 0xF0
		.db 0x14
		.db 0xE0
		.db 0x54
		.db 0x03
		.db 0x60
		.db 0xF8
		__endasm;
		REGBANK = 0;
		__asm
		.db 0x90
		.db 0xF0
		.db 0x68
		.db 0x74
		.db 0xC0
		.db 0xF0
		__endasm;
		EPIE = bmEP2IRQ | bmEP4IRQ;			// (?) Enable EP2 and EP4 interrupts
		USBCTL = bmAttach | bmHighSpeed;	// (?) Device attached, speed set to high

		XVAL(0xFA38) |= 2;

		EX1 = 1;							// (?) Enable external_1 interrupts
		EX0 = 1;							// (?) Enable external_0 interrupts
						
		for (BYTE b = 0; b < 250; b++);			
	}
}

void USBIsr(void) __interrupt (USB_VECT)
{
	UsbIrq = USBIRQ;
	
	if (UsbIrq & 0x20)
	{
		USBIRQ = 0x20;
	}

	if (UsbIrq & 0x10)
	{
		USBIRQ = 0x10;
	}

	if (UsbIrq & bmSpeedChange)
	{
		USBIRQ = bmSpeedChange;
		 
		if ((USBSTAT & bmSpeed) == bmSuperSpeed)
		{
			UsbSpeed = bmSuperSpeed;
		}
		else if ((USBSTAT & bmSpeed) == bmHighSpeed)
		{
			UsbSpeed = bmHighSpeed;
		}
		else if ((USBSTAT & bmSpeed) == bmFullSpeed)
		{
			UsbSpeed = bmFullSpeed;
		}
		else
		{
			UsbSpeed = 0;
		}
	}

	if (UsbIrq & 0x40)
	{
		USBIRQ = 0x40;
	}

	UsbIntStsF087 = XVAL(0xF087);
	UsbIntStsF086 = XVAL(0xF086);
	UsbIntStsF082 = XVAL(0xF082);
	UsbIntStsF080 = XVAL(0xF080);

	if (UsbIntStsF082 & 0x80)
	{
		XVAL(0xF082) = 0x80;
	}

	if (UsbIntStsF082 & 0x40)
	{
		XVAL(0xF082) = 0x40;
	}

	if (UsbIntStsF080 & 1)
	{
		XVAL(0xF080) = 1;
		if (EP0CS & bmSUDAV)
		{
			bmRequestType = SETUPDAT[0];
			bRequest = SETUPDAT[1]; 
			wIndex = SETUPDAT[2];
			wValue = SETUPDAT[3];
			wLength = SETUPDAT[6] | (SETUPDAT[7] << 8); 
		}
	}

	if (XVAL(0xF082) & 0x20)
	{
		XVAL(0xF082) = 0x20;
	}

	if (XVAL(0xF081) & 0x10)
	{
		XVAL(0xF081) = 0x10;
	}

	if (XVAL(0xF081) & 0x20)
	{
		XVAL(0xF081) = 0x20;
	}

	if (UsbIntStsF080 | UsbIntStsF082 | UsbIntStsF086 | UsbIntStsF087 | UsbIrq)
	{
		EX0 = 0;
	}
}

void EndpointIsr(void) __interrupt (EP_VECT)
{
	BYTE Interrupts = (EPIRQ & (bmEP2IRQ | bmEP4IRQ));
	if (Interrupts & bmEP2IRQ)
	{
		EPIE &= ~bmEP2IRQ;					// (?) Disable EP2 interrupts
		EPIRQ = bmEP2IRQ;					// acknowledge it
		UsbReceivedDataReady |= bmEP2IRQ;	//
	}

	if (Interrupts & bmEP4IRQ)
	{
		EPIE &= ~bmEP4IRQ;					// (?) Disable EP4 interrupts
		EPIRQ = bmEP4IRQ;					// acknowledge it
		UsbReceivedDataReady |= bmEP4IRQ;	// 
	}
}

static void ResetEndpoints()
{
	EPIE = bmEP2IRQ | bmEP4IRQ;				// (?) Enable EP2 and EP4 interrupts
	Endpoint_1.CStatus = 0;					// (?) Endpoint 1 status reset
	Endpoint_2.CStatus = 0;					// (?) Endpoint 2 status reset
	Endpoint_3.CStatus = 0;					// (?) Endpoint 3 status reset
	Endpoint_4.CStatus = 0;					// (?) Endpoint 4 status reset
}

static void HandleControlRequest(void)
{
	BYTE ReturnValue;
	switch(bmRequestType & 0x60)
	{
		case 0:
			ReturnValue = HandleStandardRequest();
			break;
		case 0x20:
			ReturnValue = HandleClassRequest();
			break;
		case 0x40:
			ReturnValue = HandleVendorRequest();
			break;
		default:
			ReturnValue = FALSE;
	}

	if (!ReturnValue)
	{
		EP0CS = wLength ? bmEP0STALL : bmEP0NAK;
	}
}

void HandleUSBEvents(void)
{
	if (UsbIntStsF080 | UsbIntStsF082 | UsbIntStsF086 | UsbIntStsF087 | UsbIrq)
	{
		if (UsbIrq)
		{
			if (UsbIrq & 0x40)
			{
				USBCTL &= ~bmAttach;
				ResetEndpoints();
				XVAL(0xFF88) = 0;
				XVAL(0xFF82) = 0x10;
				while(XVAL(0xFF88)!=2);
				USBCTL = bmAttach;
			}

			if (UsbIrq & bmSpeedChange)
			{
				ResetEndpoints();
			}

			UsbIrq = 0;
		}
		else
		{
			if (UsbIntStsF082 & 0xC0)
			{
				ResetEndpoints();
				XVAL(0xF092) = 0;
				XVAL(0xF096) = 0;
				if (UsbIntStsF082 & 0x40)
				{
					XVAL(0xF07A) = 1;
				}
			}
			else
			{
				if (UsbIntStsF080 & 1)
				{
					HandleControlRequest();
				}
			}

			UsbIntStsF080 = 0;
			UsbIntStsF082 = 0; 
			UsbIntStsF086 = 0; 
			UsbIntStsF087 = 0;
		}

		EX0 = 1;	
	}

	//WHY DOESN'T THIS INTERRUPT FIRE?!
	if (1)//UsbReceivedDataReady)
	{
		if (1)//UsbReceivedDataReady & bmEP4IRQ)
		{
			if (Endpoint_4.FIFOCountRegister > 0)
			{
				Endpoint_4.CStatus = 0x40;

				SendKeysEnabled = 1;
				UsbReceivedDataReady &= ~bmEP4IRQ;
				EPIE |= bmEP4IRQ;
			}
		}

		if (UsbReceivedDataReady & bmEP2IRQ)
		{
			if (Endpoint_2.FIFOCountRegister == 31) //CBW size
			{
				ScsiDataResidue = 0;

				BYTE A = Endpoint_2.FIFORegister;
				BYTE B = Endpoint_2.FIFORegister;
				BYTE C = Endpoint_2.FIFORegister;
				BYTE D = Endpoint_2.FIFORegister;

				if ((A =='U') && (B=='S') && (C=='B') && (D=='C'))
				{
					ScsiTag[0] = Endpoint_2.FIFORegister;
					ScsiTag[1] = Endpoint_2.FIFORegister;
					ScsiTag[2] = Endpoint_2.FIFORegister;
					ScsiTag[3] = Endpoint_2.FIFORegister;
					ScsiTransferSize = Endpoint_2.FIFORegister;
					ScsiTransferSize |= ((DWORD)Endpoint_2.FIFORegister)<<8;
					ScsiTransferSize |= ((DWORD)Endpoint_2.FIFORegister)<<16;
					ScsiTransferSize |= ((DWORD)Endpoint_2.FIFORegister)<<24;
					ScsiDirIn = Endpoint_2.FIFORegister & 0x80;
					ScsiLun = Endpoint_2.FIFORegister;
					ScsiCdbSize = Endpoint_2.FIFORegister;

					for(WORD a = 0; a < 16; a++)
					{
						ScsiCdb[a] = Endpoint_2.FIFORegister;
					}

					Endpoint_2.CStatus = 0x40;

					if (!HandleCDB())
					{
						ScsiStatus = 1;
						if (ScsiTransferSize == 0)
						{
							Endpoint_1.CStatus = bmSTALL;
						}
						else if (ScsiDirIn)
						{
							Endpoint_1.CStatus = bmSTALL;
						}
						else
						{
							Endpoint_2.CStatus = bmSTALL;
						}
					}

					UsbHaveCSWReady = 1;
				}
				else
				{
					Endpoint_2.CStatus = 0x40;
					Endpoint_2.CStatus = 4;
				}
			}
			else
			{
				Endpoint_2.CStatus = 0x40;
				Endpoint_2.CStatus = 4;
			}

			UsbReceivedDataReady &= ~bmEP2IRQ;
			EPIE |= bmEP2IRQ;						// (?) Enable EP2 interrupts
		}
	}

	if (UsbHaveCSWReady)
	{
		SendCSW2();
	}
}
