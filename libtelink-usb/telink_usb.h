/*************************************************************************
	> File Name: telink_usb.h
	> Author: Telink
	> Created Time: Mon 02 Apr 2018 03:36:37 PM CST
 ************************************************************************/

#pragma once

#include <libusb-1.0/libusb.h>

typedef enum {
	SCAN_ON,
	SCAN_OFF,
	CONNECT,
	DISCONNECT,
	OTA,
	BAT_STATUS
}TL_CMDType;

typedef enum{
  CHIP_8366    = 0x01,
  CHIP_8368    = 0x02,
  CHIP_8266    = 0x04,
  CHIP_8267    = 0x08,
  CHIP_8255    = 0x10,
  CHIP_8255_A2 = 0x20,
}TL_ChipTypdef;

typedef enum{
USB = 0X02,
EVK = 0X04,	
}TL_ModeTypdef;

/**
 * Get a telink usb device handle
 *
 *@RETURN: a device handle for the first found telink device, or NULL on error or if the device could not be found.
 *eg: if your pc have a evk and 8266 dongle inserted at the same time, the evk may be opened, because this function 
 *		only distinguish the vendor id, not the the product id.
 * **/
libusb_device_handle  *telink_usb_open(unsigned int vid, unsigned int pid);

/**
 * Release a telink usb device handle
 *
 *@hDev: usb device handle return by telink_usb_open()
 * **/
void telink_usb_close(libusb_device_handle *hDev);

/**
 * Download firmware to flash of telink usb device
 *
 *@hDev: usb device handle return by telink_usb_open()
 *@adr: flash address to be written
 *@file_path: download fw path
 *@Type: telink chip type, eg, 8266-->0x04, 8267/8269-->0x08
 *
 *@RETURN: 0 on success; other failed
 * **/
int telink_usb_download(libusb_device_handle *hDev, unsigned int adr, const char *file_path, TL_ChipTypdef Type);

/**
 * Perform a command by usb
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@cmd: command defined by TL_CMDType
 *@data: some parameter if needed(eg, MAC address when use CONNECT cmd)
 *
 * @RETURN: 0 on success; other see libusb_error
 * **/
int telink_usb_action(libusb_device_handle *hDev, TL_CMDType cmd, unsigned char *data);

/**
 * Get data from dongle
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@buf: data buffer for output
 *@len: the maximum number of bytes to receive into the buf
 *@size: the number of bytes actually transferred
 *
 * @RETURN: 0 on success; other see libusb_error
 * **/
int telink_usb_get_data(libusb_device_handle *hDev, unsigned char *buf, int len, int *size);

/**
 * Write memory
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@addr: memory address
 *@buf: data buffer to write
 *@len: the data length you want write
 * **/
int telink_usb_w_mem(libusb_device_handle *hDev, uint16_t addr, uint8_t *buf, uint16_t len);

/**
 * Read memory
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@addr: memory address
 *@buf: data buffer for read
 *@len: the data length you want read
 * **/
int telink_usb_r_mem(libusb_device_handle *hDev, uint16_t addr, uint8_t *buf, uint16_t len);


/**
 * Read flash : Read 4K size at most each time.
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@Type: telink chip type, eg, 8266-->0x04, 8267/8269-->0x08
 *@RW_Adr: flash address
 *@rbuf: read buffer
 *@Size: the data length you want read
 * **/
int telink_usb_r_flash(libusb_device_handle *hDev, TL_ChipTypdef Type, unsigned int RW_Adr,  char *rbuf, unsigned int Size);

/**
 * Write flash
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@Type: telink chip type, eg, 8266-->0x04, 8267/8269-->0x08
 *@RW_Adr: flash address
 *@value: the data you want to write
 *@Size: the data length you want to write
 *
 * @RETURN: 0 on success; -1 on fail
 * **/
int telink_usb_w_flash(libusb_device_handle *hDev, TL_ChipTypdef Type, unsigned int RW_Adr, unsigned long value, unsigned int Size);

/**
 * Erase 4K flash
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@Type: telink chip type, eg, 8266-->0x04, 8267/8269-->0x08
 *@RW_Adr: flash address
 *
 * @RETURN: 0 on success; -1 on fail
 * **/
int telink_usb_flash_erase(libusb_device_handle *hDev, TL_ChipTypdef Type, unsigned int RW_Adr);

/**
 * Reboot device
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@RETURN: 0 on success; other failed
 * **/
int telink_usb_reboot(libusb_device_handle *hDev);

/**
 * Start Ram firmware
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@Type: telink chip type, eg, 8266-->0x04, 8267/8269-->0x08
 *@RETURN: 1 on success; 0 on failed
 * **/
int MCU_Init(libusb_device_handle *hDev,TL_ChipTypdef Type);

/**
 * Check flash crc
 *
 *@hDev: usb device_handle return by telink_usb_open()
 *@addr: flash address
 *@size: flash size you want to check crc
 *@RETURN: crc value
 * **/
int telink_check_flash_crc(libusb_device_handle *hDev, int addr, size_t size);
