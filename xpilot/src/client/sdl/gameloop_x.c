#include <X11/Xlib.h>

#include "SDL.h"
#include "SDL_version.h"
#include "SDL_syswm.h"

#include "xpclient.h"

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
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if ((n = select(max + 1, &rfds, NULL, NULL, &tv)) == -1) {
	    if (errno == EINTR)
		continue;
	    error("Select failed");
	    return;
	}
	if (n == 0) {
	    if (result <= 1) {
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
