/* RECORDING WON'T WORK PROPERLY ON WINDOWS BECAUSE OF
 * errno = WSAGetLastError();
 */

#ifndef	_WINDOWS
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#if defined(__hpux) || defined(_WINDOWS)
#include <time.h>
#else
#include <sys/time.h>
#endif

#if defined(_WINDOWS)
#include "../client/NT/winClient.h"
#include "NT/winNet.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#ifndef	_WINDOWS
#include <netdb.h>
#endif

#ifdef __sgi
#include <bstring.h>
#endif

#ifdef	_WINDOWS
#undef	va_start		/* there are bad versions in windows.h's "stdarg.h" */
#undef	va_end
#include <varargs.h>
#endif

#include "srecord.h"
#include "version.h"
#include "config.h"
#include "const.h"
#include "error.h"
#include "net.h"
#include "packet.h"
#include "bit.h"
#include "socklib.h"


int sock_closeRec(sock_t *sock)
{
    return sock_close(sock); /* no recording code checks this value */
}


int sock_connectRec(sock_t *sock, char *host, int port)
{
    int i;

    if (playback) {
	i = *playback_ints++;
	if (i < 0)
	    errno = *playback_errnos++;
	return i;
    }
    i = sock_connect(sock, host, port);
    if (record) {
	*(playback_ints++) = i;
	if (i<0)
	    *playback_errnos++ = i;
    }
    return i;
}


int sock_get_last_portRec(sock_t *sock)
{
    int i;

    if (playback)
	return *(playback_ints++);
    i = sock_get_last_port(sock);
    if (record)
	*playback_ints++ = i;
    return i;
}


int sock_receive_anyRec(sock_t *sock, char *rbuf, int size)
{
    int i;

    if (playback) {
	i = *(playback_shorts++);
	if (i > 0) {
	    memcpy(rbuf, playback_data, i);
	    playback_data += i;
	}
	else
	    errno = *playback_errnos++;
	return i;
    }
    i = sock_receive_any(sock, rbuf, size);
    if (record) {
	*(playback_shorts++) = i;
	if (i > 0) {
	    memcpy(playback_data, rbuf, i);
	    playback_data += i;
	}
	else
	    *playback_errnos++ = errno;
    }
    return i;
}


int sock_readRec(sock_t *sock, char *rbuf, int size)
{
    int i;

    if (playback) {
	i = *(playback_shorts++);
	if (i > 0) {
	    memcpy(rbuf, playback_data, i);
	    playback_data += i;
	}
	else
	    errno = *playback_errnos++;
	return i;
    }
    i = sock_read(sock, rbuf, size);
    if (record) {
	*(playback_shorts++) = i;
	if (i > 0) {
	    memcpy(playback_data, rbuf, i);
	    playback_data += i;
	}
	else
	    *playback_errnos++ = errno;
    }
    return i;
}


int sock_writeRec(sock_t *sock, char *wbuf, int size)
{
    int i;

    if (playback) {
	return size;
/*
  errno = *(playback_ints++);
  return *(playback_ints++);
*/
    }
    i = sock_write(sock, wbuf, size);
    if (record) {
	/*
	 *(playback_ints++) = errno;
	 *(playback_ints++) = i;
	 */
    }
    if (record && i < size)
	error("Warning: DgramWrite failed, recording doesn't handle this");
    return i;
}


int sock_get_errorRec(sock_t *sock)
{
    int i;

    if (playback) {
	errno = *(playback_errnos++);
	return *(playback_ints++);
    }
    i = sock_get_error(sock);
    if (record) {
	*(playback_errnos++) = errno;
	*(playback_ints++) = i;
    }
    return i;
}


