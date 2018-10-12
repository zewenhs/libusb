#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "usbprn.h"
#include <libusb-1.0/libusb.h>
#include "config.h"

#include "telink_usb.h"

//#define DEBUG
//libusb_context *ctx = NULL; //a libusb session
extern libusb_context *ctx;
extern libusb_device_handle *m_hDev;

int LogMsg (const char * sz,...) {
    char ach[1024 * 32];
    int n = sprintf(ach, sz, (&sz+1));   /* Format the string */
    printf ("%s\n", ach);
    return n;
}

// Change these two constants to match your device's idVendor and idProduct.
// Or, just pass your idVendor and idProduct as command line arguments when running this sample.
#define kMyVendorID			0x248a
//#define kMyProductID		0x881d
#define kMyProductID		0x5320

int WriteToDevice(libusb_device_handle *hd, uint16_t deviceAddress, uint16_t length, uint8_t *writeBuffer) {
	int r;
	unsigned int i;

	uint8_t type = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE;
	unsigned char buff[4096];

	uint8_t	bmRequestType = type;
	uint8_t	bRequest = 0x02;
	uint16_t wValue = deviceAddress;
	uint16_t wIndex = 0x00;
	uint16_t wLength = length + 8;
	unsigned int timeout = 100;
	unsigned char *	data = buff;

	buff[0] = 0x02;
	buff[1] = deviceAddress >> 8;
	buff[2] = deviceAddress;
	buff[3] = buff[4] = buff[5] = buff[6] = buff[7] = writeBuffer[0]; // !! Question: ???
	memcpy (buff + 8, writeBuffer, length);


#ifdef DEBUG
    printf("length is %d\n", length);
#endif
    //data[length] = '\0';

    // returns: on success, the number of bytes actually transferred
    // Ref: http://libusb.sourceforge.net/api-1.0/group__syncio.html#gadb11f7a761bd12fc77a07f4568d56f38
	r = libusb_control_transfer(hd, bmRequestType, bRequest, wValue, wIndex, data, wLength, timeout);
	if (r < 0) {
		fprintf(stderr, "write to endpoint0 error %d\n", r);
		printf("error name:%s\n", libusb_error_name(r));
		printf("error msg:%s\n", libusb_strerror(r));
		return r;
	}
	//printf("r=%d sizeof(data):%d\n", r, sizeof(data));
	//if ((unsigned int) r < sizeof(data)) {
	if ((unsigned int) r < wLength) {
#if 1
		fprintf(stderr, "short write (%d)\n", r);
#endif
		return -1;
	}

#ifdef DEBUG
	printf("written data:");
	for (i = 0; i < length; i++)
		printf("%02x ", writeBuffer[i]);
	printf("\n");
#endif

	//printf("transter data len:%d\n", r);
    return r>=0 ? r - 8 : -1;
}

int ReadFromDevice(libusb_device_handle *hd, uint16_t deviceAddress, uint16_t length, uint8_t *writeBuffer, int *nr)
{
	int r;
	unsigned int i;

	uint8_t type = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE;
	uint8_t	bmRequestType = type;
	uint8_t	bRequest = 0x02;
	uint16_t wValue = deviceAddress;
	uint16_t wIndex = 0;
	unsigned char *	data = writeBuffer;
	uint16_t wLength = length + 8;
	unsigned int timeout = 100;

	r = libusb_control_transfer(hd, bmRequestType, bRequest, wValue, wIndex, data, wLength, timeout);
	if (r < 0) {
		fprintf(stderr, "F0 error %d\n", r);
		printf("error name:%s\n", libusb_error_name(r));
		printf("error msg:%s\n", libusb_strerror(r));
		return r;
	}
	if ((unsigned int) r < sizeof(data)) {
#ifdef DEBUG
		fprintf(stderr, "short read (%d)\n", r);
#endif
		//return -1;
	}

#ifdef DEBUG
	printf("Readed data %d bytes:", r);
	for (i = 0; i < length; i++) printf("%02x ", data[i]);
	printf("\n");
#endif

    if (nr) {
        *nr = r;
    }
    return r;
}


//================================================================================================
//
//	SignalHandler
//
//	This routine will get called when we interrupt the program (usually with a Ctrl-C from the
//	command line).
//
//================================================================================================
void SignalHandler(int sigraised) {
    fprintf(stderr, "\nInterrupted.\n");

    // Todo: release resource
    //libusb_close
	libusb_close(m_hDev); //close the device we opened
	libusb_exit(ctx); //needs to be called to end the

    exit((int)0);
}

//================================================================================================
//	main
//================================================================================================

