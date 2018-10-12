//
//  main.cpp
//  usbpc
//
//  Created by apps on 12/3/15.
//  Copyright © 2015 apps. All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "usbprn.h"
#include "cmdfunc.h"
#include <libusb-1.0/libusb.h>
#include "telink_usb.h"

#if 1

#define CMDS_MAX_ARGC 		64
#define CMDS_MAX_ARG_LENGTH 	36
#define CMDS_MAX_CMD_LENGTH 	36

int CMDS_Exec (char * cmdline);
int CMDS_Exec_Single (char * cmdline);

int bPCReadFile;
char cmdbuf[16384];
//LPBYTE lpBIBuff;
int bi_wptr;
int bi_rptr;
int bi_enable;
int bi_wptr_update;
extern libusb_device_handle *m_hDev;
extern libusb_context *ctx;

unsigned char R_buffer[R_BUF_SIZE];
unsigned char W_buffer[4];

#define BIBUFFSIZE    1024*1024*2
//#define BIBUFFSIZE    1024

int uart_mode;

#if 1
int MainTask (char * str)
{
    bi_wptr = 0;
    bi_rptr = 0;
    bi_enable = 0;

    Cmd_Func_Init ();
    uart_mode = 0;

    strcpy (cmdbuf, str);


    if (strlen (cmdbuf) > 0) {
        if (! CMDS_Exec(cmdbuf) ) return 0;
    }
    else
        CMDS_Exec_Single ("nocmd");
    Cmd_Func_Close ();

    return 1;
}

int CMDS_Exec (char * cmdline)
{

    char* pstr = cmdline;

    int ret = 1;
    int stop = 0;
    char cmdbuf[1024] = "nocmd ";

    do {
        while ((*pstr==' ' || *pstr=='\t' || *pstr==';' ||
                *pstr=='\r' || *pstr=='\n') && * pstr != '\0') pstr++;
        char * ps = pstr;
        while (*pstr!=';' && *pstr != '\0') pstr++;

        if (*pstr == '\0') stop = 1;

        if (*ps!='\0') {
            *pstr='\0';
            if (*ps == '-') {
                strcpy (cmdbuf+6, ps);
                ret = CMDS_Exec_Single (cmdbuf);
            }
            else
                ret = CMDS_Exec_Single (ps);
            if (stop)	return ret;
            else		pstr++;
        }
        else {
            return 1;
        }
    } while (ret);

    return 0;
}

int CMDS_Exec_Single (char * cmdline)
{

    int i;
    int ret;
    int argc = 0;
    char* argv[CMDS_MAX_ARGC];
    char* pstr = cmdline;
    char * pStrCmd;

    for (i=0; i<CMDS_MAX_ARGC; i++) argv[i] = cmdline + strlen (cmdline);

    // skip leading blanks
    while (*pstr == ' ' && * pstr != '\0')	pstr++;
    pStrCmd = pstr;
    int loop = 1;

    while (loop)	{
        if (argc>0 && *pstr == '"') {
            argv[argc-1]++;
            pstr++;
            while (*pstr != '"' && *pstr != '\0')	pstr++;
        }
        else {
            while (*pstr != ' ' && *pstr != '\0')	pstr++;
        }
        if (*pstr) {
            *pstr++ = '\0';
            while (*pstr == ' ') pstr++;
            argv[argc++] = pstr;
        }
        else
            loop = 0;
    }

    if ((argc > 0) && !(*argv[argc-1]))
        argc--;

    if (strcmp(pStrCmd, "") != 0)  {
       {
            ret = Cmd_Process (pStrCmd, argc, argv);
        }
    }

    return ret;
}
#endif

#endif

