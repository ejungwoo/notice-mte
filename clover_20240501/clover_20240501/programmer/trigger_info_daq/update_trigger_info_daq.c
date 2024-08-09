#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

int TDC4CH_open(char *ip);
void TDC4CH_close(int tcp_Handle);
int TDC4CH_write(int tcp_Handle, char *buf, int len);
int TDC4CH_read(int tcp_Handle, char *buf, int len);
void TDC4CH_unprotect(int tcp_Handle);
void TDC4CH_protect(int tcp_Handle);
void TDC4CH_erase(int tcp_Handle, int sector);
void TDC4CH_program(int tcp_Handle, int sector, int page, char *data);
void TDC4CH_verify(int tcp_Handle, int sector, int page, char *data);

int TDC4CH_open(char *ip)
{
  struct sockaddr_in serv_addr;
  int tcp_Handle;
  const int disable = 1;
        
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port        = htons(5000);
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open TDC4CH\n");
    return -1;
  }

  setsockopt(tcp_Handle, IPPROTO_TCP,TCP_NODELAY,(char *) &disable, sizeof(disable)); 
  
  if (connect(tcp_Handle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("client: can't connect to server\n");
    printf("ip %s, port 5000 \n", ip);
    printf("error number is %d \n", connect(tcp_Handle, (struct sockaddr *) &serv_addr,sizeof(serv_addr)));

    return -2;
  } 
  
  return tcp_Handle;
}

void TDC4CH_close(int tcp_Handle)
{
  close(tcp_Handle);
}

int TDC4CH_write(int tcp_Handle, char *buf, int len)
{
  int result, bytes_more, bytes_xferd;
  char *idxPtr;

  bytes_more = len;
  idxPtr = buf;
  bytes_xferd = 0;

  while (1) {
    idxPtr = buf + bytes_xferd;
    result=write (tcp_Handle, (char *) idxPtr, bytes_more);

    if (result<0) {
      printf("Could not write the rest of the block successfully, returned: %d\n",bytes_more);
	
      return -1;
    }
    
    bytes_xferd += result;
    bytes_more -= result;
    
    if (bytes_more <= 0)
      break;
  }

  return 0;
}

int TDC4CH_read(int tcp_Handle, char *buf, int len)
{
  int result, accum, space_left, bytes_more, buf_count;
  char *idxPtr;

  fd_set rfds;
  struct timeval tval;

  tval.tv_sec = MAX_TCP_READ;
  tval.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(tcp_Handle, &rfds);

  if (buf==NULL)
    return -1;
    
  idxPtr = buf;

  buf_count = 0;
  space_left = len;
  while (1) {
    /* read the block */
    accum = 0;
    while (1) {
      idxPtr = buf + (buf_count + accum);
      bytes_more = space_left;
      
      if ((result = read(tcp_Handle, (char *) idxPtr, (bytes_more>2048)?2048:bytes_more)) < 0) {
	printf("Unable to receive data from the server.\n");
	return -1;
      }
      
      accum += result;
      if ((accum + buf_count) >= len)
	break;
      /* in case data is smaller than wanted on */
      if(result<bytes_more) {
	printf("wanted %d got %d \n",bytes_more,result);
	return accum+buf_count;
      }
    }
    
    buf_count += accum;
    space_left -= accum;

    if (space_left <= 0)
      break;
  }

  return buf_count;
}

void TDC4CH_unprotect(int tcp_Handle)
{
  char tcpBuf[2];
	
  tcpBuf[0] = 11;

  TDC4CH_write(tcp_Handle, tcpBuf, 1);
  TDC4CH_read(tcp_Handle, tcpBuf, 1);
}

void TDC4CH_protect(int tcp_Handle)
{
  char tcpBuf[2];
	
  tcpBuf[0] = 12;

  TDC4CH_write(tcp_Handle, tcpBuf, 1);
  TDC4CH_read(tcp_Handle, tcpBuf, 1);
}

void TDC4CH_erase(int tcp_Handle, int sector)
{
  char tcpBuf[2];
	
  tcpBuf[0] = 13;
  tcpBuf[1] = sector;

  TDC4CH_write(tcp_Handle, tcpBuf, 2);
  TDC4CH_read(tcp_Handle, tcpBuf, 1);
}

