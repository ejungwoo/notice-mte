#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLOVER_VID  (0x0547)
#define CLOVER_PID (0x2207)

  extern void USB3Init(void);
  extern void USB3Exit(void);
  extern int CLOVERopen(int sid);
  extern void CLOVERclose(int sid);
  extern void CLOVERresetTIMER(int sid);  
  extern void CLOVERreset(int sid);
  extern void CLOVERstart(int sid);
  extern void CLOVERstop(int sid);
  extern unsigned long CLOVERread_RUN(int sid);
  extern void CLOVERwrite_CW(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_CW(int sid, int mid);
  extern void CLOVERwrite_RUN_NUMBER(int sid, unsigned long data);
  extern unsigned long CLOVERread_RUN_NUMBER(int sid);
  extern void CLOVERwrite_MTHR(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_MTHR(int sid, int mid);
  extern void CLOVERsend_TRIG(int sid);
  extern unsigned long CLOVERread_LINK_STATUS(int sid);
  extern void CLOVERwrite_TRIG_ENABLE(int sid, unsigned long data);
  extern unsigned long CLOVERread_TRIG_ENABLE(int sid);
  extern void CLOVERwrite_PTRIG_INTERVAL(int sid, unsigned long data);
  extern unsigned long CLOVERread_PTRIG_INTERVAL(int sid);
  extern void CLOVERwrite_ACQUISITION_TIME(int sid, unsigned long data);
  extern unsigned long CLOVERread_ACQUISITION_TIME(int sid);
  extern unsigned long CLOVERread_TCB_MID(int sid);
  extern unsigned long CLOVERread_DATA_SIZE(int sid, int mid);
  extern void CLOVERread_DATA(int sid, int mid, unsigned long data_size, char *data);
  extern void CLOVERwrite_POL(int sid, int mid, unsigned long data); 
  extern unsigned long CLOVERread_POL(int sid, int mid); 
  extern unsigned long CLOVERread_RISE_TIME(int sid, int mid, int ch); 
  extern unsigned long CLOVERread_FLAT_TOP(int sid, int mid, int ch);
  extern unsigned long CLOVERread_DECAY_TIME(int sid, int mid, int ch);
  extern float CLOVERread_RC_COEF(int sid, int mid);
  extern float CLOVERread_CR_GAIN(int sid, int mid);
  extern float CLOVERread_CR2_GAIN(int sid, int mid);
  extern void CLOVERmeasure_PED(int sid, int mid);
  extern unsigned long CLOVERread_PED(int sid, int mid, int ch);
  extern void CLOVERwrite_THR(int sid, int mid, int ch, unsigned long data);
  extern unsigned long CLOVERread_THR(int sid, int mid, int ch);
  extern void CLOVERwrite_TRIG_MODE(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_TRIG_MODE(int sid, int mid);
  extern void CLOVERwrite_GATE_WIDTH(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_GATE_WIDTH(int sid, int mid);
  extern void CLOVERwrite_PRESCALE(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_PRESCALE(int sid, int mid);
  extern void CLOVERwrite_DLY(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_DLY(int sid, int mid);
  extern void CLOVERwrite_ZERO_SUP(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_ZERO_SUP(int sid, int mid);
  extern void CLOVERwrite_WAVEFORM_SEL(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_WAVEFORM_SEL(int sid, int mid);
  extern void CLOVERwrite_PEAKSUM_WIDTH(int sid, int mid, int ch, unsigned long data);
  extern unsigned long CLOVERread_PEAKSUM_WIDTH(int sid, int mid, int ch);
  extern void CLOVERwrite_PEAKSUM_DLY(int sid, int mid, int ch, unsigned long data);
  extern unsigned long CLOVERread_PEAKSUM_DLY(int sid, int mid, int ch);
  extern void CLOVERwrite_ENABLE_FASTDAQ(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_ENABLE_FASTDAQ(int sid, int mid);
  extern void CLOVERwrite_PEAK_MODE(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_PEAK_MODE(int sid, int mid);
  extern void CLOVERwrite_ECHO(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_ECHO(int sid, int mid);
  extern void CLOVERwrite_ID(int sid, int mid, unsigned long data);
  extern unsigned long CLOVERread_ID(int sid, int mid);
  extern void CLOVERset_TRAPEZOIDAL_FILTER(int sid, int mid, int ch, unsigned long rise_time,
                                           unsigned long flat_top, unsigned long decay_time);
  extern void CLOVERset_RCCR2_FILTER(int sid, int mid, float rc_coef, float cr_gain, float cr2_gain);
  extern void CLOVERinit_ADC(int sid, int mid);
  extern void CLOVERinit_DRAM(int sid, int mid);
  extern void CLOVERinit_DAC(int sid, int mid);
  extern void CLOVERwrite_OFFSET(int sid, int mid, int ch, unsigned long data);

#ifdef __cplusplus
}
#endif


