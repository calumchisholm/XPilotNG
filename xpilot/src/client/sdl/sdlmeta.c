/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client.
 *
 * Copyright (C) 2003-2004 by
 *
 *      Juha Lindström       <juhal@users.sourceforge.net>
 *      Darel Cullen         <darelcullen@users.sourceforge.net>
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

#include "xpclient.h"
#include "sdlmeta.h"
#include "sdlwindow.h"
#include "text.h"
#include "glwidgets.h"

#define SELECTED_BG 0x009000ff
#define ROW_FG 0xffff00ff
#define ROW_BG1 0x0000a0ff
#define ROW_BG2 0x000070ff
#define HEADER_FG 0xffff00ff
#define HEADER_BG 0xff0000ff
#define ROW_HEIGHT 20
#define VERSION_WIDTH 100
#define COUNT_WIDTH 20
#define META_WIDTH 800
#define META_HEIGHT (30 * ROW_HEIGHT + 3*10 + 30)

#define METAWIDGET        100
#define METATABLEWIDGET   101
#define METAROWWIDGET     102
#define METAHEADERWIDGET  103
#define STATUSWIDGET      104

typedef struct {
    list_t                server_list;
    GLWidget	          *scrollbar;
    GLWidget              *header;
    struct _MetaRowWidget *selected;
} MetaTableWidget;

typedef struct _MetaRowWidget {
    Uint32          fg;
    Uint32          bg;
    char            count_str[3];
    MetaTableWidget *table;
    server_info_t   *sip;
    bool            is_selected;
} MetaRowWidget;

typedef struct {
    Uint32 fg;
    Uint32 bg;
} MetaHeaderWidget;

typedef struct {
    server_info_t  *sip;
} StatusWidget;

extern GLWidget *FindGLWidgeti( GLWidget *widget, Uint16 x, Uint16 y );

static void Paint_MetaRowWidget(GLWidget *widget)
{
    SDL_Rect *b;
    MetaRowWidget *row;

    if (widget->WIDGET != METAROWWIDGET) {
	error("expected METAROWWIDGET got [%d]", widget->WIDGET);
	return;
    }

    b = &(widget->bounds);
    row = (MetaRowWidget*)widget->wid_info;
    set_alphacolor(row->is_selected ? SELECTED_BG : row->bg);

    glBegin(GL_QUADS);
    glVertex2i(b->x, b->y);
    glVertex2i(b->x + b->w, b->y);
    glVertex2i(b->x + b->w, b->y + b->h);
    glVertex2i(b->x, b->y + b->h);
    glEnd();
}

static void SetBounds_MetaRowWidget(GLWidget *row, SDL_Rect *rb)
{
    int free_width;
    SDL_Rect cb;
    GLWidget *col;

    row->bounds = *rb;
    free_width = MAX(rb->w - (VERSION_WIDTH + COUNT_WIDTH), 0);

    if (!(col = row->children)) return;
    cb.x = rb->x; 
    cb.w = free_width / 2;
    cb.y = rb->y; cb.h = rb->h;
    SetBounds_GLWidget(col, &cb);

    if (!(col = col->next)) return;
    cb.x = cb.x + cb.w;
    cb.y = rb->y; cb.h = rb->h;
    SetBounds_GLWidget(col, &cb);

    if (!(col = col->next)) return;
    cb.x = cb.x + cb.w;
    cb.y = rb->y; cb.h = rb->h;
    cb.w = VERSION_WIDTH;
    SetBounds_GLWidget(col, &cb);

    if (!(col = col->next)) return;
    cb.x = cb.x + cb.w;
    cb.y = rb->y; cb.h = rb->h;
    cb.w = COUNT_WIDTH;
    SetBounds_GLWidget(col, &cb);
}

static void Button_MetaRowWidget(Uint8 button, Uint8 state, Uint16 x, 
				 Uint16 y, void *data)
{
    GLWidget *widget;
    MetaRowWidget *row;
    SDL_Event evt;

    if (state != SDL_PRESSED) return;

    widget = (GLWidget*)data;
    if (widget->WIDGET != METAROWWIDGET) {
	error("expected METAROWWIDGET got [%d]", widget->WIDGET);
	return;
    }

    row = (MetaRowWidget*)widget->wid_info;
    if (row->is_selected) {
	evt.type = SDL_USEREVENT;
	evt.user.data1 = row->sip;
	SDL_PushEvent(&evt);
    } else {
	row->table->selected->is_selected = false;
	row->table->selected = row;
	row->is_selected = true;
    }
}