int TL_Dut_cmd_Process(libusb_device_handle *hDev, TL_DutcmdTypdef cmd, TL_ModeTypdef Mode, TL_ChipTypdef Type, unsigned long int p1,unsigned long int p2)
{
    unsigned char dut_cmd_buff[10]={0};
    unsigned char mybuf[3]={0};
    unsigned char clear_buff[4]={0};
	unsigned char buffer[2]={0};
    long int t;
	struct timeval ts, te;
   	unsigned int timeout_ms = 2000;
    memset(dut_cmd_buff,0,10);
    memset(clear_buff,0,4);

    dut_cmd_buff[0]= cmd;
    dut_cmd_buff[1]= p1&0xff;
    dut_cmd_buff[2]= (p1>>8)&0xff;
    dut_cmd_buff[3]= (p1>>16)&0xff;
    dut_cmd_buff[4]= (p1>>24)&0xff;
    dut_cmd_buff[5]= p2&0xff;
    dut_cmd_buff[6]= (p2>>8)&0xff;
    dut_cmd_buff[7]= (p2>>16)&0xff;
    dut_cmd_buff[8]= (p2>>24)&0xff;

	//printf("zewen---> [FUNC]%s [LINE]:%d cmd is : 0x%x\n", __FUNCTION__, __LINE__, cmd);
	if(WriteMem(hDev,0x8004,clear_buff,3,USB)!=3)
	{
		printff(" Fail to Clear buff! \t\n");
		return 0;
	}
	if(WriteMem(hDev,0x8007,dut_cmd_buff,9,USB)!=9)
	{
		printff(" Fail to write buff! \t\n");
		return 0;
	}
	dut_cmd_buff[0]= cmd|0x80;
	if(WriteMem(hDev,0x8007,dut_cmd_buff,1,USB)!=1)
	{
		printff(" Fail to execute cmd! \t\n");
		return 0;
	}

	if(cmd == TL_DUTCMD_USB_REBOOT) //device is reboot now
	{
		return 1;
	}

	gettimeofday(&ts, NULL);
	while(dut_cmd_buff[0]&0x80)
	{
		ReadMem(hDev,0x8007,dut_cmd_buff,1,USB);
		usleep(1000);
		ReadMem(hDev,0x8004,mybuf,3,USB);
		gettimeofday(&te, NULL);
		t = (te.tv_sec - ts.tv_sec) * 1000 + (te.tv_usec - ts.tv_usec) /1000;
		if(t > timeout_ms)
		{
			printff(" Wait Flash ACK timeout: %d ms  cmd:%x 0x8007 bit0~bit6:%x  p1:%lx  p2:%lx  mybuf[0]:%x mybuf[1]:%x mybuf[2]:%x\t\n",timeout_ms, cmd, dut_cmd_buff[0] &0x7f, p1, p2, mybuf[0], mybuf[1], mybuf[2]);
        int it = 0;
        while(it < 20)
        {
            it++;
            ReadMem(hDev, 0x6bc, buffer, 2, USB);
            printf("----pc-----:%x%x\n", buffer[1], buffer[0]);
        }
			return 0;
		}
	}
	if(ReadMem(hDev,0x8004,dut_cmd_buff,4,USB)!=4)
	{
		return 0;
	}

	if((dut_cmd_buff[2]&0xff)!=cmd)
	{
		printff(" Wait Flash ACK Failure!  cmd :0x%x dut_cmd_buff[2]:0x%x\t\n", cmd, dut_cmd_buff[2]);
        int it = 0;
        while(it < 20)
        {
            it++;
            ReadMem(hDev, 0x6bc, buffer, 2, USB);
            printf("----pc-----:%x%x\n", buffer[1], buffer[0]);
        }
		//return 0;
	}
            
           
    if(cmd==TL_DUTCMD_FLASH_ASK)
    {
        int bin_buffer_adr = dut_cmd_buff[0] + dut_cmd_buff[1]*256;
        if((Type==CHIP_8255)||(Type==CHIP_8255_A2))
        {
            return (bin_buffer_adr+0x40000);
        }
        else
        {
            return bin_buffer_adr;
        }

    }

	if(cmd == TL_DUTCMD_FLASH_CRC)
	{
		if(ReadMem(hDev, 0x800c, dut_cmd_buff, 4, USB) != 4)
		{
			return 0;
		}

		int flash_crc = dut_cmd_buff[0]  | dut_cmd_buff[1] << 8 | dut_cmd_buff[2] << 16 | dut_cmd_buff[3] << 24;
		printf("flash_crc:%x ~flash_crc:%x\n", flash_crc, ~flash_crc);

		return ~flash_crc;
	}

    return 1;
}


