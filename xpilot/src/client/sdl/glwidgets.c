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

#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "xpclient.h"
#include "sdlpaint.h"
#include "images.h"
#include "text.h"
#include "glwidgets.h"

/****************************************************/
/* BEGIN: Main GLWidget stuff	    	    	    */
/****************************************************/
/* one size should fit all */
void GuiReg(widget_list_t *LI);
void GuiUnReg(widget_list_t *LI);

/* LI->data should be a SDL_Rect with the bounds */
void GuiReg(widget_list_t *LI)
{
    GLWidget *widget = (GLWidget *)(LI->GuiRegData);
    widget->guiarea = register_guiarea(widget->bounds,
    	    	    	    	widget->button,
    	    	    	    	widget->buttondata,
			    	widget->motion,
				widget->motiondata,
			    	widget->hover,
				widget->hoverdata
				);    	    	    	    	
}

/* one size should fit all */
void GuiUnReg(widget_list_t *LI)
{
    GLWidget *tmp = (GLWidget *)(LI->GuiUnRegData);
    unregister_guiarea(tmp->guiarea);
}

/* only supposed to take care of mallocs done on behalf of the
 * appropriate Init_<foo> function
 */
void Close_Widget (GLWidget *widget)
{
    if (widget->Close) widget->Close(widget);
    else {
    free(widget->wid_info);
    free(widget);
    }
}

/* IMPORTANT: compound widgets need to edit this function */
void SetBounds_GLWidget(GLWidget *widget, SDL_Rect *b)
{
    if (!widget) return;
    if (!b) return;
    
    
    if (widget->SetBounds) widget->SetBounds(widget,b);
    else {
    	widget->bounds.x = b->x;
    	widget->bounds.y = b->y;
    	widget->bounds.w = b->w;
    	widget->bounds.h = b->h;
    	if(widget->guiarea) {
    	    widget->guiarea->bounds.x = b->x;
    	    widget->guiarea->bounds.y = b->y;
    	    widget->guiarea->bounds.w = b->w;
    	    widget->guiarea->bounds.h = b->h;
    	}
    }
}
/****************************************************/
/* END: Main GLWidget stuff 	    	    	    */
/****************************************************/

/**********************/
/* Begin:  ArrowWidget*/
/**********************/
void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
void Paint_ArrowWidget(widget_list_t *LI);

void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data)
{
    if (!data) return;
    ArrowWidget *tmp = (ArrowWidget *)(((GLWidget *)data)->wid_info);
    if (state == SDL_PRESSED && !(tmp->locked)) {
	if (button == 1) {
    	    tmp->press = true;
	}
	if (button == 2) {
	    tmp->tap = true;
	    if (tmp->action) tmp->action(tmp->actiondata);
	}
    }
    if (state == SDL_RELEASED) {
	if (button == 1) {
    	    tmp->press = false;
	}
    }
}

void Paint_ArrowWidget(widget_list_t *LI)
{
    if (!LI) return;
    static int normalcolor  = 0xff0000ff;
    static int presscolor   = 0x00ff00ff;
    static int tapcolor     = 0xffffffff;
    static int lockcolor    = 0x88000088;
     
    GLWidget *tmp = (GLWidget *)(LI->DrawData);
    SDL_Rect *b = &(tmp->bounds);
    ArrowWidget *wid_info = (ArrowWidget *)(tmp->wid_info);
    
    if (wid_info->locked) {
    	set_alphacolor( lockcolor );
    } else if (wid_info->press) {
    	if (wid_info->action) {
	    wid_info->action(wid_info->actiondata);
	}
	set_alphacolor( presscolor );
    } else if (wid_info->tap) {
    	set_alphacolor( tapcolor );
    	wid_info->tap = false;
    } else {
    	set_alphacolor( normalcolor );
    }
    
    ArrowWidget_dir_t dir = wid_info->direction;
    glBegin(GL_POLYGON);
    switch ( dir ) {
    	case RIGHTARROW:
	    glVertex2i(b->x 	    ,b->y);
	    glVertex2i(b->x 	    ,b->y+b->h);
	    glVertex2i(b->x + b->w  ,b->y+b->h/2);
	    break;
    	case UPARROW:
	    glVertex2i(b->x + b->w/2,b->y);
	    glVertex2i(b->x 	    ,b->y+b->h);
	    glVertex2i(b->x + b->w  ,b->y+b->h);
	    break;
    	case LEFTARROW:
	    glVertex2i(b->x + b->w  ,b->y);
	    glVertex2i(b->x 	    ,b->y+b->h/2);
	    glVertex2i(b->x + b->w  ,b->y+b->h);
	    break;
    	case DOWNARROW:
	    glVertex2i(b->x 	    ,b->y);
	    glVertex2i(b->x + b->w/2,b->y+b->h);
	    glVertex2i(b->x + b->w  ,b->y);
	    break;
	default:
	    error("Weird direction for ArrowWidget! (direction:%i)\n",dir);
    }
    glEnd();
}

