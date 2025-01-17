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
  unsigned long adc_type;
  int num_of_ch[9];
  FILE *fp;
  unsigned long tcb_cw;
  unsigned long tcb_mthr;
  unsigned long tcb_trig_enable;
  unsigned long tcb_ptrig_interval;
  unsigned long adc_cw;
  unsigned long adc_pol;
  unsigned long adc_mthr;
  unsigned long adc_rise_time;
  unsigned long adc_flat_top;
  unsigned long adc_decay_time;
  float adc_rc_coeff;
  float adc_cr_coeff;
  float adc_cr2_coeff;
  unsigned long adc_thr;
  unsigned long adc_trig_mode;
  unsigned long adc_gate_width;
  unsigned long adc_prescale;
  unsigned long adc_delay;
  unsigned long adc_zero_sup;
  unsigned long adc_waveform_sel;
  unsigned long adc_offset;
  int ch;
  
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
  printf("link status = %lX\n", link_status);
  num_of_adc = 0;
  for (adc = 0; adc < 9; adc++) {
    adc_exist = link_status & (1 << adc);
    if (adc_exist) {
      adc_type = CLOVERread_ID(sid, adc + 1);
      if ((adc_type == 0xADC10) || (adc_type == 0xADC32)) {
        mid[num_of_adc] = adc + 1;
        if (adc_type == 0xADC10)
          num_of_ch[num_of_adc] = 10;
        else
          num_of_ch[num_of_adc] = 32;
        printf("%lX is found @ slot %d\n", adc_type, mid[num_of_adc]);
        num_of_adc = num_of_adc + 1;
      }
      else 
        printf("Unknown ADC(type = %lX) is found @ slot %d\n", adc_type, adc + 1);
    }
  }

  CLOVERreset(sid);

  // read settings
  fp = fopen("setup.txt", "rt");
  fscanf(fp, "%ld", &tcb_cw);
  fscanf(fp, "%ld", &tcb_mthr);
  fscanf(fp, "%ld", &tcb_trig_enable);
  fscanf(fp, "%ld", &tcb_ptrig_interval);
  fscanf(fp, "%ld", &adc_cw);
  fscanf(fp, "%ld", &adc_pol);
  fscanf(fp, "%ld", &adc_mthr);
  fscanf(fp, "%ld", &adc_rise_time);
  fscanf(fp, "%ld", &adc_flat_top);
  fscanf(fp, "%ld", &adc_decay_time);
  fscanf(fp, "%f", &adc_rc_coeff);
  fscanf(fp, "%f", &adc_cr_coeff);
  fscanf(fp, "%f", &adc_cr2_coeff);
  fscanf(fp, "%ld", &adc_thr);
  fscanf(fp, "%ld", &adc_trig_mode);
  fscanf(fp, "%ld", &adc_gate_width);
  fscanf(fp, "%ld", &adc_prescale);
  fscanf(fp, "%ld", &adc_delay);
  fscanf(fp, "%ld", &adc_zero_sup);
  fscanf(fp, "%ld", &adc_waveform_sel);
  fscanf(fp, "%ld", &adc_offset);
  fclose(fp);
  
  // write setting
  CLOVERwrite_CW(sid, 0, tcb_cw);
  CLOVERwrite_MTHR(sid, 0, tcb_mthr);
  CLOVERwrite_TRIG_ENABLE(sid, tcb_trig_enable);
  CLOVERwrite_PTRIG_INTERVAL(sid, tcb_ptrig_interval);
  for (adc = 0; adc < num_of_adc; adc++) {
    CLOVERinit_ADC(sid, mid[adc]);
    CLOVERinit_DRAM(sid, mid[adc]);
    CLOVERinit_DAC(sid, mid[adc]);
    CLOVERwrite_CW(sid, mid[adc], adc_cw);
    CLOVERwrite_POL(sid, mid[adc], adc_pol); 
    CLOVERwrite_MTHR(sid, mid[adc], adc_mthr);
    CLOVERset_TRAPEZOIDAL_FILTER(sid, mid[adc], adc_rise_time, adc_flat_top, adc_decay_time);
    CLOVERset_RCCR2_FILTER(sid, mid[adc], adc_rc_coeff, adc_cr_coeff, adc_cr2_coeff);
    CLOVERwrite_THR(sid, mid[adc], 0, adc_thr);
    CLOVERwrite_TRIG_MODE(sid, mid[adc], adc_trig_mode);
    CLOVERwrite_GATE_WIDTH(sid, mid[adc], adc_gate_width);
    CLOVERwrite_PRESCALE(sid, mid[adc], adc_prescale);
    CLOVERwrite_DLY(sid, mid[adc], adc_delay);
    CLOVERwrite_ZERO_SUP(sid, mid[adc], adc_zero_sup);
    CLOVERwrite_WAVEFORM_SEL(sid, mid[adc], adc_waveform_sel);
    CLOVERwrite_OFFSET(sid, mid[adc], 0, adc_offset);
  }
    
  // read back settings
  printf("TCB coincidence width = %ld ns\n", CLOVERread_CW(sid, 0));
  printf("TCB multiplicity threshold = %ld\n", CLOVERread_MTHR(sid, 0));
  printf("TCB trigger enable = %ld\n", CLOVERread_TRIG_ENABLE(sid));
  printf("TCB pedestal trigger interval = %ld ms\n", CLOVERread_PTRIG_INTERVAL(sid));
  for (adc = 0; adc < num_of_adc; adc++) {
    printf("ADC@%d coincidence width = %ld ns\n", mid[adc], CLOVERread_CW(sid, mid[adc]));
    printf("ADC@%d input polarity = %ld\n", mid[adc], CLOVERread_POL(sid, mid[adc])); 
    printf("ADC@%d multiplicity threshold = %ld\n", mid[adc], CLOVERread_MTHR(sid, mid[adc]));
    printf("ADC@%d rise time = %ld ns\n", mid[adc], CLOVERread_RISE_TIME(sid, mid[adc])); 
    printf("ADC@%d flat top = %ld ns\n", mid[adc], CLOVERread_FLAT_TOP(sid, mid[adc]));
    printf("ADC@%d decay time = %ld ns\n", mid[adc], CLOVERread_DECAY_TIME(sid, mid[adc]));
    printf("ADC@%d RC filter coefficient = %f\n", mid[adc], CLOVERread_RC_COEF(sid, mid[adc]));
    printf("ADC@%d CR filter coefficient = %f\n", mid[adc], CLOVERread_CR_GAIN(sid, mid[adc]));
    printf("ADC@%d CR2 filter coefficient = %f\n", mid[adc], CLOVERread_CR2_GAIN(sid, mid[adc]));
    for (ch = 1; ch <= num_of_ch[adc]; ch++)
      printf("ADC@%d ch%d threshold = %ld\n", mid[adc], ch, CLOVERread_THR(sid, mid[adc], ch));
    printf("ADC@%d trigger mode = %ld\n", mid[adc], CLOVERread_TRIG_MODE(sid, mid[adc]));
    printf("ADC@%d gate width = %ld ns\n", mid[adc], CLOVERread_GATE_WIDTH(sid, mid[adc]));
    printf("ADC@%d waveform prescale = %ld\n", mid[adc], CLOVERread_PRESCALE(sid, mid[adc]));
    printf("ADC@%d delay = %ld\n", mid[adc], CLOVERread_DLY(sid, mid[adc]));
    printf("ADC@%d zero suppression = %ld\n", mid[adc], CLOVERread_ZERO_SUP(sid, mid[adc]));
    printf("ADC@%d waveform selection = %ld\n", mid[adc], CLOVERread_WAVEFORM_SEL(sid, mid[adc]));
    CLOVERmeasure_PED(sid, mid[adc]);
    for (ch = 1; ch <= num_of_ch[adc]; ch++)
      printf("ADC@%d ch%d pedestal = %ld\n", mid[adc], ch, CLOVERread_PED(sid, mid[adc], ch));
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