int Sockbuf_flushRec(sockbuf_t *sbuf)
{
    int			len,
	i;

    if (BIT(sbuf->state, SOCKBUF_WRITE) == 0) {
	errno = 0;
	error("No flush on non-writable socket buffer");
	error("(state=%02x,buf=%08x,ptr=%08x,size=%d,len=%d,sock=%d)",
	      sbuf->state, sbuf->buf, sbuf->ptr, sbuf->size, sbuf->len,
	      sbuf->sock);
	return -1;
    }
    if (BIT(sbuf->state, SOCKBUF_LOCK) != 0) {
	errno = 0;
	error("No flush on locked socket buffer (0x%02x)", sbuf->state);
	return -1;
    }
    if (sbuf->len <= 0) {
	if (sbuf->len < 0) {
	    errno = 0;
	    error("Write socket buffer length negative");
	    sbuf->len = 0;
	    sbuf->ptr = sbuf->buf;
	}
	return 0;
    }

    if (BIT(sbuf->state, SOCKBUF_DGRAM) != 0) {
	errno = 0;
	i = 0;
	while ((len = sock_writeRec(&sbuf->sock, sbuf->buf, sbuf->len)) <= 0) {
	    if (len == 0
		|| errno == EWOULDBLOCK
		|| errno == EAGAIN) {
		Sockbuf_clear(sbuf);
		return 0;
	    }
	    if (errno == EINTR) {
		errno = 0;
		continue;
	    }
	    if (++i > MAX_SOCKBUF_RETRIES) {
		error("Can't send on socket (%d,%d)", sbuf->sock, sbuf->len);
		Sockbuf_clear(sbuf);
		return -1;
	    }
	    { static int send_err;
	    if ((send_err++ & 0x3F) == 0) {
		error("send (%d)", i);
	    }
	    }
	    if (sock_get_errorRec(&sbuf->sock) == -1) {
		error("sock_get_error send");
		return -1;
	    }
	    errno = 0;
	}
	if (len != sbuf->len) {
	    errno = 0;
	    error("Can't write complete datagram (%d,%d)", len, sbuf->len);
	}
	Sockbuf_clear(sbuf);
    } else {
	errno = 0;
	while ((len = sock_writeRec(&sbuf->sock, sbuf->buf, sbuf->len)) <= 0) {
	    if (errno == EINTR) {
		errno = 0;
		continue;
	    }
	    if (errno != EWOULDBLOCK
		&& errno != EAGAIN) {
		error("Can't write on socket");
		return -1;
	    }
	    return 0;
	}
	Sockbuf_advance(sbuf, len);
    }
    return len;
}


int Sockbuf_readRec(sockbuf_t *sbuf)
{
    int			max,
	i,
	len;

    if (BIT(sbuf->state, SOCKBUF_READ) == 0) {
	errno = 0;
	error("No read from non-readable socket buffer (%d)", sbuf->state);
	return -1;
    }
    if (BIT(sbuf->state, SOCKBUF_LOCK) != 0) {
	return 0;
    }
    if (sbuf->ptr > sbuf->buf) {
	Sockbuf_advance(sbuf, sbuf->ptr - sbuf->buf);
    }
    if ((max = sbuf->size - sbuf->len) <= 0) {
	static int before;
	if (before++ == 0) {
	    errno = 0;
	    error("Read socket buffer not big enough (%d,%d)",
		  sbuf->size, sbuf->len);
	}
	return -1;
    }
    if (BIT(sbuf->state, SOCKBUF_DGRAM) != 0) {
	errno = 0;
	i = 0;
	while ((len = sock_readRec(&sbuf->sock, sbuf->buf + sbuf->len, max)) <= 0) {
	    if (len == 0) {
		return 0;
	    }
#ifdef _WINDOWS
	    errno = WSAGetLastError();
#endif
	    if (errno == EINTR) {
		errno = 0;
		continue;
	    }
	    if (errno == EWOULDBLOCK
		|| errno == EAGAIN) {
		return 0;
	    }
	    if (++i > MAX_SOCKBUF_RETRIES) {
		error("Can't recv on socket");
		return -1;
	    }
	    { static int recv_err;
	    if ((recv_err++ & 0x3F) == 0) {
		error("recv (%d)", i);
	    }
	    }
	    if (sock_get_errorRec(&sbuf->sock) == -1) {
		error("sock_get_error recv");
		return -1;
	    }
	    errno = 0;
	}
	sbuf->len += len;
    } else {
	errno = 0;
	while ((len = sock_readRec(&sbuf->sock, sbuf->buf + sbuf->len, max)) <= 0) {
	    if (len == 0) {
		return 0;
	    }
	    if (errno == EINTR) {
		errno = 0;
		continue;
	    }
	    if (errno != EWOULDBLOCK
		&& errno != EAGAIN) {
		error("Can't read on socket");
		return -1;
	    }
	    return 0;
	}
	sbuf->len += len;
    }

    return sbuf->len;
}


int Sockbuf_writeRec(sockbuf_t *sbuf, char *buf, int len)
{
    if (BIT(sbuf->state, SOCKBUF_WRITE) == 0) {
	errno = 0;
	error("No write to non-writable socket buffer");
	return -1;
    }
    if (sbuf->size - sbuf->len < len) {
	if (BIT(sbuf->state, SOCKBUF_LOCK | SOCKBUF_DGRAM) != 0) {
	    errno = 0;
	    error("No write to locked socket buffer (%d,%d,%d,%d)",
		  sbuf->state, sbuf->size, sbuf->len, len);
	    return -1;
	}
	if (Sockbuf_flushRec(sbuf) == -1) {
	    return -1;
	}
	if (sbuf->size - sbuf->len < len) {
	    return 0;
	}
    }
    memcpy(sbuf->buf + sbuf->len, buf, len);
    sbuf->len += len;

    return len;
}
