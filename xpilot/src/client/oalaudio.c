/*
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      The XPilot Authors   <xpilot@xpilot.org>
 *      Juha Lindström       <juhal@users.sourceforge.net>
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
 * OpenAL audio driver.
 */

#include "xpclient.h"
#ifndef _WINDOWS
#include <AL/al.h>
#include <AL/alut.h>
#else
#include <al.h>
#include <alut.h>
#endif

#define MAX_SOUNDS 16
#define VOL_THRESHOLD 10

char audio_version[] = VERSION;

typedef struct {
    ALuint    buffer;
    ALfloat   gain;
    ALboolean loop;
} sample_t;

typedef struct {
    sample_t *sample;
    int      volume;
    long     updated;
} sound_t;

static ALuint  source[MAX_SOUNDS];
static sound_t sound[MAX_SOUNDS];

static void sample_parse_info(char *filename, sample_t *sample)
{
    char *token;
    
    sample->gain = 1.0;
    sample->loop = 0;

    strtok(filename, ",");
    if (!(token = strtok(NULL, ","))) return;
    sample->gain = atof(token);
    if (!(token = strtok(NULL, ","))) return;
    sample->loop = atoi(token);
}

static sample_t *sample_load(char *filename)
{
    ALenum    err;
    ALsizei   size, freq, bits;
    ALboolean loop;
    ALenum    format;
    ALvoid    *data;
    sample_t  *sample;
    
    if (!(sample = (sample_t*)malloc(sizeof(sample_t)))) {
	error("failed to allocate memory for a sample");
	return NULL;
    }
    sample_parse_info(filename, sample);

    /* create buffer */
    alGetError(); /* clear */
    alGenBuffers(1, &sample->buffer);
    if((err = alGetError()) != AL_NO_ERROR) {
	error("failed to create a sample buffer %x %s", 
	      err, alGetString(err));
	free(sample);
	return NULL;
    }
    alutLoadWAVFile((ALbyte *)filename, &format, &data, &size, &freq, &loop);
    if ((err = alGetError()) != AL_NO_ERROR) {
	error("failed to load sound file %s: %x %s", 
	      filename, err, alGetString(err));
	alDeleteBuffers(1, &sample->buffer);
	free(sample);
	return NULL;
    }
    alBufferData(sample->buffer, format, data, size, freq);
    if((err = alGetError()) != AL_NO_ERROR) {
	error("failed to load buffer data %x %s\n", 
	      err, alGetString(err));
	alDeleteBuffers(1, &sample->buffer);
	free(sample);
	return NULL;
    }
    alutUnloadWAV(format, data, size, freq);

    return sample;
}

static void sample_free(sample_t *sample)
{
    if (sample) {
/* alDeleteBuffers hangs on linux sometimes */
#ifdef _WINDOWS 
	if (sample->buffer)
	    alDeleteBuffers(1, &sample->buffer);
#endif
	free(sample);
    }
}

int audioDeviceInit(char *display)
{
    ALenum err;
    int    i;

    alutInit (NULL, 0);
    alListenerf(AL_GAIN, 1.0);
    alDopplerFactor(1.0);
    alDopplerVelocity(343);
    alGetError();
    alGenSources(MAX_SOUNDS, source);
    if ((err = alGetError()) != AL_NO_ERROR) {
	error("failed to create sources %x %s", 
	      err, alGetString(err));
	return -1;
    }

    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    int      i, free;
    ALenum   state;
    sample_t *sample = (sample_t *)(*private);

    if (!sample) {
	sample = sample_load(filename);
	if (!sample) {
	    error("failed to load sample %s\n", filename);
	    return;
	}
	*private = sample;
    }

    free = -1;
    for (i = 0; i < MAX_SOUNDS; i++) {
	if (free == -1) {
	    alGetSourcei(source[i], AL_SOURCE_STATE, &state);
	    if (state != AL_PLAYING) {
		free = i;
		if (!sample->loop) 
		    break;
	    }
	}
	if (sample->loop
	    && sound[i].sample == sample
	    && sound[i].updated < loops
	    && ABS(sound[i].volume - volume) < VOL_THRESHOLD) {
	    alGetSourcei(source[i], AL_SOURCE_STATE, &state);
	    if (state == AL_PLAYING) {
		alSourcef(source[i], AL_GAIN, sample->gain * volume / 100.0f);
		sound[i].updated = loops;
		return;
	    }
	}
    }

    if (free != -1) {    
	sound[free].sample  = sample;
	sound[free].volume  = volume;
	sound[free].updated = loops;
	
	alSourcef(source[free], AL_GAIN, sample->gain * volume / 100.0f);
	alSourcei(source[free], AL_BUFFER, sample->buffer);
	alSourcei(source[free], AL_LOOPING, sample->loop);
	alSourcePlay(source[free]);
    }
}

void audioDeviceEvents(void)
{
}

void audioDeviceUpdate(void)
{
    int i;
    for (i = 0; i < MAX_SOUNDS; i++)
	if (sound[i].sample 
	    && sound[i].sample->loop
	    && sound[i].updated < loops - 1)
	    alSourceStop(source[i]);
}

void audioDeviceFree(void *private) 
{
    if (private)
	sample_free((sample_t *)private);
}

void audioDeviceClose() 
{
    alDeleteSources(MAX_SOUNDS, source);
#ifdef _WINDOWS /* alutExit hangs on linux sometimes */
    alutExit();
#endif
}