int MCU_Init(libusb_device_handle *hDev,TL_ChipTypdef Type)
{
	unsigned char buffer[2]={0};
	unsigned char ram_buffer[1024*8]={0};
	signed long int size=0;
	FILE *fp = NULL;

	if(Type != 0x01  && Type != 0x02 && Type && 0x04 && Type != 0x08 && Type != 0x10 && Type != 0x20)
	{
		printf("fatal error! Chip Type:%d is invalid\n", Type);
		return 0;
	}


	if(Type == CHIP_8267)
	{
		//fp= fopen("dut_5326_flash_v0002.bin","rb");
		fp= fopen("8267_dut_flash.bin","rb");
	}
	else if(Type == CHIP_8266)
	{
		//fp= fopen("dut_5325_flash_v0222.bin","rb");
		fp= fopen("8266_dut_flash.bin","rb");
	}

	if(NULL==fp){printf(" zeeeejjnn   Fail to open file! \t\n");return 0;}
	fseek(fp,0L,SEEK_END);
    size=ftell(fp);
    fseek(fp,0,SEEK_SET);
    fread(ram_buffer,1,size,fp);
    fclose(fp);
    
    //*********    reset mcu    *********************
	buffer[0]=0x05;
	unsigned char i=0;
    int it = 0;
    while(it < 2)
    {    
        while((buffer[1]&0x05)!=0x05)
        {	
            WriteMem(hDev, 0x602, buffer, 1,USB);
            ReadMem(hDev, 0x602, buffer+1, 1,USB);
            //printf("buffer:%d\n", buffer[1]); 
            i++;
            if(i>3){printf(" TC32 USB : USB Err! \t\n");return 1;}
        }
        buffer[1] = 0;
       it++; 
       i = 0;
    }
    it = 0;
    buffer[0] = 0;
    buffer[1] = 0X99;
#if 0
    while(it < 20)
    {
        it++;
        ReadMem(hDev, 0x6bc, buffer, 2, USB);
        printf("----pc-----:%x%x\n", buffer[1], buffer[0]);
    }
#endif
	printf(" TC32 USB : USB OK \t\n");
 //********************   Disable watch dog, Disable interrupt,download file to ram and start MCU   *********************
	buffer[0]=0x00;
	
	WriteMem(hDev, 0x622, buffer, 1,USB);
	WriteMem(hDev, 0x643, buffer, 1,USB);

	buffer[0]=0x40;
	WriteMem(hDev, 0x60c, buffer, 1,USB);
	WriteMem(hDev, 0x60d, buffer, 1,USB);

	buffer[0]=0xff;
   	WriteMem(hDev, 0x104, buffer, 1,USB);

   if(WriteMem(hDev,0x8000,ram_buffer,size,USB)!=size)
   {
	   printff("Write file to ram fail via USB! \t\n");
	   return 0;
   }
   #if 0
   if((Type==CHIP_8366)||(Type==CHIP_8368))
   {
	   buffer[0]=0x01;
	   WriteMem (hDev,0x8ff0,buffer,1,USB);
   }
   #endif
   buffer[0] = 0x88;
   int t1 = WriteMem(hDev,0x602,buffer,1,USB);
   if(t1 != 1)
   {
	   printff(" Fail to start MCU via USB! \t\n");
	   return 0;
   }
 return 1;
}


