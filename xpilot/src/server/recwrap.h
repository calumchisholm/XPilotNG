#ifndef RECWRAP_H
#define RECWRAP_H

#include "net.h"

int sock_closeRec(sock_t *sock);
int sock_connectRec(sock_t *sock, char *host, int port);
int sock_get_last_portRec(sock_t *sock);
int sock_receive_anyRec(sock_t *sock, char *rbuf, int size);
int sock_readRec(sock_t *sock, char *rbuf, int size);
int sock_writeRec(sock_t *sock, char *wbuf, int size);
int sock_get_errorRec(sock_t *sock);
int Sockbuf_flushRec(sockbuf_t *sbuf);
int Sockbuf_writeRec(sockbuf_t *sbuf, char *buf, int len);
int Sockbuf_readRec(sockbuf_t *sbuf);

#endif  /* RECWRAP_H */