libusb_device_handle  *telink_usb_open(unsigned int vid, unsigned int pid)
{
    unsigned int					usbVendor = vid;
    unsigned int					usbProduct = pid;
    sig_t					oldHandler;

#if 0
    // pick up command line arguments
    if (argc > 1) {
        usbVendor = atoi(argv[1]);
        printf("VID:%08lx\t", usbVendor);
    }
    if (argc > 2) {
        usbProduct = atoi(argv[2]);
        printf("PID:%08lx\n", usbProduct);
    }
#endif

    // Set up a signal handler so we can clean up when we're interrupted from the command line
    // Otherwise we stay in our run loop forever.
    oldHandler = signal(SIGINT, SignalHandler);
    if (oldHandler == SIG_ERR) {
        fprintf(stderr, "Could not establish new signal handler.");
    }

#ifdef DEBUG
    fprintf(stderr, "Looking for devices matching vendor ID=%04x and product ID=%04x.\n", usbVendor, usbProduct);
#endif

	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_device *dev;
	libusb_device_handle *dev_handle; //a device handle
	int r; //for return values
	int cnt; //holding number of devices in list

	r = libusb_init(&ctx); //initialize the library for the session we just declared
	if(r < 0) {
		perror("Init Error\n"); //there was an error
		return NULL;
	}
	//libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_INFO); //set verbosity level to 3, as suggested in the documentation
	libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_NONE); //set verbosity level to 3, as suggested in the documentation

	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if(cnt < 0) {
		perror("Get Device Error\n"); //there was an error
		return NULL;
	}

    // Find the pid with vid
    int i = 0;
    int found = -1;
    struct libusb_device_descriptor desc;
    int ret_des;

    //perror("1111\n");

    libusb_device *ins_device = NULL;

	while ((ins_device = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		ret_des = libusb_get_device_descriptor(ins_device, &desc);
        //printf("--------------vid:%x pid:%x\n", desc.idVendor, libusb_le16_to_cpu(desc.idProduct));
        if(desc.idVendor == usbVendor) {
            //found = 1;
            if(usbProduct == libusb_le16_to_cpu(desc.idProduct))
			{
				found = 1;
				printf("found telink vendorid:0x%x, productID:0x%x\n", usbVendor, usbProduct);
            	break;
			}
            //break;
        }
	}

    //for (i = 0; i < cnt; i++) {
    //    ins_device = devs[i];
    //    ret_des = libusb_get_device_descriptor(ins_device, &desc);
    //    if(desc.idVendor == usbVendor) {
    //        found = 1;
    //        usbProduct = libusb_le16_to_cpu(desc.idProduct);
    //        break;
    //    }
    //}
    if (-1 == found) {
		printf("Cannot find  telink %x:%x device\n", usbVendor, usbProduct);
        return NULL;
    } else {
#ifdef DEBUG
		printf("find device 0x%08x:0x%08x\n", usbVendor, usbProduct);
#endif
    }

#ifdef DEBUG
	printf("%d Devices in list.\n", cnt);
#endif
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it

	dev_handle = libusb_open_device_with_vid_pid(ctx, usbVendor, usbProduct);
    if(dev_handle == NULL) {
		perror("Cannot open device\n");
        return NULL;
    }
    else {
#ifdef DEBUG
		printf("Device Opened\n");
#endif
    }
    dev = libusb_get_device(dev_handle);

#ifdef DEBUG
    printf("\tdevice speed: %d, device addr: %d\n",libusb_get_device_speed(dev), libusb_get_device_address(dev));
#endif


	if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
		printf("Kernel Driver Active\n");
		if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
			printf("Kernel Driver Detached!\n");
	}

	r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
	if(r < 0) {
		perror("Cannot Claim Interface\n");
		return NULL;
	}
    //Now, device is ready
#ifdef DEBUG
	printf("Claimed Interface\n");
#endif

#ifdef DEBUG
    //uint32_t dr = 0x1122;
    uint32_t dr = 0xaabb1234;
    //short  dr = 0x1234;
    //printf("sizeof uint32_t =%ld\n", sizeof(uint32_t));
    WriteToDevice(dev_handle, 0x8400, 4, (UInt8*)&dr);
    //int WriteMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type)
    //r = WriteMem(dev_handle, 0, (uint8_t *)&dr, 2, 2);
    dr = 0;
    //ReadFromDevice (dev_handle, 0, 4, (UInt8*)&dr, 0);
    ReadMem(dev_handle, 0x8400, (uint8_t *)&dr, 4, 2);
   // ReadMem(dev_handle, 0x8002, (uint8_t *)&dr, 2, 2);
    printf("USB Read: %x.\n", dr);
#endif

    m_hDev = dev_handle;

    //return pcDev ? 1 : 0;
    //return r ? 1 : 0;
    return dev_handle;
}

int telink_usb_w_mem(libusb_device_handle *hDev, uint16_t addr, uint8_t *buf, uint16_t len)
{
 	return  WriteToDevice(hDev, addr, len, buf);
}

