#include <unistd.h>
#include <stdio.h>
#include "NoticeLTE.h"

int main(void)
{
  int sid = 0;
  unsigned long lte_link_status;
  int num_of_tcb;
  int tcb;
  int tcb_exist;
  int mid_tcb[10];
  FILE *fp;
  unsigned long cw;
  unsigned long mthr;
  unsigned long trig_enable;
  unsigned long ptrig_interval;
  
  // init usb
  USB3Init();

  // open LTE
  LTEopen(sid);

  // get LTE link status and TCB mid
  lte_link_status = LTEread_LINK_STATUS(sid);
  printf("LTE link status = %lX\n", lte_link_status);
  num_of_tcb = 0;
  for (tcb = 0; tcb < 9; tcb++) {
    tcb_exist = lte_link_status & (1 << tcb);
    if (tcb_exist) {
      mid_tcb[num_of_tcb] = LTEread_MID(sid, tcb);;
      printf("TCB[sid = %d] is found @ slot %d\n", mid_tcb[num_of_tcb], tcb + 1);
      num_of_tcb = num_of_tcb + 1;
    }
  }
  
  LTEreset(sid);

  // read settings
  fp = fopen("setup_lte.txt", "rt");
  fscanf(fp, "%ld", &cw);
  fscanf(fp, "%ld", &mthr);
  fscanf(fp, "%ld", &trig_enable);
  fscanf(fp, "%ld", &ptrig_interval);
  fclose(fp);

  // write setting
  LTEwrite_CW(sid, cw);
  LTEwrite_MTHR(sid, mthr);
  LTEwrite_TRIG_ENABLE(sid, trig_enable);
  LTEwrite_PTRIG_INTERVAL(sid, ptrig_interval);
   
  // read back settings
  printf("LTE coincidence width = %ld ns\n", LTEread_CW(sid));
  printf("LTE multiplicity threshold = %ld\n", LTEread_MTHR(sid));
  printf("LTE trigger enable = %ld\n", LTEread_TRIG_ENABLE(sid));
  printf("LTE pedestal trigger interval = %ld ms\n", LTEread_PTRIG_INTERVAL(sid));

  // only if necessary, reset timer
  LTEresetTIMER(sid);  
  
  // reset DAQ
  LTEreset(sid);

  // close LTE
  LTEclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}


