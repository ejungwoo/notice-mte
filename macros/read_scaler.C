volatile sig_atomic_t signalInterrupted = 0;
void signalHandler(int signum) {
    printf("Interrupt signal (%d) received.\n", signum);
        signalInterrupted = 1;
}

void read_scaler(int run_number=204, bool always_break=true, double threshold1=0, double threshold2=DBL_MAX)
{
    TString pathData = "data";
    const int numChannels = 16;
    vector<int> lfChannels = {0,1,2};
    bool debugging = true;
    bool drawOnScreen = true;
    double sleepForSec = 2;

    FILE *fileScaler;
    unsigned int file_size = 0L;
    unsigned int file_size_old = 0L;
    int countsInEvent[numChannels];
    double countSumInEvent[numChannels] = {0};
    double countSOSInEvent[numChannels] = {0};
    int iEventLast = 0;

    if (always_break==false)
        signal(SIGINT, signalHandler);

    TCanvas *cvs = nullptr;
    if (drawOnScreen) {
        if (lfChannels.size()>9) { cvs = new TCanvas("cvs","",1500,800); cvs -> Divide(4,3); }
        if (lfChannels.size()>6) { cvs = new TCanvas("cvs","",1500,800); cvs -> Divide(4,2); }
        if (lfChannels.size()>4) { cvs = new TCanvas("cvs","",1500,800); cvs -> Divide(3,2); }
        else                     { cvs = new TCanvas("cvs","",1500,800); cvs -> Divide(2,2); }
    }

    TString fnConfig = Form("%s/run_config.txt",pathData.Data());
    TString fnScaler = Form("%s/lte_count_%d.dat",pathData.Data(),run_number);
    TString fnSummary = Form("summary/summary_%d.dat",run_number);
    cout << "== Reading config file" << endl;
    cout << fnConfig << endl;

    ifstream fileConfig(fnConfig);
    if (fileConfig.is_open()==false) {
        cout << "==ERROR== Cannot open " << fnConfig << endl;
    }
    else {
        int run_number_;
        double threshold1_, threshold2_;
        while (fileConfig >> run_number_ >> threshold1_ >> threshold2_) {
            cout << run_number_ << " " << threshold1_ << " " << threshold2_ << endl;
            if (run_number==run_number_ && threshold1_>=0 && threshold2_>=0) {
                threshold1 = threshold1_;
                threshold2 = threshold2_;
                break;
            }
        }
    }

    cout << "== Reading input file" << endl;
    cout << fnScaler << endl;
    cout << "== Threshold is " << threshold1 << " " << threshold2 << endl;

    TGraph* graphScaler[numChannels];
    for (auto iChannel=0; iChannel<numChannels; ++iChannel)
    {
        graphScaler[iChannel] = new TGraph();
        auto graph = graphScaler[iChannel];
        graph -> SetMarkerStyle(20);
        graph -> SetMarkerColor(iChannel+1);
        graph -> SetLineColor(iChannel+1);
    }

    while (true)
    {
        ////////////////////////////////////////
        // find file size and number of events
        fileScaler = fopen(fnScaler, "rb");
        if (fileScaler==NULL) {
            cout << "==ERROR== Cannot open " << fnScaler << endl;
            break;
        }
        fseek(fileScaler, 0L, SEEK_END);
        file_size = ftell(fileScaler);
        if (debugging) cout << "== File size is " << file_size << " (prev. " << file_size_old << ")" << endl;
        if (file_size==file_size_old)
        {
            cout << "waiting for " << sleepForSec << " seconds ... Press (Ctrl+C) to end reading" << endl;
            usleep(sleepForSec*1000000);
            if (signalInterrupted)
                break;
            continue;
        }
        fclose(fileScaler);
        int numEvents = file_size/64;

        ////////////////////////////////////////
        // find file size and number of events
        fileScaler = fopen(fnScaler, "rb");
        fseek(fileScaler, file_size_old, SEEK_SET);
        file_size_old = file_size;
        for (auto iEvent=iEventLast; iEvent<numEvents; ++iEvent)
        {
            fread(countsInEvent,4,16,fileScaler);
            int eventNumber = countsInEvent[0];
            if (debugging) cout << "Event " << iEvent << " : ";
            for (auto iChannel : lfChannels)
            {
                auto graph = graphScaler[iChannel];
                double count = double(countsInEvent[iChannel+1]);
                if (count>threshold1 && count<threshold2) {
                    countSumInEvent[iChannel] += count;
                    countSOSInEvent[iChannel] += count*count;
                    auto numPoints = graph->GetN();
                    graph -> SetPoint(numPoints, eventNumber, count);
                    numPoints++;
                    double countMean = countSumInEvent[iChannel] / numPoints;
                    double countStdv = sqrt(countSOSInEvent[iChannel]/numPoints - countMean*countMean);
                    if (debugging) cout << iChannel << ">" << count << "(" << int(countMean) << "+-" << countStdv << ")  ";
                }
            }
            if (debugging) cout << endl;
        }
        iEventLast = numEvents;

        if (drawOnScreen)
        {
            int iCvs = 1;
            for (auto iChannel : lfChannels)
            {
                cvs -> cd(iCvs++);
                auto graph = graphScaler[iChannel];
                graph -> Draw("apl");
                int numValidEvents = graph -> GetN();
                double countMean = countSumInEvent[iChannel] / numValidEvents;
                double countStdv = sqrt(countSOSInEvent[iChannel]/numValidEvents - countMean*countMean);
                graph -> GetHistogram() -> SetTitle(Form("[%d] [Ch %d] count = %.0f #pm %.0f", run_number, iChannel+1, countMean, countStdv));
            }
            cvs -> Modified();
            cvs -> Update();
        }

        if (signalInterrupted || always_break) {
            for (auto iChannel : lfChannels)
            {
                int numValidEvents = graphScaler[iChannel] -> GetN();
                double countMean = countSumInEvent[iChannel] / numValidEvents;
                double countStdv = sqrt(countSOSInEvent[iChannel]/numValidEvents - countMean*countMean);
                cout << "== Ch-" << iChannel << " = " << int(countMean) << " +- " << countStdv << endl;
            }
            break;
        }
    }
}
