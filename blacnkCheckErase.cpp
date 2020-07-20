#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include <spidev.h>
#include <stdint.h>
//#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>

char cmd[50];
int main()
{
for(int i=0;i<256;i++)
{
memset(cmd,'\0',50);
sprintf(cmd,"/root/CRc41Flash/blanckPageCheckAndUpdateToFF64K %d",i);
system(cmd);
}
return 0;
}
