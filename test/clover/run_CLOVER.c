#include <unistd.h>
#include <stdio.h>
#include "NoticeCLOVER.h"

int main(int argc, char *argv[])
{
  int sid;
  int run_number;
  int acq_time;
  unsigned long link_status;
  int num_of_adc;
  int adc;
  int adc_exist;
  int mid[9];
  unsigned long adc_type;
  char filename[100];
  FILE *fp;
  unsigned long run;
  unsigned long data_size;
  int data_written;
  char *data;
  
  if (argc > 3) {
    sid = atoi(argv[1]);
    run_number = atoi(argv[2]);
    acq_time = atoi(argv[3]);
  }
  else {
    printf("Enter TCB sid : ");
    scanf("%d", &sid);
    printf("Enter run number(0~65535) : ");
    scanf("%d", &run_number);
    printf("Enter acquisition time(0~1,000,000s, 0 for infinite) : ");
    scanf("%d", &acq_time);
  }

  // init usb
  USB3Init();
  
  // open DAQ
  CLOVERopen(sid);

  // reset DAQ
  CLOVERreset(sid);

  // get link status and ADC mid
  link_status = CLOVERread_LINK_STATUS(sid);
  num_of_adc = 0;
  for (adc = 0; adc < 9; adc++) {
    adc_exist = link_status & (1 << adc);
    if (adc_exist) {
      adc_type = CLOVERread_ID(sid, adc + 1);
      if ((adc_type == 0xADC10) || (adc_type == 0xADC32)) {
        mid[num_of_adc] = adc + 1;
        num_of_adc = num_of_adc + 1;
      }
    }
  }

  // set run number
  CLOVERwrite_RUN_NUMBER(sid, (unsigned long)run_number);
  printf("run_number = %ld\n", CLOVERread_RUN_NUMBER(sid));
  
  // set acquisition time
  CLOVERwrite_ACQUISITION_TIME(sid, (unsigned long)acq_time);
  printf("acquisition time = %ld s\n", CLOVERread_ACQUISITION_TIME(sid));
  
  // reset timer if necessary
//  CLOVERresetTIMER(int sid);  

  // open file
  sprintf(filename, "clover_%d.dat", run_number);
  fp = fopen(filename, "wb"); 
  data_written = 0;
  
  // assign data array
  data = (char *)malloc(32768);
  
  // start DAQ
  CLOVERstart(sid);
  run = CLOVERread_RUN(sid);
  printf("Run = %ld\n", run);
  
  while (run) {
    // just for debugging
//    CLOVERsend_TRIG(sid);

    for (adc = 0; adc < num_of_adc; adc++) {
      // check data size
      data_size = CLOVERread_DATA_SIZE(sid, mid[adc]);
   
      // read data
      if (data_size) {
        CLOVERread_DATA(sid, mid[adc], data_size, data);
        fwrite(data, 1, data_size * 1024, fp);
        data_written = data_written + data_size;
        printf("%d kbytes are taken\n", data_written);
      }
    }

    run = CLOVERread_RUN(sid);
  }
  
  printf("DAQ is stopped and read remaining data\n");

  // read remaining data  
  for (adc = 0; adc < num_of_adc; adc++) {
    data_size = 16;
    while (data_size) {
      // check data size
      data_size = CLOVERread_DATA_SIZE(sid, mid[adc]);
   
      // read data
      if (data_size) {
        CLOVERread_DATA(sid, mid[adc], data_size, data);
        fwrite(data, 1, data_size * 1024, fp);
        data_written = data_written + data_size;
        printf("%d kbytes are taken\n", data_written);
      }
    }
  }    
  
  // release data array
  free(data);

  // close file
  fclose(fp);

  // reset DAQ
//  CLOVERreset(sid);

  // close DAQ
  CLOVERclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}


