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

#include "xpclient.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "sdlpaint.h"
#include "images.h"
#include "text.h"
#include "glwidgets.h"

/****************************************************/
/* BEGIN: Main GLWidget stuff	    	    	    */
/****************************************************/

void DrawGLWidgetsi( GLWidget *list );
GLWidget *FindGLWidgeti( GLWidget *widget, Uint16 x, Uint16 y );

/* only supposed to take care of mallocs done on behalf of the
 * appropriate Init_<foo> function
 */
void Close_WidgetTree ( GLWidget *widget )
{
    if (!widget) return;
    
    Close_WidgetTree ( widget->next );
    Close_WidgetTree ( widget->children );
        
    if (widget->Close) widget->Close(widget);

    if (widget->wid_info) free(widget->wid_info);
    free(widget);
}

void Close_Widget (GLWidget *widget)
{
    if (!widget) {
    	error("NULL passed to Close_Widget!");
	return;
    }

    Close_WidgetTree( widget->children );

    if (widget->Close) widget->Close(widget);

    free(widget->wid_info);
    free(widget);
}

/* IMPORTANT: compound widgets need to edit this function */
void SetBounds_GLWidget(GLWidget *widget, SDL_Rect *b)
{
    if (!widget) {
    	error("NULL widget passed to SetBounds_GLWidget!");
    	return;
    }
    if (!b) {
    	error("NULL bounds passed to SetBounds_GLWidget!");
    	return;
    }
    
    
    if (widget->SetBounds) widget->SetBounds(widget,b);
    else {
    	widget->bounds.x = b->x;
    	widget->bounds.y = b->y;
    	widget->bounds.w = b->w;
    	widget->bounds.h = b->h;
    }
}

/* Eventually this will be the only visible initializer I guess */
GLWidget *Init_OptionWidget(font_data *font, xp_option_t *opt)
{
    if (!opt) {
    	error("Faulty parameter to Init_DoubleChooserWidget: opt is a NULL pointer!");
	return NULL;
    }
    
    switch ( opt->type ) {
    	case xp_bool_option:
	    return Init_BoolChooserWidget(font,opt);
    	case xp_int_option:
	    return Init_IntChooserWidget(font,opt);
    	case xp_double_option:
	    return Init_DoubleChooserWidget(font,opt);
    	default:
	    return NULL;
    }
}

bool AppendGLWidgetList( GLWidget **list, GLWidget *item )
{
    GLWidget **curr;
    if (!list) {
    	error("No list holder for Append2List %i");
    	return false;
    }

    curr = list;
    while (*curr) {
    	curr = &((*curr)->next);
    }
    *curr = item;

    return true;
}

void PrependGLWidgetList( GLWidget **list, GLWidget *item )
{
    GLWidget **curr;
    if (!list) {
    	error("No list holder for PrependGLWidgetList");
    	return;
    }
    if (!item) {
    	error("Null item sent to PrependGLWidgetList");
    }
    
    curr = &item;
    while (*curr) {
    	curr = &((*curr)->next);
    }

    *curr = *list;
    *list = item;
}

bool DelGLWidgetListItem( GLWidget **list, GLWidget *widget )
{
    GLWidget **curr;
    if (!list) {
    	error("No list holder for DelGLWidgetListItem");
    	return false;
    }
    if (!widget) {
    	error("Null widget sent to DelGLWidgetListItem");
	return false;
    }
    
    curr = list;
    while (*curr) {
    	if (*curr == widget) {
	    *curr = (*curr)->next;
	    return true;
	}
    	curr = &((*curr)->next);
    }
    
    return false;
}

/* 
 * Traverses a widget tree and calls Draw for each widget
 * the order is widget then its children (first to last)
 * then it moves onto the next widget in the list
 */
void DrawGLWidgetsi( GLWidget *list )
{
    GLWidget *curr;
    curr = list;
    while (curr) {
    	if (curr->Draw) curr->Draw(curr);
	DrawGLWidgetsi(curr->children);
	curr = curr->next;
    }
}
void DrawGLWidgets( void )
{
    DrawGLWidgetsi( MainWidgetList );
}