static GLWidget *Init_MetaRowWidget(server_info_t *sip, 
				    MetaTableWidget *table, 
				    bool is_selected,
				    unsigned int bg)
{
    GLWidget *tmp, *col;
    MetaRowWidget *row;

    if (!(tmp = Init_EmptyBaseGLWidget())) {
        error("Widget init failed");
	return NULL;
    }
    if (!(row = (MetaRowWidget*)malloc(sizeof(MetaRowWidget)))) {
        error("out of memory");
	free(tmp);
	return NULL;
    }
    sprintf(row->count_str, "%u", sip->users);
    row->fg             = ROW_FG;
    row->bg             = bg;
    row->sip            = sip;
    row->table          = table;
    row->is_selected    = is_selected;
    tmp->wid_info       = row;
    tmp->WIDGET     	= METAROWWIDGET;
    tmp->Draw	    	= Paint_MetaRowWidget;
    tmp->SetBounds      = SetBounds_MetaRowWidget;
    tmp->button         = Button_MetaRowWidget;
    tmp->buttondata     = tmp;

#define COLUMN(TEXT) \
    if ((col = Init_LabelWidget((TEXT), NULL, &(row->fg)))) { \
	((LabelWidget*)col->wid_info)->align = LEFT; \
        col->button = Button_MetaRowWidget; \
        col->buttondata = tmp; \
	AppendGLWidgetList(&(tmp->children), col); \
    }
    COLUMN(sip->hostname);
    COLUMN(sip->mapname);
    COLUMN(sip->version);
    COLUMN(row->count_str);
#undef COLUMN

    return tmp;
}

static GLWidget *Init_MetaHeaderWidget(void)
{
    GLWidget *tmp, *col;
    MetaHeaderWidget *info;

    if (!(tmp = Init_EmptyBaseGLWidget())) {
        error("Widget init failed");
	return NULL;
    }
    if (!(info = (MetaHeaderWidget*)malloc(sizeof(MetaHeaderWidget)))) {
	error("out of memory");
	free(tmp);
	return NULL;
    }
    info->fg       = HEADER_FG;
    info->bg       = HEADER_BG;
    tmp->wid_info  = info;
    tmp->WIDGET    = METAHEADERWIDGET;
    tmp->SetBounds = SetBounds_MetaRowWidget;

#define HEADER(TEXT) \
    if ((col = Init_LabelWidget((TEXT), &(info->bg), &(info->fg)))) { \
	((LabelWidget*)col->wid_info)->align = LEFT; \
	AppendGLWidgetList(&(tmp->children), col); \
    }
    HEADER("Server");
    HEADER("Map");
    HEADER("Version");
    HEADER("Pl");
#undef COLUMN

    return tmp;    
}

static void Scroll_MetaTableWidget(GLfloat pos, void *data)
{
    MetaTableWidget *info;
    GLWidget *widget, *row;
    SDL_Rect b;
    int y;
    
    widget = (GLWidget*)data;
    if (widget->WIDGET != METATABLEWIDGET) {
	error("expected METATABLEWIDGET got [%d]", widget->WIDGET);
	return;
    }

    info = (MetaTableWidget*)widget->wid_info;
    y = widget->bounds.y + ROW_HEIGHT
	- (int)(List_size(info->server_list) * ROW_HEIGHT * pos);

    for (row = widget->children; row; row = row->next) {
	if (row->WIDGET == METAROWWIDGET) {
	    b = row->bounds;
	    b.y = y;
	    SetBounds_GLWidget(row, &b);
	    y += ROW_HEIGHT;
	}
    }
}

