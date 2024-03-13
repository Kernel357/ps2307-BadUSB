#include "defs.h"
#include "usb.h"
#include "timers.h"

static const USB_DeviceDescriptor DeviceDescriptor = {
	.bLength = DEVICE_DESCRIPTOR_SIZE,
	.bDescriptorType = DEVICE_DESCRIPTOR,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = 0x40,
	.idVendor = 0x0484,
	.idProduct = 0x3711,
	.bcdDevice_Ver = 0x0004,
	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,
	.bNumConfigurations = 0x01
};

static const USB_ConfigDescriptor ConfigDescriptor = {
	.bLength = CONFIGURATION_DESCRIPTOR_SIZE,
	.bDescriptorType = CONFIGURATION_DESCRIPTOR,
	.wTotalLength = sizeof(ConfigDescriptor),
	.bNumInterfaces = 0x01,
	.bConfigurationValue = 0x01,
	.iConfiguration = 0x00,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.InterfaceDescriptor = {
		.bLength = INTERFACE_DESCRIPTOR_SIZE,
		.bDescriptorType = INTERFACE_DESCRIPTOR,
		.bInterfaceNumber = 0x00,
		.bAlternateSetting = 0x00,
		.bNumEndpoints = 0x04,
		.bInterfaceClass = 0x03,
		.bInterfaceSubClass = 0x01,
		.bInterfaceProtocol = 0x01,
		.iInterface = 0x00
	},

	.LinkToHIDReportDescriptor = {
		.bLength = LINK_TO_HID_REPORT_DESCRIPTOR_SIZE,
		.bDescriptorType = LINK_TO_HID_REPORT_DESCRIPTOR,
		.bcdHID = 0x0111,
		.bCountryCode = 0x00,
		.bNumDescriptors = 0x01,
		.bDescriptorType1 = HID_REPORT_DESCRIPTOR,
		.wDescriptorLength = sizeof(HIDReportDescriptor)
	},

	.Ep1InDescriptor = {
		.bLength = ENDPOINT_DESCRIPTOR_SIZE,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = 0x81,
		.bmAttributes = EP_BULK,
		.wMaxPacketSize = 0x0040,
		.bInterval = 0x01
	},

	.Ep2OutDescriptor = {
		.bLength = ENDPOINT_DESCRIPTOR_SIZE,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = 0x02,
		.bmAttributes = EP_BULK,
		.wMaxPacketSize = 0x0040,
		.bInterval = 0x01
	},

	.Ep3InDescriptor = {
		.bLength = ENDPOINT_DESCRIPTOR_SIZE,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = 0x83,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = 0x0080,
		.bInterval = 0x00
	},

	.Ep4OutDescriptor = {
		.bLength = ENDPOINT_DESCRIPTOR_SIZE,
		.bDescriptorType = ENDPOINT_DESCRIPTOR,
		.bEndpointAddress = 0x04,
		.bmAttributes = EP_INTERRUPT,
		.wMaxPacketSize = 0x0080,
		.bInterval = 0x01
	}
};

static const BYTE HIDReportDescriptor[] = { 
	0x05, 0x01, 
	0x09, 0x06,
	0xA1, 0x01, 
	0x05, 0x07,
	
	0x19, 0xE0, 
	0x29, 0xE7, 
	0x15, 0x00, 
	0x25, 0x01, 
	0x75, 0x01,
	
	0x95, 0x08, 
	0x81, 0x02,
	0x95, 0x01,
	0x75, 0x08, 
	0x81, 0x01,
	
	0x95, 0x05, 
	0x75, 0x01,
	0x05, 0x08,
	0x19, 0x01, 
	0x29, 0x05,
	
	0x91, 0x02,
	0x95, 0x01,
	0x75, 0x03,
	0x91, 0x01,
	0x95, 0x06,
	
	0x75, 0x08, 
	0x15, 0x00, 
	0x25, 0x65, 
	0x05, 0x07,
	0x19, 0x00,
	
	0x29, 0x65,
	0x81, 0x00, 
	0xC0 
};

