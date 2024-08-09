#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  CLOVER_VID  0x0547
#define  CLOVER_PID 0x2207

// module protocol
#define VENDOR_ADC_SELECT				         (0xE1)
#define VENDOR_ADC_WRITE               	 (0xE2)
#define VENDOR_ADC_ERASE               	 (0xE4)
#define VENDOR_ADC_FINISH              	 (0xE5)
#define VENDOR_READ_ADC_FPGA_VERSION     (0xE6)

extern int CLOVERopen(int sid);
extern void CLOVERclose(int sid);
extern int CLOVERport(int sid, int port);
extern int CLOVERerase(int sid, int sector);
extern int CLOVERfinish(int sid);
extern int CLOVERwrite(int sid, int sector, int page, char *data);
extern int CLOVERread(int sid, unsigned char *data);

#ifdef __cplusplus
}
#endif

