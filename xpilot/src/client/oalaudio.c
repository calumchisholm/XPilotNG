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
char audio_version[] = VERSION;

typedef struct {
    ALuint buffer;
    ALuint source;
    float  gain;
} sound_t;


static sound_t *sound_load(const char *filename, float gain)
{
    ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
    ALenum  err;
    ALsizei size, freq, bits;
    ALenum format;
	ALboolean loop;
    ALvoid *data;
    sound_t *snd;
    
    if (!(snd = (sound_t*)malloc(sizeof(sound_t)))) {
	error("failed to allocate memory for a sound");
	return NULL;
    }
    snd->gain = gain;

    /* create buffer */
    alGetError(); /* clear */
    alGenBuffers(1, &snd->buffer);
    if((err = alGetError()) != AL_NO_ERROR) {
	error("failed to create a sound buffer %x %s", 
	      err, alGetString(err));
	free(snd);
	return NULL;
    }

    /* create source */
    alGetError(); /* clear */
    alGenSources(1, &snd->source);
    if((err = alGetError()) != AL_NO_ERROR) {
	error("failed to create a sound source %x %s", 
	      err, alGetString(err));
	alDeleteBuffers(1, &snd->buffer);
	free(snd);
	return NULL;
    }
	
#ifndef _WINDOWS	
    alutLoadWAV(filename, &data, &format, &size, &bits, &freq);
#else
	alutLoadWAVFile((ALbyte *)filename, &format, &data, &size, &freq, &loop);
#endif

    if ((err = alGetError()) != AL_NO_ERROR) {
	error("failed to load sound file %s: %x %s", 
	      filename, err, alGetString(err));
	alDeleteBuffers(1, &snd->buffer);
	alDeleteSources(1, &snd->source);
	free(snd);
	return NULL;
    }
    alBufferData(snd->buffer, format, data, size, freq);
    if((err = alGetError()) != AL_NO_ERROR) {
	error("failed to load buffer data %x %s\n", 
	      err, alGetString(err));
	alDeleteBuffers(1, &snd->buffer);
	alDeleteSources(1, &snd->source);
	free(snd);
	return NULL;
    }
    alutUnloadWAV(format, data, size, freq);

    /* set static source properties */
    alSourcei(snd->source, AL_BUFFER, snd->buffer);
    alSourcei(snd->source, AL_LOOPING, 0);
    alSourcef(snd->source, AL_REFERENCE_DISTANCE, 10);
    alSourcefv(snd->source, AL_POSITION, zeroes);
    alSourcefv(snd->source, AL_VELOCITY, zeroes);

    return snd;
}

static void sound_free(sound_t *snd)
{
    if (snd) {
	if (snd->buffer)
	    alDeleteBuffers(1, &snd->buffer);
	if (snd->source)
	    alDeleteSources(1, &snd->source);
	free(snd);
    }
}

int audioDeviceInit(char *display)
{
    ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
    ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
    
    alutInit (NULL, 0); // init OpenAL
    
    // global settings
    alListenerf(AL_GAIN, 1.0);
    alDopplerFactor(1.0); // don't exaggerate doppler shift
    alDopplerVelocity(343); // using meters/second
    
    alListenerfv(AL_POSITION, zeroes );
    alListenerfv(AL_VELOCITY, zeroes );
    alListenerfv(AL_ORIENTATION, front );


    return 0;
}

void audioDevicePlay(char *filename, int type, int volume, void **private)
{
    char *comma;
    float gain;
    sound_t *snd = (sound_t *)(*private);

    
    if (!snd) {
	if ((comma = strchr(filename, ',')) != NULL) {
	    *comma = '\0';
	    comma++;
	    gain = atof(comma);
	} else {
	    gain = 1.0f;
	}
	snd = sound_load(filename, gain);
	if (!snd) {
	    error("failed to load sound %s\n", filename);
	    return;
	}
	*private = snd;
    }

    alSourcef(snd->source, AL_GAIN, snd->gain * volume / 100.0f);
    alSourcePlay(snd->source);
}

void audioDeviceEvents(void)
{
}

void audioDeviceFree(void *private) 
{
    if (private)
	sound_free((sound_t *)private);
}

void audioDeviceClose() 
{
    alutExit();
}
