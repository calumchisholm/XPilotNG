#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

static enum bufs {INTS, ERRNOS, SHORTS, DATA, SCHED, EI, ES, OPTTOUT, NOMORE};
static void *threshold[NOMORE];
static int notfirst[NOMORE];
static void *readto[NOMORE];
/* These cause unnecessary warnings because C is too stupid to understand
   the similarity of (void *)a = (char *)b; and (void **a) = &(char *)b; */
static void **startb[NOMORE] = {&playback_ints_start, &playback_errnos_start,
        &playback_shorts_start, &playback_data_start, &playback_sched_start,
        &playback_ei_start, &playback_es_start, &playback_opttout_start};
static void **curb[NOMORE] = {&playback_ints, &playback_errnos,
        &playback_shorts, &playback_data, &playback_sched, &playback_ei,
        &playback_es, &playback_opttout};

static int next_type;
static int next_len;

static FILE *recf1;
static FILE *recf2;

static void Write_data(int type)
{
    FILE *f;
    int len;
    char *startc = *startb[type], *curc = *curb[type];
    
    if (notfirst[type])
	f = recf2;
    else {
	f = recf1;
	notfirst[type] = 1;
    }
    len = curc - startc;
    putc(type, f);
    fwrite(&len, sizeof(int), 1, f);
    fwrite(startc, 1, len, f);
    *curb[type] = *startb[type];
}

static void Read_data(int type, int len)
{
    int num;
    char *startc, *curc;

    startc = *startb[type];
    curc = *curb[type];
    num = (char *)readto[type] - curc;
    memmove(startc, curc, num);
    readto[type] = startc + num;
    *curb[type] = *startb[type];
    fread(readto[type], 1, len, recf1);
    readto[type] += len;
}
    
void Init_recording(void)
{
    static int oldMode = 0;
    int i, j;

    recOpt = 1; /* Less robust but produces smaller files. */
    if (oldMode == 0) {
	oldMode = recordMode + 10;
	if (recordMode == 1) {
	    record = rrecord = 1;
	    recf1 = fopen("/tmp/serverrec", "wb");
	    recf2 = fopen("/tmp/serverrec2", "wb");
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
	    while (1) {
		i = getc(recf1);
		if (i == NOMORE)
		    break;
		fread(&j, sizeof(int), 1, recf1);
		Read_data(i, j);
	    }
	    fclose(recf1);
	    recf1 = fopen("/tmp/serverrec2", "rb");
	    next_type = getc(recf1);
	    if (next_type != NOMORE)
		fread(&next_len, sizeof(int), 1, recf1);
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
	*playback_opttout++ = INT_MAX;
	*playback_ei++ = INT_MAX;
	for (i = 0; i < NOMORE; i++)
	    Write_data(i);
	putc(NOMORE, recf1);
	putc(NOMORE, recf2);
	fclose(recf1);
	fclose(recf2);
	oldMode = 10;
    }
    if (oldMode == 12) {
	oldMode = 10;
	while(1); /* There might be better ways to stop playback? */
    }
}

void Handle_recording_buffers(void)
{
    int i;

    if (!recordMode)
	return;
    if (recordMode == 1) {
	for (i = 0; i < NOMORE; i++)
	    if (*curb[i] >= threshold[i])
		Write_data(i);
	return;
    }

    if (recordMode == 2) {
	if (next_type == NOMORE)
	    return;
	do {
	    if (next_len < *curb[next_type] - *startb[next_type] + threshold[next_type] - readto[next_type]) {
		Read_data(next_type, next_len);
		next_type = getc(recf1);
		if (next_type != NOMORE) {
		    fread(&next_len, sizeof(int), 1, recf1);
		    continue;
		}
	    }
	} while (0);
	return;
    }
    error("Wrong recordMode");
    exit(1);
}
