/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2004 by
 *
 *      Uoti Urpala          <uau@users.sourceforge.net>
 *      Erik Andersson
 *      Kristian Söderblom
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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

#include "xpserver.h"

char sched_version[] = VERSION;

bool sched_running = false;

static long	timer_freq;
static void	(*timer_handler)(void);
static double	frametime;	/* time between 2 frames in seconds */
static double	t_nextframe;

static inline double timeval_to_seconds(struct timeval tv)
{
    return (double)tv.tv_sec + tv.tv_usec * 1e-6;
}

static inline struct timeval seconds_to_timeval(double t)
{
    struct timeval tv;

    tv.tv_sec = (unsigned)t;
    tv.tv_usec = (unsigned)(((t - (double)tv.tv_sec) * 1e6) + 0.5);

    return tv;
}

static void setup_timer(void)
{
    struct timeval tv;
    double t;

    if (timer_freq <= 0 || timer_freq > MAX_SERVER_FPS) {
	error("illegal timer frequency: %ld", timer_freq);
	exit(1);
    }

    frametime = 1.0 / (double)timer_freq;

    gettimeofday(&tv, NULL);
    t = timeval_to_seconds(tv);
    t_nextframe = t + frametime;
}

/*
 * Configure timer tick callback.
 */
void install_timer_tick(void (*func)(void), int freq)
{
    if (func != NULL) /* NULL to change freq, keep same handler */
	timer_handler = func;
    timer_freq = freq;
    setup_timer();
}

#define NUM_SELECT_FD		((int)sizeof(int) * 8)

struct io_handler {
    int			fd;
    void		(*func)(int, void *);
    void		*arg;
};

static struct io_handler	input_handlers[NUM_SELECT_FD];
static struct io_handler	record_handlers[NUM_SELECT_FD];
static fd_set			input_mask;
int				max_fd, min_fd;
static int			input_inited = false;

static void io_dummy(int fd, void *arg)
{
    xpprintf("io_dummy called!  (%d, %p)\n", fd, arg);
}

void install_input(void (*func)(int, void *), int fd, void *arg)
{
    int i;
    static struct io_handler *handlers;

    if (playback) {
	handlers = record_handlers;
	fd += min_fd;
    }
    else
	handlers = input_handlers;

    if (input_inited == false) {
	input_inited = true;
	FD_ZERO(&input_mask);
	min_fd = fd;
	max_fd = fd;
	for (i = 0; i < NELEM(input_handlers); i++) {
	    input_handlers[i].fd = -1;
	    input_handlers[i].func = io_dummy;
	    input_handlers[i].arg = 0;
	}
    }
    if (!playback && (fd < min_fd || fd >= min_fd + NUM_SELECT_FD)) {
	error("install illegal input handler fd %d (%d)", fd, min_fd);
	ServerExit();
    }
    if (!playback && FD_ISSET(fd, &input_mask)) {
	error("input handler %d busy", fd);
	ServerExit();
    }
    handlers[fd - min_fd].fd = fd;
    handlers[fd - min_fd].func = func;
    handlers[fd - min_fd].arg = arg;
    if (playback)
	return;
    FD_SET(fd, &input_mask);
    if (fd > max_fd)
	max_fd = fd;
}

void remove_input(int fd)
{
    if (!playback) {
	if (fd < min_fd || fd >= min_fd + NUM_SELECT_FD) {
	    error("remove illegal input handler fd %d (%d)", fd, min_fd);
	    ServerExit();
	}
	if (FD_ISSET(fd, &input_mask) || playback) {
	    input_handlers[fd - min_fd].fd = -1;
	    input_handlers[fd - min_fd].func = io_dummy;
	    input_handlers[fd - min_fd].arg = 0;
	    FD_CLR(fd, &input_mask);
	    if (fd == max_fd) {
		int i = fd;
		max_fd = -1;
		while (--i >= min_fd) {
		    if (FD_ISSET(i, &input_mask)) {
			max_fd = i;
			break;
		    }
		}
	    }
	}
    }
    else {
	record_handlers[fd].fd = -1;
	record_handlers[fd].func = io_dummy;
	record_handlers[fd].arg = 0;
    }
}

