#include "defs.h"
#include "string.h"
#include "usb.h"

#define PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define TEST_UNIT_READY					0x00
#define INQUIRY							0x12
#define READ_FORMAT_CAPACITIES			0x23
#define MODE_SENSE						0x1A
#define REQUEST_SENSE					0x03

#define VENDOR_BOOT						0xBF
#define VENDOR_INFO						0x05
#define VENDOR_CHIPID					0x56
#define CUSTOM_XPEEK					0x06
#define CUSTOM_XPOKE					0x07
#define CUSTOM_IPEEK					0x08
#define CUSTOM_IPOKE					0x09

BYTE 	ScsiStatus;
DWORD	ScsiDataResidue;
DWORD	ScsiTransferSize;
BYTE	ScsiTag[4];
BYTE	ScsiDirIn;
BYTE	ScsiLun;
BYTE 	ScsiCdb[16];
BYTE	ScsiCdbSize;

BYTE HandleCDB()
{
	//Default to returning a bad status
	ScsiStatus = 1;

	switch(ScsiCdb[0])
	{
		case PREVENT_ALLOW_MEDIUM_REMOVAL:
		{
			ScsiStatus = 0;
			return 1;
		}
		case TEST_UNIT_READY:
		{
			//ScsiStatus = 0;
			return 1;
		}
		case INQUIRY:
		{
			memset(UsbBuffer, 0, 36);
			UsbBuffer[1] = 0x80; //removable media
			UsbBuffer[3] = 0x02; //because the UFI spec says so //default: 0x01
			UsbBuffer[4] = 0x1F; //additional length
			SendDataEndpoint(36, 0, 1);
			ScsiStatus = 0;
			return 1;
		}
		case READ_FORMAT_CAPACITIES:
		{
			memset(UsbBuffer, 0, 12);
			UsbBuffer[3] = 0x08; //capacity list length
			UsbBuffer[6] = 0x10; //number of blocks (sectors) (dummy 2MB)
			UsbBuffer[8] = 0x03;
			UsbBuffer[10] = 0x02; //block length (512 bytes/sector)
			SendDataEndpoint(12, 0, 1);
			ScsiStatus = 0;
			return 1;
		}
		case MODE_SENSE:
		{
			memset(UsbBuffer, 0, 8);
			UsbBuffer[0] = 0x03;
			UsbBuffer[2] = 0x80;
			SendDataEndpoint(4, 0, 1);
			ScsiStatus = 0;
			return 1;
		}
		case REQUEST_SENSE:
		{
			memset(UsbBuffer, 0, 18);
			UsbBuffer[0] = 0x70;
			UsbBuffer[2] = 0x02;
			UsbBuffer[7] = 10;
			UsbBuffer[12] = 0x3A;
			SendDataEndpoint(18, 0, 1);
			ScsiStatus = 0;
			return 1;
		}
		//Vendor-specific requests
		case 0x06:
		case 0xC6:
		case 0xC7:
		{
			switch(ScsiCdb[1])
			{
				case CUSTOM_XPEEK:
				{
					UsbBuffer[0] = XVAL((ScsiCdb[2] << 8) | ScsiCdb[3]);
					SendDataEndpoint(1, 0, 1);
					break;
				}
				case CUSTOM_XPOKE:
				{
					XVAL((ScsiCdb[2] << 8) | ScsiCdb[3]) = ScsiCdb[4];
					SendDataEndpoint(1, 0, 1);
					break;
				}
				case CUSTOM_IPEEK:
				{
					UsbBuffer[0] = IVAL(ScsiCdb[2]);
					SendDataEndpoint(1, 0, 1);
					break;
				}
				case CUSTOM_IPOKE:
				{
					IVAL(ScsiCdb[2]) = ScsiCdb[3];
					SendDataEndpoint(1, 0, 1);
					break;
				}
				case VENDOR_CHIPID:
				{
					memset(UsbBuffer, 0x00, 0x200);
					
					//Set raw command mode
					XVAL(0xF480) = 0x00;
					XVAL(0xF618) = 0xFF;
					
					//Select chip 0
					XVAL(0xF608) = 0xFE;
					
					//Reset it
					XVAL(0xF400) = 0xFF;
					while (!(XVAL(0xF41E) & 0x01));
					
					//Send read chip ID command
					XVAL(0xF400) = 0x90;
					XVAL(0xF404) = 0x00;

					for (int i = 0; i < 6; i++)
					{
						UsbBuffer[i] = XVAL(0xF408);
					}
					
					SendDataEndpoint(0x200, 0, 1);
					ScsiStatus = 0;
					return 1;
				}
				case VENDOR_INFO: //get info
				{
					memset(UsbBuffer, 0x00, 0x210);
					UsbBuffer[0x094] = 0x00;
					UsbBuffer[0x095] = 0x99;
					UsbBuffer[0x096] = 0x53;
					UsbBuffer[0x17A] = 'V';
					UsbBuffer[0x17B] = 'R';
					UsbBuffer[0x17E] = 0x23;
					UsbBuffer[0x17F] = 0x07;
					UsbBuffer[0x200] = 'I';
					UsbBuffer[0x201] = 'F';
					SendDataEndpoint(0x210, 0, 1);
					ScsiStatus = 0;
					return 1;
				}
				case VENDOR_BOOT:
				{
					//This transfers control to boot mode and will not return.
					XVAL(0xFA14) = 0x07;
					XVAL(0xF747) &= 0xEF;
					XVAL(0xFA15) = 0x06;
					XVAL(0xFA38) |= 0x01;
					XVAL(0xF08F) = 0x00;
					XVAL(0xFA68) &= 0xF7;
					XVAL(0xFA6A) &= 0xF7;
					XVAL(0xFA48) &= 0xFE;
					break;
				}
				default:
				{
					//Not handling it, then
					return 0;
				}
			}
		}
		default:
		{
			//Not handling it, then
			return 0;
		}
	}
}
