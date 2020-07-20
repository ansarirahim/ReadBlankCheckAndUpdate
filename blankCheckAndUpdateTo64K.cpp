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


//int  addresshigh,
int  addressmid;//,addresslow;
int blank=0;
char cmd[50];
char cmd2[50];
int main(int argc, char *argv[])

{
//	int ret = 0;
;;//	int fd;
//addressmid=atoi(argv[1]);
for(int  i=0;i<256;i++)
{

//blanckPageCheckAndUpdateToFF64K
sprintf(cmd,"/root/CRc41Flash/blanckCheck16 %d",i);
printf("%s\n\n",cmd);
if(system(cmd)==0)
{
sprintf(cmd2,"/root/CRc41Flash/blanckPageCheckAndUpdateToFF64K %d");
system(cmd2);
printf("CMD2=%s\n\n",cmd2);
//printf("\n Not blank\n");	//////////////////
}
//else
//printf("\n it is blank\n");
}
}