/*
 * Similar to DrawGLWidgets, but this one needs to traverse the
 * tree in reverse order! (since the things painted last will
 * be seen ontop, thus should get first pick of events
 * So it will descend to the last widget in teh list's last child
 * then traverse back trying to find the target.
 */
/*
 * Possibly this function will be hidden, and
 * GLWidget *FindGLWidget( Uint16 x, Uint16 y );
 * visible only. (so that nobody passes a non-visible
 * widget list)
 */
GLWidget *FindGLWidgeti( GLWidget *widget, Uint16 x, Uint16 y )
{
    if ( !widget ) return NULL;
    
    GLWidget *tmp;
    if ( (tmp = FindGLWidgeti( widget->next, x, y )) ) {
    	return tmp;
    }
    
    if ( (tmp = FindGLWidgeti( widget->children, x, y )) ) {
    	return tmp;
    }
    
    if(     (x >= widget->bounds.x) && (x <= (widget->bounds.x + widget->bounds.w))
    	&&  (y >= widget->bounds.y) && (y <= (widget->bounds.y + widget->bounds.h))
    	) {
    	return widget;
    } else {
    	return NULL;
    }
}
GLWidget *FindGLWidget( Uint16 x, Uint16 y )
{
    return FindGLWidgeti( MainWidgetList, x, y );
}

/****************************************************/
/* END: Main GLWidget stuff 	    	    	    */
/****************************************************/

/**********************/
/* Begin:  ArrowWidget*/
/**********************/
void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
void Paint_ArrowWidget(GLWidget *widget);

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

