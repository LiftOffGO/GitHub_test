
#include <iostream>
#include <math.h>
#include "MyUDP.h"
using namespace std;

int main()
{
  TUDP udp;
  char send_buf[50];
  int i = 0;

  while (1)
  {

    sprintf(send_buf, "receive  num: %d\n", i); //this is a test
    udp.AddSendStr(send_buf);
    udp.SendBuf();
    if (udp.rec_flag == 1)
    {
      cout << udp.receive_buf << endl;
      udp.rec_flag = 0;
    }

    sleep(1);
    i++;
    if (i > 1000)
    {
      i = 0;
    }
  }
  return 0;
}
