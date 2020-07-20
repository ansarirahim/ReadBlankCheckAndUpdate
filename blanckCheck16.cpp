#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "iosfwd"
#include <iostream>
#include <iostream>
#include "SimpleGPIO_SPI.h"
#include "SPI_SS_Def.H"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include <spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
using std::cout;
using std::endl;
bool SectorNoHavingNData[128];
bool SectorNoHavingData[128];
unsigned char hex2byte(const char *hex)
{
    unsigned short byte = 0;
    std::istringstream iss(hex);
    iss >> std::hex >> byte;
    return byte % 0x100;
}
char myBytes[2];
#define CLK_PER_SEC_ACTUAL (99000)


static void AllFlashChipsSelect(PIN_VALUE x)
{
	gpio_set_value_spi(SS2, x);
	gpio_set_value_spi(SS0, x);
	gpio_set_value_spi(SS4, x);
	gpio_set_value_spi(SS6, x);
}
void msleep(int ms)
{
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
}
int SearchForData(uint8_t *buff, long size)
{
	for (long i = 0; i < size; i++)
	{
		if (buff[i] < 0xff)
			return 0;//true;
	}
	return 1;
}

#define FLASH_CERS 0xC7
#define FLASH_SE 0xD8
//definitions for AT25512 device
#define WRITE_CYCLE_TIME (5000) //AT25512 write cycle time in us
#define WRSR (0x01)				//AT25512 write status register
#define WRITE (0x02)			//AT25512 write data to memory array
#define READ (0x03)				//AT25512 read data from memory array
#define WRDI (0x04)				//AT25512 reset write enable latch
#define RDSR (0x05)				//AT25512 read status register
#define WREN (0x06)				//AT25512 set write enable latch
static void pabort(const char *s)
{
	perror(s);
	abort();
}
typedef unsigned char tByte;
tByte FlashReadCmd[4];
tByte NVM16ReadCmd[3];
//tByte TempTxData128[128];
tByte Rxd256[256];
tByte Rxd[2048];
struct spi_ioc_transfer messageRead[2], message16Read[2], message16Read256[2];
struct spi_ioc_transfer messageWREN[1]; // = {0, };
tByte WriteEnableCmd[1];
struct spi_ioc_transfer messageData[1];
struct spi_ioc_transfer messageWriteCmd[1] = {
	0,
};
tByte WriteCmd[4];
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 10000000; //need to be speeded up when working
static uint16_t delay = 5;
//changed to AT25512 write cycle time
unsigned int Spi_address = 0;
tByte TempTxData128[128];
tByte TempTxNvmData128[128];
tByte Write16Cmd[3];
tByte TempByteBuffer[256];
char TemCharBuffer[256];
struct spi_ioc_transfer messageWrite16CmdFF[2] = {
	0,
};
struct spi_ioc_transfer messageWrite16CmdData[2] = {
	0,
};
tByte ReadFCS[4];
static void Init_16messageRead256()
{
	message16Read256[0].tx_buf = (unsigned long)NVM16ReadCmd;
	message16Read256[0].rx_buf = (unsigned long)NULL;
	message16Read256[0].len = sizeof(NVM16ReadCmd);
	message16Read256[0].cs_change = 0;//0 working for P9_17

	message16Read256[1].tx_buf = (unsigned long)NULL;
	message16Read256[1].rx_buf = (unsigned long)Rxd256;
	message16Read256[1].len = sizeof(Rxd256);
	message16Read256[1].cs_change =0;// 1;

	NVM16ReadCmd[0] = READ;
	//		    	   (readcommand+1)= &(Spi_address>>16);//
	//		    	  	    		  (readcommand+2) =&(Spi_address>>8);//
	//		    	  	    		  (readcommand+3) =&(Spi_address);
}
///////////////////////////////
static void Init_messageRead()
{
	messageRead[0].tx_buf = (unsigned long)FlashReadCmd;
	messageRead[0].rx_buf = (unsigned long)NULL;
	messageRead[0].len = sizeof(FlashReadCmd);
	messageRead[0].cs_change = 0;//0 working for P9_17

	messageRead[1].tx_buf = (unsigned long)NULL;
	messageRead[1].rx_buf = (unsigned long)Rxd256; ///Rxd
	messageRead[1].len = sizeof(Rxd256);
	messageRead[1].cs_change = 1;

	FlashReadCmd[0] = READ;
	//		    	   (readcommand+1)= &(Spi_address>>16);//
	//		    	  	    		  (readcommand+2) =&(Spi_address>>8);//
	//		    	  	    		  (readcommand+3) =&(Spi_address);
}

