#include <unistd.h>
#include <stdio.h>
#include "NoticeCLOVER.h"

int main(int argc, char *argv[])
{
  int sid;
  unsigned long tcb_mid;
  unsigned long link_status;
  int num_of_adc;
  int adc;
  int adc_exist;
  int mid[9];
  unsigned long adc_type;
  unsigned long adc_id;
  
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
  
  // check TCB mid
  tcb_mid = CLOVERread_TCB_MID(sid);
  printf("TCB sid = %ld\n", tcb_mid);
  if (tcb_mid != sid) {
    printf("Wrong TCB sid!!\n");
    CLOVERclose(sid);
    USB3Exit();
    return 0;
  }

  // get link status and ADC mid, and set module ID to ADC
  link_status = CLOVERread_LINK_STATUS(sid);
  printf("link status = %lX\n", link_status);
  num_of_adc = 0;
  for (adc = 0; adc < 9; adc++) {
    adc_exist = link_status & (1 << adc);
    if (adc_exist) {
      adc_type = CLOVERread_ID(sid, adc + 1) & 0x80;
      mid[num_of_adc] = adc + 1;
      if (adc_type) {
        printf("ADC32 is found @ slot %d\n", mid[num_of_adc]);
      }
      else {
        printf("ADC10 is found @ slot %d\n", mid[num_of_adc]);
      }
      
      // set module ID to ADC
      adc_id = ((tcb_mid & 0x7) << 4) + mid[num_of_adc];
      CLOVERwrite_ID(sid, mid[num_of_adc], adc_id);
      printf("ADC ID @ slot %d = %lX\n", mid[num_of_adc], CLOVERread_ID(sid, mid[num_of_adc]));

      num_of_adc = num_of_adc + 1;
    }
  }

  CLOVERreset(sid);

  for (adc = 0; adc < num_of_adc; adc++) {
    CLOVERinit_ADC(sid, mid[adc]);
    CLOVERinit_DRAM(sid, mid[adc]);
    CLOVERinit_DAC(sid, mid[adc]);
  }
   
  // only if necessary, reset timer
  CLOVERresetTIMER(sid);  
  
  // reset DAQ
  CLOVERreset(sid);

  // close DAQ
  CLOVERclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}


