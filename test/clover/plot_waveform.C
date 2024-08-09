#include <stdio.h>

void plot_waveform(void)
{
  char filename[100];
  FILE *fp;
  int file_size;
  int data_read;
  char header[32];
  short data[4096];
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
  int itmp;
  long long ltmp;
  int i;
  int cont;
  
  // filename here
  sprintf(filename, "clover_0.dat");

  // define some histograms
  TCanvas *c1 = new TCanvas("c1", "TOF DAQ waveform", 1500, 600);
  c1->Divide(5, 2);
  TH1F *wave1 = new TH1F("wave1", "ch1", 4080, 0, 4080 * 8);
  TH1F *wave2 = new TH1F("wave2", "ch2", 4080, 0, 4080 * 8);
  TH1F *wave3 = new TH1F("wave3", "ch3", 4080, 0, 4080 * 8);
  TH1F *wave4 = new TH1F("wave4", "ch4", 4080, 0, 4080 * 8);
  TH1F *wave5 = new TH1F("wave5", "ch5", 4080, 0, 4080 * 8);
  TH1F *wave6 = new TH1F("wave6", "ch6", 4080, 0, 4080 * 8);
  TH1F *wave7 = new TH1F("wave7", "ch7", 4080, 0, 4080 * 8);
  TH1F *wave8 = new TH1F("wave8", "ch8", 4080, 0, 4080 * 8);
  TH1F *wave9 = new TH1F("wave9", "ch9", 4080, 0, 4080 * 8);
  TH1F *wave10 = new TH1F("wave10", "ch10", 4080, 0, 4080 * 8);
  wave1->Reset();
  wave2->Reset();
  wave3->Reset();
  wave4->Reset();
  wave5->Reset();
  wave6->Reset();
  wave7->Reset();
  wave8->Reset();
  wave9->Reset();
  wave10->Reset();
  wave1->SetStats(0);
  wave2->SetStats(0);
  wave3->SetStats(0);
  wave4->SetStats(0);
  wave5->SetStats(0);
  wave6->SetStats(0);
  wave7->SetStats(0);
  wave8->SetStats(0);
  wave9->SetStats(0);
  wave10->SetStats(0);
  
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

    ch = header[16] & 0xFF;

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
   
    // if data size is 8192, it is waveform data, otherwise skip them
    if (data_length == 8192) {
      fread(data, 2, 4080, fp);
      if (ch == 1) {
        wave1->Reset();
        for (i = 0; i < 4080; i++)
          wave1->Fill(i * 8, data[i]);
        c1->cd(1);
        wave1->Draw("hist");
      }
      else if (ch == 2) {
        wave2->Reset();
        for (i = 0; i < 4080; i++)
          wave2->Fill(i * 8, data[i]);
        c1->cd(2);
        wave2->Draw("hist");
      }
      else if (ch == 3) {
        wave3->Reset();
        for (i = 0; i < 4080; i++)
          wave3->Fill(i * 8, data[i]);
        c1->cd(3);
        wave3->Draw("hist");
      }
      else if (ch == 4) {
        wave4->Reset();
        for (i = 0; i < 4080; i++)
          wave4->Fill(i * 8, data[i]);
        c1->cd(4);
        wave4->Draw("hist");
      }
      else if (ch == 5) {
        wave5->Reset();
        for (i = 0; i < 4080; i++)
          wave5->Fill(i * 8, data[i]);
        c1->cd(5);
        wave5->Draw("hist");
      }
      else if (ch == 6) {
        wave6->Reset();
        for (i = 0; i < 4080; i++)
          wave6->Fill(i * 8, data[i]);
        c1->cd(6);
        wave6->Draw("hist");
      }
      else if (ch == 7) {
        wave7->Reset();
        for (i = 0; i < 4080; i++)
          wave7->Fill(i * 8, data[i]);
        c1->cd(7);
        wave7->Draw("hist");
      }
      else if (ch == 8) {
        wave8->Reset();
        for (i = 0; i < 4080; i++)
          wave8->Fill(i * 8, data[i]);
        c1->cd(8);
        wave8->Draw("hist");
      }
      else if (ch == 9) {
        wave9->Reset();
        for (i = 0; i < 4080; i++)
          wave9->Fill(i * 8, data[i]);
        c1->cd(9);
        wave9->Draw("hist");
      }
      else if (ch == 10) {
        wave10->Reset();
cont = 0;        
for (i = 0; i < 4080; i++) 
cont = cont + data[i];
//cont = cont / 1020;
printf("ave = %d\n", cont);
        for (i = 0; i < 4080; i++) 
          wave10->Fill(i * 8, data[i]);
        c1->cd(10);
        wave10->Draw("hist");
      }
      
      c1->Modified();
      c1->Update();

      data_read = data_read + 8192;
      printf("continue : ");
      scanf("%d", &cont);
      if (!cont)
        data_read = file_size;
    }
    else {
      fread(data, 2, 4080, fp);
      data_read = data_read + 8192;
      printf("skipped!!\n");
    }
  }
  
  fclose(fp);
  
}