void Paint_ArrowWidget(GLWidget *widget)
{
    if (!widget) return;
    static int normalcolor  = 0xff0000ff;
    static int presscolor   = 0x00ff00ff;
    static int tapcolor     = 0xffffffff;
    static int lockcolor    = 0x88000088;
     
    GLWidget *tmp = widget;
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

GLWidget *Init_ArrowWidget( ArrowWidget_dir_t direction,int width, int height,
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
    tmp->Close	    	= NULL;
    tmp->SetBounds  	= NULL; /* do we want to be able to resize this? */
    tmp->button     	= button_ArrowWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->children   	= NULL;
    tmp->next	    	= NULL;
    return tmp;
}

/********************/
/* End:  ArrowWidget*/
/********************/

/**********************/
/* Begin: SlideWidget*/
/**********************/
void button_SlideWidget( Uint8 button, Uint8 state, Uint16 x, Uint16 y, void *data);
void motion_SlideWidget( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data);
void Paint_SlideWidget( GLWidget *widget );

void button_SlideWidget( Uint8 button, Uint8 state, Uint16 x, Uint16 y, void *data)
{
    if (!data) return;

    SlideWidget *tmp = (SlideWidget *)(((GLWidget *)data)->wid_info);
    if (state == SDL_PRESSED && !(tmp->sliding)) {
	if (button == 1) {
    	    tmp->sliding = true;
	}
    }
    if (state == SDL_RELEASED) {
	if (button == 1) {
    	    tmp->sliding = false;
	    if (tmp->release) tmp->release(tmp->releasedata);
	}
    }
}

void Paint_SlideWidget(GLWidget *widget)
{
    if (!widget) return;
    static int normalcolor  = 0xff0000ff;
    static int presscolor   = 0x00ff00ff;
    static int lockcolor  = 0x333333ff;
     
    GLWidget *tmp = widget;
    SDL_Rect *b = &(tmp->bounds);
    SlideWidget *wid_info = (SlideWidget *)(tmp->wid_info);
    
    if (wid_info->locked) {
    	set_alphacolor( lockcolor );
    } else if (wid_info->sliding) {
    	set_alphacolor( presscolor );
    } else {
    	set_alphacolor( normalcolor );
    }

    glBegin(GL_QUADS);
    	glVertex2i(b->x     	, b->y);
    	glVertex2i(b->x + b->w	, b->y);
    	glVertex2i(b->x + b->w	, b->y + b->h);
    	glVertex2i(b->x     	, b->y + b->h);
    glEnd();
}

GLWidget *Init_SlideWidget( bool locked,
    	     void (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data ), void *motiondata,
	     void (*release)(void *releasedata),void *releasedata
	     )
{
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_SlideWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(SlideWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_SlideWidget");
	return NULL;
    }
    tmp->WIDGET     	= SLIDEWIDGET;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= 10;
    tmp->bounds.h   	= 10;
    ((SlideWidget *)tmp->wid_info)->sliding = false;
    ((SlideWidget *)tmp->wid_info)->locked = locked;
    ((SlideWidget *)tmp->wid_info)->release = release;
    ((SlideWidget *)tmp->wid_info)->releasedata = releasedata;
    tmp->Draw	    	= Paint_SlideWidget;
    tmp->Close	    	= NULL;
    tmp->SetBounds  	= NULL;
    tmp->button     	= button_SlideWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= motion;
    tmp->motiondata 	= motiondata;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->children   	= NULL;
    tmp->next	    	= NULL;
    return tmp;
}

/********************/
/* End: SlideWidget*/
/********************/

/*************************/
/* Begin: ScrollbarWidget*/
/*************************/
void motion_ScrollbarWidget( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data);
void release_ScrollbarWidget( void *releasedata);
void Paint_ScrollbarWidget(GLWidget *widget);
void SetBounds_ScrollbarWidget(GLWidget *widget, SDL_Rect *b);
void Close_ScrollbarWidget (GLWidget *widget);

void Close_ScrollbarWidget (GLWidget *widget)
{
    if (!widget) return;
    if (widget->WIDGET !=SCROLLBARWIDGET) {
    	error("Wrong widget type for Close_ScrollbarWidget [%i]",widget->WIDGET);
	return;
    }
}

void SetBounds_ScrollbarWidget(GLWidget *widget, SDL_Rect *b)
{
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET !=SCROLLBARWIDGET) {
    	error("Wrong widget type for SetBounds_ScrollbarWidget [%i]",widget->WIDGET);
	return;
    }
    
    widget->bounds.x = b->x;
    widget->bounds.y = b->y;
    widget->bounds.w = b->w;
    widget->bounds.h = b->h;

    ScrollbarWidget *tmp = (ScrollbarWidget *)(widget->wid_info);

    SDL_Rect sb;
    switch (tmp->dir) {
    	case SB_VERTICAL:
    	    sb.x = b->x;
    	    sb.w = b->w;
	    sb.y = b->y + (int)(b->h*tmp->pos);
	    sb.h = (int)(b->h*tmp->size);
	    break;
	case SB_HORISONTAL:
    	    sb.y = b->y;
    	    sb.h = b->h;
	    sb.x = b->x + (int)(b->w*tmp->pos);
	    sb.w = (int)(b->w*tmp->size);
	    break; 
    	default :
	    error("bad direction for Scrollbar in SetBounds_ScrollbarWidget!");
	    return;
    }
    	    
    SetBounds_GLWidget(tmp->slide,&sb);
}

void Paint_ScrollbarWidget(GLWidget *widget)
{
    static int bgcolor  = 0x0000ff88;
     
    SDL_Rect *b = &(widget->bounds);
    
    set_alphacolor( bgcolor );
    
    glBegin(GL_QUADS);
    	glVertex2i(b->x     	, b->y);
    	glVertex2i(b->x + b->w	, b->y);
    	glVertex2i(b->x + b->w	, b->y + b->h);
    	glVertex2i(b->x     	, b->y + b->h);
    glEnd();
}

void motion_ScrollbarWidget( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data)
{
    if (!data) return;

    GLWidget *tmp = (GLWidget *)data;
    ScrollbarWidget *wid_info = (ScrollbarWidget *)tmp->wid_info;
    GLWidget *slide = wid_info->slide;
    
    Sint16 *coord1, *coord2, min, max, size, move;
   
    switch (wid_info->dir) {
    	case SB_VERTICAL:
	    coord1 = &(slide->bounds.y);
	    min = tmp->bounds.y;
	    size = slide->bounds.h;
	    max = min + tmp->bounds.h;
	    move = yrel;
	    break;
	case SB_HORISONTAL:
	    coord1 = &(slide->bounds.x);
	    min = tmp->bounds.x;
	    size = slide->bounds.w;
	    max = min + tmp->bounds.w;
	    move = xrel;
	    break;
    	default :
	    error("bad direction for Scrollbar in motion_ScrollbarWidget!");
	    return;
    }
    
    wid_info->oldmoves += move;
    
    if (wid_info->oldmoves > 0) {
    	*coord2 = MIN(max-size,*coord1+wid_info->oldmoves);
    } else if (wid_info->oldmoves < 0) {
    	*coord2 = MAX(min,*coord1+wid_info->oldmoves);
    }
    wid_info->oldmoves -= *coord2 - *coord1;
    *coord1 = *coord2;

    GLfloat oldpos = wid_info->pos;
    wid_info->pos = ((GLfloat)(*coord1 - min))/((GLfloat)(max - min));
    
    if ( (oldpos != wid_info->pos) && wid_info->poschange )
    	wid_info->poschange(wid_info->pos,wid_info->poschangedata);
}