GLWidget *Init_ArrowWidget(widget_list_t *list, ArrowWidget_dir_t direction,int width, int height,
    	     void (*action)( void *data), void *actiondata )
{
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_ArrowWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(ArrowWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_ArrowWidget");
	return NULL;
    }
    tmp->WIDGET     	= ARROWWIDGET;
    ((ArrowWidget *)tmp->wid_info)->direction  = direction;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= width;
    tmp->bounds.h   	= height;
    ((ArrowWidget *)tmp->wid_info)->press = false;
    ((ArrowWidget *)tmp->wid_info)->tap = false;
    ((ArrowWidget *)tmp->wid_info)->locked = false;
    ((ArrowWidget *)tmp->wid_info)->action = action;
    ((ArrowWidget *)tmp->wid_info)->actiondata = actiondata;
    tmp->Draw	    	= Paint_ArrowWidget;
    tmp->GuiReg	    	= GuiReg;
    tmp->GuiUnReg   	= GuiReg;
    tmp->Close	    	= NULL;
    tmp->SetBounds  	= NULL;
    tmp->button     	= button_ArrowWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->guiarea  	= NULL;
    tmp->listPtr    	= AppendListItem( list, Paint_ArrowWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );
    tmp->next	    	= NULL;
    return tmp;
}

/********************/
/* End:  ArrowWidget*/
/********************/

/***********************************/
/* Begin:  LabeledRadiobuttonWidget*/
/***********************************/
void button_LabeledRadiobuttonWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
void Paint_LabeledRadiobuttonWidget(widget_list_t *LI);

void button_LabeledRadiobuttonWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data)
{
    if (!data) return;
    LabeledRadiobuttonWidget *tmp = (LabeledRadiobuttonWidget *)(((GLWidget *)data)->wid_info);
    if (state == SDL_PRESSED) {
	if (button == 1) {
	    /* Toggle state, and call (*action)*/
	    tmp->state = !(tmp->state);
	    if (tmp->action)  {
		tmp->action(tmp->state,tmp->actiondata);
	    }
	}
    }
}

void Paint_LabeledRadiobuttonWidget(widget_list_t *LI)
{
    if (!LI) return;
    static int false_bg_color	= 0x00000044;
    static int true_bg_color	= 0x00000044;
    static int false_text_color	= 0xff0000ff;
    static int true_text_color	= 0x00ff00ff;
     
    GLWidget *tmp = (GLWidget *)(LI->DrawData);
    SDL_Rect *b = &(tmp->bounds);
    LabeledRadiobuttonWidget *wid_info = (LabeledRadiobuttonWidget *)(tmp->wid_info);
    
    if (wid_info->state)
    	set_alphacolor(true_bg_color);
    else
    	set_alphacolor(false_bg_color);
    
    glBegin(GL_QUADS);
    	glVertex2i(b->x     	,b->y);
   	glVertex2i(b->x + b->w  ,b->y);
    	glVertex2i(b->x + b->w  ,b->y+b->h);
    	glVertex2i(b->x     	,b->y+b->h);
     glEnd();
    
    if (wid_info->state) {
    	disp_text(wid_info->ontex, true_text_color, CENTER, CENTER, tmp->bounds.x+tmp->bounds.w/2, draw_height - tmp->bounds.y-tmp->bounds.h/2, true);
    } else {
    	disp_text(wid_info->offtex, false_text_color, CENTER, CENTER, tmp->bounds.x+tmp->bounds.w/2, draw_height - tmp->bounds.y-tmp->bounds.h/2, true);
    }
}