static const USB_DeviceQualifierDescriptor DeviceQualifierDescriptor = {
	.bLength = DEVICE_QALIFIER_DESCRIPTOR_SIZE,
	.bDescriptorType = DEVICE_QALIFIER_DESCRIPTOR,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x03,
	.bDeviceSubClass = 0x01,
	.bDeviceProtocol = 0x07,
	.bMaxPacketSize0 = 0x40,
	.bNumConfigurations = 0x01,
	.Reserved = 0x00
};

static const USB_StringLangDescriptor StringLangDescriptor = {
	.bLength = sizeof(StringLangDescriptor),
	.bDescriptorType = STRING_DESCRIPTOR,
	.wLANGID = 0x0409
};

static const USB_StringManufacturingDescriptor StringManufacturingDescriptor = {
	.bLength = sizeof(StringManufacturingDescriptor),
	.bDescriptorType = STRING_DESCRIPTOR,
	.bString[0] = 'S',
	.bString[1] = 'O',
	.bString[2] = 'B',
	.bString[3] = 'S'
};

static const USB_StringProdDescriptor StringProdDescriptor = {
	.bLength = sizeof(StringProdDescriptor),
	.bDescriptorType = STRING_DESCRIPTOR,
	.bString[0] = 'H',
	.bString[1] = 'I',
	.bString[2] = 'D',
	.bString[3] = ' ',
	.bString[4] = 'T',
	.bString[5] = 'E',
	.bString[6] = 'X',
	.bString[7] = 'T'
};

static const USB_StringSerialDescriptor StringSerialDescriptor = {
	.bLength = sizeof(StringSerialDescriptor),
	.bDescriptorType = STRING_DESCRIPTOR,
	.bString[0] = '1',
	.bString[1] = '2',
	.bString[2] = '3',
	.bString[3] = '4',
	.bString[4] = '5',
	.bString[5] = '6',
	.bString[6] = '7',
	.bString[7] = '8'
};

void EP0ACK()
{
	EP0CS = bmEP0ACK; //Successful command execution (sent via endpoint 0)
}

static BYTE SetAddress()
{
	if (wValue < 0x7F)
	{
		EP0ACK();	 //Successful command execution
		return TRUE;
	}

	return FALSE;
}