int telink_usb_r_mem(libusb_device_handle *hDev, uint16_t addr, uint8_t *buf, uint16_t len)
{
	return ReadMem(hDev, addr, buf, len, USB);
}

int IsEVKDevice (libusb_device_handle *h)
{
    DWORD id;
    int ret = ReadUSBMem(h, 0x7c, (LPBYTE) &id, 4, 0, 1024);
    if ((id & 0xffff0000) == 0x08180000)
        return 4;
    else
        return 0;
}

unsigned int GetTickCount (void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (unsigned int)(tv.tv_sec*1000 + (tv.tv_usec / 1000));
}

int EVKCommand (libusb_device_handle *hdev, int addr, int len, int cmd, LPBYTE lpB, int * ptick)
{
    int cmdbuf[16];
    cmdbuf[0] = addr;
    cmdbuf[1] = (len & 0xffff) | (cmd<<16);
    int ret = WriteUSBMem(hdev, 0x9ff8, (LPBYTE) cmdbuf, 8, 0, 1024);
    int rlen = len;
    //if ((cmd & 0x3f00) == UCMD_FLASH) rlen = 1;

    if (!ret) return 0;

    unsigned int ts = GetTickCount ();
    int tl = 1000;
    //WaitForSingleObject (NULLEVENT2, 5);
    do {
        //for (int l=0; l<tl; l++) {
        ret = ReadUSBMemCheck(hdev, 0x9ff0, (LPBYTE) cmdbuf, 16, 0, 1024);
        if (cmdbuf[0]&0xf0000000) {
            return 0;
        }
        else if (cmdbuf[0] == rlen) {
            if (ptick) *ptick = cmdbuf[1];
            return len;
        }
        else if (cmdbuf[0]) {
            //int t = l;
        }
        //WaitForSingleObject (NULLEVENT2, 1);
    } while (GetTickCount() - ts < tl);
    return 0;
}

int WriteUSBMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen)
{
    //int bRet = ReadFromDevice (pcDev, 0, 4, (UInt8*)&dr);
    int bRet = WriteToDevice (hdev, addr, len, (unsigned char*)lpB);
    //just for debug
    DEBUG_OUT("####debug:addr = %x, len = %x, lpB[0] = %x",addr,len,lpB[0]);
    
    if (bRet < 0)
        printf("error: USB tansfer have error\n");
    
     return bRet;
}

int ReadUSBMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen)
{
    //HANDLE hh = GetPrintDeviceHandle ();
    int nr;
    int bRet = ReadFromDevice (hdev, addr, len, (unsigned char*)lpB, &nr);

    if (!bRet)
        return 0;
    else
        return len;
}

int ReadUSBMemCheck(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen)
{

    int	nBytes;

    int nR = 8 + (len > maxlen ? maxlen :  len<1 ? 1 : len);

    int retry = (nR + 8) / 4;
    int bRet;

    while (retry>0 && nR>0)
    {
        retry --;
        //HANDLE hh = GetPrintDeviceHandle ();      //????? nR
        DEBUG_OUT("usb read addr = %x, len = %d",addr,len);
        bRet = ReadFromDevice (hdev, addr, len, (UInt8*)lpB, &nBytes);

        if (!bRet)
            return 0;
        else if (nBytes == nR - 8)
            return len;
        nR -= nBytes;
        lpB += nBytes;
        addr += nBytes;
    }
    return 0;
}

int WriteUSBMemCheck(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int fifo, int maxlen)
{
    BYTE rbuff[4096];
    int nc = 10;
    int l = 0;
    int retw, retr;

    while (l++<nc) {
        retw = WriteUSBMem (hdev, addr, lpB, len, fifo, maxlen);
        int ok = 1;
        retr = ReadUSBMemCheck  (hdev, addr, rbuff, len, fifo, maxlen);
        //retr = ReadUSBMem  (hdev, addr, rbuff, len, fifo, maxlen);
        for (int k=0; k<len; k++) {
            if (lpB[k] != rbuff[k]) {
				printf("ram write and read check failed: lpB[%d]=%x rbuff[%d]=%x\n", k,lpB[k],k, rbuff[k]);
                ok = 0;
                LogMsg ("USB write adrress %x", addr + k);
                break;
            }
        }
        //return retw;
        //return len;
        if (ok)
            return retw;
        else
            ok = ok;
    }
    return 0;
}

#define		UCMD_CORE		0x0100
#define		UCMD_FRD		0x8000
#define		UCMD_FSWIRE		0x4000