GLWidget *Init_LabeledRadiobuttonWidget(widget_list_t *list, string_tex_t *ontex, string_tex_t *offtex,
    	    	    	 void (*action)(bool state, void *actiondata), void *actiondata, bool start_state)
{

    if (!ontex || !(ontex->texture) || !offtex || !(offtex->texture) ) {
    	error("texure(s) missing for Init_LabeledRadiobuttonWidget.");
	return NULL;
    }
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_LabeledRadiobuttonWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(LabeledRadiobuttonWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_LabeledRadiobuttonWidget");
	return NULL;
    }
    tmp->WIDGET     	= LABELEDRADIOBUTTONWIDGET;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= MAX(ontex->width,offtex->width)+5;
    tmp->bounds.h   	= MAX(ontex->height,offtex->height);
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->state  = start_state;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->ontex  = ontex;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->offtex  = offtex;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->action  = action;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->actiondata  = actiondata;

    tmp->Draw	    	= Paint_LabeledRadiobuttonWidget;
    tmp->GuiReg	    	= GuiReg;
    tmp->GuiUnReg   	= GuiUnReg;
    tmp->Close	    	= NULL;
    tmp->SetBounds  	= NULL;
    tmp->button     	= button_LabeledRadiobuttonWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->guiarea  	= NULL;
    tmp->listPtr    	= AppendListItem( list, Paint_LabeledRadiobuttonWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );
    tmp->next	    	= NULL;
    return tmp;
}
/*********************************/
/* End:  LabeledRadiobuttonWidget*/
/*********************************/

/*****************************/
/* Begin:  BoolChooserWidget */
/*****************************/
void Paint_BoolChooserWidget(widget_list_t *LI);
void BoolChooserWidget_SetValue(bool state, void *data);
void Close_BoolChooserWidget (GLWidget *widget);
void SetBounds_BoolChooserWidget(GLWidget *widget, SDL_Rect *b);

static int num_BoolChooserWidget = 0;
static string_tex_t *BoolChooserWidget_ontex = NULL;
static string_tex_t *BoolChooserWidget_offtex = NULL;

