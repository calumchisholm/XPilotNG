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

#include <X11/Xlib.h>

#include "xpclient_sdl.h"

extern int Process_event(SDL_Event *evt);

static int Poll_input(void)
{
    SDL_Event evt;
    while (SDL_PollEvent(&evt))
	if (Process_event(&evt) == 0) 
	    return 1;
    return 0;
}

/*
 * This is a Game_loop that uses X specific hacks to improve
 * responsiveness. Basically it uses the same mechanism as the 
 * X client to listen to network and user input.
 */
void Game_loop(void)
{
    fd_set		rfds;
    fd_set		tfds;
    int			max,
			n,
			netfd,
			result,
			clientfd;
    struct timeval	tv;
    SDL_SysWMinfo       info;
    long    	    	waitingtime;

    SDL_VERSION(&info.version);
    if (!SDL_GetWMInfo(&info)) {
	error("SDL_GetWMInfo not supported");
	return;
    }

    if ((result = Net_input()) == -1) {
	error("Bad server input");
	return;
    }
    if (Poll_input())
	return;

    if (Net_flush() == -1)
	return;

    if ((clientfd = ConnectionNumber(info.info.x11.display)) == -1) {
	error("Bad client filedescriptor");
	return;
    }
    if ((netfd = Net_fd()) == -1) {
	error("Bad socket filedescriptor");
	return;
    }
    Net_key_change();
    FD_ZERO(&rfds);
    FD_SET(clientfd, &rfds);
    FD_SET(netfd, &rfds);
    max = (clientfd > netfd) ? clientfd : netfd;
    for (tfds = rfds; ; rfds = tfds) {
	if (!movement_interval && mouseMovement) {
	    gettimeofday(&tv,NULL);
	    waitingtime = next_time.tv_usec - tv.tv_usec + 1000000*(next_time.tv_sec - tv.tv_sec);
    	    tv.tv_sec = 0;
	    if (waitingtime > 0) {
	    	tv.tv_usec = waitingtime%1000000;
	    } else {
	    	tv.tv_usec = 0;
	    }
	} else {
	    tv.tv_sec = 1;
	    tv.tv_usec = 500000;/*at maxFPS 1 you get lots of messages w/o this*/
	}
	if ((n = select(max + 1, &rfds, NULL, NULL, &tv)) == -1) {
	    if (errno == EINTR)
		continue;
	    error("Select failed");
	    return;
	}
	
	if (n == 0) {
	    if (!movement_interval && mouseMovement) {
	    	Send_pointer_move(mouseMovement);
		mouseMovement = 0;
		if (Net_flush() == -1) {
		    error("Bad net flush");
		    return;
		}
	    } else if (result <= 1) {
		warn("No response from server");
		continue;
	    }
	}
	if (FD_ISSET(clientfd, &rfds)) {
	    if (Poll_input()) return;
	    if (Net_flush() == -1) {
		error("Bad net flush after input");
		return;
	    }
	}
	if (FD_ISSET(netfd, &rfds) || result > 1) {
	    if ((result = Net_input()) == -1) {
		warn("Bad net input.  Have a nice day!");
		return;
	    }
	    if (result > 0) {
		if (Poll_input()) return;
		if (Net_flush() == -1) {
		    error("Bad net flush");
		    return;
		}
		if (Poll_input()) return;
	    }
	}
    }
}
