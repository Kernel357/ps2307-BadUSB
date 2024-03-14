# BadUSB firmware for Phison 2307

## What it is?
This firmware can turn your Phison 2307 based flash drive into a HID device. This project is an attempt to make a patch for another old ps2307 firmware repository. This patch should adapt it for new versions of Windows and improve the readability of the code.

## Status:
Device is detected and runs payload on Linux and Windows

## Project structure:
- Folder "BinTemplates" contains templates for future firmware images in .bin format.

- Folder "Burner" contains a file in .bin format, which serves to enable us to flash the device. It was previously patched to bypass the protection of the header and body of the firmware.

- Folder "Firmware" contains firmware files.

- Folder "Tools" contains pre-compiled tools for building and flashing.

## How to use?
### Preparation:
First you need to download and install the SDCC tool.

Next, download the project files from the repository:  
```git clone https://github.com/Kernel357/ps2307-BadUSB```

### Building:
After that, go to the project folder and assemble the firmware with the command:  
```.\build```

The fw.bin file will appear in the ```..\Firmware\bin``` folder. You need to move this file to your ```..\Tools``` folder

### Flashing:
Next, we close the contacts with one of the two options shown in the photographs: 
![Variant 1](https://github.com/Kernel357/ps2307-BadUSB/blob/main/Docs%2FVar1.jpg)
![Variant 2](https://github.com/Kernel357/ps2307-BadUSB/blob/main/Docs%2FVar2.jpg)
Connect the device to the computer go to ..\Tools folder and let's check the device mode with the command:  
```.\DriveCom /drive=<Your device letter> /action=GetInfo```

“BootMode” should appear in the “Mode” line. If everything is good, move on to the next step, if not, then try again.

Copy the file ```BN07V502TAW_patch.BIN``` from the ```..\Burner``` folder to the ```..\Tools``` folder and in ```..\Tools``` folder execute the command:  
```.\DriveCom /drive=<Your device letter> /action=SendFirmware /burner=BN07V502TAW_patch.BIN /firmware=fw.bin```

### Testing:
After executing the command, reconnect the device, a notepad should open and the text ```“Hello world!!!”``` will appear in it.

## Gratitude:
Thanks to [bidhata](https://github.com/bidhata) with his repository [phison-2307-BadUSB](https://github.com/bidhata/phison-2307-BadUSB).
