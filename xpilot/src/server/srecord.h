#ifndef SRECORD_H
#define SRECORD_H

/* Here for sched.c, which doesn't include proto.h */
void Handle_recording_buffers(void);
void Get_recording_data(void);

extern int   playback;
extern int   record;
extern int   rrecord;
extern int   rplayback;
extern int   *playback_ints;
extern int   *playback_errnos;
extern int   *playback_errnos_start;
extern short *playback_shorts;
extern short *playback_shorts_start;
extern char  *playback_strings;
extern char  *playback_data;
extern char  *playback_sched;
extern int   *playback_ints_start;
extern char  *playback_strings_start;
extern char  *playback_data_start;
extern char  *playback_sched_start;
extern int   *playback_ei;
extern char  *playback_es;
extern int   *playback_ei_start;
extern char  *playback_es_start;
extern int   *playback_opttout;
extern int   recOpt;
#endif  /* SRECORD_H */