int flash_rw_func(libusb_device_handle *hDev, TL_ModeTypdef Mode,TL_ChipTypdef Type,unsigned int RW_Adr, char *rbuf, unsigned int Size,unsigned int value,TL_CMDTypdef cmd)
{
	struct timeval start, end;
    //unsigned long t = timeGetTime();
	gettimeofday(&start, NULL);
    switch(Mode)
    {
        case USB:
        {
            if(Type==CHIP_8255||Type==CHIP_8255_A2)
            {
                //HANDLE hDev = GetPrintDeviceHandle2(0xffff);
                //if(!MCU_Init(hDev,Type,USB)){return;}
                //if(!MCU_Init(hDev,Type)){return;}

                if(cmd==READ)
                {
                    unsigned char max_rdat_len=2;
                    //ZeroMemory(R_buffer,R_BUF_SIZE);
                    memset(R_buffer, 0, R_BUF_SIZE);
                    unsigned int num    = Size/max_rdat_len;
                    unsigned char num_s = Size-(num*max_rdat_len);
                    for(unsigned int i=0;i<num;i++)
                    {
                        if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_READ,USB,Type,RW_Adr+max_rdat_len*i,max_rdat_len)!=0)
                        {
                            unsigned int ram_adr = 0x40004;
                            if(max_rdat_len>2) ram_adr = 0x40008;
                            //if(ReadMem2(hDev,ram_adr,R_buffer+max_rdat_len*i,max_rdat_len,USB)!=max_rdat_len)
                            {
                                printff("Read Err!\t\n");
                               return -1;
                            }
                        }
                        else
                        {
                            return -1;
                        }
                    }
                    if(num_s!=0)
                    {
                        if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_READ,USB,Type,RW_Adr+num*max_rdat_len,num_s)!=0)
                        {
                            unsigned int ram_adr = 0x40004;
                            if(num_s>2) ram_adr = 0x40008;
                            //if(ReadMem2(hDev,ram_adr,R_buffer+num*max_rdat_len,num_s,USB)!=num_s)
                            {
                                printff("Read Err!\t\n");
                                return -1;
                            }
                        }
                        else
                        {
                            return -1;
                        }
                    }


                    for(unsigned int i=0;i<Size;i++)
                    {
                        if(i%16==0)printff("\n %.6x: ",RW_Adr+i);
                        if(i%8==0)printff(" ");
                        printff("%.2x ",R_buffer[i]);
                    }
                }
                else if(cmd == WRITE)
                {
                    int ram_adr = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ASK,USB,Type,0,0);
                    //ZeroMemory(W_buffer,4);
                    memset(W_buffer, 0, 4);
                    if(Size>4)
                    {
                        printff(" Write flash size exceed!!\t\n");
                        return -1;
                    }
                    for(unsigned char i=0;i<Size;i++)
                    {
                        W_buffer[0]= (unsigned char)(value>>(8*i));
                        //if(WriteMem2(hDev,ram_adr,W_buffer,1,USB)!=1)
                        {
                             printff("\n WRITE SRAM ERR! \t\n");
                             return -1;
                        }
                        if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_WRITE,USB,Type,RW_Adr+i,1)!=0)
                        {
                             printff("  Flash Sector (4K) Program at address %x \t\n",RW_Adr+i);
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
            }
            else
            {
                //HANDLE hDev = GetPrintDeviceHandle(0xffff);
                //if(!MCU_Init(hDev,Type,USB)){return;}

                if(cmd==READ)
                {
                    unsigned char max_rdat_len=2;
                    //ZeroMemory(R_buffer,R_BUF_SIZE);
                    memset(R_buffer, 0, R_BUF_SIZE);
                    unsigned int num    = Size/max_rdat_len;
                    unsigned char num_s = Size-(num*max_rdat_len);
                    for(unsigned int i=0;i<num;i++)
                    {
                        if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_READ,USB,Type,RW_Adr+max_rdat_len*i,max_rdat_len)!=0)
                        {
                            unsigned int ram_adr = 0x8004;
                            if(max_rdat_len>2) ram_adr = 0x8008;
                            if(ReadMem(hDev,ram_adr,R_buffer+max_rdat_len*i,max_rdat_len,USB)!=max_rdat_len)
                            {
                                printff("Read Err!\t\n");
                                return -1;
                            }
                        }
                        else
                        {
                            return -1;
                        }
                    }
                    if(num_s!=0)
                    {
                        if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_READ,USB,Type,RW_Adr+num*max_rdat_len,num_s)!=0)
                        {
                            unsigned int ram_adr = 0x8004;
                            if(num_s>2) ram_adr = 0x8008;
                            if(ReadMem(hDev,ram_adr,R_buffer+num*max_rdat_len,num_s,USB)!=num_s)
                            {
                                printff("Read Err!\t\n");
                                return -1;
                            }
                        }
                        else
                        {
                            return -1;
                        }
                    }
                    for(unsigned int i=0;i<Size;i++)
                    {
                        if(i%16==0)printff("\n %.6x: ",RW_Adr+i);
                        if(i%8==0)printff(" ");
                        printff("%.2x ",R_buffer[i]);
						*(rbuf + i) = R_buffer[i];
                    }
                }
                else if(cmd == WRITE)
                {
                    int ram_adr = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ASK,USB,Type,0,0);
                    //ZeroMemory(W_buffer,4);
                    memset(W_buffer, 0, 4);
                    if(Size>8)
                    {
                        printff(" Write flash size exceed!!\t\n");
                        return -1;
                    }
                    for(unsigned char i=0;i<Size;i++)
                    {
                        W_buffer[0]= (unsigned char)(value>>(8*i));
                        if(WriteMem(hDev,ram_adr,W_buffer,1,USB)!=1)
                        {
                             printff("\n WRITE SRAM ERR! \t\n");
                             return -1;
                        }
                        if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_WRITE,USB,Type,RW_Adr+i,1)!=0)
                        {
                             printff("  Flash Sector (4K) Program at address %x \t\n",RW_Adr+i);
                        }
                        else
                        {
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        case EVK:
        {
            //HANDLE hDev = GetPrintDeviceHandle(0xffff);

            //if(!MCU_Init(hDev,Type,EVK)){return;}

            if(cmd==READ)
            {
                unsigned char max_rdat_len=2;
                //ZeroMemory(R_buffer,R_BUF_SIZE);
                memset(R_buffer, 0, R_BUF_SIZE);
                unsigned int num    = Size/max_rdat_len;
                unsigned char num_s = Size-(num*max_rdat_len);
                for(unsigned int i=0;i<num;i++)
                {
                    if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_READ,EVK,Type,RW_Adr+max_rdat_len*i,max_rdat_len)!=0)
                    {
                        unsigned int ram_adr = 0x8004;
                        if(Type==CHIP_8255||Type==CHIP_8255_A2)  ram_adr = 0x40004;
                        if(max_rdat_len>2)
                        {
                            ram_adr = 0x8008;
                            if(Type==CHIP_8255||Type==CHIP_8255_A2)  ram_adr = 0x40008;
                        }
                        if(ReadMem(hDev,ram_adr,R_buffer+max_rdat_len*i,max_rdat_len,EVK)!=max_rdat_len)
                        {
                            printff("Read Err!\t\n");
                            return -1;
                        }
                    }
                    else
                    {
                        return -1;
                    }
                }
                if(num_s!=0)
                {
                    if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_READ,EVK,Type,RW_Adr+num*max_rdat_len,num_s)!=0)
                    {
                        unsigned int ram_adr = 0x8004;
                        if(Type==CHIP_8255||Type==CHIP_8255_A2)  ram_adr = 0x40004;
                        if(num_s>2)
                        {
                            ram_adr = 0x8008;
                            if(Type==CHIP_8255||Type==CHIP_8255_A2)  ram_adr = 0x40008;
                        }
                        if(ReadMem(hDev,ram_adr,R_buffer+num*max_rdat_len,num_s,EVK)!=num_s)
                        {
                            printff("Read Err!\t\n");
                            return -1;
                        }
                    }
                    else
                    {
                        return -1;
                    }
                }
                for(unsigned int i=0;i<Size;i++)
                {
                    if(i%16==0)printff("\n %.6x: ",RW_Adr+i);
                    if(i%8==0)printff(" ");
                    printff("%.2x ",R_buffer[i]);
                }
            }
            else if(cmd == WRITE)
            {
                int ram_adr = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ASK,EVK,Type,0,0);
                //ZeroMemory(W_buffer,4);
                memset(W_buffer, 0, 4);
                if(Size>8)
                {
                    printff(" Write flash size exceed!!\t\n");
                    return -1;
                }
                for(unsigned char i=0;i<Size;i++)
                {
                    W_buffer[0]= (unsigned char)(value>>(8*i));
                    if(WriteMem(hDev,ram_adr,W_buffer,1,EVK)!=1)
                    {
                         printff("\n WRITE SRAM ERR! \t\n");
                         return -1;
                    }
                    if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_WRITE,EVK,Type,RW_Adr+i,1)!=0)
                    {
                         printff("  Flash Sector (4K) Program at address %x \t\n",RW_Adr+i);
                    }
                    else
                    {
                        return -1;
                    }
                }
            }
