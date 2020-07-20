#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "iosfwd"
#include <iostream>
#include <iostream>
//#include "SimpleGPIO_SPI.h"
//#include "SPI_SS_Def.H"
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
#include <iostream>
#include <string>
using namespace std;

typedef unsigned int tShort;
typedef unsigned char tByte;
/////////


int  addresshigh,addressmid,addresslow;
int blank=0;
char cmd[50];
int main(int argc, char *argv[])

{
	int ret = 0;
	int fd;
addressmid=atoi(argv[1]);

sprintf(cmd,"/root/CRc41Flash/blanckCheck16 %d",addressmid);
printf("%s\n",cmd);
if(system(cmd)==0)
{
printf("\n Not blank\n");	//////////////////
}
else
printf("\n it is blank\n");
}
