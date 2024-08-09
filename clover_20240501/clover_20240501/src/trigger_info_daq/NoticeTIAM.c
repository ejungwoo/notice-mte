#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

//#include "nk6uvme.h"
#include "NoticeTIAM.h"

// CAEN VME function prototype, later replaced by real CAEN library
/*
void CAENVME_Init(int BdType, short ConetNode, int LinkNum_or_PID, int *Handle);
void CAENVME_End(int Handle);
void CAENVME_WriteCycle(int Handle, unsigned long Address, unsigned long *Data, int AM, int DW);
void CAENVME_ReadCycle(int Handle, unsigned long Address, unsigned long *Data, int AM, int DW);
void CAENVME_BLTReadCycle(int Handle, unsigned long Address, char *Buffer,
                         int Size, int AM, int DW, int *count);
*/

// TCPIP internal functions
int TIAM_TCPtransmit(int tcp_Handle, char *buf, int len);
int TIAM_TCPreceive(int tcp_Handle, char *buf, int len);
void TIAM_TCPwrite(int tcp_Handle, unsigned long address, unsigned long data);
unsigned long TIAM_TCPread(int tcp_Handle, unsigned long address);
void TIAM_TCPreadBLK(int tcp_Handle, unsigned long address, int nbyte, char *data);

/*
// open CAEN VME
void CAENVME_Init(int BdType, short ConetNode, int LinkNum_or_PID, int *Handle)
{
  VMEopen(LinkNum_or_PID);
  *Handle = LinkNum_or_PID;
}

// close CAEN VME
void CAENVME_End(int Handle)
{
  VMEclose(Handle);
}

// write to VME
void CAENVME_WriteCycle(int Handle, unsigned long Address, unsigned long *Data, int AM, int DW)
{
  unsigned char vme_am;
  
  if (DW == 0x02)
    vme_am = AM | 0x40;
  else
    vme_am = AM;
  
  VMEwrite(Handle, vme_am, 100, Address, *Data);
}

// read from VME
void CAENVME_ReadCycle(int Handle, unsigned long Address, unsigned long *Data, int AM, int DW)
{
  unsigned char vme_am;
  
  if (DW == 0x02)
    vme_am = AM | 0x40;
  else
    vme_am = AM;

  *Data = VMEread(Handle, vme_am, 100, Address);
}

// block read from VME
void CAENVME_BLTReadCycle(int Handle, unsigned long Address, char *Buffer,
                         int Size, int AM, int DW, int *count)
{
  unsigned char vme_am;
  
  if (DW == 0x02)
    vme_am = AM | 0x40;
  else
    vme_am = AM;

  VMEblockread(Handle, vme_am, 100, Address, Size, Buffer);
  *count = Size;
}     
*/                    

