/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	PROTOCLIENT_H
#define	PROTOCLIENT_H

#include "connectparam.h"
#include "option.h"

/*
 * about.c
 */
extern int Handle_motd(long off, char *buf, int len, long filesize);
extern void aboutCleanup(void);

#ifdef _WINDOWS
extern	void Motd_destroy(void);
extern	void Keys_destroy(void);
#endif

extern int motd_viewer;		/* so Windows can clean him up */
extern int keys_viewer;


extern void Colors_init_style_colors(void);

/*
 * default.c
 */
extern void Store_default_options(void);
extern void defaultCleanup(void);			/* memory cleanup */

extern bool Set_scaleFactor(xp_option_t *opt, double val);
extern bool Set_altScaleFactor(xp_option_t *opt, double val);

#ifdef _WINDOWS
extern char *Get_xpilotini_file(int level);
#endif

/*
 * event.c
 */
extern void Store_key_options(void);

/*
 * join.c
 */
extern int Join(Connect_param_t *conpar);

/*
 * mapdata.c
 */
extern int Mapdata_setup(const char *);


/*
 * metaclient.c
 */
extern int metaclient(int, char **);


/*
 * paintdata.c
 */
extern void paintdataCleanup(void);		/* memory cleanup */


/*
 * paintobjects.c
 */
extern int Init_wreckage(void);
extern int Init_asteroids(void);


/*
 * query.c
 */
extern int Query_all(sock_t *sockfd, int port, char *msg, int msglen);



/*
 * textinterface.c
 */
extern int Connect_to_server(int auto_connect, int list_servers,
			     int auto_shutdown, char *shutdown_reason,
			     Connect_param_t *conpar);
extern int Contact_servers(int count, char **servers,
			   int auto_connect, int list_servers,
			   int auto_shutdown, char *shutdown_message,
			   int find_max, int *num_found,
			   char **server_addresses, char **server_names,
			   Connect_param_t *conpar);

/*
 * usleep.c
 */
extern int micro_delay(unsigned usec);

/*
 * welcome.c
 */
extern int Welcome_screen(Connect_param_t *conpar);

/*
 * widget.c
 */
extern void Widget_cleanup(void);

/*
 * xinit.c
 */
#ifdef _WINDOWS
extern	void WinXCreateItemBitmaps();
#endif

/*
 * winX - The Windows X emulator
 */
#ifdef _WINDOWS
#define	WinXFlush(__w)	WinXFlush(__w)
#else
#define	WinXFlush(__w)
#endif


#endif	/* PROTOCLIENT_H */


