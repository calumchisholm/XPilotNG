#include "xpclient.h"
#include "sdlmeta.h"
#include "sdlwindow.h"
#include "text.h"
#include "glwidgets.h"


GLWidget *MetaWidget = NULL;

struct Label {
  const char *label;
  int commas;
  int yoff;
  int height;
};
    
struct Label labels[] = {
	{"Server", 0, 0, 0},
	{"IP:Port", 0, 0, 0},
	{"Version", 0, 0, 0},
	{"Users", 0, 0, 0},
	{"Map name", 0, 0, 0},
	{"Map size", 0, 0, 0},
	{"Map author", 0, 0, 0},
	{"Status", 0, 0, 0},
	{"Bases", 0, 0, 0},
	{"Teambases", 0, 0, 0},
	{"Free bases", 0, 0, 0},
	{"Queued players", 0, 0, 0},
	{"FPS", 0, 0, 0},
	{"Sound", 0, 0, 0},
	{"Timing", 0, 0, 0},
	{"Playlist", 1, 0, 0}
    };

/*
 * An array of structures with information to join a local server.
 */
static Connect_param_t *global_conpar;
static Connect_param_t *localnet_conpars;
static server_info_t *global_sip;

static const char player_header[] = "Pl";
static const char queue_header[] = "Q";
static const char bases_header[] = "Ba";
static const char team_header[] = "Tm";
static const char fps_header[] = "FPS";
static const char status_header[] = "Stat";
static const char version_header[] = "Version";
static const char map_header[] = "Map Name";
static const char server_header[] = "Server";
static const char ping_header[] = "Ping";
static const char stat_header[] = "Status";


#define SELECTED_BG 0x009000ff
#define ROW_BG1 0x0000a0ff
#define ROW_BG2 0x000070ff
#define ROW_HEIGHT 20
#define VERSION_WIDTH 100
#define COUNT_WIDTH 20
#define META_WIDTH 800
#define META_HEIGHT (30 * ROW_HEIGHT + 3*10 + 30)

#define METAWIDGET 100
#define METATABLEWIDGET 101
#define METAROWWIDGET 102

typedef struct {
    list_t                server_list;
    GLWidget	          *scrollbar;
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
    row->fg             = 0xffff00ff;
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
    y = widget->bounds.y 
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
    SDL_Rect *wb, sb, rb;

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

    y = b->y;
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
	    sb = info->scrollbar->bounds;
	    sb.x = wb->x + wb->w - 10;
	    sb.y = wb->y; 
	    sb.w = 10;
	    sb.h = wb->h;
	    SetBounds_GLWidget(info->scrollbar, &sb);
	    AppendGLWidgetList(&(widget->children), info->scrollbar);
	} else {
	    error("failed to make a scroll bar for meta table");
	    return;
	}
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
  set_alphacolor(blackRGBA);
  glBegin(GL_QUADS);
  glVertex2i(0,0);
  glVertex2i(draw_width,0);
  glVertex2i(draw_width,draw_height);
  glVertex2i(0,draw_height);
  glEnd();

  while(1) {

      glEnable(GL_SCISSOR_TEST);
      DrawGLWidgetsi(meta, 0, 0, draw_width, draw_height);
      glDisable(GL_SCISSOR_TEST);
      SDL_GL_SwapBuffers();
      
      if (!SDL_WaitEvent(&evt)) {
	  error("error while waiting for events");
	  return -1;
      }

      switch(evt.type) {
      case SDL_QUIT: 
	  return -1;

      case SDL_USEREVENT:
	  if (join_server(conpar, (server_info_t*)evt.user.data1)) {
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
      }
  }
}