void release_ScrollbarWidget( void *releasedata ) {
    if (!releasedata) return;

    GLWidget *tmp = (GLWidget *)releasedata;
    ScrollbarWidget *wid_info = (ScrollbarWidget *)tmp->wid_info;
    
    wid_info->oldmoves = 0;
}

GLWidget *Init_ScrollbarWidget( bool locked, GLfloat pos, GLfloat size,
    	    	    	    	ScrollWidget_dir_t dir,
    	    	    	    	void (*poschange)( GLfloat pos , void *poschangedata), void *poschangedata )
{
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_ScrollbarWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(ScrollbarWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_ScrollbarWidget");
	return NULL;
    }
    tmp->WIDGET     	= SCROLLBARWIDGET;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= 10;
    tmp->bounds.h   	= 10;
    tmp->Draw	    	= Paint_ScrollbarWidget;
    tmp->Close	    	= Close_ScrollbarWidget;
    tmp->SetBounds  	= SetBounds_ScrollbarWidget;
    tmp->button     	= NULL; /*add pgUp, pgDown here later */
    tmp->buttondata 	= NULL; /*add pgUp, pgDown here later */
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    ((ScrollbarWidget *)tmp->wid_info)->pos = MAX(0.0f,MIN(1.0f,pos));
    ((ScrollbarWidget *)tmp->wid_info)->size = MAX(0.0f,MIN(1.0f,size));
    ((ScrollbarWidget *)tmp->wid_info)->dir = dir;
    ((ScrollbarWidget *)tmp->wid_info)->oldmoves = 0;
    ((ScrollbarWidget *)tmp->wid_info)->poschange = poschange;
    ((ScrollbarWidget *)tmp->wid_info)->poschangedata = poschangedata;
    ((ScrollbarWidget *)tmp->wid_info)->slide = Init_SlideWidget(locked,motion_ScrollbarWidget, tmp, release_ScrollbarWidget, tmp);
    tmp->children   	= NULL;
    if ( !(((ScrollbarWidget *)tmp->wid_info)->slide) ) {
    	error("Failed to make a SlideWidget for Init_ScrollbarWidget");
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
    AppendGLWidgetList(&(tmp->children), ((ScrollbarWidget *)tmp->wid_info)->slide);
    tmp->next	    	= NULL;
    return tmp;
}
/*************************/
/*   End: ScrollbarWidget*/
/*************************/

/***********************************/
/* Begin:  LabeledRadiobuttonWidget*/
/***********************************/
void button_LabeledRadiobuttonWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
void Paint_LabeledRadiobuttonWidget( GLWidget *widget );

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

void Paint_LabeledRadiobuttonWidget(GLWidget *widget)
{
    if (!widget) return;
    static int false_bg_color	= 0x00000044;
    static int true_bg_color	= 0x00000044;
    static int false_text_color	= 0xff0000ff;
    static int true_text_color	= 0x00ff00ff;
     
    GLWidget *tmp = widget;
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

GLWidget *Init_LabeledRadiobuttonWidget( string_tex_t *ontex, string_tex_t *offtex,
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
    tmp->Close	    	= NULL;
    tmp->SetBounds  	= NULL;
    tmp->button     	= button_LabeledRadiobuttonWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->children   	= NULL;
    tmp->next	    	= NULL;
    return tmp;
}
/*********************************/
/* End:  LabeledRadiobuttonWidget*/
/*********************************/

/*****************************/
/* Begin:  BoolChooserWidget */
/*****************************/
void Paint_BoolChooserWidget(GLWidget *widget);
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
    Set_bool_option(((BoolChooserWidget *)data)->opt, state);
}

void Paint_BoolChooserWidget(GLWidget *widget)
{
    if (!widget) return;
    static int bg_color     = 0x0000ff88;
    static int name_color   = 0xffff66ff;
    
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

GLWidget *Init_BoolChooserWidget(font_data *font, xp_option_t *opt)
{
    if (!opt) {
    	error("Faulty parameter to Init_BoolChooserWidget: opt is a NULL pointer!");
	return NULL;
    }
    if (opt->type != xp_bool_option) {
    	error("Faulty parameter to Init_BoolChooserWidget: opt is not a bool option!");
	return NULL;
    }
    if (!(opt->bool_ptr)) {
    	error("bool misssing for Init_BoolChooserWidget.");
	return NULL;
    }
    if (!(opt->name) || !strlen(opt->name) ) {
    	error("name misssing for Init_BoolChooserWidget.");
	return NULL;
    }

    
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
    
    if ( !render_text(font,opt->name,&(wid_info->nametex)) ) {
    	error("Failed to render '%s' in Init_BoolChooserWidget",opt->name);
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
    
    tmp->children   	= NULL;
    if ( !(wid_info->buttonwidget = Init_LabeledRadiobuttonWidget(BoolChooserWidget_ontex,
    	    	    	    	    	BoolChooserWidget_offtex, BoolChooserWidget_SetValue,
					wid_info, *(opt->bool_ptr))) ) {
    	error("Failed to make a LabeledRadiobuttonWidget for Init_BoolChooserWidget");
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
    AppendGLWidgetList(&(tmp->children),wid_info->buttonwidget);
        
    tmp->WIDGET     	= BOOLCHOOSERWIDGET;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= 2+wid_info->nametex.width+5+wid_info->buttonwidget->bounds.w+2;
    tmp->bounds.h   	= 1+ MAX( wid_info->nametex.height,wid_info->buttonwidget->bounds.h) +1 ;
    
    wid_info->opt 	= opt;

    tmp->Draw	    	= Paint_BoolChooserWidget;
    tmp->Close  	= Close_BoolChooserWidget;
    tmp->SetBounds  	= SetBounds_BoolChooserWidget;

    tmp->button     	= NULL;
    tmp->buttondata 	= NULL;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
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
void Paint_IntChooserWidget(GLWidget *widget);
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
    
    if (*(tmp->opt->int_ptr) < tmp->opt->int_maxval) {
    	Set_int_option(tmp->opt, MIN( *(tmp->opt->int_ptr) + step, tmp->opt->int_maxval));
    	if ( (*(tmp->opt->int_ptr)) > tmp->opt->int_minval)
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->int_ptr)) == tmp->opt->int_maxval)
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
	tmp->direction = 2;
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(tmp->opt->int_ptr));
	free_string_texture(&(tmp->valuetex));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%s=%i) texture for IntChooserWidget!\n",tmp->opt->name,*(tmp->opt->int_ptr));
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

    if (*(tmp->opt->int_ptr) > tmp->opt->int_minval) {
    	Set_int_option(tmp->opt, MAX( (*(tmp->opt->int_ptr)) - step, tmp->opt->int_minval) );
    	if ( (*(tmp->opt->int_ptr)) < tmp->opt->int_maxval)
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->int_ptr)) == tmp->opt->int_minval)
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
	tmp->direction = -2;
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(tmp->opt->int_ptr));
	free_string_texture(&(tmp->valuetex));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%s=%i) texture for IntChooserWidget!\n",tmp->opt->name,*(tmp->opt->int_ptr));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_IntChooserWidget(GLWidget *widget)
{
    if (!widget) {
    	error("Paint_IntChooserWidget: argument is NULL!");
	return;
    }
    static int bg_color     = 0x0000ff88;
    static int name_color   = 0xffff66ff;
    static int value_color  = 0x00ff00ff;
    
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

GLWidget *Init_IntChooserWidget(font_data *font, xp_option_t *opt)
{
    if (!opt) {
    	error("Faulty parameter to Init_IntChooserWidget: opt is a NULL pointer!");
	return NULL;
    }
    if (opt->type != xp_int_option) {
    	error("Faulty parameter to Init_IntChooserWidget: opt is not an int option!");
	return NULL;
    }
    if (!(opt->int_ptr)) {
    	error("int misssing for Init_IntChooserWidget.");
	return NULL;
    }
    if (!(opt->name) || !strlen(opt->name) ) {
    	error("name misssing for Init_IntChooserWidget.");
	return NULL;
    }

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
    if (render_text(font,opt->name,&(wid_info->nametex))) {
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(opt->int_ptr));
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font  	= font;
	    wid_info->opt   	= opt;
	    wid_info->valuespace = valuespace;
	    wid_info->direction = 0;

    	    tmp->children   	= NULL;
	    if (!(wid_info->leftarrow  = Init_ArrowWidget(LEFTARROW,12,16,IntChooserWidget_Subtract,tmp))) {
		free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
		error("Init_IntChooserWidget couldn't init leftarrow!");
		return NULL;
	    } 	
	    AppendGLWidgetList(&(tmp->children),wid_info->leftarrow);
	    if (*(wid_info->opt->int_ptr) <= wid_info->opt->int_minval)	((ArrowWidget *)(wid_info->leftarrow->wid_info))->locked = true;

    	    if (!(wid_info->rightarrow = Init_ArrowWidget(RIGHTARROW,12,16,IntChooserWidget_Add,tmp))) {
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
	    	error("Init_IntChooserWidget couldn't init rightarrow!");
		return NULL;
	    }
	    AppendGLWidgetList(&(tmp->children),wid_info->rightarrow);
	    if (*(wid_info->opt->int_ptr) >= wid_info->opt->int_maxval)	((ArrowWidget *)(wid_info->rightarrow->wid_info))->locked = true;

    	    tmp->bounds.x   = 0;
    	    tmp->bounds.y   = 0;
    	    tmp->bounds.w   = 2/*|_text*/+ wid_info->nametex.width +5/*text___<*/ + valuespace/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;

    	    tmp->Draw	    	= Paint_IntChooserWidget;
    	    tmp->Close  	= Close_IntChooserWidget;
    	    tmp->SetBounds  	= SetBounds_IntChooserWidget;

    	    tmp->button     	= NULL;
    	    tmp->buttondata 	= NULL;
    	    tmp->motion     	= NULL;
    	    tmp->motiondata 	= NULL;
    	    tmp->hover	    	= NULL;
    	    tmp->hoverdata  	= NULL;
	    tmp->next	    	= NULL;
	    return tmp;
    	} else free_string_texture(&(wid_info->nametex));
    }
    free(tmp);
    error("Failed to initialize Init_IntChooserWidget %s (couldn't render text)",opt->name);
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
void Paint_DoubleChooserWidget(GLWidget *widget);
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
    
    if ( *(tmp->opt->dbl_ptr) < tmp->opt->dbl_maxval ) {
    	Set_double_option( tmp->opt, MIN( (*(tmp->opt->dbl_ptr))+((tmp->opt->dbl_maxval)-(tmp->opt->dbl_minval))/step,tmp->opt->dbl_maxval ) );
    	if ( (*(tmp->opt->dbl_ptr)) > tmp->opt->dbl_minval )
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->dbl_ptr)) >= tmp->opt->dbl_maxval )
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
	tmp->direction = 2;
    	char valuetext[16];
	snprintf(valuetext,15,"%1.2f",*(tmp->opt->dbl_ptr));
	free_string_texture(&(tmp->valuetex));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%s=%1.2f) texture for doubleChooserWidget!\n",tmp->opt->name,*(tmp->opt->dbl_ptr));
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

    if ( *(tmp->opt->dbl_ptr) > tmp->opt->dbl_minval ) {
    	Set_double_option( tmp->opt, MAX( (*(tmp->opt->dbl_ptr))-((tmp->opt->dbl_maxval)-(tmp->opt->dbl_minval))/step,tmp->opt->dbl_minval ) );
    	if ( (*(tmp->opt->dbl_ptr)) < tmp->opt->dbl_maxval )
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->dbl_ptr)) <= tmp->opt->dbl_minval )
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
	tmp->direction = -2;
    	char valuetext[16];
	snprintf(valuetext,15,"%1.2f",*(tmp->opt->dbl_ptr));
	free_string_texture(&(tmp->valuetex));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%s=%1.2f) texture for doubleChooserWidget!\n",tmp->opt->name,*(tmp->opt->dbl_ptr));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_DoubleChooserWidget(GLWidget *widget)
{
    if (!widget) return;
    static int bg_color     = 0x0000ff88;
    static int name_color   = 0xffff66ff;
    static int value_color  = 0x00ff00ff;
    
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

GLWidget *Init_DoubleChooserWidget( font_data *font, xp_option_t *opt)
{
    if (!opt) {
    	error("Faulty parameter to Init_DoubleChooserWidget: opt is a NULL pointer!");
	return NULL;
    }
    if (opt->type != xp_double_option) {
    	error("Faulty parameter to Init_DoubleChooserWidget: opt is not an double option!");
	return NULL;
    }
    if (!(opt->dbl_ptr)) {
    	error("int misssing for Init_DoubleChooserWidget.");
	return NULL;
    }
    if (!(opt->name) || !strlen(opt->name) ) {
    	error("name misssing for Init_DoubleChooserWidget.");
	return NULL;
    }

    int valuespace;
    GLWidget *tmp = malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_DoubleChooserWidget");
	return NULL;
    }
    tmp->wid_info   = malloc(sizeof(DoubleChooserWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_DoubleChooserWidget");
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
    if (render_text(font,opt->name,&(wid_info->nametex))) {
    	char valuetext[16];
	snprintf(valuetext,15,"%1.2f",*(opt->dbl_ptr));
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font  	= font;
	    wid_info->opt   	= opt;
	    wid_info->valuespace = valuespace;
	    wid_info->direction = 0;
    	    
    	    tmp->children   	= NULL;
	    if (!(wid_info->leftarrow  = Init_ArrowWidget(LEFTARROW,12,16,DoubleChooserWidget_Subtract,tmp))) {
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
		error("Init_DoubleChooserWidget couldn't init leftarrow!");
		return NULL;
	    } 	
	    AppendGLWidgetList(&(tmp->children),wid_info->leftarrow);
	    if (*(wid_info->opt->dbl_ptr) <= wid_info->opt->dbl_minval ) ((ArrowWidget *)(wid_info->leftarrow->wid_info))->locked = true;

    	    if (!(wid_info->rightarrow = Init_ArrowWidget(RIGHTARROW,12,16,DoubleChooserWidget_Add,tmp))) {
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
	    	error("Init_DoubleChooserWidget couldn't init rightarrow!");
		return NULL;
	    }
	    if (*(wid_info->opt->dbl_ptr) >= wid_info->opt->dbl_maxval) ((ArrowWidget *)(wid_info->rightarrow->wid_info))->locked = true;
	    AppendGLWidgetList(&(tmp->children),wid_info->rightarrow);

    	    tmp->bounds.x   = 0;
    	    tmp->bounds.y   = 0;
    	    tmp->bounds.w   = 2/*|_text*/+ wid_info->nametex.width +5/*text___<*/ + valuespace/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;

    	    tmp->Draw	    	= Paint_DoubleChooserWidget;
    	    tmp->Close  	= Close_DoubleChooserWidget;
    	    tmp->SetBounds  	= SetBounds_DoubleChooserWidget;

    	    tmp->button     	= NULL;
    	    tmp->buttondata 	= NULL;
    	    tmp->motion     	= NULL;
    	    tmp->motiondata 	= NULL;
    	    tmp->hover	    	= NULL;
    	    tmp->hoverdata  	= NULL;
	    tmp->next	    	= NULL;
	    return tmp;
    	} else free_string_texture(&(wid_info->nametex));
    }
    free(tmp);
    error("Failed to initialize Init_DoubleChooserWidget %s (couldn't render text)",opt->name);
    return NULL;
}
/****************************/
/* End: DoubleChooserWidget */
/****************************/
