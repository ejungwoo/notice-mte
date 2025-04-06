#include <unistd.h>
#include <stdio.h>
#include "NoticeLTE.h"

int main(void)
{
  int debugging = 0;
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
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  USB3Init();

  // open LTE
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  LTEopen(sid);

  // get LTE link status and TCB mid
  lte_link_status = LTEread_LINK_STATUS(sid);
  printf("LTE link status = %lX\n", lte_link_status);
  num_of_tcb = 0;
  for (tcb = 0; tcb < 9; tcb++) {
    tcb_exist = lte_link_status & (1 << tcb);
    printf("tcb_exist? %d (%ld)\n", tcb_exist, lte_link_status);
    if (tcb_exist) {
      mid_tcb[num_of_tcb] = LTEread_MID(sid, tcb);;
      printf("TCB[sid = %d] is found @ slot %d\n", mid_tcb[num_of_tcb], tcb + 1);
      num_of_tcb = num_of_tcb + 1;
    }
  }
  
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  LTEreset(sid);

  // read settings
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  fp = fopen("setup_lte.txt", "rt");
  fscanf(fp, "%ld", &cw);
  fscanf(fp, "%ld", &mthr);
  fscanf(fp, "%ld", &trig_enable);
  fscanf(fp, "%ld", &ptrig_interval);
  fclose(fp);

  // write setting
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  LTEwrite_CW(sid, cw);
  LTEwrite_MTHR(sid, mthr);
  LTEwrite_TRIG_ENABLE(sid, trig_enable);
  LTEwrite_PTRIG_INTERVAL(sid, ptrig_interval);
   
  // read back settings
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  printf("LTE coincidence width = %ld ns\n", LTEread_CW(sid));
  printf("LTE multiplicity threshold = %ld\n", LTEread_MTHR(sid));
  printf("LTE trigger enable = %ld\n", LTEread_TRIG_ENABLE(sid));
  printf("LTE pedestal trigger interval = %ld ms\n", LTEread_PTRIG_INTERVAL(sid));

  // only if necessary, reset timer
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  LTEresetTIMER(sid);  
  
  // reset DAQ
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  LTEreset(sid);

  // close LTE
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  LTEclose(sid);
  
  // close usb
  if (debugging) fprintf(stderr, "+%d %s #\n", __LINE__, __FILE__);
  USB3Exit();
  
  return 0;
}


