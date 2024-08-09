#ifndef MINITCB_V2_H
#define MINITCB_V2_H

#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

#ifdef __cplusplus
extern  "C" {
#endif

#define cvA24_U_DATA  0x39
#define cvA24_U_BLT   0x3B
#define cvD16         0x2

/*
extern int TIAM_VMEopen(int sid);
extern void TIAM_VMEclose(int vme_Handle);
extern void TIAM_VMEread_DATA(int vme_Handle, unsigned long mid, int data_size, char *data);
extern unsigned long TIAM_VMEread_DATA_SIZE(int vme_Handle, unsigned long mid);
extern unsigned long TIAM_VMEread_RUN(int vme_Handle, unsigned long mid);
extern unsigned long TIAM_VMEread_RUNNO(int vme_Handle, unsigned long mid);
extern void TIAM_VMEwrite_ENABLE_PASS(int vme_Handle, unsigned long mid, unsigned long data);
extern unsigned long TIAM_VMEread_ENABLE_PASS(int vme_Handle, unsigned long mid);
*/

extern int TIAM_TCPopen(char *ip);
extern void TIAM_TCPclose(int tcp_Handle);
extern void TIAM_TCPread_DATA(int tcp_Handle, int data_size, char *data);
extern unsigned long TIAM_TCPread_DATA_SIZE(int tcp_Handle);
extern unsigned long TIAM_TCPread_RUN(int tcp_Handle);
extern unsigned long TIAM_TCPread_RUNNO(int tcp_Handle);
extern void TIAM_TCPwrite_ENABLE_PASS(int tcp_Handle, unsigned long data);
extern unsigned long TIAM_TCPread_ENABLE_PASS(int tcp_Handle);

#ifdef __cplusplus
}
#endif

#endif