#if 0
            char tcdb_exe[]="\\bat\\tcdb.exe";
            memset(CMD_LINE,0,MAX_PATH + 1);
            GetModuleFileName(NULL,CMD_LINE,MAX_PATH);
            (strrchr(CMD_LINE, '\\'))[0] = 0; // É¾³ýÎÄ¼þÃû£¬Ö»»ñµÃÂ·¾¶×Ö´®
            strcat(CMD_LINE,tcdb_exe);
            if(cmd==READ)
            {
                strcat(CMD_LINE," rf ");

                char addr[8];
                memset(addr,0,8);
                itoa(RW_Adr,addr,16);
                strcat(CMD_LINE,addr);

                strcat(CMD_LINE," -s ");

                char Read_Size[8];
                memset(Read_Size,0,8);
                itoa(Size,Read_Size,16);
                strcat(CMD_LINE,Read_Size);
                tcdb_cmd(CMD_LINE);
            }
            else if (cmd==WRITE)
            {
               unsigned char val = 0;
                if(Size>4)
                {
                    printff(" Write flash size exceed!!\t\n");
                    return;
                }
                char tcdb_exe[]="\\bat\\tcdb.exe";
                char addr[8];
                char Wdat[8];
                for(unsigned char i=0;i<Size;i++)
                {
                    memset(CMD_LINE,0,MAX_PATH + 1);
                    GetModuleFileName(NULL,CMD_LINE,MAX_PATH);
                    (strrchr(CMD_LINE, '\\'))[0] = 0; // É¾³ýÎÄ¼þÃû£¬Ö»»ñµÃÂ·¾¶×Ö´®
                    strcat(CMD_LINE,tcdb_exe);
                    strcat(CMD_LINE," wf ");

                    memset(addr,0,8);
                    itoa(RW_Adr+i,addr,16);
                    strcat(CMD_LINE,addr);

                    strcat(CMD_LINE," ");

                    memset(Wdat,0,8);
                    val =(unsigned char)(value>>(8*i));
                    itoa(val,Wdat,16);
                    strcat(CMD_LINE,Wdat);

                    tcdb_cmd(CMD_LINE);
                }
            }
