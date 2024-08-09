#include <stdio.h>

void plot_spectrum32(void)
{
  char filename[100];
  FILE *fp;
  int file_size;
  int data_read;
  char data[8192];
  int data_length;
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
  char plot_name[32];
  char plot_title[32];
  
  // filename here
  sprintf(filename, "clover_adc_1.dat");

  // define some histograms
  TCanvas *c1 = new TCanvas("c1", "TOF DAQ waveform", 1600, 800);
  c1->Divide(8, 4);
  plot = new TH1F *[32];
  for (ch = 0; ch < 32; ch++) {
    sprintf(plot_name, "plot%d", ch);
    sprintf(plot_title, "ch %d", ch + 1);
    plot[ch] = new TH1F(plot_name, plot_title, 4096, 0, 65536);
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
    // read data
    fread(data, 1, 32, fp);
    
    // get data information
    data_length = data[0] & 0xFF;

    if (data_length == 32) {
      trigger_type = data[1] & 0xFF;

      tcb_trigger_number = data[2] & 0xFF;
      itmp = data[3] & 0xFF;
      itmp = itmp << 8;
      tcb_trigger_number = tcb_trigger_number + itmp;
      itmp = data[4] & 0xFF;
      itmp = itmp << 16;
      tcb_trigger_number = tcb_trigger_number + itmp;
      itmp = data[5] & 0xFF;
      itmp = itmp << 24;
      tcb_trigger_number = tcb_trigger_number + itmp;

      tcb_trigger_fine_time = data[6] & 0xFF;
    
      tcb_trigger_coarse_time = data[7] & 0xFF;
      ltmp = data[8] & 0xFF;
      ltmp = ltmp << 8;
      tcb_trigger_coarse_time = tcb_trigger_coarse_time + itmp;
      ltmp = data[9] & 0xFF;
      ltmp = ltmp << 16;
      tcb_trigger_coarse_time = tcb_trigger_coarse_time + itmp;
    
      tcb_trigger_time = tcb_trigger_coarse_time * 1000 + tcb_trigger_fine_time * 8;

      mid = data[10] & 0xFF;

      ch = data[11] & 0xFF;

      local_trigger_number = data[12] & 0xFF;
      itmp = data[13] & 0xFF;
      itmp = itmp << 8;
      local_trigger_number = local_trigger_number + itmp;
      itmp = data[14] & 0xFF;
      itmp = itmp << 16;
      local_trigger_number = local_trigger_number + itmp;
      itmp = data[15] & 0xFF;
      itmp = itmp << 24;
      local_trigger_number = local_trigger_number + itmp;

      local_trigger_pattern = data[16] & 0xFF;
      itmp = data[17] & 0xFF;
      itmp = itmp << 8;
      local_trigger_pattern = local_trigger_pattern + itmp;
      itmp = data[18] & 0xFF;
      itmp = itmp << 16;
      local_trigger_pattern = local_trigger_pattern + itmp;
      itmp = data[19] & 0xFF;
      itmp = itmp << 24;
      local_trigger_pattern = local_trigger_pattern + itmp;

      local_gate_fine_time = data[20] & 0xFF;
    
      local_gate_coarse_time = data[21] & 0xFF;
      ltmp = data[22] & 0xFF;
      ltmp = ltmp << 8;
      local_gate_coarse_time = local_gate_coarse_time + itmp;
      ltmp = data[23] & 0xFF;
      ltmp = ltmp << 16;
      local_gate_coarse_time = local_gate_coarse_time + itmp;
      ltmp = data[24] & 0xFF;
      ltmp = ltmp << 24;
      local_gate_coarse_time = local_gate_coarse_time + itmp;
      ltmp = data[25] & 0xFF;
      ltmp = ltmp << 32;
      local_gate_coarse_time = local_gate_coarse_time + itmp;
      ltmp = data[26] & 0xFF;
      ltmp = ltmp << 40;
      local_gate_coarse_time = local_gate_coarse_time + itmp;

      local_gate_time = local_gate_coarse_time * 1000 + local_gate_fine_time * 8;
      
      peak = data[27] & 0xFF;
      itmp = data[28] & 0xFF;
      itmp = itmp << 8;
      peak = peak + itmp;

      timing = data[29] & 0xFF;
      itmp = data[30] & 0xFF;
      itmp = itmp << 8;
      timing = timing + itmp;
          
      flag = data[31] & 0xFF;    // pile_up flag @ bit1, hit flag @ bit0
      
      plot[ch - 1]->Fill(peak);

      data_read = data_read + 32;
    }
    else {
      fread(data, 1, 8160, fp);
      data_read = data_read + 8192;
    }      
    
    printf("data_read = %d/%d\n", data_read, file_size);
  }
   
  for (ch = 1; ch <= 32; ch++) {
    c1->cd(ch); 
    plot[ch - 1]->Draw();      
  }  
  c1->Modified();
  c1->Update();
  
  fclose(fp);
  
}


