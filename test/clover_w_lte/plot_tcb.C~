#include <stdio.h>

void plot_tcb(void)
{
  char filename[100];
  FILE *fp;
  int file_size;
  int nevt;
  int evt;
  char data[64];
  int data_length;
  int run_number;
  int trigger_type;
  int tcb_trigger_number;
  long long tcb_trigger_fine_time;
  long long tcb_trigger_coarse_time;
  float tcb_trigger_time;
  int mid;
  int trigger_pattern[9];
  int itmp;
  long long ltmp;
  int i;
  int ch;
  
  // filename here
  sprintf(filename, "clover_tcb_2.dat");

  // get data file size, size should be less than 2 Gbytes
  fp = fopen(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  fclose(fp);
  nevt = file_size / 64;          // 1 event = 64 bytes

  // define some histograms
  TCanvas *c1 = new TCanvas("c1", "TCB Timing", 1500, 900);
  c1->Divide(3, 4);
  TH1F *plot_type = new TH1F("plot_type", "Trigger Type", nevt, 0, nevt);
  TH1F *plot_number = new TH1F("plot_number", "Trigger Number", nevt, 0, nevt);
  TH1F *plot_time = new TH1F("plot_time", "Trigger Time", nevt, 0, nevt);
  TH1F *plot_ch1 = new TH1F("plot_ch1", "Trigger Pattern Ch1", nevt, 0, nevt);
  TH1F *plot_ch2 = new TH1F("plot_ch2", "Trigger Pattern Ch2", nevt, 0, nevt);
  TH1F *plot_ch3 = new TH1F("plot_ch3", "Trigger Pattern Ch3", nevt, 0, nevt);
  TH1F *plot_ch4 = new TH1F("plot_ch4", "Trigger Pattern Ch4", nevt, 0, nevt);
  TH1F *plot_ch5 = new TH1F("plot_ch5", "Trigger Pattern Ch5", nevt, 0, nevt);
  TH1F *plot_ch6 = new TH1F("plot_ch6", "Trigger Pattern Ch6", nevt, 0, nevt);
  TH1F *plot_ch7 = new TH1F("plot_ch7", "Trigger Pattern Ch7", nevt, 0, nevt);
  TH1F *plot_ch8 = new TH1F("plot_ch8", "Trigger Pattern Ch8", nevt, 0, nevt);
  TH1F *plot_ch9 = new TH1F("plot_ch9", "Trigger Pattern Ch9", nevt, 0, nevt);
  plot_type->Reset();
  plot_number->Reset();
  plot_time->Reset();
  plot_ch1->Reset();
  plot_ch2->Reset();
  plot_ch3->Reset();
  plot_ch4->Reset();
  plot_ch5->Reset();
  plot_ch6->Reset();
  plot_ch7->Reset();
  plot_ch8->Reset();
  plot_ch9->Reset();
  plot_type->SetStats(0);
  plot_number->SetStats(0);
  plot_time->SetStats(0);
  plot_ch1->SetStats(0);
  plot_ch2->SetStats(0);
  plot_ch3->SetStats(0);
  plot_ch4->SetStats(0);
  plot_ch5->SetStats(0);
  plot_ch6->SetStats(0);
  plot_ch7->SetStats(0);
  plot_ch8->SetStats(0);
  plot_ch9->SetStats(0);
  
  // open data file  
  fp = fopen(filename, "rb");

  for (evt = 0; evt < nevt; evt++) {  
    // read data
    fread(data, 1, 64, fp);
    
    // get header information
    data_length = data[0] & 0xFF;
    itmp = data[1] & 0xFF;
    itmp = itmp << 8;
    data_length = data_length + itmp;
    itmp = data[2] & 0xFF;
    itmp = itmp << 16;
    data_length = data_length + itmp;
    itmp = data[3] & 0xFF;
    itmp = itmp << 24;
    data_length = data_length + itmp;

    run_number = data[4] & 0xFF;
    itmp = data[5] & 0xFF;
    itmp = itmp << 8;
    run_number = run_number + itmp;

    trigger_type = data[6] & 0xFF;

    tcb_trigger_number = data[7] & 0xFF;
    itmp = data[8] & 0xFF;
    itmp = itmp << 8;
    tcb_trigger_number = tcb_trigger_number + itmp;
    itmp = data[9] & 0xFF;
    itmp = itmp << 16;
    tcb_trigger_number = tcb_trigger_number + itmp;
    itmp = data[10] & 0xFF;
    itmp = itmp << 24;
    tcb_trigger_number = tcb_trigger_number + itmp;

    tcb_trigger_fine_time = data[11] & 0xFF;
    
    tcb_trigger_coarse_time = data[12] & 0xFF;
    ltmp = data[13] & 0xFF;
    ltmp = ltmp << 8;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + ltmp;
    ltmp = data[14] & 0xFF;
    ltmp = ltmp << 16;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + ltmp;
    ltmp = data[15] & 0xFF;
    ltmp = ltmp << 24;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + ltmp;
    ltmp = data[16] & 0xFF;
    ltmp = ltmp << 32;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + ltmp;
    ltmp = data[17] & 0xFF;
    ltmp = ltmp << 40;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + ltmp;

    tcb_trigger_time = tcb_trigger_coarse_time * 1000 + tcb_trigger_fine_time * 8;

    mid = data[18] & 0xFF;

    for (ch = 0; ch < 9; ch++) {
      trigger_pattern[ch] = data[4 * ch + 19] & 0xFF;
      itmp = data[4 * ch + 20] & 0xFF;
      itmp = itmp << 8;
      trigger_pattern[ch] = trigger_pattern[ch] + itmp;
      itmp = data[4 * ch + 21] & 0xFF;
      itmp = itmp << 16;
      trigger_pattern[ch] = trigger_pattern[ch] + itmp;
      itmp = data[4 * ch + 22] & 0xFF;
      itmp = itmp << 24;
      trigger_pattern[ch] = trigger_pattern[ch] + itmp;
    }

    plot_type->Fill(evt, trigger_type);
    plot_number->Fill(evt, tcb_trigger_number);
    plot_time->Fill(evt, tcb_trigger_time);
    plot_ch1->Fill(evt, trigger_pattern[0]);
    plot_ch2->Fill(evt, trigger_pattern[1]);
    plot_ch3->Fill(evt, trigger_pattern[2]);
    plot_ch4->Fill(evt, trigger_pattern[3]);
    plot_ch5->Fill(evt, trigger_pattern[4]);
    plot_ch6->Fill(evt, trigger_pattern[5]);
    plot_ch7->Fill(evt, trigger_pattern[6]);
    plot_ch8->Fill(evt, trigger_pattern[7]);
    plot_ch9->Fill(evt, trigger_pattern[8]);
    
    printf("%d / %d event is filled\n", evt + 1, nevt);
  }

  fclose(fp);
    
  c1->cd(1);
  plot_type->Draw("hist");
  c1->cd(2);
  plot_number->Draw("hist");
  c1->cd(3);
  plot_time->Draw("hist");
  c1->cd(4);
  plot_ch1->Draw("hist");
  c1->cd(5);
  plot_ch2->Draw("hist");
  c1->cd(6);
  plot_ch3->Draw("hist");
  c1->cd(7);
  plot_ch4->Draw("hist");
  c1->cd(8);
  plot_ch5->Draw("hist");
  c1->cd(9);
  plot_ch6->Draw("hist");
  c1->cd(10);
  plot_ch7->Draw("hist");
  c1->cd(11);
  plot_ch8->Draw("hist");
  c1->cd(12);
  plot_ch9->Draw("hist");
  c1->Modified();
  c1->Update();
}