#endif
            break;
        }
        default:break;
    }
	gettimeofday(&end, NULL);
	//printff(" File Download to 0x%.6x: %ld bytes \t\n",adr,size);
	float time_use = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
	time_use /= 1000000;
	printff("\nTotal Time: %.2f s \t\n", time_use);
	return 0;
    //t = (unsigned int)(timeGetTime()-t);
    //printff("\n Total Time: %d ms \t\n",t);
}



#define BIN_BUF_SIZE (1024*1024)
int telink_usb_download(libusb_device_handle *hDev, unsigned int adr, const char *file_path, TL_ChipTypdef Type)
{
	struct timeval start, end;
	
	signed long int size=0;
    unsigned char bin_buffer[BIN_BUF_SIZE]={0};
    
    //if(!MCU_Init(hDev,Type)){return -1;}
    FILE *fp=NULL;
    fp = fopen(file_path,"rb");
    if(NULL==fp){printff( "Fail to open bin file! \t\n");fclose(fp);return -1;}
    fseek(fp,0L,SEEK_END);
    size=ftell(fp);
    fseek(fp,0,SEEK_SET);
    fread(bin_buffer,1,size,fp);
	if(Type == CHIP_8267)
		bin_buffer[8] = 0xff;
	printf("888888888 0x08: 0x%0x\n", bin_buffer[8]);
    fclose(fp);
    #if 1 
	{
		unsigned long int EraseSector_Num = (size%0x1000)? (1+(size/0x1000)):(size/0x1000);
		unsigned long int PageWrite_Num   = (size%0x100)? (1+(size/0x100)):(size/0x100);
		unsigned char Last_Bytes_Num  = size%0x100;
		unsigned int j;
		gettimeofday(&start, NULL);
		#if 1
		for(unsigned int i=0;i<(EraseSector_Num-1);i++)
		{
			if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ERASE,USB,Type,adr+i*0x1000,4)!=0)
			{
				printff(" Flash Sector (4K) Erase at address %x \t\n",adr+i*0x1000);

				int ram_adr = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ASK,USB,Type,0,0);

				for(j=0;j<16;j++)
				{
					
					if((Type==CHIP_8255)||(Type==CHIP_8255_A2))
					{
						#if 0
						if(WriteMem2(hDev,ram_adr,bin_buffer+j*0x100+i*0x1000,256,USB)!=256)
					   {
							 printff("\n USB Download fail! \t\n");
							 return;
						}
						#endif
					}
					else
					{
						if(WriteMem_check(hDev,ram_adr,bin_buffer+j*0x100+i*0x1000,256,USB)!=256)
						{
							 printff("\n USB Download fail! \t\n");
							 return -1;
						}
					}

					if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_WRITE,USB,Type,adr+j*0x100+i*0x1000,256)!=0)
					{
						if(j%4==0) printff(" Flash Page Program at address %x \t\n",adr+j*0x100+i*0x1000);
					}
					else
					{
						printff(" Flash Page Program Error at address %x \t\n",adr+j*0x100+i*0x1000);
					}
				}
			}
			else
			{
				printff(" Flash Sector (4K) Erase Error at address %x \t\n",adr+i*0x1000);
			}
		}

		PageWrite_Num = PageWrite_Num - ((EraseSector_Num-1)*16);

		if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ERASE,USB,Type,adr+(EraseSector_Num-1)*0x1000,4)!=0)
		{
			printff(" Flash Sector (4K) Erase at address %lx \t\n",adr+(EraseSector_Num-1)*0x1000);

			int ram_adr = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ASK,USB,Type,0,0);
			for(unsigned int i=0;i<PageWrite_Num;i++)
			{
				int Page_Bytes = ((i+1)==PageWrite_Num)? ((Last_Bytes_Num==0)? 256:Last_Bytes_Num):256;
				if((Type==CHIP_8255)||(Type==CHIP_8255_A2))
				{
					#if 0
					if(WriteMem2(hDev,ram_adr,bin_buffer+256*i+(EraseSector_Num-1)*0x1000,Page_Bytes,USB)!=Page_Bytes)
					{
						 printff("\n USB Download fail! \t\n");
						 return;
					}
					#endif
				}
				else
				{
					if(WriteMem_check(hDev,ram_adr,bin_buffer+256*i+(EraseSector_Num-1)*0x1000,Page_Bytes,USB)!=Page_Bytes)
					{
						 printff("\n USB Download fail! \t\n");
						 return -1;
					}
				}

				//ToDO
				if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_WRITE,USB,Type,adr+256*i+(EraseSector_Num-1)*0x1000,Page_Bytes)!=0)
				{
					if(i%4==0)printff(" Flash Page Program at address %x \t\n",(unsigned int)(i*256)+(unsigned int)(EraseSector_Num-1)*0x1000+adr);
				}
				else
				{
					printff(" Flash Page Program Error at address %x \t\n",(unsigned int)(i*256)+(unsigned int)(EraseSector_Num-1)*0x1000+adr);
					return -1;
				}
			}
		}
		else
		{
			printff(" Flash Sector (4K) Erase Error at address %lx \t\n",adr+(EraseSector_Num-1)*0x1000);
			return -1;
		}
		#else
		if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ERASE,USB,Type,Adr,EraseSector_Num*4)!=0)
		{
			int ram_adr = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ASK,USB,Type,NULL,NULL);
			for(unsigned int i=0;i<PageWrite_Num;i++)
			{
				int Page_Bytes = ((i+1)==PageWrite_Num)? ((Last_Bytes_Num==0)? 256:Last_Bytes_Num):256;
				if((Type==CHIP_8255)||(Type==CHIP_8255_A2))
				{
					#if 0
					if(WriteMem2(hDev,ram_adr,bin_buffer+256*i,Page_Bytes,USB)!=Page_Bytes)
					{
						 printff("\n USB Download fail! \t\n");
						 return;
					}
					#endif
				}
				else
				{
					if(WriteMem(hDev,ram_adr,bin_buffer+256*i,Page_Bytes,USB)!=Page_Bytes)
					{
						 printff("\n USB Download fail! \t\n");
						 return;
					}
				}
				if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_WRITE,USB,Type,Adr+256*i,Page_Bytes)!=0)
				{
					if((i%16)==0)
					{
						printff(" Flash Sector (4K) Erase & Program at address %x \t\n",Adr+256*i);
					}
					else if((i%4)==0)
					{
						printff(" Flash Sector (4K) Program at address %x \t\n",Adr+256*i);
					}
				}
				else
				{
					if((i%16)==0)
					{
						printff(" Flash Sector (4K) Erase at address %x \t\n",Adr+256*i);
					}
					else if((i%4)==0)
					{
						printff(" Flash Sector Check Error at Addr %x \t\n",Adr+256*i);
					}
				}
			}
		}
		else
		{
			printff(" Erase Flash fail! \t\n");
			return;
		}
		#endif
		gettimeofday(&end, NULL);
		printff(" File Download to 0x%.6x: %ld bytes \t\n",adr,size);
		float time_use = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
		time_use /= 1000000;
		printff(" Total Time: %.2f s \t\n", time_use);
		
	}