void TDC4CH_program(int tcp_Handle, int sector, int page, char *data)
{
  char tcpBuf[259];
  int i;
	
  tcpBuf[0] = 14;
  tcpBuf[1] = sector & 0xFF;
  tcpBuf[2] = page & 0xFF;
	
  for (i = 0; i < 256; i++)
    tcpBuf[i + 3] = data[i] & 0xFF;

  TDC4CH_write(tcp_Handle, tcpBuf, 259);
  TDC4CH_read(tcp_Handle, tcpBuf, 1);
}

void TDC4CH_verify(int tcp_Handle, int sector, int page, char *data)
{
  char tcpBuf[256];
  int i;
	
  tcpBuf[0] = 15;
  tcpBuf[1] = sector & 0xFF;
  tcpBuf[2] = page & 0xFF;
	
  TDC4CH_write(tcp_Handle, tcpBuf, 3);
  TDC4CH_read(tcp_Handle, tcpBuf, 256);
	
  for (i = 0; i < 256; i++)
    data[i] = tcpBuf[i] & 0xFF;
}

int main(int argc, char *argv[])
{
  int tcp_Handle;               
  char ip[100];
  int com;
  char filename[256];
  char rdata[256];
  FILE *fp;
  int file_size;
  int data_size;
  char *wdata;
  int i;
  int length;
  char name_data[256];
  int last_page;
  int page_all;
  int page_written;
  int sector;
  int page;
  int err_cnt;
  int ip_addr;
  
  if (argc > 1)
    sprintf(ip, "%s", argv[1]);
  else
    sprintf(ip, "192.168.0.2");

  tcp_Handle = TDC4CH_open(ip);
  
  com = 99;

  while(com) {
    printf("-------------------------------------\n");
    TDC4CH_verify(tcp_Handle, 6, 0, rdata);
    printf("VME address = %d\n", rdata[0] & 0xFF);
    TDC4CH_verify(tcp_Handle, 7, 0, rdata);
    printf("TCP/IP address = 192.168.0.%d\n", rdata[0] & 0xFF);
 
    TDC4CH_verify(tcp_Handle, 5, 255, rdata);
    printf("FPGA firmware = %s\n", rdata);

    printf("-------------------------------------\n");
    printf("1. upload FPGA firmware\n");
    printf("2. verify FPGA firmware\n");
    printf("3. update VME address\n");
    printf("4. update IP address\n");
    printf("0. quit\n");
    printf("enter command : ");
    scanf("%d", &com);

    if (com == 1) {
      printf("enter bit filename : ");
      scanf("%s", filename);
      
      fp = fopen(filename, "rb");
      if (fp == NULL)
	      printf("File not found! Quit without uploading.\n");
      else {
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        fclose(fp);
        
        data_size = file_size / 256;
        data_size = data_size * 256;
        
        if (file_size % 256)
          data_size = data_size + 256;
    
        wdata = (char *)malloc(data_size);
        
        fp = fopen(filename, "rb");
        fread(wdata, 1, data_size, fp);
        fclose(fp);
        
        for (i = file_size; i < data_size; i++)
          wdata[i] = 0;
          
        length = strlen(filename);
        
        for (i = 0; i < length; i++)
          name_data[i] = filename[i];
	
        for (i = length; i < 256; i++)
          name_data[i] = 0;
        
        last_page = data_size % 65536;
        last_page = last_page / 256;
        
        page_all = 5 * 256 + last_page + 1;
        page_written = 0;
        
        TDC4CH_unprotect(tcp_Handle);
        
        for (sector = 0; sector < 5; sector++) {
          TDC4CH_erase(tcp_Handle, sector);
          
          for (page = 0; page < 256; page++) {
            TDC4CH_program(tcp_Handle, sector, page, wdata + sector * 65536 + page * 256);
            page_written = page_written + 1;
            printf("%d / %d are written\n", page_written, page_all);
          }
        }
        
        sector = 5;
        TDC4CH_erase(tcp_Handle, sector);
          
        for (page = 0; page < last_page; page++) {
          TDC4CH_program(tcp_Handle, sector, page, wdata + sector * 65536 + page * 256);
          page_written = page_written + 1;
          printf("%d / %d are written\n", page_written, page_all);
        }

        page = 255;        
        TDC4CH_program(tcp_Handle, sector, page, name_data);
        page_written = page_written + 1;
        printf("%d / %d are written\n", page_written, page_all);

        TDC4CH_protect(tcp_Handle);
        free(wdata);
      }
    }

    else if (com == 2) {
      printf("enter bit filename : ");
      scanf("%s", filename);
      fp = fopen(filename, "rb");
      if (fp == NULL)
	      printf("File not found! Quit without uploading. Bye!\n");
      else {
        err_cnt = 0;

        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        fclose(fp);
        
        data_size = file_size / 256;
        data_size = data_size * 256;
        
        if (file_size % 256)
          data_size = data_size + 256;
    
        wdata = (char *)malloc(data_size);
        
        fp = fopen(filename, "rb");
        fread(wdata, 1, data_size, fp);
        fclose(fp);

        for (i = file_size; i < data_size; i++)
          wdata[i] = 0;
        
        length = strlen(filename);
        
        for (i = 0; i < length; i++)
	        name_data[i] = filename[i];
	
        for (i = length; i < 256; i++)
	        name_data[i] = 0;
        
        last_page = data_size % 65536;
        last_page = last_page / 256;
        
        page_all = 5 * 256 + last_page + 1;
        page_written = 0;
        
        for (sector = 0; sector < 5; sector++) {
          for (page = 0; page < 256; page++) {
            TDC4CH_verify(tcp_Handle, sector, page, rdata);
            for (i = 0; i < 256; i++) {
              if ((rdata[i] & 0xFF) != (wdata[sector * 65536 + page * 256 + i] & 0xFF)) {
                err_cnt = err_cnt + 1;
                printf("%X : %X : %X, write = %X, read = %X, err_cnt = %d\n", sector, page, i, wdata[sector * 65536 + page * 256 + i] & 0xFF, rdata[i] & 0xFF, err_cnt);
              }
              if (err_cnt >= 100) {
                printf("Too many errors!\n");
                sector = 5;
                page = 256;
              }
            }
            page_written = page_written + 1;
            printf("%d / %d are verified\n", page_written, page_all);
          }
        }
        
        if (err_cnt < 100) {
          sector = 5;
          for (page = 0; page < last_page; page++) {
            TDC4CH_verify(tcp_Handle, sector, page, rdata);
            for (i = 0; i < 256; i++) {
              if ((rdata[i] & 0xFF) != (wdata[sector * 65536 + page * 256 + i] & 0xFF)) {
                err_cnt = err_cnt + 1;
                printf("%X : %X : %X, write = %X, read = %X, err_cnt = %d\n", sector, page, i, wdata[sector * 65536 + page * 256 + i] & 0xFF, rdata[i] & 0xFF, err_cnt);
              }
              if (err_cnt >= 100) {
                printf("Too many errors!\n");
                sector = 5;
                page = 256;
              }
            }
            page_written = page_written + 1;
            printf("%d / %d are verified\n", page_written, page_all);
          }
        }

        if (err_cnt < 100) {
          sector = 5;
          page = 255;        
          TDC4CH_verify(tcp_Handle, sector, page, rdata);
          for (i = 0; i < 256; i++) {
            if ((rdata[i] & 0xFF) != (name_data[i] & 0xFF)) {
              err_cnt = err_cnt + 1;
              printf("%X : %X : %X, write = %X, read = %X, err_cnt = %d\n", sector, page, i, wdata[sector * 65536 + page * 256 + i] & 0xFF, rdata[i] & 0xFF, err_cnt);
            }
            if (err_cnt >= 100) 
              printf("Too many errors!\n");
          }
          page_written = page_written + 1;
          printf("%d / %d are verified\n", page_written, page_all);
        }

        free(wdata);
        
        printf("Error count = %d\n", err_cnt);
      }
    }
    
    else if (com == 3) {
      printf("enter VME address : ");
      scanf("%d", &ip_addr);
      wdata = (char *)malloc(256);
      wdata[0] = ip_addr & 0xFF;
      for (i = 1; i < 256; i++)
        wdata[i] = 0;
      TDC4CH_unprotect(tcp_Handle);
      TDC4CH_erase(tcp_Handle, 6);
      TDC4CH_program(tcp_Handle, 6, 0, wdata);
      TDC4CH_protect(tcp_Handle);
      free(wdata);
    }
    
    else if (com == 4) {
      printf("enter TCP/IP address : ");
      scanf("%d", &ip_addr);
      wdata = (char *)malloc(256);
      wdata[0] = ip_addr & 0xFF;
      for (i = 1; i < 256; i++)
        wdata[i] = 0;
      TDC4CH_unprotect(tcp_Handle);
      TDC4CH_erase(tcp_Handle, 7);
      TDC4CH_program(tcp_Handle, 7, 0, wdata);
      TDC4CH_protect(tcp_Handle);
      free(wdata);
    }
  }

  TDC4CH_close(tcp_Handle);

  return 0;
}


