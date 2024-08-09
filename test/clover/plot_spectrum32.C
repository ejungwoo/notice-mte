#include <stdio.h>

void plot_spectrum32(void)
{
  char filename[100];
  FILE *fp;
  int file_size;
  int data_read;
  char header[32];
  char data[8192];
  int data_length;
  int run_number;
  int trigger_type;
  int tcb_trigger_number;
  long long tcb_trigger_fine_time;
  long long tcb_trigger_coarse_time;
  long long tcb_trigger_time;
  int mid;
  int ch;
  int local_trigger_number;
  int local_trigger_pattern;
  long long local_gate_fine_time;
  long long local_gate_coarse_time;
  long long local_gate_time;
  int peak;
  int timing;
  int flag;
  int evt;
  int itmp;
  long long ltmp;
  int i;
  int cont;
  TH1F **plot;
  char plot_name[10];
  char plot_title[10];
  
  // filename here
  sprintf(filename, "clover_10.dat");

  // define some histograms
  TCanvas *c1 = new TCanvas("c1", "TOF DAQ waveform", 1600, 800);
  c1->Divide(8, 4);
  plot = new TH1F *[32];
  for (ch = 0; ch < 32; ch++) {
    sprintf(plot_name, "plot%d", ch);
    sprintf(plot_title, "ch %d", ch + 1);
    plot[ch] = new TH1F(plot_name, plot_title, 4096, 0, 32768);
    plot[ch]->Reset();
  }
  
  // get data file size, size should be less than 2 Gbytes
  fp = fopen(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  fclose(fp);

  // open data file  
  fp = fopen(filename, "rb");
  
  data_read = 0;
  
  while (data_read < file_size) {
    // read header
    fread(header, 1, 32, fp);
    
    // get header information
    data_length = header[0] & 0xFF;
    itmp = header[1] & 0xFF;
    itmp = itmp << 8;
    data_length = data_length + itmp;
    itmp = header[2] & 0xFF;
    itmp = itmp << 16;
    data_length = data_length + itmp;
    itmp = header[3] & 0xFF;
    itmp = itmp << 24;
    data_length = data_length + itmp;

    run_number = header[4] & 0xFF;
    itmp = header[5] & 0xFF;
    itmp = itmp << 8;
    run_number = run_number + itmp;

    trigger_type = header[6] & 0xFF;

    tcb_trigger_number = header[7] & 0xFF;
    itmp = header[8] & 0xFF;
    itmp = itmp << 8;
    tcb_trigger_number = tcb_trigger_number + itmp;
    itmp = header[9] & 0xFF;
    itmp = itmp << 16;
    tcb_trigger_number = tcb_trigger_number + itmp;
    itmp = header[10] & 0xFF;
    itmp = itmp << 24;
    tcb_trigger_number = tcb_trigger_number + itmp;

    tcb_trigger_fine_time = header[11] & 0xFF;
    
    tcb_trigger_coarse_time = header[12] & 0xFF;
    ltmp = header[13] & 0xFF;
    ltmp = ltmp << 8;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + itmp;
    ltmp = header[14] & 0xFF;
    ltmp = ltmp << 16;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + itmp;
    
    tcb_trigger_time = tcb_trigger_coarse_time * 1000 + tcb_trigger_fine_time * 8;

    mid = header[15] & 0xFF;

//    ch = header[16] & 0xFF;   ch is 0 for fast data

    local_trigger_number = header[17] & 0xFF;
    itmp = header[18] & 0xFF;
    itmp = itmp << 8;
    local_trigger_number = local_trigger_number + itmp;
    itmp = header[19] & 0xFF;
    itmp = itmp << 16;
    local_trigger_number = local_trigger_number + itmp;
    itmp = header[20] & 0xFF;
    itmp = itmp << 24;
    local_trigger_number = local_trigger_number + itmp;

    local_trigger_pattern = header[21] & 0xFF;
    itmp = header[22] & 0xFF;
    itmp = itmp << 8;
    local_trigger_pattern = local_trigger_pattern + itmp;
    itmp = header[23] & 0xFF;
    itmp = itmp << 16;
    local_trigger_pattern = local_trigger_pattern + itmp;
    itmp = header[24] & 0xFF;
    itmp = itmp << 24;
    local_trigger_pattern = local_trigger_pattern + itmp;

    local_gate_fine_time = header[25] & 0xFF;
    
    local_gate_coarse_time = header[26] & 0xFF;
    ltmp = header[27] & 0xFF;
    ltmp = ltmp << 8;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[28] & 0xFF;
    ltmp = ltmp << 16;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[29] & 0xFF;
    ltmp = ltmp << 24;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[30] & 0xFF;
    ltmp = ltmp << 32;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[31] & 0xFF;
    ltmp = ltmp << 40;
    local_gate_coarse_time = local_gate_coarse_time + itmp;

    local_gate_time = local_gate_coarse_time * 1000 + local_gate_fine_time * 8;
   
    // if data size is 256, it is fast data, otherwise skip them
    if (data_length == 256) {
      for (evt = 0; evt < 32; evt++) {
        // read header
        if (evt) 
          fread(header, 1, 32, fp);
          
        for (ch = 1; ch <=32; ch++) {
          fread(data, 1, 5, fp);
          peak = data[0] & 0xFF;
          itmp = data[1] & 0xFF;
          itmp = itmp << 8;
          peak = peak + itmp;

          timing = data[2] & 0xFF;
          itmp = data[3] & 0xFF;
          itmp = itmp << 8;
          timing = timing + itmp;
          
          flag = data[4] & 0xFF;
          
          plot[ch - 1]->Fill(peak);
        }
        
        // read empty data
        fread(data, 1, 64, fp);
      }

      for (ch = 1; ch <= 32; ch++) {
        c1->cd(ch); 
        plot[ch - 1]->Draw();      
      }  
      c1->Modified();
      c1->Update();
      
      data_read = data_read + 8192;
    }
    else {
      fread(data, 2, 4080, fp);
      data_read = data_read + 8192;
    }
  }
  
  fclose(fp);
  
}


