/*
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *      Kimiko Koopman       <guido@xpilot.org>
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
 * Esound audio driver.
 */

#include "xpclient.h"

char audio_version[] = VERSION;
static int audioserver;

int audioDeviceInit(char *display)
{
    /*printf("esound audio: init\n");*/
    audioserver = esd_open_sound("");

    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    if (((int *)private)[0] == 0) {
	/* sample has not been cached yet */
	((int *)private)[0] = esd_file_cache(audioserver, "", filename);
    }
    /*printf("esound audio: play file %s, type %d, vol %d, priv %d\n",
	   filename, type, volume, ((int *)private)[0]);*/

    volume = (int)(volume * 2.55);
    esd_set_default_sample_pan(audioserver, ((int *)private)[0],
			       volume, volume);
    esd_sample_play(audioserver, ((int *)private)[0]);
}

void audioDeviceEvents(void)
{
    /* printf("esound audio: events\n"); */
}