void Close_BoolChooserWidget(GLWidget *widget)
{
    if (!widget) return;
    if (widget->WIDGET !=BOOLCHOOSERWIDGET) {
    	error("Wrong widget type for SetBounds_BoolChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    --num_BoolChooserWidget;
    if (!num_BoolChooserWidget) {
    	free_string_texture(BoolChooserWidget_ontex);
	free(BoolChooserWidget_ontex);
	BoolChooserWidget_ontex = NULL;
    	free_string_texture(BoolChooserWidget_offtex);
	free(BoolChooserWidget_offtex);
	BoolChooserWidget_offtex = NULL;
    }

    free(widget->wid_info);
    free(widget);
}

void SetBounds_BoolChooserWidget(GLWidget *widget, SDL_Rect *b)
{
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET !=BOOLCHOOSERWIDGET) {
    	error("Wrong widget type for SetBounds_BoolChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    widget->bounds.x = b->x;
    widget->bounds.y = b->y;
    widget->bounds.w = b->w;
    widget->bounds.h = b->h;
    if(widget->guiarea) {
    	widget->guiarea->bounds.x = b->x;
    	widget->guiarea->bounds.y = b->y;
    	widget->guiarea->bounds.w = b->w;
    	widget->guiarea->bounds.h = b->h;
    }

    GLWidget *tmp = ((BoolChooserWidget *)(widget->wid_info))->buttonwidget;
    SDL_Rect b2;
    
    b2.h = tmp->bounds.h;
    b2.w = tmp->bounds.w;
    b2.x = widget->bounds.x + widget->bounds.w -2 -tmp->bounds.w;
    b2.y = widget->bounds.y + 1;
	    
    SetBounds_GLWidget(tmp,&b2);
}

void BoolChooserWidget_SetValue(bool state, void *data)
{
    if (!data) return;
    *(((BoolChooserWidget *)data)->value) = state;
    if (((BoolChooserWidget *)data)->action)
    	((BoolChooserWidget *)data)->action(((BoolChooserWidget *)data)->actiondata);
}

void Paint_BoolChooserWidget(widget_list_t *LI)
{
    if (!LI) return;
    static int bg_color     = 0x0000ff88;
    static int name_color   = 0xffff66ff;
    
    GLWidget *widget = (GLWidget *)(LI->DrawData);
    BoolChooserWidget *wid_info = (BoolChooserWidget *)(widget->wid_info);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    set_alphacolor(bg_color);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();
    disp_text(&(wid_info->nametex), name_color, LEFT, CENTER, widget->bounds.x+2/*|_text*/, draw_height - widget->bounds.y - widget->bounds.h/2, true);
}

GLWidget *Init_BoolChooserWidget(widget_list_t *list, font_data *font, const char *name, bool *value,void (*action)(void *data),void *actiondata)
{
    if (!BoolChooserWidget_ontex) {
    	if ((BoolChooserWidget_ontex = malloc(sizeof(string_tex_t)))) {
	    if (!(BoolChooserWidget_offtex = malloc(sizeof(string_tex_t)))) {
	    	free(BoolChooserWidget_ontex);
		BoolChooserWidget_ontex = NULL;
	    	error("Failed to malloc BoolChooserWidget_offtex in Init_BoolChooserWidget");
	    	return NULL;
	    }
	} else {
	    error("Failed to malloc BoolChooserWidget_ontex in Init_BoolChooserWidget");
	    return NULL;
	}
	if (render_text(font,"True",BoolChooserWidget_ontex)) {
    	    if (!render_text(font,"False",BoolChooserWidget_offtex)) {
	    	error("Failed to render 'False' in Init_BoolChooserWidget");
		free_string_texture(BoolChooserWidget_ontex);
		free(BoolChooserWidget_ontex);
		BoolChooserWidget_ontex = NULL;
		free(BoolChooserWidget_offtex);
		BoolChooserWidget_offtex = NULL;
		return NULL;
	    }
    	} else {
	    error("Failed to render 'True' in Init_BoolChooserWidget");
    	    free(BoolChooserWidget_ontex);
    	    BoolChooserWidget_ontex = NULL;
    	    free(BoolChooserWidget_offtex);
    	    BoolChooserWidget_offtex = NULL;
    	    return NULL;
	}
    }
    
    if (!value || !name || !strlen(name) ) {
    	error("bool or name misssing for Init_BoolChooserWidget.");
	return NULL;
    }
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc tmp in Init_BoolChooserWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(BoolChooserWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc tmp->wid_info in Init_BoolChooserWidget");
	return NULL;
    }
    
    BoolChooserWidget *wid_info = (BoolChooserWidget *)(tmp->wid_info);
    
    if ( !render_text(font,name,&(wid_info->nametex)) ) {
    	error("Failed to render '%s' in Init_BoolChooserWidget",name);
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
    
    tmp->listPtr    	= AppendListItem( list, Paint_BoolChooserWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );
    if ( !(wid_info->buttonwidget = Init_LabeledRadiobuttonWidget(list, BoolChooserWidget_ontex,
    	    	    	    	    	BoolChooserWidget_offtex, BoolChooserWidget_SetValue,
					wid_info, *value)) ) {
	DelListItem(list,tmp->listPtr);
    	error("Failed to make a LabeledRadiobuttonWidget for Init_BoolChooserWidget");
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
        
    tmp->WIDGET     	= BOOLCHOOSERWIDGET;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= 2+wid_info->nametex.width+5+wid_info->buttonwidget->bounds.w+2;
    tmp->bounds.h   	= 1+ MAX( wid_info->nametex.height,wid_info->buttonwidget->bounds.h) +1 ;
    
    wid_info->value 	= value;
    wid_info->action	= action;
    wid_info->actiondata = actiondata;

    tmp->Draw	    	= Paint_BoolChooserWidget;
    tmp->GuiReg  	= GuiReg;
    tmp->GuiUnReg  	= GuiUnReg;
    tmp->Close  	= Close_BoolChooserWidget;
    tmp->SetBounds  	= SetBounds_BoolChooserWidget;

    tmp->button     	= NULL;
    tmp->buttondata 	= NULL;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->guiarea  	= NULL;
    tmp->next	    	= NULL;
    ++num_BoolChooserWidget;
    return tmp;
}
/***************************/
/* End:  BoolChooserWidget */
/***************************/

/***************************/
/* Begin: IntChooserWidget */
/***************************/
void IntChooserWidget_Add( void *data );
void IntChooserWidget_Subtract( void *data );
void Paint_IntChooserWidget(widget_list_t *LI);
void Close_IntChooserWidget (GLWidget *widget);
void SetBounds_IntChooserWidget(GLWidget *widget, SDL_Rect *b);

void Close_IntChooserWidget (GLWidget *widget)
{
    if (!widget) return;
    if (widget->WIDGET !=INTCHOOSERWIDGET) {
    	error("Wrong widget type for Close_IntChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    free_string_texture( &(((IntChooserWidget *)widget->wid_info)->nametex) );
    free_string_texture( &(((IntChooserWidget *)widget->wid_info)->valuetex) );
    free(widget->wid_info);
    free(widget);
}

void SetBounds_IntChooserWidget(GLWidget *widget, SDL_Rect *b)
{
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET !=INTCHOOSERWIDGET) {
    	error("Wrong widget type for SetBounds_IntChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    widget->bounds.x = b->x;
    widget->bounds.y = b->y;
    widget->bounds.w = b->w;
    widget->bounds.h = b->h;
    if(widget->guiarea) {
    	widget->guiarea->bounds.x = b->x;
    	widget->guiarea->bounds.y = b->y;
    	widget->guiarea->bounds.w = b->w;
    	widget->guiarea->bounds.h = b->h;
    }
    IntChooserWidget *tmp = (IntChooserWidget *)(widget->wid_info);
    SDL_Rect rab;
    SDL_Rect lab;
    
    lab.h = rab.h = tmp->valuetex.height-4;
    lab.w = rab.w = rab.h;
    lab.y = rab.y = widget->bounds.y + (widget->bounds.h-rab.h)/2;
    rab.x = widget->bounds.x + widget->bounds.w - rab.w -2/*>_|*/;
    lab.x = rab.x - tmp->valuespace/*_value*/ -2/*<_value_>*/ - lab.w;
	    
    SetBounds_GLWidget(tmp->leftarrow,&lab);
    SetBounds_GLWidget(tmp->rightarrow,&rab);
}

void IntChooserWidget_Add( void *data )
{
    if (!data) return;
    IntChooserWidget *tmp = ((IntChooserWidget *)((GLWidget *)data)->wid_info);

    int step;
    if (tmp->direction > 0)
    	step = 100/clientFPS;
    else
    	step = 1;
    
    if (*(tmp->value) < (*(tmp->max))) {
    	(*(tmp->value)) = MIN( (*(tmp->value)) + step,(*(tmp->max)) );
    	if ( (*(tmp->value)) > (*(tmp->min)))
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	if ( (*(tmp->value)) == (*(tmp->max)))
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
	tmp->direction = 2;
    	if (tmp->action)
    	    tmp->action(tmp->actiondata);
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(tmp->value));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%i) texture for IntChooserWidget!\n",*(tmp->value));
    } else {
    	((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
    }
}

void IntChooserWidget_Subtract( void *data )
{
    if (!data) return;
    IntChooserWidget *tmp = ((IntChooserWidget *)((GLWidget *)data)->wid_info);

    int step;
    if (tmp->direction < 0)
    	step = 100/clientFPS;
    else
    	step = 1;

    if (*(tmp->value) > (*(tmp->min))) {
    	(*(tmp->value)) = MAX( (*(tmp->value)) - step,(*(tmp->min)) );
    	if ( (*(tmp->value)) < (*(tmp->max)))
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
    	if ( (*(tmp->value)) == (*(tmp->min)))
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
	tmp->direction = -2;
    	if (tmp->action)
    	    tmp->action(tmp->actiondata);
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(tmp->value));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%i) texture for IntChooserWidget!\n",*(tmp->value));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_IntChooserWidget(widget_list_t *LI)
{
    if (!LI) {
    	error("Paint_IntChooserWidget: widget");
	return;
    }
    static int bg_color     = 0x0000ff88;
    static int name_color   = 0xffff66ff;
    static int value_color  = 0x00ff00ff;
    
    GLWidget *widget = (GLWidget *)(LI->DrawData);
    IntChooserWidget *wid_info = (IntChooserWidget *)(widget->wid_info);

    if (!wid_info) {
    	error("Paint_IntChooserWidget: wid_info missing");
	return;
    }
    
    if (wid_info->direction > 0) --(wid_info->direction);
    else if (wid_info->direction < 0) ++(wid_info->direction);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    set_alphacolor(bg_color);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();
    disp_text(&(wid_info->nametex), name_color, LEFT, CENTER, widget->bounds.x+2/*|_text*/, draw_height - widget->bounds.y - widget->bounds.h/2, true);
    disp_text(&(wid_info->valuetex), value_color, RIGHT, CENTER, wid_info->rightarrow->bounds.x-1/*value_>*/-2/*>_|*/, draw_height - widget->bounds.y - widget->bounds.h/2, true );
}

GLWidget *Init_IntChooserWidget(widget_list_t *list, font_data *font, const char *name, int *value, int *min, int *max,void (*action)(void *data),void *actiondata)
{
    int valuespace;
    GLWidget *tmp = malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_IntChooserWidget");
	return NULL;
    }
    tmp->wid_info   = malloc(sizeof(IntChooserWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_IntChooserWidget");
	return NULL;
    }

    /* hehe ugly hack to guess max size of value strings
     * monospace font is preferred
     */
    string_tex_t tmp_tex;
    if (render_text(font,"5.55",&tmp_tex)) {
    	free_string_texture(&tmp_tex);
	valuespace = tmp_tex.width+4;
    } else {
    	valuespace = 30;
    }

    IntChooserWidget *wid_info = tmp->wid_info;
    tmp->WIDGET     = INTCHOOSERWIDGET;
    if (render_text(font,name,&(wid_info->nametex))) {
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*value);
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font     = font;
	    wid_info->value    = value;
	    wid_info->min      = min;
	    wid_info->max      = max;
	    wid_info->valuespace = valuespace;
	    wid_info->direction = 0;
	    wid_info->action	= action;
	    wid_info->actiondata = actiondata;

    	    /* this needs to be the first thing added, or it will cover the sub-parts */
	    tmp->listPtr = AppendListItem( list, Paint_IntChooserWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );

	    if (!(wid_info->leftarrow  = Init_ArrowWidget(list,LEFTARROW,12,16,IntChooserWidget_Subtract,tmp))) {
    	    	DelListItem(list,tmp->listPtr);
		free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
		error("Init_IntChooserWidget couldn't init leftarrow!");
		return NULL;
	    } 	
	    if (*value <= *min)	((ArrowWidget *)(wid_info->leftarrow->wid_info))->locked = true;

    	    if (!(wid_info->rightarrow = Init_ArrowWidget(list,RIGHTARROW,12,16,IntChooserWidget_Add,tmp))) {
    	    	DelListItem(list,tmp->listPtr);
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
	    	error("Init_IntChooserWidget couldn't init rightarrow!");
		return NULL;
	    }
	    if (*value >= *max)	((ArrowWidget *)(wid_info->rightarrow->wid_info))->locked = true;

    	    tmp->bounds.x   = 0;
    	    tmp->bounds.y   = 0;
    	    tmp->bounds.w   = 2/*|_text*/+ wid_info->nametex.width +5/*text___<*/ + valuespace/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;

    	    tmp->Draw	    	= Paint_IntChooserWidget;
    	    tmp->GuiReg  	= GuiReg;
    	    tmp->GuiUnReg  	= GuiUnReg;
    	    tmp->Close  	= Close_IntChooserWidget;
    	    tmp->SetBounds  	= SetBounds_IntChooserWidget;

    	    tmp->button     	= NULL;
    	    tmp->buttondata 	= NULL;
    	    tmp->motion     	= NULL;
    	    tmp->motiondata 	= NULL;
    	    tmp->hover	    	= NULL;
    	    tmp->hoverdata  	= NULL;
    	    tmp->guiarea  	= NULL;
	    tmp->next	    	= NULL;
	    return tmp;
    	} else free_string_texture(&(wid_info->nametex));
    }
    free(tmp);
    error("Failed to initialize Init_IntChooserWidget %s (couldn't render text)",name);
    return NULL;
}
/*************************/
/* End: IntChooserWidget */
/*************************/

/******************************/
/* Begin: DoubleChooserWidget */
/******************************/
void DoubleChooserWidget_Add( void *data );
void DoubleChooserWidget_Subtract( void *data );
void Paint_DoubleChooserWidget(widget_list_t *LI);
void Close_DoubleChooserWidget (GLWidget *widget);
void SetBounds_DoubleChooserWidget(GLWidget *widget, SDL_Rect *b);

void Close_DoubleChooserWidget (GLWidget *widget)
{
    if (!widget) return;
    if (widget->WIDGET !=DOUBLECHOOSERWIDGET) {
    	error("Wrong widget type for Close_DoubleChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    free_string_texture( &(((DoubleChooserWidget *)widget->wid_info)->nametex) );
    free_string_texture( &(((DoubleChooserWidget *)widget->wid_info)->valuetex) );
    free(widget->wid_info);
    free(widget);
}

void SetBounds_DoubleChooserWidget(GLWidget *widget, SDL_Rect *b)
{
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET !=DOUBLECHOOSERWIDGET) {
    	error("Wrong widget type for SetBounds_DoubleChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    widget->bounds.x = b->x;
    widget->bounds.y = b->y;
    widget->bounds.w = b->w;
    widget->bounds.h = b->h;
    if(widget->guiarea) {
    	widget->guiarea->bounds.x = b->x;
    	widget->guiarea->bounds.y = b->y;
    	widget->guiarea->bounds.w = b->w;
    	widget->guiarea->bounds.h = b->h;
    }
    DoubleChooserWidget *tmp = (DoubleChooserWidget *)(widget->wid_info);
    SDL_Rect rab;
    SDL_Rect lab;
    
    lab.h = rab.h = tmp->valuetex.height-4;
    lab.w = rab.w = rab.h;
    lab.y = rab.y = widget->bounds.y + (widget->bounds.h-rab.h)/2;
    rab.x = widget->bounds.x + widget->bounds.w - rab.w -2/*>_|*/;
    lab.x = rab.x - tmp->valuespace/*_value*/ -2/*<_value_>*/ - lab.w;
	    
    SetBounds_GLWidget(tmp->leftarrow,&lab);
    SetBounds_GLWidget(tmp->rightarrow,&rab);
}

void DoubleChooserWidget_Add( void *data )
{
    if (!data) return;
    DoubleChooserWidget *tmp = ((DoubleChooserWidget *)((GLWidget *)data)->wid_info);

    double step;
    if (tmp->direction > 0)
    	step = ((double)clientFPS)*10.0;
    else
    	step = 1000.0;
    
    if ( *(tmp->value) < (*(tmp->max)) ) {
    	(*(tmp->value)) = MIN( (*(tmp->value))+((*(tmp->max))-(*(tmp->min)))/step,(*(tmp->max)) );
    	if ( (*(tmp->value)) > (*(tmp->min)))
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	if ( (*(tmp->value)) >= (*(tmp->max)))
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
	tmp->direction = 2;
    	if (tmp->action)
    	    tmp->action(tmp->actiondata);
    	char valuetext[16];
	snprintf(valuetext,15,"%1.2f",*(tmp->value));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%1.2f) texture for doubleChooserWidget!\n",*(tmp->value));
    } else {
    	((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
    }
}

void DoubleChooserWidget_Subtract( void *data )
{
    if (!data) return;
    DoubleChooserWidget *tmp = ((DoubleChooserWidget *)((GLWidget *)data)->wid_info);

    double step;
    if (tmp->direction < 0)
    	step = ((double)clientFPS)*10.0;
    else
    	step = 1000.0;

    if ( *(tmp->value) > (*(tmp->min)) ) {
    	(*(tmp->value)) = MAX( (*(tmp->value))-((*(tmp->max))-(*(tmp->min)))/step,(*(tmp->min)) );
    	if ( (*(tmp->value)) < (*(tmp->max)))
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
    	if ( (*(tmp->value)) <= (*(tmp->min)))
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
	tmp->direction = -2;
    	if (tmp->action)
    	    tmp->action(tmp->actiondata);
    	char valuetext[16];
	snprintf(valuetext,15,"%1.2f",*(tmp->value));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%1.2f) texture for doubleChooserWidget!\n",*(tmp->value));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_DoubleChooserWidget(widget_list_t *LI)
{
    if (!LI) return;
    static int bg_color     = 0x0000ff88;
    static int name_color   = 0xffff66ff;
    static int value_color  = 0x00ff00ff;
    
    GLWidget *widget = (GLWidget *)(LI->DrawData);
    DoubleChooserWidget *wid_info = (DoubleChooserWidget *)(widget->wid_info);

    if (wid_info->direction > 0) --(wid_info->direction);
    else if (wid_info->direction < 0) ++(wid_info->direction);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    set_alphacolor(bg_color);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();
    disp_text(&(wid_info->nametex), name_color, LEFT, CENTER, widget->bounds.x+2/*|_text*/, draw_height - widget->bounds.y - widget->bounds.h/2, true);
    disp_text(&(wid_info->valuetex), value_color, RIGHT, CENTER, wid_info->rightarrow->bounds.x-1/*value_>*/-2/*>_|*/, draw_height - widget->bounds.y - widget->bounds.h/2, true );
}

GLWidget *Init_DoubleChooserWidget(widget_list_t *list, font_data *font, const char *name, double *value, double *min, double *max,void (*action)(void *data),void *actiondata)
{
    int valuespace;
    GLWidget *tmp = malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_IntChooserWidget");
	return NULL;
    }
    tmp->wid_info   = malloc(sizeof(DoubleChooserWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_IntChooserWidget");
	return NULL;
    }
    
    /* hehe ugly hack to guess max size of value strings
     * monospace font is preferred
     */
    string_tex_t tmp_tex;
    if (render_text(font,"5.55",&tmp_tex)) {
    	free_string_texture(&tmp_tex);
	valuespace = tmp_tex.width+4;
    } else {
    	valuespace = 27;
    }
    
    DoubleChooserWidget *wid_info = tmp->wid_info;
    tmp->WIDGET     = DOUBLECHOOSERWIDGET;
    if (render_text(font,name,&(wid_info->nametex))) {
    	char valuetext[16];
	snprintf(valuetext,15,"%1.2f",*value);
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font     = font;
	    wid_info->value    = value;
	    wid_info->min      = min;
	    wid_info->max      = max;
	    wid_info->valuespace = valuespace;
	    wid_info->direction = 0;
	    wid_info->action	= action;
	    wid_info->actiondata = actiondata;
    	    
	    /* this needs to be the first thing added, or it will cover the sub-parts */
	    tmp->listPtr = AppendListItem( list, Paint_DoubleChooserWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );

	    if (!(wid_info->leftarrow  = Init_ArrowWidget(list,LEFTARROW,12,16,DoubleChooserWidget_Subtract,tmp))) {
    	    	DelListItem(list,tmp->listPtr);
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
		error("Init_DoubleChooserWidget couldn't init leftarrow!");
		return NULL;
	    } 	
	    if (*value <= *min)	((ArrowWidget *)(wid_info->leftarrow->wid_info))->locked = true;

    	    if (!(wid_info->rightarrow = Init_ArrowWidget(list,RIGHTARROW,12,16,DoubleChooserWidget_Add,tmp))) {
    	    	DelListItem(list,tmp->listPtr);
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
	    	error("Init_DoubleChooserWidget couldn't init rightarrow!");
		return NULL;
	    }
	    if (*value >= *max)	((ArrowWidget *)(wid_info->rightarrow->wid_info))->locked = true;

    	    tmp->bounds.x   = 0;
    	    tmp->bounds.y   = 0;
    	    tmp->bounds.w   = 2/*|_text*/+ wid_info->nametex.width +5/*text___<*/ + valuespace/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;

    	    tmp->Draw	    	= Paint_DoubleChooserWidget;
    	    tmp->GuiReg  	= GuiReg;
    	    tmp->GuiUnReg  	= GuiUnReg;
    	    tmp->Close  	= Close_DoubleChooserWidget;
    	    tmp->SetBounds  	= SetBounds_DoubleChooserWidget;

    	    tmp->button     	= NULL;
    	    tmp->buttondata 	= NULL;
    	    tmp->motion     	= NULL;
    	    tmp->motiondata 	= NULL;
    	    tmp->hover	    	= NULL;
    	    tmp->hoverdata  	= NULL;
    	    tmp->guiarea  	= NULL;
	    tmp->next	    	= NULL;
	    return tmp;
    	} else free_string_texture(&(wid_info->nametex));
    }
    free(tmp);
    error("Failed to initialize Init_DoubleChooserWidget %s (couldn't render text)",name);
    return NULL;
}
/****************************/
/* End: DoubleChooserWidget */
/****************************/
