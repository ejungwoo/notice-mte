#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LTE_VID  (0x0547)
#define LTE_PID (0x1309)

  extern void USB3Init(void);
  extern void USB3Exit(void);
  extern int LTEopen(int sid);
  extern void LTEclose(int sid);
  extern void LTEresetTIMER(int sid);  
  extern void LTEreset(int sid);
  extern void LTEstart(int sid);
  extern void LTEstop(int sid);
  extern unsigned long LTEread_RUN(int sid);
  extern void LTEwrite_CW(int sid, unsigned long data);
  extern unsigned long LTEread_CW(int sid);
  extern void LTEwrite_RUN_NUMBER(int sid, unsigned long data);
  extern unsigned long LTEread_RUN_NUMBER(int sid);
  extern void LTEwrite_MTHR(int sid, unsigned long data);
  extern unsigned long LTEread_MTHR(int sid);
  extern void LTEsend_TRIG(int sid);
  extern unsigned long LTEread_LINK_STATUS(int sid);
  extern void LTEwrite_TRIG_ENABLE(int sid, unsigned long data);
  extern unsigned long LTEread_TRIG_ENABLE(int sid);
  extern void LTEwrite_PTRIG_INTERVAL(int sid, unsigned long data);
  extern unsigned long LTEread_PTRIG_INTERVAL(int sid);
  extern void LTEwrite_ACQUISITION_TIME(int sid, unsigned long data);
  extern unsigned long LTEread_ACQUISITION_TIME(int sid);
  extern int LTEread_MID(int sid, int ch);
  extern void LTEwrite_ECHO(int sid, unsigned long data);
  extern unsigned long LTEread_ECHO(int sid);

#ifdef __cplusplus
}
#endif



