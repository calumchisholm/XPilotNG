/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
/*
 * This piece of code was provided by Greg Renda (greg@ncd.com), but
 * rplay support was originally done by Mark Boyns (boyns@sdsu.edu).
 */
/*
 * RPlay audio driver.
 */

#include "xpclient.h"
#include <rplay.h>

char audio_version[] = VERSION;

static int fd;


int audioDeviceInit(char *display)
{
    char host[MAX_DISP_LEN], *p;

    strlcpy(host, display, sizeof(host));

    if ((p = strrchr(host, ':')) != NULL)
	*p = 0;

    if (!*host)
	strcat(host, "localhost");

    printf("Directing sound to: %s\n", host);

    if ((fd = rplay_open(host)) < 0) {
	rplay_perror(host);
	return -1;
    }

    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    RPLAY **p = (RPLAY **)private;
    char *name;

    if (!*p) {
	if ((name = strrchr(filename, '/')) != NULL) {
	    name++;
	} else {
	    name = filename;
	}
	*p = rplay_create(RPLAY_PLAY);
	rplay_set(*p, RPLAY_INSERT, 0, RPLAY_SOUND, xp_strdup(name), NULL);
    }

    rplay_set(*p, RPLAY_CHANGE, 0, RPLAY_VOLUME, volume, NULL);
    rplay(fd, *p);
}

void audioDeviceEvents(void)
{
}
