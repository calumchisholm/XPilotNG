/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client.
 *
 * Copyright (C) 2003-2004 Juha Lindström <juhal@users.sourceforge.net>
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

#include "xpclient_sdl.h"

extern int Process_event(SDL_Event *evt);

#ifndef __GNUC__
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif

#ifndef HAVE_GETTIMEOFDAY
struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};

__inline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

    if (tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;

}
#endif /* HAVE_GETTIMEOFDAY */

void Game_loop(void)
{
    fd_set              rfds;
    int                 n, netfd;
    struct timeval      tv;
    SDL_Event	    	evt;
    long    	    	waitingtime;

    if ((netfd = Net_fd()) == -1) {
        error("Bad net fd");
        return;
    }
    while(1) {
        FD_ZERO(&rfds);
        FD_SET(netfd, &rfds);
    	tv.tv_sec = 0;
    	tv.tv_usec = 5000;/* wait max 5 ms */
        n = select(netfd + 1, &rfds, NULL, NULL, &tv);
    	if (n == -1) {
            error("Select failed");
            break;
        }
    	if (n > 0) {
	    gettimeofday(&tv,NULL);
	    waitingtime = next_time.tv_usec - tv.tv_usec + 1000000*(next_time.tv_sec - tv.tv_sec);
	    if ((waitingtime <= 0) && mouseMovement) {
	    	Send_pointer_move(mouseMovement);
		mouseMovement = 0;
		if (Net_flush() == -1) {
		    error("Bad net flush");
		    return;
		}
    	    }
	    if (Net_input() == -1) {
    	    	error("Bad net input");
    	    	break;
    	    }
    	}
    	while(SDL_PollEvent(&evt)) 
    	    Process_event(&evt);
    }
}