static void Init_16messageRead()
{
	message16Read[0].tx_buf = (unsigned long)NVM16ReadCmd;
	message16Read[0].rx_buf = (unsigned long)NULL;
	message16Read[0].len = sizeof(NVM16ReadCmd);
	message16Read[0].cs_change = 0;//0 working for P9_17

	message16Read[1].tx_buf = (unsigned long)NULL;
	message16Read[1].rx_buf = (unsigned long)Rxd;
	message16Read[1].len = sizeof(Rxd);
	message16Read[1].cs_change = 1;

	NVM16ReadCmd[0] = READ;
	//		    	   (readcommand+1)= &(Spi_address>>16);//
	//		    	  	    		  (readcommand+2) =&(Spi_address>>8);//
	//		    	  	    		  (readcommand+3) =&(Spi_address);
}
////////////////////////
static void Init_messageWREN()
{

	//setup spi transfer data structure

	messageWREN[0].tx_buf = (unsigned long)WriteEnableCmd; //send the write enable command
	messageWREN[0].rx_buf = (unsigned long)NULL;
	messageWREN[0].len = 1; // sizeof(WriteEnableCmd);
	messageWREN[0].cs_change = 0;//1;
}

static void Init_WriteCommand()
{
	//     //setup spi transfer data structure

	messageWriteCmd[0].tx_buf = (unsigned long)WriteCmd; //send the write command and address
	messageWriteCmd[0].rx_buf = (unsigned long)NULL;
	messageWriteCmd[0].len = sizeof(WriteCmd);
	messageWriteCmd[0].cs_change = 0;//1;
}

static void Init_WriteCommand16FF()
{
	//     //setup spi transfer data structure
	Write16Cmd[0] = WRITE;
	messageWrite16CmdFF[0].tx_buf = (unsigned long)Write16Cmd; //send the write command and address
	messageWrite16CmdFF[0].rx_buf = (unsigned long)NULL;
	messageWrite16CmdFF[0].len = sizeof(Write16Cmd);
	messageWrite16CmdFF[0].cs_change =0;// 1;

	messageWrite16CmdFF[1].tx_buf = (unsigned long)TempTxData128; //send the write command and address
	messageWrite16CmdFF[1].rx_buf = (unsigned long)NULL;
	messageWrite16CmdFF[1].len = sizeof(TempTxData128);
	messageWrite16CmdFF[1].cs_change = 0;//1;
}
static void Init_WriteCommand16Data()
{
	//     //setup spi transfer data structure
	Write16Cmd[0] = WRITE;
	messageWrite16CmdData[0].tx_buf = (unsigned long)Write16Cmd; //send the write command and address
	messageWrite16CmdData[0].rx_buf = (unsigned long)NULL;
	messageWrite16CmdData[0].len = sizeof(Write16Cmd);
	messageWrite16CmdData[0].cs_change = 0;//1;

	messageWrite16CmdData[1].tx_buf = (unsigned long)TempTxNvmData128; //send the write command and address
	messageWrite16CmdData[1].rx_buf = (unsigned long)NULL;
	messageWrite16CmdData[1].len = sizeof(TempTxNvmData128);
	messageWrite16CmdData[1].cs_change =0;// 1;
}
struct spi_ioc_transfer messageWrite16Cmd[1] = {
	0,
};
static void Init_Write16Command()
{
	//     //setup spi transfer data structure

	messageWrite16Cmd[0].tx_buf = (unsigned long)WriteCmd; //send the write command and address
	messageWrite16Cmd[0].rx_buf = (unsigned long)NULL;
	messageWrite16Cmd[0].len = sizeof(Write16Cmd);
	messageWrite16Cmd[0].cs_change = 0;//1;
}
char ReadStatusRegCmd[1] = {
	RDSR,
};
uint8_t StatusRegValues[2];
struct spi_ioc_transfer messageStatusReg[2]; // = {0, };
static void Init_ReadStatusReg()
{

	messageStatusReg[0].tx_buf = (unsigned long)ReadStatusRegCmd;
	messageStatusReg[0].rx_buf = (unsigned long)NULL;
	messageStatusReg[0].len = sizeof(ReadStatusRegCmd);
	messageStatusReg[0].cs_change = 0;//1;

	messageStatusReg[1].tx_buf = (unsigned long)NULL;
	messageStatusReg[1].rx_buf = (unsigned long)StatusRegValues;
	messageStatusReg[1].len = sizeof(StatusRegValues);
	messageStatusReg[1].cs_change = 0;//1;
}
const char *DIRECTION_SET_GPIO48 = "echo high > /sys/class/gpio/gpio48/direction";
const char *DIRECTION_SET_GPIO50 = "echo high > /sys/class/gpio/gpio50/direction";
const char *DIRECTION_SET_GPIO51 = "echo high > /sys/class/gpio/gpio51/direction";
const char *DIRECTION_SET_GPIO60 = "echo high > /sys/class/gpio/gpio60/direction";
const char *DIRECTION_SET_GPIO115 = "echo high > /sys/class/gpio/gpio115/direction";
const char *DIRECTION_SET_GPIO49 = "echo high > /sys/class/gpio/gpio49/direction"; //qhc

