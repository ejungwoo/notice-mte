#include <stdio.h>

void plot_spectrum(void)
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
  
  // filename here
  sprintf(filename, "clover_10.dat");

  // define some histograms
  TCanvas *c1 = new TCanvas("c1", "TOF DAQ spectform", 1500, 600);
  c1->Divide(5, 2);
  TH1F *spect1 = new TH1F("spect1", "ch1", 4096, 0, 32768);
  TH1F *spect2 = new TH1F("spect2", "ch2", 4096, 0, 32768);
  TH1F *spect3 = new TH1F("spect3", "ch3", 4096, 0, 32768);
  TH1F *spect4 = new TH1F("spect4", "ch4", 4096, 0, 32768);
  TH1F *spect5 = new TH1F("spect5", "ch5", 4096, 0, 32768);
  TH1F *spect6 = new TH1F("spect6", "ch6", 4096, 0, 32768);
  TH1F *spect7 = new TH1F("spect7", "ch7", 4096, 0, 32768);
  TH1F *spect8 = new TH1F("spect8", "ch8", 4096, 0, 32768);
  TH1F *spect9 = new TH1F("spect9", "ch9", 4096, 0, 32768);
  TH1F *spect10 = new TH1F("spect10", "ch10", 4096, -200, 32768);
  spect1->Reset();
  spect2->Reset();
  spect3->Reset();
  spect4->Reset();
  spect5->Reset();
  spect6->Reset();
  spect7->Reset();
  spect8->Reset();
  spect9->Reset();
  spect10->Reset();
  
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
   
    // if data size is 128, it is fast data, otherwise skip them
    if (data_length == 128) {
      for (evt = 0; evt < 64; evt++) {
        // read header
        if (evt) 
          fread(header, 1, 32, fp);
          
        for (ch = 1; ch <=10; ch++) {
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
          
          if (ch == 1)
            spect1->Fill(peak);
          else if (ch == 2)
            spect2->Fill(peak);
          else if (ch == 3)
            spect3->Fill(peak);
          else if (ch == 4)
            spect4->Fill(peak);
          else if (ch == 5)
            spect5->Fill(peak);
          else if (ch == 6)
            spect6->Fill(peak);
          else if (ch == 7)
            spect7->Fill(peak);
          else if (ch == 8)
            spect8->Fill(peak);
          else if (ch == 9)
            spect9->Fill(peak);
          else if (ch == 10)
            spect10->Fill(peak);
        }
        
        // read empty data
        fread(data, 1, 46, fp);
      }

      c1->cd(1); spect1->Draw();        
      c1->cd(2); spect2->Draw();        
      c1->cd(3); spect3->Draw();        
      c1->cd(4); spect4->Draw();        
      c1->cd(5); spect5->Draw();        
      c1->cd(6); spect6->Draw();        
      c1->cd(7); spect7->Draw();        
      c1->cd(8); spect8->Draw();        
      c1->cd(9); spect9->Draw();        
      c1->cd(10); spect10->Draw();        
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


