#include "xpclient.h"
#include "SDL.h"

extern int Process_event(SDL_Event *evt);

void Game_loop(void)
{
    fd_set              rfds;
    int                 n, netfd;
    struct timeval      tv;
	SDL_Event			evt;

    if ((netfd = Net_fd()) == -1) {
        error("Bad net fd");
        return;
    }
    while(1) {
        FD_ZERO(&rfds);
        FD_SET(netfd, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 5000; // wait max 5 ms
        n = select(netfd + 1, &rfds, NULL, NULL, &tv);
		if (n == -1) {
            error("Select failed");
            break;
        }
		if (n > 0) {
			if (Net_input() == -1) {
				error("Bad net input");
				break;
			}
		}
		while(SDL_PollEvent(&evt)) 
			Process_event(&evt);
    }
}
