#include <unistd.h>
#include <stdio.h>
#include "NoticeCLOVER.h"

int main(int argc, char *argv[])
{
  int sid;
  unsigned long link_status;
  int num_of_adc;
  int adc;
  int adc_exist;
  int mid[9];
  int run_number;
  char filename_adc[100];
  char filename_tcb[100];
  FILE *fp_adc;
  FILE *fp_tcb;
  unsigned long run;
  unsigned long data_size;
  int adc_data_written;
  int tcb_data_written;
  char *data;
  
  if (argc > 1) 
    sid = atoi(argv[1]);
  else {
    printf("Enter TCB sid : ");
    scanf("%d", &sid);
  }

  // init usb
  USB3Init();
  
  // open DAQ
  CLOVERopen(sid);

  // get link status and ADC mid
  link_status = CLOVERread_LINK_STATUS(sid);
  num_of_adc = 0;
  for (adc = 0; adc < 9; adc++) {
    adc_exist = link_status & (1 << adc);
    if (adc_exist) {
      mid[num_of_adc] = adc + 1;
      num_of_adc = num_of_adc + 1;
    }
  }

  // assign data array
  data = (char *)malloc(524288);

  // wait for LTE runs
  printf("Waits for LTE running...\n");
  run = CLOVERread_RUN(sid);
  while (!run)
    run = CLOVERread_RUN(sid);
    
  printf("Start to take data...\n");
  
  // read run # and open data files
  run_number = CLOVERread_RUN_NUMBER(sid);
  sprintf(filename_adc, "clover_adc_%d.dat", run_number);
  sprintf(filename_tcb, "clover_tcb_%d.dat", run_number);
  fp_adc = fopen(filename_adc, "wb"); 
  fp_tcb = fopen(filename_tcb, "wb"); 
  adc_data_written = 0;
  tcb_data_written = 0;

  while (run) {
    // check TCB data size
    data_size = CLOVERread_DATA_SIZE(sid, 0);
   
    // read TCB data
    if (data_size) {
      CLOVERread_DATA(sid, 0, data_size, data);
      fwrite(data, 1, data_size * 1024, fp_tcb);
      tcb_data_written = tcb_data_written + data_size;
      printf("TCB %d kbytes are taken\n", tcb_data_written);
    }

    for (adc = 0; adc < num_of_adc; adc++) {
      // check ADC data size
      data_size = CLOVERread_DATA_SIZE(sid, mid[adc]);
   
      // read ADC data
      if (data_size) {
        CLOVERread_DATA(sid, mid[adc], data_size, data);
        fwrite(data, 1, data_size * 1024, fp_adc);
        adc_data_written = adc_data_written + data_size;
        printf("ADC %d kbytes are taken\n", adc_data_written);
      }
    }

    run = CLOVERread_RUN(sid);
  }
  
  printf("DAQ is stopped and read remaining data\n");

  // read remaining data  
  // check TCB data size
  data_size = CLOVERread_DATA_SIZE(sid, 0);
   
  // read TCB data
  if (data_size) {
    CLOVERread_DATA(sid, 0, data_size, data);
    fwrite(data, 1, data_size * 1024, fp_tcb);
    tcb_data_written = tcb_data_written + data_size;
    printf("TCB %d kbytes are taken\n", tcb_data_written);
  }

  for (adc = 0; adc < num_of_adc; adc++) {
    data_size = 16;
    while (data_size) {
      // check ADC data size
      data_size = CLOVERread_DATA_SIZE(sid, mid[adc]);
   
      // read ADC data
      if (data_size) {
        CLOVERread_DATA(sid, mid[adc], data_size, data);
        fwrite(data, 1, data_size * 1024, fp_adc);
        adc_data_written = adc_data_written + data_size;
        printf("ADC %d kbytes are taken\n", adc_data_written);
      }
    }
  }    
  
  // release data array
  free(data);

  // close file
  fclose(fp_adc);
  fclose(fp_tcb);

  // close DAQ
  CLOVERclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}


