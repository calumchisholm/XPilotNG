#include "xpclient.h"

/* from talk.c */
selection_t selection;

void Play_beep(void)
{
    fprintf(stderr, "\aBEEP\n");
}

void Print_messages_to_stdout(void) {}

void Radar_show_target(int x, int y) {}

void Radar_hide_target(int x, int y) {}

int Startup_server_motd(void)
{
    return 0;
}

int Handle_motd(long off, char *buf, int len, long filesize) 
{
    fwrite(buf + off, 1, len, stdout);
}

void Raise_window(void) {}

void Colors_init_style_colors(void) {}