// transmit characters to TIAM_TCP
int TIAM_TCPtransmit(int tcp_Handle, char *buf, int len)
{
  int result;
  int bytes_more;
  int  bytes_xferd;
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

// receive characters from TIAM_TCP
int TIAM_TCPreceive(int tcp_Handle, char *buf, int len)
{
  int result;
  int accum;
  int space_left;
  int bytes_more;
  int buf_count;
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

// write 2byte to TIAM_TCP
void TIAM_TCPwrite(int tcp_Handle, unsigned long address, unsigned long data)
{
  char tcpBuf[4];

  tcpBuf[0] = 1;
  tcpBuf[1] = address & 0xFF;
  tcpBuf[2] = data & 0xFF;
  tcpBuf[3] = (data >> 8) & 0xFF;

  TIAM_TCPtransmit(tcp_Handle, tcpBuf, 4);
  
  TIAM_TCPreceive(tcp_Handle, tcpBuf, 1);
}

// read 2byte from TIAM_TCP
unsigned long TIAM_TCPread(int tcp_Handle, unsigned long address)
{
  char tcpBuf[2];
  unsigned long data;

  tcpBuf[0] = 2;
  tcpBuf[1] = address & 0xFF;

  TIAM_TCPtransmit(tcp_Handle, tcpBuf, 2);
  
  TIAM_TCPreceive(tcp_Handle, tcpBuf, 2);

  data = tcpBuf[1] & 0xFF;
  data = data << 8;
  data = data + (tcpBuf[0] & 0xFF);

  return data;
}

// read nbytes from TIAM_TCP
void TIAM_TCPreadBLK(int tcp_Handle, unsigned long address, int nbyte, char *data)
{
  char tcpBuf[4];

  tcpBuf[0] = 3;
  tcpBuf[1] = address & 0xFF;
  tcpBuf[2] = nbyte & 0xFF;
  tcpBuf[3] = (nbyte >> 8) & 0xFF;

  TIAM_TCPtransmit(tcp_Handle, tcpBuf, 4);
  
  TIAM_TCPreceive(tcp_Handle, data, nbyte);
}

// **************** end of internal functions ***************************

/*
// VME open TIAM
int TIAM_VMEopen(int sid)
{
  int Handle;

  CAENVME_Init(0, 0, sid, &Handle);
  return Handle;
}

// VME close TIAM
void TIAM_VMEclose(int vme_Handle)
{
  CAENVME_End(vme_Handle);
}

// VME read data
void TIAM_VMEread_DATA(int vme_Handle, unsigned long mid, int data_size, char *data)
{
  unsigned long vme_addr;
  int count;

  vme_addr = (mid & 0xFF) << 16;
  
  CAENVME_BLTReadCycle(vme_Handle, vme_addr, data, data_size, cvA24_U_BLT, cvD16, &count);
}

// VME read data size
unsigned long TIAM_VMEread_DATA_SIZE(int vme_Handle, unsigned long mid)
{
  unsigned long vme_addr;
  unsigned long data;

  vme_addr = ((mid & 0xFF) << 16) | 0x8000;
  data = 0;

  // latch data size
  CAENVME_WriteCycle(vme_Handle, vme_addr, &data, cvA24_U_DATA, cvD16);

  // read data size
  CAENVME_ReadCycle(vme_Handle, vme_addr, &data, cvA24_U_DATA, cvD16);

  return data;
}

// VME read RUN status
unsigned long TIAM_VMEread_RUN(int vme_Handle, unsigned long mid)
{
  unsigned long vme_addr;
  unsigned long data;

  vme_addr = ((mid & 0xFF) << 16) | 0x8002;

  CAENVME_ReadCycle(vme_Handle, vme_addr, &data, cvA24_U_DATA, cvD16);

  return data;
}

// VME read run number
unsigned long TIAM_VMEread_RUNNO(int vme_Handle, unsigned long mid)
{
  unsigned long vme_addr;
  unsigned long data;

  vme_addr = ((mid & 0xFF) << 16) | 0x8004;

  CAENVME_ReadCycle(vme_Handle, vme_addr, &data, cvA24_U_DATA, cvD16);

  return data;
}

// VME enable/disable LTE link pass, 0 = disable, 1 = enable
void TIAM_VMEwrite_ENABLE_PASS(int vme_Handle, unsigned long mid, unsigned long data)
{
  unsigned long vme_addr;
  unsigned long value;

  vme_addr = ((mid & 0xFF) << 16) | 0x8006;
  value = data;

  CAENVME_WriteCycle(vme_Handle, vme_addr, &value, cvA24_U_DATA, cvD16);
}

// VME read run number
unsigned long TIAM_VMEread_ENABLE_PASS(int vme_Handle, unsigned long mid)
{
  unsigned long vme_addr;
  unsigned long data;

  vme_addr = ((mid & 0xFF) << 16) | 0x8006;

  CAENVME_ReadCycle(vme_Handle, vme_addr, &data, cvA24_U_DATA, cvD16);

  return data;
}
*/

// TCP open TIAM
int TIAM_TCPopen(char *ip)
{
  struct sockaddr_in serv_addr;
  int tcp_Handle;
  const int disable = 1;
        
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port        = htons(5000);
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open TIAM_TCP\n");
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

// TCP close TIAM
void TIAM_TCPclose(int tcp_Handle)
{
  close(tcp_Handle);
}

// TCP read data
void TIAM_TCPread_DATA(int tcp_Handle, int data_size, char *data)
{
  int nkbyte;
  int nbyte;
  int i;
  
  nkbyte = data_size / 1024;
  nbyte = data_size % 1024;

  for (i = 0; i < nkbyte; i++)  
    TIAM_TCPreadBLK(tcp_Handle, 0x0, 1024, data + i * 1024);
  
  if (nbyte)
    TIAM_TCPreadBLK(tcp_Handle, 0x0, nbyte, data + nkbyte * 1024);
}

// TCP read data size
unsigned long TIAM_TCPread_DATA_SIZE(int tcp_Handle)
{
  // latch data size
  TIAM_TCPwrite(tcp_Handle, 0x80, 0);

  // read data size
  return TIAM_TCPread(tcp_Handle, 0x80);
}

// TCP read RUN status
unsigned long TIAM_TCPread_RUN(int tcp_Handle)
{
  return TIAM_TCPread(tcp_Handle, 0x82);
}

// TCP read run number
unsigned long TIAM_TCPread_RUNNO(int tcp_Handle)
{
  return TIAM_TCPread(tcp_Handle, 0x84);
}

// TCP enable/disable LTE link pass, 0 = disable, 1 = enable
void TIAM_TCPwrite_ENABLE_PASS(int tcp_Handle, unsigned long data)
{
  TIAM_TCPwrite(tcp_Handle, 0x86, data);
}

// TCP read run number
unsigned long TIAM_TCPread_ENABLE_PASS(int tcp_Handle)
{
  return TIAM_TCPread(tcp_Handle, 0x86);
}


