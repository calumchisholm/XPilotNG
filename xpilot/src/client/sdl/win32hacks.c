/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client.
 *
 * Copyright (C) 2003-2004 Juha Lindstr�m <juhal@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

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