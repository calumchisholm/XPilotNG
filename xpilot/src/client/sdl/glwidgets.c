/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client.
 *
 * Copyright (C) 2003-2004 Erik Andersson <maximan@users.sourceforge.net>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpclient_sdl.h"

#include "sdlpaint.h"
#include "images.h"
#include "text.h"
#include "glwidgets.h"

/****************************************************/
/* BEGIN: Main GLWidget stuff	    	    	    */
/****************************************************/

void DrawGLWidgetsi( GLWidget *list, int x, int y, int w, int h );
GLWidget *FindGLWidgeti( GLWidget *widget, Uint16 x, Uint16 y );

GLWidget *Init_EmptyBaseGLWidget( void )
{
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) return NULL;
    tmp->WIDGET     	= -1;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= 0;
    tmp->bounds.h   	= 0;
    tmp->wid_info   	= NULL;
    tmp->Draw	    	= NULL;
    tmp->Close	    	= NULL;
    tmp->SetBounds  	= NULL;
    tmp->button     	= NULL;
    tmp->buttondata 	= NULL;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->list   	= NULL;
    tmp->children   	= NULL;
    tmp->next	    	= NULL;
    return tmp;
}


/* only supposed to take care of mallocs done on behalf of the
 * appropriate Init_<foo> function
 */
void Close_WidgetTree ( GLWidget **widget )
{
    if (!widget) return;
    if (!(*widget)) return;
    
    Close_WidgetTree ( &((*widget)->next) );
    Close_WidgetTree ( &((*widget)->children) );
        
    if ((*widget)->Close) (*widget)->Close(*widget);

    if ((*widget)->wid_info) free((*widget)->wid_info);
    free(*widget);
    *widget = NULL;
}

void Close_Widget ( GLWidget **widget )
{
    GLWidget *tmp;

    if (!widget) {
    	error("NULL passed to Close_Widget!");
	return;
    }
    if (!(*widget)) {
    	error("pointer passed to Close_Widget points to NULL !");
	return;
    }

    Close_WidgetTree( &((*widget)->children) );

    if ((*widget)->Close) (*widget)->Close(*widget);

    if ((*widget)->wid_info) free((*widget)->wid_info);
    tmp = *widget;
    *widget = (*widget)->next;
    free(tmp);
}