void stop_sched(void)
{
    sched_running = false;
}

static void sched_select_error(void)
{
    error("sched select error");
    End_game();
}



/*
 * If you set skip_to the server calculates frames
 * until that value of main_loops as fast as it can.
 * If you manage to record a bug, you can got to the
 * frame where it happens quickly.
 */
unsigned long skip_to = 0;

/*
 * I/O + timer dispatcher.
 */
void sched(void)
{
    int i, n;
    double t_now, t_wait;
    struct timeval tv, wait_tv;

    playback = rplayback;

    if (sched_running) {
	error("sched already running");
	exit(1);
    }

    sched_running = true;
    gettimeofday(&tv, NULL);
    t_now = timeval_to_seconds(tv);
    t_nextframe = t_now + frametime;

    while (sched_running) {
	fd_set readmask = input_mask;

	gettimeofday(&tv, NULL);
	t_now = timeval_to_seconds(tv);
	t_wait = t_nextframe - t_now;

	/* heuristics for different cases */
	if (t_wait < 0) {
	    if (t_wait < -2 * frametime) {
		/* long freeze, schedule frame now */
		t_nextframe = t_now;
		t_wait = 0;
	    } else
		t_wait = 0;
	} else {
	    if (t_wait > 2 * frametime) {
		/* nasty, someone changed the time! might aswell start over */
		t_nextframe = t_now + frametime;
		t_wait = frametime;
	    }
	}

	if (main_loops < skip_to) {
	    t_nextframe = t_now;
	    t_wait = 0;
	}
	
	/* RECORDING STUFF */
	Handle_recording_buffers();
	/* RECORDING STUFF END */

	wait_tv = seconds_to_timeval(t_wait);
	n = select(max_fd + 1, &readmask, NULL, NULL, &wait_tv);

	if (n <= 0) {
	    if (n == -1 && errno != EINTR)
		sched_select_error();

	    /* RECORDING STUFF */
	    if (playback) {
		while (*playback_sched) {
		    if (*playback_sched == 127) {
			playback_sched++;
			Get_recording_data();
		    }
		    else {
			struct io_handler *ioh;
			ioh = &record_handlers[*playback_sched++ - 1];
			(*(ioh->func))(ioh->fd, ioh->arg);
		    }
		}
		playback_sched++;
	    }
	    else if (record)
		*playback_sched++ = 0;
	    /* RECORDING STUFF END */

	    {
		struct timeval tv1, tv2;
		double t1, t2;

		gettimeofday(&tv1, NULL);
		if (timer_handler)
		    (*timer_handler)();
		gettimeofday(&tv2, NULL);
		t1 = timeval_to_seconds(tv1);
		t2 = timeval_to_seconds(tv2);
		options.mainLoopTime = (t2 - t1) * 1e3;
	    }

	    t_nextframe += frametime;
	}
	else {
	    for (i = max_fd; i >= min_fd; i--) {
		if (FD_ISSET(i, &readmask)) {
		    struct io_handler *ioh;

		    /* RECORDING STUFF */
		    record = playback = 0;
		    if (rrecord && (i - min_fd > 0)) {
			if (i - min_fd + 1 > 126) { /* 127 reserved */
			    warn("recording: this shouldn't happen");
			    exit(1);
			}
			*playback_sched++ = i - min_fd + 1;
			record = 1;
		    }
		    /* RECORDING STUFF END */

		    ioh = &input_handlers[i - min_fd];
		    (*(ioh->func))(ioh->fd, ioh->arg);

		    /* RECORDING STUFF */
		    record = rrecord;
		    playback = rplayback;
		    /* RECORDING STUFF END */

		    if (--n == 0)
			break;
		}
	    }
	}
    }
}