static void SetBounds_MetaTableWidget(GLWidget *widget, SDL_Rect *b)
{
    int y;
    GLWidget *row;
    MetaTableWidget *info;
    GLfloat table_height;
    SDL_Rect *wb, sb, rb, hb;

    if (widget->WIDGET != METATABLEWIDGET) {
	error("expected METATABLEWIDGET got [%d]", widget->WIDGET);
	return;
    }

    widget->bounds = *b;
    info = (MetaTableWidget*)widget->wid_info;
    table_height = List_size(info->server_list) * ROW_HEIGHT;

    if (info->scrollbar != NULL) {
	Close_Widget(&(info->scrollbar));
	DelGLWidgetListItem(&(widget->children), info->scrollbar);
    }
    if (info->header != NULL) {
	Close_Widget(&(info->header));
	DelGLWidgetListItem(&(widget->children), info->header);
    }

    y = b->y + ROW_HEIGHT;
    for (row = widget->children; row; row = row->next) {
	rb.x = b->x;
	rb.y = y;
	rb.w = b->w - 10;
	rb.h = ROW_HEIGHT;
	SetBounds_GLWidget(row, &rb);
	y += ROW_HEIGHT;
    }

    if (table_height > b->h) {
	info->scrollbar = 
	    Init_ScrollbarWidget(false, 0.0f, ((GLfloat)b->h) / table_height, 
				 SB_VERTICAL, Scroll_MetaTableWidget, widget);
	if (info->scrollbar != NULL) {
	    wb = &(widget->bounds);
	    sb.x = wb->x + wb->w - 10;
	    sb.y = wb->y + ROW_HEIGHT; 
	    sb.w = 10;
	    sb.h = wb->h - ROW_HEIGHT;
	    SetBounds_GLWidget(info->scrollbar, &sb);
	    AppendGLWidgetList(&(widget->children), info->scrollbar);
	} else {
	    error("failed to create a scroll bar for meta table");
	    return;
	}
    }
    info->header = Init_MetaHeaderWidget();
    if (info->header != NULL) {
	hb.x = widget->bounds.x;
	hb.y = widget->bounds.y - 1; /* don't ask why */
	hb.w = widget->bounds.w - 10;
	hb.h = ROW_HEIGHT;
	SetBounds_GLWidget(info->header, &hb);
	AppendGLWidgetList(&(widget->children), info->header);
    } else {
	error("failed to create a header for meta table");
	return;
    }
}

static GLWidget *Init_MetaTableWidget(list_t servers)
{
    GLWidget *tmp, *row;
    list_iter_t iter;
    server_info_t *sip;
    MetaTableWidget *info;
    bool bg = true;

    if (!(tmp = Init_EmptyBaseGLWidget())) {
        error("Widget init failed");
	return NULL;
    }
    if (!(info = (MetaTableWidget*)malloc(sizeof(MetaTableWidget)))) {
	error("out of memory");
	free(tmp);
	return NULL;
    }
    info->server_list   = servers;
    info->scrollbar     = NULL;
    info->selected      = NULL;

    tmp->wid_info       = info;
    tmp->WIDGET     	= METATABLEWIDGET;
    tmp->SetBounds      = SetBounds_MetaTableWidget;

    for (iter = List_begin(servers); 
	 iter != List_end(servers); 
	 LI_FORWARD(iter)) {
	sip = SI_DATA(iter);
	row = Init_MetaRowWidget(sip, info, !info->selected,
				 bg ? ROW_BG1 : ROW_BG2);
	if (!row) break;
	if (!info->selected) 
	    info->selected = (MetaRowWidget*)row->wid_info;
	AppendGLWidgetList(&(tmp->children), row);
	bg = !bg;
    }

    return tmp;
}

static GLWidget *Init_MetaWidget(list_t servers)
{
    GLWidget *tmp, *table;
    SDL_Rect table_bounds;

    if (!(tmp = Init_EmptyBaseGLWidget())) {
        error("Widget init failed");
	return NULL;
    }
    tmp->WIDGET     	= METAWIDGET;
    tmp->bounds.x   	= (draw_width - META_WIDTH) / 2;
    tmp->bounds.y   	= (draw_height - META_HEIGHT) / 2;
    tmp->bounds.w       = META_WIDTH;
    tmp->bounds.h       = META_HEIGHT;

    if (!(table = Init_MetaTableWidget(servers))) {
	free(tmp);
	return NULL;
    }
    table_bounds.x = tmp->bounds.x + 10;
    table_bounds.y = tmp->bounds.y + 10;
    table_bounds.w = tmp->bounds.w - 2*10;
    table_bounds.h = tmp->bounds.h - 3*10 - 30;
    SetBounds_GLWidget(table, &table_bounds);

    AppendGLWidgetList(&(tmp->children), table);   
    return tmp;
}

