#ifndef RADAR_H
#define RADAR_H

int Radar_init(int x, int y, int w, int h);
void Radar_cleanup(void);
static void Radar_paint(widget_list_t *LI);

#endif