#endif
	return 0;
}

//Note: You must erase flash before write it, and the least erase size is 4K
int telink_usb_flash_erase(libusb_device_handle *hDev, TL_ChipTypdef Type, unsigned int RW_Adr)
{
	
	if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_ERASE,USB,Type, RW_Adr,4)!=0)
	{
		printff(" Flash Sector (4K) Erase at address %x \t\n",RW_Adr);
		return 0;
	}else{
		printff(" Flash Sector (4K) Erase Error at address %x \t\n",RW_Adr);	
		return -1;
	}
	
}

/*
 *每次最多读4K大小的FLASH
 * */
int telink_usb_r_flash(libusb_device_handle *hDev, TL_ChipTypdef Type, unsigned int RW_Adr, char *rbuf, unsigned int Size)
{
	//if(!MCU_Init(hDev,Type)){return -2;}
	
	return flash_rw_func(hDev, USB, Type,RW_Adr, rbuf, Size,0,READ);
}

int telink_usb_w_flash(libusb_device_handle *hDev, TL_ChipTypdef Type,unsigned int RW_Adr, unsigned long value, unsigned int Size)
{
	//if(!MCU_Init(hDev,Type)){return -2;}
	
	 return flash_rw_func(hDev, USB, Type, RW_Adr, 0, Size, value, WRITE);
}

