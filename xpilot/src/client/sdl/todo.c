#include "xpclient.h"

/* from talk.c */
selection_t selection;

void Play_beep(void)
{
    fprintf(stderr, "\aBEEP\n");
}

int Paint_init(void) 
{
    return 0;
}

void Paint_cleanup(void) {}

void Paint_frame(void) {}

int Bitmap_add(char *filename, int count, bool scalable)
{
    return 0;
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
    fwrite(stdout, buf + off, 1, len);
}

void Raise_window(void) {}

void Check_view_dimensions(void) {}

void Colors_init_style_colors(void) {}