static bool join_server(Connect_param_t *conpar, server_info_t *sip)
{
    char *server_addr_ptr = conpar->server_addr;
    strlcpy(conpar->server_name, sip->hostname,
            sizeof(conpar->server_name));
    strlcpy(conpar->server_addr, sip->ip_str, sizeof(conpar->server_addr));
    conpar->contact_port = sip->port;
    if (Contact_servers(1, &server_addr_ptr, 1, 0, 0, NULL,
			0, NULL, NULL, NULL, NULL, conpar)) {
	return true;
    }
    printf("Server %s (%s) didn't respond on port %d\n",
	   conpar->server_name, conpar->server_addr,
	   conpar->contact_port);
    return false;
}

int Meta_window(Connect_param_t *conpar)
{
    static char err[MSG_LEN] = {0};
    int num_serv = 0;
    server_info_t *sip;
    GLWidget *meta, *target = NULL;
    SDL_Event evt;
    
    if (!server_list ||
	List_size(server_list) < 10 ||
	server_list_creation_time + 5 < time(NULL)) {
	
	Delete_server_list();
	if ((num_serv = Get_meta_data(err)) <= 0) {
	    fprintf(stderr, "Error: couldnt get meta list\n");
	    return -1;
	} else {
	    printf("xpilot_sdl: Got %d servers\n",num_serv);
	}
    }
    
    if (Welcome_sort_server_list() == -1) {
	Delete_server_list();
	error("out of memory");
	return -1;
    }
    
    meta = Init_MetaWidget(server_list);
    if (!meta) return -1;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, draw_width, draw_height, 0);
    glDisable(GL_BLEND);
    
    while(1) {

	set_alphacolor(blackRGBA);
	glBegin(GL_QUADS);
	glVertex2i(0,0);
	glVertex2i(draw_width,0);
	glVertex2i(draw_width,draw_height);
	glVertex2i(0,draw_height);
	glEnd();
	glEnable(GL_SCISSOR_TEST);
	DrawGLWidgetsi(meta, 0, 0, draw_width, draw_height);
	glDisable(GL_SCISSOR_TEST);
	SDL_GL_SwapBuffers();
	
	SDL_WaitEvent(&evt);
	do {
	    
	    switch(evt.type) {
	    case SDL_QUIT: 
		return -1;
		
	    case SDL_USEREVENT:
		if (join_server(conpar, (server_info_t*)evt.user.data1)) {
		    glEnable(GL_BLEND);
		    glMatrixMode(GL_PROJECTION);
		    glLoadIdentity();
		    gluOrtho2D(0, draw_width, 0, draw_height);
		    return 0;
		}
		break;
		
	    case SDL_MOUSEBUTTONDOWN:
		target = FindGLWidgeti(meta, evt.button.x, evt.button.y);
		if (target && target->button)
		    target->button(evt.button.button, 
				   evt.button.state,
				   evt.button.x,
				   evt.button.y,
				   target->buttondata);
		break;
		
	    case SDL_MOUSEBUTTONUP:
		if (target && target->button) {
		    target->button(evt.button.button, 
				   evt.button.state,
				   evt.button.x,
				   evt.button.y,
				   target->buttondata);
		    target = NULL;
		}
		break;
		
	    case SDL_MOUSEMOTION:
		if (target && target->motion)
		    target->motion(evt.motion.xrel,
				   evt.motion.yrel,
				   evt.button.x,
				   evt.button.y,
				   target->motiondata);
		break;

	    case SDL_VIDEOEXPOSE:
		glDisable(GL_SCISSOR_TEST);
		set_alphacolor(blackRGBA);
		glBegin(GL_QUADS);
		glVertex2i(0,0);
		glVertex2i(draw_width,0);
		glVertex2i(draw_width,draw_height);
		glVertex2i(0,draw_height);
		glEnd();
		glEnable(GL_SCISSOR_TEST);
		break;
	    }
	} while (SDL_PollEvent(&evt));
    }	
}
