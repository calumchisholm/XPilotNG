#include "xpclient.h"
#include "SDL.h"
#include "SDL_thread.h"

extern int Process_event(SDL_Event *evt);

static SDL_mutex *selectorMutex;
static SDL_cond *selectorCond;

static int Selector(void *data)
{
    fd_set              rfds;
    int                 n, netfd;
    struct timeval      tv;
    SDL_Event           evt;

    if ((netfd = Net_fd()) == -1) {
        evt.type = SDL_USEREVENT + 1;
        evt.user.data1 = "Bad net fd";
        SDL_PushEvent(&evt);
        return 0;
    }
    while(1) {
        FD_ZERO(&rfds);
        FD_SET(netfd, &rfds);
        tv.tv_sec = 15;
        tv.tv_usec = 0;
		SDL_mutexP(selectorMutex);
        n = select(netfd + 1, &rfds, NULL, NULL, &tv);
		if (n == -1) {
            evt.type = SDL_USEREVENT + 1;
            evt.user.data1 = "Select failed";
            SDL_PushEvent(&evt);
			SDL_mutexV(selectorMutex);
            return 0;
        }
        if (n == 0) {
            evt.type = SDL_USEREVENT + 1;
            evt.user.data1 = "Timeout";
            SDL_PushEvent(&evt);
			SDL_mutexV(selectorMutex);
            return 0;
        }
		evt.type = SDL_USEREVENT;
		SDL_PushEvent(&evt);
		/* The docs for SDL_CondWait say that it releases the given mutex
		 * but at least on windows 98 it does not seem to do so */
		SDL_mutexV(selectorMutex);
		SDL_CondWait(selectorCond, selectorMutex);
    }
}


void Game_loop(void)
{
    SDL_Event evt;
	SDL_Thread *selector;

	selectorMutex = SDL_CreateMutex();
	selectorCond = SDL_CreateCond();
	selector = SDL_CreateThread(Selector, NULL);

	while (SDL_WaitEvent(&evt)) {
		switch(evt.type) {

		case SDL_USEREVENT:
			SDL_mutexP(selectorMutex);
			if (Net_input() == -1) {
				error("Bad net input, have a nice day");
				SDL_mutexV(selectorMutex);
				goto finally;
			}
			SDL_mutexV(selectorMutex);
			SDL_CondSignal(selectorCond);
			break;

		case SDL_USEREVENT + 1:
			error("Error in selector thread: %s", evt.user.data1);
			goto finally;

		default:
			if (Process_event(&evt) == 0) {
				goto finally;
			}
			break;
		}
	}

finally:
	SDL_KillThread(selector);
	SDL_DestroyMutex(selectorMutex);
	SDL_DestroyCond(selectorCond);
}
