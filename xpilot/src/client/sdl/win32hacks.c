#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

/* needed by some server specific function in socklib.c */
HWND notifyWnd;

/* these are not used by the SDL client */
/* but they are referred by the common windows code */
int ThreadedDraw;
BOOL drawPending;
int RadarDivisor;
struct {
	HANDLE eventNotDrawing;
} dinfo;


/* SDL client does not use MFC crap */
void _Trace(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void Progress(char *fmt, ...) {}
void MarkPlayersForRedraw(void) {}

int Winsock_init(void)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	
	/* I have no idea which version of winsock supports
	 * the required socket stuff. */
	wVersionRequested = MAKEWORD( 1, 0 );
	if (WSAStartup( wVersionRequested, &wsaData ))
		return -1;

	return 0;
}

void Winsock_cleanup(void)
{
	WSACleanup();
}