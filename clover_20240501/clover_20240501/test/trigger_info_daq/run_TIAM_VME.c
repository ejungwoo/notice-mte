#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NoticeTIAM.h"

int main(int argc, char *argv[])
{
  int sid = 0;                  // temporary use for Notice VME controller
  unsigned long mid;
  int vme_Handle;               
  char filename[256];
  FILE *fp;
  unsigned long run;
  unsigned long run_number;
  unsigned long data_size;
  unsigned long data_taken;
  char *data;

  if (argc > 1)
    mid = atoi(argv[1]);
  else
    mid = 2;

  // set filename
  sprintf(filename, "tiam.dat");

  // assign data array, max = 32 kbyte
  data = (char *)malloc(32 * 1024); 
  
  // open TIAM
  vme_Handle = TIAM_VMEopen(sid);

  // open data file
  fp = fopen(filename, "wb");

  // wait for LTE runs
  printf("Wait for LTE is running...\n");
  run = TIAM_VMEread_RUN(vme_Handle, mid);
  while(!run)
    run = TIAM_VMEread_RUN(vme_Handle, mid);

  run_number = TIAM_VMEread_RUNNO(vme_Handle, mid);
  printf("Start to take data, run # = %ld\n", run_number);

  run = TIAM_VMEread_RUN(vme_Handle, mid);
  
  data_taken = 0;
  
  while (run) {
    // read data size in byte
    data_size = TIAM_VMEread_DATA_SIZE(vme_Handle, mid);
    
    if (data_size) {
printf("data size = %ld\n", data_size);    
if (data_size > 32768) return -1;
      // read data
      TIAM_VMEread_DATA(vme_Handle, mid, data_size, data);
      
      // write to file, 1 event = 16 byte
      fwrite(data, 1, data_size, fp);
  
      data_taken = data_taken + data_size;
      printf("%ld bytes are taken\n", data_taken);
    }
    
    // check run status
    run = TIAM_VMEread_RUN(vme_Handle, mid);
  }
    
  // read remaining data size in byte
  data_size = TIAM_VMEread_DATA_SIZE(vme_Handle, mid);
//printf("remaining data size = %ld\n", data_size);    
    
  if (data_size) {
    // read data
    TIAM_VMEread_DATA(vme_Handle, mid, data_size, data);
      
    // write to file, 1 event = 16 byte
    fwrite(data, 1, data_size, fp);
  
    data_taken = data_taken + data_size;
    printf("%ld bytes are taken\n", data_taken);
  }

  // close data file
  fclose(fp);

  // close TIAM
  TIAM_VMEclose(vme_Handle);

  return 0;
}

