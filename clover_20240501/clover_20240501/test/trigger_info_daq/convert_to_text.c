#include <stdio.h>
#include <string.h>

int main(void)
{
  char data_filename[256];
  char text_filename[256];
  FILE *data_fp;
  FILE *text_fp;
  unsigned int file_size;
  int nevt;
  int evt;
  char data[16];
  int event_num;
  int channel;
  double lte_time;
  double local_time;
  int itmp;
  double ftmp;

  // set filename
  sprintf(data_filename, "tiam.dat");
  sprintf(text_filename, "tiam.txt");
  
  // get file size to know # of events, 1 event = 16 byte
  data_fp = fopen(data_filename, "rb");
  fseek(data_fp, 0L, SEEK_END);
  file_size = ftell(data_fp);
  fclose(data_fp);
  nevt = file_size / 16;
  
  data_fp = fopen(data_filename, "rb");
  text_fp = fopen(text_filename, "wt");
  
  fprintf(text_fp, "Channel, Trigger#, LTE trigger time in us, Local trigger time in us\n");
  fprintf(text_fp, "-------------------------------------------------------------------\n");
  
  for (evt = 0; evt < nevt; evt++) {
    fread(data, 1, 16, data_fp);

    // event#
    memcpy(&event_num, data, 4);

    // channel
    channel = data[4] & 0xFF;

    // LTE trigger time
    itmp = data[5] & 0xFF;
    ftmp = itmp;
    lte_time = ftmp * 0.008;        // lte_ftime = 8 ns unit
    itmp = data[6] & 0xFF;
    ftmp = itmp;
    lte_time = lte_time + ftmp;
    itmp = data[7] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0;
    lte_time = lte_time + ftmp;
    itmp = data[8] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0;
    lte_time = lte_time + ftmp;

    // local trigger time
    itmp = data[9] & 0xFF;
    ftmp = itmp;
    local_time = ftmp * 0.008;        // local_ftime = 8 ns unit
    itmp = data[10] & 0xFF;
    ftmp = itmp;
    local_time = local_time + ftmp;
    itmp = data[11] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0;
    local_time = local_time + ftmp;
    itmp = data[12] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0;
    local_time = local_time + ftmp;
    itmp = data[13] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0 * 256.0;
    local_time = local_time + ftmp;
    itmp = data[14] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0 * 256.0 * 256.0;
    local_time = local_time + ftmp;
    itmp = data[15] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0 * 256.0 * 256.0 * 256.0;
    local_time = local_time + ftmp;

    fprintf(text_fp, "%d, ", channel);
    fprintf(text_fp, "%d, ", event_num);
    fprintf(text_fp, "%lf, ", lte_time);
    fprintf(text_fp, "%lf\n", local_time);
  }
  
  fclose(data_fp);
  fclose(text_fp);

  return 0;
}

