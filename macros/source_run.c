#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "NoticeLTE.h"

volatile sig_atomic_t stopLoop = 0;

void signalHandler(int signum) {
    printf("Interrupt signal (%d) received.\n", signum);
    stopLoop = 1;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);

    int sid=0;
    int run_number;
    int acq_time=0;
    unsigned long link_status;
    int tcb;
    int tcb_exist;
    int mid;
    char tag_filename[100];
    char count_filename[100];
    FILE *tag_fp;
    FILE *count_fp;
    unsigned long run;
    unsigned long tag_data_size;
    unsigned long count_data_size;
    int tag_evt;
    int count_evt;
    char *tag_data;
    char *count_data;

    if (argc == 2) {
        run_number = atoi(argv[1]);
    }
    else if (argc > 3) {
        sid = atoi(argv[1]);
        run_number = atoi(argv[2]);
        acq_time = atoi(argv[3]);
    }
    else {
        printf("Enter LTE sid : 0\n");
        //scanf("%d", &sid);
        printf("Enter run number(0~65535) : ");
        scanf("%d", &run_number);
        printf("Enter acquisition time(0~1,000,000s, 0 for infinite) : 0\n");
        //scanf("%d", &acq_time);
    }

    // init usb
    USB3Init();

    // open LTE
    LTEopen(sid);

    // reset DAQ
    LTEreset(sid);

    // get link status and TCB sid
    link_status = LTEread_LINK_STATUS(sid);
    for (tcb = 0; tcb < 9; tcb++) {
        tcb_exist = link_status & (1 << tcb);
        if (tcb_exist) {
            mid = LTEread_MID(sid, tcb);
            printf("TCB%d is linked @%d\n", mid, tcb + 1);
        }
    }

    // set run number
    LTEwrite_RUN_NUMBER(sid, (unsigned long)run_number);
    printf("run_number = %ld\n", LTEread_RUN_NUMBER(sid));

    // set acquisition time
    LTEwrite_ACQUISITION_TIME(sid, (unsigned long)acq_time);
    printf("acquisition time = %ld s\n", LTEread_ACQUISITION_TIME(sid));

    // reset timer if necessary
    //  LTEresetTIMER(sid_lte);  

    // open file
    sprintf(tag_filename, "data/lte_tag_%d.dat", run_number);
    sprintf(count_filename, "data/lte_count_%d.dat", run_number);
    //tag_fp = fopen(tag_filename, "wb");
    //count_fp = fopen(count_filename, "wb");
    tag_evt = 0;
    count_evt = 0;

    // assign data array
    tag_data = (char *)malloc(65536); 
    count_data = (char *)malloc(65536); 

    // start DAQ
    LTEstart(sid);
    run = LTEread_RUN(sid);
    printf("Run = %ld\n", run);

    int counter=0;
    while (run) {
        // just for debugging
        //    LTEsend_TRIG(sid_lte);
        if (counter==0) {
            tag_fp = fopen(tag_filename, "wb");
            count_fp = fopen(count_filename, "wb");
            counter = 1;
        }
        else {
            tag_fp = fopen(tag_filename, "awb");
            count_fp = fopen(count_filename, "awb");
        }
        // check tag data size
        tag_data_size = LTEread_TAG_DATA_SIZE(sid);

        if (tag_data_size) {
            // read tag data
            LTEread_TAG_DATA(sid, tag_data_size, tag_data);

            // write to charge data file
            fwrite(tag_data, 1, tag_data_size * 4, tag_fp);

            // add tag event number
            tag_evt = tag_evt + tag_data_size / 4;    // 1 event = 16 bytes
            printf("tag data : %d events are taken\n", tag_evt);
        }

        // check count data size
        count_data_size = LTEread_COUNT_DATA_SIZE(sid);

        if (count_data_size) {
            // read count data
            LTEread_COUNT_DATA(sid, count_data_size, count_data);

            // write to file
            fwrite(count_data, 1, count_data_size * 4, count_fp);

            // add count event number
            count_evt = count_evt + count_data_size / 16;  // 1 event = 64 bytes
            printf("count data : %d events are taken\n", count_evt);
        }

        fclose(tag_fp);
        fclose(count_fp);

        // check run
        run = LTEread_RUN(sid);

        if (stopLoop) {
            LTEstop(sid);
            break;} // loop out
    }

    printf("LTE is stopped and read remaining data\n");

    // read remaining data
    // check tag data size
    tag_data_size = LTEread_TAG_DATA_SIZE(sid);

    if (tag_data_size) {
        // read tag data
        LTEread_TAG_DATA(sid, tag_data_size, tag_data);

        // write to charge data file
        fwrite(tag_data, 1, tag_data_size * 4, tag_fp);

        // add tag event number
        tag_evt = tag_evt + tag_data_size / 4;    // 1 event = 16 bytes
        printf("tag data : %d events are taken\n", tag_evt);
    }

    tag_fp = fopen(tag_filename, "awb");
    count_fp = fopen(count_filename, "awb");

    // check count data size
    count_data_size = LTEread_COUNT_DATA_SIZE(sid);

    if (count_data_size) {
        // read count data
        LTEread_COUNT_DATA(sid, count_data_size, count_data);

        // write to file
        fwrite(count_data, 1, count_data_size * 4, count_fp);

        // add count event number
        count_evt = count_evt + count_data_size / 16;  // 1 event = 64 bytes
        printf("count data : %d events are taken\n", count_evt);
    }

    // release data array
    free(tag_data);
    free(count_data);

    // close file
    fclose(tag_fp);  
    fclose(count_fp);  

    // close LTE
    LTEclose(sid);

    // close usb
    USB3Exit();

    return 0;
}