/* IMPORTANT: compound widgets need to edit this function */
void SetBounds_GLWidget( GLWidget *widget, SDL_Rect *b )
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
GLWidget *Init_OptionWidget( font_data *font, xp_option_t *opt )
{
    if (!opt) {
    	error("Faulty parameter to Init_DoubleChooserWidget: opt is a NULL pointer!");
	return NULL;
    }
    
    switch ( Option_get_type(opt) ) {
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
    	error("No list holder for AppendGLWidgetList %i");
    	return false;
    }
    if (!item) {
    	error("Null item sent to AppendGLWidgetList");
    }

    item->list = list;

    curr = list;
    while (*curr) {
    	/* Just a trivial check if item already is in the list
	 * doesn't check for item's trailers however!
	 * Make sure items aren't added twice!
	 */
	if (*curr == item) {
	    error("AppendGLWidgetList: item is already in the list!");
	    return false;
	}
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
    
    item->list = list;

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
    
    /* We don't clear widget->list here, because it still 'belongs'
     * to list until we link it somewhere else
     */
    
    curr = list;
    while (*curr) {
    	if (*curr == widget) {
	    *curr = (*curr)->next;
	    widget->next = NULL;
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
void DrawGLWidgetsi( GLWidget *list, int x, int y, int w, int h)
{
    int x2,y2,w2,h2;
    GLWidget *curr;
    
    curr = list;
    
    while (curr) {
    	x2 = MAX(x,curr->bounds.x);
    	y2 = MAX(y,curr->bounds.y);
    	w2 = MIN(x+w,curr->bounds.x+curr->bounds.w) - x2;
    	h2 = MIN(y+h,curr->bounds.y+curr->bounds.h) - y2;
	
	glScissor(x2, draw_height - y2 - h2, w2+1, h2+1);
	if (curr->Draw) curr->Draw(curr);
	
	DrawGLWidgetsi(curr->children,x2, y2, w2, h2);
	glScissor(x, draw_height - y - h, w, h);
	
	curr = curr->next;
    }
}
void DrawGLWidgets( void )
{
    glScissor(0, 0, draw_width, draw_height);
    glEnable(GL_SCISSOR_TEST);
    DrawGLWidgetsi( MainWidget , 0, 0, draw_width, draw_height );
    glDisable(GL_SCISSOR_TEST);
}

/*
 * Similar to DrawGLWidgets, but this one needs to traverse the
 * tree in reverse order! (since the things painted last will
 * be seen ontop, thus should get first pick of events
 * So it will descend to the last child in the list's last widget
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
    GLWidget *tmp;
    
    if ( !widget ) return NULL;
    
    if ( (tmp = FindGLWidgeti( widget->next, x, y )) ) {
    	return tmp;
    }
    
    if(     (x >= widget->bounds.x) && (x <= (widget->bounds.x + widget->bounds.w))
    	&&  (y >= widget->bounds.y) && (y <= (widget->bounds.y + widget->bounds.h)))
    {
    	if ( (tmp = FindGLWidgeti( widget->children, x, y )) ) {
    	    return tmp;
    	} else return widget;
    } else {
    	return NULL;
    }
}
GLWidget *FindGLWidget( Uint16 x, Uint16 y )
{
    return FindGLWidgeti( MainWidget, x, y );
}

/****************************************************/
/* END: Main GLWidget stuff 	    	    	    */
/****************************************************/

/**********************/
/* Begin:  ArrowWidget*/
/**********************/
void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data );
void Paint_ArrowWidget( GLWidget *widget );

void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data )
{
    ArrowWidget *tmp;
    
    if (!data) return;
    tmp = (ArrowWidget *)(((GLWidget *)data)->wid_info);
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

void Paint_ArrowWidget( GLWidget *widget )
{
    GLWidget *tmp;
    SDL_Rect *b;
    ArrowWidget *wid_info;
    ArrowWidget_dir_t dir;
    static int normalcolor  = 0xff0000ff;
    static int presscolor   = 0x00ff00ff;
    static int tapcolor     = 0xffffffff;
    static int lockcolor    = 0x88000088;
    
	if (!widget) return;	
    tmp = widget;
    b = &(tmp->bounds);
    wid_info = (ArrowWidget *)(tmp->wid_info);
    
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
    
    dir = wid_info->direction;
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
    GLWidget *tmp	= Init_EmptyBaseGLWidget();
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
    tmp->bounds.w   	= width;
    tmp->bounds.h   	= height;
    ((ArrowWidget *)tmp->wid_info)->press = false;
    ((ArrowWidget *)tmp->wid_info)->tap = false;
    ((ArrowWidget *)tmp->wid_info)->locked = false;
    ((ArrowWidget *)tmp->wid_info)->action = action;
    ((ArrowWidget *)tmp->wid_info)->actiondata = actiondata;
    tmp->Draw	    	= Paint_ArrowWidget;
    tmp->button     	= button_ArrowWidget;
    tmp->buttondata 	= tmp;
    return tmp;
}

/********************/
/* End:  ArrowWidget*/
/********************/

/**********************/
/* Begin: SlideWidget*/
/**********************/
void button_SlideWidget( Uint8 button, Uint8 state, Uint16 x, Uint16 y, void *data );
void Paint_SlideWidget( GLWidget *widget );

void button_SlideWidget( Uint8 button, Uint8 state, Uint16 x, Uint16 y, void *data )
{
    SlideWidget *tmp;
    
    if (!data) return;

    tmp = (SlideWidget *)(((GLWidget *)data)->wid_info);
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

void Paint_SlideWidget( GLWidget *widget )
{
    GLWidget *tmp;
    SDL_Rect *b;
    SlideWidget *wid_info;

   
    static int normalcolor  = 0xff0000ff;
    static int presscolor   = 0x00ff00ff;
    static int lockcolor  = 0x333333ff;
    int color;

    if (!widget) return;
    tmp = widget;
    b = &(tmp->bounds);
    wid_info = (SlideWidget *)(tmp->wid_info);
    
    if (wid_info->locked) {
    	color = lockcolor;
    } else if (wid_info->sliding) {
    	color = presscolor;
    } else {
    	color = normalcolor;
    }

    glBegin(GL_QUADS);
    	set_alphacolor(color);
	glVertex2i(b->x     	, b->y);
    	set_alphacolor(color);
    	glVertex2i(b->x + b->w	, b->y);
    	set_alphacolor(color & 0xffffff77);
    	glVertex2i(b->x + b->w	, b->y + b->h);
    	set_alphacolor(color & 0xffffff77);
    	glVertex2i(b->x     	, b->y + b->h);
    glEnd();
}

GLWidget *Init_SlideWidget( bool locked,
    	     void (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data ), void *motiondata,
	     void (*release)(void *releasedata),void *releasedata )
{
    GLWidget *tmp	= Init_EmptyBaseGLWidget();
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
    tmp->button     	= button_SlideWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= motion;
    tmp->motiondata 	= motiondata;
    return tmp;
}

/********************/
/* End: SlideWidget*/
/********************/

/*************************/
/* Begin: ScrollbarWidget*/
/*************************/
void motion_ScrollbarWidget( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data );
void release_ScrollbarWidget( void *releasedata );
void Paint_ScrollbarWidget( GLWidget *widget );
void SetBounds_ScrollbarWidget( GLWidget *widget, SDL_Rect *b );
void Close_ScrollbarWidget ( GLWidget *widget );

void Close_ScrollbarWidget ( GLWidget *widget )
{
    if (!widget) return;
    if (widget->WIDGET !=SCROLLBARWIDGET) {
    	error("Wrong widget type for Close_ScrollbarWidget [%i]",widget->WIDGET);
	return;
    }
}

void SetBounds_ScrollbarWidget( GLWidget *widget, SDL_Rect *b )
{
    ScrollbarWidget *tmp;
    SDL_Rect sb;

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

    tmp = (ScrollbarWidget *)(widget->wid_info);

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

void Paint_ScrollbarWidget( GLWidget *widget )
{
    static int bgcolor  = 0x00000044;
    SDL_Rect *b = &(widget->bounds);
    
    set_alphacolor( bgcolor );
    
    glBegin(GL_QUADS);
    	glVertex2i(b->x     	, b->y);
    	glVertex2i(b->x + b->w	, b->y);
    	glVertex2i(b->x + b->w	, b->y + b->h);
    	glVertex2i(b->x     	, b->y + b->h);
    glEnd();
}

void motion_ScrollbarWidget( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data )
{
    GLWidget *tmp;
    ScrollbarWidget *wid_info;
    GLWidget *slide;
    Sint16 *coord1, coord2 = 0, min, max, size, move;
    GLfloat oldpos;

    if (!data) return;

    tmp = (GLWidget *)data;
    wid_info = (ScrollbarWidget *)tmp->wid_info;
    slide = wid_info->slide;
    
   
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
    
    if (!(wid_info->oldmoves)) return;
    
    if (wid_info->oldmoves > 0) {
    	coord2 = MIN(max-size,*coord1+wid_info->oldmoves);
    } else if (wid_info->oldmoves < 0) {
    	coord2 = MAX(min,*coord1+wid_info->oldmoves);
    }
    wid_info->oldmoves -= coord2 - *coord1;
    *coord1 = coord2;

    oldpos = wid_info->pos;
    wid_info->pos = ((GLfloat)(*coord1 - min))/((GLfloat)(max - min));
    
    if ( (oldpos != wid_info->pos) && wid_info->poschange )
    	wid_info->poschange(wid_info->pos,wid_info->poschangedata);
}

void release_ScrollbarWidget( void *releasedata )
{
    GLWidget *tmp;
    ScrollbarWidget *wid_info;
    if (!releasedata) return;

    tmp = (GLWidget *)releasedata;
    wid_info = (ScrollbarWidget *)tmp->wid_info;
    
    wid_info->oldmoves = 0;
}

void ScrollbarWidget_SetSlideSize( GLWidget *widget, GLfloat size )
{
    ScrollbarWidget *sb;
    
    if (!widget) return;
    if (widget->WIDGET !=SCROLLBARWIDGET) {
    	error("Wrong widget type for SetBounds_ScrollbarWidget [%i]",widget->WIDGET);
	return;
    }
    if (!(sb = (ScrollbarWidget *)(widget->wid_info))) {
    	error("ScrollbarWidget_SetSlideSize: wid_info missing!");
	return;
    }
    
    sb->size = MIN(1.0f,MAX(0.0f,size));
    
    SetBounds_ScrollbarWidget(widget,&(widget->bounds));
}

GLWidget *Init_ScrollbarWidget( bool locked, GLfloat pos, GLfloat size, ScrollWidget_dir_t dir,
    	    	    	    	void (*poschange)( GLfloat pos , void *poschangedata),
				void *poschangedata )
{
    GLWidget *tmp	= Init_EmptyBaseGLWidget();
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
    tmp->bounds.w   	= 10;
    tmp->bounds.h   	= 10;
    tmp->Draw	    	= Paint_ScrollbarWidget;
    tmp->Close	    	= Close_ScrollbarWidget;
    tmp->SetBounds  	= SetBounds_ScrollbarWidget;
    /*add pgUp, pgDown here later with button*/
    ((ScrollbarWidget *)tmp->wid_info)->pos = MAX(0.0f,MIN(1.0f,pos));
    ((ScrollbarWidget *)tmp->wid_info)->size = MAX(0.0f,MIN(1.0f,size));
    ((ScrollbarWidget *)tmp->wid_info)->dir = dir;
    ((ScrollbarWidget *)tmp->wid_info)->oldmoves = 0;
    ((ScrollbarWidget *)tmp->wid_info)->poschange = poschange;
    ((ScrollbarWidget *)tmp->wid_info)->poschangedata = poschangedata;
    ((ScrollbarWidget *)tmp->wid_info)->slide = Init_SlideWidget(locked,motion_ScrollbarWidget, tmp, release_ScrollbarWidget, tmp);
    if ( !(((ScrollbarWidget *)tmp->wid_info)->slide) ) {
    	error("Failed to make a SlideWidget for Init_ScrollbarWidget");
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
    AppendGLWidgetList(&(tmp->children), ((ScrollbarWidget *)tmp->wid_info)->slide);
    return tmp;
}
/*************************/
/*   End: ScrollbarWidget*/
/*************************/

/***********************/
/* Begin:  LabelWidget*/
/***********************/
void Paint_LabelWidget( GLWidget *widget );
void Close_LabelWidget ( GLWidget *widget );

void Close_LabelWidget( GLWidget *widget )
{
    if (!widget) return;
    if (widget->WIDGET !=LABELWIDGET) {
    	error("Wrong widget type for Close_LabelWidget [%i]",widget->WIDGET);
	return;
    }
    free_string_texture(&(((LabelWidget *)widget->wid_info)->tex));
}

bool LabelWidget_SetColor( GLWidget *widget , int *bgcolor, int *fgcolor )
{
    LabelWidget *wi;
    
    if (!widget) return false;
    if (widget->WIDGET !=LABELWIDGET) {
    	error("Wrong widget type for LabelWidget_SetColor [%i]",widget->WIDGET);
	return false;
    }
    
    if ( !(wi = widget->wid_info) ) {
    	error("LabelWidget_SetColor: widget->wid_info missing!");
	return false;
    }
    wi->bgcolor = bgcolor;
    wi->fgcolor = fgcolor;
    
    return true;
}
void Paint_LabelWidget( GLWidget *widget )
{
    GLWidget *tmp;
    SDL_Rect *b;
    LabelWidget *wid_info;
    int x, y;

    if (!widget) return;
     
    tmp = widget;
    b = &(tmp->bounds);
    wid_info = (LabelWidget *)(tmp->wid_info);
    
    if ( wid_info->bgcolor && *(wid_info->bgcolor) ) {
    	set_alphacolor(*(wid_info->bgcolor));
    
    	glBegin(GL_QUADS);
    	    glVertex2i(b->x     	,b->y);
   	    glVertex2i(b->x + b->w  ,b->y);
    	    glVertex2i(b->x + b->w  ,b->y+b->h);
    	    glVertex2i(b->x     	,b->y+b->h);
     	glEnd();
    }
    
    x = wid_info->align == LEFT   ? tmp->bounds.x :
	wid_info->align == CENTER ? tmp->bounds.x + tmp->bounds.w / 2 :
	tmp->bounds.x + tmp->bounds.w;
    y = wid_info->valign == DOWN   ? tmp->bounds.y :
	wid_info->valign == CENTER ? tmp->bounds.y + tmp->bounds.h / 2 :
	tmp->bounds.y + tmp->bounds.h;

    if (wid_info->fgcolor)
    	disp_text(&(wid_info->tex), 
		  *(wid_info->fgcolor), 
		  wid_info->align, 
		  wid_info->valign, 
		  x, 
		  draw_height - y, 
		  true);
    else
    	disp_text(&(wid_info->tex), 
		  whiteRGBA, 
		  wid_info->align, 
		  wid_info->valign, 
		  x, 
		  draw_height - y, 
		  true);
}

GLWidget *Init_LabelWidget( const char *text , int *bgcolor, int *fgcolor, int align, int valign  )
{
    GLWidget *tmp;

    if (!text) {
    	error("text missing for Init_LabelWidget.");
	return NULL;
    }
    tmp	= Init_EmptyBaseGLWidget();
    if ( !tmp ) {
        error("Failed to malloc in Init_LabelWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(LabelWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_LabelWidget");
	return NULL;
    }

    if ( !render_text(&gamefont, text, &(((LabelWidget *)tmp->wid_info)->tex)) ) {
    	free(tmp->wid_info);
    	free(tmp);
        error("Failed to render text in Init_LabelWidget");
	return NULL;
    } 
    
    tmp->WIDGET     	= LABELWIDGET;
    tmp->bounds.w   	= (((LabelWidget *)tmp->wid_info)->tex).width;
    tmp->bounds.h   	= (((LabelWidget *)tmp->wid_info)->tex).height;
    ((LabelWidget *)tmp->wid_info)->fgcolor  = fgcolor;
    ((LabelWidget *)tmp->wid_info)->bgcolor  = bgcolor;
    ((LabelWidget *)tmp->wid_info)->align    = align;
    ((LabelWidget *)tmp->wid_info)->valign   = valign;
    tmp->Draw	    	= Paint_LabelWidget;
    tmp->Close     	= Close_LabelWidget;
    return tmp;
}
/********************/
/* End:  LabelWidget*/
/********************/

/***********************************/
/* Begin:  LabeledRadiobuttonWidget*/
/***********************************/
void button_LabeledRadiobuttonWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data );
void Paint_LabeledRadiobuttonWidget( GLWidget *widget );

void button_LabeledRadiobuttonWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data )
{
    LabeledRadiobuttonWidget *tmp;
    if (!data) return;
    tmp = (LabeledRadiobuttonWidget *)(((GLWidget *)data)->wid_info);
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

void Paint_LabeledRadiobuttonWidget( GLWidget *widget )
{
    GLWidget *tmp;
    SDL_Rect *b;
    LabeledRadiobuttonWidget *wid_info;
    static int false_bg_color	= 0x00000044;
    static int true_bg_color	= 0x00000044;
    static int false_text_color	= 0xff0000ff;
    static int true_text_color	= 0x00ff00ff;

    if (!widget) return;
     
    tmp = widget;
    b = &(tmp->bounds);
    wid_info = (LabeledRadiobuttonWidget *)(tmp->wid_info);
    
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
    	    	    	    	    	void (*action)(bool state, void *actiondata),
					void *actiondata, bool start_state )
{
    GLWidget *tmp;

    if (!ontex || !(ontex->texture) || !offtex || !(offtex->texture) ) {
    	error("texure(s) missing for Init_LabeledRadiobuttonWidget.");
	return NULL;
    }
    tmp	= Init_EmptyBaseGLWidget();
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
    tmp->bounds.w   	= MAX(ontex->width,offtex->width)+5;
    tmp->bounds.h   	= MAX(ontex->height,offtex->height);
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->state  = start_state;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->ontex  = ontex;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->offtex  = offtex;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->action  = action;
    ((LabeledRadiobuttonWidget *)tmp->wid_info)->actiondata  = actiondata;

    tmp->Draw	    	= Paint_LabeledRadiobuttonWidget;
    tmp->button     	= button_LabeledRadiobuttonWidget;
    tmp->buttondata 	= tmp;
    return tmp;
}
/*********************************/
/* End:  LabeledRadiobuttonWidget*/
/*********************************/

/*****************************/
/* Begin:  BoolChooserWidget */
/*****************************/
void Paint_BoolChooserWidget( GLWidget *widget );
void BoolChooserWidget_SetValue( bool state, void *data );
void Close_BoolChooserWidget ( GLWidget *widget );
void SetBounds_BoolChooserWidget( GLWidget *widget, SDL_Rect *b );

static int num_BoolChooserWidget = 0;
static string_tex_t *BoolChooserWidget_ontex = NULL;
static string_tex_t *BoolChooserWidget_offtex = NULL;

void Close_BoolChooserWidget( GLWidget *widget )
{
    if (!widget) return;
    if (widget->WIDGET !=BOOLCHOOSERWIDGET) {
    	error("Wrong widget type for Close_BoolChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    free_string_texture( &(((BoolChooserWidget *)(widget->wid_info))->nametex) );
    
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

void SetBounds_BoolChooserWidget( GLWidget *widget, SDL_Rect *b )
{
    GLWidget *tmp;
    SDL_Rect b2;
    
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

    tmp = ((BoolChooserWidget *)(widget->wid_info))->buttonwidget;
    
    b2.h = tmp->bounds.h;
    b2.w = tmp->bounds.w;
    b2.x = widget->bounds.x + widget->bounds.w - 2 - tmp->bounds.w;
    b2.y = widget->bounds.y + 1;
	    
    SetBounds_GLWidget(tmp,&b2);
}

void BoolChooserWidget_SetValue( bool state, void *data )
{
    Set_bool_option(((BoolChooserWidget *)data)->opt, state,
		    xp_option_origin_config);
}

void Paint_BoolChooserWidget( GLWidget *widget )
{
    /* static int bg_color     = 0x0000ff88; */
    static int name_color   = 0xffff66ff;
    BoolChooserWidget *wid_info;

    if (!widget) return;
    
    wid_info = (BoolChooserWidget *)(widget->wid_info);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*set_alphacolor(bg_color);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();*/
    disp_text(&(wid_info->nametex), name_color, LEFT, CENTER, widget->bounds.x+2/*|_text*/, draw_height - widget->bounds.y - widget->bounds.h/2, true);
}

GLWidget *Init_BoolChooserWidget( font_data *font, xp_option_t *opt )
{
    GLWidget *tmp;
    BoolChooserWidget *wid_info;
    
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
    
    tmp	= Init_EmptyBaseGLWidget();
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
    
    wid_info = (BoolChooserWidget *)(tmp->wid_info);
    
    if ( !render_text(font,opt->name,&(wid_info->nametex)) ) {
    	error("Failed to render '%s' in Init_BoolChooserWidget",opt->name);
	free(tmp->wid_info);
	free(tmp);
	return NULL;
    }
    
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
    tmp->bounds.w   	= 2+wid_info->nametex.width+5+wid_info->buttonwidget->bounds.w+2;
    tmp->bounds.h   	= 1+ MAX( wid_info->nametex.height,wid_info->buttonwidget->bounds.h) +1 ;
    
    wid_info->opt 	= opt;

    tmp->Draw	    	= Paint_BoolChooserWidget;
    tmp->Close  	= Close_BoolChooserWidget;
    tmp->SetBounds  	= SetBounds_BoolChooserWidget;

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
void Paint_IntChooserWidget( GLWidget *widget );
void Close_IntChooserWidget ( GLWidget *widget );
void SetBounds_IntChooserWidget( GLWidget *widget, SDL_Rect *b );

void Close_IntChooserWidget ( GLWidget *widget )
{
    if (!widget) return;
    if (widget->WIDGET !=INTCHOOSERWIDGET) {
    	error("Wrong widget type for Close_IntChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    free_string_texture( &(((IntChooserWidget *)widget->wid_info)->nametex) );
    free_string_texture( &(((IntChooserWidget *)widget->wid_info)->valuetex) );
}

void SetBounds_IntChooserWidget( GLWidget *widget, SDL_Rect *b )
{
    IntChooserWidget *tmp;
    SDL_Rect rab,lab;

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

    tmp = (IntChooserWidget *)(widget->wid_info);
    
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
    IntChooserWidget *tmp;
    char valuetext[16];
    int step;
    
    if (!data) return;
    tmp = ((IntChooserWidget *)((GLWidget *)data)->wid_info);

    if (tmp->direction > 0)
    	step = 100/clientFPS;
    else
    	step = 1;
    
    if (*(tmp->opt->int_ptr) < tmp->opt->int_maxval) {
    	Set_int_option(tmp->opt, MIN( *(tmp->opt->int_ptr) + step,
				      tmp->opt->int_maxval),
				      xp_option_origin_config);
    	if ( (*(tmp->opt->int_ptr)) > tmp->opt->int_minval)
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->int_ptr)) == tmp->opt->int_maxval)
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
	tmp->direction = 2;
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
    IntChooserWidget *tmp;
    int step;
    char valuetext[16];
    
    if (!data) return;
    tmp = ((IntChooserWidget *)((GLWidget *)data)->wid_info);

    if (tmp->direction < 0)
    	step = 100/clientFPS;
    else
    	step = 1;

    if (*(tmp->opt->int_ptr) > tmp->opt->int_minval) {
    	Set_int_option(tmp->opt,
		       MAX( (*(tmp->opt->int_ptr)) - step,
			    tmp->opt->int_minval),
		       xp_option_origin_config);
    	if ( (*(tmp->opt->int_ptr)) < tmp->opt->int_maxval)
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->int_ptr)) == tmp->opt->int_minval)
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
	tmp->direction = -2;
	snprintf(valuetext,15,"%i",*(tmp->opt->int_ptr));
	free_string_texture(&(tmp->valuetex));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%s=%i) texture for IntChooserWidget!\n",tmp->opt->name,*(tmp->opt->int_ptr));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_IntChooserWidget( GLWidget *widget )
{
    /* static int bg_color     = 0x0000ff88; */
    static int name_color   = 0xffff66ff;
    static int value_color  = 0x00ff00ff;
    IntChooserWidget *wid_info;

    if (!widget) {
    	error("Paint_IntChooserWidget: argument is NULL!");
	return;
    }
    
    wid_info = (IntChooserWidget *)(widget->wid_info);

    if (!wid_info) {
    	error("Paint_IntChooserWidget: wid_info missing");
	return;
    }
    
    if (wid_info->direction > 0) --(wid_info->direction);
    else if (wid_info->direction < 0) ++(wid_info->direction);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*set_alphacolor(bg_color);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();*/
    disp_text(&(wid_info->nametex), name_color, LEFT, CENTER, widget->bounds.x+2/*|_text*/, draw_height - widget->bounds.y - widget->bounds.h/2, true);
    disp_text(&(wid_info->valuetex), value_color, RIGHT, CENTER, wid_info->rightarrow->bounds.x-1/*value_>*/-2/*>_|*/, draw_height - widget->bounds.y - widget->bounds.h/2, true );
}

GLWidget *Init_IntChooserWidget( font_data *font, xp_option_t *opt )
{
    int valuespace;
    GLWidget *tmp;
    IntChooserWidget *wid_info;
    char valuetext[16];
    string_tex_t tmp_tex;

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

    tmp = Init_EmptyBaseGLWidget();
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
    if (render_text(font,"555.55",&tmp_tex)) {
    	free_string_texture(&tmp_tex);
	valuespace = tmp_tex.width+4;
    } else {
    	valuespace = 50;
    }

    wid_info = tmp->wid_info;
    tmp->WIDGET     = INTCHOOSERWIDGET;
    if (render_text(font,opt->name,&(wid_info->nametex))) {
	snprintf(valuetext,15,"%i",*(opt->int_ptr));
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font  	= font;
	    wid_info->opt   	= opt;
	    wid_info->valuespace = valuespace;
	    wid_info->direction = 0;

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

    	    tmp->bounds.w   = 2/*|_text*/+ wid_info->nametex.width +5/*text___<*/ + valuespace/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;

    	    tmp->Draw	    	= Paint_IntChooserWidget;
    	    tmp->Close  	= Close_IntChooserWidget;
    	    tmp->SetBounds  	= SetBounds_IntChooserWidget;

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
void Paint_DoubleChooserWidget( GLWidget *widget );
void Close_DoubleChooserWidget ( GLWidget *widget );
void SetBounds_DoubleChooserWidget( GLWidget *widget, SDL_Rect *b );

void Close_DoubleChooserWidget ( GLWidget *widget )
{
    if (!widget) return;
    if (widget->WIDGET !=DOUBLECHOOSERWIDGET) {
    	error("Wrong widget type for Close_DoubleChooserWidget [%i]",widget->WIDGET);
	return;
    }
    
    free_string_texture( &(((DoubleChooserWidget *)widget->wid_info)->nametex) );
    free_string_texture( &(((DoubleChooserWidget *)widget->wid_info)->valuetex) );
}

void SetBounds_DoubleChooserWidget( GLWidget *widget, SDL_Rect *b )
{
    DoubleChooserWidget *tmp;
    SDL_Rect rab,lab;
    
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

    tmp = (DoubleChooserWidget *)(widget->wid_info);
    
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
    DoubleChooserWidget *tmp;
    double step;
    char valuetext[16];
    
    if (!data) return;
    tmp = ((DoubleChooserWidget *)((GLWidget *)data)->wid_info);

    if (tmp->direction > 0)
    	step = ((double)clientFPS)*10.0;
    else
    	step = 1000.0;
    
    if ( *(tmp->opt->dbl_ptr) < tmp->opt->dbl_maxval ) {
    	Set_double_option( tmp->opt,
			   MIN( (*(tmp->opt->dbl_ptr))+((tmp->opt->dbl_maxval)-(tmp->opt->dbl_minval))/step,tmp->opt->dbl_maxval ),
			   xp_option_origin_config);
    	if ( (*(tmp->opt->dbl_ptr)) > tmp->opt->dbl_minval )
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->dbl_ptr)) >= tmp->opt->dbl_maxval )
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
	tmp->direction = 2;
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
    DoubleChooserWidget *tmp;
    double step;
    char valuetext[16];
    
    if (!data) return;
    tmp = ((DoubleChooserWidget *)((GLWidget *)data)->wid_info);

    if (tmp->direction < 0)
    	step = ((double)clientFPS)*10.0;
    else
    	step = 1000.0;

    if ( *(tmp->opt->dbl_ptr) > tmp->opt->dbl_minval ) {
    	Set_double_option( tmp->opt, MAX( (*(tmp->opt->dbl_ptr))-((tmp->opt->dbl_maxval)-(tmp->opt->dbl_minval))/step,tmp->opt->dbl_minval ),
			   xp_option_origin_config);
    	if ( (*(tmp->opt->dbl_ptr)) < tmp->opt->dbl_maxval )
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
    	if ( (*(tmp->opt->dbl_ptr)) <= tmp->opt->dbl_minval )
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
	tmp->direction = -2;
	snprintf(valuetext,15,"%1.2f",*(tmp->opt->dbl_ptr));
	free_string_texture(&(tmp->valuetex));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%s=%1.2f) texture for doubleChooserWidget!\n",tmp->opt->name,*(tmp->opt->dbl_ptr));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_DoubleChooserWidget( GLWidget *widget )
{
    /* static int bg_color     = 0x0000ff88; */
    static int name_color   = 0xffff66ff;
    static int value_color  = 0x00ff00ff;
    DoubleChooserWidget *wid_info;

    if (!widget) return;
    
    wid_info = (DoubleChooserWidget *)(widget->wid_info);

    if (wid_info->direction > 0) --(wid_info->direction);
    else if (wid_info->direction < 0) ++(wid_info->direction);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /*set_alphacolor(bg_color);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();*/
    disp_text(&(wid_info->nametex), name_color, LEFT, CENTER, widget->bounds.x+2/*|_text*/, draw_height - widget->bounds.y - widget->bounds.h/2, true);
    disp_text(&(wid_info->valuetex), value_color, RIGHT, CENTER, wid_info->rightarrow->bounds.x-1/*value_>*/-2/*>_|*/, draw_height - widget->bounds.y - widget->bounds.h/2, true );
}

GLWidget *Init_DoubleChooserWidget( font_data *font, xp_option_t *opt )
{
    int valuespace;
    GLWidget *tmp;
    string_tex_t tmp_tex;
    DoubleChooserWidget *wid_info;
    char valuetext[16];
    
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

    tmp = Init_EmptyBaseGLWidget();
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
    if (render_text(font,"555.55",&tmp_tex)) {
    	free_string_texture(&tmp_tex);
	valuespace = tmp_tex.width+4;
    } else {
    	valuespace = 50;
    }
    
    wid_info = tmp->wid_info;
    tmp->WIDGET     = DOUBLECHOOSERWIDGET;
    if (render_text(font,opt->name,&(wid_info->nametex))) {
	snprintf(valuetext,15,"%1.2f",*(opt->dbl_ptr));
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font  	= font;
	    wid_info->opt   	= opt;
	    wid_info->valuespace = valuespace;
	    wid_info->direction = 0;
    	    
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

    	    tmp->bounds.w   = 2/*|_text*/+ wid_info->nametex.width +5/*text___<*/ + valuespace/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;

    	    tmp->Draw	    	= Paint_DoubleChooserWidget;
    	    tmp->Close  	= Close_DoubleChooserWidget;
    	    tmp->SetBounds  	= SetBounds_DoubleChooserWidget;

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

/**********************/
/* Begin: ListWidget  */
/**********************/
void SetBounds_ListWidget( GLWidget *widget, SDL_Rect *b );
void Paint_ListWidget( GLWidget *widget );

bool ListWidget_Append( GLWidget *list, GLWidget *item )
{
    GLWidget *curr1, **curr2;
    ListWidget *wid_info;
    
    if (!list) {
    	error("ListWidget_Append: *list is NULL!");
	return false;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_Append: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return false;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_Append: list->wid_info missing!");
	return false;
    }
    if (!item) {
    	error("ListWidget_Append: *item is NULL");
	return false;
    }

    curr1 = item;
    while (curr1) {   	
	curr2 = &(list->children);
    	while (*curr2) {
	    if (*curr2 == curr1) break;
	    curr2 = &((*curr2)->next);
    	}

    	if (*curr2) {
	    error("ListWidget_Append: Attempt to append an existing item!");
	    break;
	}
	
	*curr2 = curr1;
	curr1 = curr1->next;
	/* disengage added item from the item list t be added */
	(*curr2)->next = NULL;

     	++wid_info->num_elements;
    }
    
    /* This works since SetBounds_ListWidget copies the content
     * if the SDL_Rect *
     */
    SetBounds_ListWidget( list, &(list->bounds));

    return true;
}

bool ListWidget_Prepend( GLWidget *list, GLWidget *item )
{
    GLWidget *curr1, *curr2, **entry_pt, *first;
    ListWidget *wid_info;
    /*int y_rel;*/
    
    if (!list) {
    	error("ListWidget_Prepend: *list is NULL!");
	return false;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_Prepend: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return false;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_Prepend: list->wid_info missing!");
	return false;
    }
    if (!item) {
    	error("ListWidget_Prepend: *item is NULL");
	return false;
    }
    
    entry_pt = &(list->children);
    first = *entry_pt;
    
    curr1 = item;
    while (curr1) {
    	curr2 = list->children;
    	while (curr2) {
	    if (curr2 == item) break;
	    curr2 = curr2->next;
    	}
	
	if (curr2) {
	    error("ListWidget_Append: Attempt to append an existing item!");
	    break;
	}
    	
	*entry_pt = curr1;
	curr1 = curr1->next;
	(*entry_pt)->next = first;
	entry_pt = &((*entry_pt)->next);

    	++wid_info->num_elements;
    }
    
    /* This works since SetBounds_ListWidget copies the content
     * if the SDL_Rect *
     */
    SetBounds_ListWidget( list, &(list->bounds));
    
    return true;
}

bool ListWidget_Insert( GLWidget *list, GLWidget *target, GLWidget *item )
{
    GLWidget **curr, *curr1, *curr2;
    ListWidget *wid_info;
    
    if (!list) {
    	error("ListWidget_Insert: *list is NULL!");
	return false;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_Insert: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return false;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_Insert: list->wid_info missing!");
	return false;
    }
    if (!item) {
    	error("ListWidget_Insert: *item is NULL");
	return false;
    }

    curr = &(list->children);
    while (*curr) {
	if (*curr == target) break;
	curr = &((*curr)->next);
    }
    
    if (!(*curr)) {
	    error("ListWidget_Insert: target is not in the list!");
	    return false;
    }
    
    curr1 = item;
    while (curr1) {
    	curr2 = list->children;
    	while (curr2) {
	    if (curr2 == item) break;
	    curr2 = curr2->next;
    	}
	
	if (curr2) {
	    error("ListWidget_Append: Attempt to append an existing item!");
	    break;
	}
    	
	*curr = curr1;
	curr1 = curr1->next;
	(*curr)->next = target;
	curr = &((*curr)->next);

    	++wid_info->num_elements;
    }
    
    /* This works since SetBounds_ListWidget copies the content
     * if the SDL_Rect *
     */
    SetBounds_ListWidget( list, &(list->bounds));
    
    return true;
}

bool ListWidget_Remove( GLWidget *list, GLWidget *item )
{
    GLWidget **curr;
    ListWidget *wid_info;
    
    if (!list) {
    	error("ListWidget_Remove: *list is NULL!");
	return false;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_Remove: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return false;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_Remove: list->wid_info missing!");
	return false;
    }
    if (!item) {
    	error("ListWidget_Remove: *item is NULL");
	return false;
    }

    curr = &(list->children);
    while (*curr) {
	if (*curr == item) {
	    break;
	}
	curr = &((*curr)->next);
    }
    
    if (!(*curr)) {
	    error("ListWidget_Remove: item is not in the list!");
	    return false;
    }
    
    *curr = item->next;
    
    --wid_info->num_elements;
    
    /* This works since SetBounds_ListWidget copies the content
     * if the SDL_Rect *
     */
    SetBounds_ListWidget( list, &(list->bounds));
    
    return true;
}

bool ListWidget_SetScrollorder( GLWidget *list, bool order )
{
    /*GLWidget **curr;*/
    ListWidget *wid_info;
    
    if (!list) {
    	error("ListWidget_SetScrollorder: *list is NULL!");
	return false;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_SetScrollorder: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return false;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_SetScrollorder: list->wid_info missing!");
	return false;
    }

    wid_info->reverse_scroll = order;
        
    /* This works since SetBounds_ListWidget copies the content
     * if the SDL_Rect *
     */
    SetBounds_ListWidget( list, &(list->bounds));
    
    return true;
}

int ListWidget_NELEM( GLWidget *list )
{
    ListWidget *wid_info;
    
    if (!list) {
    	error("ListWidget_NELEM: *list is NULL!");
	return -1;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_NELEM: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return -1;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_Remove: list->wid_info missing!");
	return -1;
    }
    
    return wid_info->num_elements;
}

GLWidget *ListWidget_GetItemByIndex( GLWidget *list, int i )
{
    GLWidget *curr;
    ListWidget *wid_info;
    int j;
    
    if (!list) {
    	error("ListWidget_NELEM: *list is NULL!");
	return NULL;
    }
    if (list->WIDGET != LISTWIDGET) {
    	error("ListWidget_NELEM: list is not a LISTWIDGET! [%i]",list->WIDGET);
	return NULL;
    }
    if (!(wid_info = list->wid_info)) {
    	error("ListWidget_Remove: list->wid_info missing!");
	return NULL;
    }
    
    if ( i > ( wid_info->num_elements - 1) ) return NULL;
    
    curr = list->children;
    j = 0;
    while (curr && (j<i)) {
	curr = curr->next;
    	++j;
    }
    
    if ( j != i ) return NULL;
    
    return curr;
}


void Paint_ListWidget( GLWidget *widget )
{
    ListWidget *wid_info;
    GLWidget *curr;

    if (!widget) return;
    
    wid_info = (ListWidget *)(widget->wid_info);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    curr = widget->children;
    while (curr) {
    	if (*(wid_info->bg)) {
    	    set_alphacolor(*(wid_info->bg));

    	    glVertex2i(curr->bounds.x	    	    	,curr->bounds.y	    	    	);
    	    glVertex2i(curr->bounds.x+widget->bounds.w	,curr->bounds.y	    	    	);
    	    glVertex2i(curr->bounds.x+widget->bounds.w	,curr->bounds.y+curr->bounds.h	);
    	    glVertex2i(curr->bounds.x 	    	    	,curr->bounds.y+curr->bounds.h	);
    	}
    	curr = curr->next;
    }

    glEnd();
}

/* This setbounds method has very special behaviour; basically
 * only one corner of the bounding box is regarded, this is 
 * because the list is an expanding/contracting entity, and
 * thus the others might soon get overridden anyway.
 * The only way to properly bound a ListWidget is to make a
 * bounding widget adopt it. (i.e. a scrollpane)
 */
void SetBounds_ListWidget( GLWidget *widget, SDL_Rect *b )
{
    ListWidget *tmp;
    SDL_Rect bounds,b2;
    GLWidget *curr;    
    
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET !=LISTWIDGET) {
    	error("Wrong widget type for SetBounds_ListWidget [%i]",widget->WIDGET);
	return;
    }

    if (!(tmp = (ListWidget *)(widget->wid_info))) {
    	error("SetBounds_ListWidget: wid_info missing!");
	return;
    }
    
    bounds.y = b->y;
    bounds.x = b->x;
    bounds.w = 0;
    bounds.h = 0;
    curr = widget->children;
    while(curr) {
    	bounds.w = MAX(bounds.w,curr->bounds.w);
	bounds.h += curr->bounds.h;
	curr = curr->next;
    }

    if (tmp->v_dir == LW_UP) {
    	bounds.y += b->h - bounds.h;
    }
    if (tmp->h_dir == LW_LEFT) {
    	bounds.x += b->w - bounds.w;
    }
    
    widget->bounds.y = bounds.y;
    widget->bounds.h = bounds.h;
    widget->bounds.x = bounds.x;
    widget->bounds.w = bounds.w;
    
    curr = widget->children;
    while(curr) {
    	bounds.h -= curr->bounds.h;

	b2.y = bounds.y;
	if (tmp->reverse_scroll) {
    	    b2.y += bounds.h;
    	} else {
	    bounds.y += curr->bounds.h;
	}

    	b2.x = bounds.x;
	if (tmp->h_dir == LW_LEFT) {
	    b2.x += bounds.w - curr->bounds.w;
	}
	
	b2.h = curr->bounds.h;
	b2.w = widget->bounds.w;
	/*b2.w = curr->bounds.w;*/ /*TODO: make this optional*/
	
	SetBounds_GLWidget( curr, &b2 );

	curr = curr->next;
    }
}

GLWidget *Init_ListWidget( Uint16 x, Uint16 y, Uint32 *bg, Uint32 *highlight_color
    	    	    	    ,ListWidget_ver_dir_t v_dir, ListWidget_hor_dir_t h_dir
			    ,bool reverse_scroll )
{
    GLWidget *tmp;
    ListWidget *wid_info;
    
    tmp	= Init_EmptyBaseGLWidget();
    if ( !tmp ) {
        error("Failed to malloc GLWidget in Init_ListWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(ListWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc MainWidget in Init_ListWidget");
	return NULL;
    }
    wid_info = ((ListWidget *)tmp->wid_info);
    wid_info->num_elements = 0;
    wid_info->bg	= bg;
    wid_info->highlight_color	= highlight_color;
    wid_info->reverse_scroll	= reverse_scroll;
    wid_info->v_dir	= v_dir;
    wid_info->h_dir	= h_dir;
    
    tmp->WIDGET     	= LISTWIDGET;
    tmp->bounds.x   	= x;
    tmp->bounds.y   	= y;
    tmp->bounds.w   	= 0;
    tmp->bounds.h   	= 0;
    tmp->SetBounds  	= SetBounds_ListWidget;
    tmp->Draw	    	= Paint_ListWidget;

    return tmp;
}

/*******************/
/* End: ListWidget */
/*******************/

/****************************/
/* Begin: ScrollPaneWidget  */
/****************************/
void ScrollPaneWidget_poschange( GLfloat pos , void *data );
void SetBounds_ScrollPaneWidget(GLWidget *widget, SDL_Rect *b );

void ScrollPaneWidget_poschange( GLfloat pos , void *data )
{
    GLWidget *widget;
    ScrollPaneWidget *wid_info;
    SDL_Rect bounds;
    /*GLWidget *curr;*/
    
    if ( !data ) {
        error("NULL data to ScrollPaneWidget_poschange!");
	return;
    }
    widget = (GLWidget *)data;
    wid_info = ((ScrollPaneWidget *)(widget->wid_info));

    if (wid_info->content) {
    	bounds.y = widget->bounds.y - pos*(wid_info->content->bounds.h);
    	bounds.x = wid_info->content->bounds.x;
    	bounds.w = wid_info->content->bounds.w;
    	bounds.h = wid_info->content->bounds.h;
    	SetBounds_GLWidget(wid_info->content,&bounds);
    }    
}

void SetBounds_ScrollPaneWidget(GLWidget *widget, SDL_Rect *b )
{
    ScrollPaneWidget *wid_info;
    SDL_Rect bounds;
    GLfloat pos;
    
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET != SCROLLPANEWIDGET) {
    	error("Wrong widget type for SetBounds_ScrollPaneWidget [%i]",widget->WIDGET);
	return;
    }

    if (!(wid_info = (ScrollPaneWidget *)(widget->wid_info))) {
    	error("SetBounds_ScrollPaneWidget: wid_info missing!");
	return;
    }
    
    widget->bounds.x = b->x;
    widget->bounds.w = b->w;
    widget->bounds.y = b->y;
    widget->bounds.h = b->h;
    
    if (wid_info->scroller) {
    	bounds.y = widget->bounds.y;
    	bounds.h = widget->bounds.h;
    	bounds.w = wid_info->scroller->bounds.w;
    	bounds.x = widget->bounds.x + widget->bounds.w - wid_info->scroller->bounds.w;
    
    	SetBounds_GLWidget(wid_info->scroller,&bounds);
    } else {
    	error("SetBounds_ScrollPaneWidget: scroller missing!");
	return;
    }
    
    if (wid_info->masque) {
    	bounds.y = widget->bounds.y;
    	bounds.h = widget->bounds.h;
    	bounds.w = widget->bounds.w - wid_info->scroller->bounds.w;
    	bounds.x = widget->bounds.x;
    
    	SetBounds_GLWidget(wid_info->masque,&bounds);
    } else {
    	error("SetBounds_ScrollPaneWidget: masque missing!");
	return;
    }
    
    if (wid_info->content) {
    	pos = ((ScrollbarWidget *)(wid_info->scroller->wid_info))->pos;
    	bounds.y = widget->bounds.y - pos*(wid_info->content->bounds.h);
    	bounds.h = wid_info->content->bounds.h;
	
	ScrollbarWidget_SetSlideSize(wid_info->scroller,MIN(((GLfloat)widget->bounds.h)/((GLfloat)bounds.h),1.0f));
	
    	bounds.w = widget->bounds.w - wid_info->scroller->bounds.w;
    	bounds.x = widget->bounds.x;
    
    	SetBounds_GLWidget(wid_info->content,&bounds);
    }
}

GLWidget *Init_ScrollPaneWidget( GLWidget *content )
{
    GLWidget *tmp;
    ScrollPaneWidget *wid_info;
    /*SDL_Rect b;*/
    
    tmp	= Init_EmptyBaseGLWidget();
    if ( !tmp ) {
        error("Failed to malloc GLWidget in Init_ScrollPaneWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(ScrollPaneWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc MainWidget in Init_ScrollPaneWidget");
	return NULL;
    }
    wid_info = ((ScrollPaneWidget *)tmp->wid_info);
    wid_info->content	= content;
    
    tmp->WIDGET     	= SCROLLPANEWIDGET;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= 0;
    tmp->bounds.h   	= 0;
    tmp->SetBounds  	= SetBounds_ScrollPaneWidget;
    
    if ( !AppendGLWidgetList(&(tmp->children),(wid_info->masque = Init_EmptyBaseGLWidget()))
    	) {
	error("Init_ScrollPaneWidget: Failed to init masque!");
	Close_Widget(&(wid_info->scroller));
	Close_Widget(&tmp);
    	return NULL;
    }
    
    if ( !AppendGLWidgetList(&(tmp->children),
    	    (wid_info->scroller = Init_ScrollbarWidget(false,0.0f,1.0f,SB_VERTICAL
	    	    	    	    ,ScrollPaneWidget_poschange,tmp)))
	) {
	error("Init_ScrollPaneWidget: Failed to init scroller!");
	Close_Widget(&tmp);
    	return NULL;
    }
    
    if (wid_info->content) {
    	if ( !AppendGLWidgetList(&(wid_info->masque->children),wid_info->content) ) {
	    error("Init_ScrollPaneWidget: Failed to adopt the content to the masque!");
	    Close_Widget(&tmp);
    	    return NULL;
    	}
    
        tmp->bounds.w = wid_info->content->bounds.w;
        tmp->bounds.h = wid_info->content->bounds.h;
    }
    
    tmp->bounds.w += wid_info->scroller->bounds.w;
    
    return tmp;
}

/**************************/
/* End: ScrollPaneWidget  */
/**************************/

/**********************/
/* Begin: MainWidget  */
/**********************/
void button_MainWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data );
void SetBounds_MainWidget( GLWidget *widget, SDL_Rect *b );

void button_MainWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data )
{
    GLWidget *widget;
    WrapperWidget *wid_info;
    
    widget = (GLWidget *)data;
    wid_info = ((WrapperWidget *)widget->wid_info);
    if (state == SDL_PRESSED) {
    	if (button != 1) {
	    if (!wid_info->confmenu) {
    	    	wid_info->confmenu = Init_ConfMenuWidget( wid_info->font, x, y );
		if (!(wid_info->confmenu)) {
		    error("failed to create conf menu!");
		    return;
		}
		AppendGLWidgetList(&(widget->children), wid_info->confmenu);
	    } else {
	    	DelGLWidgetListItem(&(widget->children), wid_info->confmenu);
		Close_Widget(&(wid_info->confmenu));
	    }
	}
    }
}

void SetBounds_MainWidget( GLWidget *widget, SDL_Rect *b )
{
    WrapperWidget *wid_info;
    SDL_Rect bounds = {0,0,0,0};
    GLWidget *subs[4];
    int i;
    bool change;
    
    if (!widget) return;
    if (!b) return;
    if (widget->WIDGET != MAINWIDGET) {
    	error("Wrong widget type for SetBounds_MainWidget [%i]",widget->WIDGET);
	return;
    }

    if (!(wid_info = (WrapperWidget *)(widget->wid_info))) {
    	error("SetBounds_MainWidget: wid_info missing!");
	return;
    }
    
    subs[0] = wid_info->radar;
    subs[1] = wid_info->scorelist;
    subs[2] = wid_info->chat_msgs;
    subs[3] = wid_info->game_msgs;
    
    widget->bounds.x = b->x;
    widget->bounds.w = b->w;
    widget->bounds.y = b->y;
    widget->bounds.h = b->h;
    
    for ( i=0; i < 4 ; ++i) {
    	if (subs[i]) {
	    change = false;
	    
    	    bounds.x = subs[i]->bounds.x;
    	    bounds.w = subs[i]->bounds.w;
    	    bounds.y = subs[i]->bounds.y;
    	    bounds.h = subs[i]->bounds.h;

	    if ( bounds.x + bounds.w + wid_info->BORDER > widget->bounds.x + widget->bounds.w ) {
	    	bounds.x = widget->bounds.x + widget->bounds.w - (bounds.w + wid_info->BORDER);
		change = true;
	    }

	    if ( bounds.y + bounds.h + wid_info->BORDER  > widget->bounds.y + widget->bounds.h ) {
	    	bounds.y = widget->bounds.y + widget->bounds.h - (bounds.h + wid_info->BORDER);
		change = true;
	    }
	
	    if ( change )    
    	    	SetBounds_GLWidget(subs[i],&bounds);
	    
    	}
    }
}

GLWidget *Init_MainWidget( font_data *font )
{
    GLWidget *tmp;
    WrapperWidget *wid_info;
    
    tmp	= Init_EmptyBaseGLWidget();
    if ( !tmp ) {
        error("Failed to malloc GLWidget in Init_MainWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(WrapperWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc MainWidget in Init_MainWidget");
	return NULL;
    }
    wid_info = ((WrapperWidget *)tmp->wid_info);
    wid_info->confmenu	= NULL;
    wid_info->font	= font;
    wid_info->BORDER	= 10;
    
    tmp->WIDGET     	= MAINWIDGET;
    tmp->bounds.w   	= draw_width;
    tmp->bounds.h   	= draw_height;
    tmp->button     	= button_MainWidget;
    tmp->buttondata 	= tmp;
    tmp->SetBounds 	= SetBounds_MainWidget;
    
    if ( !AppendGLWidgetList(&(tmp->children),(wid_info->radar = Init_RadarWidget())) ) {
	error("radar initialization failed");
	Close_Widget(&tmp);
	return NULL;
    }
    if ( !AppendGLWidgetList(&(tmp->children),(wid_info->scorelist = Init_ScorelistWidget())) ) {
	error("scorelist initialization failed");
	Close_Widget(&tmp);
	return NULL;
    }
    if ( !AppendGLWidgetList(&(tmp->children),
    	    	(wid_info->chat_msgs = Init_ListWidget(200 + wid_info->BORDER,wid_info->BORDER,
		&nullRGBA,&greenRGBA,LW_DOWN,LW_RIGHT,instruments.showReverseScroll)))
    	) {
	error("Failed to initialize chat msg list");
	Close_Widget(&tmp);
	return NULL;
    }
    if ( !AppendGLWidgetList(&(tmp->children),
    	    (wid_info->game_msgs = Init_ListWidget(wid_info->BORDER,tmp->bounds.h-wid_info->BORDER,
	    &nullRGBA,&greenRGBA,LW_UP,LW_RIGHT,!instruments.showReverseScroll)))
	) {
	error("Failed to initialize game msg list");
	Close_Widget(&tmp);
	return NULL;
    }

    return tmp;
}
/*******************/
/* End: MainWidget */
/*******************/

/**************************/
/* Begin: ConfMenuWidget  */
/**************************/
void Paint_ConfMenuWidget( GLWidget *widget );

void SetBounds_ConfMenuWidget( GLWidget *widget, SDL_Rect *b )
{
    ConfMenuWidget *wid_info;
    SDL_Rect bounds;
    
    if (!widget ) {
    	error("SetBounds_ConfMenuWidget: tried to change bounds on NULL ConfMenuWidget!");
	return;
    }
    if ( widget->WIDGET != CONFMENUWIDGET ) {
    	error("SetBounds_ConfMenuWidget: Wrong widget type! [%i]",widget->WIDGET);
	return;
    }
    if (!(wid_info = (ConfMenuWidget *)(widget->wid_info))) {
    	error("SetBounds_ConfMenuWidget: wid_info missing!");
	return;
    }
    if (!b ) {
    	error("SetBounds_ConfMenuWidget: tried to set NULL bounds on ConfMenuWidget!");
	return;
    }
        
    bounds.x = b->x;
    bounds.y = b->y;
    bounds.w = b->w;
    bounds.h = b->h;
    
    widget->bounds.x = b->x;
    widget->bounds.y = b->y;
    widget->bounds.w = b->w;
    widget->bounds.h = b->h;
    
    SetBounds_GLWidget(wid_info->scrollpane,&bounds);
}

void Paint_ConfMenuWidget( GLWidget *widget )
{
    int edgeColor = 0xff0000ff;
    int bgColor = 0x0000ff88;

    if (!widget ) {
    	error("tPaint_ConfMenuWidget: tried to paint NULL ConfMenuWidget!");
	return;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    set_alphacolor(bgColor);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();
    glBegin(GL_LINE_LOOP);
    	set_alphacolor(edgeColor);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y	    	    	);
    	set_alphacolor(bgColor | 0x000000ff);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y	    	    	);
    	set_alphacolor(edgeColor);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h	);
    	set_alphacolor(bgColor | 0x000000ff);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h	);
    glEnd();
}

GLWidget *Init_ConfMenuWidget( font_data *font, Uint16 x, Uint16 y )
{
    GLWidget *tmp, *item, *dummy, *list;
    ConfMenuWidget *wid_info;
    SDL_Rect bounds;
    int i;
    xp_option_t *opt;

    tmp	= Init_EmptyBaseGLWidget();
    if ( !tmp ) {
        error("Failed to malloc in Init_ConfMenu");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(ConfMenuWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_ConfMenu");
	return NULL;
    }
    wid_info = ((ConfMenuWidget *)(tmp->wid_info));
    
    tmp->WIDGET     	= CONFMENUWIDGET;
    tmp->Draw	    	= Paint_ConfMenuWidget;
    tmp->SetBounds  	= SetBounds_ConfMenuWidget;
    
    dummy = Init_EmptyBaseGLWidget();
    if ( !dummy ) {
        error("Failed to malloc in Init_ConfMenu");
	return NULL;
    }
    
    for ( i=0 ; i < num_options; ++i ) {
    	opt = Option_by_index(i);
	item = Init_OptionWidget(font,opt);
	if (item) {
	    AppendGLWidgetList( &(dummy->next), item );
	}
    }

    if (!dummy) {
    	error("Init_ConfMenuWidget: Couldn't make dummy!");
	Close_Widget(&tmp);
	return NULL;
    }

    if (!(list = Init_ListWidget(0,0,&nullRGBA,&nullRGBA,LW_DOWN,LW_RIGHT,false))) {
    	error("Init_ConfMenuWidget: Couldn't make the list widget!");
	Close_WidgetTree(&(dummy->next));
	Close_Widget(&tmp);
	return NULL;
    }
    
    ListWidget_Append(list,dummy->next);

    if ( !AppendGLWidgetList(&(tmp->children),(wid_info->scrollpane = Init_ScrollPaneWidget(list))) ) {
    	error("Init_ConfMenuWidget: Couldn't make the scrollpane!");
	Close_WidgetTree(&dummy);
	Close_Widget(&tmp);
	return NULL;
    }
    
    Close_Widget(&dummy);
   
    tmp->bounds.x   	= x;
    tmp->bounds.y   	= y;
    tmp->bounds.w   	= wid_info->scrollpane->bounds.w+2;
    tmp->bounds.h   	= 512;
    
    bounds.x = tmp->bounds.x + 1;
    bounds.y = tmp->bounds.y + 1;
    bounds.w = tmp->bounds.w - 2;
    bounds.h = tmp->bounds.h - 2;
    
    SetBounds_GLWidget(wid_info->scrollpane,&bounds);
       
    return tmp;
}
/***********************/
/* End: ConfMenuWidget */
/***********************/

/****************************/
/* Begin: ImageButtonWidget */
/****************************/

static void Button_ImageButtonWidget(Uint8 button, Uint8 state, Uint16 x, 
			      Uint16 y, void *data)
{
    GLWidget *widget;
    ImageButtonWidget *info;

    widget = (GLWidget*)data;
    if (widget->WIDGET != IMAGEBUTTONWIDGET) {
	error("expected IMAGEBUTTONWIDGET got [%d]", widget->WIDGET);
	return;
    }
    info = (ImageButtonWidget*)widget->wid_info;
    if (info->state == state) return;
    info->state = state;

    if (state != SDL_PRESSED && info->onClick) {
	if (x >= widget->bounds.x
	    && x <= widget->bounds.x + widget->bounds.w
	    && y >= widget->bounds.y
	    && y <= widget->bounds.y + widget->bounds.h)
	    info->onClick(widget);
    }
}

static void Close_ImageButtonWidget(GLWidget *widget)
{
    ImageButtonWidget *info;
    if (!widget) return;
    if (widget->WIDGET != IMAGEBUTTONWIDGET) {
    	error("Wrong widget type for Close_ImageButtonWidget [%i]",
	      widget->WIDGET);
	return;
    }
    info = (ImageButtonWidget*)widget->wid_info;
    free_string_texture(&(info->tex));
    if (info->imageUp) glDeleteTextures(1, &(info->imageUp));
    if (info->imageDown) glDeleteTextures(1, &(info->imageDown));
}

static void Paint_ImageButtonWidget(GLWidget *widget)
{
    SDL_Rect *b;
    ImageButtonWidget *info;
    int x, y, c;

    if (!widget) return;
     
    b = &(widget->bounds);
    info = (ImageButtonWidget*)(widget->wid_info);

    if (info->state != SDL_PRESSED) {
	if (info->imageUp) {
	    set_alphacolor(info->bg);
	    glBindTexture(GL_TEXTURE_2D, info->imageUp);
	    glEnable(GL_TEXTURE_2D);
	    glBegin(GL_QUADS);
	    glTexCoord2f(info->txcUp.MinX, info->txcUp.MinY); 
	    glVertex2i(b->x, b->y);
	    glTexCoord2f(info->txcUp.MaxX, info->txcUp.MinY); 
	    glVertex2i(b->x + b->w , b->y);
	    glTexCoord2f(info->txcUp.MaxX, info->txcUp.MaxY); 
	    glVertex2i(b->x + b->w , b->y + b->h);
	    glTexCoord2f(info->txcUp.MinY, info->txcUp.MaxY); 
	    glVertex2i(b->x, b->y + b->h);
	    glEnd();
	}
    } else {
	if (info->imageDown) {
	    set_alphacolor(info->bg);
	    glBindTexture(GL_TEXTURE_2D, info->imageDown);
	    glEnable(GL_TEXTURE_2D);
	    glBegin(GL_QUADS);
	    glTexCoord2f(info->txcDown.MinX, info->txcDown.MinY); 
	    glVertex2i(b->x, b->y);
	    glTexCoord2f(info->txcDown.MaxX, info->txcDown.MinY); 
	    glVertex2i(b->x + b->w , b->y);
	    glTexCoord2f(info->txcDown.MaxX, info->txcDown.MaxY); 
	    glVertex2i(b->x + b->w , b->y + b->h);
	    glTexCoord2f(info->txcDown.MinY, info->txcDown.MaxY); 
	    glVertex2i(b->x, b->y + b->h);
	    glEnd();
	}
    }
    
    x = widget->bounds.x + widget->bounds.w / 2;
    y = widget->bounds.y + widget->bounds.h / 2;
    c = (int)(info->fg ? info->fg : whiteRGBA);
    if (info->state == SDL_PRESSED) {
	x += 1;
	y += 1;
    }
    disp_text(&(info->tex), c,
	      CENTER, CENTER, 
	      x, draw_height - y, 
	      true);
}

GLWidget *Init_ImageButtonWidget(const char *text,
				 const char *upImage,
				 const char *downImage,
				 Uint32 bg, 
				 Uint32 fg,
				 void (*onClick)(GLWidget *widget))
{
    GLWidget *tmp;
    ImageButtonWidget *info;
    SDL_Surface *surface;
    char imagePath[256];
    int width, height;
    
    if (!text) {
    	error("text missing for Init_ImageButtonWidget.");
	return NULL;
    }
    tmp	= Init_EmptyBaseGLWidget();
    if ( !tmp ) {
        error("Failed to malloc in Init_ImageButtonWidget");
	return NULL;
    }
    info = malloc(sizeof(ImageButtonWidget));
    if (!info) {
    	free(tmp);
        error("Failed to malloc in Init_ImageButtonWidget");
	return NULL;
    }

    info->onClick = onClick;
    info->fg = fg;
    info->bg = bg;
    info->state = SDL_RELEASED;
    info->imageUp = 0;
    info->imageDown = 0;

    if (!render_text(&gamefont, text, &(info->tex))) {
    	free(info);
    	free(tmp);
        error("Failed to render text in Init_ImageButtonWidget");
	return NULL;
    }
    width = info->tex.width + 1;
    height = info->tex.height + 1;

#ifdef HAVE_SDL_IMAGE
    sprintf(imagePath, "%s%s", CONF_TEXTUREDIR, upImage);
    surface = IMG_Load(imagePath);
    if (surface) {
	info->imageUp = SDL_GL_LoadTexture(surface, &(info->txcUp));
	SDL_FreeSurface(surface);
	if (width < surface->w) width = surface->w;
	if (height < surface->h) height = surface->h;
    } else {
	error("Failed to load button image %s", imagePath);
    }
    sprintf(imagePath, "%s%s", CONF_TEXTUREDIR, downImage);
    surface = IMG_Load(imagePath);
    if (surface) {
	info->imageDown = SDL_GL_LoadTexture(surface, &(info->txcDown));
	SDL_FreeSurface(surface);
	if (width < surface->w) width = surface->w;
	if (height < surface->h) height = surface->h;
    } else {
	error("Failed to load button image %s", imagePath);
    }
#endif

    tmp->WIDGET     	= IMAGEBUTTONWIDGET;
    tmp->wid_info       = info;
    tmp->bounds.w   	= width;
    tmp->bounds.h   	= height;
    tmp->Draw	    	= Paint_ImageButtonWidget;
    tmp->Close     	= Close_ImageButtonWidget;
    tmp->button         = Button_ImageButtonWidget;
    tmp->buttondata     = tmp;
    return tmp;
}
/**************************/
/* End: ImageButtonWidget */
/**************************/
