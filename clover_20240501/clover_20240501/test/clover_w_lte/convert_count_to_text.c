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
  int data[16];
  int i;

  if (argc > 1) {
    sprintf(data_filename, "%s.dat", argv[1]);
    sprintf(text_filename, "%s.txt", argv[1]);
  }
  else {
    printf("./convert_tag_to_text.exe data_file_name(without extension)\n");
    return 0;
  }
  
  // get file size to know # of events, 1 event = 128 byte
  data_fp = fopen(data_filename, "rb");
  fseek(data_fp, 0L, SEEK_END);
  file_size = ftell(data_fp);
  fclose(data_fp);
  nevt = file_size / 64;
  
  data_fp = fopen(data_filename, "rb");
  text_fp = fopen(text_filename, "wt");

  fprintf(text_fp, "Second, IN1, IN2, .... , IN9, IN10\n");
  fprintf(text_fp, "-----------------------------------------------------------------------------------\n");

  for (evt = 0; evt < nevt; evt++) {
    fread(data, 4, 16, data_fp);
    
    for (i = 0; i < 10; i++)
      fprintf(text_fp, "%d, ", data[i]);
    fprintf(text_fp, "%d\n", data[10]);
  }
      
  fclose(data_fp);
  fclose(text_fp);

  return 0;
}

