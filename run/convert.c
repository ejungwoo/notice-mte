#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
  char data_filename[256];
  char text_filename[256];
  FILE *data_fp;
  FILE *text_fp;
  unsigned int file_size;
  int nevt;
  int evt;
  char data[16];
  int trig_num;
  double trig_time;
  int trig_type;
  int ext_pattern[10];
  int itmp;
  double ftmp;
  int i;

  if (argc > 1) {
    sprintf(data_filename, "lte_tag_%s.dat", argv[1]);
    sprintf(text_filename, "lte_tag_%s.txt", argv[1]);
  }
  else {
    printf("./convert_tag_to_text.exe data_file_name(without extension)\n");
    return 0;
  }
  
  // get file size to know # of events, 1 event = 32 byte
  data_fp = fopen(data_filename, "rb");
  fseek(data_fp, 0L, SEEK_END);
  file_size = ftell(data_fp);
  fclose(data_fp);
  nevt = file_size / 16;
  
  data_fp = fopen(data_filename, "rb");
  text_fp = fopen(text_filename, "wt");
  
  fprintf(text_fp, "Trigger #, Trigger time in us, Trigger type, External trigger pattern[1..10]\n");
  fprintf(text_fp, "----------------------------------------------------------------------------\n");
  
  for (evt = 0; evt < nevt; evt++) {
    fread(data, 1, 16, data_fp);
    
    // trigger logic #
    memcpy(&trig_num, data, 4);
    
    // trigger time
    itmp = data[4] & 0xFF;
    ftmp = itmp;
    trig_time = ftmp * 0.008;        // trig_ftime = 8 ns unit
    itmp = data[5] & 0xFF;
    ftmp = itmp;
    trig_time = trig_time + ftmp;
    itmp = data[6] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0;
    trig_time = trig_time + ftmp;
    itmp = data[7] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0;
    trig_time = trig_time + ftmp;
    itmp = data[8] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0 * 256.0;
    trig_time = trig_time + ftmp;
    itmp = data[9] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0 * 256.0 * 256.0;
    trig_time = trig_time + ftmp;
    itmp = data[10] & 0xFF;
    ftmp = itmp;
    ftmp = ftmp * 256.0 * 256.0 * 256.0 * 256.0 * 256.0;
    trig_time = trig_time + ftmp;

    // trigger type 
    trig_type = data[11] & 0xFF;

    // external trigger pattern   
    ext_pattern[0] = data[12] & 0x1;
    ext_pattern[1] = (data[12] >> 1) & 0x1;
    ext_pattern[2] = (data[12] >> 2) & 0x1;
    ext_pattern[3] = (data[12] >> 3) & 0x1;
    ext_pattern[4] = (data[12] >> 4) & 0x1;
    ext_pattern[5] = (data[12] >> 5) & 0x1;
    ext_pattern[6] = (data[12] >> 6) & 0x1;
    ext_pattern[7] = (data[12] >> 7) & 0x1;
    ext_pattern[8] = data[13] & 0x1;
    ext_pattern[9] = (data[13] >> 1) & 0x1;
    
    fprintf(text_fp, "%d, ", trig_num);
    fprintf(text_fp, "%lf, ", trig_time);
    fprintf(text_fp, "%d, ", trig_type);
    for (i = 0; i < 10; i++)
      fprintf(text_fp, "%d", ext_pattern[i]);
    fprintf(text_fp, "\n");
  }
  
  fclose(data_fp);
  fclose(text_fp);

  return 0;
}

