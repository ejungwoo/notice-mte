#include <unistd.h>
#include <stdio.h>
#include "NoticeLTE.h"
#include "NoticeCLOVER.h"

int main(int argc, char *argv[])
{
  int sid;
  
  if (argc > 2) {
    sid = atoi(argv[1]);
    run_flag = atoi(argv[2]);
  }
  else {
    printf("Enter LTE sid : ");
    scanf("%d", &sid);
    printf("Enter run start or stop(0:run stop start) : ");
    scanf("%d", &run_number);
    printf("Enter acquisition time(0~1,000,000s, 0 for infinite) : ");
    scanf("%d", &acq_time);
  }

  // init usb
  USB3Init();
  
  // open LTE
  LTEopen(sid);

  // reset DAQ
  LTEreset(sid);

  // get link status and TCB sid
  link_status = LTEread_LINK_STATUS(sid);
  for (tcb = 0; tcb < 9; tcb++) {
    tcb_exist = link_status & (1 << tcb);
    if (tcb_exist) {
      mid = LTEread_MID(sid, tcb);
      printf("TCB%d is linked @%d\n", mid, tcb + 1);
    }
  }

  // set run number
  LTEwrite_RUN_NUMBER(sid, (unsigned long)run_number);
  printf("run_number = %ld\n", LTEread_RUN_NUMBER(sid));
  
  // set acquisition time
  LTEwrite_ACQUISITION_TIME(sid, (unsigned long)acq_time);
  printf("acquisition time = %ld s\n", LTEread_ACQUISITION_TIME(sid));
  
  // reset timer if necessary
//  LTEresetTIMER(sid_lte);  

  // open file
  sprintf(tag_filename, "lte_tag_%d.dat", run_number);
  sprintf(count_filename, "lte_count_%d.dat", run_number);
  tag_fp = fopen(tag_filename, "wb");
  count_fp = fopen(count_filename, "wb");
  tag_evt = 0;
  count_evt = 0;
  
  // assign data array
  tag_data = (char *)malloc(65536); 
  count_data = (char *)malloc(65536); 
  
  // start DAQ
  LTEstart(sid);
  run = LTEread_RUN(sid);
  printf("Run = %ld\n", run);
  
  while (run) {
    // just for debugging
//    LTEsend_TRIG(sid_lte);

    // check tag data size
    tag_data_size = LTEread_TAG_DATA_SIZE(sid);
   
    if (tag_data_size) {
      // read tag data
      LTEread_TAG_DATA(sid, tag_data_size, tag_data);
      
      // write to charge data file
      fwrite(tag_data, 1, tag_data_size * 4, tag_fp);
      
      // add tag event number
      tag_evt = tag_evt + tag_data_size / 4;    // 1 event = 16 bytes
      printf("tag data : %d events are taken\n", tag_evt);
    }

    // check count data size
    count_data_size = LTEread_COUNT_DATA_SIZE(sid);
    
    if (count_data_size) {
      // read count data
      LTEread_COUNT_DATA(sid, count_data_size, count_data);
      
      // write to file
      fwrite(count_data, 1, count_data_size * 4, count_fp);
      
      // add count event number
      count_evt = count_evt + count_data_size / 16;  // 1 event = 64 bytes
      printf("count data : %d evetns are taken\n", count_evt);
    }
    
    // check run
    run = LTEread_RUN(sid);
  }

  printf("LTE is stopped and read remaining data\n");

  // read remaining data
  // check tag data size
  tag_data_size = LTEread_TAG_DATA_SIZE(sid);
   
  if (tag_data_size) {
    // read tag data
    LTEread_TAG_DATA(sid, tag_data_size, tag_data);
      
    // write to charge data file
    fwrite(tag_data, 1, tag_data_size * 4, tag_fp);
      
    // add tag event number
    tag_evt = tag_evt + tag_data_size / 4;    // 1 event = 16 bytes
    printf("tag data : %d events are taken\n", tag_evt);
  }

  // check count data size
  count_data_size = LTEread_COUNT_DATA_SIZE(sid);
    
  if (count_data_size) {
    // read count data
    LTEread_COUNT_DATA(sid, count_data_size, count_data);
      
    // write to file
    fwrite(count_data, 1, count_data_size * 4, count_fp);
      
    // add count event number
    count_evt = count_evt + count_data_size / 16;  // 1 event = 64 bytes
    printf("count data : %d evetns are taken\n", count_evt);
  }

  // release data array
  free(tag_data);
  free(count_data);

  // close file
  fclose(tag_fp);  
  fclose(count_fp);  

  // close LTE
  LTEclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}

