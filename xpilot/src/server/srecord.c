#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define SERVER
#include "const.h"
#include "global.h"
#include "error.h"

int   playback = 0;
int   record = 0;
int   *playback_ints;
int   *playback_ints_start;
int   *playback_errnos;
int   *playback_errnos_start;
short *playback_shorts;
short *playback_shorts_start;
char  *playback_data;
char  *playback_data_start;
char  *playback_sched;
char  *playback_sched_start;
int   *playback_ei;
int   *playback_ei_start;
char  *playback_es;
char  *playback_es_start;
int   *playback_opttout;
int   *playback_opttout_start;

int   rrecord;
int   rplayback;
int   recOpt;

enum bufs {INTS, ERRNOS, SHORTS, DATA, SCHED, EI, ES, OPTTOUT, NOMORE};
static char *threshold[NOMORE];
static char *readto[NOMORE];
static char **startb[NOMORE] = {&playback_ints_start, &playback_errnos_start,
        &playback_shorts_start, &playback_data_start, &playback_sched_start,
        &playback_ei_start, &playback_es_start, &playback_opttout_start};
static char **curb[NOMORE] = {&playback_ints, &playback_errnos,
        &playback_shorts, &playback_data, &playback_sched, &playback_ei,
        &playback_es, &playback_opttout};

static FILE *recf1;

static void Write_data(int type)
{
    int len;
    char *startc = *startb[type], *curc = *curb[type];

    len = curc - startc;
    fwrite(&len, sizeof(int), 1, recf1);
    fwrite(startc, 1, len, recf1);
    *curb[type] = *startb[type];
}

static void Read_data(int type, int len)
{
    int num;
    char *startc, *curc;

    startc = *startb[type];
    curc = *curb[type];
    num = readto[type] - curc;
    if (num != 0) {
	errno = 0;
	error("Recording out of sync");
	exit(1);
    }
    *curb[type] = *startb[type];
    fread(*startb[type], 1, len, recf1);
    readto[type] = startc + len;
}

static void Dump_data(void)
{
    int i;

    *playback_sched++ = 127;
    for (i = 0; i < NOMORE; i++)
	Write_data(i);
    errno = 0;
    error("dumping");
}

void Get_recording_data(void)
{
    int i, len;

    for (i = 0; i < NOMORE; i++) {
	fread(&len, sizeof(int), 1, recf1);
	if (len > 149000) {
	    errno = 0;
	    error("Incorrect chunk length reading recording");
	    exit(1);
	}
	Read_data(i, len);
    }
    *(int*)readto[EI] = INT_MAX;
    *(int*)readto[OPTTOUT] = INT_MAX;
}

void Init_recording(void)
{
    static int oldMode = 0;
    int i;

    recOpt = 1; /* Less robust but produces smaller files. */
    if (oldMode == 0) {
	oldMode = recordMode + 10;
	if (recordMode == 1) {
	    record = rrecord = 1;
	    recf1 = fopen("/tmp/serverrec", "wb");
	    for (i = 0; i < NOMORE; i++) {
		/* These sizes are not sensible,
		   different buffers should have
		   different sizes */
		*startb[i] = malloc(150000);
		*curb[i] = *startb[i];
		threshold[i] = *startb[i] + 50000;
	    }
	    return;
	} else if (recordMode == 2) {
	    rplayback = 1;
	    for (i = 0; i < NOMORE; i++) {
		*startb[i] = malloc(150000);
		*curb[i] = *startb[i];
		readto[i] = *startb[i];
		threshold[i] = *startb[i] + 100000;
	    }
	    recf1 = fopen("/tmp/serverrec", "rb");
	    Get_recording_data();
	    return;
	} else if (recordMode == 0)
	    return;
	else {
	    errno = 0;
	    error("You IDIOT!!!");
	    exit(1);
	}
    }
    if (recordMode != 0 || oldMode < 11 || oldMode > 12) {
	errno = 0;
	error("You IDIOT!!!");
	exit(1);
    }
    if (oldMode == 11) {
	Dump_data();
	fclose(recf1);
	oldMode = 10;
    }
    if (oldMode == 12) {
	oldMode = 10;
	errno = 0;
	error("End of playback.");
	exit(1); /* There might be better ways to stop playback? */
    }
}

void Handle_recording_buffers(void)
{
    int i;

    if (recordMode != 1)
	return;

    if (recordMode == 1) {
	for (i = 0; i < NOMORE; i++)
	    if (*curb[i] >= threshold[i])
		Dump_data();
	return;
    }
}
