#ifndef WIDGETS_H
#define WIDGETS_H

#include "SDL.h"

/* TODO: merge this stuff into one... maybe merge it with the stuff in glwidgets.<c|h> aswell */

/*****************************************/
/* BEGIN: Guiarea structures and methods */
typedef struct guiarea_struct guiarea_t;
struct guiarea_struct {
    SDL_Rect bounds;
    void (*button)( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
    void *buttondata;
    void (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data);
    void *motiondata;
    void (*hover)( int over, Uint16 x , Uint16 y , void *data);
    void *hoverdata;
    guiarea_t *next;
} ;

static guiarea_t *guiarea_list = NULL;

guiarea_t *register_guiarea(	SDL_Rect bounds,
    	    	    	    	void (*button)( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data),
				void *buttondata,
			    	void (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data),
				void *motiondata,
			    	void (*hover)( int over, Uint16 x , Uint16 y , void *data),
				void *hoverdata
				);
int unregister_guiarea(guiarea_t *guiarea);
guiarea_t *find_guiarea(Uint16 x,Uint16 y);
void clean_guiarea_list(void);

/* END: Guiarea structures and methods */
/***************************************/

/*********************************************************/
/* BEGIN: Widget function calling structures and methods */

/* Basically its supposed to help you to make lists of items
 * so you can add/remove them from the gui in a group easily
 * and (later) help make a more intuitive way of configuring
 * the GUI.
 * Basically HUD stuff like radar,scorelist,meters(maybe 
 * grouped is preferable) message windows, console etc.
 * should be drag&drop-able, if possible resizable, and
 * rightclick should spawn a config menu with (for that item)
 * iteresting variables to change.
 * Basically I hope someday we don't _always_ have to read 4
 * pages of options to set the client up right.
 */

typedef struct widget_list_struct widget_list_t;
struct widget_list_struct {
    void (*Draw)(widget_list_t *LI);
    void *DrawData;
    void (*GuiReg)(widget_list_t *LI);
    void *GuiRegData;
    void (*GuiUnReg)(widget_list_t *LI);
    void *GuiUnRegData;
    widget_list_t *next;
};

/* Now you might wonder why it is not infact *DrawData that
 * is infact sent to the (*Draw)(void *LI) function. Infact
 * that was the original idea, but in order to gain some
 * flexibility I changed it to send the entire widget_list_t
 * item. Basically so that the functions may change the
 * data used by the others.
 */
widget_list_t *MakeWidgetList(
    	    	    void (*Draw)(widget_list_t *LI), void *DrawData,
		    void (*GuiReg)(widget_list_t *LI), void *GuiRegData,
		    void (*GuiUnReg)(widget_list_t *LI), void *GuiUnRegData
		    );

widget_list_t *AppendListItem( widget_list_t *list,
    	    	    	    	void (*Draw)(widget_list_t *LI), void *DrawData,
				void (*GuiReg)(widget_list_t *LI), void *GuiRegData,
				void (*GuiUnReg)(widget_list_t *LI), void *GuiUnRegData );

int DelListItem( widget_list_t *list, widget_list_t *item );
/* TODO: void MoveToLast( widget_list_t *list, widget_list_t *item );
 * to move the item to the end of the list (drawn on top) and
 * move the guiarea identically
 */

void CleanList( widget_list_t *list );
/* TODO: take measures against adding the same list twice etc,
 * what to do when someone adds items to a list that is already
 * on gui. (naughty)
 * atm you should either add the guiarea for the new item from
 * outside or DelListGuiAreas();AppendListItem(..);AddListGuiAreas();
 * but until a more treelike structure for guiareas is around that
 * will cause the list to grab events from widgets who might be drawn
 * over them.
 * Again: a unified structure would be nice, so event grab layers
 * is inherently the same as drawing layers.
 */
void AddListGuiAreas( widget_list_t *list );
void DelListGuiAreas( widget_list_t *list );
void DrawWidgetList( widget_list_t *list );
/* END: Widget function calling structures and method */
/******************************************************/
#endif
