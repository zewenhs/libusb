//
//  usbprn.hpp
//  usbpc
//
//  Created by apps on 12/7/15.
//  Copyright © 2015 apps. All rights reserved.
//

#ifndef usbprn_hpp
#define usbprn_hpp

#include <stdio.h>
#include <libusb-1.0/libusb.h>

typedef int HANDLE;
typedef unsigned char BYTE;
typedef unsigned char UInt8;
typedef unsigned char * LPBYTE;
typedef unsigned long long DWORD;

//#define DEBUG
#define printff printf

/*
typedef enum{
USB = 0X02,
EVK = 0X04,	
}TL_ModeTypdef;

#if 1
typedef enum{
  CHIP_8366    = 0x01,
  CHIP_8368    = 0x02,
  //CHIP_8266    = 0x04,
  //CHIP_8267    = 0x08,
  CHIP_8255    = 0x10,
  CHIP_8255_A2 = 0x20,
}TL_ChipTypdef;
#endif 
*/
typedef enum{
	FLASH = 0x01,
	SRAM = 0X02,
	DIG_REG = 0X4,
	ANA_REG = 0X8,
	DEFAULT = 0X10,
}TL_ObjTypdef;

typedef enum{
  DOWNLOAD	=0x01,
  ERASE		=0x02,
  ACTIVATE	=0x04,
  SETTING   =0x08,
  READ      =0x10,
  WRITE     =0x20,
}TL_CMDTypdef;

typedef enum
{
    TL_DUTCMD_FLASH_ASK        = 0x40,
    TL_DUTCMD_FLASH_WRITE      = 0x41,
    TL_DUTCMD_FLASH_CHECK      = 0x42,
    TL_DUTCMD_FLASH_WBYTE	   = 0x43,   //write_bytes
    TL_DUTCMD_FLASH_BYTE_CHK   = 0x44,
    TL_DUTCMD_FLASH_WID		   = 0x45,   //write_id
    TL_DUTCMD_FLASH_ID_CHK	   = 0x46,
    TL_DUTCMD_FLASH_ZERO       = 0x47,
    TL_DUTCMD_FLASH_CPID       = 0x48,
    TL_DUTCMD_FLASH_FAST_WRITE = 0x49,

    TL_DUTCMD_FLASH_ERASE      = 0x4d,
    TL_DUTCMD_FLASH_PROTECT    = 0x4e,
    TL_DUTCMD_FLASH_READ	   = 0x4f,
    TL_DUTCMD_FLASH_CRC        = 0x50,
   	TL_DUTCMD_USB_REBOOT       = 0x57,
}TL_DutcmdTypdef;


#define R_BUF_SIZE	(1024*4)

int GetPrinterHandle(int argc, const char *argv[]);
//int WriteToDevice(IOUSBDeviceInterface **dev, UInt16 deviceAddress, UInt16 length, UInt8 writeBuffer[]);
//int ReadFromDevice(IOUSBDeviceInterface **dev, UInt16 deviceAddress, UInt16 length, UInt8 writeBuffer[], int * nr);


int WriteUSBReg(libusb_device_handle *hdev, int addr, int dat);
int ReadUSBReg(libusb_device_handle *hdev, int addr, int  dat);

int WriteUSBMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen);
int ReadUSBMem(libusb_device_handle * hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen);
int WriteUSBMemCheck(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen);
int ReadUSBMemCheck(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen);

//int WriteUSBMemCheck(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen);
int WriteMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type);
int WriteMem_check(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type);
int ReadMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type);
//int ReadMem(HANDLE hdev, int addr, LPBYTE lpB, int len, int type);
int ReadUartMem(libusb_device_handle *hdev, LPBYTE lpB);
//int ReadUartMem(HANDLE hdev, LPBYTE lpB);

int IsEVKDevice (libusb_device_handle *h);
int EVKCommand (libusb_device_handle *hdev, int addr, int len, int cmd, LPBYTE lpB, int * ptick);


#endif /* usbprn_hpp */
