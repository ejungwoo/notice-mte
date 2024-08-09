#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NoticeTIAM.h"

int main(int argc, char *argv[])
{
  int tcp_Handle;               
  char ip[100];
  char filename[256];
  FILE *fp;
  unsigned long run;
  unsigned long run_number;
  unsigned long data_size;
  unsigned long data_taken;
  char *data;

  if (argc > 1)
    sprintf(ip, "%s", argv[1]);
  else
    sprintf(ip, "192.168.0.2");

  // set filename
  sprintf(filename, "tiam.dat");

  // assign data array, max = 32 kbyte
  data = (char *)malloc(32 * 1024); 
  
  // open TIAM
  tcp_Handle = TIAM_TCPopen(ip);

  // open data file
  fp = fopen(filename, "wb");

  // wait for LTE runs
  printf("Wait for LTE is running...\n");
  run = TIAM_TCPread_RUN(tcp_Handle);
  while(!run)
    run = TIAM_TCPread_RUN(tcp_Handle);

  run_number = TIAM_TCPread_RUNNO(tcp_Handle);
  printf("Start to take data, run # = %ld\n", run_number);

  run = TIAM_TCPread_RUN(tcp_Handle); 
  
  data_taken = 0;
  
  while (run) {
    // read data size in byte
    data_size = TIAM_TCPread_DATA_SIZE(tcp_Handle);
    
    if (data_size) {
printf("data size = %ld\n", data_size);    
      // read data
      TIAM_TCPread_DATA(tcp_Handle, data_size, data);
      
      // write to file, 1 event = 16 byte
      fwrite(data, 1, data_size, fp);
  
      data_taken = data_taken + data_size;
      printf("%ld bytes are taken\n", data_taken);
    }
    
    // check run status
    run = TIAM_TCPread_RUN(tcp_Handle); 
  }
    
  // read remaining data size in byte
  data_size = TIAM_TCPread_DATA_SIZE(tcp_Handle);
printf("remaining data size = %ld\n", data_size);    
    
  if (data_size) {
    // read data
    TIAM_TCPread_DATA(tcp_Handle, data_size, data);
      
    // write to file, 1 event = 16 byte
    fwrite(data, 1, data_size, fp);
  
    data_taken = data_taken + data_size;
    printf("%ld bytes are taken\n", data_taken);
  }

  // close data file
  fclose(fp);

  // close TIAM
  TIAM_TCPclose(tcp_Handle);

  return 0;
}

