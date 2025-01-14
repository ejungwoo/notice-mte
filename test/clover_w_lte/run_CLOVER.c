#include <unistd.h>
#include <stdio.h>
#include "NoticeLTE.h"

int main(int argc, char *argv[])
{
  int sid_lte = 0;
  int run_number;
  int acq_time;
  unsigned long run;
  
  if (argc > 2) {
    run_number = atoi(argv[1]);
    acq_time = atoi(argv[2]);
  }
  else {
    printf("Enter run number(0~65535) : ");
    scanf("%d", &run_number);
    printf("Enter acquisition time(0~1,000,000s, 0 for infinite) : ");
    scanf("%d", &acq_time);
  }

  // init usb
  USB3Init();
  
  // open LTE
  LTEopen(sid_lte);

  // reset DAQ
  LTEreset(sid_lte);

  // set run number
  LTEwrite_RUN_NUMBER(sid_lte, (unsigned long)run_number);
  printf("run_number = %ld\n", LTEread_RUN_NUMBER(sid_lte));
  
  // set acquisition time
  LTEwrite_ACQUISITION_TIME(sid_lte, (unsigned long)acq_time);
  printf("acquisition time = %ld s\n", LTEread_ACQUISITION_TIME(sid_lte));
  
  // start DAQ
  LTEstart(sid_lte);
  run = LTEread_RUN(sid_lte);
  printf("Run = %ld\n", run);
  
  // close LTE
  LTEclose(sid_lte);
  
  // close usb
  USB3Exit();
  
  return 0;
}


