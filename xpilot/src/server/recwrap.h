#ifndef RECWRAP_H
#define RECWRAP_H

#include "net.h"

/* int CreateDgramSocketWrap(int port); */
void DgramCloseRec(int fd);
int DgramConnectRec(int fd, char *host, int port);
/* char *DgramLastaddrWrap(void); */
int DgramLastportRec(void);
int DgramReceiveAnyRec(int fd, char *rbuf, int size);
/* int DgramSendWrap(int fd, char *host, int port, char *sbuf, int size); */
int DgramWriteRec(int fd, char *wbuf, int size);
int GetSocketErrorRec(int fd);
/* int SetSocketNonBlockingWrap(int fd, int flag); */
/* int SetSocketSendBufferSizeWrap(int fd, int size); */
/* int SetSocketReceiveBufferSizeWrap(int fd, int size); */
int sendRec(int s, const void *msg, int len, unsigned int flags);
int Sockbuf_flushRec(sockbuf_t *sbuf);
int Sockbuf_writeRec(sockbuf_t *sbuf, char *buf, int len);
int Sockbuf_readRec(sockbuf_t *sbuf);

#endif  /* RECWRAP_H */