int WriteMem_check(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type)
{
	//DEBUG_OUT("adr = %x, len=%d",addr,len);
	//printf("adr = %x, len=%d\n",addr,len);
    BYTE ah = addr>>16;
    //WriteI2CMem (hdev, 0xffff, &ah, 1, 1);
    int ret = 0;

    int al = addr & 0xffff;
    int step = 1024;
    //int step = 3072;

    for (int i=0; i<len; i+=step) {
        int n = len - i > step ? step : len - i;
        int fadr = al + i;
        if (type & 0x200) fadr = al;
        //ret += WriteUSBMem (hdev, fadr, lpB+i, n, type & 0x200, step);
        int rw;
		
		//printf("fadr:%x n:%d i:%d type&0x200:%d, step:%d\n", fadr, n, i, type&0x200, step); 
        if ( (type & 15) != 4)
            rw = WriteUSBMemCheck (hdev, fadr, lpB+i, n, type & 0x200, step);//此处n即为要写入的字节大小, fadr 为写入的ram地址, lpB+i 为写入数据buf的起始地址， 且此for循环只会执行一次
        else {
            rw = WriteUSBMem (hdev, 0xa000, lpB+i, n, type & 0x200, step);
            if (rw == n)
                rw = EVKCommand (hdev, fadr, n, UCMD_CORE|UCMD_FSWIRE, lpB+i, NULL);
            else
                return ret;
        }
        if (rw)
            ret += rw;
        else
            break;
    }
    return ret;
}


int WriteMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type)
{
	//DEBUG_OUT("adr = %x, len=%d",addr,len);
	//printf("adr = %x, len=%d\n",addr,len);
    BYTE ah = addr>>16;
    //WriteI2CMem (hdev, 0xffff, &ah, 1, 1);
    int ret = 0;

    int al = addr & 0xffff;
    int step = 1024;
    //int step = 3072;

    for (int i=0; i<len; i+=step) {
        int n = len - i > step ? step : len - i;
        int fadr = al + i;
        if (type & 0x200) fadr = al;
        //ret += WriteUSBMem (hdev, fadr, lpB+i, n, type & 0x200, step);
        int rw;
		
		//printf("fadr:%x n:%d i:%d type&0x200:%d, step:%d\n", fadr, n, i, type&0x200, step); 
        if ( (type & 15) != 4)
            rw = WriteUSBMem (hdev, fadr, lpB+i, n, type & 0x200, step);//此处n即为要写入的字节大小, fadr 为写入的ram地址, lpB+i 为写入数据buf的起始地址， 且此for循环只会执行一次
        else {
            rw = WriteUSBMem (hdev, 0xa000, lpB+i, n, type & 0x200, step);
            if (rw == n)
                rw = EVKCommand (hdev, fadr, n, UCMD_CORE|UCMD_FSWIRE, lpB+i, NULL);
            else
                return ret;
        }
        if (rw)
            ret += rw;
        else
            break;
    }
    return ret;
}

int ReadMem(libusb_device_handle *hdev, int addr, LPBYTE lpB, int len, int type)
{

    BYTE ah = addr>>16;
    //WriteI2CMem (hdev, 0xff08, &ah, 1, 1);
    int ret = 0;

    int al = addr & 0xffff;
    //int step = 32;
    int step = 3072;

    for (int i=0; i<len; i+=step) {
        int n = len - i > step ? step : len - i;
        int fadr = al + i;
        if (type & 0x200) fadr = al;
        if ( (type & 15) != 4) {
            //ret += ReadUSBMem (hdev, fadr, lpB+i, n, type & 0x200, step);
            ret += ReadUSBMemCheck (hdev, fadr, lpB+i, n, type & 0x200, step);
        }
        else {
            if (EVKCommand (hdev, fadr, n, UCMD_CORE|UCMD_FSWIRE|UCMD_FRD, lpB+i, NULL) == n)
            {
                //ret += ReadUSBMem (hdev, 0xa000, lpB+i, n, type & 0x200, step);
                ret += ReadUSBMemCheck (hdev, 0xa000, lpB+i, n, type & 0x200, step);
            }
            else
                return ret;
        }
    }
    //return ret;
    return len;
}

int ReadUartMem(libusb_device_handle *hdev, LPBYTE lpB)
{

    DWORD	nB;
    ////////  UART replacement pending //////////////
    //int ret = ReadFile(hdev, lpB, 2048, &nB, NULL);
    int ret = 0;
    if (ret==0)
        return -1;
    else
        return nB;

}

int WriteUSBReg(libusb_device_handle *hdev, int addr, int dat)
{

    BYTE buff[16];
    *((int *) buff) = dat;

    int bRet = WriteUSBMem(hdev, addr, buff, 1, 0, 32);
    return bRet;
}

int ReadUSBReg(libusb_device_handle *hdev, int addr, int dat)
{

    BYTE buff[16];
    int bRet = ReadUSBMem(hdev, addr, buff, 1, 0, 32);
    dat = buff[0];
    return bRet;
}