const char *Export_GPIO48 = "echo 48 > /sys/class/gpio/export";
const char *Export_GPIO50 = " echo 50 > /sys/class/gpio/export";
const char *Export_GPIO51 = "echo 51 > /sys/class/gpio/export";
const char *Export_GPIO60 = "echo 60 > /sys/class/gpio/export";

const char *DIRECTION_CLEAR_GPIO48 = "echo low > /sys/class/gpio/gpio48/direction";
const char *DIRECTION_CLEAR_GPIO50 = "echo low > /sys/class/gpio/gpio50/direction";
const char *DIRECTION_CLEAR_GPIO51 = "echo low > /sys/class/gpio/gpio51/direction";
const char *DIRECTION_CLEAR_GPIO60 = "echo low > /sys/class/gpio/gpio60/direction";
const char *DIRECTION_CLEAR_GPIO115 = "echo low > /sys/class/gpio/gpio115/direction";
const char *DIRECTION_CLEAR_GPIO49 = "echo low > /sys/class/gpio/gpio49/direction"; //qhc
////////////////////////////////////////////////////

const char *Clear_GPIO48 = "echo 0 > /sys/class/gpio/gpio48/value";
const char *Clear_GPIO50 = "echo 0 > /sys/class/gpio/gpio50/value";
const char *Clear_GPIO51 = "echo 0 > /sys/class/gpio/gpio51/value";
const char *Clear_GPIO60 = "echo 0 > /sys/class/gpio/gpio60/value";

const char *Set_GPIO48 = "echo 1 > /sys/class/gpio/gpio48/value";
const char *Set_GPIO50 = "echo 1 > /sys/class/gpio/gpio50/value";
const char *Set_GPIO51 = "echo 1 > /sys/class/gpio/gpio51/value";
const char *Set_GPIO60 = "echo 1 > /sys/class/gpio/gpio60/value";
#include <iostream>
#include <string>
using namespace std;

typedef unsigned int tShort;
typedef unsigned char tByte;
/////////

int aclem, HowManyFpages;
int SizeofMemory[2];
#define FLASH_MEMORY 0

int  addresshigh,addressmid,addresslow;

int fromIndex,toIndex;
int dataFound=0;
int blank=0;
int main(int argc, char *argv[])

{
	int ret = 0;
	int fd;
addressmid=atoi(argv[1]);
	//////////////////
	fd = open("/dev/spidev1.0", O_RDWR); //open the spi device
	if (fd < 0)
		pabort("can't open device");

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode); //set the spi mode
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode); //get the spi mode (test)
	if (ret == -1)
		pabort("can't get spi mode");

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits); //set the spi bits per word
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits); //get the spi bits per word (test)
	if (ret == -1)
		pabort("can't get bits per word");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed); //set the spi max speed
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed); //get the spi max speed (test)
	if (ret == -1)
		pabort("can't get max speed hz");

	speed = 10000;
	tByte yeed = 80;

//	for ( addressmid=0;addressmid<0x100;addressmid++)
	{
addresslow=0;
///printf("\nPage=%.2X %.2x",addressmid,addresslow);
//addresslow=0;
		{
						        		 //printf("\nAdressH=0x%.2X AdressM=X%.2X AdressL%.2X, Adress=%ld", addresshigh,addressmid,addresslow,Spi_address );
			char readcommand[3] = {
				READ,
			//	addresshigh,
				(char)addressmid,
				(char)addresslow,
			};
			uint8_t rx[256] = {
				0,
			}; //create an array of data to be received
			struct spi_ioc_transfer message[2] = {
				0,
			}; //setup spi transfer data structure
			message[0].tx_buf = (unsigned long)readcommand;
			message[0].rx_buf = (unsigned long)NULL;
			message[0].len = sizeof(readcommand);
			message[0].cs_change = 0;//0 working for P9_17
			message[1].tx_buf = (unsigned long)NULL;
			message[1].rx_buf = (unsigned long)rx;
			message[1].len = sizeof(rx);
			message[1].cs_change = 0;
//			gpio_set_value_spi(Css, LOW);
///			usleep(5000);
			ret = ioctl(fd, SPI_IOC_MESSAGE(2), &message); //spi check if sent
///			gpio_set_value_spi(Css, HIGH);

			if (ret < 1)
				pabort("can't send spi message");
			

blank=SearchForData(rx, 256);//)
		}
	}
	close(fd);
	////////////////////////////
///printf("\nblank=%d\n",blank);
	return blank;
}