#define EP_IN	0x88
#define EP_OUT	5
#define TIME_OUT	1000
#define TIME_OUT_2	2000

int telink_usb_action(libusb_device_handle *hDev, TL_CMDType cmd, unsigned char *data)
{
	unsigned char buf[10] = {0};
	int size = -10;
	int ret = -1;
	switch(cmd)
	{
		case SCAN_ON:
			buf[0] = 0xfa; buf[1]= 0x01; buf[2] = 0x01;
			ret = libusb_bulk_transfer(hDev, EP_OUT, buf, 3, &size, TIME_OUT);
			break;

		case SCAN_OFF:
			buf[0] = 0xfa; buf[1]= 0x01; buf[2] = 0x00;
			ret = libusb_bulk_transfer(hDev, EP_OUT, buf, 3, &size, TIME_OUT);
			break;

		case CONNECT:
			if(data == NULL)
			{
				printf("fatal error!!! NULL pointer!!!\n");
				return -1;
			}
			buf[0] = 0xfa; buf[1] = 0x02;
			memcpy(buf + 2, data, 6);
			ret = libusb_bulk_transfer(hDev, EP_OUT, buf, 8, &size, TIME_OUT);
			break;

		case DISCONNECT:
			buf[0] = 0xfa; buf[1] = 0x03; buf[2] = 0x00;
			ret = libusb_bulk_transfer(hDev, EP_OUT, buf, 3, &size, TIME_OUT);
			break;

		case OTA:
			buf[0] = 0xfa; buf[1] = 0x04; buf[2] = 0x00;
			ret =libusb_bulk_transfer(hDev, EP_OUT, buf, 3, &size, TIME_OUT);
			break;

		case BAT_STATUS:
			buf[0] = 0xfa; buf[1] = 0x05; buf[2] = 0x00;
			ret = libusb_bulk_transfer(hDev, EP_OUT, buf, 3, &size, TIME_OUT);
			break;
	}

	return ret;
}

int telink_usb_get_data(libusb_device_handle *hDev, unsigned char *buf, int len, int *size)
{
	int ret = -1;
	ret = libusb_bulk_transfer(hDev, EP_IN, buf, len, size, TIME_OUT_2);
	
	return ret;
}

void telink_usb_close(libusb_device_handle *m_hDev)
{
	libusb_close(m_hDev); //close the device we opened
	libusb_exit(ctx); //needs to be called to end the
}

int telink_usb_reboot(libusb_device_handle *hDev)
{
#if 1
	if(TL_Dut_cmd_Process(hDev,TL_DUTCMD_USB_REBOOT, USB, CHIP_8266, 0, 0) == 0) //此命令与chip type 无关, cmd:0X57
	{
		printf("reboot failed !\n");
		return -1;
	}


	return 0;
#endif
#if 0
	unsigned char buffer[1]={0};
	buffer[0]=0x84;
	if(WriteMem(hDev,0x602,buffer,1, USB) != 1)
	{
		printff(" Fail to reboot device! \t\n");
		return -1;
	}
	return 0;
#endif
}

int telink_check_flash_crc(libusb_device_handle *hDev, int addr, size_t size)
{
	int flash_crc = TL_Dut_cmd_Process(hDev,TL_DUTCMD_FLASH_CRC, USB, CHIP_8267, addr, size);
	if(flash_crc == 0)
	{
		printf("check flash crc cmd fail !\n");
		return -1;
	}else
		return flash_crc;

}