static BYTE GetDescriptor()
{
	BYTE Type = wValue & 0xFF;
	BYTE Index = wIndex & 0xFF;
	BYTE ReturnValue = FALSE;

	switch (Type)
	{
		case DEVICE_DESCRIPTOR:
		{		
			BYTE* DescriptorIter = &DeviceDescriptor.bLength;

			for (BYTE i = 0; i < sizeof(DeviceDescriptor); i++)
			{
				Endpoint_0.FIFORegister = *DescriptorIter;
				DescriptorIter = DescriptorIter + 1;
			}

			SendControlResponse(0x12);
			ReturnValue = TRUE;

			break; 
		}
		case CONFIGURATION_DESCRIPTOR: 
		{
			BYTE* DescriptorIter = &ConfigDescriptor.bLength;

			for (BYTE i = 0; i < sizeof(ConfigDescriptor); i++)
			{
				Endpoint_0.FIFORegister = *DescriptorIter;
				DescriptorIter = DescriptorIter + 1;
			}

			SendControlResponse(wLength < sizeof(ConfigDescriptor) ? wLength : sizeof(ConfigDescriptor));
			ReturnValue = TRUE;

			break;
		}
		case DEVICE_QALIFIER_DESCRIPTOR:
		{
			BYTE* DescriptorIter = &DeviceQualifierDescriptor.bLength;

			for (BYTE i = 0; i < sizeof(DeviceQualifierDescriptor); i++)
			{
				Endpoint_0.FIFORegister = *DescriptorIter;
				DescriptorIter = DescriptorIter + 1;
			}
			
			SendControlResponse(wLength < sizeof(DeviceQualifierDescriptor) ? wLength : sizeof(DeviceQualifierDescriptor));
			ReturnValue = TRUE;

			break;
		}
		case HID_REPORT_DESCRIPTOR:
		{
			for (BYTE i = 0; i < sizeof(HIDReportDescriptor); i++)
			{
				Endpoint_0.FIFORegister = HIDReportDescriptor[i];
			}
			
			SendControlResponse(wLength < sizeof(HIDReportDescriptor) ? wLength : sizeof(HIDReportDescriptor));
			ReturnValue = TRUE;

			//IsDeviceReady = 1;
		
			break;
		}

		case STRING_DESCRIPTOR:
		{
			switch (Index)
			{
				case STRING_LANG_DESCRIPTOR:
				{
					BYTE* DescriptorIter = &StringLangDescriptor.bLength;

					for (BYTE i = 0; i < sizeof(StringLangDescriptor); i++)
					{
						Endpoint_0.FIFORegister = *DescriptorIter;
						DescriptorIter = DescriptorIter + 1;
					}

					SendControlResponse(wLength < sizeof(StringLangDescriptor) ? wLength : sizeof(StringLangDescriptor));
					ReturnValue = TRUE;

					break;
				}

				case STRING_MAN_DESCRIPTOR:
				{
					BYTE* DescriptorIter = &StringManufacturingDescriptor.bLength;

					for (BYTE i = 0; i < sizeof(StringManufacturingDescriptor); i++)
					{
						Endpoint_0.FIFORegister = *DescriptorIter;
						DescriptorIter = DescriptorIter + 1;
					}

					SendControlResponse(wLength < sizeof(StringManufacturingDescriptor) ? wLength : sizeof(StringManufacturingDescriptor));
					ReturnValue = TRUE;

					break;
				}

				case STRING_PROD_DESCRIPTOR:
				{
					BYTE* DescriptorIter = &StringProdDescriptor.bLength;

					for (BYTE i = 0; i < sizeof(StringProdDescriptor); i++)
					{
						Endpoint_0.FIFORegister = *DescriptorIter;
						DescriptorIter = DescriptorIter + 1;
					}

					SendControlResponse(wLength < sizeof(StringProdDescriptor) ? wLength : sizeof(StringProdDescriptor));
					ReturnValue = TRUE;

					break;
				}

				case STRING_SN_DESCRIPTOR:
				{
					BYTE* DescriptorIter = &StringSerialDescriptor.bLength;

					for (BYTE i = 0; i < sizeof(StringSerialDescriptor); i++)
					{
						Endpoint_0.FIFORegister = *DescriptorIter;
						DescriptorIter = DescriptorIter + 1;
					}

					SendControlResponse(wLength < sizeof(StringSerialDescriptor) ? wLength : sizeof(StringSerialDescriptor));
					ReturnValue = TRUE;

					break;
				}

				default:
					break;
			}

			break;
		}

		default:
			break;
	}

	return ReturnValue;
}

static BYTE SetConfiguration()
{
	if (wValue <= 1)
	{
		EP0ACK(); // Successful command execution
		return TRUE;
	}

	return FALSE;
}

static BYTE SetInterface()
{
	if (wValue <= 1)
	{
		EP0ACK(); // Successful command execution
		return TRUE;
	}

	return FALSE;
}

BYTE HandleStandardRequest()
{
	switch(bRequest)
	{
		case 0x05:
		{
			return SetAddress();
			break;
		}
		case 0x06:
		{
			return GetDescriptor();
			break;
		}
		case 0x09:
		{
			return SetConfiguration();
			break;
		}
		case 0x11:
		{
			return SetInterface();
			break;
		}
		default:
		{
			return FALSE;
			break;
		}
	}
}

static BYTE GetMaxLUN()
{
	Endpoint_0.FIFORegister = 0x00;
	SendControlResponse(wLength < 0x01 ? wLength : 0x01);

	return TRUE;
}

BYTE HandleClassRequest()
{
	switch(bRequest)
	{
		case 0x09:
		{
			EP0CS = 0x05;
			return TRUE;
		}
		case 0x0A:
		{
			EP0ACK(); //Successful command execution
			return TRUE;
		}
		case 0xFE:
		{
			return GetMaxLUN();
		}
		default:
		{
			return FALSE;
		}
	}
}

BYTE HandleVendorRequest()
{
	return FALSE;
}
