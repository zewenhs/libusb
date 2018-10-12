/*********************************************************************************
 * 
 * Copyright (C) 2010 Telink Semiconductor 
 * $Author: mingjian $
 * 
 *********************************************************************************/
#ifndef _CMDFUNC_H
#define _CMDFUNC_H

#include <stdio.h>

int Cmd_Process (char *, int argc, char* argv[]) ;


void  Cmd_Func_Init ();
void  Cmd_Func_Close ();

int UartGet (int mode);

#define		TO_LEADING	0x01
#define		TO_RETURN	0x02
#define		TO_WORD		0x04

typedef     unsigned char * LPBYTE;

void Bin2TextOut (LPBYTE lpS, int n, int format);
int freadfromtext(FILE *f, LPBYTE lpD, int size, int offset);

int Hex2Int (char * pc);
int Bin2Text (LPBYTE lpD, LPBYTE lpS, int n, int len, int badr);
int GetOpt (int argc, char* argv[], char * poptc, int * popti, int size, char * fi, char * fo);
//int GetOpt (int & argc, char* argv[], int & bfifo, int & bbin, int & bpair, int & bexit, 
//			int & size, char * fi, char * fo) ;

#define		ADR_CMD			0x9ff8
#define		UCMD_CORE		0x0100
#define		UCMD_ANALOG		0x0200
#define		UCMD_OTP		0x0300
#define		UCMD_FLASH		0x0400
#define		UCMD_ERASE		0x0500
#define		UCMD_UART		0x0600
#define		UCMD_SUSB		0x0700
#define		UCMD_RST		0x0800
#define		UCMD_STALL		0x0900
#define		UCMD_BOOT		0x0a00
#define		UCMD_CONT		0x0b00
#define		UCMD_BP			0x0c00
#define		UCMD_SPEED		0x0f00

#define		UCMD_FRD		0x8000
#define		UCMD_FSWIRE		0x4000
#define		UCMD_FERASE		0x0001
#define		UCMD_FCHECK		0x0002
//#define		UCMD_FSWIRE		0x0001

#define RC_CMD_HELP   "\
tcdb -- TC32 EVK Development Tool\r\n\
\r\n\
usage \r\n\
    tcdb [wf|rf|wc|rc|rst|boot|lrun|uart|bi|vcd [adr [dat]]] -i file_input -o file_output -s size -beum\r\n\
      wf:   write flash;  rf: read flash;  wc:   write device;  rc:   read device\r\n\
      rst:  reset MCU;    boot: boot MCU;  lrun: run program in EVK sram\r\n\
	  uart: uart;         bi: trace on;    vcd:  convert trace file to vcd file\r\n\
	  -u:   select EVK, otherwise select swire device\r\n\
	  -b:   binary file format\r\n\
	  -e:   erase flash before write\r\n\
	  -m:   boot MCU after program\r\n\
\r\n\
tcdb\r\n\
    when cmd argument not specified, [wf -i boot.bin -m] set as default\r\n\
    download boot.bin to flash memory of swire slave device\r\n\
tcdb -u\r\n\
	download boot.bin to flash memory of EVK\r\n\
tcdb rf 0 -s 2k\r\n\
    read 2K byte data from flash memory of swire slave device\r\n\
tcdb wf 0 -i main.bin -b -e\r\n\
	program with erase main.bin to flash memory of swire slave device\r\n\
tcdb wf adr -s 1 -e\r\n\
    flash block erase at adr\r\n\
tcdb uart -s 1\r\n\
	set uart in ASCII mode with usb buffer size 1\r\n\
tcdb uart 2 -s 4\r\n\
	set uart in hex mode with usb buffer size 4\r\n\
tcdb uart -s 40\r\n\
	set uart in ASCII mode with usb buffer size 64\r\n\
tcdb bi -o tr.log 1000\r\n\
	dump trace data to tr.log with usb read file size 4K-byte\r\n\
tcdb vcd -i tr_def.h -o tr 21\r\n\
	convert tr.log to tr.vcd, 33 ns per cycle\r\n\
"

#endif  // _CMDFUNC_H
