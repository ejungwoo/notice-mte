#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NoticeTIAM.h"

int main(int argc, char *argv[])
{
  int tcp_Handle;               
  char ip[100];
  unsigned long pass;

  if (argc > 1)
    sprintf(ip, "%s", argv[1]);
  else
    sprintf(ip, "192.168.0.2");

  // open TIAM
  tcp_Handle = TIAM_TCPopen(ip);
  
  // show present setting
  pass = TIAM_TCPread_ENABLE_PASS(tcp_Handle);

  if (pass)
    printf("LTE link PASS is enabled.\n");
  else
    printf("LTE link PASS is disabled.\n");

  printf("Change setting(0 = disable, 1 = enable) : ");
  scanf("%ld", &pass);
  
  // change setting
  TIAM_TCPwrite_ENABLE_PASS(tcp_Handle, pass);  
  
  // show changed setting
  pass = TIAM_TCPread_ENABLE_PASS(tcp_Handle);

  if (pass)
    printf("LTE link PASS is enabled.\n");
  else
    printf("LTE link PASS is disabled.\n");

  // close TIAM
  TIAM_TCPclose(tcp_Handle);

  return 0;
}

