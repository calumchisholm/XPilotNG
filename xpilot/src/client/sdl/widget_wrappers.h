#ifndef WIDGETS_H
#define WIDGETS_H

#include "SDL.h"

typedef struct {
    SDL_Rect bounds;
    void (*button)(/*button*/Uint8,/*state*/Uint8,/*x*/Uint16,/*y*/Uint16, void *data);
    void *buttondata;
    void (*motion)(/*xrel*/Sint16,/*yrel*/Sint16,/*x*/Uint16,/*y*/Uint16, void *data);
    void *motiondata;
    void (*hover)(int over,/*x*/Uint16,/*y*/Uint16, void *data);
    void *hoverdata;
    void *next;
} guiarea_t;

static guiarea_t *guiarea_list = NULL;

guiarea_t *register_guiarea(	SDL_Rect bounds,
    	    	    	    	void (*button)(/*button*/Uint8,/*state*/Uint8,/*x*/Uint16,/*y*/Uint16, void *data),
				void *buttondata,
			    	void (*motion)(/*xrel*/Sint16,/*yrel*/Sint16,/*x*/Uint16,/*y*/Uint16, void *data),
				void *motiondata,
			    	void (*hover)(int over,/*x*/Uint16,/*y*/Uint16, void *data),
				void *hoverdata
				);
int unregister_guiarea(guiarea_t *guiarea);
guiarea_t *find_guiarea(Uint16 x,Uint16 y);
void clean_guiarea_list(void);

typedef struct {
    void (*Draw)(void *LI);
    void *DrawData;
    void (*GuiReg)(void *LI);
    void *GuiRegData;
    void (*GuiUnReg)(void *LI);
    void *GuiUnRegData;
    void *next;
} widget_list_t;

widget_list_t *MakeWidgetList(
    	    	    void (*Draw)(void *LI), void *DrawData,
		    void (*GuiReg)(void *LI), void *GuiRegData,
		    void (*GuiUnReg)(void *LI), void *GuiUnRegData
		    );

widget_list_t *AppendListItem( widget_list_t *list,
    	    	    	    	void (*Draw)(void *LI), void *DrawData,
				void (*GuiReg)(void *LI), void *GuiRegData,
				void (*GuiUnReg)(void *LI), void *GuiUnRegData );

int DelListItem( widget_list_t *list, widget_list_t *item );
void CleanList( widget_list_t *list );
void AddListGuiAreas( widget_list_t *list );
void DelListGuiAreas( widget_list_t *list );
void DrawWidgetList( widget_list_t *list );

#endif
