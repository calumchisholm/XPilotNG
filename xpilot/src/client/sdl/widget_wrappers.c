/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>

#include "widget_wrappers.h"

guiarea_t *register_guiarea(	SDL_Rect bounds,
    	    	    	    	void (*button)(/*button*/Uint8,/*state*/Uint8,/*x*/Uint16,/*y*/Uint16, void *data),
				void *buttondata,
			    	void (*motion)(/*xrel*/Sint16,/*yrel*/Sint16,/*x*/Uint16,/*y*/Uint16, void *data),
				void *motiondata,
			    	void (*hover)(int over,/*x*/Uint16,/*y*/Uint16, void *data),
				void *hoverdata
			    	)
{
    guiarea_t *tmp = (guiarea_t *)malloc(sizeof(guiarea_t));
    tmp->bounds.x = bounds.x;
    tmp->bounds.y = bounds.y;
    tmp->bounds.w = bounds.w;
    tmp->bounds.h = bounds.h;
    tmp->button = button;
    tmp->buttondata = buttondata;
    tmp->motion = motion;
    tmp->motiondata = motiondata;
    tmp->hover = hover;
    tmp->hoverdata = hoverdata;
    tmp->next = guiarea_list;
    guiarea_list = tmp;

    return tmp;
}

int unregister_guiarea(guiarea_t *guiarea)
{
    guiarea_t **tmp = &guiarea_list;
    int failure = 1;
    while(*tmp) {
    	if (*tmp == guiarea) {
	    failure = 0;
	    *tmp = (*tmp)->next;
	    free(*tmp);
	    *tmp = NULL;
	    break;
	}
	tmp = &((*tmp)->next);
    }
    
    return failure;
}

/* TODO: add boolean so some areas can be transparent to some actions */
guiarea_t *find_guiarea(Uint16 x,Uint16 y)
{
    guiarea_t *tmp = guiarea_list;
    while(tmp) {
    	if( 	(x >= tmp->bounds.x) && (x <= (tmp->bounds.x + tmp->bounds.w))
	    &&	(y >= tmp->bounds.y) && (y <= (tmp->bounds.y + tmp->bounds.h))
	    ) break;
	tmp = tmp->next;
    }
    return tmp;
}

void clean_guiarea_list(void) {
    guiarea_t *tmp = guiarea_list;
    while(guiarea_list) {
    	tmp = guiarea_list->next;
	free(guiarea_list);
	guiarea_list = tmp;
    }
}

widget_list_t *MakeWidgetList(
    	    	    void (*Draw)(widget_list_t *LI), void *DrawData,
		    void (*GuiReg)(widget_list_t *LI), void *GuiRegData,
		    void (*GuiUnReg)(widget_list_t *LI), void *GuiUnRegData
		    )
{
    static widget_list_t *tmp;
    tmp = malloc(sizeof(widget_list_t));
    tmp->Draw	    	= Draw;
    tmp->DrawData   	= DrawData;
    tmp->GuiReg     	= GuiReg;
    tmp->GuiRegData 	= GuiRegData;
    tmp->GuiUnReg   	= GuiUnReg;
    tmp->GuiUnRegData	= GuiUnRegData;
    tmp->next = NULL;
    return tmp;
}

widget_list_t *AppendListItem( widget_list_t *list,
    	    	    	    	void (*Draw)(widget_list_t *LI), void *DrawData,
				void (*GuiReg)(widget_list_t *LI), void *GuiRegData,
				void (*GuiUnReg)(widget_list_t *LI), void *GuiUnRegData )
{
    if (!list) return NULL;
    static widget_list_t **curr;
    curr = &list;
    while (*curr) {
    	curr = &((*curr)->next);
    }
    return (*curr) = MakeWidgetList(Draw,DrawData,GuiReg,GuiRegData,GuiUnReg,GuiUnRegData);
}

/* do we want theese following two functions to call GuiUnReg() ? */
int DelListItem( widget_list_t *list, widget_list_t *item )
{
    if (!list) return 0;
    
    widget_list_t **curr = &list;
    while (*curr) {
    	if ((*curr) == item) {
	    widget_list_t *tmp = *curr;
	    *curr = (*curr)->next;
	    free(tmp);
	    return 1;
	}
    	curr = &((*curr)->next);
    }
    return 0;
}

void CleanList( widget_list_t *list )
{
    if (!list) return;
    widget_list_t **curr = &list;
    while (*curr) {
    	widget_list_t *tmp = *curr;
    	*curr = (*curr)->next;
    	free(tmp);
    }
    return;
}

void AddListGuiAreas( widget_list_t *list )
{
    if (!list) return;
    widget_list_t *curr = list;
    while (curr) {
    	if (curr->GuiReg) {
	    curr->GuiReg(curr);
	}
	curr = curr->next;
    }
}

/* Might want this to call the last object first instead */
void DelListGuiAreas( widget_list_t *list )
{
    if (!list) return;
    widget_list_t *curr = list;
    while (curr) {
    	if (curr->GuiUnReg) curr->GuiUnReg(curr);
	curr = curr->next;
    }
}

void DrawWidgetList( widget_list_t *list )
{
    if (!list) return;
    widget_list_t *curr = list;
    while (curr) {
    	if (curr->Draw) curr->Draw(curr);
	curr = curr->next;
    }
}

